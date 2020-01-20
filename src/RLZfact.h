#ifndef __RLZfact_H__
#define __RLZfact_H__

#include <vector>
#include <stdio.h> 
#include <stdlib.h>
#include <iostream>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include "SuffixTree.h"

using namespace std;

struct Phrase{
    int id;
    long int pos;
    long int length;

    Phrase(int idd, long int poss, long int lenn){
        id = idd;
        pos = poss;
        length = lenn;
    }

    void print();
};

class RLZfact{
    public:
    // vector<long int> Q;
    // map<long int, long int> B; // key: pos, value: rank 
    // vector<long int> B2; // pos in input array
    // boost::dynamic_bitset<> Bitarray;
    // long int inputLen;
    // int phraseLength = 15;
    long int stringID;
    vector<Phrase> phrases;
    
    RLZfact();
    // RLZfact(vector<long int> Qarr, map<long int, long int> Bmap, vector<long int> B2, boost::dynamic_bitset<> bitarray, long int len); // parse from processed RLZ fact.
    RLZfact(vector<Phrase> phrases, long int id);
    RLZfact(SuffixTree & tree, string s, long int id); // actually do RLZ factorization
    
    long int size(){return phrases.size();}
    Phrase getPhrase(long int rank);
    string reconstruct(string * ref);
};

#endif