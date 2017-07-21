
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
    
    eligible_generators=prev.eligible_generators & eligible_vertices;
        // the eligible vertices is a subset of the eligible vertices
    
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
    
    std::vector<int> assigned_color;
    std::vector<bitarray> color_class;  // indexed by color; indicates which vertices have been assigned this color
    std::vector<bitarray> prev_neighbors;
    
// methods:
    ListAssignment(
        int n,
        const std::vector<bitarray> &neighbors,
        const std::vector<int> &f);
    
    bool has_feasible_coloring(bitarray vertices_to_skip);
    bool verify(int res,int mod,int splitlevel);
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
        //printf("         L[%2d]=%2d\n",i,color_info[cur_color].L[i]);
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
    
    
    // These are used for backtracking checking of feasible colorings.
    assigned_color.resize(n);  // initialize the size of the assigned colors array
    color_class.resize(color_info.size());
    
    // set up the previous neighbors bitarrays for backtracking checking of feasible colorings
    prev_neighbors.resize(n);
    mask=(1<<n)-1;  // declared as bitarray above; has the n lowest order 1s
    for (int i=n-1; i>=0; i--)
    {
        prev_neighbors[i]=neighbors[i] & mask;
            // only keep neighbors with lower index
        mask>>=1;
    }
    
}


bool ListAssignment::has_feasible_coloring(bitarray vertices_to_skip)
{
    // clear the data structures
    for (int i=cur_color; i>=0; i--)
        color_class[i]=0;  // no vertices have been assigned this color
    
    /*
    printf("Checking coloring, vertices_to_skip=");
    print_binary(vertices_to_skip,n);
    printf("\n");
    //*/
    
    int v=0;
    bitarray v_mask=1;  // has a 1 in bit position v
    while (v_mask & vertices_to_skip)
    {
        v++;
        v_mask<<=1;
        if (v>=n)  // there are no vertices to check; 
                   // TODO: we could replace this with a bitwise check outside the loop
        {
            //printf("There are no vertices to check for coloring!\n");
            return true;
        }
    }
    
    assigned_color[v]=0;
    
    if (vertices_to_skip & (1<<n) )
    {
        //*
        printf("\nChecking for feasible coloring.\n");
        for (int i=0; i<n; i++)
        {
            printf("v=%2d  prev_neighbors=",i);
            print_binary(prev_neighbors[i],n);
            printf("\n");
        }
        printf("\n");
        //*/
    }
    
    while (true)
    {
        // When we start the loop, we are attempting to color v with assigned_color[v], and we need to check if assigned_color[v] is valid (meaning that this color is in v's list, and that no previous neighbor is colored with this color).
        // If c[v] is valid, then we move to the next vertex.
        // If not, we increment the color for v until we find a good color or run out of colors for v.
        
        if (vertices_to_skip & (1<<n) )
        {
            //*
            printf("cur_color=%d ",cur_color);
            printf("v=%2d  ",v);
            print_binary(v_mask,n);
            printf("  ");
            for (int i=0; i<=v; i++)
                printf(" %2d",assigned_color[i]);
            //printf("\n");
            printf("  color_class[ass[v]]=");
            print_binary(color_class[assigned_color[v]],n);
            printf("  color_info[ass[v]].cclass=");
            print_binary(color_info[assigned_color[v]].colorability_class,n);
            printf("\n");
            //*/
        }
        
        /*/ Sanity check that assigned_color[] and color_class[] have the same information.
        bitarray mask=1;
        for (int i=0; i<=v; i++)  // vertex
        {
            if ((mask & vertices_to_skip)==0)
            {
                for (int j=0; j<=cur_color; j++)  // color
                {
                    if (i!=v && assigned_color[i]==j)
                    {
                        if ((color_class[j] & mask)==0)  // the bit should be set
                        {
                            printf("Discrepancy!  color not set when it should be!\n");
                            printf("i=%2d color=%2d  color_class[j]=",i,j);
                            print_binary(color_class[j],n);
                            printf("  mask=");
                            print_binary(mask,n);
                            printf("\n");
                            exit(99);
                        }
                    }
                    else
                        if ((color_class[j] & mask)!=0)  // the bit should NOT be set
                        {
                            printf("Discrepancy!  color set when it should NOT be!\n");
                            printf("i=%2d color=%2d  color_class[j]=",i,j);
                            print_binary(color_class[j],n);
                            printf("  mask=");
                            print_binary(mask,n);
                            printf("\n");
                            exit(99);
                        }
                }
            }
            mask<<=1;
        }
        //*/
        

        if (assigned_color[v]<=cur_color)
        {
            if (
                ((color_info[assigned_color[v]].colorability_class & v_mask)!=0) &&
                        // assigned_color[v] is an admissible color for v
                ((color_class[assigned_color[v]] & prev_neighbors[v])==0)
                        // no prev nbr has been assigned this color
                )
            {
                // assigned_color[v] can be used for v
                
                color_class[assigned_color[v]]|=v_mask;  // record that v has this color
                
                v++;  // advance to the next vertex
                v_mask<<=1;
                
                while (v_mask & vertices_to_skip)  // skip vertices if necessary
                {
                    v++;
                    v_mask<<=1;
                    if (v>=n)
                        break;
                }
                
                if (v>=n)  // we could replace this with an & test
                {
                    //printf("We have found a feasible coloring!\n");
                    return true;
                }
                else
                {
                    // we check the next vertex; initialize the color we check to 0.
                    assigned_color[v]=0;
                }
            }
            else
                // assigned_color[v] is not valid for v
                assigned_color[v]++;
        }
        else  // there are no more colors to try for v
        {
            // hence we must backtrack
            v--;
            v_mask>>=1;
            
            while (v_mask & vertices_to_skip)  // skip vertices if necessary
            {
                v--;
                v_mask>>=1;
                if (!v_mask)
                    break;
            }
            
            if (v_mask)  // hence, v>=0
            {
                color_class[assigned_color[v]]&=~v_mask;  // clear v's bit from this color
                assigned_color[v]++;  // advance to a new (possible) color for v
            }
            else
            {
                // we have backed up past the first vertex, but without finding a feasible coloring
                //printf("There is NO feasible coloring!\n");
                return false;
            }
            
        }
    }
    
    printf("We should never arrive here!\n");
    exit(23);
    
    return false;
}


void print_long(long long int x, int width)
{
    char suffix[]=" tmbtqqssond";
        // the first letter of thousands, million, billion, trillion, etc
    char buffer[width];
    int pos;  // position in the buffer
    int last_digit;
    int digit_count;
    int negative;  // flag to indicate if x is negative
    
    for (pos=0; pos<width; pos++)
        buffer[pos]=' ';  // put spaces in
    
    if (x==0)
        buffer[width-1]='0';
    else
    {
        negative=0;  // flag
        if (x<0)
        {
            negative=1;
            x=-x;
        }
        
        digit_count=0;
        pos=width-1;
        while ((x>0) && (pos>=0))  // digits still remaining and we haven't filled the buffer
        {
            last_digit=x%10;
            x/=10;
            
            digit_count++;
            if ( ((digit_count%3)==1) && (digit_count>1) )
            {
                buffer[pos]=suffix[digit_count/3];
                pos--;
                if (pos<0)
                    break;
            }
            buffer[pos]='0'+last_digit;
            pos--;
        }
        
        if ( (negative) && (pos>=0) )
            buffer[pos]='-';
    }
    
    //for (pos=0; pos<width; pos++)
    //    printf("%c",buffer[pos]);
    
    printf("%.*s",width,buffer);  // how to print a fixed-length char array (a string should be null terminated)
}


bool ListAssignment::verify(int res,int mod,int splitlevel)
{
    // This functions contains the main loop that generates and verifies list assignments.
    
    unsigned long long int count=0, num_feasible_colorings=0;
    int odometer=mod;  // for parallelization; keeps track of the number of nodes of the search tree at level splitlevel
                       // remember that decrementing odometer happens before testing against the residue

    while (cur_color>=0)
    {
        /*/
        printf("Starting main loop, cur_color=%d\n",cur_color);
        for (int i=0; i<=cur_color; i++)
        {
            printf("color=%2d  ",i);
            print_binary(color_info[i].colorability_class,n);
            printf("\n");
        }
        for (int v=0; v<n; v++)
        {
            printf("   v=%2d  f[v]=%d  L[v]=%d  needed=%d\n",
                       v,f[v],color_info[cur_color].L[v],f[v]-color_info[cur_color].L[v]);
        }
        printf("  eligible_vertices=");
        print_binary(color_info[cur_color].eligible_vertices,n);
        printf("\n");
        printf("eligible_generators=");
        print_binary(color_info[cur_color].eligible_generators,n);
        printf("\n");
        //*/
        
        /*
        for (int v=0; v<n; v++)
            if (color_info[cur_color].L[v]>f[v])
            {
                printf("A list is too full!\n");
                exit(99);
            }
        //*/
        
        count++;
        if ((count&0xFFFFF)==0)  //((count&0xFFFFFF)==0)
        {
            printf("count=");// %20llu\n",count);
            print_long(count,20);
            printf("     num_feasible_colorings=");
            print_long(num_feasible_colorings,15);
            printf("\n");
        
            //*/
            printf("cur_color=%d\n",cur_color);
            for (int i=0; i<=cur_color; i++)
            {
                printf("color=%2d  ",i);
                print_binary(color_info[i].colorability_class,n);
                printf("\n");
            }
            //for (int v=0; v<n; v++)
            //    printf("   v=%2d  L[v]=%d  f[v]=%d\n",v,color_info[cur_color].L[v],f[v]);
            printf("  eligible_vertices=");
            print_binary(color_info[cur_color].eligible_vertices,n);
            printf("\n");
            printf("eligible_generators=");
            print_binary(color_info[cur_color].eligible_generators,n);
            printf("\n");
            //*/
        }
        
        
        // When this loop starts, cur_color points to the next colorability class that we will try to generate a subgraph for and add to our list assignment.
        
        if (color_info[cur_color].generate_subgraph())
        {
            //printf("Successfully generated a new subgraph to use as a colorability class.\n");
            
            // We need to check if this partial list assignment is suitable, ie, if there is a feasible coloring.
            if (!has_feasible_coloring(0))
            {
                // This partial list assignment needs to be advanced.
                
                while (true)
                {
                    //*
                    // This code allows for parallelization.
                    if (cur_color==splitlevel)
                        // we need to check whether we should go further (deepen the search tree) or not
                    {
                        odometer--;
                        if (odometer<0)
                            odometer=mod-1;  // reset the odometer
                        
                        //printf("v=%d splitlevel=%d odometer=%d residue=%d modulus=%d\n",v,splitlevel,odometer,res,mod);
                        
                        if (odometer!=res)  // we will not check this branch
                        {
                            // we continue on the level of cur_color with the next colorability class
                            //printf("cur_color=%d splitlevel=%d odometer=%d residue=%d modulus=%d\n",cur_color,splitlevel,odometer,res,mod);
                            
                            // and now we just want to loop again
                            break;
                        }
                    }
                    //*/
                    
                    color_info[cur_color+1].setup_next_from(color_info[cur_color],f);  // initialize the new colorability_class info
                    cur_color++;
                    
                    /*
                    printf("incrementing cur_color to %d\n",cur_color);
                    printf(" colorability_class=");
                    print_binary(color_info[cur_color].colorability_class,n);
                    printf("\n");
                    printf("  eligible_vertices=");
                    print_binary(color_info[cur_color].eligible_vertices,n);
                    printf("\n");
                    //*/
                    
                    if ( (color_info[cur_color].colorability_class & 
                          color_info[cur_color].eligible_vertices)
                        ==color_info[cur_color].colorability_class )
                        // this subgraph remains eligible, so we'll try to add it again
                        if (has_feasible_coloring(0))
                            break;  // proceed to the next subgraph for this colorability class
                        else
                            ;  // we should go back to the beginning of the loop and add this subgraph again
                    else
                        break;  // we'll need to find the next subgraph for this colorability class
                        
                }
            }
            else  // There is a feasible coloring, so we continue on to the next subgraph.
            {
                // We keep track of feasible colorings separately for verifying the parallelization is working correctly.
                num_feasible_colorings++;
                
                ;  // we do nothing and go back to the beginning of the loop to generate the next subgraph
            }
        }
        else
        {
            // There are no more subgraphs to generate, but have not found a suitable coloring.
            // This is a violation only if every vertex has a full list.
            //printf("Could not generate any more subgraphs\n");
            
            // Note that color_info[cur_color].colorability_class is NOT valid, so we back up.
            cur_color--;
            
            if (!has_feasible_coloring(color_info[cur_color+1].eligible_vertices))  // but we need to include those vertices
                // vertices that are eligible do not have full lists, and so can have a singleton added to them.  Thus, they can always be colored.  So we skip them when checking for a feasible coloring.
            {
                printf("No more subgraphs to generate!, no feasible coloring, count=%llu\n",count);
                
                // print that out
                has_feasible_coloring(color_info[cur_color].eligible_vertices | (1<<n) );
                
                //*/
                printf("Full list assignment created, cur_color=%d\n",cur_color);
                for (int i=0; i<=cur_color; i++)
                {
                    printf("color=%2d  ",i);
                    print_binary(color_info[i].colorability_class,n);
                    printf("\n");
                }
                for (int v=0; v<n; v++)
                    printf("   v=%2d  f[v]=%d  L[v]=%d  needed=%d\n",
                            v,f[v],color_info[cur_color+1].L[v],f[v]-color_info[cur_color+1].L[v]);
                printf("  eligible_vertices=");
                print_binary(color_info[cur_color+1].eligible_vertices,n);
                printf("\n");
                printf("eligible_generators=");
                print_binary(color_info[cur_color+1].eligible_generators,n);
                printf("\n");
                //*/
                
                return false;
            }
            else  // We have a feasible coloring, so proceed with the search.
                ;
        }
    }
    
    printf("All list assignments checked, final count=%llu, num_feasible_colorings=%llu\n",count,num_feasible_colorings);
    // No bad list assignment has been found, so the graph is f-choosable.
    return true;
}


#endif  // LISTASSIGNMENT_H__HOLJNEPCVXFOXMPEAIHG
