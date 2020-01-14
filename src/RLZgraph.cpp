#include "RLZfact.h"
#include "RLZgraph.h"

using namespace std;

//construction
RLZgraph::RLZgraph(string ref):tree(ref){
    // tree (ref);
    this->ref = ref;
    // colors.resize(ref.length());
    // ends.resize(ref.length());
}

void RLZgraph::addString(string s){
    RLZfact newFact (ref, tree, s);
    int colorid = rlzarr.size();
    rlzarr.push_back(newFact);
    int c = 0;
    for (Phrase p : newFact.phrases){
        // for(int i=0;i<p.length;i++){
        //     colors[p.pos+i].append(backPhrase(colorid, c,false);
        // }
        auto it = phraseEnds.find(p.pos+p.length-1);
        if (it== phraseEnds.end()){
            vector<backPhrase> newVec;
            newVec.push_back(backPhrase(colorid, c));
            phraseEnds[p.pos+p.length-1] = newVec;
            if (p.pos+p.length-1 < ref.length()-1) numNodes += 1;
        } else {
            phraseEnds[p.pos+p.length-1].push_back(backPhrase(colorid, c));
        }

        it = phraseStarts.find(p.pos);
        if (it== phraseStarts.end()){
            vector<backPhrase> newVec;
            newVec.push_back(backPhrase(colorid, c));
            phraseStarts[p.pos] = newVec;

            auto itEnds = phraseEnds.find(p.pos);
            if (itEnds == phraseEnds.end()) numNodes+=1;
        } else {
            phraseStarts[p.pos].push_back(backPhrase(colorid, c));
        }
        numEdges ++;
        c++;
    }
}

vector<long int> RLZgraph::adjQuery(long int pos){
    vector<long int> neighbors;
    auto it = phraseEnds.find(pos);
    if (it != phraseEnds.end()){
        for(backPhrase bp : phraseEnds[pos]){
            // if (bp.isend)
            neighbors.push_back(rlzarr[bp.stringID].getPhrase(bp.rank+1).pos);
        }
    }
    neighbors.push_back(pos+1);

    return neighbors;
}
// void RLZgraph::construction(string ref, vector<RLZfact> factarr){
//     int breakid = 1;
//     int inputid = 1;
//     int loc = 0;
//     this->ref = ref;
//     int length = ref.length();

//     // initialize the first break at the beginning of reference
//     H[0]=0;
//     HB[0].push_back(make_pair(0,0));
//     BR = boost::dynamic_bitset<>(length);
//     BR.set(0,true);

//     long int pos_ref;
//     long int pos_input=0;
    
//     for (RLZfact fact : factarr){
//         for (int i=0;i<fact.Q.size();i++){
//             pos_ref = fact.Q[i]; 
//             pos_input = fact.B2[i];
//             // cout << pos_ref << ", " << pos_input << endl;
//             int len;
//             if (i<fact.Q.size()-1) len = fact.B2[i+1] - fact.B2[i];
//             else len = ref.size() - fact.B2[i];
//             //left break
//             // cout << "len: " << len << endl;

//             // cout << breakid << "," << inputid << "," << pos_input << endl;
//             if (H[pos_ref] == 0 && pos_ref!=0){
//                 H[pos_ref] = breakid;
//                 BR.set(pos_ref, true);
//                 HB[breakid].push_back(make_pair(0,pos_ref));
//                 breakid++;
//             }
//             HB[H[pos_ref]].push_back(make_pair(inputid, pos_input));    
            
//             //right break
//             if(H[pos_ref+len] == 0 & pos_ref+len<ref.size() ){
//                 H[pos_ref+len] = breakid;
//                 BR.set(pos_ref+len, true);
//                 HB[breakid].push_back(make_pair(0,pos_ref+len));
//                 breakid++;
//             }
//         }
//         inputid++;
//     }
// }
// RLZgraph::RLZgraph(string ref, vector<RLZfact> factarr){
//     rlzarr = factarr;
//     construction(ref, factarr);
// }

// RLZgraph::RLZgraph(string ref, vector<string> inputStrings){
//     vector<RLZfact> factarr;
//     for (string s : inputStrings){
//         factarr.push_back(RLZfact(ref, s));
//     }
//     rlzarr = factarr;
//     construction(ref, factarr);
// }

// //traversals
// int nextOne(vector<int> BR, int pos, int length){
//     int i=(length-pos)-1;
//     while(i>0){
//         if ((BR[0] & (1<<i)) == 1){
//             return length-i;
//         }
//         i--;
//     }
//     return -1;
// }

// vector<int> prefixSum(vector<int> B){
//     vector<int> ret(B.size());
//     ret[0] = 0;
//     for (int i=1;i<B.size();i++){
//         ret[i]=ret[i-1]+B[i-1];
//     }
//     return ret;
// }

// int nextOnePhrase(vector<int> prefixsum, int pos){
//     int i=0;
//     for (; i < prefixsum.size(); i++){
//         if (prefixsum[i] > pos){
//             return i-1;
//         }
//     }
//     return i;
// }


// vector<long int> RLZgraph::adjQuery(int pos){
//     // cout << "adjQuery" << endl;
//     vector<long int> neighbors;
//     long int breakid=H[pos];
//     long int ref_next = 0;
    
//     for(pair<int, int> p : HB[breakid]){
//         // if in reference
//         if (p.first == 0){
//             ref_next = BR.find_next(pos);
//             // cout << ref_next << endl;
//             neighbors.push_back(ref_next);
//         }
//         // if in input sequence
//         else{
//             long int next_phrase = rlzarr[p.first-1].Bitarray.find_next(p.second);
//             // cout << next_phrase << endl;
            
//             if (next_phrase - p.second == ref_next - pos){
//                 long int next_ref = rlzarr[p.first-1].Q[rlzarr[p.first-1].B[next_phrase]];
//                 neighbors.push_back(next_ref);
//             }
//         }
//     }
//     return neighbors;
// }

// string RLZgraph::access(int pos){
//     long int length = BR.find_next(pos)-pos;
//     return ref.substr(pos, length);
// }

// void RLZgraph::DFS(long int curr_pos, vector<bool> visited){
//     cout << curr_pos << endl;
//     vector<long int> neighbors = adjQuery(curr_pos);
//     for(long int n : neighbors){
//         cout << n << ",";
//     }
//     cout << endl;
//     cout << H[curr_pos] << endl;
//     if (neighbors.size()!=0){
//         for(long int pos : neighbors){
//             if (!visited[H[pos]]) DFS(pos, visited);
//         }
//     }
//     cout << access(curr_pos) << endl;
// }

// void RLZgraph::print_DFS(){
//     vector<bool> visited(H.size());
//     long int curr_pos =0;
//     DFS(curr_pos, visited);
// }

//updates
void RLZgraph::insertSeq(string seq){}
void RLZgraph::mergeGraph(RLZgraph graph){}
void RLZgraph::mergeFact(RLZfact fact){}