#ifndef __RLZgraph_H__
#define __RLZgraph_H__

#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZfact.h"

class RLZgraph{
    public: 
    map<int, int> H; //(pos, breakid)
    map<int, vector<pair<int, int> > > HB; //(breakid, [(inputid, inputpos)])
    vector<RLZfact> rlzarr; // a vector of rlz factorizations. one for each intpu string
    vector<int> BR; //bit array

    void construction(vector<RLZfact> factarr);

    //construction
    RLZgraph(vector<RLZfact> factarr);
    RLZgraph(string ref, vector<string> inputStrings);

    // utils
    

    // traversals
    vector<int> adjQuery(int pos);

    // updates
    void insertSeq(string seq);
    void mergeGraph(RLZgraph graph);
    void mergeFact(RLZfact fact);
};

#endif