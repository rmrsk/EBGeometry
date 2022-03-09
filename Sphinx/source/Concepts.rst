.. _Chap:Concepts:

Signed distance fields
----------------------

The signed distance function is defined as a function :math:`S: \mathbb{R}^3 \rightarrow \mathbb{R}`, and returns the *signed distance* to the object.
The signed distance function has the additional property:

.. math::
   :label: Eikonal

   \left|\nabla S(\mathbf{x})\right| = 1 \quad\textrm{everywhere}.   	   
   
In EBGeometry we use the following convention: 

.. math::

   S(\mathbf{x}) =
   \begin{cases}
   > 0, & \textrm{for points outside the object}, \\
   < 0, & \textrm{for points inside the object}.
   \end{cases}

Signed distance functions are also *implicit functions* (but the reverse statement is not true).
For example, the signed distance function for a sphere with center :math:`\mathbf{x}_0` and radius :math:`R` can be written

.. math::

   S_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right| - R.

An example of an implicit function for the same sphere is

.. math::
   
   I_{\textrm{sph}}\left(\mathbf{x}\right) = \left|\mathbf{x} - \mathbf{x}_0\right|^2 - R^2.

An important difference between these is the Eikonal property in :eq:`Eikonal`, ensuring that the signed distance function always returns the exact distance to the object.

Transformations
---------------

Signed distance functions retain the Eikonal property for the following set of transformations:

* Rotations.
* Translations.

Unions
------

Unions of signed distance fields are also signed distance fields *provided that the objects do not intersect or touch*.
For overlapping objects the signed distance function is not well-defined (since the interior and exterior are not well-defined).

For non-overlapping objects represented as signed distance fields :math:`\left(S_1\left(\mathbf{x}\right), S_2\left(\mathbf{x}\right), \ldots\right)`, the composite signed distance field is

.. math::

   S\left(\mathbf{x}\right) = S_k\left(\mathbf{x}\right),

where :math:`k` is index of the closest object (which is found by evaluating :math:`\left|S_i\left(\mathbf{x}\right)\right|`.

.. important::

   
