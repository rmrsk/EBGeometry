// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_RegionNames.hpp
 * @brief  RegionNames: the names of an object's regions, materials, surfaces and surface groups, for showing them.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONNAMES_HPP
#define EBGEOMETRY_REGIONNAMES_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

// Our includes
#include "EBGeometry_DomainSetup.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_RegionSettings.hpp"
#include "EBGeometry_SurfacePieces.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

/**
 * @brief The names of an object's regions, materials, surfaces and surface labels, from its expression's index, and
 * of surface groups: what a viewer shows a piece of surface or a group by (Render::view()).
 * @details Host only. Anything without a name, or beyond the lists, is named by its kind and number ("region 3",
 * "surface group 1"). Names can also be given colors (setColor()), which a viewer colors the surfaces by.
 */
class RegionNames
{
public:
  /**
   * @brief No names: everything is named by its number.
   */
  RegionNames() = default;

  /**
   * @brief The names an expression's index gives: a region by its name and its material ("2 wall (steel)"), a
   * surface by its label (or its number).
   * @tparam T Floating-point precision.
   * @param[in] a_index The index of the expression the tape was compiled from.
   */
  template <class T>
  EBGEOMETRY_HOST
  explicit RegionNames(const ExprIndex<T>& a_index) : m_inventory(a_index)
  {
    for (uint32_t k = 0; k < a_index.getNumMaterialLabels(); k++) {
      m_materials.push_back(a_index.getMaterialLabelName(MaterialLabelId(k)));
    }

    for (uint32_t k = 0; k < a_index.getNumSurfaceLabels(); k++) {
      m_surfaceLabels.push_back(a_index.getSurfaceLabelName(SurfaceLabelId(k)));
    }

    for (uint32_t k = 0; k < a_index.getNumRegions(); k++) {
      const MaterialLabelId material = a_index.getMaterialOf(k);

      m_regionNames.push_back(a_index.getRegion(k).m_name);
      m_regionMaterials.push_back(material.isNone() ? None : material.value());
    }

    for (uint32_t s = 0; s < a_index.getNumSurfaces(); s++) {
      const SurfaceLabelId label = a_index.getSurfaceLabel(a_index.getLeafOfSurface(s));

      m_surfaces.push_back(!label.isNone() ? m_surfaceLabels[label.value()] : std::string());
    }
  }

  /**
   * @brief No material, no label.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief Register a domain: its name, and how it is made (its subdomains and surface groups, see DomainSetup). The
   * first registered is the active one, whose surface groups the names name and a viewer shows.
   * @param[in] a_name   Its name ("heat transfer").
   * @param[in] a_domain How it is made.
   * @return Its number, from 0 in the order registered.
   */
  EBGEOMETRY_HOST
  uint32_t
  addDomain(std::string a_name, const DomainSetup& a_domain)
  {
    m_domainNames.push_back(std::move(a_name));
    m_domains.push_back(a_domain);
    m_domainGroups.emplace_back();
    m_domainSubdomains.emplace_back();

    if (m_activeDomain == None) {
      m_activeDomain = 0;
    }

    return static_cast<uint32_t>(m_domains.size() - 1U);
  }

  /**
   * @brief How many domains are registered.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumDomains() const noexcept
  {
    return static_cast<uint32_t>(m_domains.size());
  }

  /**
   * @brief How a domain is made, to edit.
   * @param[in] a_domain Its number (below getNumDomains()).
   * @return Its setup.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  DomainSetup&
  getDomain(const uint32_t a_domain)
  {
    EBGEOMETRY_REQUIRE(
      a_domain < m_domains.size(), "RegionNames: domain %u of %zu", unsigned(a_domain), m_domains.size());

    return m_domains[a_domain];
  }

  /**
   * @brief How a domain is made.
   * @param[in] a_domain Its number (below getNumDomains()).
   * @return Its setup.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const DomainSetup&
  getDomain(const uint32_t a_domain) const
  {
    EBGEOMETRY_REQUIRE(
      a_domain < m_domains.size(), "RegionNames: domain %u of %zu", unsigned(a_domain), m_domains.size());

    return m_domains[a_domain];
  }

  /**
   * @brief A domain's name.
   * @param[in] a_domain Its number (below getNumDomains()).
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::string&
  domainName(const uint32_t a_domain) const
  {
    EBGEOMETRY_REQUIRE(
      a_domain < m_domains.size(), "RegionNames: domain %u of %zu", unsigned(a_domain), m_domains.size());

    return m_domainNames[a_domain];
  }

  /**
   * @brief Make a domain the active one: the one whose surface groups setSurfaceGroup() and surfaceGroup() name.
   * @param[in] a_domain Its number (below getNumDomains()).
   */
  EBGEOMETRY_HOST
  void
  setActiveDomain(const uint32_t a_domain)
  {
    EBGEOMETRY_REQUIRE(
      a_domain < m_domains.size(), "RegionNames: domain %u of %zu", unsigned(a_domain), m_domains.size());

    m_activeDomain = a_domain;
  }

  /**
   * @brief The active domain.
   * @return Its number, or None when none is registered.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getActiveDomain() const noexcept
  {
    return m_activeDomain;
  }

  /**
   * @brief Name a surface group of a domain.
   * @param[in] a_domain The domain.
   * @param[in] a_group  The surface group.
   * @param[in] a_name   Its name: the boundary condition's tag.
   */
  EBGEOMETRY_HOST
  void
  setSurfaceGroup(const uint32_t a_domain, const uint32_t a_group, std::string a_name)
  {
    const uint32_t active = m_activeDomain;

    this->setActiveDomain(a_domain);
    this->setSurfaceGroup(a_group, std::move(a_name));

    m_activeDomain = active;
  }

  /**
   * @brief Name a subdomain of a domain ("fluid", "solid").
   * @param[in] a_domain    The domain (below getNumDomains()).
   * @param[in] a_subdomain The subdomain (its number in the domain's Subdomains).
   * @param[in] a_name      Its name.
   */
  EBGEOMETRY_HOST
  void
  setSubdomain(const uint32_t a_domain, const uint32_t a_subdomain, std::string a_name)
  {
    EBGEOMETRY_REQUIRE(
      a_domain < m_domains.size(), "RegionNames: domain %u of %zu", unsigned(a_domain), m_domains.size());

    std::vector<std::string>& subdomains = m_domainSubdomains[a_domain];

    if (subdomains.size() <= a_subdomain) {
      subdomains.resize(a_subdomain + 1U);
    }

    subdomains[a_subdomain] = std::move(a_name);
  }

  /**
   * @brief A subdomain's name in the active domain, with its number or not: "1 solid" or "solid"; "subdomain 1".
   * @param[in] a_subdomain The subdomain.
   * @param[in] a_number    Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  subdomain(const uint32_t a_subdomain, const bool a_number = true) const
  {
    static const std::vector<std::string> none;

    return named((m_activeDomain < m_domains.size()) ? m_domainSubdomains[m_activeDomain] : none,
                 a_subdomain,
                 "subdomain",
                 a_number);
  }

  /**
   * @brief Give a name a color: a region's, a material's, a surface label's or a surface group's. Colored by that
   * kind of key, its surfaces take it; others take the palette's.
   * @param[in] a_name  The name ("steel", "wall", "inlet").
   * @param[in] a_color Its color, linear RGB in [0, 1] (as Render::Settings::m_color).
   */
  EBGEOMETRY_HOST
  void
  setColor(std::string a_name, const Vec3T<double>& a_color)
  {
    m_colors[std::move(a_name)] = a_color;
  }

  /**
   * @brief The color a key has, colored by a kind of key: by the name of the region, material, surface label (a
   * surface's too) or surface group it is.
   * @param[in]  a_coloring The kind of key.
   * @param[in]  a_key      The key.
   * @param[out] a_color    Its color, if it has one.
   * @return Whether it has one.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  bool
  colorOf(const RegionColoring a_coloring, const uint32_t a_key, Vec3T<double>& a_color) const
  {
    const std::string* name = nullptr;

    const auto from = [&name, a_key](const std::vector<std::string>& a_names) {
      name = (a_key < a_names.size()) ? &a_names[a_key] : nullptr;
    };

    switch (a_coloring) {
    case RegionColoring::Region: {
      from(m_regionNames);

      break;
    }
    case RegionColoring::Material: {
      from(m_materials);

      break;
    }
    case RegionColoring::SurfaceLabel: {
      from(m_surfaceLabels);

      break;
    }
    case RegionColoring::Surface: {
      from(m_surfaces);

      break;
    }
    case RegionColoring::SurfaceGroup: {
      from(this->groupNames());

      break;
    }
    default: {
      break;
    }
    }

    if (name == nullptr || name->empty()) {
      return false;
    }

    const auto color = m_colors.find(*name);

    if (color == m_colors.end()) {
      return false;
    }

    a_color = color->second;

    return true;
  }

  /**
   * @brief What the index says about the surfaces: every surface, its parts, the pairs of regions it may separate
   * (see possiblePieces(), coverageOf()).
   * @return The inventory; empty for names not built from an index.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const SurfaceInventory&
  getInventory() const noexcept
  {
    return m_inventory;
  }

  /**
   * @brief A region's material.
   * @param[in] a_region The region.
   * @return The material label's number, or None.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  materialOf(const uint32_t a_region) const noexcept
  {
    return (a_region < m_regionMaterials.size()) ? m_regionMaterials[a_region] : None;
  }

  /**
   * @brief Name a surface group of the active domain (with no domain registered, of those a viewer shows).
   * @param[in] a_group The group.
   * @param[in] a_name  Its name.
   */
  EBGEOMETRY_HOST
  void
  setSurfaceGroup(const uint32_t a_group, std::string a_name)
  {
    std::vector<std::string>& groups = this->groupNames();

    if (groups.size() <= a_group) {
      groups.resize(a_group + 1U);
    }

    groups[a_group] = std::move(a_name);
  }

  /**
   * @brief Forget a group's name; the names of the groups after it move down by one (as SurfaceGroups::removeSurfaceGroup()
   * numbers them).
   * @param[in] a_group The group.
   */
  EBGEOMETRY_HOST
  void
  removeSurfaceGroup(const uint32_t a_group)
  {
    std::vector<std::string>& groups = this->groupNames();

    if (a_group < groups.size()) {
      groups.erase(groups.begin() + static_cast<std::ptrdiff_t>(a_group));
    }
  }

  /**
   * @brief A region's name and material, with its number or not: "2 wall (steel)" or "wall (steel)"; the
   * background is "background", a region without a name "region 2".
   * @param[in] a_region The region.
   * @param[in] a_number Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  region(const uint32_t a_region, const bool a_number = true) const
  {
    const std::string number = std::to_string(a_region);
    const bool        named  = a_region < m_regionNames.size() && !m_regionNames[a_region].empty();

    std::string shown = named              ? (a_number ? number + " " : "") + m_regionNames[a_region]
                        : (a_region == 0U) ? (a_number ? number + " " : "") + "background"
                                           : "region " + number;

    const uint32_t material = this->materialOf(a_region);

    if (material < m_materials.size()) {
      shown += " (" + m_materials[material] + ")";
    }

    return shown;
  }

  /**
   * @brief A material's name, with its number or not.
   * @param[in] a_material The material label's number.
   * @param[in] a_number   Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  material(const uint32_t a_material, const bool a_number = true) const
  {
    return named(m_materials, a_material, "material", a_number);
  }

  /**
   * @brief A surface's name, with its number or not: its label's, or "surface 3".
   * @param[in] a_surface The surface (SurfaceRef::m_surface).
   * @param[in] a_number  Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surface(const uint32_t a_surface, const bool a_number = true) const
  {
    return named(m_surfaces, a_surface, "surface", a_number);
  }

  /**
   * @brief A surface label's name, with its number or not.
   * @param[in] a_label  The label's number.
   * @param[in] a_number Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surfaceLabel(const uint32_t a_label, const bool a_number = true) const
  {
    return named(m_surfaceLabels, a_label, "label", a_number);
  }

  /**
   * @brief A surface group's name, with its number or not: "0 inlet" or "inlet"; "surface group 0".
   * @param[in] a_group  The group.
   * @param[in] a_number Whether to put its number first.
   * @return The name.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::string
  surfaceGroup(const uint32_t a_group, const bool a_number = true) const
  {
    return named(this->groupNames(), a_group, "surface group", a_number);
  }

protected:
  /**
   * @brief The surfaces, from the index.
   */
  SurfaceInventory m_inventory;

  /**
   * @brief The domains, their names and their surface groups' names; and the active one, or None.
   */
  std::vector<DomainSetup> m_domains;

  /**
   * @brief The domains' names.
   */
  std::vector<std::string> m_domainNames;

  /**
   * @brief Each domain's subdomains' names.
   */
  std::vector<std::vector<std::string>> m_domainSubdomains;

  /**
   * @brief Each domain's surface groups' names.
   */
  std::vector<std::vector<std::string>> m_domainGroups;

  /**
   * @brief The active domain, or None.
   */
  uint32_t m_activeDomain = None;

  /**
   * @brief The surface groups' names in use: the active domain's, or, with no domain, the names' own.
   * @return The names.
   */
  [[nodiscard]] std::vector<std::string>&
  groupNames() noexcept
  {
    return (m_activeDomain < m_domains.size()) ? m_domainGroups[m_activeDomain] : m_surfaceGroups;
  }

  /**
   * @brief The surface groups' names in use.
   * @return The names.
   */
  [[nodiscard]] const std::vector<std::string>&
  groupNames() const noexcept
  {
    return (m_activeDomain < m_domains.size()) ? m_domainGroups[m_activeDomain] : m_surfaceGroups;
  }

  /**
   * @brief The regions' own names (RegionName()), empty for none; the lists below too leave the unnamed empty.
   */
  std::vector<std::string> m_regionNames;

  /**
   * @brief Each region's material label number, or None.
   */
  std::vector<uint32_t> m_regionMaterials;

  /**
   * @brief The colors given to names.
   */
  std::map<std::string, Vec3T<double>> m_colors;

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
   * @param[in] a_number Whether to put the number before a name.
   * @return The name.
   */
  [[nodiscard]] static std::string
  named(const std::vector<std::string>& a_names, const uint32_t a_index, const char* a_kind, const bool a_number)
  {
    const std::string number = std::to_string(a_index);

    return (a_index < a_names.size() && !a_names[a_index].empty()) ? (a_number ? number + " " : "") + a_names[a_index]
                                                                   : std::string(a_kind) + " " + number;
  }
};

} // namespace EBGeometry

#endif
