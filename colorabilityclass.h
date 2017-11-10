
// colorabilityclass.h
// This is a C++ library for generating partial list assignments on a graph.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#pragma once  // for compilers that support this
#ifndef COLORABILITYCLASS_H__UOMEICEIHALUAFETHAEF  // include guard
#define COLORABILITYCLASS_H__UOMEICEIHALUAFETHAEF


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
        
        //printf("Searching for vertex v to poke its generator\n");
        for (int v=n-1; v>=0; v--)
        {
            /*
            printf("v=%2d mask=",v);
            print_binary(mask,n);
            printf(" eligible_generators=");
            print_binary(eligible_generators,n);
            printf(" min_L=%2d L[v]=%d",min_L,L[v]);
            printf("\n");
            //*/
            
            //*
            if ((mask & eligible_vertices) && (L[v]==0))
                // this if needs to be separate from the if below, since we're also testing vertices that might not have eligible generators
            {
                //printf("We found L[%d]==0\n",v);
                
                eligible_generators|=1<<n;  // this ensures the following loop terminates
                min_v=v;
                while (!(mask & eligible_generators))
                {
                    min_v++;
                    mask<<=1;
                }
                eligible_generators^=1<<n;  // clear this high bit
                
                if (mask & eligible_generators)
                    // Note that eligible_generators is a subset of eligible_vertices.
                {
                    // We can advance the vertex min_v, and it has the potential of covering v.
                    //printf("We're advancing min_v=%d\n",min_v);
                    min_L=0;
                    break;
                }
                else
                {
                    // There's no way for v to get a color in its list.  Thus, we return that there are no more subgraphs to generate.
                    /*
                    printf("v=%d cannot get a color in its list\n",v);
                    printf("el_verts=");
                    print_binary(eligible_vertices,n);
                    printf("\n");
                    printf(" el_gens=");
                    print_binary(eligible_generators,n);
                    printf("\n");
                    //*/
                    
                    return false;  // we are not returning a subgraph; we must backtrack
                }
            }
            //*/
            
            if ( (mask & eligible_generators) &&
                    (L[v]<min_L) )
            {
                min_L=L[v];
                min_v=v;
            }
            mask>>=1;
        }
        
        /*
        printf("Done with loop: min_v=%d min_L=%d\n",min_v,min_L);
        printf("eligible_vertices=");
        print_binary(eligible_vertices,n);
        printf("\n");
        //*/
        
        if (min_L<n)  // we found a vertex to poke!
        {
            if (generator[min_v].next(eligible_vertices))
            {
                // we successfully generated a new subgraph!
                //printf("We successfully generated a new subgraph!\n");
                colorability_class=
                    generator[min_v].
                    layer[generator[min_v].cur_layer].union_subset;
                
                /*
                printf("Generated subgraph!, min_v=%2d  subgraph=",min_v);
                print_binary(colorability_class,n);
                printf("\n");
                //*/
                
                return true;
            }
            else
            {
                eligible_generators^=(1<<min_v);  // vertex min_v no longer has an eligible generator, since it has finished generating all its subgraphs
                
                // We need to check if any vertex v with L[v]==0 now has no way to have a color added to its list.
                /*
                if (eligible_generators)  // only test if there are eligible generators remaining
                {
                    for (int v=n-1; v>0; v--)  // should we include v==0?  Problem with test below.
                            // starting at min_v doesn't work, since when there had previously been two vertices to mark, only one was, and we might need to check the new one now.
                    {
                        if (L[v]==0 && 
                            ( ( (((1<<n)-1) ^ ((1<<v)-1)) & eligible_generators)==0 )
                                // problem with this test when v==0
                        )
                        {
                            /*
                            if (v>min_v)
                            {
                                printf("WEIRD! L[%d]==0, but no way it can be generated min_v=%d.\n",v,min_v);
                                printf("  el_gens=");
                                print_binary(eligible_generators,n);
                                printf("\n");
                            }
                            //*-/
                            
                            colorability_class=1<<v;
                            eligible_vertices^=1<<v;
                                // this prevents this colorability_class from being added multiple times
                                // FIXME:  Is this okay?
                            return true;
                            
                            // FIXME:  What if more than one vertex becomes unreachable?
                            // The problem is that we need to add different colorability classes, which we can't do here.  But that should happen in the next call. (?)
                        }
                    }
                }
                else
                    return false;  // there no more eligible generators
                //*/
                
                
                // We loop again, trying to find another generator to poke.
            }
        }
        else
            return false;  // there are no subgraphs left to generate
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
    
    eligible_generators=prev.eligible_generators & eligible_vertices;
        // the eligible generators is a subset of the eligible vertices
    
    colorability_class=prev.colorability_class;  // we copy this as a default, so that we can use it when adding a colorability class multiple times
}


#endif  // COLORABILITYCLASS_H__UOMEICEIHALUAFETHAEF
