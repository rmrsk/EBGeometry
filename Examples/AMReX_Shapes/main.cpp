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

using namespace amrex;

using T    = float;
using SDF  = EBGeometry::SignedDistanceFunction<T>;
using Vec3 = EBGeometry::Vec3T<T>;    

namespace amrex {
  namespace EB2 {

    /*!
      @brief This is just an EBGeometry-exposed signed distance field usable with AMReX.
    */
    class AMReXSDF {
    public:

      /*!
	@brief Full constructor.
	@param[in] a_filename File name. Must be a PLY file and will be parser by the PLY parser.
	@param[in] a_flipSign Hook for swapping inside/outside. 
      */
      AMReXSDF(std::shared_ptr<SDF>& a_sdf){
	m_sdf = a_sdf;
      }

      /*!
	@brief Copy constructor. 
	@param[in] a_other Other SDF. 
      */
      AMReXSDF(const AMReXSDF& a_other) {
	this->m_sdf = a_other.m_sdf;
      }

      /*!
	@brief AMReX's implicit function definition. 
      */
      Real operator() (AMREX_D_DECL(Real x, Real y, Real z)) const noexcept {
	return m_sdf->signedDistance(Vec3(x,y,z));
      };

      /*!
	@brief Also an AMReX implicit function implementation
      */
      inline Real operator() (const RealArray& p) const noexcept {
        return this->operator()(AMREX_D_DECL(p[0],p[1],p[2]));
      }

    protected:

      /*!
	@brief EBGeometry signed distance function. 
      */
      std::shared_ptr<SDF> m_sdf;
    };
  }
}

int main (int argc, char* argv[]) {
  amrex::Initialize(argc, argv);

  int n_cell = 128;
  int max_grid_size = 32;
  int whichGeom = 0;

  std::string filename;

  // read parameters
  ParmParse pp;
  pp.query("n_cell", n_cell);
  pp.query("max_grid_size", max_grid_size);
  pp.query("which_geom", whichGeom);

	
  Geometry geom;
  RealBox rb;

  std::shared_ptr<SDF> func;
  if(whichGeom == 0){ // Sphere.
    rb = RealBox({-1,-1,-1}, {1,1,1});
    func = std::make_shared<EBGeometry::SphereSDF<T>> (Vec3::zero(), T(0.5), false);      
  }
  else if(whichGeom == 1){ // Plane.
    rb = RealBox({-1,-1,-1}, {1,1,1});

    func = std::make_shared<EBGeometry::PlaneSDF<T> > (Vec3::zero(), Vec3::one(), false);
  }
  else if(whichGeom == 2){ // Infinite cylinder. 
    rb = RealBox({-1,-1,-1}, {1,1,1});

    func = std::make_shared<EBGeometry::InfiniteCylinderSDF<T> > (Vec3::zero(), T(0.1), 2, false);
  }  
  else if(whichGeom == 3){ // Finite cylinder.
    rb = RealBox({-2,-2,-2}, {2,2,2});

    func = std::make_shared<EBGeometry::CylinderSDF<T> > (-Vec3::one(), Vec3::one(), 0.25, false);
  }
  else if(whichGeom == 4){ // Capsule.
    rb = RealBox({-2,-2,-2}, {2,2,2});

    func = std::make_shared<EBGeometry::CapsuleSDF<T> > (-Vec3::one(), Vec3::one(), 0.25, false);
  }
  else if(whichGeom == 5){ // Box.
    rb = RealBox({-2,-2,-2}, {2,2,2});    

    func = std::make_shared<EBGeometry::BoxSDF<T> > (-Vec3::one(), Vec3::one(), false);
  }
  else if(whichGeom == 6){ // Rounded box.
    rb = RealBox({-2,-2,-2}, {2,2,2});        

    auto box = std::make_shared<EBGeometry::BoxSDF<T> > (-Vec3::one(), Vec3::one(), false);
    func = std::make_shared<EBGeometry::RoundedSDF<T> >(box, 0.25);
  }
  else if(whichGeom == 7){ // Torus.
    rb = RealBox({-2,-2,-2}, {2,2,2});            

    func = std::make_shared<EBGeometry::TorusSDF<T> >(Vec3::zero(), 1.0, 0.25, false);
  }
  else if(whichGeom == 8){ // Infinite cone.
    rb = RealBox({-2,-2,-2}, {2,2,2});                

    func = std::make_shared<EBGeometry::InfiniteConeSDF<T> >(Vec3(0.0, 0.0, 1.0), 30.0, false);
  }
  else if(whichGeom == 9){ // Finite cone.
    rb = RealBox({-2,-2,-2}, {2,2,2});                    

    func = std::make_shared<EBGeometry::ConeSDF<T> >(Vec3(0.0, 0.0, 1.0), 2.0, 30, false);
  }
  if(whichGeom == 10){ // Spherical shell.
    rb = RealBox({-1,-1,-1}, {1,1,1});                        

    auto sphere = std::make_shared<EBGeometry::SphereSDF<T>> (Vec3::zero(), T(0.5), false);
    func = std::make_shared<EBGeometry::AnnularSDF<T>> (sphere, 0.1);
  }      

  Array<int,AMREX_SPACEDIM> is_periodic{false, false, false};
  Geometry::Setup(&rb, 0, is_periodic.data());
  Box domain(IntVect(0), IntVect(n_cell-1));
  geom.define(domain);

  EB2::AMReXSDF sdf(func);

  auto gshop = EB2::makeShop(sdf);
  EB2::Build(gshop, geom, 0, 0);

  // Put some data
  MultiFab mf;
  {
    BoxArray ba(geom.Domain());
    ba.maxSize(max_grid_size);
    DistributionMapping dm{ba};

    std::unique_ptr<EBFArrayBoxFactory> factory
      = amrex::makeEBFabFactory(geom, ba, dm, {2,2,2}, EBSupport::full);

    mf.define(ba, dm, 1, 0, MFInfo(), *factory);
    mf.setVal(1.0);
  }

  EB_WriteSingleLevelPlotfile("plt", mf, {"rho"}, geom, 0.0, 0);

  amrex::Finalize();
}
