#include <stdio.h>

int main() {

    int n;
    int a = 124; //проверка
    int s = 0;
    int k;

    for (n = a; n != 0; n = n / 10) {
        k = n % 10;
        s = s + k;
    }

    printf("%d\n", s);

    return 0;

}

//Сумма цифр числа