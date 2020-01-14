#include <iostream>
#include <math.h>
#include "bitvector.h"

using namespace std;

BitVector::BitVector(long int length){
    this->length = length;
    array.resize(length/SIZE+1);
     if (length > SIZE){
        array[0] = 1ul<<(SIZE-1);
        long int i = 1;
        long int chunks = this->length / SIZE;
        while (i<chunks){
            array[i] = 0;
            i++;
        }
    } else {
        array[0] = 1ul<<(length-1);
    }
}

void BitVector::addBit(long pos){
    pos = length-1 - pos;
    long int chunk = (length-pos)/SIZE;
    long int pos2 = pos>length%SIZE ? pos - length%SIZE : pos;
    // cout << pos2 << endl;
    array[chunk] = array[chunk] | (1ul<<pos2);
}

void BitVector::clearBit(long pos){
    pos = length-1- pos;
    long int chunk = (length-pos)/SIZE;
    long int pos2 = pos>length%SIZE ? pos - length%SIZE : pos;
    array[chunk] = array[chunk] ^ (1ul<<pos2);
}

void BitVector::setNumber(long number){
    array[0] = number;
}


void BitVector::printNum(){
    unsigned long int num[this->length];
    long i=length-1;
    long j=0;
    int shift = (length % SIZE);
    while(j<length/SIZE+1){
        unsigned long int n = array[array.size()-j-1];
        int count = 0 ;
        while (n>0){
            num[i]=n%2;
            // cout << i << " " << num[i] << endl;
            n = n/2;
            i--;
            count ++;
        }
        int std = SIZE;
        if (j==0) std = shift;
        for(int m=count;m<std;m++) {num[i] = 0; 
            // cout << i << " " << num[i] << endl;
        i--;}       
        j++;
    }
    // for (;i>=0;i--) num[i] = 0;
    for(int m=0;m<this->length;m++) cout << num[m];
    cout << endl;
}

RankSelect::RankSelect(BitVector input){
    long int chunk = input.length/SIZE;

}