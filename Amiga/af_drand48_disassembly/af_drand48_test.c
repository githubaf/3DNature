/*
Zum Disassemblieren...
sc af_drand48_test.c TO af_drand48_test_sasc LINK DEBUG=FULLFLUSH MATH=STANDARD 
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

int main(void)
{
   volatile double x,y;
   srand48(11111);

   x=drand48();

   return 0;
}

