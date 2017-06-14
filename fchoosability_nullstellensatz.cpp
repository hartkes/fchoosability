
// fchoosability_nullstellensatiz.cpp
// C++ program to determine if a graph is f-choosable, using the Combinatorial Nullstellensatz.
// Copyright 2017 by Stephen Hartke.
// Licensed under the GPL version 3.

#include <iostream>
#include <string>
#include <string.h>  // C library, for strlen; remove when using C++ std::string
#include <stdlib.h>
#include <stdio.h>
#include <ctime>  // for reporting runtime
#include "fgraph.h"
#include "compositions.h"

extern "C" 
{
    #include "exact.h"  // libexact is a C library, not C++
}


int leading_coefficient(const fGraph& G)
    // We assume that sum(G.f[i]-1)==G.num_edges, 
    // and that G.f[i]>=2 for all i.
{
    // for libexact
    exact_t *e;
    int soln_size;
    const int *soln;
    
    int n,num_edges;
    int coeff,sign;
    //int count;
    int i,j,k;
    
    n=G.n;
    num_edges=G.get_number_of_edges();
    
    if ((n<=1) || (num_edges==0))
    {
        printf("PROBLEM: n=%d num_edges=%d\n",n,num_edges);
        exit(7);
    }
    
    // Initialize the exact cover library.
    e=exact_alloc();
    
    // In the exact cover problem, we have 2 columns for each edge (whether x_i or -x_j is chosen from the factor corresponding to the edge v_i v_j).
    for (j=0; j<2*num_edges; j++)
        exact_declare_col(e,j,1);
    
    // We also have n+e rows:
    // The first n rows are the computation of the degree of the monomial.
    for (i=0; i<n; i++)
        exact_declare_row(e,i,G.f[i]-1);
    
    // The last e edges are ensuring exactly 1 term is chosen from each factor.
    for ( ; i<n+num_edges; i++)
        exact_declare_row(e,i,1);
    
    // Loop over the edges of G to set the matrix.
    k=0;  // index of this edge
    for (j=0; j<n; j++)
        for (i=0; i<j; i++)
            if (G.get_adj_sorted(i,j))  // i,j is an edge
            {
                // mark the contributions to x_i and x_j
                exact_declare_entry(e,i,2*k);
                exact_declare_entry(e,j,2*k+1);
                
                // ensure that exactly one term is chosen from each factor
                exact_declare_entry(e,n+k,2*k);
                exact_declare_entry(e,n+k,2*k+1);
                k++;
            }
    
    //count=0;
    coeff=0;
    while ((soln=exact_solve(e,&soln_size)) !=NULL)
    {
        sign=0;  // (-1)^sign is the contribution we will add to coeff
        for (i=soln_size-1; i>=0; i--)
            sign^=(soln[i]&1);  // the odd-indexed cols have negative sign; we want the parity of those
                // sign is thus 0 or 1 at all times
        coeff+=(1-2*sign);
        //count+=1;
    }
    //printf("count=%d\n",count);
    if (coeff!=0)
        printf("  coeff=%2d  <---------\n",coeff);
    
    exact_free(e);
    
    return coeff;
}


int is_fchoosable(const fGraph& G)
    // We test if an fgraph is f-choosable.
    // A returned value of 1 means that we can prove using the Combinatorial Nullstellensatz that G is f-choosable.
    // Other returned values means that such a proof fails.
{
    int sum_f_minus_1,diff;
    int i;
    int condition_of_H;  // return value of H->remove_vertices_with_f_1()
    fGraph H;
    Compositions C;
    
    
    for (i=G.n-1; i>=0; i--)
        if (G.f[i]<=0)
        {
            printf("f[%d]=%d<=0, which means the graph is not f-choosable.\n",i,G.f[i]);
            return 0;
        }
    
    sum_f_minus_1=0;
    for (i=G.n-1; i>=0; i--)
        sum_f_minus_1+=(G.f[i]-1);
    diff=sum_f_minus_1-G.get_number_of_edges();
    
    //printf("diff=%d\n",diff);
    
    if (diff<0)
    {
        printf("sum(f[i]-1)<num_edges, so the Nullstellensatz cannot be used.\n");
        return 3;
    }
    else if (diff==0)
    {
        H.copy_from(G);
        condition_of_H=H.remove_vertices_with_f_1();
        if (   (condition_of_H==2) || 
             ( (condition_of_H==1) && (leading_coefficient(H)!=0) )
           )
                    // f[i]>=2 for all i, so we can call leading_coefficient
        {
            printf("Success! <===============================================================================\n");
            printf("  f(G)=");
            for (i=0; i<G.n-1; i++)
                printf("%d,",G.f[i]);
            printf("%d\n",G.f[G.n-1]);
            
            return 1;  // G is proved to be f-choosable by the Combin Nullst.
        }
        else
            return 2;  // the Combin Nullst is inconclusive
    }
    // else if (diff>0)
    
    C.first(diff,G.n);  // initialize the compositions; we know that diff>0
    do  // iterate over all compositions
    {
        /*
        printf("C->x: ");
        for (i=0; i<G.n; i++)
           printf("%d ",C->x[i]);
        printf("\n");
        //*/
        
        // reset the adjacencies in H, in case they were changed when removing vertices with f[i]==1
        H.copy_from(G);
        
        // set H's f vector to be G's f vector minus the composition
        for (i=G.n-1; i>=0; i--)
            H.f[i]= G.f[i] - C.x[i];
        
        condition_of_H=H.remove_vertices_with_f_1();
        if (   (condition_of_H==2) || 
             ( (condition_of_H==1) && (leading_coefficient(H)!=0) )
           )
                    // f[i]>=2 for all i, so we can call leading_coefficient
        {
            printf("Success! <===============================================================================\n");
            printf("  f(H)=");
            for (i=0; i<G.n; i++)
                printf("%d,",G.f[i]-C.x[i]);  // H.f[i] might have been modified when removing vertices with f[i]==1
            printf("%d\n",G.f[G.n-1]-C.x[G.n-1]);
            
            return 1;  // G is proved to be f-choosable by the Combin Nullst.
        }
        
    } while (C.next());
    
    return 2;  // the Combin Nullst is inconclusive
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
        if (val==2)
            printf("  The Nullstellensatz was inconclusive.\n");
        
        end=clock();
        printf("    CPU time used: %.3f seconds\n\n",((double)(end-start))/CLOCKS_PER_SEC);
    }
    
    return (val==1);  // return true if the graph can be proven f-choosable
}
