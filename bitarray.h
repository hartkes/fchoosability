
// bitarray.h
// This is a C++ library for using unsigned ints as bit arrays.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#pragma once  // for compilers that support this
#ifndef BITARRAY_H__AEGHAOPHAIHAIJINGOOB  // include guard
#define BITARRAY_H__AEGHAOPHAIHAIJINGOOB

#include <cstdint>
#include <cstdio>

typedef uint_fast64_t bitarray;

// uint_fast64_t is an unsigned integer type of width at least 64 bits 
// that is as fast as any other integer type with at least this width.
// Note that std::bitset provides an array of bits in the C++ Standard 
// Library, so we avoid that name.
// We use a typedef so that we can easily change to larger widths if 
// necessary.  gcc and Clang provide 128-bit integers that are emulated
// from two 64-bit integers.



/////////////////////////////////////////////////////////////////////////////
// Functions for iterating through subsets of a given universe set.
// Note that we iterate through the subsets in *reverse* lexicographic order.
// Also, we *do* include the empty set.
/////////////////////////////////////////////////////////////////////////////


inline
bool first_subset(bitarray &x, const bitarray &universe)
    // Sets up the first subset, which is the universe itself.
    // Returns true, unless universe itself is empty.
{
    x=universe;
    return (x!=0);
}


inline
void predecessor_of_first_subset(bitarray &x, const bitarray &universe)
    // Constructs the predecessor of the first subset, so that when next is called, the first subset is obtained.
{
    x=universe+1;
}


inline
bool next_subset(bitarray &x, const bitarray &universe)
{
    // Returns the next subset of the universe, counting down in lex order.
    // Returns false if there are no more subsets remaining, otherwise true.
    // Note that the empty set *is* included.
    
    // An explanation of why this works can be found at:
    // http://lhearen.top/2016/07/06/Bit-manipulation/ section 3 on Sets
    // It's also in the fxtbook (Section 1.25 Generating bit subsets of a given word), as well as Knuth's The Art of Computing Volume 4A: Combinatorial Algorithms, Section 7.1.3 subsection Working with fragmented fields.
    
    // Note that if x==0 (the empty set) initially, then false is returned, but x is also reset to the universe (since -1 is the all 1s vector).  We do this to avoid having a branch in this code.
    
    bool return_value=(x!=0);
    x=(x-1) & universe;
    return return_value;
}





/////////////////////////////////////////////////////////////////////////////
// Functions for outputting a bitarray.
/////////////////////////////////////////////////////////////////////////////


void print_binary(bitarray x, int num_bits)
{
    for (int i=0; i<num_bits; i++)
    {
        printf("%1d",(int)(x&1));  // print the low bit first
        
        if ((i&3)==3)  // print a separator every 4 bits
            printf("_");
        
        x>>=1;
    }
}
// TODO: We could also have this return a std::string as well.


#endif  // BITARRAY_H__AEGHAOPHAIHAIJINGOOB
