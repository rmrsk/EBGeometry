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

using T    = float;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;

int
main(int argc, char* argv[])
{

  // Name of the current exec program
  std::string              current_exec_name = argv[0];
  std::vector<std::string> all_args;

  std::string file;

  // Read the input file.
  if (argc == 2) {
    file = "../Objects/" + std::string(argv[1]);
  }
  else {
    std::cerr << "Missing file name. Use ./a.out 'filename' where 'filename' "
                 "is one of the files in ../Objects\n";
  }

  // Three representations of the same object. First, we get the DCEL mesh, and then
  // we convert it to a full BVH tree representation. Then we flatten that tree.
  const auto dcelSDF = EBGeometry::Parser::readIntoDCEL<T>(file);
  const auto bvhSDF  = EBGeometry::DCEL::buildFullBVH<T, BV, K>(dcelSDF);
  const auto linSDF  = bvhSDF->flattenTree();

  // Compute signed distance for this position and time all SDF representations.
  const Vec3 point = Vec3::one();

  const auto t1         = std::chrono::high_resolution_clock::now();
  const T    directDist = dcelSDF->signedDistance(point);
  const auto t2         = std::chrono::high_resolution_clock::now();

  const auto t3      = std::chrono::high_resolution_clock::now();
  const T    bvhDist = bvhSDF->signedDistance(point);
  const auto t4      = std::chrono::high_resolution_clock::now();

  const auto t5      = std::chrono::high_resolution_clock::now();
  const T    linDist = linSDF->signedDistance(point);
  const auto t6      = std::chrono::high_resolution_clock::now();

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
