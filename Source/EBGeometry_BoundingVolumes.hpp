/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_BoundingVolumes.hpp
  @brief  Declaration of a various bounding volumes used for bounding volume hierarchy. 
  @author Robert Marskar
*/

#ifndef EBGeometry_BoundingVolumes_H
#define EBGeometry_BoundingVolumes_H

// Std includes
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace for encapsulating various bounding volumes for usage with BVHs
*/
namespace BoundingVolumes {
  
  /*!
    @brief Class which encloses a set of points using a bounding sphere. 
    @details The template parameter T is the floating-point precision which is used. 
  */
  template <class T>
  class BoundingSphereT {
  public:

    /*!
      @brief Typename for possible algorithms that support the computation of a bounding sphere for a set of 3D points. 
    */
    enum class BoundingVolumeAlgorithm {
      Ritter,
    };

    /*!
      @brief Alias to cut down on typing
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Default constructor. Leaves object in undefined state. 
    */
    BoundingSphereT();

    /*!
      @brief Full constructor. Sets the center and radius of the bounding sphere.
      @param[in] a_center Bounding sphere center
      @param[in] a_radius Bounding sphere radius
    */
    BoundingSphereT(const Vec3T<T>& a_center, const T& a_radius);

    /*!
      @brief Full constructor. Constructs a bounding sphere that encloses all the other bounding spheres
      @param[in] a_otherSpheres Other bounding spheres.
    */
    BoundingSphereT(const std::vector<BoundingSphereT<T> >& a_otherSpheres);

    /*!
      @brief Copy constructor. Sets the center and radius from the other sphere. 
      @param[in] a_other Other sphere
    */    
    BoundingSphereT(const BoundingSphereT& a_other);

    /*!
      @brief Template constructor which takes a set of 3D points (mixed precision allowed).
      @details This computes the bounding sphere using the supplied algorithm. 
      @param[in] a_points Set of 3D points
      @param[in] a_alg    Bounding sphere algorithm. 
      @note This calls the define(...) function. 
    */
    template <class P>
    BoundingSphereT(const std::vector<Vec3T<P> >& a_points, const BoundingVolumeAlgorithm& a_alg = BoundingVolumeAlgorithm::Ritter);

    /*!
      @brief Destructor (does nothing). 
    */
    virtual ~BoundingSphereT();

    /*!
      @brief Copy assignment operator
      @param[in] a_other Other sphere
    */    
    BoundingSphereT& operator=(const BoundingSphereT& a_other) = default;

    /*!
      @brief Template define function which takes a set of 3D points (mixed precision allowed).
      @details This computes the bounding sphere using the supplied algorithm. 
      @param[in] a_points Set of 3D points
      @param[in] a_alg    Bounding sphere algorithm. 
    */
    template <class P>
    inline
    void define(const std::vector<Vec3T<P> >& a_points, const BoundingVolumeAlgorithm& a_alg) noexcept;

    /*!
      @brief Check if this bounding sphere intersect another bounding sphere
      @param[in] a_other Other bounding sphere.
      @return True if the two sphere intersect. 
    */
    inline
    bool intersects(const BoundingSphereT& a_other) const noexcept;

    /*!
      @brief Get modifiable radius for this sphere
    */
    inline
    T& getRadius() noexcept;

    /*!
      @brief Get immutable radius for this sphere
    */
    inline
    const T& getRadius() const noexcept;

    /*!
      @brief Get modifiable center for this sphere
    */
    inline
    Vec3& getCentroid() noexcept;

    /*!
      @brief Get immutable center for this sphere
    */
    inline
    const Vec3& getCentroid() const noexcept;

    /*!
      @brief Compute the overlapping volume between this bounding sphere and another
      @param[in] a_other Other bounding sphere
      @return The overlapping volume, computing using standard expressions. 
    */
    inline
    T getOverlappingVolume(const BoundingSphereT<T>& a_other) const noexcept;

    /*!
      @brief Get the distance to this bounding sphere (points inside the sphere have a zero distance)
      @param[in] a_x0 3D point
      @return Returns the distance to the sphere (a point inside has a zero distance)
    */
    inline
    T getDistance(const Vec3& a_x0) const noexcept;

    /*!
      @brief Get the sphere volume
      @return Sphere volume
    */
    inline
    T getVolume() const noexcept;

    /*!
      @brief Get the sphere area
      @return Sphere area. 
    */
    inline
    T getArea() const noexcept;

  protected:

    /*!
      @brief Sphere radius
    */
    T m_radius;

    /*!
      @brief Sphere center
    */    
    Vec3 m_center;

    /*!
      @brief Template function which computes the bounding sphere for a set of points (mixed precision allowed) using Ritter's algorithm
    */
    template <class P>
    inline
    void buildRitter(const std::vector<Vec3T<P> >& a_points) noexcept;
  };

  /*!
    @brief Axis-aligned bounding box as bounding volume
    @details This class represents a Cartesian box that encloses a set of 3D points.
    @note The template parameter T is the precision. 
  */
  template <class T>
  class AABBT {
  public:

    /*!
      @brief Alias which cuts down on typing
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Default constructor (does nothing)
    */
    AABBT();

    /*!
      @brief Full constructor taking the low/high corners of the bounding box
      @param[in] a_lo Low corner
      @param[in] a_hi High
    */    
    AABBT(const Vec3T<T>& a_lo, const Vec3T<T>& a_hi);

    /*!
      @brief Copy constructor of another bounding box
      @param[in] a_other Other bounding box
    */
    AABBT(const AABBT& a_other);

    /*!
      @brief Constructor which creates an AABB which encloses a set of other AABBs. 
      @param[in] a_others Other bounding boxes
    */    
    AABBT(const std::vector<AABBT<T> >& a_others);

    /*!
      @brief Template constructor (since mixed precision allowed) which creates an AABB that encloses a set of 3D points
      @param[in] a_points Set of 3D points
      @note Calls the define function
    */
    template <class P>
    AABBT(const std::vector<Vec3T<P> >& a_points);

    /*!
      @brief Destructor (does nothing)
    */
    virtual ~AABBT();    

    /*!
      @brief Copy assignment
      @param[in] a_other Other bounding box
    */
    AABBT& operator=(const AABBT<T>& a_other) = default;

    /*!
      @brief Define function (since mixed precision allowed) which sets this AABB such that it encloses a set of 3D points
      @param[in] a_points Set of 3D points
    */    
    template <class P>
    inline
    void define(const std::vector<Vec3T<P> >& a_points) noexcept;

    /*!
      @brief Check if this AABB intersects another AABB
      @param[in] a_other The other AABB
      @return True if they intersect and false otherwise. 
    */
    inline
    bool intersects(const AABBT& a_other) const noexcept;

    /*!
      @brief Get the modifiable lower-left corner of the AABB
    */
    inline
    Vec3T<T>& getLowCorner() noexcept;

    /*!
      @brief Get the immutable lower-left corner of the AABB
    */
    inline
    const Vec3T<T>& getLowCorner() const noexcept;

    /*!
      @brief Get the modifiable upper-right corner of the AABB
    */    
    inline
    Vec3T<T>& getHighCorner() noexcept;

    /*!
      @brief Get the immutable upper-right corner of the AABB
    */        
    inline
    const Vec3T<T>& getHighCorner() const noexcept;

    /*!
      @brief Get bounding volume centroid.
    */
    inline
    Vec3 getCentroid() const noexcept;

    /*!
      @brief Compute the overlapping volume between this AABB and another AABB.
      @param[in] a_other The other AABB
      @return Returns overlapping volume
    */
    inline
    T getOverlappingVolume(const AABBT<T>& a_other) const noexcept;

    /*!
      @brief Get the distance to this AABB (points inside the bounding box have a zero distance)
      @param[in] a_x0 3D point
      @return Returns the distance to the bounding box (a point inside has a zero distance)
    */
    inline
    T getDistance(const Vec3& a_x0) const noexcept;

    /*!
      @brief Compute the bounding box volume
    */
    inline
    T getVolume() const noexcept;

    /*!
      @brief Compute the bounding box area
    */    
    inline
    T getArea() const noexcept;
    
  protected:

    /*!
      @brief Lower-left corner of bounding box
    */
    Vec3 m_loCorner;

    /*!
      @brief Upper-right corner of bounding box
    */    
    Vec3 m_hiCorner;
  };

  /*!
    @brief Intersection method for testing if two bounding spheres overlap
    @param[in] a_u One bounding sphere
    @param[in] a_v The other bounding sphere
  */
  template <class T>
  bool intersects(const BoundingSphereT<T>& a_u, const BoundingSphereT<T>& a_v) noexcept;

  /*!
    @brief Intersection method for testing if two bounding boxes overlap
    @param[in] a_u One bounding box
    @param[in] a_v The other bounding box
  */  
  template <class T>
  bool intersects(const AABBT<T>& a_u, const AABBT<T>& a_v) noexcept;

  /*!
    @brief Compute the overlapping volume between two bounding spheres
    @param[in] a_u One bounding sphere
    @param[in] a_v The other bounding sphere
  */
  template <class T>
  T getOverlappingVolume(const BoundingSphereT<T>& a_u, const BoundingSphereT<T>& a_v) noexcept;

  /*!
    @brief Compute the overlapping volume between two bounding boxes
    @param[in] a_u One bounding box
    @param[in] a_v The other bounding box
  */
  template <class T>
  T getOverlappingVolume(const AABBT<T>& a_u, const AABBT<T>& a_v) noexcept;
}

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_BoundingVolumesImplem.hpp"

#endif
