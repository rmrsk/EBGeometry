.. _Chap:BuildingGNUMake:

Building with GNU Make
========================

.. contents:: On this page
   :local:
   :depth: 2

A minimal ``Makefile``
------------------------

A minimal ``Makefile`` that compiles a single ``main.cpp`` against ``EBGeometry``:

.. code-block:: make

   # Path to the root of the EBGeometry source tree
   EBGEOMETRY_DIR := /path/to/EBGeometry

   CXX      := g++
   CXXFLAGS := -std=c++17 -O3 -mavx -mfma -msse4.1
   CXXFLAGS += -I$(EBGEOMETRY_DIR)

   # Uncomment to enable runtime assertions:
   # CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS

   TARGET   := my_program
   SRCS     := main.cpp

   $(TARGET): $(SRCS)
   	$(CXX) $(CXXFLAGS) $^ -o $@

   .PHONY: clean
   clean:
   	rm -f $(TARGET)

Because ``EBGeometry`` is header-only, there are no object files or static libraries
to build; the ``$(TARGET)`` rule is the only build step required.  Every example under
:file:`Examples/EBGeometry_<something>` ships a ``GNUmakefile`` following this same pattern —
see :ref:`Chap:Examples`.

Conditional SIMD selection
----------------------------

To choose the SIMD level at make-time rather than hard-coding it:

.. code-block:: make

   EBGEOMETRY_DIR := /path/to/EBGeometry

   CXX      := g++
   CXXFLAGS := -std=c++17 -O3 -I$(EBGEOMETRY_DIR)

   # SIMD=avx (default), sse41, or none
   SIMD ?= avx
   ifeq ($(SIMD),avx)
     CXXFLAGS += -mavx -mfma -msse4.1
   else ifeq ($(SIMD),sse41)
     CXXFLAGS += -msse4.1
   endif

   # Assertions: make ASSERTIONS=1 to enable
   ifeq ($(ASSERTIONS),1)
     CXXFLAGS += -DEBGEOMETRY_ENABLE_ASSERTIONS
   endif

   TARGET := my_program
   $(TARGET): main.cpp
   	$(CXX) $(CXXFLAGS) $< -o $@

Invoke with, for example:

.. code-block:: bash

   make SIMD=avx                    # AVX + FMA (default)
   make SIMD=sse41 ASSERTIONS=1     # SSE4.1, assertions on
   make SIMD=none                   # scalar fallback

See :ref:`Chap:SIMDAndAssertions` for what each SIMD level means for BVH branching factor and
SoA width, and for the semantics of ``EBGEOMETRY_ENABLE_ASSERTIONS``.
