// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_BVH.hpp
 * @brief  Bounding volume hierarchies: everything in EBGeometry_PackedBVH.hpp,
 * EBGeometry_TreeBVH.hpp and EBGeometry_BVHBuild.hpp.
 * @details Kept so that code including this header keeps compiling. Device code that only queries
 * a PackedBVH can include EBGeometry_PackedBVH.hpp alone.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_BVH_HPP
#define EBGEOMETRY_BVH_HPP

#include "EBGeometry_BVHBuild.hpp"
#include "EBGeometry_PackedBVH.hpp"
#include "EBGeometry_TreeBVH.hpp"

#endif
