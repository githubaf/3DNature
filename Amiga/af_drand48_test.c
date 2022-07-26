/*
test consecutive calls to drand48()
 SAS/C behaves differently from gcc!

gcc af_drand48_test.c -o af_drand48_test_linux
m68k-amigaos-gcc af_drand48_test.c -noixemul -o af_drand48_test_amiga -lm
sc af_drand48_test.c LINK MATH=STANDARD TO af_drand48_test_sasc 

*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>


char *PrintSeedBuff(void)
{
   static char Buffer[256];
   unsigned short SeedBuf[3]={0,0,0};
   unsigned short *OldSeedBuf; 

   OldSeedBuf=seed48(SeedBuf);
   sprintf(Buffer,"x[0]=0x%04hx x[1]=0x%04hx x[2]=0x%04hx",OldSeedBuf[0],OldSeedBuf[1],OldSeedBuf[2]);

   SeedBuf[0]=OldSeedBuf[0];
   SeedBuf[1]=OldSeedBuf[1];
   SeedBuf[2]=OldSeedBuf[2];

   seed48(SeedBuf);  /* reset to current value */
   return Buffer;
}


int main(void)
{

   printf("--- initial seed ---\n");
   printf("%f (sas/c soll: 0.655280) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.833024) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.451051) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.487438) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.449567) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.840612) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.781184) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.217187) %s\n",drand48(), PrintSeedBuff());

   printf("--- 11111 ---\n");

   srand48(11111);

   printf("%f (sas/c soll: 0.655280) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.833024) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.451051) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.487438) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.449567) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.840612) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.781184) %s\n",drand48(), PrintSeedBuff());
   printf("%f (sas/c soll: 0.217187) %s\n",drand48(), PrintSeedBuff());
   return 0;
}

