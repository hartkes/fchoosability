
name=fchoosability_nullstellensatz
CXX=g++
#CXX=clang++

CPPFLAGS= -std=c++11 -O5
DEBUGFLAGS= -Wall -g -D_GLIBCXX_DEBUG
RELEASEFLAGS= 
INCLUDES=-I./libexact-1.0
LIBRARIES=-L./libexact-1.0 -lexact

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
