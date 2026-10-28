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

// clang-format off
#define EBGEOMETRY_INSTANTIATE_ALL(PREC)                                     \
  /* -- Vectors ---------------------------------------------------------- */ \
  template class Vec2T<PREC>;                                                \
  template class Vec3T<PREC>;                                                \
  template struct Array<PREC, 3>;                                            \
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
  /* -- CSG: blends, BVH unions, expressions and tapes ----------------------*/ \
  template struct SmoothMinOp<PREC>;                                         \
  template struct SmoothMaxOp<PREC>;                                         \
  template struct ExpMinOp<PREC>;                                            \
  template struct ExpMaxOp<PREC>;                                            \
  template class BVHUnion<PREC, SphereSDF<PREC>, 4>;                       \
  template class BVHSmoothUnion<PREC, SphereSDF<PREC>, 4>;                 \
  template class BVHSmoothUnion<PREC, BoxSDF<PREC>, 4, ExpMinOp<PREC>>;    \
  template struct Affine3T<PREC>;                                            \
  template class Expr<PREC>;                                                 \
  template class Tape<PREC>;                                                 \
  template class Tape<PREC, BVHUnion<PREC, SphereSDF<PREC>, 4>, MeshSDF<PREC, 8>>; \
                                                                               \
  /* -- File readers ------------------------------------------------------*/ \
  template class PolygonSoup<PREC>;                                          \
                                                                               \
  /* -- Triangles -----------------------------------------------------------*/\
  template class Triangle<PREC>;                                            \
  template struct TriangleSoAT<PREC, 4>;                                     \
  template struct TriangleAoSoA<PREC, 4>;                                    \
  template struct TriangleSoAT<PREC, 16>;                                    \
  template struct TriangleDistance::Vec3V<PREC>;                             \
                                                                               \
  /* -- Mesh distance functions --------------------------------------------*/\
  template class FlatMeshSDF<PREC>;                                          \
  template class MeshSDF<PREC, 4>;                                           \
  template class TriMeshSDF<PREC, 4, 4>;                                     \
  template class BVHUnion<PREC, TriMeshSDF<PREC, 4, 4>, 4>;                  \
  template class BVHSmoothUnion<PREC, MeshSDF<PREC, 4>, 4>;                  \
  template struct ClosestFace<PREC>;                                         \
                                                                               \
  /* -- Point clouds --------------------------------------------------------*/\
  template struct PointSoAT<PREC>;                                           \
  template struct PointAoSoA<PREC>;                                          \
  template struct PointCloud::Hit<PREC>;                                     \
  template class PointCloud::KBest<PREC>;                                    \
  template class PointCloudBVH<PREC>;                                        \
  template class PointCloudHashGrid<PREC>;                                   \
                                                                               \
  namespace BoundingVolumes {                                                \
  template class AABBT<PREC>;                                                \
  }                                                                          \
                                                                               \
  namespace DCEL {                                                           \
  template class MeshT<PREC>;                                               \
  template class FaceT<PREC>;                                               \
  template class EdgeT<PREC>;                                               \
  template class VertexT<PREC>;                                             \
  }                                                                          \
                                                                               \
  /* -- BVH ---------------------------------------------------------------- */\
  template class BVH::TreeBVH<PREC, Vec3T<PREC>, BoundingVolumes::AABBT<PREC>, 4>; \
  template class BVH::PackedBVH<PREC, Vec3T<PREC>, 4>;                       \
  template class BVH::PackedBVH<PREC, uint32_t, 4>;                          \
  template struct BVH::ChildBoxes<PREC, 4>;                                 \
  template struct BVH::PointQuery<PREC>;                                     \
  template struct BVH::RayQuery<PREC>;                                       \
                                                                               \
  /* -- Ray queries ---------------------------------------------------------*/\
  template struct RayT<PREC>;                                                \
  template struct HitT<PREC>;                                                \
  template struct RayOptions<PREC>;                                          \
  template struct SpanEndT<PREC>;                                            \
  template struct RaySpansT<PREC>;                                           \
  template struct RayForm<BVHUnion<PREC, SphereSDF<PREC>, 4>>;               \
  template struct RayForm<Tape<PREC>>;                                       \
  template struct RayForm<PlaneSDF<PREC>>;                                   \
  template struct RayForm<SphereSDF<PREC>>;                                  \
  template struct RayForm<BoxSDF<PREC>>;                                     \
  template struct RayForm<CylinderSDF<PREC>>;                                \
  template struct RayForm<InfiniteCylinderSDF<PREC>>;                        \
  template struct RayForm<CapsuleSDF<PREC>>;                                 \
  template struct RayForm<TriMeshSDF<PREC, 4, 4>>;                           \
  template struct RayForm<MeshSDF<PREC, 4>>;                                 \
  template struct RayForm<FlatMeshSDF<PREC>>;                                \
  template struct RayForm<Fused::Node<Fused::UnionOp, SphereSDF<PREC>, BoxSDF<PREC>>>;         \
  template struct RayForm<Fused::Node<Fused::IntersectionOp, SphereSDF<PREC>, BoxSDF<PREC>>>;  \
  template struct RayForm<Fused::Node<Fused::ComplementOp, SphereSDF<PREC>>>;                  \
  template struct RayForm<Fused::Node<Fused::AffineOp<PREC>, SphereSDF<PREC>>>;                \
  /* -- Renderer ----------------------------------------------------------*/ \
  template struct Render::Camera<PREC>;                                      \
  template struct Render::Settings<PREC>;                                    \
  template struct Render::PixelQuery<PREC, Tape<PREC>>;                      \
  template class Render::Renderer<PREC>;                                     \
  template Render::Stats Render::Renderer<PREC>::render<Tape<PREC>>(         \
    const Tape<PREC>&, const Render::Camera<PREC>&, Render::Image&) const;   \
  template Render::Stats Render::Renderer<PREC>::renderOnDevice<Tape<PREC>>( \
    const Tape<PREC>&, const Render::Camera<PREC>&, Render::Image&) const;   \
  template void Render::view<Tape<PREC>>(const Tape<PREC>&, int, char**);    \
  template void Render::view<Tape<PREC>>(const Tape<PREC>&, const Tape<PREC>&, int, char**); \
  template void Render::view<Tape<PREC>>(const Tape<PREC>&, Pool&, int, char**); \
  template Render::Stats Render::Renderer<PREC>::render<TriMeshSDF<PREC, 4, 4>>( \
    const TriMeshSDF<PREC, 4, 4>&, const Render::Camera<PREC>&, Render::Image&) const; \
                                                                               \
  /* -- GPU memory foundation (POD storage) ------------------------------- */ \
  template struct PODVector<PREC>;                                           \
  template struct PODSpan<PREC>;                                             \
  template struct PODVector<Vec3T<PREC>>;

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

  // GPU memory foundation: the non-template Pool/MemoryResource machinery and the PODVector
  // build/query surface (odr-used so clang-tidy and the strong warning set analyse their bodies
  // for both precisions).
  {
    HostMemoryResource& resource = hostMemoryResource();

    Pool pool(resource, 256);

    PODVector<T> vec;
    vec.reserveFrom(pool, 4);
    vec.push_back(pool.base(), T(1));

    const T source[2] = {T(2), T(3)};
    vec.assign(pool.base(), source, 2);

    pool.freeze();

    (void)vec.at(pool.base(), 0);
    (void)vec.data(pool.base());
    (void)vec.bind(pool.base());
    (void)static_cast<const PODVector<T>&>(vec).bind(static_cast<const void*>(pool.base()));

    Pool mirrored = Pool::mirror(pool, resource);

    (void)mirrored.base();
    (void)mirrored.mirrorOf();
    (void)pool.usedBytes();
    (void)pool.capacityBytes();
    (void)pool.control();
    (void)pool.id();
    (void)pool.resource().isDeviceAccessible();
    (void)vec.endByte();
  }

  (void)Parser::readPLY<T>(file);
  (void)Parser::readPLY<T>(files);
  (void)Parser::readSTL<T>(file);
  (void)Parser::readSTL<T>(files);
  (void)Parser::readOBJ<T>(file);
  (void)Parser::readOBJ<T>(files);
  (void)Parser::readVTK<T>(file);
  (void)Parser::readVTK<T>(files);

  // A second, unfrozen Pool for every DCEL-mesh-building entry point below (the one above is
  // frozen by this point, and MeshT reservations are forbidden against a frozen Pool).
  Pool meshPool(hostMemoryResource());

  (void)Parser::readIntoDCEL<T>(file, meshPool);
  (void)Parser::readIntoDCEL<T>(files, meshPool);
  (void)Parser::readIntoFlatMeshSDF<T>(file, meshPool);
  (void)Parser::readIntoFlatMeshSDF<T>(files, meshPool);
  using DefaultMeshSDF    = MeshSDF<T, BVH::DefaultBranchingRatio<T>()>;
  using DefaultTriMeshSDF = TriMeshSDF<T, BVH::DefaultBranchingRatio<T>(), TriangleSoA::DefaultWidth<T>()>;

  (void)Parser::readIntoPolygonSoup<T>(file);
  (void)Parser::readIntoPolygonSoup<T>(files);
  (void)Parser::readIntoMeshSDF<T>(file, meshPool);
  (void)Parser::readIntoMeshSDF<T>(files, meshPool);
  (void)Parser::readIntoMeshSDF<T>(
    file, meshPool, BVH::Construction::SAH, DefaultMeshSDF::defaultConstructionOptions());
  (void)Parser::readIntoMeshSDF<T>(
    files, meshPool, BVH::Construction::SAH, DefaultMeshSDF::defaultConstructionOptions());
  (void)Parser::readIntoTriangles<T>(file);
  (void)Parser::readIntoTriangles<T>(files);
  (void)Parser::readIntoTriMeshSDF<T>(file, meshPool);
  (void)Parser::readIntoTriMeshSDF<T>(files, meshPool);
  (void)Parser::readIntoTriMeshSDF<T>(
    file, meshPool, BVH::Construction::SAH, DefaultTriMeshSDF::defaultConstructionOptions(4));
  (void)Parser::readIntoTriMeshSDF<T>(
    files, meshPool, BVH::Construction::SAH, DefaultTriMeshSDF::defaultConstructionOptions(4));

  // Queries on any function of a point.
  {
    const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));

    (void)approximateBoundingVolumeOctree(sphere, -Vec3T<T>::ones(), Vec3T<T>::ones(), 2U, T(0));
    (void)normal(sphere, Vec3T<T>::ones(), T(1.0e-3));
  }

  // GPU::transform, through a query functor (a host loop in this host-only translation unit).
  {
    struct SphereQuery
    {
      SphereSDF<T> sphere;

      T
      operator()(const Vec3T<T>& a_point) const noexcept
      {
        return sphere.signedDistance(a_point);
      }
    };

    const SphereQuery           query{SphereSDF<T>(Vec3T<T>::zeros(), T(1))};
    const std::vector<Vec3T<T>> points{Vec3T<T>::ones()};
    std::vector<T>              results(1);

    GPU::transform(query, points, results);
    GPU::transform<256>(query, points, results);
    GPU::transform(query, PODSpan<const Vec3T<T>>{points.data(), 1}, PODSpan<T>{results.data(), 1});

    Pool                pool(emulatedDeviceMemoryResource());
    PODVector<Vec3T<T>> in;
    PODVector<T>        out;

    in.reserveFrom(pool, 1);
    out.reserveFrom(pool, 1);
    in.assign(pool.base(), points.data(), 1);
    out.assign(pool.base(), results.data(), 1);
    pool.freeze();

    GPU::transform(query, pool, in, out);
    (void)GPU::deviceAvailable();
    (void)GPU::memoryResource();
  }

  // The ray queries on a shape with a ray form and on a marched one, for rays and for segments.
  {
    const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));
    const TorusSDF<T>  torus(Vec3T<T>::zeros(), T(1), T(0.25));
    const RayT<T>      ray{-T(3) * Vec3T<T>::ones(), Vec3T<T>(T(1), T(0), T(0))};

    (void)firstHit(sphere, ray, T(0), T(10));
    (void)firstHit(torus, ray, T(0), T(10));
    (void)firstHit(torus, Vec3T<T>::zeros(), Vec3T<T>::ones());
    (void)intersects(sphere, ray, T(0), T(10));
    (void)intersects(torus, ray, T(0), T(10));
    (void)intersects(torus, Vec3T<T>::zeros(), Vec3T<T>::ones());

    // Compositions traced from their operands' spans, and marched on request.
    const auto fused = Fused::Difference(Fused::Translate(sphere, Vec3T<T>::ones()), torus);

    RayOptions<T> march;

    march.m_march = true;

    (void)firstHit(fused, ray, T(0), T(10));
    (void)firstHit(fused, ray, T(0), T(10), march);
    (void)intersects(fused, Vec3T<T>::zeros(), Vec3T<T>::ones());

    Pool                                         pool(hostMemoryResource());
    const std::vector<SphereSDF<T>>              spheres(2, sphere);
    const std::vector<BoundingVolumes::AABBT<T>> boxes(2, sphere.computeBoundingVolume());
    const BVHUnion<T, SphereSDF<T>, 4>           bvhUnion(pool, spheres, boxes);
    const Tape<T>                                tape(pool, Difference(Union(sphere, torus), Complement(sphere)));

    (void)firstHit(bvhUnion, ray, T(0), T(10));
    (void)firstHit(tape, ray, T(0), T(10));
    (void)firstHit(Fused::Union(bvhUnion, tape), ray, T(0), T(10));
  }

  // Every builder function, compiled into a tape.
  {
    const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));
    const BoxSDF<T>    box(-Vec3T<T>::ones(), Vec3T<T>::ones());

    const std::vector<SphereSDF<T>> spheres(2, sphere);
    const std::vector<Expr<T>>      exprs = {sphere, box};

    const Expr<T> expr =
      Union(std::vector<Expr<T>>{Union(sphere, box),
                                 Union(spheres),
                                 SmoothUnion(sphere, box, T(0.1)),
                                 SmoothUnion(exprs, T(0.1), BlendKind::Exponential),
                                 Intersection(sphere, box),
                                 Intersection(exprs),
                                 SmoothIntersection(sphere, box, T(0.1)),
                                 SmoothIntersection(exprs, T(0.1)),
                                 Difference(sphere, box),
                                 Difference(sphere, spheres),
                                 SmoothDifference(sphere, box, T(0.1)),
                                 SmoothDifference(sphere, spheres, T(0.1)),
                                 FiniteRepetition(sphere, Vec3T<T>::ones(), Vec3T<T>::ones(), Vec3T<T>::ones()),
                                 Complement(sphere),
                                 Translate(sphere, Vec3T<T>::ones()),
                                 Rotate(sphere, T(30), size_t(1)),
                                 Rotate(sphere, T(30), Vec3T<T>::ones()),
                                 Scale(sphere, T(2)),
                                 Reflect(sphere, size_t(0)),
                                 Offset(sphere, T(0.1)),
                                 Annular(sphere, T(0.1)),
                                 Elongate(sphere, Vec3T<T>::ones()),
                                 Blur(sphere, T(0.1)),
                                 Mollify(sphere, T(0.1), size_t(3))});

    const Tape<T> tape(meshPool, expr);

    (void)tape.signedDistance(Vec3T<T>::zeros());
    (void)tape.computeBoundingVolume();
    (void)tape.getDistanceQuality();
    (void)expr.computeBoundingVolume();
    (void)expr.getDistanceQuality();
    (void)tape.getNumPrograms();
    (void)tape.getNumClauses();
    (void)tape.getNumBVHUnions();
    (void)tape.getCallDepth();
    (void)Tape<T>(tape.layout(), tape.location()).signedDistance(Vec3T<T>::zeros());
    (void)tape.rebasedView(meshPool);
    (void)tape.deepCopy(meshPool);
    (void)normal(tape, Vec3T<T>::ones(), T(1.0e-3));

    const PrimitiveRef<T> leaf = tape.closestPrimitive(Vec3T<T>::zeros());

    (void)tape.template isPrimitive<SphereSDF<T>>(leaf);
    (void)tape.template primitive<SphereSDF<T>>(leaf);
    tape.visitPrimitive(leaf, [](const auto& a_primitive) { (void)a_primitive.signedDistance(Vec3T<T>::zeros()); });
  }

  // Every fused operation, a fused shape with a pool-resident operand, and its views and copies.
  {
    const SphereSDF<T> sphere(Vec3T<T>::zeros(), T(1));
    const BoxSDF<T>    box(-Vec3T<T>::ones(), Vec3T<T>::ones());

    const auto analytic = Fused::SmoothUnion(
      Fused::operands(Fused::Union(sphere, box),
                      Fused::Intersection(sphere, box),
                      Fused::Difference(sphere, box, sphere),
                      Fused::Complement(sphere),
                      Fused::SmoothUnion(sphere, box, T(0.1), BlendKind::Exponential),
                      Fused::SmoothIntersection(sphere, box, T(0.1)),
                      Fused::SmoothIntersection(Fused::operands(sphere, box, sphere), T(0.1)),
                      Fused::SmoothDifference(sphere, box, T(0.1)),
                      Fused::SmoothDifference(sphere, Fused::operands(box, sphere), T(0.1)),
                      Fused::Translate(Fused::Rotate(sphere, T(30), size_t(1)), Vec3T<T>::ones()),
                      Fused::Rotate(sphere, T(30), Vec3T<T>::ones()),
                      Fused::Scale(Fused::Reflect(sphere, size_t(0)), T(2)),
                      Fused::Annular(Fused::Offset(sphere, T(0.1)), T(0.1)),
                      Fused::Elongate(sphere, Vec3T<T>::ones()),
                      Fused::FiniteRepetition(sphere, Vec3T<T>::ones(), Vec3T<T>::ones(), Vec3T<T>::ones()),
                      Fused::Blur(sphere, T(0.1)),
                      Fused::Mollify(sphere, T(0.1), size_t(3))),
      T(0.1));

    (void)analytic.signedDistance(Vec3T<T>::zeros());
    (void)analytic.computeBoundingVolume();
    (void)analytic.rebasedView(meshPool);
    (void)analytic.deepCopy(meshPool);

    const DefaultTriMeshSDF mesh = Parser::readIntoTriMeshSDF<T>(file, meshPool);
    const auto              hole = Fused::Difference(mesh, Fused::Translate(sphere, Vec3T<T>::ones()));
    using Hole                   = std::decay_t<decltype(hole)>;

    (void)hole.signedDistance(Vec3T<T>::zeros());
    (void)hole.computeBoundingVolume();
    (void)Hole(hole.layout(), hole.location()).signedDistance(Vec3T<T>::zeros());
    (void)hole.rebasedView(meshPool);
    (void)hole.deepCopy(meshPool);
  }

  (void)PolygonSoup<T>().convertToDCEL(meshPool);
  (void)PolygonSoup<T>().convertToTriangles();

  // PackedBVH::traverse() is a member template: walk a BVH of points with each query type, a visit
  // callback and each visit order.
  {
    using Packed = BVH::PackedBVH<T, Vec3T<T>, 4>;

    std::vector<std::pair<Vec3T<T>, BoundingVolumes::AABBT<T>>> points{
      {Vec3T<T>::zeros(), BoundingVolumes::AABBT<T>(Vec3T<T>::zeros(), Vec3T<T>::zeros())}};

    Pool         bvhPool(hostMemoryResource());
    const Packed bvh = Packed::buildBottomUp(bvhPool, points, size_t(1));

    T          best  = Math::Limits<T>::infinity();
    const auto leaf  = [](T&, size_t, size_t) noexcept {};
    const auto bound = [](const T& a_best) noexcept { return a_best; };
    const auto visit = [](T&, uint32_t, const typename Packed::Node&, const T&) noexcept { return true; };

    const BVH::PointQuery<T> point{Vec3T<T>::zeros()};
    const BVH::RayQuery<T>   ray(Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0)));

    bvh.traverse(point, best, leaf, bound);
    bvh.traverse(point, best, leaf, bound, visit, BVH::FarthestFirst{});
    bvh.traverse(ray, best, leaf, bound, BVH::DescendAll{}, BVH::AsStored{});
    bvh.traverse(ray, best, leaf, bound, visit, [](const T& a_a, const T& a_b) noexcept { return a_a < a_b; });
    bvh.traverse(
      ray, best, leaf, bound, visit, [](const T&, uint32_t a_a, const T&, uint32_t a_b) noexcept { return a_a < a_b; });
  }
}

template void
instantiateFunctionTemplates<double>();
template void
instantiateFunctionTemplates<float>();

} // namespace EBGeometry
