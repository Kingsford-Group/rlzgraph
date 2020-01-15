#ifndef __RLZgraph_H__
#define __RLZgraph_H__

#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZfact.h"
#include "SuffixTree.h"
#include <boost/dynamic_bitset.hpp>

struct backPhrase{
    int stringID;
    long int rank;
    // bool isend;

    backPhrase(int id, long int poss){//, bool end){
        stringID = id;
        rank = poss;
        // isend = end;
    }
};

class RLZgraph{
    public: 
    string ref;
    SuffixTree tree;
    long int numNodes=0;
    long int numEdges=0;

    map<long int, vector<backPhrase> > phraseEnds;
    map<long int, vector<backPhrase> > phraseStarts;
    // boost::dynamic_bitset<> ends;
    // map<int, int> H; //(pos, breakid)
    // map<int, vector<pair<int, int> > > HB; //(breakid, [(inputid, inputpos)])
    vector<RLZfact> rlzarr; // a vector of rlz factorizations. one for each intpu string
    // boost::dynamic_bitset<> BR; //bit array

    

    // initial graph with only the reference
    RLZgraph():ref(""), tree(""){};
    RLZgraph(string ref);
    
    void addString(string s);
    // void construction(string ref, vector<RLZfact> factarr);

    // //construction
    // RLZgraph(string ref, vector<RLZfact> factarr);
    // RLZgraph(string ref, vector<string> inputStrings);

    // utils
    

    // traversals
    vector<long int> adjQuery(long int pos); // returns neighbors of the node defined by break at position pos
    vector<long int> adjQuery(long int pos, long int color, long int rank); // traverse along a given axis

    string reconstruct(long int color); // reconstruct a string

    vector<long int> traverse(long int pos); // returns neighbors that locate only downstream of the queried position
    string access(int pos); //returns substring of the node defined by break at position pos
    void DFS(long int curr_pos, vector<bool> visited);
    void print_DFS();

    // updates
    void insertSeq(string seq);
    void mergeGraph(RLZgraph graph);
    void mergeFact(RLZfact fact);
};

#endif