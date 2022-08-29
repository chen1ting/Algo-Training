#include<iostream>
#include<vector>
#include<string>
#include<queue>
#include<utility>
using namespace std;



/* https://leetcode.com/problems/flood-fill/

An image is represented by an m x n integer grid image where image[i][j] represents the pixel value of the image.

You are also given three integers sr, sc, and color. You should perform a flood fill on the image starting from the pixel image[sr][sc].

To perform a flood fill, consider the starting pixel, plus any pixels connected 4-directionally to the starting pixel of the same color as the starting pixel,

plus any pixels connected 4-directionally to those pixels (also with the same color), and so on. Replace the color of all of the aforementioned pixels with color.

Return the modified image after performing the flood fill.
*/
vector<vector<int>> floodFill(vector<vector<int>>& image, int sr, int sc, int color) {
	
	queue<pair<int, int>> q;
	q.push(pair<int, int>(sr, sc));
	while (q.size() > 0) {
		pair<int, int> cur_point = q.front();
		
	}
}

