.PHONY: all debug release clean

Compiler = g++
RELEASE_FLAGS = -O2 -Wall -DNDEBUG 
DEBUG_FLAGS   = -g -O0 -Wall 
RELEASE_EXEC  = daytime
DEBUG_EXEC    = daytime-dbg
SOURCES       = daytime.cpp
BOOST = -lboost_program_options
all: debug release

debug: $(DEBUG_EXEC)

$(DEBUG_EXEC): $(SOURCES)
	$(Compiler) $(DEBUG_FLAGS) $(SOURCES) -o $(DEBUG_EXEC) $(BOOST)

release: $(RELEASE_EXEC)

$(RELEASE_EXEC): $(SOURCES)
	$(Compiler) $(RELEASE_FLAGS) $(SOURCES) -o $(RELEASE_EXEC) $(BOOST)

clean:
	rm -f $(RELEASE_EXEC) $(DEBUG_EXEC)
