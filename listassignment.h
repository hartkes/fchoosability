
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
#include "colorabilityclass.h"


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
    
    bool has_feasible_coloring();
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


inline
bool ListAssignment::has_feasible_coloring()
{
    // clear the data structures
    for (int i=cur_color; i>=0; i--)
        color_class[i]=0;  // no vertices have been assigned this color
    
    int v=0;
    bitarray v_mask=1;  // has a 1 in bit position v
    
    assigned_color[v]=0;
    
    /*
    printf("\nChecking for feasible coloring.\n");
    for (int i=0; i<n; i++)
    {
        printf("v=%2d  prev_neighbors=",i);
        print_binary(prev_neighbors[i],n);
        printf("\n");
    }
    printf("\n");
    //*/
    
    while (true)
    {
        // When we start the loop, we are attempting to color v with assigned_color[v], and we need to check if assigned_color[v] is valid (meaning that this color is in v's list, and that no previous neighbor is colored with this color).
        // If c[v] is valid, then we move to the next vertex.
        // If not, we increment the color for v until we find a good color or run out of colors for v.
        
        /*
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
        if ((count&0xFFFFF)==0) //(1) //((count&0xFFFFF)==0)  //((count&0xFFFFFF)==0)
        {
            printf("\ncount=");// %20llu\n",count);
            print_long(count,20);
            printf("     num_feasible_colorings=");
            print_long(num_feasible_colorings,15);
            printf("\n");
        
            //*/
            printf("cur_color=%d\n",cur_color);
            
            int maxvalue=(cur_color>=n ? cur_color : n-1);
            for (int i=0; i<=maxvalue; i++)
            {
                if (i<cur_color)  // cur_color does not include a valid colorability_class (yet)
                {
                    printf("color=%2d  ",i);
                    print_binary(color_info[i].colorability_class,n);
                    
                    /*
                    if ((i==0) && (cur_color==1) && ((color_info[i].colorability_class&0b100001)==0b100001) && (__builtin_popcountll(color_info[i].colorability_class)==4))
                        printf("four");
                    if ((i==0) //&& (cur_color==2) 
                        &&((color_info[0].colorability_class==0b101101) 
                         ||(color_info[0].colorability_class==0b110011)
                         ||(color_info[0].colorability_class==0b001010)
                         ||(color_info[0].colorability_class==0b010100))
                       )
                        printf("this1");
                    if ((i==1) //&& (cur_color==2) 
                        &&((color_info[0].colorability_class==0b101101) 
                         ||(color_info[0].colorability_class==0b110011)
                         ||(color_info[0].colorability_class==0b001010)
                         ||(color_info[0].colorability_class==0b010100))
                        &&((color_info[1].colorability_class==0b101101) 
                         ||(color_info[1].colorability_class==0b110011)
                         ||(color_info[1].colorability_class==0b001010)
                         ||(color_info[1].colorability_class==0b010100))
                       )
                        printf("this2");
                    if ((i==2) //&& (cur_color==2) 
                        &&((color_info[0].colorability_class==0b101101) 
                         ||(color_info[0].colorability_class==0b110011)
                         ||(color_info[0].colorability_class==0b001010)
                         ||(color_info[0].colorability_class==0b010100))
                        &&((color_info[1].colorability_class==0b101101) 
                         ||(color_info[1].colorability_class==0b110011)
                         ||(color_info[1].colorability_class==0b001010)
                         ||(color_info[1].colorability_class==0b010100))
                        &&((color_info[2].colorability_class==0b101101) 
                         ||(color_info[2].colorability_class==0b110011)
                         ||(color_info[2].colorability_class==0b001010)
                         ||(color_info[2].colorability_class==0b010100))
                       )
                        printf("this3");
                    if ((i==3) //&& (cur_color==2) 
                        &&((color_info[0].colorability_class==0b101101) 
                         ||(color_info[0].colorability_class==0b110011)
                         ||(color_info[0].colorability_class==0b001010)
                         ||(color_info[0].colorability_class==0b010100))
                        &&((color_info[1].colorability_class==0b101101) 
                         ||(color_info[1].colorability_class==0b110011)
                         ||(color_info[1].colorability_class==0b001010)
                         ||(color_info[1].colorability_class==0b010100))
                        &&((color_info[2].colorability_class==0b101101) 
                         ||(color_info[2].colorability_class==0b110011)
                         ||(color_info[2].colorability_class==0b001010)
                         ||(color_info[2].colorability_class==0b010100))
                        &&((color_info[3].colorability_class==0b101101) 
                         ||(color_info[3].colorability_class==0b110011)
                         ||(color_info[3].colorability_class==0b001010)
                         ||(color_info[3].colorability_class==0b010100))
                       )
                        printf("this4");
                    //*/
                }
                else
                {
                    printf("          ");
                    for (int k=n+(n/4); k>0; k--)
                        printf(" ");
                }
                if (i<n)
                    printf("   v=%2d  f[v]=%d  L[v]=%d  needed=%d",
                        i,f[i],color_info[cur_color].L[i],f[i]-color_info[cur_color].L[i]);
                printf("\n");
            }
            printf(" el_verts=");
            print_binary(color_info[cur_color].eligible_vertices,n);
            printf("\n");
            printf("  el_gens=");
            print_binary(color_info[cur_color].eligible_generators,n);
            printf("\n");
            //*/
        }
        
        
        // When this loop starts, cur_color points to the next colorability class that we will try to generate a subgraph for and add to our list assignment.
        
        if (color_info[cur_color].generate_subgraph())
        {
            /*
            printf("Successfully generated a new subgraph to use as a colorability class.\n");
            printf("cur_color=%2d  ",cur_color);
            print_binary(color_info[cur_color].colorability_class,n);
            printf("\n");
            //*/
            
            // We need to check if this partial list assignment is suitable, ie, if there is a feasible coloring.
            if (!has_feasible_coloring())
            {
                //printf("We do not have a feasible coloring.\n");
                
                // This partial list assignment needs to be advanced.
                int multiplicity;  // declared outside the loop so it can be used afterward
                for (multiplicity=__builtin_popcountll(color_info[cur_color].colorability_class); 
                        // the popcount counts the number of bits set; this is a gcc builtin.
                     multiplicity>0; multiplicity--)
                    // we limit the multiplicity of a colorability class to its size
                {
                    
                    //*
                    // This code allows for parallelization.
                    if (cur_color==splitlevel)
                        // we need to check whether we should go further (deepen the search tree) or not
                    {
                        // FIXME: TODO:  This break does not allow the search to expand beyond the splitlevel.
                        // This can be simulated by setting the residue equal to the modulus (and hence no branch is ever expanded).
                        //break;
                        
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
                        
                        printf("Odometer tick! cur_color=%d splitlevel=%d odometer=%d residue=%d modulus=%d\n",cur_color,splitlevel,odometer,res,mod);
                    }
                    //*/
                    
                    color_info[cur_color+1].setup_next_from(color_info[cur_color],f);  // initialize the new colorability_class info
                    cur_color++;
                    //printf("next set up, cur_color=%2d\n",cur_color);
                    
                    // At this point, this partial list assignment (up through cur_color) does not have a feasible coloring.
                    // Thus, if this list assignment is full, then this is a bad list assignment and we terminate.
                    // A list assignment is full if and only if there are no eligible vertices.
                    // However, the eligible vertices remaining after colorability class cur_color is added is calculated 
                    // when setting up for the cur_color+1 colorability class.
                    
                    /*
                    printf(">el_verts=");
                    print_binary(color_info[cur_color].eligible_vertices,n);
                    printf("\n");
                    //*/
                    
                    if (color_info[cur_color].eligible_vertices==0)
                    {
                        cur_color--;  // No colorability class has been assigned to cur_color yet.
                        
                        // This is a full list assignment that has no feasible coloring: it's a bad list assignment!
                        //*/
                        printf("BAD full list assignment created, cur_color=%d\n",cur_color);
                        for (int i=0; i<=cur_color; i++)
                        {
                            printf("color=%2d  ",i);
                            print_binary(color_info[i].colorability_class,n);
                            printf("\n");
                        }
                        for (int v=0; v<n; v++)
                            printf("   v=%2d  f[v]=%d  L[v]=%d  needed=%d\n",
                                    v,f[v],color_info[cur_color+1].L[v],f[v]-color_info[cur_color+1].L[v]);
                        printf(" el_verts=");
                        print_binary(color_info[cur_color+1].eligible_vertices,n);
                        printf("\n");
                        printf("  el_gens=");
                        print_binary(color_info[cur_color+1].eligible_generators,n);
                        printf("\n");
                        //*/
                        
                        return false;
                        
                    }
                    
                    // We use the Small Pot Lemma here.
                    // We currently have cur_color colorability classes in our list assignment.
                    // If we have at least n-1 colorability classes, then by the Small Pot Lemma we don't need to add an nth one.
                    // We do this after setting up the next level and calculating eligible_vertices for cur_color+1,
                    // so that we don't miss a bad full list assignment.
                    //*
                    if (cur_color>=n-1)
                    {
                        //printf("Applying the Small Pot Lemma! cur_color=%d count=%20llu\n",cur_color,count);
                        cur_color--;
                        break;
                    }
                    //*/
                    
                    /*
                    printf("incrementing cur_color to %d\n",cur_color);
                    printf(" colorability_class=");
                    print_binary(color_info[cur_color].colorability_class,n);
                    printf("\n");
                    printf("  eligible_vertices=");
                    print_binary(color_info[cur_color].eligible_vertices,n);
                    printf("\n");
                    //*/
                    
                    if ( ( color_info[cur_color].colorability_class & 
                          ~color_info[cur_color].eligible_vertices) == 0 )
                        // This subgraph remains eligible, so we'll try to add it again.
                        // We test if colorability_class is a subset of the eligible vertices.
                        //     We test if bitarray x is a subset of bitarray y using (x & ~y) == 0.
                        //     This is just the negation of the implication x=>y.
                        if (has_feasible_coloring())
                            break;  // proceed to the next subgraph for this colorability class
                        else
                            ;  // we should go back to the beginning of the loop and add this subgraph again
                    else
                        break;  // we'll need to find the next subgraph for this colorability class
                        
                }
                
                //printf("multiplicity=%d\n",multiplicity);
                
                if (multiplicity==0)
                    // This colorability class has multiplicity equal to its size, and so the vertices in the colorability class can always be colored.  Hence we don't need to add any more colors to the lists of those vertices, so we mark them as ineligible.
                {
                    //printf("max multiplicity!, cur_color=%d\n",cur_color);
                    color_info[cur_color].eligible_vertices&=(~color_info[cur_color].colorability_class);
                }
                
            }
            else  // There is a feasible coloring, so we continue on to the next subgraph.
            {
                //printf("We have a feasible coloring!\n");
                
                // We keep track of feasible colorings separately for verifying the parallelization is working correctly.
                num_feasible_colorings++;
                
                ;  // we do nothing and go back to the beginning of the loop to generate the next subgraph
            }
        }
        else
        {
            // There are no more subgraphs to generate, so we backtrack.
            //printf("Could not generate any more subgraphs\n");
            cur_color--;
        }
    }
    
    printf("All list assignments checked, final count=%llu, num_feasible_colorings=%llu\n",count,num_feasible_colorings);
    // No bad list assignment has been found, so the graph is f-choosable.
    return true;
}


#endif  // LISTASSIGNMENT_H__HOLJNEPCVXFOXMPEAIHG
