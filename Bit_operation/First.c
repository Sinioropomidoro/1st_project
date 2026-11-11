#include <stdio.h>

int main() {
    unsigned int a;
    printf("Введите число a: ");
    scanf("%u", &a);

    int first = 0, last = 0;
    int n = sizeof(a) * 8;

    for (int i = 0; i < n; i++) {
        if (a & (1u << i)) {
            if (first == 0) {
                first = i + 1;
            }
            last = i + 1;
        }
    }

    if (first == 0) {
        printf("В числе нет единичных битов\n");
    } else {
        printf("Первый справа единичный бит: %d\n", first);
    }

    return 0;
}