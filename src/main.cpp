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
string Output_graph_name="";
string Output_phrase_name="";

bool parse_argument(int argc, char * argv[]){
    bool success=true;
	bool specify_mq=false;
	for(int i=1; i<argc; i++){
		if(string(argv[i])=="--help"){
			print_help();
			exit(0);
		}
		else if(string(argv[i])=="--version"){
			print_version();
			exit(0);
		}
        else if (string(argv[i])=="-r"){
            Input_ref = string(argv[++i]);
            // i++;
        }
        else if (string(argv[i])=="-i"){
            Input_strings = string(argv[++i]);
            // i++;
        }
        else if (string(argv[i])=="-g"){
            Output_graph_name = string(argv[++i]);
            // i++;
        }        
        else if (string(argv[i])=="-p"){
            Output_phrase_name = string(argv[++i]);
            // i++;
        }
        else{
            cerr << "Unknown argument: " << string(argv[i]) << endl;\
            exit(1);
        }
    }
    if (Input_strings=="") return false;
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

        if (Output_graph_name!=""){
            cout<<"Writing graph to " << Output_graph_name << endl;
            graph.writeGraph(Output_graph_name);
        }

        if (Output_phrase_name!=""){
            cout<<"Writing phrases to " << Output_phrase_name << endl;
            graph.writePhrases(Output_phrase_name);
        }

        // int k=0;
        // for (RLZfact fact : graph.rlzarr){
        //     int loc = 0;
        //     printf("The %u th string: ",k);
        //     for (long int j=0;j<fact.phrases.size();j++){
        //         printf("(%lu, %lu)", fact.getPhrase(j).pos, fact.getPhrase(j).length);
        //         assert(ref.substr(fact.getPhrase(j).pos, fact.getPhrase(j).length).compare(strings[k].substr(loc, fact.getPhrase(j).length))==0);
        //         loc+=fact.getPhrase(j).length;
        //     }
        //     cout << endl;
        //     k++;
        // }

        // string test = graph.reconstruct(3);
        for (int i=id;i<strings.size();i++){
            cout << i<< endl;
            string test = graph.reconstruct(i-id);
            string test2 = graph.rlzarr[i-id].reconstruct(graph.ref);
            // cout << test.length() << endl;
            // cout << strings[i].length()  << endl;
            // cout << "======" << endl;
            assert(test.compare(strings[i])==0);
            // cout  << test << endl;
            // cout << strings[i] << endl;
            // cout << "============" << endl;
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
    else {
        print_help();
        exit(1);
    }
}
