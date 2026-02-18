.. _Chap:Parsers:

Reading data
============

Routines for parsing surface grid from files into ``EBGeometry``'s DCEL grids are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

.. important::

   ``EBGeometry`` is currently limited to reading STL, PLY, and VTK files, and then reconstructing DCEL grids from those.
   PLY and and VTK files can contain associated data on the nodes and faces, but this is not automatically populated when constructing the DCEL grids. 
   It is also possible to build DCEL grids from polygon soups read using third-party codes (see :ref:`Chap:ThirdPartyParser`).

Quickstart
----------

If you have one or multiple files, you can quickly turn them into signed distance fields using

.. code-block:: c++

   std::vector<std::string> files; // <---- List of file names.
   
   const auto distanceFields = EBGeometry::Parser::readIntoLinearBVH<float>(files);

This will build DCEL meshes for each input file, and wrap the meshes in BVHs.
See :ref:`Chap:LinearSTL` for further details.

.. tip::

   If the input files consist only of triangles, use the version

   .. code-block:: c++

      std::vector<std::string> files; // <---- List of file names.
   
      const auto distanceFields = EBGeometry::Parser::readIntoTriangleBVH<float>(files);

   This version will convert all DCEL polygons to triangles, and usually provides a nice code speedup.

Reading mesh files
------------------

``EBGeometry`` supports a native parser for binary and ASCII files, which can be read into a few different representations:

#. Into a DCEL mesh, see :ref:`Chap:ImplemDCEL`.
#. Into a signed distance function representation of a DCEL mesh, see :ref:`Chap:ImplemCSG`.
#. Into a signed distance function representation of a DCEL mesh, but using a BVH accelerator in full representation.
#. Into a signed distance function representation of a DCEL mesh, but using a BVH accelerator in compact representation.

.. important::

   The ``EBGeometry`` parser will read input files into internal objects that represent each file type.
   Conversion of these objects into DCEL meshes is not required, and it is possible to creating bounding volume hierarchies directly from the facets.
   This is useful when only an acceleration structure is needed for looking up facets or triangles, but no signed distance function is otherwise required. 

DCEL representation
___________________

To read one or multiple files and turn it into DCEL meshes, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 124-138
   :dedent: 2	   

Note that this will only expose the DCEL mesh, but not include any signed distance functionality.

DCEL mesh SDF
_____________

To read one or multiple files and also turn it into signed distance representations, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 140-154
   :dedent: 2	   

DCEL mesh SDF with full BVH
___________________________

To read one or multiple files and turn it into signed distance representations using a full BVH representation, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 156-176
   :dedent: 2	   

.. _Chap:LinearSTL:

DCEL mesh SDF with compact BVH
_______________________________

To read one or multiple STL files and turn it into signed distance representations using a compact BVH representation, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 178-198
   :dedent: 2

	   
Triangle meshes with BVH
________________________

To read one or multiple files and turn it into signed distance representations using a compact BVH representation, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 200-217
   :dedent: 2	   

This version differs from the DCEL meshes in that each DCEL polygon is converted into triangles after parsing.
The code will throw an error if not all DCEL polygon are not actual triangles.

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
   :lines: 37-56
   :dedent: 2

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
