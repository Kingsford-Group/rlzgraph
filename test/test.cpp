#include <vector>
#include <string>
#include <iostream>
using namespace std;

int main(){
    vector<char> buf (20,-1);
    cout << buf.data() << endl;
}