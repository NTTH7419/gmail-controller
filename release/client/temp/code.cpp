#include <iostream>

using namespace std;

int calculateSum(int n){
    int sum = 0;
    for (int i = 0; i < n; i++){
        sum += n;
    }

    return sum;
}

int factorial(int n){
    if (n == 0 || n == 1) return 1;

    return factorial(n-1) + factorial(n-2);
}

