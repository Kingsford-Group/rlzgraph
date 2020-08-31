#include "../src/RLZ.hpp"
#include <string>
#include <iostream>
#include <unordered_set>

using namespace std;

int numPhrases(RLZ & rlz){
    unordered_set<int> positions;
    for(auto pair : rlz.phrases){
        if (pair.second->start > rlz.csa_rev.size() - 1){
            positions.insert(pair.second->start);
            positions.insert(pair.second->start+1);
            continue;
        }
        if (positions.find(rlz.csa_rev[pair.second->start]) == positions.end())
            positions.insert(rlz.csa_rev[pair.second->start]);
        if (positions.find(rlz.csa_rev[pair.second->start]+pair.second->length) == positions.end())
            positions.insert(rlz.csa_rev[pair.second->start]+pair.second->length);
    }
        // cerr << "Number of unique positions (with opt): " << positions.size() << endl;
    return positions.size();
}

int main(int argc, char** argv){
    string ref = "ATATTCGACGAGAT";
    string s1 = "ATAATTCGATTCGAA";
    string s2 = "ATTTCGAGAM";

    reverse(ref.begin(), ref.end());
    RLZ rlz(ref);
    rlz.RLZFactor(s1);

    rlz.processSources(stoi(argv[1]));

    rlz.print_phrases();
    rlz.print_sources();
    rlz.print_comp_string(0);

    cout << rlz.decode(0) << endl;

    cout << "Num unique positions: " << numPhrases(rlz) << endl;

    
    // cout << " i SA ISA PSI LF BWT   T[SA[i]..SA[i]-1]" << endl;
    // csXprintf(cout, "%2I %2S %3s %3P %2p %3B   %:3T", rlz.csa_rev); 

    return 0;
}