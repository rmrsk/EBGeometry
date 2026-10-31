// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: GPL-3.0-or-later

// Every BVH::Construction value, for tests that check a BVH user accepts all of them. One list, so a value
// added to the enum is added to every such test at once. Test infrastructure only.

#ifndef EBGEOMETRY_TEST_CONSTRUCTIONS_HPP
#define EBGEOMETRY_TEST_CONSTRUCTIONS_HPP

#include <EBGeometry.hpp>

inline constexpr EBGeometry::BVH::Construction allConstructions[] = {EBGeometry::BVH::Construction::CentroidSplit,
                                                                     EBGeometry::BVH::Construction::MidpointSplit,
                                                                     EBGeometry::BVH::Construction::SAH,
                                                                     EBGeometry::BVH::Construction::ClusterSAH,
                                                                     EBGeometry::BVH::Construction::Morton,
                                                                     EBGeometry::BVH::Construction::Nested,
                                                                     EBGeometry::BVH::Construction::Hilbert};

// A value outside the enum, for the death tests that check a BVH user rejects it.
inline constexpr EBGeometry::BVH::Construction invalidConstruction = static_cast<EBGeometry::BVH::Construction>(99);

#endif
