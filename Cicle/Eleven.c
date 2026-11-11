#include <stdio.h>

int main(){
    
    int n=15; //проверка
    int k=0;
    int m=1;


    for (m = 0; m <= n; m = m * 2) { 
        k++;

    }

    printf("%d\n", k - 1); 

    return 0;

}