// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Many objects in a BVH union, handed to AMReX's EB2 grid generation as an implicit function: spheres of
// random sizes at random places in a cylinder, joined by a smooth union (the fluid is the space between them),
// or a city of randomly sized buildings. The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on
// GPUs. See README.md and the PackedSpheres and ManyObjects examples under Examples/.

#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

#include "../../../EBGeometry.hpp"

#include <cmath>
#include <random>
#include <string>
#include <vector>

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This integration supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

using T        = Real;
using Vec3     = EBGeometry::Vec3T<T>;
using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
using Sphere   = EBGeometry::SphereSDF<T>;
using Packing  = EBGeometry::BVHSmoothUnion<T, Sphere, 4, EBGeometry::SmoothMinOp<T>>;
using Building = EBGeometry::BoxSDF<T>;
using City     = EBGeometry::BVHUnion<T, Building, 4>;

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

// Build AMReX's EB from the implicit function on a domain around a box, and write a plot file.
template <class S>
void
buildEB(const ShapeIF<S>& a_sdf, const AABB& a_box, const int a_numCells, const int a_maxGridSize)
{
  // Cubic cells, a_numCells along the box's longest side and 10% to spare; each side's count is rounded up
  // to a multiple of 8.
  const Vec3 center = T(0.5) * (a_box.getLowCorner() + a_box.getHighCorner());
  const Vec3 extent = T(1.1) * (a_box.getHighCorner() - a_box.getLowCorner());
  const T    dx     = extent[extent.maxDir(true)] / T(a_numCells);

  IntVect cells;
  RealBox rb;

  for (int d = 0; d < AMREX_SPACEDIM; d++) {
    cells[d] = 8 * int(std::ceil(extent[d] / (8 * dx)));

    rb.setLo(d, center[d] - T(0.5) * dx * T(cells[d]));
    rb.setHi(d, center[d] + T(0.5) * dx * T(cells[d]));
  }

  const Array<int, AMREX_SPACEDIM> periodic{AMREX_D_DECL(0, 0, 0)};
  const Geometry                   geom(Box(IntVect(0), cells - 1), rb, 0, periodic);

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

// Hand an object in a pool to EB2. In a GPU build the pool is frozen and mirrored to device memory in one piece,
// and the object is rebased onto the mirror. The pools must outlive the EB generation.
template <class S>
void
handOver(EBGeometry::Pool& a_pool, const S& a_object, const int a_numCells, const int a_maxGridSize)
{
#if defined(AMREX_USE_GPU)
  a_pool.freeze();

  const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(a_pool, EBGeometry::deviceMemoryResource());
  const ShapeIF<S>       sdf(a_object, a_object.rebasedView(devicePool));
#else
  const ShapeIF<S> sdf(a_object, a_object);
#endif

  buildEB(sdf, a_object.computeBoundingVolume(), a_numCells, a_maxGridSize);
}

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);
  {
    int         n_cell        = 128;
    int         max_grid_size = 32;
    std::string scene         = "spheres";
    int         num_spheres   = 3000;
    int         lots          = 20;

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("scene", scene);
    pp.query("num_spheres", num_spheres);
    pp.query("lots", lots);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    if (scene == "spheres") {
      // Rejection sampling: a center drawn in the box around a cylinder along x (radius 26, five times as
      // long) is kept only inside the cylinder; then the radius is drawn. A fixed seed makes every MPI rank
      // build the same packing.
      const T                           radius = T(26);
      std::mt19937_64                   rng(0);
      std::uniform_real_distribution<T> unit(T(-1), T(1));
      std::vector<Sphere>               spheres;
      std::vector<AABB>                 boxes;

      while (int(spheres.size()) < num_spheres) {
        const Vec3 center = radius * Vec3(T(2.5) * unit(rng), unit(rng), unit(rng));

        if (center[1] * center[1] + center[2] * center[2] <= radius * radius) {
          spheres.emplace_back(center, T(0.6) + T(1.2) * (unit(rng) + T(1)));
          boxes.push_back(spheres.back().computeBoundingVolume());
        }
      }

      handOver(pool, Packing(pool, spheres, boxes, T(1.5)), n_cell, max_grid_size);
    }
    else if (scene == "city") {
      // One building per lot, of random width, length and height, never overlapping its neighbors. A fixed
      // seed makes every MPI rank build the same city.
      std::mt19937_64                   rng(12345);
      std::uniform_real_distribution<T> unit(T(0), T(1));
      std::vector<Building>             buildings;
      std::vector<AABB>                 boxes;

      for (int i = 0; i < lots; i++) {
        for (int j = 0; j < lots; j++) {
          const Vec3 lo(T(3.1) * T(i), T(0), T(3.1) * T(j));
          const Vec3 hi = lo + Vec3(T(1) + T(2) * unit(rng), T(1) + T(19) * unit(rng), T(1) + T(2) * unit(rng));

          buildings.emplace_back(lo, hi);
          boxes.emplace_back(lo, hi);
        }
      }

      handOver(pool, City(pool, buildings, boxes), n_cell, max_grid_size);
    }
    else {
      amrex::Abort("Unknown scene '" + scene + "': use spheres or city");
    }
  }
  amrex::Finalize();
}
