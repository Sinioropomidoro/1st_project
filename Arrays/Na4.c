#include <stdio.h>
#include <stdlib.h>

#define N 10

int main(void) {
    int re[N];
    int im[N];

    for (int i = 0; i < N; i++) {
        re[i] = rand() % 65536 - 32768;
        im[i] = rand() % 65536 - 32768;
    }

    for (int i = 0; i < N; i++) {
        printf("s[%d] = %.1f + j*(%.1f)\n", i, (double)re[i], (double)im[i]);
    }

    long long energy = 0;
    for (int k = 0; k < N; k++) {
        energy += (long long)re[k] * re[k] + (long long)im[k] * im[k];
    }
    printf("E = %lld\n", energy);

    int maxRe = re[0];
    int maxIndex = 0;
    for (int k = 1; k < N; k++) {
        if (re[k] > maxRe) {
            maxRe = re[k];
            maxIndex = k;
        }
    }
    printf("max re = %d, index = %d\n", maxRe, maxIndex);

    return 0;
}