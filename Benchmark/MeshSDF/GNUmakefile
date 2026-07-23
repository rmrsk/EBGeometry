# Benchmark: EBGeometry TriMeshSDF vs fcpw vs TriangleMeshDistance (closest-point on a triangle mesh).
# Requires the mesh submodule, fcpw (Enoki is vendored inside fcpw; Eigen is a nested submodule), and
# TriangleMeshDistance:
#   git submodule update --init Benchmark/fcpw common-3d-test-models Benchmark/TriangleMeshDistance
#   git -C Benchmark/fcpw submodule update --init deps/eigen   # Eigen only (skip GPU slang-rhi)
# fcpw is built with its Enoki CPU vectorization: FCPW_SIMD_WIDTH matches the ISA (4=SSE, 8=AVX2,
# 16=AVX-512); adjust it below to your machine. EBGEOMETRY_HOME defaults to the repo root.

EBGEOMETRY_HOME ?= ../..
CXX             ?= g++
CXXFLAGS        ?= -std=c++17 -O3 -march=native
FCPW_SIMD_WIDTH ?= 8

INCLUDES := -I$(EBGEOMETRY_HOME) \
            -I$(EBGEOMETRY_HOME)/Benchmark/fcpw/include \
            -I$(EBGEOMETRY_HOME)/Benchmark/fcpw/deps/eigen \
            -I$(EBGEOMETRY_HOME)/Benchmark/fcpw/deps/enoki/include \
            -I$(EBGEOMETRY_HOME)/Benchmark/TriangleMeshDistance/TriangleMeshDistance/include

DEFINES := -DFCPW_USE_ENOKI -DFCPW_SIMD_WIDTH=$(FCPW_SIMD_WIDTH)

TARGET := MeshSDF.ex

$(TARGET): main.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) $< -o $@

.PHONY: run clean
run: $(TARGET)
	./$(TARGET)

clean:
	$(RM) $(TARGET)
