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

// Binding for exposing EBGeometry's signed distance functions to Chombo
template <class T, class BV, int K>
class ChomboSDF : public BaseIF
{
public:
  ChomboSDF() = delete;

  ChomboSDF(const std::string a_filename)
  {
    m_rootNode = EBGeometry::Parser::readIntoLinearBVH<T, BV, K>(a_filename);
  }

  ChomboSDF(const ChomboSDF& a_other)
  {
    m_rootNode = a_other.m_rootNode;
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

    return Real(m_rootNode->signedDistance(m_rootNode->transformPoint(p)));
  }

  BaseIF*
  newImplicitFunction() const
  {
    return (BaseIF*)(new ChomboSDF(*this));
  }

protected:
  std::shared_ptr<EBGeometry::BVH::LinearBVH<T, EBGeometry::DCEL::FaceT<T>, BV, K>> m_rootNode;
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

  int nCells    = 128;
  int whichGeom = 0;
  int gridSize  = 16;
  pp.query("which_geom", whichGeom);
  pp.query("n_cells", nCells);
  pp.query("grid_size", gridSize);

  RealVect    loCorner;
  RealVect    hiCorner;
  std::string filename;

  if (whichGeom == 0) { // Airfoil
    loCorner = -50 * RealVect::Unit;
    hiCorner = 250 * RealVect::Unit;

    filename = "../Resources/airfoil.stl";
  }
  else if (whichGeom == 1) { // Sphere
    loCorner = -400 * RealVect::Unit;
    hiCorner = 400 * RealVect::Unit;

    filename = "../Resources/sphere.stl";
  }
  else if (whichGeom == 2) { // Dodecahedron
    loCorner = -2 * RealVect::Unit;
    hiCorner = 2 * RealVect::Unit;

    filename = "../Resources/dodecahedron.stl";
  }
  else if (whichGeom == 3) { // Horse
    loCorner = -0.12 * RealVect::Unit;
    hiCorner = 0.12 * RealVect::Unit;

    filename = "../Resources/horse.stl";
  }
  else if (whichGeom == 4) { // Porsche
    loCorner = -10 * RealVect::Unit;
    hiCorner = 10 * RealVect::Unit;

    filename = "../Resources/porsche.stl";
  }
  else if (whichGeom == 5) { // Orion
    loCorner = -10 * RealVect::Unit;
    hiCorner = 10 * RealVect::Unit;

    filename = "../Resources/orion.stl";
  }
  else if (whichGeom == 6) { // Armadillo
    loCorner = -125 * RealVect::Unit;
    hiCorner = 125 * RealVect::Unit;

    filename = "../Resources/armadillo.stl";
  }

  auto impFunc = static_cast<BaseIF*>(new ChomboSDF<T, BV, K>(filename));

  // Set up the Chombo EB geometry.
  ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
  const Real    dx = (hiCorner[0] - loCorner[0]) / nCells;
  ;

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
