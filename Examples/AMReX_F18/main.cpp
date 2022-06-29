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
#include <filesystem>

// Our include
#include "../../EBGeometry.hpp"

using namespace amrex;

constexpr size_t K = 4;
using T            = Real;
using Face         = EBGeometry::DCEL::FaceT<T>;
using Mesh         = EBGeometry::DCEL::MeshT<T>;
using BV           = EBGeometry::BoundingVolumes::AABBT<T>;
using Prim         = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
using Vec3         = EBGeometry::Vec3T<T>;

// F18 geometry, using nifty EBGeometry bindings and accelerators.
class F18
{
public:
  F18()
  {
    // Get all the STL files in this directory .
    std::vector<std::string> stlFiles;
    for (const auto& entry : std::filesystem::directory_iterator("../Resources/F18")) {
      const std::string f   = entry.path();
      const std::string ext = f.substr(f.find_last_of(".") + 1);

      if (ext == "stl" || ext == "STL") {
        stlFiles.emplace_back(f);
      }
    }

    // Read each STL file and make them into flattened BVH-trees. These STL files
    // have the normal vector pointing inwards so we flip it.
    std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> bvhDCEL;
    for (const auto& f : stlFiles) {
      const auto mesh = EBGeometry::Parser::readIntoDCEL<T>(f);
      mesh->flip();

      // Create the BVH.
      bvhDCEL.emplace_back(EBGeometry::DCEL::buildFlatBVH<T, BV, K>(mesh));
    }

    // Optimized, faster union which uses a BVH for bounding the BVHs.
    m_union = std::make_shared<EBGeometry::UnionBVH<T, Prim, BV, K>>(
      bvhDCEL, true, [](const std::shared_ptr<const Prim>& a_prim) -> BV { return a_prim->getBoundingVolume(); });
  }

  F18(const F18& a_other)
  {
    this->m_union = a_other.m_union;
  }

  Real operator()(AMREX_D_DECL(Real x, Real y, Real z)) const noexcept
  {
    return Real(m_union->value(EBGeometry::Vec3T(x, y, z)));
  };

  inline Real
  operator()(const RealArray& p) const noexcept
  {
    return this->operator()(AMREX_D_DECL(p[0], p[1], p[2]));
  }

protected:
  std::shared_ptr<EBGeometry::UnionBVH<T, Prim, BV, K>> m_union;
};

int
main(int argc, char* argv[])
{
  amrex::Initialize(argc, argv);

  int n_cell        = 128;
  int max_grid_size = 16;
  int which_geom    = 0;

  std::string filename;

  // read parameters
  ParmParse pp;
  pp.query("n_cell", n_cell);
  pp.query("max_grid_size", max_grid_size);
  pp.query("which_geom", which_geom);

  Geometry geom;
  {
    RealBox rb = RealBox({-1, -1, -1}, {13, 5, 9});

    Array<int, AMREX_SPACEDIM> is_periodic{false, false, false};
    Geometry::Setup(&rb, 0, is_periodic.data());
    Box domain(IntVect(0), IntVect(n_cell - 1));
    geom.define(domain);
  }

  F18 f18 = F18();

  auto gshop = EB2::makeShop(f18);
  EB2::Build(gshop, geom, 0, 0);

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

  amrex::Finalize();
}
