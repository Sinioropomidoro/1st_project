#include <stdio.h>

int main() {

    int n;
    int a=492; //проверка
    int s=0;
    int k;

    for (n = a; n != 0; n = n/10){
        k = n % 10;
        if (k > s) {
            s = k;
        }
    }

    printf("Максимальная цифра в числе %d — это %d\n", a, s);

    return 0;
}

//Максимальная цифра числа