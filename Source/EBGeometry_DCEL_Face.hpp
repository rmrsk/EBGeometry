/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_Face.hpp
  @brief  Declaration of a polygon face class for use in DCEL descriptions of polygon tesselations. 
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_Face
#define EBGeometry_DCEL_Face

// Std includes
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_Polygon2D.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  // Forward declarations of other DCEL functionality. 
  template <class T> class VertexT;
  template <class T> class EdgeT;
  template <class T> class FaceT;
  template <class T> class EdgeIteratorT;

  /*!
    @brief Class which represents a polygon face in a double-edge connected list (DCEL). 
    @details This class is a polygon face in a DCEL mesh. It contains pointer storage to one of the half-edges that circulate the inside of the polygon face,
    as well as having a normal vector, a centroid, and an area. This class supports signed distance computations. These computations require algorithms that compute
    e.g. the winding number of the polygon, or the number of times a ray cast passes through it. Thus, one of its central features is that it can be embedded in 2D 
    by projecting it along the cardinal direction of its normal vector. To be fully consistent with a DCEL structure the class stores a reference to one of its half edges,
    but for performance reasons it also stores references to the other half edges. 
    @note To compute the distance from a point to the face one must determine if the point projects "inside" or "outside" the polygon. There are several algorithms for this,
    and by default this class uses a crossing number algorithm. Other algorithms can be set in setInsideOutsideAlgorithm (see CD_DCELAlgorithms.H)
  */
  template <class T>
  class FaceT {
  public:

    /*!
      @brief Alias to cut down on typing
    */
    using Vec3 = Vec3T<T>;

    /*!
      @brief Alias to cut down on typing
    */
    using Vertex = VertexT<T>;

    /*!
      @brief Alias to cut down on typing
    */    
    using Edge = EdgeT<T>;

    /*!
      @brief Alias to cut down on typing
    */        
    using Face = FaceT<T>;
    
    /*!
      @brief Alias to cut down on typing
    */        
    using VertexPtr = std::shared_ptr<Vertex>;

    /*!
      @brief Alias to cut down on typing
    */            
    using EdgePtr = std::shared_ptr<Edge>;

    /*!
      @brief Alias to cut down on typing
    */                
    using FacePtr = std::shared_ptr<Face>;

    /*!
      @brief Alias to cut down on typing
    */            
    using EdgeIterator = EdgeIteratorT<T>;

    /*!
      @brief Default constructor. Sets the half-edge to zero and the inside/outside algorithm to crossing number algorithm
    */
    FaceT();

    /*!
      @brief Partial constructor. Calls default constructor but associates a half-edge
      @param[in] a_edge Half-edge
    */
    FaceT(const EdgePtr& a_edge);

    /*!
      @brief Partial constructor. 
      @details Calls default constructor but sets the normal vector and half-edge equal to the other face's (rest is undefined)
    */
    FaceT(const Face& a_otherFace);

    /*!
      @brief Destructor (does nothing)
    */
    ~FaceT();

    /*!
      @brief Define function which sets the normal vector and half-edge
      @param[in] a_normal Normal vector
      @param[in] a_edge   Half edge
    */
    inline
    void define(const Vec3& a_normal, const EdgePtr& a_edge) noexcept;

    /*!
      @brief Reconcile face. This will compute the normal vector, area, centroid, and the 2D embedding of the polygon
      @note "Everything" must be set before doing this, i.e. the face must be complete with half edges and there can be no dangling edges. 
    */
    inline
    void reconcile() noexcept;

    /*!
      @brief Set the half edge
      @param[in] a_halfEdge Half edge
    */
    inline
    void setHalfEdge(const EdgePtr& a_halfEdge) noexcept;

    /*!
      @brief Set the inside/outside algorithm when determining if a point projects to the inside or outside of the polygon. 
      @param[in] a_algorithm Desired algorithm
      @note See CD_DCELAlgorithms.H for options (and CD_DCELPolyImplem.H for how the algorithms operate). 
    */
    inline
    void setInsideOutsideAlgorithm(typename Polygon2D<T>::InsideOutsideAlgorithm& a_algorithm) noexcept;

    /*!
      @brief Get modifiable half-edge
    */
    inline
    EdgePtr& getHalfEdge() noexcept;

    /*!
      @brief Get immutable half-edge
    */
    inline
    const EdgePtr& getHalfEdge() const noexcept;

    /*!
      @brief Get modifiable centroid
    */
    inline
    Vec3T<T>& getCentroid() noexcept;

    /*!
      @brief Get immutable centroid
    */
    inline
    const Vec3T<T>& getCentroid() const noexcept;

    /*!
      @brief Get modifiable centroid position in specified coordinate direction
      @param[in] a_dir Coordinate direction
    */
    inline
    T& getCentroid(const size_t a_dir) noexcept;

    /*!
      @brief Get immutable centroid position in specified coordinate direction
      @param[in] a_dir Coordinate direction
    */
    inline
    const T& getCentroid(const size_t a_dir) const noexcept;

    /*!
      @brief Get modifiable normal vector
    */
    inline
    Vec3T<T>& getNormal() noexcept;

    /*!
      @brief Get immutable normal vector
    */
    inline
    const Vec3T<T>& getNormal() const noexcept;

    /*!
      @brief Compute the signed distance to a point.
      @param[in] a_x0 Point in space
      @details This algorithm operates by checking if the input point projects to the inside of the polygon. If it does then the distance is just the 
      projected distance onto the polygon plane and the sign is well-defined. Otherwise, we check the distance to the edges of the polygon. 
    */
    inline
    T signedDistance(const Vec3& a_x0) const noexcept;

    /*!
      @brief Compute the unsigned squared distance to a point.
      @param[in] a_x0 Point in space
      @details This algorithm operates by checking if the input point projects to the inside of the polygon. If it does then the distance is just the 
      projected distance onto the polygon plane. Otherwise, we check the distance to the edges of the polygon. 
    */
    inline
    T unsignedDistance2(const Vec3& a_x0) const noexcept;

    /*!
      @brief Return the coordinates of all the vertices on this polygon.
      @details This builds a list of all the vertex coordinates and returns it.
    */
    inline
    std::vector<Vec3T<T> > getAllVertexCoordinates() const noexcept;

    /*!
      @brief Return all the vertices on this polygon
      @details This builds a list of all the vertices and returns it.
    */
    inline
    std::vector<VertexPtr> gatherVertices() const noexcept;

    /*!
      @brief Get the lower-left-most coordinate of this polygon face
    */
    inline
    Vec3T<T> getSmallestCoordinate() const noexcept;

    /*!
      @brief Get the upper-right-most coordinate of this polygon face
    */
    inline
    Vec3T<T> getHighestCoordinate() const noexcept;
  
  protected:

    /*!
      @brief This polygon's half-edge. A valid face will always have != nullptr
    */
    EdgePtr m_halfEdge;

    /*!
      @brief Pointers to all the half-edges of this face. Exists for performance reasons (in signedDistance(...))
    */
    std::vector<EdgePtr > m_edges; // Exists because of performance reasons. 

    /*!
      @brief Polygon face area
    */
    T m_area;

    /*!
      @brief Polygon face normal vector
    */
    Vec3 m_normal;

    /*!
      @brief Polygon face centroid position
    */
    Vec3 m_centroid;

    /*!
      @brief 2D embedding of this polygon. This is the 2D view of the current object projected along its normal vector cardinal. 
    */
    std::shared_ptr<Polygon2D<T> > m_poly2;

    /*!
      @brief Algorithm for inside/outside tests
    */
    typename Polygon2D<T>::InsideOutsideAlgorithm m_poly2Algorithm;

    /*!
      @brief Compute the area of this polygon
    */
    inline
    void computeArea() noexcept;

    /*!
      @brief Compute the centroid position of this polygon
    */
    inline
    void computeCentroid() noexcept;

    /*!
      @brief Compute the normal position of this polygon
    */
    inline
    void computeNormal() noexcept;

    /*!
      @brief Compute the 2D embedding of this polygon
    */
    inline
    void computePolygon2D() noexcept;

    /*!
      @brief Normalize the normal vector, ensuring it has a length of 1
    */
    inline
    void normalizeNormalVector() noexcept;

    /*!
      @brief Get the area of this polygon face
    */
    inline
    T getArea() noexcept;

    /*!
      @brief Get the area of this polygon face
    */
    inline
    T getArea() const noexcept;

    /*!
      @brief Compute and store all the half-edges around this polygon face
    */
    inline
    void computeAndStoreEdges() noexcept;

    /*!
      @brief Compute the projection of a point onto the polygon face plane
      @param[in] a_p Point in space
    */
    inline
    Vec3T<T> projectPointIntoFacePlane(const Vec3& a_p) const noexcept;

    /*!
      @brief Check if a point projects to inside or outside the polygon face
      @param[in] a_p Point in space
      @return Returns true if a_p projects to inside the polygon and false otherwise. 
    */
    inline
    bool isPointInsideFace(const Vec3& a_p) const noexcept;
  };
}

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_DCEL_FaceImplem.hpp"

#endif
