#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "RLZgraph.h"
#include "RLZfact.h"
#include "SuffixTree.h"

// extern string Input_ref = "";
// extern string Input_strings = "";

void print_help(){
    printf("--------------------------------------------------------------------------------------------\n");
    printf("|  USAGE: rlzgraph -r <ref.fa> -i <input.fa> -g <output_graph_name> -p <output_phrase_name> |\n");
    printf("--------------------------------------------------------------------------------------------\n");
    printf("-i is required. If reference fasta is missing, the first sequence of the input fasta file will be used as reference.\n");
}
void print_version(){}



vector<string> readFASTA(string filename){
    fstream input(filename);
    if (!input.good()) {
        cerr << "BAD INPUT!: " << filename << endl;
        exit(1);
    }

    vector<string> strings;
    string line, content;
    int id = 0;
    while (getline(input, line).good()){
        if (line[0] == '>' ){
            id++;
            if (!content.empty()){
                strings.push_back(content);
                content.clear();
            }
        } else if (!line.empty()){
            if(line.find(' ')!=string::npos){
                content.clear();
            } else {
                content+=line;
            }
        }
    } 
    if (!content.empty()){
        strings.push_back(content);
        content.clear();
    }
    cout << "Read " << id << " strings. "<< endl;
    return strings;
}

#endif
