#include <stdio.h>

int main(void) {
    unsigned int a;
    scanf("%u", &a);

    // сначала чётные индексы: 0, 2, 4, ...
    for (int i = 0; i < 32; i += 2) {
        printf("%u ", (a >> i) & 1);
    }
    printf("\n");

    // затем нечётные: 1, 3, 5, ...
    for (int i = 1; i < 32; i += 2) {
        printf("%u ", (a >> i) & 1);
    }
    printf("\n");

    return 0;
}
