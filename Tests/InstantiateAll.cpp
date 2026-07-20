// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Single translation unit that explicitly instantiates EBGeometry's public class
// templates. It is compiled but never run: it exists so that clang-tidy (see
// clang-tidy-check.sh and the Static-analysis CI job) analyses every class body,
// regardless of which classes the unit tests happen to exercise. Every class is
// instantiated for both `double` and `float` -- the two precisions can hit
// different narrowing/alignment/static_assert code paths, and float has
// historically had zero compile-time coverage anywhere in the project. When you
// add a new public class template, add it to the EBGEOMETRY_INSTANTIATE_ALL
// macro below (not as a bare `template class Foo<T>;` line) so both precisions
// stay covered together.

#include "EBGeometry.hpp"

namespace EBGeometry {

using Meta = short;

// clang-format off
#define EBGEOMETRY_INSTANTIATE_ALL(PREC)                                     \
  /* -- Vectors, polygons ------------------------------------------------ */ \
  template class Vec2T<PREC>;                                                \
  template class Vec3T<PREC>;                                                \
  template class Polygon2D<PREC>;                                            \
                                                                               \
  /* -- Type-erased base ------------------------------------------------- */ \
  template class ImplicitFunction<PREC>;                                     \
                                                                               \
  /* -- Analytic distance-formula traits ----------------------------------*/ \
  template struct PlaneOp<PREC>;                                             \
  template struct SphereOp<PREC>;                                            \
  template struct BoxOp<PREC>;                                               \
  template struct TorusOp<PREC>;                                             \
  template struct CylinderOp<PREC>;                                          \
  template struct InfiniteCylinderOp<PREC>;                                  \
  template struct CapsuleOp<PREC>;                                           \
  template struct InfiniteConeOp<PREC>;                                      \
  template struct ConeOp<PREC>;                                              \
  template struct RoundedBoxOp<PREC>;                                        \
  template struct RoundedCylinderOp<PREC>;                                   \
                                                                               \
  /* -- Concrete trait-based leaves ---------------------------------------*/ \
  template class ImplicitFunction<PREC, PlaneOp<PREC>>;                      \
  template class ImplicitFunction<PREC, SphereOp<PREC>>;                     \
  template class ImplicitFunction<PREC, BoxOp<PREC>>;                        \
  template class ImplicitFunction<PREC, TorusOp<PREC>>;                      \
  template class ImplicitFunction<PREC, CylinderOp<PREC>>;                   \
  template class ImplicitFunction<PREC, InfiniteCylinderOp<PREC>>;           \
  template class ImplicitFunction<PREC, CapsuleOp<PREC>>;                    \
  template class ImplicitFunction<PREC, InfiniteConeOp<PREC>>;               \
  template class ImplicitFunction<PREC, ConeOp<PREC>>;                       \
  template class ImplicitFunction<PREC, RoundedBoxOp<PREC>>;                 \
  template class ImplicitFunction<PREC, RoundedCylinderOp<PREC>>;            \
                                                                               \
  /* -- Analytic signed distance functions --------------------------------*/ \
  template class PlaneSDF<PREC>;                                             \
  template class SphereSDF<PREC>;                                            \
  template class BoxSDF<PREC>;                                               \
  template class TorusSDF<PREC>;                                             \
  template class CylinderSDF<PREC>;                                          \
  template class InfiniteCylinderSDF<PREC>;                                  \
  template class CapsuleSDF<PREC>;                                           \
  template class InfiniteConeSDF<PREC>;                                      \
  template class ConeSDF<PREC>;                                              \
  template class RoundedBoxSDF<PREC>;                                        \
  template class RoundedCylinderSDF<PREC>;                                   \
  template class PerlinSDF<PREC>;                                            \
                                                                               \
  /* -- CSG reduction traits ----------------------------------------------*/ \
  template struct UnionOp<PREC>;                                             \
  template struct IntersectionOp<PREC>;                                     \
  template struct DifferenceOp<PREC>;                                       \
  template struct SmoothMinOp<PREC>;                                        \
  template struct SmoothMaxOp<PREC>;                                        \
  template struct ExpMinOp<PREC>;                                           \
                                                                               \
  /* -- CSG implicit functions (smooth combiners: default Blend plus one  */ \
  /* -- non-default Blend each, so both lowerings stay compile-checked) ---*/ \
  template class UnionIF<PREC>;                                              \
  template class SmoothUnionIF<PREC>;                                        \
  template class SmoothUnionIF<PREC, ExpMinOp<PREC>>;                        \
  template class IntersectionIF<PREC>;                                       \
  template class SmoothIntersectionIF<PREC>;                                 \
  template class SmoothIntersectionIF<PREC, ExpMinOp<PREC>>;                 \
  template class DifferenceIF<PREC>;                                         \
  template class SmoothDifferenceIF<PREC>;                                   \
  template class SmoothDifferenceIF<PREC, ExpMinOp<PREC>>;                   \
  template class FiniteRepetitionIF<PREC>;                                   \
                                                                               \
  /* -- BVH-accelerated CSG unions (default + non-default Blend, so both  */ \
  /* -- native and host-fallback tape lowerings stay compile-checked) ------*/ \
  template class BVHUnionIF<PREC, ImplicitFunction<PREC>,                    \
                            BoundingVolumes::AABBT<PREC>, 4>;                \
  template class BVHSmoothUnionIF<PREC, ImplicitFunction<PREC>,              \
                                  BoundingVolumes::AABBT<PREC>, 4>;          \
  template class BVHSmoothUnionIF<PREC, ImplicitFunction<PREC>,              \
                                  BoundingVolumes::AABBT<PREC>, 4,           \
                                  ExpMinOp<PREC>>;                           \
                                                                               \
  /* -- Transformation distance-formula traits ----------------------------*/ \
  template struct ComplementOp<PREC>;                                       \
  template struct TranslateOp<PREC>;                                        \
  template struct RotateOp<PREC>;                                           \
  template struct OffsetOp<PREC>;                                           \
  template struct ScaleOp<PREC>;                                            \
  template struct AnnularOp<PREC>;                                          \
  template struct ElongateOp<PREC>;                                         \
  template struct ReflectOp<PREC>;                                          \
                                                                               \
  /* -- Flat tape + interpreter -------------------------------------------*/ \
  /* DeviceTape<PREC> is deliberately absent: it is GPU-only (inert without  */ \
  /* an offload compiler), so it cannot be instantiated in this host-only    */ \
  /* TU. Its device-side analogue is Tests/GPU/EBGeometry_GPUTape.hpp, which */ \
  /* instantiates and exercises it for both precisions under nvcc (via the   */ \
  /* .cu twin) or hipcc (via the .hip twin).                                 */ \
  template class Tape<PREC>;                                                 \
  template struct TapeView<PREC>;                                            \
  template class TapeBuilder<PREC>;                                          \
  template struct TapeBVHNode<PREC>;                                         \
  template struct TapeBVHBlock<PREC>;                                        \
                                                                               \
  /* -- Transformation implicit functions ----------------------------------*/\
  template class ComplementIF<PREC>;                                         \
  template class TranslateIF<PREC>;                                         \
  template class RotateIF<PREC>;                                            \
  template class OffsetIF<PREC>;                                            \
  template class ScaleIF<PREC>;                                             \
  template class AnnularIF<PREC>;                                           \
  template class BlurIF<PREC>;                                              \
  template class MollifyIF<PREC>;                                           \
  template class ElongateIF<PREC>;                                          \
  template class ReflectIF<PREC>;                                           \
                                                                               \
  /* -- File readers ------------------------------------------------------*/ \
  template class STL<PREC>;                                                  \
  template class PLY<PREC>;                                                  \
  template class VTK<PREC>;                                                  \
  template class OBJ<PREC>;                                                  \
                                                                               \
  /* -- Triangles -----------------------------------------------------------*/\
  template class Triangle<PREC, Meta>;                                      \
  template struct TriangleSoAT<PREC, 4>;                                     \
  template struct TriangleAoSoA<PREC, Meta, 4>;                              \
                                                                               \
  /* -- Mesh distance functions --------------------------------------------*/\
  template class FlatMeshSDF<PREC, Meta>;                                    \
                                                                               \
  /* -- Point clouds --------------------------------------------------------*/\
  template struct PointSoAT<PREC>;                                           \
  template struct PointAoSoA<PREC, Meta>;                                    \
  template class PointCloudBVH<PREC, Meta>;                                  \
  template class PointCloudHashGrid<PREC, Meta>;                             \
                                                                               \
  namespace BoundingVolumes {                                                \
  template class AABBT<PREC>;                                                \
  template class SphereT<PREC>;                                              \
  }                                                                          \
                                                                               \
  namespace BVH {                                                            \
  template struct PackedNode<PREC, 4>;                                       \
  template struct PackedNode<PREC, 8>;                                       \
  }                                                                          \
                                                                               \
  namespace DCEL {                                                           \
  template class MeshT<PREC, Meta>;                                         \
  template class FaceT<PREC, Meta>;                                         \
  template class EdgeT<PREC, Meta>;                                         \
  template class VertexT<PREC, Meta>;                                       \
  template class EdgeIteratorT<PREC, Meta>;                                 \
  }

EBGEOMETRY_INSTANTIATE_ALL(double)
EBGEOMETRY_INSTANTIATE_ALL(float)

#undef EBGEOMETRY_INSTANTIATE_ALL
// clang-format on

// Free-function templates (Parser::read*) and member templates (convertToDCEL)
// are not reached by explicit class instantiation, so odr-use them here to pull
// their bodies into the analysis. This function is compiled but never called.
template <class T>
[[maybe_unused]] static void
instantiateFunctionTemplates()
{
  const std::string              file;
  const std::vector<std::string> files;

  (void)Parser::readPLY<T>(file);
  (void)Parser::readPLY<T>(files);
  (void)Parser::readSTL<T>(file);
  (void)Parser::readSTL<T>(files);
  (void)Parser::readOBJ<T>(file);
  (void)Parser::readOBJ<T>(files);
  (void)Parser::readVTK<T>(file);
  (void)Parser::readVTK<T>(files);

  (void)Parser::readIntoDCEL<T, Meta>(file);
  (void)Parser::readIntoDCEL<T, Meta>(files);
  (void)Parser::readIntoMesh<T, Meta>(file);
  (void)Parser::readIntoMesh<T, Meta>(files);
  (void)Parser::readIntoPackedBVH<T, Meta>(file);
  (void)Parser::readIntoPackedBVH<T, Meta>(files);
  (void)Parser::readIntoTriangles<T, Meta>(file);
  (void)Parser::readIntoTriangles<T, Meta>(files);
  (void)Parser::readIntoTriangleBVH<T, Meta>(file);
  (void)Parser::readIntoTriangleBVH<T, Meta>(files);

  (void)STL<T>().template convertToDCEL<Meta>();
  (void)PLY<T>().template convertToDCEL<Meta>();
  (void)VTK<T>().template convertToDCEL<Meta>();
  (void)OBJ<T>().template convertToDCEL<Meta>();

  // The free tape compiler is not reached by explicit class instantiation, so odr-use it here
  // over a small sphere so both precisions are analysed (Tape::value / TapeView::value are covered
  // by the explicit Tape/TapeView instantiations above).
  const SphereSDF<T> sphere;
  const Tape<T>      tape = EBGeometry::compile<T>(sphere);
  (void)tape.value(Vec3T<T>::zeros());

  // The shared PackedBVH node-emission stages, the per-point SFC binning helpers, and the
  // non-template RadixSort host references: odr-use them so their bodies join the analysis (the
  // template ones are additionally reached through the SFC-constructor instantiations above, but
  // odr-using them directly keeps the coverage independent of that constructor's internals).
  const std::vector<Vec3T<T>>     positions(1, Vec3T<T>::zeros());
  BVH::PackedNode<T, 4>           packedNode;
  const BoundingVolumes::AABBT<T> pointBV(positions.front(), positions.front());
  BVH::fillLeafNode<T, 4>(packedNode, &pointBV, 0, 1, 1, 1);
  BVH::fillInteriorNode<T, 4>(&packedNode, 0, 0, 1);
  (void)BVH::paddedDfsIndex(0, 0, 0, 4);
  (void)BVH::paddedSubtreeSize(0, 0, 4);
  (void)BVH::paddedTreeDepth(1, 4);
  (void)SFC::computeBin<T>(positions.front(), positions.front(), Vec3T<T>::ones());
  (void)SFC::binningDelta<T>(positions.front(), positions.front());

  std::vector<uint64_t> radixKeys;
  std::vector<uint32_t> radixValues;
  RadixSort::hostStableSort(radixKeys, radixValues);
  RadixSort::hostExclusiveScan(radixValues.data(), 0, 1);
  RadixSort::hostPass(radixKeys.data(), radixValues.data(), 0, 0, 1, nullptr, nullptr);
  (void)RadixSort::digit(0, 0);
}

template void
instantiateFunctionTemplates<double>();
template void
instantiateFunctionTemplates<float>();

} // namespace EBGeometry
