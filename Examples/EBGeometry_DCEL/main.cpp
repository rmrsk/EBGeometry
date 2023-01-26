/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>
#include <chrono>
#include <random>

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
    file = "../Resources/" + std::string(argv[1]);
  }
  else {
    std::cout << "Missing file name. Use ./a.out 'filename' where 'filename' "
                 "is one of the files in ../Resources. Setting this equal to the armadillo file\n";
    file = "../Resources/armadillo.stl";
  }

  // Three representations of the same object. Note that this reads the mesh three
  // times and builds the BVH twice (there are converters that avoid this, users will
  // only use one of these representations).
  const auto dcelSDF = EBGeometry::Parser::readIntoMesh<T>(file);
  const auto bvhSDF  = EBGeometry::Parser::readIntoFullBVH<T, BV, K>(file);
  const auto linSDF  = EBGeometry::Parser::readIntoLinearBVH<T, BV, K>(file);

  // Sample some random points around the object.
  constexpr size_t Nsamp = 100;

  Vec3 lo = Vec3::infinity();
  Vec3 hi = -Vec3::infinity();

  for (const auto& v : dcelSDF->getMesh()->getAllVertexCoordinates()) {
    lo = min(lo, v);
    hi = max(hi, v);
  }
  const Vec3 delta = hi - lo;

  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));

  std::uniform_real_distribution<T> dist(0.0, 1.0);
  std::vector<Vec3>                 ranPoints;

  for (size_t i = 0; i < Nsamp; i++) {
    ranPoints.emplace_back(3 * (lo + delta * Vec3(dist(rng), dist(rng), dist(rng))));
  }

  // Compute sum of distances to the random points and time the results.
  T dcelSum = 0.0;
  T bvhSum  = 0.0;
  T linSum  = 0.0;

  const auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    dcelSum += dcelSDF->signedDistance(x);
  }
  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    bvhSum += bvhSDF->signedDistance(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    linSum += linSDF->signedDistance(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<T, std::micro> dcelTime = t1 - t0;
  const std::chrono::duration<T, std::micro> bvhTime  = t2 - t1;
  const std::chrono::duration<T, std::micro> linTime  = t3 - t2;

  if (std::abs(bvhSum - dcelSum) > std::numeric_limits<T>::epsilon()) {
    std::cerr << "Full BVH did not give same distance! Diff = " << bvhSum - dcelSum << "\n";
  }
  if (std::abs(linSum - dcelSum) > std::numeric_limits<T>::epsilon()) {
    std::cerr << "Linearized BVH did not give same distance! Diff = " << linSum - dcelSum << "\n";
  }

  // clang-format off
  std::cout << "Bounding box = " << lo << "\t" << hi << "\n";
  std::cout << "Accumulated distance and time using direct DCEL = " << dcelSum << ", which took " << dcelTime.count() / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using full BVH    = " << bvhSum  << ", which took " << bvhTime.count()  / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using compact BVH = " << linSum  << ", which took " << linTime.count()  / Nsamp << " us\n";
  std::cout << "Relative speedup using BVH vs direct DCL        = " << dcelTime.count()/linTime.count() << "\n";
  // clang-format on  

  return 0;
}
