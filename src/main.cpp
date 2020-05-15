#include <iostream>
#include <stdio.h> 
#include <fstream>
#include <stdlib.h>
#include <vector>
#include <map>
#include "RLZ.hpp"
#include "util.hpp"

using namespace std;

string Input_ref="";
string Input_strings="";
string Output_graph_name="";
string Output_phrase_name="";
int Strings_to_use = 0;
int ref_idx=0;

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
        else if (string(argv[i])=="-ii"){
            ref_idx = stoi(argv[++i]);
        }
        else if (string(argv[i])=="-g"){
            Output_graph_name = string(argv[++i]);
            // i++;
        }        
        else if (string(argv[i])=="-p"){
            Output_phrase_name = string(argv[++i]);
            // i++;
        }
        else if (string(argv[i])=="-n"){
            Strings_to_use = stoi(argv[++i]);
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
        vector<string> strings = readFASTA(Input_strings, Strings_to_use);
        int start = 0;
        int id = 0;

        if (Input_ref!=""){
            vector<string> refv = readFASTA(Input_ref, 0);
            ref = refv[0]; 
        } else {
            ref = strings[ref_idx];
            id = 1;
        }

        cout << "Finished reading files." << endl;
        cout << "Length of reference: " << ref.length() << endl;

        RLZ rlz(ref);
        cout << "Built the initial SA" << endl;

        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
            cout << i << endl;
            rlz.RLZFactor(strings[i]);
        }

        cout << "Finished adding sequences." << endl;

        // if (Output_graph_name!=""){
        //     cout<<"Writing graph to " << Output_graph_name << endl;
        //     graph.writeGraph(Output_graph_name);
        // }

        // if (Output_phrase_name!=""){
        //     cout<<"Writing phrases to " << Output_phrase_name << endl;
        //     graph.writePhrases(Output_phrase_name);
        // }

        // cout << graph.tree.size << endl;
        // printf("Graph built with %lu nodes and %lu edges.\n", graph.numNodes, graph.numEdges);

        printf("Total number of phrases is: %u\n", rlz.numPhrases);
        printf("Total number of unique phrases is: %u\n", rlz.phrases.size());

        // int j = 0;
        // for (int i=0;i<strings.size();i++){
        //     if (id != 0 && ref_idx == i) continue;    // does not add ref string

        //     cout << i<< endl;
        //     string test = graph.reconstruct(j);
            
        //     cout << "reconstructed from graph" << endl;

        //     // cout << graph.rlzarr.size() << endl;
        //     // cout << i-id << endl;
        //     string test2 = graph.rlzarr[j].reconstruct(graph.ref);
        //     cout << "reconstructed from phrases" << endl;
        //     // cout << test.length() << endl;
        //     // cout << test2.length() << endl;
        //     // cout << strings[i].length()  << endl;

        //     assert(test.compare(strings[i])==0);
        //     assert(test2.compare(strings[i])==0);
        //     // cout << "======" << endl;
        //     // cout  << test << endl;
        //     // cout << strings[i] << endl;
        //     // cout << "============" << endl;
        //     j++;
        // }
        // cout << "reconstruct works alright" << endl;

    }
    else {
        print_help();
        exit(1);
    }
}
