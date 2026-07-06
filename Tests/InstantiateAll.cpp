// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

// Single translation unit that explicitly instantiates EBGeometry's public class
// templates. It is compiled but never run: it exists so that clang-tidy (see
// clang-tidy-check.sh and the Static-analysis CI job) analyses every class body,
// regardless of which classes the unit tests happen to exercise. When you add a
// new public class template, add it here so the analysis keeps full coverage.

#include "EBGeometry.hpp"

namespace EBGeometry {

using T    = double;
using Meta = short;

// -- Vectors, polygons -------------------------------------------------------
template class Vec2T<T>;
template class Vec3T<T>;
template class Polygon2D<T>;

// -- Abstract bases ----------------------------------------------------------
template class ImplicitFunction<T>;
template class SignedDistanceFunction<T>;

// -- Analytic signed distance functions --------------------------------------
template class PlaneSDF<T>;
template class SphereSDF<T>;
template class BoxSDF<T>;
template class TorusSDF<T>;
template class CylinderSDF<T>;
template class InfiniteCylinderSDF<T>;
template class CapsuleSDF<T>;
template class InfiniteConeSDF<T>;
template class ConeSDF<T>;
template class RoundedBoxSDF<T>;
template class RoundedCylinderSDF<T>;
template class PerlinSDF<T>;

// -- CSG implicit functions --------------------------------------------------
template class UnionIF<T>;
template class SmoothUnionIF<T>;
template class IntersectionIF<T>;
template class SmoothIntersectionIF<T>;
template class DifferenceIF<T>;
template class SmoothDifferenceIF<T>;
template class FiniteRepetitionIF<T>;

// -- Transformation implicit functions ---------------------------------------
template class ComplementIF<T>;
template class TranslateIF<T>;
template class RotateIF<T>;
template class OffsetIF<T>;
template class ScaleIF<T>;
template class AnnularIF<T>;
template class BlurIF<T>;
template class MollifyIF<T>;
template class ElongateIF<T>;
template class ReflectIF<T>;

// -- File readers ------------------------------------------------------------
template class STL<T>;
template class PLY<T>;
template class VTK<T>;
template class OBJ<T>;

// -- Triangles ---------------------------------------------------------------
template class Triangle<T, Meta>;

// -- Mesh distance functions -------------------------------------------------
template class FlatMeshSDF<T, Meta>;

// Free-function templates (Parser::read*) and member templates (convertToDCEL)
// are not reached by explicit class instantiation, so odr-use them here to pull
// their bodies into the analysis. This function is compiled but never called.
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

  (void)STL<T>().convertToDCEL<Meta>();
  (void)PLY<T>().convertToDCEL<Meta>();
  (void)VTK<T>().convertToDCEL<Meta>();
  (void)OBJ<T>().convertToDCEL<Meta>();
}

} // namespace EBGeometry

namespace EBGeometry::BoundingVolumes {

template class AABBT<double>;
template class SphereT<double>;

} // namespace EBGeometry::BoundingVolumes

namespace EBGeometry::DCEL {

template class MeshT<double, short>;
template class FaceT<double, short>;
template class EdgeT<double, short>;
template class VertexT<double, short>;

} // namespace EBGeometry::DCEL
