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

#ifndef __RLZ_H__
#define __RLZ_H__

#include "RLZ.hpp"
#include <sdsl/bit_vectors.hpp>
#include <unordered_set>


using namespace std;
using namespace sdsl;

class RLZGraph{

    csa_wt<> csa;                   // stores reference 

    vector< sd_vector<> > adjMatrix; // stores graph structure
    int nodeNum;
    int edgeNum;

    vector<Phrase*> phrases;         // unique phrases ordered by their index
    vector<int_vector> paths;       // stores paths of each compressed string by phrase indices

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
    RLZGraph(string & fname);

    /**
     * @brief write all of the graph components onto disk
     */
    void write_complete_graph();
};


#endif
