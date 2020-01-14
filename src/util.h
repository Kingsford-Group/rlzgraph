#ifndef __UTIL_H__
#define __UTIL_H__

#include <stdlib.h>
#include <iostream>
#include <fstream>
#include "RLZgraph.h"
#include "RLZfact.h"
#include "SuffixTree.h"

extern string Input_ref = "";
extern string Input_strings = "";

void print_help(){}
void print_version(){}


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
        }
        if (string(argv[i])=="-i"){
            Input_strings = string(argv[++i]);
        }
    }
    if (Input_ref == "" && Input_strings=="") return false;
    return success;
}

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
            if (!content.empty()) strings.push_back(content);
            content.clear();
        } else if (!line.empty()){
            if(line.find(' ')!=string::npos){
                content.clear();
            } else {
                content+=line;
            }
        }
    }
    cout << "Read " << id << " strings. "<< endl;
    return strings;
}

#endif
