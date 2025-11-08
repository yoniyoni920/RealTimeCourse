#include <stdio.h>
#include <dos.h>

unsigned long int watchptr_last_value;
unsigned long int *curr_watchptr;
char* curr_printf_str;

void interrupt (*isp_1_save) (void);
void set_watch(char*, unsigned long int*);
void interrupt watch_var_single_step(void);
int compute_factors(unsigned long, unsigned long[]);
void end_watch();

void main(void) {
    int i, j;
    unsigned long int n, array[32];

    printf("\nEnter n:\n");
    scanf("%lu", &n);
    printf("n = %lu\n", n);
    j = compute_factors(n, array);
    printf("factors:\n");

    for(i=0; i < j; i++)
        printf(" %lu ", array[i]);
}

int compute_factors(unsigned long int n, unsigned long int factors[]) {
    unsigned long int i = 2;
    int j = 0;

    set_watch("debugger message: n = %lu\n", &n);

    factors[j++] = 1;

    for(i=2; i <= (n/i); i++) {
        while((n % i) == 0) {
            n = n/i; // Oddly div function causes some weird corruption on my end (Tested on DOSBox X)
            factors[j++] = i;
        }
    }

    if (n != 1)
        factors[j++] = n;

    end_watch();
    return j;
}

// Interrupt code
void interrupt watch_var_single_step(void) {
    if (curr_watchptr != NULL && watchptr_last_value != *curr_watchptr) {
        printf(curr_printf_str, *curr_watchptr);
    }
    watchptr_last_value = *curr_watchptr;
}

void set_watch(char* printf_str, unsigned long int *watchptr){
    isp_1_save = getvect(1); //Preserve old pointer
    setvect(1, watch_var_single_step); // Set entry to new handler

    curr_watchptr = watchptr;
    watchptr_last_value = *watchptr; // Save last value
    curr_printf_str = printf_str;

    _FLAGS = _FLAGS | 0x100; // Turn on TF
}

void end_watch() {
    _FLAGS = _FLAGS & 0xFEFF; //Turn off TF
    curr_printf_str = NULL;
    curr_watchptr = NULL;

    setvect(1, isp_1_save); /* Set entry to original handler */
}
// Interrupt code