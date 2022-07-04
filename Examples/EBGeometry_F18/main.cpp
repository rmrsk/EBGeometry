/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <vector>
#include <chrono>
#include <random>
#include <iostream>
#include <filesystem>

#include "../../EBGeometry.hpp"

using namespace EBGeometry;
using namespace EBGeometry::DCEL;

constexpr size_t K = 2;
using T            = float;
using Face         = EBGeometry::DCEL::FaceT<T>;
using Mesh         = EBGeometry::DCEL::MeshT<T>;
using BV           = EBGeometry::BoundingVolumes::AABBT<T>;
using LinBVH       = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
using Vec3         = EBGeometry::Vec3T<T>;

int
main(int argc, char* argv[])
{

  // Get all the STL files in the Resources/F18 directory.
  std::vector<std::string> stlFiles;
  for (const auto& entry : std::filesystem::directory_iterator("../Resources/F18/")) {
    const std::string f   = entry.path();
    const std::string ext = f.substr(f.find_last_of(".") + 1);

    if (ext == "stl" || ext == "STL") {
      stlFiles.emplace_back(f);
    }
  }

  // Read each STL file. These STL files happen to have normal vectors pointing inwards
  // so we flip them.
  std::vector<std::shared_ptr<Mesh>>                                       slowSDFs;
  std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> fastSDFs;
  for (const auto& f : stlFiles) {
    const auto mesh = EBGeometry::Parser::readIntoDCEL<T>(f);
    mesh->flip();

    // Store the grids as basic DCEL grids and as linearized BVHs.
    slowSDFs.emplace_back(mesh);
    fastSDFs.emplace_back(EBGeometry::DCEL::buildFlatBVH<T, BV, K>(mesh));
  }

  // Create four representations of each object. We use a slow/fast union and slow/fast SDF representation of each object.
  EBGeometry::BVH::BVConstructorT<Mesh, BV> bvConstructorSlow = [](const std::shared_ptr<const Mesh>& a_prim) -> BV {
    return BV(a_prim->getAllVertexCoordinates());
  };

  EBGeometry::BVH::BVConstructorT<LinBVH, BV> bvConstructorFast =
    [](const std::shared_ptr<const LinBVH>& a_prim) -> BV { return a_prim->getBoundingVolume(); };

  EBGeometry::Union<T, Mesh>             slowUnionSlowSDF(slowSDFs, false);
  EBGeometry::Union<T, LinBVH>           slowUnionFastSDF(fastSDFs, false);
  EBGeometry::UnionBVH<T, Mesh, BV, K>   fastUnionSlowSDF(slowSDFs, false, bvConstructorSlow);
  EBGeometry::UnionBVH<T, LinBVH, BV, K> fastUnionFastSDF(fastSDFs, false, bvConstructorFast);

  // Sample some random positions
  constexpr size_t Nsamp = 100;

  const Vec3 lo    = fastUnionFastSDF.getBoundingVolume().getLowCorner();
  const Vec3 hi    = fastUnionFastSDF.getBoundingVolume().getHighCorner();
  const Vec3 delta = hi - lo;

  std::mt19937_64 rng(std::chrono::system_clock::now().time_since_epoch().count());

  std::uniform_real_distribution<T> dist(0.0, 1.0);
  std::vector<Vec3>                 ranPoints;

  for (size_t i = 0; i < Nsamp; i++) {
    Vec3 x = Vec3(dist(rng), dist(rng), dist(rng));

    x[0] *= 3 * delta[0];
    x[1] *= 3 * delta[1];
    x[2] *= 3 * delta[2];

    x += -3 * lo;

    ranPoints.emplace_back(x);
  }

  // Time the calculation of a couple of random points for the four union representations.
  T sumSlowSlow = 0.0;
  T sumSlowFast = 0.0;
  T sumFastSlow = 0.0;
  T sumFastFast = 0.0;

  const auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlowSlow += slowUnionSlowSDF.value(x);
  }
  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlowFast += slowUnionFastSDF.value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumFastSlow += fastUnionSlowSDF.value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumFastFast += fastUnionFastSDF.value(x);
  }
  const auto t4 = std::chrono::high_resolution_clock::now();

  // Make sure they all produce the same result!.
  if (std::abs(sumSlowSlow) - std::abs(sumFastFast) > std::numeric_limits<T>::min()) {
    std::cerr << "Got wrong distance! Diff = " << std::abs(sumSlowFast) - std::abs(sumFastFast) << "\n";
  }

  const std::chrono::duration<T, std::micro> slowSlowTime = (t1 - t0);
  const std::chrono::duration<T, std::micro> slowFastTime = (t2 - t1);
  const std::chrono::duration<T, std::micro> fastSlowTime = (t3 - t2);
  const std::chrono::duration<T, std::micro> fastFastTime = (t4 - t3);

  std::cout << "Bounding volume = " << lo << "\t" << hi << "\n";
  std::cout << "Slow union/slow SDF (microseconds/call) = " << slowSlowTime.count() / Nsamp << "\n";
  std::cout << "Slow union/fast SDF (microseconds/call) = " << slowFastTime.count() / Nsamp << "\n";
  std::cout << "Fast union/slow SDF (microseconds/call) = " << fastSlowTime.count() / Nsamp << "\n";
  std::cout << "Fast union/fast SDF (microseconds/call) = " << fastFastTime.count() / Nsamp << "\n";

  return 0;
}
