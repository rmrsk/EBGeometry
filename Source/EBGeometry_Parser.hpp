// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Parser.hpp
 * @brief  Declaration of utilities for reading files into EBGeometry data
 * structures.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PARSER_HPP
#define EBGEOMETRY_PARSER_HPP

// Std includes
#include <cstddef>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_PolygonSoup.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"
#include "EBGeometry_TriangleSoA.hpp"

namespace EBGeometry {

/**
 * @brief Namespace which encapsulates possible file parsers for building
 * EBGeometry data structures.
 */
namespace Parser {

/**
 * @brief Simple enum for separating ASCII and binary files
 */
enum class Encoding
{
  ASCII,  ///< File is plain text
  Binary, ///< File is binary-encoded
  Unknown ///< Encoding could not be determined
};

/**
 * @brief Various supported file types
 */
enum class FileType
{
  STL,        ///< Stereolithography format (.stl)
  PLY,        ///< Polygon File Format (.ply)
  VTK,        ///< Legacy VTK polydata format (.vtk)
  OBJ,        ///< Wavefront OBJ format (.obj)
  Unsupported ///< File type is not recognised
};

/**
 * @brief Determine file type from the file extension.
 * @param[in] a_filename File name.
 * @return Detected FileType (STL, PLY, VTK, OBJ, or Unsupported).
 */
[[nodiscard]] inline static Parser::FileType
getFileType(const std::string& a_filename) noexcept;

/**
 * @brief Determine file encoding (ASCII or binary) by inspecting the file header.
 * @param[in] a_filename File name.
 * @return Detected Encoding (ASCII, Binary, or Unknown).
 */
[[nodiscard]] inline static Parser::Encoding
getFileEncoding(const std::string& a_filename) noexcept;

/**
 * @brief Read a Stanford PLY file, ASCII or binary.
 * @details The scalar vertex properties other than x, y and z, and the scalar face properties, are
 * kept as the soup's properties.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_filename PLY file name.
 * @return The soup, cleaned (see PolygonSoup::clean()).
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] PolygonSoup<T>
readPLY(const std::string& a_filename);

/**
 * @brief Read several Stanford PLY files.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_filenames PLY file names.
 * @return One cleaned soup per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<PolygonSoup<T>>
readPLY(const std::vector<std::string>& a_filenames);

/**
 * @brief Read an STL file, ASCII or binary.
 * @details The facet normals in the file are ignored. If the file holds several solids (uncommon, but
 * allowed), only the first is read.
 * @tparam T Floating-point precision for vertex coordinates.
 * @param[in] a_filename STL file name.
 * @return The soup, cleaned (see PolygonSoup::clean()).
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] PolygonSoup<T>
readSTL(const std::string& a_filename);

/**
 * @brief Read several STL files.
 * @tparam T Floating-point precision for vertex coordinates.
 * @param[in] a_filenames STL file names.
 * @return One cleaned soup per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<PolygonSoup<T>>
readSTL(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a Wavefront OBJ file.
 * @details Only the vertices and faces are read; texture coordinates, normals, groups and materials
 * are ignored. A face record with fewer than three vertices is not a face.
 * @tparam T Floating-point precision for vertex coordinates.
 * @param[in] a_filename OBJ file name.
 * @return The soup, cleaned (see PolygonSoup::clean()).
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] PolygonSoup<T>
readOBJ(const std::string& a_filename);

/**
 * @brief Read several Wavefront OBJ files.
 * @tparam T Floating-point precision for vertex coordinates.
 * @param[in] a_filenames OBJ file names.
 * @return One cleaned soup per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<PolygonSoup<T>>
readOBJ(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a legacy VTK POLYDATA file, ASCII or binary.
 * @details The first component of each POINT_DATA and CELL_DATA scalar array is kept as a vertex or
 * face property. An array with a different number of values than there are points or polygons (as
 * when CELL_DATA also covers lines or vertices) is dropped with a warning.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_filename VTK file name.
 * @return The soup, cleaned (see PolygonSoup::clean()).
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] PolygonSoup<T>
readVTK(const std::string& a_filename);

/**
 * @brief Read several legacy VTK POLYDATA files.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_filenames VTK file names.
 * @return One cleaned soup per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<PolygonSoup<T>>
readVTK(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a file of any supported format, chosen by its extension.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_filename File name (.stl, .ply, .vtk or .obj).
 * @return The soup, cleaned (see PolygonSoup::clean()).
 * @throws ParseError if the extension is not supported, or the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] inline static PolygonSoup<T>
readIntoPolygonSoup(const std::string& a_filename);

/**
 * @brief Read several files of any supported format.
 * @tparam T Floating-point precision for vertex coordinates and properties.
 * @param[in] a_files File names (.stl, .ply, .vtk or .obj).
 * @return One cleaned soup per file.
 * @throws ParseError if any of the files cannot be read.
 */
template <typename T>
[[nodiscard]] inline static std::vector<PolygonSoup<T>>
readIntoPolygonSoup(const std::vector<std::string>& a_files);

/**
 * @brief Read a file containing a single watertight object and return it as a DCEL mesh.
 * @details readIntoPolygonSoup() followed by PolygonSoup::convertToDCEL(). Face i of the mesh is
 * face i of the soup: the file's i-th face, not counting degenerate faces, which are removed (see
 * PolygonSoup::clean()). That index is the face id the mesh SDFs report from getClosestFace().
 * @tparam T    Floating-point precision for vertex coordinates.
 * @param[in]     a_filename File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool     Pool to reserve the mesh's vertex, edge and face storage from. The
 * returned mesh resolves its storage through a_pool, so a_pool must outlive it.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return The DCEL mesh, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static EBGeometry::DCEL::MeshT<T>
readIntoDCEL(const std::string& a_filename, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read several files, each containing a single watertight object, as DCEL meshes.
 * @tparam T    Floating-point precision for vertex coordinates.
 * @param[in]     a_files    File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool     Pool to reserve every mesh's storage from; must outlive them.
 * @param[in]     a_onDefect As for the single-file overload.
 * @return One DCEL mesh per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<EBGeometry::DCEL::MeshT<T>>
readIntoDCEL(const std::vector<std::string>& a_files, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file into a FlatMeshSDF: the signed distance to its DCEL mesh by a scan over every
 * face, with no BVH.
 * @tparam T    Floating-point precision.
 * @param[in]     a_filename File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool     Pool to reserve the mesh's storage from; must outlive the FlatMeshSDF.
 * @param[in]     a_onDefect As for readIntoDCEL().
 * @return The FlatMeshSDF, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static FlatMeshSDF<T>
readIntoFlatMeshSDF(const std::string& a_filename, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read several files into FlatMeshSDFs.
 * @tparam T    Floating-point precision.
 * @param[in]     a_files    File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool     Pool to reserve every mesh's storage from; must outlive them.
 * @param[in]     a_onDefect As for readIntoDCEL().
 * @return One FlatMeshSDF per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<FlatMeshSDF<T>>
readIntoFlatMeshSDF(const std::vector<std::string>& a_files, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file into a MeshSDF: its DCEL mesh with a packed K-ary BVH over the faces.
 * @details Supports any polygon mesh. For a triangle mesh, readIntoTriMeshSDF() is usually faster to
 * query. The leaf sizes are MeshSDF::defaultConstructionOptions().
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @param[in]     a_filename     File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve the mesh and its BVH from; must outlive the MeshSDF.
 * @param[in]     a_construction BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return The MeshSDF, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>()>
[[nodiscard]] inline static MeshSDF<T, K>
readIntoMeshSDF(const std::string&      a_filename,
                Pool&                   a_pool,
                const BVH::Construction a_construction = BVH::Construction::SAH,
                const OnDefect          a_onDefect     = OnDefect::Throw);

/**
 * @brief Read a file into a MeshSDF, with leaf-size settings.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @param[in]     a_filename     File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve the mesh and its BVH from; must outlive the MeshSDF.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_options      Leaf-size settings, in faces. The chosen method reads only its own
 * field; start from MeshSDF::defaultConstructionOptions().
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return The MeshSDF, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>()>
[[nodiscard]] inline static MeshSDF<T, K>
readIntoMeshSDF(const std::string&              a_filename,
                Pool&                           a_pool,
                const BVH::Construction         a_construction,
                const BVH::ConstructionOptions& a_options,
                const OnDefect                  a_onDefect = OnDefect::Throw);

/**
 * @brief Read several files into MeshSDFs.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @param[in]     a_files        File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve every mesh and BVH from; must outlive them.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return One MeshSDF per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>()>
[[nodiscard]] inline static std::vector<MeshSDF<T, K>>
readIntoMeshSDF(const std::vector<std::string>& a_files,
                Pool&                           a_pool,
                const BVH::Construction         a_construction = BVH::Construction::SAH,
                const OnDefect                  a_onDefect     = OnDefect::Throw);

/**
 * @brief Read several files into MeshSDFs, with leaf-size settings.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @param[in]     a_files        File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve every mesh and BVH from; must outlive them.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_options      Leaf-size settings, in faces.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return One MeshSDF per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>()>
[[nodiscard]] inline static std::vector<MeshSDF<T, K>>
readIntoMeshSDF(const std::vector<std::string>& a_files,
                Pool&                           a_pool,
                const BVH::Construction         a_construction,
                const BVH::ConstructionOptions& a_options,
                const OnDefect                  a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file into a TriMeshSDF: its faces as triangles, in SIMD groups of W, under a packed
 * K-ary BVH.
 * @details Polygon faces are fan-triangulated (see readIntoTriangles()). The DCEL mesh the triangles
 * are cut from lives in a Pool private to this call, so it takes no space in a_pool. The leaf sizes
 * are TriMeshSDF::defaultConstructionOptions(4): at most four groups of W triangles per leaf.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>() (4, independent of
 * compiler flags).
 * @tparam W    Triangles per SIMD group. Defaults to TriangleSoA::DefaultWidth<T>().
 * @param[in]     a_filename     File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve the BVH from; must outlive the TriMeshSDF.
 * @param[in]     a_construction BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return The TriMeshSDF, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static TriMeshSDF<T, K, W>
readIntoTriMeshSDF(const std::string&      a_filename,
                   Pool&                   a_pool,
                   const BVH::Construction a_construction = BVH::Construction::SAH,
                   const OnDefect          a_onDefect     = OnDefect::Throw);

/**
 * @brief Read a file into a TriMeshSDF, with leaf-size settings.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @tparam W    Triangles per SIMD group. Defaults to TriangleSoA::DefaultWidth<T>().
 * @param[in]     a_filename     File name (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve the BVH from; must outlive the TriMeshSDF.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_options      Leaf-size settings, in triangles. The chosen method reads only its own
 * field; start from TriMeshSDF::defaultConstructionOptions().
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return The TriMeshSDF, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static TriMeshSDF<T, K, W>
readIntoTriMeshSDF(const std::string&              a_filename,
                   Pool&                           a_pool,
                   const BVH::Construction         a_construction,
                   const BVH::ConstructionOptions& a_options,
                   const OnDefect                  a_onDefect = OnDefect::Throw);

/**
 * @brief Read several files into TriMeshSDFs.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @tparam W    Triangles per SIMD group. Defaults to TriangleSoA::DefaultWidth<T>().
 * @param[in]     a_files        File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve every BVH from; must outlive them.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return One TriMeshSDF per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static std::vector<TriMeshSDF<T, K, W>>
readIntoTriMeshSDF(const std::vector<std::string>& a_files,
                   Pool&                           a_pool,
                   const BVH::Construction         a_construction = BVH::Construction::SAH,
                   const OnDefect                  a_onDefect     = OnDefect::Throw);

/**
 * @brief Read several files into TriMeshSDFs, with leaf-size settings.
 * @tparam T    Floating-point precision.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>().
 * @tparam W    Triangles per SIMD group. Defaults to TriangleSoA::DefaultWidth<T>().
 * @param[in]     a_files        File names (.stl, .ply, .vtk or .obj).
 * @param[in,out] a_pool         Pool to reserve every BVH from; must outlive them.
 * @param[in]     a_construction BVH construction method.
 * @param[in]     a_options      Leaf-size settings, in triangles.
 * @param[in]     a_onDefect     As for readIntoDCEL().
 * @return One TriMeshSDF per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static std::vector<TriMeshSDF<T, K, W>>
readIntoTriMeshSDF(const std::vector<std::string>& a_files,
                   Pool&                           a_pool,
                   const BVH::Construction         a_construction,
                   const BVH::ConstructionOptions& a_options,
                   const OnDefect                  a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file and return its faces as flat triangles.
 * @details readIntoPolygonSoup() followed by PolygonSoup::convertToTriangles(): each face becomes one
 * or more Triangle objects with the mesh's vertex positions and normals, a polygon by a fan from its
 * first vertex. Each triangle carries the id of the face it was cut from, as readIntoDCEL() numbers
 * the faces. The intermediate DCEL mesh lives in a Pool private to this call and is freed on return.
 * @tparam T    Floating-point precision.
 * @param[in] a_filename File name (.stl, .ply, .vtk or .obj).
 * @param[in] a_onDefect As for readIntoDCEL().
 * @return The triangles, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<Triangle<T>>
readIntoTriangles(const std::string& a_filename, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read several files and return each one's faces as flat triangles.
 * @tparam T    Floating-point precision.
 * @param[in] a_files    File names (.stl, .ply, .vtk or .obj).
 * @param[in] a_onDefect As for readIntoDCEL().
 * @return One triangle list per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<std::vector<Triangle<T>>>
readIntoTriangles(const std::vector<std::string>& a_files, const OnDefect a_onDefect = OnDefect::Throw);
} // namespace Parser

} // namespace EBGeometry

#include "EBGeometry_ParserImplem.hpp"

#endif
