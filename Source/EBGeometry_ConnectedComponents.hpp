// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: BSD-3-Clause

/**
 * @file   EBGeometry_ConnectedComponents.hpp
 * @brief  connectedComponents(): the connected components of a tape's regions, or of groups of them (host only).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CONNECTEDCOMPONENTS_HPP
#define EBGEOMETRY_CONNECTEDCOMPONENTS_HPP

// Std includes
#include <cstdint>
#include <memory>
#include <vector>

// Our includes
#include "EBGeometry_BoundingVolumes.hpp"
#include "EBGeometry_Domain.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Tape.hpp"
#include "EBGeometry_Vec.hpp"

namespace EBGeometry {

template <class T>
class ConnectedComponents;

namespace ComponentsDetail {

template <class T>
struct Cells;

/**
 * @brief The components of a search's cells for a grouping of the regions.
 * @details Internal helper; not part of the public API.
 * @tparam T Floating-point precision.
 * @param[in] a_cells     The cells.
 * @param[in] a_groupOf   Each region's group.
 * @param[in] a_numGroups The number of groups, at least one more than the largest given (0: just that).
 * @return The components.
 */
template <class T>
ConnectedComponents<T>
label(const std::shared_ptr<const Cells<T>>& a_cells, const std::vector<uint32_t>& a_groupOf, uint32_t a_numGroups);

} // namespace ComponentsDetail

/**
 * @brief How connectedComponents() resolves the space it searches.
 * @tparam T Floating-point precision.
 */
template <class T>
struct ComponentOptions
{
  /**
   * @brief The resolution h, the size of the smallest cells (those near a surface), positive.
   * @details Two parts of a region joined by a neck thinner than h, or separated by a wall thinner than h, may count
   * as one component; a part thinner than h may be missed. With h the cell size of the solver the components are for,
   * they are the components that solver sees.
   */
  T m_resolution = T(0);

  /**
   * @brief The box searched: the space a solver computes in, say. Empty (the default): the tape's bounding box grown by
   * two cells on every side, so that the outside is one shell around the object. Either is grown at its high corner to
   * a whole number of cells of size h.
   */
  BoundingVolumes::AABBT<T> m_box = BoundingVolumes::AABBT<T>();
};

/**
 * @brief One connected component: a connected part of space where every point is in one region (or one group of
 * regions).
 * @tparam T Floating-point precision.
 */
template <class T>
struct ConnectedComponent
{
  /**
   * @brief What it is a component of: the region, or the group of regions (a material, a subdomain).
   */
  uint32_t m_group = 0;

  /**
   * @brief Its volume within the box searched, approximate: the error is of the order of its surface's area times h.
   */
  T m_volume = T(0);

  /**
   * @brief A box around it, approximate: within h of the smallest one.
   */
  BoundingVolumes::AABBT<T> m_box;

  /**
   * @brief A point in it, as far from its surface as the search found one: what a solver asks for to name it.
   */
  Vec3T<T> m_seed = Vec3T<T>::zeros();

  /**
   * @brief Whether it reaches the boundary of the box searched. A component that does not is enclosed by others: a
   * cavity, or a floating body.
   */
  bool m_reachesBox = false;
};

/**
 * @brief The connected components of the regions of a tape, or of groups of them (see connectedComponents()): how
 * many each has, and for each its volume, box, a point in it, whether it reaches the box searched, and the
 * components it touches.
 * @details Components are numbered by group, and within a group those reaching the box first, then by decreasing
 * volume. A host-only value, which holds no reference to the tape.
 * @tparam T Floating-point precision.
 */
template <class T>
class ConnectedComponents
{
public:
  /**
   * @brief What componentAt() gives for a point outside the box searched.
   */
  static constexpr uint32_t None = 0xFFFFFFFFU;

  /**
   * @brief How many components there are.
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumComponents() const noexcept;

  /**
   * @brief A component.
   * @param[in] a_component The component (below getNumComponents()); aborts with a message otherwise, in every build.
   * @return It.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const ConnectedComponent<T>&
  component(uint32_t a_component) const;

  /**
   * @brief How many groups there are: the regions of the tape, or the groups given (one more than the largest).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  getNumGroups() const noexcept;

  /**
   * @brief The components of a group, those reaching the box first, then by decreasing volume.
   * @param[in] a_group The group (below getNumGroups()); aborts with a message otherwise, in every build.
   * @return The components, none for a group found nowhere in the box.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  std::vector<uint32_t>
  componentsOf(uint32_t a_group) const;

  /**
   * @brief The components that touch a component (they share a face of the search's cells), in increasing order:
   * components of other groups.
   * @param[in] a_component The component (below getNumComponents()); aborts with a message otherwise, in every build.
   * @return Its neighbors.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const std::vector<uint32_t>&
  neighbors(uint32_t a_component) const;

  /**
   * @brief The component holding a point: that of the search's cell holding it. Within h of a surface, where the
   * cells are smallest, it can be the component across that surface.
   * @param[in] a_point The point.
   * @return The component, or None for a point outside the box searched.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint32_t
  componentAt(const Vec3T<T>& a_point) const noexcept;

  /**
   * @brief The resolution searched with (ComponentOptions::m_resolution).
   * @return h.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  T
  resolution() const noexcept;

  /**
   * @brief The box searched, grown to a whole number of cells.
   * @return The box.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  const BoundingVolumes::AABBT<T>&
  box() const noexcept;

  /**
   * @brief How many cells the search kept (its cost: one evaluation of every region's function per cell, and about a
   * seventh more for the cells refined).
   * @return The count.
   */
  [[nodiscard]] EBGEOMETRY_HOST
  uint64_t
  getNumCells() const noexcept;

private:
  /**
   * @brief No components: only connectedComponents() makes them.
   */
  ConnectedComponents() = default;

  template <class U>
  friend ConnectedComponents<U>
  ComponentsDetail::label(const std::shared_ptr<const ComponentsDetail::Cells<U>>& a_cells,
                          const std::vector<uint32_t>&                             a_groupOf,
                          uint32_t                                                 a_numGroups);

  /**
   * @brief The search's cells, which the components of several groupings can share.
   */
  std::shared_ptr<const ComponentsDetail::Cells<T>> m_cells;

  /**
   * @brief Each cell's component.
   */
  std::vector<uint32_t> m_componentOfCell;

  /**
   * @brief The components.
   */
  std::vector<ConnectedComponent<T>> m_components;

  /**
   * @brief Each component's neighbors.
   */
  std::vector<std::vector<uint32_t>> m_neighbors;

  /**
   * @brief The components of group g are m_firstOfGroup[g] up to m_firstOfGroup[g + 1].
   */
  std::vector<uint32_t> m_firstOfGroup;
};

/**
 * @brief The connected components of every region of a tape, the outside (region 0) included: its enclosed cavities,
 * and the separate bodies of a region.
 * @details Searches the box with an adaptive octree. A cell where one region's function is below minus the cell's
 * half-diagonal lies wholly in that region (the functions are distance bounds), and is kept whole; every other cell
 * is refined, down to cells of size h, each in the region of its center. Cells of the same region that share a face
 * are joined into components. Exact for a model whose features are all much larger than h; see
 * ComponentOptions::m_resolution for what h may merge or miss. The cost grows with the area of the surfaces over h
 * squared. Aborts with a message, in every build, for a tape whose value is not a distance (DistanceQuality::
 * NotADistance), for a resolution that is not positive, for an empty or unbounded box (the tape's own, when no box is
 * given), and for a box more than 2^19 cells long. Host only; the tape's pool must be readable on the host.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitives and features.
 * @param[in] a_tape    The tape.
 * @param[in] a_options The resolution and the box.
 * @return The components; their groups are the regions.
 */
template <class T, class... UserPrimitives>
[[nodiscard]] EBGEOMETRY_HOST
ConnectedComponents<T>
connectedComponents(const Tape<T, UserPrimitives...>& a_tape, const ComponentOptions<T>& a_options);

/**
 * @brief The connected components of groups of a tape's regions: regions of one group that touch form one component
 * (materials: a_groupOf[r] the material of region r, say).
 * @details As connectedComponents(tape, options), the cells of regions of one group joined. Aborts with a message, in
 * every build, when a_groupOf does not give one group per region.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitives and features.
 * @param[in] a_tape    The tape.
 * @param[in] a_groupOf Each region's group, one per region of the tape.
 * @param[in] a_options The resolution and the box.
 * @return The components; their groups are those given.
 */
template <class T, class... UserPrimitives>
[[nodiscard]] EBGEOMETRY_HOST
ConnectedComponents<T>
connectedComponents(const Tape<T, UserPrimitives...>& a_tape,
                    const std::vector<uint32_t>&      a_groupOf,
                    const ComponentOptions<T>&        a_options);

/**
 * @brief The connected components of a domain's subdomains, and of the regions it excludes: group s is subdomain s,
 * group getNumSubdomainFunctions() - 1 the excluded regions (Domain::Excluded).
 * @details As connectedComponents(tape, groupOf, options), with each region's subdomain as its group. The tables
 * pool must be readable on the host.
 * @tparam T              Floating-point precision.
 * @tparam UserPrimitives The tape's user primitives and features.
 * @param[in] a_domain  The domain.
 * @param[in] a_options The resolution and the box.
 * @return The components.
 */
template <class T, class... UserPrimitives>
[[nodiscard]] EBGEOMETRY_HOST
ConnectedComponents<T>
connectedComponents(const Domain<T, UserPrimitives...>& a_domain, const ComponentOptions<T>& a_options);

} // namespace EBGeometry

#include "EBGeometry_ConnectedComponentsImplem.hpp"

#endif
