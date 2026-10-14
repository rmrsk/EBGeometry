# GNU makefile for building this EBGeometry example on its own.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location two levels up.
# PRECISION selects the floating-point type (float or double). ASSERTIONS
# (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime assertion checks. GPU selects where
# the queries run: OFF (the default) builds plain C++ and runs them in a host loop,
# and HIP or CUDA compiles for a GPU with hipcc or nvcc and runs them in a kernel
# (HIP_ARCH and CUDA_ARCH pick the target; run `make clean` after switching).
# Override any of these on the command line, e.g.:
#
#   make PRECISION=float ASSERTIONS=ON EBGEOMETRY_HOME=/path/to/EBGeometry
#   make GPU=HIP HIP_ARCH=gfx942

EBGEOMETRY_HOME ?= ../..
PRECISION       ?= double
ASSERTIONS      ?= OFF
GPU             ?= OFF
HIP_ARCH        ?= gfx90a
CUDA_ARCH       ?= sm_70

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -march=native -Wall -Wextra

ifeq ($(GPU),HIP)
  CXX      = hipcc
  CXXFLAGS = -std=c++17 -O3 -Wall -Wextra -Wno-unused-parameter -x hip --offload-arch=$(HIP_ARCH)
else ifeq ($(GPU),CUDA)
  CXX      = nvcc
  CXXFLAGS = -std=c++17 -O3 -x cu -arch=$(CUDA_ARCH)
else ifneq ($(GPU),OFF)
  $(error GPU must be OFF, HIP or CUDA)
endif

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

TARGET := DevicePointCloud.ex

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
