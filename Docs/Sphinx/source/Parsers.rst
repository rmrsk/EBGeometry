.. _Chap:Parsers:

Reading data
============

Routines for parsing surface grids from files into EBGeometry's DCEL grids (see :ref:`Chap:DCEL`
for the concept, :ref:`Chap:ImplemDCEL` for the concrete API) -- or directly into BVH-accelerated
signed distance functions, including the triangle-mesh representation described conceptually in
:ref:`Chap:Triangles` -- are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

.. important::

   EBGeometry is currently limited to reading STL, PLY, OBJ, and VTK (legacy or XML polydata)
   files, and then reconstructing DCEL grids from those.
   PLY and VTK files can contain associated data on the nodes and faces, but this is not
   automatically populated when constructing the DCEL grids.
   It is also possible to build DCEL grids from polygon soups read using third-party codes (see
   :ref:`Chap:ThirdPartyParser`).

Quickstart
----------

Every ``readInto*`` function in ``EBGeometry::Parser`` takes an explicit
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__, in addition to a single file name or a
``std::vector<std::string>`` of file names (with a vector-per-file result for the latter): every
DCEL mesh it builds reserves its vertex/edge/face storage from that Pool. The Pool is
caller-owned and caller-managed -- EBGeometry never constructs one for you -- so it must outlive
every mesh (or SDF wrapper retaining one, see :ref:`Chap:MeshSDFClasses`) built into it. One Pool
can back many meshes, built one after another with their arrays laid out contiguously in the same
block, which is cheaper than giving every mesh its own Pool -- see :ref:`Chap:MemoryModel` for how
this works internally, and the pitfalls in :ref:`Sec:DCELMemoryModel` for what it means in practice
(in particular, everyone sharing a Pool must finish building before anyone can freeze it). If you
have one or multiple mesh files, the quickest way to turn them into BVH-accelerated signed
distance fields is

.. code-block:: c++

   std::vector<std::string> files; // <---- List of file names.

   EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

   const auto distanceFields = EBGeometry::Parser::readIntoPackedBVH<float>(files, pool);

This will build a DCEL mesh for each input file and wrap it in a :cpp:class:`MeshSDF`, backed by
a SIMD-accelerated ``PackedBVH`` over the mesh's faces.  See :ref:`Chap:PackedBVHParser` for
further details.

.. tip::

   If the input files consist only of triangles, use the version

   .. code-block:: c++

      std::vector<std::string> files; // <---- List of file names.

      EBGeometry::Pool pool(EBGeometry::hostMemoryResource());

      const auto distanceFields = EBGeometry::Parser::readIntoTriangleBVH<float>(files, pool);

   This version will convert all DCEL polygons to triangles, pack them into SIMD-width groups,
   and usually provides a nice code speedup over ``readIntoPackedBVH``.

Reading raw file data
----------------------

At the lowest level, ``readPLY<T>``, ``readSTL<T>``, ``readOBJ<T>``, and ``readVTK<T>`` (each
with a single-filename and a ``std::vector<std::string>`` overload) parse a file into a raw,
format-specific data structure (``PLY<T>``, ``STL<T>``, ``OBJ<T>``, or ``VTK<T>``) holding
nothing more than the vertex coordinates and face index lists as they appear in the file --
no DCEL topology, no signed distance functionality. Two small helpers support this layer:
``getFileType`` inspects a file's extension to determine which of the four formats it is (or
``FileType::Unsupported``), and ``getFileEncoding`` inspects the file header to determine
whether it is ``Encoding::ASCII`` or ``Encoding::Binary``. Most users will not call these raw
readers directly -- they exist mainly as the common first step every ``readInto*`` function
below builds on -- but they are useful if you need the raw vertex/face data without paying for
DCEL construction at all.

.. note::

   If an STL file contains multiple solids (uncommon, but technically valid STL), ``readSTL``
   only reads the first one.

For the raw readers' exact signatures, see the Doxygen entries for
`readPLY <doxygen/html/namespaceEBGeometry_1_1Parser.html#ac78a6a540855effb6af095bb6c5c2982>`__,
`readSTL <doxygen/html/namespaceEBGeometry_1_1Parser.html#a24946a908c8fd9026f9262dab9574ef4>`__,
`readOBJ <doxygen/html/namespaceEBGeometry_1_1Parser.html#a93ee4c1806ebd72f36988aefe9032180>`__,
and `readVTK <doxygen/html/namespaceEBGeometry_1_1Parser.html#a061839316e9e89fb1b6c93bca52255d2>`__.

Reading mesh files
------------------

Above the raw per-format readers, EBGeometry offers five ways to turn a file directly into a
higher-level representation, each doing progressively more work:

#. Into a DCEL mesh, with no signed-distance functionality -- ``readIntoDCEL``, see
   :ref:`Chap:ImplemDCEL`.
#. Into a bare DCEL signed distance function (:cpp:class:`FlatMeshSDF`, :math:`O(N)` scan, no
   BVH) -- ``readIntoMesh``.
#. Into a signed distance function representation of a DCEL mesh, using a ``PackedBVH`` over
   DCEL faces (any polygon) -- :cpp:class:`MeshSDF`, via ``readIntoPackedBVH``.
#. Into a signed distance function representation of a triangle mesh, using a ``PackedBVH`` over
   SoA triangle groups -- :cpp:class:`TriMeshSDF`, via ``readIntoTriangleBVH``.
#. Into a flat, unconnected list of ``Triangle`` objects (each with precomputed vertex
   positions, normals, and edge normals, but no half-edge topology linking them) -- via
   ``readIntoTriangles``. Internally this still parses through a DCEL mesh first and then
   extracts each face as an independent ``Triangle``; use it when you need per-triangle data as
   plain values (e.g. to feed your own acceleration structure) rather than any of EBGeometry's
   own SDF wrappers.

See :ref:`Chap:MeshSDFClasses` for how the three SDF classes (options 2-4 above) compare.

.. important::

   The EBGeometry parser will read input files into internal objects that represent each file type.
   Conversion of these objects into DCEL meshes is not required, and it is possible to create bounding volume hierarchies directly from the facets.
   This is useful when only an acceleration structure is needed for looking up facets or triangles, but no signed distance function is otherwise required.

DCEL representation
___________________

To read one or multiple files and turn it into DCEL meshes, use
``readIntoDCEL<T, Meta>(filename, pool)`` (or the ``std::vector<std::string>`` overload for
multiple files at once, which reserves every mesh's storage from the same ``pool``), returning a
``shared_ptr<DCEL::MeshT<T, Meta>>`` (or a vector thereof).
Note that this will only expose the DCEL mesh, but not include any signed distance functionality.

.. note::

   ``readIntoDCEL`` never freezes or ``bind()``\ s ``pool`` (it may still be shared with more
   files -- see :ref:`Chap:MemoryModel`), so the returned mesh is not yet queryable through its
   no-argument accessors (``mesh->getVertex(i)``, ``mesh->signedDistance(p)``, ...). Freeze
   ``pool`` and call ``mesh->bind(pool)`` yourself if you want to query a raw ``readIntoDCEL``
   result directly; every other ``readInto*`` function below does this for you as part of building
   its SDF wrapper.

DCEL mesh SDF
_____________

To read one or multiple files and also turn it into a bare (BVH-free) signed distance
representation, use ``readIntoMesh<T, Meta>(filename, pool)``, returning a
``shared_ptr<FlatMeshSDF<T, Meta>>`` (or a vector thereof for the multi-file overload). The
returned ``FlatMeshSDF`` retains the mesh (see :ref:`Chap:MeshSDFClasses`), so ``pool`` must
outlive it.

.. warning::

   ``FlatMeshSDF``'s constructor freezes ``pool`` (see :ref:`Chap:MemoryModel`): once a call to
   ``readIntoMesh`` (or a direct ``FlatMeshSDF`` construction) has run, no more meshes can be built
   into that same ``pool`` -- including from a second, separate ``readIntoMesh`` call. The
   multi-file overload handles this correctly internally (it builds every file's mesh before
   wrapping any of them); if you need several *independent* ``readIntoMesh`` calls sharing one
   file's worth of build work, use the multi-file overload rather than chaining single-file calls
   by hand, or give each single-file call its own ``Pool``.

.. _Chap:PackedBVHParser:

DCEL mesh SDF with PackedBVH
_____________________________

``readIntoPackedBVH<T, Meta, K>(filename, pool, build)`` wraps a DCEL mesh in a ``PackedBVH``
(depth-first flat layout) with SIMD traversal, returning a ``shared_ptr<MeshSDF<T, Meta, K>>`` (or
a vector thereof). It supports any polygon, not just triangles; the BVH branching factor ``K``
defaults to 4 and the build strategy ``a_build`` defaults to ``BVH::Build::SAH``. The returned
``MeshSDF`` retains the mesh, so ``pool`` must outlive it. For maximum throughput on
triangle-only meshes, prefer ``readIntoTriangleBVH`` below.

.. warning::

   ``MeshSDF``'s constructor freezes ``pool`` too, for the same reason as ``FlatMeshSDF`` above --
   the same caution about chaining single-file calls by hand applies here.

Triangle meshes with PackedBVH
________________________________

``readIntoTriangleBVH<T, Meta, K, W, StoragePolicy>(filename, pool, maxLeafGroups, build)``
converts all DCEL polygons to triangles, packs them into SoA groups of ``W``, and builds a
``PackedBVH``, returning a ``shared_ptr<TriMeshSDF<T, Meta, K, W, StoragePolicy>>`` (or a vector
thereof). SIMD intrinsics evaluate up to ``W`` triangles per leaf visit. ``K`` and ``W`` default to
the SIMD-optimal values for ``T`` on the current ISA (``BVH::DefaultBranchingRatio<T>()`` and
``TriangleSoA::DefaultWidth<T>()``, see :ref:`Chap:MeshSDFClasses`); ``maxLeafGroups`` (default 4)
bounds the number of full ``W``-sized SoA groups per BVH leaf; ``StoragePolicy`` defaults to
``BVH::ValueStorage<TriangleAoSoA<T, Meta, W>>``, matching ``TriMeshSDF``'s own default (see
:ref:`Chap:MeshSDFClasses` for the rationale, and why ``readIntoPackedBVH``/``MeshSDF`` above has
no equivalent parameter). The code will raise an error if any face is not a triangle. Unlike
``readIntoMesh``/``readIntoPackedBVH``, the returned ``TriMeshSDF`` extracts flat ``Triangle``
values from the intermediate DCEL mesh and does not retain it, so ``pool`` only needs to outlive
this call -- though since a ``Pool`` never individually frees what it reserves (see
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__), that intermediate mesh's storage stays
reserved in ``pool`` regardless.

Flat triangle list
____________________

``readIntoTriangles<T, Meta>(filename, pool)`` returns a flat ``std::vector<shared_ptr<Triangle<T,
Meta>>>`` (or, for the multi-file overload, one such vector per file) -- every face of the
parsed mesh as an independent, self-contained ``Triangle`` value, with no DCEL/half-edge
topology connecting them. As with ``readIntoTriangleBVH`` above, the intermediate DCEL mesh built
along the way is not retained by the result, so ``pool`` only needs to outlive this call. Use this
when some other part of your code wants raw triangle values (for example, to build a custom
acceleration structure) rather than any of EBGeometry's own SDF wrappers.

.. note::

   ``readIntoDCEL``, ``readIntoMesh``, ``readIntoPackedBVH``, ``readIntoTriangleBVH``, and
   ``readIntoTriangles`` are declared ``inline static`` in the header, so this project's current
   Doxygen configuration (``EXTRACT_STATIC = NO``) does not extract them into the generated API
   reference individually -- their exact signatures are documented via the Doxygen comment on
   each function directly in :file:`Source/EBGeometry_Parser.hpp`. The raw per-format readers
   above (``readPLY``/``readSTL``/``readOBJ``/``readVTK``, declared without ``static``) do not
   have this limitation and are linked individually.

.. _Chap:PolySoups:

From soups to DCEL
------------------

EBGeometry also supports the creation of DCEL grids from polygon soups, which can then later be turned into an SDF representation.
A triangle soup is represented as

.. code-block:: c++

   std::vector<Vec3T<T>> vertices;
   std::vector<std::vector<size_t>> faces;

Here, ``vertices`` contains the :math:`x,y,z` coordinates of each vertex, while each entry ``faces`` contains a list of vertices for the face.

Turning a soup into a DCEL mesh is a two- (optionally three-) step process, using the functions
in namespace ``EBGeometry::Soup``:

* ``containsDegeneratePolygons(vertices, facets)`` is an optional up-front check: it returns
  ``true`` if any face has fewer than three vertices, or two or more vertices that coincide
  after lexicographic sorting. Useful for validating a soup produced by an external tool before
  spending time compressing/converting it.
* ``compress(vertices, facets)`` discards duplicate vertices from the soup in place, updating
  ``facets`` to reference the compressed vertex list.
* ``soupToDCEL(mesh, pool, vertices, facets, id)`` builds the vertices, half-edges, and faces of
  the (already-compressed) soup into the output DCEL mesh, reconciles pair edges (internally, via
  ``reconcilePairEdgesDCEL``, which links each half-edge :math:`u \to v` to its reverse
  :math:`v \to u`), and runs a mesh sanity check. This also computes the vertex and edge normal
  vectors. ``mesh`` can be freshly default-constructed (``DCEL::MeshT<T, Meta> mesh;``, no ``Pool``
  needed at construction); ``soupToDCEL`` reserves its vertex/edge/half-edge storage from ``pool``
  itself, sized from ``vertices``/``facets``. Since ``pool`` may still be shared with more meshes
  after this call, ``soupToDCEL`` never freezes/binds it -- see :ref:`Chap:MemoryModel` and
  :ref:`Sec:DCELMemoryModel` -- so ``mesh`` is not yet queryable through its no-argument accessors
  at this point; freeze and ``bind()`` it yourself once you are done building into ``pool``, the
  same way :ref:`Chap:MeshSDFClasses`'s constructors do.

.. note::

   Every function in ``EBGeometry::Soup`` is also declared ``inline static``, so -- for the same
   reason noted above for the ``readInto*`` family -- none of them currently appear individually
   in the generated Doxygen API reference; the namespace page exists but is effectively empty.
   Their exact signatures and contracts are documented via the Doxygen comment on each function
   directly in :file:`Source/EBGeometry_Soup.hpp`.

.. warning::

   ``soupToDCEL`` will issue plenty of warnings if the polygon soup is not watertight and orientable.

.. _Chap:ThirdPartyParser:

Using third-party sources
-------------------------

By design, EBGeometry does not include much functionality for parsing files into polygon soups.
There are many open source third-party codes for achieving this (and we have tested several of them):

#. `happly <https://github.com/nmwsharp/happly>`_ or `miniply <https://github.com/vilya/miniply>`_ for Stanford PLY files.
#. `stl_reader <https://github.com/sreiter/stl_reader>`_ for STL files.
#. `tinyobjloader <https://github.com/tinyobjloader/tinyobjloader>`_ for OBJ files.

In almost every case, the above codes can be read into polygon soups, and one can then turn the soup into a DCEL mesh as described in :ref:`Chap:PolySoups`.
