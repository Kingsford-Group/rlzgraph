#ifndef __RLZ_H__
#define __RLZ_H__

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_alphabet_strategy.hpp>
#include <string>
#include <unordered_set>

using namespace std;
using namespace sdsl;

typedef int_vector<>::size_type size_type;

/**
 * Each phrase is a tulple of (start, len) that describes a substring in the compressed string.
 */
struct Phrase{
    int start;
    int length;

    bool operator==(const Phrase & p) const{
        return (this->start == p.start && this->length == p.length);
    }

    void print() const{
        printf("(%ul, %ul)", start, length);
    }
};

/**
 * Each source is desribed by a range on the suffix array.
 * For beginnings, there is only one interval.
 * For endings, there are multiple intervals.
*/
struct Source{
    const Phrase * p;
    pair<int, int> beg_interval;       // stored as one pair of begin and end pair
    vector<pair<int, int> > end_interval;  //stored as pairs of interval begins and ends. Each character has a continuous interval.
    int length;

    bool operator==(const Source & s) const{
        return (*(this->p) == *(s.p));
    }
};

/**
 * @brief hash function for phrases
 * 
 */
class PhraseHash {
public: 
    // id is returned as hash function 
    size_t operator()(const Phrase& p) const
    { 
        return p.start * p.length; 
    } 
}; 

class SourceHash{
public: 
    // id is returned as hash function 
    size_t operator()(const Source& s) const
    { 
        return size_t(s.p);
    } 
};

/**
 * @brief Stores all phrases, sources and the compressed strings in the form of phrases. Also it stores the reference string as a csa. 
 */
class RLZ{

    public: 
    csa_wt<> csa;       // the compressed suffix array to store the *reversed* reference
    vector<vector<const Phrase *> > compressed_strings;
    unordered_set<Phrase, PhraseHash> phrases;
    unordered_set<Source, SourceHash> sources;
    bool optimized = false;
    
    RLZ(string ref);

    /**
     * @brief Factor the input string into phrases
     * 
     * @param to_process  input string 
     * @return int the number of phrases
     */
    int RLZFactor(string to_process);
    
    /**
     * @brief Query the bwt until it cannot go further. Creates a source if it has not been created.
     * 
     * @param strIt -- iterator of the input string
     * @param end  -- end iterator of the input string
     * @return Phrase* -- returned source
     */
    const Phrase* query_bwt(string::iterator & strIt, string::iterator end);
    
    /**
     * @brief Print compressed string as a series of phrases
     * 
     * @param stringID 
     */
    void print_comp_string(int stringID);

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
    const Phrase* create_phrase(int pos, int length);

 
};

#endif