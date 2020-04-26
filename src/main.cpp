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

        RLZgraph graph (ref);
        
        cout << "Built the initial tree" << endl;
        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
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

        cout << graph.tree.size << endl;
        printf("Graph built with %lu nodes and %lu edges.\n", graph.numNodes, graph.numEdges);


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
        // graph.printGraph();
        // string test = graph.reconstruct(3);
        int j = 0;
        for (int i=0;i<strings.size();i++){
            if (id != 0 && ref_idx == i) continue;    // does not add ref string

            cout << i<< endl;
            string test = graph.reconstruct(j);
            
            cout << "reconstructed from graph" << endl;

            // cout << graph.rlzarr.size() << endl;
            // cout << i-id << endl;
            string test2 = graph.rlzarr[j].reconstruct(graph.ref);
            cout << "reconstructed from phrases" << endl;
            // cout << test.length() << endl;
            // cout << test2.length() << endl;
            // cout << strings[i].length()  << endl;

            assert(test.compare(strings[i])==0);
            assert(test2.compare(strings[i])==0);
            // cout << "======" << endl;
            // cout  << test << endl;
            // cout << strings[i] << endl;
            // cout << "============" << endl;
            j++;
        }
        cout << "reconstruct works alright" << endl;

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


    }
    else {
        print_help();
        exit(1);
    }
}
