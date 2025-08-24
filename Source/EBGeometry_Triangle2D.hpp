/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */
/*!
  @file   EBGeometry_Triangle2D.hpp
  @brief  Declaration of a class that encapsulates the projection of a 3D triangle
  into a 2D plane.
  @author Robert Marskar
*/

#ifndef EBGeometry_Triangle2D
#define EBGeometry_Triangle2D

// Std includes
#include <array>

// Our includes
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {
  /*!
    @brief Class for embedding a triangle face into 2D.
    @details This class is required for determining whether or not a 3D point
    projected to the plane of a triangle lies inside or outside the
    triangle. To do this we compute the 2D embedding of the triangle face,
    reducing the problem to a tractable dimension where we can use well-tested
    algorithm. Once the 2D embedding is computed, we can use well-known
    algorithms for checking if a point lies inside or outside. The supported
    algorithms are 1) The winding number algorithm (computing the winding number),
    2) Computing the subtended angle of the point with the edges of the polygon
    (sums to 360 degrees if the point is inside), or computing the crossing number
    which checks how many times a ray cast from the point crosses the edges of the
    triangle.
  */
  template <class T>
  class Triangle2D
  {
  public:
    /*!
      @brief Alias for 2D vector type
    */
    using Vec2 = Vec2T<T>;

    /*!
      @brief Alias for 3D vector type
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Supported algorithms for performing inside/outside tests when
      checking if a point projects to the inside or outside of a polygon face.
    */
    enum class InsideOutsideAlgorithm // NOLINT
    {
      SubtendedAngle,
      CrossingNumber,
      WindingNumber
    };

    /*!
      @brief Default constructor. Must subsequently call the define function.
    */
    Triangle2D() noexcept = default;

    /*!
      @brief Full constructor
      @param[in] a_normal Normal vector of the 3D triangle
      @param[in] a_vertices Vertex coordinates of the 3D triangle
    */
    Triangle2D(const Vec3& a_normal, const std::array<Vec3, 3>& a_vertices) noexcept;

    /*!
      @brief Copy constructor.
      @param[in] a_triangle2D Other triangle
    */
    Triangle2D(const Triangle2D& a_triangle2D) noexcept = default;

    /*!
      @brief Move constructor.
      @param[in, out] a_triangle2D Other triangle
    */
    Triangle2D(Triangle2D&& a_triangle2D) noexcept = default;

    /*!
      @brief Destructor (does nothing)
    */
    ~Triangle2D() noexcept = default;

    /*!
      @brief Copy assignment.
      @param[in] a_triangle2D Other triangle
    */
    Triangle2D&
    operator=(const Triangle2D& a_triangle2D) noexcept = default;

    /*!
      @brief Move assignment.
      @param[in, out] a_triangle2D Other triangle
    */
    Triangle2D&
    operator=(Triangle2D&& a_triangle2D) noexcept = default;

    /*!
      @brief Define function. Puts object in usable state. 
      @param[in] a_normal Normal vector of the 3D triangle
      @param[in] a_vertices Vertex coordinates of the 3D triangle
    */
    void
    define(const Vec3& a_normal, const std::array<Vec3, 3>& a_vertices) noexcept;

    /*!
      @brief Check if a point is inside or outside the 2D polygon
      @param[in] a_point     3D point coordinates
      @param[in] a_algorithm Inside/outside algorithm
      @details This will call the function corresponding to a_algorithm.
    */
    bool
    isPointInside(const Vec3& a_point, InsideOutsideAlgorithm a_algorithm) const noexcept;

    /*!
      @brief Check if a point is inside a 2D polygon, using the winding number
      algorithm
      @param[in] a_point 3D point coordinates
      @return Returns true if the 3D point projects to the inside of the 2D
      polygon
    */
    bool
    isPointInsideWindingNumber(const Vec3& a_point) const noexcept;

    /*!
      @brief Check if a point is inside a 2D polygon, by computing the number of
      times a ray crosses the polygon edges.
      @param[in] a_point 3D point coordinates
      @return Returns true if the 3D point projects to the inside of the 2D
      polygon
    */
    bool
    isPointInsideCrossingNumber(const Vec3& a_point) const noexcept;

    /*!
      @brief Check if a point is inside a 2D polygon, using the subtended angles
      @param[in] a_point 3D point coordinates
      @return Returns true if the 3D point projects to the inside of the 2D
      polygon
    */
    bool
    isPointInsideSubtend(const Vec3& a_point) const noexcept;

  private:
    /*!
      @brief The corresponding 2D x-direction (one direction is ignored)
    */
    int m_xDir = -1;

    /*!
      @brief The corresponding 2D y-direction (one direction is ignored)
    */
    int m_yDir = -1;

    /*!
      @brief List of points in 2D.
      @details This is the position of the vertices, projected into a 2D plane.
    */
    std::array<Vec2, 2> m_vertices{Vec2::max(), Vec2::max()};

    /*!
      @brief Project a 3D point onto the 2D polygon plane (this ignores one of the
      vector components)
      @param[in] a_poitn 3D point
      @return 2D point, ignoring one of the coordinate directions.
    */
    Vec2
    projectPoint(const Vec3& a_point) const noexcept;

    /*!
      @brief Compute the winding number for a point P with the 2D polygon
      @param[in] P 2D point
      @return Returns winding number.
    */
    int
    computeWindingNumber(const Vec2& a_point) const noexcept;

    /*!
      @brief Compute the crossing number for a point P with the 2D polygon
      @param[in] a_point 2D point
      @return Returns crossing number.
    */
    int
    computeCrossingNumber(const Vec2& a_point) const noexcept;

    /*!
      @brief Compute the subtended angle for a point a_point with the 2D polygon
      @param[in] a_point 2D point
      @return Returns subtended angle.
    */
    T
    computeSubtendedAngle(const Vec2& a_point) const noexcept;
  };
} // namespace EBGeometry

#include "EBGeometry_Triangle2DImplem.hpp" // NOLINT

#endif
