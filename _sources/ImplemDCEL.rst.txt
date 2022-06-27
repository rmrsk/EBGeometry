.. _Chap:ImplemDCEL:

DCEL
====

The DCEL functionality exists under the namespace ``EBGeometry::DCEL`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **Signed distance functionality** for the above types.
*  **File parsers for reading files** into DCEL structures.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. note::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons. 

Classes
-------

The main DCEL functionality (vertices, edges, faces) is provided by the following classes:

*  **Vertices** are implemented as a template ``EBGeometry::DCEL::EdgeT``

  .. code-block:: c++
     
     template <class T>
     class VertexT

  The DCEL vertex class stores the vertex position, normal vector, and the outgoing half-edge from the vertex.
  Note that the class has member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. 
  
  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1VertexT.html>`__.

*  **Edges** are implemented as a template ``EBGeometry::DCEL::EdgeT``

  .. code-block:: c++
		  
     template <class T>
     class EdgeT

  The half-edges store a reference to their face, as well as pointers to the previous edge, next edge, pair edge, and starting vertex.
  For performance reasons, the edge also stores the length and inverse length of the edge.

  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1EdgeT.html>`__.

*  **Faces** are implemented as a template ``EBGeometry::DCEL::FaceT``

  .. code-block:: c++
		  
     template <class T>
     class FaceT

  For performance reasons, a polygon face stores all it's half-edges (to avoid iteration when computing the signed distance).
  It also stores:

  * The normal vector.
  * A 2D embedding of the polygon face.
  * Centroid position.    

  The normal vector and 2D embedding of the facet exist because the signed distance computation requires them.
  The centroid position exists only because BVH partitioners will use it for partitioning the surface mesh.

  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1FaceT.html>`__.

*  **Mesh** is implemented as a template ``EBGeometry::DCEL::MeshT``

  .. code-block:: c++
		  
     template <class T>
     class MeshT

  The Mesh stores all the vertices, half-edges, and faces.
  For example, to obtain all the facets one can call ``EBGeometry::DCEL::MeshT<T>::getFaces()`` which will return all the DCEL faces of the surface mesh. 
  Typically, the mesh is not created by the user but automatically created when reading the mesh from an input file.

  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1MeshT.html>`__.

All of the above DCEL classes have member functions of the type:

.. code-block:: c++

   T signedDistance(const Vec3T<T>& a_point) const noexcept;
   T unsignedDistance2(const Vec3T<T>& a_point) const noexcept;

Thus, they fulfill the template requirements of the primitive type for the BVH implementation, see :ref:`Chap:BVHConstraints`.
See :ref:`Chap:BVHIntegration` for details regarding DCEL integration with BVHs.

.. _Chap:BVHIntegration:

BVH integration
---------------

DCEL functionality can easily be embedded in BVHs.
In this case it is the facets that are embedded in the BVHs, and we require that we can create bounding volumes that contain all the vertices in a facet.
Moreover, partitioning functions that partition a set of polygon faces into ``K`` new sets of faces are also required.

EBGeometry provides some simplistic functions that are needed (see :ref:`Chap:BVHConstruction`) when building BVHs for DCEL geometries .

.. note::
   
   The functions are defined in :file:`Source/EBGeometry_DCEL_BVH.hpp`. 

For the bounding volume constructor, we provide a function

.. code-block:: c++

   template <class T, class BV>
   EBGeometry::BVH::BVConstructorT<EBGeometry::DCEL::FaceT<T>, BV> defaultBVConstructor =
   [](const std::shared_ptr<const EBGeometry::DCEL::FaceT<T>>& a_primitive) -> BV {
     return BV(a_primitive->getAllVertexCoordinates());
   };

Note the extra template constraint on the bounding volume type ``BV``, which must be able to construct a bounding volume from a finite point set (in this case the vertex coordinates).

For the stop function we provide a simple function

.. code-block:: c++

   template <class T, class BV, size_t K>
   EBGeometry::BVH::StopFunctionT<T, EBGeometry::DCEL::FaceT<T>, BV, K> defaultStopFunction =
   [](const BVH::NodeT<T, EBGeometry::DCEL::FaceT<T>, BV, K>& a_node) -> bool {
     return (a_node.getPrimitives()).size() < K;
   };

Note that this simply terminates the leaf partitioning if there are not enough primitives (polygon faces) available, or there are fewer than a pre-defined number of primitives.

For the partitioning function we include a simple function that partitions the primitives along the longest axis:

.. code-block:: c++

   template <class T, class BV, size_t K>
   EBGeometry::BVH::PartitionerT<EBGeometry::DCEL::FaceT<T>, BV, K> chunkPartitioner =
   [](const PrimitiveList<T>& a_primitives) -> std::array<PrimitiveList<T>, K> {
     Vec3T<T> lo = Vec3T<T>::max();
     Vec3T<T> hi = -Vec3T<T>::max();
     
     for (const auto& p : a_primitives) {
       lo = min(lo, p->getCentroid());
       hi = max(hi, p->getCentroid());
     }

     const size_t splitDir = (hi - lo).maxDir(true);

     // Sort the primitives along the above coordinate direction.
     PrimitiveList<T> sortedPrimitives(a_primitives);

     std::sort(
       sortedPrimitives.begin(), sortedPrimitives.end(),
       [splitDir](const std::shared_ptr<const FaceT<T>>& f1, const std::shared_ptr<const FaceT<T>>& f2) -> bool {
         return f1->getCentroid(splitDir) < f2->getCentroid(splitDir);
       });

     return EBGeometry::DCEL::equalCounts<T, K>(sortedPrimitives);
   };		


For a list of all DCEL partitioner, see :file:`Source/EBGeometry_DCEL_BVH.hpp`.

Code example
------------

Constructing a compact BVH representation of polygon mesh is therefore done as follows:

.. code-block:: c++

   using T    = float;
   using BV   = EBGeometry::BoundingVolumes::AABBT<T>;
   using Vec3 = EBGeometry::Vec3T<T>;
   using Face = EBGeometry::DCEL::FaceT<T>;

   constexpr int K = 4;

   // Read the mesh from file and put it in a DCEL format. 
   std::shared_ptr<EBGeometry::DCEL::Mesh<T> > mesh = EBGeometry::Parser::read("MyFile.stl");

   // Make a BVH node and build the BVH.
   auto root = std::make_shared<EBGeometry::BVH::NodeT<T, Face, BV, K> >(mesh->getFaces());

   // Build the BVH hierarchy
   root->topDownSortAndPartitionPrimitives(EBGeometry::DCEL::defaultBVConstructor<T, BV>,
                                           EBGeometry::DCEL::spatialSplitPartitioner<T, K>,
					   EBGeometry::DCEL::defaultStopFunction<T, BV, K>);

   // Flatten the tree onto a tighter representation. Then delete the old tree. 
   auto compactBVH = root->flattenTree();

   root = nullptr;
