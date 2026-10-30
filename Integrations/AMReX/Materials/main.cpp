// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Which part of an object each cut cell lies on: the armadillo with a cylinder bored through its chest,
// compiled into a Tape and handed to AMReX's EB2 grid generation as an implicit function, and each cut cell
// labelled with the tape's leaf closest to it (the armadillo 0, the bore 1) and, on the armadillo, with the id
// of its closest mesh face: the indices into tables of per-part and per-face data, such as boundary
// conditions or materials. The same code runs on CPUs and, in a CUDA or HIP build of AMReX, on GPUs. See
// README.md and the Materials example under Examples/.

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
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;
// The library's default branching factor and triangle width: the same type in every compilation pass, and
// one of the tape's built-in primitive types.
using Mesh =
  EBGeometry::TriMeshSDF<T, EBGeometry::BVH::DefaultBranchingRatio<T>(), EBGeometry::TriangleSoA::DefaultWidth<T>()>;

// AMReX's implicit function, which can also name the part of the object closest to a point. EBGeometry's value
// is negative inside the object and AMReX reads negative values as fluid, so the sign is flipped: the fluid is
// around the object. Deriving from GPUable lets EB2 and ParallelFor evaluate it in device kernels. It holds the
// tape twice, as a view of the host pool and of its device mirror (the same object in a CPU build), and each
// compilation pass uses its own.
class MaterialsIF : public GPUable
{
public:
  MaterialsIF(const Tape& a_host, const Tape& a_device) noexcept : m_host(a_host), m_device(a_device)
  {}

  AMREX_GPU_HOST_DEVICE
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return -tape().signedDistance(Vec3(x, y, z));
  }

  Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

  // The leaf the value at a point comes from, in expression order, and on a mesh leaf the id of the mesh's
  // closest face (its position in the file, less any zero-area faces the reader removed); -1 where there is
  // none. Stored in a Real, a face id is exact up to 2^24 faces when AMReX is built in single precision.
  AMREX_GPU_HOST_DEVICE
  void
  label(const Vec3& a_point, Real& a_leaf, Real& a_face) const noexcept
  {
    const EBGeometry::PrimitiveRef<T> ref = tape().closestPrimitive(a_point);

    a_leaf = (ref.m_leaf == EBGeometry::PrimitiveRef<T>::None) ? Real(-1) : Real(ref.m_leaf);
    a_face = Real(-1);

    // The reference holds the point in the primitive's own frame, so the mesh is asked there, after the
    // translation, scaling and rotation the expression applied to it.
    if (tape().isPrimitive<Mesh>(ref)) {
      a_face = Real(tape().primitive<Mesh>(ref).getClosestFace(ref.m_localPoint).faceId);
    }
  }

private:
  AMREX_GPU_HOST_DEVICE
  const Tape&
  tape() const noexcept
  {
#if defined(EBGEOMETRY_DEVICE_COMPILE)
    return m_device;
#else
    return m_host;
#endif
  }

  Tape m_host;
  Tape m_device;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);
  try {
    int         n_cell        = 128;
    int         max_grid_size = 32;
    std::string filename      = "../../../common-3d-test-models/data/armadillo.obj";

    ParmParse pp;
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("filename", filename);

    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const Mesh mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(filename, pool);

    // Stand the mesh on y = 0 with height 2, turned to face +z, and bore a cylinder through its body along z.
    const Vec3 lo = mesh.computeBoundingVolume().getLowCorner();
    const Vec3 hi = mesh.computeBoundingVolume().getHighCorner();

    const Expr armadillo = EBGeometry::Rotate(
      EBGeometry::Scale(EBGeometry::Translate(mesh, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))),
                        T(2) / (hi[1] - lo[1])),
      T(180),
      1);

    const EBGeometry::CylinderSDF<T> bore(Vec3(T(0), T(1.22), T(-2)), Vec3(T(0), T(1.22), T(2)), T(0.16));

    const Tape tape(pool, EBGeometry::Difference(armadillo, bore));

    // In a GPU build the pool is frozen and mirrored to device memory in one piece, and the tape is rebased
    // onto the mirror. The pools must outlive the EB generation.
#if defined(AMREX_USE_GPU)
    pool.freeze();

    const EBGeometry::Pool devicePool = EBGeometry::Pool::mirror(pool, EBGeometry::deviceMemoryResource());
    const MaterialsIF      sdf(tape, tape.rebasedView(devicePool));
#else
    const MaterialsIF sdf(tape, tape);
#endif

    // Cubic cells, n_cell along the object's longest side and 10% to spare; each side's count is rounded up
    // to a multiple of 8.
    const AABB box    = tape.computeBoundingVolume();
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
    MultiFab   mf(grids, dm, 2, 0, MFInfo(), *factory);

    // Label each cut cell with the leaf and the face closest to its centre; the other cells keep -1.
    mf.setVal(-1.0);

    const auto& flags = factory->getMultiEBCellFlagFab();
    const auto  plo   = geom.ProbLoArray();
    const auto  dx    = geom.CellSizeArray();

    for (MFIter mfi(mf); mfi.isValid(); ++mfi) {
      const Box& bx = mfi.validbox();

      if (flags[mfi].getType(bx) == FabType::singlevalued) {
        const auto cut    = flags.const_array(mfi);
        const auto labels = mf.array(mfi);

        ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) {
          if (cut(i, j, k).isSingleValued()) {
            const Vec3 p(plo[0] + (i + 0.5) * dx[0], plo[1] + (j + 0.5) * dx[1], plo[2] + (k + 0.5) * dx[2]);

            sdf.label(p, labels(i, j, k, 0), labels(i, j, k, 1));
          }
        });
      }
    }

    EB_WriteSingleLevelPlotfile("plt", mf, {"leaf", "face_id"}, geom, 0.0, 0);

    // The EB index space keeps a copy of the implicit function, which must not outlive the pool.
    EB2::IndexSpace::clear();
  } catch (const EBGeometry::Parser::ParseError& e) {
    amrex::Abort(std::string("Cannot read the mesh: ") + e.what() +
                 "\nThe default mesh is in the common-3d-test-models submodule: git submodule update --init");
  }
  amrex::Finalize();
}
