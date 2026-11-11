#include <stdio.h>
#include <limits.h>

int main() {
    // 1. Эксперимент с int
    int max = 2147483647; // Максимальное для 4 байт
    max = max + 1;
    printf("Результат int (max + 1): %d\n", max);

    // 2. Эксперимент с unsigned int
    unsigned int u_it = 0;
    u_it = u_it - 1;
    printf("Результат unsigned int (0 - 1): %u\n", u_it);

    // 3. Эксперимент с char
    char min_chr = -128; // Минимальное для 1 байта
    min_chr = min_chr - 1;
    printf("Результат char (min - 1): %hhd\n", min_chr);

    return 0;
}
