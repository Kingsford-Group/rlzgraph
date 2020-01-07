#include "RLZfact.h"

using namespace std;

RLZfact::RLZfact(){}
RLZfact::RLZfact(string ref, vector<int> Qarr, vector<int> Barr, vector<int> lenarr){
        R = ref;
        Q = Qarr;
        B = Barr;
        inputLen = lenarr;
    }
RLZfact::RLZfact(string ref, string s){}