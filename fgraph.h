
// fgraph.h
// This is a C++ library for dealing with undirected graphs with an f-vector.
// The main class inherits from the main class in graph.h.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#pragma once  // for compilers that support this
#ifndef FGRAPH_H__IECHAIFEUS  // include guard
#define FGRAPH_H__IECHAIFEUS


#include <vector>
#include "graph.h"

/////////////////////////////////////////////////////////////////////////////
// Helper functions for "radix64" representation.
/////////////////////////////////////////////////////////////////////////////

const char mapping[]="0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz@#";
const int inverse_mapping[]={
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,63,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
         0, 1, 2, 3, 4, 5, 6, 7, 8, 9,-1,-1,-1,-1,-1,-1,
        62,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,
        25,26,27,28,29,30,31,32,33,34,35,-1,-1,-1,-1,-1,
        -1,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,
        51,52,53,54,55,56,57,58,59,60,61,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,
        -1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1,-1};

inline
char encode_6bits(int x)
{
    return mapping[x];
}

inline
int decode_6bits(char x)
{
    return inverse_mapping[(size_t)x];  // cast char x to be appropriate for indexing
}


/////////////////////////////////////////////////////////////////////////////
// The main class for undirected graphs that also have an f-vector.
/////////////////////////////////////////////////////////////////////////////

class fGraph : public UndirectedGraph
    // This base class adds an f-vector (indexed by the vertices) to an undirected graph.
    // The f-vector is used for determining f-choosability.
    // The adjacency matrix can hold 0,1 values, or arbitrary integers.
{
public:
    std::vector<int> f;  // the f-vector, indexed by the vertices
    fGraph();
    fGraph(const fGraph &G);  // create a copy of the fgraph G
    fGraph(char *fgraph6);  // create a graph from the given fgraph6 string
    //~fGraph();  // no destructor needed, since f is automatically deallocated
    
    void allocate(int n);
    void copy_from(const fGraph& H);
    
    void read_fgraph6_string(char *fgraph6);
    int remove_vertices_with_f_1();
};


fGraph::fGraph() 
       :UndirectedGraph()  // call the base constructor
{
    f.resize(0);
}


fGraph::fGraph(const fGraph& G) 
       :UndirectedGraph()  // call the base constructor
{
    int i;
    
    //printf("Constructor fGraph, about to call allocate\n");
    allocate(G.n);  // allocate the data structures
    //printf("allocate has been called\n");
    
    // We need to copy the adjacencies from G to the new graph.
    for (i=nchoose2-1; i>=0; i--)  // counting down is faster because the condition is testing against 0
        adj[i]=G.adj[i];
    
    // We need to copy the f vector from G to the new graph.
    for (i=n-1; i>=0; i--)
        f[i]=G.f[i];
    
    //printf("Done with constructor\n");
}


fGraph::fGraph(char *fgraph6)  // create a graph from the given fgraph6 string
       :UndirectedGraph()  // call the base constructor
{
    read_fgraph6_string(fgraph6);
}


void fGraph::allocate(int n)
{
    //printf("fGraph::allocate, passed-in n=%d, this->n=%d\n",n,this->n);
    
    f.resize(n);
    UndirectedGraph::allocate(n);  // this handles the UndirectedGraph structures, as well as changes n and nchoose2
    
    //printf("done fGraph::allocate, n=%d, this->n=%d, dss=%d\n",n,this->n,data_structure_size);
}


void fGraph::copy_from(const fGraph& H)
{
    int i;
    
    allocate(H.n);
    
    // copy the adj matrix
    for (i=nchoose2-1; i>=0; i--)
        adj[i]=H.adj[i];
    
    // copy the f vector
    for (i=n-1; i>=0; i--)
        f[i]=H.f[i];
}


void fGraph::read_fgraph6_string(char *fgraph6)
    // Reads in fgraph6 format
{
    char *cur;  // the current character in the string that we're considering
    int gn;  // the number of vertices in the graph in fgraph6
    int val,mask;
    int i,j;
    
    cur=fgraph6;
    gn=decode_6bits(*cur);
    
    //printf("reading fgraph6 string, new n=%d\n",gn);
    allocate(gn);  // this also sets this->n to gn
    
    cur+=2;  // advance past the underscore
    
    // read in the f-vector
    for (i=0; i<n; i++)
    {
        f[i]=decode_6bits(*cur);
        cur++;
    }
    
    cur++;  // advance past the underscore
    
    // read in the adjacency matrix
    val=decode_6bits(*cur);
    mask=1<<5;  // start with the high bit
    for (j=0; j<n; j++)  // adj matrix is bit packed in colex order in fgraph6 format
        for (i=0; i<j; i++)
        {
            set_adj_sorted(i,j,(val&mask)!=0);  // test whether that bit is nonzero
            mask>>=1;
            if (!mask)  // mask has become 0
            {
                cur++;
                val=decode_6bits(*cur);
                mask=1<<5;
            }
        }
}


int fGraph::remove_vertices_with_f_1()
    // We iteratively remove all of the vertices with f[i]=1,
    // decreasing the f of their neighbors by 1.
    // If an f[i] that is 0 or negative is encountered, the function returns False.
{
    int i,j,new_i;
    int new_n;
    
    // First we check if any of the initial f[i] values are <=0.
    for (i=n-1; i>=0; i--)
        if (f[i]<=0)
            return 0;
    
    /*
    printf("Trying to prune\n");
    printf("f=");
    for (i=0; i<n; i++)
        printf("%1d ",f[i]);
    printf("\n");
    print_adj_matrix();
    //*/
    
    new_n=n;
    i=n-1;
    while (i>=0)
    {
        // While checking, we maintain the condition that f[i]>=0, and f[i]==0 if and only if i is a vertex that has been eliminated.
        
        //printf("considering i=%d\n",i);
        
        if (f[i]==1)
            // Since f[i]==1, we need to remove the vertex i.
            // First, we'll delete all of the edges from i and reduce the f of the neighbors by 1.
        {
            //printf("  removing i=%d\n",i);
        
            for (j=i-1; j>=0; j--)
                if (get_adj_sorted(j,i)>0)  // j is a neighbor of i
                {
                    f[j]--;  // decrease f[j] by 1
                    if (f[j]==0)
                        return 0;  // this cannot occur
                    set_adj_sorted(j,i,0);  // remove the edge
                }
            
            new_i=i-1;
            for (j=i+1; j<n; j++)
                if (get_adj_sorted(i,j)>0)  // j is a neighbor of i
                {
                    f[j]--;  // decrease f[j] by 1
                    if (f[j]==0)
                        return 0;  // if an f[j] goes down to 0, then we cannot check this graph; return False
                    else if (f[j]==1)
                        new_i=j;
                    set_adj_sorted(i,j,0);  // remove the edge
                }
            
            f[i]=0;  // marks a vertex that has been eliminated (now isolated)
            new_n--;
            // FIXME:  It should never be the case that new_n can go down to -1.
            // PROBLEM!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!11
            
            i=new_i;  // what to check next; if a larger index went down to 1, check that
        }
        else
            // vertex i has f[i]>1, so it is not removed; move along.
            i--;
    }
    
    /*
    printf("Pruned.\n");
    printf("f=");
    for (i=0; i<n; i++)
        printf("%1d ",f[i]);
    printf("\n");
    print_adj_matrix();
    //*/
    
    if (new_n<=1)  // all but at most one of the vertices successfully removed in a greedy fashion
    {
        printf("new_n=%d\n",new_n);
        if (new_n<0)
            exit(8);
        return 2;
    }
    
    // we now compress the graph, moving the isolated vertices to the end
    
    for ( ; (n>0) && (f[n-1]==0); n--);  // find the last non-isolated vertex
    
    for (j=0; j<n; j++)
        if (f[j]==0)  // this is an isolated vertex
            if (j<n-1)  // j and n-1 are different vertices
            {
                // we swap vertices j and n-1
                //printf("Swapping vertices j=%d and n-1=%d\n",j,n-1);
                for (i=n-2; i>=0; i--)
                    if (i!=j)
                        set_adj(j,i,get_adj(n-1,i));
                
                f[j]=f[n-1];
                f[n-1]=0;
                
                for (n--; (n>0) && (f[n-1]==0); n--);  // find the last non-isolated vertex
            }
    
    if (n!=new_n)
    {
        printf("Problem! n=%d new_n=%d\n",n,new_n);
        exit(7);
    }
    
    /*
    printf("Compressed.\n");
    printf("f=");
    for (i=0; i<n; i++)
        printf("%1d ",f[i]);
    printf("\n");
    print_adj_matrix();
    //*/
    
    return 1;  // this graph is now ready to be checked with the Nullstellensatz
}


#endif  // FGRAPH_H__IECHAIFEUS
