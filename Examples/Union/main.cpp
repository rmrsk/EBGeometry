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
  
  // Declare precision. 
  using precision = float;

  // Aliases for cutting down on typing.
  using BV        = BoundingVolumes::AABBT<precision>;  
  using Vec3      = Vec3T<precision>;  
  using Face      = FaceT<precision>;
  using SDF       = SignedDistanceFunction<precision>;
  using slowSDF   = SignedDistanceDcel<precision>;
  using fastSDF   = SignedDistanceBVH<precision, BV, K>;

  std::string inputFile;

  // Create an empty DCEL mesh
  std::cout << "Parsing input file\n";  
  auto mesh = EBGeometry::Dcel::Parser::PLY<precision>::readASCII(file);

  // Create a signed distance function from the mesh. This is the object
  // that will iterate through each and every facet in the input mesh. 
  auto slow = std::make_shared<slowSDF>(mesh, false);

  // Create a bounding-volume hierarchy of the same mesh type. We begin by create the root node and supplying all the mesh faces to it. Here,
  // our bounding volume hierarchy bounds the facets in a binary tree.
  auto root = std::make_shared<BVH::NodeT<precision, Face, BV, K> > (mesh->getFaces());

  std::cout << "Partitioning BVH\n";  
  root->topDownSortAndPartitionPrimitives(defaultBVConstructor<precision, BV>,
  					  spatialSplitPartitioner<precision, K>,
					  defaultStopFunction<precision, BV, K>);


  auto fast = std::make_shared<fastSDF>(root, false);


  // Make a union of SDFs. 


  EBGeometry::BVH::BVConstructorT<SDF, BV> bvConstructor = [](const std::shared_ptr<const SDF>& a_prim){
    return ((const std::shared_ptr<const fastSDF>&) a_prim)->getBoundingVolume();
  };

  // Make the union. 
  UnionBVH<precision, BV, 2> u({fast}, false, bvConstructor);  

  // Query the distance to a point. 
  std::cout << "Distance to point using direct method    = " << slow->signedDistance(Vec3::one()) << std::endl;
  std::cout << "Distance to point using bounding volumes = " << fast->signedDistance(Vec3::one()) << std::endl;
  std::cout << "Distance to point using union = " << u.signedDistance(Vec3::one()) << std::endl;    
  
  return 0;
}
