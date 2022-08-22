//
// Created by 陈一婷 on 3/5/22.
//

#include <iomanip>
#include "SortingAlgo.h"

SortingAlgo::SortingAlgo(int n) {
    size = n;
    lafter = (int*)malloc(sizeof(int)*size);
    lrand = (int*)malloc(sizeof(int)*size);
    lasce = (int*)malloc(sizeof(int)*size);
    ldesc = (int*)malloc(sizeof(int)*size);
    heap = (int*)malloc(sizeof(int)*(size+1));
    randList();
    ascendingList();
    descendingList();
}

SortingAlgo::~SortingAlgo() {
    free(lafter);
    free(lrand);
    free(lasce);
    free(ldesc);
    free(heap);
}

void SortingAlgo::randList() {
    srand(time(NULL));
    for(int i=0;i<size;i++)
        lrand[i] = rand() % size;
    cout << "Random List Generated!" << endl << "First 10 Elements:\t";
    for(int i=0;i<10;i++)
        printf("%6d", lrand[i]);
    cout<<endl;
}

void SortingAlgo::ascendingList() {
    for(int i=0;i<size;i++)
        lasce[i] = i;
    cout << "Ascending List Generated!" << endl << "First 10 Elements:";
    for(int i=0;i<10;i++)
        printf("%6d", lasce[i]);
    cout<<endl;
}

void SortingAlgo::descendingList() {
    for(int i=0;i<size;i++)
        ldesc[i] = size-i;
    cout << "Descending List Generated!" << endl << "First 10 Elements:";
    for(int i=0;i<10;i++)
        printf("%6d", ldesc[i]);
    cout<<endl;
}

void SortingAlgo::lswap(int pos1, int pos2){
    int tmp = lafter[pos1];
    lafter[pos1] = lafter[pos2];
    lafter[pos2] = tmp;
}

void SortingAlgo::DeepCopy(int* lst) {
    for(int i=0;i<size;i++)
        lafter[i] = lst[i];
};

void SortingAlgo::printSorted(){
    cout<<"List Sorted!\nFirst 10 Elements:";
    for(int i=0;i<10;i++)
        printf("%6d", lafter[i]);
    cout<<endl;
}
void SortingAlgo::optChoice() {
    int case_type;
    int opt;
    while(true) {
        cout << "Choose Sorting Algo:\n1. Insertion\n2. MergeSort\n3. QuickSort\n4. HeapSort\n0. Re-enter size\n";
        cin >> opt;
        int best_time, worst_time, ave_time;
        switch (opt) {
            case 0:
                return;
            case 1:
                best_time = InsertionSort(lasce);//O(n)
                worst_time = InsertionSort(ldesc);//O(n^2)
                ave_time = InsertionSort(lrand);//O(n^2)
                printSorted();
                printf("[Insertion Sort of size %d]: Best Case:%d; Worst Case:%d; Average Case:%d\n", size, best_time, worst_time, ave_time);
                break;
            case 2:
                best_time = MergeSort(lasce);//O(nlgn)
                worst_time = MergeSort(ldesc);//O(nlgn)
                ave_time = MergeSort(lrand);//O(nlgn)
                printSorted();
                printf("[Merge Sort of size %d]: Best Case:%d; Worst Case:%d; Average Case:%d\n", size, best_time, worst_time, ave_time);
                break;
            case 3:
                ave_time = QuickSort(lrand);//O(nlgn)
                printSorted();
                printf("[Quick Sort of size %d]: Average Case:%d\n", size, ave_time);
                break;
            case 4:
                ave_time = HeapSort(lrand);//O(nlgn)
                for(int i=1;i<=size;i++)
                    printf("%6d", heap[i]);
                printf("\n[Heap Sort of size %d]: Average Case:%d\n", size, ave_time);
                break;
            default:
                cout<<"Invalid Option"<<endl;break;
        }
    }
}


int SortingAlgo::InsertionSort(int* lst) {
    DeepCopy(lst);
    auto start = chrono::high_resolution_clock::now();
    for(int i=1;i<size;i++){
        for(int j=i; j>0;j--){
            if(lafter[j]<lafter[j-1]){
                lswap(j-1, j);
            }
            else
                break;
        }
    }
    return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count();
}

int SortingAlgo::MergeSort(int *lst) {
    DeepCopy(lst);
    auto start = chrono::high_resolution_clock::now();
    mergesort(0, size-1);
    return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count();
}

void SortingAlgo::mergesort(int l, int r) {
    if(l>=r)
        return;
    if(r-l>1){
        int mid = (l+r)/2;
        mergesort(l, mid);
        mergesort(mid+1, r);
    }
    merge(l, r);
}

void SortingAlgo::merge(int l, int r) {
    int mid = (l+r)/2;
    int l1 = l, l2 = mid+1, tmp;
    while(l1<=mid && l2 <=r){//while both list are not traversed through
        if(lafter[l1]<lafter[l2])
            l1++;
        else if(lafter[l1]>lafter[l2]){
            tmp = lafter[l2++];
            for(int i=++mid; i>l1; i--)
                lafter[i] = lafter[i-1];
            lafter[l1++] = tmp;
        }
        else{
            if(l1 == mid && l2 == r)
                break;
            tmp = lafter[l2++];
            l1++;
            for(int i=++mid; i>l1; i--)
                lafter[i] = lafter[i-1];
            lafter[l1++] = tmp;
        }
    }
}

int SortingAlgo::QuickSort(int *lst) {
    DeepCopy(lst);
    auto start = chrono::high_resolution_clock::now();
    quicksort(0, size-1);
    return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count();
}

void SortingAlgo::quicksort(int l, int r) {
    if(l>=r)
        return;
    int pivot_pos = partition(l, r);
    quicksort(l, pivot_pos-1);
    quicksort(pivot_pos+1, r);
}

int SortingAlgo::partition(int l, int r) {
    lswap(l, (l+r)/2);
    int lst_small = l, pivot_val = lafter[l];
    for(int i=l+1;i<=r;i++){
        if(lafter[i]<=pivot_val)
            lswap(++lst_small, i);
    }
    lswap(l, lst_small);
    return lst_small;
}

int SortingAlgo::HeapSort(int *lst) {
    auto start = chrono::high_resolution_clock::now();
    constructHeap(lst);

    for(int i=size; i>0; i--)
        deleteMax();
    return chrono::duration_cast<chrono::microseconds>(chrono::high_resolution_clock::now() - start).count();
}

void SortingAlgo::constructHeap(int* lst){
    heapsize = size;
    for(int i=0;i<size;i++)
        heap[i+1]=lst[i];
    heapify(1);
}

void SortingAlgo::heapify(int root){
    if(root*2>heapsize)//passed-in root is actually a leaf
        return;
    heapify(2*root);//left child
    heapify(2*root+1);//right child
    fixHeap(root);
}

void SortingAlgo::fixHeap(int root) {
    //iterative method
    int node = root, child = root*2, value = heap[root];
    while(child<=heapsize){
        if(child<heapsize && heap[child]<heap[child+1]) //if right child bigger than the left child
            child++;
        if(value >= heap[child])
            break;
        heap[node] = heap[child];
        node = child;   //moving to the next level
        child *= 2;
    }
    heap[node] = value;
}

void SortingAlgo::deleteMax() {
    //swap value of max with right lowest node
    int max = heap[1];
    heap[1] = heap[heapsize];
    heap[heapsize--]=max;
    fixHeap(1);
}



