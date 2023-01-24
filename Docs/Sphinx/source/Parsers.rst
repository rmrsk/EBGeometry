.. _Chap:Parsers:

Reading data
============

Routines for parsing surface files from grids into EBGeometry's DCEL grids are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

.. warning::

   EBGeometry is currently limited to reading binary and ASCII STL files and reconstructing DCEL grids from those.
   However, it is a simple matter to also reconstructor DCEL grids from triangle soups read using third-party codes (see :ref:`Chap:ThirdPartyParser`).

Reading STL files
-----------------

EBGeometry supports a native parser for binary and ASCII STL files.
To read one or multiple STL files and turn it into an implicit function, use

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 54-68

Alternatively, the files can be read directly into a full or linearized bounding volume hierarchy:

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 70-100

.. _Chap:PolySoups:

From soups to DCEL
------------------

EBGeometry also supports the creation of DCEL grids from polygon soups.
A triangle soup is represented as

.. code-block:: c++
		
   std::vector<Vec3T<T>> vertices;
   std::vector<std::vector<size_t>> faces;

Here, ``vertices`` contains the :math:`x,y,z` coordinates of each vertex, while each entry ``faces`` contains a list of vertices for the face. 

To turn this into a DCEL mesh, one should compress the triangle soup (get rid of duplicate vertices) and then construct the DCEL mesh:

.. literalinclude:: ../../../Source/EBGeometry_Parser.hpp
   :language: c++
   :lines: 119-137

The ``compress`` function will discard duplicate vertices from the soup, while the ``soupToDCEL`` will simply turn the remaining polygon soup into a DCEL mesh.

.. tip::
   
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
