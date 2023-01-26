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
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

/*!
  @brief Namespace which encapsulates possible file parsers for building
  EBGeometry data structures.
*/
namespace Parser {

  /*!
    @brief Simple enum for separating ASCII and binary files
  */
  enum class Encoding
  {
    ASCII,
    Binary,
    Unknown
  };

  /*!
    @brief Various supported file types
  */
  enum class FileType
  {
    STL,
    PLY,
    Unsupported
  };

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh
    @param[in] a_filename File name
  */
  template <typename T>
  inline static std::shared_ptr<EBGeometry::DCEL::MeshT<T>>
  readIntoDCEL(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes
    @param[in] a_files File names
  */
  template <typename T>
  inline static std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T>>>
  readIntoDCEL(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as an implicit function.
    @param[in] a_filename File name
  */
  template <typename T>
  inline static std::shared_ptr<MeshSDF<T>>
  readIntoMesh(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as an implicit functions.
    @param[in] a_files File names
  */
  template <typename T>
  inline static std::vector<std::shared_ptr<MeshSDF<T>>>
  readIntoMesh(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh enclosed in a full BVH.
    @param[in] a_filename File name
  */
  template <typename T, typename BV = EBGeometry::BoundingVolumes::AABBT<T>, size_t K = 4>
  inline static std::shared_ptr<FastMeshSDF<T, BV, K>>
  readIntoFullBVH(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes enclosed in BVHs.
    @param[in] a_files File names
  */
  template <typename T, typename BV = EBGeometry::BoundingVolumes::AABBT<T>, size_t K = 4>
  inline static std::vector<std::shared_ptr<FastMeshSDF<T, BV, K>>>
  readIntoFullBVH(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh enclosed in a linearized BVH
    @param[in] a_filename File name
  */
  template <typename T, typename BV = EBGeometry::BoundingVolumes::AABBT<T>, size_t K = 4>
  inline static std::shared_ptr<FastCompactMeshSDF<T, BV, K>>
  readIntoLinearBVH(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes enclosed in linearized BVHs.
    @param[in] a_files File names
  */
  template <typename T, typename BV = EBGeometry::BoundingVolumes::AABBT<T>, size_t K = 4>
  inline static std::vector<std::shared_ptr<FastCompactMeshSDF<T, BV, K>>>
  readIntoLinearBVH(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Get file type
    @param[in] a_filenames 
  */
  inline static Parser::FileType
  getFileType(const std::string a_filename) noexcept;

  /*!
    @brief Check if triangle soup contains degenerate polygons
    @param[out] a_vertices Vertices
    @param[out] a_facets   Facets
  */
  template <typename T>
  inline static bool
  containsDegeneratePolygons(const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
                             const std::vector<std::vector<size_t>>&  a_facets) noexcept;

  /*!
    @brief Compress triangle soup (removes duplicate vertices)
    @param[out] a_vertices   Vertices
    @param[out] a_facets     STL facets
  */
  template <typename T>
  inline static void
  compress(std::vector<EBGeometry::Vec3T<T>>& a_vertices, std::vector<std::vector<size_t>>& a_facets) noexcept;

  /*!
    @brief Turn raw vertices into DCEL vertices. Does not include vertex normal vectors. 
    @param[out] a_verticesDCEL DCEL vertices
    @param[in]  a_verticesRaw  Raw vertices
  */
  template <typename T>
  inline static void
  soupToDCEL(EBGeometry::DCEL::MeshT<T>&              a_mesh,
             const std::vector<EBGeometry::Vec3T<T>>& a_vertices,
             const std::vector<std::vector<size_t>>&  a_facets) noexcept;

  /*!
    @brief Reconcile pair edges, i.e. find the pair edge for every constructed
    half-edge
    @param[in,out] a_edges Half edges.
  */
  template <typename T>
  inline static void
  reconcilePairEdgesDCEL(std::vector<std::shared_ptr<EBGeometry::DCEL::EdgeT<T>>>& a_edges) noexcept;

  /*!
    @brief Class for reading STL files.
    @note T is the precision used when storing the mesh. 
  */
  template <typename T>
  class STL
  {
  public:
    using Vec3         = EBGeometry::Vec3T<T>;
    using Vertex       = EBGeometry::DCEL::VertexT<T>;
    using Edge         = EBGeometry::DCEL::EdgeT<T>;
    using Face         = EBGeometry::DCEL::FaceT<T>;
    using Mesh         = EBGeometry::DCEL::MeshT<T>;
    using EdgeIterator = EBGeometry::DCEL::EdgeIteratorT<T>;

    /*!
      @brief Read a single STL object from the input file. The file can be binary or ASCII. 
      If the STL file contains multiple solids, this routine returns the first one. 
      @param[in] a_filename STL file name. 
    */
    inline static std::shared_ptr<Mesh>
    readSingle(const std::string a_filename) noexcept;

    /*!
      @brief Read a single STL object from the input file. The file can be binary or ASCII. 
      @param[in] a_filename STL file name. 
    */
    inline static std::vector<std::pair<std::shared_ptr<Mesh>, std::string>>
    readMulti(const std::string a_filename) noexcept;

  protected:
    /*!
      @brief Check if the input STL file is an ASCII file or a binary
      @param[in] a_filename File name
      @return Returns Encoding::ASCII or Encoding::Binary,
    */
    inline static Encoding
    getEncoding(const std::string a_filename) noexcept;

    /*!
      @brief ASCII reader STL files, possibly containing multiple objects. Each object becomes a DCEL mesh
      @param[in] a_filename Input filename
    */
    inline static std::vector<std::pair<std::shared_ptr<Mesh>, std::string>>
    readASCII(const std::string a_filename) noexcept;

    /*!
      @brief Binary reader for STL files, possibly containing multiple objects. Each object becomes a DCEL mesh
      @param[in] a_filename Input filename
    */
    inline static std::vector<std::pair<std::shared_ptr<Mesh>, std::string>>
    readBinary(const std::string a_filename) noexcept;

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
    readSTLSoupASCII(std::vector<Vec3>&                a_vertices,
                     std::vector<std::vector<size_t>>& a_facets,
                     std::string&                      a_objectName,
                     const std::vector<std::string>&   a_fileContents,
                     const size_t                      a_firstLine,
                     const size_t                      a_lastLine) noexcept;
  };

  /*!
    @brief Class for reading Stanford PLY files.
    @note T is the precision used for storing the mesh.
  */
  template <typename T>
  class PLY
  {
  public:
    using Vec3         = EBGeometry::Vec3T<T>;
    using Vertex       = EBGeometry::DCEL::VertexT<T>;
    using Edge         = EBGeometry::DCEL::EdgeT<T>;
    using Face         = EBGeometry::DCEL::FaceT<T>;
    using Mesh         = EBGeometry::DCEL::MeshT<T>;
    using EdgeIterator = EBGeometry::DCEL::EdgeIteratorT<T>;

    /*!
      @brief Static function which reads an ASCII .ply file and returns a DCEL
      mesh.
      @param[in]  a_filename File name
    */
    inline static std::shared_ptr<Mesh>
    read(const std::string a_filename) noexcept;

  protected:
    /*!
      @brief Check if the input PLY file is an ASCII file or a binary
      @param[in] a_filename File name
      @return Returns Encoding::ASCII or Encoding::Binary,
    */
    inline static Encoding
    getEncoding(const std::string a_filename) noexcept;

    /*!
      @brief Read an ASCII PLY file into a triangle soup. 
      @details 
      @param[out]   a_vertices Raw vertices
      @param[out]   a_faces    Raw polygon faces
      @param[in]    a_filename File name
    */
    inline static void
    readPLYSoupASCII(std::vector<Vec3>&                a_vertices,
                     std::vector<std::vector<size_t>>& a_faces,
                     std::ifstream&                    a_fileStream) noexcept;

    /*!
      @brief Read a binary PLY file into a triangle soup. 
      @details 
      @param[out]   a_vertices Raw vertices
      @param[out]   a_faces    Raw polygon faces
      @param[in]    a_filename File name
    */
    inline static void
    readPLYSoupBinary(std::vector<Vec3>&                a_vertices,
                      std::vector<std::vector<size_t>>& a_faces,
                      std::ifstream&                    a_fileStream) noexcept;
  };
} // namespace Parser

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ParserImplem.hpp"

#endif
