#include <stdio.h>

void bubble_sort(int arr[], int n) {
    for (int i = 0; i < n - 1; i++) {
        int swapped = 0;
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int tmp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = tmp;
                swapped = 1;
            }
        }
        if (!swapped) break;
    }
}

int main(void) {
    int a[] = {4, 0};
    int n = sizeof(a) / sizeof(a[0]);

    bubble_sort(a, n);

    for (int i = 0; i < n; i++) {
        printf("%d ", a[i]);
    }
    printf("\n");
    
    return 0;
}


//Добавить комментарий с любым текстом в этот же файл
//Измените файл еще раз (можно добавить еще комментарий или убрать старый)

