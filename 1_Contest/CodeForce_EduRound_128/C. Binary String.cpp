/*
 * You are given a string 𝑠 consisting of characters 0 and/or 1.

You have to remove several (possibly zero) characters from the beginning of the string, and then several (possibly zero) characters from the end of the string.
The string may become empty after the removals. The cost of the removal is the maximum of the following two values:

the number of characters 0 left in the string;
the number of characters 1 removed from the string.
What is the minimum cost of removal you can achieve?

Input
The first line contains one integer 𝑡 (1≤𝑡≤104) — the number of test cases.
Each test case consists of one line containing the string 𝑠 (1≤|𝑠|≤2⋅105), consisting of characters 0 and/or 1.
The total length of strings 𝑠 in all test cases does not exceed 2⋅105.

5
101110110
1001001001001
0000111111
00000
1111

Output
For each test case, print one integer — the minimum cost of removal you can achieve.
 */
#include<iostream>
#include<string>
using namespace std;
int main(){
    int loop_times;
    cin >> loop_times;
    string s;
    for(int z = 0; z < loop_times; z++) {
        cin >> s;
        //idea 1: maintain 2 arrays to calculate the cost from left to right and right to left.
        
    }
}