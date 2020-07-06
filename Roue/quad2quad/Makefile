# Makefile to build class 'moonlib' for Pure Data.
# Needs Makefile.pdlibbuilder as helper makefile for platform-dependent build
# settings and rules.
#
# use : make pdincludepath=/path/to/pure-data/src/
#
# The following command will build the external and install the distributable
# files into a subdirectory called build/moonlib :
#
# make install pdincludepath=../pure-data/src/ objectsdir=./build

lib.name = quad2quad

ldlibs = -lgsl -lgslcblas

objects = quad2quad

class.sources = $(addsuffix .c,$(objects))

# all extra files to be included in binary distribution of the library
datafiles = \
    $(addsuffix -help.pd,$(objects)) \
    LICENSE.txt \
    README.txt

cflags = -Wno-unused -Wno-unused-parameter

PDLIBBUILDERDIR ?= .
include $(PDLIBBUILDERDIR)/Makefile.pdlibbuilder

VERSION = $(shell git describe --abbrev=4)

update-pdlibbuilder:
	curl https://raw.githubusercontent.com/pure-data/pd-lib-builder/master/Makefile.pdlibbuilder > ./Makefile.pdlibbuilder

deken-source:
	@rm -rf build_src
	@mkdir -p build_src/quad2quad
	@cp $(class.sources) $(datafiles) Makefile.pdlibbuilder Makefile build_src/quad2quad
	@cp -r $(datadirs) build_src/quad2quad
	cd build_src/ ; deken upload -v $(VERSION) quad2quad

deken-binary:
	@rm -rf build
	@make install objectsdir=./build
	cd build/ ; deken upload -v $(VERSION) quad2quad


