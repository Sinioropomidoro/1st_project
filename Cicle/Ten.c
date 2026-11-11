#include <stdio.h>

int main() {

    int n;
    int a = 1234; //проверка
    int s=0;
    int k;

    for (n=a; n!=0; n=n/10) {
        k=n%10;
        s=s*10+k;
    }
    
    printf("Реверс числа: %d\n", s);

    return 0;

}

// Реверс числа 1234 = 4321