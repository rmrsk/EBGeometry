// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RegionNames.hpp
 * @brief  RegionNames: the names of an object's regions, materials, surfaces and a scheme's groups, for showing them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONNAMES_HPP
#define EBGEOMETRY_REGIONNAMES_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Macros.hpp"

namespace EBGeometry {

/**
 * @brief The names of an object's regions, materials, surfaces and surface labels, from its expression's index, and
 * of a scheme's groups: what a viewer shows a piece of surface or a group by (Render::view()).
 * @details Host only. Anything without a name, or beyond the lists, is named by its kind and number ("region 3",
 * "group 1").
 */
class RegionNames
{
public:
  /**
   * @brief No names: everything is named by its number.
   */
  RegionNames() = default;

  /**
   * @brief The names an expression's index gives: a region by its material (the background "background" without
   * one), a surface by its label (or its number).
   * @tparam T Floating-point precision.
   * @param[in] a_index The index of the expression the tape was compiled from.
   */
  template <class T>
  EBGEOMETRY_HOST
  explicit RegionNames(const ExprIndex<T>& a_index)
  {
    for (uint32_t k = 0; k < a_index.getNumMaterialLabels(); k++) {
      m_materials.push_back(a_index.getMaterialLabelName(MaterialLabelId(k)));
    }

    for (uint32_t k = 0; k < a_index.getNumSurfaceLabels(); k++) {
      m_surfaceLabels.push_back(a_index.getSurfaceLabelName(SurfaceLabelId(k)));
    }

    for (uint32_t k = 0; k < a_index.getNumRegions(); k++) {
      const MaterialLabelId material = a_index.getMaterialOf(k);

      m_regions.push_back(!material.isNone() ? m_materials[material.value()]
                          : (k == 0U)        ? std::string("background")
                                             : std::string());
    }

    for (uint32_t s = 0; s < a_index.getNumSurfaces(); s++) {
      const SurfaceLabelId label = a_index.getSurfaceLabel(a_index.getLeafOfSurface(s));

      m_surfaces.push_back(!label.isNone() ? m_surfaceLabels[label.value()] : std::string());
    }
  }

  /**
   * @brief Name a scheme's group.
   * @param[in] a_group The group.
   * @param[in] a_name  Its name.
   */
  EBGEOMETRY_HOST
  void
  setSurfaceGroup(const uint32_t a_group, std::string a_name)
  {
    if (m_surfaceGroups.size() <= a_group) {
      m_surfaceGroups.resize(a_group + 1U);
    }

    m_surfaceGroups[a_group] = std::move(a_name);
  }

  /**
   * @brief Forget a group's name; the names of the groups after it move down by one (as Scheme::removeSurfaceGroup()
   * numbers them).
   * @param[in] a_group The group.
   */
  EBGEOMETRY_HOST
  void
  removeSurfaceGroup(const uint32_t a_group)
  {
    if (a_group < m_surfaceGroups.size()) {
      m_surfaceGroups.erase(m_surfaceGroups.begin() + static_cast<std::ptrdiff_t>(a_group));
    }
  }

  /**
   * @brief A region's name, with its number: "2 copper", or "region 2".
   * @param[in] a_region The region.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  region(const uint32_t a_region) const
  {
    return named(m_regions, a_region, "region");
  }

  /**
   * @brief A material's name, with its number.
   * @param[in] a_material The material label's number.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  material(const uint32_t a_material) const
  {
    return named(m_materials, a_material, "material");
  }

  /**
   * @brief A surface's name, with its number: its label's, or "surface 3".
   * @param[in] a_surface The surface (SurfaceRef::m_surface).
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surface(const uint32_t a_surface) const
  {
    return named(m_surfaces, a_surface, "surface");
  }

  /**
   * @brief A surface label's name, with its number.
   * @param[in] a_label The label's number.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surfaceLabel(const uint32_t a_label) const
  {
    return named(m_surfaceLabels, a_label, "label");
  }

  /**
   * @brief A surface group's name, with its number: "0 dirichlet", or "surface group 0".
   * @param[in] a_group The group.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surfaceGroup(const uint32_t a_group) const
  {
    return named(m_surfaceGroups, a_group, "surface group");
  }

protected:
  /**
   * @brief The names of the regions, materials, surfaces, surface labels and groups; an empty one is unnamed.
   */
  std::vector<std::string> m_regions;

  /**
   * @brief The materials' names.
   */
  std::vector<std::string> m_materials;

  /**
   * @brief The surfaces' names.
   */
  std::vector<std::string> m_surfaces;

  /**
   * @brief The surface labels' names.
   */
  std::vector<std::string> m_surfaceLabels;

  /**
   * @brief The groups' names.
   */
  std::vector<std::string> m_surfaceGroups;

  /**
   * @brief A name from a list, after its number, or the kind and the number.
   * @param[in] a_names The list.
   * @param[in] a_index The number.
   * @param[in] a_kind  What it is.
   * @return The name.
   */
  [[nodiscard]] static std::string
  named(const std::vector<std::string>& a_names, const uint32_t a_index, const char* a_kind)
  {
    const std::string number = std::to_string(a_index);

    return (a_index < a_names.size() && !a_names[a_index].empty()) ? number + " " + a_names[a_index]
                                                                   : std::string(a_kind) + " " + number;
  }
};

} // namespace EBGeometry

#endif
