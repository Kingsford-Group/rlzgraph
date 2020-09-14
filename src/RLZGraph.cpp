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

#include "RLZGraph.hpp"

RLZGraph::RLZGraph(RLZ & rlz){

    this->adjMatrix.resize(rlz.totalLength+2);

    unordered_set<Phrase *> phrases;
    vector<bool> boundaries;

    // create source and sink nodes of the graph
    Phrase * s = new Phrase(-1,0);
    Phrase * t = new Phrase(-2,0);
    int sIdx = rlz.totalLength;
    int tIdx = rlz.totalLength+1;

    // set up s, t links to the beginning and ending of reference
    adjMatrix[sIdx] = adjMatrix[sIdx] | (1<<)

    // iterate through all phrases in the compressed strings
    for (auto compString : rlz.compressed_strings){
        Phrase * prev = s;
        for (Phrase * curr : compString){
            
            /* First node */
            if (prev == s){
                adjMatrix[sIdx] 
            }

            auto it = phrases.find(p);
            if (it == phrases.end()){
                int left = p->start;
                int right = p->start+ p->length;
                int prevRight = prev->start+prev->length

                /* record phrase */
                phrases.insert(p);
                
                /* introduce segmentation */
                boundaries[left] = true;
                boundaries[right] = true;

                /* add edges */
                // reference
                adjMatrix[left-1] = adjMatrix[left-1] | (1<<(left));
                adjMatrix[right-1] = adjMatrix[right-1] | (1<<(right));
                // phrase
                adjMatrix[prevRight] = adjMatrix[prevRight] | (1<<(left)); 
            }
            prev = curr;
        }
        adjMatrix
    }

}