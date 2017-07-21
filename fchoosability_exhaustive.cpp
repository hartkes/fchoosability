
// fchoosability_exhaustive.cpp
// C++ program to determine if a graph is f-choosable, exhaustively checking all lists.
// Copyright 2017 by Stephen Hartke.
// Licensed under the GPL version 3.

#include <iostream>
#include <string>
#include <cstdio>  // for printf
#include <ctime>  // for reporting runtime
#include <unistd.h>  // to use getopt to parse the command line
#include "fgraph.h"
#include "bitarray.h"
#include "subgraph.h"
#include "listassignment.h"


/* Command line parameters can be used for parallelization.
 * The system used is inspired by that used in Brendan McKay's geng and in Brinkmann and McKay's plantri.
 * -m specifies a modulus, and the search tree is split into roughly m pieces to examine.
 * This is done by chopping the search tree at a given height (given by the variable splitlevel),
 * counting the nodes (ie, precolorings) at that level, and only expanding those with the specified residue.
 * 
 * One advantage is that modulo classes work as expected, in that 1 (mod 4) and 3 (mod 4) gives 1 (mod 2).
 * Note that there's a limit to this, as the number of nodes on the level being split is only 100*modulus.
 * 
 * If the modulus is larger than the number of nodes at that level, the last residues (up to mod-1) are the ones that are actually examined, since we start counting at the mod-1 and count down.
 */



bool is_fchoosable(const fGraph& G,int res,int mod,int splitlevel)
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
    if (list_assignment.verify(res,mod,splitlevel))
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


int main(int argc, char *argv[])
{
    std::string line_in;
    fGraph G;
    int val=0;
    clock_t start,end;  // for reporting CPU runtime
    
    int res,mod,splitlevel_arg,splitlevel;  // for parallelizing
    int opt;  // for parsing the command line
    
    // defaults
    splitlevel_arg=-1;
    res=-1;
    mod=-1;
    
    // parse the command line
    while ((opt=getopt(argc,argv,"r:m:s:"))!=-1)  // the colons indicate the options take required arguments
    {
        switch (opt)
        {
            case 'r':
                sscanf(optarg,"%d",&res);
                break;
            case 'm':
                sscanf(optarg,"%d",&mod);
                break;
            case 's':
                sscanf(optarg,"%d",&splitlevel_arg);
                break;
            case '?':
                printf("Error parsing command line arguments; problem with option %c\n",optopt);
                printf("USAGE: fchoosability_exhaustive -r residue -m modulus -s splitlevel\n");
                printf("-r and -m must be used together; -s can only be used if -r/-m also are.\n");
                exit(8);
            default:
                ;
        }
    }
    printf("Parallelization parameters set: res=%d mod=%d splitlevel_arg=%d\n",res,mod,splitlevel_arg);
    
    if ((res==-1) ^ (mod==-1))  // using bitwise xor here since there is no logical xor
    {
        printf("-r and -m must be used together\n");
        exit(8);
    }
    if ((splitlevel_arg!=-1) && (mod==-1))
    {
        printf("-s can only be used if -r and -m also are.\n");
        exit(8);
    }
    
    
    
    
    
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
        
        if (mod==-1)  // not using parallelization
        {
            int sum=0;
            for (int i=G.n-1; i>=0; i--)
                sum+=G.f[i];
            splitlevel=sum+1;  // will never reach this level
            printf("not parallelizing\n");
        }
        else 
        {
            if (splitlevel_arg!=-1)
                splitlevel=splitlevel_arg;
            else
                splitlevel=3;
                //splitlevel=splitlevel_heuristic(max_num_colors,num_verts_to_precolor,G,
                //                                mod);
            printf("parallelizing with splitlevel=%d\n",splitlevel);
        }
        
        val=is_fchoosable(G,res,mod,splitlevel);
        
        end=clock();
        printf("    CPU time used: %.3f seconds\n\n",((double)(end-start))/CLOCKS_PER_SEC);
    }
    
    return val;  // return true if the graph is f-choosable
}
