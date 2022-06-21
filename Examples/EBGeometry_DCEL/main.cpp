/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>
#include <chrono>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

// Degree of bounding volume hierarchies. We use a 4-ary tree here, where each
// regular node has four children.
constexpr int K = 4;

int
main(int argc, char* argv[])
{

  std::string              current_exec_name = argv[0]; // Name of the current exec program
  std::vector<std::string> all_args;

  std::string file;

  // Read the input file.
  if (argc == 2) {
    file = "../Scenes/PLY/" + std::string(argv[1]);
  }
  else {
    std::cerr << "Missing file name. Use ./a.out 'filename' where 'filename' "
                 "is one of the files in ../PLY\n";
  }

  // Declare the precision T as float.
  using T = float;

  // Aliases for cutting down on typing.
  using BV   = BoundingVolumes::AABBT<T>;
  using Vec3 = Vec3T<T>;

  // Parse the mesh from file. One can call the signed distance function
  // directly on the mesh, but it will iterate through every facet.
  std::cout << "Parsing input file\n";
  std::shared_ptr<EBGeometry::DCEL::MeshT<T>> directSDF = EBGeometry::Parser::PLY<T>::readIntoDCEL(file);

  std::shared_ptr<EBGeometry::DCEL::MeshT<T>> stlSTL =
    EBGeometry::Parser::STL<T>::readSingleASCII("../Scenes/STL/sphere.stl");

  // Create a bounding-volume hierarchy of the same mesh type. We begin by
  // create the root node and supplying all the mesh faces to it. Here, our
  // bounding volume hierarchy bounds the facets in a binary tree.
  std::cout << "Partitioning BVH\n";
  auto bvhSDF = std::make_shared<BVH::NodeT<T, FaceT<T>, BV, K>>(directSDF->getFaces());
  bvhSDF->topDownSortAndPartitionPrimitives(EBGeometry::DCEL::defaultBVConstructor<T, BV>,
                                            EBGeometry::DCEL::defaultPartitioner<T, BV, K>,
                                            EBGeometry::DCEL::defaultStopFunction<T, BV, K>);

  // Create the linear representation of the conventional BVH SDF above.
  std::cout << "Flattening BVH tree\n";
  auto linSDF = bvhSDF->flattenTree();

  // Compute signed distance for this position and time all SDF representations.
  const Vec3 point = Vec3::one();

  const auto t1         = std::chrono::high_resolution_clock::now();
  const T    directDist = directSDF->signedDistance(point);
  const auto t2         = std::chrono::high_resolution_clock::now();

  const auto t3      = std::chrono::high_resolution_clock::now();
  const T    bvhDist = bvhSDF->signedDistance(point);
  const auto t4      = std::chrono::high_resolution_clock::now();

  const auto t5      = std::chrono::high_resolution_clock::now();
  const T    linDist = linSDF->signedDistance(point);
  const auto t6      = std::chrono::high_resolution_clock::now();

  // Kill all the SDF representations.
  directSDF = nullptr;
  bvhSDF    = nullptr;
  linSDF    = nullptr;

  // Get the timings.
  const std::chrono::duration<T, std::micro> directTime = t2 - t1;
  const std::chrono::duration<T, std::micro> bvhTime    = t4 - t3;
  const std::chrono::duration<T, std::micro> linTime    = t6 - t5;

  std::cout << "Distance and time using direct query     = " << directDist << ", which took " << directTime.count()
            << " us\n";
  std::cout << "Distance and time using bvh query        = " << bvhDist << ", which took " << bvhTime.count()
            << " us\n";
  std::cout << "Distance and time using linear bvh query = " << linDist << ", which took " << linTime.count()
            << " us\n";

  return 0;
}
