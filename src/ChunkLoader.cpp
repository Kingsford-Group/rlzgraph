#include <iostream>
#include <fstream>
#include <vector>
#include <ios>
#include <chrono>
#include <assert.h>

using namespace std;
using namespace std::chrono;

enum chunkStatus{
    ENDSTR = -1,
    ENDFILE = -2,
    EXCEEDNUMBER = -3
};

void printArray(vector<char> arr){
    cout << "[";
    for (auto item : arr){
        cout << (int)item << ",";
    }
    cout << "]" << endl;
}


class ChunkLoader{
    public:
        string buf;
        int bufIdx;
        int bufSize;

        int strIdx;
        int numSeq;
        int refIdx;

        int currpos;

        ifstream inputFile;

        ChunkLoader(string fname, int numSeq, int refIdx, int bufSize):inputFile(fname),buf((size_t)bufSize, (char)ENDFILE){
            this->bufSize = bufSize;
            this->bufIdx = 0;
            this->strIdx = -1;
            this->currpos = ios_base::beg;
            this->numSeq = numSeq;
            this->refIdx = refIdx;

            // this->inputFile(fname);
            // buf(1024,-1);
        }

        char next(bool newStart){            
            string line;

            // chunk has not been loaded yet
            if ((bufIdx == 0 && buf[bufIdx] == ENDFILE) || bufIdx == bufSize){

                    // check if exceeds numSeq
                    if (strIdx == numSeq){
                        cout << "Exceeds number of sequence!" << endl;
                        return EXCEEDNUMBER;
                    }

                    currpos = inputFile.tellg();
                    getline(inputFile, line);

                    // check for the start of a new sequence
                    if (line[0] == '>'){
                        if (!newStart){
                            inputFile.seekg(currpos, ios_base::beg);                            
                            return -1;
                        } else{
                            strIdx += 1;

                            // skip reference sequence
                            while (strIdx == refIdx){
                                getline(inputFile, line);
                                if (line[0] =='>'){
                                    strIdx += 1;
                                    line.clear();
                                }
                            }
                            currpos = inputFile.tellg();
                        }
                    }

                    // if not ">", go back to the start of the line and load buffer
                    else{
                        inputFile.seekg(currpos, ios_base::beg);
                    }

                    if (currpos != -1){
                        fill(buf.begin(), buf.end(), ENDFILE);
                        inputFile.read(&buf[0], bufSize);
                    }
                    else{
                        return ENDFILE;
                    }
                    
                    bufIdx = 0; 
                    if (buf[bufIdx] == '\n'){
                        bufIdx ++;
                        return next(false);
                    } else{
                        return buf[bufIdx++];
                    }
            } 

            // if ">" is in the middle of the buffer
            else if (buf[bufIdx] == '>'){

                // let file starts at ">"
                currpos += bufIdx;
                bufIdx = bufSize;

                if (inputFile.eof())
                    inputFile.clear();
                inputFile.seekg(currpos, inputFile.beg);
                return ENDSTR;
            } 
            else{
                // skip "\n" character
                if (buf[bufIdx] == '\n'){
                    bufIdx ++;
                    return next(false);
                }
                // either returns next character of end of file enum
                return buf[bufIdx++];
            }
        }
};

int main(){
    string fname = "/home/yutongq/RLZGraph/rlzgraph/test/ecoli_O157_50_1.fasta";
    int numSeq = 50;
    int refIdx = 60;
    int bufSize = 5000;
    ChunkLoader loader(fname, numSeq, refIdx, bufSize);
    bool newStart = true; 
    int counter = 0;

    auto start = chrono::high_resolution_clock::now();
    char cc = 0;

    // string ofname_1 = "/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_1.txt";
    // string ofname_2 = "/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_2.txt";

    // ofstream f1(ofname_1);
    // ofstream f2(ofname_2);
    
    while (true){
        string s;
        int innerCounter = 0;
        char c;
        while(true){
            c = loader.next(newStart);
            if (newStart)
                newStart = false;
            if (c > 0){
                // s += c;
                innerCounter += 1;
            }
            else break;
        }

        newStart = true;
        // cout << "[" << loader.buf <<"]" << endl;
        // cout << "c " << (int)c << endl; 
        if (c == ENDFILE || c == EXCEEDNUMBER){
            cout << "status " << (int)c << endl;
            break;
        }
        // cout << "SeqID " << loader.strIdx << endl;
        // cout << loader.bufIdx << endl;
        // assert(loader.strIdx < 49);

        // cout << "currpos " << loader.currpos << endl;
        // cout << innerCounter << endl;
        // // f1 << s << endl;
        // // f1 << endl << "SeqID" << loader.strIdx << endl;
    }

    auto stop = high_resolution_clock::now();
    auto duration = duration_cast<microseconds>(stop - start);

    cout << "Time taken by chunk "<< duration.count() << " microseconds." << endl;
    

    start = high_resolution_clock::now();
    string line;
    vector<string> strings;
    ifstream input(fname);
    string currString;
    while(getline(input, line)){
        counter +=1;
        if (line[0] != '>'){
            currString += line;
            line.clear();
        } else if (currString != ""){
            strings.push_back(currString);
            // f2 << currString << endl;
            // cout << currString.length() << endl;
            currString.clear();
            line.clear();
        }
    }

    for (string s : strings){
        for (char c : s){
            counter += 1;
        }
    }
    // strings.push_back(currString);
    // f2 << currString << endl;
    // cout << currString.length() << endl;
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop-start);
    cout << "Time taken by default "<< duration.count() << " microseconds." << endl;

}