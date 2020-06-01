#include "RLZ.hpp"

#include <ctime>
#include <ratio>
#include <chrono>

using namespace std::chrono;

/* Construct a suffix array using the provided reference*/
RLZ::RLZ(string ref){
    string revRef;
    revRef.assign(ref.rbegin(), ref.rend());

    construct_im(csa, ref, 1);
    construct_im(csa_rev, revRef, 1);

    totalLength = csa.size();
}

int RLZ::RLZFactor(string & to_process){

    vector<Phrase*> string1;
    auto strIt = to_process.begin();
    while (strIt != to_process.end()){
        auto start = high_resolution_clock::now();
        Phrase * p = query_bwt(strIt, to_process.end());
        auto end = high_resolution_clock::now();
        auto duration = duration_cast<microseconds>(end - start);
        // cout << duration.count()<< "," << p->length << endl;
        string1.push_back(p);
    }

    compressed_strings.push_back(string1);
    numPhrases+=string1.size();

    return string1.size();
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
        case 2:
            cout <<"smallest" << endl;
            reset_phrases();
            break;
        default:
            cout <<"leftmost" << endl;
            set_phrases_leftmost();
            break;
    }
}


void RLZ::optimize_phrases(){

    Phrase * oldPointer;
    
    vector<int> bp_count (csa_rev.size());
    // multimap<int, int> bp_count_sorted;
    vector<vector<Source*> > bp_pointer(csa_rev.size());

    unordered_set<Source*> phrase_mark;
    unordered_map<size_t, Phrase*> new_phrases;

    // mark the break points
    for(auto sourceIt = sources.begin(); sourceIt != sources.end(); sourceIt ++){
        for(int b = (*sourceIt)->beg_interval.first; b <= (*sourceIt)->beg_interval.second; b++){
            Source * ss = *sourceIt;
            bp_count[b] += 1;
            bp_pointer[b].push_back(ss);
        }
        for (int e : (*sourceIt)->end_interval){
            if (e < (*sourceIt)->beg_interval.first || e > (*sourceIt)->beg_interval.second){
                Source * ss = *sourceIt;
                bp_count[e] +=1;
                bp_pointer[e].push_back(ss);
            }
        }
    }

    // cerr << "Here" << endl;
    vector<int> bp_count_copy = bp_count;

    // iteratively find all sources
    auto max = max_element(bp_count.begin(), bp_count.end());
    int argmax = distance(bp_count.begin(), max);
    
    PhraseHash hasher;

    do{ 
        // reset start for all phrases at argmax
        for(int it = 0; it < bp_pointer[argmax].size(); it ++){
            Source * s = bp_pointer[argmax][it];
            auto ret = phrase_mark.find(s);
            
            if (ret == phrase_mark.end()){
                //TODO a bp could be a beginning or an ending of the same phrase. Maybe need a better way to distinguish them. For now it does not seem to affect the approximation ratio.
                // rank of argmax in this source's end.
                int rank = 0;
                int end_pos = 0;
                int start_pos = 0;

                int deleted = 0;

                // if bp hits a beginning.
                if (argmax >= s->beg_interval.first && argmax <= s->beg_interval.second){
                    // cerr << "begin" << endl;
                    for(int b = s->beg_interval.first; b <= s->beg_interval.second; b++){
                        // cerr << b << endl;
                        bp_count[b] -= 1;
                        deleted+=1;
                    }
                    start_pos = argmax;
                    rank = argmax - s->beg_interval.first;
                    end_pos = s->end_interval[rank];

                    // remove the other ends of other sources
                    for(int i = 0; i < s->end_interval.size(); i++){
                        int e = s->end_interval[i];
                        if (e < s->beg_interval.first || e > s->beg_interval.second){
                            if (e != end_pos){ 
                                bp_count[e] -= 1;
                                deleted+=1;
                            } 
                        }else if (e == end_pos){
                            bp_count[e] +=1;
                            deleted-=1;
                        }
                    }
                } 
                // it bp hits an end.
                else {
                    end_pos = argmax;
                    for(int i = 0; i < s->end_interval.size(); i++){
                        int e = s->end_interval[i];
                        if (e < s->beg_interval.first || e > s->beg_interval.second){
                            bp_count[e] -= 1;
                            deleted+=1;
                        }
                        if (e == argmax){
                            rank = i;
                        }
                    }
                    start_pos = s->beg_interval.first + rank;

                    // remove the other ends of other sources
                    for(int b =s->beg_interval.first ; b<=s->beg_interval.second; b++){
                        if (b != start_pos){
                            bp_count[b] -= 1;
                            deleted+=1;
                        }
                    }
                }
                
                int total_num = 0;
                total_num += s->beg_interval.second + 1 - s->beg_interval.first;
                for(int e : s->end_interval){
                    if (e < s->beg_interval.first || e > s->beg_interval.second){
                        total_num+=1;
                    }
                }

                s->p->setStart(start_pos);
                new_phrases[hasher(*(s->p))] = s->p;
                phrase_mark.insert(s);

                int r = start_pos - s->beg_interval.first;
                assert(s->end_interval[r] == argmax || start_pos == argmax);
            } else {
                int r = s->p->start - s->beg_interval.first;
                if(s->end_interval[r] == argmax || s->p->start == argmax){
                    bp_count[argmax] -= 1;
                }
            }
        } 
        assert(bp_count[argmax] == 0);

        max = max_element(bp_count.begin(), bp_count.end());
        argmax = distance(bp_count.begin(), max);
    }while(*max > 1);

    // cerr << "Here" << endl;

    while (new_phrases.size() < sources.size()){
        for (auto * s : sources){
            auto ret = phrase_mark.find(s);
            if (ret == phrase_mark.end()){
                s->p->setStart(s->beg_interval.first);
                new_phrases[hasher(*(s->p))] =s->p;
            }
        }
    }

    // cerr << "done" << endl;

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
        int j = 0;
        vector<Source *> sourceVec;
        for(Source * s : sources){
            sourceVec.push_back(s);
            sourceNum += s->end_interval.size();
            GRBVar ySource [s->end_interval.size()];
            double yCoff [s->end_interval.size()];
            for (int i =0; i < s->end_interval.size(); i++){
                string varName = "y_"+to_string(j)+"_"+to_string(i); 

                GRBVar y = model.addVar(0.0, 1.0, 0.0, GRB_BINARY, varName);
                GRBVar x1 = xVec[s->beg_interval.first+i];
                GRBVar x2 = xVec[s->end_interval[i]];

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
            sourceExp.addTerms(yCoff, ySource, s->end_interval.size());
            string constrName = "c"+to_string(j);
            model.addConstr(sourceExp, GRB_GREATER_EQUAL, 1, constrName);
            j++;
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
                s->p->setStart(s->beg_interval.first + stoi(splitS[2]));
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
        p->setStart(s->beg_interval.first);
        auto new_got = new_phrases.find(hasher(*p));
        new_phrases[hasher(*p)] = p;
    }
    phrases = new_phrases;
}

void RLZ::set_phrases_leftmost(){
    PhraseHash hasher;
    unordered_map<size_t, Phrase*> new_phrases;
    for(auto * s : sources){
        Phrase * p = s->p;
        int min = RAND_MAX;
        int argmin = 0;
        for(int b = s->beg_interval.first ; b <= s->beg_interval.second; b++){
            int actual = csa_rev[b];
            if (actual <= min){
                min = actual;
                argmin = b;
            }
        }
        p->setStart(argmin);
        auto new_got = new_phrases.find(hasher(*p));
        new_phrases[hasher(*p)] = p;
    }
    phrases = new_phrases;
}


Phrase* RLZ::query_bwt(string::iterator & strIt, string::iterator end){
    // check if the first character is in the alphabet
    string::iterator strStart = strIt;
    bool inAlphabet = false;
    for(int i=1; i<csa.sigma; i++){
        if (csa.comp2char[i] == *strIt){
            inAlphabet = true; 
        }
    }

    if (!inAlphabet){
        auto find = newChar_rev.find(*strIt);
        int idx = 0;
        if (find == newChar_rev.end()){
            newChar_rev[*strIt] = totalLength;
            newChar[totalLength] = *strIt;
            totalLength+=1;
        }
        idx = newChar_rev[*strIt];
        auto ret_cp = create_phrase(idx, 1);
        Phrase * p = ret_cp.first;
        strIt++;
        return p;
    }
    
    size_type l = 0;
    size_type r = csa.size()-1;
    int l_res = 0;
    int r_res = r;
    int length = 1;
    while(strIt != end && r+1-l > 0){
        l_res = l;
        r_res = r;

        backward_search(csa, l, r, (*strIt), l, r);

        strIt ++;
        length ++; 
    }

    // whether the last character is found or the last character is the end of the string
    if (r < l){
        strIt --;
        length -= 2;
    } else{
        l_res = l;
        r_res = r;
        length -= 1;
    }

    // cerr << "ref: " << extract(csa, csa[l_res]-1, csa[l_res]) << endl;
    // cerr << "str: " ;
    // auto sss = strStart;
    // for(int i = 0; i < length; i++){
    //     cerr << *(sss++) ;
    // }
    // cerr << endl;

    // Find the start intervals in csa_rev
    size_type l_start = 0;
    size_type r_start = csa_rev.size() -1 ;
    string::iterator strEnd = strIt;
    do{ 
        strEnd -- ;
        backward_search(csa_rev, l_start, r_start, (*strEnd), l_start, r_start);
    }while(strEnd != strStart);
    pair<int, int> beg_interval = make_pair(l_start, r_start);

    auto ret_cp= create_phrase(l_start, length);
    Phrase * p = ret_cp.first;

    // this is a newly inserted phrase
    if (ret_cp.second == true){
        // end interval is one more LF operation
        vector<int> end_interval; 
        for (int i=0; i < csa.sigma;i++){
            size_type new_l = 0;
            size_type new_r = 0;

            backward_search(csa, l_res, r_res, csa.comp2char[i], new_l, new_r);

            if (int(new_l) <= int(new_r)){
                for(int i = new_l; i <= new_r; i++){
                    int e ;
                    if (csa[i] == csa.size() - 1){
                        e = csa_rev.isa[csa.size() - 1];
                    }
                    else{
                        e = csa_rev.isa[csa.size() - 2 - csa[i]];
                    }
                    end_interval.push_back(e);
                }
            }
        }

        sort(end_interval.begin(), end_interval.end());
        // cerr << "------- " << length << " " << (strIt != end) << endl;
        // cerr << "ref last: " << extract(csa, csa[l_res]-1, csa[l_res]) << endl;
        // cerr << "str next: " << *strIt << endl;
        // for (int b = beg_interval.first; b <= beg_interval.second; b++){
        //     cerr << extract(csa_rev, csa_rev[b]+length, csa_rev[b]+length) << " ";
        //     cerr << csa_rev[b]+length << ", ";
        // }
        // cerr << endl;
        // for (int e : end_interval){
        //     cerr << extract(csa_rev, csa_rev[e], csa_rev[e]) << " ";
        //     cerr << csa_rev[e] << ", ";
        // }
        // cerr << endl;

        // for(int i =0 ; i<end_interval.size(); i++){
        //     assert(csa_rev[end_interval[i]] == csa_rev[beg_interval.first+i]+length);
        // }

        Source * source = new Source {p, beg_interval, end_interval, length-1};
        auto ret = sources.insert(source);

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

pair<Phrase *, bool> RLZ::create_phrase(int pos, int length){
    Phrase * phrase = new Phrase{pos, length};

    PhraseHash hasher;
    auto ret = phrases.find(hasher(*phrase));

    if (ret == phrases.end()){

        phrases[hasher(*phrase)] = phrase;
        return make_pair(phrases[hasher(*phrase)], true);
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
            toReturn += newChar[p->start];
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
            toReturn += newChar[p->start];
        }
        else {
            toReturn += extract(csa_rev, p->start, p->start+p->length-1);
        }
    }
    return toReturn;
}

void RLZ::transferSourceEnds(Source * s){
    vector<int> new_endInterval;
    for (int i : s->end_interval){
        int new_end = 0;
        // cerr << i << " --> " ;
        if (csa[i] == csa.size()-1) {
            new_end = i;
        }else{
            new_end = csa_rev.isa[csa.size() - 2 - csa[i]];
        }
        // cerr << new_end << endl; 
        new_endInterval.push_back(new_end);
    }
    sort(new_endInterval.begin(), new_endInterval.end());
    s->end_interval = new_endInterval;
}

void RLZ::transferSourceStarts(Source * s){
    Phrase * p = s->p;
    string sub = extract(csa, csa[p->start], csa[p->start] + p->length - 1);
    size_type l = 0; 
    size_type r = csa_rev.size()-1;
    reverse(sub.begin(), sub.end());
    backward_search(csa_rev, l, r, sub.begin(), sub.end(), l, r );   // search for the pattern in the reversed reversed BWT.
    s->beg_interval = make_pair(l,r);
}

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