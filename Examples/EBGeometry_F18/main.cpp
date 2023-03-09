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
using T            = double;
using Vec3         = EBGeometry::Vec3T<T>;
using BV           = EBGeometry::BoundingVolumes::AABBT<T>;
using SlowSDF      = EBGeometry::MeshSDF<T>;
using FastSDF      = EBGeometry::FastCompactMeshSDF<T, BV, K>;

int
main(int argc, char* argv[])
{
  // TLDR: This examples generates the implicit function for a geometry consisting of multiple
  //       parts, where each part is available as an STL file. The implicit function is reconstructed
  //       using four different approaches:
  //
  //       1. Using a standard CSG union and linear search through triangles.
  //       2. Using a standard CSG union and BVH-accelerated search through triangles.
  //       3. Using a BVH-accelerated CSG union and linear search through triangles.
  //       4. Using a BVH-accelerated CSG union and BVH-accelerated search through triangles.
  //
  //       For performance comparison, random positions are sampled inside the bounding volume of the geometry
  //       and the number of calls per second are reported.

  // Get the filenames of all the STL files in the Resources/F18 directory.
  std::vector<std::string> stlFiles;
  for (const auto& entry : std::filesystem::directory_iterator("../Resources/F18/")) {
    const std::string f   = entry.path();
    const std::string ext = f.substr(f.find_last_of(".") + 1);

    if (ext == "stl" || ext == "STL") {
      stlFiles.emplace_back(f);
    }
  }

  // Read each STL file into a DCEL mesh. These STL files happen to have normal vectors pointing inwards
  // so we need to flip them. We also store the bounding volumes for each part, and create the total bounding
  // volume that encloses all parts.
  std::vector<std::shared_ptr<SlowSDF>> slowSDFs;
  std::vector<std::shared_ptr<FastSDF>> fastSDFs;
  std::vector<BV>                       boundingVolumes;

  for (const auto& f : stlFiles) {
    // Read the DCEL mesh.
    const auto mesh = EBGeometry::Parser::readIntoDCEL<T>(f);
    mesh->flip();

    // Store the grids as basic DCEL grids and as linearized BVHs.
    slowSDFs.emplace_back(std::make_shared<SlowSDF>(mesh));
    fastSDFs.emplace_back(std::make_shared<FastSDF>(mesh));

    boundingVolumes.emplace_back(mesh->getAllVertexCoordinates());
  }

  const BV bv = BV(boundingVolumes);

  // Print some stats on the input mesh
  size_t numFacets = 0;
  for (const auto& sdf : slowSDFs) {
    numFacets += sdf->getMesh()->getFaces().size();
  }
  std::cout << "Number of components = " << slowSDFs.size() << "\n";
  std::cout << "Number of triangles  = " << numFacets << "\n";

  // Create four representations of the same object. We use a slow/fast union and
  // slow/fast SDF representation. See EBGeometry_CSG.hpp for these signatures (and EBGeometry_DCEL_BVH.hpp for the BV constructors)
  const auto slowUnionSlowSDF = EBGeometry::Union<T, SlowSDF>(slowSDFs);
  const auto slowUnionFastSDF = EBGeometry::Union<T, FastSDF>(fastSDFs);
  const auto fastUnionSlowSDF = EBGeometry::FastUnion<T, SlowSDF, BV, K>(slowSDFs, boundingVolumes);
  const auto fastUnionFastSDF = EBGeometry::FastUnion<T, FastSDF, BV, K>(fastSDFs, boundingVolumes);

  // Sample some random positions
  constexpr size_t Nsamp = 100;

  const Vec3 lo    = bv.getLowCorner();
  const Vec3 hi    = bv.getHighCorner();
  const Vec3 delta = hi - lo;

  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));

  std::uniform_real_distribution<T> dist(0.0, 1.0);
  std::vector<Vec3>                 ranPoints;

  for (size_t i = 0; i < Nsamp; i++) {
    Vec3 x = Vec3(dist(rng), dist(rng), dist(rng));

    x[0] *= 2 * delta[0];
    x[1] *= 2 * delta[1];
    x[2] *= 2 * delta[2];

    x += -2 * lo;

    ranPoints.emplace_back(x);
  }

  // Time the calculation of a couple of random points for the four union representations.
  T sumSlowSlow = 0.0;
  T sumSlowFast = 0.0;
  T sumFastSlow = 0.0;
  T sumFastFast = 0.0;

  const auto t0 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlowSlow += slowUnionSlowSDF->value(x);
  }
  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumSlowFast += slowUnionFastSDF->value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumFastSlow += fastUnionSlowSDF->value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();
  for (const auto& x : ranPoints) {
    sumFastFast += fastUnionFastSDF->value(x);
  }
  const auto t4 = std::chrono::high_resolution_clock::now();

  // Debug -- make sure all functions produce the same result!.
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
