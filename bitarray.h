
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
    
    bitarray y;
    
    // Note that if x==0 (the empty set) initially, then false is returned, but x is also reset to the universe (since -1 is the all 1s vector).  We do this to avoid having a branch in this code.
    
    bool return_value=((x!=0) & (universe!=0));
        // We need to be able to correctly handle when the universe changes.
        // As long as x!=0 and universe!=0, then there will be a valid next subset.
        // However, if x==0 or universe==0, then there are no more subsets.
        // (If universe==0, then we immediately return false; we do not consider the empty set of an empty universe to be valid.  This is a choice to make the calling code easier.).
    
    // Note that the following code does not work if the universe has shrunk.
    // As an example, old universe=111=7 (LSB first), set=101=5, new universe=110=3.
    //x=(x-1) & universe;
    
    // To create our successor, we first substract 1.
    x--;
    
    // We now need to clear the bits that are outside of the universe.
    // However, we cannot simply clear them, we must advance our set to the next subset in the universe.
    // So we find the highest bit of x that is not in the universe; all lower-order bits in the universe
    // should be put into the subset (since they come after in the subset order).
    
    y=x&(~universe);
    
    // The following operations fill in 1s in all lower-order bit positions after the highest-order bit.
    // The code is inspired by:
    // http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
    y|=(y>>1);
    y|=(y>>2);
    y|=(y>>4);
    y|=(y>>8);
    y|=(y>>16);
    y|=(y>>32);
    
    // We now set those bits in x and then & with the universe.
    // Note that the highest-order bit in y will be erased from x by &ed with universe.
    x=(x|y) & universe;
    
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
