#include <stdio.h>

int main() {

    int n;

    printf("Введите N: ");
    scanf("%d", &n);

    int number = 1;

    for (int i = 1; i <= n; i++) { 
        for (int j = 1; j <= n; j++) {     

            printf("%d ", number);
            number++;

        }

        printf("\n");

    }

    return 0;
}