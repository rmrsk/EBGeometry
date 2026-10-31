// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry_CSG.hpp
 * @brief  Constructive solid geometry: the blend operators, the BVH unions, the expression builder
 * (Expr, Union, Intersection, Difference, Translate, ...), the Tape that evaluates a composed object,
 * and the fused operations (Fused::Union, ...).
 * @details Includes EBGeometry_Blend.hpp, EBGeometry_BVHUnion.hpp, EBGeometry_CSGRayForms.hpp,
 * EBGeometry_Expr.hpp, EBGeometry_Fused.hpp, EBGeometry_OffsetRayForms.hpp and EBGeometry_Tape.hpp.
 * @author Robert Marskar
 */

#ifndef EBGEOMETRY_CSG_HPP
#define EBGEOMETRY_CSG_HPP

// Our includes
#include "EBGeometry_BVHUnion.hpp"
#include "EBGeometry_Blend.hpp"
#include "EBGeometry_CSGRayForms.hpp"
#include "EBGeometry_Expr.hpp"
#include "EBGeometry_Fused.hpp"
#include "EBGeometry_OffsetRayForms.hpp"
#include "EBGeometry_SmoothRayForms.hpp"
#include "EBGeometry_Tape.hpp"

#endif
