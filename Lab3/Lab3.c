#include <dos.h>

volatile int uei_count = 0;
volatile unsigned long int current_count = 0;

void interrupt (*old0x70isr)(void);
void interrupt (*old0x08isr)(void);

void interrupt new0x70isr(void)
{
    char port70hsave, port71hsave;

    uei_count++;

    // Acknowledge the RTC interrupt by reading Register C
    port70hsave = inportb(0x70);
    port71hsave = inportb(0x71);
    outportb(0x70, 0xC);
    inportb(0x71);
    outportb(0x70, port70hsave);
    outportb(0x71, port71hsave);

    // Sends End-of-Interrupt to slave and master PIC
    outportb(0xA0, 0x20);
    outportb(0x20, 0x20);
} // new0x70isr

void interrupt new0x08isr(void) {
    if (uei_count == 1) {
        current_count++;
    }
    old0x08isr(); // Run original ISR8
}

unsigned long int count8_latch(int latch)
{
    char al;

    // Reset counters
    current_count = 0;
    uei_count = 0;
    
    // Modify latch
    outportb(0x43, 0x34); // mode 2, counter 0
    outportb(0x40, latch & 0xFF); // For low
    outportb(0x40, latch >> 8); // Shift 8 bits to the right to high

    // Override ISRs
    old0x70isr = getvect(0x70);
    setvect(0x70, new0x70isr);
    old0x08isr = getvect(0x08);
    setvect(0x08, new0x08isr);

    // Read/write to status B
    inportb(0x70);
    outportb(0x70, 0xB);
    outportb(0x70, 0x8B);
    al = inportb(0x71); // Read from status B
    al = al & 0x8F; // 10001111 Turn off periodic, alarm and update interrupts
    al = al | 0x12; // 00010010 Turn on update interrupt with 24h
    outportb(0x71, al);

    // All of this might be optional, but might not be optional in real DOS machines
    inportb(0x71);
    al = inportb(0x21); // Master pic thing
    al = al & 0xFB;
    outportb(0x21, al);

    while(uei_count < 2); // Wait until 2 RTC updates happen

    // Restore ISRs
    setvect(0x70, old0x70isr);
    setvect(0x08, old0x08isr);

    return current_count;
}

int main() {
    char str[16];
    unsigned int i, latch = 1;
    unsigned int initial_latch, increment, no_of_times;
    unsigned long int* counters;

    printf("\nEnter initial latch value, increment value, number of times\n");
    scanf("%u %u %u",  &initial_latch, &increment, &no_of_times);   

    counters = malloc(sizeof(unsigned long int) * no_of_times);

    for(i=0; i < no_of_times; i++)
    {
        latch = initial_latch +i*increment;  
        counters[i] = count8_latch(latch);
    } // for

    for(i=0; i < no_of_times; i++)
        printf("latch %u = %lu\n", initial_latch +i*increment, counters[i]);

    free(counters);

    return 0;
}
