# change to your local directories!
PD_APP_DIR = /usr/local/include/pd
GEM_DIR = /home/arsene/pd-externals/Gem

CPPFLAGS = -I$(GEM_DIR)/src -I$(PD_APP_DIR) 

#linux doesnt work yet
UNAME := $(shell uname -s)
ifeq ($(UNAME),Linux)
 CPPFLAGS += `pkg-config --cflags opencv` 
# CXXFLAGS = -g -O2 -fPIC -freg-struct-return -Os -falign-loops=32 -falign-functions=32 -falign-jumps=32 -funroll-loops -ffast-math -mmmx
 CXXFLAGS = -g -O2 -fPIC 
 LDFLAGS = -shared -rdynamic `pkg-config --libs opencv`
 LIBS = 
 EXTENSION = pd_linux
 USER_EXTERNALS=$(HOME)/pd-externals
endif
ifeq ($(UNAME),Darwin)
 CPPFLAGS += 
 CXXFLAGS = -arch i386 -g -fast -msse3
 LDFLAGS = -arch i386 -bundle -bundle_loader $(PD_APP_DIR)/bin/pd -undefined dynamic_lookup -mmacosx-version-min=10.6
 LIBS = -lm
 EXTENSION = pd_darwin
 USER_EXTERNALS=$(HOME)/Library/Pd
endif

.SUFFIXES = $(EXTENSION)

SOURCES = pix_cvblob

all:
	g++ $(CPPFLAGS) $(CXXFLAGS) -o $(SOURCES).o -c $(SOURCES).cpp
	g++ -o $(SOURCES).$(EXTENSION) $(LDFLAGS) $(SOURCES).o $(LIBS)
	rm -fr ./*.o

clean:
	rm -f $(SOURCES)*.o
	rm -f $(SOURCES)*.$(EXTENSION)

distro: clean all
	rm *.o
