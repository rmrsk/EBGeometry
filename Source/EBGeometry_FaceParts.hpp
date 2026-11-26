// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_FaceParts.hpp
 * @brief  Parts of a mesh picked by face: a parts file read into FaceGroups and written from them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FACEPARTS_HPP
#define EBGEOMETRY_FACEPARTS_HPP

// Std includes
#include <cstdint>
#include <string>

// Our includes
#include "EBGeometry_FaceGroups.hpp"
#include "EBGeometry_ParseError.hpp"
#include "EBGeometry_PolygonSoup.hpp"

namespace EBGeometry {

namespace Parser {

/**
 * @brief A fingerprint of a mesh's face numbering: its vertex count and each face's vertex indices, in order.
 * @details It does not depend on the precision or on the vertices' coordinates, so it changes exactly when a face id
 * can come to mean another face: when the mesh file gains, loses or reorders faces or vertices. A parts file records
 * it, and readFaceParts() rejects the file when the mesh no longer has it. The value is a 64-bit FNV-1a hash.
 * @tparam T Floating-point precision.
 * @param[in] a_soup The mesh, as a reader returns it (readIntoPolygonSoup()).
 * @return The fingerprint.
 */
template <typename T>
[[nodiscard]] inline uint64_t
meshFingerprint(const PolygonSoup<T>& a_soup) noexcept;

/**
 * @brief Read a parts file: named parts of a mesh, each a set of the mesh's faces.
 * @details A parts file is text. Its first line is `ebg-parts 1`; its second `mesh "<file>" fingerprint <hex>`, the
 * mesh file it was written for (as written, for the reader's information) and meshFingerprint() of that mesh. Each
 * further line is a part's name followed by face ids, single (`12`) or as inclusive ranges (`40-58`), separated by
 * spaces; a part may continue on further lines that start with the same name. A `#` starts a comment, and blank lines
 * are skipped. Face ids are the reader's: file order, without the faces it removes (see readIntoPolygonSoup()), the
 * ids closestPart() and getClosestFace() report. The parts take group ids in the order their names first appear.
 * @tparam T Floating-point precision.
 * @param[in] a_file The parts file.
 * @param[in] a_soup The mesh the parts are for.
 * @return The parts' names, and each face's part or FaceGroups::None, one entry per face of a_soup.
 * @throws ParseError (with the line) if the file cannot be read, a line is malformed, a face id is out of range, a face
 * is in two parts, or the fingerprint is not a_soup's (the mesh changed since the parts were picked).
 */
template <typename T>
[[nodiscard]] inline FaceGroups
readFaceParts(const std::string& a_file, const PolygonSoup<T>& a_soup);

/**
 * @brief Write a parts file that readFaceParts() reads back into a_parts.
 * @details Each part's faces are written in increasing order, consecutive ids as ranges, on lines of at most about a
 * hundred characters. A part with no faces is written as its name alone, so that its name is kept.
 * @tparam T Floating-point precision.
 * @param[in] a_file     The parts file, overwritten.
 * @param[in] a_parts    The parts: names, and one entry per face of a_soup (a part id or FaceGroups::None).
 * @param[in] a_soup     The mesh the parts are for; its fingerprint is written.
 * @param[in] a_meshFile The mesh's file name, written for the reader's information.
 * @throws ParseError if the file cannot be written, a_parts.m_group does not have one entry per face, a part id is out
 * of range, or a name is empty, contains whitespace or starts with `#`.
 */
template <typename T>
inline void
writeFaceParts(const std::string&    a_file,
               const FaceGroups&     a_parts,
               const PolygonSoup<T>& a_soup,
               const std::string&    a_meshFile);

/**
 * @brief The groups a mesh file names, with picked parts on top.
 * @details A face in a picked part takes that part; every other face keeps its group from the file, or none. The
 * result names the file's groups first and then the picked parts, each name once: a picked part named like a file group
 * is that group, so its faces join the group's remaining faces.
 * @param[in] a_fileGroups The groups the mesh file names (PolygonSoup::getFaceGroups()); may be empty.
 * @param[in] a_picked     The picked parts (readFaceParts()), one entry per face.
 * @return The combined groups, one entry per face.
 * @throws ParseError if a_fileGroups is not empty and has another face count than a_picked.
 */
[[nodiscard]] inline FaceGroups
overlayFaceGroups(const FaceGroups& a_fileGroups, const FaceGroups& a_picked);

} // namespace Parser

} // namespace EBGeometry

#include "EBGeometry_FacePartsImplem.hpp"

#endif
