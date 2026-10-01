// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Every BVH::Build value, for tests that check a BVH user accepts all of them. One list, so a value
// added to the enum is added to every such test at once. Test infrastructure only.

#ifndef EBGEOMETRY_TEST_BUILD_METHODS_HPP
#define EBGEOMETRY_TEST_BUILD_METHODS_HPP

#include <EBGeometry.hpp>

inline constexpr EBGeometry::BVH::Build allBuildMethods[] = {EBGeometry::BVH::Build::CentroidSplit,
                                                             EBGeometry::BVH::Build::MidpointSplit,
                                                             EBGeometry::BVH::Build::SAH,
                                                             EBGeometry::BVH::Build::ClusterSAH,
                                                             EBGeometry::BVH::Build::Morton,
                                                             EBGeometry::BVH::Build::Nested,
                                                             EBGeometry::BVH::Build::Hilbert};

// A value outside the enum, for the death tests that check a BVH user rejects it.
inline constexpr EBGeometry::BVH::Build invalidBuildMethod = static_cast<EBGeometry::BVH::Build>(99);

#endif
