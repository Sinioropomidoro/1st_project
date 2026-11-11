#include <stdio.h>
#include <string.h>

int main(void) {
    char card[] = "4276123456789012";

    int len = strlen(card);
    int hidden = 0;                 

    for (int i = 0; i < len; i++) {
        if (i < len - 4) {
            putchar('*');
            hidden++;
        } else {
            putchar(card[i]);
        }
    }
    printf("\n");

    printf("Закрыто цифр: %d\n", hidden);

    return 0;
}