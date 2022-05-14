/*
 * There is a field divided into 𝑛 rows and 𝑚 columns. Some cells are empty (denoted as E), other cells contain robots (denoted as R).

You can send a command to all robots at the same time. The command can be of one of the four types:

move up;
move right;
move down;
move left.
When you send a command, all robots at the same time attempt to take one step in the direction you picked. If a robot tries to move outside the field, it explodes; otherwise, every robot moves to an adjacent cell in the chosen direction.

You can send as many commands as you want (possibly, zero), in any order. Your goal is to make at least one robot reach the upper left corner of the field. Can you do this without forcing any of the robots to explode?

Input
The first line contains one integer 𝑡 (1≤𝑡≤5000) — the number of test cases.

Each test case starts with a line containing two integers 𝑛 and 𝑚 (1≤𝑛,𝑚≤5) — the number of rows and the number of columns, respectively. Then 𝑛 lines follow; each of them contains a string of 𝑚 characters. Each character is either E (empty cell} or R (robot).

Additional constraint on the input: in each test case, there is at least one robot on the field.

Output
If it is possible to make at least one robot reach the upper left corner of the field so that no robot explodes, print YES. Otherwise, print NO.
 */
