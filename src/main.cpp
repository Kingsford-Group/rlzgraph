#include <iostream>
#include <fstream>
#include <vector>
#include <map>
#include "RLZ.hpp"
#include "RLZGraph.hpp"

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
string Output_source_name="";
string Output_compressed_name="";

bool writePhrase = false;
bool writeSource = false;
bool writeCompressed = false;
bool writeGraph = false;

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

void process_mem_usage(double& vm_usage, double& resident_set)
{
   using std::ios_base;
   using std::ifstream;
   using std::string;

   vm_usage     = 0.0;
   resident_set = 0.0;

   // 'file' stat seems to give the most reliable results
   //
   ifstream stat_stream("/proc/self/stat",ios_base::in);

   // dummy vars for leading entries in stat that we don't care about
   //
   string pid, comm, state, ppid, pgrp, session, tty_nr;
   string tpgid, flags, minflt, cminflt, majflt, cmajflt;
   string utime, stime, cutime, cstime, priority, nice;
   string O, itrealvalue, starttime;

   // the two fields we want
   //
   unsigned long vsize;
   long rss;

   stat_stream >> pid >> comm >> state >> ppid >> pgrp >> session >> tty_nr
               >> tpgid >> flags >> minflt >> cminflt >> majflt >> cmajflt
               >> utime >> stime >> cutime >> cstime >> priority >> nice
               >> O >> itrealvalue >> starttime >> vsize >> rss; // don't care about the rest

   stat_stream.close();

   long page_size_kb = sysconf(_SC_PAGE_SIZE) / 1024; // in case x86-64 is configured to use 2MB pages
   vm_usage     = vsize / 1024.0;
   resident_set = rss * page_size_kb;
}

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
int numBoundaries(RLZ & rlz){
    unordered_set<int> positions;
    for(auto p : rlz.phrases){
        if (p->start > rlz.csa_rev.size() - 1){
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
    cout << "Verifying" << endl;
    for (int i=0;i<strings.size();i++){
        if (id != 0 && ref_idx == i) continue;    // does not add ref string
        string test = rlz.decode(j);
        assert(test.compare(strings[i])==0);
        j++;
    }
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
        
        /* ---- Construct Reference String -------------------------------------------------------------------------------*/

        ref = reverseComp(ref) + '#' + ref;

        reverse(ref.begin(), ref.end());

        cout << "Finished reading files." << endl;
        cout << "Length of reference: " << ref.length() << endl;

        /* ---- Build Reference Compressed Suffix Array ------------------------------------------------------------------*/

        RLZ rlz(ref);
        cout << "Built the initial SA" << endl;

        cout << "SA size: "<< rlz.csa.size() << endl;

        /* ----- Begin RLZ Factorization (default = smallest ) ----------------------------------------------------------------------------------------*/
        // sources are not set at this stage

        for(int i = 0; i<strings.size(); i++){
            if (id != 0 && i==ref_idx) continue;    // does not add ref string
            rlz.RLZFactor(strings[i]);
            // cout << "Done for " << i << endl;
        }

        verify(rlz, strings, ref_idx, id);
        cerr << numBoundaries(rlz) << "\t";
        
        // rlz.print_comp_string(0);
        if (writePhrase){
            string fname1 = Output_phrase_name+"_smallest";
            rlz.write_phrases(fname1);
        }
        if (writeCompressed){
            string fname13 = Output_compressed_name+"_smallest";
            rlz.write_compString(fname13);
        }

        
        cout << " ================================================= " << endl;
        cout << "    Constructing graph" << endl;

        // cout << "reference: " << extract(rlz.csa, 0, rlz.csa.size()-1) << endl;
        // cout << extract(rlz.csa, 0,1) << endl;
        // auto start_time = high_resolution_clock::now();
        RLZGraph g2 (rlz);

        cout << "Number of nodes (smallest): " << g2.get_nodeNum() << endl;
        cout << "Number of edges (smallest): " << g2.get_edgeNum() << endl;
        cout << "Number of phrases (smallest): " << numPhrases(rlz) << endl;

        /* ---- Reset Phrase Boundaries (leftmost) -----------------------------------------------------------------------------------------*/

        rlz.processSources(2);
        verify(rlz, strings, ref_idx, id);
        cerr << numBoundaries(rlz) << "\t";
        
        //output all sources
        if (writeSource){
            cout << "Writing sources to file" << endl;
            rlz.write_sources(Output_source_name);
        }

        if (writePhrase){
            string fname2 = Output_phrase_name+"_leftmost";
            rlz.write_phrases(fname2);
        }

        if (writeCompressed){
            string fname23 = Output_compressed_name+"_leftmost";
            rlz.write_compString(fname23);
        }

        cout << " ================================================= " << endl;
        cout << "    Constructing graph" << endl;

        // cout << "reference: " << extract(rlz.csa, 0, rlz.csa.size()-1) << endl;
        // cout << extract(rlz.csa, 0,1) << endl;
        // auto start_time = high_resolution_clock::now();
        RLZGraph g1 (rlz);

        cout << "Number of nodes (leftmost): " << g1.get_nodeNum() << endl;
        cout << "Number of edges (leftmost): " << g1.get_edgeNum() << endl;
        cout << "Number of phrases (leftmost): " << numPhrases(rlz) << endl;

        /* ---- Reset Phrase Boundaries (greedy) -----------------------------------------------------------------------------------------*/


        // rlz.processSources(0);
        // // verify(rlz, strings, ref_idx, id);
        // cerr << numBoundaries(rlz) << "\t";

        // if (writePhrase){
        //     string fname3 = Output_phrase_name+"_greedy";
        //     rlz.write_phrases(fname3);
        // }

        // if (writeCompressed){
        //     string fname33 = Output_compressed_name+"_greedy";
        //     rlz.write_compString(fname33);
        // }

        /* ---- Reset Phrase Boiundaries (ILP) ----------------------------------------------------------------------------------------*/

        rlz.processSources(1);
        verify(rlz, strings, ref_idx, id);
        cerr << numBoundaries(rlz) << "\t";
        
        if (writePhrase){
            string fname4 = Output_phrase_name + "_ILP";
            rlz.write_phrases(fname4);
        }
        if (writeCompressed){
            string fname43 = Output_compressed_name + "_ILP";
            rlz.write_compString(fname43);
        }

        /* --------------------------------------------------------*/
    
        double vm, rss;
        process_mem_usage(vm, rss);
        cout << "VM: " << vm << "; RSS: " << rss << endl;
        
        /* ======================================================================================================================= */
        /* ---- Construct RLZGraph -----------------------------------------------------------------------------------------*/

        cout << " ================================================= " << endl;
        cout << "    Constructing graph" << endl;

        // cout << "reference: " << extract(rlz.csa, 0, rlz.csa.size()-1) << endl;
        // cout << extract(rlz.csa, 0,1) << endl;
        // auto start_time = high_resolution_clock::now();
        RLZGraph g (rlz);

        cout << "Number of nodes (ILP): " << g.get_nodeNum() << endl;
        cout << "Number of edges (ILP): " << g.get_edgeNum() << endl;
        cout << "Number of phrases (ILP): " << numPhrases(rlz) << endl;

        // auto end_time = high_resolution_clock::now();
        // duration<double> time_span = duration_cast<duration<double>>(end_time - start_time);
        // cerr << time_span.count() << endl;

        // cout << "----- printing all edges -----" << endl;
        // g.print_edges(cout);



        // g.verify();

        
        // if (writeGraph)
        // {
        //     cout << "writing graph to file" << endl;
        //     ofstream out (Output_graph_name);
        //     g.write_complete_graph(out);
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
