// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_GPUTape.cu
 * @brief  CUDA golden test for DeviceTape: upload compiled tapes and evaluate them in a kernel.
 * @details Tier 7 of the GPU port. For four representative device-eligible scenes (a deep mixed
 * CSG tree, a nested transform chain, a 216-leaf BVH sharp union, and a BVH smooth union) this
 * program compiles the tape on the host, uploads it with DeviceTape, evaluates it for every query
 * point in a grid-stride kernel, and compares the device results against BOTH host references:
 * the host-side tape interpreter (tape.value) and the recursive value() oracle (tree.value). Both
 * float and double run unconditionally, making this binary the device-side analogue of
 * Tests/InstantiateAll.cpp for Tape/DeviceTape (which the host-only object library cannot cover).
 *
 * The scene constructions and margins replicate Tests/TestTape.cpp line-for-line (origins cited at
 * each helper) so host and device coverage stay in lockstep.
 *
 * This is a plain main() program (no Catch2): the CI CUDA lane configures without
 * EBGEOMETRY_BUILD_TESTS, so Catch2 is never fetched there. Without a CUDA device the program
 * exits with code 77, which the GPUTapeGolden ctest registration maps to "Skipped" via
 * SKIP_RETURN_CODE.
 *
 * Scratch strategy: exact global scratch buffers of numPoints * numCoordSlots /
 * numPoints * numValueSlots entries, sliced by point index, so any tape size works with no
 * per-thread cap (fixed-size local scratch is the later performance-tier path).
 * @author Robert Marskar
 */

#include "EBGeometry.hpp" // Pulls in Tape + DeviceTape (active: this TU is compiled by nvcc).

#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <vector>

// ctest SKIP_RETURN_CODE (see the GPUTapeGolden test registration in CMakeLists.txt).
constexpr int SKIP_EXIT_CODE = 77;

using namespace EBGeometry;

// ── Kernel ────────────────────────────────────────────────────────────────────

/**
 * @brief Grid-stride kernel: a_out[i] = a_view.value(a_points[i], per-point scratch slices).
 * @param[in]  a_view         Device-pointer tape view (by value; trivially copyable).
 * @param[in]  a_points       Query points (device, a_numPoints entries).
 * @param[out] a_out          Results (device, a_numPoints entries).
 * @param[in]  a_numPoints    Number of query points.
 * @param[out] a_coordScratch Global scratch, a_numPoints * a_view.numCoordSlots entries (device).
 * @param[out] a_valueScratch Global scratch, a_numPoints * a_view.numValueSlots entries (device).
 */
template <class T>
EBGEOMETRY_GLOBAL void
evaluateTape(
  TapeView<T> a_view, const Vec3T<T>* a_points, T* a_out, int a_numPoints, Vec3T<T>* a_coordScratch, T* a_valueScratch)
{
  for (int i = blockIdx.x * blockDim.x + threadIdx.x; i < a_numPoints; i += gridDim.x * blockDim.x) {
    a_out[i] = a_view.value(a_points[i],
                            a_coordScratch + size_t(i) * a_view.numCoordSlots,
                            a_valueScratch + size_t(i) * a_view.numValueSlots);
  }
}

// ── Host-side helpers (replicated line-for-line from Tests/TestTape.cpp) ──────

namespace {

template <class T>
using IF = ImplicitFunction<T>;

// Margin helpers copied from Tests/TestTape.cpp (exactMargin/formulaMargin): two independent
// traversals of the same trait statics should agree up to floating-point reordering only; smooth
// blends get a slightly looser bound.
template <class T>
double
exactMargin()
{
  return std::is_same_v<T, float> ? 1.0e-4 : 1.0e-12;
}

template <class T>
double
formulaMargin()
{
  return std::is_same_v<T, float> ? 1.0e-3 : 1.0e-9;
}

// A spread of inside/outside/surface-ish query points; copied from Tests/TestTape.cpp
// (queryPoints).
template <class T>
std::vector<Vec3T<T>>
queryPoints()
{
  return {Vec3T<T>::zeros(),
          Vec3T<T>(0.5, 0, 0),
          Vec3T<T>(1.5, 0, 0),
          Vec3T<T>(-1.2, 0.7, 0.3),
          Vec3T<T>(2, 2, 2),
          Vec3T<T>(-3, 1, -2),
          Vec3T<T>(0.1, -0.4, 0.9),
          Vec3T<T>(3, 0, 0),
          Vec3T<T>(0, 3, 0),
          Vec3T<T>(-0.5, -0.5, -0.5)};
}

// A row of disjoint unit spheres, centers a_spacing apart, with tight AABBs; copied from
// Tests/TestTape.cpp (makeSphereRow).
template <class T>
void
makeSphereRow(int                                         a_numSpheres,
              T                                           a_spacing,
              std::vector<std::shared_ptr<SphereSDF<T>>>& a_spheres,
              std::vector<BoundingVolumes::AABBT<T>>&     a_bvs)
{
  for (int i = 0; i < a_numSpheres; i++) {
    const Vec3T<T> center(T(i) * a_spacing, 0, 0);

    a_spheres.push_back(std::make_shared<SphereSDF<T>>(center, T(1)));
    a_bvs.emplace_back(center - Vec3T<T>::ones(), center + Vec3T<T>::ones());
  }
}

// A 6x6x6 grid (216 entries) of mixed, transformed primitives -- spheres, rotated boxes, scaled
// tori, offset capsules -- with conservative AABBs; copied from Tests/TestTape.cpp
// (makeMixedScene). Every entry lowers natively, so the whole scene is device-eligible.
template <class T>
void
makeMixedScene(std::vector<std::shared_ptr<IF<T>>>& a_primitives, std::vector<BoundingVolumes::AABBT<T>>& a_bvs)
{
  constexpr int n = 6;

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        const Vec3T<T> center(T(3 * i), T(3 * j), T(3 * k));

        std::shared_ptr<IF<T>> primitive;

        switch ((i + j + k) % 4) {
        case 0: {
          primitive = Translate<T>(std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(0.8)), center);

          break;
        }
        case 1: {
          primitive = Translate<T>(
            Rotate<T>(std::make_shared<BoxSDF<T>>(Vec3T<T>(-0.7, -0.7, -0.7), Vec3T<T>(0.7, 0.7, 0.7)), T(30), 2),
            center);

          break;
        }
        case 2: {
          primitive =
            Translate<T>(Scale<T>(std::make_shared<TorusSDF<T>>(Vec3T<T>::zeros(), T(0.5), T(0.2)), T(1.5)), center);

          break;
        }
        default: {
          primitive = Translate<T>(
            Offset<T>(std::make_shared<CapsuleSDF<T>>(Vec3T<T>(0, -0.4, 0), Vec3T<T>(0, 0.4, 0), T(0.3)), T(0.1)),
            center);

          break;
        }
        }

        a_primitives.push_back(primitive);
        a_bvs.emplace_back(center - Vec3T<T>(1.5, 1.5, 1.5), center + Vec3T<T>(1.5, 1.5, 1.5));
      }
    }
  }
}

// A 16x16x16 uniform grid over [-3, 18]^3 -- spanning the mixed scene, whose centers run 0..15,
// with inside/on-surface/far coverage -- plus the standard 10-point query spread (4106 points).
template <class T>
std::vector<Vec3T<T>>
testPoints()
{
  std::vector<Vec3T<T>> points = queryPoints<T>();

  constexpr int n = 16;

  const T lo = T(-3);
  const T hi = T(18);

  for (int i = 0; i < n; i++) {
    for (int j = 0; j < n; j++) {
      for (int k = 0; k < n; k++) {
        const T x = lo + (hi - lo) * T(i) / T(n - 1);
        const T y = lo + (hi - lo) * T(j) / T(n - 1);
        const T z = lo + (hi - lo) * T(k) / T(n - 1);

        points.emplace_back(x, y, z);
      }
    }
  }

  return points;
}

// ── Per-scene driver ──────────────────────────────────────────────────────────

/**
 * @brief Compile a_tree, upload it, evaluate it on the device for every test point, and compare
 * against both CPU references (host tape interpreter and recursive value() oracle).
 * @param[in] a_name      Scene name for diagnostics.
 * @param[in] a_precision Precision name for diagnostics.
 * @param[in] a_tree      Source implicit-function tree (must be device-eligible).
 * @param[in] a_margin    Absolute comparison margin.
 * @return Zero on success, one on any mismatch (or a non-device-eligible tape, which is a test
 * FAILURE here, not a skip).
 */
template <class T>
int
runScene(const char* a_name, const char* a_precision, const ImplicitFunction<T>& a_tree, double a_margin)
{
  const Tape<T> tape = compile<T>(a_tree);

  if (!tape.isDeviceEligible()) {
    std::printf("FAIL [%s, %s]: tape is not device-eligible\n", a_name, a_precision);

    return 1;
  }

  const std::vector<Vec3T<T>> points    = testPoints<T>();
  const int                   numPoints = static_cast<int>(points.size());

  // CPU references: host tape interpreter and the recursive value() oracle.
  std::vector<T> cpuTape(points.size());
  std::vector<T> oracle(points.size());

  for (size_t i = 0; i < points.size(); i++) {
    cpuTape[i] = tape.value(points[i]);
    oracle[i]  = a_tree.value(points[i]);
  }

  // Upload; move-construct once so the move path is exercised, then evaluate through the moved-to
  // owner's view.
  DeviceTape<T> deviceTape(tape);
  DeviceTape<T> moved(std::move(deviceTape));

  // Device buffers: points, results, and exact global scratch sliced per point.
  Vec3T<T>* devicePoints       = nullptr;
  T*        deviceOut          = nullptr;
  Vec3T<T>* deviceCoordScratch = nullptr;
  T*        deviceValueScratch = nullptr;

  EBGEOMETRY_CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&devicePoints), points.size() * sizeof(Vec3T<T>)));
  EBGEOMETRY_CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&deviceOut), points.size() * sizeof(T)));
  EBGEOMETRY_CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&deviceCoordScratch),
                                   points.size() * size_t(tape.getNumCoordSlots()) * sizeof(Vec3T<T>)));
  EBGEOMETRY_CUDA_CHECK(cudaMalloc(reinterpret_cast<void**>(&deviceValueScratch),
                                   points.size() * size_t(tape.getNumValueSlots()) * sizeof(T)));

  EBGEOMETRY_CUDA_CHECK(
    cudaMemcpy(devicePoints, points.data(), points.size() * sizeof(Vec3T<T>), cudaMemcpyHostToDevice));

  evaluateTape<T>
    <<<256, 128>>>(moved.view(), devicePoints, deviceOut, numPoints, deviceCoordScratch, deviceValueScratch);

  EBGEOMETRY_CUDA_CHECK(cudaGetLastError());
  EBGEOMETRY_CUDA_CHECK(cudaDeviceSynchronize());

  std::vector<T> gpuOut(points.size());

  EBGEOMETRY_CUDA_CHECK(cudaMemcpy(gpuOut.data(), deviceOut, points.size() * sizeof(T), cudaMemcpyDeviceToHost));

  EBGEOMETRY_CUDA_CHECK(cudaFree(devicePoints));
  EBGEOMETRY_CUDA_CHECK(cudaFree(deviceOut));
  EBGEOMETRY_CUDA_CHECK(cudaFree(deviceCoordScratch));
  EBGEOMETRY_CUDA_CHECK(cudaFree(deviceValueScratch));

  // Compare against BOTH references with the same absolute-margin convention as
  // Tests/TestTape.cpp (withinAbsT).
  int numMismatches = 0;

  for (size_t i = 0; i < points.size(); i++) {
    const double gpu       = double(gpuOut[i]);
    const double deltaTape = std::abs(gpu - double(cpuTape[i]));
    const double deltaTree = std::abs(gpu - double(oracle[i]));

    // Negated conjunction so that a NaN device result fails loudly (NaN comparisons are false, so
    // the naive delta > margin form would silently count a NaN as passing).
    if (!(deltaTape <= a_margin && deltaTree <= a_margin)) {
      if (numMismatches < 5) {
        std::printf("  mismatch [%s, %s] at point (%g, %g, %g): gpu=%.12g cpuTape=%.12g oracle=%.12g margin=%g\n",
                    a_name,
                    a_precision,
                    double(points[i][0]),
                    double(points[i][1]),
                    double(points[i][2]),
                    gpu,
                    double(cpuTape[i]),
                    double(oracle[i]),
                    a_margin);
      }

      numMismatches++;
    }
  }

  std::printf("%s [%s, %s]: %d points, %d mismatches (pool: %zu bytes)\n",
              numMismatches == 0 ? "PASS" : "FAIL",
              a_name,
              a_precision,
              numPoints,
              numMismatches,
              moved.getNumBytes());

  return numMismatches == 0 ? 0 : 1;
}

/**
 * @brief Run all four golden scenes for one precision.
 * @param[in] a_precision Precision name for diagnostics.
 * @return Number of failing scenes.
 */
template <class T>
int
runAllScenes(const char* a_precision)
{
  int failures = 0;

  // (1) Deep mixed tree: Difference(SmoothUnion(Translate(Sphere), Box, s), Rotate(Cylinder));
  // copied from Tests/TestTape.cpp ("deep mixed tree matches value()").
  {
    const auto sphere = std::make_shared<SphereSDF<T>>(Vec3T<T>::zeros(), T(1));
    const auto box    = std::make_shared<BoxSDF<T>>(Vec3T<T>(-0.8, -0.8, -0.8), Vec3T<T>(0.8, 0.8, 0.8));
    const auto cyl    = std::make_shared<CylinderSDF<T>>(Vec3T<T>(0, -1, 0), Vec3T<T>(0, 1, 0), T(0.5));

    const T s = T(0.4);

    const auto smoothUnion = SmoothUnion<T>(Translate<T>(sphere, Vec3T<T>(0.2, 0, 0)), box, s);
    const auto tree        = Difference<T>(smoothUnion, Rotate<T>(cyl, T(25), 0));

    failures += runScene<T>("deep mixed tree", a_precision, *tree, formulaMargin<T>());
  }

  // (2) Nested transform chain: Offset(Annular(Translate(Rotate(Scale(Box)))));
  // copied from Tests/TestTape.cpp ("nested tape evaluation through a host callback is safe",
  // inner tree).
  {
    const auto box  = std::make_shared<BoxSDF<T>>(Vec3T<T>(-1, -1, -1), Vec3T<T>(1, 1, 1));
    const auto tree = Offset<T>(
      Annular<T>(Translate<T>(Rotate<T>(Scale<T>(box, T(1.5)), T(20), 1), Vec3T<T>(0.5, 0, 0)), T(0.1)), T(0.2));

    failures += runScene<T>("nested transform chain", a_precision, *tree, exactMargin<T>());
  }

  // (3) BVH sharp union over the 216-leaf mixed scene (K=4); copied from Tests/TestTape.cpp
  // (requireBVHUnionGolden).
  {
    using BV = BoundingVolumes::AABBT<T>;

    std::vector<std::shared_ptr<IF<T>>> primitives;
    std::vector<BV>                     bvs;

    makeMixedScene<T>(primitives, bvs);

    const BVHUnionIF<T, IF<T>, BV, 4> bvhUnion(primitives, bvs);

    failures += runScene<T>("BVH sharp union (216 leaves, K=4)", a_precision, bvhUnion, exactMargin<T>());
  }

  // (4) BVH smooth union over an 8-sphere row with the default SmoothMinOp blend; copied from
  // Tests/TestTape.cpp ("BVH smooth union with the default SmoothMinOp blend lowers natively").
  {
    using BV = BoundingVolumes::AABBT<T>;

    std::vector<std::shared_ptr<SphereSDF<T>>> spheres;
    std::vector<BV>                            bvs;

    makeSphereRow<T>(8, T(1.5), spheres, bvs);

    const T smoothLen = T(0.25);

    const BVHSmoothUnionIF<T, SphereSDF<T>, BV, 4> smooth(spheres, bvs, smoothLen);

    failures += runScene<T>("BVH smooth union (8 spheres, K=4)", a_precision, smooth, formulaMargin<T>());
  }

  return failures;
}

} // namespace

/**
 * @brief Host entry point: skip (exit 77) without a CUDA device, otherwise run every golden scene
 * for both precisions.
 * @return Zero on success, one on any mismatch, 77 (SKIP_EXIT_CODE) when no CUDA device exists.
 */
int
main()
{
  int deviceCount = 0;

  const cudaError_t err = cudaGetDeviceCount(&deviceCount);

  if (err != cudaSuccess || deviceCount == 0) {
    std::printf("GPUTape: no CUDA device available (%s) -- skipping\n",
                err == cudaSuccess ? "device count is zero" : cudaGetErrorString(err));

    return SKIP_EXIT_CODE;
  }

  int failures = 0;

  failures += runAllScenes<float>("float");
  failures += runAllScenes<double>("double");

  std::printf("GPUTape: %d scene(s) failed\n", failures);

  return failures == 0 ? 0 : 1;
}
