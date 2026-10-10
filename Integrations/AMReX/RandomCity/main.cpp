// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Std includes
#include <algorithm>
#include <cmath>
#include <random>
#include <type_traits>
#include <vector>

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../../EBGeometry.hpp"

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This example supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

// EBGeometry precision and BVH branching factor. K is the library's default, which never depends on
// compiler flags: a CUDA/HIP build compiles this file twice (a host pass and a device pass), and the
// union type must be identical in both.
using T            = Real;
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();

using Vec3     = EBGeometry::Vec3T<T>;
using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
using Building = EBGeometry::BoxSDF<T>;
using Union    = EBGeometry::BVHUnion<T, Building, K>;

// The buildings sit on an M x M lattice of lots, each Wmax + dx by Lmax + dx, so none overlap.
constexpr int M    = 50;
constexpr T   dx   = 0.1;
constexpr T   Wmin = 1;
constexpr T   Wmax = 6;
constexpr T   Lmin = 1;
constexpr T   Lmax = 6;
constexpr T   Hmin = 0.2;
constexpr T   Hmax = 20;

/*!
  @brief AMReX implicit function for a union of many buildings (boxes).
  @details Deriving from amrex::GPUable tells AMReX's EB2::GeometryShop that it may evaluate this
  functor inside device kernels. AMReX can still call it on the host as well, so it holds two
  descriptors of the same union: one resolving against host memory and one against a device mirror,
  and each compilation pass uses its own. In a CPU build both are the same host object. The class is
  trivially copyable, which is what lets AMReX capture it by value in a kernel; the pools the
  descriptors resolve against are owned by main() and must outlive every evaluation.

  With m_useBVH false, the functor visits every building instead of letting the BVH skip the distant
  ones. The two give the same field; only the cost differs.
*/
class CityIF : public amrex::GPUable
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_hostUnion   Union resolving against host memory.
    @param[in] a_deviceUnion The same union rebased onto a device mirror (a_hostUnion in a CPU build).
    @param[in] a_useBVH      Use the BVH, or visit every building.
  */
  CityIF(const Union& a_hostUnion, const Union& a_deviceUnion, const bool a_useBVH) noexcept
    : m_hostUnion(a_hostUnion), m_deviceUnion(a_deviceUnion), m_useBVH(a_useBVH)
  {}

  /*!
    @brief AMReX's implicit function definition, callable on host and device.
    @details EBGeometry's signed distance is negative inside the buildings, and AMReX reads negative
    values as fluid. The sign is flipped so that the fluid is the space around the buildings.
  */
  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    const Vec3 point(x, y, z);

#if defined(EBGEOMETRY_DEVICE_COMPILE)
    const Union& city = m_deviceUnion;
#else
    const Union& city = m_hostUnion;
#endif

    return m_useBVH ? -city.signedDistance(point) : -visitAll(city, point);
  }

  /*!
    @brief The same implicit function for AMReX's host-only code paths.
  */
  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

private:
  /*!
    @brief The union without the BVH: the smallest value over every building.
    @param[in] a_union Union whose buildings to visit.
    @param[in] a_point Query point.
    @return The same value as a_union.signedDistance(a_point).
  */
  AMREX_GPU_HOST_DEVICE
  static T
  visitAll(const Union& a_union, const Vec3& a_point) noexcept
  {
    T d = EBGeometry::Math::Limits<T>::infinity();

    for (const Building& building : a_union.getBVH().getPrimitives()) {
      d = EBGeometry::Math::min(d, building.signedDistance(a_point));
    }

    return d;
  }

  /*!
    @brief Union resolving against host memory.
  */
  Union m_hostUnion;

  /*!
    @brief Union resolving against device memory (m_hostUnion in a CPU build).
  */
  Union m_deviceUnion;

  /*!
    @brief Use the BVH, or visit every building.
  */
  bool m_useBVH;
};

static_assert(std::is_trivially_copyable_v<CityIF>, "CityIF must be trivially copyable");

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  {
    bool use_bvh         = true;
    int  n_cell          = 128;
    int  max_grid_size   = 16;
    int  num_coarsen_opt = 0;

    ParmParse pp;
    pp.query("bvh", use_bvh);
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("num_coarsen_opt", num_coarsen_opt);

    // Generate random buildings on the lattice, and their bounding boxes. A fixed seed makes every MPI
    // rank build the same city.
    std::vector<Building> buildings;
    std::vector<AABB>     boundingVolumes;

    std::mt19937_64                   rng(0);
    std::uniform_real_distribution<T> udist(0, 1.0);
    std::normal_distribution<T>       ndist(0.5 * (Hmin + Hmax), std::sqrt(0.5 * (Hmin + Hmax)));

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++) {
        const T W = Wmin + udist(rng) * (Wmax - Wmin);
        const T L = Lmin + udist(rng) * (Lmax - Lmin);
        const T H = std::clamp(ndist(rng), Hmin, Hmax);

        const T xLo = i * (Wmax + dx) + 0.5 * (dx + Wmax - W);
        const T xHi = xLo + W;

        const T yLo = j * (Lmax + dx) + 0.5 * (dx + Lmax - L);
        const T yHi = yLo + L;

        const T xs = 0.5 * (udist(rng) - 0.5) * (Wmax - W);
        const T ys = 0.5 * (udist(rng) - 0.5) * (Lmax - L);

        const Vec3 lo(xLo + xs, yLo + ys, 0.0);
        const Vec3 hi(xHi + xs, yHi + ys, H);

        buildings.emplace_back(lo, hi);
        boundingVolumes.emplace_back(lo, hi);
      }
    }

    // The pools own the BVH and building storage the union descriptors resolve against. They are
    // declared before, and so destroyed after, everything that evaluates the union.
    EBGeometry::Pool hostPool(EBGeometry::hostMemoryResource());

    const Union hostUnion(hostPool, buildings, boundingVolumes);

#if defined(AMREX_USE_GPU)
    // Copy the finished pool to the device in one piece, then rebase the union onto the copy.
    hostPool.freeze();

    EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());

    const Union deviceUnion = hostUnion.rebasedView(devicePool);
#else
    const Union& deviceUnion = hostUnion;
#endif

    {
      Geometry geom;
      {
        RealBox rb({-Wmax, -Lmax, 0}, {M * (Wmax + dx) + Wmax, M * (Lmax + dx) + Lmax, 2 * Hmax});

        Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
        Geometry::Setup(&rb, 0, is_periodic.data());
        Box domain(IntVect(0), IntVect(n_cell - 1));
        geom.define(domain);
      }

      const CityIF city(hostUnion, deviceUnion, use_bvh);

      auto gshop = EB2::makeShop(city);

      EB2::Build(gshop, geom, 0, 0, 1, true, true, num_coarsen_opt);

      // Put some data
      MultiFab mf;
      {
        BoxArray boxArray(geom.Domain());
        boxArray.maxSize(max_grid_size);
        DistributionMapping dm{boxArray};

        std::unique_ptr<EBFArrayBoxFactory> factory =
          amrex::makeEBFabFactory(geom, boxArray, dm, {2, 2, 2}, EBSupport::full);

        mf.define(boxArray, dm, 1, 0, MFInfo(), *factory);
        mf.setVal(1.0);
      }

      EB_WriteSingleLevelPlotfile("plt", mf, {"rho"}, geom, 0.0, 0);
    }

    // The EB index space keeps its own copy of gshop and may evaluate it again (e.g. to build finer
    // levels), so it must not outlive the pools. Everything that used it is gone by now.
    EB2::IndexSpace::clear();
  }

  amrex::Finalize();
}
