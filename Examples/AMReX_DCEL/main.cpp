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
#include "../../EBGeometry.hpp"

using namespace amrex;

/*!
  @brief This is an AMReX-capable version of the EBGeometry BVH accelerator. It
  is templated as T, Meta BV, K which indicate the EBGeometry precision, triangle meta-data,
  bounding volume, and tree degree.
*/
template <class T, class Meta, class BV, size_t K>
class AMReXSDF
{
public:
  /*!
    @brief Full constructor.
    @param[in] a_filename File name. Must be an STL file.
    @param[in] a_use_bvh  If true, accelerate with a BVH; otherwise query the raw mesh.
  */
  AMReXSDF(const std::string a_filename, const bool a_use_bvh)
  {
    if (a_use_bvh) {
      m_sdf = EBGeometry::Parser::readIntoTriangleBVH<T, Meta, K>(a_filename);
    }
    else {
      m_sdf = EBGeometry::Parser::readIntoMesh<T, Meta>(a_filename);
    }
  }

  /*!
    @brief Copy constructor.
  */
  AMReXSDF(const AMReXSDF& a_other) = default;

  /*!
    @brief AMReX's implicit function definition.
  */
  Real
  operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return m_sdf->value(EBGeometry::Vec3T<T>(x, y, z));
  }

  /*!
    @brief Also an AMReX implicit function implementation
  */
  inline Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

protected:
  /*!
    @brief Signed distance function (BVH-accelerated or flat mesh).
  */
  std::shared_ptr<EBGeometry::ImplicitFunction<T>> m_sdf;
};

int
main(int argc, char* argv[])
{

  amrex::Initialize(argc, argv);

  {
    int  n_cell          = 128;
    int  max_grid_size   = 16;
    int  which_geom      = 0;
    int  num_coarsen_opt = 4;
    bool use_bvh         = true;

    std::string filename;

    // read parameters
    ParmParse pp;
    pp.query("use_bvh", use_bvh);
    pp.query("n_cell", n_cell);
    pp.query("max_grid_size", max_grid_size);
    pp.query("which_geom", which_geom);
    pp.query("num_coarsen_opt", num_coarsen_opt);

    Geometry geom;
    {
      // Mesh file (STL/PLY/VTK/OBJ). Override with 'filename=<path>' in the inputs file. The default is an OBJ
      // from the common-3d-test-models submodule (see the "Building and using" docs for how to fetch it); the
      // path is relative to this example's source folder, where the executable is run.
      filename = "../../common-3d-test-models/data/armadillo.obj";
      pp.query("filename", filename);

      RealBox rb({-100, -75, -100}, {100, 125, 100}); // bounds the default armadillo mesh

      Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
      Geometry::Setup(&rb, 0, is_periodic.data());
      Box domain(IntVect(0), IntVect(n_cell - 1));
      geom.define(domain);
    }

    // Create our signed distance function. K is the tree degree while T is the
    // EBGeometry precision.
    constexpr int K = 4;

    using T    = float;
    using Meta = EBGeometry::DCEL::DefaultMetaData;
    using Vec3 = EBGeometry::Vec3T<T>;
    using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

    AMReXSDF<T, Meta, BV, K> sdf(filename, use_bvh);

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

  amrex::Finalize();
}
