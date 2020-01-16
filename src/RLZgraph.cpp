#include "RLZfact.h"
#include "RLZgraph.h"

using namespace std;

//construction
RLZgraph::RLZgraph(string ref):tree(ref){
    // tree (ref);
    this->ref = ref;
    RLZNode * source = new RLZNode(0);
    RLZNode * sink = new RLZNode(ref.length()+1);
    source->length = ref.length()+1;
    sink->length = 0;
    nodeDict.insert(make_pair(0,source));
    nodeDict.insert(make_pair(ref.length()+1,sink));
    source->next = sink;
    
    // colors.resize(ref.length());
    // ends.resize(ref.length());
}

void RLZgraph::addString(string s){
    int colorid = rlzarr.size();
    RLZfact newFact (ref, tree, s, colorid);
    rlzarr.push_back(newFact);

    int r = 0;
    for (Phrase p : newFact.phrases){
        numEdges++;
        RLZNode * currStart;
        RLZNode * currEnd;

        long int start = p.pos+1;
        long int end = p.pos+p.length+1;

        // if the node is not there yet, add the node, split its predecessor
        auto startit = nodeDict.find(start);

        if (startit == nodeDict.end()){
            auto previt = nodeDict.upper_bound(start);
            RLZNode * prevNode = previt->second;

            long int oldpos = prevNode->pos;

            prevNode->length = prevNode->length-(start-oldpos);

            prevNode->pos = start;
            previt = nodeDict.upper_bound(previt->first);

            currStart = new RLZNode(oldpos);
            numNodes ++;
            nodeDict[oldpos] = currStart;
            nodeDict[start] = prevNode;
            currStart->length = start - oldpos;

            if (previt->second->pos != ref.length()+1) 
                previt->second->next = currStart; 
            currStart->next = prevNode;
        
        }

        auto endit =nodeDict.find(end);
        auto previt = nodeDict.upper_bound(end);

        RLZNode* prevNode = previt->second;
        long int oldpos = prevNode->pos;

        // add back pointer

        if (endit == nodeDict.end()){

            prevNode->length = prevNode->length-(end-oldpos);

            prevNode->pos = end;
            previt = nodeDict.upper_bound(previt->first);
            // previt--;


            currEnd = new RLZNode(oldpos);
            numNodes ++;
            currEnd->Ends[colorid].push_back(r);
            nodeDict[oldpos] = currEnd;
            nodeDict[end] = prevNode;
            currEnd->length = end - currEnd->pos;

            currEnd->next = previt->second;

            if (previt->second->pos != ref.length()+1) 
                previt->second->next = currEnd;            
            currEnd->next = prevNode;
            cout << currEnd->next->pos << endl;
            cout << prevNode->next->pos << endl;

        } else {
            prevNode->Ends[colorid].push_back(r);

        }
        
        r++;
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

// parameters: position in ref; curr break serves as an END
vector<long int> RLZgraph::adjQuery(long int pos, long int color, long int rank){
    long int ref_pos;
    bool isEnd = false;
    Phrase curr = rlzarr[color].getPhrase(rank);
    if (pos == curr.pos+curr.length-1) isEnd = true;
    if (isEnd){
        ref_pos = rlzarr[color].getPhrase(rank+1).pos;
        rank+=1;
    }
    else {
        ref_pos = pos+ 1;
    }
    vector<long int> toRet {ref_pos, color, rank};

    return toRet;
}

vector<RLZNode*> RLZgraph::adjQuery(RLZNode * node){
    vector<RLZNode*> neighbors;
    auto it = node->Ends.begin();
    for (;it!=node->Ends.end();it++){
        for (long int rank : it->second){
            long int ref_pos = rlzarr[it->first].getPhrase(rank+1).pos;
            neighbors.push_back(nodeDict[ref_pos]);
        }
    }
    neighbors.push_back(node->next);
    return neighbors;
}

pair<RLZNode*, long int> RLZgraph::adjQuery(RLZNode * node, long int color, long int rank){
    auto colorit = node->Ends.find(color);
    bool isEnd = false;
    long int retRank = rank;
    RLZNode * toRet = NULL;
    if (colorit!=node->Ends.end()){
        for (long int r : colorit->second){
            if (r == rank){

                //check if it is the last phrase
                if (r == rlzarr[color].size()-1) return make_pair(toRet, retRank);

                toRet = nodeDict[rlzarr[color].getPhrase(rank+1).pos+1];

                isEnd = true;
                retRank+=1;
            }
        }
    }
    
    if (!isEnd) { toRet = node->next;}
    return make_pair(toRet, retRank);
}


string RLZgraph::reconstruct(long int color){
    long int rank = 0;
    Phrase curr = rlzarr[color].getPhrase(rank);
    string s = "";
    // long int pos = curr.pos;
    RLZNode* currNode = nodeDict[curr.pos];
    do {

        // s+= ref.substr(pos);
        // vector<long int> next = adjQuery(pos, color, rank);
        pair<RLZNode*, long int> next = adjQuery(currNode, color, rank);

        if (next.first == NULL) break;

        currNode = next.first;
        rank = next.second;

        s += ref.substr(currNode->pos-1, currNode->length);
        // rank = next[1];
        // pos = next[0];
        // if (color > 2)
        // cout << &(currNode)<< endl;
        // cout << &(currNode->next)<< endl;
    }while (rank <= rlzarr[color].size()-1 && currNode !=NULL);
    return s;
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
