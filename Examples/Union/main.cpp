/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::Dcel;

// Degree of bounding volume hierarchies. 
constexpr int K = 2;



int main(int argc, char *argv[]) {

  std::string current_exec_name = argv[0]; // Name of the current exec program
  std::vector<std::string> all_args;

  std::string file;
  
  if (argc == 2) {
    file = "../PLY/" + std::string(argv[1]);
  }
  else{
    std::cerr << "Missing file name. Use ./a.out 'filename' where 'filename' is one of the files in ../PLY\n";

    return 1;
  }
  
  // Declare T. 
  using T = float;

  // Aliases for cutting down on typing.
  using BV        = BoundingVolumes::AABBT<T>;  
  using Vec3      = Vec3T<T>;  
  using Face      = FaceT<T>;
  using SDF       = SignedDistanceFunction<T>;
  using slowSDF   = SignedDistanceDcel<T>;
  using fastSDF   = SignedDistanceBVH<T, BV, K>;
  using Sphere    = EBGeometry::SphereSDF<T>;  

  std::string inputFile;

  // Create an empty DCEL mesh
  std::cout << "Parsing input file\n";  
  auto mesh = EBGeometry::Dcel::Parser::PLY<T>::readASCII(file);

  // Create a signed distance function from the mesh. This is the object
  // that will iterate through each and every facet in the input mesh. 
  auto slow = std::make_shared<slowSDF>(mesh, false);

  // Create a bounding-volume hierarchy of the same mesh type. We begin by create the root node and supplying all the mesh faces to it. Here,
  // our bounding volume hierarchy bounds the facets in a binary tree.
  auto root = std::make_shared<BVH::NodeT<T, Face, BV, K> > (mesh->getFaces());

  std::cout << "Partitioning BVH\n";  
  root->topDownSortAndPartitionPrimitives(defaultBVConstructor<T, BV>,
  					  spatialSplitPartitioner<T, K>,
					  defaultStopFunction<T, BV, K>);


  auto fast = std::make_shared<fastSDF>(root, false);


  // Make a sphere array consisting of about 10^6 spheres. The distance between the spheres is 2*radius
  std::vector<std::shared_ptr<SDF> > spheres;

  constexpr T radius = 0.1;
  constexpr int Nx = 50;
  constexpr int Ny = 50;
  constexpr int Nz = 50;

  for (int i = 0; i < Nx; i++){
    for (int j = 0; j < Ny; j++){
      for (int k = 0; k < Nz; k++){
	const T x = i * (2 * radius);
	const T y = j * (2 * radius);
	const T z = j * (2 * radius);

	const Vec3 center(x,y,z);
	
	spheres.emplace_back(std::make_shared<Sphere>( center, radius, false));
	spheres.emplace_back(std::make_shared<Sphere>(-center, radius, false));
      }
    }
  }

  // Make a standard union of these spheres. This is the union object which iterates through each and every
  // object in the scene. 
  EBGeometry::Union<T> slowUnion(spheres, false);

  // Make a fast union. For the BV constructor we make axis-aligned bounding boxes around each sphere. We use a binary
  // BVH tree here.

  EBGeometry::BVH::BVConstructorT<SDF, BV> aabbConstructor = [](const std::shared_ptr<const SDF>& a_prim){
    const Sphere& sph = static_cast<const Sphere&> (*a_prim);

    const Vec3& c = sph.getCenter();
    const T&    r = sph.getRadius();
    
    const Vec3 lo = c - r*Vec3::one();
    const Vec3 hi = c + r*Vec3::one();

    return BV(lo, hi);
  };
  
  EBGeometry::UnionBVH<T, BV, 2> fastUnion(spheres, false);//, aabbConstructor);
  

  std::cout << "Distance to point using slow union = " << slowUnion.signedDistance(Vec3::one()) << std::endl;    
  
  return 0;
}
