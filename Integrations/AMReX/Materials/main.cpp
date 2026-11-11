// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// One subdomain of a domain handed to AMReX's EB2 grid generation, and its boundary conditions at the cut cells: a
// cable (a core in a jacket, in a box of air), its regions declared with Overlay(), and a domain of two subdomains,
// the air and the jacket, with three surface groups. AMReX cuts one implicit function, so one subdomain is
// cut (subdomain = 0 or 1); each cut cell is labeled with the surface group of the surface there (the boundary
// condition's id), its role for the domain (a boundary or an interface) and the region on the other side. The same
// code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs. See README.md and the Materials example under
// Examples/.

#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

#include "../../../EBGeometry.hpp"

#include <cmath>

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This integration supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

using T      = Real;
using Vec3   = EBGeometry::Vec3T<T>;
using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
using Expr   = EBGeometry::Expr<T>;
using Tape   = EBGeometry::Tape<T>;
using Domain = EBGeometry::Domain<T>;

// AMReX's implicit function: a subdomain's function, negative in the subdomain and zero on its boundary only (never
// on a contact between two of its own regions). The subdomain is where the code computes, which AMReX reads as fluid
// where the value is negative, so the sign is EBGeometry's own. It holds the domain (which holds its tape) twice, as a
// view of the host pool and of its device mirror (the same object in a CPU build), each compilation pass using its
// own. Deriving from GPUable lets EB2 and ParallelFor evaluate it in device kernels.
class SubdomainIF : public GPUable
{
public:
  SubdomainIF(const Domain& a_host, const Domain& a_device, const uint32_t a_subdomain) noexcept
    : m_host(a_host), m_device(a_device), m_subdomain(a_subdomain)
  {}

  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return domain().signedDistance(Vec3(x, y, z), m_subdomain);
  }

  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

  // The subdomain's surface at a point: its surface group (the boundary condition's id, -1 for none), its role for
  // the domain (1 a boundary, 2 an interface with another subdomain) and the region on the other side.
  AMREX_GPU_HOST_DEVICE
  void
  label(const Vec3& a_point, Real& a_condition, Real& a_role, Real& a_across) const noexcept
  {
    const EBGeometry::SurfaceRef<T> surface = domain().closestSurface(a_point, m_subdomain);

    const uint32_t group = domain().surfaceGroupOf(surface);
    const uint32_t mine  = domain().subdomains().subdomainOf(surface.m_ownerRegion);

    a_condition = (group == EBGeometry::SurfaceGroups::None) ? Real(-1) : Real(group);
    a_role      = Real(static_cast<int>(domain().role(surface)));
    a_across    = Real((mine == m_subdomain) ? surface.m_neighborRegion : surface.m_ownerRegion);
  }

private:
  AMREX_GPU_HOST_DEVICE
  const Domain&
  domain() const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return m_device;
#else
    return m_host;
#endif
  }

  Domain   m_host;
  Domain   m_device;
  uint32_t m_subdomain;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);
  {
    int n_cell        = 128;
    int max_grid_size = 32;
    int subdomain     = 0;

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("subdomain", subdomain);

    // The regions: the air (1), the jacket (2) on top of it, then the core (3); beyond the air's box, the outside (0).
    const Expr air =
      EBGeometry::RegionName(EBGeometry::MaterialLabel(
                               EBGeometry::BoxSDF<T>(Vec3(T(-1.5), T(-1.5), T(-3)), Vec3(T(1.5), T(1.5), T(3))), "air"),
                             "air");
    const Expr jacket = EBGeometry::RegionName(
      EBGeometry::MaterialLabel(EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2)), Vec3(T(0), T(0), T(2)), T(1)),
                                "PTFE"),
      "jacket");
    const Expr core = EBGeometry::RegionName(
      EBGeometry::MaterialLabel(EBGeometry::CylinderSDF<T>(Vec3(T(0), T(0), T(-2.5)), Vec3(T(0), T(0), T(2.5)), T(0.4)),
                                "copper"),
      "core");
    const Expr object = EBGeometry::Overlay(air, jacket, core);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tape tape(pool, object);

    // The domain: two subdomains, the air (0) and the jacket (1), coupled through the jacket's outside; the core is
    // in neither. Its surface groups: every surface of the core (0), the air's box (1), and the interface of the air
    // and the jacket (2). Built in the tape's pool, for the tape.
    EBGeometry::DomainSetup setup;

    (void)setup.addSubdomain({1});
    (void)setup.addSubdomain({2});

    setup.add(setup.addSurfaceGroup(), EBGeometry::SurfaceSelector::region(3));
    setup.add(setup.addSurfaceGroup(), EBGeometry::SurfaceSelector::interface(1, 0));
    setup.add(setup.addSurfaceGroup(), EBGeometry::SurfaceSelector::interface(1, 2));

    const Domain domain(pool, tape, setup);

    // In a GPU build the pool is frozen and mirrored to device memory in one piece, and the domain (with its tape) is
    // rebased onto the mirror. The pools must outlive the EB generation.
#if defined(AMREX_USE_GPU)
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
    const SubdomainIF      sdf(domain, domain.rebasedView(devicePool), uint32_t(subdomain));
#else
    const SubdomainIF sdf(domain, domain, uint32_t(subdomain));
#endif

    // Cubic cells, n_cell along the object's longest side and 10% to spare; each side's count is rounded up to a
    // multiple of 8.
    const AABB box    = tape.computeBoundingVolume();
    const Vec3 center = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const Vec3 extent = T(1.1) * (box.getHighCorner() - box.getLowCorner());
    const T    h      = extent[extent.maxDir(true)] / T(n_cell);

    IntVect cells;
    RealBox rb;

    for (int d = 0; d < AMREX_SPACEDIM; d++) {
      cells[d] = 8 * int(std::ceil(extent[d] / (8 * h)));

      rb.setLo(d, center[d] - T(0.5) * h * T(cells[d]));
      rb.setHi(d, center[d] + T(0.5) * h * T(cells[d]));
    }

    const Array<int, AMREX_SPACEDIM> periodic{AMREX_D_DECL(0, 0, 0)};
    const Geometry                   geom(Box(IntVect(0), cells - 1), rb, 0, periodic);

    auto gshop = EB2::makeShop(sdf);
    EB2::Build(gshop, geom, 0, 0);

    BoxArray grids(geom.Domain());
    grids.maxSize(max_grid_size);
    const DistributionMapping dm(grids);

    const auto factory = makeEBFabFactory(geom, grids, dm, {2, 2, 2}, EBSupport::full);
    MultiFab   mf(grids, dm, 3, 0, MFInfo(), *factory);

    // Label each cut cell from the surface closest to its center; the other cells keep -1.
    mf.setVal(-1.0);

    const auto& flags = factory->getMultiEBCellFlagFab();
    const auto  plo   = geom.ProbLoArray();
    const auto  dx    = geom.CellSizeArray();

    for (MFIter mfi(mf); mfi.isValid(); ++mfi) {
      const Box& bx = mfi.validbox();

      if (flags[mfi].getType(bx) == FabType::singlevalued) {
        const auto cut    = flags.const_array(mfi);
        const auto labels = mf.array(mfi);

        ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) {
          if (cut(i, j, k).isSingleValued()) {
            const Vec3 p(
              plo[0] + (i + Real(0.5)) * dx[0], plo[1] + (j + Real(0.5)) * dx[1], plo[2] + (k + Real(0.5)) * dx[2]);

            sdf.label(p, labels(i, j, k, 0), labels(i, j, k, 1), labels(i, j, k, 2));
          }
        });
      }
    }

    EB_WriteSingleLevelPlotfile("plt", mf, {"condition", "role", "across"}, geom, 0.0, 0);

    // The EB index space keeps a copy of the implicit function, which must not outlive the pool.
    EB2::IndexSpace::clear();
  }
  amrex::Finalize();
}
