#include <stdio.h>

int main() {

    int s;
    int i;
    int n = 5;

    for (s=1, i=1; i<=n; i++) {
        s = s * i;
    }

    printf("Результат: %d\n", s);

    return 0;
}

// Факториал числа 