#ifndef __SuffixTree_H__
#define __SuffixTree_H__

#include <iostream>
#include <vector>

using namespace std;

static int ALP_SIZE = 5;

class Node{
    public:
    vector<Node *> children; 

    Node *suffixLink; 
   
    int start; 
    int *end; 

    // position where it is created.
    int label;
   
    /*for leaf nodes, it stores the index of suffix for 
      the path  from root to leaf*/
    int suffixIndex; 
    int height;

    Node(int start, int * end, Node * root);
};

class SuffixTree{
    public:
    string text;
    Node * root = NULL;
    Node * lastNewNode = NULL;
    Node * activeNode = NULL;
    int activeEdge = -1;
    int activeLength = 0;
    
    // remainingSuffixCount tells how many suffixes yet to 
    // be added in tree 
    int remainingSuffixCount = 0; 
    int leafEnd = -1; 
    int *rootEnd = NULL; 
    int *splitEnd = NULL; 
    int size = -1; //Length of input string 

    int edgeLength(Node * currNode);
    bool walkDown(Node * currNode);
    int getIdx(char c);
    void extendSuffixTree(int pos);
    SuffixTree(string ref);
    pair<int, int> traverse(string s);

};

#endif