.. _Chap:ImplemOctree:

Octree subdivision
==================

EBGeometry uses octree subdivision (see :ref:`Chap:Octree` for the conceptual picture) for one
job: estimating a bounding box for a function that has no closed-form bound. It keeps no tree. The
free function
`approximateBoundingVolumeOctree <doxygen/html/namespaceEBGeometry.html#a2c8abb547ef889198ea25deedd70ea5e>`__
(:file:`Source/EBGeometry_FunctionQueries.hpp`) walks the subdivision depth-first with a fixed
stack, keeping only a running minimum and maximum, so it allocates nothing and is callable on a
device whenever the function it is given is. The same header holds
`normal <doxygen/html/namespaceEBGeometry.html#af196161d66e46c5834e178aad6605b50>`__, the finite-difference normal of
any function.

Both take the function as anything that is evaluated at a point: an analytic shape or a mesh
distance field (through its ``signedDistance()``), an ``ImplicitFunction<T>`` (through its
``value()``), or a callable such as a lambda, tried in that order.

.. _Sec:OctreeBoundingVolume:

Estimating a bounding box for an implicit function
--------------------------------------------------

``approximateBoundingVolumeOctree(function, lo, hi, depth, safety)`` estimates an
``AABBT<T>`` for a function :math:`I` that has no closed-form bound -- for example, one built up
from several nested CSG operations (see :ref:`Chap:ConstructiveSolidGeometry`). Given an initial
box and a depth, the algorithm is:

#. A cell is kept if the surface may pass through it, that is, if

   .. math::

      \left|I\left(\mathbf{x}_c\right)\right| \leq (1 + \sigma)\left|\Delta\mathbf{x}\right|,

   where :math:`\mathbf{x}_c` is the cell's center, :math:`\Delta\mathbf{x}` is its half-diagonal,
   and :math:`\sigma \geq 0` is a safety factor. This is a direct consequence of the Eikonal
   property (see :ref:`Chap:GeometryRepresentations`): since :math:`I` changes by at most the
   distance moved, evaluating it at a single point bounds how far away the surface can be from
   that point, so a cell whose center is farther from the surface than the cell itself extends
   cannot contain it.
#. Starting from the initial box, each kept cell is split into its eight octants and each octant is
   tested in turn, depth-first, down to the given depth. A cell is described by its level and its
   integer position among the cells of that level, so its corners are computed directly from the
   initial box rather than accumulated level by level.
#. The box around the kept cells at the deepest level is the result.

The depth is at most ``MaxOctreeDepth`` (24, which bounds the traversal stack), and a deeper one is
rejected. If the initial box is empty or inverted along any axis, does not contain the surface, or
no cell survives to the deepest level, the function returns the maximal box, from
``-Vec3T<T>::max()`` to ``Vec3T<T>::max()``, signalling that the initial box needs to be chosen
more generously or that the function is far from a signed distance.
``ImplicitFunction<T>`` keeps a member function of the same name that calls the free function with
the object itself.

.. tip::

   A deeper subdivision gives a tighter box, at the cost of more evaluations of :math:`I` (one per
   candidate cell, at every level).

.. warning::

   This is an approximation, not an exact bound: it is only as tight as the finest cell size
   actually reached, and an implicit function whose rate of change meaningfully exceeds unity
   (violating the Eikonal property) can, in principle, still have surface features that fall
   outside the estimate unless :math:`\sigma` is chosen generously enough to compensate.

See the :ref:`Chap:ExampleOctreeBoundingVolume` example for both a single shape and a lambda.

Normals of any function
-----------------------

``normal(function, point, delta)`` approximates the gradient of the function at ``point`` by
central differences with step ``delta`` along each axis, and normalizes it. For a signed distance
this is the outward unit normal of the level set through ``point``. The error is of order
``delta`` squared, plus rounding of order machine epsilon divided by ``delta``, and the result is
undefined where the gradient vanishes (at a local extremum of the function).
