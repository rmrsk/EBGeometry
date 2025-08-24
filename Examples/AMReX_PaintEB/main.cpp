/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

// AMReX includes
#include <AMReX.H>
#include <AMReX_EB2.H>
#include <AMReX_EB2_IF.H>
#include <AMReX_ParmParse.H>
#include <AMReX_PlotFileUtil.H>

// Our include
#include "../../EBGeometry.hpp"

#warning "Example must be updated to use the trimesh class"

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
      faces[i]->getMetaData() = 1.0 * i;
    }

    m_sdf = std::make_shared<EBGeometry::FastTriMeshSDF<T, Meta, BV, K>>(mesh);
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
  */
  inline std::vector<std::pair<std::shared_ptr<const Face>, T>>
  getClosestFaces(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return m_sdf->getClosestFaces(EBGeometry::Vec3T<T>(x, y, z), true);
  }

protected:
  /*!
    @brief DCEL mesh represented as a BVH of its facets, exposed as an implicit function. 
  */
  std::shared_ptr<EBGeometry::FastCompactMeshSDF<T, Meta, BV, K>> m_sdf;
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
  RealBox  rb;

  if (which_geom == 0) { // Airfoil case
    rb       = RealBox({-100, -100, -75}, {400, 100, 125});
    filename = "../Resources/airfoil.stl";
  }
  else if (which_geom == 1) { // Sphere case
    rb       = RealBox({-400, -400, -400}, {400, 400, 400});
    filename = "../Resources/sphere.stl";
  }
  else if (which_geom == 2) { // Dodecahedron
    rb       = RealBox({-2., -2., -2.}, {2., 2., 2.});
    filename = "../Resources/dodecahedron.stl";
  }
  else if (which_geom == 3) { // Horse
    rb       = RealBox({-0.12, -0.12, -0.12}, {0.12, 0.12, 0.12});
    filename = "../Resources/horse.stl";
  }
  else if (which_geom == 4) { // Car
    //	    rb = RealBox({-20,-20,-20}, {20,20,20}); // Doesn't work.
    rb       = RealBox({-10, -5, -5}, {10, 5, 5}); // Works.
    filename = "../Resources/porsche.stl";
  }
  else if (which_geom == 5) { // Orion
    rb       = RealBox({-10, -5, -10}, {10, 10, 10});
    filename = "../Resources/orion.stl";
  }
  else if (which_geom == 6) { // Armadillo
    rb       = RealBox({-100, -75, -100}, {100, 125, 100});
    filename = "../Resources/armadillo.stl";
  }
  else if (which_geom == 7) { // Adirondacks
    rb       = RealBox({0, 0, 0}, {200, 200, 50});
    filename = "../Resources/adirondack.stl";
  }

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

        mf_array(i, j, k) = 1.0 * (candidateFaces.front().first)->getMetaData();
      });
    }
  }

  EB_WriteSingleLevelPlotfile("plt", mf, {"facet_id"}, geom, 0.0, 0);

  amrex::Finalize();
}
