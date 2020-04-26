#include "SuffixTree.h"

using namespace std; 

Node::Node (int start, int *end, Node * root) { 

        this->children.resize(ALP_SIZE);
        // cerr << "New Node: " << start << "," << *end <<endl;
        int i; 
        for (i = 0; i < ALP_SIZE; i++) 
            this->children[i] = NULL; 
    
        /*For root node, suffixLink will be set to NULL 
        For internal nodes, suffixLink will be set to root 
        by default in  current extension and may change in 
        next extension*/
        this->suffixLink = root; 
        this->start = start; 
        this->end = end; 

        this->label = start;
    
        /*suffixIndex will be set to -1 by default and 
        actual suffix index will be set later for leaves 
        at the end of all phases*/
        this->suffixIndex = -1; 

        this->height= 0;
    } 

int SuffixTree::edgeLength(Node * currNode){
    if (currNode == root) return 0;
    else return *(currNode->end)- currNode->start + 1;
}

bool SuffixTree::walkDown(Node * currNode){
    if (activeLength >= edgeLength(currNode)) { 
        activeEdge += edgeLength(currNode); 
        activeLength -= edgeLength(currNode); 
        activeNode = currNode; 
        return true; 
    } 
    return false; 
}

int SuffixTree::getIdx(char c){
    switch (c){
        case 'A': return 0;
        case 'C':return 1;
        case 'G': return 2;
        case 'T': return 3;
        case 'N': return 4;
        case 'M': return 5;
        case 'Y': return 6;
        case 'S': return 7;
        case 'R': return 8;
        case 'K': return 9;
        case 'B': return 10;
        case 'D': return 11;
        case 'H': return 12;
        case 'V': return 13;
        case 'U': return 14;
        case 'W': return 15;
        case '$': return 16;
    }
   cerr << "Unrecognized Character: " << c << endl;
   exit(1);
}

void SuffixTree::extendSuffixTree(int pos, string & text){
    // cout << "============= Pos: " << pos << endl;
    // cout << "ActiveNode: " << activeNode << endl;
    leafEnd = pos;
    remainingSuffixCount++;
    lastNewNode=NULL;
    int walkdown = 0;

    // cout << "HERE: " << text<< endl;

    while(remainingSuffixCount>0){
        if (activeLength==0) activeEdge = pos;

        // if (pos == 319){
        //     cout << "=======================" << endl;
        //     cout << walkdown <<endl;
        //     cout << activeLength << endl;
        //     cout << edgeLength(activeNode) << endl;
        // }

        // if the next character is not in tree yet, creates new node
        // cout << (activeNode ->children[getIdx(text[activeEdge])]==NULL)<<endl;
        // cout << "ActiveEdge: " << activeEdge << endl;
        if (activeNode->children[getIdx(text[activeEdge])] == NULL){
            // cout << "case 1" << endl;
            // cout << "Index: " << getIdx(text[activeEdge]) << endl;
            activeNode->children[getIdx(text[activeEdge])] = new Node(pos, &leafEnd, root);
            activeNode->children[getIdx(text[activeEdge])]->label = pos - activeNode->height;
            activeNode->children[getIdx(text[activeEdge])]->height = activeNode->height + edgeLength(activeNode->children[getIdx(text[activeEdge])]);
            // cout << activeNode ->children[getIdx(text[activeEdge])]->start<<endl;
            if (lastNewNode != NULL){
                lastNewNode->suffixLink = activeNode;
                lastNewNode = NULL;
            }
        }
        else{
            Node * next = activeNode->children[getIdx(text[activeEdge])];
            if (walkDown(next)) {
                walkdown += edgeLength(activeNode);
                continue;
            }

            // current character in tree
            // cout << "pos: " << pos << endl;
            if (text[next->start + activeLength] == text[pos]){
                // cout << "case 2" << endl;
                if (lastNewNode != NULL && activeNode != root){
                    lastNewNode->suffixLink = activeNode;
                    lastNewNode = NULL;
                }

                activeLength ++;
                break;
            }

            // cout << "case 3" << endl;

            // current character not in tree (needs to extend)
            splitEnd = (int*) malloc(sizeof(int)); 
            *splitEnd = next->start + activeLength - 1; 

            // the common path
            Node *split = new Node(next->start, splitEnd, root); 
            split->label = next->label;
            activeNode->children[getIdx(text[activeEdge])] = split;
            split->height = activeNode->height + edgeLength(split);

            // Corresponds to the newly added branch
            split->children[getIdx(text[pos])] = new Node(pos, &leafEnd, root);
            split->children[getIdx(text[pos])]->label = pos - split->height;
            split->children[getIdx(text[pos])]->height = split->height + edgeLength(split->children[getIdx(text[pos])]);
            
            next->start += activeLength;
            // cout << "Next->start: " << next->start << endl;
            split->children[getIdx(text[next->start])] = next;

            if (lastNewNode != NULL){
                lastNewNode -> suffixLink = split;
            }
            lastNewNode = split;
            
        }

        remainingSuffixCount--;
        if (activeNode==root && activeLength>0){
            activeLength--;
            activeEdge = pos - remainingSuffixCount + 1;
        } else if (activeNode!=root){
            activeNode = activeNode -> suffixLink;
        }
    }

}

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


SuffixTree::SuffixTree(string ref){
    this->ref = ref;
    // string reversed = ref.substr(0, ref.length());
    // reversed = reverseComp(reversed);
    // this->ref+="$";
    // this->ref+= reversed;
    size = this->ref.length();

    int i;
    int rootEnd1 = -1;
    rootEnd = &rootEnd1;
    root = new Node(-1, rootEnd,0);

    activeNode = root;
    for (i=0;i<size;i++){
        // cout << i << " " << this->ref[i] << endl;  
        // cout << this->ref.length() << endl; 
        extendSuffixTree(i, this->ref);
    }
    // cout << "forward" << endl;

    // add the reversed string
    // for (i=size-1; i>=0; i--) {
    // cout << i << endl;
    //     extendSuffixTree(i);
    // }
}

void SuffixTree::addString(string & text){
    for (int i = 0; i<text.length(); i++){
        extendSuffixTree(i, text);
    }
}

// go as far as possible in the tree.
pair<int, int> SuffixTree::traverse(string s){
    // cout << "Traverse" <<endl;
    int i = 0;
    Node * next = root;
    int length = 0;
    int inEdge = 0;
    int start = -1;

    if (root->children[getIdx(s[0])] == NULL){
        int oldsize = size;
        int newsize = size+1;
        root->children[getIdx(s[0])] = new Node(size, &(newsize), root);
        size = size+1;
        ref += s[0];
        cout << "Ref has been changed! Added character: " << s[0] << endl;
        return make_pair(oldsize, 1);
    }

    while (i < s.length()){
        // cout << "Curr Node: " << next->label << endl;
        // for (int i=0;i<ALP_SIZE;i++){
            // cout << next->children[i] << ", ";
        // }
        // cout << endl;

        // printf("InEdge: %u, Length: %u\n", inEdge,length);
        //cout <<next->start + inEdge <<endl;
        // cout << next->start << "," << *(next->end) << "," << next->label<< endl;
        // cout << s[i] << endl;

        if (inEdge == length){
            if (next->children[getIdx(s[i])]!=NULL){
                inEdge = 1;
                next = next->children[getIdx(s[i])];
                start = next->label;
                length = edgeLength(next);
            }
            else return make_pair(start, i);
        }
        else {
            if (s[i] == ref[next->start + inEdge]){
                inEdge++;
                // length--;
            } else {
                return make_pair(start, i);
            }
            
        }
        i++;
    }
    return make_pair(start,s.length()); //no match
}

