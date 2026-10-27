// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// A mesh composed with analytic shapes: the xyzrgb dragon on a table, blended into it by a smooth union,
// compiled into a Tape and handed to Chombo's GeometryShop as an implicit function. See README.md and the MeshCSG
// example under Examples/.

#include "BRMeshRefine.H"
#include "BaseIF.H"
#include "DisjointBoxLayout.H"
#include "EBAMRIO.H"
#include "EBCellFactory.H"
#include "EBISLayout.H"
#include "EBIndexSpace.H"
#include "GeometryShop.H"
#include "ParmParse.H"

#include "EBGeometry.hpp"

#include <string>
#include <vector>

using T    = Real;
using Vec3 = EBGeometry::Vec3T<T>;
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
using Expr = EBGeometry::Expr<T>;
using Tape = EBGeometry::Tape<T>;

// Chombo's implicit function. EBGeometry's value is negative inside the object; the sign is flipped so that
// the fluid is around the object. The object is a trivially copyable handle into a pool, which must outlive
// every copy of this function.
template <class S>
class ShapeIF : public BaseIF
{
public:
  explicit ShapeIF(const S& a_shape) : m_shape(a_shape)
  {}

  Real
  value(const RealVect& a_point) const override
  {
    return -Real(m_shape.signedDistance(Vec3(a_point[0], a_point[1], a_point[2])));
  }

  BaseIF*
  newImplicitFunction() const override
  {
    return new ShapeIF(m_shape);
  }

private:
  S m_shape;
};

// Build Chombo's EB from the implicit function on a cubic domain around a box, and write the implicit
// function's value in every cell to example.hdf5.
template <class S>
void
buildEB(const S& a_shape, const int a_numCells, const int a_gridSize)
{
  const AABB box    = a_shape.computeBoundingVolume();
  const Vec3 centre = T(0.5) * (box.getLowCorner() + box.getHighCorner());
  const Vec3 extent = box.getHighCorner() - box.getLowCorner();
  const T    half   = T(0.55) * extent[extent.maxDir(true)];

  const RealVect      lo(D_DECL(centre[0] - half, centre[1] - half, centre[2] - half));
  const Real          dx = 2 * half / a_numCells;
  const ProblemDomain domain(IntVect::Zero, (a_numCells - 1) * IntVect::Unit);

  const ShapeIF<S> implicitFunction(a_shape);
  GeometryShop     workshop(implicitFunction, -1, dx * RealVect::Unit);
  EBIndexSpace*    ebis = Chombo_EBIS::instance();
  ebis->define(domain, lo, dx, workshop, a_gridSize, -1);

  Vector<int> procs;
  Vector<Box> boxes;
  domainSplit(domain, boxes, a_gridSize, a_gridSize);
  mortonOrdering(boxes);
  LoadBalance(procs, boxes);
  const DisjointBoxLayout grids(boxes, procs);

  EBISLayout ebisl;
  ebis->fillEBISLayout(ebisl, grids, domain, 1);

  LevelData<EBCellFAB> data(grids, 1, IntVect::Zero, EBCellFactory(ebisl));

  for (DataIterator dit(grids); dit.ok(); ++dit) {
    FArrayBox& fab = data[dit()].getFArrayBox();

    for (BoxIterator bit(fab.box()); bit.ok(); ++bit) {
      fab(bit(), 0) = implicitFunction.value(lo + (bit() + 0.5 * RealVect::Unit) * dx);
    }
  }

  Vector<LevelData<EBCellFAB>*> levels(1, &data);
  writeEBAMRname(&levels, "example.hdf5");

  ebis->clear();
}

int
main(int argc, char* argv[])
{
#ifdef CH_MPI
  MPI_Init(&argc, &argv);
#endif
  {
    ParmParse pp(argc - 2, argv + 2, nullptr, argv[1]);

    int         n_cells   = 128;
    int         grid_size = 16;
    std::string filename  = "../../../common-3d-test-models/data/xyzrgb_dragon.obj";

    pp.query("n_cells", n_cells);
    pp.query("grid_size", grid_size);
    pp.query("filename", filename);

    // The default mesh is in the common-3d-test-models submodule: git submodule update --init.
    EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

    const auto mesh = EBGeometry::Parser::readIntoTriMeshSDF<T>(
      filename, pool, EBGeometry::BVH::Construction::SAH, EBGeometry::Parser::OnDefect::Warn);

    // Scale the mesh to 2.4 along its longest side and stand it on a table top at y = 0.
    const Vec3 lo     = mesh.computeBoundingVolume().getLowCorner();
    const Vec3 hi     = mesh.computeBoundingVolume().getHighCorner();
    const T    scale  = T(2.4) / (hi - lo)[(hi - lo).maxDir(true)];
    const Expr dragon = EBGeometry::Scale(
      EBGeometry::Translate(mesh, Vec3(T(-0.5) * (lo[0] + hi[0]), -lo[1], T(-0.5) * (lo[2] + hi[2]))), scale);

    const T hx = T(0.5) * scale * (hi - lo)[0] + T(0.15);
    const T hz = T(0.5) * scale * (hi - lo)[2] + T(0.15);

    std::vector<Expr> table{
      EBGeometry::RoundedBoxSDF<T>(Vec3(T(0), T(-0.06), T(0)), Vec3(2 * hx, T(0.12), 2 * hz), T(0.03))};

    for (const T x : {-hx + T(0.12), hx - T(0.12)}) {
      for (const T z : {-hz + T(0.12), hz - T(0.12)}) {
        table.push_back(EBGeometry::CylinderSDF<T>(Vec3(x, T(-0.75), z), Vec3(x, T(-0.06), z), T(0.06)));
      }
    }

    const Tape tape(pool, EBGeometry::SmoothUnion(dragon, EBGeometry::Union(table), T(0.3)));

    buildEB(tape, n_cells, grid_size);
  }
#ifdef CH_MPI
  MPI_Finalize();
#endif
  return 0;
}
