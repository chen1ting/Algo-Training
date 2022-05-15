/*
 * The bitwise AND of an array nums is the bitwise AND of all integers in nums.

 For example, for nums = [1, 5, 3], the bitwise AND is equal to 1 & 5 & 3 = 1.
 Also, for nums = [7], the bitwise AND is 7.
 You are given an array of positive integers candidates.
 Evaluate the bitwise AND of every combination of numbers of candidates.
 Each number in candidates may only be used once in each combination.

 Return the size of the largest combination of candidates with a bitwise AND greater than 0.
 */
#include <iostream>
#include <vector>
using namespace std;

int largestCombination(vector<int>& candidates) {
//since input is smaller than 10^7, we can form a bitmap to store the bit position of each number
    vector<int> bin_map(24, 0); //bin_map is short for binary map
    for(int num : candidates){
        int i = 0;
        while (num != 0) {
            bin_map[i++] += num % 2;
            num = num / 2;
        }
    }
    return *max_element(bin_map.begin(), bin_map.end());
}

int main(){
    vector<int> candidates = {16,17,71,62,12,24,14};
    cout << largestCombination(candidates);
}