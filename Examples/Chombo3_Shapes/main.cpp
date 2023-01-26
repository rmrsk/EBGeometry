/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

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

using T       = Real;
using ImpFunc = EBGeometry::ImplicitFunction<T>;
using Vec3    = EBGeometry::Vec3T<T>;

// Binding for exposing EBGeometry's signed distance functions to Chombo
class EBGeometryIF : public BaseIF
{
public:
  EBGeometryIF() = delete;

  EBGeometryIF(std::shared_ptr<ImpFunc>& a_implicitFunction)
  {
    m_implicitFunction = a_implicitFunction;
  }

  EBGeometryIF(const EBGeometryIF& a_other)
  {
    m_implicitFunction = a_other.m_implicitFunction;
  }

  Real
  value(const RealVect& a_point) const override final
  {
#if CH_SPACEDIM == 2
    Vec3 p(a_point[0], a_point[1], 0.0);
#else
    Vec3 p(a_point[0], a_point[1], a_point[2]);

#endif

    return Real(m_implicitFunction->value(p));
  }

  BaseIF*
  newImplicitFunction() const
  {
    return (BaseIF*)(new EBGeometryIF(*this));
  }

protected:
  std::shared_ptr<ImpFunc> m_implicitFunction;
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

  std::shared_ptr<ImpFunc> impFunc = nullptr;
  if (whichGeom == 0) { // Sphere at origin.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::SphereSDF<T>>(Vec3::zero(), T(0.5));
  }
  else if (whichGeom == 1) { // Plane.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::PlaneSDF<T>>(Vec3::zero(), Vec3::one());
  }
  else if (whichGeom == 2) { // Infinite cylinder.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::InfiniteCylinderSDF<T>>(Vec3::zero(), T(0.1), 2);
  }
  else if (whichGeom == 3) { // Finite cylinder.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::CylinderSDF<T>>(-Vec3::one(), Vec3::one(), 0.25);
  }
  else if (whichGeom == 4) { // Capsule.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::CapsuleSDF<T>>(-Vec3::one(), Vec3::one(), 0.25);
  }
  else if (whichGeom == 5) { // Box.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::BoxSDF<T>>(-Vec3::one(), Vec3::one());
  }
  else if (whichGeom == 6) { // Rounded box.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::BoxSDF<T>>(-Vec3::one(), Vec3::one());
    impFunc = EBGeometry::Offset(impFunc, 0.25);
  }
  else if (whichGeom == 7) { // Torus.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::TorusSDF<T>>(Vec3::zero(), 1.0, 0.25);
  }
  else if (whichGeom == 8) { // Infinite cone.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::InfiniteConeSDF<T>>(Vec3(0.0, 0.0, 1.0), 30.0);
  }
  else if (whichGeom == 9) { // Finite cone.
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::ConeSDF<T>>(Vec3(0.0, 0.0, 1.0), 2.0, 30);
  }
  if (whichGeom == 10) { // Spherical shell.
    loCorner = -RealVect::Unit;
    hiCorner = RealVect::Unit;

    impFunc = std::make_shared<EBGeometry::SphereSDF<T>>(Vec3::zero(), T(0.5));
    impFunc = EBGeometry::Annular(impFunc, 0.1);
  }

  impFunc = EBGeometry::Complement(impFunc);

  // Set up the Chombo EB geometry.
  ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
  const Real    dx = (hiCorner[0] - loCorner[0]) / nCells;

  auto          baseIF = (BaseIF*)(new EBGeometryIF(impFunc));
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
