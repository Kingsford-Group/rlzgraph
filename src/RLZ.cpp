#include "RLZ.hpp"
#include "optimize_algo.hpp"
#include <iostream>

using namespace std;

/* Construct a suffix array using the provided reference*/
RLZ::RLZ(string ref){
    string revRef;
    revRef.assign(ref.rbegin(), ref.rend());

    construct_im(csa, ref, 1);
    construct_im(csa_rev, revRef, 1);
    
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

    processSources();

    return phrases.size();
}

void RLZ::processSources(){
    //TODO Change that to global boolean set from user-given parameter
    bool optimize = true;

    for(auto & s : sources){
        transferSourceStarts(s);
        transferSourceEnds(s);
    }
    
    if (optimize)
        optimize_phrases(phrases, sources);
    else
        reset_phrases(phrases, sources);
}

// void RLZ::backward_search_rank(int & l, int & r, char c, int & l_beg, int & r_beg){
//     assert(l <= r); assert(r < csa.size());
//     size_type cc = csa.char2comp[c];
//     if (cc == 0 and c > 0) {     // character not in BWT
//         l = 1;
//         r = 0;
//     } else {
//         size_type c_begin = csa.C[cc];
//         if (l == 0 and r+1 == csa.size()) {
//             l = int(c_begin);
//             r = csa.C[cc+1] - 1;
//             l_beg = l;
//             r_beg = r;
//         } else {
//             int l_rank = csa.bwt.rank(l,c);
//             int r_rank = csa.bwt.rank(r+1,c);

//             if (l_rank == 0 && r_rank != 0) {
//                 l_beg = l_beg + csa.bwt.select(l_rank+1, c) - l;
//                 r_beg = r_beg -(r - csa.bwt.select(r_rank, c));
//             } else if (l_rank != 0 && r_rank != 0){
//                 l_beg = l_beg + csa.bwt.select(l_rank, c) - l;
//                 r_beg = r_beg -(r - csa.bwt.select(r_rank, c));
//             }

//             l = c_begin + l_rank; // count c in bwt[0..l-1]
//             r = c_begin + r_rank - 1; // count c in bwt[0..r]
//         }
//         cerr << "in rank func " << l_beg << ", " << r_beg << endl;
//     }
//     assert(r+1-l >= 0);
// }


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
    int r_res = r;
    // int beg_s = 0;
    // int beg_e = r;
    int length = 1;
    while(strIt != end && r+1-l > 0){
        l_res = l;
        r_res = r;

        // backward_search_rank(l, r, (*strIt), beg_s, beg_e);
        backward_search(csa, l, r, (*strIt), l, r);
        
        strIt ++;
        length ++; 
    }

    // whether the last character is found or the last character is the end of the string
    if (r < l){
        strIt --;
        length --;
    } else{
        l_res = l;
        r_res = r;
    }

    // start interval is (l_res, r_res)
    pair<int, int> beg_interval = make_pair(-1, -1);
    const Phrase* p= create_phrase(csa[l_res], length-1);

    // end interval is one more LF operation
    vector<int> end_interval; 
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
            for(int i = new_l; i <= new_r; i++){
                end_interval.push_back(i);
            }
        }
    }

    Source source = {p, beg_interval, end_interval, length-1};
    auto ret = sources.insert(source);
    return p;
}


void RLZ::print_comp_string(int stringID){
    cout << "Printing compressed strings" << endl;
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

string RLZ::decode(int stringID){
    string toReturn;
    for(const Phrase * p : compressed_strings[stringID]){
        toReturn += extract(csa_rev, p->start, p->start+p->length-1);
    }
    return toReturn;
}

void RLZ::transferSourceEnds(const Source & s){
    vector<int> new_endInterval;
    for (int i : s.end_interval){
        int new_end = 0;
        cerr << i << " --> " ;
        if (csa[i] == csa.size()-1) {
            new_end = i;
        }else{
            new_end = csa_rev.isa[csa.size() - 2 - csa[i]];
        }
        cerr << new_end << endl; 
        new_endInterval.push_back(new_end);
    }
    s.end_interval = new_endInterval;
}

void RLZ::transferSourceStarts(const Source & s){
    const Phrase * p = s.p;
    string sub = extract(csa, p->start, p->start + p->length - 1);
    size_type l = 0; 
    size_type r = csa_rev.size()-1;
    reverse(sub.begin(), sub.end());
    backward_search(csa_rev, l, r, sub.begin(), sub.end(), l, r );   // search for the pattern in the reversed reversed BWT.
    s.beg_interval = make_pair(l,r);
}