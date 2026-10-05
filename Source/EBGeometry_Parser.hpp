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
#include <memory>
#include <string>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_DCEL_Mesh.hpp"
#include "EBGeometry_MeshDistanceFunctions.hpp"
#include "EBGeometry_OBJ.hpp"
#include "EBGeometry_PLY.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_Pool.hpp"
#include "EBGeometry_STL.hpp"
#include "EBGeometry_Triangle.hpp"
#include "EBGeometry_TriangleAoSoA.hpp"
#include "EBGeometry_TriangleSoA.hpp"
#include "EBGeometry_VTK.hpp"

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
 * @return Detected FileType (STL, PLY, VTK, or Unsupported).
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
 * @brief Throw unless a polygon soup read from a file can describe a mesh.
 * @details Every reader runs this last. A soup that fails Soup::isValid (a vertex index out of
 * range, or a non-finite coordinate, as corrupted files produce) is rejected rather than read as
 * garbage.
 * @tparam T Floating-point precision type for vertex coordinates.
 * @param[in] a_vertices Vertex coordinate list.
 * @param[in] a_facets   Index lists.
 * @param[in] a_filename File the soup was read from, for the message.
 * @throws ParseError if the soup is invalid.
 */
template <typename T>
inline static void
requireValidSoup(const std::vector<Vec3T<T>>&            a_vertices,
                 const std::vector<std::vector<size_t>>& a_facets,
                 const std::string&                      a_filename);

/**
 * @brief Read a single PLY file into a raw PLY data structure.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filename PLY file name.
 * @return Populated PLY<T> object containing vertex coordinates and face indices.
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] PLY<T>
readPLY(const std::string& a_filename);

/**
 * @brief Read multiple PLY files into raw PLY data structures.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filenames List of PLY file names.
 * @return Vector of populated PLY<T> objects, one per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<PLY<T>>
readPLY(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a single STL file into a raw STL data structure.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filename STL file name.
 * @return Populated STL<T> object containing vertex coordinates and face indices.
 * @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
 * will only read the first one.
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] STL<T>
readSTL(const std::string& a_filename);

/**
 * @brief Read multiple STL files into raw STL data structures.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filenames List of STL file names.
 * @return Vector of populated STL<T> objects, one per file.
 * @note If the STL file contains multiple solids (which is uncommon but technically supported), this routine
 * will only read the first one.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<STL<T>>
readSTL(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a single Wavefront OBJ file into a raw OBJ data structure.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filename OBJ file name.
 * @return Populated OBJ<T> object containing vertex coordinates and face indices.
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] OBJ<T>
readOBJ(const std::string& a_filename);

/**
 * @brief Read multiple Wavefront OBJ files into raw OBJ data structures.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filenames List of OBJ file names.
 * @return Vector of populated OBJ<T> objects, one per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<OBJ<T>>
readOBJ(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a single VTK legacy polydata file into a raw VTK data structure.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filename VTK file name.
 * @return Populated VTK<T> object containing vertex coordinates and face indices.
 * @throws ParseError if the file cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] VTK<T>
readVTK(const std::string& a_filename);

/**
 * @brief Read multiple VTK legacy polydata files into raw VTK data structures.
 * @tparam T Floating-point precision used for vertex coordinates.
 * @param[in] a_filenames List of VTK file names.
 * @return Vector of populated VTK<T> objects, one per file.
 * @throws ParseError if any of the files cannot be read or is malformed.
 */
template <typename T>
[[nodiscard]] std::vector<VTK<T>>
readVTK(const std::vector<std::string>& a_filenames);

/**
 * @brief Read a file containing a single watertight object and return it as a DCEL mesh.
 * @details Face i of the mesh is the file's i-th face, not counting zero-area faces, which are
 * removed (see Soup::removeDegeneratePolygons). That index is the face id the mesh SDFs report from
 * getClosestFace().
 * @tparam T    Floating-point precision for vertex coordinates.
 * @param[in]     a_filename File name (STL, PLY, or VTK).
 * @param[in,out] a_pool     Pool to reserve the constructed mesh's vertex/edge/face storage from.
 * The returned mesh resolves its storage through a_pool, so a_pool must outlive it.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return The constructed DCEL mesh, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static EBGeometry::DCEL::MeshT<T>
readIntoDCEL(const std::string a_filename, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read multiple files containing single watertight objects and return them as DCEL meshes.
 * @tparam T    Floating-point precision for vertex coordinates.
 * @param[in]     a_files List of file names (STL, PLY, or VTK).
 * @param[in,out] a_pool  Pool to reserve every constructed mesh's storage from -- all meshes
 * share this one Pool, laid out contiguously.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Vector of the constructed DCEL meshes, one per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<EBGeometry::DCEL::MeshT<T>>
readIntoDCEL(const std::vector<std::string>& a_files, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file and return it as a bare DCEL signed-distance function (O(N) scan, no BVH).
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @param[in]     a_filename File name (STL, PLY, or VTK).
 * @param[in,out] a_pool     Pool to reserve the constructed mesh's vertex/edge/face storage from.
 * The returned FlatMeshSDF resolves its mesh through a_pool, so a_pool must outlive it.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return The FlatMeshSDF over the parsed DCEL mesh, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static FlatMeshSDF<T>
readIntoMesh(const std::string a_filename, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read multiple files and return each as a bare DCEL signed-distance function.
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @param[in]     a_files List of file names (STL, PLY, or VTK).
 * @param[in,out] a_pool  Pool to reserve every constructed mesh's storage from -- all meshes
 * share this one Pool, laid out contiguously.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Vector of FlatMeshSDF objects, one per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<FlatMeshSDF<T>>
readIntoMesh(const std::vector<std::string>& a_files, Pool& a_pool, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read a file and return it enclosed in a SIMD-accelerated PackedBVH over DCEL faces.
 * @details Supports any polygon mesh, not just triangles. For triangle-only meshes with
 * maximum throughput, prefer readIntoTriangleBVH which uses SoA leaf grouping.
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @tparam K    BVH branching factor (number of children per internal node).
 * @param[in]     a_filename File name (STL, PLY, or VTK).
 * @param[in,out] a_pool     Pool to reserve the underlying DCEL mesh's storage from. The
 * returned MeshSDF holds the mesh and its BVH in a_pool, so a_pool must outlive it.
 * @param[in]     a_construction    Preset BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return The MeshSDF enclosing the mesh, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = 4>
[[nodiscard]] inline static MeshSDF<T, K>
readIntoPackedBVH(const std::string       a_filename,
                  Pool&                   a_pool,
                  const BVH::Construction a_construction = BVH::Construction::SAH,
                  const OnDefect          a_onDefect     = OnDefect::Throw);

/**
 * @brief Read multiple files and return each enclosed in a SIMD-accelerated PackedBVH over DCEL faces.
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @tparam K    BVH branching factor (number of children per internal node).
 * @param[in]     a_files List of file names (STL, PLY, or VTK).
 * @param[in,out] a_pool  Pool to reserve every underlying DCEL mesh's storage from -- all meshes
 * share this one Pool, laid out contiguously. Must outlive the returned MeshSDF objects.
 * @param[in]     a_construction Preset BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Vector of MeshSDF objects, one per file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T, size_t K = 4>
[[nodiscard]] inline static std::vector<MeshSDF<T, K>>
readIntoPackedBVH(const std::vector<std::string>& a_files,
                  Pool&                           a_pool,
                  const BVH::Construction         a_construction = BVH::Construction::SAH,
                  const OnDefect                  a_onDefect     = OnDefect::Throw);

/**
 * @brief Read a file and return the mesh enclosed in a SIMD-optimised triangle BVH.
 * @details Triangles are grouped into SoA bundles of W and packed into a linearised K-ary BVH.
 * At query time the BVH uses SIMD intrinsics to evaluate W triangles per leaf visit.
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>() (4, independent of
 * compiler flags); BVH::HostBranchingRatio<T>() is the value tuned to the host's SIMD flags, for
 * host-only code.
 * @tparam W    SIMD lane width: triangles per SoA group. Defaults to TriangleSoA::DefaultWidth<T>() (4);
 * TriangleSoA::HostWidth<T>() is the host-tuned value.
 * @param[in]     a_filename      File name (STL, PLY, VTK or OBJ).
 * @param[in,out] a_pool          Pool the returned TriMeshSDF's BVH is reserved from; must outlive
 * it. The intermediate DCEL mesh lives in a Pool private to this call (see readIntoTriangles), so
 * it takes no space in a_pool and is not mirrored along with it.
 * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf; the
 * actual raw-triangle leaf-size bound used is a_maxLeafGroups * W (see TriMeshSDF's mesh-based
 * constructor for the tree-quality/SIMD-occupancy trade-off). Defaults to 4.
 * @param[in]     a_construction         Preset BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return The TriMeshSDF enclosing the mesh, by value.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static TriMeshSDF<T, K, W>
readIntoTriangleBVH(const std::string       a_filename,
                    Pool&                   a_pool,
                    const size_t            a_maxLeafGroups = 4,
                    const BVH::Construction a_construction  = BVH::Construction::SAH,
                    const OnDefect          a_onDefect      = OnDefect::Throw);

/**
 * @brief Read multiple files and return each mesh enclosed in a SIMD-optimised triangle BVH.
 * @tparam T    Floating-point precision for signed-distance evaluation.
 * @tparam K    BVH branching factor. Defaults to BVH::DefaultBranchingRatio<T>() (see single-file overload).
 * @tparam W    SIMD lane width: triangles per SoA group. Defaults to TriangleSoA::DefaultWidth<T>().
 * @param[in]     a_files         List of file names (STL, PLY, or VTK).
 * @param[in,out] a_pool          Pool every returned TriMeshSDF's BVH is reserved from (see the
 * single-file overload).
 * @param[in]     a_maxLeafGroups Maximum number of full W-sized TriangleSoA groups per BVH leaf (see
 * the single-file overload for details). Defaults to 4.
 * @param[in]     a_construction         Preset BVH construction method. SAH is the default and recommended choice.
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Vector of TriMeshSDF objects, one per file.
 */
template <typename T, size_t K = BVH::DefaultBranchingRatio<T>(), size_t W = TriangleSoA::DefaultWidth<T>()>
[[nodiscard]] inline static std::vector<TriMeshSDF<T, K, W>>
readIntoTriangleBVH(const std::vector<std::string>& a_files,
                    Pool&                           a_pool,
                    const size_t                    a_maxLeafGroups = 4,
                    const BVH::Construction         a_construction  = BVH::Construction::SAH,
                    const OnDefect                  a_onDefect      = OnDefect::Throw);

/**
 * @brief Read a file and return all faces as a flat list of Triangle objects.
 * @details The mesh is first parsed into a DCEL, then each face is extracted into an
 * independent Triangle with precomputed vertex positions, normals, and edge normals; a polygon face
 * is fan-triangulated. Each triangle's face id is the index of the mesh face it was cut from, as
 * readIntoDCEL() numbers them. The intermediate DCEL mesh lives in a Pool private to this call and is
 * freed on return, so it never occupies (or gets mirrored along with) any Pool of the caller's.
 * @tparam T    Floating-point precision for vertex coordinates and normals.
 * @param[in] a_filename File name (STL, PLY, VTK or OBJ).
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Flat vector of Triangle objects, by value.
 * @throws ParseError if the file cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<Triangle<T>>
readIntoTriangles(const std::string a_filename, const OnDefect a_onDefect = OnDefect::Throw);

/**
 * @brief Read multiple files and return all faces from each as flat lists of Triangle objects.
 * @tparam T    Floating-point precision for vertex coordinates and normals.
 * @param[in] a_files List of file names (STL, PLY, VTK or OBJ).
 * @param[in]     a_onDefect Whether faces oriented inconsistently, an edge shared by three or more
 * faces, or faces that fold back onto each other throw (OnDefect::Throw, the default) or load with a
 * warning (OnDefect::Warn); see OnDefect.
 * @return Outer vector indexed by file; each inner vector is the flat triangle list for that file.
 * @throws ParseError if any of the files cannot be read, is malformed, or has no faces.
 */
template <typename T>
[[nodiscard]] inline static std::vector<std::vector<Triangle<T>>>
readIntoTriangles(const std::vector<std::string>& a_files, const OnDefect a_onDefect = OnDefect::Throw);
} // namespace Parser

} // namespace EBGeometry

#include "EBGeometry_ParserImplem.hpp"

#endif
