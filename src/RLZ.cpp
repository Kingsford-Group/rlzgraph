#include "RLZ.hpp"

#include <ctime>
#include <ratio>
#include <chrono>

using namespace std::chrono;

/** 
 * @brief Construct a suffix array using the provided reference
 * */
RLZ::RLZ(string ref){

    // produce a reversed reference string
    string revRef;
    revRef.assign(ref.rbegin(), ref.rend());

    // produce a BWT for both the reversed and the forward reference string
    construct_im(csa, ref, 1);
    construct_im(csa_rev, revRef, 1);

    // record the size of the BWT
    totalLength = csa.size();
}


/**
 * @brief Process the input string into phrases
 */
int RLZ::RLZFactor(string & to_process){

    //reverse the query
    string revQuery;
    revQuery.assign(to_process.rbegin(), to_process.rend());

    // the compressed string
    vector<Phrase*> compressed;

    auto strIt = revQuery.begin();
    while (strIt != revQuery.end()){
    //     auto start = high_resolution_clock::now();

        // get one phrase per bwt query
        Phrase * p = query_bwt(strIt, revQuery.end());
        // auto end = high_resolution_clock::now();
        // auto duration = duration_cast<microseconds>(end - start);
        // cout << duration.count()<< "," << p->length << endl;
        compressed.push_back(p);
    }

    compressed_strings.push_back(compressed);
    numPhrases += compressed.size();

    return compressed.size();
}

void RLZ::processSources(int option){
    cout << "Total number of sources: " << sources.size() << endl;
    // for(auto * s : sources){
    //     transferSourceStarts(s);
    //     transferSourceEnds(s);
    // }
    
    switch (option){
        case 0: 
            cout <<"Greedy" << endl;
            optimize_phrases();
            break;
        case 1:
            cout <<"ILP" << endl; 
            optimize_phrases_ILP();
            break;
        // case 2:
        //     cout <<"smallest" << endl;
        //     reset_phrases();
        //     break;
        default:
            cout <<"leftmost" << endl;
            reset_phrases();
            break;
    }
}

/**
 * @brief Chooses one source for each phrase using the greedy approach
 */
void RLZ::optimize_phrases(){

    // stores weighted positions. weight is number of source boundaries
    vector<WeightedPos> weightedPoses(csa.size());
    // stores set of sources corresponding a particular position.
    vector< unordered_set<Source *> > posToSource(csa.size());
    // stores status of each source
    vector<bool> sourceStatus(sources.size());
    // temporarily stores each source in vector for identifying each source.
    vector<Source*> sourcesVec(sources.size());
    // stores updated phrases
    unordered_map<size_t, Phrase*> new_phrases;
    PhraseHash hasher;

    int sourceCounter = 0;

    /**
     * For each source, add count at its left and right boundaries. Avoid adding duplicated positions using a hashset.
     */
    for (auto sourceIt = sources.begin(); sourceIt != sources.end(); sourceIt ++){
        // ensure that only each position is counted once for each source.
        unordered_set<int> uniquePos;
        for (int left : (*sourceIt)->start_loc){
            int sLength = (*sourceIt)->length;
            int right = left+sLength;

            //check if a position is already updated
            auto left_it = uniquePos.find(left);
            auto right_it = uniquePos.find(right);
            
            // mark position in reference if there is a source boundary. 
            // left boundary 
            if (left_it == uniquePos.end()){
                uniquePos.insert(left);
                // create a new position object if it has not been created.
                if (weightedPoses[left].pos == -1){
                    WeightedPos leftPos {left, 1};
                    weightedPoses[left] = leftPos;
                } else {
                    weightedPoses[left].weight += 1; 
                }
            }
            
            // right boundary
            if (right_it == uniquePos.end()){
                uniquePos.insert(right);
                if (weightedPoses[right].pos == -1){
                    WeightedPos rightPos {right, 1};
                    weightedPoses[right] = rightPos;
                } else {
                    weightedPoses[right].weight += 1;
                }
            }

            // insert source pointer to position
            posToSource[left].insert((*sourceIt));
            posToSource[right].insert((*sourceIt));
        }

        // update sourceStatus
        sourceStatus[sourceCounter] = true;
        sourcesVec[sourceCounter] = (*sourceIt);

        sourceCounter ++;
    }

    // create a sorted set of weightedPoses. 
    set<WeightedPos*,ComparePosPtr> weightedPoses_Set;
    // add position pointers to the set.
    for (auto pos: weightedPoses){
        if (pos.pos > -1)
            weightedPoses_Set.insert(&pos);
    }

    // Start to greedily extract sources until all positions left have weight = 1
    // It then means that no there is no need to make choices for the rest of the phrases. 
    // The phrases that were not looked at will have the default boundaries (leftmost).
    // --set.end() returns the largest element in the set.
    while(weightedPoses.size() > 0 && (*--weightedPoses_Set.end())->weight > 0){
        
        // positions that need to be updated
        vector<int> storedPos;
    
        // remove the largest element in the set
        auto it = --weightedPoses_Set.end();
        WeightedPos currPos = **it;
        storedPos[currPos.pos]  = 1;
        weightedPoses_Set.erase(it);
        
        // for all sources at that position, reset its corresponding phrase, remove all of its other occurrences in the set
        for (auto sourceIt = posToSource[currPos.pos].begin(); sourceIt != posToSource[currPos.pos].end(); sourceIt++){
            
            // update corresponding phrase
            (*sourceIt)->p->setStart(currPos.pos);
            new_phrases[hasher(*(*sourceIt)->p)] = (*sourceIt)->p;
            
            // remove all other occurrences
            for(int loc : (*sourceIt)->start_loc){

                int left = loc;
                int right = loc + (*sourceIt)->length;

                if (storedPos[left] != 1) {
                    //remove left and right positions from set
                    weightedPoses_Set.erase(&weightedPoses[left]);
                    // indicate that this position is modified
                    storedPos[left] = 1;
                }                
                if (storedPos[right]!= 1){
                    weightedPoses_Set.erase(&weightedPoses[right]);
                    storedPos[right] = 1;
                }

                // update weight
                weightedPoses[left].weight --;
                weightedPoses[right].weight --;

                // remove the current source from posToSource
                if (left != currPos.pos) posToSource[left].erase(*sourceIt);
                if (right != currPos.pos) posToSource[right].erase(*sourceIt);
            }
        }
        posToSource[currPos.pos].clear();

        // add back the updated positions to set
        for(int pos=0; pos < storedPos.size(); pos++){
            if (storedPos[pos] == 1 && weightedPoses[pos].weight > 0){
                weightedPoses_Set.insert(&weightedPoses[pos]);
            }
        }
    }
    phrases = new_phrases;
}

/**
 * @brief split string on a character
 * 
 * @param s string to split 
 * @param c character to split on
 * @return vector<string> vector containing splited strings
 */
vector<string> split(const string & s, const char& c){
    string buff = "";
    vector<string> ret;
    for(auto n : s){
        if (n != c)
            buff += n;
        else if (n == c && buff != ""){
            ret.push_back(buff);
            buff.clear();
        }
    }
    if (buff != "")
        ret.push_back(buff);
    return ret;
}

/**
 * @brief ILP formulation for solving RLZ optimization problem
 * 
 * min sum(x_p) for all positions in reference
 * 
 * s.t.
 * sum(ys) >= 1 for all ys of one phrase
 * ys <= min(x_p1, x_p2)     s = (p1, p2-1)
 * 
 */
void RLZ::optimize_phrases_ILP(){

    try{
        GRBEnv env = GRBEnv(true);
        env.set("LogFile", "mip1.log");
        env.start();

        GRBModel model = GRBModel(env);

        GRBVar xVec [csa_rev.size()];
        vector<GRBVar> yVec;

        for(int i = 0; i < csa_rev.size(); i++){
            string varName = "x"+to_string(i);
            GRBVar x = model.addVar(0.0, 1.0, 1.0, GRB_BINARY, varName);
            xVec[i] = x;
        }

        int sourceNum = 0;
        int sourceId = 0;
        vector<Source *> sourceVec;
        for(Source * s : sources){
            sourceVec.push_back(s);
            sourceNum += s->start_loc.size();
            GRBVar ySource [s->start_loc.size()];
            double yCoff [s->start_loc.size()];
            for (int i =0; i < s->start_loc.size(); i++){
                string varName = "y_"+to_string(sourceId)+"_"+to_string(i); 

                GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                GRBVar x1 = xVec[s->start_loc[i]];
                GRBVar x2 = xVec[s->start_loc[i] + s->length];

                string constrName0 = "c"+to_string(sourceNum)+"_0";
                string constrName1 = "c"+to_string(sourceNum)+"_1";
                string constrName2 = "c"+to_string(sourceNum)+"_2";
                model.addConstr(y >= x1 + x2 - 1, constrName0);
                model.addConstr(y <= x1, constrName1);
                model.addConstr(y <= x2, constrName2);

                yVec.push_back(y);
                ySource[i] = y;
                yCoff[i] = 1;
                sourceNum+=1;
            }

            GRBLinExpr sourceExp;
            sourceExp.addTerms(yCoff, ySource, s->start_loc.size());
            string constrName = "c"+to_string(sourceId);
            model.addConstr(sourceExp, GRB_GREATER_EQUAL, 1, constrName);
            sourceId++;
        }

        model.optimize();

        cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

        unordered_map<size_t, Phrase*> new_phrases;
        PhraseHash hasher;

        for (GRBVar yvar : yVec){
            if (yvar.get(GRB_DoubleAttr_X) == 1){
                string name = yvar.get(GRB_StringAttr_VarName);
                vector<string> splitS = split(name, '_');
                Source * s = sourceVec[stoi(splitS[1])];
                s->p->setStart(s->start_loc[stoi(splitS[2])]);
                new_phrases[hasher(*(s->p))] = s->p;
            }
        }

        phrases = new_phrases;

    }catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}

void RLZ::reset_phrases(){
    PhraseHash hasher;
    unordered_map<size_t, Phrase*> new_phrases;
    for(auto * s : sources){
        Phrase * p = s->p;

        int min_loc = INT_MAX;
        for(int i : s->start_loc){
            if (i < min_loc) min_loc = i;
        }
        p->setStart(min_loc);

        new_phrases[hasher(*p)] = p;
    }
    phrases = new_phrases;
}

// void RLZ::set_phrases_leftmost(){
//     PhraseHash hasher;
//     unordered_map<size_t, Phrase*> new_phrases;
//     for(auto * s : sources){
//         Phrase * p = s->p;
//         int min = RAND_MAX;
//         int argmin = 0;
//         for(int b : s->start_loc){
//             int actual = csa_rev[b];
//             if (actual <= min){
//                 min = actual;
//                 argmin = b;
//             }
//         }
//         p->setStart(argmin);
//         auto new_got = new_phrases.find(hasher(*p));
//         new_phrases[hasher(*p)] = p;
//     }
//     phrases = new_phrases;
// }

/* check if the current character is in the reference alphabet. 
 * If not, check if it is recorded as the new character and the algorithm will not query BWT*/
Phrase* RLZ::check_alphabet(string::iterator & strIt){
    bool inAlphabet = false;

    // iterate through alphabet of the reference
    for(int i=1; i<csa.sigma; i++){
        if (csa.comp2char[i] == *strIt){
            inAlphabet = true;
            break;
        }
    }

    // see if it is already recorded as a new alphabet.
    if (!inAlphabet){
        auto find = newChar_toIdx.find(*strIt);
        int idx = 0;

        // if not found, add to the end of the newChar hashmap with the new length
        if (find == newChar_toIdx.end()){
            newChar_toIdx[*strIt] = totalLength;
            newChar_toChar[totalLength] = *strIt;
            totalLength++;
        }
        idx = newChar_toIdx[*strIt];
        auto ret_cp = create_phrase(idx, 1);
        Phrase * p = ret_cp.first;
        strIt++;
        return p;
    }

    else return 0;
}

Phrase* RLZ::query_bwt(string::iterator & strIt, string::iterator end){

    // check if the first character is in the alphabet
    Phrase * alphabetCheck = check_alphabet(strIt);
    if (alphabetCheck != 0) return alphabetCheck;

    // start BWT query
    string::iterator strStart = strIt;
    

    // stores current l and r
    size_type l = 0;
    size_type r = csa.size()-1;

    // stores final l and r
    int l_res = 0;
    int r_res = r;

    // stores matched length
    int length = 0;
    while(true){
        l_res = l;
        r_res = r;

        backward_search(csa, l, r, (*strIt), l, r);

        //found a match
        if (r+1-l > 0) length ++; 
        else break;
        
        if (strIt != end) strIt ++;
        else break;
    }

    vector<int> start_loc;
    int min_loc = INT_MAX;
    for(int i = l_res; i<=r_res; i++){
        int pos = csa[i];
        start_loc.push_back(pos);
        if (pos < min_loc) min_loc = pos;
    }

    // get the phrase pointer by either creating a new one or finding the existing one
    auto Pret = create_phrase(min_loc, length);
    Phrase * p = Pret.first;

    // create or find the corresponding source
    if (Pret.second){
        Source * source = new Source {p, start_loc, length};
        auto Sret = sources.insert(source);
    }
    
    return p;


    // // whether the last character is found or the last character is the end of the string
    // if (r < l){
    //     strIt --;
    //     length -= 2;
    // } else{
    //     l_res = l;
    //     r_res = r;
    //     length -= 1;
    // }

    // cerr << "ref: " << extract(csa, csa[l_res]-1, csa[l_res]) << endl;
    // cerr << "str: " ;
    // auto sss = strStart;
    // for(int i = 0; i < length; i++){
    //     cerr << *(sss++) ;
    // }
    // cerr << endl;

    // Find the start intervals in csa_rev
    // size_type l_start = 0;
    // size_type r_start = csa_rev.size() -1 ;
    // string::iterator strEnd = strIt;
    // do{ 
    //     strEnd -- ;
    //     backward_search(csa_rev, l_start, r_start, (*strEnd), l_start, r_start);
    // }while(strEnd != strStart);
    // pair<int, int> beg_interval = make_pair(l_start, r_start);

    // auto ret_cp= create_phrase(l_start, length);
    // Phrase * p = ret_cp.first;

    // // this is a newly inserted phrase
    // if (ret_cp.second == true){
    //     // end interval is one more LF operation
    //     vector<int> end_interval; 
    //     for (int i=0; i < csa.sigma;i++){
    //         size_type new_l = 0;
    //         size_type new_r = 0;

    //         backward_search(csa, l_res, r_res, csa.comp2char[i], new_l, new_r);

    //         if (int(new_l) <= int(new_r)){
    //             for(int i = new_l; i <= new_r; i++){
    //                 int e ;
    //                 if (csa[i] == csa.size() - 1){
    //                     e = csa_rev.isa[csa.size() - 1];
    //                 }
    //                 else{
    //                     e = csa_rev.isa[csa.size() - 2 - csa[i]];
    //                 }
    //                 end_interval.push_back(e);
    //             }
    //         }
    //     }

    //     sort(end_interval.begin(), end_interval.end());
    //     // cerr << "------- " << length << " " << (strIt != end) << endl;
    //     // cerr << "ref last: " << extract(csa, csa[l_res]-1, csa[l_res]) << endl;
    //     // cerr << "str next: " << *strIt << endl;
    //     // for (int b = beg_interval.first; b <= beg_interval.second; b++){
    //     //     cerr << extract(csa_rev, csa_rev[b]+length, csa_rev[b]+length) << " ";
    //     //     cerr << csa_rev[b]+length << ", ";
    //     // }
    //     // cerr << endl;
    //     // for (int e : end_interval){
    //     //     cerr << extract(csa_rev, csa_rev[e], csa_rev[e]) << " ";
    //     //     cerr << csa_rev[e] << ", ";
    //     // }
    //     // cerr << endl;

    //     // for(int i =0 ; i<end_interval.size(); i++){
    //     //     assert(csa_rev[end_interval[i]] == csa_rev[beg_interval.first+i]+length);
    //     // }

    //     Source * source = new Source {p, beg_interval, end_interval, length-1};
    //     auto ret = sources.insert(source);

    // }

    // return p;
}


void RLZ::print_comp_string(int stringID){
    cout << "Printing compressed strings" << endl;
    int length = 0;
    for( const Phrase* p : compressed_strings[stringID]){
        p->print(cout);
        cout << ",";
        length += p->length;
    }
    cout << endl;
    cout << "Length: " << length << endl;
}

/**
 * @brief create a phrase if it does not already exist
 * 
 * @param pos 
 * @param length 
 * @return pair<Phrase *, bool> return the pointer to the already created phrase. Boolean indicates if the phrase is newly created.
 */
pair<Phrase *, bool> RLZ::create_phrase(int pos, int length){
    Phrase * phrase = new Phrase{pos, length};

    PhraseHash hasher;
    auto phrase_hash = hasher(*phrase);
    auto ret = phrases.find(phrase_hash);

    // if the phrase is not there, add phrase to hashmap
    // otherwise, create return the already existed phrase.
    if (ret == phrases.end()){

        phrases[phrase_hash] = phrase;
        return make_pair(phrases[phrase_hash], true);
    } 
    return make_pair(ret->second, false);
}

void RLZ::print_sources(){
    cout << "Printing sources." << endl;
    for(auto s: sources){
        s->print(cout, true);
    }
}

void RLZ::print_phrases(){
    cout << "Printing phrases." << endl;
    for(auto p : phrases){
        p.second->print(cout);
    }
    cout << endl;
}

string RLZ::decode(int stringID){
    string toReturn;
    for(Phrase * p : compressed_strings[stringID]){
        if (p->start > csa.size() - 1 ){
            assert(p->length == 1);
            // p->print();
            toReturn += newChar_toChar[p->start];
        }
        else {
            toReturn += extract(csa_rev, csa_rev[p->start], csa_rev[p->start]+p->length-1);
        }
    }
    return toReturn;
}

string RLZ::decode_refCoord(int stringID){
    string toReturn;
    for(Phrase * p : compressed_strings[stringID]){
        if (p->start > csa.size() - 1 ){
            assert(p->length == 1);
            // p->print();
            toReturn += newChar_toChar[p->start];
        }
        else {
            toReturn += extract(csa_rev, p->start, p->start+p->length-1);
        }
    }
    return toReturn;
}

// void RLZ::transferSourceEnds(Source * s){
//     vector<int> new_endInterval;
//     for (int i : s->end_interval){
//         int new_end = 0;
//         // cerr << i << " --> " ;
//         if (csa[i] == csa.size()-1) {
//             new_end = i;
//         }else{
//             new_end = csa_rev.isa[csa.size() - 2 - csa[i]];
//         }
//         // cerr << new_end << endl; 
//         new_endInterval.push_back(new_end);
//     }
//     sort(new_endInterval.begin(), new_endInterval.end());
//     s->end_interval = new_endInterval;
// }

// void RLZ::transferSourceStarts(Source * s){
//     Phrase * p = s->p;
//     string sub = extract(csa, csa[p->start], csa[p->start] + p->length - 1);
//     size_type l = 0; 
//     size_type r = csa_rev.size()-1;
//     reverse(sub.begin(), sub.end());
//     backward_search(csa_rev, l, r, sub.begin(), sub.end(), l, r );   // search for the pattern in the reversed reversed BWT.
//     s->beg_interval = make_pair(l,r);
// }

void RLZ::write_phrases(string & fname){
    ofstream out (fname);
    
    auto it = sources.begin();
    for(;it!=sources.end(); it++){
        (*it)->p->print(out);
    }
    out << endl;
}

void RLZ::write_sources(string & fname){
    ofstream out (fname);

    auto it = sources.begin();
    for(;it!=sources.end(); it++){
        (*it)->print(out, true);
    }
}