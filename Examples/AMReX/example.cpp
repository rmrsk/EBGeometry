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

    class SignedDistanceBVH {
    public:
      
      using P    = float;
      using Vec3 = EBGeometry::Vec3T<P>;
      using Face = EBGeometry::Dcel::FaceT<P>;      
      using BV   = EBGeometry::BoundingVolumes::AABBT<P>;
      using Node = EBGeometry::BVH::NodeT<P, Face, BV, K>;            

      SignedDistanceBVH(const std::string a_filename, const bool a_flipSign) {

	auto mesh = EBGeometry::Dcel::Parser::PLY<P>::readASCII(a_filename);

	m_rootNode = std::make_shared<Node> (mesh->getFaces());

	m_rootNode->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultStopFunction<P, BV, K>,
						      EBGeometry::Dcel::spatialSplitPartitioner<P, K>,
						      EBGeometry::Dcel::defaultBVConstructor<P, BV>);
      }

      SignedDistanceBVH(const SignedDistanceBVH& a_other) {
	m_rootNode = a_other.m_rootNode;
	m_flipSign = a_other.m_flipSign;	
      }

      Real operator() (AMREX_D_DECL(Real x, Real y, Real z)) const noexcept {

	const Real sign = (m_flipSign) ? -1.0 : 1.0;

	return sign * m_rootNode->pruneTree(Vec3(x,y,z));
      };

    inline Real operator() (const RealArray& p) const noexcept {
        return this->operator()(AMREX_D_DECL(p[0],p[1],p[2]));
    }      
      
    protected:

      std::shared_ptr<Node> m_rootNode;

      bool m_flipSign;
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
	    filename = "airfoil.ply";
	  }
	  else if (which_geom == 1){ // Sphere case
	    rb = RealBox({-400,-400,-400}, {400,400,400});
	    filename = "sphere.ply";
	  }


	  Array<int,AMREX_SPACEDIM> is_periodic{false, false, false};
	  Geometry::Setup(&rb, 0, is_periodic.data());
	  Box domain(IntVect(0), IntVect(n_cell-1));
	  geom.define(domain);
        }

	// Create the signed distance function. 
	EB2::SignedDistanceBVH sphere(filename, false);

	auto gshop = EB2::makeShop(sphere);
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
