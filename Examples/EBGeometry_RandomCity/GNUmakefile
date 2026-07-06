# GNU makefile for building this EBGeometry example on its own.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location two levels up.
# PRECISION selects the floating-point type (float or double). Override either on
# the command line, e.g.:
#
#   make PRECISION=float EBGEOMETRY_HOME=/path/to/EBGeometry

EBGEOMETRY_HOME ?= ../..
PRECISION       ?= double

CXX      ?= g++
CXXFLAGS ?= -std=c++17 -O3 -march=native -Wall -Wextra

TARGET := EBGeometry_RandomCity

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
