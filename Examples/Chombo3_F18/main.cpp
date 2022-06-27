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

#include <filesystem>

// Our includes
#include "EBGeometry.hpp"


constexpr size_t K = 4;
using T            = Real;
using Face         = EBGeometry::DCEL::FaceT<T>;
using Mesh         = EBGeometry::DCEL::MeshT<T>;
using BV           = EBGeometry::BoundingVolumes::AABBT<T>;
using Prim         = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
using Vec3         = EBGeometry::Vec3T<T>;

class F18 : public BaseIF
{
public:
  F18()
  {
    // Get all the STL files in this directory .
    std::vector<std::string> stlFiles;
    for (const auto& entry : std::filesystem::directory_iterator("../Objects/F18")) {
      const std::string f   = entry.path();
      const std::string ext = f.substr(f.find_last_of(".") + 1);

      if (ext == "stl" || ext == "STL") {
        stlFiles.emplace_back(f);
      }
    }

    // Read each STL file and make them into flattened BVH-trees. These STL files
    // have the normal vector pointing inwards so we flip it.
    std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> bvhDCEL;
    for (const auto& f : stlFiles) {
      const auto mesh = EBGeometry::Parser::readIntoDCEL<T>(f);
      mesh->flip();

      // Create the BVH.
      bvhDCEL.emplace_back(EBGeometry::DCEL::buildFlatBVH<T, BV, K>(mesh));
    }

    // Optimized, faster union which uses a BVH for bounding the BVHs.
    m_union = std::make_shared<EBGeometry::UnionBVH<T, Prim, BV, K>>(
      bvhDCEL, true, [](const std::shared_ptr<const Prim>& a_prim) -> BV { return a_prim->getBoundingVolume(); });    
  }

  F18(const F18& a_other)
  {
    this->m_union = a_other.m_union;
  }

  Real
  value(const RealVect& a_point) const override final
  {
    return Real(m_union->value(Vec3(a_point[0], a_point[1], a_point[2])));
  }

  BaseIF*
  newImplicitFunction() const
  {
    return (BaseIF*)(new F18(*this));
  }

protected:
  std::shared_ptr<EBGeometry::UnionBVH<T, Prim, BV, K>> m_union;  
};

int
main(int argc, char* argv[])
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif

  // Parse input file
  char*     inFile = argv[1];
  ParmParse pp(argc - 2, argv + 2, NULL, inFile);

  int nCells    = 256;
  int whichGeom = 0;
  int gridSize  = 16;
  pp.query("which_geom", whichGeom);
  pp.query("n_cells", nCells);
  pp.query("grid_size", gridSize);

  const RealVect loCorner = -3*RealVect::Unit;
  const RealVect hiCorner = 11*RealVect::Unit;

  auto impFunc = static_cast<BaseIF*> (new F18());

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
