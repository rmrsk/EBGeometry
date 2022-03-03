/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::Dcel;

// Degree of bounding volume hierarchies. 
constexpr int K = 2;

int main() {
  
  // Declare precision. 
  using precision = float;

  // Aliases for cutting down on typing.
  using BV        = BoundingVolumes::AABBT<precision>;  
  using Vec3      = Vec3T<precision>;  
  using Face      = FaceT<precision>;
  using slowSDF   = SignedDistanceDcel<precision>;
  using fastSDF   = SignedDistanceBVH<precision, BV, K>;    

  // Create an empty DCEL mesh
  std::cout << "Parsing input file\n";  
  auto mesh = EBGeometry::Dcel::Parser::PLY<precision>::readASCII("sphere.ply");

  // Create a signed distance function from the mesh. This is the object
  // that will iterate through each and every facet in the input mesh. 
  slowSDF slow(mesh, false);

  // Create a bounding-volume hierarchy of the same mesh type. We begin by create the root node and supplying all the mesh faces to it. Here,
  // our bounding volume hierarchy bounds the facets in a binary tree.
  auto root = std::make_shared<BVH::NodeT<precision, Face, BV, K> > (mesh->getFaces());

  std::cout << "Partitioning BVH\n";  
  root->topDownSortAndPartitionPrimitives(defaultStopFunction<precision, BV, K>,
  					  spatialSplitPartitioner<precision, K>,
  					  defaultBVConstructor<precision, BV>);


  fastSDF fast(root, false);

  // Query the distance to a point. 
  std::cout << "Distance to point using direct method    = " << slow(Vec3::one()) << std::endl;
  std::cout << "Distance to point using bounding volumes = " << fast(Vec3::one()) << std::endl;  
  
  return 0;
}
