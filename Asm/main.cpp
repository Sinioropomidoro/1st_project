#include <iostream>

int add_sub(int a, int b, int c) {
    int result;

    __asm__ (
        "movl %2, %%eax;" 
        "addl %1, %%eax;" 
        "subl %3, %%eax;" 
        : "=a" (result)      
        : "b" (b),           
          "c" (a),           
          "d" (c)            
        : "cc"               
    );

    return result; 
}

int main() {
    int a = 10;
    int b = 5;
    int c = 3;

    int res = add_sub(a, b, c);

    std::cout << "Результат: " << res << std::endl;

    return 0;
}
