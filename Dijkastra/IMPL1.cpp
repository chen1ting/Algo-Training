#include "IMPL1.h"
#include <iostream>
constexpr int MAX = 0x3f3f3f;

IMPL1::IMPL1(int n)
{
	v = n;//initialize the nunmber of nodes
	adj_mtx.resize(v, std::vector<int>(v, MAX));
	S.resize(v);
	d.resize(v);
	pi.resize(v);
	for (int i = 0; i < v; i++)
		adj_mtx[i][i] = 0; //any vertex distance to itself is 0
}

IMPL1::~IMPL1() {
	adj_mtx.clear();
	S.clear();
	d.clear();
	pi.clear();
}

void IMPL1::dijkstra(int src)
{
	//Initialization: for each vertex, set pi to -1 and d to infinity
	fill(S.begin(),S.end(), 0);
	fill(d.begin(),d.end(), MAX);
	fill(pi.begin(),pi.end(), -1);
	//Change the source
	this->source = src;
	d[src] = 0;
	pi[src] = src;
	//Put all vertices in priority queue, Q, in d[v]'s increading order
	for (int j = 0; j < v; j++)
		Q.push_back(j);
	//extract from Q
	for (int iter = 0; iter < v - 1; iter++)
	{
		int u = findCheapest(); //the cheapist element should be in the first position of Q
		S[u] = 1; //Add u to S
		//for each vertex adjacent to u:
		for (int i = 0; i < v; i++)
		{
			if (S[i] != 1 && d[i] > adj_mtx[u][i] + d[u]) //if the vertex is unvisited and d[u]+wt[u,i] is shorter
			{//update i's parent and distance
				d[i] = adj_mtx[u][i] + d[u];
				pi[i] = u;
			}
		}
	}
}

void IMPL1::printPath(int target)
{
	std::stack<int> path;
	int parent = pi[target];
	path.push(target);
	while (parent != pi[parent]) //while haven't reach parent 
	{
		path.push(parent);
		parent = pi[parent];
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

void IMPL1::printSol()
{
	cout << "Vertex \t Distance from Source" << endl;
	for (int i = 0; i < v; i++)
		cout << i << " \t\t" << d[i] << endl;
}

void IMPL1::printGraph()
{
	for (int i = 0; i < v; i++)
	{
		cout << i << ": { ";
		for (int j = 0; j < v; j++)
		{
			if (j != i && adj_mtx[i][j] != MAX)
				cout << j << ",wt: "<<adj_mtx[i][j]<<";  ";
		}
		cout << " }\n";
	}
}

void IMPL1::makeEdge(int i, int j, int wt)
{
	adj_mtx[i][j] = wt;
}

int IMPL1::findCheapest()
{
	// Initialize min value
	int min = INT_MAX, min_index;

	for (int i = 0; i < v; i++)
		if (S[i] == 0 && d[i] <= min)
			min = d[i], min_index = i;

	return min_index;
}
