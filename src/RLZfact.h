#ifndef __RLZfact_H__
#define __RLZfact_H__

#include <vector>
#include <stdio.h> 
#include <stdlib.h>
#include <iostream>

using namespace std;

class RLZfact{
    public:
    string R;
    vector<int> Q;
    vector<int> B; // for now it's the length of each phrase
    vector<int> inputLen;
    
    RLZfact();
    RLZfact(string ref, vector<int> Qarr, vector<int> Barr, vector<int> lenarr);
    RLZfact(string ref, string s);
    
};

#endif