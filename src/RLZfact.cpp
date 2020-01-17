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
        Phrase phrase (idd, (long int)ret.first, (long int)ret.second);
        // if (ret.first + ret.second -1 >= ref.length()) {
        //     cout << ref[ret.first] << endl;
        //     cout << ret.first <<"," << ret.second << endl;
        //     cout << s.substr(i, s.length()-i) << endl;
        //     exit(-1);
        // }
        phrases.push_back(phrase);
        // phrase.print();
        // cout << endl;
        i+= ret.second;
        idd++;
    }

}

string RLZfact::reconstruct(string & ref){
    string s ="";
    for (Phrase p : phrases){
        s+=ref.substr(p.pos,p.length);
    }
    return s;
}

Phrase RLZfact::getPhrase(long int rank){
    return phrases[rank];
}

void Phrase::print(){
    printf("(%lu, %lu)", this->pos, this->length);
}