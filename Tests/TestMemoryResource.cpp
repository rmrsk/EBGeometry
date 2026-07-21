// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

#include "EBGeometry.hpp"

#include <cstdint>
#include <cstring>

#include <catch2/catch_test_macros.hpp>

using namespace EBGeometry;

// ─────────────────────────────────────────────────────────────────────────────
// HostMemoryResource -- the always-compiled host path. The device/managed/pinned
// resources need an offload backend, so only the host resource is behaviorally
// testable without a GPU.
// ─────────────────────────────────────────────────────────────────────────────

TEST_CASE("HostMemoryResource: allocate returns non-null, correctly-aligned, writeable blocks", "[MemoryResource]")
{
  HostMemoryResource& resource = hostMemoryResource();

  for (size_t alignment = 1; alignment <= 256; alignment *= 2) {
    const size_t bytes = 1000;

    void* ptr = resource.allocate(bytes, alignment);

    REQUIRE(ptr != nullptr);
    REQUIRE(reinterpret_cast<uintptr_t>(ptr) % alignment == 0);

    // The full byte range must be writeable (a too-small allocation would corrupt/segfault under
    // the sanitizers).
    std::memset(ptr, 0xAB, bytes);

    auto* bytePtr = static_cast<unsigned char*>(ptr);

    REQUIRE(bytePtr[0] == 0xAB);
    REQUIRE(bytePtr[bytes - 1] == 0xAB);

    resource.deallocate(ptr, bytes, alignment);
  }
}

TEST_CASE("HostMemoryResource: deallocate round-trips without leak", "[MemoryResource]")
{
  HostMemoryResource& resource = hostMemoryResource();

  // Many alloc/free cycles: under the ASan/UBSan sanitizer preset a missing free (leak) or a
  // double free would be caught here.
  for (int i = 0; i < 64; i++) {
    void* ptr = resource.allocate(4096, 64);

    REQUIRE(ptr != nullptr);

    resource.deallocate(ptr, 4096, 64);
  }

  // A null deallocate is a no-op (must not crash).
  resource.deallocate(nullptr, 0, 8);
}

TEST_CASE("HostMemoryResource: accessibility flags", "[MemoryResource]")
{
  HostMemoryResource& resource = hostMemoryResource();

  REQUIRE(resource.isHostAccessible() == true);
  REQUIRE(resource.isDeviceAccessible() == false);
}

TEST_CASE("hostMemoryResource: returns the same process-wide instance", "[MemoryResource]")
{
  REQUIRE(&hostMemoryResource() == &hostMemoryResource());
}
