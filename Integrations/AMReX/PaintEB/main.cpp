// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../../EBGeometry.hpp"

using namespace amrex;

/*!
  @brief This is an AMReX-capable version of the EBGeometry BVH accelerator. It
  is templated as T, BV, K which indicate the EBGeometry precision, bounding volume,
  tree degree, and meta-data type for the triangles (defaults to an integer)
*/
template <class T, class BV, size_t K, class Meta = double>
class AMReXSDF
{
public:
  /*!
    @brief Shortcut for DCEL face type
  */
  using Face = EBGeometry::DCEL::FaceT<T, Meta>;

  /*!
    @brief Full constructor.
    @param[in] a_filename File name. Must be an STL file.
  */
  AMReXSDF(const std::string a_filename)
  {
    // Read in the mesh into a DCEL mesh and partition it into a bounding volume hierarchy
    auto mesh = EBGeometry::Parser::readIntoDCEL<T, Meta>(a_filename);

    // Set the meta-data for all facets to their "index", i.e. position in the list of facets
    auto& faces = mesh->getFaces();
    for (size_t i = 0; i < faces.size(); i++) {
      faces[i].getMetaData() = 1.0 * i;
    }

    m_sdf = std::make_shared<EBGeometry::MeshSDF<T, Meta, K>>(mesh, EBGeometry::BVH::Build::SAH);
  }

  /*!
    @brief Copy constructor.
    @param[in] a_other Other SDF.
  */
  AMReXSDF(const AMReXSDF& a_other)
  {
    this->m_sdf = a_other.m_sdf;
  }

  /*!
    @brief AMReX's implicit function definition.
  */
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return m_sdf->value(EBGeometry::Vec3T<T>(x, y, z));
  };

  /*!
    @brief Also an AMReX implicit function implementation
  */
  inline Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

  /*!
    @brief Get the face(s) that are closest to the input point.
    @details The DCEL is index-based, so this returns (face index, unsigned distance) pairs. Resolve
    a face index against the mesh with getFaceMetaData() (or m_sdf->getMesh()).
  */
  inline std::vector<std::pair<EBGeometry::DCEL::DCELIndex, T>>
  getClosestFaces(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return m_sdf->getClosestFaces(EBGeometry::Vec3T<T>(x, y, z), true);
  }

  /*!
    @brief Resolve a face index (from getClosestFaces) to its metadata.
    @param[in] a_faceIdx Index into the DCEL mesh's face array.
  */
  inline Meta
  getFaceMetaData(EBGeometry::DCEL::DCELIndex a_faceIdx) const noexcept
  {
    return m_sdf->getMesh()->getFaces()[a_faceIdx].getMetaData();
  }

protected:
  /*!
    @brief DCEL mesh represented as a BVH of its facets, exposed as an implicit function.
  */
  std::shared_ptr<EBGeometry::MeshSDF<T, Meta, K>> m_sdf;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  int n_cell          = 128;
  int max_grid_size   = 16;
  int which_geom      = 0;
  int num_coarsen_opt = 0;

  std::string filename;

  // read parameters
  ParmParse pp;
  pp.query("n_cell", n_cell);
  pp.query("max_grid_size", max_grid_size);
  pp.query("which_geom", which_geom);
  pp.query("num_coarsen_opt", num_coarsen_opt);

  Geometry geom;
  // Mesh file (STL/PLY/VTK/OBJ). Override with 'filename=<path>' in the inputs file. The default is a
  // small OBJ from the common-3d-test-models submodule (see the "Building and using" docs for how to
  // fetch it); the path is relative to this example's source folder, where the executable is run.
  filename = "../../common-3d-test-models/data/suzanne.obj";
  pp.query("filename", filename);

  RealBox rb({-5, -1, 2}, {0, 4, 6}); // bounds the default suzanne mesh

  Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
  Geometry::Setup(&rb, 0, is_periodic.data());
  Box domain(IntVect(0), IntVect(n_cell - 1));
  geom.define(domain);

  // Create our signed distance function. K is the tree degree while T is the
  // EBGeometry precision.
  constexpr int K = 4;

  using T    = float;
  using Vec3 = EBGeometry::Vec3T<T>;
  using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

  AMReXSDF<T, BV, K> sdf(filename);

  // Create the EB geometry
  auto gshop = EB2::makeShop(sdf);
  EB2::Build(gshop, geom, 0, 0, 1, true, true, num_coarsen_opt);

  // Create some data
  MultiFab mf;

  BoxArray boxArray(geom.Domain());
  boxArray.maxSize(max_grid_size);
  DistributionMapping dm{boxArray};

  RealVect dx;
  for (int dir = 0; dir < SpaceDim; dir++) {
    dx[dir] = rb.length(dir) / (n_cell - 1);
  }

  std::unique_ptr<EBFArrayBoxFactory> factory = amrex::makeEBFabFactory(geom, boxArray, dm, {2, 2, 2}, EBSupport::full);

  mf.define(boxArray, dm, 1, 0, MFInfo(), *factory);
  mf.setVal(-1.0);

  const auto& ebFlags = factory->getMultiEBCellFlagFab();

  // Go through the multifab. For each cell set the value in the cell equal to the closest triangle distance.

  for (amrex::MFIter mfi(mf); mfi.isValid(); ++mfi) {
    const auto& bx       = mfi.validbox();
    const auto& mf_array = mf.array(mfi);
    const auto& flags    = ebFlags[mfi].getType(bx);

    if (flags == FabType::singlevalued) {
      amrex::ParallelFor(bx, [=] AMREX_GPU_DEVICE(int i, int j, int k) {
        amrex::Real x = rb.lo()[0] + (i + 0.5) * dx[0];
        amrex::Real y = rb.lo()[1] + (j + 0.5) * dx[1];
        amrex::Real z = rb.lo()[2] + (k + 0.5) * dx[2];

        const auto& candidateFaces = sdf.getClosestFaces(x, y, z);

        mf_array(i, j, k) = 1.0 * sdf.getFaceMetaData(candidateFaces.front().first);
      });
    }
  }

  EB_WriteSingleLevelPlotfile("plt", mf, {"facet_id"}, geom, 0.0, 0);

  amrex::Finalize();
}
