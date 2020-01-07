#include "../src/bitvector.h"
#include "../src/RLZfact.h"
#include "../src/RLZgraph.h"

int main(){
    long int length = 66;

    BitVector bit (length);
    bit.addBit(62);
    bit.addBit(65);
    bit.clearBit(62);

    bit.printNum();
}