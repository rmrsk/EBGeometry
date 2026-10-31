# GNU makefile for building this EBGeometry example on its own.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location two levels up.
# PRECISION selects the floating-point type (float or double). ASSERTIONS
# (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime assertion checks. Override any
# of these on the command line, e.g.:
#
#   make PRECISION=float ASSERTIONS=ON EBGEOMETRY_HOME=/path/to/EBGeometry

EBGEOMETRY_HOME ?= ../..
PRECISION       ?= double
ASSERTIONS      ?= OFF

# make predefines CXX (as g++ on most systems), so `CXX ?=` would never apply: set it only when it
# still has make's built-in value. -std=c++17 is on the compile line, not in CXXFLAGS, so that a
# CXXFLAGS from the environment or the command line changes the optimization and warning flags only.
ifeq ($(origin CXX),default)
  CXX = g++
endif
CXXFLAGS ?= -O3 -march=native -Wall -Wextra

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

TARGET := HostTuning.ex

$(TARGET): main.cpp
	$(CXX) -std=c++17 $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
