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
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_PLY.hpp"
#include "EBGeometry_STL.hpp"
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

#warning "STL and PLY should both have functionality for converting into DCEL"
#warning "First thing we should do is to remove the STL namespace and put everything in the readSTL file"
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
    @brief Get file type
    @param[in] a_filename File name
  */
  inline static Parser::FileType
  getFileType(const std::string a_filename) noexcept;

  /*!
    @brief Get file encoding. 
    @param[in] a_filename File name    
  */
  inline static Parser::Encoding
  getFileEncoding(const std::string a_filename) noexcept;

  /*!
    @brief Read a single PLY file
    @param[in] a_filename PLY file name.
  */
  template <typename T>
  PLY<T>
  readPLY(const std::string& a_filename) noexcept;

  /*!
    @brief Read multiple PLY files. 
    @param[in] a_filenames PLY file names.
  */
  template <typename T>
  std::vector<PLY<T>>
  readPLY(const std::vector<std::string>& a_filenames) noexcept;

  /*!
    @brief Read a single STL file
    @param[in] a_filename STL file name.
    @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
    will only read the first one. 
  */
  template <typename T>
  STL2<T>
  readSTL(const std::string& a_filename) noexcept;

  /*!
    @brief Read multiple STL files. 
    @param[in] a_filenames STL file names.
    @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
    will only read the first one.     
  */
  template <typename T>
  std::vector<STL2<T>>
  readSTL(const std::vector<std::string>& a_filenames) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh
    @param[in] a_filename File name
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  inline static std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  readIntoDCEL(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes
    @param[in] a_files File names
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  inline static std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>>
  readIntoDCEL(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as an implicit function.
    @param[in] a_filename File name
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  inline static std::shared_ptr<MeshSDF<T, Meta>>
  readIntoMesh(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as an implicit functions.
    @param[in] a_files File names
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  inline static std::vector<std::shared_ptr<MeshSDF<T, Meta>>>
  readIntoMesh(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh enclosed in a full BVH.
    @param[in] a_filename File name
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  inline static std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>
  readIntoFullBVH(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes enclosed in BVHs.
    @param[in] a_files File names
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  inline static std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>>
  readIntoFullBVH(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh enclosed in a linearized BVH
    @param[in] a_filename File name
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  inline static std::shared_ptr<FastCompactMeshSDF<T, Meta, BV, K>>
  readIntoLinearBVH(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes enclosed in linearized BVHs.
    @param[in] a_files File names
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  inline static std::vector<std::shared_ptr<FastCompactMeshSDF<T, Meta, BV, K>>>
  readIntoLinearBVH(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as a DCEL mesh enclosed in a full BVH.
    @param[in] a_filename File name
  */
  template <typename T, typename Meta, typename BV = EBGeometry::BoundingVolumes::AABBT<T>, size_t K = 4>
  inline static std::shared_ptr<FastTriMeshSDF<T, Meta, BV, K>>
  readIntoTriangleBVH(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes enclosed in BVHs.
    @param[in] a_files File names
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  inline static std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>>
  readIntoTriangleBVH(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Read a file containing a single watertight object and return it as an implicit function.
    @param[in] a_filename File name
  */
  template <typename T, typename Meta>
  inline static std::vector<std::shared_ptr<Triangle<T, Meta>>>
  readIntoTriangles(const std::string a_filename) noexcept;

  /*!
    @brief Read multiple files containing single watertight objects and return them as an implicit functions.
    @param[in] a_files File names
  */
  template <typename T, typename Meta>
  inline static std::vector<std::vector<std::shared_ptr<Triangle<T, Meta>>>>
  readIntoTriangles(const std::vector<std::string> a_files) noexcept;

  /*!
    @brief Class for reading STL files.
    @note T is the precision used when storing the mesh. 
  */
  template <typename T, typename Meta>
  class STL
  {
  public:
    /*!
      @brief Alias for vector type
    */
    using Vec3 = EBGeometry::Vec3T<T>;

    /*!
      @brief Alias for vertex type
    */
    using Vertex = EBGeometry::DCEL::VertexT<T, Meta>;

    /*!
      @brief Alias for edge type
    */
    using Edge = EBGeometry::DCEL::EdgeT<T, Meta>;

    /*!
      @brief Alias for face type
    */
    using Face = EBGeometry::DCEL::FaceT<T, Meta>;

    /*!
      @brief Alias for mesh type
    */
    using Mesh = EBGeometry::DCEL::MeshT<T, Meta>;

    /*!
      @brief Alias for edge iterator type
    */
    using EdgeIterator = EBGeometry::DCEL::EdgeIteratorT<T, Meta>;

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
      @param[out] a_fileContents File contents
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
    /*!
      @brief Alias for vector type
    */
    using Vec3 = EBGeometry::Vec3T<T>;

    /*!
      @brief Alias for vertex type
    */
    using Vertex = EBGeometry::DCEL::VertexT<T>;

    /*!
      @brief Alias for edge type
    */
    using Edge = EBGeometry::DCEL::EdgeT<T>;

    /*!
      @brief Alias for face type
    */
    using Face = EBGeometry::DCEL::FaceT<T>;

    /*!
      @brief Alias for mesh type
    */
    using Mesh = EBGeometry::DCEL::MeshT<T>;

    /*!
      @brief Alias for edge iterator type
    */
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
      @brief Read an ASCII PLY file into a triangle soup. 
      @details 
      @param[out]   a_vertices   Raw vertices
      @param[out]   a_faces      Raw polygon faces
      @param[in]    a_fileStream File stream
    */
    inline static void
    readPLYSoupASCII(std::vector<Vec3>&                a_vertices,
                     std::vector<std::vector<size_t>>& a_faces,
                     std::ifstream&                    a_fileStream) noexcept;

    /*!
      @brief Read a binary PLY file into a triangle soup. 
      @details 
      @param[out]   a_vertices   Raw vertices
      @param[out]   a_faces      Raw polygon faces
      @param[in]    a_fileStream File stream
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
