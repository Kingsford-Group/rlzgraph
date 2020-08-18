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

string reverseComp(string toreverse){
    string s ="";
    for (int i=0;i<toreverse.length();i++){
        s+=revCompHelper(toreverse[i]);
    }
    reverse(s.begin(), s.end());
    return s;
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

    string s = strings[0];
    string ref = s + "#" + reverseComp(s);
    csa_wt<> csa;
    construct_im(csa, ref, 1);
    for (int i = 0; i < csa.size(); i++){
        out << csa[i] << ",";
    }
    out << endl;

    // for (string & s : strings){
    //     string ref = s + "#" + reverseComp(s);
    //     csa_wt<> csa;
    //     construct_im(csa, ref, 1);
    //     for (int i = 0; i < csa.size(); i++){
    //         out << csa[i] << ",";
    //     }
    //     out << endl;
    // }    
}