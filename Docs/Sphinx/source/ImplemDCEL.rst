.. _Chap:ImplemDCEL:

DCEL
====

The DCEL functionality exists under the namespace ``EBGeometry::DCEL`` and contains the following functionality:

*  **Fundamental data types** like vertices, half-edges, polygons, and entire surface grids.
*  **BVH functionality** for putting DCEL grids into bounding volume hierarchies.

.. important::

   The DCEL functionality is *not* restricted to triangles, but supports N-sided polygons, including *meta-data* attached to the vertices, edges, and facets. 

Main types
----------

The main DCEL functionality (vertices, edges, faces) is provided by the following classes:

*  **Vertices** are implemented as a template ``EBGeometry::DCEL::EdgeT``

  .. code-block:: c++
     
     template <class T, class Meta>
     class VertexT

  The DCEL vertex class stores the vertex position, normal vector, and the outgoing half-edge from the vertex.
  Note that the class has member functions for computing the vertex pseudonormal, see :ref:`Chap:NormalDCEL`. 
  
  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1VertexT.html>`__.

*  **Edges** are implemented as a template ``EBGeometry::DCEL::EdgeT``

  .. code-block:: c++
		  
     template <class T, class Meta>
     class EdgeT

  The half-edges store a reference to their face, as well as pointers to the next edge, pair edge, and starting vertex.

  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1EdgeT.html>`__.

*  **Faces** are implemented as a template ``EBGeometry::DCEL::FaceT``

  .. code-block:: c++
		  
     template <class T, class Meta>
     class FaceT

  Faces also store

  * The normal vector.
  * A 2D embedding of the polygon face.
  * Centroid position.    

  The normal vector and 2D embedding of the facet exist because the signed distance computation requires them.
  The centroid position exists only because BVH partitioners will use it for partitioning the surface mesh.

  The full API is given in the doxygen documentation `here <doxygen/html/classDCEL_1_1FaceT.html>`__.

*  **Mesh** is implemented as a template ``EBGeometry::DCEL::MeshT``

  .. code-block:: c++
		  
     template <class T, class Meta>
     class MeshT : public SignedDistanceFunction<T>

  The mesh stores all the vertices, half-edges, and faces, and if it is watertight and orientable it is also a signed distance function.
  Typically, the mesh is not created by the user but automatically created when reading the mesh from an input file.

The above DCEL classes have member functions of the type:

.. code-block:: c++

   T signedDistance(const Vec3T<T>& a_point) const noexcept;
   T unsignedDistance2(const Vec3T<T>& a_point) const noexcept;

which can be used to compute the distance to the various features on the mesh.

Meta-data can be attached to the DCEL primitives by selecting an appropriate type for ``Meta`` above (which defaults to ``short``). 


.. _Chap:BVHIntegration:

BVH integration
---------------

DCEL grids can easily be embedded in BVHs by enclosing bounding volumes around the polygons (e.g., triangles).
Partitioning and bounding volume constructors are provided in :file:`Source/EBGeometry_MeshDistanceFunctions.hpp`.
