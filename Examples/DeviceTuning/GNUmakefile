# GNU makefile for building this EBGeometry example on its own.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location two levels up.
# PRECISION selects the floating-point type (float or double). ASSERTIONS
# (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime assertion checks. GPU (OFF, HIP or
# CUDA) compiles the example for a GPU with hipcc or nvcc, so that its queries
# through GPU::transform run in kernels, which is what it tunes (HIP_ARCH and
# CUDA_ARCH pick the target; run `make clean` after switching). Override any of
# these on the command line, e.g.:
#
#   make PRECISION=float ASSERTIONS=ON EBGEOMETRY_HOME=/path/to/EBGeometry
#   make GPU=HIP HIP_ARCH=gfx942

EBGEOMETRY_HOME ?= ../..
PRECISION       ?= double
ASSERTIONS      ?= OFF
GPU             ?= OFF
HIP_ARCH        ?= gfx90a
# The GPU in this machine (nvidia-smi), so that the binary carries its machine code; sm_70 without one.
CUDA_ARCH       ?= sm_$(or $(shell nvidia-smi --query-gpu=compute_cap --format=csv,noheader 2>/dev/null | grep -m1 -E '^[0-9]+\.[0-9]+$$' | tr -d .),70)

# make predefines CXX (as g++ on most systems), so `CXX ?=` would never apply: set it only when it
# still has make's built-in value. -std=c++17 is on the compile line, not in CXXFLAGS, so that a
# CXXFLAGS from the environment or the command line changes the optimisation and warning flags only.
ifeq ($(origin CXX),default)
  CXX = g++
endif
CXXFLAGS ?= -O3 -march=native -Wall -Wextra

ifeq ($(GPU),HIP)
  CXX      = hipcc
  CXXFLAGS = -O3 -Wall -Wextra -Wno-unused-parameter -x hip --offload-arch=$(HIP_ARCH)
else ifeq ($(GPU),CUDA)
  CXX      = nvcc
  CXXFLAGS = -O3 -x cu -arch=$(CUDA_ARCH)
else ifneq ($(GPU),OFF)
  $(error GPU must be OFF, HIP or CUDA)
endif

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

TARGET := DeviceTuning.ex

$(TARGET): main.cpp
	$(CXX) -std=c++17 $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@ $(LDLIBS)

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
