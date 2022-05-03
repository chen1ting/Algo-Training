#include "SortingAlgo.h"

int main() {
    int opt;
    int n;
    while(true){
        cout<<"Enter List Size (enter negative integer to exit):\n";
        cin>>n;
        if(n<1){
            cout<<"Negative Integer detected, program stopping...";
            return 0;
        }
        SortingAlgo list(n);
        list.optChoice();
    }
}