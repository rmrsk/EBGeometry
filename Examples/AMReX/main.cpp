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

// BVH tree degree
constexpr int K = 2;

namespace amrex {
  namespace EB2 {

    using prec = float;
    using Vec3 = EBGeometry::Vec3T<prec>;
    using BV   = EBGeometry::BoundingVolumes::AABBT<prec>;
    using Node = EBGeometry::BVH::NodeT<prec, EBGeometry::Dcel::FaceT<prec>, BV, K>;            
    

    class SignedDistanceBVH : public EBGeometry::SignedDistanceBVH<prec, BV, K> {
    public:
      

      SignedDistanceBVH(const std::string a_filename, const bool a_flipSign) {

	auto mesh = EBGeometry::Dcel::Parser::PLY<prec>::readASCII(a_filename);

	m_rootNode = std::make_shared<Node> (mesh->getFaces());

	m_rootNode->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultBVConstructor<prec, BV>,
						      EBGeometry::Dcel::spatialSplitPartitioner<prec, K>,
						      EBGeometry::Dcel::defaultStopFunction<prec, BV, K>);
      }

      SignedDistanceBVH(const SignedDistanceBVH& a_other) {
	this->m_rootNode     = a_other.m_rootNode;
	this->m_flipSign     = a_other.m_flipSign;
	this->m_transformOps = a_other.m_transformOps;
      }

      Real operator() (AMREX_D_DECL(Real x, Real y, Real z)) const noexcept {

	const Real sign = (m_flipSign) ? -1.0 : 1.0;

	return sign * m_rootNode->signedDistance(this->transformPoint(Vec3(x,y,z)));
      };

      inline Real operator() (const RealArray& p) const noexcept {
        return this->operator()(AMREX_D_DECL(p[0],p[1],p[2]));
      }      
    };
  }
}

int main (int argc, char* argv[])
{
    amrex::Initialize(argc, argv);

    {
        int n_cell = 128;
        int max_grid_size = 32;
        int which_geom = 0;

	std::string filename;

        // read parameters
	ParmParse pp;
	pp.query("n_cell", n_cell);
	pp.query("max_grid_size", max_grid_size);
	pp.query("which_geom", which_geom);

	
        Geometry geom;
        {
	  RealBox rb;
	  
	  if(which_geom == 0){// Airfoil case
	    rb = RealBox({-100,-100,-75}, {400,100,125}); 
	    filename = "../PLY/airfoil.ply";
	  }
	  else if (which_geom == 1){ // Sphere case
	    rb = RealBox({-400,-400,-400}, {400,400,400});
	    filename = "../PLY/sphere.ply";
	  }
	  else if (which_geom == 2){ // Dodecahedron
	    rb = RealBox({-2.,-2.,-2.}, {2.,2.,2.});
	    filename = "../PLY/dodecahedron.ply";
	  }
	  else if (which_geom == 3){ // Horse
	    rb = RealBox({-0.12,-0.12,-0.12}, {0.12,0.12,0.12});
	    filename = "../PLY/horse.ply";
	  }
	  else if (which_geom == 4){ // Car
	    //	    rb = RealBox({-20,-20,-20}, {20,20,20}); // Doesn't work. 
	    rb = RealBox({-10,-5,-5}, {10,5,5}); // Works. 
	    filename = "../PLY/porsche.ply";
	  }
	  else if (which_geom == 5){ // Orion
	    rb = RealBox({-10,-5,-10}, {10,10,10}); 
	    filename = "../PLY/orion.ply";
	  }
	  else if (which_geom == 6){ // Armadillo
	    rb = RealBox({-100,-75,-100}, {100,125,100}); 
	    filename = "../PLY/armadillo.ply";
	  }	  	  	  	  

	  Array<int,AMREX_SPACEDIM> is_periodic{false, false, false};
	  Geometry::Setup(&rb, 0, is_periodic.data());
	  Box domain(IntVect(0), IntVect(n_cell-1));
	  geom.define(domain);
        }

	// Create the signed distance function. 
	EB2::SignedDistanceBVH sdf(filename, false);

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
    }

    amrex::Finalize();
}
