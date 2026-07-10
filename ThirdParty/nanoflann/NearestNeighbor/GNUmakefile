# Standalone build for the EBGeometry-vs-nanoflann nearest-neighbor comparison.
# nanoflann is a single header; it is fetched here rather than vendored (this example is a throwaway
# comparison harness, not a redistributed part of EBGeometry). Licensing for this file is handled in
# the repository-root REUSE.toml (ThirdParty/**/GNUmakefile).

EBGEOMETRY_HOME ?= ../../..
PRECISION       ?= double
CXX             ?= g++
CXXFLAGS        ?= -std=c++17 -O3 -march=native -Wall -Wextra

NANOFLANN_URL   := https://raw.githubusercontent.com/jlblancoc/nanoflann/master/include/nanoflann.hpp

NearestNeighbor.ex: main.cpp nanoflann.hpp
	$(CXX) $(CXXFLAGS) -DEBGEOMETRY_PRECISION=$(PRECISION) -I$(EBGEOMETRY_HOME) -I. main.cpp -o $@

# Fetch the nanoflann single header if it is not already present.
nanoflann.hpp:
	curl -sSL -o $@ $(NANOFLANN_URL)

.PHONY: clean realclean
clean:
	rm -f NearestNeighbor.ex

realclean: clean
	rm -f nanoflann.hpp
