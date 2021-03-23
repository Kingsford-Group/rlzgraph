#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "RLZ.hpp"
#include "RLZGraph.hpp"
// #include "vg.hpp"
#include <sdsl/bit_vectors.hpp>

#include <ctime>
#include <ratio>
#include <chrono>
#include <sys/time.h>
#include <sys/resource.h>
// #include "util.cpp"

using namespace std;
using namespace std::chrono;
// using namespace vg;
using namespace sdsl;

string Input_ref="";
string Input_strings="";
string Output_graph_name="";
string Output_phrase_name="";
string Output_source_name="";
string Output_compressed_name="";

bool writePhrase = false;
bool writeSource = false;
bool writeCompressed = false;
bool writeGraph = false;
bool runGreedy = false;
bool runILP = false;

int Strings_to_use = 0;
int ref_idx=0;

void print_help(){
    printf("-------------------------------------------------\n");
    printf("|  USAGE: rlzgraph [OPTIONS] -i <input.fasta>   |\n");
    printf("-------------------------------------------------\n");
    printf("Required Arguments:\n");
    printf("        -i      <input.fasta>       Fasta file containing complete genomic sequences. Each sequence will be treated as one complete genome.\n");
    printf("                                    If reference fasta is missing, the first sequence of the input fasta file will be used as reference.\n");
    printf("Optional Arguments: \n");
    printf("        -ii     <ref idx>           The index of the reference sequence in input.fasta\n");
    printf("        -n      <num seq>           Number of sequences to use in input.fasta\n");
    printf("        -r      <ref.fasta>         Fasta file containing one complete genomic sequence that will be used as the reference\n");
    printf("        -p      <out.phrases>       Output file that will contain all unique phrases. Results from different heuristics will be stored in \"out.phrases_[heuristic]\"\n");
    printf("        -g      <out.gfa>         Output file that will contain the graph structure and the paths. This file will be in .gfa format\n");
    printf("        -s      <out.sources>       Output file that will contain all sources.\n");
    printf("        -c      <out.compressed>    Output file that will contain strings of phrases for each input sequence.\n");
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
                transform(line.begin(), line.end(), line.begin(), ::toupper);
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
        }
        else if (string(argv[i])=="-i"){
            Input_strings = string(argv[++i]);
        }
        else if (string(argv[i])=="-ii"){
            ref_idx = stoi(argv[++i]);
        }
        else if (string(argv[i])=="-g"){
            Output_graph_name = string(argv[++i]);
            writeGraph = true;
        }        
        else if (string(argv[i])=="-p"){
            Output_phrase_name = string(argv[++i]);
            writePhrase = true;
        }
        else if (string(argv[i])=="-s"){
            Output_source_name = string(argv[++i]);
            writeSource = true;
        }
        else if (string(argv[i])=="-c"){
            Output_compressed_name = string(argv[++i]);
            writeCompressed = true;
        }
        else if (string(argv[i])=="-n"){
            Strings_to_use = stoi(argv[++i]);
        }
        else if (string(argv[i])=="-ilp"){
            runILP = true;
        }
        else if (string(argv[i])=="-greedy"){
            runGreedy = true;
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

/**
 * @brief Calculate the number of unique phrase boundaries
 * 
 * @param rlz 
 * @return int 
 */
int numBoundaries(RLZ * rlz){
    unordered_set<int> positions;
    for(auto p : rlz->phrases){
        if (p->start > rlz->csa_rev.size() - 1){
            positions.insert(p->start);
            positions.insert(p->start+1);
            continue;
        }
        if (positions.find(p->start) == positions.end())
            positions.insert(p->start);
        if (positions.find(p->start+p->length) == positions.end())
            positions.insert(p->start+p->length);
    }
    return positions.size();
}

/**
 * @brief Calculate the number of phrases in compressed strings
 * 
 * @param rlz 
 * @return int 
 */
int numPhrases(RLZ & rlz){
    int counter = 0;
    for (auto compString : rlz.compressed_strings){
        counter += compString.size();
    }
    return counter;
}


/**
 * @brief Verify rlz factorization
 * 
 * @param rlz The rlz factorization
 * @param strings Actual strings
 * @param ref_idx the index of the reference used
 * @param id Whether the reference is provided
 */
void verify(RLZ & rlz, vector<string> strings, int ref_idx, int id){
    int j = 0;
    for (int i=0;i<strings.size();i++){
        if (id != 0 && ref_idx == i) continue;    // does not verify ref string
        cout << "Verifying " << i << endl;
        string test = rlz.decode(j);
        assert(test.compare(strings[i])==0);
        j++;
    }
}

/**
 * @brief Verify path and graph construction
 * 
 * @param paths The constructed paths
 * @param strings Actual strings
 * @param ref_idx The index of the reference used
 * @param id Whether the reference is provided
 */
void verifyPaths(vector<RLZPath*> paths, vector<string> strings, int ref_idx, int id, string & ref){
    int j = 1;

    // verify reference string separately
    string reftest = paths[0]->reconstruct();
    // reverse(reftest.begin(), reftest.end());
    // cout << reftest << endl;
    assert(reftest.compare(ref) == 0);

    for (int i=0;i<strings.size();i++){
        if (id != 0 && ref_idx == i) continue;    // does not verify ref string
        cout << "Verifying " << i << endl;
        string test = paths[j]->reconstruct();
        // cout << test << endl;
        assert(test.compare(strings[i])==0);
        j++;
    }
    cout << "Verified all paths" << endl;
}

/**
 * @brief Produce the reverse complement of input string
 * 
 * @param toreverse input string
 * @return string reverse complement
 */
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
        
        /* ---- Read Input Files -----------------------------------------------------------------------------------------*/

        string ref;
        string ref_orig;
        vector<string> strings = readFASTA(Input_strings, Strings_to_use);
        int start = 0;
        int id = 0;

        if (Input_ref!=""){
            vector<string> refv = readFASTA(Input_ref, 0);
            ref_orig = refv[0]; 
        } else {
            ref_orig = strings[ref_idx];
            id = 1;
        }
        
        /* ---- Construct Reference String -------------------------------------------------------------------------------*/

        ref = reverseComp(ref_orig) + '#' + ref_orig;

        reverse(ref.begin(), ref.end());

        // cout << "Refrence: " << ref << endl;

        cout << "Finished reading files." << endl;
        cout << "Length of reference: " << ref.length() << endl;

        /* ---- Build Reference Compressed Suffix Array ------------------------------------------------------------------*/
        auto start_ref = high_resolution_clock::now();

        RLZ rlz(ref);
        cout << "Built the initial SA" << endl;

        cout << "SA size: "<< rlz.csa.size() << endl;

        auto end_ref = high_resolution_clock::now();
        auto duration_ref = duration_cast<duration<double> >(end_ref - start_ref);
        cout << "Time (Ref Construction): " << duration_ref.count() << endl;

        /* ----- Begin RLZ Factorization (default = smallest ) ----------------------------------------------------------------------------------------*/
        // sources are not set at this stage

        auto start_factor = high_resolution_clock::now();
        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
            rlz.RLZFactor(strings[i]);
            cout << "Done for " << i << endl;
        }
        auto end_factor = high_resolution_clock::now();
        auto duration_factor = duration_cast<duration<double> >(end_factor - start_factor);
        cout << "Time (Factorization): " << duration_factor.count() << endl;

        // verify(rlz, strings, ref_idx, id);
        cout << "Boundaries (smallest): " << numBoundaries(&rlz) << endl;
        
        // rlz.print_comp_string(0);
        // if (writePhrase){
        //     string fname1 = Output_phrase_name+"_smallest";
        //     rlz.write_phrases(fname1);
        // }
        // if (writeCompressed){
        //     string fname13 = Output_compressed_name+"_smallest";
        //     rlz.write_compString(fname13);
        // }

        /* ---- Reset Phrase Boundaries (leftmost) -----------------------------------------------------------------------------------------*/

        rlz.processSources(2);
        cout << "Boundaries (leftmost): " << numBoundaries(&rlz) << endl;
        
        //output all sources
        if (writeSource){
            cout << "Writing sources to file" << endl;
            rlz.write_sources(Output_source_name);
        }

        if (!runGreedy && writePhrase){
            string fname2 = Output_phrase_name+"_leftmost";
            rlz.write_phrases(fname2);
        }

        if (!runGreedy && writeCompressed){
            string fname23 = Output_compressed_name+"_leftmost";
            rlz.write_compString(fname23);
        }

        // verify(rlz, strings, ref_idx, id);

        /* ---- Reset Phrase Boundaries (greedy) -----------------------------------------------------------------------------------------*/

        if (runGreedy){
            rlz.processSources(0);
            cout << "Boundaries (greedy): "<< numBoundaries(&rlz) << endl;

            if (writePhrase){
                string fname3 = Output_phrase_name+"_greedy";
                rlz.write_phrases(fname3);
            }

            if (writeCompressed){
                string fname33 = Output_compressed_name+"_greedy";
                rlz.write_compString(fname33);
            }
            // verify(rlz, strings, ref_idx, id);
        }

        /* ---- Reset Phrase Boiundaries (ILP) ----------------------------------------------------------------------------------------*/
        if (runILP){
            rlz.processSources(1);
            // verify(rlz, strings, ref_idx, id);
            cout << "Boundaries (ILP): " << numBoundaries(&rlz) << endl;
            
            if (writePhrase){
                string fname4 = Output_phrase_name + "_ILP";
                rlz.write_phrases(fname4);
            }
            if (writeCompressed){
                string fname43 = Output_compressed_name + "_ILP";
                rlz.write_compString(fname43);
            }
        }
        
        /* ======================================================================================================================= */
        /* ---- Construct RLZGraph -----------------------------------------------------------------------------------------*/

        cout << " ================================================= " << endl;
        cout << "    Constructing graph" << endl;

        // cout << "reference: " << extract(rlz.csa, 0, rlz.csa.size()-1) << endl;
        auto start_time = high_resolution_clock::now();
        RLZGraph g (rlz);
        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double> >(end_time - start_time);
        cout << "Time (Graph construction): " << time_span.count() << endl;

        // cout << "----- printing all edges -----" << endl;
        // g.print_edges(cout);
        // g.print_edges(cout);

        cout << "Number of nodes: " << g.get_nodeNum() << endl;
        cout << "Number of edges: " << g.get_edgeNum() << endl;
        cout << "Number of phrases: " << numPhrases(rlz) << endl;

        ofstream output_g (Output_graph_name);
        if (writeGraph){
            g.write_complete_graph(output_g);
        }

        //print rss
        struct rusage r;
        getrusage(RUSAGE_SELF, &r);

        cout << "RSS: " << r.ru_maxrss / 1024 << " MB" << endl;
            // VG vg_graph;

            
            // // create vg nodes
            // // need to increment nodeIdx by one because vg node indices start from 1
            // for(RLZNode * node : g.Nodes){
            //     vg_graph.create_node(node->seq, node->nodeIdx+1);
            // }

            // // create vg edges
            // for(RLZEdge * edge : g.Edges){
            //     vg_graph.create_edge(edge->first->nodeIdx+1, edge->second->nodeIdx+1, edge->from_start, edge->to_end);
            // }

            // // create vg paths
            // // iterate over compressed strings and map each phrase to corresponding node idx
            
            // vector<RLZPath*> paths = g.create_paths();
            // verifyPaths(paths, strings, ref_idx, id, ref_orig);

            // int pathId =0;
            // cout << "Outputing paths" << endl;

            // string fname_paths = Output_graph_name + "_paths";
            // ofstream out_paths(fname_paths);
            // for(RLZPath* p : paths){
            //     p->print(out_paths);
            // }
                // create VG::Path
                // cout << "Path" << endl;
                
                // list<NodeTraversal> newNodePath;
                // for (int idx=0; idx < p->size(); idx ++){
                    // NodeTraversal currNode (vg_graph.get_node(p->path[idx]->nodeIdx+1),p->reversed[idx]);
                    // newNodePath.push_back(currNode);

                // }

                // Path newPath = vg_graph.create_path(newNodePath);
                // newPath.set_name(to_string(pathId));

                // extend VG.paths
                // vg_graph.paths.extend(newPath, false, false);

                // pathId +=1;
            // }

        //     vg_graph.serialize_to_file(Output_graph_name, 1000);
        // }
        // cout << "reading from file" << endl;
        // {
        //     ifstream in ("data.sdsl");
        //     g.set_edges(in);
        // }

        // g.print_edges(cout);
        // g.verify();

    }
    else {
        print_help();
        exit(1);
    }
}
