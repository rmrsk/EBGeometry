// SPDX-FileCopyrightText: 2022 Robert Marskar <robert.marskar@sintef.no>
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file   EBGeometry.hpp
 * @brief  EBGeometry single-file access point.
 * @author Robert Marskar
 */

#include "Source/EBGeometry_AnalyticDistanceFunctions.hpp"
#include "Source/EBGeometry_BVH.hpp"
#include "Source/EBGeometry_BoundingVolumes.hpp"
#include "Source/EBGeometry_CSG.hpp"
#include "Source/EBGeometry_Constants.hpp"
#include "Source/EBGeometry_DCEL.hpp"
#include "Source/EBGeometry_DCEL_Edge.hpp"
#include "Source/EBGeometry_DCEL_Face.hpp"
#include "Source/EBGeometry_DCEL_Iterator.hpp"
#include "Source/EBGeometry_DCEL_Mesh.hpp"
#include "Source/EBGeometry_DCEL_Vertex.hpp"
#include "Source/EBGeometry_GPU.hpp"
#include "Source/EBGeometry_ImplicitFunction.hpp"
#include "Source/EBGeometry_Macros.hpp"
#include "Source/EBGeometry_MeshDistanceFunctions.hpp"
#include "Source/EBGeometry_OBJ.hpp"
#include "Source/EBGeometry_Octree.hpp"
#include "Source/EBGeometry_PLY.hpp"
#include "Source/EBGeometry_Parser.hpp"
#include "Source/EBGeometry_PointAoSoA.hpp"
#include "Source/EBGeometry_PointCloudBVH.hpp"
#include "Source/EBGeometry_PointCloudHashGrid.hpp"
#include "Source/EBGeometry_PointSoA.hpp"
#include "Source/EBGeometry_Polygon2D.hpp"
#include "Source/EBGeometry_Random.hpp"
#include "Source/EBGeometry_SFC.hpp"
#include "Source/EBGeometry_STL.hpp"
#include "Source/EBGeometry_SimpleTimer.hpp"
#include "Source/EBGeometry_Soup.hpp"
#include "Source/EBGeometry_Transform.hpp"
#include "Source/EBGeometry_Triangle.hpp"
#include "Source/EBGeometry_TriangleAoSoA.hpp"
#include "Source/EBGeometry_TriangleSoA.hpp"
#include "Source/EBGeometry_VTK.hpp"
#include "Source/EBGeometry_Vec.hpp"

// EBGeometry_Tape.hpp is intentionally included out of alphabetical order, after CSG and Transform:
// its flatten() lowering and single-pass interpreter need the complete CSG combiner and transform
// node definitions (not just their forward declarations) to define the flatten() overrides.
#include "Source/EBGeometry_Tape.hpp"

// EBGeometry_DeviceTape.hpp is inert (empty after preprocessing) unless the TU is translated by an
// offload compiler (CUDA), so it is always safe to include here.
#include "Source/EBGeometry_DeviceTape.hpp"

/**
 * @brief Namespace containing all of EBGeometry's functionality.
 * @details EBGeometry is a header-only C++17 library for implicit functions, signed distance
 *          functions, constructive solid geometry (CSG), and DCEL surface meshes. Including
 *          EBGeometry.hpp exposes every public class and free function through this namespace.
 */
namespace EBGeometry {}
