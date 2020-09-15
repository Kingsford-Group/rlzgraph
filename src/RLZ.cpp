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

    // cout << " i SA ISA PSI LF BWT   T[SA[i]..SA[i]-1]" << endl;
    // csXprintf(cout, "%2I %2S %3s %3P %2p %3B   %:3T", csa);

    // the compressed string
    vector<Phrase*> compressed;

    auto strIt = to_process.begin();
    int length = 0;
    while (strIt != to_process.end()){
    //     auto start = high_resolution_clock::now();

        // get one phrase per bwt query
        Phrase * p = query_bwt(strIt, to_process.end());
        // auto end = high_resolution_clock::now();
        // auto duration = duration_cast<microseconds>(end - start);
        // cout << duration.count()<< "," << p->length << endl;
        length += p->length;
        // cerr << "Done for one phrase: " << p->start << "," << p->length << endl;
        // cerr << "Current length: " << length << endl;
        compressed.push_back(p);
    }

    compressed_strings.push_back(compressed);
    numPhrases += compressed.size();

    return compressed.size();
}

void RLZ::processSources(int option){
    
    // for(auto * s : sources){
    //     transferSourceStarts(s);
    //     transferSourceEnds(s);
    // }


    // if the start locations are not initialized, convert csa idx to reference index
    // find the leftmost locations on the fly.
    
    vector<int> SA (csa.size(),-1);
    
    if (!processed){
        processed = true;
        auto start_time = high_resolution_clock::now();
        cerr << totalBoundaries <<"\t";
        cout << "Processing source boundaries" << endl;
        unordered_set<Phrase*, PhrasePtrHash> min_phrases;
        PhraseHash hasher;
        int counter = 0;
        int actualBoundaries = 0;

        for(auto * s : sources){

            int min_loc = INT_MAX;
            int i = s->beg_interval.first;
            int counter_i = 0;
            if (i != -1){
                // only get the first 1000 locations to save time.
                while (i <= s->beg_interval.second && counter_i < 1000){

                    int loc;
                    if (SA[i]!=-1) loc = SA[i];
                    else loc = csa[i];
                    s->start_loc.push_back(loc);
                    if (loc < min_loc){
                        min_loc = loc;
                    }
                    i++;
                    counter_i++;
                    actualBoundaries++;
                }
            } else {
                min_loc = s->start_loc[0];
                actualBoundaries++;
            }
            s->p->start = min_loc;
            // min_phrases[hasher(*(s->p))]=(s->p);
            min_phrases.insert(s->p);

            // if (counter % 10000 == 0)
            //    cout << counter << endl;
            counter += 1;
        }
        phrases = min_phrases;

        cerr << actualBoundaries << "\t";

        auto end_time = high_resolution_clock::now();
        duration<double> time_span = duration_cast<duration<double>>(end_time - start_time);
        cerr << time_span.count() << "\t";
    }


    
    switch (option){
        case 0: {
            cout <<"Greedy" << endl;
            auto t11 = high_resolution_clock::now();
            optimize_phrases();
            auto t22 = high_resolution_clock::now();
            duration<double> time_span3 = duration_cast<duration<double>>(t22 - t11);
            cerr << time_span3.count() << "\t";

            break;
        }
        case 1:{
            cout <<"ILP" << endl; 
            auto t11 = high_resolution_clock::now();
            optimize_phrases_ILP();
            auto t22 = high_resolution_clock::now();
            duration<double> time_span3 = duration_cast<duration<double>>(t22 - t11);
            cerr << time_span3.count() << "\t";

            break;
        }
        // case 2:
        //     cout <<"smallest" << endl;
        //     reset_phrases();
        //     break;
        case 2:{
            cout <<"leftmost" << endl;
            auto t1 = high_resolution_clock::now();
            reset_phrases();
            auto t2 = high_resolution_clock::now();
            duration<double> time_span2 = duration_cast<duration<double>>(t2 - t1);
            cerr << time_span2.count() << "\t";
            break;
        }
    }
}

/**
 * @brief Chooses one source for each phrase using the greedy approach
 */
void RLZ::optimize_phrases(){

    // print_sources();

    // stores weighted positions. weight is number of source boundaries
    vector<WeightedPos> weightedPoses(totalLength+1);

    // stores set of sources corresponding a particular position.
    // the first in pair stores the source pointer
    // the second in pair indicates whether this position is a left (1) or a right (0) boundary.
    vector< unordered_set<pair<Source *, bool>, pair_hash> > posToSource(totalLength+1);
    
    // stores status of each source
    // vector<bool> sourceStatus(sources.size());
    
    // temporarily stores each source in vector for identifying each source.
    unordered_set<Source*, SourceHash> sourcesLeft(sources);
    
    // stores updated phrases
    // unordered_map<size_t, Phrase*> new_phrases;
    unordered_set<Phrase*,PhrasePtrHash> new_phrases;
    PhraseHash hasher;

    // int sourceCounter = 0;

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
                //update position count
                weightedPoses[left].pos=left;
                weightedPoses[left].weight += 1; 

                // insert source pointer to position
                posToSource[left].insert(make_pair((*sourceIt), true));

            }
            
            // right boundary
            if (right_it == uniquePos.end()){
                uniquePos.insert(right);
                //update position count
                weightedPoses[right].pos=right;
                weightedPoses[right].weight += 1;
                
                // insert source pointer to position
                posToSource[right].insert(make_pair((*sourceIt),false));
            }

        }

        // // update sourceStatus
        // sourceStatus[sourceCounter] = true;
        // sourcesVec[sourceCounter] = (*sourceIt);

        // sourceCounter ++;
    }

    // create a sorted set of weightedPoses. 
    set<WeightedPos*,ComparePosPtr> weightedPoses_Set;
    // add position pointers to the set.
    for (int i =0 ; i < weightedPoses.size();i++){
        WeightedPos * pos = &weightedPoses[i];
        if (pos->weight > 0)
            weightedPoses_Set.insert(pos);
    }

    // Start to greedily extract sources until all positions left have weight = 1
    // It then means that no there is no need to make choices for the rest of the phrases. 
    // The phrases that were not looked at will have the default boundaries (leftmost).
    // --set.end() returns the largest element in the set.
    while((weightedPoses_Set.size() > 0) && ((*--weightedPoses_Set.end())->weight > 1)){
        
        // positions that need to be updated
        vector<int> storedPos(totalLength);
    
        // remove the largest element in the set
        auto it = --weightedPoses_Set.end();
        WeightedPos currPos = **it;
        // cout << weightedPoses_Set.size() << endl;
        // cout << currPos.pos << ", " << currPos.weight << endl;
        storedPos[currPos.pos]  = 1;
        weightedPoses_Set.erase(it);
        
        // for all sources at that position, reset its corresponding phrase, remove all of its other occurrences in the set
        for (auto pairIt = posToSource[currPos.pos].begin(); pairIt != posToSource[currPos.pos].end(); pairIt++){
            
            bool indicator = pairIt->second;
            Source * currSource = pairIt->first;

            // update corresponding phrase
            if (indicator){
                currSource->p->setStart(currPos.pos);
                // new_phrases[hasher(*currSource->p)] = currSource->p;
                new_phrases.insert(currSource->p);
            } else {
                currSource->p->setStart(currPos.pos - currSource->length);
                // new_phrases[hasher(*currSource->p)] = currSource->p;
                new_phrases.insert(currSource->p);
            }

            // remove all other occurrences
            for(int loc : currSource->start_loc){

                int left = loc;
                int right = loc + currSource->length;

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
                if (left != currPos.pos) posToSource[left].erase(make_pair(currSource,true));
                if (right != currPos.pos) posToSource[right].erase(make_pair(currSource,false));
            }

            sourcesLeft.erase(currSource);
        }
        posToSource[currPos.pos].clear();

        // add back the updated positions to set
        for(int pos=0; pos < storedPos.size(); pos++){
            if (storedPos[pos] == 1 && weightedPoses[pos].weight > 0){
                weightedPoses_Set.insert(&weightedPoses[pos]);
            }
        }
    }

    // add all the phrases that has not changed
    for(auto s : sourcesLeft){
        // new_phrases[hasher(*s->p)] = s->p;
        new_phrases.insert(s->p);
    }

    phrases = new_phrases;
    cout << phrases.size() << endl;
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

        // GRBVar xVec [totalLength];

        GRBVar * xVec = new GRBVar [totalLength+1];
        double * xCoff = new double[totalLength+1];
        vector<GRBVar> yVec;

        for(int i = 0; i <= totalLength; i++){
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
            GRBVar * ySource = new GRBVar [s->start_loc.size()];
            double * yCoff  = new double [s->start_loc.size()];
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

            delete[] ySource;
            delete[] yCoff;
        }

        model.optimize();

        cout << "Obj: " << model.get(GRB_DoubleAttr_ObjVal) << endl;

        // unordered_map<size_t, Phrase*> new_phrases;
        unordered_set<Phrase*, PhrasePtrHash> new_phrases;
        PhraseHash hasher;

        unordered_set<int> source_check; 
        
        for (GRBVar yvar : yVec){
            if (yvar.get(GRB_DoubleAttr_X) == 1){
                string name = yvar.get(GRB_StringAttr_VarName);
                vector<string> splitS = split(name, '_');

                Source * s = sourceVec[stoi(splitS[1])];
                
                // check if the source has already been processed 
                auto check = source_check.find(stoi(splitS[1]));
                if (check != source_check.end()){
                    continue;
                } else {
                    source_check.insert(stoi(splitS[1]));
                }

                int left = s->start_loc[stoi(splitS[2])];
                int right = left + s->length;
                
                s->p->setStart(left);
                
                // add the updated phrase to the new phrase set
                // auto find = new_phrases.find(hasher(*(s->p)));
                // if (find == new_phrases.end()){
                //     new_phrases[hasher(*(s->p))] = s->p;
                // } else {
                //     cout << s->p->start << endl;
                // }
                new_phrases.insert(s->p);
            }
        }
        phrases = new_phrases;
        int count = 0;
        for( int i = 0; i <= totalLength; i++){
            GRBVar xvar = xVec[i];
            if (xvar.get(GRB_DoubleAttr_X) == 1){
                count ++;
            }
        }
        cout << count << endl;



    }catch(GRBException e) {
        cout << "Error code = " << e.getErrorCode() << endl;
        cout << e.getMessage() << endl;
    } catch(...) {
        cout << "Exception during optimization" << endl;
    }
}

void RLZ::reset_phrases(){
    PhraseHash hasher;
    // unordered_map<size_t, Phrase*> new_phrases;
    unordered_set<Phrase*, PhrasePtrHash> new_phrases;
    for(auto * s : sources){
        Phrase * p = s->p;

        int min_loc = INT_MAX;
        for(int i : s->start_loc){
            if (i < min_loc) min_loc = i;
        }
        p->setStart(min_loc);

        // new_phrases[hasher(*p)] = p;
        new_phrases.insert(p);
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

        vector<int> start_loc{idx};

        // create or find the corresponding source
        if (ret_cp.second){
            Source * source = new Source {p, make_pair(-1,-1), start_loc,1};
            auto Sret = sources.insert(source);
            totalBoundaries += 1; 
        }
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

        // cerr << "In bwt query loop: " << l <<", "<< r << ", "<< *strIt << endl;

        //found a match
        if (r+1-l > 0 && strIt != end){
            length ++;
            strIt ++;
        } 
        else {
            break;
        }
    }

    // assign to the leftmost
    vector<int> start_loc;
    pair<int, int> beg_interval {l_res, r_res};
    // int min_loc = INT_MAX;
    // for(int i = l_res; i<=r_res; i++){
    //     int pos = csa[i];
    //     start_loc.push_back(pos);
    //     if (pos < min_loc) min_loc = pos;
    // }

    //assign to the smallest
    int min_loc = csa[l_res];

    // get the phrase pointer by either creating a new one or finding the existing one
    auto Pret = create_phrase(min_loc, length);
    Phrase * p = Pret.first;

    // create or find the corresponding source
    if (Pret.second){
        Source * source = new Source {p, beg_interval, start_loc, length};
        auto Sret = sources.insert(source);
        totalBoundaries += source->beg_interval.second - source->beg_interval.first +1; 
    }
    
    return p;

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

    // PhraseHash hasher;
    // auto phrase_hash = hasher(*phrase);
    auto ret = phrases.find(phrase);

    // if the phrase is not there, add phrase to hashmap
    // otherwise, create return the already existed phrase.
    if (ret == phrases.end()){

        // phrases[phrase_hash] = phrase;
        phrases.insert(phrase);
        return make_pair(phrase, true);
    } 
    free(phrase);
    return make_pair(*ret, false);
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
        p->print(cout);
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
            string ext = extract(csa, p->start, p->start+p->length-1);
            reverse(ext.begin(), ext.end());
            toReturn += ext;
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
void RLZ::write_phrases(string & fname){
    ofstream out (fname);
    
    auto it = phrases.begin();
    for(;it!=phrases.end(); it++){
        (*it)->print(out);
        out << endl;
    }
}

void RLZ::write_sources(string & fname){
    ofstream out (fname);

    auto it = sources.begin();
    for(;it!=sources.end(); it++){
        (*it)->print(out, true);
    }
}

void RLZ::write_compString(string & fname){
    ofstream out (fname);

    for (auto strings: compressed_strings){
        for(auto phrase : strings){
            phrase->print(out);
        }
        out << endl;
    }
}
