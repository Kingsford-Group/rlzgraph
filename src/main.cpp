#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "RLZ.hpp"

#include <ctime>
#include <ratio>
#include <chrono>
// #include "util.cpp"

using namespace std;
using namespace std::chrono;


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
    if (!content.empty())
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

char revCompHelper(char c){
    switch (c){
        case 'A': return 'T';
        case 'C':return 'G';
        case 'G': return 'C';
        case 'T': return 'A';
        case 'N': return 'W';
        case 'M': return 'A';
        case 'Y': return 'B';
        case 'S': return 'D';
        case 'R': return 'H';
        case 'K': return 'V';
        case 'B': return 'M';
        case 'D': return 'Y';
        case 'H': return 'S';
        case 'V': return 'R';
        case 'U': return 'K';
        case 'W': return 'N';
        case '$': return 16;
    }
    cerr << "Unrecognized Character (revComp): " << c << endl;
    exit(1);
}

int numPhrases(RLZ & rlz){
    unordered_set<int> positions;
    for(auto pair : rlz.phrases){
        if (pair.second->start > rlz.csa_rev.size() - 1){
            positions.insert(pair.second->start);
            positions.insert(pair.second->start+1);
            continue;
        }
        if (positions.find(pair.second->start) == positions.end())
            positions.insert(pair.second->start);
        if (positions.find(pair.second->start+pair.second->length) == positions.end())
            positions.insert(pair.second->start+pair.second->length);
    }
        // cerr << "Number of unique positions (with opt): " << positions.size() << endl;
    return positions.size();
}

void verify(RLZ & rlz, vector<string> strings, int ref_idx, int id){
    int j = 0;
    for (int i=0;i<strings.size();i++){
        if (id != 0 && ref_idx == i) continue;    // does not add ref string
        cout << "Verifying " << i << endl;
        string test = rlz.decode(j);
        assert(test.compare(strings[i])==0);
        j++;
    }
}

string reverseComp(string toreverse){
    string s ="";
    for (int i=0;i<toreverse.length();i++){
        s+=revCompHelper(toreverse[i]);
    }
    reverse(s.begin(), s.end());
    return s;
}

int main(int argc, char* argv[]){

    bool success = parse_argument(argc, argv);

    if (success){
        
        // read in fasta files
        string ref;
        vector<string> strings = readFASTA(Input_strings, Strings_to_use);
        int start = 0;
        int id = 0;

        // 
        if (Input_ref!=""){
            vector<string> refv = readFASTA(Input_ref, 0);
            ref = refv[0]; 
        } else {
            ref = strings[ref_idx];
            id = 1;
        }
        
        ref = reverseComp(ref) + '#' + ref;

        cout << "Finished reading files." << endl;
        cout << "Length of reference: " << ref.length() << endl;

        reverse(ref.begin(), ref.end());

        RLZ rlz(ref);
        cout << "Built the initial SA" << endl;

        cout << "SA size: "<< rlz.csa.size() << endl;

        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
            rlz.RLZFactor(strings[i]);
            cout << "Done for " << i << endl;
        }

        // rlz.print_comp_string(0);
        // cout << rlz.decode(0) << endl;

        // verify(rlz, strings, ref_idx, id);
        cout << "Number of phrase boundaries for smallest\t" << numPhrases(rlz) << endl;
        
        // rlz.print_comp_string(0);
        string fname1 = "smallest_ecoli.phrases";
        string fname13 = "smallest_ecoli.comp";

        rlz.write_phrases(fname1);
        rlz.write_compString(fname13);

        // cout << rlz.decode(0) << endl;

        rlz.processSources(2);
        // verify(rlz, strings, ref_idx, id);
        cout << "Number of phrase boundaries for leftmost\t" << numPhrases(rlz) << endl;
        
        // rlz.print_comp_string(0);
        string fname2 = "leftmost_ecoli.phrases";
        string fname23 = "leftmost_ecoli.comp";
        rlz.write_phrases(fname2);
        rlz.write_compString(fname23);

        // cout << rlz.decode(0) << endl;

        // rlz.processSources(0);

        
        // rlz.print_comp_string(0);
        // string fname3 = "greedy_K12.phrases";
        // string fname33 = "greedy_K12.comp";

        // rlz.write_phrases(fname3);
        // rlz.write_compString(fname33);


        // cout << rlz.decode(0) << endl;
        
        // verify(rlz, strings, ref_idx, id);
        // cout << "Number of phrase boundaries for greedy: " << numPhrases(rlz) << endl;

        rlz.processSources(1);

        
        rlz.print_comp_string(0);
        string fname3 = "ILP_ecoli.phrases";
        string fname33 = "ILP_ecoli.comp";

        rlz.write_phrases(fname3);
        rlz.write_compString(fname33);


        // cout << rlz.decode(0) << endl;
        
        verify(rlz, strings, ref_idx, id);
        cout << "Number of phrase boundaries for ILP: " << numPhrases(rlz) << endl;


    }
    else {
        print_help();
        exit(1);
    }
}
