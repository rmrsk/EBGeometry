.. _Chap:ImplemDCEL:

DCEL
====

See :ref:`Chap:DCEL` for the conceptual picture of half-edge meshes (what a DCEL is, and why
signed distance queries need one) before reading the concrete API below.

The DCEL functionality exists under the namespace ``EBGeometry::DCEL`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. important::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons, including *meta-data* attached to the vertices, edges, and facets. The latter is particularly useful in case one wants to associate e.g. boundary conditions to specific triangles.

Main types
----------

The main DCEL functionality (vertices, edges, faces) is provided by classes templated on both a
floating-point precision ``T`` and a user-defined meta-data type ``Meta`` attached to each
instance. The mesh owns its vertices, half-edges, and faces in flat arrays; each element is a
trivially copyable value type, and every topological link between them is an integer index
(``DCELIndex``) into those arrays -- with ``InvalidIndex`` denoting "no link" (e.g. the pair edge
of a boundary half-edge) -- rather than a pointer:

*  ``VertexT<T, Meta>`` stores the vertex position, its (outward) normal vector, and the index of
   an outgoing half-edge from the vertex, plus the indices of every polygon face sharing that
   vertex. It also has member functions for computing the vertex pseudonormal, see
   :ref:`Chap:NormalDCEL`. For the full API, see the Doxygen reference for
   `VertexT <doxygen/html/classEBGeometry_1_1DCEL_1_1VertexT.html>`__.

*  ``EdgeT<T, Meta>`` represents a half-edge: it stores the indices of its owning face, its next
   edge, its pair edge (``InvalidIndex`` on a boundary), and its origin vertex. For the full API,
   see the Doxygen reference for `EdgeT <doxygen/html/classEBGeometry_1_1DCEL_1_1EdgeT.html>`__.

*  ``FaceT<T, Meta>`` represents a polygon face. Besides the index of one of its half-edges, it also stores the face
   normal vector, a 2D embedding of the polygon, and its centroid position: the normal and 2D
   embedding exist because the signed distance computation needs them, and the centroid exists
   because BVH partitioners use it when partitioning the surface mesh. For the full API, see the
   Doxygen reference for `FaceT <doxygen/html/classEBGeometry_1_1DCEL_1_1FaceT.html>`__.

*  ``MeshT<T, Meta>`` stores an entire DCEL mesh -- all of its vertices, half-edges, and faces --
   and provides brute-force (:math:`\mathcal{O}(N)`) distance queries, ``signedDistance()`` and
   ``unsignedDistance2()``, that scan every face directly. It is not itself a
   ``SignedDistanceFunction<T>``: for anything beyond small meshes, one instead wraps a
   ``MeshT<T, Meta>`` in one of the BVH-accelerated classes described in
   :ref:`Chap:MeshSDFClasses`, which hold a ``shared_ptr<MeshT<T, Meta>>`` internally; the
   accelerated hierarchy stores face indices into that mesh and resolves each pruned candidate
   face's distance against it. A mesh is
   typically never constructed by hand -- it is built by a file parser reading vertices and faces
   from disk, see :ref:`Chap:Parsers`. For the full API, see the Doxygen reference for
   `MeshT <doxygen/html/classEBGeometry_1_1DCEL_1_1MeshT.html>`__.

Meta-data can be attached to the DCEL primitives by selecting an appropriate type for ``Meta`` above.

.. _Chap:BVHIntegration:

BVH integration
---------------

A ``MeshT<T, Meta>`` is never queried directly for anything beyond tiny meshes -- see
:ref:`Chap:BVH` for why an :math:`\mathcal{O}(N)` scan over faces doesn't scale, and
:ref:`Chap:ImplemBVH` for how ``TreeBVH``/``PackedBVH`` are actually built and traversed. This
section covers only the DCEL-specific half of that integration: how a mesh's faces become BVH
primitives in the first place.

Embedding a mesh in a BVH is a matter of pairing each ``FaceT<T, Meta>`` with a bounding volume
and handing the resulting list to a ``TreeBVH``. Concretely,
``MeshDistanceFunctionsDetail::buildDCELTreeBVH<T, Meta, BV, K>`` (in
:file:`Source/EBGeometry_MeshDistanceFunctionsImplem.hpp`, the shared helper behind both
``MeshSDF`` and ``TriMeshSDF``'s construction) does this by:

#. Building each face's bounding volume ``BV`` directly from its vertex coordinates
   (``FaceT::getAllVertexCoordinates()``) -- this is why ``FaceT`` stores its vertices' positions
   accessibly, rather than requiring a caller to walk its half-edges to collect them.
#. Constructing a ``TreeBVH<T, FaceT<T, Meta>, BV, K>`` from the resulting
   ``(face, bounding volume)`` pairs.
#. Partitioning that tree according to the requested ``BVH::Build`` strategy (``TopDown``,
   ``Morton``, ``Nested``, or ``SAH`` -- see :ref:`Chap:BVHConstruction`), where the
   ``BVCentroidPartitioner``/``BinnedSAHPartitioner`` used by the default and SAH strategies
   consult ``FaceT::getCentroid()`` (see above) when deciding how to split a set of faces.

``MeshSDF`` then packs this ``TreeBVH`` into a ``PackedBVH`` of faces directly (``pack()``), while
``TriMeshSDF`` additionally converts each face into a triangle and groups triangles into
SIMD-width ``TriangleSoAT`` blocks while packing (``packWith()``) -- see :ref:`Chap:MeshSDFClasses`
for how the two differ, and :ref:`Chap:Parsers` for the file-reading entry points that produce a
``MeshSDF``/``TriMeshSDF`` from a mesh file directly, without driving any of the steps above by
hand.
