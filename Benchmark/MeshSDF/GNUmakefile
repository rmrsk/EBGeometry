# Benchmark: EBGeometry TriMeshSDF vs fcpw (closest-point on a triangle mesh).
# Requires the Submodules/fcpw submodule plus its Eigen dependency, and the mesh submodule:
#   git submodule update --init Submodules/fcpw Submodules/common-3d-test-models
#   git -C Submodules/fcpw submodule update --init deps/eigen   # Eigen only (skip GPU slang-rhi)
# EBGEOMETRY_HOME defaults to the repo root (two levels up).

EBGEOMETRY_HOME ?= ../..
CXX             ?= g++
CXXFLAGS        ?= -std=c++17 -O3 -march=native

INCLUDES := -I$(EBGEOMETRY_HOME) \
            -I$(EBGEOMETRY_HOME)/Submodules/fcpw/include \
            -I$(EBGEOMETRY_HOME)/Submodules/fcpw/deps/eigen

TARGET := MeshSDF.ex

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(INCLUDES) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
