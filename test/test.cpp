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

    return 0;
}