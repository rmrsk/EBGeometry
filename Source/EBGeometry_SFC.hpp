/* EBGeometry
 * Copyright © 2024 Robert Marskar
 * Please refer to Copyright.txt and LICENSE in the EBGeometry root directory.
 */

/*!
  @file   EBGeometry_SFC.hpp
  @brief  Declaration of various space-filling curves
  @author Robert Marskar
*/

#ifndef EBGeometry_SFC
#define EBGeometry_SFC

// Std includes
#include <cstdint>

// Our includes
#include "EBGeometry_Index.hpp"
#include "EBGeometry_NamespaceHeader.hpp"

namespace SFC {

  /*!
    @brief Alias for SFC code
  */
  using Code = uint64_t;

  /*!
    @brief Alias for 3D cell index
  */
  using Index = std::array<unsigned int, 3>;

  /*!
    @brief Maximum available bits. Using same number = 21 for all dimensions (strictly speaking, we could use 32 bits in 2D and all 64 bits in 1D).
  */
  static constexpr unsigned int ValidBits = 21;

  /*!
    @brief Maximum permitted span along any spatial coordinate. 
  */
  static constexpr Code ValidSpan = ((uint64_t)1 << ValidBits) - 1;

  /*!
    @brief Encodable SFC concept -- class must have a static function static uint64_t encode(const Index&). This is the main interface for SFCs
  */
  template <typename T>
  concept Encodable = requires(const Index& point, const SFC::Code code) {
    { T::encode(point) } -> SFC::Code;
    { T::decode(code) } -> Index;
  };

  /*!
    @brief Implementation of the Morton SFC
  */
  struct Morton
  {
    /*!
      @brief Encode an input point into a Morton index with a 64-bit representation.
      @param[in] a_point 
    */
    inline static uint64_t
    encode(const Index& a_point) noexcept;

    /*!
      @brief Decode the 64-bit Morton code into an Index.
      @param[in] a_code Morton code
    */
    inline static Index
    decode(const uint64_t& a_code) noexcept;

  protected:
    /*!
      @brief Mask for magic-bits encoding of 3D Morton code
    */
    static constexpr uint_fast64_t Mask_64[6]{
      0x1fffff, 0x1f00000000ffff, 0x1f0000ff0000ff, 0x100f00f00f00f00f, 0x10c30c30c30c30c3, 0x1249249249249249};
  };

  /*!
    @brief Implementation of a nested index SFC.
    @details The SFC is encoded by the code = i + j * N + k * N * N in 3D, where i,j,k are the block indices. 
  */
  struct Nested
  {
    /*!
      @brief Encode the input point into the SFC code.
      @param[in] a_point 
    */
    inline static uint64_t
    encode(const Index& a_point) noexcept;

    /*!
      @brief Decode the 64-bit SFC code into an Index.
      @param[in] a_code SFC code.
    */
    inline static Index
    decode(const uint64_t& a_code) noexcept;
  };
} // namespace SFC

#include "EBGeometry_NamespaceFooter.hpp"

#include "EBGeometry_SFCImplem.hpp"

#endif
