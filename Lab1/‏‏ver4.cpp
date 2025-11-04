#include <iostream>
#include <math.h>
int test_prime(unsigned long long int n,unsigned long long int* k,unsigned long* count);
// TIP To <b>Run</b> code, press <shortcut actionId="Run"/> or click the <icon src="AllIcons.Actions.Execute"/> icon in the gutter.
int main(void)
{
    unsigned long long int n, k;
    int is_prime;
    unsigned long count;

    printf("Enter unsigned integer:\n");
    scanf("%Lu", &n);

    system("time");

    is_prime = test_prime(n,&k, &count	);

    system("time");

    if (is_prime == 1)
        printf("%Lu IS a prime\n", n);
    else
    {
        printf("%Lu is NOT a prime\n", n);
        printf("%Lu = %Lu * %Lu\n", n, k, n/k);
    } /* else */

    printf("count = %lu\n", count);

    return 0;
} /* main */

//forth version deterministic counts the amount if checks and
//prints the amount to show the algo is deterministic
int test_prime(unsigned long long int n,unsigned long long int* k,unsigned long* count) {
    *count = 0 ;
    int is_prime = true;
    if (n == 2 || n == 3){*k = 1;}
    *count = 1;
    if (n % 2 == 0 ) { *k = 2;  is_prime = false; }
    *count = 2;
    if (n % 3 == 0 && (is_prime)) { *k = 3;  is_prime = false; }
    *count = 3;
    int sqrt_n = sqrt(n);

    for (unsigned long long i = 5; i <= sqrt_n; i += 6) {
        if (n % i == 0 && (is_prime)) { *k = i; is_prime = false; }
        (*count)++;  // for i
        if (n % (i + 2) == 0 && (is_prime)) { *k = i + 2; is_prime = false; }
        (*count)++;  // for i + 2
    }

    return is_prime;
}