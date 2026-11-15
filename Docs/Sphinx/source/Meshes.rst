.. _Chap:Meshes:

Surface meshes
==============

A surface mesh, read from an STL, PLY, OBJ or VTK file, describes an object by its boundary: planar faces
(triangles, or polygons) joined at their edges. EBGeometry turns such a mesh into a signed distance
function, which then composes, renders and runs on a GPU like any other shape.

.. important::

   A signed distance needs an inside, so the mesh must be *watertight* (no holes) and *orientable* (every
   face wound the same way). On a mesh with holes, flipped faces or self-intersections only the unsigned
   distance is well defined: the sign is unreliable near the defect.

The readers accept holes. A mesh that cannot form a surface (an edge shared by more than two faces, faces
folding back onto each other) is an error, or, when asked, read with a warning (see :ref:`Chap:Parsers`).

.. _Chap:DCEL:

Half-edge meshes
----------------

EBGeometry stores a mesh as a doubly-connected edge list (DCEL), also known as a half-edge mesh, of three
kinds of element:

* faces, planar polygons;
* half-edges, each running along one side of one face;
* vertices.

As :numref:`Fig:DCEL` shows, the half-edges circulate the inside of their face, each knowing the next
one. A half-edge also knows its starting vertex and its *pair*, the half-edge along the same edge in the
neighboring face. Walking the half-edges of a face visits its edges and vertices, and stepping to a pair
moves to the neighboring face.

.. _Fig:DCEL:
.. figure:: /_static/DCEL.png
   :width: 45%
   :align: center

   The half-edge structure. Each half-edge knows the next half-edge, its pair and its starting vertex;
   each vertex its position and one of its outgoing half-edges.

Every element is stored in an array of the mesh and refers to the others by their index in those
arrays, never by a pointer, so that the mesh can be copied to a GPU as it is.

Signed distance to a mesh
-------------------------

The signed distance to a mesh is the signed distance to its closest face. The point closest to
:math:`\mathbf{x}` on a face lies in one of three places, see :numref:`Fig:PolygonProjection`.

.. _Fig:PolygonProjection:
.. figure:: /_static/PolygonProjection.png
   :width: 25%
   :align: center

   Where the closest point on a face can lie, after projecting :math:`\mathbf{x}` onto the face's plane.

#. **Inside the face.** When :math:`\mathbf{x}` projects onto the face's plane inside the face, the
   signed distance is :math:`\mathbf{n}_f\cdot\left(\mathbf{x} - \mathbf{x}_f\right)`, with
   :math:`\mathbf{n}_f` the face's normal and :math:`\mathbf{x}_f` a point on the face. Whether the
   projection is inside a polygon is decided in the polygon's plane, by the crossing number by default;
   the winding number and the subtended angle can be chosen instead (``FaceT::setInsideOutsideAlgorithm()``).
#. **On an edge.** Otherwise, the point may project onto an edge
   :math:`\mathbf{e}(t) = \mathbf{x}_0 + \left(\mathbf{x}_1 - \mathbf{x}_0\right)t` at some
   :math:`t \in [0, 1]`. The distance is the distance to that point, signed by
   :math:`\mathbf{n}_e\cdot\left(\mathbf{x} - \mathbf{x}_0\right)`, with :math:`\mathbf{n}_e` the edge's
   pseudonormal.
#. **At a vertex.** Otherwise the closest point is a vertex :math:`\mathbf{x}_v`, and the sign is that
   of :math:`\mathbf{n}_v\cdot\left(\mathbf{x}-\mathbf{x}_v\right)`, with :math:`\mathbf{n}_v` the
   vertex's pseudonormal.

Testing every face costs as much as the mesh is large; a bounding volume hierarchy over the faces keeps the
test to the faces near the point (see :ref:`Chap:BVH`).

.. _Chap:NormalDCEL:

Pseudonormals
-------------

A face has one normal; an edge and a vertex, where faces meet at an angle, do not. The sign near them
comes from *pseudonormals* :cite:`1407857`, which give the right sign at every point. An edge's is the
average of its two faces' normals,

.. math::

   \mathbf{n}_{e} = \frac{\mathbf{n}_{f} + \mathbf{n}_{f^\prime}}{\left|\mathbf{n}_{f} + \mathbf{n}_{f^\prime}\right|},

and a vertex's the average of the normals of the faces around it, weighted by the angle :math:`\alpha_i`
each face makes at the vertex (see :numref:`Fig:Pseudonormal`):

.. math::

  \mathbf{n}_{v} = \frac{\sum_i\alpha_i\mathbf{n}_{f_i}}{\left|\sum_i\alpha_i\mathbf{n}_{f_i}\right|}.

.. _Fig:Pseudonormal:
.. figure:: /_static/Pseudonormal.png
   :width: 25%
   :align: center

   Edge and vertex pseudonormals.

Both are computed when the mesh is built.

.. _Chap:Triangles:

Triangles in SIMD blocks
------------------------

A mesh of triangles can also be stored as self-contained triangles, without the half-edge structure: each
triangle holds its three vertices, its face normal and the pseudonormals of its edges and vertices, so that
nothing else is read to find its distance. Triangles stored this way are packed in blocks of :math:`W`,
coordinate by coordinate, so that one SIMD instruction computes the distance to all :math:`W` at once (see
:ref:`Chap:SIMDAcceleration`). A polygon face is cut into a fan of triangles from its first vertex, which
gives the same distance since the face is planar and convex. Each triangle remembers the face it was cut
from.

.. _Sec:ChoosingMeshSDF:

Choosing a mesh distance function
---------------------------------

EBGeometry has three mesh distance functions, all shapes that compose, ray trace exactly and run on a GPU:

.. list-table::
   :header-rows: 1
   :widths: 20 25 55

   * - Class
     - Holds
     - Use it for
   * - ``TriMeshSDF``
     - triangles in SIMD blocks, under a BVH
     - a triangle mesh (or any mesh, cut into triangles): the fastest, and the default choice
   * - ``MeshSDF``
     - the half-edge mesh, under a BVH
     - polygon faces kept as they are, or a mesh that changes after it is built (flipped, its vertices
       moved), since every query reads the mesh itself
   * - ``FlatMeshSDF``
     - the half-edge mesh, no BVH
     - checking the others, and meshes of a few faces: every query tests every face

All three name the face closest to a point by its *face id*, the face's position in the file (after the
reader removes faces of zero area), so that per-face data of your own (a material, a boundary condition) is an array indexed by it.
:ref:`Chap:ImplemMeshSDF` describes the classes, and :ref:`Chap:ExampleMeshSDF` reads a mesh and queries
it.
