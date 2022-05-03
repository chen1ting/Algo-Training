//
// Created by 陈一婷 on 3/5/22.
//

#ifndef SORTING_SORTINGALGO_H
#define SORTING_SORTINGALGO_H
#include <chrono>
#include <iostream>
using namespace std;
class SortingAlgo {
public:
    explicit SortingAlgo(int n);
    ~SortingAlgo();
    void optChoice();

private:
    int InsertionSort(int* lst);    //sort the list in the class, return runtime (milliseconds)
    int MergeSort(int* lst);        //return runtime (milliseconds)
    void mergesort(int l, int r);
    void merge(int l, int r);
    int QuickSort(int* lst);
    void quicksort(int l, int r);
    int partition(int l, int r);
    int HeapSort(int* lst);
    void constructHeap(int* lst);
    void heapify(int root);
    void fixHeap(int root);
    void deleteMax(); // return the maxvalue

    //utility functions
    void randList();
    void ascendingList();
    void descendingList();
    void lswap(int pos1, int posj);
    void DeepCopy(int* lst);    //copy input list to the result list
    void printSorted();
    int* lafter;    //list after sorting
    int* lrand;     //random list
    int* lasce;     //ascending list
    int* ldesc;     //descending list
    int* heap;
    int size;
    int heapsize;
};

#endif //SORTING_SORTINGALGO_H
