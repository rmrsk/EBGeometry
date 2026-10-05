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
using T            = amrex::Real;
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
    const EBGeometry::Vec3T<T> point(x, y, z);

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
