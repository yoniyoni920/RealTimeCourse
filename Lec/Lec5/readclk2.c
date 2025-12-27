/* readclk2.c */

#include <stdio.h>
#include <dos.h>

int  convert_to_binary(int x)
{
 int i;
 int temp, scale, result;
 

  temp =0;
  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  result = temp;
  temp = 0;

  scale = 1;
  for(i=0; i < 4; i++)
   {
     temp = temp + (x % 2)*scale;
     scale *= 2;
     x = x >> 1;
   } // for

  temp *= 10;
  result = temp + result;
  return result;

} // convert_to_binary

void readclk(char str[])
{
  int i;
  int hour, min, sec;
  char al;

  hour = min = sec = 0;

  outportb(0x70, 0);
  sec = inportb(0x71);



  outportb(0x70, 2);
  min = inportb(0x71);


  outportb(0x70, 4);
  hour = inportb(0x71);



  sec = convert_to_binary(sec);
  min = convert_to_binary(min);
  hour = convert_to_binary(hour);

  sprintf(str,"%02d:%02d:%02d", hour, min, sec);

} // readclk

int main()
{
  char str[16], al;
  int flag = 1;
  struct REGPACK regpack;
  union REGS regs;


  while(flag)
  {
    putchar(13);
    readclk(str);
    printf(str);

regpack.r_ax = 256;
intr(0x16, &regpack);
flag = ((regpack.r_flags & 64) != 0);
    

  } // while


  al = getchar();

return 0;

}  // main
