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

RLZfact::RLZfact(vector<Phrases> phrases){
    this->phrases = phrases;
}

RLZfact::RLZfact(string & ref, SuffixTree & tree, string s){
    // perform factorization
    int i = 0;
    int idd = 0;
    while (i< s1.length()) {
        // cout << "--------------------- " << i << endl;
        pair<int, int> ret = tree->traverse(s1.substr(i, s1.length()-i));
        Phrase phrase (idd, ret.first, ret.second);
        phrases.push_back(phrase);
        i+= ret.second;
        idd++;
    }

}