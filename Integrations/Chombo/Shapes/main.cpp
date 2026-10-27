// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// One analytic shape, a torus, handed to Chombo's GeometryShop as an implicit function. See README.md and the
// Shapes example under Examples/.

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

#include <cmath>

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;

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

// Build Chombo's EB from the implicit function on a domain around a box, and write the implicit
// function's value in every cell to example.hdf5.
template <class S>
void
buildEB(const S& a_shape, const int a_numCells, const int a_gridSize)
{
  // Cubic cells, a_numCells along the box's longest side and 10% to spare; each side's count is rounded up
  // to a multiple of the grid size.
  const AABB box    = a_shape.computeBoundingVolume();
  const Vec3 centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
  const Vec3 extent = T(1.1) * (box.getHighCorner() - box.getLowCorner());
  const Real dx     = extent[extent.maxDir(true)] / a_numCells;

  IntVect  cells;
  RealVect lo;

  for (int d = 0; d < SpaceDim; d++) {
    cells[d] = a_gridSize * int(std::ceil(extent[d] / (a_gridSize * dx)));
    lo[d]    = centre[d] - 0.5 * dx * cells[d];
  }

  const ProblemDomain domain(IntVect::Zero, cells - IntVect::Unit);

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

    pp.query("n_cells", n_cells);
    pp.query("grid_size", grid_size);

    const EBGeometry::TorusSDF<T> torus(Vec3::zeros(), T(1), T(0.25));

    buildEB(torus, n_cells, grid_size);
  }
#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}
