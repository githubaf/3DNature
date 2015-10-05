/*----------------------- mygif.h ----------------------------*/
#include <stdio.h>
#include <stdlib.h>   /* added 4 Nov. 94 */
#include <m68881.h>
#include <string.h>

#define BIGNUM  1e37
#define WEEBIT  0.000001
#define PIE     3.141592654
#define PIEBY2  1.570796327
#define MAXCODE 4096
#define CODSTA  6
#define CODLIM  63
#define CLEAR   32
#define EOI     33
#define STACOD  34
#define AND     &&
#define OR      ||
#define EQ      ==
#define NE      !=

double top, bottom, span, top2, xhold, xhold2, 
             **tworows, **xprop, **yprop, **xprop2;
int numrows, numcols, codesize, codelim, block_index, 
     togif, xnodes2, byte_index = 0, bitcount = 0;
char thebyte, block[256], *datafile, 
         signature[8] = "GIF87a";
FILE *dfile, *gfile;

extern int x_nodes, y_nodes, x_extra, y_extra, z_only, 
                  tgrid, idisplay, rads, southhemi;
extern char *grd_file, *gif_file;

short int topcode;
short int color_map[32][3] =
{{  0,   0,   0},  /*  black */
 {110,   0,   0}, 
 {133,   1,   1}, 
 {155,   5,   5}, 
 {180,   5,   5}, 
 {205,  10,  10}, 
 {235,  15,  15}, 
 {255,  20,  20},  /* red  */
 {250,   0, 145}, 
 {245,   0, 180}, 
 {215,   0, 230}, 
 {165,   0, 245}, 
 {125,   0, 255},  
 { 65,  65, 255},  /* blue */
 {  0,  88, 255}, 
 {  0, 130, 225}, 
 {  0, 160, 200}, 
 {  0, 184, 143}, 
 {  0, 200,  90}, 
 {  0, 215,   0},  /* green */
 { 70, 215,   0}, 
 {110, 220,   5}, 
 {150, 225,  10}, 
 {195, 230,   5}, 
 {220, 235,   0}, 
 {240, 240,   0},  /* yellow */
 {245, 240,  40}, 
 {250, 240,  85}, 
 {255, 240, 130}, 
 {255, 245, 170}, 
 {255, 250, 215}, 
 {255, 255, 255}}; /* white */

struct codenode
{  short int            nodecode;
   struct codenode *nextnode[32];
};
struct codenode *rootnode, *curnode, *holdnode;

/* mygif.c */
void            GifMaker(void);
struct codenode *IMakeCodenode(void);
void            FreeCodeNodes(struct codenode *nodeptr);
void            DataMaxMin(void);
void            WriteHeader(void);
void            EncodeColor(short color);
void            SendCode(short codenum);
void            StartNewTable(void);
void            SendBlock2File(void);
void            MakeHeader(void);
char            HighNum(short anum);
char            LowNum(short anum);
void            Terminator(void);
void            InitTable(void);
double          ReadDatum(void);
void            PostColor(double zz);

extern double   **DoubleMatrix(int nrows, int ncols);
extern void     FreeMatrixd(double **matptr);
