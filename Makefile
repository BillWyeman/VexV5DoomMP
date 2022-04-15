################################################################################
######################### User configurable parameters #########################
# filename extensions
CEXTS:=c
ASMEXTS:=s S
CXXEXTS:=cpp c++ cc
WADEXTS:=wad

# probably shouldn't modify these, but you may need them below
ROOT=.
FWDIR:=$(ROOT)/firmware
BINDIR=$(ROOT)/bin
SRCDIR=$(ROOT)/src
INCDIR=$(ROOT)/include

WARNFLAGS+=
EXTRA_CFLAGS=-Wfatal-errors
EXTRA_CXXFLAGS=

# Set to 1 to enable hot/cold linking
USE_PACKAGE:=1

# Set this to 1 to add additional rules to compile your project as a PROS library template
IS_LIBRARY:=1
# TODO: CHANGE THIS!
LIBNAME:=libbest
VERSION:=1.0.0
EXCLUDE_SRC_FROM_LIB=$(wildcard ./src/*.cpp)
EXCLUDE_SRC_FROM_LIB+=$(wildcard ./src/*.c)
EXCLUDE_SRC_FROM_LIB+=$(wildcard ./src/doom/*.c)
EXCLUDE_SRC_FROM_LIB+=$(wildcard ./src/doom/*.h)
EXCLUDE_SRC_FROM_LIB+=$(wildcard ./src/doom/chocdoom/*.c)
EXCLUDE_SRC_FROM_LIB+=$(wildcard ./src/doom/chocdoom/*.h)
# this line excludes opcontrol.c and similar files
EXCLUDE_SRC_FROM_LIB+=$(foreach file, $(SRCDIR)/opcontrol $(SRCDIR)/initialize $(SRCDIR)/autonomous,$(foreach cext,$(CEXTS),$(file).$(cext)) $(foreach cxxext,$(CXXEXTS),$(file).$(cxxext)))

# files that get distributed to every user (beyond your source archive) - add
# whatever files you want here. This line is configured to add all header files
# that are in the the include directory get exported
TEMPLATE_FILES=$(INCDIR)/**/*.h $(INCDIR)/**/*.hpp

.DEFAULT_GOAL=quick

upload:
	prosv5 upload

################################################################################
################################################################################
########## Nothing below this line should be edited by typical users ###########
-include ./common.mk
