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
    bool reversed;

    void print(ostream & out){
        out << refIdx << "," << nodeIdx << "," << seq << endl;
    }
};

/**
 * @brief Records the reference index of the first and second node of the edge.
 * 
 */
struct FirstPassEdge{
    int first;
    int second;
    bool from_start = false;
    bool to_end = false;

    FirstPassEdge(int first, int second, bool from_start, bool to_end){
        this->first = first;
        this->second = second;
        this->from_start = from_start;
        this->to_end = to_end;
    }

    bool operator==(const FirstPassEdge & edge) const{
        return (edge.first == this->first && edge.second == this->second && edge.from_start == this->from_start && edge.to_end == this->to_end);
    }

    void print(ostream & out){
        out << first << "," << second << "," << to_string(from_start) << "," << to_string(to_end) << endl;
    }
};

struct RLZEdge{
    RLZNode * first;
    RLZNode * second;

    // setting both of the variables to false indicates normal edge
    bool from_start = false;    // out edge from start
    bool to_end = false;        // in edge to end

    RLZEdge(RLZNode * first, RLZNode * second, bool from_start, bool to_end){
        this->first = first;
        this->second = second;
        this->from_start = from_start;
        this->to_end = to_end;
    }

    void print(ostream & out){
        out << first->nodeIdx << ";" << second->nodeIdx << "," << to_string(from_start) << ";" << to_string(to_end) << endl;
    }

    bool operator==(const RLZEdge & edge) const{
        return (edge.first == this->first && edge.second == this->second && edge.from_start == this->from_start && edge.to_end == this->to_end);
    }
};

struct RLZEdgePtrHash{
    size_t operator()(const RLZEdge * edge) const {
        hash<string> hasher;
        return hasher(to_string(edge->first->refIdx)+"/"+to_string(edge->second->refIdx)+"/"+to_string(edge->to_end)+"/"+to_string(edge->from_start));
    }
};

struct FirstPassEdgeHash{
    size_t operator()(const FirstPassEdge & edge) const {
        hash<string> hasher;
        return hasher(to_string(edge.first)+"/"+to_string(edge.second)+"/"+to_string(edge.to_end)+"/"+to_string(edge.from_start));
    }
};

struct RLZPath{
    vector<RLZNode*> path;
    vector<bool> reversed;  // true for the i-th node if incoming edge of node i attaches to its end. (node i is reversed)

    void addNode(RLZNode * node, bool rev=false){
        path.push_back(node);
        reversed.push_back(rev);
    }

    void print(ostream & out){
        for (int idx=0; idx<path.size(); idx++){
            if (reversed[idx] == true){
                out << "-";
            }
            out << path[idx]->nodeIdx;
            out << ",";
        }
        out << endl;
    }

    int size(){
        return path.size();
    }

    string reconstruct(){
        string toReturn;
        for(int idx=0; idx < path.size(); idx++){
            // cout << path[idx]->nodeIdx << ";" << reversed[idx] << endl;
            if (reversed[idx] == true){
                toReturn += reverseComp(path[idx]->seq);
            } else{
                toReturn += path[idx]->seq;
            }
        }
        return toReturn;
    }

    char revCompHelper(char c){
        switch (c){
            case 'A': return 'T';
            case 'C':return 'G';
            case 'G': return 'C';
            case 'T': return 'A';
            case 'N': return 'W';
            case 'M': return 'A';
            case 'Y': return 'B';
            case 'S': return 'D';
            case 'R': return 'H';
            case 'K': return 'V';
            case 'B': return 'M';
            case 'D': return 'Y';
            case 'H': return 'S';
            case 'V': return 'R';
            case 'U': return 'K';
            case 'W': return 'N';
            case '$': return 16;
        }
        cerr << "Unrecognized Character (revComp): " << c << endl;
        exit(1);
    }


    /**
     * @brief Produce the reverse complement of input string
     * 
     * @param toreverse input string
     * @return string reverse complement
     */
    string reverseComp(string toreverse){
        string s ="";
        for (int i=0;i<toreverse.length();i++){
            s+=revCompHelper(toreverse[i]);
        }
        reverse(s.begin(), s.end());
        return s;
    }
};

class RLZGraph{
    public:
    RLZ * rlz;    // stores factorization

    // vector< sd_vector<> > adjMatrix; // stores graph structure
    // int nodeNum;
    // int edgeNum;

    unordered_map<int, RLZNode*> idx_to_node; // maps position in reference to node
    unordered_set<RLZEdge*, RLZEdgePtrHash> Edges;
    vector<RLZNode*> Nodes;

    sd_vector<> NodeBoundaries;         // stores node boundaries
    vector<RLZPath*> paths;

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

    /***
     * @brief convert compressed strings into paths
     * @return Returns a list of RLZPath instances. 
    */
    vector<RLZPath*> create_paths();

    private:
    void add_edge_const(vector<bit_vector> & tmpAdj, int idx1, int idx2);
};




#endif
