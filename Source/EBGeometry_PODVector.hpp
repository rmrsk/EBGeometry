// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_PODVector.hpp
 * @brief  Placement-independent POD array views for the GPU memory foundation.
 * @details A @ref EBGeometry::PODVector<T> is a trivially-copyable descriptor of a contiguous
 * array of @c T living inside a @ref EBGeometry::Pool. Crucially it stores a byte @e offset from
 * the pool's base rather than a pointer, so the same @ref EBGeometry::PODVector value resolves
 * correctly against a host base @e and against a device base after a @ref EBGeometry::Pool::mirror
 * -- with zero pointer patching. A structure built entirely from @ref EBGeometry::PODVector
 * members is therefore itself trivially copyable and survives a host-to-device @c memcpy unchanged.
 *
 * Two access styles are provided:
 * - @ref EBGeometry::PODVector::at / @ref EBGeometry::PODVector::data resolve @c base + @c offset on
 *   every call. Always correct, host and device, during build and after freeze. This is the default.
 * - @ref EBGeometry::PODVector::bind returns a @ref EBGeometry::PODSpan<T> -- a raw pointer + size
 *   for a genuinely hot inner loop. A span captures a raw pointer, so it must only be taken against
 *   a @b frozen pool (whose base can no longer move); the @ref EBGeometry::Pool freeze state machine
 *   makes a live span across a base move structurally impossible.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_PODVECTOR_HPP
#define EBGEOMETRY_PODVECTOR_HPP

// Std includes
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <type_traits>

// Our includes
#include "EBGeometry_GPU.hpp"
#include "EBGeometry_Macros.hpp"
#include "EBGeometry_Pool.hpp"

namespace EBGeometry {

/**
 * @brief Raw pointer + size view into a frozen @ref Pool block, for hot inner loops.
 * @details Obtained from @ref PODVector::bind. Because it captures a raw pointer, it is only valid
 * for the lifetime of a frozen (or mirrored) pool, whose base cannot move.
 * @tparam T Element type.
 */
template <class T>
struct PODSpan
{
  /// @brief Raw pointer to element 0 inside a frozen pool's block.
  T* m_ptr = nullptr;

  /// @brief Number of elements.
  uint32_t m_size = 0;

  /**
   * @brief Element access.
   * @param[in] a_i Element index (must be < @c m_size).
   * @return Reference to element @p a_i.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T&
  operator[](uint32_t a_i) const noexcept
  {
    EBGEOMETRY_EXPECT(a_i < m_size);

    return m_ptr[a_i];
  }

  /**
   * @brief Iterator to the first element.
   * @return Pointer to element 0.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T*
  begin() const noexcept
  {
    return m_ptr;
  }

  /**
   * @brief Iterator past the last element.
   * @return Pointer to one past element @c m_size-1.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T*
  end() const noexcept
  {
    return m_ptr + m_size;
  }

  /**
   * @brief Number of elements.
   * @return @c m_size.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  size() const noexcept
  {
    return m_size;
  }
};

static_assert(std::is_trivially_copyable_v<PODSpan<float>>, "PODSpan<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PODSpan<double>>, "PODSpan<double> must be trivially copyable");

/**
 * @brief Placement-independent descriptor of a POD array inside a @ref Pool.
 * @details Stores a byte offset (not a pointer) plus element size and capacity, so the same value
 * resolves against any pool base -- the key to zero-patch host-to-device mirroring. Trivially
 * copyable and exactly 16 bytes.
 * @tparam T Element type (must be trivially copyable, since it is device-visible storage).
 */
template <class T>
struct PODVector
{
  static_assert(std::is_trivially_copyable_v<T>, "PODVector<T>: T must be trivially copyable (device-visible storage)");

  /// @brief Byte offset of element 0 from @ref Pool::base.
  uint64_t m_offset = 0;

  /// @brief Number of constructed elements.
  uint32_t m_size = 0;

  /// @brief Reserved element slots (fixed after @ref reserveFrom; never reallocates).
  uint32_t m_capacity = 0;

  /**
   * @brief Reserve @p a_capacity slots from @p a_pool, aligned to @c alignof(T).
   * @details Sets @c m_offset and @c m_capacity and resets @c m_size to zero. A host build-phase
   *          operation.
   * @param[in,out] a_pool     Pool to reserve from (must not be frozen).
   * @param[in]     a_capacity Number of element slots to reserve.
   */
  EBGEOMETRY_HOST
  void
  reserveFrom(Pool& a_pool, uint32_t a_capacity)
  {
    m_offset   = a_pool.reserve(a_capacity, sizeof(T), alignof(T));
    m_size     = 0;
    m_capacity = a_capacity;
  }

  /**
   * @brief Mutable pointer to element 0 against @p a_base.
   * @param[in] a_base Base address of the pool holding this array.
   * @return Pointer to element 0.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T*
  data(void* a_base) const noexcept
  {
    return reinterpret_cast<T*>(static_cast<unsigned char*>(a_base) + m_offset);
  }

  /**
   * @brief Const pointer to element 0 against @p a_base.
   * @param[in] a_base Base address of the pool holding this array.
   * @return Const pointer to element 0.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T*
  data(const void* a_base) const noexcept
  {
    return reinterpret_cast<const T*>(static_cast<const unsigned char*>(a_base) + m_offset);
  }

  /**
   * @brief Mutable element access against @p a_base.
   * @param[in] a_base Base address of the pool holding this array.
   * @param[in] a_i    Element index (must be < @c m_size).
   * @return Reference to element @p a_i.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  T&
  at(void* a_base, uint32_t a_i) const noexcept
  {
    EBGEOMETRY_EXPECT(a_i < m_size);

    return this->data(a_base)[a_i];
  }

  /**
   * @brief Const element access against @p a_base.
   * @param[in] a_base Base address of the pool holding this array.
   * @param[in] a_i    Element index (must be < @c m_size).
   * @return Const reference to element @p a_i.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  const T&
  at(const void* a_base, uint32_t a_i) const noexcept
  {
    EBGEOMETRY_EXPECT(a_i < m_size);

    return this->data(a_base)[a_i];
  }

  /**
   * @brief Number of constructed elements.
   * @return @c m_size.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  uint32_t
  size() const noexcept
  {
    return m_size;
  }

  /**
   * @brief Whether the array is empty.
   * @return True if @c m_size == 0.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  bool
  empty() const noexcept
  {
    return m_size == 0;
  }

  /**
   * @brief Append one element into the pre-reserved capacity.
   * @details No reallocation ever: appending past capacity is an invariant violation. A host
   *          build-phase operation.
   * @param[in] a_base  Base address of the pool holding this array.
   * @param[in] a_value Value to append.
   */
  EBGEOMETRY_HOST
  void
  push_back(void* a_base, const T& a_value)
  {
    EBGEOMETRY_EXPECT(m_size < m_capacity); // the no-realloc invariant

    this->data(a_base)[m_size++] = a_value;
  }

  /**
   * @brief Bulk-fill the array from a contiguous host source.
   * @details Copies @p a_count elements and sets @c m_size to @p a_count. A host build-phase
   *          operation (the finalize path).
   * @param[in] a_base  Base address of the pool holding this array.
   * @param[in] a_src   Source array of at least @p a_count elements.
   * @param[in] a_count Number of elements to copy (must be <= @c m_capacity).
   */
  EBGEOMETRY_HOST
  void
  assign(void* a_base, const T* a_src, uint32_t a_count)
  {
    EBGEOMETRY_EXPECT(a_count <= m_capacity);

    std::memcpy(this->data(a_base), a_src, static_cast<size_t>(a_count) * sizeof(T));

    m_size = a_count;
  }

  /**
   * @brief Bind a mutable raw-pointer view for a hot loop.
   * @details Call only against a frozen pool; see the class documentation.
   * @param[in] a_base Base address of the (frozen) pool holding this array.
   * @return A @ref PODSpan over the array.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  PODSpan<T>
  bind(void* a_base) const noexcept
  {
    return PODSpan<T>{this->data(a_base), m_size};
  }

  /**
   * @brief Bind a const raw-pointer view for a hot loop.
   * @details Call only against a frozen pool; see the class documentation.
   * @param[in] a_base Base address of the (frozen) pool holding this array.
   * @return A @ref PODSpan of const elements over the array.
   */
  [[nodiscard]] EBGEOMETRY_HOST_DEVICE
  PODSpan<const T>
  bind(const void* a_base) const noexcept
  {
    return PODSpan<const T>{this->data(a_base), m_size};
  }
};

static_assert(sizeof(PODVector<float>) == 16, "PODVector layout must be 8 (offset) + 4 (size) + 4 (capacity)");
static_assert(sizeof(PODVector<double>) == 16, "PODVector layout must be 8 (offset) + 4 (size) + 4 (capacity)");
static_assert(std::is_trivially_copyable_v<PODVector<float>>, "PODVector<float> must be trivially copyable");
static_assert(std::is_trivially_copyable_v<PODVector<double>>, "PODVector<double> must be trivially copyable");

} // namespace EBGeometry

#endif // EBGEOMETRY_PODVECTOR_HPP
