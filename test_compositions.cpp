
#include <stdlib.h>
#include <stdio.h>
#include "compositions.h"

int main()
{
    Compositions *C;
    int n=5,k=3;
    int count;
    int i;
    
    C=new Compositions(5,3);
    C->first();
    count=0;
    while (1)
    {
        // do something with the composition
        
        // we print the composition
        count++;
        printf("%4d: ",count);
        for (i=0; i<k; i++)
            printf("%d ",C->x[i]);
        printf("\n");
        
        if (!C->next())
            break;
    }
}
