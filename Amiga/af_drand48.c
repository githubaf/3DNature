/*
AF, selco, 25. Juli 2022,HGW
af_drand48() behaves on SAS/C differently than on gcc/m68k-amigaos-gcc
Even m68k-amigaos-gcc with and without -noixemul result in different values.

gcc af_drand48.c -DSASC_DRAND48_TEST -o af_drand48_linux && ./af_drand48_linux
m68k-amigaos-gcc af_drand48.c -DSASC_DRAND48_TEST -noixemul -o af_drand48_amiga -lm && vamos af_drand48_amiga
*/


unsigned long long seed=0;


void af_af_srand48(long int seedval)
{
    seed=seedval*65536LL+0x330e;
}

double af_drand48() 
{
    seed = (0x5DEECE66DL * seed + 0xBL) & ((1LL << 48) - 1);
    unsigned short seed_0= seed&0xffff;
    unsigned short seed_1=(seed&0xffff0000) >> 16;

    //printf("            Seed_0=%04hx, Seed_1=%04hx\n",seed_0, seed_1);

    seed_0=(short)seed_0+(short)seed_1;
    seed=(seed&0xffffffff0000)+ seed_0;
    //printf("SASC-Fixed: Seed_0=%04hx, Seed_1=%04hx\n",seed_0, seed_1);

    return (double)seed / (1LL << 48);
}


#ifdef SASC_DRAND48_TEST
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
float SASC_Results1[8]=
{
0.000000,
0.000985,
0.568917,
0.767175,
0.081369,
0.652252,
0.286671,
0.176834
};

char * SASC_SeedBuffs1[8]=
{
"x[0]=0x0000 x[1]=0x0000 x[2]=0x000b",
"x[0]=0x0040 x[1]=0x942d x[2]=0x7ae7",
"x[0]=0x91a4 x[1]=0x92bc x[2]=0x7122",
"x[0]=0xc465 x[1]=0x8f38 x[2]=0x46bd",
"x[0]=0x14d4 x[1]=0xa6bf x[2]=0x9343",
"x[0]=0xa6f9 x[1]=0xf9a3 x[2]=0xdf35",
"x[0]=0x4963 x[1]=0x402b x[2]=0xe7c7",
"x[0]=0x2d45 x[1]=0x0262 x[2]=0x7c28"
};


float SASC_Results2[8]=
{
0.655280,
0.833024,
0.451051,
0.487438,
0.449567,
0.840612,
0.781184,
0.217187
};

char * SASC_SeedBuffs2[8]=
{
"x[0]=0xa7c0 x[1]=0x67b7 x[2]=0xb8b8",
"x[0]=0xd541 x[1]=0x12ce x[2]=0x0931",
"x[0]=0x7378 x[1]=0x1527 x[2]=0x050f",
"x[0]=0x7cc8 x[1]=0xbbfc x[2]=0x5d6a",
"x[0]=0x7316 x[1]=0xd019 x[2]=0xd246",
"x[0]=0xd732 x[1]=0x5a71 x[2]=0xc64a",
"x[0]=0xc7fb x[1]=0xb2cf x[2]=0x9c5c",
"x[0]=0x3799 x[1]=0x95b0 x[2]=0xd0e7"
};


float SASC_Results3[8]=
{
0.563208,
0.003215,
0.346170,
0.920138,
0.354232,
0.663992,
0.742035,
0.957116
};

char * SASC_SeedBuffs3[8]=
{
"x[0]=0x902e x[1]=0x69da x[2]=0xbadb",
"x[0]=0x00d2 x[1]=0xc1e6 x[2]=0x1330",
"x[0]=0x589e x[1]=0x9473 x[2]=0xdfee",
"x[0]=0xeb8e x[1]=0x23ee x[2]=0x504f",
"x[0]=0x5aae x[1]=0xf373 x[2]=0x1f21",
"x[0]=0xa9fb x[1]=0x6667 x[2]=0x4d7f",
"x[0]=0xbdf5 x[1]=0xfcb0 x[2]=0x15ce",
"x[0]=0xf505 x[1]=0x8878 x[2]=0xe539"
};



int DoubleEqual(double a, double b)
{
   if(fabs((a-b) < 0.00001))
   {
      return 1;
   }
   else
   {
      return 0;
   }
}


int main(void)
{
   int i;

   /* initial seed, i.e. without call to af_srand48() */
   printf("---- initial seed ---- \n");
   for(i=0;i<8;i++)
   {  
      double af_rnd=af_drand48();
      printf("%s %3i) %f (sas/c gives: %f) %012llx %s\n\n",DoubleEqual(af_rnd,SASC_Results1[i])?"Ok ":"Bad",i+1,af_rnd,SASC_Results1[i],seed,SASC_SeedBuffs1[i]);
   }
   
   printf("---- srand(11111) ---- \n");

   af_af_srand48(11111);
   for(i=0;i<8;i++)
   {
      double af_rnd=af_drand48();
      printf("%s %3i) %f (sas/c gives: %f) %012llx %s\n\n",DoubleEqual(af_rnd,SASC_Results2[i])?"Ok ":"Bad",i+1,af_rnd,SASC_Results2[i],seed,SASC_SeedBuffs2[i]);
   }

   printf("---- srand(5282870) ---- \n");

   af_af_srand48(5282870);
   for(i=0;i<8;i++)
   {
      double af_rnd=af_drand48();
      printf("%s %3i) %f (sas/c gives: %f) %012llx %s\n\n",DoubleEqual(af_rnd,SASC_Results3[i])?"Ok ":"Bad",i+1,af_rnd,SASC_Results3[i],seed,SASC_SeedBuffs3[i]);
   }

   printf("Sizeof float=%d\n",sizeof(float));
   printf("Sizeof double=%d\n",sizeof(double));
   printf("Sizeof long=%d\n",sizeof(long));
   printf("Sizeof long long=%d\n",sizeof(long long));


   return 0;
}

#endif
