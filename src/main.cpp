#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZfact.h"
#include "RLZgraph.h"

using namespace std;

void printdec2bin(int n, int len){
    int num[len];
    int i=0;
    while(n>0){
        num[i]=n%2;
        n = n/2;
        i++;
    }
    for(int j=i-1;j>=0;j--) cout << num[j];
}

int main(){
    string ref = "ATATTCGACGAGAT";
    vector<int> Q ={0,2,3,1};
    vector<int> B ={3,6,5,1};
    // int b =1;
    // b = (b << 14) | (1<< 11)|(1<<5)|1;
    // cout << b << endl;
    vector<int> len = {15};
    // printf("%16s\n", b);
    RLZfact fact(ref, Q, B, len);

    vector<RLZfact> factarr = {fact};
    
    RLZgraph graph(factarr);

    for (auto it : graph.HB){
        cout << it.first << ": ";
        for (pair<int, int> p : it.second){
            printf("(%u,%u), ", p.first, p.second);
        }
        cout << endl;
    }

    printdec2bin(graph.BR[0], ref.length());
}