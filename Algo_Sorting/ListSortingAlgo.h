#pragma once
#include <stdio.h>      /* printf, scanf, puts, NULL */
#include <stdlib.h>     /* srand, rand, malloc */
#include <iostream>
#include <chrono>
#include <time.h>       /* time for random seed*/
class ListSortingAlgo
{
public:
	ListSortingAlgo(int n);
	~ListSortingAlgo();
	void PrintList();			//Print list
	void InsertionSort();		//Insertion Sort: input should be a list of intergers and the length of list (if not in a class)
	void MergeSort(int start, int end);
	void QuickSort(int start, int end);
	bool CheckCorrectness();
	
	int* list;
	int* list_after_sorting;
	int n; //list size

private:
	void Merge(int start, int end);	//Does the dirty work for MergeSort
	int Partition(int start, int end);  //Does the dirty work for QuickSort, return the position of pivot
	//Utility funcs
	void GenerateRandList();	//Generate a random list of size n
	void Swap(int i, int j);	//Take the position of items needing Swap. Swap the content of the index

	bool additional_space_required; //flag to indicate whether the sorting method used additional space to hold sorted list
};