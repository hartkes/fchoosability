
name=fchoosability_nullstellensatz
CXX=g++
#CXX=clang++

CPPFLAGS=-O5 -std=c++11 -D_GLIBCXX_DEBUG
INCLUDES=-I./libexact-1.0
LIBRARIES=-L./libexact-1.0 -lexact

$(name): $(name).cpp graph.h fgraph.h compositions.h
	$(CXX) $(CPPFLAGS) $(INCLUDES) $(name).cpp $(LIBRARIES) -o $(name)

test_compositions: test_compositions.cpp compositions.h
	$(CXX) $(CPPFLAGS) test_compositions.cpp -o test_compositions
