
#include <iostream>
#include <string>
#include <cstdio>  // for printf
#include <vector>
#include "bitarray.h"
#include "subgraph.h"
#include "graph.h"
#include "fgraph.h"

int main()
{
    std::string line_in;
    fGraph G;
    std::vector<bitarray> neighbors;
    int i,j;
    int root;
    bitarray eligible_vertices;
    unsigned long int count;
    
    while (std::getline(std::cin,line_in))
    {
        if (line_in.length()<=3)  // this line is too short, probably end of file
            continue;
        
        if (line_in[0]=='>')  // treat this line as a comment
            continue;
        
        G.read_fgraph6_string(line_in);
        printf("Input read: n=%d %s\n",G.n,line_in.c_str());
        
        // create the neighbor bit masks for G
        neighbors.resize(G.n);
        for (i=G.n-1; i>=0; i--)
        {
            neighbors[i]=0;
            for (j=G.n-1; j>=0; j--)
            {
                neighbors[i]<<=1;
                if ((j!=i) && (G.get_adj(i,j)!=0))  // i and j are different and adjacent
                    neighbors[i]|=1;  // set the low order bit
            }
            
            printf("%2d: ",i);
            print_binary(neighbors[i],G.n);
            printf("\n");
        }
        
        root=5;  //0;  //5;
        //eligible_vertices=(1<<G.n)-1;
        //                  109876543210
        eligible_vertices=0b001001000001;
        
        ConnectedSubgraph S(G.n,neighbors);
        S.initialize(root,eligible_vertices);
        count=0;
        printf("Starting subgraph generation.\n");
        while (S.next(eligible_vertices))
        {
            // do something with the subgraph
            count++;
            printf("SUBGRAPH count=%10lu cur_layer=%2d ",count,S.cur_layer);
            print_binary(S.layer[S.cur_layer].union_subset,G.n);
            printf("\n");
        };
        printf("   Final count=%10lu\n",count);
    }
}
