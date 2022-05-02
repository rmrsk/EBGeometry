/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */
/*!
  @file   EBGeometry_Polygon2D.hpp
  @brief  Declaration of a two-dimensional polygon class for embedding 3D
  polygon faces
  @author Robert Marskar
*/

#ifndef EBGeometry_Polygon2D
#define EBGeometry_Polygon2D

// Std includes
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Class for embedding a polygon face into 2D.
  @details This class is required for determining whether or not a 3D point
  projected to the plane of an N-sided polygon lies inside or outside the
  polygon face. To do this we compute the 2D embedding of the polygon face,
  reducing the problem to a tractable dimension where we can use well-tested
  algorithm. The 2D embedding of a polygon occurs by taking a set of 3D points
  and a corresponding normal vector, and projecting those points along one of
  the 3D Cartesian axes such that the polygon has the largest area. In essence,
  we simply find the direction with the smallest normal vector component and
  ignore that. Once the 2D embedding is computed, we can use well-known
  algorithms for checking if a point lies inside or outside. The supported
  algorithms are 1) The winding number algorithm (computing the winding number),
  2) Computing the subtended angle of the point with the edges of the polygon
  (sums to 360 degrees if the point is inside), or computing the crossing number
  which checks how many times a ray cast from the point crosses the edges of the
  polygon.
*/
template <class T>
class Polygon2D
{
public:
  /*!
    @brief Supported algorithms for performing inside/outside tests when
    checking if a point projects to the inside or outside of a polygon face.
  */
  enum class InsideOutsideAlgorithm
    {
      SubtendedAngle,
      CrossingNumber,
      WindingNumber
    };

  /*!
    @brief Alias to cut down on typing
  */
  using Vec2 = Vec2T<T>;

  /*!
    @brief Alias to cut down on typing
  */
  using Vec3 = Vec3T<T>;

  /*!
    @brief Disallowed constructor, use the one with the normal vector and points
  */
  Polygon2D() = delete;

  /*!
    @brief Full constructor
    @param[in] a_normal Normal vector of the 3D polygon face
    @param[in] a_points Vertex coordinates of the 3D polygon face
  */
  Polygon2D(const Vec3& a_normal, const std::vector<Vec3>& a_points);

  /*!
    @brief Destructor (does nothing
  */
  ~Polygon2D() = default;

  /*!
    @brief Check if a point is inside or outside the 2D polygon
    @param[in] a_point     3D point coordinates
    @param[in] a_algorithm Inside/outside algorithm
    @details This will call the function corresponding to a_algorithm.
  */
  inline bool
  isPointInside(const Vec3& a_point, const InsideOutsideAlgorithm a_algorithm) const noexcept;

  /*!
    @brief Check if a point is inside a 2D polygon, using the winding number
    algorithm
    @param[in] a_point 3D point coordinates
    @return Returns true if the 3D point projects to the inside of the 2D
    polygon
  */
  inline bool
  isPointInsidePolygonWindingNumber(const Vec3& a_point) const noexcept;

  /*!
    @brief Check if a point is inside a 2D polygon, using the subtended angles
    @param[in] a_point 3D point coordinates
    @return Returns true if the 3D point projects to the inside of the 2D
    polygon
  */
  inline bool
  isPointInsidePolygonSubtend(const Vec3& a_point) const noexcept;

  /*!
    @brief Check if a point is inside a 2D polygon, by computing the number of
    times a ray crosses the polygon edges.
    @param[in] a_point 3D point coordinates
    @return Returns true if the 3D point projects to the inside of the 2D
    polygon
  */
  inline bool
  isPointInsidePolygonCrossingNumber(const Vec3& a_point) const noexcept;

private:
  /*!
    @brief 3D coordinate direction to ignore
  */
  size_t m_ignoreDir;

  /*!
    @brief The corresponding 2D x-direction.
  */
  size_t m_xDir;

  /*!
    @brief The corresponding 2D y-direction.
  */
  size_t m_yDir;

  /*!
    @brief Projected set of points in 2D
  */
  std::vector<Vec2> m_points;

  /*!
    @brief Project a 3D point onto the 2D polygon plane (this ignores one of the
    vector components)
    @param[in] a_poitn 3D point
    @return 2D point, ignoring one of the coordinate directions.
  */
  inline Vec2
  projectPoint(const Vec3& a_point) const noexcept;

  /*!
    @brief Define function. This find the direction to ignore and then computes
    the 2D points.
    @param[in] a_normal Normal vector for polygon face
    @param[in] a_points Vertex coordinates for polygon face.
  */
  inline void
  define(const Vec3& a_normal, const std::vector<Vec3>& a_points);

  /*!
    @brief Compute the winding number for a point P with the 2D polygon
    @param[in] P 2D point
    @return Returns winding number.
  */
  inline int
  computeWindingNumber(const Vec2& P) const noexcept;

  /*!
    @brief Compute the crossing number for a point P with the 2D polygon
    @param[in] P 2D point
    @return Returns crossing number.
  */
  inline size_t
  computeCrossingNumber(const Vec2& P) const noexcept;

  /*!
    @brief Compute the subtended angle for a point P with the 2D polygon
    @param[in] P 2D point
    @return Returns subtended angle.
  */
  inline T
  computeSubtendedAngle(const Vec2& P) const noexcept;
};

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_Polygon2DImplem.hpp"

#endif
