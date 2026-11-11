#include <stdio.h>
#include <math.h>

double det(double m[3][3], int n)
{
    if (n == 2) {
        return m[0][0] * m[1][1] - m[0][1] * m[1][0];
    }

    return m[0][0] * (m[1][1] * m[2][2] - m[1][2] * m[2][1])
         - m[0][1] * (m[1][0] * m[2][2] - m[1][2] * m[2][0])
         + m[0][2] * (m[1][0] * m[2][1] - m[1][1] * m[2][0]);
}

int main(void)
{
    int n;
    double a[3][3]; 
    double b[3];      
    double x[3];

    printf("Введите N (2 или 3): ");
    scanf("%d", &n);
    if (n < 2 || n > 3) {
        printf("N должно быть 2 или 3\n");
        return 1;
    }

    printf("Введите строки: коэффициенты и свободный член через пробел\n");
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            scanf("%lf", &a[i][j]);
        }
        scanf("%lf", &b[i]);
    }

    double d = det(a, n);

    if (fabs(d) < 1e-9) {
        printf("Нет единственного решения.\n");
        return 0;
    }

    for (int k = 0; k < n; k++) {
        double t[3][3];

        for (int i = 0; i < n; i++)
            for (int j = 0; j < n; j++)
                t[i][j] = a[i][j];

        for (int i = 0; i < n; i++)
            t[i][k] = b[i];

        x[k] = det(t, n) / d;
    }

    for (int k = 0; k < n; k++) {
        printf("x%d = %g\n", k + 1, x[k]);
    }

    return 0;
}