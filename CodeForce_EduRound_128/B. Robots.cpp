/*
 * There is a field divided into 𝑛 rows and 𝑚 columns. Some cells are empty (denoted as E), other cells contain robots (denoted as R).

You can send a command to all robots at the same time. The command can be of one of the four types:
move up; move right; move down; move left.
When you send a command, all robots at the same time attempt to take one step in the direction you picked.
If a robot tries to move outside the field, it explodes; otherwise, every robot moves to an adjacent cell in the chosen direction.

You can send as many commands as you want (possibly, zero), in any order.
Your goal is to make at least one robot reach the upper left corner of the field.
 Can you do this without forcing any of the robots to explode?

Input
The first line contains one integer 𝑡 (1≤𝑡≤5000) — the number of test cases.
Each test case starts with a line containing two integers 𝑛 and 𝑚 (1≤𝑛,𝑚≤5) — the number of rows and the number of columns, respectively. Then 𝑛 lines follow; each of them contains a string of 𝑚 characters. Each character is either E (empty cell} or R (robot).
Additional constraint on the input: in each test case, there is at least one robot on the field.
6
1 3
ERR
2 2
ER
RE
2 2
ER
ER
1 1
R
4 3
EEE
EEE
ERR
EER
3 3
EEE
EER
REE

Output
If it is possible to make at least one robot reach the upper left corner of the field so that no robot explodes, print YES. Otherwise, print NO.
 */

#include<iostream>
#include<string>
#define MAX_MN 5
using namespace std;

int main(){
    int n, m, loop_times = 0;
    int** robot_map = new int*[MAX_MN]; //2D array for the map

    for(int j = 0; j < MAX_MN; j++) //initialize 2D array
        robot_map[j] = new int[MAX_MN];
    cin >> loop_times;
    for(int z = 0; z < loop_times; z++){
        cin >> n;   //nums of lines
        cin >> m;   //nums of columns
        char tmp_char;
        bool first_r = true;
        int first_r_y;
        for(int i = 0; i < n; i++){
            for(int j = 0; j < m; j++){
                cin>>tmp_char;
                if(tmp_char == 'E')
                    robot_map[i][j] = 0;
                else{
                    robot_map[i][j] = 1;
                    if(first_r) {
                        first_r_y = j;
                        first_r = false;
                    }
                }
            }
        }
        bool can = true;
        for(int j = 0; j < first_r_y; j++){
            for(int i = 0; i < n; i++){
                if(robot_map[i][j] == 1){
                    can = false;
                    break;
                }
            }
            if(!can)
                break;
        }
        if(can)
            cout<<"YES"<<endl;
        else
            cout<<"NO"<<endl;
    }
    //delete the 2D array
    for(int j = 0; j < MAX_MN; j++)
        delete[] robot_map[j];
    delete[] robot_map;
    return 0;
}