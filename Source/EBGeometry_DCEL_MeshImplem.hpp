/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_DCEL_MeshImplem.hpp
  @brief  Implementation of EBGeometry_DCEL_Mesh.hpp
  @author Robert Marskar
*/

#ifndef EBGeometry_DCEL_MeshImplem
#define EBGeometry_DCEL_MeshImplem

// Our includes
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_DCEL_Iterator.hpp"
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace DCEL {

  template <class T>
  inline MeshT<T>::MeshT()
  {
    m_algorithm = SearchAlgorithm::Direct2;
  }

  template <class T>
  inline MeshT<T>::MeshT(std::vector<FacePtr>&   a_faces,
                         std::vector<EdgePtr>&   a_edges,
                         std::vector<VertexPtr>& a_vertices)
    : MeshT()
  {
    this->define(a_faces, a_edges, a_vertices);
  }

  template <class T>
  inline MeshT<T>::~MeshT()
  {}

  template <class T>
  inline void
  MeshT<T>::define(std::vector<FacePtr>&   a_faces,
                   std::vector<EdgePtr>&   a_edges,
                   std::vector<VertexPtr>& a_vertices) noexcept
  {
    m_faces    = a_faces;
    m_edges    = a_edges;
    m_vertices = a_vertices;
  }

  template <class T>
  inline void
  MeshT<T>::incrementWarning(std::map<std::string, size_t>& a_warnings, const std::string& a_warn) const noexcept
  {
    a_warnings.at(a_warn) += 1;
  }

  template <class T>
  inline void
  MeshT<T>::printWarnings(const std::map<std::string, size_t>& a_warnings) const noexcept
  {
    for (const auto& warn : a_warnings) {
      if (warn.second > 0) {
        std::cerr << "In file 'CD_DCELMeshImplem.H' function "
	  "MeshT<T>::sanityCheck() - warnings about error '"
                  << warn.first << "' = " << warn.second << "\n";
      }
    }
  }

  template <class T>
  inline void
  MeshT<T>::sanityCheck() const noexcept
  {

    const std::string f_null       = "nullptr face";
    const std::string f_noEdge     = "face with no edge";
    const std::string f_degenerate = "degenerate face";

    const std::string e_null       = "nullptr edges";
    const std::string e_degenerate = "degenerate edge";
    const std::string e_noPairEdge = "no pair edge (not watertight)";
    const std::string e_noNextEdge = "no next edge (badly linked dcel)";
    const std::string e_noPrevEdge = "no previous edge (badly linked dcel)";
    const std::string e_noOrigVert = "no origin vertex found for half edge (badly linked dcel)";
    const std::string e_noFace     = "no face found for half edge (badly linked dcel)";
    const std::string e_noPrevNext = "previous edge's next edge is not this edge (badly linked dcel)";
    const std::string e_noNextPrev = "next edge's previous edge is not this edge (badly linked dcel)";

    const std::string v_null   = "nullptr vertex";
    const std::string v_noEdge = "no referenced edge for vertex (unreferenced vertex)";

    std::map<std::string, size_t> warnings = {{f_null, 0},
                                              {f_noEdge, 0},
                                              {f_degenerate, 0},
                                              {e_null, 0},
                                              {e_degenerate, 0},
                                              {e_noPairEdge, 0},
                                              {e_noNextEdge, 0},
                                              {e_noPrevEdge, 0},
                                              {e_noOrigVert, 0},
                                              {e_noFace, 0},
                                              {e_noPrevNext, 0},
                                              {e_noNextPrev, 0},
                                              {v_null, 0},
                                              {v_noEdge, 0}};

    for (const auto& f : m_faces) {
      const auto& halfEdge = f->getHalfEdge();

      // Check for duplicate vertices
      auto vertices = f->gatherVertices();
      std::sort(vertices.begin(), vertices.end());
      auto       it           = std::unique(vertices.begin(), vertices.end());
      const bool noDuplicates = (it == vertices.end());

      if (f == nullptr) {
        incrementWarning(warnings, f_null);
      }
      else if (halfEdge == nullptr) {
        incrementWarning(warnings, f_noEdge);
      }
      if (!noDuplicates) {
        incrementWarning(warnings, f_degenerate);
      }
    }

    for (const auto& e : m_edges) {
      const auto& nextEdge  = e->getNextEdge();
      const auto& prevEdge  = e->getPreviousEdge();
      const auto& pairEdge  = e->getPairEdge();
      const auto& curVertex = e->getVertex();
      const auto& curFace   = e->getFace();

      // Check basic points for current edge.
      if (e == nullptr) {
        incrementWarning(warnings, e_null);
      }
      else if (e->getVertex() == e->getOtherVertex()) {
        incrementWarning(warnings, e_degenerate);
      }
      else if (pairEdge == nullptr) {
        incrementWarning(warnings, e_noPairEdge);
      }
      else if (nextEdge == nullptr) {
        incrementWarning(warnings, e_noNextEdge);
      }
      else if (prevEdge == nullptr) {
        incrementWarning(warnings, e_noPrevEdge);
      }
      else if (curVertex == nullptr) {
        incrementWarning(warnings, e_noOrigVert);
      }
      else if (curFace == nullptr) {
        incrementWarning(warnings, e_noFace);
      }

      // Check that the next edge's previous edge is this edge.
      if (prevEdge->getNextEdge() != e) {
        incrementWarning(warnings, e_noPrevNext);
      }
      else if (nextEdge->getPreviousEdge() != e) {
        incrementWarning(warnings, e_noNextPrev);
      }
    }

    // Vertex check
    for (const auto& v : m_vertices) {
      if (v == nullptr) {
        incrementWarning(warnings, v_null);
      }
      else if (v->getOutgoingEdge() == nullptr) {
        incrementWarning(warnings, v_noEdge);
      }
    }

    this->printWarnings(warnings);
  }

  template <class T>
  inline void
  MeshT<T>::setSearchAlgorithm(const SearchAlgorithm a_algorithm) noexcept
  {
    m_algorithm = a_algorithm;
  }

  template <class T>
  inline void
  MeshT<T>::setInsideOutsideAlgorithm(typename Polygon2D<T>::InsideOutsideAlgorithm a_algorithm) noexcept
  {
    for (auto& f : m_faces) {
      f->setInsideOutsideAlgorithm(a_algorithm);
    }
  }

  template <class T>
  inline void
  MeshT<T>::reconcile(typename DCEL::MeshT<T>::VertexNormalWeight a_weight) noexcept
  {
    this->reconcileFaces();
    this->reconcileEdges();
    this->reconcileVertices(a_weight);
  }

  template <class T>
  inline void
  MeshT<T>::reconcileFaces() noexcept
  {
    for (auto& f : m_faces) {
      f->reconcile();
    }
  }

  template <class T>
  inline void
  MeshT<T>::reconcileEdges() noexcept
  {
    for (auto& e : m_edges) {
      e->reconcile();
    }
  }

  template <class T>
  inline void
  MeshT<T>::reconcileVertices(typename DCEL::MeshT<T>::VertexNormalWeight a_weight) noexcept
  {
    for (auto& v : m_vertices) {
      switch (a_weight) {
      case VertexNormalWeight::None:
        v->computeVertexNormalAverage();
        break;
      case VertexNormalWeight::Angle:
        v->computeVertexNormalAngleWeighted();
        break;
      default:
        std::cerr << "In file 'CD_DCELMeshImplem.H' function "
	  "DCEL::MeshT<T>::reconcileVertices(VertexNormalWeighting) - "
	  "unsupported algorithm requested\n";
      }
    }
  }

  template <class T>
  inline std::vector<std::shared_ptr<VertexT<T>>>&
  MeshT<T>::getVertices() noexcept
  {
    return (m_vertices);
  }

  template <class T>
  inline const std::vector<std::shared_ptr<VertexT<T>>>&
  MeshT<T>::getVertices() const noexcept
  {
    return (m_vertices);
  }

  template <class T>
  inline std::vector<std::shared_ptr<EdgeT<T>>>&
  MeshT<T>::getEdges() noexcept
  {
    return (m_edges);
  }

  template <class T>
  inline const std::vector<std::shared_ptr<EdgeT<T>>>&
  MeshT<T>::getEdges() const noexcept
  {
    return (m_edges);
  }

  template <class T>
  inline std::vector<std::shared_ptr<FaceT<T>>>&
  MeshT<T>::getFaces() noexcept
  {
    return (m_faces);
  }

  template <class T>
  inline const std::vector<std::shared_ptr<FaceT<T>>>&
  MeshT<T>::getFaces() const noexcept
  {
    return (m_faces);
  }

  template <class T>
  inline std::vector<Vec3T<T>>
  MeshT<T>::getAllVertexCoordinates() const noexcept
  {
    std::vector<Vec3> vertexCoordinates;
    for (const auto& v : m_vertices) {
      vertexCoordinates.emplace_back(v->getPosition());
    }

    return vertexCoordinates;
  }

  template <class T>
  inline T
  MeshT<T>::signedDistance(const Vec3& a_point) const noexcept
  {
    return this->signedDistance(a_point, m_algorithm);
  }

  template <class T>
  inline T
  MeshT<T>::unsignedDistance2(const Vec3& a_point) const noexcept
  {
    T minDist2 = std::numeric_limits<T>::max();

    for (const auto& f : m_faces) {
      const T curDist2 = f->unsignedDistance2(a_point);

      minDist2 = std::min(minDist2, curDist2);
    }

    return minDist2;
  }

  template <class T>
  inline T
  MeshT<T>::signedDistance(const Vec3& a_point, SearchAlgorithm a_algorithm) const noexcept
  {
    T minDist = std::numeric_limits<T>::max();

    switch (a_algorithm) {
    case SearchAlgorithm::Direct: {
      minDist = this->DirectSignedDistance(a_point);

      break;
    }
    case SearchAlgorithm::Direct2: {
      minDist = this->DirectSignedDistance2(a_point);

      break;
    }
    default: {
      std::cerr << "Error in file 'CD_DCELMeshImplem.H' MeshT<T>::signedDistance "
	"unsupported algorithm requested\n";

      break;
    }
    }

    return minDist;
  }

  template <class T>
  inline T
  MeshT<T>::DirectSignedDistance(const Vec3& a_point) const noexcept
  {
    T minDist  = m_faces.front()->signedDistance(a_point);
    T minDist2 = minDist * minDist;

    for (const auto& f : m_faces) {
      const T curDist  = f->signedDistance(a_point);
      const T curDist2 = curDist * curDist;

      if (curDist2 < minDist2) {
        minDist  = curDist;
        minDist2 = curDist2;
      }
    }

    return minDist;
  }

  template <class T>
  inline T
  MeshT<T>::DirectSignedDistance2(const Vec3& a_point) const noexcept
  {
    FacePtr closest  = m_faces.front();
    T       minDist2 = closest->unsignedDistance2(a_point);

    for (const auto& f : m_faces) {
      const T curDist2 = f->unsignedDistance2(a_point);

      if (curDist2 < minDist2) {
        closest  = f;
        minDist2 = curDist2;
      }
    }

    return closest->signedDistance(a_point);
  }
} // namespace DCEL

#include "EBGeometry_NamespaceFooter.hpp"

#endif
