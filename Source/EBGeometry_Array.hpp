// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_Array.hpp
 * @brief  A fixed-size array that compiles for both host and device.
 * @details std::array's member functions are constexpr host functions, which nvcc rejects in device
 * code unless every translation unit is compiled with --expt-relaxed-constexpr. The library uses
 * this instead, everywhere under Source/; Scripts/CheckDeviceMath.py enforces that.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_ARRAY_HPP
#define EBGEOMETRY_ARRAY_HPP

#include <cstddef>

#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief Fixed-size array with host- and device-callable members.
 * @details An aggregate, like std::array: `Array<int, 3> a = {1, 2, 3};` works, a default-constructed
 * Array of a scalar type is uninitialized, and `Array<int, 3> a{};` is zeroed. It is trivially
 * copyable whenever T is.
 * @tparam T Element type.
 * @tparam N Number of elements, at least 1.
 */
template <class T, std::size_t N>
struct Array
{
  static_assert(N > 0, "Array<T, N> requires N > 0");

  /**
   * @brief The elements. Public so that the type is an aggregate; use operator[] or data().
   */
  T m_data[N];

  /**
   * @brief Element access.
   * @param[in] a_i Index, which must be less than N.
   * @return Reference to element @p a_i.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr T&
  operator[](const std::size_t a_i) noexcept
  {
    return m_data[a_i];
  }

  /**
   * @brief Element access.
   * @param[in] a_i Index, which must be less than N.
   * @return Const reference to element @p a_i.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr const T&
  operator[](const std::size_t a_i) const noexcept
  {
    return m_data[a_i];
  }

  /**
   * @brief Number of elements.
   * @return N.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  static constexpr std::size_t
  size() noexcept
  {
    return N;
  }

  /**
   * @brief Pointer to the first element.
   * @return The address of the element storage.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr T*
  data() noexcept
  {
    return m_data;
  }

  /**
   * @brief Pointer to the first element.
   * @return The address of the element storage.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr const T*
  data() const noexcept
  {
    return m_data;
  }

  /**
   * @brief Iterator to the first element.
   * @return Pointer to the first element.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr T*
  begin() noexcept
  {
    return m_data;
  }

  /**
   * @brief Iterator to the first element.
   * @return Pointer to the first element.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr const T*
  begin() const noexcept
  {
    return m_data;
  }

  /**
   * @brief Iterator past the last element.
   * @return Pointer past the last element.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr T*
  end() noexcept
  {
    return m_data + N;
  }

  /**
   * @brief Iterator past the last element.
   * @return Pointer past the last element.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr const T*
  end() const noexcept
  {
    return m_data + N;
  }

  /**
   * @brief Set every element to a value.
   * @param[in] a_value Value to assign.
   */
  EBGEOMETRY_HOST_DEVICE
  constexpr void
  fill(const T& a_value) noexcept
  {
    for (std::size_t i = 0; i < N; i++) {
      m_data[i] = a_value;
    }
  }

  /**
   * @brief Element-wise equality.
   * @param[in] a_other Array to compare with.
   * @return True if every element compares equal.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr bool
  operator==(const Array& a_other) const noexcept
  {
    for (std::size_t i = 0; i < N; i++) {
      if (!(m_data[i] == a_other.m_data[i])) {
        return false;
      }
    }

    return true;
  }

  /**
   * @brief Element-wise inequality.
   * @param[in] a_other Array to compare with.
   * @return True if any element differs.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  constexpr bool
  operator!=(const Array& a_other) const noexcept
  {
    return !(*this == a_other);
  }
};

} // namespace EBGeometry

#endif
