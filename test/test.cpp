#include "../src/bitvector.h"
#include "../src/RLZfact.h"
#include "../src/RLZgraph.h"

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
        for (int j=0;j<fact.phrases.size();j++){
            printf("(%u, %u)", fact.getPhrase(j).pos, fact.getPhrase(j).length);
        }
        cout << endl;
        i++;
    }

    string t1 = graph.reconstruct(0);
    string t2 = graph.reconstruct(1);
    assert(s1==t1);
    assert(s2==t2);

    return 0;
}