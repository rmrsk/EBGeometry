/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_Vertex.hpp
  @brief  Declaration of a vertex class for use in DCEL descriptions of polygon tesselations. 
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_Vertex
#define EBGeometry_DCEL_Vertex

// Std includes
#include <vector>
#include <memory>

// Our includes
#include "EBGeometry_Vec.hpp"
#include "EBGeometry_DCEL.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  // Forward declare classes. 
  template <class T> class VertexT;
  template <class T> class EdgeT;
  template <class T> class FaceT;
  template <class T> class EdgeIteratorT;

  /*!
    @brief Class which represents a vertex node in a double-edge connected list (DCEL). 
    @details This class is used in DCEL functionality which stores polygonal surfaces in a mesh. The VertexT class
    has a position, a normal vector, and a pointer to one of the outgoing edges from the vertex. For performance
    reasons we also include pointers to all the polygon faces that share this vertex. 
    @note The normal vector is outgoing, i.e. a point x is "outside" the vertex if the dot product between n and
    (x - x0) is positive. 
  */
  template <class T>
  class VertexT {
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
      @brief Alias to cut down on typing. Note that this is std::shared_ptr<VertexT<T> >
    */    
    using VertexPtr = std::shared_ptr<Vertex>;

    /*!
      @brief Alias to cut down on typing. Note that this is std::shared_ptr<EdgeT<T> >
    */    
    using EdgePtr = std::shared_ptr<Edge>;

    /*!
      @brief Alias to cut down on typing. Note that this is std::shared_ptr<FaceT<T> >
    */    
    using FacePtr = std::shared_ptr<Face>;

    /*!
      @brief Alias to cut down on typing. Note that this is std::shared_ptr<EdgeIteratorT<T> >
    */        
    using EdgeIterator = EdgeIteratorT<T>;

    /*!
      @brief Default constructor. 
      @details This initializes the position and the normal vector to zero vectors, and the polygon face list is empty
    */
    VertexT();

    /*!
      @brief Partial constructor. 
      @param[in] a_position Vertex position
      @details This initializes the position to a_position and the normal vector to the zero vector. The polygon face list is empty.
    */
    VertexT(const Vec3& a_position);

    /*!
      @brief Constructor. 
      @param[in] a_position    Vertex position
      @param[in] a_normal Vertex normal vector
      @details This initializes the position to a_position and the normal vector to a_normal. The polygon face list is empty.
    */
    VertexT(const Vec3& a_position, const Vec3& a_normal);

    /*!
      @brief Full copy constructor
      @param[in] a_otherVertex Other vertex
      @details This copies the position, normal vector, and outgoing edge pointer from the other vertex. The polygon face list. 
    */
    VertexT(const Vertex& a_otherVertex);

    /*!
      @brief Destructor (does nothing)
    */
    ~VertexT();

    /*!
      @brief Define function
      @param[in] a_position    Vertex position
      @param[in] a_edge   Pointer to outgoing edge
      @param[in] a_normal Vertex normal vector
      @details This sets the position, normal vector, and edge pointer.
    */
    inline
    void define(const Vec3& a_position, const EdgePtr& a_edge, const Vec3& a_normal) noexcept;

    /*!
      @brief Set the vertex position
      @param[in] a_position Vertex position
    */
    inline
    void setPosition(const Vec3& a_position) noexcept;

    /*!
      @brief Set the vertex normal vector
      @param[in] a_normal Vertex normal vector
    */
    inline
    void setNormal(const Vec3& a_normal) noexcept;

    /*!
      @brief Set the reference to the outgoing edge
      @param[in] a_edge Pointer to an outgoing edge
    */
    inline
    void setEdge(const EdgePtr& a_edge) noexcept;

    /*!
      @brief Add a face to the polygon face list. 
      @param[in] a_face Pointer to face. 
    */
    inline
    void addFace(const FacePtr& a_face) noexcept;

    /*!
      @brief Normalize the normal vector, ensuring its length is 1
    */
    inline
    void normalizeNormalVector() noexcept;

    /*!
      @brief Compute the vertex normal, using an average the normal vector in this vertex's face list (m_faces)
    */
    inline
    void computeVertexNormalAverage() noexcept;

    /*!
      @brief Compute the vertex normal, using an average of the normal vectors in the input face list
      @param[in] a_faces Faces
      @note This computes the vertex normal as n = sum(normal(face))/num(faces)
    */
    inline
    void computeVertexNormalAverage(const std::vector<FacePtr>& a_faces) noexcept;

    /*!
      @brief Compute the vertex normal, using the pseudonormal algorithm which weights the normal with the subtended angle to each connected face. 
      @details This calls the other version with a_faces = m_faces
      @note This computes the normal vector using the pseudnormal algorithm from Baerentzen and Aanes in 
      "Signed distance computation using the angle weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49)
    */
    inline
    void computeVertexNormalAngleWeighted() noexcept;

    /*!
      @brief Compute the vertex normal, using the pseudonormal algorithm which weights the normal with the subtended angle to each connected face. 
      @param[in] a_faces Faces to use for computation. 
      @note This computes the normal vector using the pseudnormal algorithm from Baerentzen and Aanes in 
      "Signed distance computation using the angle weighted pseudonormal" (DOI: 10.1109/TVCG.2005.49)
    */
    inline
    void computeVertexNormalAngleWeighted(const std::vector<FacePtr>& a_faces) noexcept;

    /*!
      @brief Return modifiable vertex position.
    */
    inline
    Vec3T<T>& getPosition() noexcept;

    /*!
      @brief Return immutable vertex position.
    */
    inline
    const Vec3T<T>& getPosition() const noexcept;

    /*!
      @brief Return modifiable vertex normal vector.
    */
    inline
    Vec3T<T>& getNormal() noexcept;

    /*!
      @brief Return immutable vertex normal vector.
    */
    inline
    const Vec3T<T>& getNormal() const noexcept;

    /*!
      @brief Return modifiable pointer to outgoing edge.
    */
    inline
    EdgePtr& getOutgoingEdge() noexcept;

    /*!
      @brief Return immutable pointer to outgoing edge.
    */
    inline
    const EdgePtr& getOutgoingEdge() const noexcept;

    /*!
      @brief Get modifiable polygon face list for this vertex. 
    */
    inline
    std::vector<FacePtr>& getFaces() noexcept;

    /*!
      @brief Get immutable polygon face list for this vertex. 
    */
    inline
    const std::vector<FacePtr>& getFaces() const noexcept;

    /*!
      @brief Get the signed distance to this vertex
      @param[in] a_x0 Position in space. 
      @return The returned distance is |a_x0 - m_position| and the sign is given by the sign of m_normal * |a_x0 - m_position|. 
    */
    inline
    T signedDistance(const Vec3& a_x0) const noexcept;

    /*!
      @brief Get the squared unsigned distance to this vertex
      @details This is faster to compute than signedDistance, and might be preferred for some algorithms. 
      @return Returns the vector length of (a_x - m_position)
    */
    inline
    T unsignedDistance2(const Vec3& a_x0) const noexcept;

  protected:

    /*!
      @brief Pointer to an outgoing edge from this vertex. 
    */
    EdgePtr m_outgoingEdge;

    /*!
      @brief Vertex position
    */
    Vec3 m_position;

    /*!
      @brief Vertex normal vector
    */
    Vec3 m_normal;

    /*!
      @brief List of faces connected to this vertex (these must be "manually" added)
    */
    std::vector<FacePtr > m_faces;
  };
}

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_DCEL_VertexImplem.hpp"

#endif
