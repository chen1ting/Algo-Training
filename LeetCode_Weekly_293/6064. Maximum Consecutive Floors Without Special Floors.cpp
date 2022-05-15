/*
 * Alice manages a company and has rented some floors of a building as office space.
 * Alice has decided some of these floors should be special floors, used for relaxation only.

 You are given two integers bottom and top, which denote that Alice has rented all the floors from bottom to top (inclusive).
 You are also given the integer array special, where special[i] denotes a special floor that Alice has designated for relaxation.

Return the maximum number of consecutive floors without a special floor.
 */
#include <iostream>
#include <vector>
using namespace std;
/* v1: exceed memory
int maxConsecutive(int bottom, int top, vector<int>& special) {
    vector<int> spc_floors(top - bottom + 1, 1);
    for(int i : special)
        spc_floors[i-bottom] = 0;
    int max_cnt = 0, cur_cnt = 0;
    for(int i : spc_floors){
        if(i == 0){
            max_cnt = cur_cnt > max_cnt ? cur_cnt:max_cnt;
            cur_cnt = 0;
            continue;
        }
        cur_cnt += 1;
    }
    return cur_cnt > max_cnt ? cur_cnt : max_cnt;   //to make it work under situations where last floor is 1
} */
int maxConsecutive(int bottom, int top, vector<int>& special){
    sort(special.begin(), special.end());
    int max_cnt = special[0] - bottom;
    for(size_t i = 1; i < special.size(); i++){
        int new_floors = special[i] - special [i-1] - 1;
        if( new_floors > max_cnt)
            max_cnt = new_floors;
    }
    int lst_floors = top - special[special.size() - 1];
    return max_cnt > lst_floors ? max_cnt : lst_floors;
}

int main(){
    vector<int> floors = {4,6};
    cout << maxConsecutive(2, 9, floors);
}