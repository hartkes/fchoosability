
// fchoosability_exhaustive.cpp
// C++ program to determine if a graph is f-choosable, exhaustively checking all lists.
// Copyright 2017 by Stephen Hartke.
// Licensed under the GPL version 3.

#include <iostream>
#include <string>
#include <cstdio>  // for printf
#include <ctime>  // for reporting runtime
#include "fgraph.h"
#include "bitarray.h"
#include "subgraph.h"
#include "listassignment.h"


bool is_fchoosable(const fGraph& G)
    // We test if the fgraph G is f-choosable.
{
    std::vector<bitarray> neighbors;
    //unsigned long int count;
    
    // create the neighbor bit masks for G
    neighbors.resize(G.n);
    for (int i=G.n-1; i>=0; i--)
    {
        neighbors[i]=0;
        for (int j=G.n-1; j>=0; j--)
        {
            neighbors[i]<<=1;
            if ((j!=i) && (G.get_adj(i,j)!=0))  // i and j are different and adjacent
                neighbors[i]|=1;  // set the low order bit
        }
        
        /*
        printf("%2d: ",i);
        print_binary(neighbors[i],G.n);
        printf("\n");
        //*/
    }
    
    ListAssignment list_assignment(G.n,neighbors,G.f);
    if (list_assignment.verify())
    {
        printf("This graph is f-choosable!\n");
        return true;
    }
    else
    {
        printf("This graph is NOT f-choosable!\n");
        return false;
    }
}


int main()
{
    std::string line_in;
    fGraph G;
    int val=0;
    clock_t start,end;  // for reporting CPU runtime
    
    // We read lines in from stdin.
    // Each line should be in fgraph6 format.
    // If a line starts with '>', then it is treated as a comment.
    
    while (std::getline(std::cin,line_in))
    {
        if (line_in.length()<=3)  // this line is too short, probably end of file
            continue;
        
        if (line_in[0]=='>')  // treat this line as a comment
            continue;
        
        start=clock();  // record starting time
        
        G.read_fgraph6_string(line_in);
        
        printf("Input read: n=%d %s\n",G.n,line_in.c_str());
        
        val=is_fchoosable(G);
        
        end=clock();
        printf("    CPU time used: %.3f seconds\n\n",((double)(end-start))/CLOCKS_PER_SEC);
    }
    
    return val;  // return true if the graph is f-choosable
}
