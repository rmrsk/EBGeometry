# Benchmark: PointCloudBVH vs picoflann vs nanoflann (all-nearest-neighbor).
# Requires the Submodules/{picoflann,nanoflann} submodules:
#   git submodule update --init Submodules/picoflann Submodules/nanoflann
# EBGEOMETRY_HOME defaults to the repo root (two levels up).

EBGEOMETRY_HOME ?= ../..
CXX             ?= g++
CXXFLAGS        ?= -std=c++17 -O3 -march=native

INCLUDES := -I$(EBGEOMETRY_HOME) \
            -I$(EBGEOMETRY_HOME)/Submodules/picoflann \
            -I$(EBGEOMETRY_HOME)/Submodules/nanoflann/include

TARGET := NearestNeighbor.ex

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
