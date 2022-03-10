.. _Chap:ImplemSDF:

Signed distance function
========================

In EBGeometry we have encapsulated the concept of a signed distance function in an abstract class

.. code-block:: c++

   template <class T>
   class SignedDistanceFunction {
   public:

      void translate(const Vec3T<T>& a_translation) noexcept;
      void rotate(const T a_angle, const int a_axis) noexcept;
   
      T signedDistance(const Vec3T<T>& a_point) const noexcept = 0;

   protected:

      Vec3T<T> transformPoint(const Vec3T<T>& a_point) const noexcept;   
   };

We point out that the BVH and DCEL functionalities are fundamentally also signed distance functions.
The ``SignedDistanceFunction`` class exists so that we have a common entry point for performing distance field manipulations like rotations and translations.

.. note::

When implementing the ``signedDistance`` function, one can transform the input point by first calling ``transformPoint``.

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

.. _Chap:AnalyticSDF:

Analytic functions
------------------

Above, we have shown how users can supply a DCEL or BVH structure to implement ``SignedDistanceFunction``.
In addition, the file :file:`Source/EBGeometry_AnalyticSignedDistanceFunctions.hpp` defines various other analytic shapes such as:

* **Sphere**

  .. code-block:: c++

     template<class T>
     class EBGeometry::SphereSDF : public EBGeometry::SignedDistanceFunction<T>;

* **Box**

  .. code-block:: c++

     template<class T>
     class EBGeometry::BoxSDF : public EBGeometry::SignedDistanceFunction<T>;     
     
* **Torus**

  .. code-block:: c++

     template<class T>
     class EBGeometry::TorusSDF : public EBGeometry::SignedDistanceFunction<T>;     

* **Cylinder**

  .. code-block:: c++

     template<class T>
     class EBGeometry::CylinderSDF : public EBGeometry::SignedDistanceFunction<T>;
