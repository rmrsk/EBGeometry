// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Std includes
#include <algorithm>
#include <cmath>
#include <memory>
#include <random>
#include <utility>
#include <vector>

// Chombo includes
#include "BRMeshRefine.H"
#include "BaseIF.H"
#include "DisjointBoxLayout.H"
#include "EBAMRIO.H"
#include "EBCellFactory.H"
#include "EBISLayout.H"
#include "EBIndexSpace.H"
#include "EBLevelDataOps.H"
#include "GeometryShop.H"
#include "ParmParse.H"

// Our includes
#include "EBGeometry.hpp"

// EBGeometry precision and BVH branching factor.
using T            = Real;
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();

using Vec3     = EBGeometry::Vec3T<T>;
using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
using Building = EBGeometry::BoxSDF<T>;
using Union    = EBGeometry::BVHUnion<T, Building, K>;

// The buildings sit on an M x M lattice of lots, each Wmax + dx by Lmax + dx, so none overlap.
constexpr int M    = 10;
constexpr T   dx   = 0.25;
constexpr T   Wmin = 1;
constexpr T   Wmax = 2;
constexpr T   Lmin = 1;
constexpr T   Lmax = 2;
constexpr T   Hmin = 1;
constexpr T   Hmax = 2;

/*!
  @brief Chombo implicit function for a union of many buildings (boxes).
  @details Holds the union by value and the Pool its storage lives in by shared_ptr: Chombo copies
  implicit functions through newImplicitFunction(), and every copy shares the one pool, which lives as
  long as the last of them.

  With m_useBVH false, value() visits every building instead of letting the BVH skip the distant
  ones. The two give the same field; only the cost differs.
*/
class RandomCity : public BaseIF
{
public:
  RandomCity() = delete;

  /*!
    @brief Build the buildings and the union over them.
    @param[in] a_useBVH Use the BVH, or visit every building.
  */
  explicit RandomCity(const bool a_useBVH) : RandomCity(a_useBVH, makeBuildings())
  {}

  /*!
    @brief Chombo's implicit function definition.
    @details EBGeometry's signed distance is negative inside the buildings. The sign is flipped so
    that the fluid is the space around the buildings.
  */
  Real
  value(const RealVect& a_point) const override final
  {
#if CH_SPACEDIM == 2
    const Vec3 p(a_point[0], a_point[1], 0.0);
#else
    const Vec3 p(a_point[0], a_point[1], a_point[2]);
#endif

    return m_useBVH ? -m_union.signedDistance(p) : -visitAll(p);
  }

  BaseIF*
  newImplicitFunction() const override
  {
    return new RandomCity(*this);
  }

private:
  /*!
    @brief The buildings and their bounding boxes.
  */
  using Buildings = std::pair<std::vector<Building>, std::vector<AABB>>;

  /*!
    @brief Build the union over a_buildings in a new pool.
    @param[in] a_useBVH    Use the BVH, or visit every building.
    @param[in] a_buildings The buildings and their bounding boxes.
  */
  RandomCity(const bool a_useBVH, const Buildings& a_buildings)
    : m_pool(std::make_shared<EBGeometry::Pool>(EBGeometry::hostMemoryResource())),
      m_union(*m_pool, a_buildings.first, a_buildings.second),
      m_useBVH(a_useBVH)
  {}

  /*!
    @brief Generate random buildings on the lattice. A fixed seed makes every MPI rank build the same
    city.
    @return The buildings and their bounding boxes.
  */
  static Buildings
  makeBuildings()
  {
    Buildings buildings;

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

        buildings.first.emplace_back(lo, hi);
        buildings.second.emplace_back(lo, hi);
      }
    }

    return buildings;
  }

  /*!
    @brief The union without the BVH: the smallest value over every building.
    @param[in] a_point Query point.
    @return The same value as m_union.signedDistance(a_point).
  */
  T
  visitAll(const Vec3& a_point) const noexcept
  {
    T d = EBGeometry::Math::Limits<T>::infinity();

    for (const Building& building : m_union.getBVH().getPrimitives()) {
      d = EBGeometry::Math::min(d, building.signedDistance(a_point));
    }

    return d;
  }

  /*!
    @brief Pool holding the union's BVH and buildings, shared by every copy of this object.
  */
  std::shared_ptr<EBGeometry::Pool> m_pool;

  /*!
    @brief BVH-accelerated union of the buildings, resolving against m_pool.
  */
  Union m_union;

  /*!
    @brief Use the BVH, or visit every building.
  */
  bool m_useBVH;
};

int
main(int argc, char* argv[])
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif

  {
    // Parse input file
    char*     inFile = argv[1];
    ParmParse pp(argc - 2, argv + 2, NULL, inFile);

    bool useBVH   = true;
    int  nCells   = 128;
    int  gridSize = 16;

    pp.query("bvh", useBVH);
    pp.query("n_cells", nCells);
    pp.query("grid_size", gridSize);

    RealVect loCorner = -std::max(Wmax, Lmax) * RealVect::Unit;
    RealVect hiCorner = M * std::max(Wmax + dx, std::max(Lmax + dx, Hmax + dx)) * RealVect::Unit;
    loCorner[2]       = 0.0;

    const RandomCity impFunc(useBVH);

    // Set up the Chombo EB geometry.
    ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
    const Real    cellSize = (hiCorner[0] - loCorner[0]) / nCells;

    GeometryShop  workshop(impFunc, -1, cellSize * RealVect::Zero);
    EBIndexSpace* ebisPtr = Chombo_EBIS::instance();
    ebisPtr->define(domain, loCorner, cellSize, workshop, gridSize, -1);

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

        const RealVect pos        = loCorner + (iv + 0.5 * RealVect::Unit) * cellSize;
        fab.getFArrayBox()(iv, 0) = impFunc.value(pos);
      }
    }

    // Write to HDF5
    Vector<LevelData<EBCellFAB>*> amrData;
    amrData.push_back(&data);
    writeEBAMRname(&amrData, "example.hdf5");
  }

#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}
