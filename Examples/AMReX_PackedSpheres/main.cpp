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
using Prim = EBGeometry::SphereSDF<T>;

constexpr int K         = 4;
constexpr int M         = 20;
constexpr T   dx        = 0.5;
constexpr T   Rmin      = 1;
constexpr T   Rmax      = 6;
constexpr T   smoothLen = 0.5 * Rmin;

// Packed spheres geometry, using a BVH accelerator for the CSG union
class PackedSpheres
{
public:
  PackedSpheres(const bool use_bvh)
  {
    m_useBVH = use_bvh;

    // Generate some spheres.
    std::vector<std::shared_ptr<Prim>> spheres;

    // Use a fixed seed = 0 so that every MPI rank agrees on how to randomize the spheres.
    std::mt19937_64                   rng(0);
    std::uniform_real_distribution<T> udist(0, 1.0);

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++) {
        for (int k = 0; k < M; k++) {
          const T R = Rmin + udist(rng) * (Rmax - Rmin);

          const Vec3 center(0.5 * dx + i * (dx + Rmax), 0.5 * dx + j * (dx + Rmax), 0.5 * dx + k * (dx + Rmax));

          spheres.emplace_back(std::make_shared<Prim>(center, R));
        }
      }
    }

    // Create the standard and fast CSG unions.
    m_slowUnion = EBGeometry::SmoothUnion<T, Prim>(spheres, smoothLen);
    m_fastUnion = EBGeometry::FastSmoothUnion<T, Prim, BV, K>(
      spheres,
      [](const std::shared_ptr<const Prim>& a_sphere) {
        const Vec3 lo = a_sphere->getCenter() - a_sphere->getRadius() * Vec3::one();
        const Vec3 hi = a_sphere->getCenter() + a_sphere->getRadius() * Vec3::one();

        return BV(lo, hi);
      },
      smoothLen);

    // AMReX uses the opposite to EBGeometry.
    m_slowUnion = EBGeometry::Complement<T>(m_slowUnion);
    m_fastUnion = EBGeometry::Complement<T>(m_fastUnion);
  }

  PackedSpheres(const PackedSpheres& a_other)
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
  bool                                             m_useBVH;
  std::shared_ptr<EBGeometry::ImplicitFunction<T>> m_slowUnion;
  std::shared_ptr<EBGeometry::ImplicitFunction<T>> m_fastUnion;
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
    RealBox rb =
      RealBox({-Rmax - dx, -Rmax - dx, -Rmax - dx}, {M * (dx + Rmax) + dx, M * (dx + Rmax) + dx, M * (dx + Rmax) + dx});

    Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
    Geometry::Setup(&rb, 0, is_periodic.data());
    Box domain(IntVect(0), IntVect(n_cell - 1));
    geom.define(domain);
  }

  PackedSpheres packedSpheres = PackedSpheres(use_bvh);

  auto gshop = EB2::makeShop(packedSpheres);
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
