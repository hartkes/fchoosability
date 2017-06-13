
name=fchoosability_nullstellensatz
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

all: $(name)

debug:
	make "BUILD=debug"

$(name): $(name).cpp graph.h fgraph.h compositions.h
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(name).cpp $(LIBRARIES) -o $(name)

clean:
	rm -f $(name)

test_compositions: test_compositions.cpp compositions.h
	$(CXX) $(CPPFLAGS) test_compositions.cpp -o test_compositions
