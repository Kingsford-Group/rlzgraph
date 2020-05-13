#ifndef __RLZ_H__
#define __RLZ_H__

#include <sdsl/suffix_arrays.hpp>
#include <sdsl/csa_alphabet_strategy.hpp>
#include <string>

using namespace std;
using namespace sdsl;
using namespace alphabet_type;

/**
 * Each phrase is described by the range on the suffix array. 
 * start == end if the phrase only occurs once on the reference 
 */
struct Phrase{
    int start;
    int end;
    int length;
};


/**
 * Class RLZ 
 */
class RLZ{

    public: 
    csa_wt<> csa;       // the compressed suffix array to store the *reversed* reference
    vector<vector<Phrase> > compressed_strings;
    
    /* Construct a suffix array using the provided reference*/
    RLZ(string ref){
        construct_im(csa, ref, 1);
    }
    
    /* Factorize the input string using the constructed suffix array
     * Returns the number of phrases in the input string
     * */
    int RLZFactor(string to_process){
        vector<Phrase> phrases;
        auto strIt = to_process.begin();
        while (strIt != str.end()){
           Phrase p = query_bwt(strIt, str.end());
           phrases.push_back(p); 
        }
        compressed_strings.push_back(phrases)
        return phrses.size();
    }

    /* Query the bwt until it cannot go further.
     * Returns one phrase.
     */
    Phrase query_bwt(string::iterator strIt, string::iterator end){
        do {
            int l = csa.C[char2comp[*strIt]];
            int r = csa.C[char2comp[*strIt]+1];
            
            strIt++;

            int l_res = 
            int r_res;

            
        }
        while(strIt != end &&  r+1-l > 0);

};

#endif