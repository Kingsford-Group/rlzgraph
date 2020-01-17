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
    nodeDict[0] = source;
    cout << "Size: " << nodeDict.size() << endl;
    nodeDict[ref.length()+1] = sink;
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

            if (previt != nodeDict.end() && previt->second->pos != ref.length()+1) 
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
            // cout << currEnd->next->pos << endl;
            // cout << prevNode->next->pos << endl;

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

string RLZgraph::access(RLZNode* node){
    return ref.substr(node->pos-1, node->length);
}


string RLZgraph::reconstruct(long int color){
    long int rank = 0;
    Phrase curr = rlzarr[color].getPhrase(rank);
    string s = "";
    // long int pos = curr.pos;
    RLZNode* currNode = nodeDict[curr.pos+1];
    do {
        pair<RLZNode*, long int> next = adjQuery(currNode, color, rank);

        if (next.first == NULL) break;

        currNode = next.first;
        rank = next.second;

        s += access(currNode);

    }while (rank <= rlzarr[color].size()-1 && currNode !=NULL);
    return s;
}

// writes in the format: <pos> <length> <color,rank>
void RLZgraph::writeGraph(string outname){
    fstream output(outname, ios::out);
    if (!output.good()){
        cerr<< "CHECK YOUR OUTPUT FILE (" << outname << ")!!" << endl;
        exit(1);
    }

    output << "POS\tLENGTH\tCOLOR,RANK" << endl;
    auto nodeit = nodeDict.end();
    while(nodeit!=nodeDict.begin()){
        nodeit--;
        output << nodeit->second->pos << "\t" << nodeit->second->length <<"\t";
        auto colorit = nodeit->second->Ends.begin();
        for (;colorit!=nodeit->second->Ends.end();colorit++){
            for(long int rank : colorit->second){
                output<< colorit->first <<","<<rank<<"\t";
            }
        }
        output << endl;
    }

    output.close();
}

//updates
void RLZgraph::insertSeq(string seq){}
void RLZgraph::mergeGraph(RLZgraph graph){}
void RLZgraph::mergeFact(RLZfact fact){}
