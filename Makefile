
CXX=g++
#CXX=clang++

CPPFLAGS= -std=c++11 -O5

# -D_GLIBCXX_DEBUG (among other things) turns on array bounds checking for [] on std::vector
DEBUGFLAGS= -Wall -g -D_GLIBCXX_DEBUG -fsanitize=address

RELEASEFLAGS= 
INCLUDES=-I./libexact-1.0
LIBRARIES=-L./libexact-1.0 -lexact


# debug option inspired by:
# https://stackoverflow.com/questions/1534912/how-to-build-in-release-mode-with-optimizations-in-gcc/1536681#1536681

ifeq ($(BUILD),debug)
# "Debug" build - no optimization, and debugging symbols
CPPFLAGS += $(DEBUGFLAGS)
else
# "Release" build - optimization, and no debug symbols
CPPFLAGS += $(RELEASEFLAGS)
endif

all: fchoosability_nullstellensatz fchoosability_exhaustive

debug:
	make "BUILD=debug"

fchoosability_exhaustive: fchoosability_exhaustive.cpp graph.h fgraph.h bitarray.h subgraph.h listassignment.h
	$(CXX) $(CPPFLAGS) $(INCLUDES) fchoosability_exhaustive.cpp $(LIBRARIES) -o fchoosability_exhaustive

fchoosability_nullstellensatz: fchoosability_nullstellensatz.cpp graph.h fgraph.h compositions.h
	$(CXX) $(CPPFLAGS) $(INCLUDES) fchoosability_nullstellensatz.cpp $(LIBRARIES) -o fchoosability_nullstellensatz

clean:
	rm -f fchoosability_exhaustive fchoosability_nullstellensatz

test_compositions: test_compositions.cpp compositions.h
	$(CXX) $(CPPFLAGS) $(DEBUGFLAGS) test_compositions.cpp -o test_compositions

test_bitarray: test_bitarray.cpp bitarray.h
	$(CXX) $(CPPFLAGS) $(DEBUGFLAGS) test_bitarray.cpp -o test_bitarray

test_subgraph: test_subgraph.cpp bitarray.h subgraph.h
	$(CXX) $(CPPFLAGS) $(DEBUGFLAGS) test_subgraph.cpp -o test_subgraph


