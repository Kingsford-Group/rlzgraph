#include "RLZfact.h"
#include "RLZgraph.h"

using namespace std;

//construction
void RLZgraph::construction(vector<RLZfact> factarr){
    int breakid = 1;
    int inputid = 1;
    int loc = 0;
    int length = factarr[0].R.length();

    // initialize the first break at the beginning of reference
    H[0]=0;
    HB[0].push_back(make_pair(0,0));
    BR.push_back(1<<length);
    
    for (RLZfact fact : factarr){
        for (int i=0;i<fact.Q.size();i++){
            int pos = fact.Q[i];
            int len = fact.B[i];

            //left break
            if (H[pos] == 0 && pos!=0){
                H[pos] = breakid;
                BR[0] = BR[0] | (1<<(length-pos));
                HB[breakid].push_back(make_pair(0,pos));
                breakid++;
            }
            HB[H[pos]].push_back(make_pair(inputid, loc));    
            
            //right break
            if(H[pos+len] == 0){
                H[pos+len] = breakid;
                BR[0] = BR[0] | (1<<(length-(pos+len)));
                HB[breakid].push_back(make_pair(0,pos+len));
                breakid++;
            }
            loc = loc+len; 
        }
    }
}
RLZgraph::RLZgraph(vector<RLZfact> factarr){    
    construction(factarr);
}

RLZgraph::RLZgraph(string ref, vector<string> inputStrings){
    vector<RLZfact> factarr;
    for (string s : inputStrings){
        factarr.push_back(RLZfact(ref, s));
    }
    construction(factarr);
}

//traversals
int nextOne(vector<int> BR, int pos, int length){
    int i=(length-pos)-1;
    while(i>0){
        if ((BR[0] & (1<<i)) == 1){
            return length-i;
        }
        i--;
    }
    return -1;
}

vector<int> prefixSum(vector<int> B){
    vector<int> ret(B.size());
    ret[0] = 0;
    for (int i=1;i<B.size();i++){
        ret[i]=ret[i-1]+B[i-1];
    }
    return ret;
}

int nextOnePhrase(vector<int> prefixsum, int pos){
    int i=0;
    for (; i < prefixsum.size(); i++){
        if (prefixsum[i] > pos){
            return i-1;
        }
    }
    return i;
}


vector<int> RLZgraph::adjQuery(int pos){
    vector<int> neighbors;
    int breakid=H[pos];
    int ref_next = 0;
    
    for(pair<int, int> p : HB[breakid]){
        if (p.first == 0){
            ref_next = nextOne(BR, pos, rlzarr[0].R.length());
            neighbors.push_back(ref_next);
        }
        else{
            vector<int> prefixsum = prefixSum(rlzarr[p.first-1].B);
            int next = rlzarr[p.first-1].B[nextOnePhrase(prefixsum, p.second)];
            /* If the phrase is longer than the block */
            if (next-p.second == (ref_next - pos)){
                int next_ref = rlzarr[p.first-1].Q[next]; //TODO
            }
        }
    }
    
    return neighbors;
}

//updates
void RLZgraph::insertSeq(string seq){}
void RLZgraph::mergeGraph(RLZgraph graph){}
void RLZgraph::mergeFact(RLZfact fact){}