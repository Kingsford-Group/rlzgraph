/**
 * @file RLZGraph.hpp
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

RLZGraph::RLZGraph(RLZ & rlz){

    this->rlz = &rlz;

    /* Initialize temporary adjacency matrix*/
    vector<bit_vector> tmpAdj (rlz.totalLength+2);
    for (int i=0; i<tmpAdj.size(); i++){
        tmpAdj[i] = bit_vector(rlz.totalLength+2, 0);
    }

    unordered_set<Phrase *, PhrasePtrHash> phrases;
    bit_vector boundaries (rlz.totalLength+1,0);

    // create source and sink nodes of the graph
    int sIdx = rlz.totalLength;
    int tIdx = rlz.totalLength+1;
    Phrase * s = new Phrase{sIdx,1};
    Phrase * t = new Phrase{tIdx,1};

    // set up s, t links to the beginning and ending of reference
    // tmpAdj[sIdx][0] = 1;
    int edge_added = 0;

    add_edge_const(tmpAdj, sIdx+1,0);
    edge_added++;
    // tmpAdj[tIdx][rlz.csa.size()-1] = 1;
    add_edge_const(tmpAdj, rlz.csa.size(), tIdx);
    edge_added++;

    boundaries[0] = 1;
    boundaries[rlz.csa.size()] = 1;

    // cout << "End of original reference: " << rlz.csa.size()-1 << endl;
    // cout << "End of entire reference string: " << rlz.totalLength-1 << endl;
    // cout << "idx of source: " << sIdx << "; idx of sink: " << tIdx << endl;

    // iterate through all phrases in the compressed strings
    for (auto compString : rlz.compressed_strings){
        Phrase * prev = s;
        for (Phrase * curr : compString){

            int left = curr->start;
            int right = curr->start + curr->length;
            int prevLeft = prev->start;
            int prevRight = prev->start + prev->length;
            
            /* Introduce segmentation*/
            boundaries[left] = 1;
            boundaries[right] = 1;

            /* Add edges */

            //for phrase boundaries
            // tmpAdj[prevLeft][left] = 1;  
            add_edge_const(tmpAdj, prevRight, left);
            // cout << prevRight << "," << left << endl;
            edge_added++;

            //for reference
            // tmpAdj[left-1][left] = 1;       
            // tmpAdj[right-1][right] = 1;
            // add_edge_const(tmpAdj, left-1, left);
            // add_edge_const(tmpAdj, right-1, right);

            /* add phrase */
            phrases.insert(curr); 

            prev=curr;
        }
        /*Connect to sink*/
        // tmpAdj[prev->start+prev->length][tIdx] = 1;
        add_edge_const(tmpAdj, prev->start+prev->length, tIdx);
        // cout << prev->start+prev->length << "," << tIdx << endl;

        edge_added++;
    }

    /*Record correspondance between node index and reference index*/
    this->nodeMap.resize(rlz.totalLength+2, -1);     // maps position to node
    for(int i=0; i<boundaries.size()-1; i++){
        if (boundaries[i]){
            // cout << "Node (pos, idx): " << i << "," << nodeNum << endl;
            nodeMap[i] = nodeNum;
            this->NodeToPos.push_back(i);
            nodeNum ++;
        }
    }

    // increment for source and sink
    this->NodeToPos.push_back(sIdx); //source
    this->NodeToPos.push_back(tIdx); //sink
    nodeMap[sIdx] = nodeNum++;
    nodeMap[tIdx] = nodeNum++;

    // cout << "Node num: " << nodeNum << endl;
    // cout << "NodeToPos size: " << NodeToPos.size() << endl;

    this->adjMatrix.resize(nodeNum-1); // sink will not have any outgoing edges
    cout << "=====================================================" << endl;
    /* Compress the bit vectors, preserve only positions with boundary points */
    for(int i=1; i<NodeToPos.size(); i++){
        // initialize bit vector for current node
        bit_vector b(nodeNum,0);

        // get reference to uncompressed adjmatrix
        bit_vector * bVec = &(tmpAdj[NodeToPos[i]]);

        // get number of ones via rank operation (num edges)
        int numOnes = rank_support_v<1>(bVec)(bVec->size());
        // cout << "NumOnes: " << NodeToPos[i] << ", " << numOnes << endl;

        // support structure for select operation
        bit_vector::select_1_type b_sel(bVec);
        
        //assign edge adjacencies
        for (int j = 1; j <= numOnes; j++){
            int pos = b_sel(j);
            // cout << NodeToPos[i-1] <<"," << pos << endl;
            int idx = nodeMap[pos];
            b[idx] = 1;
            // cout << NodeToPos[i] << "," << pos << endl;
            this->edgeNum++;
        }

        // take care of the reference edge
        if (i != NodeToPos.size()-1){
            if (NodeToPos[i] < rlz.csa.size()){
                b[i] = 1;
                // this->edgeNum++;
            }
        }

        // convert to succinct representation
        sd_vector<> sdb(b);
        adjMatrix[i-1] = sdb;
    }

    // cout << edge_added << " vs " << edgeNum << endl; 
}

void RLZGraph::add_edge_const(vector<bit_vector> & tmpAdj, int idx1, int idx2){
    // this->edgeNum ++;
    tmpAdj[idx1][idx2] =1;
    // cout << "Add Edge (" << idx1 << "," << idx2 << ")" << endl;
}

void RLZGraph::set_edges(ifstream & in){
    int counter = 0;
    while (in.good()){
        bit_vector b;
        load(b, in);

        sd_vector<> bb (b);
        adjMatrix[counter] = bb;
        counter ++;

        auto rank =  sd_vector<>::rank_1_type(&bb);
        int numOnes = rank(bb.size());
        this->edgeNum += numOnes;
    }
    this->nodeNum = counter + 1;
}

void RLZGraph::print_edges(ostream & out){
    for(int i=0; i<adjMatrix.size() ; i++){

        // prints the bit vector stored
        // out << adjMatrix[i] << endl;

        // count number of ones for select operations
        auto rank =  sd_vector<>::rank_1_type(&(adjMatrix[i]));
        int numOnes = rank(adjMatrix[i].size());

        sd_vector<>:: select_1_type select_adj(&(adjMatrix[i]));
        for(int j=1; j<=numOnes;j++){
            int idx = select_adj(j);
            out << "Edge idx: (" << i << "," << idx << ")" <<  endl;
            // cout << "Edge pos: (" << NodeToPos[i] << "," << NodeToPos[idx] << ")" <<  endl;
        }
    }
}

void RLZGraph::verify(){
    // verify reference
    for (int i = 0; i<nodeMap[rlz->csa.size()]-1; i++){
        assert(adjMatrix[i][i+1] == 1);
    }
    // verify reference source
    assert(adjMatrix[adjMatrix.size()-1][0] == 1);

    // verify compressed strings
    for (auto compString : rlz->compressed_strings){
        // verify source
        Phrase * fp = compString[0];
        assert(adjMatrix[adjMatrix.size()-1][nodeMap[fp->start]] == 1);

        for (int i = 0; i < compString.size()-1; i++){
            Phrase * p1 = compString[i];
            Phrase * p2 = compString[i+1];
            assert(adjMatrix[nodeMap[p1->start+p1->length]-1][nodeMap[p2->start]] == 1);
        }
    }
}

void RLZGraph::write_complete_graph(ofstream & out){
    for (auto vect : adjMatrix){
        serialize(vect, out);
    }
}