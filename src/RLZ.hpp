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
};

/**
 * Each source is desribed by a range on the suffix array.
 * For beginnings, there is only one interval.
 * For endings, there are multiple intervals.
*/
struct Source{
    Phrase * p;
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
    vector<vector<Phrase *> > compressed_strings;
    unordered_set<Phrase, PhraseHash> phrases;
    unordered_set<Source, SourceHash> sources;
    
    /* Construct a suffix array using the provided reference*/
    RLZ(string ref){
        construct_im(csa, ref, 1);
    }
    
    /**
     * @brief Factor the input string into phrases
     * 
     * @param to_process  input string 
     * @return int the number of phrases
     */
    int RLZFactor(string to_process){
        vector<Phrase*> phrases;
        auto strIt = to_process.begin();
        while (strIt != to_process.end()){
           Phrase * p = query_bwt(strIt, to_process.end());
           phrases.push_back(p);
        }
        compressed_strings.push_back(phrases);
        return phrases.size();
    }

    /**
     * @brief Query the bwt until it cannot go further. Creates a source if it has not been created.
     * 
     * @param strIt -- iterator of the input string
     * @param end  -- end iterator of the input string
     * @return Phrase* -- returned source
     */
    Phrase* query_bwt(string::iterator strIt, string::iterator end){
        size_type l = 0;
        size_type r = csa.size()-1;
        size_type l_res = 0;
        size_type r_res = 0;
        int length = 0;
        do {
            l_res = l;
            r_res = r;
            backward_search(csa, l, r, char(*strIt), l, r);
            strIt ++;
            length ++;
        }
        while(strIt != end &&  r+1-l > 0);

        // start interval is (l_res, r_res)
        // end interval is one more LF operation
        pair<int, int> beg_interval = make_pair(l_res, r_res);
        vector<pair<int, int>> end_interval; 
        for (int i=1; i < csa.sigma;i++){
            int cc = csa.C[i];
            int l_rank = csa.bwt.rank(l_res, csa.comp2char[i]);
            int r_rank = csa.bwt.rank(r_res+1, csa.comp2char[i]);
            pair<int, int> end_pair = make_pair(cc+l_rank, cc+r_rank);
            end_interval.push_back(end_pair);
        }

        Phrase* p= create_phrase(l_res, length-1);
        Source source = {p, beg_interval, end_interval, length-1};
        auto ret = sources.insert(source);
        return p;
    }
    
    /**
     * @brief Decode the compressed string
     * 
     * @param stringID id of the compressed strings to decompress
     * @return string the decompressed string
     */
    string decode(int stringID){
        
    }

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
    Phrase* create_phrase(int pos, int length){
        Phrase phrase = {csa[pos], length};
        auto ret = phrases.insert(phrase);
        Phrase p = *(ret.first);
        return &p;
    }

    
};

#endif