/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_ImplicitFunction.hpp
  @brief  Abstract base class for representing an implicit function.
  @author Robert Marskar
*/

#ifndef EBGeometry_ImplicitFunction
#define EBGeometry_ImplicitFunction

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Abstract representation of an implicit function function (not necessarily signed distance). 

  The value function must be implemented by subclasses. 
*/
template <class T>
class ImplicitFunction
{
public:
  /*!
    @brief Disallowed, use the full constructor
  */
  ImplicitFunction() = default;

  /*!
    @brief Destructor (does nothing)
  */
  virtual ~ImplicitFunction() = default;

  /*!
    @brief Value function. Points are outside the object if value > 0.0 and inside
    if value < 0.0
    @param[in] a_point 3D point.
  */
  virtual T
  value(const Vec3T<T>& a_point) const noexcept = 0;

  /*!
    @brief Alternative signature for the value function.
    @param[in] a_point 3D point.
  */
  T
  operator()(const Vec3T<T>& a_point) const noexcept;

  /*!
    @brief Compute an approximation to the bounding volume for the implicit surface, using octrees.
    @details This routine will try to compute a bonding using octree subdivision of the implicit function. This routine will
    characterize a cubic region in space as being 'inside', 'outside', or intersected by computing the value function at the
    center of the cube. If the value function is larger than the extents of the cube, we assume that there is no intersection
    inside the cube. The success of this algorithm therefore relies on the implicit function also being a signed distance 
    function, or at the very least not being horrendously far from being an SDF. 
    @param[in] a_initialLowCorner Initial low corner. 
    @param[in] a_initialHighCorner Initial high corner. 
    @param[in] a_maxTreeDepth Maximum permitted octree depth.
    @param[in] a_safety Safety factor when determining intersection. a_safety=1 sets safety factor to cube width, a_safety=2 sets twice the cube width, etc. 
    @note The bounding volume type BV MUST have a constructor BV(std::vector<Vec3T<T>>).
  */
  template <class BV>
  inline BV
  approximateBoundingVolumeOctree(const Vec3T<T>&    a_initialLowCorner,
                                  const Vec3T<T>&    a_initialHighCorner,
                                  const unsigned int a_maxTreeDepth,
                                  const T&           a_safety = 0.0) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ImplicitFunctionImplem.hpp"

#endif
