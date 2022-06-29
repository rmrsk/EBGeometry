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

constexpr size_t K = 4;
using T            = float;
using Face         = EBGeometry::DCEL::FaceT<T>;
using Mesh         = EBGeometry::DCEL::MeshT<T>;
using BV           = EBGeometry::BoundingVolumes::AABBT<T>;
using Prim         = EBGeometry::BVH::LinearBVH<T, Face, BV, K>;
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

  // Read each STL file and make them into flattened BVH-trees. These STL files
  // happen to normal vectors pointing inwards so we flip the DCEL normals. Otherwise
  // we could just have called EBGeomtry::Parser::readIntoLinearBVH.
  std::vector<std::shared_ptr<EBGeometry::BVH::LinearBVH<T, Face, BV, K>>> SDFs;
  for (const auto& f : stlFiles) {

    const auto mesh = EBGeometry::Parser::readIntoDCEL<T>(f);
    mesh->flip();

    SDFs.emplace_back(EBGeometry::DCEL::buildFlatBVH<T, BV, K>(mesh));
  }

  // Two types of unions; a standard one and one that use anothe rBVH
  EBGeometry::BVH::BVConstructorT<Prim, BV> bvConstructor = [](const std::shared_ptr<const Prim>& a_prim) -> BV {
    return a_prim->getBoundingVolume();
  };

  EBGeometry::Union<Prim, T>           slowUnion(SDFs, false);
  EBGeometry::UnionBVH<T, Prim, BV, K> fastUnion(SDFs, false, bvConstructor);

  // Sample some random positions
  constexpr size_t Nsamp = 1000;

  const Vec3 lo    = fastUnion.getBoundingVolume().getLowCorner();
  const Vec3 hi    = fastUnion.getBoundingVolume().getHighCorner();
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

  // Time the union calculation
  T sumSlow = 0.0;
  T sumFast = 0.0;

  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);

  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlow += slowUnion.value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumFast += fastUnion.value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  if (std::abs(sumSlow) - std::abs(sumFast) > std::numeric_limits<T>::min()) {
    std::cerr << "Got wrong distance! Diff = " << std::abs(sumSlow) - std::abs(sumFast) << "\n";
  }

  slowTime += (t2 - t1);
  fastTime += (t3 - t2);

  std::cout << "Time using slow union (us) = " << slowTime.count() / Nsamp << "\n";
  std::cout << "Time using fast union (us) = " << fastTime.count() / Nsamp << "\n";
  std::cout << "Average speedup = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";

  return 0;
}
