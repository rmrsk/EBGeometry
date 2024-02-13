#ifndef AMREX_HAVE_NO_CONFIG_H
#define AMREX_HAVE_NO_CONFIG_H
#ifndef AMREX_TESTING
#define AMREX_TESTING 1
#endif
#ifndef BL_USE_MPI
#define BL_USE_MPI 1
#endif
#ifndef AMREX_USE_MPI
#define AMREX_USE_MPI 1
#endif
#ifndef BL_SPACEDIM
#define BL_SPACEDIM 3
#endif
#ifndef AMREX_SPACEDIM
#define AMREX_SPACEDIM 3
#endif
#ifndef BL_FORT_USE_UNDERSCORE
#define BL_FORT_USE_UNDERSCORE 1
#endif
#ifndef AMREX_FORT_USE_UNDERSCORE
#define AMREX_FORT_USE_UNDERSCORE 1
#endif
#ifndef BL_Linux
#define BL_Linux 1
#endif
#ifndef AMREX_Linux
#define AMREX_Linux 1
#endif
#ifndef BL_USE_ASSERTION
#define BL_USE_ASSERTION 1
#endif
#ifndef AMREX_USE_ASSERTION
#define AMREX_USE_ASSERTION 1
#endif
#ifndef AMREX_USE_EB
#define AMREX_USE_EB 1
#endif
#ifndef OMPI_SKIP_MPICXX
#define OMPI_SKIP_MPICXX 1
#endif
#ifdef __cplusplus
#endif
#if defined(AMREX_USE_OMP) && !defined(_OPENMP) && !defined(AMREX_USE_HIP)
#error libamrex was built with OpenMP
#endif
#endif
