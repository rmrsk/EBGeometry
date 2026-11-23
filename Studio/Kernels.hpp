// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// ebgstudio's device kernels, one translation unit each, so that a GPU build compiles them in parallel.
//
// Nearly all of a GPU build's time is ptxas on six render kernels: the shaded and the inspecting (cost view) frame of
// the view of regions, of its tape and of the exploded view. Compiled in one translation unit they take one ptxas
// after another (about six minutes for float on CUDA 12.5); one each, they take about as long as the largest. Each
// is instantiated in Kernels.cpp, compiled once per index (EBGEOMETRY_STUDIO_KERNEL = 0 to Count - 1), and declared
// extern in main.cpp, which then compiles none of them: an extern template of the kernel itself, since the launches
// around it are inline and would bring it back.
//
// A kernel the library launches that is not listed here is compiled in main.cpp as before, so a change in the
// renderer's kernels makes the build slower, never wrong; a listed kernel that no longer exists fails to link.

#ifndef EBGEOMETRY_STUDIO_KERNELS_HPP
#define EBGEOMETRY_STUDIO_KERNELS_HPP

#include "EBGeometry.hpp"

#include <cstddef>

#ifndef EBGEOMETRY_STUDIO_PRECISION
#define EBGEOMETRY_STUDIO_PRECISION float
#endif

namespace StudioKernels {

using T = EBGEOMETRY_STUDIO_PRECISION;

// What the viewer draws: the view of regions, its tape (the cost views of the object's queries) and the exploded view.
using View     = EBGeometry::RegionView<EBGeometry::Tape<T>>;
using Object   = EBGeometry::Tape<T>;
using Exploded = EBGeometry::Render::ViewerDetail::ExplodedView<View>;

// A frame's functor: every pixel of one query.
template <class Q>
using Frame = EBGeometry::Render::RenderDetail::FrameOfPixels<Q>;

// The six kernels' functors, by index.
using Kernel0 = Frame<EBGeometry::Render::PixelQuery<T, View>>;
using Kernel1 = Frame<EBGeometry::Render::RenderDetail::InspectQuery<T, View>>;
using Kernel2 = Frame<EBGeometry::Render::PixelQuery<T, Object>>;
using Kernel3 = Frame<EBGeometry::Render::RenderDetail::InspectQuery<T, Object>>;
using Kernel4 = Frame<EBGeometry::Render::PixelQuery<T, Exploded>>;
using Kernel5 = Frame<EBGeometry::Render::RenderDetail::InspectQuery<T, Exploded>>;

inline constexpr int Count = 6;

} // namespace StudioKernels

// The kernel of functor F, as GPU::parallelFor() launches it: declared (PREFIX extern) or instantiated (no PREFIX). The
// parameter repeats the kernel's EBGEOMETRY_GRID_CONSTANT, which nvcc requires of an instantiation directive.
#define EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(PREFIX, F)                                                  \
  PREFIX template EBGEOMETRY_GLOBAL                                                                    \
  void EBGeometry::GPU::TransformDetail::parallelForKernel<EBGeometry::GPU::DefaultBlockSize,          \
                                                           EBGeometry::GPU::DefaultMinResidentThreads, \
                                                           F>(EBGEOMETRY_GRID_CONSTANT const F, std::size_t);

#endif
