#include <stdio.h>

int main() {
    int year;

    printf("Введите год: ");

    if (scanf("%d", &year) != 1) {
        return 1;
    }

    if (year % 400 == 0) {
        printf("YES\n");
    } else if (year % 100 == 0) {
        printf("NO\n");
    } else if (year % 4 == 0) {
        printf("YES\n");
    } else {
        printf("NO\n");
    }

    return 0;
}
