#include "ListSortingAlgo.h"
int main()
{
	int list_size = 1000000;

	ListSortingAlgo quick(list_size);
	//Execute and time Merge Sort
	auto begin_exe = std::chrono::high_resolution_clock::now();\
	quick.QuickSort(0, list_size-1);
	auto end_exe = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_exe - begin_exe);
	std::string check = quick.CheckCorrectness()? "Correct. ": "ERROR!!!";
	std::cout <<"QuickSort:" << check << std::endl;
	printf("%d elements: %d microseconds\n", list_size, duration.count());	
	ListSortingAlgo merge(list_size);

	//Execute and time Merge Sort
	begin_exe = std::chrono::high_resolution_clock::now();
	merge.MergeSort(0, list_size-1);
	end_exe = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end_exe - begin_exe);
	check = merge.CheckCorrectness()? "Correct. ": "ERROR!!!";
	std::cout << "MergeSort:" << check << std::endl;
	printf("%d elements: %d microseconds\n", list_size, duration.count());

	ListSortingAlgo insertion(list_size);
	//Execute and time Insertion Sort
	begin_exe = std::chrono::high_resolution_clock::now();
	insertion.InsertionSort();
	end_exe = std::chrono::high_resolution_clock::now();
	duration = std::chrono::duration_cast<std::chrono::microseconds>(end_exe - begin_exe);
	check = insertion.CheckCorrectness()? "Correct. ": "ERROR!!!";
	std::cout <<"InsertionSort:" << check << std::endl;
	printf("%d elements: %d microseconds\n", list_size, duration.count());
}
