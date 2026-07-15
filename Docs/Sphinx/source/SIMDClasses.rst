.. _Chap:SIMDClasses:

SIMD-accelerated classes
==========================

SIMD acceleration in EBGeometry is implemented with hand-written compiler intrinsics
(``__m128``/``__m256``/``__m512`` and their ``double`` counterparts), not compiler
auto-vectorisation. As of today it is confined to the places listed below; everything else
in the library is scalar code. This is a statement about the current state of the
implementation, not an architectural ceiling -- further classes may become SIMD-accelerated in
the same style in the future. This page lists the SIMD-accelerated classes as they exist today,
and explains precisely what gets vectorised in each.

.. contents:: On this page
   :local:
   :depth: 1

Per-triangle signed-distance evaluation: ``TriangleSoAT<T, W>`` / ``TriangleAoSoA<T, Meta, W>``
-----------------------------------------------------------------------------------------------

:file:`Source/EBGeometry_TriangleSoA.hpp` / :file:`EBGeometry_TriangleSoAImplem.hpp` (and the
metadata-carrying wrapper :file:`EBGeometry_TriangleAoSoA.hpp` /
:file:`EBGeometry_TriangleAoSoAImplem.hpp`)

See :ref:`Chap:Triangles` for the conceptual picture of why triangles are packed this way.

**What it stores:** the vertex positions, vertex normals, edge normals, and face normal of
:math:`W` triangles, laid out as a *structure of arrays* (one flat, ``alignas``-aligned array
per coordinate/component, rather than :math:`W` separate ``Triangle`` objects). ``TriangleSoAT``
is geometry-only; ``TriangleAoSoA<T, Meta, W>`` adds a physically separate ``std::array<Meta, W>``
of per-triangle metadata alongside it, never interleaved with the coordinates, so the
signed-distance kernel touches exactly the same bytes either way -- metadata is read only afterward,
via ``getMetaData(lane)``. This is the exact same relationship ``PointAoSoA`` has with ``PointSoAT``
below, and it is what lets ``TriMeshSDF`` answer "which triangle (and its metadata) is closest" (see
``TriMeshSDF::getClosestTriangle()`` in :ref:`Chap:MeshSDFClasses`) at no cost to the throughput
path. See :ref:`Chap:MeshSDFClasses` for the rest of the ``TriMeshSDF`` machinery this is packed
into.

**What is vectorised:** the entire closest-point-on-triangle signed-distance query,
``TriangleSoAT::signedDistance(point)`` (and, via delegation, ``TriangleAoSoA::signedDistance(point)``).
This is the full algorithm -- classifying the
projection of the query point against the triangle's three edge regions and interior region,
computing the squared distance and sign for whichever region applies, per triangle -- executed
for all :math:`W` triangles in the block *simultaneously*, one SIMD lane per triangle. The
result is the minimum signed distance over the whole block, found with vectorised compare/blend
instructions rather than a loop. The metadata-retrieving companion --
``TriangleSoAT::signedDistances(point)`` (the per-lane array) and
``TriangleAoSoA::signedDistance(point, Meta&)`` (which reports the winning lane's metadata) -- instead
takes a scalar per-lane path, since recovering *which* lane won is the one thing the SIMD horizontal
reduction discards; it is used only by the (non-throughput) closest-triangle query.

**What this means in practice:** evaluating a ``TriangleSoAT<T, W>`` block costs roughly the
same number of instructions as evaluating a *single* triangle scalar, but produces the answer
for :math:`W` of them. This is the leaf-level cost inside every ``TriMeshSDF::signedDistance()``
BVH leaf visit.

**Choosing and tuning** :math:`W`: the width is chosen from ISA auto-detection at compile time
(the compiler-predefined macros described in :ref:`Chap:SIMDAcceleration`), via
``EBGeometry::TriangleSoA::DefaultWidth<T>()``, and ``TriMeshSDF``/
``Parser::readIntoTriangleBVH`` default to the ISA-appropriate :math:`W` for whichever precision
``T`` is in use. See :ref:`Chap:MeshSDFClasses` for the full ISA/precision-to-default table, how
to override :math:`W` explicitly, and the data-alignment requirements it relies on.

Per-point distance evaluation: ``PointSoAT<T, W>`` / ``PointAoSoA<T, Meta, W>``
--------------------------------------------------------------------------------

:file:`Source/EBGeometry_PointSoA.hpp` / :file:`EBGeometry_PointSoAImplem.hpp` (and the
metadata-carrying wrapper :file:`EBGeometry_PointAoSoA.hpp` / :file:`EBGeometry_PointAoSoAImplem.hpp`)

**What it stores:** the positions of :math:`W` points, laid out as a *structure of arrays* (one
flat, ``alignas``-aligned array per coordinate, rather than :math:`W` separate point objects).
``PointSoAT`` is position-only; ``PointAoSoA<T, Meta, W>`` adds a physically separate
``std::array<Meta, W>`` of per-point metadata alongside it, never interleaved with the positions, so
the distance kernel touches exactly the same bytes either way -- metadata is read only afterward, via
``getMetaData(lane)``.

**What is vectorised:** the squared distance from a query point to all :math:`W` lane positions,
computed in one SIMD batch -- one ``_mm(128\|256\|512)_load_p[sd]`` per coordinate array, then
vectorised subtract/multiply/add for all :math:`W` lanes at once. This one kernel
(``getDistances2()``, which returns the whole :math:`W`-lane array) backs every distance query:
``getMinimumDistance2()`` / ``getMaximumDistance2()`` horizontally reduce the lanes to the
nearest / farthest, and the ``*Distance()`` forms add a single ``sqrt``. The full array is what an
all-neighbors (k-nearest-neighbor) leaf scan wants; the reduced minimum is what a closest-point leaf
scan wants.

**What this means in practice:** evaluating the distance to a whole :math:`W`-point block costs
roughly the same as one scalar point distance, but produces the answer for :math:`W` of them. This is
the leaf-level cost of a point-cloud ``PackedBVH`` search -- see ``Examples/ClosestPointBVH`` (closest
point) and ``Examples/NearestNeighborBVH`` (k nearest neighbors), both built on the ``PointCloudBVH``
class.

**Choosing and tuning** :math:`W`: the width is chosen from ISA auto-detection at compile time via
``EBGeometry::PointSoA::DefaultWidth<T>()`` -- the width that fills one SIMD register exactly for
``T``, on the same ISA/precision table as ``TriangleSoA``'s. Pass a different ``W`` explicitly as the
final template argument to override it.

SIMD-accelerated bounding-box pruning: ``BVH::PackedBVH<T, P, K>``
-----------------------------------------------------------------------

:file:`Source/EBGeometry_BVH.hpp` / :file:`EBGeometry_BVHImplem.hpp`

See :ref:`Chap:BVH` for the conceptual picture of bounding volume hierarchies and tree pruning.

**What it stores:** each interior node's :math:`K` children's bounding boxes, laid out as a
*structure of arrays* (``ChildAABBSoA``: flat, ``alignas``-aligned low/high-corner coordinate
arrays across all :math:`K` children), alongside the usual index-offset node data. See
:ref:`Chap:PackedBVH` for the rest of the packed representation.

**What is vectorised:** the point-to-bounding-box squared-distance test used to decide which
children to descend into (or prune) during traversal, in ``PackedBVH::pruneTraverse()``. All
:math:`K` children of a node are tested against the query point in a single SIMD batch --
one ``_mm(256\|512)_load_p[sd]`` per coordinate array, then vectorised subtract/max/multiply/add
to get all :math:`K` squared distances at once -- rather than a scalar loop over children.
``PackedBVH`` has no ``signedDistance()`` of its own; ``MeshSDF``/``TriMeshSDF::signedDistance()``
each build a thin wrapper around ``pruneTraverse()``, supplying a signed-distance leaf-eval and
pruning rule. Any caller can invoke ``pruneTraverse()`` directly with its own leaf-eval/pruning-
rule pair to get the same SIMD node pruning over a different search (e.g. nearest-neighbor search
over a primitive type with no ``signedDistance()`` at all). See :ref:`Chap:PruneTraverse` for the
full callback contract and traversal algorithm.

**What this means in practice:** the cost of deciding which subtree(s) to visit next no longer
scales with :math:`K` the way a scalar loop would; a wider branching factor (larger :math:`K`)
is close to "free" for this step as long as it still fits in one SIMD batch for the target ISA,
which is exactly why :math:`K` is chosen per-ISA (see below).

**Choosing and tuning** :math:`K`: the branching factor is likewise chosen from ISA
auto-detection at compile time, via ``BVH::DefaultBranchingRatio<T>()`` -- a separate function
from ``TriangleSoA``'s, but driven by the same underlying ISA macros, so ``TriMeshSDF``/
``Parser::readIntoTriangleBVH`` still end up with a matching :math:`(K, W)` pair for whichever
precision ``T`` is in use. See :ref:`Chap:MeshSDFClasses` for the full ISA/precision-to-default
table and how to override :math:`K` explicitly.
