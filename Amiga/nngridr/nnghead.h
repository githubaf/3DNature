/*----------------------- nnghead.h ----------------------------*/
#include <stdio.h>
#include <stdlib.h>    /* added 4 Nov. 94 */
#include <math.h>
#include <m68881.h>
#include <string.h>
#include <time.h>

#define MAXRAND        0x7fffffff
#define RANSEED        367367
#define BIGNUM         1E37
#define WEEBIT         0.0001
#define MAXPATHLEN     81
#define LINESIZE       81
#define EQ             ==
#define NE             !=
#define AND            &&
#define OR             ||

time_t  t1now, t2now;

extern double  ra, rg;

double  xstart, ystart, xterm, yterm, horilap, 
        vertlap,arriba, bI, bJ, xspace, yspace, nuldat;
extern double magx, magy, magz, maxxy[2][3];
int     igrad, x_nodes, y_nodes, non_neg, z_only, 
        comma, ichoro,densi, tgrid, updir, sdip, rads, 
        ioK = 0, atime, badfile, gnup, ixmag = 0, 
        iymag = 0, izmag = 0, indent = 8, magcnt, 
        ccolor, updir, idisplay = 0, x_extra, y_extra, 
        southhemi, mxmn, saygo, igif = 0, extrap;
extern int datcnt, imag, optim, togif;

char     *grd_file, *dat_file, *gif_file, *datset, 
        *scratch, *x_least, *y_least, *x_great, 
        *y_great, *x_lap, *y_lap, *taut1, *taut2,
        *infform, *indform, *out1form, *out1aform, 
        *out3form, *out3aform,
        oneline[LINESIZE], inbuf[LINESIZE],
        *intstr,
        *funcstr = "Functional",
        *chorstr = "Choropleth",
        *densstr = "Density",
        *titlstr = 
"NNGRIDR - Natural Neighbour GRIDding algoRithm",
        *cpwtstr = "(c) 1991 D F Watson ",
        *rgridstr = "(using rectangular grid)",
        *tgridstr = "(using triangular grid)",
        *degrees = "measured in degrees",
        *radians = "measured in radians",
        *col2 = "(two-column input format)",
        *col3 = "(three-column input format)";

FILE    *filep, *fopen();

/* nngridr.c */
void                   Initialize(void);
void                   GetOptions(void);
void                   InitScrn(void);
void                   InterpStr(void);
void                   ShowMenu(void);
void                   ShowState(void);
void                   InString(void);
void                   WriteInit(void);
void                   MakeGnufl(void);
void                   MakeToGif(void);
void                   Leader(int num);
void                   Spacer(int num);
int                    GetSize(double anum);

/* nncrunch.c */
extern int             ReadData(void);
extern int             NatNeiSort();
extern int             Gradient(void);
extern int             MakeGrid(void);
extern int             ChoroPleth(void);
extern void            GifMaker(void);
