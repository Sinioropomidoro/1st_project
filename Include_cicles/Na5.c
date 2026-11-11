#include <stdio.h>

int main() {
    int n;
    printf("Введите N: ");
    scanf("%d", &n);

    for (int i = 0; i < n; i++) {          
        char letter = 'A' + i;              
        int count = n - i;                 

        for (int k = 0; k < i; k++) {
            printf(" ");
        }

        for (int j = 0; j < count; j++) { 
            printf("%c", letter);
        }
        printf("\n");
    }

    return 0;
}