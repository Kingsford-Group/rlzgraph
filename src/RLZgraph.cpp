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
            prevNode->Starts[colorid].push_back(r);
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
        auto rankit = node->Ends.find(rank);
        if (rankit != node->Ends.end()){
            //check if it is the last phrase
            if (rankit->first == rlzarr[color].size()-1) return make_pair(toRet, retRank);

            toRet = nodeDict[rlzarr[color].getPhrase(rank+1).pos+1];

            isEnd = true;
            retRank+=1;
        }
        // for (long int r : colorit->second){
        //     if (r == rank){

        //         //check if it is the last phrase
        //         if (r == rlzarr[color].size()-1) return make_pair(toRet, retRank);

        //         toRet = nodeDict[rlzarr[color].getPhrase(rank+1).pos+1];

        //         isEnd = true;
        //         retRank+=1;
        //     }
        // }
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


vector<RLZNode *> RLZgraph::superpath(RLZNode* curr, long int rank, long int color){
    vector<RLZNode *> path;
    
    return path;
}

vector<Bubble> RLZgraph::findAllBubbles(){
    vector<Bubble> bubbles;
    RLZNode * node = nodeDict.find(0)->second;
    vector<RLZNode*> neighbors;
    while (node != NULL){
        neighbors = adjQuery(node);
        if (neighbors.size()> 1){
            RLZNode * nextRef = neighbors[neighbors.size()-1];
            vector<RLZNode *> refPath = superpath(nextRef, nextRef->pos, -1);
            for (int i=0; i<neighbors.size()-1; i++){
                for (auto colorit = neighbors[i]->Starts.begin(); colorit!=neighbors[i]->Starts.end();colorit++){
                    for (long int rank : colorit->second){
                        vector<RLZNode *> otherPath = superpath(neighbors[i], rank, colorit->first);
                        Bubble b(nextRef, neighbors[i], refPath, otherPath);
                        bubbles.push_back(b);
                    }
                }
            }
        }
        node = node->next;
    }
    return bubbles;
}

vector<Bubble> RLZgraph::findBubbles(int color){
    vector<Bubble> bubbles;

    return bubbles;
}

// bubble between two colors. 
vector<Bubble> RLZgraph::findBubbles(int color1, int color2){

    if (color2 == -1) 
        return findBubbles(color1);
    if (color1 == -1)
        return findBubbles(color2);

    vector<Bubble> bubbles;
    RLZNode * node = nodeDict.find(0)->second;

    long int rank = 0;

    while (node != NULL){
        // pair(node, rank)
        auto next1 = adjQuery(node, color1, rank); 
        auto next2 = adjQuery(node, color2, rank);

        if (next1.first == NULL || next2.first == NULL) break;

        if (next1.first!= next2.first){
            Bubble b;
            vector<RLZNode *> nextPath1 = superpath(next1.first, next1.second, color1);
            vector<RLZNode *> nextPath2 = superpath(next2.first, next2.second, color2);
            if (nextPath1[nextPath1.size()-1] == nextPath2[nextPath2.size()-1]) {
                b.begin = node;
                b.end = nextPath1[nextPath1.size()-1];
                b.firstPath = nextPath1;
                b.secondPath = nextPath2;
            } else {
                cerr << "Bubble finding: the ends are not the same!" << endl;
                exit(1);
            }
            bubbles.push_back(b);
        }
    }
    return bubbles;
}

//updates
void RLZgraph::insertSeq(string seq){}
void RLZgraph::mergeGraph(RLZgraph graph){}
void RLZgraph::mergeFact(RLZfact fact){}
