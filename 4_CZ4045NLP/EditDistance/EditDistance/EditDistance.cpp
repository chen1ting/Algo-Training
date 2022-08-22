// EditDistance.cpp : This file contains the 'main' function. Program execution begins and ends there.
//
/*
* 
* Problem statement:
Given two strings str1 and str2 and below operations that can be performed on str1. 
Find minimum number of edits (operations) required to convert ‘str1’ into ‘str2’.

Viable Operation:
1. Insert: cost = 1
2. Remove: cost = 1
3. Replace: cost = 2 ( Levenshiten definition, replace = 1 delete + 1 insert)

*/
#include <iostream>
#include <string>
#include <vector>
using namespace std;

// Utility function to find the smallest number among three numbers
int minOfThree(int x, int y, int z) {
    return min(min(x, y), z);
}

int editDistance(string s1, string s2) {
    int m = s1.size();
    int n = s2.size();
    // Initilization: 2D array to store dp costs, fill base case
    vector<vector<int>> dp(m + 1, vector<int>(n + 1, 0));
    vector<vector<char>> operations(m + 1, vector<char>(n + 1));
    
    for (size_t i = 1; i <= m; i++) {
        dp[i][0] = i;
    }
    for (size_t j = 1; j <= n; j++) {
        dp[0][j] = j;
    }

    // Iteration: incrementally add in the values
    for (size_t i = 1; i <= m; i++) {
        for (size_t j = 1; j <= n;j++) {
            dp[i][j] = minOfThree(dp[i - 1][j] + 1, dp[i][j - 1] + 1, dp[i - 1][j - 1] + (s1[i] == s2[j] ? 0 : 1));
        }
    }
    return dp[m][n];
}

void printResult(string s1, string s2) {
    cout << "Edit Distance between \""<<s1<<"\" and \"" << s2 << "\": " << editDistance(s1, s2) << endl;
}

int main()
{
    printResult("idea", "deal");
    string s1 = "compute the edit distance", s2 = "the edit distance is computed";
    while (true) {
        printResult(s1, s2);
        cout << "Enter your own string. (enter \"-1\" to quit)\nString1:";
        cin >> s1;
        if (s1 == "-1") {
            break;
        }
        cout << "String2:";
        cin >> s2;
    }
}