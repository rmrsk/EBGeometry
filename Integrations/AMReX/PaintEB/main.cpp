// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Std includes
#include <cstdint>
#include <string>
#include <type_traits>

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../../EBGeometry.hpp"

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This example supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

// EBGeometry precision and BVH branching factor. K is the library's default, which never depends on
// compiler flags: a CUDA/HIP build compiles this file twice (a host pass and a device pass), and the
// MeshSDF type must be identical in both. float is enough for the EB geometry; see the MeshSDF
// integration for why.
using T            = float;
constexpr size_t K = EBGeometry::BVH::DefaultBranchingRatio<T>();
using SDF          = EBGeometry::MeshSDF<T, K>;

/*!
  @brief AMReX implicit function wrapping an EBGeometry MeshSDF, which can also report the face
  closest to a point.
  @details Deriving from amrex::GPUable tells AMReX's EB2::GeometryShop that it may evaluate this
  functor inside device kernels. AMReX can still call it on the host as well, so it holds two
  descriptors of the same MeshSDF: one resolving against host memory and one against a device mirror,
  and each compilation pass uses its own. In a CPU build both are the same host object. The class is
  trivially copyable, which is what lets AMReX and amrex::ParallelFor capture it by value in a kernel;
  the pools the descriptors resolve against are owned by main() and must outlive every evaluation.
*/
class PaintIF : public amrex::GPUable
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_hostSDF   MeshSDF resolving against host memory.
    @param[in] a_deviceSDF The same MeshSDF rebased onto a device mirror (a_hostSDF in a CPU build).
  */
  PaintIF(const SDF& a_hostSDF, const SDF& a_deviceSDF) noexcept : m_hostSDF(a_hostSDF), m_deviceSDF(a_deviceSDF)
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
    return sdf().signedDistance(point(AMREX_D_DECL(x, y, z)));
  }

  /*!
    @brief The same implicit function for AMReX's host-only code paths.
  */
  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

  /*!
    @brief The id of the face closest to a point: its index in the mesh, which is the file's face order
    (less any zero-area faces the reader removed).
  */
  AMREX_GPU_HOST_DEVICE
  uint32_t
  getClosestFace(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return sdf().getClosestFace(point(AMREX_D_DECL(x, y, z))).faceId;
  }

private:
  /*!
    @brief The descriptor this compilation pass resolves against.
  */
  AMREX_GPU_HOST_DEVICE
  const SDF&
  sdf() const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return m_deviceSDF;
#else
    return m_hostSDF;
#endif
  }

  /*!
    @brief Narrow an AMReX position to EBGeometry's precision.
  */
  AMREX_GPU_HOST_DEVICE
  static EBGeometry::Vec3T<T>
  point(AMREX_D_DECL(Real x, Real y, Real z)) noexcept
  {
    return EBGeometry::Vec3T<T>(static_cast<T>(x), static_cast<T>(y), static_cast<T>(z));
  }

  /*!
    @brief MeshSDF resolving against host memory.
  */
  SDF m_hostSDF;

  /*!
    @brief MeshSDF resolving against device memory (m_hostSDF in a CPU build).
  */
  SDF m_deviceSDF;
};

static_assert(std::is_trivially_copyable_v<PaintIF>, "PaintIF must be trivially copyable");

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  {
    int n_cell          = 128;
    int max_grid_size   = 16;
    int num_coarsen_opt = 0;

    // Mesh file (STL/PLY/VTK/OBJ). Override with 'filename=<path>' in the inputs file. The default is a
    // small OBJ from the common-3d-test-models submodule (see the "Building and using" docs for how to
    // fetch it); the path is relative to this example's source folder, where the executable is run.
    std::string filename = "../../../common-3d-test-models/data/suzanne.obj";

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("num_coarsen_opt", num_coarsen_opt);
    pp.query("filename", filename);

    // The pools own the mesh and BVH storage the SDF descriptors resolve against. They are declared
    // before, and so destroyed after, everything that evaluates the SDF.
    EBGeometry::Pool hostPool(EBGeometry::hostMemoryResource());

    const SDF hostSDF = EBGeometry::Parser::readIntoMeshSDF<T, K>(filename, hostPool);

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
        RealBox rb({-5, -1, 2}, {0, 4, 6}); // bounds the default suzanne mesh

        Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
        Geometry::Setup(&rb, 0, is_periodic.data());
        Box domain(IntVect(0), IntVect(n_cell - 1));
        geom.define(domain);
      }

      const PaintIF sdf(hostSDF, deviceSDF);

      // Create the EB geometry
      auto gshop = EB2::makeShop(sdf);

      EB2::Build(gshop, geom, 0, 0, 1, true, true, num_coarsen_opt);

      // Create some data
      BoxArray boxArray(geom.Domain());
      boxArray.maxSize(max_grid_size);
      DistributionMapping dm{boxArray};

      std::unique_ptr<EBFArrayBoxFactory> factory =
        amrex::makeEBFabFactory(geom, boxArray, dm, {2, 2, 2}, EBSupport::full);

      MultiFab mf;
      mf.define(boxArray, dm, 1, 0, MFInfo(), *factory);
      mf.setVal(-1.0);

      const auto& ebFlags = factory->getMultiEBCellFlagFab();
      const auto  probLo  = geom.ProbLoArray();
      const auto  cellDx  = geom.CellSizeArray();

      // Paint each cut cell with the id of the face closest to its center. Regular and covered cells
      // keep -1.
      for (amrex::MFIter mfi(mf); mfi.isValid(); ++mfi) {
        const auto& bx       = mfi.validbox();
        const auto& mf_array = mf.array(mfi);
        const auto& flags    = ebFlags[mfi].getType(bx);

        if (flags == FabType::singlevalued) {
          const auto& cellFlags = ebFlags.const_array(mfi);

          amrex::ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) {
            if (cellFlags(i, j, k).isSingleValued()) {
              const Real x = probLo[0] + (i + 0.5) * cellDx[0];
              const Real y = probLo[1] + (j + 0.5) * cellDx[1];
              const Real z = probLo[2] + (k + 0.5) * cellDx[2];

              mf_array(i, j, k) = Real(sdf.getClosestFace(x, y, z));
            }
          });
        }
      }

      EB_WriteSingleLevelPlotfile("plt", mf, {"facet_id"}, geom, 0.0, 0);
    }

    // The EB index space keeps its own copy of gshop and may evaluate it again (e.g. to build finer
    // levels), so it must not outlive the pools. Everything that used it is gone by now.
    EB2::IndexSpace::clear();
  }

  amrex::Finalize();
}
