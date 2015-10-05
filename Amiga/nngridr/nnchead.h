/*----------------------- nnchead.h ----------------------------*/
#include <stdio.h>
#include <stdlib.h>   /* added 4 Nov. 94 */
#include <math.h>
#include <m68881.h>

#define SQ(x)   (x) * (x)
#define BIGNUM  1E37
#define EPSILON 0.00001           
#define MAXRAND 0x7fffffff
#define NRANGE  10
#define EQ      ==
#define NE      !=
#define AND     &&
#define OR      ||

struct datum                           
{  double       values[3];
   struct datum *nextdat;
};
struct datum    *rootdat = NULL, *curdat, *holddat;

struct simp
{  int          vert[3];
   double       cent[3];
   struct simp  *nextsimp;
};
struct simp     *rootsimp = NULL, *cursimp, *holdsimp, 
                *lastsimp, *prevsimp;

struct temp
{  int          end[2];
   struct temp  *nexttemp;
};
struct temp     *roottemp = NULL, *curtemp, *lasttemp, 
                *prevtemp;

struct neig
{  int          neinum;
   double       narea;
   double       coord;
   struct neig  *nextneig;
};
struct neig     *rootneig = NULL, *curneig, *lastneig;

double          **points, **joints, wbit, magx = 1, 
                magy = 1, magz = 1, maxxy[2][3], 
                maxhoriz, aaa, bbb, ccc, det, 
                work3[3][3], xx, sumx, sumy, sumz, 
                sumx2, sumy2, sumxy, sumxz, sumyz, 
                asum, pi, piby2, piby32, rad2deg, 
                bigtri[3][3];
extern double   xstart, ystart, xterm, yterm, 
                horilap, vertlap, arriba, bI, bJ, nuldat;
int             datcnt, datcnt3, numtri, imag, numnei, 
                 ext, *jndx, neicnt, optim, goodflag,
                scor[3][2] = {{1,2}, {2,0}, {0,1}};
extern int      igrad, ichoro, x_nodes, y_nodes, extrap,
                z_only, non_neg, tgrid, updir, densi, 
                gnup, mxmn, indent, sdip, rads, atime;
extern char     *dat_file, *grd_file, *indform, 
                *infform, *out1form, *out1aform, 
                *out3form, *out3aform;
extern FILE     *filep;

/* nncrunch.c */
int             ReadData(void);
/* int             NatNeiSort(); not relevant, commented out 20/12/94 */
void            ChoroPleth(void);
void            FindNeigh(int ipt);
void            TriCentr(void);
void            TriNeigh(void);
void            Gradient(void);
int             MakeGrid(void);
void            FindProp(double wxd, double wyd);
double          Surface(void);
double          Meld(double asurf, double wxd, double wyd);
void            TooSteep(void);
void            TooShallow(void);
void            TooNarrow(void);
struct datum    *IMakeDatum(void);
struct simp     *IMakeSimp(void);
struct temp     *IMakeTemp(void);
struct neig     *IMakeNeig(void);
int             *IntVect(int ncols);
void            FreeVecti(int *vectptr);
double          *DoubleVect(int ncols);
void            FreeVectd(double *vectptr);
int             **IntMatrix(int nrows, int ncols);
void            FreeMatrixi(int **matptr);
float           **FloatMatrix(int nrows, int ncols);
void            FreeMatrixf(float **matptr);
double          **DoubleMatrix(int nrows, int ncols);
void            FreeMatrixd(double **matptr);

/* nngridr.c */
extern void     Leader(int num);
extern void     InString(void);
