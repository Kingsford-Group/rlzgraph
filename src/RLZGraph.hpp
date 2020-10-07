/**
 * @file RLZGraph.hpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Defines RLZ Graph object
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __RLZGRAPH_H__
#define __RLZGRAPH_H__

#include "RLZ.hpp"
#include <sdsl/bit_vectors.hpp>
#include <unordered_set>
#include <fstream>

#include <ctime>
#include <ratio>
#include <chrono>

using namespace std::chrono;
using namespace std;
using namespace sdsl;

struct Phrase;

struct RLZNode{
    string seq;
    int refIdx;
    int nodeIdx;

    void print(ostream & out){
        out << refIdx << "," << nodeIdx << "," << seq << endl;
    }
};

struct RLZEdge{
    RLZNode * first;
    RLZNode * second;

    void print(ostream & out){
        out << first->nodeIdx << ";" << second->nodeIdx << endl;
    }
};

struct pairEdgeHash{
    size_t operator()(const pair<int, int> & pair) const {
        hash<string> hasher;
        return hasher(to_string(pair.first)+"/"+to_string(pair.second));
    }
};

class RLZGraph{
    public:
    RLZ * rlz;    // stores factorization

    // vector< sd_vector<> > adjMatrix; // stores graph structure
    // int nodeNum;
    // int edgeNum;

    unordered_map<int, RLZNode*> idx_to_node;
    vector<RLZEdge*> Edges;
    vector<RLZNode*> Nodes;

    // vector<int> NodeToPos;          // maps node id to position in reference
    // vector<int> nodeMap;            // maps position to node


    // vector<Phrase*> phrases;         // unique phrases ordered by their index
    // vector<Phrase*> paths;       // stores paths of each compressed string by phrase indices

    RLZGraph(){
    }

    /**
     * @brief Construct a new RLZGraph object from RLZ factorization
     * 
     * @param rlz rlz factorization
     */
    // initialize phrase unordered set
    // for phrase in compressedStrings
    //     segment reference
    //     if introduced new nodes, nodeNum ++; edgeNum ++
    //     insert phrase into phrase set
    //     if new edges are added: edgeNum ++
    // initialize phrase vector to unordered set size
    // Iterate through phrase unordered set to push_back phrases to vector
    // iterate through all compressedStrings to assign phrase to phrase id.
    //     insert those into paths
    RLZGraph(RLZ & rlz);

    /**
     * @brief Read RLZGraph object from file
     * 
     * @param fname input filename
     */
    RLZGraph(ifstream & in);

    int get_edgeNum(){
        return Edges.size();
    }

    int get_nodeNum(){
        return Nodes.size();
    }

    void set_edges(ifstream & in);

    /**
     * @brief Print all the edges
     */
    void print_edges(ostream & out);

    /**
     * @brief Print all the nodes
     */
    void print_nodes(ostream & out);

    /**
     * @brief Verify that all phrase adjacencies are represented
     */
    void verify();

    /**
     * @brief write all of the graph components onto disk
     */
    void write_complete_graph(ofstream & out);

    private:
    void add_edge_const(vector<bit_vector> & tmpAdj, int idx1, int idx2);
};




#endif
