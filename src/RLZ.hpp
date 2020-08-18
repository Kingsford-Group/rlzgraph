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
#include <string>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

using namespace std;
using namespace sdsl;

typedef int_vector<>::size_type size_type;

/**
 * @brief Each phrase is a tulple of (start, len) that describes a substring in the compressed string.
 */
struct Phrase{
    int start;
    int length;

    /**
     * @brief Compare phrases based on start position and length
     * 
     * @param p another phrase 
     * @return true if both start and length are equal
     */
    bool operator==(const Phrase & p) const{
        return (this->start == p.start && this->length == p.length);
    }

    void print(ostream & out) const{
        out << "(" << start << "," << length << ")";
    }

    // used to reset start points
    void setStart(int s){
        start = s;
    }
};

/**
 * @brief Each source is desribed by a range on the suffix array.
 * For beginnings, there is only one interval, stored as integer pair
 * For endings, there are multiple intervals.
*/
 
struct Source{
    Phrase * p;
    mutable pair<int, int> beg_interval;       // stored as one pair of begin and end pair
    mutable vector<int> end_interval;  //stored as pairs of interval begins and ends. Each character has a continuous interval.
    int length;

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
            p->print(out);
            out << endl;
        }

        // prints begin and end intervals
        out << "Begin: " << beg_interval.first <<"," << beg_interval.second<< endl;
        out << "End: ";
        for(int i=0; i<end_interval.size(); i++){
            out << end_interval[i] << ",";
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
        return hasher(to_string(p.start) + "/" + to_string(p.length)); 
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
        return size_t(s->p);
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
    unordered_map<size_t, Phrase*> phrases;
    unordered_set<Source*, SourceHash> sources;
    unordered_map<int, char> newChar;
    unordered_map<char, int> newChar_rev;
    bool optimized = false;
    int numPhrases = 0;
    int totalLength = 0;
    
    RLZ(string ref);

    /**
     * @brief Factor the input string into phrases
     * 
     * @param to_process  input string 
     * @return int the number of phrases
     */
    int RLZFactor(string & to_process);
    
    /**
     * @brief Query the bwt until it cannot go further. Creates a source if it has not been created.
     * 
     * @param strIt -- iterator of the input string
     * @param end  -- end iterator of the input string
     * @return Phrase* -- returned source
     */
    Phrase* query_bwt(string::iterator & strIt, string::iterator end);

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
     */
    void processSources(bool optimize);
    
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