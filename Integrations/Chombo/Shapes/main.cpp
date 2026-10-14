// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// This integration composes the analytic shapes, and a few transforms (Offset, Annular),
// with EBGeometry's expression builder, compiles the result into a Tape, and hands it to Chombo's
// GeometryShop as an implicit function.

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

#include <optional>

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;

// Binding for exposing a compiled EBGeometry object to Chombo. The tape is a trivially copyable handle
// into the Pool it was compiled into, which must outlive this object and every copy of it.
class EBGeometryIF : public BaseIF
{
public:
  EBGeometryIF() = delete;

  explicit EBGeometryIF(const EBGeometry::Tape<T>& a_tape) : m_tape(a_tape)
  {}

  EBGeometryIF(const EBGeometryIF& a_other) : m_tape(a_other.m_tape)
  {}

  Real
  value(const RealVect& a_point) const override final
  {
#if CH_SPACEDIM == 2
    Vec3 p(a_point[0], a_point[1], 0.0);
#else
    Vec3 p(a_point[0], a_point[1], a_point[2]);

#endif

    // EBGeometry's value is negative inside the shape. The sign is flipped here, not in the
    // expression, so that the fluid is the space around the shape.
    return -Real(m_tape.signedDistance(p));
  }

  BaseIF*
  newImplicitFunction() const
  {
    return (BaseIF*)(new EBGeometryIF(*this));
  }

protected:
  EBGeometry::Tape<T> m_tape;
};

int
main(int argc, char* argv[])
{

#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif

  // Set up domain.

  // Parse input file
  char*     inFile = argv[1];
  ParmParse pp(argc - 2, argv + 2, NULL, inFile);

  int nCells    = 128;
  int whichGeom = 0;
  int gridSize  = 16;
  pp.query("which_geom", whichGeom);
  pp.query("n_cells", nCells);
  pp.query("grid_size", gridSize);

  RealVect loCorner;
  RealVect hiCorner;

  std::optional<EBGeometry::Expr<T>> impFunc;
  if (whichGeom == 0) { // Sphere at origin.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = EBGeometry::SphereSDF<T>(Vec3::zeros(), T(0.5));
  }
  else if (whichGeom == 1) { // Plane.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = EBGeometry::PlaneSDF<T>(Vec3::zeros(), Vec3::ones());
  }
  else if (whichGeom == 2) { // Infinite cylinder.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = EBGeometry::InfiniteCylinderSDF<T>(Vec3::zeros(), T(0.1), 2);
  }
  else if (whichGeom == 3) { // Finite cylinder.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::CylinderSDF<T>(-Vec3::ones(), Vec3::ones(), 0.25);
  }
  else if (whichGeom == 4) { // Capsule.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::CapsuleSDF<T>(-Vec3::ones(), Vec3::ones(), 0.25);
  }
  else if (whichGeom == 5) { // Box.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::BoxSDF<T>(-Vec3::ones(), Vec3::ones());
  }
  else if (whichGeom == 6) { // Rounded box.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::BoxSDF<T>(-Vec3::ones(), Vec3::ones());
    impFunc = EBGeometry::Offset(*impFunc, 0.25);
  }
  else if (whichGeom == 7) { // Torus.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::TorusSDF<T>(Vec3::zeros(), 1.0, 0.25);
  }
  else if (whichGeom == 8) { // Infinite cone.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::InfiniteConeSDF<T>(Vec3(0.0, 1.0, 0.0), 30.0);
  }
  else if (whichGeom == 9) { // Finite cone.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = EBGeometry::ConeSDF<T>(Vec3(0.0, 1.0, 0.0), 2.0, 30);
  }
  if (whichGeom == 10) { // Spherical shell.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = EBGeometry::SphereSDF<T>(Vec3::zeros(), T(0.5));
    impFunc = EBGeometry::Annular(*impFunc, 0.1);
  }

  if (!impFunc) {
    MayDay::Error("which_geom must be between 0 and 10");
  }

  // Set up the Chombo EB geometry.
  ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
  const Real    dx = (hiCorner[0] - loCorner[0]) / nCells;

  // The tape is compiled into this Pool, which must outlive the EB generation below.
  EBGeometry::Pool          pool(EBGeometry::hostMemoryResource());
  const EBGeometry::Tape<T> tape(pool, *impFunc);

  auto          baseIF = (BaseIF*)(new EBGeometryIF(tape));
  GeometryShop  workshop(*baseIF, -1, dx * RealVect::Zero);
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
      fab.getFArrayBox()(iv, 0) = baseIF->value(pos);
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
