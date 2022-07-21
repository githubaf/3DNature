/*
gcc -Wall version_test.c -o version_test_linux
m68k-amigaos-gcc -Wall version_test.c -noixemul -o version_test_amiga -lm
sc version_test LINK MATH=STANDARD
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


unsigned char VersionBytes[4]={0x3f,0x82,0x8f,0x5c};    // 1.02 as read from file

float byteToFloat(unsigned char* arr)
{
   int i= 0x5c | 0x8f<<8 | 0x82 << 16 | 0x3f << 24;
   return *(float*) &i;
}

int main(void)
{
   float Version=byteToFloat(VersionBytes);

   printf("Version=%f\n",Version);

   printf("---- Test with fabs() -------\n");

   printf("abs(Version-1.00)=%f\n",(abs(Version - 1.00)));

   if((abs(Version - 1.00) < .0001))  // version-check behaves differently on gcc and SAS/C
   {
      printf("abs Diff < .0001\n");
   }
   else
   {
      printf("abs Diff>=.0001\n"); 
   }

   printf("---- Test with fabs() -------\n");

   printf("fabs(Version-1.00)=%f\n",(fabs(Version - 1.00)));
   
   if((fabs(Version - 1.00) < .0001))
   {  
      printf("fabs Diff < .0001\n");
   }
   else
   {
      printf("fabs Diff>=.0001\n");
   }


   return 0;
}
