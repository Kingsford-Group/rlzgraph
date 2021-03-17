/**
 * @file RLZGraph.cpp
 * @author Yutong Qiu (yutongq@andrew.cmu.edu)
 * @brief Defines RLZ Graph object
 * @version 0.1
 * @date 2020-09-07
 * 
 * @copyright Copyright (c) 2020
 * 
 */

// #include "RLZ.hpp"
#include "RLZGraph.hpp"


RLZGraph:: RLZGraph(RLZ & rlz){

    this->rlz = &rlz;

    // actual length of the reference string, not including reverse complement.
    int actualRefLength = (rlz.csa.size()-2) / 2;

    // there is boundary at the end of the reference.
    bit_vector boundaries (actualRefLength + rlz.totalLength - rlz.csa.size() + 1, 0); // need to skip the "end-of-string" mark added by sdsl as we are trying to reverse the reference string
    // cout << boundaries.size() << endl;
    // each node is defined by its left boundary
    // vector<RLZNode*> Nodes;
    unordered_set<FirstPassEdge, FirstPassEdgeHash> firstPassEdges;
    // vector<RLZEdge*> Edges;

    RLZNode * source = new RLZNode{"$", -1, 1, false};
    RLZNode * sink = new RLZNode{"#", -1, 2, false};

    Nodes.push_back(source);
    Nodes.push_back(sink);
    boundaries[0] = 1;
    boundaries[actualRefLength] = 1;

    firstPassEdges.insert(FirstPassEdge{-1,0,false,false});
    firstPassEdges.insert(FirstPassEdge{actualRefLength, -2,false,false});


    /*Create edges and create segmentation*/
    for(auto compString : rlz.compressed_strings){
        int prevIdx = -1;
        Phrase * prevPh = 0;
        for (auto * ph : compString){
            // ph->print(cout);
            // cout  << endl;
            
            int left;
            int right;

            // convert refIdx to normal order (the index in phrases are of the reversed reference)
            if (ph->start < actualRefLength ){
                // cout << ph->start << "," << ph->length << "," << to_string(ph->reversed) << endl;
                right = actualRefLength-ph->start;
                left = actualRefLength-(ph->start + ph->length);
                
                // if reversed, enters from the larger idx
                if (ph->reversed){
                    int tmp = right;
                    right = left;
                    left = tmp ;
                }
            } else {
                // cout << actualRefLength <<";" << rlz.totalLength << ";" << ph->start << endl;
                assert(ph->length == 1);
                left = ph->start;
                right = ph->start + 1;
            }

            // mark the boundaries
            // cout << left <<", " << right << endl;

            boundaries[left] = 1;
            boundaries[right] = 1;
            // cout << left << endl;
            // cout << right << endl;
            
            // determine the directionality of edges
            bool from_start = false;
            bool to_end = false;
            if (ph->reversed)
                to_end = true;
            if (prevPh && prevPh->reversed)
                from_start = true;

            // connect the current break with the previous node
            firstPassEdges.insert(FirstPassEdge{prevIdx, left, from_start, to_end});

            // cout << "FirstPassEdge: " << prevIdx << "," << left << "," << from_start <<"," << to_end << endl;

            prevPh = ph;
            prevIdx = right;
        }

        // connect the last phrase with the sink, check if the last phrase is reversed
        if (prevPh && prevPh->reversed)
            firstPassEdges.insert(FirstPassEdge{prevIdx, -2, true, false}); 
        else
            firstPassEdges.insert(FirstPassEdge{prevIdx, -2, false, false});
    }

    /* 
    * Map node to reference positions and node idx
    **/

    // rank and select data structure for boundaries
    sd_vector<> compressed_bound (boundaries);
    auto rank =  sd_vector<>::rank_1_type(&compressed_bound);
    int numOnes = rank(compressed_bound.size());
    sd_vector<>:: select_1_type select_bound(&(compressed_bound));

    NodeBoundaries = compressed_bound;

    // stores reference position for this and the next node
    int currIdx = 0;
    int nextIdx = 0;

    int nodeIdx = 3;    // starting from 2 because source and sink takes 0 and 1
    for(int i=2; i<=numOnes; i++){
        nextIdx = select_bound(i);

        string currSeq;
        if (currIdx >= actualRefLength){
            currSeq = rlz.newChar_toChar[currIdx - actualRefLength + rlz.csa.size()];
        } else {
            currSeq = extract(rlz.csa, actualRefLength-1-(nextIdx-1), actualRefLength-1-currIdx);
            reverse(currSeq.begin(), currSeq.end());
        }
        // cout << "Node "<< nodeIdx << ": "<< currIdx << ";" << nextIdx << ";" << currSeq << endl;

        RLZNode * newNode = new RLZNode{currSeq, currIdx, nodeIdx, false};

        Nodes.push_back(newNode);
        idx_to_node[currIdx] = newNode;

        // add reference edges in both directions
        if (currIdx <= actualRefLength - 1 && currIdx > 0){
            firstPassEdges.insert(FirstPassEdge{currIdx, currIdx, false,false});
            firstPassEdges.insert(FirstPassEdge{currIdx, currIdx, true, true});
        }

        currIdx = nextIdx;
        nodeIdx++;

    }

    // print idx to node
    // cout << "idx_to_node: "<< endl;
    // for (auto pair : idx_to_node){
    //     cout << pair.first << ": " << pair.second->nodeIdx << endl;        
    // }   

    /* Maps first pass edges to final edges 
       For each edge (u,v), set u's index to u-1 */
    for(FirstPassEdge e : firstPassEdges){
        RLZEdge * newEdge;
        // cout << "Orig Edge: ";
        // e.print(cout);

        // if it is (source, node)
        if (e.first == -1){
            // cout << "Dealing with source" << endl;
            RLZNode * secondNode;
            int nextIdx = 0;
            if (e.second != 0)
                nextIdx = select_bound(rank(e.second));
            if (e.to_end){
                secondNode = idx_to_node[nextIdx];
            } else{
                secondNode = idx_to_node[e.second];
            }
            newEdge = new RLZEdge{source, secondNode,e.from_start, e.to_end};

        // if it is (node, sink)
        } else if (e.second == -2){
            // cout << "Dealing with sink" << endl;
            int prevIdx = 0;
            if (e.first != 0)
                prevIdx = select_bound(rank(e.first));
            RLZNode * node = idx_to_node[prevIdx];    
            newEdge = new RLZEdge{node, sink,e.from_start, e.to_end};

        // other edges
        } else {
            // cout << "Other cases" << endl;
            RLZNode * firstNode;
            RLZNode * secondNode;


            int prevIdx = 0;
            int nextIdx = 0;
            if (e.first != 0)
                prevIdx = select_bound(rank(e.first));
            if (e.second != 0)
                nextIdx = select_bound(rank(e.second));

            if (e.from_start){
                firstNode = idx_to_node[e.first];
            } else{
                firstNode = idx_to_node[prevIdx];
            }

            if (e.to_end){
                secondNode = idx_to_node[nextIdx];
            } else{
                secondNode = idx_to_node[e.second];
            }
            newEdge = new RLZEdge{firstNode, secondNode, e.from_start, e.to_end};
            // cout << "after edge: ";
            // newEdge->print(cout);
        }
        

        // cout << "New edge: ";
        // newEdge->print(cout);
        Edges.insert(newEdge);
    }

    paths = create_paths();

}

vector<RLZPath*> RLZGraph::create_paths(){
    cout << "Creating paths" << endl;
    map<Phrase*, pair<int, int> > phrase_mapping;   // maps each phrase to a range of nodes
    int actualRefLength = (rlz->csa.size()-2) / 2;

    vector<RLZPath*> createdPaths;

    /* Create reference paths */
    RLZPath * refPath = new RLZPath();
    for (RLZNode * n : Nodes){
        if (n->nodeIdx > 1 && n->refIdx <= actualRefLength - 1){
            refPath->addNode(n);
            // n->print(cout);
        }
    }
    createdPaths.push_back(refPath);

    auto rank = sd_vector<>::rank_1_type(&NodeBoundaries);
    int pathidx = 0;
    for (auto compString : rlz->compressed_strings){
        cout << "Path: " << pathidx << endl;
        pathidx++;
        RLZPath * newPath = new RLZPath();
        RLZNode * prevNode = Nodes[0];
        for (Phrase* ph : compString){
            // ph->print(cout); 
            // cout << endl;
            auto found = phrase_mapping.find(ph);
            if (found == phrase_mapping.end()){
                int left;
                int right;
            
                if (ph->start < actualRefLength ){
                    right = actualRefLength-ph->start;
                    left = actualRefLength-(ph->start + ph->length);
                } else {
                    assert(ph->length == 1);
                    left = ph->start;
                    right = left + 1;
                }

                // rank needs to plus 2 because of source and sink
                int leftNodeIdx = rank(left)+2;
                int rightNodeIdx =rank(right)-1+2;
                
                phrase_mapping[ph] = make_pair(leftNodeIdx, rightNodeIdx);
            }

            // cout << phrase_mapping[ph].first << "," << phrase_mapping[ph].second << endl;

            // add nodes to path
            int node_range_start = phrase_mapping[ph].first;
            int node_range_end = phrase_mapping[ph].second+1;
            int step = 1;
            if (ph->reversed){
                int tmp = node_range_start-1;
                node_range_start = node_range_end-1;
                node_range_end = tmp;
                step = -1;
            }
            for(int nodeIdx=node_range_start; nodeIdx!=node_range_end; nodeIdx+=step){
                RLZNode * currNode = Nodes[nodeIdx];
                if (ph->reversed){
                    // cout << "-";
                    newPath->addNode(currNode, true);
                } else{
                    newPath->addNode(currNode, false);
                }
                // currNode->print(cout);
                
                // TODO delete this in complete version
                // verify if the edge is present in edge sets
                // bool edgeExists = false;
                // for(RLZEdge* e : Edges){
                //     if (e->first->nodeIdx == prevNode->nodeIdx && e->second->nodeIdx == currNode->nodeIdx){
                //         edgeExists = true;
                //     }
                // }

                // if (!edgeExists){
                //     cout << "The following edge does not exist!" << endl;
                //     cout << "(" << prevNode->nodeIdx << "," << currNode->nodeIdx << ")" << endl;
                //     assert(1==0);
                // }
                //// End of checking existance of edge

                prevNode = currNode;

            }
        }
        createdPaths.push_back(newPath);
    }
    return createdPaths;
}

/**
 * @brief write all of the graph components onto disk
 * The file is written in GFA format
 */
void RLZGraph::write_complete_graph(ofstream & out){

    //header
    out << "H\tVN:Z:1.0" << endl;
    
    // nodes
    for (RLZNode* n : Nodes){
        out << "S\t" << n->nodeIdx << "\t" << n->seq << endl;
    }

    //paths
    /*
    int i = 0;
    for (RLZPath* p : paths){
        out << "P\t" << i << "\t";
        for(int idx=0; idx < p->path.size(); idx++){
            out << p->path[idx]->nodeIdx;
            if (p->reversed[idx] == true){
                out << "-";
            }else{
                out << "+";
            }
            if (idx!=p->path.size()-1){
                out << ",";
            }
        }
        out << endl;
    }
    */

    // edges
    for (RLZEdge* e:Edges){
        char first_sign = e->from_start ? '-' : '+';
        char second_sign = e->to_end ? '-' : '+';
        out << "L\t" << e->first->nodeIdx << "\t" << first_sign << "\t" << e->second->nodeIdx << "\t" << second_sign << "\t" << "0M" << endl;
    }

}
