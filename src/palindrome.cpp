/**
 * Runtime analysis
 * 
 * This algorithm adopts the Karp-Rabin Algorithm. 
 * I first randomly generate a prime number between 10^9 and 10^10. Then I compute all r[] and a[] values in order to compute h[i,j] in linear time for string comparison. 
 * 
 * For each location i, we check for odd-length and even-length palindromes using binary search twice. The first search will stop at the first mismatch character from the center of the palindrome (i). The second search will perform the same task but not include the mismatched character.
 *      - The total runtime is O(nlogn)
 * 
 * Since Karp-Rabin algorithm is a Monte Carlo algorithm, I convert it to a Las Vegas algorithm by adding a validation step. For each maximum-length palidrome output, I check by bruteforce to see if it is truly a palindrome with one mismatch. If not, rerun the entire algorithm.
 *      - Validation step takes O(n)
 * 
 * The number of comparisons made is O(nlogn), which is <10^7. Since I only pick a prime that is between 10^9 and 10^10, the error rate for each loction is < log(10^6)/10^4 < 1/1000. 
 * 
 * The expected number of iteration of the algorithm is then 1000/999.
 * Therefore, the total runtime of this algorithm is O(1000/999(n+nlogn))
 */
#include <iostream>
#include <stdio.h> 
#include <math.h>
#include <stdlib.h>
#include <map>
#include <vector>

using namespace std;

static int b = 52;
static map<char, int> dict;

long long mod(long long a, long long b){
    if (a >= 0){
        return a%b;
    } else 
        return a%b+b;
}

class Hash{
    public:
    string s;
    int size;
    long long prime;
    vector<long long> r;
    vector<long long> a;

    Hash(string s, long long prime){
        this->s = s;
        this->prime = prime;
        // cout << "Prime: " << prime << endl;
        size = s.length();
        r = vector<long long>(size+1);
        a = vector<long long>(size+1);
    }

    void rehash(long long prime){
        this->prime = prime;
        r[0] = 1;
        for (int i=1; i<size+1;i++){
            r[i] = (r[i-1]*b)% prime; 
        }
        a[0] = 0;
        for (int i=1; i<size+1;i++){
            a[i] = (a[i-1]*b+ (long long)dict[s[i-1]])%prime;
        }
    }

    int hash(int start, int end){
        return mod(a[end+1]-a[start]*r[end-start+1], prime);
    }
};

class PalindromeChecker{
    public:
    string s;
    string rev;
    long long prime;
    int size;
    Hash *H1;
    Hash *H2;
    
    PalindromeChecker(string s, string rev){
        this->s = s;
        this->rev = rev;
        this->size = s.length();
        H1 = new Hash(s, 3);
        H2 = new Hash(rev, 3);
    }

    void rehash(){
        this->prime = randomPrime();
        H1->rehash(prime);
        H2->rehash(prime);
    }

    long long randomPrime(){
        int min = int(pow(10,9));
        int range = int(pow(10,10)-pow(10,9));

        int num;
        int numtries;
        long long ranNum;
        do{
            num = 0;
            ranNum = rand() % range+min;
            int upper = floor(sqrt(ranNum));
            numtries = upper-2+1;
            for (int i = 2; i<= upper; i++){
                int result = remainder(ranNum, i);
                if (result != 0) num+=1; 
            }
        }while (num != numtries);

        return ranNum;
    }

    bool equal_mid(int mid, int length){
        if (H1->hash(mid, mid+length) == H2->hash(s.length()-1-mid,s.length()-1-mid+length)){
            return true;
        }
        return false;
    }

    bool equal_given(int lower, int upper, int mid, bool even){
        int rev_lower;
        int rev_upper;
        if (even){
            rev_lower = size-1-mid + (lower - mid)+1;
            rev_upper = size-1-mid + (upper - mid)+1;            
        }else{
            rev_lower = size-1-mid + (lower - mid);
            rev_upper = size-1-mid + (upper - mid);
        }

        int h1 = H1->hash(lower, upper);
        int h2 = H2->hash(rev_lower, rev_upper);
        if (h1==h2){
            return true;
        }
        return false;
    }

    int BinarySearch(int lower, int upper, int mid, int prev_upper, bool flag, bool even){
        if (lower >= upper){
            return upper;
        }
        if (equal_given(lower, upper-1, mid, even)){
            if (flag == true){
                return BinarySearch(upper, prev_upper, mid, prev_upper, flag, even);
            }
            else return upper-1;
        }
        else {
            flag = true;
            return BinarySearch(lower, lower+int((upper-lower)/2), mid, upper, flag, even);
        }
    }

    bool checkEquality_t(int lower, int upper, int mid, bool even){
        int rev_lower;
        int rev_upper;
        if (even){
            rev_lower = size-1-mid + (lower - mid)+1;
            rev_upper = size-1-mid + (upper - mid)+1;            
        }else{
            rev_lower = size-1-mid + (lower - mid);
            rev_upper = size-1-mid + (upper - mid);
        }
        for(int i=lower; i<=upper; i++){
            if(s[i] != rev[rev_lower+(i-lower)]){
                return false;
            }
        }
        return true;
    }   
};

map<char, int> getAlphabet(){
    map<char, int> dict;
    for (char i='A'; i<='Z'; i++){
        dict.insert(pair<char, int>(i,i-'A'));
    }
    for (char i='a'; i<='z'; i++){
        dict.insert(pair<char, int>(i, i-'a'+26));
    }
    return dict;
}

int main(){
    string inputS;
    cin >> inputS;
    int size = inputS.length();
    char * rev = new char[size+1];

    for (int i=0;i<size;i++){
        rev[size-1-i] = inputS[i];
    }

    string reverseS = string(rev);

    dict = getAlphabet();

    PalindromeChecker PC (inputS, reverseS);

    string answer;
    int bestUpper=0;
    int bestFupper=0;
    bool bestEven=false;
    int bestMid=0;
    int maxLength =1;

    bool testFirst; // test the first segment
    bool testSecond; // test the second segment
    do {
        maxLength = 1;
        PC.rehash();
        for (int i=1; i<size; i++){
            int upper;
            int f_upper;
            int upper_ori;
            int lower;

            // odd
            if (i<=size/2){
                upper_ori = i+i+1;
                lower = i+1;
                f_upper = PC.BinarySearch(lower, upper_ori, i, upper_ori, false, false);
                if (f_upper+1 < upper_ori){
                    upper = PC.BinarySearch(f_upper+1,upper_ori,i,upper_ori,false,false);
                    if (!PC.equal_given(f_upper+1, upper, i, false)){
                        upper = upper -1;
                    }
                } else {upper = f_upper;}
            }
            else if (i<size-1){
                upper_ori = size;
                lower = i+1;
                f_upper = PC.BinarySearch(lower, upper_ori, i, upper_ori,false, false);
                if (f_upper+1 < upper_ori){
                    upper = PC.BinarySearch(f_upper+1,upper_ori,i,upper_ori,false,false);
                    if (!PC.equal_given(f_upper+1, upper, i, false)){
                        upper = upper -1;
                    }
                } else {upper = f_upper;}
            } else {
                upper = i;
                f_upper = i;
            }        
            if ((upper - i)*2+1>= maxLength){
                maxLength = (upper - i)*2+1;
                bestUpper = upper;
                bestFupper = f_upper;
                bestMid = i;
                bestEven = false;
            }

            // cout << "--- EVEN ----" << endl;
            if (i<=size/2){
                upper_ori = i+i;
                lower = i;
                f_upper = PC.BinarySearch(lower, upper_ori, i, upper_ori,false, true);
                if (f_upper+1 < upper_ori){
                    upper = PC.BinarySearch(f_upper+1, upper_ori, i, upper_ori,false, true);
                    if (!PC.equal_given(f_upper+1, upper, i, true)){
                        upper = upper -1;
                    }
                }else {upper = f_upper;}
            } else {
                upper_ori = size;
                lower = i;
                f_upper = PC.BinarySearch(lower, upper_ori, i, upper_ori,false, true);
                if (f_upper+1 < upper_ori){
                    upper = PC.BinarySearch(f_upper+1, upper_ori, i, upper_ori,false, true);
                    if (!PC.equal_given(f_upper+1, upper, i, true)){
                        upper = upper -1;
                    }
                }
                else {upper = f_upper;}
            }
            if ((upper - i+1)*2>= maxLength){
                maxLength = (upper - i+1)*2;
                bestUpper = upper;
                bestFupper = f_upper;
                bestMid = i;
                bestEven = true;
            }
        }
        

        answer = inputS.substr(bestUpper +1 - maxLength, maxLength);
        // test validity of the found palidrome
        testFirst = PC.checkEquality_t(bestMid, bestFupper-1, bestMid, bestEven);
        testSecond = PC.checkEquality_t(bestFupper+1, bestUpper, bestMid, bestEven);
    } while (!testFirst | !testSecond);
    cout << answer << endl;
    return 0;
}