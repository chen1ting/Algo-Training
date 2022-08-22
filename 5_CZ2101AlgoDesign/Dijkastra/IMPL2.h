#pragma once
#include<iostream>
#include<vector>
#include<stack>
using namespace std;
//2. adjList +minimizing heap for priority queue
/*Suppose the input graph G = (V, E) is stored in an array of adjacency lists and
we use a minimizing heap for the priority queue. Implement the Dijkstra¡¯s
algorithm using this setting and analyze its time complexity with respect to |V|
and |E| both theoretically and empirically.
*/

//graph implementation
struct AdjListNode
{
	int dst; //destination
	int wt;
	struct AdjListNode* nxt;
};

struct List
{
	// Pointer to head node of list
	struct AdjListNode* head;
};

struct AdjList
{
	int V;
	struct List* array;
};

//heap implementation
struct MinHeapNode
{
	int  v;
	int dist;	//distance
};

struct MinHeap
{
	int size;		// Number of heap nodes present currently
	int* pos;		// This is needed for updateDis()
	struct MinHeapNode** array;
};

class IMPL2 {
public:
	IMPL2(int n);					//constructor
	~IMPL2();						//destructor
	void dijkstra(int source);		//use the dijkstra algorithm to traverse through all the nodes, and obtain d & pi
	void printPath(int target);			//print out the path from given source to target
	void printSol();
	void makeEdge(int i, int j, int wt);	//make a edge from vertex i to vertex j with the given weight
private:
	//adjacency list
	struct AdjListNode* newAdjListNode(int dest, int weight);

	//heap
	struct MinHeapNode* newMinHeapNode(int v, int dist);
	void heapSwap(struct MinHeapNode** a, struct MinHeapNode** b);
	void fixHeap(int idx); 	// A standard function to heapify at given idx(node). This function also updates position of nodes when they are swapped. Position is needed for updateDis()
	struct MinHeapNode* extractMin(); // Standard function to extract minimum node from heap
	void updateDis(int v, int dist);
	bool isInMinHeap(int v);

	//variables
	int v;						//number of vertices
	int source;					//source of a path
	struct AdjList* adj_list;	//adjacency list for graph
	std::vector<int> S;			//array for priority queue
	std::vector<int> d;			//distance from the source
	std::vector<int> pi;		//parent node
	struct MinHeap* minHeap;
};
