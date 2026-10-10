// SPDX-FileCopyrightText: 2023 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Std includes
#include <memory>
#include <string>

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

// EBGeometry precision, BVH branching factor and SoA width. float is enough for the EB geometry,
// which needs only the sign of the implicit function and where it crosses the grid edges.
using T            = float;
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();
using SDF          = EBGeometry::TriMeshSDF<T, K, W>;

/*!
  @brief Chombo implicit function wrapping an EBGeometry TriMeshSDF.
  @details Holds the TriMeshSDF by value and the Pool its storage lives in by shared_ptr: Chombo copies
  implicit functions through newImplicitFunction(), and every copy shares the one pool, which lives as
  long as the last of them.
*/
class ChomboSDF : public BaseIF
{
public:
  ChomboSDF() = delete;

  /*!
    @brief Read a mesh into a TriMeshSDF.
    @param[in] a_filename Mesh file (STL, PLY, VTK or OBJ).
  */
  explicit ChomboSDF(const std::string& a_filename)
    : m_pool(std::make_shared<EBGeometry::Pool>(EBGeometry::hostMemoryResource())),
      m_sdf(EBGeometry::Parser::readIntoTriMeshSDF<T, K, W>(a_filename, *m_pool))
  {}

  /*!
    @brief Chombo's implicit function definition.
    @details EBGeometry's signed distance is negative inside the mesh. The sign is flipped so that the
    fluid is outside the mesh.
  */
  Real
  value(const RealVect& a_point) const override final
  {
    using Vec3 = EBGeometry::Vec3T<T>;

#if CH_SPACEDIM == 2
    const Vec3 p(static_cast<T>(a_point[0]), static_cast<T>(a_point[1]), T(0));
#else
    const Vec3 p(static_cast<T>(a_point[0]), static_cast<T>(a_point[1]), static_cast<T>(a_point[2]));
#endif

    return -Real(m_sdf.signedDistance(p));
  }

  BaseIF*
  newImplicitFunction() const override
  {
    return new ChomboSDF(*this);
  }

private:
  /*!
    @brief Pool holding the mesh's triangles and BVH, shared by every copy of this object.
  */
  std::shared_ptr<EBGeometry::Pool> m_pool;

  /*!
    @brief The signed distance function, resolving against m_pool.
  */
  SDF m_sdf;
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

    int nCells   = 128;
    int gridSize = 16;

    // Mesh file and the cube that holds it. The default is the armadillo from the common-3d-test-models
    // submodule (see the "Building and using" docs for how to fetch it); the path is relative to this
    // example's folder, where the executable is run.
    std::string filename = "../../../common-3d-test-models/data/armadillo.obj";
    Real        domainLo = -125;
    Real        domainHi = 125;

    pp.query("n_cells", nCells);
    pp.query("grid_size", gridSize);
    pp.query("filename", filename);
    pp.query("domain_lo", domainLo);
    pp.query("domain_hi", domainHi);

    const RealVect loCorner = domainLo * RealVect::Unit;
    const RealVect hiCorner = domainHi * RealVect::Unit;

    const ChomboSDF impFunc(filename);

    // Set up the Chombo EB geometry.
    ProblemDomain domain(IntVect::Zero, (nCells - 1) * IntVect::Unit);
    const Real    dx = (hiCorner[0] - loCorner[0]) / nCells;

    GeometryShop  workshop(impFunc, -1, dx * RealVect::Zero);
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
