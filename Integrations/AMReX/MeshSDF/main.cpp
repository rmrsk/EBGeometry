// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Std includes
#include <string>
#include <type_traits>

// Our include
#include "../../../EBGeometry.hpp"

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This example supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

// EBGeometry precision, BVH branching factor and SoA width. K and W are the library's defaults, which
// never depend on compiler flags: a CUDA/HIP build compiles this file twice (a host pass and a device
// pass), and the TriMeshSDF type must be identical in both.
//
// T is EBGeometry's precision and is deliberately NOT amrex::Real. EBGeometry is templated on its
// floating-point type, so the geometry can be carried in float while AMReX runs in whatever precision
// it was configured for; EBGeometryIF::operator() converts at the boundary (AMReX hands in Real
// coordinates, EBGeometry answers in T, the result widens back to Real). float is the right choice
// here for two reasons:
//
//   * AMReX only consumes the *sign* of the implicit function plus the positions where it crosses
//     grid edges -- EB2::build_faces/build_cells test levelset(i,j,k) < 0 and take all sub-cell
//     geometry from the edge intercepts -- so float's ~7 digits is far more than the cut-cell
//     representation can carry. Measured against an otherwise identical double build on the
//     armadillo at n_cell=128: identical cell types, and volume fractions differing by at most
//     4.4e-4 (fcompare, max norm).
//   * Consumer GPUs run FP64 at a small fraction of FP32 (1/64 on Ampere GeForce), and the signed
//     distance query is the entire cost of EB generation here. Measured query throughput on one
//     RTX 3080 Ti over a 2.1M-node grid: 2.07 M queries/s in double against 32.25 M queries/s in
//     float, a 15.6x difference that carries straight through to EB2::Build (4.24 s -> 0.29 s).
//     On a host build the two precisions are within a few percent of each other, so this costs
//     nothing there.
//
// Use amrex::Real instead if you need the level set itself to be a double-precision distance field
// (EB2::Level::fillLevelSet) rather than only the EB geometry derived from it.
using T            = float;
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
constexpr size_t W = EBGeometry::TriangleSoA::DefaultWidth<T>();
using SDF          = EBGeometry::TriMeshSDF<T, K, W>;

/*!
  @brief AMReX implicit function wrapping an EBGeometry TriMeshSDF.
  @details Deriving from amrex::GPUable tells AMReX's EB2::GeometryShop that it may evaluate this
  functor inside device kernels. AMReX can still call it on the host as well (for example the CPU path
  of GeometryShop::getBoxType), so it holds two descriptors of the same TriMeshSDF: one resolving
  against host memory and one against a device mirror, and each compilation pass uses its own. In a
  CPU build both are the same host object. The class is trivially copyable, which is what lets AMReX
  capture it by value in a kernel; the pools the descriptors resolve against are owned by main() and
  must outlive every evaluation.
*/
class EBGeometryIF : public amrex::GPUable
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_hostSDF   TriMeshSDF resolving against host memory.
    @param[in] a_deviceSDF The same TriMeshSDF rebased onto a device mirror (a_hostSDF in a CPU build).
  */
  EBGeometryIF(const SDF& a_hostSDF, const SDF& a_deviceSDF) noexcept : m_hostSDF(a_hostSDF), m_deviceSDF(a_deviceSDF)
  {}

  /*!
    @brief AMReX's implicit function definition, callable on host and device.
    @details EBGeometry's signed distance is negative inside the mesh; AMReX reads negative values as
    fluid, so the fluid is inside the mesh.
  */
  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    // Narrow to EBGeometry's precision T here; the returned T widens back to amrex::Real on return.
    const EBGeometry::Vec3T<T> point(static_cast<T>(x), static_cast<T>(y), static_cast<T>(z));

#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return m_deviceSDF.signedDistance(point);
#else
    return m_hostSDF.signedDistance(point);
#endif
  }

  /*!
    @brief The same implicit function for AMReX's host-only code paths.
  */
  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

private:
  /*!
    @brief Signed distance function resolving against host memory.
  */
  SDF m_hostSDF;

  /*!
    @brief Signed distance function resolving against device memory (m_hostSDF in a CPU build).
  */
  SDF m_deviceSDF;
};

static_assert(std::is_trivially_copyable_v<EBGeometryIF>, "EBGeometryIF must be trivially copyable");

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  {
    int n_cell          = 128;
    int max_grid_size   = 16;
    int num_coarsen_opt = 4;

    // Mesh file (STL/PLY/VTK/OBJ). Override with 'filename=<path>' in the inputs file. The default is an
    // OBJ from the common-3d-test-models submodule (see the "Building and using" docs for how to fetch
    // it); the path is relative to this example's source folder, where the executable is run. Every
    // face must be a triangle.
    std::string filename = "../../../common-3d-test-models/data/armadillo.obj";

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("num_coarsen_opt", num_coarsen_opt);
    pp.query("filename", filename);

    // The pools own the mesh and BVH storage the SDF descriptors resolve against. They are declared
    // before, and so destroyed after, everything that evaluates the SDF.
    EBGeometry::Pool hostPool(EBGeometry::hostMemoryResource());

    const SDF hostSDF = EBGeometry::Parser::readIntoTriangleBVH<T, K, W>(filename, hostPool);

#if defined(AMREX_USE_GPU)
    // Copy the finished pool to the device in one piece, then rebase the SDF onto the copy.
    hostPool.freeze();

    EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(hostPool, EBGeometry::deviceMemoryResource());

    const SDF deviceSDF = hostSDF.rebasedView(devicePool);
#else
    const SDF& deviceSDF = hostSDF;
#endif

    {
      Geometry geom;
      {
        RealBox rb({-100, -75, -100}, {100, 125, 100}); // bounds the default armadillo mesh

        Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
        Geometry::Setup(&rb, 0, is_periodic.data());
        Box domain(IntVect(0), IntVect(n_cell - 1));
        geom.define(domain);
      }

      const EBGeometryIF sdf(hostSDF, deviceSDF);

      // Two ways to get a triangle mesh into AMReX's EB machinery. They are not equivalent, and the
      // difference is in what each one asks of the geometry rather than in how fast it goes.
      //
      // 1. This example: an EBGeometry TriMeshSDF wrapped as an implicit function for
      //    EB2::GeometryShop. GeometryShop sees a black-box scalar field, so everything it needs is
      //    derived from evaluating that field:
      //      * the nodal level set is filled by evaluating the signed distance at every node, which
      //        is a nearest-primitive search over the BVH (on the armadillo at K = W = 4 that is
      //        ~23 leaf visits and ~214 triangle tests per node);
      //      * GeometryShop::getBoxType classifies a box by evaluating every node in it and reducing
      //        the sign counts. Its CPU path returns as soon as it has seen both signs; its GPU path
      //        is a sum reduction and cannot exit early, so it always pays the full sweep;
      //      * edge intercepts come from BrentRootFinder bracketing the implicit function along each
      //        cut edge, which costs tens of evaluations per edge.
      //    In exchange you get a true signed distance field, every input format EBGeometry reads
      //    (STL, PLY, VTK, OBJ), and CSG unions and transforms over multiple objects.
      //
      // 2. AMReX's own STL path (eb2.geom_type=stl, eb2.stl_file=...), which never computes a
      //    distance at all. It decides which side of the surface a node is on by casting a segment to
      //    a reference point and counting triangle crossings, writes +/-1 into the level set, and
      //    intersects each cut edge with the triangles directly in closed form. That is much less
      //    work per node -- a one-dimensional query with no nearest-ness and no shrinking search
      //    radius -- but it requires a watertight mesh (parity is ill-defined otherwise, and a
      //    segment grazing an edge or vertex can flip it), reads only STL, and leaves behind a level
      //    set that is a sign rather than a distance.
      //
      // Measured on one RTX 3080 Ti at n_cell=128 over the same 99,976-triangle mesh, EB2::Build:
      // in a single-precision AMReX build ~0.23 s for this path against ~0.14 s for the STL path; in
      // a double-precision AMReX build ~0.29 s for this path against ~1.02 s for the STL path, since
      // STLtools is forced to amrex::Real while T above stays float either way. Mesh ingest --
      // parsing plus the BVH build, ~0.48 s here and independent of grid size -- is a separate fixed
      // cost paid before any of this; see EBGeometry issue #157.
      auto gshop = EB2::makeShop(sdf);

      EB2::Build(gshop, geom, 0, 0, 1, true, true, num_coarsen_opt);

      // Put some data
      MultiFab mf;
      {
        BoxArray boxArray(geom.Domain());
        boxArray.maxSize(max_grid_size);
        DistributionMapping dm{boxArray};

        std::unique_ptr<EBFArrayBoxFactory> factory =
          amrex::makeEBFabFactory(geom, boxArray, dm, {2, 2, 2}, EBSupport::full);

        mf.define(boxArray, dm, 1, 0, MFInfo(), *factory);
        mf.setVal(1.0);
      }

      EB_WriteSingleLevelPlotfile("plt", mf, {"rho"}, geom, 0.0, 0);
    }

    // The EB index space keeps its own copy of gshop and may evaluate it again (e.g. to build finer
    // levels), so it must not outlive the pools. Everything that used it is gone by now.
    EB2::IndexSpace::clear();
  }

  amrex::Finalize();
}
