/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <chrono>
#include <random>
#include <string>
#include <vector>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

// Degree of bounding volume hierarchies. We use a 4-ary tree here, where each
// regular node has four children.
constexpr int K = 4;

using T    = double;
using Meta = short;
using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;

int
main(int argc, char* argv[])
{
  std::string file;

  if (argc == 2) {
    file = "../Resources/" + std::string(argv[1]);
  }
  else {
    std::cout << "Missing file name. Use ./a.out 'filename' where 'filename' "
                 "is one of the files in ../Resources. Setting this equal to the armadillo file\n";
    file = "../Resources/armadillo_binary.stl";
  }

  // Three representations of the same object:
  //   dcelSDF  – FlatMeshSDF: O(N) brute-force over all faces.
  //   meshSDF  – MeshSDF: PackedBVH over DCEL faces (any polygon, SIMD traversal).
  //   triSDF   – TriMeshSDF: PackedBVH over SoA triangle groups (triangles only, SIMD leaf eval).
  //
  // Note that this reads the mesh and builds the BVH tree independently for each
  // representation. There are converters that avoid this, but users will almost always
  // only use one of these representations.
  const auto dcelSDF = EBGeometry::Parser::readIntoMesh<T, Meta>(file);
  const auto meshSDF = EBGeometry::Parser::readIntoPackedBVH<T, Meta, K>(file);
  const auto triSDF  = EBGeometry::Parser::readIntoTriangleBVH<T, Meta, K>(file);

  // Sample some random points around the object.
  constexpr size_t Nsamp = 1000;

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

  // Compute sum of distances to the random points and time each representation.
  T dcelSum = 0.0;
  T meshSum = 0.0;
  T triSum  = 0.0;

  const auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    dcelSum += dcelSDF->signedDistance(x);
  }
  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    meshSum += meshSDF->signedDistance(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    triSum += triSDF->signedDistance(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  const std::chrono::duration<T, std::micro> dcelTime = t1 - t0;
  const std::chrono::duration<T, std::micro> meshTime = t2 - t1;
  const std::chrono::duration<T, std::micro> triTime  = t3 - t2;

  if (std::abs(meshSum - dcelSum) > std::numeric_limits<T>::epsilon()) {
    std::cerr << "MeshSDF did not give same distance as FlatMeshSDF! Diff = " << meshSum - dcelSum << "\n";
  }
  if (std::abs(triSum - dcelSum) > std::numeric_limits<T>::epsilon()) {
    std::cerr << "TriMeshSDF did not give same distance as FlatMeshSDF! Diff = " << triSum - dcelSum << "\n";
  }

  // clang-format off
  std::cout << "Bounding box = " << lo << "\t" << hi << "\n";
  std::cout << "Accumulated distance and time using FlatMeshSDF           = " << dcelSum << ", which took " << dcelTime.count() / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using MeshSDF (PackedBVH)   = " << meshSum << ", which took " << meshTime.count() / Nsamp << " us\n";
  std::cout << "Accumulated distance and time using TriMeshSDF (PackedBVH)= " << triSum  << ", which took " << triTime.count()  / Nsamp << " us\n";
  std::cout << "Relative speedup MeshSDF vs FlatMeshSDF                   = " << dcelTime.count() / meshTime.count() << "\n";
  std::cout << "Relative speedup TriMeshSDF vs FlatMeshSDF                = " << dcelTime.count() / triTime.count()  << "\n";
  // clang-format on

  return 0;
}
