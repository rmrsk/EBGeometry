/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

// Std includes
#include <chrono>
#include <random>

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../EBGeometry.hpp"

using namespace amrex;

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using SDF  = EBGeometry::SignedDistanceFunction<T>;
using Prim = EBGeometry::BoxSDF<T>;

constexpr int K    = 4;
constexpr int M    = 50;
constexpr T   dx   = 0.1;
constexpr T   Wmin = 1;
constexpr T   Wmax = 6;
constexpr T   Lmin = 1;
constexpr T   Lmax = 6;
constexpr T   Hmin = 0.2;
constexpr T   Hmax = 20;

// City geometry, using a BVH accelerator for the CSG union of the buildings.
class City
{
public:
  City(const bool use_bvh)
  {
    m_useBVH = use_bvh;

    // Generate some random buildings on a lattice -- none of these should overlap.
    std::vector<std::shared_ptr<Prim>> buildings;

    // Use a fixed seed = 0 so that every MPI rank agrees on how to randomize the buildings.
    std::mt19937_64                   rng(0);
    std::uniform_real_distribution<T> udist(0, 1.0);
    std::normal_distribution<T>       ndist(0.5 * (Hmin + Hmax), sqrt(0.5 * (Hmin + Hmax)));

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++) {
        const T W = Wmin + udist(rng) * (Wmax - Wmin);
        const T L = Lmin + udist(rng) * (Lmax - Lmin);
        const T H = std::max(Hmin, std::min(Hmax, ndist(rng)));

        const T xLo = i * (Wmax + dx) + 0.5 * (dx + Wmax - W);
        const T xHi = xLo + W;

        const T yLo = j * (Lmax + dx) + 0.5 * (dx + Lmax - L);
        const T yHi = yLo + L;

        const T xs = 0.5 * (udist(rng) - 0.5) * (Wmax - W);
        const T ys = 0.5 * (udist(rng) - 0.5) * (Lmax - L);

        const Vec3 lo(xLo + xs, yLo + ys, 0.0);
        const Vec3 hi(xHi + xs, yHi + ys, H);

        buildings.emplace_back(std::make_shared<Prim>(lo, hi, false));
      }
    }

    m_slowUnion = std::make_shared<EBGeometry::Union<T, Prim>>(buildings, true);
    m_fastUnion = std::make_shared<EBGeometry::UnionBVH<T, Prim, BV, K>>(
      buildings, true, [](const std::shared_ptr<const Prim>& a_box) {
        return BV(a_box->getLowCorner(), a_box->getHighCorner());
      });
  }

  City(const City& a_other)
  {
    this->m_useBVH    = a_other.m_useBVH;
    this->m_slowUnion = a_other.m_slowUnion;
    this->m_fastUnion = a_other.m_fastUnion;
  }

  Real operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    if (m_useBVH) {
      return Real(m_fastUnion->value(EBGeometry::Vec3T(x, y, z)));
    }
    else {
      return Real(m_slowUnion->value(EBGeometry::Vec3T(x, y, z)));
    }
  };

  inline Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

protected:
  bool                                                  m_useBVH;
  std::shared_ptr<EBGeometry::Union<T, Prim>>           m_slowUnion;
  std::shared_ptr<EBGeometry::UnionBVH<T, Prim, BV, K>> m_fastUnion;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  bool use_bvh         = true;
  int  n_cell          = 128;
  int  max_grid_size   = 16;
  int  num_coarsen_opt = 0;

  std::string filename;

  // read parameters
  ParmParse pp;
  pp.query("bvh", use_bvh);
  pp.query("n_cell", n_cell);
  pp.query("max_grid_size", max_grid_size);
  pp.query("num_coarsen_opt", num_coarsen_opt);

  Geometry geom;
  {
    RealBox rb = RealBox({-Wmax, -Lmax, 0}, {M * (Wmax + dx) + Wmax, M * (Lmax + dx) + Lmax, (2 * Hmax)});

    Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
    Geometry::Setup(&rb, 0, is_periodic.data());
    Box domain(IntVect(0), IntVect(n_cell - 1));
    geom.define(domain);
  }

  City city = City(use_bvh);

  auto gshop = EB2::makeShop(city);
  EB2::Build(gshop, geom, 0, 0, true, true, num_coarsen_opt);

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

  amrex::Finalize();
}
