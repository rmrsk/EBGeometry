.. _Chap:ImplemCSG:

Geometry representation
=======================

Implicit functions
------------------

EBGeometry implements implement functions and signed distance functions through virtual classes

.. code-block:: c++

   // Implicit function implementation requires a value function
   template <class T>
   class ImplicitFunction
   {
      T value(const Vec3T<T>& a_point) const noexcept = 0;
   }

   // Signed distance fields implementation require an additional function:
   template <class T>
   class SignedDistanceFunction : public ImplicitFunction<T>
   {
      T value(const Vec3T<T>& a_point) const noexcept {
         return this->signedDistance(a_point);
      }

      T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;
   }

Note that ``T`` is a floating point precision, which is supported because DCEL meshes can take up quite a bit of computer memory.
These declarations are found in

* :file:`Source/EBGeometry_ImplicitFunction.hpp` for implicit functions.
* :file:`Source/EBGeometry_SignedDistanceFunction.hpp` for signed distance field.

Various useful implementations of implicit functions and distance fields are found in:

* :file:`Source/EBGeometry_AnalyticDistanceFields.hpp` for various pre-defined analytic distance fields.
* :file:`Source/EBGeometry_MeshDistanceFields.hpp` for various distance field representations for DCEL meshes. 

Transformations
---------------

Various transformations for implicit functions are defined in :file:`Source/EBGeometry_Transform.hpp`, such as rotations, translations, etc.
These are also available through functions that automatically cast the resulting implicit function to ``ImplicitFunction<T>``:

.. literalinclude:: ../../../Source/EBGeometry_Transform.hpp   
   :language: c++
   :lines: 20-89

CSG operations
--------------

CSG operations for implicit functions are defined in :file:`Source/EBGeometry_CSG.hpp`.
These also include accelerated variants that take advantage of BVH partitioning of the objects.

.. literalinclude:: ../../../Source/EBGeometry_CSG.hpp   
   :language: c++
   :lines: 23-141

Bounding volumes
----------------

For simple shapes, bounding volumes can be directly constructed given an implicit function.
E.g. one can easily find the bounding volume for a sphere with a known center and radius, or for a DCEL mesh.
However, more complicated implicit functions require us to compute the bounding volume, or at the very least an approximation to it.
``ImplicitFunction<T>`` has a member function that uses spatial subdivision (based on octrees) for doing this:

.. code-block:: c++

   // Compute an approximate bounding volume BV.
   template <class BV>
   inline BV
   approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                   const Vec3T<T>&    a_initialHighCorner,
                                   const unsigned int a_maxTreeDepth,
                                   const T&           a_safety = 0.0) const noexcept;

This function initializes a cubic region in space and uses octree refinement near the implicit surface.
At the end of the octree recusion the vertices of the octree leaves are collected and a bounding volume of type ``BV`` that encloses them is computed.
The success of this method relies on the implicit function being a signed distance function (or at least an approximation to it).
