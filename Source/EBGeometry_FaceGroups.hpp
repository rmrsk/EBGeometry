// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_FaceGroups.hpp
 * @brief  FaceGroups: named groups of a mesh's faces, as a file names them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_FACEGROUPS_HPP
#define EBGEOMETRY_FACEGROUPS_HPP

// Std includes
#include <cstdint>
#include <string>
#include <vector>

namespace EBGeometry {

/**
 * @brief Named groups of a mesh's faces: an OBJ file's `g` and `o` groups, or the named solids of an ASCII STL
 * file, as the readers keep them (PolygonSoup::getFaceGroups()).
 * @details Face i is in group m_group[i], whose name is m_names[m_group[i]], or in none (None). A face belongs to
 * one group at most: in an OBJ file the last `g` or `o` line before it. SurfaceLabel(shape, groups) turns the groups into
 * labels of the mesh's parts, its faces (see hasParts).
 */
struct FaceGroups
{
  /**
   * @brief A face in no group.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief The groups' names, in the order they first appear in the file.
   */
  std::vector<std::string> m_names;

  /**
   * @brief Each face's group, or None; empty when the file names no groups.
   */
  std::vector<uint32_t> m_group;

  /**
   * @brief Whether any face is in a group.
   * @return True if the file named groups.
   */
  [[nodiscard]] bool
  empty() const noexcept
  {
    return m_names.empty();
  }
};

} // namespace EBGeometry

#endif
