#include "../src/bitvector.h"
#include "../src/RLZfact.h"
#include "../src/RLZgraph.h"

struct classcomp2{
    bool operator() (const long int & lhs, const long int & rhs) const{
        return lhs > rhs;
    }
};


int main(){
    string ref = "ATATTCGACGAGAT";
    string s1 = "ATAATTCGATTCGAT";
    string s2 = "ATTTCGAGATATA";

    RLZgraph graph(ref);

    // graph.addString("TTCGA");
    graph.addString(s1);
    graph.addString(s2);

    int i = 0;
    for (RLZfact fact : graph.rlzarr){
        printf("The %u th string: ",i);
        for (long int j=0;j<fact.phrases.size();j++){
            printf("(%lu, %lu)", fact.getPhrase(j).pos, fact.getPhrase(j).length);
        }
        cout << endl;
        i++;
    }

    auto it = graph.nodeDict.end();
    it--;
    for (;it!=graph.nodeDict.begin(); it--){
        printf("Node at %lu (pos: %lu, length:%lu): \n",it->first, it->second->pos,it->second->length);
        auto colorit = it->second->Ends.begin();
        for (;colorit!=it->second->Ends.end();colorit++){
            printf("   It has ends at string %lu: ", colorit->first);
            auto rankit = colorit->second.begin();
            for (; rankit!=colorit->second.end(); rankit++){
                cout << *rankit << ",";
            }
            cout << endl;
        }
        printf("   And the next node is at %lu.\n", it->second->next->pos);
    }

    string t1 = graph.reconstruct(0);
    string t2 = graph.reconstruct(1);
    // assert(s1==t1);
    // assert(s2==t2);
    cout << t1 << endl;
    cout << endl;
    cout << t2 << endl;

    // map<long int, vector<long int>, classcomp2> Starts;

    // Starts.insert(make_pair(1, vector<long int>{2,3}));
    // Starts.insert(make_pair(4, vector<long int> {444,11}));
    // Starts.insert(make_pair(3, vector<long int> {123}));

    // auto iit = Starts.upper_bound(2);
    // cout << iit->first << endl;

    // for(auto it = Starts.begin(); it!=Starts.end(); it++){
    //     cout << it->first << endl;
    // }

    return 0;
}