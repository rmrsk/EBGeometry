// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// The classic CSG solid, a cube rounded by a ball and drilled along each axis, compiled into a Tape and handed
// to AMReX's EB2 grid generation as an implicit function. The same code runs on CPUs and, in a
// CUDA or HIP build of AMReX, on GPUs. See README.md and the BasicCSG example under Examples/.

#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

#include "../../../EBGeometry.hpp"

#include <vector>

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This integration supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
using Tape = EBGeometry::Tape<T>;

// AMReX's implicit function. EBGeometry's value is negative inside the object and AMReX reads negative
// values as fluid, so the sign is flipped: the fluid is around the object. Deriving from GPUable lets EB2
// evaluate it in device kernels. It holds the object twice, as a view of the host pool and of its device
// mirror (the same object in a CPU build), and each compilation pass uses its own.
template <class S>
class ShapeIF : public GPUable
{
public:
  ShapeIF(const S& a_host, const S& a_device) noexcept : m_host(a_host), m_device(a_device)
  {}

  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return -m_device.signedDistance(Vec3(x, y, z));
#else
    return -m_host.signedDistance(Vec3(x, y, z));
#endif
  }

  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

private:
  S m_host;
  S m_device;
};

// Build AMReX's EB from the implicit function on a cubic domain around a box, and write a plot file.
template <class S>
void
buildEB(const ShapeIF<S>& a_sdf, const AABB& a_box, const int a_numCells, const int a_maxGridSize)
{
  const Vec3 centre = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3 extent = a_box.getHighCorner() - a_box.getLowCorner();
  const T    half   = T(0.55) * extent[extent.maxDir(true)];

  const RealBox                    rb({AMREX_D_DECL(centre[0] - half, centre[1] - half, centre[2] - half)},
                                      {AMREX_D_DECL(centre[0] + half, centre[1] + half, centre[2] + half)});
  const Array<int, AMREX_SPACEDIM> periodic{AMREX_D_DECL(0, 0, 0)};
  const Geometry                   geom(Box(IntVect(0), IntVect(a_numCells - 1)), rb, 0, periodic);

  // A mesh's fine features can cut a cell edge more than twice, which EB2 refuses unless it may cover such
  // cells; allow it unless the inputs say otherwise.
  ParmParse eb2("eb2");
  if (!eb2.contains("cover_multiple_cuts")) {
    eb2.add("cover_multiple_cuts", 1);
  }

  auto gshop = EB2::makeShop(a_sdf);
  EB2::Build(gshop, geom, 0, 0);

  BoxArray grids(geom.Domain());
  grids.maxSize(a_maxGridSize);
  const DistributionMapping dm(grids);

  const auto factory = makeEBFabFactory(geom, grids, dm, {2, 2, 2}, EBSupport::full);
  MultiFab   mf(grids, dm, 1, 0, MFInfo(), *factory);

  mf.setVal(1.0);
  EB_WriteSingleLevelPlotfile("plt", mf, {"rho"}, geom, 0.0, 0);

  // The EB index space keeps a copy of the implicit function, which must not outlive the pool.
  EB2::IndexSpace::clear();
}

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);
  {
    int n_cell        = 128;
    int max_grid_size = 32;

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);

    const EBGeometry::BoxSDF<T>    cube(-Vec3::ones(), Vec3::ones());
    const EBGeometry::SphereSDF<T> ball(Vec3::zeros(), T(1.35));

    const std::vector<EBGeometry::Expr<T>> drills{
      EBGeometry::CylinderSDF<T>(T(-1.5) * Vec3(1, 0, 0), T(1.5) * Vec3(1, 0, 0), T(0.55)),
      EBGeometry::CylinderSDF<T>(T(-1.5) * Vec3(0, 1, 0), T(1.5) * Vec3(0, 1, 0), T(0.55)),
      EBGeometry::CylinderSDF<T>(T(-1.5) * Vec3(0, 0, 1), T(1.5) * Vec3(0, 0, 1), T(0.55))};

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Tape tape(pool, EBGeometry::Difference(EBGeometry::Intersection(cube, ball), EBGeometry::Union(drills)));

    // In a GPU build the pool is frozen and mirrored to device memory in one piece, and the object is
    // rebased onto the mirror. The pools must outlive the EB generation.
#if defined(AMREX_USE_GPU)
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
    const ShapeIF<Tape>    sdf(tape, tape.rebasedView(devicePool));
#else
    const ShapeIF<Tape> sdf(tape, tape);
#endif

    buildEB(sdf, tape.computeBoundingVolume(), n_cell, max_grid_size);
  }
  amrex::Finalize();
}
