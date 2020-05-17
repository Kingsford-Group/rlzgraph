#include "RLZ.hpp"


int main(int argc, const char** argv){

    string s = argv[1];
    string input = argv[2];
    reverse(s.begin(), s.end());
    RLZ rlz(s);

    rlz.RLZFactor(input);

    rlz.print_comp_string(0);

    rlz.processSources();

    rlz.print_comp_string(0);

    rlz.print_phrases();
    rlz.print_sources();

    cout << rlz.decode(0) << endl;

    cout << " i SA ISA PSI LF BWT   T[SA[i]..SA[i]-1]" << endl;
    csXprintf(cout, "%2I %2S %3s %3P %2p %3B   %:3T", rlz.csa); 

    cout << endl;

    cout << " i SA ISA PSI LF BWT   T[SA[i]..SA[i]-1]" << endl;
    csXprintf(cout, "%2I %2S %3s %3P %2p %3B   %:3T", rlz.csa_rev); 
}
