#include <stdio.h>
#include <dos.h>
#include <conio.h>
volatile int rtc_tick = 0;
volatile unsigned long count;
void interrupt (*int8save)(void);
void interrupt newint8(void);
void interrupt new0x70isr(void);
unsigned long counters[50];
unsigned long int count8_latch(int latch)
{
    unsigned long int result;
    void interrupt (*oldint8)(void);
    void interrupt (*old0x70)(void);
    char old_reg_b;

    
    oldint8 = getvect(8);
    setvect(8, newint8);

    old0x70 = getvect(0x70);
    setvect(0x70, new0x70isr);

   
    outportb(0x43, 0x16);      
    outportb(0x40, latch & 0xFF);
    outportb(0x43, 0x26);      
    outportb(0x40, latch >> 8);

  
    outportb(0x70, 0x8B);        
    old_reg_b = inportb(0x71);   
    outportb(0x70, 0x8B);
    outportb(0x71, old_reg_b | 0x12); 


    outportb(0x70, 0x0C);
    inportb(0x71);


    
 
    rtc_tick = 0;
    while(rtc_tick == 0); 
    

    count = 0;
    rtc_tick = 0;
    
 
    while(rtc_tick == 0);
    
 
    result = count;


 
    outportb(0x43, 0x16);
    outportb(0x40, 0);
    outportb(0x43, 0x26);
    outportb(0x40, 0);

    
    outportb(0x70, 0x8B);
    outportb(0x71, old_reg_b);

    setvect(8, oldint8);
    setvect(0x70, old0x70);

    return result;
}
void interrupt newint8(void)
{
   count++;             
   outportb(0x20, 0x20);

}

void interrupt new0x70isr(void)
{
    rtc_tick = 1;
    outportb(0x70, 0x0C); 
    inportb(0x71);        


    outportb(0xA0, 0x20); 
    outportb(0x20, 0x20);
int main()
{
  char str[16];
  unsigned int i, latch = 1;
  unsigned int initial_latch, increment, no_of_times;



printf("\nEnter initial latch value, increment value, number of times\n");
scanf("%u %u %u",  &initial_latch, &increment, &no_of_times);   

  for(i=0; i < no_of_times; i++)
  {
    latch = initial_latch +i*increment;  
    counters[i] = count8_latch(latch);

  } // for

    for(i=0; i < no_of_times; i++)
      printf("latch %u = %lu\n", initial_latch +i*increment, counters[i]);

 return 0;
}  // main
