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
  template class EllipsoidSDF<PREC>;                                         \
  template class CappedConeSDF<PREC>;                                        \
  template class RoundConeSDF<PREC>;                                         \
  template class PolygonPrismSDF<PREC>;                                      \
  template class TubeSDF<PREC>;                                              \
  template class CappedTorusSDF<PREC>;                                       \
  template class CutSphereSDF<PREC>;                                         \
  template class CutHollowSphereSDF<PREC>;                                   \
  template class WedgeSDF<PREC>;                                             \
                                                                               \
  /* -- CSG: blends, BVH unions, expressions and tapes ----------------------*/ \
  template struct SmoothMinOp<PREC>;                                         \
  template struct SmoothMaxOp<PREC>;                                         \
  template struct ExpMinOp<PREC>;                                            \
  template struct ExpMaxOp<PREC>;                                            \
  template class BVHUnion<PREC, 4, SphereSDF<PREC>>;                       \
  template class BVHSmoothUnion<PREC, 4, SmoothMinOp<PREC>, SphereSDF<PREC>>;                 \
  template class BVHSmoothUnion<PREC, 4, ExpMinOp<PREC>, BoxSDF<PREC>>;    \
  template class BVHUnion<PREC, 4, SphereSDF<PREC>, BoxSDF<PREC>, InfiniteCylinderSDF<PREC>>; \
  template class BVHSmoothUnion<PREC, 4, SmoothMinOp<PREC>, SphereSDF<PREC>, BoxSDF<PREC>>; \
  template struct Affine3T<PREC>;                                            \
  template class Expr<PREC>;                                                 \
  template class ExprIndex<PREC>;                                            \
  template class Tape<PREC>;                                                 \
  template class Tape<PREC, BVHUnion<PREC, 4, SphereSDF<PREC>>, MeshSDF<PREC, 8>>; \
  template class Tape<PREC, SphereCloudBVH<PREC>>;                           \
  template class BVHUnion<PREC, 4, SphereCloudBVH<PREC>>;                    \
  template struct SurfaceRef<PREC>;                                          \
  template struct SurfaceHitT<PREC>;                                         \
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
  template class BVHUnion<PREC, 4, TriMeshSDF<PREC, 4, 4>>;                  \
  template class BVHUnion<PREC, 4, TriMeshSDF<PREC, 4, 4>, SphereSDF<PREC>>; \
  template class BVHSmoothUnion<PREC, 4, SmoothMinOp<PREC>, MeshSDF<PREC, 4>>;                  \
  template struct ClosestFace<PREC>;                                         \
                                                                               \
  /* -- Point clouds --------------------------------------------------------*/\
  template struct PointSoAT<PREC>;                                           \
  template struct PointAoSoA<PREC>;                                          \
  template struct PointCloud::Hit<PREC>;                                     \
  template class PointCloud::KBest<PREC>;                                    \
  template class PointCloudBVH<PREC>;                                        \
  template class PointCloudHashGrid<PREC>;                                   \
  template struct SphereAoSoA<PREC>;                                         \
  template struct SphereCloud::Sphere<PREC>;                                 \
  template struct SphereCloud::Hit<PREC>;                                    \
  template struct SphereCloud::FromArray<PREC>;                              \
  template struct SphereCloud::FromRadii<PREC>;                              \
  template class SphereCloudBVH<PREC>;                                       \
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
  template struct RayForm<BVHUnion<PREC, 4, SphereSDF<PREC>>>;               \
  template struct RayForm<BVHUnion<PREC, 4, SphereSDF<PREC>, BoxSDF<PREC>>>; \
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
  template std::vector<HitT<PREC>> Render::Renderer<PREC>::pickAll<Tape<PREC>>( \
    const Tape<PREC>&, const Render::Camera<PREC>&, PREC, PREC) const;       \
  template void Render::view<Tape<PREC>>(const Tape<PREC>&, const Tape<PREC>&, int, char**); \
  template HitT<PREC> Render::Renderer<PREC>::pick<Tape<PREC>>(const Tape<PREC>&, const Render::Camera<PREC>&, PREC, PREC) const; \
  template void Render::view<Tape<PREC>>(const Tape<PREC>&, Pool&, int, char**); \
  template std::string Render::toCode<PREC>(const Render::Camera<PREC>&, const Render::Settings<PREC>&); \
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
  // build/query surface (odr-used so clang-tidy and the strong warning set analyze their bodies
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
    GPU::transform<128, 0>(query, points, results);
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

    // The build blocks, and the parallel point-cloud builds that use them.
    struct Descending
    {
      bool
      operator()(const uint64_t a_lhs, const uint64_t a_rhs) const noexcept
      {
        return a_lhs > a_rhs;
      }
    };

    std::vector<Vec3T<T>> scratch(GPU::boundingBoxScratchSize(1));
    uint64_t              keys[2] = {2, 1};

    (void)GPU::boundingBox(points.data(), 1, scratch.data());
    GPU::bitonicSort(keys, 2);
    GPU::bitonicSort(keys, 2, Descending{});

    // Both builds: on the host, and through GPU::parallelFor() (emulated device memory); and a rebuild.
    Pool cloudPool(hostMemoryResource());
    Pool parallelPool(emulatedDeviceMemoryResource());

    PointCloudHashGrid<T> grid = PointCloudHashGrid<T>::build(cloudPool, points, T(1), 4);

    (void)PointCloudHashGrid<T>::build(parallelPool, PointCloud::FromArray<T>{points.data()}, uint32_t(points.size()));

    grid.rebuild(PointCloud::FromArray<T>{points.data()}, uint32_t(points.size()));
    (void)grid.maxPoints();
    (void)GPU::boundingBoxOf<T>(PointCloud::FromArray<T>{points.data()}, 1, scratch.data());

    // The same for PointCloudBVH, and the exclusive scan its build uses.
    PointCloudBVH<T> bvh = PointCloudBVH<T>::build(cloudPool, points, 16, 4);

    (void)PointCloudBVH<T>::build(parallelPool, PointCloud::FromArray<T>{points.data()}, uint32_t(points.size()));

    bvh.rebuild(PointCloud::FromArray<T>{points.data()}, uint32_t(points.size()));
    (void)bvh.maxPoints();

    // The same for SphereCloudBVH, from one array of spheres and from centers and radii kept apart.
    const std::vector<SphereCloud::Sphere<T>> spheres = {{points[0], T(1)}};
    const std::vector<T>                      radii   = {T(1)};

    using CentersAndRadii = SphereCloud::FromCentersAndRadii<T, PointCloud::FromArray<T>, SphereCloud::FromRadii<T>>;

    SphereCloudBVH<T> spheresBVH = SphereCloudBVH<T>::build(cloudPool, spheres, 16, 4);

    (void)spheresBVH.closestSurface(Vec3T<T>::zeros());
    (void)spheresBVH.getNumSurfaces();
    (void)Tape<T, SphereCloudBVH<T>>(cloudPool, toExpr(spheresBVH)).closestSurface(Vec3T<T>::zeros());

    (void)SphereCloudBVH<T>::build(cloudPool, points, radii);
    (void)SphereCloudBVH<T>::build(
      parallelPool,
      CentersAndRadii{PointCloud::FromArray<T>{points.data()}, SphereCloud::FromRadii<T>{radii.data()}},
      1U);

    spheresBVH.rebuild(SphereCloud::FromArray<T>{spheres.data()}, 1U);
    (void)spheresBVH.maxSpheres();

    // And its contact queries, by index and for any sphere.
    T separations = T(0);

    spheresBVH.forEachContact(
      0U, T(0), [&separations](const SphereCloud::Hit<T>& a_hit) { separations += a_hit.distance; });
    spheresBVH.forEachContact(
      spheres[0], T(0), [&separations](const SphereCloud::Hit<T>& a_hit) { separations += a_hit.distance; });
    (void)separations;

    // Its layout, which a tape stores it as, and its ray form, through the leaf group's ray intervals.
    const SphereCloudBVH<T> fromLayout(spheresBVH.layout(), spheresBVH.location());
    const RayT<T>           ray{points[0], Vec3T<T>(T(1), T(0), T(0))};

    (void)firstHit(fromLayout, ray, T(0), T(1));
    (void)firstCrossing(fromLayout, ray, T(0), T(1));

    Array<T, 4> entry;
    Array<T, 4> exit;

    spheresBVH.getBVH().getPrimitives()[0].rayIntervals(ray.m_origin, ray.m_direction, entry, exit);

    std::vector<uint32_t> values(3, 1);
    std::vector<uint32_t> sums(GPU::exclusiveScanScratchSize(3));

    (void)GPU::exclusiveScan(values.data(), 3, sums.data());
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
    (void)firstCrossing(fused, ray, T(0), T(10));
    (void)firstCrossing(fused, ray, T(0), T(10), march);
    (void)firstCrossing(sphere, Vec3T<T>::zeros(), Vec3T<T>::ones());
    (void)intersects(fused, Vec3T<T>::zeros(), Vec3T<T>::ones());

    // Smooth compositions traced as the plain operation and marched where the blend changes it.
    (void)firstHit(Fused::SmoothUnion(sphere, torus, T(0.1)), ray, T(0), T(10));
    (void)firstHit(Fused::SmoothIntersection(sphere, torus, T(0.1)), ray, T(0), T(10));
    (void)firstHit(Fused::SmoothDifference(sphere, torus, T(0.1)), ray, T(0), T(10));

    Pool                                         pool(hostMemoryResource());
    const std::vector<SphereSDF<T>>              spheres(2, sphere);
    const std::vector<BoundingVolumes::AABBT<T>> boxes(2, sphere.computeBoundingVolume());
    const BVHUnion<T, 4, SphereSDF<T>>           bvhUnion(pool, spheres, boxes);
    const Tape<T>                                tape(pool, Difference(Union(sphere, torus), Complement(sphere)));

    (void)firstHit(bvhUnion, ray, T(0), T(10));
    (void)firstHit(tape, ray, T(0), T(10));
    (void)firstCrossing(tape, ray, T(0), T(10));
    (void)firstCrossing(bvhUnion, ray, T(0), T(10));
    (void)bvhUnion.closestSurface(Vec3T<T>::zeros());
    (void)bvhUnion.getNumSurfaces();
    (void)bvhUnion.getFirstSurface(0U);
    (void)bvhUnion.getNumMembers();
    (void)bvhUnion.getMemberIndex(0U);
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
    (void)tape.getNumPrimitives();
    (void)tape.getLargestPlainUnion();
    (void)tape.getNumSurfaceLabels();
    (void)tape.getSurfaceLabel(0U);
    (void)tape.getSurfaceLabel(tape.closestLeaf(Vec3T<T>::zeros()));
    (void)ExprIndex<T>(SurfaceLabel(expr, "all")).pathOf(0U);
    (void)tape.closestSurface(Vec3T<T>::zeros());
    (void)tape.firstSurface(RayT<T>{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0))}, T(0), T(1));
    (void)tape.firstSurface(Vec3T<T>::zeros(), Vec3T<T>::ones());
    (void)closestSurface(BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones()), Vec3T<T>::zeros());
    (void)firstSurface(BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones()), Vec3T<T>::zeros(), Vec3T<T>::ones());
    (void)ExprIndex<T>(expr).getPartSurfaceLabel(0U, 0U);
    (void)ExprIndex<T>(expr).getPartSurfaceLabels(0U);
    (void)tape.getNumSurfaces();
    (void)tape.getFirstSurface(0U);
    (void)ExprIndex<T>(expr).getNumSurfaces();
    (void)ExprIndex<T>(expr).getFirstSurface(0U);
    (void)ExprIndex<T>(expr).getLeafOfSurface(0U);
    {
      const Expr<T> regions = Overlay(MaterialLabel(expr, "a"), expr, expr);
      (void)Overlay({MaterialLabel(expr, "b"), expr, SphereSDF<T>(Vec3T<T>::zeros(), T(1))});
      const ExprIndex<T> index(regions);

      (void)index.getNumRegions();
      (void)index.getRegion(0U);
      (void)index.getRegionOfLeaf(0U);
      (void)index.getMaterialOf(0U);
      (void)index.getNumRegionGroups();
      (void)index.getRegionGroup(0U);
      (void)index.getNumMaterialLabels();
      (void)index.getMaterialLabelName(index.findMaterialLabel("a"));
    }
    (void)tape.getCallDepth();
    (void)Tape<T>(tape.layout(), tape.location()).signedDistance(Vec3T<T>::zeros());
    (void)tape.rebasedView(meshPool);
    (void)tape.deepCopy(meshPool);
    (void)normal(tape, Vec3T<T>::ones(), T(1.0e-3));

    // A tape with a feature (see TapeFeature), whose ray code compiles the feature's.
    const Tape<T, TapeFeature::ExactOffsets> exact(meshPool, expr);
    {
      const Tape<T> regions(meshPool, Overlay(expr, expr));

      (void)regions.getNumRegions();
      (void)regions.region(Vec3T<T>::zeros());
      (void)regions.signedDistance(Vec3T<T>::zeros(), 1U);
      (void)regions.closestSurface(Vec3T<T>::zeros(), 1U);

      const RayT<T> along{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0))};

      (void)RegionView<Tape<T>>(regions).firstHit(along, T(0), T(1), RayOptions<T>());
      (void)RegionView<Tape<T>>(regions).firstSeenThrough(along, T(0), T(1), RayOptions<T>());

      DomainSetup setup;

      (void)setup.addSubdomain({0U, 1U});
      setup.add(setup.addSurfaceGroup(), SurfaceSelector::interface(0U, 1U));
      (void)setup.surfaceGroupOf(regions.closestSurface(Vec3T<T>::zeros(), 1U));
      (void)setup.role(regions.closestSurface(Vec3T<T>::zeros(), 1U));

      const Domain     domain(meshPool, regions, setup);
      const Subdomains subdomains = domain.subdomains();

      (void)subdomains.role(regions.closestSurface(Vec3T<T>::zeros(), subdomains, 0U));
      (void)regions.signedDistance(Vec3T<T>::zeros(), subdomains, 0U);
      (void)domain.surfaceGroups().surfaceGroupOf(regions.closestSurface(Vec3T<T>::zeros(), 1U));
      (void)ExprIndex<T>(Overlay(expr, expr)).getInterfaces();
      (void)RegionNames(ExprIndex<T>(Overlay(expr, expr))).region(1U);
      (void)findSurfacePieces(regions, 4U);
      (void)domain.touches(0U, 1U);
      (void)domain.labelAt(Vec3T<T>::zeros());
      (void)domain.signedDistance(Vec3T<T>::zeros(), Domain<T>::Excluded);
      (void)domain.closestSurface(Vec3T<T>::zeros(), 0U);
      (void)domain.surfaceGroupOf(domain.closestSurface(Vec3T<T>::zeros(), 0U));
      (void)domain.role(domain.closestSurface(Vec3T<T>::zeros(), 0U));
      (void)domain.rebasedView(meshPool);
      {
        T values[2];

        domain.signedDistances(Vec3T<T>::zeros(), values);
      }
      (void)RegionSettings{}.selectionOf(regions.closestSurface(Vec3T<T>::zeros(), 1U));
      {
        const SurfaceInventory inventory(ExprIndex<T>(Overlay(expr, expr)));

        for (const PossiblePiece& piece : possiblePieces(inventory)) {
          (void)coverageOf(piece, inventory, setup);
        }
      }
      (void)ExprIndex<T>(Overlay(RegionName(expr, "a"), expr)).findRegion("a");
      (void)ExprIndex<T>(Overlay(expr, expr)).checkLabels(Overlay(expr, expr), regions);
      (void)regions.firstRegionChange(RayT<T>{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0))}, T(0), T(1));
      (void)regions.firstRegionChange(Vec3T<T>::zeros(), Vec3T<T>::ones());
      (void)regions.firstSurface(RayT<T>{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0))}, T(0), T(1), subdomains, 0U);
    }

    (void)exact.signedDistance(Vec3T<T>::zeros());
    (void)exact.firstHit(RayT<T>{Vec3T<T>::zeros(), Vec3T<T>(T(1), T(0), T(0))}, T(0), T(1));

    const LeafRef<T> leaf = tape.closestLeaf(Vec3T<T>::zeros());

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
  (void)PolygonSoup<T>().getFaceGroups();
  PolygonSoup<T>().setFaceGroups(FaceGroups{});
  (void)SurfaceLabel(BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones()), FaceGroups{{"a"}, std::vector<uint32_t>(6, 0U)});
  (void)BoxSDF<T>(-Vec3T<T>::ones(), Vec3T<T>::ones()).closestPart(Vec3T<T>::zeros());
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

  // A refit by tree level, through a schedule, on a host pool (the sweep) and on an emulated device pool
  // (the levels, as host loops in this translation unit).
  {
    using Packed = BVH::PackedBVH<T, Vec3T<T>, 4>;

    std::vector<std::pair<Vec3T<T>, BoundingVolumes::AABBT<T>>> points{
      {Vec3T<T>::zeros(), BoundingVolumes::AABBT<T>(Vec3T<T>::zeros(), Vec3T<T>::zeros())}};

    Pool         refitPool(hostMemoryResource());
    const Packed bvh = Packed::buildBottomUp(refitPool, points, size_t(1));

    const BVH::RefitSchedule schedule(refitPool, bvh);

    refitPool.freeze();

    Pool       devicePool = Pool::mirror(refitPool, GPU::memoryResource());
    const auto box        = [](const Vec3T<T>& a_p) noexcept { return BoundingVolumes::AABBT<T>(a_p, a_p); };

    BVH::refit(refitPool, bvh, schedule, box);
    BVH::refit(devicePool, bvh, schedule, box);
    (void)schedule.numLevels();
  }

  // A reconcile through a schedule, on a host pool and on an emulated device pool (the passes, as host loops in
  // this translation unit).
  {
    Pool                 reconcilePool(hostMemoryResource());
    const DCEL::MeshT<T> mesh = Parser::readIntoDCEL<T>(std::string("mesh.stl"), reconcilePool);

    const DCEL::ReconcileSchedule schedule(reconcilePool, mesh);

    reconcilePool.freeze();

    Pool devicePool = Pool::mirror(reconcilePool, GPU::memoryResource());

    DCEL::reconcile(reconcilePool, mesh, schedule);
    DCEL::reconcile(devicePool, mesh, schedule);
  }
}

template void
instantiateFunctionTemplates<double>();
template void
instantiateFunctionTemplates<float>();

} // namespace EBGeometry
