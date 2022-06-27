DEBUG = FALSE
TEST = TRUE
USE_ASSERTION = TRUE

USE_EB = TRUE

USE_MPI  = FALSE
USE_OMP  = FALSE

COMP = gnu
CXXSTD = c++17

DIM = 3

LIBRARIES += -lstd++fs

CXX=g++-8
CC=gcc-8

AMREX_HOME ?= ../../../../amrex

include $(AMREX_HOME)/Tools/GNUMake/Make.defs

include ./Make.package

Pdirs := Base Boundary AmrCore EB

Ppack	+= $(foreach dir, $(Pdirs), $(AMREX_HOME)/Src/$(dir)/Make.package)

include $(Ppack)

include $(AMREX_HOME)/Tools/GNUMake/Make.rules
