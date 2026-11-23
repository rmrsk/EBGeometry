# GNU makefile for building ebgstudio on its own, here in Studio/ (the binary is Studio/ebgstudio).
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location one level up.
# PRECISION selects the floating-point type models are built in (float, the
# default, or double). ASSERTIONS (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime
# assertion checks. GPU (OFF, HIP or CUDA) compiles it for a GPU with hipcc or
# nvcc, so that render and view draw there (HIP_ARCH and CUDA_ARCH pick the
# target; run `make clean` after switching). VIEWER (SDL2 or OFF) builds the
# window of `ebgstudio view`, which needs SDL2; it is SDL2 when sdl2-config is
# found. A GPU build compiles the render kernels in parallel, one translation unit each
# (Kernels.cpp, see Kernels.hpp), JOBS at a time (default 7, the kernels and main.cpp at once,
# needing about 30 GB of memory for CUDA; JOBS=3 needs about 15 GB). Override any of these on the
# command line, e.g.:
#
#   make GPU=CUDA
#   make PRECISION=double VIEWER=OFF
#   make GPU=HIP HIP_ARCH=gfx942
#   make GPU=CUDA JOBS=3

EBGEOMETRY_HOME ?= ..
PRECISION       ?= float
ASSERTIONS      ?= OFF
GPU             ?= OFF
VIEWER          ?= $(if $(shell command -v sdl2-config 2>/dev/null),SDL2,OFF)
HIP_ARCH        ?= gfx90a
# The GPU in this machine (nvidia-smi), so that the binary carries its machine code; sm_70 without one.
CUDA_ARCH       ?= sm_$(or $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader 2>/dev/null | grep -m1 -E '^[0-9]+\.[0-9]+$$' | tr -d .),70)

# make predefines CXX (as g++ on most systems), so `CXX ?=` would never apply: set it only when it
# still has make's built-in value. -std=c++17 is on the compile line, not in CXXFLAGS, so that a
# CXXFLAGS from the environment or the command line changes the optimization and warning flags only.
ifeq ($(origin CXX),default)
  CXX = g++
endif
CXXFLAGS ?= -O3 -march=native -Wall -Wextra

ifeq ($(GPU),HIP)
  CXX       = hipcc
  CXXFLAGS  = -O3 -Wall -Wextra -Wno-unused-parameter -x hip --offload-arch=$(HIP_ARCH)
  LINKFLAGS = --offload-arch=$(HIP_ARCH)
else ifeq ($(GPU),CUDA)
  CXX       = nvcc
  CXXFLAGS  = -O3 -x cu -arch=$(CUDA_ARCH)
  LINKFLAGS = -arch=$(CUDA_ARCH)
else ifneq ($(GPU),OFF)
  $(error GPU must be OFF, HIP or CUDA)
endif

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

# The renderer draws on std::thread; older C libraries need -pthread for it (nvcc takes -lpthread).
ifeq ($(GPU),CUDA)
  LDLIBS += -lpthread
else
  LDLIBS += -pthread
endif

# VIEWER=SDL2 builds the window of `ebgstudio view`; it needs SDL2 and sdl2-config.
ifeq ($(VIEWER),SDL2)
  CXXFLAGS += -DEBGEOMETRY_VIEWER_SDL2 $(shell sdl2-config --cflags)
  LDLIBS   += $(shell sdl2-config --libs)
  ifeq ($(GPU),CUDA)
    CXXFLAGS += -DSDL_DISABLE_IMMINTRIN_H
  endif
else ifneq ($(VIEWER),OFF)
  $(error VIEWER must be OFF or SDL2)
endif

TARGET  := ebgstudio
DEFINES := -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_STUDIO_PRECISION=$(PRECISION)
KERNELS := $(foreach k,0 1 2 3 4 5,kernel$(k).o)
JOBS    ?= 7

ifeq ($(GPU),OFF)
$(TARGET): main.cpp Writer.hpp
	$(CXX) -std=c++17 $(CXXFLAGS) $(DEFINES) $< -o $@ $(LDLIBS)
else
MAKEFLAGS += -j$(JOBS)

$(TARGET): main.o $(KERNELS)
	$(CXX) $(LINKFLAGS) $^ -o $@ $(LDLIBS)

main.o: main.cpp Writer.hpp Kernels.hpp
	$(CXX) -std=c++17 $(CXXFLAGS) $(DEFINES) -DEBGEOMETRY_STUDIO_SPLIT_KERNELS -c $< -o $@

kernel%.o: Kernels.cpp Kernels.hpp
	$(CXX) -std=c++17 $(CXXFLAGS) $(DEFINES) -DEBGEOMETRY_STUDIO_KERNEL=$* -c $< -o $@
endif

.PHONY: clean
clean:
	$(RM) $(TARGET) main.o $(KERNELS)
