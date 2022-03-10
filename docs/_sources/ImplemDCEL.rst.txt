.. _Chap:ImplemDCEL:

DCEL
====

The DCEL functionality exists under the namespace ``EBGeometry::Dcel`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **Signed distance functionality** for the above types.
*  **File parsers for reading files** into DCEL structures.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. note::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons. 

Classes
-------

The main DCEL functionality (vertices, edges, faces) is provided by the following classes:

*  **Vertices** are implemented as a template ``EBGeometry::Dcel::EdgeT``

  .. literalinclude:: ../../Source/EBGeometry_DcelVertex.hpp
     :language: c++
     :lines: 41-43

  The DCEL vertex class stores the vertex position, normal vector, and the outgoing half-edge from the vertex.
  Note that the class has member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. 
  
  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1VertexT.html>`_.

*  **Edges** are implemented as a template ``EBGeometry::Dcel::EdgeT``

  .. literalinclude:: ../../Source/EBGeometry_DcelEdge.hpp
     :language: c++
     :lines: 43-45

  The half-edges store a reference to their face, as well as pointers to the previous edge, next edge, pair edge, and starting vertex.
  For performance reasons, the edge also stores the length and inverse length of the edge.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1EdgeT.html>`_.

*  **Faces** are implemented as a template ``EBGeometry::Dcel::FaceT``

  .. literalinclude:: ../../Source/EBGeometry_DcelFace.hpp
     :language: c++
     :lines: 44-46

  For performance reasons, a polygon face stores all it's half-edges (to avoid iteration when computing the signed distance).
  It also stores:

  * The normal vector.
  * A 2D embedding of the polygon face.
  * Centroid position.    

  The normal vector and 2D embedding of the facet exist because the signed distance computation requires them.
  The centroid position exists only because BVH partitioners will use it for partitioning the surface mesh.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1FaceT.html>`_.

*  **Mesh** is implemented as a template ``EBGeometry::Dcel::MeshT``

  .. literalinclude:: ../../Source/EBGeometry_DcelMesh.hpp
     :language: c++
     :lines: 42-44

  The Mesh stores all the vertices, half-edges, and faces.
  For example, to obtain all the facets one can call ``EBGeometry::Dcel::MeshT<T>::getFaces()`` which will return all the DCEL faces of the surface mesh. 
  Typically, the mesh is not created by the user but automatically created when reading the mesh from an input file.

  The full API is given in the doxygen documentation `here <doxygen/html/classDcel_1_1MeshT.html>`_.

All of the above DCEL classes have member functions of the type:

.. code-block:: c++

   T signedDistance(const Vec3T<T>& a_point) const noexcept;
   T unsignedDistance2(const Vec3T<T>& a_point) const noexcept;

Thus, they fulfill the template requirements of the primitive type for the BVH implementation, see :ref:`Chap:BVHConstraints`.
See :ref:`Chap:BVHIntegration` for details regarding DCEL integration with BVHs.


File parsers
------------

Routines for parsing surface files from grids into EBGeometry's DCEL grids are given in the namespace ``EBGeometry::Dcel::Parser``.
Currently, this is limited to the following file formats:

*  **PLY** Only ASCII formats currently supported, `<https://en.wikipedia.org/wiki/PLY_(file_format)>`_.

   When reading a PLY file into the DCEL data structures, it is sufficient to call the following static function:

   .. code-block::

      template<class T>
      using Mesh = EBGeometry::Dcel::MeshT<T>;
      
      template<class T>
      std::shared_ptr<Mesh> EBGeometry::Parser::PLY<T>::readASCII(const std::string a_filename);

   .. warning::

      Although the parser will do it's best to read files that contains holes or incomplete faces, success will fluctuate.
      Moreover, the signed distance function is not well-defined for such cases.      

   Calling the ``readASCII`` function will read the input file (which is assumed to be a PLY file) and create the DCEL data structures.

.. note::

   If the file format of your surface mesh file is not one of the above, consider either providing a new plugin or convert your file (e.g. to PLY) using MeshLab, Blender, etc. 

.. _Chap:BVHIntegration:

BVH integration
---------------

DCEL functionality can easily be embedded in BVHs.
In this case it is the facets that are embedded in the BVHs, and we require that we can create bounding volumes that contain all the vertices in a facet.
Moreover, partitioning functions that partition a set of polygon faces into ``K`` new sets of faces are also required.

Construction methods
____________________

EBGeometry provides some simplistic functions that are needed (see :ref:`Chap:BVHConstruction`) when building BVHs for DCEL geometries .

.. note::
   
   The functions are defined in :file:`Source/EBGeometry_DcelBVH.hpp`. 

For the bounding volume constructor, we provide a function

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 49-52

Note the extra template constraint on the bounding volume type ``BV``, which must be able to construct a bounding volume from a finite point set (the vertex coordinates).

For the stop function we provide a simple function

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 34, 61-67

Note that this simply terminates the leaf partitioning if there are not enough primitives (polygon faces) available, or there are fewer than a pre-defined number of primitives.

For the partitioning function we include a simple function that partitions the primites along the longest axis:

.. literalinclude:: ../../Source/EBGeometry_DcelBVH.hpp
   :language: c++
   :lines: 74-78, 83-137

Code example
____________

Constructing a compact BVH representation of polygon mesh is therefore done as follows:

.. code-block:: c++

   using T    = float;
   using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
   using Vec3 = EBGeometry::Vec3T<T>;
   using Face = EBGeometry::Dcel::FaceT<T>;

   constexpr int K = 4;

   // Read the mesh from file and put it in a DCEL format. 
   std::shared_ptr<EBGeometry::Dcel::Mesh<T> > mesh = EBGeometry::Dcel::Parser::Ply("MyFile.ply");

   // Make a BVH node and build the BVH.
   auto root = std::make_shared<EBGeometry::BVH::NodeT<T, Face, BV, K> >(mesh->getFaces());

   // Build the BVH hierarchy
   root->topDownSortAndPartitionPrimitives(EBGeometry::Dcel::defaultBVConstructor<T, BV>,
                                           EBGeometry::Dcel::spatialSplitPartitioner<T, K>,
					   EBGeometry::Dcel::defaultStopFunction<T, BV, K>);

   // Flatten the tree onto a tighter representation. Then delete the old tree. 
   auto compactBVH = root->flattenTree();

   root = nullptr;
