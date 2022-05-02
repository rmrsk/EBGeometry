.. _Chap:ImplemSDF:

Signed distance function
========================

In EBGeometry we have encapsulated the concept of a signed distance function in an abstract class

.. literalinclude:: ../../Source/EBGeometry_SignedDistanceFunction.hpp   
   :language: c++

We point out that the BVH and DCEL classes are fundamentally also signed distance functions, and they also inherit from ``SignedDistanceFunction``.
The ``SignedDistanceFunction`` class also exists so that we have a common entry point for performing distance field manipulations like rotations and translations.

When implementing the ``signedDistance`` function, one can transform the input point by first calling ``transformPoint``.
The functions ``translate`` and ``rotate`` will translate or rotate the object.
It is also possible to *scale* an object, but this is not simply a coordinate transform so it is implemented as a separate signed distance function.
For example, in order to rotate a DCEL mesh (without using the BVH accelerator) we can implement the following signed distance function:

.. code-block:: c++

   template <class T>
   class MySignedDistanceFunction : public SignedDistanceFunction<T> {
   public:
      T signedDistance(const Vec3T<T>& a_point) const noexcept override {
         return m_mesh->signedDistance(this->transformPoint(a_point));
      }

   protected:
      // DCEL mesh object, must be constructed externally and
      // supplied to MyDistanceFunction (e.g. through the constructor). 
      std::shared_ptr<EBGeometry::Dcel::MeshT<T> > m_mesh;
   };

Alternatively, using a BVH structure:

.. code-block:: c++

   template <class T, class P, class BV, int K>
   class MySignedDistanceFunction : public SignedDistanceFunction<T> {
   public:
      T signedDistance(const Vec3T<T>& a_point) const noexcept override {
         return m_bvh->signedDistance(this->transformPoint(a_point));
      }

   protected:
      // BVH object, must be constructed externally
      // and supplied to MyDistanceFunction (e.g. through the constructor). 
      std::shared_ptr<EBGeometry::BVH::LinearBVH<T, P, BV, K> > m_bvh;
   };

Normal vector
-------------

The normal vector of ``EBGeometry::SignedDistanceFunction<T>`` is computed using centered finite differences:

.. math::

   n_i\left(\mathbf{x}\right) = \frac{1}{2\Delta}\left[S\left(\mathbf{x} + \Delta\mathbf{\hat{i}}\right) - S\left(\mathbf{x} - \Delta\mathbf{\hat{i}}\right)\right],

where :math:`i` is a coordinate direction and :math:`\Delta > 0`.
This is done for each component, and the normalized vector is then returned. 

Transformations
---------------

The following transformations are possible:

* Translation, which defines the operation :math:`\mathbf{x}^\prime = \mathbf{x} - \mathbf{t}` where :math:`\mathbf{t}` is a translation vector.
* Rotation, which defines the operation :math:`\mathbf{x}^\prime = R\left(\mathbf{x}, \theta, a\right)` where :math:`\mathbf{x}` is rotated an angle :math:`\theta` around the coordinate axis :math:`a`.

Transformations are applied sequentially.
The APIs are as follows:

.. code-block:: c++
		
  void translate(const Vec3T<T>& a_translation) noexcept;  // a_translation are Cartesian translations vector
  void rotate(const T a_angle, const int a_axis) noexcept; // a_angle in degrees, and a_axis being the Cartesian axis
  
E.g. the following code will first translate, then 90 degrees about the :math:`x`-axis. 

.. code-block::

   MySignedDistanceFunction<float> sdf;

   sdf.translate({1,0,0});
   sdf.rotate(90, 0);

Note that if the transformations are to be applied, the implementation of ``signedDistance(...)`` must transform the input point, as shown in the examples above.

Rounding
--------

Distance functions can be rounded by displacing the SDF by a specified distance.
For example, given a distance functions :math:`S\left(\mathbf{x}\right)`, the rounded distance functions is

.. math::

   S_r\left(\mathbf{x}\right) = S\left(\mathbf{x}\right) - r,

where :math:`r` is some rounding radius.
Note that the rounding does not preserve the volume of the original SDF, so subsequent *scaling* of the object is usually necessary.

The rounded SDF is implemented in :file:`Source/EBGeometry_AnalyticDistanceFunctions.hpp` 

.. literalinclude:: ../../Source/EBGeometry_AnalyticDistanceFunctions.hpp
   :language: c++
   :lines: 45-89

To use it, simply pass an SDF into the constructor and use the new distance function.

Scaling
-------

Scaling of distance functions are possible through the transformation

.. math::

   S_c\left(\mathbf{x}\right) = c S\left(\frac{\mathbf{x}}{c}\right),

where :math:`c` is a scaling factor.
We point out that anisotropic stretching does not preserve the distance field.

The rounded SDF is implemented in :file:`Source/EBGeometry_AnalyticDistanceFunctions.hpp`

.. literalinclude:: ../../Source/EBGeometry_AnalyticDistanceFunctions.hpp
   :language: c++
   :lines: 143-188

.. _Chap:AnalyticSDF:

Analytic functions
------------------

Above, we have shown how users can supply a DCEL or BVH structure to implement ``SignedDistanceFunction``.
In addition, the file :file:`Source/EBGeometry_AnalyticSignedDistanceFunctions.hpp` defines various other analytic shapes such as:

* **Sphere**

  .. code-block:: c++

     template <class T>
     class SphereSDF : public SignedDistanceFunction<T>		

* **Box**

  .. code-block:: c++

     template <class T>
     class BoxSDF : public SignedDistanceFunction<T>		  

* **Torus**

  .. code-block:: c++

     template <class T>
     class TorusSDF : public SignedDistanceFunction<T>		  

* **Capped cylinder**

  .. code-block:: c++

     template <class T>
     class CylinderSDF : public SignedDistanceFunction<T>

* **Infinite cylinder**

  .. code-block:: c++

     template <class T>
     class InfiniteCylinderSDF : public SignedDistanceFunction<T>

* **Capsule/rounded cylinder**

  .. code-block:: c++

     template <class T>
     class CapsuleSDF : public SignedDistanceFunction<T>  

* **Infinite cone**

  .. code-block:: c++

     template <class T>
     class InfiniteConeSDF : public SignedDistanceFunction<T>    

* **Cone**

  .. code-block:: c++

     template <class T>
     class ConeSDF : public SignedDistanceFunction<T>      
