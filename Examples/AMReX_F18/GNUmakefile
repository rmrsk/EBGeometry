DEBUG = FALSE
TEST = FALSE
USE_ASSERTION = FALSE

USE_EB = TRUE

USE_MPI  = TRUE
USE_OMP  = FALSE

COMP = gnu
CXXSTD = c++17

DIM = 3

#LIBRARIES += -lstd++fs

CXX=g++
CC=gcc

AMREX_HOME ?= ../../../../amrex

include $(AMREX_HOME)/Tools/GNUMake/Make.defs

include ./Make.package

Pdirs := Base Boundary AmrCore EB

Ppack	+= $(foreach dir, $(Pdirs), $(AMREX_HOME)/Src/$(dir)/Make.package)

include $(Ppack)

include $(AMREX_HOME)/Tools/GNUMake/Make.rules
