#include "RLZ.hpp"

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
        Phrase * p = query_bwt(strIt, to_process.end());
        string1.push_back(p);
    }

    compressed_strings.push_back(string1);
    numPhrases+=string1.size();

    return string1.size();
}

void RLZ::processSources(){
    //TODO Change that to global boolean set from user-given parameter
    bool optimize = true;

    for(auto * s : sources){
        transferSourceStarts(s);
        transferSourceEnds(s);
    }
    
    if (optimize)
        optimize_phrases(phrases, sources, csa.size()-1);
    else
        reset_phrases(phrases, sources);
}


void optimize_phrases(unordered_map<size_t, Phrase*> & phrases, unordered_set<Source*, SourceHash> & sources, int size){

    cerr << "processing" << endl;
    
    Phrase * oldPointer;

    for(auto s : sources){
        if (s->beg_interval.first == 772 && s->length == 4){
            oldPointer = s->p;
            cerr << s << endl;
            cerr << s->p << endl;
            cerr << s->p->start << endl;
            cerr << "(772, 4)---" << endl;
        }

        if (s->beg_interval.first == 346 && s->length == 3){
            cerr << s << endl;
            cerr << s->p << endl;
            cerr << s->p->start << endl;
            cerr << "(346, 3)---" << endl;
        }
    }
    
    vector<int> bp_count (size);
    vector<vector<Source*> > bp_pointer(size);
    // vector<bool> bp_mark(size);

    unordered_set<Source*> phrase_mark;
    unordered_map<size_t, Phrase*> new_phrases;

    // mark the break points
    for(auto s : sources){
        for(int b = s->beg_interval.first; b <= s->beg_interval.second; b++){
            Source * ss = s;
            bp_count[b] += 1;
            bp_pointer[b].push_back(ss);
            // if (b == 346){
            //     cerr << bp_pointer[b][bp_pointer[b].size()-1] << endl;
            //     cerr << bp_pointer[b][bp_pointer[b].size()-1]->p->start << endl;
            //     cerr << bp_pointer[b][bp_pointer[b].size()-1]->p << endl;
            //     cerr << "346 ---" << endl;
            // }
        }
        for (int e : s->end_interval){
            if (e < s->beg_interval.first || e > s->beg_interval.second){
                Source * ss = s;
                bp_count[e] +=1;
                bp_pointer[e].push_back(ss);
            }
        }
    }

    // int j = 0;
    // for (auto v : bp_pointer){
    //     for(auto s : v){
    //         if (s->beg_interval.first == 346 && s->length == 3){
    //             cerr << s << endl;
    //             cerr << s->p << endl;
    //             cerr << s->p->start << endl;
    //             cerr << "---" << j << "---" << endl;
    //         }
    //     }
    //     j++;
    // }

    vector<int> bp_count_copy = bp_count;

    // iteratively find all sources
    while(new_phrases.size() < phrases.size()){
        auto max = max_element(bp_count.begin(), bp_count.end());
        int argmax = distance(bp_count.begin(), max);

        PhraseHash hasher;

        // int k = 0;
        // cerr << argmax << "=== " << endl;
        // for (auto v : bp_pointer){
        //     for(auto s : v){
        //         if (s->beg_interval.first == 346 && s->length == 3){
        //             cerr << "Source pointer: " << s << endl;
        //             cerr << "Phrase pointer: " << s->p << endl;
        //             cerr << s->p->start << endl;
        //             cerr << "---" << k << "---" << endl;
        //         }
        //     }
        //     k++;
        // }
        
        cerr << "Original Count: " << bp_count_copy[argmax] << endl;
        cerr << "Before Count: " << bp_count[argmax] << endl;

        // reset start for all phrases at argmax
        for(Source * s : bp_pointer[argmax]){

            // cerr << s->beg_interval.first << ", " << s->length << endl;
            // cerr << "Source pointer: " << s << endl;
            // cerr << "Phrase pointer: " << s->p << endl;
            // s->print();
            // if (s->beg_interval.first == 772 && s->length == 4){
            //     cerr << s << endl;
            //     cerr << s->p << endl;
            //     cerr << s->p->start << endl;
            // }

            auto ret = phrase_mark.find(s);
            if (ret == phrase_mark.end()){
                //TODO a bp could be a beginning or an ending of the same phrase. Maybe need a better way to distinguish them. For now it does not seem to affect the approximation ratio.
                // rank of argmax in this source's end.
                int rank = 0;
                int end_pos = 0;
                int start_pos = 0;

                // if bp hits a beginning.
                if (argmax >= s->beg_interval.first && argmax <= s->beg_interval.second){
                    cerr << "begin" << endl;
                    for(int b = s->beg_interval.first; b <= s->beg_interval.second; b++){
                        // cerr << b << endl;
                        bp_count[b] -= 1;
                    }
                    start_pos = argmax;
                    rank = argmax - s->beg_interval.first;
                    end_pos = s->end_interval[rank];

                    // remove the other ends of other sources
                    for(int i = 0; i < s->end_interval.size(); i++){
                        if (i!=rank && (s->end_interval[i] < s->beg_interval.first || s->end_interval[i] > s->beg_interval.second)){
                            // cerr << s->end_interval[i] << endl;
                            bp_count[s->end_interval[i]] -= 1;
                        }
                    }
                } 
                // it bp hits an end.
                else {
                    cerr << "end" << endl;
                    end_pos = argmax;
                    for(int i = 0; i < s->end_interval.size(); i++){
                        int e = s->end_interval[i];
                        if (e < s->beg_interval.first || e > s->beg_interval.second){
                            // cerr << e << endl;
                            bp_count[e] -= 1;
                        }
                        if (e == argmax){
                            rank = i;
                        }
                    }
                    start_pos = s->beg_interval.first + rank;

                    // remove the other ends of other sources
                    for(int i =s->beg_interval.first ; i<=s->beg_interval.second; i++){
                        if (i != start_pos){
                            // cerr << i << endl;
                            bp_count[i] -= 1;
                        }
                    }
                }

                s->p->setStart(start_pos);
                new_phrases[hasher(*(s->p))] = s->p;
                phrase_mark.insert(s);

                s->print();
                int r = start_pos - s->beg_interval.first;
                assert(s->end_interval[r] == argmax || start_pos == argmax);


                // cerr << start_pos << ", " << end_pos << endl;
            } else {
                cerr << "already in " << argmax << endl;
                int r = s->p->start - s->beg_interval.first;
                s->print();
                cerr << endl;
                if(s->end_interval[r] == argmax || s->p->start == argmax){
                    bp_count[argmax] -= 1;
                }
            }
        }

        // int m = 0;
        // cerr << argmax << "=== after" << endl;
        // for (auto v : bp_pointer){
        //     for(auto s : v){
        //         if (s->beg_interval.first == 772 && s->length == 4){
        //             cerr << s << endl;
        //             cerr << s->p << endl;
        //             cerr << s->p->start << endl;
        //             cerr << "---" << m << "---" << endl;
        //         }
        //     }
        //     m++;
        // }
        

        cerr << "Count: " << bp_count[argmax] << ", "<< bp_pointer[argmax].size() << endl;
        cerr << "--------" << endl;
        assert(bp_count[argmax] == 0);
    }

    phrases = new_phrases;
    cerr << "done processing" << endl;
}

void reset_phrases( unordered_map<size_t, Phrase*> & phrases, unordered_set<Source*, SourceHash> & sources){
    PhraseHash hasher;
    unordered_map<size_t, Phrase*> new_phrases;
    for(auto * s : sources){
        int oldpos = s->p->start;
        int oldlength = s->p->length;
        size_t oldpointer = size_t(s->p);

        Phrase * p = s->p;

        auto got = phrases.find(hasher(*p));

        p->setStart(s->beg_interval.first);
        auto new_got = new_phrases.find(hasher(*p));
        // if(new_got != phrases.end()){
        //     cerr << "???? Collision ????" << endl;
        //     cerr << &(new_got->second) <<": ";
        //     new_got->second.print();
        //     cerr << endl;
        //     cerr << p <<": ";
        //     s.p->print();
        //     cerr << endl;
        //     // assert(1==0);
        // }
        new_phrases[hasher(*p)] = p;
    }
    phrases = new_phrases;
}


Phrase* RLZ::query_bwt(string::iterator & strIt, string::iterator end){
    // check if the first character is in the alphabet
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
            cerr << "not in alphabet: " << (*strIt) <<  endl;
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
    // int beg_s = 0;
    // int beg_e = r;
    int length = 1;
    while(strIt != end && r+1-l > 0){
        l_res = l;
        r_res = r;

        // backward_search_rank(l, r, (*strIt), beg_s, beg_e);
        backward_search(csa, l, r, (*strIt), l, r);
        
        strIt ++;
        length ++; 
    }

    // whether the last character is found or the last character is the end of the string
    if (r < l){
        strIt --;
        length --;
    } else{
        l_res = l;
        r_res = r;
    }

    // start interval is (l_res, r_res)
    pair<int, int> beg_interval = make_pair(l_res, r_res);

    auto ret_cp= create_phrase(l_res, length-1);
    Phrase * p = ret_cp.first;

    // this is a newly inserted phrase
    if (ret_cp.second == true){
        // end interval is one more LF operation
        vector<int> end_interval; 
        for (int i=0; i < csa.sigma;i++){
            size_type new_l = 0;
            size_type new_r = 0;
            // fflush(stdout);
            // cerr <<  l_res << "," << r_res << endl;
            backward_search(csa, l_res, r_res, csa.comp2char[i], new_l, new_r);
            // int cc = csa.C[i];
            // int l_rank = csa.bwt.rank(l, csa.comp2char[i]);
            // int r_rank = csa.bwt.rank(r+1, csa.comp2char[i]);
            // cerr << new_l << "," << new_r << "," << (new_l <= new_r) << endl;
            if (int(new_l) <= int(new_r)){
                // cerr << "if " << new_l << "," << new_r << "," << (new_l <= new_r) << endl;
                for(int i = new_l; i <= new_r; i++){
                    end_interval.push_back(i);
                }
            }
        }

        sort(end_interval.begin(), end_interval.end());

        Source * source = new Source {p, beg_interval, end_interval, length-1};
        auto ret = sources.insert(source);

    }

    return p;
}


void RLZ::print_comp_string(int stringID){
    cout << "Printing compressed strings" << endl;
    int length = 0;
    for( const Phrase* p : compressed_strings[stringID]){
        p->print();
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

    // if (pos == 20 && length == 6 && phrases.size() > 270){
    //     
    //     cerr << a << endl;
    //     cerr << "true" << endl;
    //     auto ret1 = phrases.find(a);
    //     cerr << ret->second.start << endl;
    // }

    if (ret == phrases.end()){

        phrases[hasher(*phrase)] = phrase;
        return make_pair(phrases[hasher(*phrase)], true);
    } 
    return make_pair(ret->second, false);
}

void RLZ::print_sources(){
    cout << "Printing sources." << endl;
    for(auto s: sources){
        s->print();
    }
}

void RLZ::print_phrases(){
    cout << "Printing phrases." << endl;
    for(auto p : phrases){
        p.second->print();
    }
    cout << endl;
}

string RLZ::decode(int stringID){
    string toReturn;
    for(Phrase * p : compressed_strings[stringID]){
        if (p->start > csa.size() - 1 ){
            assert(p->length == 1);
            p->print();
            toReturn += newChar[p->start];
        }
        else {
            toReturn += extract(csa_rev, csa_rev[p->start], csa_rev[p->start]+p->length-1);
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