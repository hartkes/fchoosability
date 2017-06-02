
// compositions.h
// This is a C++ library for generating integer compositions.
// The library is contained in one header file to enable use of inline functions.

// Copyright 2017, Stephen G. Hartke
// Licensed under the GPL version 3.


#include <vector>

class Compositions
{
public:
    int n,k;  // compositions of n into k parts
    std::vector<int> x;  // the vector
    int first_nonzero;  // the position of the first nonzero entry of x
    
    Compositions(int n,int k);
    ~Compositions();
    
    // These methods generate compositions in colex order.
    // The algorithms are taken from Jorg Arndt's fxtbook.
    void first();
    int next();
};

inline
Compositions::Compositions(int n,int k)
{
    this->n=n;
    this->k=k;
    x.resize(k,0);  // resize the vector to length k, filling with 0s
}

inline
Compositions::~Compositions()
{
    // Since x is of type std::vector, deallocation will happen automatically.
}

inline
void Compositions::first()
{
    int i;
    
    x[0]=n;  // all of the nonzero values are in the first position
    first_nonzero=0;
    for (i=k-1; i>0; i--)  // setting the remaining elements to 0 might not be necessary with std::vector::resize()
        x[i]=0;
}

inline
int Compositions::next()
    // returns 0 if no more compositions to generate; otherwise returns 1
{
    int v;
    
    //for (i=0; x[i]==0; i++);  // find first nonzero position; not needed with first_nonzero variable
    
    if (first_nonzero>=k-1)  // this is the last composition
        return 0;
    
    v=x[first_nonzero];  // value of the first nonzero
    x[first_nonzero]=0;
    first_nonzero++;
    x[first_nonzero]++;
    v--;  // amount that should be transferred to x[0]
    if (v!=0)
    {
        x[0]=v;
        first_nonzero=0;
    }
    return 1;
}
