// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_RegionNeighborhood.hpp
 * @brief  RegionNeighborhood: some regions of an expression with their neighbors, as a smaller expression (host only).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_REGIONNEIGHBORHOOD_HPP
#define EBGEOMETRY_REGIONNEIGHBORHOOD_HPP

// Std includes
#include <cstdint>
#include <vector>

// Our includes
#include "EBGeometry_DomainSetup.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_LabelId.hpp"

namespace EBGeometry {

/**
 * @brief Some regions of an expression with their neighbors: a smaller expression in which those regions have the
 * same solids and the same surfaces, with the same regions on both sides of each, numbered anew (see
 * regionNeighborhood()).
 * @details What a domain computes on, compiled on its own: a tape of object() gives the regions of the neighborhood
 * the same signs (inside, outside) as the whole expression's tape gives them, and their surfaces the same regions,
 * materials and labels on both sides, so a domain built for it from setup() has the same subdomains and surface
 * groups. Away from those regions, the values of their functions may differ (a dropped region no longer cuts a kept
 * one there), never their signs. The regions, surfaces and labels are numbered as ExprIndex numbers those of
 * object(); wholeRegion() and region() map the regions both ways. Host only, a plain value.
 * @tparam T Floating-point precision.
 */
template <class T>
class RegionNeighborhood
{
public:
  /**
   * @brief What region() gives for a region that was dropped.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief No empty neighborhood: see regionNeighborhood().
   */
  RegionNeighborhood() = delete;

  /**
   * @brief The smaller expression, to compile into a tape.
   * @return It.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const Expr<T>&
  object() const noexcept;

  /**
   * @brief How many regions it has, the background (region 0) included.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumRegions() const noexcept;

  /**
   * @brief The whole expression's region that one of its regions is.
   * @param[in] a_region Its region (below getNumRegions()).
   * @return The whole expression's region; 0 for the background.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  wholeRegion(uint32_t a_region) const;

  /**
   * @brief Its region that a region of the whole expression is.
   * @param[in] a_wholeRegion A region of the whole expression.
   * @return Its region, or None if that region was dropped.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  region(uint32_t a_wholeRegion) const;

  /**
   * @brief Its surface that a surface of the whole expression is (ExprIndex::getFirstSurface() numbering).
   * @param[in] a_wholeSurface A surface of the whole expression.
   * @return Its surface, or None if the leaf of that surface was dropped.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  surface(uint32_t a_wholeSurface) const;

  /**
   * @brief The whole expression's surface that one of its surfaces is.
   * @param[in] a_surface Its surface.
   * @return The whole expression's surface.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  wholeSurface(uint32_t a_surface) const;

  /**
   * @brief A setup written for the whole expression, translated to this one: regions through the map, materials and
   * surface labels by their names, surfaces through the leaves kept.
   * @details A selector that names a region, a material, a label or a surface that was dropped selects nothing here,
   * and is left out; every other selector selects the same surfaces of the neighborhood's regions as before. The
   * subdomains and the surface groups keep their numbers. Every region of the setup's subdomains must be in the
   * neighborhood (build it from them, as Language::Model::domainObject() does): aborts with a message otherwise, in
   * every build.
   * @param[in] a_wholeSetup A setup for the whole expression.
   * @return The setup for object().
   */
  [[nodiscard]] EBGEOMETRY_HOST
  DomainSetup
  setup(const DomainSetup& a_wholeSetup) const;

private:
  template <class U>
  friend RegionNeighborhood<U>
  regionNeighborhood(const Expr<U>& a_expr, const std::vector<uint32_t>& a_regions);

  /**
   * @brief Hold a smaller expression.
   * @param[in] a_object The smaller expression.
   */
  EBGEOMETRY_HOST
  explicit RegionNeighborhood(Expr<T> a_object) : m_object(std::move(a_object))
  {}

  /**
   * @brief The smaller expression.
   */
  Expr<T> m_object;

  /**
   * @brief The whole expression's region of each of its regions.
   */
  std::vector<uint32_t> m_wholeRegion;

  /**
   * @brief Its region of each region of the whole expression, or None.
   */
  std::vector<uint32_t> m_region;

  /**
   * @brief The whole expression's surface of each of its surfaces.
   */
  std::vector<uint32_t> m_wholeSurface;

  /**
   * @brief Its surface of each surface of the whole expression, or None.
   */
  std::vector<uint32_t> m_surface;

  /**
   * @brief Its surface label of each surface label of the whole expression (by name), or None.
   */
  std::vector<uint32_t> m_surfaceLabel;

  /**
   * @brief Its material of each material of the whole expression (by name), or None.
   */
  std::vector<uint32_t> m_material;
};

/**
 * @brief Some regions of an expression and their neighbors: the regions that may touch them, as a smaller expression.
 * @details Two regions are neighbors when their bounding boxes meet, touching faces and edges included: a region
 * whose box meets the box of a region given is kept; every other operand of the Overlays is dropped (a nested Overlay
 * left with none, whole). Maps and clipping intersections above the Overlays are kept. A box is a conservative stand-in
 * for a region: it never drops a region that touches one given (so every surface of theirs keeps the regions on both
 * of its sides), but it may keep one that does not, when their boxes meet while the regions do not (a region in the
 * hollow of an L-shaped one, or whose box is cut away by later regions). A region's box is its operand's, carried up
 * through the maps above it and clipped by the clipping intersections' boxes. Regions in a FiniteRepetition are kept
 * whole, with it. Host only.
 * @tparam T Floating-point precision.
 * @param[in] a_expr    An expression; one with no Overlay has one region, the whole object, and is its own
 *                      neighborhood.
 * @param[in] a_regions Regions of the expression (ExprIndex numbers), at least one; aborts with a message otherwise,
 *                      in every build. The background (region 0) meets every region: with it, every region is kept.
 * @return The neighborhood.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST
RegionNeighborhood<T>
regionNeighborhood(const Expr<T>& a_expr, const std::vector<uint32_t>& a_regions);

} // namespace EBGeometry

#include "EBGeometry_RegionNeighborhoodImplem.hpp"

#endif
