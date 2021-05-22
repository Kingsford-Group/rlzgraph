#include <iostream>
#include <fstream>
#include <vector>
#include <ios>

using namespace std;

class ChunkLoader{
    public:
        vector<char> buf;
        int bufIdx;

        int strIdx;
        int numSeq;
        int refIdx;
        
        ifstream inputFile;

        ChunkLoader(string fname, int numSeq, int refIdx):inputFile(fname),buf(4,-1){
            this->bufIdx = 0;
            this->strIdx = 0;
            this->numSeq = numSeq;
            this->refIdx = refIdx;

            // this->inputFile(fname);
            // buf(1024,-1);
        }

        char next(bool newStart){
            
            string line;
            // chunk has not been loaded yet
            if ((bufIdx == 0 && buf[bufIdx] == -1) || bufIdx == buf.size()){
                while (!inputFile.eof()){                    

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

                    int currpos = inputFile.tellg();
                    getline(inputFile, line);

                    // read a line and see if it is a '>'
                    // if so -- increment strIdx and return -1
                    if (line[0] == '>'){
                        strIdx += 1;
                        if (!newStart)
                            return -1;
                    }
                    // else -- go back to the start of the line and load buffer
                    else{
                        inputFile.seekg(currpos, ios_base::beg);
                        inputFile.read(buf.data(), buf.size());
                        bufIdx = 0;
                        return buf[bufIdx++];
                    }
                }
            } 
            else if (buf[bufIdx] == '>'){
                strIdx += 1;
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
    string fname = "/home/yutongq/RLZGraph/rlzgraph/test/simple1.fasta";
    ChunkLoader loader(fname, 3, 1);
    bool newStart = true; 
    for (int i=0; i<numSeq; i++){
        while(true){
            char c = loader.next(newStart);
            if (newStart)
                newStart = false;
            if (c != -1)
                cout << c;
            else break;
        }
        cout << endl;
        cout << "END" << endl;
    }


    string line;
    ifstream input(fname);
    while(getline(input, line)){
        cout << line << endl;
    }
}