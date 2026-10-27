// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A city of randomly sized buildings in a BVH union, handed to Chombo's GeometryShop as an implicit
// function. See README.md and the ManyObjects
// example under Examples/.

#include "BRMeshRefine.H"
#include "BaseIF.H"
#include "DisjointBoxLayout.H"
#include "EBAMRIO.H"
#include "EBCellFactory.H"
#include "EBISLayout.H"
#include "EBIndexSpace.H"
#include "GeometryShop.H"
#include "ParmParse.H"

#include "EBGeometry.hpp"

#include <random>
#include <vector>

using T        = Real;
using Vec3     = EBGeometry::Vec3T<T>;
using AABB     = EBGeometry::BoundingVolumes::AABBT<T>;
using Building = EBGeometry::BoxSDF<T>;
using City     = EBGeometry::BVHUnion<T, Building, 4>;

// Chombo's implicit function. EBGeometry's value is negative inside the object; the sign is flipped so that
// the fluid is around the object. The object is a trivially copyable handle into a pool, which must outlive
// every copy of this function.
template <class S>
class ShapeIF : public BaseIF
{
public:
  explicit ShapeIF(const S& a_shape) : m_shape(a_shape)
  {}

  Real
  value(const RealVect& a_point) const override
  {
    return -Real(m_shape.signedDistance(Vec3(a_point[0], a_point[1], a_point[2])));
  }

  BaseIF*
  newImplicitFunction() const override
  {
    return new ShapeIF(m_shape);
  }

private:
  S m_shape;
};

// Build Chombo's EB from the implicit function on a cubic domain around a box, and write the implicit
// function's value in every cell to example.hdf5.
template <class S>
void
buildEB(const S& a_shape, const int a_numCells, const int a_gridSize)
{
  const AABB box    = a_shape.computeBoundingVolume();
  const Vec3 centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
  const Vec3 extent = box.getHighCorner() - box.getLowCorner();
  const T    half   = T(0.55) * extent[extent.maxDir(true)];

  const RealVect      lo(D_DECL(centre[0] - half, centre[1] - half, centre[2] - half));
  const Real          dx = 2 * half / a_numCells;
  const ProblemDomain domain(IntVect::Zero, (a_numCells - 1) * IntVect::Unit);

  const ShapeIF<S> implicitFunction(a_shape);
  GeometryShop     workshop(implicitFunction, -1, dx * RealVect::Unit);
  EBIndexSpace*    ebis = Chombo_EBIS::instance();
  ebis->define(domain, lo, dx, workshop, a_gridSize, -1);

  Vector<int> procs;
  Vector<Box> boxes;
  domainSplit(domain, boxes, a_gridSize, a_gridSize);
  mortonOrdering(boxes);
  LoadBalance(procs, boxes);
  const DisjointBoxLayout grids(boxes, procs);

  EBISLayout ebisl;
  ebis->fillEBISLayout(ebisl, grids, domain, 1);

  LevelData<EBCellFAB> data(grids, 1, IntVect::Zero, EBCellFactory(ebisl));

  for (DataIterator dit(grids); dit.ok(); ++dit) {
    FArrayBox& fab = data[dit()].getFArrayBox();

    for (BoxIterator bit(fab.box()); bit.ok(); ++bit) {
      fab(bit(), 0) = implicitFunction.value(lo + (bit() + 0.5 * RealVect::Unit) * dx);
    }
  }

  Vector<LevelData<EBCellFAB>*> levels(1, &data);
  writeEBAMRname(&levels, "example.hdf5");

  ebis->clear();
}

int
main(int argc, char* argv[])
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif
  {
    ParmParse pp(argc - 2, argv + 2, nullptr, argv[1]);

    int n_cells   = 128;
    int grid_size = 16;
    int lots      = 60;

    pp.query("n_cells", n_cells);
    pp.query("grid_size", grid_size);
    pp.query("lots", lots);

    // One building per lot, of random width, length and height, never overlapping its neighbours. A fixed
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

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const City city(pool, buildings, boxes);

    buildEB(city, n_cells, grid_size);
  }
#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}
