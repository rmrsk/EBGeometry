// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_SIMD.hpp
 * @brief  Thin wrappers over SSE4.1, AVX and AVX-512 registers, so that one kernel template serves
 * every SIMD width and the scalar path.
 * @details A kernel written against these operations -- arithmetic operators, comparisons that give
 * a mask, select(), sqrt() and abs() -- compiles for a plain float or double (one lane, host and
 * device) and for each SIMD::Pack the compiler's flags provide. The packs exist only in a host
 * compilation pass: device code sees NativeWidth() == 1 and takes the scalar instantiation.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_SIMD_HPP
#define EBGEOMETRY_SIMD_HPP

// Std includes
#include <cmath>
#include <cstddef>
#include <type_traits>

#if (defined(__SSE4_1__) || defined(__AVX__) || defined(__AVX512F__)) && !defined(EBGEOMETRY_DEVICE_COMPILE)
#include <immintrin.h>
#endif

// Our includes
#include "EBGeometry_GPU.hpp"

namespace EBGeometry {

/**
 * @brief SIMD register wrappers and the scalar operations that mirror them.
 */
namespace SIMD {

/**
 * @brief Pick @p a_ifTrue where @p a_mask holds, else @p a_ifFalse (scalar form).
 * @tparam T Floating-point type.
 * @param[in] a_mask    Condition.
 * @param[in] a_ifTrue  Value where the condition holds.
 * @param[in] a_ifFalse Value where it does not.
 * @return The chosen value.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline T
select(const bool a_mask, const T a_ifTrue, const T a_ifFalse) noexcept
{
  return a_mask ? a_ifTrue : a_ifFalse;
}

/**
 * @brief Whether both conditions hold (scalar form).
 * @param[in] a_lhs One condition.
 * @param[in] a_rhs The other.
 * @return a_lhs && a_rhs.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline bool
both(const bool a_lhs, const bool a_rhs) noexcept
{
  return a_lhs && a_rhs;
}

/**
 * @brief Square root (scalar form).
 * @param[in] a_x Argument.
 * @return The square root.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline float
sqrt(const float a_x) noexcept
{
  return std::sqrt(a_x);
}

/**
 * @brief Square root (scalar form).
 * @param[in] a_x Argument.
 * @return The square root.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline double
sqrt(const double a_x) noexcept
{
  return std::sqrt(a_x);
}

/**
 * @brief Absolute value (scalar form).
 * @param[in] a_x Argument.
 * @return The absolute value.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline float
abs(const float a_x) noexcept
{
  return std::abs(a_x);
}

/**
 * @brief Absolute value (scalar form).
 * @param[in] a_x Argument.
 * @return The absolute value.
 */
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
inline double
abs(const double a_x) noexcept
{
  return std::abs(a_x);
}

/**
 * @brief The widest SIMD register, in lanes of T, that this compilation pass has: 16 floats or 8
 * doubles under AVX-512F, 8 or 4 under AVX, 4 or 2 under SSE4.1, and 1 otherwise and in every device
 * pass.
 * @tparam T Floating-point type.
 * @return The lane count.
 */
template <class T>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr size_t
WidestPack() noexcept
{
  static_assert(std::is_same_v<T, float> || std::is_same_v<T, double>, "SIMD: T must be float or double");

#if defined(EBGEOMETRY_DEVICE_COMPILE)
  return 1;
#elif defined(__AVX512F__)
  return 64 / sizeof(T);
#elif defined(__AVX__)
  return 32 / sizeof(T);
#elif defined(__SSE4_1__)
  return 16 / sizeof(T);
#else
  return 1;
#endif
}

/**
 * @brief The lane count to evaluate a group of @p W values of T with: the widest register this
 * compilation pass has whose lane count divides @p W, or 1 (the scalar path) if none does.
 * @tparam T Floating-point type.
 * @tparam W Group width.
 * @return A lane count N for which Pack<T, N> exists, or 1.
 */
template <class T, size_t W>
[[nodiscard]] EBGEOMETRY_HOST_DEVICE
constexpr size_t
NativeWidth() noexcept
{
  // The narrowest register is 16 bytes (SSE), so no pack is narrower than that.
  for (size_t n = WidestPack<T>(); n * sizeof(T) >= 16; n /= 2) {
    if (W % n == 0) {
      return n;
    }
  }

  return 1;
}

template <class T, size_t N>
struct Pack;

/**
 * @brief The scalar type of a scalar or pack type: T for T, and T for Pack<T, N>.
 * @tparam V Scalar or pack type.
 */
template <class V>
struct Scalar
{
  /**
   * @brief The scalar type.
   */
  using type = V;
};

/**
 * @brief The scalar type of Pack<T, N>.
 * @tparam T Floating-point type.
 * @tparam N Lane count.
 */
template <class T, size_t N>
struct Scalar<Pack<T, N>>
{
  /**
   * @brief The scalar type.
   */
  using type = T;
};

/**
 * @brief N lanes of T in one SIMD register. Defined only for the (T, N) pairs the compiler's flags
 * provide (see NativeWidth()), and never in a device pass.
 * @details Each specialization holds the register and provides construction from one value
 * (broadcast), load() from and store() to memory aligned to the register size, the four arithmetic
 * operators, and the comparisons <, > and >=, which give a mask. select(), both(), sqrt() and abs()
 * are free functions, as for the scalar types. Every comparison is ordered: it is false where either side is
 * NaN, as the scalar comparison is.
 * @tparam T Floating-point type.
 * @tparam N Lane count.
 */
template <class T, size_t N>
struct Pack;

#if !defined(EBGEOMETRY_DEVICE_COMPILE)

#if defined(__SSE4_1__)

/**
 * @brief Mask of a Pack<float, 4>: a lane is all ones where the comparison held.
 */
struct MaskF4
{
  /**
   * @brief The mask, as a register.
   */
  __m128 m;

  /// @cond
  friend inline MaskF4
  both(const MaskF4 a_lhs, const MaskF4 a_rhs) noexcept
  {
    return MaskF4{_mm_and_ps(a_lhs.m, a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Mask of a Pack<double, 2>: a lane is all ones where the comparison held.
 */
struct MaskD2
{
  /**
   * @brief The mask, as a register.
   */
  __m128d m;

  /// @cond
  friend inline MaskD2
  both(const MaskD2 a_lhs, const MaskD2 a_rhs) noexcept
  {
    return MaskD2{_mm_and_pd(a_lhs.m, a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Four floats in an SSE register.
 */
template <>
struct Pack<float, 4>
{
  /**
   * @brief The register.
   */
  __m128 v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m128 a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const float a_x) noexcept : v(_mm_set1_ps(a_x))
  {}

  /**
   * @brief Load four lanes from 16-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const float* a_p) noexcept
  {
    return Pack(_mm_load_ps(a_p));
  }

  /**
   * @brief Store the four lanes to 16-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(float* a_p) const noexcept
  {
    _mm_store_ps(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm_add_ps(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm_sub_ps(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm_mul_ps(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm_div_ps(a.v, b.v)); }
  friend MaskF4 operator<(const Pack a, const Pack b) noexcept { return MaskF4{_mm_cmplt_ps(a.v, b.v)}; }
  friend MaskF4 operator>(const Pack a, const Pack b) noexcept { return MaskF4{_mm_cmpgt_ps(a.v, b.v)}; }
  friend MaskF4 operator>=(const Pack a, const Pack b) noexcept { return MaskF4{_mm_cmpge_ps(a.v, b.v)}; }
  friend Pack select(const MaskF4 m, const Pack a, const Pack b) noexcept { return Pack(_mm_blendv_ps(b.v, a.v, m.m)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm_sqrt_ps(a.v)); }
  friend Pack abs(const Pack a) noexcept { return Pack(_mm_andnot_ps(_mm_set1_ps(-0.0F), a.v)); }
  /// @endcond
  // clang-format on
};

/**
 * @brief Two doubles in an SSE register.
 */
template <>
struct Pack<double, 2>
{
  /**
   * @brief The register.
   */
  __m128d v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m128d a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const double a_x) noexcept : v(_mm_set1_pd(a_x))
  {}

  /**
   * @brief Load two lanes from 16-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const double* a_p) noexcept
  {
    return Pack(_mm_load_pd(a_p));
  }

  /**
   * @brief Store the two lanes to 16-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(double* a_p) const noexcept
  {
    _mm_store_pd(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm_add_pd(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm_sub_pd(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm_mul_pd(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm_div_pd(a.v, b.v)); }
  friend MaskD2 operator<(const Pack a, const Pack b) noexcept { return MaskD2{_mm_cmplt_pd(a.v, b.v)}; }
  friend MaskD2 operator>(const Pack a, const Pack b) noexcept { return MaskD2{_mm_cmpgt_pd(a.v, b.v)}; }
  friend MaskD2 operator>=(const Pack a, const Pack b) noexcept { return MaskD2{_mm_cmpge_pd(a.v, b.v)}; }
  friend Pack select(const MaskD2 m, const Pack a, const Pack b) noexcept { return Pack(_mm_blendv_pd(b.v, a.v, m.m)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm_sqrt_pd(a.v)); }
  friend Pack abs(const Pack a) noexcept { return Pack(_mm_andnot_pd(_mm_set1_pd(-0.0), a.v)); }
  /// @endcond
  // clang-format on
};

#endif

#if defined(__AVX__)

/**
 * @brief Mask of a Pack<float, 8>: a lane is all ones where the comparison held.
 */
struct MaskF8
{
  /**
   * @brief The mask, as a register.
   */
  __m256 m;

  /// @cond
  friend inline MaskF8
  both(const MaskF8 a_lhs, const MaskF8 a_rhs) noexcept
  {
    return MaskF8{_mm256_and_ps(a_lhs.m, a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Mask of a Pack<double, 4>: a lane is all ones where the comparison held.
 */
struct MaskD4
{
  /**
   * @brief The mask, as a register.
   */
  __m256d m;

  /// @cond
  friend inline MaskD4
  both(const MaskD4 a_lhs, const MaskD4 a_rhs) noexcept
  {
    return MaskD4{_mm256_and_pd(a_lhs.m, a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Eight floats in an AVX register.
 */
template <>
struct Pack<float, 8>
{
  /**
   * @brief The register.
   */
  __m256 v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m256 a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const float a_x) noexcept : v(_mm256_set1_ps(a_x))
  {}

  /**
   * @brief Load eight lanes from 32-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const float* a_p) noexcept
  {
    return Pack(_mm256_load_ps(a_p));
  }

  /**
   * @brief Store the eight lanes to 32-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(float* a_p) const noexcept
  {
    _mm256_store_ps(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm256_add_ps(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm256_sub_ps(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm256_mul_ps(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm256_div_ps(a.v, b.v)); }
  friend MaskF8 operator<(const Pack a, const Pack b) noexcept { return MaskF8{_mm256_cmp_ps(a.v, b.v, _CMP_LT_OQ)}; }
  friend MaskF8 operator>(const Pack a, const Pack b) noexcept { return MaskF8{_mm256_cmp_ps(a.v, b.v, _CMP_GT_OQ)}; }
  friend MaskF8 operator>=(const Pack a, const Pack b) noexcept { return MaskF8{_mm256_cmp_ps(a.v, b.v, _CMP_GE_OQ)}; }
  friend Pack select(const MaskF8 m, const Pack a, const Pack b) noexcept { return Pack(_mm256_blendv_ps(b.v, a.v, m.m)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm256_sqrt_ps(a.v)); }
  friend Pack abs(const Pack a) noexcept { return Pack(_mm256_andnot_ps(_mm256_set1_ps(-0.0F), a.v)); }
  /// @endcond
  // clang-format on
};

/**
 * @brief Four doubles in an AVX register.
 */
template <>
struct Pack<double, 4>
{
  /**
   * @brief The register.
   */
  __m256d v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m256d a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const double a_x) noexcept : v(_mm256_set1_pd(a_x))
  {}

  /**
   * @brief Load four lanes from 32-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const double* a_p) noexcept
  {
    return Pack(_mm256_load_pd(a_p));
  }

  /**
   * @brief Store the four lanes to 32-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(double* a_p) const noexcept
  {
    _mm256_store_pd(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm256_add_pd(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm256_sub_pd(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm256_mul_pd(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm256_div_pd(a.v, b.v)); }
  friend MaskD4 operator<(const Pack a, const Pack b) noexcept { return MaskD4{_mm256_cmp_pd(a.v, b.v, _CMP_LT_OQ)}; }
  friend MaskD4 operator>(const Pack a, const Pack b) noexcept { return MaskD4{_mm256_cmp_pd(a.v, b.v, _CMP_GT_OQ)}; }
  friend MaskD4 operator>=(const Pack a, const Pack b) noexcept { return MaskD4{_mm256_cmp_pd(a.v, b.v, _CMP_GE_OQ)}; }
  friend Pack select(const MaskD4 m, const Pack a, const Pack b) noexcept { return Pack(_mm256_blendv_pd(b.v, a.v, m.m)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm256_sqrt_pd(a.v)); }
  friend Pack abs(const Pack a) noexcept { return Pack(_mm256_andnot_pd(_mm256_set1_pd(-0.0), a.v)); }
  /// @endcond
  // clang-format on
};

#endif

#if defined(__AVX512F__)

/**
 * @brief Mask of a Pack<float, 16>: an AVX-512 mask register, one bit per lane.
 */
struct MaskF16
{
  /**
   * @brief The mask register.
   */
  __mmask16 m;

  /// @cond
  friend inline MaskF16
  both(const MaskF16 a_lhs, const MaskF16 a_rhs) noexcept
  {
    return MaskF16{static_cast<__mmask16>(a_lhs.m & a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Mask of a Pack<double, 8>: an AVX-512 mask register, one bit per lane.
 */
struct MaskD8
{
  /**
   * @brief The mask register.
   */
  __mmask8 m;

  /// @cond
  friend inline MaskD8
  both(const MaskD8 a_lhs, const MaskD8 a_rhs) noexcept
  {
    return MaskD8{static_cast<__mmask8>(a_lhs.m & a_rhs.m)};
  }
  /// @endcond
};

/**
 * @brief Sixteen floats in an AVX-512 register.
 * @details abs() goes through the integer domain: a floating-point andnot on a 512-bit register needs
 * AVX512DQ, while the integer one is base AVX-512F.
 */
template <>
struct Pack<float, 16>
{
  /**
   * @brief The register.
   */
  __m512 v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m512 a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const float a_x) noexcept : v(_mm512_set1_ps(a_x))
  {}

  /**
   * @brief Load sixteen lanes from 64-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const float* a_p) noexcept
  {
    return Pack(_mm512_load_ps(a_p));
  }

  /**
   * @brief Store the sixteen lanes to 64-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(float* a_p) const noexcept
  {
    _mm512_store_ps(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm512_add_ps(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm512_sub_ps(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm512_mul_ps(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm512_div_ps(a.v, b.v)); }
  friend MaskF16 operator<(const Pack a, const Pack b) noexcept { return MaskF16{_mm512_cmp_ps_mask(a.v, b.v, _CMP_LT_OQ)}; }
  friend MaskF16 operator>(const Pack a, const Pack b) noexcept { return MaskF16{_mm512_cmp_ps_mask(a.v, b.v, _CMP_GT_OQ)}; }
  friend MaskF16 operator>=(const Pack a, const Pack b) noexcept { return MaskF16{_mm512_cmp_ps_mask(a.v, b.v, _CMP_GE_OQ)}; }
  friend Pack select(const MaskF16 m, const Pack a, const Pack b) noexcept { return Pack(_mm512_mask_blend_ps(m.m, b.v, a.v)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm512_sqrt_ps(a.v)); }
  friend Pack abs(const Pack a) noexcept
  {
    return Pack(_mm512_castsi512_ps(_mm512_andnot_si512(_mm512_castps_si512(_mm512_set1_ps(-0.0F)), _mm512_castps_si512(a.v))));
  }
  /// @endcond
  // clang-format on
};

/**
 * @brief Eight doubles in an AVX-512 register.
 * @details abs() goes through the integer domain, as for Pack<float, 16>.
 */
template <>
struct Pack<double, 8>
{
  /**
   * @brief The register.
   */
  __m512d v;

  /**
   * @brief Uninitialized.
   */
  Pack() noexcept = default;

  /**
   * @brief From a register.
   * @param[in] a_v Register.
   */
  explicit Pack(const __m512d a_v) noexcept : v(a_v)
  {}

  /**
   * @brief Every lane set to @p a_x.
   * @param[in] a_x Value.
   */
  explicit Pack(const double a_x) noexcept : v(_mm512_set1_pd(a_x))
  {}

  /**
   * @brief Load eight lanes from 64-byte aligned memory.
   * @param[in] a_p Address.
   * @return The pack.
   */
  static Pack
  load(const double* a_p) noexcept
  {
    return Pack(_mm512_load_pd(a_p));
  }

  /**
   * @brief Store the eight lanes to 64-byte aligned memory.
   * @param[out] a_p Address.
   */
  void
  store(double* a_p) const noexcept
  {
    _mm512_store_pd(a_p, v);
  }

  // clang-format off
  /// @cond
  friend Pack operator+(const Pack a, const Pack b) noexcept { return Pack(_mm512_add_pd(a.v, b.v)); }
  friend Pack operator-(const Pack a, const Pack b) noexcept { return Pack(_mm512_sub_pd(a.v, b.v)); }
  friend Pack operator*(const Pack a, const Pack b) noexcept { return Pack(_mm512_mul_pd(a.v, b.v)); }
  friend Pack operator/(const Pack a, const Pack b) noexcept { return Pack(_mm512_div_pd(a.v, b.v)); }
  friend MaskD8 operator<(const Pack a, const Pack b) noexcept { return MaskD8{_mm512_cmp_pd_mask(a.v, b.v, _CMP_LT_OQ)}; }
  friend MaskD8 operator>(const Pack a, const Pack b) noexcept { return MaskD8{_mm512_cmp_pd_mask(a.v, b.v, _CMP_GT_OQ)}; }
  friend MaskD8 operator>=(const Pack a, const Pack b) noexcept { return MaskD8{_mm512_cmp_pd_mask(a.v, b.v, _CMP_GE_OQ)}; }
  friend Pack select(const MaskD8 m, const Pack a, const Pack b) noexcept { return Pack(_mm512_mask_blend_pd(m.m, b.v, a.v)); }
  friend Pack sqrt(const Pack a) noexcept { return Pack(_mm512_sqrt_pd(a.v)); }
  friend Pack abs(const Pack a) noexcept
  {
    return Pack(_mm512_castsi512_pd(_mm512_andnot_si512(_mm512_castpd_si512(_mm512_set1_pd(-0.0)), _mm512_castpd_si512(a.v))));
  }
  /// @endcond
  // clang-format on
};

#endif

#endif

} // namespace SIMD

} // namespace EBGeometry

#endif
