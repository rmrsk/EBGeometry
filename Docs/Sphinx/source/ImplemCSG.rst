.. _Chap:ImplemCSG:

CSG
===

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

   // Signed distance function implementation requires a signeDistance function
   template <class T>
   class SignedDistanceFunction : public ImplicitFunction<T>
   {
      T value(const Vec3T<T>& a_point) const noexcept {
         return this->signedDistance(a_point);
      }

      T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;
   }

Definitions are found in:
   
* :file:`Source/EBGeometry_ImplicitFunction.hpp` for implicit functions.
* :file:`Source/EBGeometry_SignedDistanceFunction.hpp` for signed distance field. 
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
