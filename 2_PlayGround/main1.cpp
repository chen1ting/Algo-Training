#include <iostream>
#define NUM_DIGITS 11
using namespace std;
int main(void)
{
    int t, sum;
    long pesel;
    int wt[NUM_DIGITS] = {1,3,7,9,1,3,7,9,1,3,1};
    cin >> t;
    for(int times = 0; times < t; times++){
        cin >> pesel;
        sum = 0;
        for(int i = 1; i <= NUM_DIGITS; i++){
            sum += (pesel % 10) * wt[NUM_DIGITS - i];
            pesel /=10;
        }
        char output = (sum%10) == 0? 'Y':'N';
        cout << output << endl;
    }
    return 0;
}
