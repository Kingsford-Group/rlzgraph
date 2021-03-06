/**
 * @file RLZ.hpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Defines RLZ factorization object
 * @version 0.1
 * @date 2020-05-15
 * 
 * @copyright Copyright (c) 2020
 * 
 */
#ifndef __RLZ_H__
#define __RLZ_H__

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_alphabet_strategy.hpp>
#include "gurobi_c++.h"
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>
#include <climits>

#include "ChunkLoader.cpp"

using namespace std;
using namespace sdsl;

typedef int_vector<>::size_type size_type;

/**
 * @brief Each phrase is a tulple of (start, len) that describes a substring in the compressed string.
 */
struct Phrase{
    int start;
    int length;
    bool reversed;  // records if the node should be traversed in reverse direction

    /**
     * @brief Compare phrases based on start position and length
     * 
     * @param p another phrase 
     * @return true if both start and length are equal
     */
    bool operator==(const Phrase & ph) const{
        return (this->start == ph.start) && (this->length == ph.length) && (this->reversed == ph.reversed);
    }


    void print(ostream & out) const{
        out << "(" << start << "," << length << "," << to_string(reversed) << ")";
    }

    // used to reset start points
    void setStart(int s){
        start = s;
    }

    void setDirection(bool rev){
        reversed = rev;
    }
};

/**
 * @brief Each source is desribed by a range on the suffix array.
 * For beginnings, there is only one interval, stored as integer pair
 * For endings, there are multiple intervals.
*/
 
struct Source{
    Phrase * p;
    pair<int, int> beg_interval;
    vector<int> start_loc;
    vector<bool> reversed;  // records if the node should be traversed in reverse direction.
    int length;
    map<int, int> loc_to_idx; // maps start_loc to its index in the vector;

    /**
     * @brief compare sources based on corresponding phrase
     * 
     * @param s  
     * @return true if two sources are from the same phrase
     */
    bool operator==(const Source & s) const{
        return (*(this->p) == *(s.p));
    }

    /* Prints begin and end intervals and corresponding phrase*/
    void print(ostream & out, bool phrase) const{

        // prints phrase
        if (phrase){
            if (p->start == 3499) cout << "FOUND" << endl;
            p->print(out);
            out << endl;
        }

        // prints begin and end intervals
        for (int loc : start_loc){
            out << loc << ", ";
        }
        out << endl;

        for (bool rev : reversed){
            out << rev << "," ;
        }
        out << endl;
    
    }
};

/**
 * @brief hash function for phrases
 * Hash value is computed by concatenating start and length
 */
class PhraseHash {
public: 
    // id is returned as hash function 
    size_t operator()(const Phrase& p) const
    {   
        hash<string> hasher;
        return hasher(to_string(p.start) + "/" + to_string(p.length)+"/"+to_string(p.reversed)); 
    } 
}; 

/**
 * @brief hash function for phrases pointers
 * Hash value is computed by concatenating start and length
 */
class PhrasePtrHash{
    public:
    size_t operator()(const Phrase * p) const{
        hash<string> hasher;
        return hasher(to_string(p->start) + "/" + to_string(p->length)+"/"+to_string(p->reversed)); 
    }
};

/**
 * @brief Compare phrases by start and length
 */
class PhrasePtrEqual{
    public:
    bool operator()(const Phrase * p1, const Phrase * p2) const {
        if (p1->start == p2->start && p1->length == p2->length){
            return true;
        } else {
            return false;
        }
    }
};

/**
 * @brief hash function for sources
 * Hash value is the pointer location of corresponding phrase
 */
class SourceHash{
public: 
    // id is returned as hash function 
    size_t operator()(const Source * s) const
    { 
        PhrasePtrHash hasher;
        return hasher(s->p);
    } 
};


/**
 * @brief hash function for sources
 * Hash value is the pointer location of corresponding phrase
 */
class SourceEqual{
public: 
    // id is returned as hash function 
    bool operator()(const Source * s1, const Source * s2) const
    { 
        if (s1->p == s2->p){
            return true;
        }
        else return false;
    } 
};

/**
 * @brief Compare sources
 * Source A < Source B if its SA interval begins or ends before B.
 */
class SourceMore{
    public:
    bool operator()(const Source * s1, const Source *s2) const{
        if (s1->beg_interval.first > s2->beg_interval.first){
            return true;
        } else if (s1->beg_interval.first == s2->beg_interval.first){
            return s1->beg_interval.second > s2->beg_interval.second;
        } else return false;
    }
};

/**
 * @brief pairing of reference position and number of source intersections. Used in greedy approach to optimize phrases.
 */
struct WeightedPos{
    int pos;
    int weight;
};


/**
 * @brief compare weighted pos a and weighted pos b. Return true if a.weight < b.weight. If they have equal weights, return true if a.pos < b.pos
 */
class ComparePos{
    bool comp(const WeightedPos & a, const WeightedPos & b) const{
        if (a.weight < b.weight) {
            return true;
        } else if (a.weight == b.weight){
            return a.pos < b.pos;
        }
        return false;
    }
};

struct ComparePosPtr {
  bool operator()(const WeightedPos* a, const WeightedPos* b) const  { 
        if (a->weight < b->weight) {
            return true;
        } else if (a->weight == b->weight){
            return a->pos < b->pos;
        }
        return false;
    }
};

struct pair_hash{
    size_t operator()(const pair<Source*, bool> & pair) const{
        return size_t(pair.first->p);
    }
};

/**
 * @brief Stores all phrases, sources and the compressed strings in the form of phrases. Also it stores the reference string as a csa. 
 */
class RLZ{

    public: 
    csa_wt<> csa;       // the compressed suffix array to store the *reversed* reference
    csa_wt<> csa_rev;   // the compressed suffix array to store the *actual* reference
    vector<vector<Phrase *> > compressed_strings;   
    // unordered_map<size_t, Phrase*> phrases;
    unordered_set<Phrase*, PhrasePtrHash, PhrasePtrEqual> phrases;
    unordered_set<Source*, SourceHash, SourceEqual> sources;
    unordered_map<int, char> newChar_toChar;
    unordered_map<char, int> newChar_toIdx;
    bool optimized = false;
    int numPhrases = 0;
    int totalBoundaries = 0;
    int totalLength = 0;
    bool processed=false;
    
    RLZ(string ref);

    /**
     * @brief Factor the input string into phrases
     * 
     * @param to_process  input string 
     * @return int the number of phrases
     */
    int RLZFactor(string & to_process);

    /**
     * @brief Factor the input string into phrases
     * 
     * @param loader  Chunk loader
     * @return int the number of phrases
     */
    int RLZFactor(ChunkLoader & loader);
    
    /**
     * @brief Query the bwt until it cannot go further. Creates a source if it has not been created.
     * 
     * @param strIt -- iterator of the input string
     * @param end  -- end iterator of the input string
     * @return Phrase* -- returned source
     */
    Phrase* query_bwt(string::iterator & strIt, string::iterator end);

    /**
     * @brief Query the bwt until it cannot go further. Creates a source if it has not been created.
     * 
     * @param nextChar -- first char to read, stored from the last iteration
     * @param loader -- Chunk loader
     * @return Phrase* -- returned source
     */
    Phrase* query_bwt(char * nextChar, ChunkLoader & loader);

    /**
     * @brief Substitute each phrase with corresponding substring
     * 
     * @param stringID 
     * @return string 
     */
    string decode(int stringID);

    /**
     * @brief Substitute each phrase with corresponding substring (phrase already in reference coordinate)
     * 
     * @param stringID 
     * @return string 
     */
    string decode_refCoord(int stringID);


    /**
     * @brief Print compressed string as a series of phrases
     * 
     * @param stringID 
     */
    void print_comp_string(int stringID);

    void print_sources();
    void print_phrases();


    /***************************
     *    Helper functions     *
    ****************************/

    /**
    * @brief Create a phrase object
    * 
    * @param pos 
    * @param length 
    * @return Phrase* 
    */
    pair<Phrase *, bool> create_phrase(int pos, int length);

    /**
     * @brief Search for the occurrence of the given parameter in the given range. Updates the resulting range and (possibly) shrinked range.
     * 
     * @param l Left boundary to search for (and stores the left boundary result) 
     * @param r Right boudary to search for (and stores the right boundary result)
     * @param C Character to search for
     * @param l_beg Left starting range (stores the shrinked left range)
     * @param r_beg Right starting range (stores the shrinked right range)
     */
    void backward_search_rank(int & l, int & r, char C, int & l_beg, int & r_beg);

        
    /**
     * @brief Go through all phrases (unqiue) and find the begin intervals of their sources
     * Query each phrase on the reversed reversed BWT. (backward search)
     * @param 0 if use greedy
     *        1 if use ILP
     *        2 if use smallest
     *        3 if use leftmost
     */
    void processSources(int option);
    
    /**
     * @brief transfer source boundaries from SA intervals to individual positions in the reference coordinate
     * 
     * @return total number of boundaries 
     */
    int transferSources();

    /**
     * @brief Transfer all end intervals of each source to the reversed reversed BWT using iSA
     * 
     * @param s source to modify
     */
    void transferSourceEnds(Source * s);

    /**
     * @brief Transfer all end intervals of each source to the reversed reversed BWT using iSA
     * 
     * @param s source to modify
     */
    void transferSourceStarts(Source * s);

    
    /**
     * @brief Optimize the phrase boundaries that result in the most number of overlaps. Updates the phrase set.
     * Greedy approach.
     * 
     * @param phrases 
     * @param sources 
     */
    void optimize_phrases();


        /**
     * @brief Optimize the phrase boundaries that result in the most number of overlaps using ILP. Updates the phrase set.
     * 
     * @param phrases 
     * @param sources 
     */
    void optimize_phrases_ILP();


    /**
     * @brief Set "start" of each phrase to the left most boundary of each source
     * 
     * @param phrases Holder of the updated phrase set
     * @param sources Set of sources
     */
    void reset_phrases();

    /**
     * @brief Set the phrase position to the left most position in the original reference
     * 
     * @param phrases 
     * @param sources 
     * @param csa 
     */
    void set_phrases_leftmost();


    /**
     * @brief Write sources to file
     * 
     * @param fname 
     */
    void write_sources(string & fname);

    
    /**
     * @brief Iterate over all sources and output its corresponding phrase
     * 
     * @param fname 
     */
    void write_phrases(string & fname);

    void write_compString(string & fname);

    /**
     * @brief Check if the alphabet contains the current character. If not, create a new phrase from the new alphabet
     * 
     * @param strIt 
     * @return Phrase* 
     */
    Phrase* check_alphabet(string::iterator & strIt);

    /**
     * @brief Check if the alphabet contains the current character. If not, create a new phrase from the new alphabet
     * 
     * @param currChar -- the first character to check
     * @return Phrase* 
     */
    Phrase* check_alphabet(char currChar);
    
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

/**
 * @brief Set "start" of each phrase to the left most boundary of each source
 * 
 * @param phrases Holder of the updated phrase set
 * @param sources Set of sources
 */
void reset_phrases( unordered_map<size_t, Phrase*> & phrases, unordered_set<Source*, SourceHash> & sources);

/**
 * @brief Set the phrase position to the left most position in the original reference
 * 
 * @param phrases 
 * @param sources 
 * @param csa 
 */
void set_phrases_leftmost(unordered_map<size_t, Phrase*> & phrases, unordered_set<Source*, SourceHash> & sources, csa_wt<> & csa);
#endif
