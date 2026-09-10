#include <stdio.h>

int main() {
	int a = 555;
	int w = 59;
	int l = 1004;
	int di = 8;
	double a2 = 0.1;
	double b = 0.2;
	double sum = a2 + b;

	printf("|%d|\n", a);
	printf("|%10d|\n", a);
	printf("|%-10d|\n\n", a);

	printf("Words: %10d\n", w);
	printf("Letters: %8d\n", l);
	printf("Digits: %9d\n\n", di);

	printf("a = %6f; b = %6f\n", a2, b);
	printf("a + b = %.17f\n", sum);

	return 0;
}

