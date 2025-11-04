#include <iostream>
#include <math.h>
int test_prime(unsigned long long int n,unsigned long long int* k);
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main(void)
{
    unsigned long long int n, k;
    int is_prime;


    printf("Enter unsigned integer:\n");
    scanf("%Lu", &n);

    system("time");

    is_prime = test_prime(n,&k);

    system("time");

    if (is_prime == 1)
        printf("%Lu IS a prime\n", n);
    else
    {
        printf("%Lu is NOT a prime\n", n);
        printf("%Lu = %Lu * %Lu\n", n, k, n/k);
    } /* else */

    return 0;
} /* main */
//first version stoppes if prime
int test_prime(unsigned long long int n,unsigned long long int* k) {

    if (n == 2 || n == 3)
        return true;
    if (n % 2 == 0) { *k = 2;  return false; }
    if (n % 3 == 0) { *k = 3;  return false; }
    int sqrt_n = sqrt(n);

    for (unsigned long long i = 5; i <= sqrt_n; i += 6) {
        if (n % i == 0) { *k = i; return false; }
        if (n % (i + 2) == 0) { *k = i + 2; return false; }
    }

    return true;
}