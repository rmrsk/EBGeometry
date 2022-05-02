/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_Edge.hpp
  @brief  Declaration of a half-edge class for use in DCEL descriptions of
  polygon tesselations.
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_Edge
#define EBGeometry_DCEL_Edge

// Std includes
#include <vector>
#include <memory>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  // Forward declare classes.
  template <class T>
  class VertexT;
  template <class T>
  class EdgeT;
  template <class T>
  class FaceT;
  template <class T>
  class EdgeIteratorT;

  /*!
    @brief Class which represents a half-edge in a double-edge connected list
    (DCEL).
    @details This class is used in DCEL functionality which stores polygonal
    surfaces in a mesh. The information contain in an EdgeT object contains the
    necessary object for logically circulating the inside of a polygon face. This
    means that a polygon face has a double-connected list of half-edges which
    circulate the interior of the face. The EdgeT object is such a half-edge; it
    represents the outgoing half-edge from a vertex, located such that it can be
    logically represented as a half edge on the "inside" of a polygon face. It
    contains pointers to the polygon face, next half edge, and the previous half
    edge. It also contains a pointer to the "pair" half edge, i.e. the
    corresponding half-edge on the other face that shares this edge. Since this
    class is used with DCEL functionality and signed distance fields, this class
    also has a signed distance function and thus a "normal vector". For numericaly
    efficiency, some extra storage is also allocated (such as the vector between
    the starting vertex and the end vertex).
    @note The normal vector is outgoing, i.e. a point x is "outside" if the dot
    product between n and (x - x0) is positive.
  */
  template <class T>
  class EdgeT
  {
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
      @brief Default constructor. Sets all pointers to zero and vectors to zero
      vectors
    */
    EdgeT();

    /*!
      @brief Copy constructor. Copies all information from the other half-edge.
      @param[in] a_otherEdge Other edge
    */
    EdgeT(const Edge& a_otherEdge);

    /*!
      @brief Partial constructor. Calls the default constructor but sets the
      starting vertex.
      @param[in] a_vertex Starting vertex.
    */
    EdgeT(const VertexPtr& a_vertex);

    /*!
      @brief Destructor (does nothing)
    */
    ~EdgeT();

    /*!
      @brief Define function. Sets the starting vertex, edges, and normal vectors
      @param[in] a_vertex       Starting vertex
      @param[in] a_pairEdge     Pair half-edge
      @param[in] a_nextEdge     Next half-edge
      @param[in] a_previousEdge Previous half-edge
      @param[in] a_normal       Edge normal vector
    */
    inline void
    define(const VertexPtr& a_vertex,
           const EdgePtr&   a_pairEdge,
           const EdgePtr&   a_nextEdge,
           const EdgePtr&   a_previousEdge,
           const Vec3       a_normal) noexcept;

    /*!
      @brief Set the starting vertex
      @param[in] a_vertex Starting vertex
    */
    inline void
    setVertex(const VertexPtr& a_vertex) noexcept;

    /*!
      @brief Set the pair edge
      @param[in] a_pairEdge Pair edge
    */
    inline void
    setPairEdge(const EdgePtr& a_pairEdge) noexcept;

    /*!
      @brief Set the next edge
      @param[in] a_nextEdge Next edge
    */
    inline void
    setNextEdge(const EdgePtr& a_nextEdge) noexcept;

    /*!
      @brief Set the previous edge
      @param[in] a_previousEdge Previous edge
    */
    inline void
    setPreviousEdge(const EdgePtr& a_previousEdge) noexcept;

    /*!
      @brief Set the pointer to this half-edge's face.
    */
    inline void
    setFace(const FacePtr& a_face) noexcept;

    /*!
      @brief Compute edge normal and edge length (for performance reasons)
    */
    inline void
    reconcile() noexcept;

    /*!
      @brief Get modifiable starting vertex
      @return Returns m_vertex
    */
    inline VertexPtr&
    getVertex() noexcept;

    /*!
      @brief Get immutable starting vertex
      @return Returns m_vertex
    */
    inline const VertexPtr&
    getVertex() const noexcept;

    /*!
      @brief Get modifiable end vertex
      @return Returns the next half-edge's starting vertex
    */
    inline VertexPtr&
    getOtherVertex() noexcept;

    /*!
      @brief Get immutable end vertex
      @return Returns the next half-edge's starting vertex
    */
    inline const VertexPtr&
    getOtherVertex() const noexcept;

    /*!
      @brief Get modifiable pair edge
      @return Returns the pair edge
    */
    inline EdgePtr&
    getPairEdge() noexcept;

    /*!
      @brief Get immutable pair edge
      @return Returns the pair edge
    */
    inline const EdgePtr&
    getPairEdge() const noexcept;

    /*!
      @brief Get modifiable previous edge
      @return Returns the previous edge
    */
    inline EdgePtr&
    getPreviousEdge() noexcept;

    /*!
      @brief Get immutable previous edge
      @return Returns the previous edge
    */
    inline const EdgePtr&
    getPreviousEdge() const noexcept;

    /*!
      @brief Get modifiable next edge
      @return Returns the next edge
    */
    inline EdgePtr&
    getNextEdge() noexcept;

    /*!
      @brief Get immutable next edge
      @return Returns the next edge
    */
    inline const EdgePtr&
    getNextEdge() const noexcept;

    /*!
      @brief Get modifiable half-edge normal vector
    */
    inline Vec3T<T>&
    getNormal() noexcept;

    /*!
      @brief Get immutable half-edge normal vector
    */
    inline const Vec3T<T>&
    getNormal() const noexcept;

    /*!
      @brief Get modifiable half-edge face
    */
    inline FacePtr&
    getFace() noexcept;

    /*!
      @brief Get immutable half-edge face
    */
    inline const FacePtr&
    getFace() const noexcept;

    /*!
      @brief Get the signed distance to this half edge
      @details This routine will check if the input point projects to the edge or
      one of the vertices. If it projectes to one of the vertices we compute the
      signed distance to the corresponding vertex. Otherwise we compute the
      projection to the edge and compute the sign from the normal vector.
    */
    inline T
    signedDistance(const Vec3& a_x0) const noexcept;

    /*!
      @brief Get the signed distance to this half edge
      @details This routine will check if the input point projects to the edge or
      one of the vertices. If it projectes to one of the vertices we compute the
      squared distance to the corresponding vertex. Otherwise we compute the
      squared distance of the projection to the edge. This is faster than
      signedDistance()
    */
    inline T
    unsignedDistance2(const Vec3& a_x0) const noexcept;

  protected:
    /*!
      @brief Half-edge normal vector
      @details Computed in computeNormal which sets the normal vector to be the
      average of the normal vector of the connected faces
    */
    Vec3 m_normal;

    /*!
      @brief Vector from the starting vertex to the end vertex. Exists for
      performance reasons.
    */
    Vec3 m_x2x1;

    /*!
      @brief Squared inverse edge length. Exists for performance reasons.
    */
    T m_invLen2;

    /*!
      @brief Starting vertex
    */
    VertexPtr m_vertex;

    /*!
      @brief Pair edge
    */
    EdgePtr m_pairEdge;

    /*!
      @brief Previous edge
    */
    EdgePtr m_previousEdge;

    /*!
      @brief Next edge
    */
    EdgePtr m_nextEdge;

    /*!
      @brief Enclosing polygon face
    */
    FacePtr m_face;

    /*!
      @brief Returns the "projection" of a point to an edge.
      @details This function parametrizes the edge as x(t) = x0 + (x1-x0)*t and
      returns where on the this edge the point a_x0 projects. If projects onto the
      edge if t = [0,1] and to one of the start/end vertices otherwise.
    */
    inline T
    projectPointToEdge(const Vec3& a_x0) const noexcept;

    /*!
      @brief Normalize the normal vector, ensuring it has length 1
    */
    inline void
    normalizeNormalVector() noexcept;

    /*!
      @brief Compute the edge length.
      @details This computes the vector m_x2x1 (vector from starting vertex to end
      vertex) and the inverse length squared.
    */
    inline void
    computeEdgeLength() noexcept;

    /*!
      @brief Compute normal vector as average of face normals
    */
    inline void
    computeNormal() noexcept;
  };
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_DCEL_EdgeImplem.hpp"

#endif
