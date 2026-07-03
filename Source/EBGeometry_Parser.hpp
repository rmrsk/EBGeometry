// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
  @file   EBGeometry_Parser.hpp
  @brief  Declaration of utilities for reading files into EBGeometry data
  structures.
  @author Robert Marskar
*/

#ifndef EBGEOMETRY_PARSER_HPP
#define EBGEOMETRY_PARSER_HPP

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

/**
  @brief Namespace which encapsulates possible file parsers for building
  EBGeometry data structures.
*/
namespace Parser {

  /**
    @brief Simple enum for separating ASCII and binary files
  */
  enum class Encoding
  {
    ASCII,  ///< File is plain text
    Binary, ///< File is binary-encoded
    Unknown ///< Encoding could not be determined
  };

  /**
    @brief Various supported file types
  */
  enum class FileType
  {
    STL,        ///< Stereolithography format (.stl)
    PLY,        ///< Polygon File Format (.ply)
    VTK,        ///< VTK legacy or XML polydata format (.vtk)
    Unsupported ///< File type is not recognised
  };

  /**
    @brief Determine file type from the file extension.
    @param[in] a_filename File name.
    @return Detected FileType (STL, PLY, VTK, or Unsupported).
  */
  [[nodiscard]] inline static Parser::FileType
  getFileType(const std::string a_filename) noexcept;

  /**
    @brief Determine file encoding (ASCII or binary) by inspecting the file header.
    @param[in] a_filename File name.
    @return Detected Encoding (ASCII, Binary, or Unknown).
  */
  [[nodiscard]] inline static Parser::Encoding
  getFileEncoding(const std::string a_filename) noexcept;

  /**
    @brief Read a single PLY file into a raw PLY data structure.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filename PLY file name.
    @return Populated PLY<T> object containing vertex coordinates and face indices.
  */
  template <typename T>
  [[nodiscard]] PLY<T>
  readPLY(const std::string& a_filename) noexcept;

  /**
    @brief Read multiple PLY files into raw PLY data structures.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filenames List of PLY file names.
    @return Vector of populated PLY<T> objects, one per file.
  */
  template <typename T>
  [[nodiscard]] std::vector<PLY<T>>
  readPLY(const std::vector<std::string>& a_filenames) noexcept;

  /**
    @brief Read a single STL file into a raw STL data structure.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filename STL file name.
    @return Populated STL<T> object containing vertex coordinates and face indices.
    @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
    will only read the first one.
  */
  template <typename T>
  [[nodiscard]] STL<T>
  readSTL(const std::string& a_filename) noexcept;

  /**
    @brief Read multiple STL files into raw STL data structures.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filenames List of STL file names.
    @return Vector of populated STL<T> objects, one per file.
    @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
    will only read the first one.
  */
  template <typename T>
  [[nodiscard]] std::vector<STL<T>>
  readSTL(const std::vector<std::string>& a_filenames) noexcept;

  /**
    @brief Read a single VTK legacy polydata file into a raw VTK data structure.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filename VTK file name.
    @return Populated VTK<T> object containing vertex coordinates and face indices.
  */
  template <typename T>
  [[nodiscard]] VTK<T>
  readVTK(const std::string& a_filename) noexcept;

  /**
    @brief Read multiple VTK legacy polydata files into raw VTK data structures.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filenames List of VTK file names.
    @return Vector of populated VTK<T> objects, one per file.
  */
  template <typename T>
  [[nodiscard]] std::vector<VTK<T>>
  readVTK(const std::vector<std::string>& a_filenames) noexcept;

  /**
    @brief Read a file containing a single watertight object and return it as a DCEL mesh.
    @tparam T    Floating-point precision for vertex coordinates.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the constructed DCEL mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  readIntoDCEL(const std::string a_filename) noexcept;

  /**
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes.
    @tparam T    Floating-point precision for vertex coordinates.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to the constructed DCEL meshes, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>>
  readIntoDCEL(const std::vector<std::string> a_files) noexcept;

  /**
    @brief Read a file containing a single watertight object and return it as a bare DCEL signed-distance function.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the MeshSDF wrapping the parsed DCEL mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::shared_ptr<MeshSDF<T, Meta>>
  readIntoMesh(const std::string a_filename) noexcept;

  /**
    @brief Read multiple files containing single watertight objects and return them as bare DCEL signed-distance functions.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to MeshSDF objects, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::vector<std::shared_ptr<MeshSDF<T, Meta>>>
  readIntoMesh(const std::vector<std::string> a_files) noexcept;

  /**
    @brief Read a file containing a single watertight object and return it enclosed in a full (tree-based) BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam BV   Bounding volume type (e.g. AABBT<T>).
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the FastMeshSDF enclosing the mesh in a K-ary BVH.
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  [[nodiscard]] inline static std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>
  readIntoFullBVH(const std::string a_filename) noexcept;

  /**
    @brief Read multiple files and return each object enclosed in a full (tree-based) BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam BV   Bounding volume type (e.g. AABBT<T>).
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to FastMeshSDF objects, one per file.
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            typename BV   = EBGeometry::BoundingVolumes::AABBT<T>,
            size_t K      = 4>
  [[nodiscard]] inline static std::vector<std::shared_ptr<FastMeshSDF<T, Meta, BV, K>>>
  readIntoFullBVH(const std::vector<std::string> a_files) noexcept;

  /**
    @brief Read a file containing a single watertight object and return it enclosed in a linearised (packed) BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the FastCompactMeshSDF enclosing the mesh in a linearised K-ary BVH.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4>
  [[nodiscard]] inline static std::shared_ptr<FastCompactMeshSDF<T, Meta, K>>
  readIntoCompactBVH(const std::string a_filename) noexcept;

  /**
    @brief Read multiple files and return each object enclosed in a linearised (packed) BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to FastCompactMeshSDF objects, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4>
  [[nodiscard]] inline static std::vector<std::shared_ptr<FastCompactMeshSDF<T, Meta, K>>>
  readIntoCompactBVH(const std::vector<std::string> a_files) noexcept;

  /**
    @brief Read a file and return the mesh enclosed in a SIMD-optimised triangle BVH.
    @details Triangles are grouped into SoA bundles of W and packed into a linearised K-ary BVH.
    At query time the BVH uses SIMD intrinsics to evaluate W triangles per leaf visit.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type.
    @tparam K    BVH branching factor (number of children per internal node). Default 4.
    @tparam W    SIMD lane width: triangles per SoA group. Defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH
                 (8 with AVX, 4 with SSE4.1).
    @param[in] a_filename    File name (STL, PLY, or VTK).
    @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf. Should be a multiple of W;
                             defaults to 8 (one group for W=8, two groups for W=4).
    @return Shared pointer to the FastTriMeshSDF enclosing the mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4, size_t W = EBGEOMETRY_SOA_DEFAULT_WIDTH>
  [[nodiscard]] inline static std::shared_ptr<FastTriMeshSDF<T, Meta, K, W>>
  readIntoTriangleBVH(const std::string a_filename, const size_t a_maxLeafSize = 8U) noexcept;

  /**
    @brief Read multiple files and return each mesh enclosed in a SIMD-optimised triangle BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type.
    @tparam K    BVH branching factor (number of children per internal node). Default 4.
    @tparam W    SIMD lane width: triangles per SoA group. Defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH.
    @param[in] a_files       List of file names (STL, PLY, or VTK).
    @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf.
    @return Vector of shared pointers to FastTriMeshSDF objects, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4, size_t W = EBGEOMETRY_SOA_DEFAULT_WIDTH>
  [[nodiscard]] inline static std::vector<std::shared_ptr<FastTriMeshSDF<T, Meta, K, W>>>
  readIntoTriangleBVH(const std::vector<std::string> a_files, const size_t a_maxLeafSize = 8U) noexcept;

  /**
    @brief Read a file and return all faces as a flat list of Triangle objects.
    @details The mesh is first parsed into a DCEL, then each face is extracted into an
    independent Triangle with precomputed vertex positions, normals, and edge normals.
    @tparam T    Floating-point precision for vertex coordinates and normals.
    @tparam Meta Per-face metadata type.
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Flat vector of shared pointers to Triangle objects.
  */
  template <typename T, typename Meta>
  [[nodiscard]] inline static std::vector<std::shared_ptr<Triangle<T, Meta>>>
  readIntoTriangles(const std::string a_filename) noexcept;

  /**
    @brief Read multiple files and return all faces from each as flat lists of Triangle objects.
    @tparam T    Floating-point precision for vertex coordinates and normals.
    @tparam Meta Per-face metadata type.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Outer vector indexed by file; each inner vector is the flat triangle list for that file.
  */
  template <typename T, typename Meta>
  [[nodiscard]] inline static std::vector<std::vector<std::shared_ptr<Triangle<T, Meta>>>>
  readIntoTriangles(const std::vector<std::string> a_files) noexcept;
} // namespace Parser

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ParserImplem.hpp"

#endif
