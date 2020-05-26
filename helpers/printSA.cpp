/**
 * @file printSA.cpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Given an input fasta, print SA of all sequences.
 * @version 0.1
 * @date 2020-05-26
 * 
 * @copyright Copyright (c) 2020
 * 
 */

#include <sdsl/suffix_arrays.hpp>
#include <iostream>
#include <fstream>
#include <algorithm>

using namespace sdsl;
using namespace std;

vector<string> readFASTA(string filename){
    fstream input(filename);
    if (!input.good()) {
        cerr << "BAD INPUT!: " << filename << endl;
        exit(1);
    }

    vector<string> strings;
    string line, content;
    int id = 0;
    while (getline(input, line)){
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

int main(int argc, char * argv[]){
    if (argc != 3){
        cout << "!!!! Usage: printSA <fastafile> <outputfile>" << endl;
        return -1;
    }
    string fastaName = argv[1];
    string outputName = argv[2];

    vector<string> strings = readFASTA(fastaName);

    ofstream out (outputName);

    for (string & s : strings){
        csa_wt<> csa;
        construct_im(csa, s, 1);
        for (int i = 0; i < csa.size(); i++){
            out << csa[i] << ",";
        }
        out << endl;
    }    
}