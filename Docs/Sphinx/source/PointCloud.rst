.. _Chap:PointCloud:

Clouds of points and spheres
============================

A *point cloud* is simply a set of points in space, with no connectivity, orientation, or
inside/outside notion. Unlike a surface mesh, there is therefore no *signed* distance to a point
cloud -- only unsigned distances between points. The operations of interest are proximity queries:

* **Closest point.** Given an arbitrary query point, find the cloud point nearest to it.
* **k nearest.** Find the :math:`k` closest cloud points to a query, in ascending order of distance.
* **k-nearest-neighbor graph.** For *every* point in the cloud, find its :math:`k` nearest *other*
  points -- the classic all-nearest-neighbors problem.

Answering one query by scanning all :math:`N` points is :math:`\mathcal{O}(N)`, so the
all-nearest-neighbors graph is :math:`\mathcal{O}(N^2)` -- infeasible for large clouds. A cloud is therefore
always held in an acceleration structure, a tree or a grid, that lets a query rule out the vast majority of
points without ever measuring the distance to them; :ref:`Chap:CloudSearch` describes the two and when to
choose which.

Clouds of spheres
-----------------

Give each point a radius and the cloud becomes a set of spheres -- particles of differing sizes -- with a surface,
an inside and a signed distance: the smallest :math:`|p - c_i| - r_i` over the spheres, negative inside one. A
sphere cloud is therefore a shape: it composes, ray traces and renders like any other. The hierarchy of
:ref:`Chap:CloudSearch` serves it, built over the spheres' centers, with each node's bounding volume grown to hold its
spheres whole. Where spheres overlap, the value inside is the depth below the deepest sphere's surface, which can be
less than the distance to the union's surface, as for any union of shapes (see :ref:`Chap:ConstructiveSolidGeometry`).

Spheres also touch one another. The *contacts* of a particle are the particles whose surfaces are closer to its
own than a gap, or overlap it: the pairs a particle code computes its contact forces from at every step. They are
found with that hierarchy, from the particle's own sphere grown by the gap.
