#ifndef __bitvector_H__
#define __bitvector_H__

#include <map>
#include <vector>

using namespace std;

static int SIZE = 64; // size of long int

class BitVector{
    public: 
    vector<unsigned long int>array;
    long length;

    BitVector(long int length);
    // BitVector(string number);

    void addBit(long pos);
    void clearBit(long pos);
    void printNum();
};

class RankSelect{
    public:
    BitVector array;
    vector<map<BitVector, vector<long>> > rankTable;   // key: number of ones, value: prefix sum array
    vector<BitVector> prefixSum;
    vector<BitVector> prefixSum2;

    RankSelect(BitVector input);

    long rank(long i);    // number of 1s at or before position i;
    long select(long i);   // the i-th bit
};


#endif