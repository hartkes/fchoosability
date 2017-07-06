
// listassignment.h
// This is a C++ library for generating partial list assignments on a graph.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#pragma once  // for compilers that support this
#ifndef LISTASSIGNMENT_H__HOLJNEPCVXFOXMPEAIHG  // include guard
#define LISTASSIGNMENT_H__HOLJNEPCVXFOXMPEAIHG


#include <vector>
#include "bitarray.h"
#include "subgraph.h"


////////////////////////////////////////////////////////////////////////////////////////
class ColorabilityClassInfo
////////////////////////////////////////////////////////////////////////////////////////
{
public:
    // we only needs these for the copy and copy assignment constructors
    int n;  // number of vertices in the whole graph
    const std::vector<bitarray> &neighbors;  // adjacencies in the graph
    
    bitarray colorability_class;  // the set of vertices assigned this color
    std::vector<int> L;  // for each vertex, the number of colors already assigned to its list; does not include colorability_class for this color
    bitarray eligible_vertices;  // set of vertices whose lists are not full (ie, L[v]<f[v]) and so can be included in this color's colorability class
    
    std::vector<ConnectedSubgraph> generator;  // pointers to subgraph generators rooted at each vertex
    bitarray eligible_generators;  // set of vertices whose generators are eligible to generate another subgraph; this is a subset of eligible vertices; eligible vertices not included are those generators that have finished generating subgraphs rooted at that vertex.

    ColorabilityClassInfo(int n,const std::vector<bitarray> &neighbors);
    ColorabilityClassInfo(const ColorabilityClassInfo &other);
    ColorabilityClassInfo& operator=(const ColorabilityClassInfo &other);  //FIXME: Why do we need this????
    
    bool generate_subgraph();
    void setup_next_from(const ColorabilityClassInfo &prev,const std::vector<int> &f);
};


ColorabilityClassInfo::ColorabilityClassInfo(int n,const std::vector<bitarray> &neighbors)
  : // intialization list
    n(n),
    neighbors(neighbors),
    L(n,0),  // intialize L to 0
    generator(n,ConnectedSubgraph(n,neighbors))
{
}


ColorabilityClassInfo::ColorabilityClassInfo(const ColorabilityClassInfo &other)
    // copy constructor, used in initialization of vectors of these objects
  : // intialization list
    n(other.n),
    neighbors(other.neighbors),
    colorability_class(other.colorability_class),
    L(other.n,0),
    eligible_vertices(other.eligible_vertices),
    generator(other.n,ConnectedSubgraph(other.n,other.neighbors)),
    eligible_generators(other.eligible_generators)
{
    //printf("ColorabilityClassInfo copy constructor\n");
}


ColorabilityClassInfo& ColorabilityClassInfo::operator=(const ColorabilityClassInfo &other)
    // copy assignment constructor, used in initialization of vectors of these objects

{
    printf("ColorabilityClassInfo copy assignment constructor\n");
    ColorabilityClassInfo tmp(other);
    //FIXME: This is not actually implemented, since it seems to never be called.
    exit(5);
}


bool ColorabilityClassInfo::generate_subgraph()
    // Generates another connected subgraph to use as the next colorability class.
    // This subgraph is stored in the member colorability_class.
    // Returns false if there are no more subgraphs to generate.
{
    while (true)  // continue until we generate a subgraph
    {
        // Find the vertex at which the subgraph generator which we'll poke is rooted.
        // As a heuristic, we'll find the vertex with minimum L value (ie, number of colors currently assigned to that vertex's list), and then among those vertices, last by index.
        
        int min_L=n;
        int min_v=0;
        bitarray mask=1<<(n-1);
        
        printf("Searching for vertex v to poke its generator\n");
        for (int v=n-1; v>=0; v--)
        {
            //*
            printf("v=%2d mask=",v);
            print_binary(mask,n);
            printf(" eligible_generators=");
            print_binary(eligible_generators,n);
            printf(" min_L=%2d L[v]=%d",min_L,L[v]);
            printf("\n");
            //*/
            
            if ( (mask & eligible_generators) &&
                    (L[v]<min_L) )
            {
                min_L=L[v];
                min_v=v;
            }
            mask>>=1;
        }
        
        printf("Done with loop: min_v=%d min_L=%d\n",min_v,min_L);
        
        if (min_L<n)  // we found a vertex to poke!
        {
            if (generator[min_v].next())
            {
                // we successfully generated a new subgraph!
                printf("We successfully generated a new subgraph!\n");
                colorability_class=
                    generator[min_v].
                    layer[generator[min_v].cur_layer].union_subset;
                
                //*
                printf("Generated subgraph!, min_v=%2d  subgraph=",min_v);
                print_binary(colorability_class,n);
                printf("\n");
                //*/
                
                return true;
            }
            else
            {
                eligible_generators^=(1<<min_v);  // vertex min_v no longer has an eligible generator, since it has finished generating all its subgraphs
                
                // We try to find another generator to poke.
            }
        }
        else
            return false;
    }
    
    // we should never get here, but in case we do:
    printf("Something wrong with generating the next subgraph!\n");
    exit(93);
    return false;
}


void ColorabilityClassInfo::setup_next_from(const ColorabilityClassInfo &prev,
                                            const std::vector<int> &f)
{
    // We assume that prev.colorability_class is a valid connected subgraph that is not represented in the L or eligible_vertices members of prev.
    
    eligible_vertices=prev.eligible_vertices;  // we will modify as necessary
    
    bitarray mask=1<<(n-1);
    for (int v=n-1; v>=0; v--)
    {
        // TODO: we could eliminate the if statement by having a conditional add to L[v]; the tricky thing is how to handle eligible vertices; can't be a xor anymore; probably can use an & mask (e_v&=);
        if (prev.colorability_class & mask)  // vertex v is in the subgraph
        {
            //printf("       v=%2d is in the subgraph\n",v);
            L[v]=prev.L[v]+1;
            if (L[v]>=f[v])
                eligible_vertices^=mask;
            //eligible_vertices^=((L[v]!=f[v])-1)&mask;  // if v's list is now full, v is no longer eligible to be put in other subgraphs
        }
        else
            L[v]=prev.L[v];
        
        mask>>=1;
    }
    
    for (int i=n-1; i>=0; i--)
        generator[i].copy_from(prev.generator[i]);
    eligible_generators=prev.eligible_generators;
    
    colorability_class=prev.colorability_class;  // we copy this as a default, so that we can use it when adding a colorability class multiple times
}


////////////////////////////////////////////////////////////////////////////////////////
class ListAssignment
////////////////////////////////////////////////////////////////////////////////////////
{
public:
    int n;  // number of vertices in the whole graph
    const std::vector<bitarray> &neighbors;  // adjacencies in the graph
    const std::vector<int> &f;  // the f vector of allowed list sizes
    
    std::vector<ColorabilityClassInfo> color_info;
    int cur_color;  // the current (and last) colorability class; thus there are cur_color+1 total colorability classes
    
// methods:
    ListAssignment(
        int n,
        const std::vector<bitarray> &neighbors,
        const std::vector<int> &f);
    
    bool verify();
};


ListAssignment::ListAssignment(
    int n,
    const std::vector<bitarray> &neighbors,
    const std::vector<int> &f)
  :  // initializer list
    n(n),
    neighbors(neighbors),
    f(f)
{
    //printf("ListAssignment constructor.\n");
    
    int sum=0;
    for (int i=n-1; i>=0; i--)
        sum+=f[i];
    printf("Reserving colorability class info stack to size %d\n",sum);
    color_info.resize(sum,ColorabilityClassInfo(n,neighbors));
    
    // intialize the stack for the first colorability class
    cur_color=0;
    color_info[cur_color].colorability_class=0;  // not yet decided, so initialize to 0
    color_info[cur_color].eligible_vertices=0;  // we fill in the values in the following loop
    for (int i=n-1; i>=0; i--)
    {
        color_info[cur_color].L[i]=0;  // no colors assigned yet
        printf("         L[%2d]=%2d\n",i,color_info[cur_color].L[i]);
        color_info[cur_color].eligible_vertices<<=1;
        color_info[cur_color].eligible_vertices|=(f[i]>0);  // mark vertices eligible if they have room in their lists
    }
    color_info[cur_color].eligible_generators=color_info[cur_color].eligible_vertices;
    
    bitarray mask=1;
    for (int i=0; i<n; i++)  // we need a separate loop since eligible_vertices must be computed
    {
        color_info[cur_color].generator[i].initialize(
            i,  // root
            color_info[cur_color].eligible_vertices & mask);
                // the eligible vertices are only those with lower index (or the same) as the root
        
        /*/
        printf("i=%2d eligible_vertices=",i);
        print_binary(color_info[cur_color].eligible_vertices,n);
        printf("  mask=");
        print_binary(mask,n);
        printf("\n");
        //*/
        
        mask<<=1;  // shift left 1
        mask|=1;  // and put a 1 in the low order bit
    }
}


bool ListAssignment::verify()
{
    // This functions contains the main loop that generates and verifies list assignments.
    
    while (cur_color>=0)
    {
        //*/
        printf("Starting main loop, cur_color=%d\n",cur_color);
        for (int i=0; i<=cur_color; i++)
        {
            printf("color=%2d  ",i);
            print_binary(color_info[i].colorability_class,n);
            printf("\n");
        }
        for (int v=0; v<n; v++)
        {
            printf("   v=%2d  L[v]=%d  f[v]=%d\n",v,color_info[cur_color].L[v],f[v]);
        }
        printf("  eligible_vertices=");
        print_binary(color_info[cur_color].eligible_vertices,n);
        printf("\n");
        printf("eligible_generators=");
        print_binary(color_info[cur_color].eligible_generators,n);
        printf("\n");
        //*/
        
        
        
        // When this loop starts, cur_color points to the next colorability class that we will try to generate and add to our list assignment.
        
        if (color_info[cur_color].generate_subgraph())
        {
            
            // We need to check if this partial list assignment is suitable, ie, if there is a feasible coloring.
            // TODO: At the moment, we assume it's fine.
            
            
            
            // This partial list assignment is okay, so we advance.
            
            while ( (color_info[cur_color].colorability_class & 
                     color_info[cur_color].eligible_vertices)
                   ==color_info[cur_color].colorability_class )
                // As long as this connected subgraph is eligible, keep adding it to the list assignment.
            {
                color_info[cur_color+1].setup_next_from(color_info[cur_color],f);
                cur_color++;
                printf("incrementing cur_color to %d\n",cur_color);
                printf(" colorability_class=");
                print_binary(color_info[cur_color].colorability_class,n);
                printf("\n");
                printf("  eligible_vertices=");
                print_binary(color_info[cur_color].eligible_vertices,n);
                printf("\n");
            }
        }
        else  // There are no more subgraphs to generate, so we must backtrack.
        {
            printf("No more subgraphs to generate!  We must backtrack!\n");
            exit(4);
        }
        
    }
    
    
    return false;
}


#endif  // LISTASSIGNMENT_H__HOLJNEPCVXFOXMPEAIHG
