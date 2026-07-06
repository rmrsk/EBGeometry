DEBUG = FALSE
TEST = FALSE
USE_ASSERTION = FALSE

CXXFLAGS += -O3 -march=native

USE_EB = TRUE

USE_MPI  = TRUE
USE_OMP  = FALSE

COMP = gnu

DIM = 3

AMREX_HOME ?= ../../../../amrex

include $(AMREX_HOME)/Tools/GNUMake/Make.defs

include ./Make.package

Pdirs := Base Boundary AmrCore EB

Ppack	+= $(foreach dir, $(Pdirs), $(AMREX_HOME)/Src/$(dir)/Make.package)

include $(Ppack)

include $(AMREX_HOME)/Tools/GNUMake/Make.rules
