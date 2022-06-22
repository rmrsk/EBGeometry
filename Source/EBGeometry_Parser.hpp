/* EBGeometry
 * Copyright © 2022 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_Parser.hpp
  @brief  Declaration of utilities for reading files into EBGeometry data
  structures.
  @author Robert Marskar
*/

#ifndef EBGeometry_Parser
#define EBGeometry_Parser

// Std includes
#include <vector>
#include <memory>
#include <map>

// Our includes
#include "EBGeometry_DCEL_Vertex.hpp"
#include "EBGeometry_DCEL_Edge.hpp"
#include "EBGeometry_DCEL_Face.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace which encapsulates possible file parsers for building
  EBGeometry data structures.
*/
namespace Parser {

  /*!
    @brief Compress triangle soup (removes duplicate vertices)
    @param[out] a_vertices   Vertices
    @param[out] a_facets     STL facets
  */
  template <typename T>
  inline static void
  compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets);  

  /*!
    @brief Class for reading STL files.
    @note T is the precision used when storing the mesh. 
  */
  template <typename T>
  class STL
  {
  public:
    using Vec3         = Vec3T<T>;
    using Vertex       = DCEL::VertexT<T>;
    using Edge         = DCEL::EdgeT<T>;
    using Face         = DCEL::FaceT<T>;
    using Mesh         = DCEL::MeshT<T>;
    using EdgeIterator = DCEL::EdgeIteratorT<T>;

    /*!
      @brief ASCII reader for reading an STL file, possibly containing multiple objects. Each object becomes a DCEL mesh
      @param[in] a_filename Input filename
    */
    inline static std::map<std::shared_ptr<Mesh>, std::string>
    readASCII(const std::string a_filename);

  protected:
    /*!
      @brief Read an STL object as a triangle soup into a raw vertices and facets
      @param[out] a_vertices   Vertices
      @param[out] a_facets     STL facets
      @param[out] a_objectName Object name
      @param[out] a_filename   File name 
      @param[out] a_firstLine  Line number in a_filename containing the 'solid' identifier. 
      @param[out] a_lastLine   Line number in a_filename containing the 'endsolid' identifier. 
    */
    inline static void
    readTriangleSoupASCII(std::vector<Vec3>&                a_vertices,
                          std::vector<std::vector<size_t>>& a_facets,
                          std::string&                      a_objectName,
                          const std::vector<std::string>&   a_fileContents,
                          const size_t                      a_firstLine,
                          const size_t                      a_lastLine);


  };

  /*!
    @brief Class for reading Stanford PLY files.
    @note T is the precision used for storing the mesh.
  */
  template <typename T>
  class PLY
  {
  public:
    using Vertex       = DCEL::VertexT<T>;
    using Edge         = DCEL::EdgeT<T>;
    using Face         = DCEL::FaceT<T>;
    using Mesh         = DCEL::MeshT<T>;
    using EdgeIterator = DCEL::EdgeIteratorT<T>;

    /*!
      @brief Static function which reads an ASCII .ply file and returns a DCEL
      mesh.
      @param[in]  a_filename File name
    */
    inline static std::shared_ptr<Mesh>
    readIntoDCEL(const std::string a_filename);

    /*!
      @brief Static function which reads an ASCII .ply file and puts it in a mesh.
      @param[out] a_mesh     DCEL mesh
      @param[in]  a_filename File name
    */
    inline static void
    readIntoDCEL(Mesh& a_mesh, const std::string a_filename);

  protected:
    /*!
      @brief Read an ASCII header
      @details This reads the number of vertices and faces in the PLY file. Note
      that it only reads the header.
      @param[out]   a_numVertices Number of vertices
      @param[out]   a_numFaces    Number of faces
      @param[in,out] a_inputStream File stream. On output, the filestream is at the
      end of the PLY header.
    */
    inline static void
    readHeaderASCII(size_t& a_numVertices, size_t& a_numFaces, std::ifstream& a_inputStream);

    /*!
      @brief Read ASCII vertices.
      @param[out] a_vertices DCEL vertices. These are constructed in this routine.
      @param[in]  a_numVertices Number of vertices to read
      @param[in]  a_inputStream Input file stream.
      @note The next getline() from a_inputStream must read the first vertex (i.e.
      don't rewind the stream before entering this routine)
    */
    inline static void
    readVerticesIntoDCEL(std::vector<std::shared_ptr<Vertex>>& a_vertices,
                         const size_t                          a_numVertices,
                         std::ifstream&                        a_inputStream);

    /*!
      @brief Read ASCII faces and create mesh connectivity.
      @param[out]   a_faces       DCEL faces. Constructured in this routine.
      @param[out]   a_edges       DCEL edges. Constructured in this routine.
      @param[out]   a_vertices    DCEL edges. Constructured in
      readVerticesIntoDCEL.
      @param[in]    a_numFaces    Total number of faces in mesh.
      @param[in,out] a_inputStream Input stream
      @note The next getline() from inputStream must read the first face, i.e. we
      assume that read_ascii_vertices was called IMMEDIATELY before this function.
      That function will center the fstream on the correct line in the input file.
    */
    inline static void
    readFacesIntoDCEL(std::vector<std::shared_ptr<Face>>&         a_faces,
                      std::vector<std::shared_ptr<Edge>>&         a_edges,
                      const std::vector<std::shared_ptr<Vertex>>& a_vertices,
                      const size_t                                a_numFaces,
                      std::ifstream&                              a_inputStream);

    /*!
      @brief Reconcile pair edges, i.e. find the pair edge for every constructed
      half-edge
      @param[in,out] a_edges Half edges.
    */
    inline static void
    reconcilePairEdgesDCEL(std::vector<std::shared_ptr<Edge>>& a_edges);
  };
} // namespace Parser

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ParserImplem.hpp"

#endif
