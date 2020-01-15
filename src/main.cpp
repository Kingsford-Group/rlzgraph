#include <iostream>
#include <stdio.h> 
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZfact.h"
#include "RLZgraph.h"
#include "util.h"
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

string Input_ref="";
string Input_strings="";


bool parse_argument(int argc, char * argv[]){
    bool success=true;
	bool specify_mq=false;
	for(int i=1; i<argc; i++){
		if(string(argv[i])=="--help"){
			print_help();
			exit(0);
		}
		if(string(argv[i])=="--version"){
			print_version();
			exit(0);
		}
        if (string(argv[i])=="-r"){
            Input_ref = string(argv[++i]);
            i++;
        }
        if (string(argv[i])=="-i"){
            Input_strings = string(argv[++i]);
            i++;
        }
        else{
            cerr << "Unknown argument: " << string(argv[i]) << endl;\
            exit(1);
        }
    }
    if (Input_ref == "" && Input_strings=="") return false;
    return success;
}

int main(int argc, char* argv[]){

    bool success = parse_argument(argc, argv);

    if (success){
        
        string ref;
        vector<string> strings = readFASTA(Input_strings);
        int start = 0;
        int id = 0;

        if (Input_ref!=""){
            vector<string> refv = readFASTA(Input_ref);
            ref = refv[0]; 
        } else {
            ref = strings[0];
            id = 1;
        }

        cout << "Finished reading files." << endl;
        cout << "Length of reference: " << ref.length() << endl;

        RLZgraph graph (ref);
        cout << "Built the initial tree" << endl;
        for(int i = id; i<strings.size(); i++){
            cout << i << endl;
            graph.addString(strings[i]);
        }
        cout << "Finished adding sequences." << endl;

        for (int i=id;i<strings.size();i++){
            cout << i<< endl;
            string test = graph.reconstruct(i-id);
            assert(test == strings[i]);
        }

/*
        cout << "Size of starts: " << graph.phraseStarts.size() <<endl; 
        auto it = graph.phraseStarts.begin();
        while(it != graph.phraseStarts.end()){
            printf("Pos: %lu, Phrase: ", it->first);
            for (backPhrase p : it->second){
                printf("(%u, %lu),", p.stringID, p.rank);
            }
            cout << endl;
            it++;
        }

                
        cout << "Size of Ends: " << graph.phraseEnds.size() <<endl; 
        it = graph.phraseEnds.begin();
        while(it != graph.phraseEnds.end()){
            printf("Pos: %lu, Phrase: ", it->first);
            for (backPhrase p : it->second){
                printf("(%u, %lu),", p.stringID, p.rank);
            }
            cout << endl;
            it++;
        }
*/
        printf("Graph built with %lu nodes and %lu edges.\n", graph.numNodes, graph.numEdges);
    }
    
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
