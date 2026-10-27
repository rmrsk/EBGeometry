// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Painting the embedded boundary: a mesh handed to AMReX's EB2 grid generation as an implicit function, and
// each cut cell labelled with the id of the mesh face closest to it (getClosestFace()), the index into an
// array of per-face data such as a boundary condition or a material. The same code runs on CPUs and, in a
// CUDA or HIP build of AMReX, on GPUs. See README.md and the MeshSDF example under Examples/.

#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

#include "../../../EBGeometry.hpp"

#include <cmath>
#include <string>

using namespace amrex;

#if defined(AMREX_USE_GPU) && !defined(AMREX_USE_CUDA) && !defined(AMREX_USE_HIP)
#error "This integration supports CPU, CUDA and HIP builds of AMReX; EBGeometry has no SYCL memory resource yet."
#endif

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
// The library's default branching factor: the same type in every compilation pass.
using Mesh = EBGeometry::MeshSDF<T, EBGeometry::BVH::DefaultBranchingRatio<T>()>;

// AMReX's implicit function, which can also name the face closest to a point. EBGeometry's value is negative
// inside the mesh and AMReX reads negative values as fluid, so the sign is flipped: the fluid is around the
// mesh. Deriving from GPUable lets EB2 and ParallelFor evaluate it in device kernels. It holds the mesh twice,
// as a view of the host pool and of its device mirror (the same object in a CPU build), and each compilation
// pass uses its own.
class PaintIF : public GPUable
{
public:
  PaintIF(const Mesh& a_host, const Mesh& a_device) noexcept : m_host(a_host), m_device(a_device)
  {}

  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return -mesh().signedDistance(Vec3(x, y, z));
  }

  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

  // The face's index in the mesh: its position in the file, less any zero-area faces the reader removed.
  // Stored in a Real, it is exact up to 2^24 faces when AMReX is built in single precision.
  AMREX_GPU_HOST_DEVICE
  Real
  closestFace(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return Real(mesh().getClosestFace(Vec3(x, y, z)).faceId);
  }

private:
  AMREX_GPU_HOST_DEVICE
  const Mesh&
  mesh() const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return m_device;
#else
    return m_host;
#endif
  }

  Mesh m_host;
  Mesh m_device;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);
  try {
    int         n_cell        = 128;
    int         max_grid_size = 32;
    std::string filename      = "../../../common-3d-test-models/data/suzanne.obj";

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("filename", filename);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Mesh mesh = EBGeometry::Parser::readIntoMeshSDF<T>(filename, pool);

    // In a GPU build the pool is frozen and mirrored to device memory in one piece, and the mesh is
    // rebased onto the mirror. The pools must outlive the EB generation.
#if defined(AMREX_USE_GPU)
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
    const PaintIF          sdf(mesh, mesh.rebasedView(devicePool));
#else
    const PaintIF sdf(mesh, mesh);
#endif

    // Cubic cells, n_cell along the mesh's longest side and 10% to spare; each side's count is rounded up
    // to a multiple of 8.
    const AABB box    = mesh.computeBoundingVolume();
    const Vec3 centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
    const Vec3 extent = T(1.1) * (box.getHighCorner() - box.getLowCorner());
    const T    h      = extent[extent.maxDir(true)] / T(n_cell);

    IntVect cells;
    RealBox rb;

    for (int d = 0; d < AMREX_SPACEDIM; d++) {
      cells[d] = 8 * int(std::ceil(extent[d] / (8 * h)));

      rb.setLo(d, centre[d] - T(0.5) * h * T(cells[d]));
      rb.setHi(d, centre[d] + T(0.5) * h * T(cells[d]));
    }

    const Array<int, AMREX_SPACEDIM> periodic{AMREX_D_DECL(0, 0, 0)};
    const Geometry                   geom(Box(IntVect(0), cells - 1), rb, 0, periodic);

    // A mesh's fine features can cut a cell edge more than twice, which EB2 refuses unless it may cover such
    // cells; allow it unless the inputs say otherwise.
    ParmParse eb2("eb2");
    if (!eb2.contains("cover_multiple_cuts")) {
      eb2.add("cover_multiple_cuts", 1);
    }

    auto gshop = EB2::makeShop(sdf);
    EB2::Build(gshop, geom, 0, 0);

    BoxArray grids(geom.Domain());
    grids.maxSize(max_grid_size);
    const DistributionMapping dm(grids);

    const auto factory = makeEBFabFactory(geom, grids, dm, {2, 2, 2}, EBSupport::full);
    MultiFab   mf(grids, dm, 1, 0, MFInfo(), *factory);

    // Paint each cut cell with the id of the face closest to its centre; the other cells keep -1.
    mf.setVal(-1.0);

    const auto& flags = factory->getMultiEBCellFlagFab();
    const auto  lo    = geom.ProbLoArray();
    const auto  dx    = geom.CellSizeArray();

    for (MFIter mfi(mf); mfi.isValid(); ++mfi) {
      const Box& bx = mfi.validbox();

      if (flags[mfi].getType(bx) == FabType::singlevalued) {
        const auto cells = flags.const_array(mfi);
        const auto paint = mf.array(mfi);

        ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) {
          if (cells(i, j, k).isSingleValued()) {
            paint(i, j, k) =
              sdf.closestFace(lo[0] + (i + 0.5) * dx[0], lo[1] + (j + 0.5) * dx[1], lo[2] + (k + 0.5) * dx[2]);
          }
        });
      }
    }

    EB_WriteSingleLevelPlotfile("plt", mf, {"face_id"}, geom, 0.0, 0);

    // The EB index space keeps a copy of the implicit function, which must not outlive the pool.
    EB2::IndexSpace::clear();
  } catch (const EBGeometry::Parser::ParseError& e) {
    amrex::Abort(std::string("Cannot read the mesh: ") + e.what() +
                 "\nThe default mesh is in the common-3d-test-models submodule: git submodule update --init");
  }
  amrex::Finalize();
}
