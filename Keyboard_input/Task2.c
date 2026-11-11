#include <stdio.h>

int main() {
    float tmpr;
    char scale;

    printf("Input temperature: ");

    if (scanf("%f%c", &tmpr, &scale) != 2) {
        return 1;
    }

    double fg;
    double c;

    if ((scale == 'C') || (scale == 'c')) {
        fg = ((tmpr * 1.77) + 32);
        printf("Ouput temperature: %.1ff\n", fg);
    } else if ((scale == 'f') || (scale == 'F')) {
        c = ((tmpr - 32) * 5/9);
        printf("Ouput temperature: %.1fc\n", c);
    } else {
        printf("Ouput error\n");
    }
    
    return 0;
}
