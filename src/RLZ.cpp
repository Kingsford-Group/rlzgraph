#include "RLZ.hpp"
#include <iostream>

using namespace std;

/* Construct a suffix array using the provided reference*/
RLZ::RLZ(string ref){
    construct_im(csa, ref, 1);
    totalLength = csa.size();
}

int RLZ::RLZFactor(string & to_process){
    vector<const Phrase*> phrases;
    auto strIt = to_process.begin();
    while (strIt != to_process.end()){
        const Phrase * p = query_bwt(strIt, to_process.end());
        phrases.push_back(p);
    }
    compressed_strings.push_back(phrases);
    numPhrases+=phrases.size();
    return phrases.size();
}


const Phrase* RLZ::query_bwt(string::iterator & strIt, string::iterator end){
    // check if the first character is in the alphabet
    bool inAlphabet = false;
    for(int i=1; i<csa.sigma; i++){
        if (csa.comp2char[i] == *strIt){
            inAlphabet = true; 
        }
    }

    if (!inAlphabet){
        auto find = newChar.find(*strIt);
        int idx = 0;
        if (find == newChar.end()){
            newChar[*strIt] = totalLength;
            totalLength+=1;
        }
        idx = newChar[*strIt];
        const Phrase* p= create_phrase(idx, 1);
        strIt++;
        return p;
    }
    
    size_type l = 0;
    size_type r = csa.size()-1;
    int l_res = 0;
    int r_res = 0;
    int length = 1;
    while(strIt != end && r+1-l > 0){
        l_res = l;
        r_res = r;
        backward_search(csa, l, r, char(*strIt), l, r);
        // fflush(stdout);
        // printf("In loop: (Left: %u, Right: %u)\n", l, r);
        strIt ++;
        length ++;
    }

    if (r < l){
        strIt --;
        length --;
    } else{
        l_res = l;
        r_res = r;
    }

    
    // fflush(stdout);
    // cerr <<  l_res << "," << r_res << endl;


    // start interval is (l_res, r_res)
    pair<int, int> beg_interval = make_pair(int(l_res), int(r_res));
    const Phrase* p= create_phrase(csa[l_res], length-1);

    // end interval is one more LF operation
    vector<pair<int, int> > end_interval; 
    for (int i=0; i < csa.sigma;i++){
        size_type new_l = 0;
        size_type new_r = 0;
        // fflush(stdout);
        // cerr <<  l_res << "," << r_res << endl;
        backward_search(csa, l_res, r_res, csa.comp2char[i], new_l, new_r);
        // int cc = csa.C[i];
        // int l_rank = csa.bwt.rank(l, csa.comp2char[i]);
        // int r_rank = csa.bwt.rank(r+1, csa.comp2char[i]);
        // cerr << new_l << "," << new_r << "," << (new_l <= new_r) << endl;
        if (int(new_l) <= int(new_r)){
            // cerr << "if " << new_l << "," << new_r << "," << (new_l <= new_r) << endl;

            pair<int, int> end_pair = make_pair(new_l, new_r);
            end_interval.push_back(end_pair);
        }
    }

    Source source = {p, beg_interval, end_interval, length-1};
    auto ret = sources.insert(source);
    return p;
}


void RLZ::print_comp_string(int stringID){
    for( const Phrase* p : compressed_strings[stringID]){
        p->print();
        cout << ",";
    }
    cout << endl;
}

const Phrase* RLZ::create_phrase(int pos, int length){
    Phrase phrase = {pos, length};
    auto ret = phrases.insert(phrase);
    
    return &(*(ret.first));
}

void RLZ::print_sources(){
    cout << "Printing sources." << endl;
    for(auto s: sources){
        s.print();
    }
}

void RLZ::print_phrases(){
    cout << "Printing phrases." << endl;
    for(auto s : phrases){
        s.print();
    }
    cout << endl;
}


// int main(int argc, const char** argv){

//     string s = argv[1];
//     string input = argv[2];
//     reverse(s.begin(), s.end());
//     RLZ rlz(s);

//     rlz.RLZFactor(input);

//     rlz.print_comp_string(0);

//     rlz.print_phrases();
//     rlz.print_sources();

    

//     cout << " i SA ISA PSI LF BWT   T[SA[i]..SA[i]-1]" << endl;
//     csXprintf(cout, "%2I %2S %3s %3P %2p %3B   %:3T", rlz.csa); 
// }
