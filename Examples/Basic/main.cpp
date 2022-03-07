/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>
#include <chrono>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::Dcel;

// Degree of bounding volume hierarchies. 
constexpr int K = 4;

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
  using slowSDF   = SignedDistanceDcel<T>;
  using fastSDF   = SignedDistanceBVH<T, BV, K>;

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
  root->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultBVConstructor<T, BV>,
  					  EBGeometry::Dcel::spatialSplitPartitioner<T, K>,
					  EBGeometry::Dcel::defaultStopFunction<T, BV, K>);

  auto linearBVH = root->flattenTree();


  auto fast = std::make_shared<fastSDF>(root, false);


  const Vec3 point = Vec3::one();
  
  const auto t1 = std::chrono::high_resolution_clock::now();
  const T directDist = slow->signedDistance(point);
  const auto t2 = std::chrono::high_resolution_clock::now();
  
  const auto t3 = std::chrono::high_resolution_clock::now();
  const T bvhDist = fast->signedDistance(point);  
  const auto t4 = std::chrono::high_resolution_clock::now();

  fast = nullptr;
  
  const auto t5 = std::chrono::high_resolution_clock::now();
  const T linDist = linearBVH.signedDistance(point);    
  const auto t6 = std::chrono::high_resolution_clock::now();



  const std::chrono::duration<T, std::milli> directTime = t2-t1;
  const std::chrono::duration<T, std::milli> bvhTime    = t4-t3;
  const std::chrono::duration<T, std::milli> linTime    = t6-t5;        

  std::cout << "Distance and time using direct query     = " << directDist << ", which took " << directTime.count() << " ms\n";
  std::cout << "Distance and time using bvh query        = " << bvhDist    << ", which took " << bvhTime.count() << " ms\n";
  std::cout << "Distance and time using linear bvh query = " << linDist    << ", which took " << linTime.count() << " ms\n";      
  
  return 0;
}
