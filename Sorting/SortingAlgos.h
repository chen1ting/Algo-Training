//
// Created by 陈一婷 on 3/5/22.
//

#ifndef SORTING_SORTINGALGOS_H
#define SORTING_SORTINGALGOS_H


class SortingAlgos {
public:
    int InsertionSort();    //sort the list in the class, return runtime (milliseconds)

private:
    int* lbefore;   //list before sorting
    int* lafter;    //list after sorting
    int size;
};


#endif //SORTING_SORTINGALGOS_H
