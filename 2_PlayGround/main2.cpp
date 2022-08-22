#include <iostream>
using namespace std;

int MinRemove(int arr[], int n){
    int rem[n], len = 0;
    //initializing
    for (int i = 0; i < n; i++)
        rem[i] = 1;

    // Find LIS of array
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (arr[i] > arr[j]
                && (i - j) <= (arr[i] - arr[j])) {
                rem[i] = max(rem[i], rem[j] + 1);
            }
        }
        len = max(len, rem[i]);
    }

    // Return min changes for array to strictly increasing
    return n - len;
}

int main()
{
    int num_q, num_ppl;
    cin >> num_q;
    for(int iter_q = 0; iter_q < num_q; iter_q++){
        cin >> num_ppl;
        int* line = new int[ppl];
        for(int iter_p = 0; iter_p < num_ppl; iter_p++){
            cin >> line[iter_p];
        }
        cout << MinRemove(line, num_ppl) << endl;
        delete[] line;
    }
    return 0;
}

// This code is contributed by Sania Kumari Gupta
