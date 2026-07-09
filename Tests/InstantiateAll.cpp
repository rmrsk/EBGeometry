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
  /* -- Abstract bases --------------------------------------------------- */ \
  template class ImplicitFunction<PREC>;                                     \
  template class SignedDistanceFunction<PREC>;                                \
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
  /* -- CSG implicit functions --------------------------------------------*/ \
  template class UnionIF<PREC>;                                              \
  template class SmoothUnionIF<PREC>;                                        \
  template class IntersectionIF<PREC>;                                       \
  template class SmoothIntersectionIF<PREC>;                                 \
  template class DifferenceIF<PREC>;                                         \
  template class SmoothDifferenceIF<PREC>;                                   \
  template class FiniteRepetitionIF<PREC>;                                   \
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
                                                                               \
  /* -- Points --------------------------------------------------------------*/\
  template class Point<PREC, Meta>;                                         \
                                                                               \
  /* -- Mesh distance functions --------------------------------------------*/\
  template class FlatMeshSDF<PREC, Meta>;                                    \
                                                                               \
  namespace BoundingVolumes {                                                \
  template class AABBT<PREC>;                                                \
  template class SphereT<PREC>;                                              \
  }                                                                          \
                                                                               \
  namespace DCEL {                                                           \
  template class MeshT<PREC, Meta>;                                         \
  template class FaceT<PREC, Meta>;                                         \
  template class EdgeT<PREC, Meta>;                                         \
  template class VertexT<PREC, Meta>;                                       \
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
}

template void
instantiateFunctionTemplates<double>();
template void
instantiateFunctionTemplates<float>();

} // namespace EBGeometry
