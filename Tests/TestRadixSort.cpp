// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <catch2/catch_test_macros.hpp>

#include <algorithm>
#include <cstdint>
#include <numeric>
#include <string_view>
#include <vector>

using namespace EBGeometry;

// Deterministic 64-bit generator (SplitMix64) -- no <random> engine variability across platforms.
namespace {

class SplitMix64
{
public:
  explicit SplitMix64(uint64_t a_seed) : m_state(a_seed)
  {}

  uint64_t
  next()
  {
    m_state += 0x9E3779B97F4A7C15ull;

    uint64_t z = m_state;
    z          = (z ^ (z >> 30)) * 0xBF58476D1CE4E5B9ull;
    z          = (z ^ (z >> 27)) * 0x94D049BB133111EBull;

    return z ^ (z >> 31);
  }

private:
  uint64_t m_state;
};

std::vector<uint32_t>
iotaValues(size_t a_n)
{
  std::vector<uint32_t> values(a_n);
  std::iota(values.begin(), values.end(), uint32_t(0));

  return values;
}

// Reference stable sort of (key, value) pairs by key only (values witness stability).
void
referenceStableSort(std::vector<uint64_t>& a_keys, std::vector<uint32_t>& a_values)
{
  std::vector<size_t> perm(a_keys.size());
  std::iota(perm.begin(), perm.end(), size_t(0));
  std::stable_sort(
    perm.begin(), perm.end(), [&a_keys](size_t a_lhs, size_t a_rhs) -> bool { return a_keys[a_lhs] < a_keys[a_rhs]; });

  std::vector<uint64_t> keys(a_keys.size());
  std::vector<uint32_t> values(a_values.size());

  for (size_t i = 0; i < perm.size(); i++) {
    keys[i]   = a_keys[perm[i]];
    values[i] = a_values[perm[i]];
  }

  a_keys   = std::move(keys);
  a_values = std::move(values);
}

// Reference stable sort by one digit only (what a single pass must produce).
void
referenceDigitSort(std::vector<uint64_t>& a_keys, std::vector<uint32_t>& a_values, uint32_t a_pass)
{
  std::vector<size_t> perm(a_keys.size());
  std::iota(perm.begin(), perm.end(), size_t(0));
  std::stable_sort(perm.begin(), perm.end(), [&a_keys, a_pass](size_t a_lhs, size_t a_rhs) -> bool {
    return RadixSort::digit(a_keys[a_lhs], a_pass) < RadixSort::digit(a_keys[a_rhs], a_pass);
  });

  std::vector<uint64_t> keys(a_keys.size());
  std::vector<uint32_t> values(a_values.size());

  for (size_t i = 0; i < perm.size(); i++) {
    keys[i]   = a_keys[perm[i]];
    values[i] = a_values[perm[i]];
  }

  a_keys   = std::move(keys);
  a_values = std::move(values);
}

std::vector<uint64_t>
makeKeys(const char* a_kind, size_t a_n)
{
  SplitMix64 rng(20260720ull);

  std::vector<uint64_t> keys(a_n);

  for (size_t i = 0; i < a_n; i++) {
    if (a_kind == std::string_view("random")) {
      keys[i] = rng.next();
    }
    else if (a_kind == std::string_view("allEqual")) {
      keys[i] = 0xDEADBEEFCAFEF00Dull;
    }
    else if (a_kind == std::string_view("sorted")) {
      keys[i] = uint64_t(i) * 3ull;
    }
    else if (a_kind == std::string_view("reverse")) {
      keys[i] = uint64_t(a_n - i) * 7ull;
    }
    else { // "highByte": only the most significant byte differs (low passes see all-equal digits)
      keys[i] = (rng.next() & 0xFF00000000000000ull) | 0x123456ull;
    }
  }

  return keys;
}

} // namespace

TEST_CASE("RadixSort: digit extraction table", "[RadixSort]")
{
  REQUIRE(RadixSort::RadixBits == 8u);
  REQUIRE(RadixSort::RadixSize == 256u);
  REQUIRE(RadixSort::NumPasses64 == 8u);

  const uint64_t key = 0x0123456789ABCDEFull;

  const uint32_t expected[8] = {0xEF, 0xCD, 0xAB, 0x89, 0x67, 0x45, 0x23, 0x01};

  for (uint32_t pass = 0; pass < RadixSort::NumPasses64; pass++) {
    REQUIRE(RadixSort::digit(key, pass) == expected[pass]);
  }

  REQUIRE(RadixSort::digit(0ull, 0) == 0u);
  REQUIRE(RadixSort::digit(~0ull, 7) == 0xFFu);
}

TEST_CASE("RadixSort: blockRangeBegin covers the whole range with contiguous ordered slabs", "[RadixSort]")
{
  for (uint32_t numItems : {0u, 1u, 7u, 255u, 256u, 1000u}) {

    for (uint32_t numBlocks : {1u, 2u, 3u, 16u, 300u}) {
      REQUIRE(RadixSort::blockRangeBegin(numItems, numBlocks, 0) == 0u);
      REQUIRE(RadixSort::blockRangeBegin(numItems, numBlocks, numBlocks) == numItems);

      for (uint32_t b = 0; b < numBlocks; b++) {
        const uint32_t begin = RadixSort::blockRangeBegin(numItems, numBlocks, b);
        const uint32_t end   = RadixSort::blockRangeBegin(numItems, numBlocks, b + 1);

        REQUIRE(begin <= end);
        REQUIRE(end <= numItems);
      }
    }
  }
}

TEST_CASE("RadixSort: one hostPass equals std::stable_sort restricted to that digit", "[RadixSort]")
{
  SplitMix64 rng(42ull);

  const size_t n = 1500;

  std::vector<uint64_t> keys(n);

  // Few distinct digit values per pass so that duplicates abound -- the value payloads then
  // witness stability, which a correct-but-unstable pass would break.
  for (auto& k : keys) {
    k = rng.next() & 0x0303030303030303ull;
  }

  for (uint32_t pass = 0; pass < RadixSort::NumPasses64; pass++) {

    for (uint32_t numBlocks : {1u, 7u, 64u}) {
      std::vector<uint64_t> outKeys(n);
      std::vector<uint32_t> outValues(n);

      const std::vector<uint32_t> values = iotaValues(n);

      RadixSort::hostPass(keys.data(), values.data(), uint32_t(n), pass, numBlocks, outKeys.data(), outValues.data());

      std::vector<uint64_t> refKeys   = keys;
      std::vector<uint32_t> refValues = values;

      referenceDigitSort(refKeys, refValues, pass);

      REQUIRE(outKeys == refKeys);
      REQUIRE(outValues == refValues);
    }
  }
}

TEST_CASE("RadixSort: full hostStableSort equals std::stable_sort", "[RadixSort]")
{
  for (const char* kind : {"random", "allEqual", "sorted", "reverse", "highByte"}) {

    for (size_t n : {size_t(0), size_t(1), size_t(255), size_t(256), size_t(257), size_t(100000)}) {
      std::vector<uint64_t> keys   = makeKeys(kind, n);
      std::vector<uint32_t> values = iotaValues(n);

      std::vector<uint64_t> refKeys   = keys;
      std::vector<uint32_t> refValues = values;

      RadixSort::hostStableSort(keys, values);
      referenceStableSort(refKeys, refValues);

      REQUIRE(keys == refKeys);
      REQUIRE(values == refValues);
    }
  }
}

TEST_CASE("RadixSort: hostExclusiveScan equals std::exclusive_scan", "[RadixSort]")
{
  SplitMix64 rng(7ull);

  for (uint32_t n : {0u, 1u, 2u, 255u, 256u, 257u, 10000u}) {

    for (uint32_t numBlocks : {1u, 2u, 3u, 32u, 300u}) {
      std::vector<uint32_t> data(n);

      for (auto& v : data) {
        v = uint32_t(rng.next() & 0xFFFFull);
      }

      std::vector<uint32_t> expected(n);
      std::exclusive_scan(data.begin(), data.end(), expected.begin(), uint32_t(0));

      RadixSort::hostExclusiveScan(data.data(), n, numBlocks);

      REQUIRE(data == expected);
    }
  }
}

TEST_CASE("RadixSort: per-block scan steps compose", "[RadixSort]")
{
  // blockExclusiveScan returns the block total and scans in place; blockAddOffset shifts a slab.
  std::vector<uint32_t> data = {3, 1, 4, 1, 5};

  const uint32_t total = RadixSort::blockExclusiveScan(data.data(), 0, 5);

  REQUIRE(total == 14u);
  REQUIRE(data == std::vector<uint32_t>({0, 3, 4, 8, 9}));

  RadixSort::blockAddOffset(data.data(), 1, 4, 100);

  REQUIRE(data == std::vector<uint32_t>({0, 103, 104, 108, 9}));

  // Empty slab: no writes, zero total.
  REQUIRE(RadixSort::blockExclusiveScan(data.data(), 2, 2) == 0u);
}
