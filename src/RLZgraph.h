#ifndef __RLZgraph_H__
#define __RLZgraph_H__

#include <iostream>
#include <fstream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <fstream>
#include <map>
#include <set>
#include "RLZfact.h"
#include "SuffixTree.h"
#include <boost/dynamic_bitset.hpp>
#include <unordered_set>


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

    map<long int, unordered_set<long int> > Ends; // color: [rank]
    map<long int, unordered_set<long int> > Starts;
//     unordered_map<long int, vector<long int> > Ends; // color: [rank]
//     unordered_map<long int, vector<long int> > Starts;
// >>>>>>> 468633a044c17bb21dc42fde65452fdd129672ab

    RLZNode * next; // next node in reference coordinate
    RLZNode * prev; // previous node in reference coordinate

    RLZNode(){}
    RLZNode(long int pos){
        this->pos = pos;
    }
    RLZNode(long int pos, long int length){
        this->pos = pos;
        this->length = length;
    }
};

class Bubble{
    public:
    RLZNode * begin;
    RLZNode * end;
    vector<RLZNode *> firstPath;
    vector<RLZNode *> secondPath;

    Bubble(){}
    Bubble(RLZNode* b, RLZNode* e, vector<RLZNode *> refPath, vector<RLZNode *> otherPath);
};

class RLZgraph{
    public: 
    string * ref;
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
    RLZgraph():tree(""){};
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

    // writes in the format: <pos>\t<length>\t<color,rank>
    void writeGraph(string outname);
    // writes in the format: <stringID>\t<phrases>
    void writePhrases(string outname);

    // void DFS(long int curr_pos, vector<bool> visited);
    // void print_DFS();

    vector<Bubble> findAllBubbles();
    vector<Bubble> findBubbles(int color1, int color2); // finds bubbles between only color1 and color2
    vector<Bubble> findBubbles(int color); // finds bubbles between reference and color.


    vector<RLZNode*> superpath(RLZNode* next, long int pos, long int color); // the second parameter could be pos or rank. Use rank if it is in the compressed string. color = -1 if reference 

    void printGraph();

    // updates
    void insertSeq(string seq);
    void mergeGraph(RLZgraph graph);
    void mergeFact(RLZfact fact);
};

#endif