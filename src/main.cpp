#include <iostream>
#include <stdio.h> 
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZfact.h"
#include "RLZgraph.h"
#include <boost/dynamic_bitset.hpp>

using namespace std;

// void printdec2bin(boost::dynamic_bitset<> n, int len){
//     int num[len];
//     int i=0;
//     while(n>0){
//         num[i]=n%2;
//         n = n/2;
//         i++;
//     }
//     for(int j=i-1;j>=0;j--) cout << num[j];
// }


int main(){
    string ref = "ATATTCGACGAGAT";
    string s1 = "ATAATTCGATTCGAT";

    RLZgraph graph(ref);
    graph.addString(s1);
    
    // string ref = "ATATTCGACGAGAT";
    // vector<long int> Q ={0,2,3,1};
    // vector<long int> B2 ={0,3,9,14};
    // map<long int, long int> B;
    // B[0]=0;
    // B[3]=1;
    // B[9]=2;
    // B[14]=3;

    // long int len=15;
    
    // boost::dynamic_bitset<> Bitarray(len);
    // Bitarray[0] = 1;
    // Bitarray[3] = 1;
    // Bitarray[9] = 1;
    // Bitarray[14] = 1;
    // // int b =1;
    // // b = (b << 14) | (1<< 11)|(1<<5)|1;
    // // cout << b << endl;
    // // printf("%16s\n", b);
    // RLZfact fact(Q, B, B2, Bitarray, len);

    // vector<RLZfact> factarr = {fact};
    
    // RLZgraph graph(ref, factarr);

    // for (int i = 0; i < 5; i++){
    //     cout << i <<": " ;
    //     for(pair<long int, long int> p : graph.HB[i])
    //         printf("(%u,%u)", p.first, p.second);
    //     cout << endl;
    // }

    // for(auto it = graph.H.cbegin();it!=graph.H.cend();++it)
    //     cout << it->first << ": " << it->second << endl;

    // graph.print_DFS();
    // printdec2bin(graph.BR, len);
}