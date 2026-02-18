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
#include "EBGeometry_VTK.hpp"
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
    VTK,
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
  STL<T>
  readSTL(const std::string& a_filename) noexcept;

  /*!
    @brief Read multiple STL files.
    @param[in] a_filenames STL file names.
    @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
    will only read the first one.
  */
  template <typename T>
  std::vector<STL<T>>
  readSTL(const std::vector<std::string>& a_filenames) noexcept;

  /*!
    @brief Read a single VTK file
    @param[in] a_filename VTK file name.
  */
  template <typename T>
  VTK<T>
  readVTK(const std::string& a_filename) noexcept;

  /*!
    @brief Read multiple VTK files.
    @param[in] a_filenames VTK file names.
  */
  template <typename T>
  std::vector<VTK<T>>
  readVTK(const std::vector<std::string>& a_filenames) noexcept;

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
} // namespace Parser

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ParserImplem.hpp"

#endif
