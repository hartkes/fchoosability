
// fchoosability_nullstellensatiz.cpp
// C++ program to determine if a graph is f-choosable, using the Combinatorial Nullstellensatz.
// Copyright 2017 by Stephen Hartke.
// Licensed under the GPL version 3.

#include <stdlib.h>
#include <stdio.h>
#include <string.h>  // C library, for strlen; remove when using C++ std::string
#include <time.h>  // for reporting runtime
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
    int count;
    int i,j,k;
    
    //printf("Starting to compute the leading coefficient of the graph polynomial. n=%d\n",G.n);
    
    n=G.n;
    num_edges=G.get_number_of_edges();
    
    if ((n<=1) || (num_edges==0))
    {
        printf("PROBLEM: n=%d num_edges=%d\n",n,num_edges);
        exit(7);
    }
    
    // Initialize the exact cover library.
    //printf("initializing libexact  n=%d, num_edges=%d\n",n,num_edges);
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
    
    //printf("libexact problem declared\n");
    
    count=0;
    coeff=0;
    while ((soln=exact_solve(e,&soln_size)) !=NULL)
    {
        sign=0;  // (-1)^sign is the contribution we will add to coeff
        for (i=soln_size-1; i>=0; i--)
        {
            sign^=(soln[i]&1);  // the odd-indexed cols have negative sign; we want the parity of those
                // sign is thus 0 or 1 at all times
            //printf("%d (%d) ",soln[i],sign);
        }
        //printf("\n");
        //printf("+");
        coeff+=(1-2*sign);
        count+=1;
    }
    //printf("count=%d\n",count);
    if (coeff!=0)
        //printf("  coeff=%2d  <=======================================================================\n",coeff);
        printf("  coeff=%2d  <---------\n",coeff);
    else
    {
        //printf("coeff=%d\n",coeff);
        ;
    }
    
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
            for (i=0; i<G.n; i++)
                printf("%d ",G.f[i]);
            printf("\n");
            
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
        //printf("resetting H, G.n=%d, H->n=%d H->dss=%d, G.nchoose2=%d\n",G.n,H->n,H->data_structure_size,G.nchoose2);
        H.copy_from(G);
        //printf("Done resetting H\n");
        
        // set H's f vector to be G's f vector minus the composition
        for (i=G.n-1; i>=0; i--)
            H.f[i]= G.f[i] - C.x[i];
        
        /*
        printf("  f(H)=");
        for (i=0; i<H.n; i++)
            printf("%d ",H.f[i]);
        printf("\n");
        H.print_adj_matrix();
        //*/
        
        if ((condition_of_H=H.remove_vertices_with_f_1()))
            // f[i]>=2 for all i, so we can call leading_coefficient
        {
            //printf("vertices of f==1 successfully removed! condition_of_H=%d\n",condition_of_H);
            if ((condition_of_H==2) || (leading_coefficient(H)!=0))
            {
                printf("Success! <===============================================================================\n");
                printf("  f(H)=");
                for (i=0; i<G.n; i++)
                    printf("%d ",G.f[i]-C.x[i]);  // H.f[i] might have been modified when removing vertices with f[i]==1
                printf("\n");
                
                return 1;  // G is proved to be f-choosable by the Combin Nullst.
            }
        }
        
    } while (C.next());
    
    return 2;  // the Combin Nullst is inconclusive
}



int main(int argc, char *argv[])
{
    const int max_line_length=1000;
    char line_in[max_line_length];
    int val;
    
    fGraph G;
    
    clock_t start,end;  // for reporting CPU runtime
    
    // We read lines in from stdin.
    // Each line should be in fgraph6 format.
    // If a line starts with '>', then it is treated as a comment.
    
    //printf("Starting.\n");
    
    while (fgets(line_in,max_line_length,stdin)!=0)
    {
        //printf("start of loop\n");
        
        if (line_in[0]=='>')  // treat this line as a comment
            continue;
        
        //printf("len line_in=%d\n",strlen(line_in));
        if (strlen(line_in)<=3)  // this line is too short, probably end of file
            continue;
        
        //printf("about to start clock\n");
        start=clock();  // record starting time
        
        //printf("reading in graph6 string\n");
        G.read_fgraph6_string(line_in);
        
        printf("Input read: n=%d %s",G.n,line_in);
        
        /*
        printf("f= ");
        for (int i=0; i<G.n; i++)
            printf("%1d ",G.f[i]);
        printf("\n");
        G.print_adj_matrix();
        printf("\n");
        //*/
        
        val=is_fchoosable(G);
        if (val==2)
            printf("  The Nullstellensatz was inconclusive.\n");
        
        end=clock();
        printf("    CPU time used: %.3f seconds\n",((double)(end-start))/CLOCKS_PER_SEC);
    }
    
    //printf("G about to go out of scope.\n");
    
}
