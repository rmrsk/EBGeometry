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

namespace amrex {
  namespace EB2 {

    /*!
      @brief This is an AMReX-capable version of the EBGeometry BVH accelerator. It is templated as T, BV, K which indicate
      the EBGeometry precision, bounding volume, and tree degree. 
    */
    template <class T, class BV, int K>
    class SignedDistanceBVH {
    public:

      /*!
	@brief Alias for builder node, for encapsulating a "standard" BVH node
      */
      using BuilderNode = EBGeometry::BVH::NodeT<T, EBGeometry::Dcel::FaceT<T>, BV, K>;

      /*!
	@brief Alias for linearized BVH node
      */
      using LinearNode = EBGeometry::BVH::LinearBVH<T, EBGeometry::Dcel::FaceT<T>, BV, K>;

      /*!
	@brief Alias for always-3D vector
      */
      using Vec3 = EBGeometry::Vec3T<T>;

      /*!
	@brief Full constructor.
	@param[in] a_filename File name. Must be a PLY file and will be parser by the PLY parser.
	@param[in] a_flipSign Hook for swapping inside/outside. 
      */
      SignedDistanceBVH(const std::string a_filename, const bool a_flipSign) {

	// 1. Read mesh from file. 
	auto mesh = EBGeometry::Dcel::Parser::PLY<T>::readASCII(a_filename);

	// 2. Create a standard BVH hierarchy. This is not a compact ree. 
	auto root = std::make_shared<BuilderNode>(mesh->getFaces());
	root->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultBVConstructor<T, BV>,
						EBGeometry::Dcel::defaultPartitioner<T, K>,
						EBGeometry::Dcel::defaultStopFunction<T, BV, K>);

	// 3. Flatten the tree onto a tighter memory representation. 
	m_rootNode = root->flattenTree();
      }

      /*!
	@brief Copy constructor. 
	@param[in] a_other Other SDF. 
      */
      SignedDistanceBVH(const SignedDistanceBVH& a_other) {
	this->m_rootNode     = a_other.m_rootNode;
	this->m_flipSign     = a_other.m_flipSign;
      }

      /*!
	@brief AMReX's implicit function definition. 
      */
      Real operator() (AMREX_D_DECL(Real x, Real y, Real z)) const noexcept {
	const Real sign = (m_flipSign) ? -1.0 : 1.0;

	return sign * m_rootNode->signedDistance(Vec3(x,y,z));
      };

      /*!
	@brief Also an AMReX implicit function implementation
      */
      inline Real operator() (const RealArray& p) const noexcept {
        return this->operator()(AMREX_D_DECL(p[0],p[1],p[2]));
      }

    protected:

      /*!
	@brief Root node of the linearized BVH hierarchy. 
      */
      std::shared_ptr<LinearNode> m_rootNode;

      /*!
	@brief Hook for flipping the sign
      */
      bool m_flipSign;      
    };
  }
}

int main (int argc, char* argv[]) {
  amrex::Initialize(argc, argv);

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

  // Create our signed distance function. K is the tree degree while T is the EBGeometry precision. 
  constexpr int K = 2;
	
  using T    = float;
  using Vec3 = EBGeometry::Vec3T<T>;
  using BV   = EBGeometry::BoundingVolumes::AABBT<T>;

  EB2::SignedDistanceBVH<T, BV, K> sdf(filename, false);

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
