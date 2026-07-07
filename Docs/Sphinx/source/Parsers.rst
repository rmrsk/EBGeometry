.. _Chap:Parsers:

Reading data
============

Routines for parsing surface grids from files into ``EBGeometry``'s DCEL grids (or directly into BVH-accelerated signed distance functions) are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

.. important::

   ``EBGeometry`` is currently limited to reading STL, PLY, and VTK files, and then reconstructing DCEL grids from those.
   PLY and and VTK files can contain associated data on the nodes and faces, but this is not automatically populated when constructing the DCEL grids.
   It is also possible to build DCEL grids from polygon soups read using third-party codes (see :ref:`Chap:ThirdPartyParser`).

Quickstart
----------

Every reader function in ``EBGeometry::Parser`` comes in two overloads: one that takes a single
file name, and one that takes a ``std::vector<std::string>`` of file names and returns a vector
of results, one per file.  If you have one or multiple mesh files, the quickest way to turn them
into BVH-accelerated signed distance fields is

.. code-block:: c++

   std::vector<std::string> files; // <---- List of file names.

   const auto distanceFields = EBGeometry::Parser::readIntoPackedBVH<float>(files);

This will build a DCEL mesh for each input file and wrap it in a :cpp:class:`MeshSDF`, backed by
a SIMD-accelerated ``PackedBVH`` over the mesh's faces.  See :ref:`Chap:PackedBVHParser` for
further details.

.. tip::

   If the input files consist only of triangles, use the version

   .. code-block:: c++

      std::vector<std::string> files; // <---- List of file names.

      const auto distanceFields = EBGeometry::Parser::readIntoTriangleBVH<float>(files);

   This version will convert all DCEL polygons to triangles, pack them into SIMD-width groups,
   and usually provides a nice code speedup over ``readIntoPackedBVH``.

Reading mesh files
------------------

``EBGeometry`` supports a native parser for binary and ASCII files, which can be read into a few different representations:

#. Into a DCEL mesh, with no signed-distance functionality — see :ref:`Chap:ImplemDCEL`.
#. Into a bare DCEL signed distance function (:cpp:class:`FlatMeshSDF`, :math:`O(N)` scan, no BVH).
#. Into a signed distance function representation of a DCEL mesh, using a ``PackedBVH`` over DCEL faces (any polygon) — :cpp:class:`MeshSDF`.
#. Into a signed distance function representation of a triangle mesh, using a ``PackedBVH`` over SoA triangle groups — :cpp:class:`TriMeshSDF`.

See :ref:`Chap:MeshSDFClasses` for how these three SDF classes compare.

.. important::

   The ``EBGeometry`` parser will read input files into internal objects that represent each file type.
   Conversion of these objects into DCEL meshes is not required, and it is possible to creating bounding volume hierarchies directly from the facets.
   This is useful when only an acceleration structure is needed for looking up facets or triangles, but no signed distance function is otherwise required.

DCEL representation
___________________

To read one or multiple files and turn it into DCEL meshes, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 162-182
   :dedent: 0

Note that this will only expose the DCEL mesh, but not include any signed distance functionality.

DCEL mesh SDF
_____________

To read one or multiple files and also turn it into a bare (BVH-free) signed distance representation, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 184-204
   :dedent: 0

.. _Chap:PackedBVHParser:

DCEL mesh SDF with PackedBVH
_____________________________

``readIntoPackedBVH`` wraps a DCEL mesh in a ``PackedBVH`` (depth-first flat layout) with SIMD traversal.
It supports any polygon, not just triangles.
For maximum throughput on triangle-only meshes, prefer ``readIntoTriangleBVH`` below.

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 206-232
   :dedent: 0

Triangle meshes with PackedBVH
________________________________

``readIntoTriangleBVH`` converts all DCEL polygons to triangles, packs them into SoA groups of ``W``,
and builds a ``PackedBVH``.
SIMD intrinsics evaluate up to ``W`` triangles per leaf visit.
The code will raise an error if any face is not a triangle.

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 234-280
   :dedent: 0

.. _Chap:PolySoups:

From soups to DCEL
------------------

``EBGeometry`` also supports the creation of DCEL grids from polygon soups, which can then later be turned into an SDF representation.
A triangle soup is represented as

.. code-block:: c++

   std::vector<Vec3T<T>> vertices;
   std::vector<std::vector<size_t>> faces;

Here, ``vertices`` contains the :math:`x,y,z` coordinates of each vertex, while each entry ``faces`` contains a list of vertices for the face.

To turn this into a DCEL mesh, one should compress the triangle soup (get rid of duplicate vertices) and then construct the DCEL mesh:

.. literalinclude:: ../../../Source/EBGeometry_Soup.hpp
   :language: c++
   :lines: 44-72
   :dedent: 0

The ``compress`` function will discard duplicate vertices from the soup, while the ``soupToDCEL`` will tie the remaining polygons into a DCEL mesh.
This function will also compute the vertex and edge normal vectors.

.. warning::

   ``soupToDCEL`` will issue plenty of warnings if the polygon soup is not watertight and orientable.

.. _Chap:ThirdPartyParser:

Using third-party sources
-------------------------

By design, ``EBGeometry`` does not include much functionality for parsing files into polygon soups.
There are many open source third-party codes for achieving this (and we have tested several of them):

#. `happly <https://github.com/nmwsharp/happly>`_ or `miniply <https://github.com/vilya/miniply>`_ for Stanford PLY files.
#. `stl_reader <https://github.com/sreiter/stl_reader>`_ for STL files.
#. `tinyobjloader <https://github.com/tinyobjloader/tinyobjloader>`_ for OBJ files.

In almost every case, the above codes can be read into polygon soups, and one can then turn the soup into a DCEL mesh as described in :ref:`Chap:PolySoups`.
