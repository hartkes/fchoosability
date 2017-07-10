
// subgraph.h
// This is a C++ library for generating connected subgraphs.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#pragma once  // for compilers that support this
#ifndef SUBGRAPH_H__HEERAYOOPUWOOXUKOONO  // include guard
#define SUBGRAPH_H__HEERAYOOPUWOOXUKOONO


#include <vector>
#include "bitarray.h"


struct LayerStruct
{
    bitarray layer_universe;  // set of all vertices on this layer
    bitarray layer_subset;    // subset of layer_universe that is currently chosen
    bitarray union_universe;  // union of the universe of this and all previous layers
    bitarray union_subset;    // union of the subset of this and all previous layers; this is the entire vertex set of the generated connected subgraph
};


class ConnectedSubgraph
{
public:
    int n;  // number of vertices in the whole graph
    const std::vector<bitarray> &neighbors;  // adjacencies in the graph
    int root;  // the root vertex
    bitarray eligible_vertices;  // the vertices that are eligible to be in the subgraph
    
    std::vector<LayerStruct> layer;  // the layers
    int cur_layer;  // the current (and last) layer; thus there are cur_layer+1 total layers
    
// methods:
    ConnectedSubgraph(int n, const std::vector<bitarray> &neighbors);
    ConnectedSubgraph(const ConnectedSubgraph &other);
    
    void initialize(int root, bitarray eligible_vertices);
    void copy_from(const ConnectedSubgraph &S);
    int next(const bitarray additional_constraints);
};


ConnectedSubgraph::ConnectedSubgraph(int n, const std::vector<bitarray> &neighbors)
  : // initializer list
    n(n),
    neighbors(neighbors)
{
    //printf("ConnectedSubgraph constructor, n and neighbors set\n");
    layer.resize(n);  // there cannot be more than n layers
}


ConnectedSubgraph::ConnectedSubgraph(const ConnectedSubgraph &other)
    // copy constructor, used in initialization of vectors of these objects
  : // intialization list
    n(other.n),
    neighbors(other.neighbors),
    root(other.root),
    eligible_vertices(other.eligible_vertices),
    layer(other.layer),
    cur_layer(other.cur_layer)
{
    //printf("ConnectedSubgraph copy constructor\n");
}


void ConnectedSubgraph::initialize(int root, bitarray eligible_vertices)
{
    this->root=root;
    this->eligible_vertices=eligible_vertices;
    
    /*/
    printf("ConnectedSubgraph initialize, root=%2d, eligible_vertices=",root);
    print_binary(eligible_vertices,n);
    printf("\n");
    //*/
    
    // We intialize to the predecessor of the first subgraph.  Thus when next() is called, the first subgraph is generated, which is the connected component containing the root.
    cur_layer=0;
    layer[cur_layer].layer_universe=
    layer[cur_layer].union_universe=
    layer[cur_layer].union_subset=  // note that we do this to avoid an "else" in next()
        1<<root;
    predecessor_of_first_subset(layer[cur_layer].layer_subset,layer[cur_layer].layer_universe);

    // TODO: should we have a sanity check that the root is in the eligible_vertices?
    // At the moment, we're just assuming this.
    
    // TODO:  What if the only connected subgraph consists solely of the root?  Should we generate that?
}

inline
void ConnectedSubgraph::copy_from(const ConnectedSubgraph &S)
{
    // We assume that n and neighbors are the same for S and this object.
    // We copy the rest of the values.
    root=S.root;
    eligible_vertices=S.eligible_vertices;
    cur_layer=S.cur_layer;

    // We assume that layers are already correctly sized.
    // layers.resize(n)
    
    for (int i=cur_layer; i>=0; i--)  // only need to copy layers that are used
    {
        layer[i]=S.layer[i];  // this copies the entire struct, using the default copy assignment constructor
    }
}

inline
int ConnectedSubgraph::next(const bitarray additional_constraints)
    // Generate the next connected subgraph, in "decreasing" order.
    // Note that the algorithm must correctly handle vertices in the set that were previously eligible which are now not.
    // The additional_constraints parameter gives a way to remove eligible vertices.
    // It will be &ed with the eligible_vertices to give the new set of eligible vertices.
    // Thus, the new set of eligible vertices is always a subset of the previous set of eligible vertices.
{
    /*
    printf("ConnectedSubgraph::next()  cur_layer=%2d\n",cur_layer);
    printf("elig_vertices=");
    print_binary(eligible_vertices,n);
    printf("\n");
    for (int i=0; i<=cur_layer; i++)
    {
        printf("layer=%2d  l_s=",i);
        print_binary(layer[i].layer_subset,n);
        printf("  l_u=");
        print_binary(layer[i].layer_universe,n);
        printf("  u_s=");
        print_binary(layer[i].union_subset,n);
        printf("  u_u=");
        print_binary(layer[i].union_universe,n);
        printf("\n");
    }
    //*/
    
    // We change our set of eligible vertices to incorporate the new constraints.
    eligible_vertices&=additional_constraints;
    
    // We search for the highest layer whose subset contains an ineligible vertex.
    int i=0;
    while ( (i<cur_layer) &&
            ((layer[i].layer_subset & eligible_vertices) == layer[i].layer_subset) )
            // FIXME: Testing if bitarray x is a subset of y might be better as (x & ~y) == 0.
            // This is just the negation of the implication x=>y.
    {
        // layer[i].layer_universe&=eligible_vertices;
            // We don't recalculate each layer_universe (or union_universe) since these will be recalculated when we return to this layer.
        // TODO: Check this assumption!!!! Do we need to use layer_universe or union_universe at some later point?
        i++;
    }
    
    /*
    printf("i=%d\n",i);
    
    cur_layer=i;
    printf("Trying to advance layer %d\n",cur_layer);
    //*/
    
    
    // We try to advance to the next subset of the current layer.
    // If there are no more subsets, then we backtrack to the previous layer and try again.
    // We continue backtracking until we find a layer with a new subset, or until we backtrack all the way to the root.
    // Note that if eligible vertices has changed such that there are no eligible vertices on this layer, then 
    while (!next_subset(layer[cur_layer].layer_subset,
                        layer[cur_layer].layer_universe & eligible_vertices))
    {
        // there are no more subsets for this layer, so we backtrack to the previous layer
        cur_layer--;
        
        if (cur_layer<0)  // this should only happen if the root is ineligible
            return false;
        
        // Since we are stopping the iteration *before* we get to the singleton root, we do not need the following code.
        // if (cur_layer==0)  // we have backed up to the root, no more subgraphs to generate
        //     return false;
        // UPDATE: That's not entirely true, because the set of eligible vertices could change.  So we have a check for cur_layer==0 at the our bottom return statement.
    }
    
    if ((cur_layer==1 && layer[cur_layer].layer_subset==0))
        // we have backed up so we have the root on layer 0 and the empty set on layer 1.
        // However, we do not want to generate the singleton subgraph consisting solely of the root; this occurs when layer 1 has the empty subset.
        return false;
    
    // Since the subset of the current layer has changed, we need to recalculate the union of all the subsets up through the current layer.
    if (cur_layer>0)
        layer[cur_layer].union_subset=
            layer[cur_layer-1].union_subset | layer[cur_layer].layer_subset;
    // else  // this only occurs with the root level, in which case the union_subset has already been initialized correctly.
    
    // We have advanced to the next subset for cur_layer.
    // We now expand to the next layers, growing layer by layer until we fill the connected component.
    bitarray mask;
    while (true)
    {
        // Compute layer[cur_layer+1].layer_universe for the next layer.
        // This will be all vertices that are not in layer[cur_layer].union_universe that are adjacent to a vertex in layer[cur_layer].layer_subset.
        layer[cur_layer+1].layer_universe=0;
        mask=1<<(n-1);
        for (int i=n-1; i>=0; i--)
        {
            /*
            printf("i=%2d, mask=",i);
            print_binary(mask,n);
            printf("  (layer[cur_layer].layer_subset&mask)=");
            print_binary(layer[cur_layer].layer_subset&mask,n);
            printf("\n");
            //*/
            
            // We add the appropriate neighbors of vertex i to the next layer's universe.
            
            layer[cur_layer+1].layer_universe |= 
                ( (((layer[cur_layer].layer_subset&mask)==0)-1)  
                        // If i is set in layer[cur_layer].layer_subset, then this gives the all 1s vector.  Otherwise, it is 0.
                 &(
                    neighbors[i] &                        // the neighbors of i
                    (~layer[cur_layer].union_universe) &  // that are not already in a layer
                    eligible_vertices                     // and are eligible
                  ) 
                );
            mask>>=1;
        }
        
        if (first_subset(layer[cur_layer+1].layer_subset,
                         layer[cur_layer+1].layer_universe))
        {
            // the next layer is not empty, so we proceed to the next layer
            layer[cur_layer+1].union_universe=
                layer[cur_layer].union_universe | layer[cur_layer+1].layer_universe;
            layer[cur_layer+1].union_subset=
                layer[cur_layer].union_subset | layer[cur_layer+1].layer_subset;
            cur_layer++;
            
            /*
            printf("cur_layer=%2d ",cur_layer);
            print_binary(layer[cur_layer].layer_universe,n);
            printf("\n");
            */
        }
        else
            break;  // no more layers to generate
    }
    
    /*
    for (int i=0; i<=cur_layer; i++)
    {
        printf("layer=%2d  l_s=",i);
        print_binary(layer[i].layer_subset,n);
        printf("  l_u=");
        print_binary(layer[i].layer_universe,n);
        printf("  u_s=");
        print_binary(layer[i].union_subset,n);
        printf("  u_u=");
        print_binary(layer[i].union_universe,n);
        printf("\n");
    }
    //*/
    
    return (cur_layer>0);
        // If cur_layer==0, then we have just the root, which is not a valid subgraph (for us).  So we return false.  This can happen if, for instance, the only eligible vertex is the root.
        // Otherwise, we have generated the next subgraph.
}


#endif  // SUBGRAPH_H__HEERAYOOPUWOOXUKOONO
