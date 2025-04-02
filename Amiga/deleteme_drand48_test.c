#include <stdio.h>
#include <stdlib.h>
#include <math.h>


#ifndef __SASC
unsigned short Drand48SeedBuffer[]={0,0,0};                /* gcc, SAS/C */

/*
void sasc_af_srand48(long int seedval)
{
    Drand48SeedBuffer[0]=0x330e;
    Drand48SeedBuffer[1]=((unsigned long)seedval) & 0xFFFF;
    Drand48SeedBuffer[2]=((unsigned long)seedval>> 16) & 0xFFFF;
}
*/

/*
double sasc_af_drand48(void)
{
    double Random;
    Random=erand48(Drand48SeedBuffer);
    Drand48SeedBuffer[0]=(short)Drand48SeedBuffer[0]+(short)Drand48SeedBuffer[1];  <-- Additional calculation by SAS/C

    return Random;
}
*/

#define af_drand48 sasc_af_drand48
#define af_srand48 sasc_af_srand48

#endif

#define X0 0x330E
#define X1 0xABCD
#define X2 0x1234
#define N 16
#define A0 0xE66D
#define A1 0xDEEC
#define A2 0x5
#define C 0xB

#define MASK ((unsigned)(1 << (N - 1)) + (1 << (N - 1)) - 1)
#define LOW(x) ((unsigned)(x) & MASK)
#define HIGH(x) LOW((x) >> N)
#define SET3(x, x0, x1, x2) ((x)[0] = (x0), (x)[1] = (x1), (x)[2] = (x2))
#define SEED(x0, x1, x2) (SET3(x, x0, x1, x2), SET3(a, A0, A1, A2), c = C)
#define CARRY(x, y) ((long)(x) + (long)(y) > MASK)
#define ADDEQU(x, y, z) (z = CARRY(x, (y)), x = LOW(x + (y)))
#define MUL(x, y, z) { long l = (long)(x) * (long)(y); (z)[0] = LOW(l); (z)[1] = HIGH(l); }


static unsigned x[3] = { /*X0, X1, X2*/ 0,0,0 }, a[3] = { A0, A1, A2 }, c = C;

void sasc_af_srand48(long seedval)
{
    SEED(X0, LOW(seedval), HIGH(seedval));
}

static void next()
{
   unsigned p[2], q[2], r[2], carry0, carry1;

   MUL(a[0], x[0], p);
   ADDEQU(p[0], c, carry0);
   ADDEQU(p[1], carry0, carry1);
   MUL(a[0], x[1], q);
   ADDEQU(p[1], q[0], carry0);
   MUL(a[1], x[0], r);
   x[2] = LOW(carry0 + carry1 + CARRY(p[1], r[0]) + q[1] + r[1] + a[0] * x[2] + a[1] * x[1] + a[2] * x[0]);
   x[1] = LOW(p[1] + r[0]);
   x[0] = LOW(p[0]);
}

double sasc_af_drand48(void)
{
   double Result;
   static double two16m = 1.0 / (1L << N);
   next();
   Result= two16m * (two16m * (two16m * x[0] + x[1]) + x[2]);
   printf("x[0]=0x%02hx x[1]=0x%02hx\n",x[0],x[1]);
   x[0]=(short)x[0]+(short)x[1];  /* done by SAS/C */
   printf("x[0]=0x%02hx x[1]=0x%02hx x[2]=0x%02hx\n",x[0],x[1],x[2]);
   printf("------------------\n");
   return Result;
}



void printBuffer(void)
{
}

int main(void)
{
double x;

   /* start with default seed */
   printf("\ndefault seed  --------\n");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.000000) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.000985) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.568917) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.767175) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.081369) < 0.001 ?  "ok" : "wrong");
   
   /* manually set seed */
   printf("\nsrand(0) -------------\n");
   af_srand48(0);
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.170828) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.017026) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.366464) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.309560) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.229357) < 0.001 ?  "ok" : "wrong");

   /* manually set seed */
   printf("\nsrand(11111) ---------\n");
   af_srand48(11111);

   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.655280) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.833024) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.451051) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.487438) < 0.001 ?  "ok" : "wrong");
   
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.449567) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.840612) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.781184) < 0.001 ?  "ok" : "wrong");
   x=af_drand48(); printf("%f %s\n", x, fabs (x- 0.217187) < 0.001 ?  "ok" : "wrong");

   return 0;
}

