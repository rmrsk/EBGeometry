.. _Chap:GeometryRepresentations:

Geometry representations
==========================

Signed distance fields
------------------------

The signed distance function is defined as a function :math:`S: \mathbb{R}^3 \rightarrow \mathbb{R}`, and returns the *signed distance* to the object.
It has the additional property

.. math::
   :label: Eikonal

   \left|\nabla S(\mathbf{x})\right| = 1 \quad\textrm{everywhere}.

The normal vector is always

.. math::

   \mathbf{n} = \nabla S\left(\mathbf{x}\right).

EBGeometry uses the following convention for the sign:

.. math::

   S(\mathbf{x}) =
   \begin{cases}
   > 0, & \textrm{for points outside the object}, \\
   < 0, & \textrm{for points inside the object},
   \end{cases}

which means that the normal vector :math:`\mathbf{n}` points away from the object.

Conceptually, every signed distance field in EBGeometry is just such a function :math:`S`,
mapping a query point :math:`\mathbf{x}` to a distance

.. math::

   d = S(\mathbf{x}).

Every kind of signed distance field described on the following pages -- analytic shapes,
surface meshes, and constructive solid geometry (CSG) combinations of either -- implements
exactly this mapping, which is why they can be freely combined and swapped for one another.

Implicit functions
--------------------

Like distance functions, implicit functions also determine whether or not a point :math:`\mathbf{x}` is inside or outside an object.
Signed distance functions are also *implicit functions*, but not vice versa.
For example, the signed distance function for a sphere with center :math:`\mathbf{x}_0` and radius :math:`R` can be written

.. math::

   S_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right| - R.

An example of an implicit function for the same sphere is

.. math::

   I_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right|^2 - R^2.

An important difference between these is the Eikonal property in :eq:`Eikonal`, ensuring that the signed distance function always returns the exact distance to the object.
Signed distance functions are more useful objects, but many operations (e.g. CSG unions) do not preserve the signed distance property (but still provide *bounds* for the signed distance).
