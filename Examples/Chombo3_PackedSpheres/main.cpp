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
using Prim = EBGeometry::SphereSDF<T>;

constexpr int K    = 4;
constexpr int M    = 20;
constexpr T   dx   = 0.25;
constexpr T   Rmin = 1;
constexpr T   Rmax = 6;

class PackedSpheres : public BaseIF
{
public:
  PackedSpheres() = delete;

  PackedSpheres(const bool use_bvh)
  {
    m_useBVH = use_bvh;

    // Generate some spheres.
    std::vector<std::shared_ptr<Prim>> spheres;

    // Use a fixed seed = 0 so that every MPI rank agrees on how to randomize the buildings.
    std::mt19937_64                   rng(0);
    std::uniform_real_distribution<T> udist(0, 1.0);

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++) {
        for (int k = 0; k < M; k++) {
          const T R = Rmin + udist(rng) * (Rmax - Rmin);

          const Vec3 center(0.5 * dx + i * (dx + Rmax), 0.5 * dx + j * (dx + Rmax), 0.5 * dx + k * (dx + Rmax));

          spheres.emplace_back(std::make_shared<Prim>(center, R, false));
        }
      }
    }

    m_slowUnion = std::make_shared<EBGeometry::Union<T, Prim>>(spheres, true);
    m_fastUnion = std::make_shared<EBGeometry::UnionBVH<T, Prim, BV, K>>(
      spheres, true, [](const std::shared_ptr<const Prim>& a_sphere) {
        const Vec3 lo = a_sphere->getCenter() - a_sphere->getRadius() * Vec3::one();
        const Vec3 hi = a_sphere->getCenter() + a_sphere->getRadius() * Vec3::one();
        return BV(lo, hi);
      });
  }

  PackedSpheres(const PackedSpheres& a_other)
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
    return (BaseIF*)(new PackedSpheres(*this));
  }

protected:
  bool                                                  m_useBVH;
  std::shared_ptr<EBGeometry::Union<T, Prim>>           m_slowUnion;
  std::shared_ptr<EBGeometry::UnionBVH<T, Prim, BV, K>> m_fastUnion;
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

  bool useBVH    = true;
  int  nCells    = 128;
  int  whichGeom = 0;
  int  gridSize  = 16;

  pp.query("use_bvh", useBVH);
  pp.query("which_geom", whichGeom);
  pp.query("n_cells", nCells);
  pp.query("grid_size", gridSize);

  const RealVect loCorner = -(Rmax + dx) * RealVect::Unit;
  const RealVect hiCorner = (M * (Rmax + dx) + dx) * RealVect::Unit;

  BaseIF* impFunc = static_cast<BaseIF*>(new PackedSpheres(useBVH));

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
