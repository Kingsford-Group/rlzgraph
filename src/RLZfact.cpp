#include "RLZfact.h"

using namespace std;

RLZfact::RLZfact(){}


// RLZfact::RLZfact(vector<long int> Qarr, map<long int, long int> Bmap, vector<long int> B2, boost::dynamic_bitset<> bits, long int len){
//         Q = Qarr;
//         Bitarray = bits;
//         B = Bmap;
//         this->B2 = B2;
//         inputLen = len;
// }

RLZfact::RLZfact(vector<Phrase> phrases, long int id){
    stringID = id;
    this->phrases = phrases;
}

RLZfact::RLZfact(string & ref, SuffixTree & tree, string s, long int id){
    stringID = id;
    // perform factorization
    int i = 0;
    int idd = 0;
    while (i< s.length()) {
        // cout << "--------------------- " << i << endl;
        // cout << "--- Adding from position: " << i << endl;
        pair<int, int> ret = tree.traverse(s.substr(i, s.length()-i));
        Phrase phrase (idd, ret.first, ret.second);
        phrases.push_back(phrase);
        i+= ret.second;
        idd++;
    }

}

Phrase RLZfact::getPhrase(long int rank){
    return phrases[rank];
}
