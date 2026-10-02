.. _Chap:Parsers:

Reading data
============

Routines for parsing surface grids from files into EBGeometry's DCEL grids (see :ref:`Chap:DCEL`
for the concept, :ref:`Chap:ImplemDCEL` for the concrete API) -- or directly into BVH-accelerated
signed distance functions, including the triangle-mesh representation described conceptually in
:ref:`Chap:Triangles` -- are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

.. important::

   EBGeometry is currently limited to reading STL, PLY, OBJ, and legacy VTK polydata (``.vtk``)
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
this works internally, and :ref:`Sec:DCELMemoryModel` for what it means in practice. If you
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

   This version fan-triangulates every DCEL polygon, packs the triangles into SIMD-width groups,
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
   only reads the first one. In a binary STL, the two "attribute" bytes after each triangle are
   ignored; some exporters store a colour there.

A file that cannot be read in full is rejected rather than read in part: a partial mesh has holes,
and a mesh with holes gives wrong signs without any other symptom. If a file is missing, has an
unsupported extension, is truncated (it ends before the vertex and face counts in its header say
it should, or an ASCII STL has no ``endsolid``), or is corrupted (a line or header count that
cannot be parsed, a face that refers to a vertex that does not exist, a coordinate that is not a
finite number, faces that cannot be joined into a half-edge mesh or that fold back onto each other),
the reader throws ``EBGeometry::Parser::ParseError``. The ``readInto*`` functions
also throw for a file that contains no faces. A
`ParseError <doxygen/html/classEBGeometry_1_1Parser_1_1ParseError.html>`__ is a
``std::runtime_error`` that also reports the file, the line where the problem was found (0 for a
binary file, or when there is no meaningful line), and the reason:

.. code-block:: cpp

   try {
     const auto sdf = EBGeometry::Parser::readIntoTriangleBVH<T, Meta>("part.stl", pool);
     // ...
   }
   catch (const EBGeometry::Parser::ParseError& e) {
     std::cerr << e.what() << '\n'; // part.stl:12: malformed vertex line 'vertex 1 zero 0'
   }

OBJ files state neither counts nor an end marker, so a truncated OBJ file cannot be detected; it
reads as whatever faces it still contains. A vertex that no face uses is ignored.

.. _Sec:OnDefect:

Loading meshes with surface defects
___________________________________

Some meshes in the wild are not clean, closed surfaces: neighbouring faces wound in opposite
directions, an edge shared by three or more faces, or faces that fold back onto each other. Such a
mesh still gives correct *distances*, but the *sign* of the distance near the defect is unreliable,
so by default the readers reject it. Every ``readInto*`` function, and every format class's
``convertToDCEL``, takes a last argument of type ``Parser::OnDefect`` that changes this:

.. code-block:: cpp

   // Load anyway: print a warning to std::cerr naming the defect, and build the mesh.
   const auto sdf = EBGeometry::Parser::readIntoPackedBVH<T, Meta>(
     "scan.obj", pool, EBGeometry::BVH::Construction::SAH, EBGeometry::Parser::OnDefect::Warn);

``OnDefect::Throw`` is the default. ``OnDefect::Warn`` only covers defects the mesh survives: a
corrupted file, an index out of range, a non-finite coordinate, or a face that visits the same
vertex twice (which corrupts the half-edge mesh built from it) still throws. Holes are never a
defect.

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
``DCEL::MeshT<T, Meta>`` by value (or a ``std::vector`` of them). The mesh resolves its storage
through ``pool``, so ``pool`` must outlive it and every copy of it.
Note that this will only expose the DCEL mesh, but not include any signed distance functionality.

.. note::

   The returned mesh is queryable immediately (``mesh.getVertex(i)``, ``mesh.signedDistance(p)``,
   ...), and stays queryable as further meshes are built into the same ``pool``. Nothing has to be
   frozen first -- see :ref:`Chap:MemoryModel`.

DCEL mesh SDF
_____________

To read one or multiple files and also turn it into a bare (BVH-free) signed distance
representation, use ``readIntoMesh<T, Meta>(filename, pool)``, returning a
``FlatMeshSDF<T, Meta>`` by value (or a ``std::vector`` of them for the multi-file overload). The
returned ``FlatMeshSDF`` resolves its mesh through ``pool`` (see :ref:`Chap:MeshSDFClasses`), so
``pool`` must outlive it and every copy of it. Repeated calls can share one ``pool`` freely, in any
combination with the other ``readInto*`` functions.

.. _Chap:PackedBVHParser:

DCEL mesh SDF with PackedBVH
_____________________________

``readIntoPackedBVH<T, Meta, K>(filename, pool, construction)`` wraps a DCEL mesh in a ``PackedBVH``
(depth-first flat layout) with SIMD traversal, returning a ``MeshSDF<T, Meta, K>`` by value (or a
``std::vector`` of them). It supports any polygon, not just triangles; the BVH branching factor
``K`` defaults to 4 and the construction method ``a_construction`` defaults to ``BVH::Construction::SAH``. The returned
``MeshSDF`` holds the mesh and its BVH in ``pool``, so ``pool`` must outlive it and every copy of
it. For maximum throughput on triangle-only meshes, prefer ``readIntoTriangleBVH`` below.

Triangle meshes with PackedBVH
________________________________

``readIntoTriangleBVH<T, Meta, K, W>(filename, pool, maxLeafGroups, construction)``
converts all DCEL polygons to triangles, packs them into SoA groups of ``W``, and builds a
``PackedBVH``, returning a ``TriMeshSDF<T, Meta, K, W>`` by value (or a ``std::vector`` of them).
SIMD intrinsics evaluate up to ``W`` triangles per leaf visit. ``K`` and ``W`` default to 4
(``BVH::DefaultBranchingRatio<T>()`` and ``TriangleSoA::DefaultWidth<T>()``, independent of compiler
flags; see :ref:`Sec:DefaultKW`); ``maxLeafGroups`` (default 4)
bounds the number of full ``W``-sized SoA groups per BVH leaf. Faces with more than three vertices
are fan-triangulated, which is exact for the planar convex faces the DCEL mesh requires. Unlike
``readIntoMesh``/``readIntoPackedBVH``, the returned ``TriMeshSDF`` extracts flat ``Triangle``
values from the intermediate DCEL mesh and does not retain it; its BVH is reserved from
``pool``, so ``pool`` must outlive it. The intermediate mesh itself lives in a ``Pool`` private to
the call and is freed on return. A ``Pool`` never frees individual reservations (see
`Pool <doxygen/html/classEBGeometry_1_1Pool.html>`__), so building the mesh in ``pool`` would leave
it reserved there, and mirrored to the device along with the BVH.

Flat triangle list
____________________

``readIntoTriangles<T, Meta>(filename)`` returns a flat ``std::vector<Triangle<T, Meta>>``
(or, for the multi-file overload, one such vector per file) -- every face of the parsed mesh as an
independent, self-contained ``Triangle`` value, with no DCEL/half-edge topology connecting them. Each
triangle carries its face's normal and metadata, and its vertices' and half-edges' normals -- the
same extraction ``TriMeshSDF``'s mesh constructor performs, so ``readIntoTriangleBVH`` and
``TriMeshSDF(mesh, ...)`` build identical triangles.
The triangles are plain values, and the intermediate DCEL mesh lives in a ``Pool`` private to the
call, so no ``Pool`` of yours is involved. ``readIntoTriangleBVH`` works the same way: only the
returned ``TriMeshSDF``'s BVH is reserved from the ``pool`` you pass. Use this
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

Turning a soup into a DCEL mesh is a three-step process, with optional checks before and between the
steps, using the functions in namespace ``EBGeometry::Soup``. The file readers run the three steps,
the validity check before them and the two checks after compression themselves.

* ``isValid(vertices, facets, reason)`` checks that every vertex coordinate is finite and every face
  index is in range, and says why not in ``reason``. The readers run it first, before compressing.
* ``containsDegeneratePolygons(vertices, facets)`` is an optional up-front check: it returns
  ``true`` if any face has fewer than three vertices, two or more coincident vertices, or zero
  area (collinear vertices). Useful for validating a soup produced by an external tool.
* ``compress(vertices, facets)`` discards duplicate vertices from the soup in place, updating
  ``facets`` to reference the compressed vertex list.
* ``removeDegeneratePolygons(vertices, facets)`` removes faces that have no area, and returns how
  many it removed. A zero-area triangle whose three vertices are collinear is usually a
  *T-junction filler*, written by CAD exporters to close the gap where one edge meets the middle of
  another. It is removed, and its middle vertex is inserted into the face across its longest edge,
  so the mesh stays closed. This matters for the sign of the distance: a zero-area face has no
  normal, and left in place it would corrupt the edge and vertex pseudonormals next to it.
* ``soupToDCEL(mesh, pool, vertices, facets, id)`` builds the vertices, half-edges, and faces of
  the (already-compressed) soup into the output DCEL mesh, reconciles pair edges (internally, via
  ``reconcilePairEdgesDCEL``, which links each half-edge :math:`u \to v` to its reverse
  :math:`v \to u`), and runs a mesh sanity check. This also computes the vertex and edge normal
  vectors. ``mesh`` can be freshly default-constructed (``DCEL::MeshT<T, Meta> mesh;``, no ``Pool``
  needed at construction); ``soupToDCEL`` reserves its vertex/edge/half-edge storage from ``pool``
  itself, sized from ``vertices``/``facets``. ``mesh`` is attached to ``pool`` by that first reserve
  and is queryable as soon as ``soupToDCEL`` returns, whether or not ``pool`` is shared with further
  meshes -- see :ref:`Chap:MemoryModel` and :ref:`Sec:DCELMemoryModel`.
* ``findRepeatedVertex(facets)`` reports a face that visits a vertex twice. Such a face corrupts the
  half-edge mesh, so the readers reject it whatever ``OnDefect`` says.
* ``findTopologyDefect(facets)``, run between the two steps above, reports a face that visits a
  vertex twice, or an edge that two faces run along in the same direction -- which is what happens
  when neighbouring faces are oriented inconsistently, or when three or more faces share one edge.
  Such faces cannot be joined into a half-edge mesh. An edge used by only one face (a hole) is not
  reported.
* ``findFoldedFeature(mesh)``, run on the finished mesh, reports an edge or vertex whose
  pseudonormal is zero because the faces around it fold back onto each other; the sign of the
  distance near it would be undefined.

.. note::

   Every function in ``EBGeometry::Soup`` is also declared ``inline static``, so -- for the same
   reason noted above for the ``readInto*`` family -- none of them currently appear individually
   in the generated Doxygen API reference; the namespace page exists but is effectively empty.
   Their exact signatures and contracts are documented via the Doxygen comment on each function
   directly in :file:`Source/EBGeometry_Soup.hpp`.

.. warning::

   ``soupToDCEL`` will issue plenty of warnings if the polygon soup is not watertight and orientable.
   The format classes' ``convertToDCEL`` functions, which the readers use, go further. They share
   one function, ``Soup::readSoupIntoDCEL``, which runs the steps and checks above in order and
   throws ``ParseError`` if ``findRepeatedVertex``, ``findTopologyDefect`` or ``findFoldedFeature``
   reports anything; with ``OnDefect::Warn`` (see :ref:`Sec:OnDefect`) the last two only print a
   warning. A mesh with holes is still read.

.. _Chap:ThirdPartyParser:

Using third-party sources
-------------------------

By design, EBGeometry does not include much functionality for parsing files into polygon soups.
There are many open source third-party codes for achieving this (and we have tested several of them):

#. `happly <https://github.com/nmwsharp/happly>`_ or `miniply <https://github.com/vilya/miniply>`_ for Stanford PLY files.
#. `stl_reader <https://github.com/sreiter/stl_reader>`_ for STL files.
#. `tinyobjloader <https://github.com/tinyobjloader/tinyobjloader>`_ for OBJ files.

In almost every case, the above codes can be read into polygon soups, and one can then turn the soup into a DCEL mesh as described in :ref:`Chap:PolySoups`.
