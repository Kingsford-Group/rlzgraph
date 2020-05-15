/**
 * @file optimize_algo.hpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Algorithm that optimizes the number of nodes in the RLZ graph
 * @version 0.1
 * @date 2020-05-15
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#ifndef __OPTIMIZE_ALGO_H__
#define __OPTIMIZE_ALGO_H__

#include <unordered_set>
#include "RLZ.hpp"

void optimize_phrases(unordered_set<Phrase, PhraseHash> & phrases, unordered_set<Source, SourceHash> & sources);

void reset_phrases(unordered_set<Phrase, PhraseHash> & phrases, unordered_set<Source, SourceHash> & sources);




#endif


