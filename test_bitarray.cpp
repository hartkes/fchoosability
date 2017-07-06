
#include <cstdio>
#include "bitarray.h"

int main()
{
    bitarray x,universe;
    int width=7;
    int count;
    
    printf("sizeof(bitarray)=%lu bits\n\n",sizeof(bitarray)*8);
    
    universe=38+16;
    printf(" univ= ");
    print_binary(universe,width);
    printf("\n");
    
    first_subset(x,universe);
    count=0;
    do
    {
        printf("%5d: ",count);
        print_binary(x,width);
        printf("\n");
        count++;
        
        //if (count>20)
        //    break;
    } while (next_subset(x,universe));
    
    
    printf("Test using second method.\n");
    
    predecessor_of_first_subset(x,universe);
    count=0;
    while (next_subset(x,universe))
    {
        printf("%5d: ",count);
        print_binary(x,width);
        printf("\n");
        count++;
        
        //if (count>20)
        //    break;
    };
    
}
