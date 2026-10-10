// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Std includes
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

using Vec3   = EBGeometry::Vec3T<T>;
using AABB   = EBGeometry::BoundingVolumes::AABBT<T>;
using Sphere = EBGeometry::SphereSDF<T>;
using Blend  = EBGeometry::SmoothMinOp<T>;
using Union  = EBGeometry::BVHSmoothUnion<T, Sphere, K, Blend>;

// The spheres sit on an M x M x M lattice with spacing Rmax + dx, so neighbours may overlap.
constexpr int M         = 20;
constexpr T   dx        = 0.5;
constexpr T   Rmin      = 1;
constexpr T   Rmax      = 6;
constexpr T   smoothLen = 0.5 * Rmin;

/*!
  @brief Chombo implicit function for a smooth union of many spheres.
  @details Holds the union by value and the Pool its storage lives in by shared_ptr: Chombo copies
  implicit functions through newImplicitFunction(), and every copy shares the one pool, which lives as
  long as the last of them.

  With m_useBVH false, value() visits every sphere instead of letting the BVH skip the distant ones,
  and blends the two smallest values just as the BVH union does. The two give the same field; only
  the cost differs.
*/
class PackedSpheres : public BaseIF
{
public:
  PackedSpheres() = delete;

  /*!
    @brief Build the spheres and the union over them.
    @param[in] a_useBVH Use the BVH, or visit every sphere.
  */
  explicit PackedSpheres(const bool a_useBVH) : PackedSpheres(a_useBVH, makeSpheres())
  {}

  /*!
    @brief Chombo's implicit function definition.
    @details EBGeometry's signed distance is negative inside the spheres. The sign is flipped so that
    the fluid is the space between the spheres.
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
    return new PackedSpheres(*this);
  }

private:
  /*!
    @brief The spheres and their bounding boxes.
  */
  using Spheres = std::pair<std::vector<Sphere>, std::vector<AABB>>;

  /*!
    @brief Build the union over a_spheres in a new pool.
    @param[in] a_useBVH  Use the BVH, or visit every sphere.
    @param[in] a_spheres The spheres and their bounding boxes.
  */
  PackedSpheres(const bool a_useBVH, const Spheres& a_spheres)
    : m_pool(std::make_shared<EBGeometry::Pool>(EBGeometry::hostMemoryResource())),
      m_union(*m_pool, a_spheres.first, a_spheres.second, smoothLen),
      m_useBVH(a_useBVH)
  {}

  /*!
    @brief Generate the spheres. A fixed seed makes every MPI rank build the same spheres.
    @return The spheres and their bounding boxes.
  */
  static Spheres
  makeSpheres()
  {
    Spheres spheres;

    std::mt19937_64                   rng(0);
    std::uniform_real_distribution<T> udist(0, 1.0);

    for (int i = 0; i < M; i++) {
      for (int j = 0; j < M; j++) {
        for (int k = 0; k < M; k++) {
          const T R = Rmin + udist(rng) * (Rmax - Rmin);

          const Vec3 center(0.5 * dx + i * (dx + Rmax), 0.5 * dx + j * (dx + Rmax), 0.5 * dx + k * (dx + Rmax));

          spheres.first.emplace_back(center, R);
          spheres.second.emplace_back(center - R * Vec3::ones(), center + R * Vec3::ones());
        }
      }
    }

    return spheres;
  }

  /*!
    @brief The smooth union without the BVH: visit every sphere and blend the two smallest values.
    @param[in] a_point Query point.
    @return The same value as m_union.signedDistance(a_point).
  */
  T
  visitAll(const Vec3& a_point) const noexcept
  {
    T a = EBGeometry::Math::Limits<T>::infinity();
    T b = EBGeometry::Math::Limits<T>::infinity();

    for (const Sphere& sphere : m_union.getBVH().getPrimitives()) {
      const T d = sphere.signedDistance(a_point);

      if (d < a) {
        b = a;
        a = d;
      }
      else if (d < b) {
        b = d;
      }
    }

    return Blend()(a, b, smoothLen);
  }

  /*!
    @brief Pool holding the union's BVH and spheres, shared by every copy of this object.
  */
  std::shared_ptr<EBGeometry::Pool> m_pool;

  /*!
    @brief BVH-accelerated smooth union of the spheres, resolving against m_pool.
  */
  Union m_union;

  /*!
    @brief Use the BVH, or visit every sphere.
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

    const RealVect loCorner = -(Rmax + dx) * RealVect::Unit;
    const RealVect hiCorner = (M * (Rmax + dx) + dx) * RealVect::Unit;

    const PackedSpheres impFunc(useBVH);

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
