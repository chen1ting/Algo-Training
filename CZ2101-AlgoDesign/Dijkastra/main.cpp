#include "IMPL1.h"
#include "IMPL2.h"
#include <chrono>
#include <fstream>
constexpr int TEST_TIMES = 20;
int main()
{
	vector<int> v_record;
	vector<int> e_record;
	vector<int> src_record;
	vector<int> t1_record;
	vector<int> t2_record;
	int v_num = 50;	//v_num represents number of node in the graph
	while (v_num < 1000)
	{
		int e_num = v_num;
		for (int iter = 0; iter < TEST_TIMES; iter++)
		{
			IMPL1 g1(v_num);
			IMPL2 g2(v_num);

			//Generating a random graph
			int i = 0;
			vector<vector<int>> edge(e_num, vector<int>(2, -1));
			srand(time(0));//truely random
			while (i < e_num)
			{
				//build a connection betwween two random vertex
				edge[i][0] = rand() % v_num;
				edge[i][1] = rand() % v_num;
				if (edge[i][0] == edge[i][1])//if the random function generated same source and distance for the edge, skip this and generate again.
					continue;
				for (int j = 0; j < i; j++)//check whether the edge connecting the same src and destination has appeared before, if so, redo
				{
					if ((edge[j][0] == edge[i][0] && edge[j][1] == edge[i][1]) ||
						(edge[j][1] == edge[j][0] && edge[j][0] == edge[j][1]))
						continue;
				}
				int wt = rand() % v_num + 1;
				g1.makeEdge(edge[i][0], edge[i][1], wt); //weight must be bigger than 0;
				g2.makeEdge(edge[i][0], edge[i][1], wt); //weight must be bigger than 0;
				i++;
			}

			srand(time(0));
			int source = rand() % v_num;	//random generation a source
			auto impl1_start = std::chrono::high_resolution_clock::now();
			g1.dijkstra(source);
			auto impl1_end = std::chrono::high_resolution_clock::now();
			auto duration1 = std::chrono::duration_cast<std::chrono::microseconds>(impl1_end - impl1_start);
			auto impl2_start = std::chrono::high_resolution_clock::now();
			g2.dijkstra(source);
			auto impl2_end = std::chrono::high_resolution_clock::now();
			auto duration2 = std::chrono::duration_cast<std::chrono::microseconds>(impl2_end - impl2_start);
			//record everything into vector
			v_record.push_back(v_num);
			e_record.push_back(e_num);
			t1_record.push_back(duration1.count());
			t2_record.push_back(duration2.count());
			src_record.push_back(source);
		}
		cout << v_num << endl;
		v_num += 20;
	}
	
	//after while loop, print the record to csv file
	ofstream record_of;
	record_of.open("Sparse_Graph.csv");
	record_of << "NumOfVertices,NumOfEdge,Source,Mtx+Arr runtime(ms),List+Heap runtime(ms)" << endl;
	for (int i = 0; i < v_record.size(); i++)
	{
		record_of << v_record[i] << "," <<
			e_record[i] << "," <<
			src_record[i] << "," <<
			t1_record[i] << "," <<
			t2_record[i] << endl;
	}
}