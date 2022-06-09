.. _Chap:Parsers:

File parsers
============

Routines for parsing surface files from grids into EBGeometry's DCEL grids are given in the namespace ``EBGeometry::Parser``.
The source code is implemented in :file:`Source/EBGeometry_Parser.hpp`.

Read into DCEL
--------------

Currently, this is limited to the following file formats:

*  **PLY** Only ASCII formats currently supported, `<https://en.wikipedia.org/wiki/PLY_(file_format)>`_.

   When reading a PLY file into the DCEL data structures, it is sufficient to call the following static function:

   .. code-block::

      template<class T>
      using Mesh = EBGeometry::DCEL::MeshT<T>;
      
      template<class T>
      std::shared_ptr<Mesh> EBGeometry::Parser::PLY<T>::readASCII(const std::string a_filename);

   .. warning::

      Although the parser will do it's best to read files that contains holes or incomplete faces, success will fluctuate.
      Moreover, the signed distance function is not well-defined for such cases.      

   Calling the ``readASCII`` function will read the input file (which is assumed to be a PLY file) and create the DCEL data structures.

.. note::

   If the file format of your surface mesh file is not one of the above, consider either providing a new plugin or convert your file (e.g. to PLY) using MeshLab, Blender, etc. 
