/*
#AF,selco, HGW. 28July 2022
# Wrong result 9 with -ffast-math
m68k-amigaos-gcc af_fast_math_test.c -noixemul -O0 -mregparm -ffast-math -fbaserel -fomit-frame-pointer -o af_fast_math_test_amiga -lm && vamos test_amiga
gcc af_fast_math_test.c -O0 -ffast-math -fomit-frame-pointer -o af_fast_math_test_linux && ./af_fast_math_test_linux

#correct resut 10 without -ffast-math
m68k-amigaos-gcc af_fast_math_test.c -noixemul -O0 -mregparm -fbaserel -fomit-frame-pointer -o af_fast_math_test_amiga -lm && vamos af_fast_math_test_amiga
gcc af_fast_math_test.c -O0 -fomit-frame-pointer -o af_fast_math_test_linux && ./af_fast_math_test_linux

*/

#include <stdio.h>
#include <string.h>

int main(void)
{

   double sunshade;
   short Red;

   unsigned long long HexValue=0x3fee666666666666;  /* 0.95  */
   memcpy(&sunshade, &HexValue, sizeof(double));
   Red=200;
   printf("Resultat  %hd (erwarte 10)\n",Red -= (sunshade * Red));
   
   printf("\n");

   HexValue=0x3fee666666666667;  /* 0.9500000000000001  */
   memcpy(&sunshade, &HexValue, sizeof(double));
   Red=200;
   printf("Red -=        (sunshade * Red) %hd (erwarte 10)\n",Red -= (sunshade * Red));

   Red=200;
   printf("Red  =  Red - (sunshade * Red) %hd (erwarte 10)\n",Red = Red - (sunshade * Red));

   Red=200;
   printf("Red -= (short)(sunshade * Red) %hd (erwarte 10)\n",Red -= (short)(sunshade * Red));

   return 0;
}
