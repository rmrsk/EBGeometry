// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVHBuild.hpp
 * @brief  Host-side BVH construction: the PackedBVH constructors that build from a TreeBVH or
 * partition a primitive list directly (top-down, along a space-filling curve, or ClusterSAH).
 * @details Includes EBGeometry_TreeBVH.hpp, whose partitioners and leaf predicate these constructors
 * use, and EBGeometry_PackedBVH.hpp. Code that only queries a PackedBVH, device code in particular,
 * can include EBGeometry_PackedBVH.hpp alone.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVHBUILD_HPP
#define EBGEOMETRY_BVHBUILD_HPP

// Our includes
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_SFC.hpp"
#include "EBGeometry_TreeBVH.hpp"

#include "EBGeometry_BVHBuildImplem.hpp"

#endif
