/* EBGeometry
 * Copyright © 2023 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

#include <string>
#include <chrono>
#include <thread>
#include <random>
#include <math.h>

#include "../../EBGeometry.hpp"

// Declare our precision.
using T = double;

// Aliases for cutting down on typing.
using AABB = EBGeometry::BoundingVolumes::AABBT<T>;
using Vec3 = EBGeometry::Vec3T<T>;
using SDF  = EBGeometry::SignedDistanceFunction<T>;
using Box  = EBGeometry::BoxSDF<T>;

using namespace std::chrono_literals;

int
main()
{

  // TLDR: This program places some random boxes on a lattice; the boxes represent buildings in a "random city". The buildings are placed on a
  //       an MxM sized lattice where the building width, length, and height are drawn from a uniform distribution with user-specified parameters.

  // Building parameters:
  //
  // K         : BVH tree branching factor
  // N         : Number of random points to sample.
  // M         : Number of buildings per coordinate direction
  // dx        : Minimum separation between the buildings.
  // Wmin, Wmax: Minimum and maximum building width
  // Lmin, Lmax: Minimum and maximum building length
  // Hmin, Hmax: Minimum and maximum building height

  constexpr int N    = 500;
  constexpr int K    = 4;
  constexpr int M    = 234;
  constexpr T   dx   = 0.1;
  constexpr T   Wmin = 1;
  constexpr T   Wmax = 3;
  constexpr T   Lmin = 1;
  constexpr T   Lmax = 3;
  constexpr T   Hmin = 1;
  constexpr T   Hmax = 20;

  std::cout << "Domain is (0,0,0) to " << Vec3(M * (Wmax + dx), M * (Lmax + dx), M * (Hmax + dx)) << "\n";

  // Generate some random buildings on a lattice -- none of these should overlap.
  std::vector<std::shared_ptr<Box>> buildings;

  std::mt19937_64 rng(static_cast<size_t>(std::chrono::system_clock::now().time_since_epoch().count()));
  std::uniform_real_distribution<T> udist(0, 1.0);

  for (int i = 0; i <= M; i++) {
    for (int j = 0; j <= M; j++) {
      const T W = Wmin + udist(rng) * (Wmax - Wmin);
      const T L = Lmin + udist(rng) * (Lmax - Lmin);
      const T H = Hmin + udist(rng) * (Hmax - Hmin);

      const T xLo = i * (Wmax + dx) + 0.5 * (dx + Wmax - W);
      const T xHi = xLo + W;

      const T yLo = j * (Lmax + dx) + 0.5 * (dx + Lmax - L);
      const T yHi = yLo + L;

      const Vec3 lo(xLo, yLo, 0.0);
      const Vec3 hi(xHi, yHi, H);

      buildings.emplace_back(std::make_shared<Box>(lo, hi));
    }
  }

  // Create a standard CSG union.
  EBGeometry::Union<T, Box> slowUnion(buildings);

  // Create an optimized union. Cover each building with an AABB box.
  std::cout << "Partitioning " << std::pow(M, 2) << " buildings" << std::endl;
  EBGeometry::UnionBVH<T, Box, AABB, K> fastUnion(buildings, [](const std::shared_ptr<const Box>& a_box) {
    return AABB(a_box->getLowCorner(), a_box->getHighCorner());
  });

  // Sample some random points in the bounding box of the BVH.
  const AABB& bv = fastUnion.getBoundingVolume();
  const Vec3  lo = bv.getLowCorner();
  const Vec3  hi = bv.getHighCorner();

  std::vector<Vec3> randomPositions;
  for (int i = 0; i < N; i++) {
    const T x = lo[0] + udist(rng) * (hi[0] - lo[0]);
    const T y = lo[1] + udist(rng) * (hi[1] - lo[1]);
    const T z = lo[2] + udist(rng) * (hi[2] - lo[2]);

    randomPositions.emplace_back(Vec3(x, y, z));
  }

  // Time the results using the slow and fast union.
  std::chrono::duration<T, std::micro> slowTime(0.0);
  std::chrono::duration<T, std::micro> fastTime(0.0);

  T sumSlow = 0.0;
  T sumFast = 0.0;

  const auto t1 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumSlow += slowUnion.value(x);
  }
  const auto t2 = std::chrono::high_resolution_clock::now();
  for (const auto& x : randomPositions) {
    sumFast += fastUnion.value(x);
  }
  const auto t3 = std::chrono::high_resolution_clock::now();

  if (sumSlow != sumFast) {
    std::cerr << "Got wrong distance!" << std::endl;

    return 2;
  }

  slowTime += (t2 - t1);
  fastTime += (t3 - t2);

  std::cout << "Time using slow union (us) = " << slowTime.count() / N << "\n";
  std::cout << "Time using fast union (us) = " << fastTime.count() / N << "\n";
  std::cout << "Average speedup = " << (1.0 * slowTime.count()) / (1.0 * fastTime.count()) << "\n";

  return 0;
}
