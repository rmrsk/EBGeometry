/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

// Std includes
#include <chrono>
#include <random>

// Chombo includes
#include "EBISLayout.H"
#include "DisjointBoxLayout.H"
#include "BaseIF.H"
#include "GeometryShop.H"
#include "ParmParse.H"
#include "EBIndexSpace.H"
#include "BRMeshRefine.H"
#include "EBCellFactory.H"
#include "EBLevelDataOps.H"
#include "EBAMRIO.H"

// Our includes
#include "EBGeometry.hpp"

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using SDF  = EBGeometry::SignedDistanceFunction<T>;
using Prim = EBGeometry::BoxSDF<T>;

constexpr int K    = 4;
constexpr int M    = 10;
constexpr T   dx   = 0.25;
constexpr T   Wmin = 1;
constexpr T   Wmax = 2;
constexpr T   Lmin = 1;
constexpr T   Lmax = 2;
constexpr T   Hmin = 1;
constexpr T   Hmax = 2;

class RandomCity : public BaseIF
{
public:
  RandomCity() = delete;

  RandomCity(const bool use_bvh)
  {
    m_useBVH = use_bvh;

    // Generate some random buildings on a lattice -- none of these should overlap.
    std::vector<std::shared_ptr<Prim>> buildings;
    std::vector<BV>                    boundingVolumes;

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

        buildings.emplace_back(std::make_shared<Prim>(lo, hi));
        boundingVolumes.emplace_back(BV(lo, hi));
      }
    }

    m_slowUnion = EBGeometry::Union<T, Prim>(buildings);
    m_fastUnion = EBGeometry::FastUnion<T, Prim, BV, K>(buildings, boundingVolumes);

    // AMReX uses the opposite sign for the value functions.
    m_slowUnion = EBGeometry::Complement<T>(m_slowUnion);
    m_fastUnion = EBGeometry::Complement<T>(m_fastUnion);
  }

  RandomCity(const RandomCity& a_other)
  {
    this->m_useBVH    = a_other.m_useBVH;
    this->m_slowUnion = a_other.m_slowUnion;
    this->m_fastUnion = a_other.m_fastUnion;
  }

  Real
  value(const RealVect& a_point) const override final
  {
    using Vec3 = EBGeometry::Vec3T<T>;

#if CH_SPACEDIM == 2
    Vec3 p(a_point[0], a_point[1], 0.0);
#else
    Vec3 p(a_point[0], a_point[1], a_point[2]);
#endif

    if (m_useBVH) {
      return Real(m_fastUnion->value(p));
    }
    else {
      return Real(m_slowUnion->value(p));
    }
  }

  BaseIF*
  newImplicitFunction() const
  {
    return (BaseIF*)(new RandomCity(*this));
  }

protected:
  bool                                             m_useBVH;
  std::shared_ptr<EBGeometry::ImplicitFunction<T>> m_slowUnion;
  std::shared_ptr<EBGeometry::ImplicitFunction<T>> m_fastUnion;
};

int
main(int argc, char* argv[])
{
  constexpr int K = 4;

  using T  = float;
  using BV = EBGeometry::BoundingVolumes::AABBT<T>;

#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif

  // Parse input file
  char*     inFile = argv[1];
  ParmParse pp(argc - 2, argv + 2, NULL, inFile);

  bool useBVH   = true;
  int  nCells   = 128;
  int  gridSize = 16;

  pp.query("use_bvh", useBVH);
  pp.query("n_cells", nCells);
  pp.query("grid_size", gridSize);

  RealVect loCorner = -std::max(Wmax, Lmax) * RealVect::Unit;
  RealVect hiCorner = M * std::max(Wmax + dx, std::max(Lmax + dx, Hmax + dx)) * RealVect::Unit;
  loCorner[2]       = 0.0;

  BaseIF* impFunc = static_cast<BaseIF*>(new RandomCity(useBVH));

  // Set up the Chombo EB geometry.
  ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
  const Real    dx = (hiCorner[0] - loCorner[0]) / nCells;

  GeometryShop  workshop(*impFunc, -1, dx * RealVect::Zero);
  EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
  ebisPtr->define(domain, loCorner, dx, workshop, gridSize, -1);

  // Set up the grids
  Vector<int> procs;
  Vector<Box> boxes;
  domainSplit(domain, boxes, gridSize, gridSize);
  mortonOrdering(boxes);
  LoadBalance(procs, boxes);
  DisjointBoxLayout dbl(boxes, procs);

  // Fill the EBIS layout
  EBISLayout ebisl;
  ebisPtr->fillEBISLayout(ebisl, dbl, domain, 1);

  // Allocate some data that we can output
  LevelData<EBCellFAB> data(dbl, 1, IntVect::Zero, EBCellFactory(ebisl));
  for (DataIterator dit(dbl); dit.ok(); ++dit) {
    EBCellFAB& fab = data[dit()];
    fab.setVal(0.0);

    const Box region = fab.getRegion();
    for (BoxIterator bit(region); bit.ok(); ++bit) {
      const IntVect iv = bit();

      const RealVect pos        = loCorner + (iv + 0.5 * RealVect::Unit) * dx;
      fab.getFArrayBox()(iv, 0) = impFunc->value(pos);
    }
  }

  // Write to HDF5
  Vector<LevelData<EBCellFAB>*> amrData;
  amrData.push_back(&data);
  writeEBAMRname(&amrData, "example.hdf5");

#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}
