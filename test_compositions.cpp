
#include <stdlib.h>
#include <stdio.h>
#include "compositions.h"

int main()
{
    Compositions C;
    int n=5,k=3;
    unsigned long long int count;
    int i;
    int exceeding;
    
    n=25;
    k=17;
    // final count = 103077446706
    // final count =   4312148069
    C.first(n,k);
    count=0;
    while (1)
    {
        // do something with the composition
        
        exceeding=0;
        for (i=k-1; i>=0; i--)
            if (C.x[i]>5)
                break;
        if (i<0)  // no element exceeds 5
            count++;
        
        // we print the composition
        if ((count&0xffffff)==0)  // ((count&0xfffffff)==0)
        {
            printf("%20llu: ",count);
            for (i=0; i<k; i++)
                printf("%2d ",C.x[i]);
            printf("\n");
        }
        
        if (!C.next())
            break;
    }
    printf("final count=%20llu\n",count);
}
