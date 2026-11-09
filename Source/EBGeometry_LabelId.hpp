// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_LabelId.hpp
 * @brief  LabelId: the number of a label, typed by what it labels (SurfaceLabelId, MaterialLabelId).
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_LABELID_HPP
#define EBGEOMETRY_LABELID_HPP

// Std includes
#include <cstdint>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief The number of a label, typed by what it labels, so that a number of one kind cannot index a table of
 * another: a surface label's number does not compile where a material's is expected.
 * @details A trivially copyable wrapper of a 32-bit number, usable on the host and the device. The labels of each
 * kind are numbered from 0 in the order their names first appear in the expression (see ExprIndex); value() is the
 * number, for indexing an array of your own. None is no label.
 * @tparam Tag What the label labels (SurfaceLabelTag, MaterialLabelTag).
 */
template <class Tag>
struct LabelId
{
  /**
   * @brief The number of no label.
   */
  static constexpr uint32_t NoneValue = 0xFFFFFFFFU;

  /**
   * @brief The number.
   */
  uint32_t m_value = NoneValue;

  /**
   * @brief No label.
   */
  constexpr LabelId() noexcept = default;

  /**
   * @brief A label's number.
   * @param[in] a_value The number (NoneValue for none).
   */
  EBGEOMETRY_HOST_DEVICE
  constexpr explicit LabelId(const uint32_t a_value) noexcept : m_value(a_value)
  {}

  /**
   * @brief No label.
   * @return The id of no label.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr LabelId
  none() noexcept
  {
    return LabelId();
  }

  /**
   * @brief The number, for indexing an array of your own.
   * @return The number; NoneValue for no label, which no table of labels reaches.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr uint32_t
  value() const noexcept
  {
    return m_value;
  }

  /**
   * @brief Whether this is no label.
   * @return True for none.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr bool
  isNone() const noexcept
  {
    return m_value == NoneValue;
  }

  /**
   * @brief Equality.
   * @param[in] a_other Another id of the same kind.
   * @return True if the numbers are equal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr bool
  operator==(const LabelId& a_other) const noexcept
  {
    return m_value == a_other.m_value;
  }

  /**
   * @brief Inequality.
   * @param[in] a_other Another id of the same kind.
   * @return True if the numbers differ.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr bool
  operator!=(const LabelId& a_other) const noexcept
  {
    return m_value != a_other.m_value;
  }
};

/**
 * @brief The tag of surface labels (SurfaceLabel()).
 */
struct SurfaceLabelTag
{
};

/**
 * @brief The number of a surface label: SurfaceLabel() names surfaces, and the tape and ExprIndex number the names.
 */
using SurfaceLabelId = LabelId<SurfaceLabelTag>;

static_assert(std::is_trivially_copyable_v<SurfaceLabelId> && sizeof(SurfaceLabelId) == sizeof(uint32_t),
              "SurfaceLabelId must be a plain 32-bit value, for copies to a device");

/**
 * @brief The tag of material labels (MaterialLabel()).
 */
struct MaterialLabelTag
{
};

/**
 * @brief The number of a material label: MaterialLabel() names the material of a region (see Overlay()), and
 * ExprIndex numbers the names.
 */
using MaterialLabelId = LabelId<MaterialLabelTag>;

static_assert(std::is_trivially_copyable_v<MaterialLabelId> && sizeof(MaterialLabelId) == sizeof(uint32_t),
              "MaterialLabelId must be a plain 32-bit value, for copies to a device");

} // namespace EBGeometry

#endif
