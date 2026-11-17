# GNU makefile for building the ParticleWalls MPI integration.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the EBGeometry repository root); it
# defaults to the in-tree location three levels up. MPICXX is the MPI compiler wrapper (mpicxx). PRECISION selects
# the floating-point type (float or double). ASSERTIONS (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime assertion
# checks. NP is the number of ranks for `make run`. Override any of these on the command line, e.g.:
#
#   make PRECISION=float ASSERTIONS=ON EBGEOMETRY_HOME=/path/to/EBGeometry
#   make run NP=4

EBGEOMETRY_HOME ?= ../../..
MPICXX          ?= mpicxx
MPIRUN          ?= mpirun
NP              ?= 4
PRECISION       ?= double
ASSERTIONS      ?= OFF
CXXFLAGS        ?= -O3 -march=native -Wall -Wextra

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

TARGET := ParticleWalls.ex

$(TARGET): main.cpp
	$(MPICXX) -std=c++17 $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@

.PHONY: run clean
run: $(TARGET)
	$(MPIRUN) -n $(NP) ./$(TARGET)

clean:
	$(RM) $(TARGET)
