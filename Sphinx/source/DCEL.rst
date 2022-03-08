.. _Chap:DCEL:

DCEL mesh structure
===================

Basic concept
-------------

EBGeometry uses a doubly-connected edge list (DCEL) structure for storing surface meshes.
The DCEL structures consist of the following objects:

* Planar polygons (facets)
* Half-edges
* Vertices

As shown in :numref:`Fig:DCEL`, the half-edge is the most important data structure.
Half-edges circulate the inside of the facet, with pointer-access to the previous and next half-edge.
A half-edge also stores a reference to it's starting vertex, as well as a reference to it's pair-edge.
From the DCEL structure we can easily obtain all edges or vertices belonging to a single facet, and also "jump" to a neighboring facet by fetching the pair edge. 

.. _Fig:DCEL:
.. figure:: /_static/DCEL.png
   :width: 480px
   :align: center

   DCEL mesh structure. Each half-edge stores references to previous/next half-edges, the pair edge, and the starting vertex.
   Vertices store a coordinate as well as a reference to one of the outgoing half-edges.

In EBGeometry the half-edge data structure is implemented in it's own namespace ``EBGeometry::Dcel``.
This is a comparatively standard implementation of the DCEL structure, supplemented by functions that permit signed distance computations to various features on such a mesh.

.. important::

   A signed distance field requires a *watertight and orientable* surface mesh.
   If the surface mesh consists of holes or flipped facets, the signed distance function does not exist. 

Signed distance
---------------

When computing the signed distance function, the closest point on the surface mesh can be one of the vertices, (half-) edges, or faces, see :numref:`Fig:PolygonProjection`.

.. _Fig:PolygonProjection:
.. figure:: /_static/PolygonProjection.png
   :width: 240px
   :align: center

   Possible closest-feature cases after projecting a point :math:`\mathbf{x}` to the plane of a polygon face.

It is therefore necessary to distinguish between three cases:

#. **Facet/Polygon face**.
   
   When computing the distance from a point :math:`\mathbf{x}` to the polygon face we first determine if the projection of :math:`\mathbf{x}` to the face's plane lies inside or outside the face.
   This is more involved than one might think, and it is done by first computing the two-dimensional projection of the polygon face, ignoring one of the coordinates.
   Next, we determine, using 2D algorithms, if the projected point lies inside the embedded 2D representation of the polygon face. 
   Various algorithms for this are available, such as computing the winding number, the crossing number, or the subtended angle between the point and the 2D polygon.

   .. note::
   
      EBGeometry uses the crossing number algorithm by default.
      
   If the point projects to the inside of the face, the signed distance is just :math:`d = \mathbf{n}_f\cdot\left(\mathbf{x} - \mathbf{x}_f\right)` where :math:`\mathbf{n}_f` is the face normal and :math:`\mathbf{x}_f` is a point on the face plane (e.g., a vertex).
   If the point projects to *outside* the polygon face, the closest feature is either an edge or a vertex.
   
#. **Edge**.
   
   When computing the signed distance to an edge, the edge is parametrized as :math:`\mathbf{e}(t) = \mathbf{x}_0 + \left(\mathbf{x}_1 - \mathbf{x}_0\right)t`, where :math:`\mathbf{x}_0` and :math:`\mathbf{x}_1` are the starting and ending vertex coordinates.
   The point :math:`\mathbf{x}` is projected to this line, and if the projection yields :math:`t^\prime \in [0,1]` then the edge is the closest point.
   In that case the signed distance is the projected distance and the sign is given by the sign of :math:`\mathbf{n}_e\cdot\left(\mathbf{x} - \mathbf{x}_0\right)` where :math:`\mathbf{n}_e` is the pseudonormal vector of the edge. 
   Otherwise, the closest point is one of the vertices.

#. **Vertex**.

   If the closest point is a vertex then the signed distance is simply :math:`\mathbf{n}_v\cdot\left(\mathbf{x}-\mathbf{x}_v\right)` where :math:`\mathbf{n}_v` is the vertex pseudonormal and :math:`\mathbf{x}_v` is the vertex position.

Normal vectors
--------------

The normal vectors for edges (:math:`\mathbf{n}_e`) and vertices (:math:`\mathbf{n}_v`) are not uniquely defined.
For both edges and vertices we use the pseudonormals from :cite:`1407857`:

.. math::

   \mathbf{n}_{e} = \frac{1}{2}\left(\mathbf{n}_{f} + \mathbf{n}_{f^\prime}\right).

where :math:`f` and :math:`f^\prime` are the two faces connecting the edge.
The vertex pseudonormal are given by

.. math::

  \mathbf{n}_{v} = \frac{\sum_i\alpha_i\mathbf{n}_{f_i}}{\left|\sum_i\alpha_i\right|},

where the sum runs over all faces which share :math:`v` as a vertex, and where :math:`\alpha_i` is the subtended angle of the face :math:`f_i`, see :numref:`Fig:Pseudonormal`. 

.. _Fig:Pseudonormal:
.. figure:: /_static/Pseudonormal.png
   :width: 240px
   :align: center

   Edge and vertex pseudonormals.
   
