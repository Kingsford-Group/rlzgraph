#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "RLZ.hpp"
// #include "util.cpp"

using namespace std;

string Input_ref="";
string Input_strings="";
string Output_graph_name="";
string Output_phrase_name="";
int Strings_to_use = 0;
int ref_idx=0;

void print_help(){
    printf("--------------------------------------------------------------------------------------------\n");
    printf("|  USAGE: rlzgraph -r <ref.fa> -i <input.fa> -g <output_graph_name> -p <output_phrase_name> |\n");
    printf("--------------------------------------------------------------------------------------------\n");
    printf("-i is required. If reference fasta is missing, the first sequence of the input fasta file will be used as reference.\n");
    printf("Optional Arguments: \n");
    printf("          -ii <ref idx>         The index of the reference sequence\n");
    printf("          -n <num seq>          Number of sequences to use\n");
}

void print_version(){}

vector<string> readFASTA(string filename, int num_seq){
    fstream input(filename);
    if (!input.good()) {
        cerr << "BAD INPUT!: " << filename << endl;
        exit(1);
    }

    vector<string> strings;
    string line, content;
    int id = 0;
    while (getline(input, line)){
        if (num_seq > 0){
            if (id > num_seq){
                break;
            }
        }
        if (line[0] == '>' ){
            if(id != 0){
                strings.push_back(content);
                content.clear();
            }
            id++;
        } else if (!line.empty()){
            if(line.find(' ')!=string::npos){
                content.clear();
            } else {
                content+=line;
            }
        }
    }
    strings.push_back(content);
    cout << "Read " << strings.size() << " strings. "<< endl;
    return strings;
}


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

        reverse(ref.begin(), ref.end());

        RLZ rlz(ref);
        cout << "Built the initial SA" << endl;

        cout << "SA size: "<< rlz.csa.size() << endl;

        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
            cout << i << endl;
            rlz.RLZFactor(strings[i]);
        }
        // rlz.print_comp_string(0);

        // vector<long> pointers;

        // for (Phrase * p : rlz.compressed_strings[0]){
        //     if (p->start == 45 && p->length == 5){
        //         cerr << p << endl;
        //         pointers.push_back(long(p));
        //     }
        // }

        // for (const Source s : rlz.sources){
        //     if (long(s.p) == pointers[0]){
        //         s.print();
        //     }
        // }

        rlz.processSources(true);


        cout << "Finished adding sequences." << endl;

        // rlz.print_comp_string(0);


        // for (const Source s : rlz.sources){
        //     if (long(s.p) == pointers[0]){
        //         s.print();
        //     }
        // }


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
        //     string test = rlz.decode(j);
        //     // cout << test << endl;
        //     // cout << strings[i] << endl;
        //     cout << "reconstructed from rlz" << endl;

        //     assert(test.compare(strings[i])==0);
        //     // cout << "======" << endl;
        //     // cout  << test << endl;
        //     // cout << strings[i] << endl;
        //     // cout << "============" << endl;
        //     j++;
        // }

        // cout << "reconstruct works alright" << endl;
        
        unordered_set<int> positions;
        for(auto pair : rlz.phrases){
            positions.insert(rlz.csa_rev[pair.second->start]);
            positions.insert(rlz.csa_rev[pair.second->start]+pair.second->length);
        }
        // cerr << "Number of unique positions (with opt): " << positions.size() << endl;
        cerr << positions.size() << "," ;

        reset_phrases(rlz.phrases, rlz.sources);

        unordered_set<int> positions2;
        for(auto pair : rlz.phrases){
            positions2.insert(rlz.csa_rev[pair.second->start]);
            positions2.insert(rlz.csa_rev[pair.second->start]+pair.second->length);
        }
        // cerr << "Number of unique positions (without opt): " << positions2.size() << endl;
        cerr << positions2.size() << "," ;

        set_phrases_leftmost(rlz.phrases, rlz.sources, rlz.csa_rev);

        unordered_set<int> positions3;
        for(auto pair : rlz.phrases){
            positions3.insert(rlz.csa_rev[pair.second->start]);
            positions3.insert(rlz.csa_rev[pair.second->start]+pair.second->length);
        }
        // cerr << "Number of unique positions (leftmost): " << positions2.size() << endl;
        cerr << positions3.size() << endl;


    }
    else {
        print_help();
        exit(1);
    }
}
