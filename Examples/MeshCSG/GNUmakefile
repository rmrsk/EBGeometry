# GNU makefile for building this EBGeometry example on its own.
#
# EBGEOMETRY_HOME must point at the directory that contains EBGeometry.hpp (the
# EBGeometry repository root); it defaults to the in-tree location two levels up.
# PRECISION selects the floating-point type (float or double). ASSERTIONS
# (ON/OFF) toggles EBGEOMETRY_EXPECT() runtime assertion checks. VIEWER (OFF or
# SDL2) builds the window the example opens at the end, which needs SDL2.
# Override any of these on the command line, e.g.:
#
#   make PRECISION=float ASSERTIONS=ON EBGEOMETRY_HOME=/path/to/EBGeometry
#   make VIEWER=SDL2

EBGEOMETRY_HOME ?= ../..
PRECISION       ?= double
ASSERTIONS      ?= OFF
VIEWER          ?= OFF

# make predefines CXX (as g++ on most systems), so `CXX ?=` would never apply: set it only when it
# still has make's built-in value. -std=c++17 is on the compile line, not in CXXFLAGS, so that a
# CXXFLAGS from the environment or the command line changes the optimisation and warning flags only.
ifeq ($(origin CXX),default)
  CXX = g++
endif
CXXFLAGS ?= -O3 -march=native -Wall -Wextra

ifeq ($(ASSERTIONS),ON)
  CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
endif

# The renderer draws on std::thread; older C libraries need -pthread for it.
LDLIBS += -pthread

# VIEWER=SDL2 opens a window at the end (Render::view()); it needs SDL2 and sdl2-config.
ifeq ($(VIEWER),SDL2)
  CXXFLAGS += -DEBGEOMETRY_VIEWER_SDL2 $(shell sdl2-config --cflags)
  LDLIBS   += $(shell sdl2-config --libs)
else ifneq ($(VIEWER),OFF)
  $(error VIEWER must be OFF or SDL2)
endif

TARGET := MeshCSG.ex

$(TARGET): main.cpp
	$(CXX) -std=c++17 $(CXXFLAGS) -I$(EBGEOMETRY_HOME) -DEBGEOMETRY_PRECISION=$(PRECISION) $< -o $@ $(LDLIBS)

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
