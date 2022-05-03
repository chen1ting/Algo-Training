#include<iostream>
using namespace std;

int unlimitedKnapSack(int* wt, int* p, int max_wt, int item_type){
    int* prft = new int[max_wt+1];
    prft[0] = 0;
    for(int i=1;i<=max_wt;i++){
        prft[i] = prft[i-1];
        for(int j = 0; j < item_type; j++){
            if(wt[j] <= i){
                int take_j = p[j]+prft[i-wt[j]];
                if( take_j > prft[i])
                    prft[i] = take_j;
            }
        }
    }
    return prft[max_wt];
}

int main(){
    int max_c = 14;
    int n = 3;

    int wt1[] = {4, 6, 8};
    int p1[] = {7, 6, 9};

    int wt2[] = {5, 6, 8};
    int p2[] = {7, 6, 9};

    cout<<unlimitedKnapSack(wt1, p1, max_c, n)<<endl;
    cout<<unlimitedKnapSack(wt2, p2, max_c, n)<<endl;

}