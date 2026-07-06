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
  readPLY(const std::string& a_filename);

  /**
    @brief Read multiple PLY files into raw PLY data structures.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filenames List of PLY file names.
    @return Vector of populated PLY<T> objects, one per file.
  */
  template <typename T>
  [[nodiscard]] std::vector<PLY<T>>
  readPLY(const std::vector<std::string>& a_filenames);

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
  readSTL(const std::string& a_filename);

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
  readSTL(const std::vector<std::string>& a_filenames);

  /**
    @brief Read a single VTK legacy polydata file into a raw VTK data structure.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filename VTK file name.
    @return Populated VTK<T> object containing vertex coordinates and face indices.
  */
  template <typename T>
  [[nodiscard]] VTK<T>
  readVTK(const std::string& a_filename);

  /**
    @brief Read multiple VTK legacy polydata files into raw VTK data structures.
    @tparam T Floating-point precision used for vertex coordinates.
    @param[in] a_filenames List of VTK file names.
    @return Vector of populated VTK<T> objects, one per file.
  */
  template <typename T>
  [[nodiscard]] std::vector<VTK<T>>
  readVTK(const std::vector<std::string>& a_filenames);

  /**
    @brief Read a file containing a single watertight object and return it as a DCEL mesh.
    @tparam T    Floating-point precision for vertex coordinates.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the constructed DCEL mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>
  readIntoDCEL(const std::string a_filename);

  /**
    @brief Read multiple files containing single watertight objects and return them as DCEL meshes.
    @tparam T    Floating-point precision for vertex coordinates.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to the constructed DCEL meshes, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::vector<std::shared_ptr<EBGeometry::DCEL::MeshT<T, Meta>>>
  readIntoDCEL(const std::vector<std::string> a_files);

  /**
    @brief Read a file and return it as a bare DCEL signed-distance function (O(N) scan, no BVH).
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the FlatMeshSDF wrapping the parsed DCEL mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::shared_ptr<FlatMeshSDF<T, Meta>>
  readIntoMesh(const std::string a_filename);

  /**
    @brief Read multiple files and return each as a bare DCEL signed-distance function.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to FlatMeshSDF objects, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData>
  [[nodiscard]] inline static std::vector<std::shared_ptr<FlatMeshSDF<T, Meta>>>
  readIntoMesh(const std::vector<std::string> a_files);

  /**
    @brief Read a file and return it enclosed in a SIMD-accelerated PackedBVH over DCEL faces.
    @details Supports any polygon mesh, not just triangles. For triangle-only meshes with
    maximum throughput, prefer readIntoTriangleBVH which uses SoA leaf grouping.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_filename File name (STL, PLY, or VTK).
    @return Shared pointer to the MeshSDF enclosing the mesh.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4>
  [[nodiscard]] inline static std::shared_ptr<MeshSDF<T, Meta, K>>
  readIntoPackedBVH(const std::string a_filename);

  /**
    @brief Read multiple files and return each enclosed in a SIMD-accelerated PackedBVH over DCEL faces.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type stored in the DCEL mesh.
    @tparam K    BVH branching factor (number of children per internal node).
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Vector of shared pointers to MeshSDF objects, one per file.
  */
  template <typename T, typename Meta = DCEL::DefaultMetaData, size_t K = 4>
  [[nodiscard]] inline static std::vector<std::shared_ptr<MeshSDF<T, Meta, K>>>
  readIntoPackedBVH(const std::vector<std::string> a_files);

  /**
    @brief Read a file and return the mesh enclosed in a SIMD-optimised triangle BVH.
    @details Triangles are grouped into SoA bundles of W and packed into a linearised K-ary BVH.
    At query time the BVH uses SIMD intrinsics to evaluate W triangles per leaf visit.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type.
    @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>() — the SIMD-optimal value for
                 T on the current ISA (K=16/float or K=8/double on AVX-512F; K=8/float or K=4/double
                 on AVX; K=4 otherwise). Override only when benchmarking or using non-SIMD builds.
    @tparam W    SIMD lane width: triangles per SoA group. Defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH
                 (16 with AVX-512F, 8 with AVX, 4 with SSE4.1).
    @param[in] a_filename    File name (STL, PLY, or VTK).
    @param[in] a_build       BVH build strategy. SAH is the default and recommended choice.
    @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf. Should be a multiple of W.
    @return Shared pointer to the TriMeshSDF enclosing the mesh.
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            size_t K      = BVH::DefaultBranchingRatio<T>(),
            size_t W      = EBGEOMETRY_SOA_DEFAULT_WIDTH>
  [[nodiscard]] inline static std::shared_ptr<TriMeshSDF<T, Meta, K, W>>
  readIntoTriangleBVH(const std::string a_filename,
                      const BVH::Build  a_build       = BVH::Build::SAH,
                      const size_t      a_maxLeafSize = 8U);

  /**
    @brief Read multiple files and return each mesh enclosed in a SIMD-optimised triangle BVH.
    @tparam T    Floating-point precision for signed-distance evaluation.
    @tparam Meta Per-face metadata type.
    @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>() (see single-file overload).
    @tparam W    SIMD lane width: triangles per SoA group. Defaults to EBGEOMETRY_SOA_DEFAULT_WIDTH.
    @param[in] a_files       List of file names (STL, PLY, or VTK).
    @param[in] a_build       BVH build strategy. SAH is the default and recommended choice.
    @param[in] a_maxLeafSize Maximum number of triangles per BVH leaf.
    @return Vector of shared pointers to TriMeshSDF objects, one per file.
  */
  template <typename T,
            typename Meta = DCEL::DefaultMetaData,
            size_t K      = BVH::DefaultBranchingRatio<T>(),
            size_t W      = EBGEOMETRY_SOA_DEFAULT_WIDTH>
  [[nodiscard]] inline static std::vector<std::shared_ptr<TriMeshSDF<T, Meta, K, W>>>
  readIntoTriangleBVH(const std::vector<std::string> a_files,
                      const BVH::Build               a_build       = BVH::Build::SAH,
                      const size_t                   a_maxLeafSize = 8U);

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
  readIntoTriangles(const std::string a_filename);

  /**
    @brief Read multiple files and return all faces from each as flat lists of Triangle objects.
    @tparam T    Floating-point precision for vertex coordinates and normals.
    @tparam Meta Per-face metadata type.
    @param[in] a_files List of file names (STL, PLY, or VTK).
    @return Outer vector indexed by file; each inner vector is the flat triangle list for that file.
  */
  template <typename T, typename Meta>
  [[nodiscard]] inline static std::vector<std::vector<std::shared_ptr<Triangle<T, Meta>>>>
  readIntoTriangles(const std::vector<std::string> a_files);
} // namespace Parser

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_ParserImplem.hpp"

#endif
