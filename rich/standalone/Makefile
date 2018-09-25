CC=g++
SOURCES := $(wildcard *.cxx)                                                      
OBJECTS = $(SOURCES:.c=.o)                                                      
PROJNAME = $(notdir $(PWD))
COMPILE_FLAGS = -O3
BINARY= $(PROJNAME)
$(BINARY): $(OBJECTS)
	$(CC) $(COMPILE_FLAGS) $(LDFLAGS) -o  $@ $(OBJECTS)