#include "IMPL2.h"
constexpr int MAX = 0x3f3f3f;

IMPL2::IMPL2(int n)
{
	v = n;
	S.resize(v);
	d.resize(v);
	pi.resize(v);
	
	//1. malloc an array of adjacency lists for graph representation 
	adj_list = (struct AdjList*)malloc(sizeof(struct AdjList));
	adj_list->V = v;
	// Initialize each adjacency list as empty by making head as NULL
	adj_list->array = (struct List*)malloc(v * sizeof(struct List));
	for (int i = 0; i < v; i++)
		adj_list->array[i].head = NULL;
	
	//2. malloc a heap structure for priority list
	minHeap = (struct MinHeap*)malloc(sizeof(struct MinHeap));
	minHeap->pos = (int*)malloc(v * sizeof(int));
	minHeap->size = v;
	minHeap->array = (struct MinHeapNode**)malloc(v * sizeof(struct MinHeapNode*));
}
IMPL2::~IMPL2()
{
	//might cause memory leak problem. But have to free somewhere
	free(adj_list->array);
	free(adj_list);
	free(minHeap->array);
	free(minHeap);

}

void IMPL2::dijkstra(int src)
{
	//initialize everything
	fill(S.begin(), S.end(), 0);
	fill(d.begin(), d.end(), MAX);
	fill(pi.begin(), pi.end(), -1);

	// Initialize min heap with all vertices. dist value of all vertices
	for (int i = 0; i < v; i++)
	{
		minHeap->array[i] = newMinHeapNode(i, d[i]);
		minHeap->pos[i] = i;
	}
	// Make distance value of src vertex as 0 so that it is extracted first 
	//minHeap->array[src] = newMinHeapNode(src, d[src]);
	//minHeap->pos[src] = src;
	d[src] = 0;
	pi[src] = src;
	updateDis(src, d[src]);

	// In the followin loop, min heap contains all nodes whose shortest distance is not yet finalized.
	while (minHeap->size > 0)
	{
		// Extract the vertex with minimum distance value
		struct MinHeapNode* minHeapNode = extractMin();
		// Store the extracted vertex number
		int u = minHeapNode->v;
		S[u] = 1;
		// Traverse through all adjacent vertices of u (the extracted vertex) and update their distance values
		struct AdjListNode* neighbours = adj_list->array[u].head;
		while (neighbours != NULL)
		{
			int next_v = neighbours->dst;

			// If shortest distance to v is not finalized yet, and distance to v through u is less than its previously calculated distance
			if (isInMinHeap(next_v) && S[next_v] != 1 && neighbours->wt + d[u] < d[next_v])
			{
				d[next_v] = d[u] + neighbours->wt;
				pi[next_v] = u;
				// update distance value in minHeap also
				updateDis(next_v, d[next_v]);
			}
			neighbours = neighbours->nxt;
		}
	}
}

MinHeapNode* IMPL2::newMinHeapNode(int vertex, int dist)
{
	struct MinHeapNode* minHeapNode = (struct MinHeapNode*)malloc(sizeof(struct MinHeapNode));
	minHeapNode->v = vertex;
	minHeapNode->dist = dist;
	return minHeapNode;
}
void IMPL2::heapSwap(MinHeapNode** a, MinHeapNode** b)
{
	struct MinHeapNode* t = *a;
	*a = *b;
	*b = t;
}
void IMPL2::fixHeap(int idx)
{
	int smallest, left, right;
	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;

	if (left < minHeap->size && minHeap->array[left]->dist < minHeap->array[smallest]->dist)
		smallest = left;

	if (right < minHeap->size && minHeap->array[right]->dist < minHeap->array[smallest]->dist)
		smallest = right;

	if (smallest != idx)
	{
		// The nodes to be swapped in min heap
		MinHeapNode* smallestNode = minHeap->array[smallest];
		MinHeapNode* idxNode = minHeap->array[idx];
		// Swap positions
		minHeap->pos[smallestNode->v] = idx;
		minHeap->pos[idxNode->v] = smallest;
		// Swap nodes
		heapSwap(&minHeap->array[smallest], &minHeap->array[idx]);
		fixHeap(smallest);//recursive function call
	}
}
void IMPL2::updateDis(int vertex, int dist)
{	// This is a O(Logn) loop
	// Get the index of v in  heap array
	int i = minHeap->pos[vertex];

	// Get the node and update its dist value
	minHeap->array[i]->dist = dist;

	// Travel up while the complete tree is not hepified.
	while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist)
	{
		// Swap this node with its parent
		minHeap->pos[minHeap->array[i]->v] =(i - 1) / 2;
		minHeap->pos[minHeap->array[(i - 1) / 2]->v] = i;
		heapSwap(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
		// move to parent index
		i = (i - 1) / 2;
	}
}
MinHeapNode* IMPL2::extractMin()
{
	if (minHeap->size == 0)
		return NULL;
	// Store the root node
	struct MinHeapNode* root = minHeap->array[0];

	// Replace root node with last node
	struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
	minHeap->array[0] = lastNode;

	// Update position of last node
	minHeap->pos[root->v] = minHeap->size - 1;
	minHeap->pos[lastNode->v] = 0;

	// Reduce heap size and heapify root
	--minHeap->size;
	fixHeap(0);

	return root;
}
bool IMPL2::isInMinHeap(int v)
{
	if (minHeap->pos[v] < minHeap->size)
		return true;
	return false;
}

AdjListNode* IMPL2::newAdjListNode(int dest, int weight)
{
	struct AdjListNode* newNode =(struct AdjListNode*)malloc(sizeof(struct AdjListNode));
	newNode->dst = dest;
	newNode->wt = weight;
	newNode->nxt = NULL;
	return newNode;
}
void IMPL2::makeEdge(int i, int j, int wt)
{
	// Add an edge from i to j. 
	// A new node is added to the adjacency list of i.  The node is added at the beginning
	struct AdjListNode* newNode = newAdjListNode(j, wt);
	newNode->nxt = adj_list->array[i].head;
	adj_list->array[i].head = newNode;
}

void IMPL2::printPath(int target)
{
	std::stack<int> path;
	int parent = pi[target];
	path.push(target);
	while (parent != pi[parent] && pi[parent]!=-1) //while haven't reach parent 
	{
		path.push(parent);
		parent = pi[parent];
	}
	if (pi[parent] == -1)
	{
		std::cout << "no path between src to target" << std::endl;
		return;
	}
	std::cout << source;
	while (!path.empty())
	{
		std::cout << "->" << path.top();
		path.pop();
	}
	std::cout << std::endl << "Distance:" << d[target];
	std::cout << std::endl;
}
void IMPL2::printSol()
{
	cout << "Vertex \t Distance from Source" << endl;
	for (int i = 0; i < v; i++)
		cout << i << " \t\t" << d[i] << endl;
}
