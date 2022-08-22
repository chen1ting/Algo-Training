#pragma once
#include<vector>
#include<stack>
#include <limits.h>
using namespace std;
//adjMatrix + array for priority queue
/*Suppose the input graph G = (V, E) is stored in an adjacency matrix and we
use an array for the priority queue.Implement the Dijkstra¡¯s algorithm using this
settingand analyze its time complexity with respect to | V | and | E | both
theoretically and empirically.
*/

class IMPL1 {
public:
	IMPL1(int n);					//constructor
	~IMPL1();
	void dijkstra(int src);		//use the dijkstra algorithm to traverse through all the nodes, and obtain d & pi
	void printPath(int target);			//print out the path from given source to target
	void printSol();
	void printGraph();
	void makeEdge(int i, int j, int wt);	//make a edge from vertex i to vertex j with the given weight
	
private:	
	int findCheapest();
	//variables
	int v;					//number of vertices
	int source;				//source of a path
	vector<vector<int>> adj_mtx;			//2 dimensional array for adjacency matrix
	vector<int> S;		//array for priority queue
	vector<int> d;		//distance from the source
	vector<int> pi;	//parent node
	vector<int> Q;	//priority queue to find the next closest vertex
};

