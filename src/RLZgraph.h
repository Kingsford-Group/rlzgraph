#ifndef __RLZgraph_H__
#define __RLZgraph_H__

#include <iostream>
#include <fstream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <map>
#include "RLZfact.h"
#include "SuffixTree.h"
#include <boost/dynamic_bitset.hpp>


struct classcomp{
    bool operator() (const long int & lhs, const long int & rhs) const{
        return lhs > rhs;
    }
};

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

class RLZNode{
    public:
    long int pos;
    long int length;

    map<long int, vector<long int> > Ends; // color: [rank]
    map<long int, vector<long int>, classcomp> Starts;

    RLZNode * next; // next node in reference coordinate

    RLZNode(){}
    RLZNode(long int pos){
        this->pos = pos;
    }
    RLZNode(long int pos, long int length){
        this->pos = pos;
        this->length = length;
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

    map<long int, RLZNode *,classcomp> nodeDict;
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
    
    vector<RLZNode *> adjQuery(RLZNode * node);
    pair<RLZNode *, long int > adjQuery(RLZNode * node, long int color, long int rank);

    string reconstruct(long int color); // reconstruct a string of color.
    
    // vector<long int> traverse(long int pos); 

    //returns substring of the node defined by break at position pos
    string access(RLZNode* node); 

    // writes in the format: <pos> <length> <color,rank>
    void writeGraph(string outname);

<<<<<<< HEAD
=======
    vector<long int> traverse(long int pos); // returns neighbors that locate only downstream of the queried position
    string access(RLZNode * node); //returns substring of the node defined by break at position pos
    
    void writeGraph(string name);
    
>>>>>>> 36bbb10e062cc01aadb53870800b98686b24dcb3
    void DFS(long int curr_pos, vector<bool> visited);
    void print_DFS();

    

    // updates
    void insertSeq(string seq);
    void mergeGraph(RLZgraph graph);
    void mergeFact(RLZfact fact);
};

#endif