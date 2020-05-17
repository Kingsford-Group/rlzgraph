/**
 * @file optimize_algo.cpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Implements graph optimization algorithm
 * @version 0.1
 * @date 2020-05-15
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <unordered_set>
#include <unordered_map>
#include "RLZ.hpp"

using namespace std;

/**
 * @brief Optimize the phrase boundaries that result in the most number of overlaps. Updates the phrase set
 * 
 * @param phrases 
 * @param sources 
 */
void optimize_phrases(unordered_map<size_t, Phrase> & phrases, unordered_set<Source, SourceHash> & sources){

}

/**
 * @brief Set "start" of each phrase to the left most boundary of each source
 * 
 * @param phrases Holder of the updated phrase set
 * @param sources Set of sources
 */
void reset_phrases( unordered_map<size_t, Phrase> & phrases, unordered_set<Source, SourceHash> & sources){
    PhraseHash hasher;
    unordered_map<size_t, Phrase> new_phrases;
    for(auto & s : sources){
        Phrase * p = s.p;

        auto got = phrases.find(hasher(*p));
        if (got == phrases.end()) continue;
        
        p->setStart(s.beg_interval.first);
        auto new_got = new_phrases.find(hasher(*p));
        assert(new_got == phrases.end());
        new_phrases[hasher(*p)] = *p;
    }
}
