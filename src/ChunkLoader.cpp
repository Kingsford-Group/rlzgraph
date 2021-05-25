#include <iostream>
#include <fstream>
#include <vector>
#include <ios>
#include <chrono>

using namespace std;
using namespace std::chrono;


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
        
        int currStr;
        int currpos;

        ifstream inputFile;

        ChunkLoader(string fname, int numSeq, int refIdx, int bufSize):inputFile(fname),buf((size_t)bufSize, (char)-1){
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
            // cout << "In next currpos0 " << currpos << endl;

            // chunk has not been loaded yet
            if ((bufIdx == 0 && buf[bufIdx] == -1) || bufIdx == bufSize){
                // while (!inputFile.eof()){                    

                    // skip all the content for the reference string
                    while (strIdx == refIdx){
                        getline(inputFile, line);
                        if (line[0]=='>'){
                            strIdx += 1;
                            line.clear();
                        }
                    }

                    // check if exceeds numSeq
                    if (strIdx == numSeq){
                        return -1;
                    }

                    currpos = inputFile.tellg();
                    getline(inputFile, line);
                    // cout << "In next currpos1 " << currpos << endl;


                    // read a line and see if it is a '>'
                    // if so -- increment strIdx and return -1
                    if (line[0] == '>'){
                        if (!newStart){
                            inputFile.seekg(currpos, ios_base::beg);                            
                            return -1;
                        } else{
                            strIdx += 1;
                            currpos = inputFile.tellg();
                        }
                    }
                    // else -- go back to the start of the line and load buffer
                    else{
                        inputFile.seekg(currpos, ios_base::beg);
                    }
                    // cout << "In next currpos2 " << currpos << endl;

                    if (currpos != -1){
                        fill(buf.begin(), buf.end(), -1);
                        inputFile.read(&buf[0], bufSize);
                    }
                    else{
                        return -1;
                    }
                    
                    bufIdx = 0; 
                    if (buf[bufIdx] == '\n'){
                        bufIdx ++;
                        return next(false);
                    } else{
                        return buf[bufIdx++];
                    }
                    
                // }
            } 
            else if (buf[bufIdx] == '>'){
                // strIdx += 1;
                currpos += bufIdx;
                // cout << "In next currpos3 " << currpos << endl;
                // cout << "In next currpos4 " << inputFile.tellg() << endl;
                // cout << "EOF? "<< inputFile.eof() << endl;
                bufIdx = bufSize;
                if (inputFile.eof())
                    inputFile.clear();
                inputFile.seekg(currpos, inputFile.beg);
                // cout << "In next currpos5 " << inputFile.tellg() << endl;

                return -1;
            } 
            else{
                if (buf[bufIdx] == '\n'){
                    bufIdx ++;
                    return next(false);
                }
                return buf[bufIdx++];
            }
        }
};

int main(){
    string fname = "/home/yutongq/RLZGraph/rlzgraph/test/ecoli_O157_5_1.fasta";
    int numSeq = 5;
    int refIdx = 6;
    int bufSize = 500;
    ChunkLoader loader(fname, numSeq, refIdx, bufSize);
    bool newStart = true; 
    int counter = 0;

    auto start = chrono::high_resolution_clock::now();
    char cc = 0;

    string ofname_1 = "/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_1.txt";
    string ofname_2 = "/home/yutongq/RLZGraph/rlzgraph/test/chunk_test_2.txt";

    ofstream f1(ofname_1);
    ofstream f2(ofname_2);
    
    for (int i=0; i<numSeq; i++){
        string s;
        while(true){
            char c = loader.next(newStart);
            // cout << "[" << loader.buf <<"]" << endl;
            // cout << "Position: " << loader.currpos << endl;
            if (newStart)
                newStart = false;
            if (c != -1)
                s += c;
                // counter += 1;
            else break;
            // counter += 1;
            // if (counter > 200){
            //     cout << "Infinite loop??" << endl;
            //     break;
            // }
        }
        f1 << s << endl;
        // f1 << endl << "SeqID" << loader.strIdx << endl;
        newStart = true;
        cout << "SeqID " << loader.strIdx << endl;
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
        if (line[0] != '>'){
            currString += line;
            line.clear();
        } else if (currString != ""){
            strings.push_back(currString);
            f2 << currString << endl;
            cout << currString.length() << endl;
            currString.clear();
            line.clear();
        }
    }
    strings.push_back(currString);
    f2 << currString << endl;
    cout << currString.length() << endl;
    stop = high_resolution_clock::now();
    duration = duration_cast<microseconds>(stop-start);
    cout << "Time taken by default "<< duration.count() << " microseconds." << endl;

}