// SPDX-FileCopyrightText: 2026 Robert Marskar <robert.marskar@sintef.no>
// SPDX-License-Identifier: BSD-3-Clause

// One of ebgstudio's device kernels (see Kernels.hpp), chosen by EBGEOMETRY_STUDIO_KERNEL: compiled once per index in
// a GPU build, beside main.cpp.

#include "Kernels.hpp"

#if !defined(EBGEOMETRY_STUDIO_KERNEL)
#error "Kernels.cpp: compile it with -DEBGEOMETRY_STUDIO_KERNEL=<0 to StudioKernels::Count - 1>"
#elif EBGEOMETRY_STUDIO_KERNEL == 0
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel0)
#elif EBGEOMETRY_STUDIO_KERNEL == 1
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel1)
#elif EBGEOMETRY_STUDIO_KERNEL == 2
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel2)
#elif EBGEOMETRY_STUDIO_KERNEL == 3
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel3)
#elif EBGEOMETRY_STUDIO_KERNEL == 4
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel4)
#elif EBGEOMETRY_STUDIO_KERNEL == 5
EBGEOMETRY_STUDIO_KERNEL_DIRECTIVE(, StudioKernels::Kernel5)
#else
#error "Kernels.cpp: EBGEOMETRY_STUDIO_KERNEL is out of range"
#endif
