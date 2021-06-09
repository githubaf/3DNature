/*----------------------- nncrunch.c ----------------------------*/
#include "WCS.h"

/* ReadData opens an XY or XYZ file. If rootdat is 0 (which it always
will be in WCS) it allocates rootdat, rootsimp, roottemp and rootneig
structures. It reads in the coordinate triplets or pairs as a linked list
of datum structures beginning with rootdat (which remains empty).
   When the number of data are known (datacnt) jndx (int), points
(Double Matrix) and joints (Double Matrix) are allocated. The points matrix
is allocated larger if gradients are used. 
   Data are copied and scaled from the datum structures to the points
matrix with points[pt][0] being the x coordinate, [pt][1] as the y and
[pt][2] as the z coordinate. If the data is density data then the z value is
set to 1.
   As data is copied to the matrix it is summed and products of various
x, y, z combinations are summed into the sumx, sumy... variables. Maximum
and minimum of all x, y and z coords are kept in maxxy[][].
   After each coordinate is copied its datum structure is freed.
   The first datum struct (rootdat) is not freed.
   Maximum and minimum values of all variables are checked against the
boundaries of the grid region and adjusted outward if necessary.
   Ratios of width and height and vertical are checked to see if they
fall within acceptable limits for the gridding and gradient algorithms.
   rootdat is set to null. Note: I added the freeing of rootdat which wasn't
in the original code.
*/

/*************************************************************************/

short NNGrid_DataInit(struct NNGrid *NNG)
{
short success = 1;
double minx, maxx, miny, maxy;
int i0, i1;
struct datum *DT;

/* provide some user_messages here */
 if (! MD_Win)
  return (0);
 if (! MD_Win->TC)
  return (0);
 if (MD_Win->ControlPts < 3)
  return (0);

   NNG->bigtri[0][0] = NNG->bigtri[0][1] = NNG->bigtri[1][1] = 
      NNG->bigtri[2][0] = -1;
   NNG->bigtri[1][0] = NNG->bigtri[2][1] = 5;

   NNG->rootdat = Datum_New();
   NNG->rootsimp = Simp_New();
   NNG->roottemp = Temp_New();
   NNG->rootneig = Neig_New();
   if (! NNG->rootdat || ! NNG->rootsimp || ! NNG->roottemp || ! NNG->rootneig)
    {
    success = 0;
    goto EndRead;
    } /* if memory failure */
   NNG->rootdat->values[0] = NNG->rootdat->values[1] = 
         NNG->rootdat->values[2] = 0.0;
   NNG->curdat = NNG->rootdat;
   NNG->datcnt = 0;
   minx = NNG->xstart - NNG->horilap;
   maxx = NNG->xterm + NNG->horilap;
   miny = NNG->ystart - NNG->vertlap;
   maxy = NNG->yterm + NNG->vertlap;

/* copy in-bounds data to new datum structures and change sign of x values
     to conform with nngridr convention */
   DT = MD_Win->TC->nextdat;
   while (DT)
    {
    if (-DT->values[0] > minx AND -DT->values[0] < maxx AND 
            DT->values[1] > miny AND DT->values[1] < maxy)
     {
     if (NNG->curdat->nextdat = Datum_New())
      {
      NNG->curdat = NNG->curdat->nextdat;
      NNG->datcnt++;
      NNG->curdat->values[0] = -DT->values[0];
      NNG->curdat->values[1] = DT->values[1];
      NNG->curdat->values[2] = NNG->densi ? 1: DT->values[2];
      } /* if memory allocated for new datum */
     else
      {
/* out of memory message */
      success = 0;
      break;
      } /* else no memory */
     } /* if values in grid region */
    DT = DT->nextdat;
    } /* while */

   if (! success)
    goto EndRead;

/* transfer datum values to points and joints matrices */
   if (NNG->datcnt>3)
    {
    NNG->datcnt3 = NNG->datcnt + 3;
    NNG->IntVectCols = NNG->datcnt3;
    NNG->jndx = IntVect(NNG->datcnt3);
    NNG->sumx = NNG->sumy = NNG->sumz = NNG->sumx2 = NNG->sumy2 = 
         NNG->sumxy = NNG->sumxz = NNG->sumyz = 0;
    NNG->maxxy[0][0] = NNG->maxxy[0][1] = NNG->maxxy[0][2] = 
         -(NNG->maxxy[1][0] = NNG->maxxy[1][1] = NNG->maxxy[1][2] = 
         BIGNUM);
    NNG->PointMatxRows = NNG->datcnt + 4;
    NNG->PointMatxCols = NNG->igrad ? 6: 3;
    NNG->points = DoubleMatrix(NNG->PointMatxRows, NNG->PointMatxCols);
    NNG->JointMatxRows = NNG->datcnt3;
    NNG->JointMatxCols = 2;
    NNG->joints = DoubleMatrix(NNG->JointMatxRows, NNG->JointMatxCols); 
    NNG->curdat = NNG->rootdat->nextdat;
    for (i0=0; i0<NNG->datcnt; i0++)
     {
     NNG->sumx += NNG->points[i0][0] = 
            NNG->curdat->values[0] * NNG->magx;
     NNG->sumx2 += SQ(NNG->points[i0][0]);
     if (NNG->maxxy[0][0] < NNG->points[i0][0]) 
            NNG->maxxy[0][0] = NNG->points[i0][0];  
     if (NNG->maxxy[1][0] > NNG->points[i0][0]) 
            NNG->maxxy[1][0] = NNG->points[i0][0];  
     NNG->sumy += NNG->points[i0][1] = 
            NNG->curdat->values[1] * NNG->magy;
     NNG->sumy2 += SQ(NNG->points[i0][1]);
     NNG->sumxy += NNG->points[i0][0] * NNG->points[i0][1];
     if (NNG->maxxy[0][1] < NNG->points[i0][1]) 
      NNG->maxxy[0][1] = NNG->points[i0][1];  
     if (NNG->maxxy[1][1] > NNG->points[i0][1]) 
      NNG->maxxy[1][1] = NNG->points[i0][1];  

     if (NNG->densi)
      NNG->points[i0][2] = 1;
     else
      {
      NNG->sumz += NNG->points[i0][2] = 
               NNG->curdat->values[2] * NNG->magz;
      NNG->sumxz += NNG->points[i0][0] * NNG->points[i0][2];
      NNG->sumyz += NNG->points[i0][1] * NNG->points[i0][2];
      if (NNG->maxxy[0][2] < NNG->points[i0][2]) 
       NNG->maxxy[0][2] = NNG->points[i0][2]; 
      if (NNG->maxxy[1][2] > NNG->points[i0][2]) 
       NNG->maxxy[1][2] = NNG->points[i0][2]; 
      } /* else not density data */

     NNG->curdat = NNG->curdat->nextdat;
     } /* for i0=0... */

    NNG->det = (NNG->datcnt * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
          - (NNG->sumx * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumy * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2));
    NNG->aaa = ((NNG->sumz * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
          - (NNG->sumxz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumyz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
         NNG->det;
    NNG->bbb = 
         ((NNG->datcnt * (NNG->sumxz * NNG->sumy2 - NNG->sumyz * NNG->sumxy))
          - (NNG->sumz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumy * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))) / 
         NNG->det;
    NNG->ccc = 
         ((NNG->datcnt * (NNG->sumx2 * NNG->sumyz - NNG->sumxy * NNG->sumxz))
          - (NNG->sumx * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))
          + (NNG->sumz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
         NNG->det;

/* adjust max/min ranges for the extent of gridded region */
    if (NNG->maxxy[0][0] < maxx * NNG->magx) 
         NNG->maxxy[0][0] = maxx * NNG->magx; 
    if (NNG->maxxy[1][0] > minx * NNG->magx) 
         NNG->maxxy[1][0] = minx * NNG->magx; 
    if (NNG->maxxy[0][1] < maxy * NNG->magy) 
         NNG->maxxy[0][1] = maxy * NNG->magy; 
    if (NNG->maxxy[1][1] > miny * NNG->magy) 
         NNG->maxxy[1][1] = miny * NNG->magy;

/* maximum values have minimums subtracted */ 
    for (i0=0; i0<3; i0++) 
         NNG->maxxy[0][i0] -= NNG->maxxy[1][i0];
    NNG->maxhoriz = NNG->maxxy[0][0]; 
    if (NNG->maxhoriz < NNG->maxxy[0][1]) 
         NNG->maxhoriz = NNG->maxxy[0][1];
    NNG->wbit = NNG->maxhoriz * EPSILON;

/* ratios of width and height and steepness are evaluated for OKness
   Note that the functions TooWhatever turn off gradient analysis - this
	should set up a requester asking to continue or scale data or just
	automatically scale the data */
    if (NNG->maxxy[0][0] / NNG->maxxy[0][1] > 2 
         OR NNG->maxxy[0][1] / NNG->maxxy[0][0] > 2)
     { 
     if (! TooNarrow(NNG))
      {
      success = 0;
      goto EndRead;
      } /* if */
     } /* if */
    if (! NNG->ichoro AND NNG->igrad)
     {
     if (NNG->maxxy[0][2] / NNG->maxxy[0][0] > 60
            OR NNG->maxxy[0][2] / NNG->maxxy[0][1] > 60)
      { 
      if (! TooSteep(NNG))
       {
       success = 0;
       goto EndRead;
       } /* if */
      } /* if */
     if (NNG->maxxy[0][2] / NNG->maxxy[0][0] < .017
            OR NNG->maxxy[0][2] / NNG->maxxy[0][1] < .017)
      {
      if (! TooShallow(NNG))
       {
       success = 0;
       goto EndRead;
       } /* if */
      } /* if */
     } /* if not choropleth and gradients used */

/* store bigtriangle data in points matrix beyond normal data */
    for (i0=0; i0<3; i0++)
     {
     NNG->points[NNG->datcnt+i0][0] = NNG->maxxy[1][0] + 
            NNG->bigtri[i0][0] * NNG->maxxy[0][0] * NRANGE;
     NNG->points[NNG->datcnt+i0][1] = NNG->maxxy[1][1] + 
            NNG->bigtri[i0][1] * NNG->maxxy[0][1] * NRANGE;
     if (NNG->densi)
      NNG->points[NNG->datcnt+i0][2] = 1;
     else NNG->points[NNG->datcnt+i0][2] =
            NNG->aaa + NNG->bbb * NNG->points[NNG->datcnt+i0][0] + 
            NNG->ccc * NNG->points[NNG->datcnt+i0][1];
     } /* for i0=0... */
    } /* if datcnt >= 3 */
   else
    {
    success = 0;
    User_Message("Map View: Build DEM",
	"Insufficient data in gridded region to triangulate!\
 Increase the size of the gridded region or add more control points.", "OK", "o");
    goto EndRead;
    } /* else insufficient data */

/* perturb all data points by a fraction to assure that no points are
      coincident */
   srand(367);     
   for (i0=0; i0<NNG->datcnt; i0++) 
    for (i1=0; i1<2; i1++)
     NNG->points[i0][i1] += NNG->wbit * (0.5 - (double)rand() / MAXRAND);

/* Watson cleverly lets the machine tell him what the related values of Pi
     are rather than defining them as I have done elsewhere. A: he gets higher
     precision and B: he gets values that are internally consistent with
     the processor, compiler and math libraries. */

   if (NNG->igrad)
    {
    NNG->piby2 = 2 * atan(1.0);
    NNG->pi = NNG->piby2 * 2;
    NNG->piby32 = 3 * NNG->piby2;
    NNG->rad2deg = 90 / NNG->piby2;
    } /* if gradients */

EndRead:
   Datum_Del(NNG->rootdat);
   NNG->rootdat = NULL;

   return (success);

} /* NNGrid_DataInit() */

/***********************************************************************/
#ifdef GJHJHGJHGJDS
// Obsolete - data is not read from a file but transferred via pointer
from Build DEM window and modified in NNGrid_DataInit()

short ReadData(struct NNGrid *NNG)
{
short success = 1;
FILE *filep;

   double temp[3], minx, maxx, miny, maxy;
   int i0, i1;

   NNG->bigtri[0][0] = NNG->bigtri[0][1] = NNG->bigtri[1][1] = 
      NNG->bigtri[2][0] = -1;
   NNG->bigtri[1][0] = NNG->bigtri[2][1] = 5;

   if ((filep = fopen(NNG->dat_file,"r")) EQ NULL)
   {
      User_Message("Map View: Build DEM",
	"Unable to open data file.", "OK", "o");
      return 0;
   }
   NNG->rootdat = Datum_New();
   NNG->rootsimp = Simp_New();
   NNG->roottemp = Temp_New();
   NNG->rootneig = Neig_New();
   if (! NNG->rootdat || ! NNG->rootsimp || ! NNG->roottemp || ! NNG->rootneig)
   {
      fclose(filep);
      return (0);
   }
   NNG->rootdat->values[0] = NNG->rootdat->values[1] = 
         NNG->rootdat->values[2] = 0;
   NNG->curdat = NNG->rootdat;
   NNG->datcnt = 0;
   minx = NNG->xstart - NNG->horilap;
   maxx = NNG->xterm + NNG->horilap;
   miny = NNG->ystart - NNG->vertlap;
   maxy = NNG->yterm + NNG->vertlap;
   if (NNG->densi)   
   {  while (fscanf(filep, "%le%le", &temp[0], &temp[1]) NE EOF)
      {
         if (temp[0]>minx AND temp[0]<maxx AND 
            temp[1]>miny AND temp[1]<maxy)
         {  if (NNG->curdat->nextdat EQ NULL) 
               NNG->curdat->nextdat = Datum_New();
            NNG->curdat = NNG->curdat->nextdat;
            NNG->datcnt++;
            for (i1=0; i1<2; i1++) 
               NNG->curdat->values[i1] = temp[i1];
            NNG->curdat->values[2] = 1;
         }
      }
   }
   else      
   {  while (fscanf(filep, "%le%le%le", &temp[0], &temp[1], &temp[2]) NE EOF)
      {
         if (temp[0]>minx AND temp[0]<maxx AND 
            temp[1]>miny AND temp[1]<maxy)
         {  if (NNG->curdat->nextdat EQ NULL) 
               NNG->curdat->nextdat = Datum_New();
            NNG->curdat = NNG->curdat->nextdat;
            NNG->datcnt++;
            for (i1=0; i1<3; i1++) 
               NNG->curdat->values[i1] = temp[i1];
         }
      }
   }
   fclose(filep);
   if (NNG->datcnt>3)
   {  NNG->datcnt3 = NNG->datcnt + 3;
      NNG->IntVectCols = NNG->datcnt3;
      NNG->jndx = IntVect(NNG->datcnt3);
      NNG->sumx = NNG->sumy = NNG->sumz = NNG->sumx2 = NNG->sumy2 = 
         NNG->sumxy = NNG->sumxz = NNG->sumyz = 0;
      NNG->maxxy[0][0] = NNG->maxxy[0][1] = NNG->maxxy[0][2] = 
         -(NNG->maxxy[1][0] = NNG->maxxy[1][1] = NNG->maxxy[1][2] = 
         BIGNUM);
      NNG->PointMatxRows = NNG->datcnt + 4;
      NNG->PointMatxCols = NNG->igrad ? 6: 3;
      NNG->points = DoubleMatrix(NNG->PointMatxRows, NNG->PointMatxCols);
      NNG->JointMatxRows = NNG->datcnt3;
      NNG->JointMatxCols = 2;
      NNG->joints = DoubleMatrix(NNG->JointMatxRows, NNG->JointMatxCols); 
      NNG->curdat = NNG->rootdat->nextdat;
      for (i0=0; i0<NNG->datcnt; i0++)
      {  NNG->sumx += NNG->points[i0][0] = 
            NNG->curdat->values[0] * NNG->magx;
         NNG->sumx2 += SQ(NNG->points[i0][0]);
         if (NNG->maxxy[0][0] < NNG->points[i0][0]) 
            NNG->maxxy[0][0] = NNG->points[i0][0];  
         if (NNG->maxxy[1][0] > NNG->points[i0][0]) 
            NNG->maxxy[1][0] = NNG->points[i0][0];  
         NNG->sumy += NNG->points[i0][1] = 
            NNG->curdat->values[1] * NNG->magy;
         NNG->sumy2 += SQ(NNG->points[i0][1]);
         NNG->sumxy += NNG->points[i0][0] * NNG->points[i0][1];
         if (NNG->maxxy[0][1] < NNG->points[i0][1]) 
            NNG->maxxy[0][1] = NNG->points[i0][1];  
         if (NNG->maxxy[1][1] > NNG->points[i0][1]) 
            NNG->maxxy[1][1] = NNG->points[i0][1];  
/********************************************
The statement     if (!densi)
which was the fifth line from the bottom on page 106
has been replaced by the following two lines
6 June 1994
***********************************************/
         if (NNG->densi) NNG->points[i0][2] = 1;
         else
         {  NNG->sumz += NNG->points[i0][2] = 
               NNG->curdat->values[2] * NNG->magz;
            NNG->sumxz += NNG->points[i0][0] * NNG->points[i0][2];
            NNG->sumyz += NNG->points[i0][1] * NNG->points[i0][2];
            if (NNG->maxxy[0][2] < NNG->points[i0][2]) 
               NNG->maxxy[0][2] = NNG->points[i0][2]; 
            if (NNG->maxxy[1][2] > NNG->points[i0][2]) 
               NNG->maxxy[1][2] = NNG->points[i0][2]; 
         }
         NNG->curdat = NNG->curdat->nextdat;
      }
      NNG->det = (NNG->datcnt * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
          - (NNG->sumx * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumy * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2));
      NNG->aaa = ((NNG->sumz * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
          - (NNG->sumxz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumyz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
         NNG->det;
      NNG->bbb = 
         ((NNG->datcnt * (NNG->sumxz * NNG->sumy2 - NNG->sumyz * NNG->sumxy))
          - (NNG->sumz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
          + (NNG->sumy * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))) / 
         NNG->det;
      NNG->ccc = 
         ((NNG->datcnt * (NNG->sumx2 * NNG->sumyz - NNG->sumxy * NNG->sumxz))
          - (NNG->sumx * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))
          + (NNG->sumz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
         NNG->det;

/* adjust max/min ranges for the extent of gridded region */
      if (NNG->maxxy[0][0] < maxx * NNG->magx) 
         NNG->maxxy[0][0] = maxx * NNG->magx; 
      if (NNG->maxxy[1][0] > minx * NNG->magx) 
         NNG->maxxy[1][0] = minx * NNG->magx; 
      if (NNG->maxxy[0][1] < maxy * NNG->magy) 
         NNG->maxxy[0][1] = maxy * NNG->magy; 
      if (NNG->maxxy[1][1] > miny * NNG->magy) 
         NNG->maxxy[1][1] = miny * NNG->magy;

/* maximum values have minimums subtracted */ 
      for (i0=0; i0<3; i0++) 
         NNG->maxxy[0][i0] -= NNG->maxxy[1][i0];
      NNG->maxhoriz = NNG->maxxy[0][0]; 
      if (NNG->maxhoriz < NNG->maxxy[0][1]) 
         NNG->maxhoriz = NNG->maxxy[0][1];
      NNG->wbit = NNG->maxhoriz * EPSILON;

/* ratios of width and height and steepness are evaluated for OKness
   Note that the functions TooWhatever turn off gradient analysis - this
	should set up a requester asking to continue or scale data or just
	automatically scale the data */
      if (NNG->maxxy[0][0] / NNG->maxxy[0][1] > 2 
         OR NNG->maxxy[0][1] / NNG->maxxy[0][0] > 2)
      { 
         if (! TooNarrow(NNG))
         {
         success = 0;
         goto EndRead;
	 }
      }
      if (!NNG->ichoro AND NNG->igrad)
      {  if (NNG->maxxy[0][2] / NNG->maxxy[0][0] > 60
            OR NNG->maxxy[0][2] / NNG->maxxy[0][1] > 60)
         { 
            if (! TooSteep(NNG))
            {
            success = 0;
            goto EndRead;
	    }
	 }
         if (NNG->maxxy[0][2] / NNG->maxxy[0][0] < .017
            OR NNG->maxxy[0][2] / NNG->maxxy[0][1] < .017)
         {
            if (! TooShallow(NNG))
            {
            success = 0;
            goto EndRead;
	    }
	 }
      }

/* store bigtriangle data in points matrix beyond normal data */
      for (i0=0; i0<3; i0++)
      {  NNG->points[NNG->datcnt+i0][0] = NNG->maxxy[1][0] + 
            NNG->bigtri[i0][0] * NNG->maxxy[0][0] * NRANGE;
         NNG->points[NNG->datcnt+i0][1] = NNG->maxxy[1][1] + 
            NNG->bigtri[i0][1] * NNG->maxxy[0][1] * NRANGE;
         if (NNG->densi) NNG->points[NNG->datcnt+i0][2] = 1;
         else NNG->points[NNG->datcnt+i0][2] =
            NNG->aaa + NNG->bbb * NNG->points[NNG->datcnt+i0][0] + 
            NNG->ccc * NNG->points[NNG->datcnt+i0][1];
      }
   }
   else
   {
      success = 0;
      User_Message("Map View: Build DEM",
	"Insufficient data in gridded region to triangulate!\
 Increase the size of the gridded region or add more control points", "OK", "o");
      goto EndRead;
   }

/* perturb all data points by a fraction to assure that no points are
      coincident */
   srand(367);     
   for (i0=0; i0<NNG->datcnt; i0++) 
      for (i1=0; i1<2; i1++)
         NNG->points[i0][i1] += NNG->wbit * 
            (0.5 - (double)rand() / MAXRAND);

/* Watson cleverly lets the machine tell him what the related values of Pi
     are rather than defining them as I have done elsewhere. A: he gets higher
     precision and B: he gets values that are internally consistent with
     the processor, compiler and math libraries. */

   if (NNG->igrad)
   {  NNG->piby2 = 2 * atan(1.0);
      NNG->pi = NNG->piby2 * 2;
      NNG->piby32 = 3 * NNG->piby2;
      NNG->rad2deg = 90 / NNG->piby2;
   }

EndRead:
   Datum_Del(NNG->rootdat);
   NNG->rootdat = NULL;

   return (success);

} /* ReadData() */
#endif
/***********************************************************************/

short ChoroPleth(struct NNGrid *NNG)
{
short success = 1;
int i0, i1;
double cmax[2][2];
struct BusyWindow *BWGR;

   BWGR = BusyWin_New("ChoroPleth", NNG->datcnt, 0, 'BWGR');

   NNG->maxxy[0][2] = -BIGNUM;
   NNG->maxxy[1][2] = BIGNUM;
   NNG->sumz = NNG->sumxz = NNG->sumyz = 0;
   for (i0=0; i0<NNG->datcnt; i0++)
   {
      if (! FindNeigh(NNG, i0))
       {
       success = 0;
       break;
       }
      cmax[0][0] = cmax[0][1] = -BIGNUM;
      cmax[1][0] = cmax[1][1] = BIGNUM;
      NNG->cursimp = NNG->rootsimp;
      for (i1=0; i1<NNG->numtri; i1++)
      {
         NNG->cursimp = NNG->cursimp->nextsimp;
         NNG->joints[i1][0] = NNG->cursimp->cent[0];
         if (cmax[0][0] < NNG->joints[i1][0]) 
            cmax[0][0] = NNG->joints[i1][0];
         if (cmax[1][0] > NNG->joints[i1][0]) 
            cmax[1][0] = NNG->joints[i1][0];
         NNG->joints[i1][1] = NNG->cursimp->cent[1];
         if (cmax[0][1] < NNG->joints[i1][1]) 
            cmax[0][1] = NNG->joints[i1][1];
         if (cmax[1][1] > NNG->joints[i1][1]) 
            cmax[1][1] = NNG->joints[i1][1];
      }
      cmax[0][0] -= cmax[1][0];
      cmax[0][1] -= cmax[1][1];
      for (i1=0; i1<3; i1++)
      {  NNG->joints[NNG->datcnt+i1][0] = cmax[1][0] + 
            NNG->bigtri[i1][0] * cmax[0][0] * NRANGE;
         NNG->joints[NNG->datcnt+i1][1] = cmax[1][1] + 
            NNG->bigtri[i1][1] * cmax[0][1] * NRANGE;
      }
      NNG->neicnt = NNG->numtri;
      if (! TriCentr(NNG))
      {
         success = 0;
         break;
      }
      NNG->cursimp = NNG->rootsimp;
      for (NNG->asum=0, i1=0; i1<NNG->numtri; i1++)
      {  NNG->cursimp = NNG->cursimp->nextsimp;
         if (NNG->cursimp->vert[0] < NNG->datcnt) NNG->asum +=
            fabs((NNG->joints[NNG->cursimp->vert[1]][0] - 
               NNG->joints[NNG->cursimp->vert[0]][0])
              *  (NNG->joints[NNG->cursimp->vert[2]][1] - 
               NNG->joints[NNG->cursimp->vert[0]][1])
              -  (NNG->joints[NNG->cursimp->vert[2]][0] - 
               NNG->joints[NNG->cursimp->vert[0]][0])
              *  (NNG->joints[NNG->cursimp->vert[1]][1] - 
               NNG->joints[NNG->cursimp->vert[0]][1])) / 2;
      }
      if (NNG->asum > 0) NNG->points[i0][2] /= NNG->asum; /* conditional added 4/4/95 */
      else NNG->points[i0][2] = 0;
      NNG->sumz += NNG->points[i0][2];
      NNG->sumxz += NNG->points[i0][0] * NNG->points[i0][2];
      NNG->sumyz += NNG->points[i0][1] * NNG->points[i0][2];
      if (NNG->maxxy[0][2] < NNG->points[i0][2]) 
         NNG->maxxy[0][2] = NNG->points[i0][2]; 
      if (NNG->maxxy[1][2] > NNG->points[i0][2]) 
         NNG->maxxy[1][2] = NNG->points[i0][2]; 
      if (CheckInput_ID() == ID_BW_CLOSE)
      {
         success = 0;
         break;
      } /* if user abort */
      BusyWin_Update(BWGR, i0 + 1);
   }
   if (! success)
      goto EndChoro;
   NNG->maxxy[0][2] -= NNG->maxxy[1][2];
   if (NNG->igrad)
   {   if (NNG->maxxy[0][2] / NNG->maxhoriz > 60
         OR NNG->maxxy[0][2] / NNG->maxxy[0][1] > 60) 
         if (! TooSteep(NNG))
         {
         success = 0;
         goto EndChoro;
	 }
      if (NNG->maxxy[0][2] / NNG->maxhoriz < .017
         OR NNG->maxxy[0][2] / NNG->maxxy[0][1] < .017) 
         if (! TooShallow(NNG))
         {
         success = 0;
         goto EndChoro;
	 }
   }
   NNG->det = (NNG->datcnt * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
       - (NNG->sumx * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
       + (NNG->sumy * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2));
   NNG->aaa = ((NNG->sumz * (NNG->sumx2 * NNG->sumy2 - NNG->sumxy * NNG->sumxy))
       - (NNG->sumxz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
       + (NNG->sumyz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
      NNG->det;
   NNG->bbb = ((NNG->datcnt * (NNG->sumxz * NNG->sumy2 - NNG->sumyz * NNG->sumxy))
       - (NNG->sumz * (NNG->sumx * NNG->sumy2 - NNG->sumy * NNG->sumxy))
       + (NNG->sumy * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))) / 
      NNG->det;
   NNG->ccc = ((NNG->datcnt * (NNG->sumx2 * NNG->sumyz - NNG->sumxy * NNG->sumxz))
       - (NNG->sumx * (NNG->sumx * NNG->sumyz - NNG->sumy * NNG->sumxz))
       + (NNG->sumz * (NNG->sumx * NNG->sumxy - NNG->sumy * NNG->sumx2))) / 
      NNG->det;
   for (i1=0; i1<3; i1++)
   {  NNG->points[NNG->datcnt+i1][2] =
         NNG->aaa + NNG->bbb * NNG->points[NNG->datcnt+i1][0] + 
         NNG->ccc * NNG->points[NNG->datcnt+i1][1];
      if (NNG->igrad)
      {  NNG->points[NNG->datcnt+i1][3] = -NNG->bbb;
         NNG->points[NNG->datcnt+i1][4] = -NNG->ccc;
         NNG->points[NNG->datcnt+i1][5] = 1;
      }
   }

EndChoro:
   if (BWGR) BusyWin_Del(BWGR);

   return (success);

} /* ChoroPleth() */

/***********************************************************************/

short Gradient(struct NNGrid *NNG)
{
short success = 1;
int i0, i1, i2, i3;
double u2, wxd, wyd, wxde, wydn, xc, xe, xn;
struct BusyWindow *BWGR;

   BWGR = BusyWin_New("Gradient", NNG->datcnt, 0, 'BWGR');

   for (i0=0; i0<NNG->datcnt; i0++)
   {
      if (! FindNeigh(NNG, i0))
      {
         success = 0;
         break;
      }
      if (! NNG->ext) 
      {
         if (! TriNeigh(NNG))
         {
            success = 0;
            break;
	 }
         wxd = NNG->points[i0][0];
         wyd = NNG->points[i0][1];
         if (! Find_Prop(NNG, wxd, wyd))
         {
            success = 0;
            break;
	 }
         xc = Surface(NNG);
         wxde = wxd + NNG->wbit;
         if (! Find_Prop(NNG, wxde, wyd))
         {
            success = 0;
            break;
	 }
         xe = Surface(NNG);
         wydn = wyd + NNG->wbit;
         if (! Find_Prop(NNG, wxd, wydn))
         {
            success = 0;
            break;
	 }
         xn = Surface(NNG);
         NNG->points[i0][3] = (xc - xe) / NNG->wbit;
         NNG->points[i0][4] = (xc - xn) / NNG->wbit;
         NNG->asum /= NNG->pi; 
         NNG->points[i0][5] = 1 - sqrt(NNG->asum / 
            (NNG->asum + SQ(NNG->points[i0][2] - xc)));
      }
      else     
      {
         NNG->points[i0][3] = NNG->points[i0][4] = 
            NNG->points[i0][5] = NNG->xxxx = 0;
         NNG->cursimp = NNG->rootsimp;
         for (i1=0; i1<NNG->numtri; i1++)
         {
            NNG->cursimp = NNG->cursimp->nextsimp;
            for (i2=0; i2<2; i2++) 
               for (i3=0; i3<3; i3++)
                  NNG->work3[i2][i3] = 
                     NNG->points[NNG->cursimp->vert[0]][i3] - 
                     NNG->points[NNG->cursimp->vert[i2+1]][i3];
            NNG->work3[2][0] = NNG->work3[0][1] * NNG->work3[1][2] - 
               NNG->work3[1][1] * NNG->work3[0][2];
            NNG->work3[2][1] = NNG->work3[0][2] * NNG->work3[1][0] - 
               NNG->work3[1][2] * NNG->work3[0][0];
            NNG->work3[2][2] = NNG->work3[0][0] * NNG->work3[1][1] - 
               NNG->work3[1][0] * NNG->work3[0][1];
            u2 = 1;
            if (NNG->work3[2][2]<0) u2 = -1;
            NNG->xxxx += sqrt(SQ(NNG->work3[2][0]) + 
               SQ(NNG->work3[2][1]) + SQ(NNG->work3[2][2]));
            for (i2=0; i2<3; i2++) NNG->points[i0][i2+3] += 
               NNG->work3[2][i2] * u2;
         }
         NNG->xxxx = 1 - sqrt(SQ(NNG->points[i0][3]) + 
            SQ(NNG->points[i0][4]) + 
            SQ(NNG->points[i0][5])) / NNG->xxxx;
         NNG->points[i0][3] /= NNG->points[i0][5];
         NNG->points[i0][4] /= NNG->points[i0][5];
         NNG->points[i0][5] = NNG->xxxx; 
      }
      if (CheckInput_ID() == ID_BW_CLOSE)
      {
         success = 0;
         break;
      } /* if user abort */
      BusyWin_Update(BWGR, i0 + 1);
   }
   if (BWGR) BusyWin_Del(BWGR);
   if (! success)
      return (0);
   for (i0=0; i0<3; i0++)
   {  NNG->points[NNG->datcnt+i0][3] = -NNG->bbb;
      NNG->points[NNG->datcnt+i0][4] = -NNG->ccc;
      NNG->points[NNG->datcnt+i0][5] = 1;
   }

   return (1);

} /* Gradient() */

/***********************************************************************/

short FindNeigh(struct NNGrid *NNG, int ipt)
{
short success = 1;
int i0, i1, i2, i3, j1, j2, j3, j4, j5;

   if (NNG->rootsimp->nextsimp EQ NULL)
   { 
      if (! (NNG->rootsimp->nextsimp = Simp_New()))
         return (0);
   }
   NNG->cursimp = NNG->rootsimp->nextsimp;
   NNG->cursimp->vert[0] = NNG->datcnt;
   NNG->cursimp->vert[1] = NNG->datcnt + 1;
   NNG->cursimp->vert[2] = NNG->datcnt + 2;
   NNG->cursimp->cent[0] = NNG->cursimp->cent[1] = 0.5;
   NNG->cursimp->cent[2] = BIGNUM;
   NNG->numtri = 1;
   NNG->lasttemp = NNG->roottemp;
   for (i2=0; i2<3; i2++)
   {
      j1 = 0;
      if (j1 EQ i2) j1++;
      j2 = j1 + 1;
      if (j2 EQ i2) j2++;
      if (NNG->lasttemp->nexttemp EQ NULL) 
         if (! (NNG->lasttemp->nexttemp = Temp_New()))
         {
            success = 0;
            break;
	 }
      NNG->lasttemp = NNG->lasttemp->nexttemp;
      NNG->lasttemp->end[0] = NNG->cursimp->vert[j1];
      NNG->lasttemp->end[1] = NNG->cursimp->vert[j2];
   }
   if (! success)
      return (0);
   NNG->curtemp = NNG->roottemp;
   for (i1=0; i1<3; i1++)
   {
      NNG->curtemp = NNG->curtemp->nexttemp;
      for (i2=0; i2<2; i2++)
      {
         NNG->work3[i2][0] = NNG->points[NNG->curtemp->end[i2]][0] - 
            NNG->points[ipt][0];
         NNG->work3[i2][1] = NNG->points[NNG->curtemp->end[i2]][1] - 
            NNG->points[ipt][1];
         NNG->work3[i2][2] = NNG->work3[i2][0] * 
            (NNG->points[NNG->curtemp->end[i2]][0] + 
            NNG->points[ipt][0]) / 2 + NNG->work3[i2][1] * 
            (NNG->points[NNG->curtemp->end[i2]][1] + 
            NNG->points[ipt][1]) / 2;
      }
      NNG->xxxx = NNG->work3[0][0] * NNG->work3[1][1] - 
         NNG->work3[1][0] * NNG->work3[0][1];
      NNG->cursimp->cent[0] = (NNG->work3[0][2] * NNG->work3[1][1] - 
         NNG->work3[1][2] * NNG->work3[0][1]) / NNG->xxxx;
      NNG->cursimp->cent[1] = (NNG->work3[0][0] * NNG->work3[1][2] - 
         NNG->work3[1][0] * NNG->work3[0][2]) / NNG->xxxx;
      NNG->cursimp->cent[2] = SQ(NNG->points[ipt][0] - 
         NNG->cursimp->cent[0]) + SQ(NNG->points[ipt][1] - 
         NNG->cursimp->cent[1]);
      NNG->cursimp->vert[0] = NNG->curtemp->end[0];
      NNG->cursimp->vert[1] = NNG->curtemp->end[1];
      NNG->cursimp->vert[2] = ipt;
      NNG->lastsimp = NNG->cursimp;
      if (NNG->cursimp->nextsimp EQ NULL) 
         if (! (NNG->cursimp->nextsimp = Simp_New()))
         {
            success = 0;
            break;
	 }
      NNG->cursimp = NNG->cursimp->nextsimp; 
   }
   if (! success)
      return (0);
   NNG->numtri += 2;
   for (i0=0; i0<NNG->datcnt; i0++)
   {  if (i0 NE ipt)
      {  j4 = 0;
         j3 = -1;
         NNG->lasttemp = NNG->roottemp;
         NNG->cursimp = NNG->rootsimp;
         for (i1=0; i1<NNG->numtri; i1++)
         {
            NNG->prevsimp = NNG->cursimp;
            NNG->cursimp = NNG->cursimp->nextsimp;
            NNG->xxxx = NNG->cursimp->cent[2] - 
               SQ(NNG->points[i0][0] - NNG->cursimp->cent[0]);
            if (NNG->xxxx > 0)
            {
               NNG->xxxx -= SQ(NNG->points[i0][1] - 
                  NNG->cursimp->cent[1]);
               if (NNG->xxxx > 0)
               {
                 j4--;
                 for (i2=0; i2<3; i2++)
                 {
                   j1 = 0; 
                   if (j1 EQ i2) j1++; 
                   j2 = j1 + 1;    
                   if (j2 EQ i2) j2++;
                   if (j3>1)
                   {
                     j5 = j3;
                     NNG->curtemp = NNG->roottemp;
                     for (i3=0; i3<=j5; i3++)
                     {
                       NNG->prevtemp = NNG->curtemp;
                       NNG->curtemp =
                          NNG->curtemp->nexttemp;
                       if (NNG->cursimp->vert[j1] EQ 
                          NNG->curtemp->end[0])
                       {
                         if (NNG->cursimp->vert[j2] EQ 
                            NNG->curtemp->end[1])
                         {
                           if (NNG->curtemp EQ NNG->lasttemp) 
                              NNG->lasttemp = NNG->prevtemp;
                           else
                           {
                             NNG->prevtemp->nexttemp = 
                                NNG->curtemp->nexttemp;
                             NNG->curtemp->nexttemp = 
                                NNG->lasttemp->nexttemp;
                             NNG->lasttemp->nexttemp = 
                                NNG->curtemp;
                           }
                           j3--;
                           goto NextOne;
                         }
                       }
                     }
                   }
                   if (NNG->lasttemp->nexttemp EQ NULL) 
                      if (! (NNG->lasttemp->nexttemp = Temp_New()))
                      {
                         success = 0;
                         goto EndFind;
		      }
                   NNG->lasttemp = NNG->lasttemp->nexttemp;
                   j3++;
                   NNG->lasttemp->end[0] = 
                      NNG->cursimp->vert[j1];
                   NNG->lasttemp->end[1] = 
                      NNG->cursimp->vert[j2];
NextOne:; }
                  if (NNG->cursimp EQ NNG->lastsimp) 
                     NNG->lastsimp = NNG->prevsimp;
                  else
                  {
                     NNG->prevsimp->nextsimp = 
                        NNG->cursimp->nextsimp;
                     NNG->cursimp->nextsimp = 
                        NNG->lastsimp->nextsimp;
                     NNG->lastsimp->nextsimp = NNG->cursimp;
                     NNG->cursimp = NNG->prevsimp;
                  }
               }
            }
         }
         if (j3 > -1)
         {
            NNG->curtemp = NNG->roottemp;
            NNG->cursimp = NNG->lastsimp->nextsimp;
            for (i1=0; i1<=j3; i1++)
            {
               NNG->curtemp = NNG->curtemp->nexttemp;
               if (NNG->curtemp->end[0] EQ ipt OR 
                  NNG->curtemp->end[1] EQ ipt)
               {
                  for (i2=0; i2<2; i2++)
                  {
                     NNG->work3[i2][0] = 
                        NNG->points[NNG->curtemp->end[i2]][0] - 
                        NNG->points[i0][0];
                     NNG->work3[i2][1] = 
                        NNG->points[NNG->curtemp->end[i2]][1] - 
                        NNG->points[i0][1];
                     NNG->work3[i2][2] = NNG->work3[i2][0] * 
                        (NNG->points[NNG->curtemp->end[i2]][0] + 
                        NNG->points[i0][0]) / 2 + 
                        NNG->work3[i2][1] *
                        (NNG->points[NNG->curtemp->end[i2]][1] + 
                        NNG->points[i0][1]) / 2;
                  }
                  NNG->xxxx = NNG->work3[0][0] * NNG->work3[1][1] - 
                     NNG->work3[1][0] * NNG->work3[0][1];
                  NNG->cursimp->cent[0] = (NNG->work3[0][2] * 
                     NNG->work3[1][1] - NNG->work3[1][2] * 
                     NNG->work3[0][1]) / NNG->xxxx;
                  NNG->cursimp->cent[1] = (NNG->work3[0][0] * 
                     NNG->work3[1][2] - NNG->work3[1][0] * 
                     NNG->work3[0][2]) / NNG->xxxx;
                  NNG->cursimp->cent[2] = 
                     SQ(NNG->points[i0][0] - 
                     NNG->cursimp->cent[0]) +
                     SQ(NNG->points[i0][1] - 
                     NNG->cursimp->cent[1]);
                  NNG->cursimp->vert[0] = NNG->curtemp->end[0];
                  NNG->cursimp->vert[1] = NNG->curtemp->end[1];
                  NNG->cursimp->vert[2] = i0;
                  NNG->lastsimp = NNG->cursimp;
                  if (NNG->cursimp->nextsimp EQ NULL) 
                     if (! (NNG->cursimp->nextsimp = Simp_New()))
                     {
                        success = 0;
                        goto EndFind;
		     }
                  NNG->cursimp = NNG->cursimp->nextsimp; 
                  j4++;
               }
            }
            NNG->numtri += j4;
         }
      }
   }
   for (i0=0; i0<NNG->datcnt; i0++) NNG->jndx[i0] = 0;
   NNG->cursimp = NNG->rootsimp;
   for (NNG->ext=0, i1=0; i1<NNG->numtri; i1++)
   {
      NNG->cursimp = NNG->cursimp->nextsimp;
      for (i2=0; i2<3; i2++) 
      {
         if (NNG->cursimp->vert[i2] < NNG->datcnt)
         {
           if (NNG->cursimp->vert[i2] NE ipt) 
               NNG->jndx[NNG->cursimp->vert[i2]] = 1;
         }
         else NNG->ext = 1; 
      }
   }

EndFind:
 return (success);

} /* FindNeigh() */

/***********************************************************************/

short TriCentr(struct NNGrid *NNG)
{
short success = 1;
int i0, i1, i2, i3, j1, j2, j3, j4, j5;

   if (NNG->rootsimp->nextsimp EQ NULL) 
      if (! (NNG->rootsimp->nextsimp = Simp_New()))
         return (0);
   NNG->lastsimp = NNG->cursimp = NNG->rootsimp->nextsimp;
   NNG->cursimp->vert[0] = NNG->datcnt;
   NNG->cursimp->vert[1] = NNG->datcnt + 1;
   NNG->cursimp->vert[2] = NNG->datcnt + 2;
   NNG->cursimp->cent[0] = NNG->cursimp->cent[1] = 0.5;
   NNG->cursimp->cent[2] = BIGNUM;
   NNG->numtri = 1;
   for (i0=0; i0<NNG->neicnt; i0++)
   {  j3 = -1;
      j4 = 0;
      NNG->lasttemp = NNG->roottemp;
      NNG->cursimp = NNG->rootsimp;
      for (i1=0; i1<NNG->numtri; i1++)
      {  NNG->prevsimp = NNG->cursimp;
         NNG->cursimp = NNG->cursimp->nextsimp;
         NNG->xxxx = NNG->cursimp->cent[2] - SQ(NNG->joints[i0][0] - 
            NNG->cursimp->cent[0]);
         if (NNG->xxxx > 0)
         {  NNG->xxxx -= SQ(NNG->joints[i0][1] - 
               NNG->cursimp->cent[1]);
            if (NNG->xxxx > 0)
            {  j4--;
               for (i2=0; i2<3; i2++)
               {  j1 = 0;
                  if (j1 EQ i2) j1++;
                  j2 = j1 + 1;
                  if (j2 EQ i2) j2++;
                  if (j3>1)
                  {  j5 = j3;
                     NNG->curtemp = NNG->roottemp;
                     for (i3=0; i3<=j5; i3++)
                     {  NNG->prevtemp = NNG->curtemp;
                        NNG->curtemp = NNG->curtemp->nexttemp;
                        if (NNG->cursimp->vert[j1] EQ 
                           NNG->curtemp->end[0])
                        {  if (NNG->cursimp->vert[j2] EQ 
                              NNG->curtemp->end[1])
                           {  if (NNG->curtemp EQ NNG->lasttemp) 
                                 NNG->lasttemp = NNG->prevtemp;
                              else
                              {  NNG->prevtemp->nexttemp = 
                                   NNG->curtemp->nexttemp;
                                 NNG->curtemp->nexttemp = 
                                   NNG->lasttemp->nexttemp;
                                 NNG->lasttemp->nexttemp = 
                                   NNG->curtemp;
                              }
                              j3--;
                              goto NextOne;
                           }
                        }
                     }
                  }
                  if (NNG->lasttemp->nexttemp EQ NULL) 
                     if (! (NNG->lasttemp->nexttemp = Temp_New()))
                     {
                        success = 0;
                        goto EndTri;
		     }
                  NNG->lasttemp = NNG->lasttemp->nexttemp;
                  j3++;
                  NNG->lasttemp->end[0] = 
                     NNG->cursimp->vert[j1];
                  NNG->lasttemp->end[1] = 
                     NNG->cursimp->vert[j2];
NextOne:;      }
               if (NNG->cursimp EQ NNG->lastsimp) 
                  NNG->lastsimp = NNG->prevsimp;
               else
               {  NNG->prevsimp->nextsimp = 
                     NNG->cursimp->nextsimp;
                  NNG->cursimp->nextsimp = 
                     NNG->lastsimp->nextsimp;
                  NNG->lastsimp->nextsimp = NNG->cursimp;
                  NNG->cursimp = NNG->prevsimp;
               }
            }
         }
      }
      NNG->curtemp = NNG->roottemp;
      NNG->cursimp = NNG->lastsimp->nextsimp;
      for (i1=0; i1<=j3; i1++)
      {  NNG->curtemp = NNG->curtemp->nexttemp;
         for (i2=0; i2<2; i2++)
         {  NNG->work3[i2][0] = 
               NNG->joints[NNG->curtemp->end[i2]][0] - 
               NNG->joints[i0][0];
            NNG->work3[i2][1] = 
               NNG->joints[NNG->curtemp->end[i2]][1] - 
               NNG->joints[i0][1];
            NNG->work3[i2][2] = NNG->work3[i2][0] * 
               (NNG->joints[NNG->curtemp->end[i2]][0] + 
               NNG->joints[i0][0]) / 2 + 
               NNG->work3[i2][1] * 
               (NNG->joints[NNG->curtemp->end[i2]][1] + 
               NNG->joints[i0][1]) / 2;
         }
         NNG->xxxx = NNG->work3[0][0] * NNG->work3[1][1] - 
            NNG->work3[1][0] * NNG->work3[0][1];
         NNG->cursimp->cent[0] = 
            (NNG->work3[0][2] * NNG->work3[1][1] - 
            NNG->work3[1][2] * NNG->work3[0][1]) / NNG->xxxx;
         NNG->cursimp->cent[1] = 
            (NNG->work3[0][0] * NNG->work3[1][2] - 
            NNG->work3[1][0] * NNG->work3[0][2]) / NNG->xxxx;
         NNG->cursimp->cent[2] = 
            SQ(NNG->joints[i0][0] - NNG->cursimp->cent[0]) +
            SQ(NNG->joints[i0][1] - NNG->cursimp->cent[1]);
         NNG->cursimp->vert[0] = NNG->curtemp->end[0];
         NNG->cursimp->vert[1] = NNG->curtemp->end[1];
         NNG->cursimp->vert[2] = i0;
         NNG->lastsimp = NNG->cursimp;
         if (NNG->cursimp->nextsimp EQ NULL) 
            if (! (NNG->cursimp->nextsimp = Simp_New()))
            {
               success = 0;
               goto EndTri;
	    }
         NNG->cursimp = NNG->cursimp->nextsimp;   
         j4++;
      }
      NNG->numtri += j4;
   }

EndTri:
   return (success);

} /* TriCentr() */

/***********************************************************************/

short TriNeigh(struct NNGrid *NNG)
{
short success = 1;
int i0, i1, i2, i3, j1, j2, j3, j4, j5;

   if (NNG->rootsimp->nextsimp EQ NULL) 
      if (! (NNG->rootsimp->nextsimp = Simp_New()))
         return (0);
   NNG->lastsimp = NNG->cursimp = NNG->rootsimp->nextsimp;
   NNG->cursimp->vert[0] = NNG->datcnt;
   NNG->cursimp->vert[1] = NNG->datcnt + 1;
   NNG->cursimp->vert[2] = NNG->datcnt + 2;
   NNG->cursimp->cent[0] = NNG->cursimp->cent[1] = 0.5;
   NNG->cursimp->cent[2] = BIGNUM;
   NNG->numtri = 1;
   for (i0=0; i0<NNG->datcnt; i0++)
   {  if (NNG->jndx[i0])
      {  j3 = -1;
         NNG->lasttemp = NNG->roottemp;
         NNG->cursimp = NNG->rootsimp;
         for (i1=0; i1<NNG->numtri; i1++)
         {  NNG->prevsimp = NNG->cursimp;
            NNG->cursimp = NNG->cursimp->nextsimp;
            NNG->xxxx = NNG->cursimp->cent[2] - 
               SQ(NNG->points[i0][0] - NNG->cursimp->cent[0]);
            if (NNG->xxxx > 0)
            {  NNG->xxxx -= SQ(NNG->points[i0][1] - 
                  NNG->cursimp->cent[1]);
               if (NNG->xxxx > 0)
               {  for (i2=0; i2<3; i2++)
                  {  j1 = 0;
                     if (j1 EQ i2) j1++;
                     j2 = j1 + 1;
                     if (j2 EQ i2) j2++;
                     if (j3>1)
                     {  j5 = j3;
                        NNG->curtemp = NNG->roottemp;
                        for (i3=0; i3<=j5; i3++)
                        { NNG->prevtemp = NNG->curtemp;
                          NNG->curtemp = 
                             NNG->curtemp->nexttemp;
                          if (NNG->cursimp->vert[j1] EQ 
                             NNG->curtemp->end[0])
                          { if (NNG->cursimp->vert[j2] EQ 
                               NNG->curtemp->end[1])
                            { if (NNG->curtemp EQ NNG->lasttemp) 
                                 NNG->lasttemp = NNG->prevtemp;
                              else
                              { NNG->prevtemp->nexttemp = 
                                  NNG->curtemp->nexttemp;
                                NNG->curtemp->nexttemp = 
                                  NNG->lasttemp->nexttemp;
                                NNG->lasttemp->nexttemp = 
                                  NNG->curtemp;
                              }
                              j3--;
                              goto NextOne;
                            }
                          }
                        }
                     }
                     if (NNG->lasttemp->nexttemp EQ NULL)
                     if (! (NNG->lasttemp->nexttemp = Temp_New()))
                     {
                        success = 0;
                        goto EndTri;
		     }
                     NNG->lasttemp = NNG->lasttemp->nexttemp;
                     j3++;
                     NNG->lasttemp->end[0] = 
                        NNG->cursimp->vert[j1];
                     NNG->lasttemp->end[1] = 
                        NNG->cursimp->vert[j2];
NextOne:; }
                  if (NNG->cursimp EQ NNG->lastsimp) 
                     NNG->lastsimp = NNG->prevsimp;
                  else
                  {  NNG->prevsimp->nextsimp = 
                        NNG->cursimp->nextsimp;
                     NNG->cursimp->nextsimp = 
                        NNG->lastsimp->nextsimp;
                     NNG->lastsimp->nextsimp = NNG->cursimp;
                     NNG->cursimp = NNG->prevsimp;
                  }
               }
            }
         }
         NNG->curtemp = NNG->roottemp;
         NNG->cursimp = NNG->lastsimp->nextsimp;
         for (i1=0; i1<=j3; i1++)
         {  NNG->curtemp = NNG->curtemp->nexttemp;
            for (i2=0; i2<2; i2++)
            { NNG-> work3[i2][0] = 
                  NNG->points[NNG->curtemp->end[i2]][0] - 
                  NNG->points[i0][0];
               NNG->work3[i2][1] = 
                  NNG->points[NNG->curtemp->end[i2]][1] - 
                  NNG->points[i0][1];
               NNG->work3[i2][2] = NNG->work3[i2][0] * 
                  (NNG->points[NNG->curtemp->end[i2]][0] + 
                  NNG->points[i0][0]) / 2 + NNG->work3[i2][1] * 
                  (NNG->points[NNG->curtemp->end[i2]][1] + 
                  NNG->points[i0][1]) / 2;
            }
            NNG->xxxx = NNG->work3[0][0] * NNG->work3[1][1] - 
               NNG->work3[1][0] * NNG->work3[0][1];
            NNG->cursimp->cent[0] = 
               (NNG->work3[0][2] * NNG->work3[1][1] - 
               NNG->work3[1][2] * NNG->work3[0][1]) / NNG->xxxx;
            NNG->cursimp->cent[1] = 
               (NNG->work3[0][0] * NNG->work3[1][2] - 
               NNG->work3[1][0] * NNG->work3[0][2]) / NNG->xxxx;
            NNG->cursimp->cent[2] = SQ(NNG->points[i0][0] - 
               NNG->cursimp->cent[0]) + SQ(NNG->points[i0][1] - 
               NNG->cursimp->cent[1]);
            NNG->cursimp->vert[0] = NNG->curtemp->end[0];
            NNG->cursimp->vert[1] = NNG->curtemp->end[1];
            NNG->cursimp->vert[2] = i0;
            NNG->lastsimp = NNG->cursimp;
            if (NNG->cursimp->nextsimp EQ NULL) 
               if (! (NNG->cursimp->nextsimp = Simp_New()))
               {
                  success = 0;
                  goto EndTri;
	       }
            NNG->cursimp = NNG->cursimp->nextsimp;
         }
         NNG->numtri += 2;
      }
   }
   NNG->cursimp = NNG->rootsimp;
   for (NNG->asum=0, i0=0; i0<NNG->numtri; i0++) 
   {  NNG->cursimp = NNG->cursimp->nextsimp;
      for (i1=0; i1<2; i1++)
      {  NNG->work3[0][i1] = NNG->points[NNG->cursimp->vert[1]][i1] - 
            NNG->points[NNG->cursimp->vert[0]][i1];
         NNG->work3[1][i1] = NNG->points[NNG->cursimp->vert[2]][i1] - 
            NNG->points[NNG->cursimp->vert[0]][i1];
      }
      NNG->xxxx = NNG->work3[0][0] * NNG->work3[1][1] - 
         NNG->work3[0][1] * NNG->work3[1][0];
      if (NNG->xxxx < 0)
      {  j4 = NNG->cursimp->vert[2];
         NNG->cursimp->vert[2] = NNG->cursimp->vert[1];
         NNG->cursimp->vert[1] = j4;
         if (NNG->cursimp->vert[0] < NNG->datcnt) 
            NNG->asum -= NNG->xxxx / 2;
      }
      else if (NNG->cursimp->vert[0] < NNG->datcnt) 
         NNG->asum += NNG->xxxx / 2;
   }

EndTri:
   return (success);

} /* TriNeigh() */

/***********************************************************************/

short MakeGrid(struct NNGrid *NNG, short Units)
{
char ObjName[32];
short success = 1, col, *SaveMap, *SavePtr;
float *GridPtr;
double wxd, wyd, surf, x_increm, y_increm,
        MaxAmp, MinAmp, RangeAmp, DataRow, DataCol, LatStep, LonStep;
long i0, x, y, k, Low_X, Low_Y, High_X, High_Y, zip, RowZip, Row, Col,
	SaveSize;
struct BusyWindow *BWGR;
struct clipbounds cb;
struct elmapheaderV101 Hdr;

   NNG->GridSize = NNG->x_nodes * NNG->y_nodes * sizeof (float);
   if ((NNG->Grid = (float *)get_Memory(NNG->GridSize, MEMF_ANY)) != NULL)
    {
    GridPtr = NNG->Grid;
    for (i0=0; i0<NNG->datcnt; i0++)
     NNG->jndx[i0] = 1;
    if (! TriNeigh(NNG))
     {
     success = 0;
     goto EndGrid;
     } /* if */
    x_increm = ((.00000001 + NNG->xterm - NNG->xstart) * NNG->magx) /
         (float)(NNG->x_nodes-1);   /* add scale factor 24/4/95 */
    y_increm = ((.00000001 + NNG->yterm - NNG->ystart) * NNG->magy) /
         (float)(NNG->y_nodes-1);   /* add scale factor 24/4/95 */
    wyd = NNG->ystart * NNG->magy - y_increm;   /* add scale factor 24/4/95 */
    if (NNG->arriba < 0)
     wyd = NNG->yterm * NNG->magy + y_increm;   /* add scale factor 24/4/95 */
    BWGR = BusyWin_New("Gridding", NNG->y_nodes, 0, 'BWGR');
    for (y=0; y<NNG->y_nodes; y++) 
     {
     wyd += y_increm * NNG->arriba;
     NNG->points[NNG->datcnt3][1] = wyd;
     wxd = NNG->xstart * NNG->magx - x_increm;   /* add scale factor 24/4/95 */
     for (x=0; x<NNG->x_nodes; x++, GridPtr++) 
      {
      wxd += x_increm;
      NNG->points[NNG->datcnt3][0] = wxd;
      if (! Find_Prop(NNG, wxd, wyd))
       {
       success = 0;
       goto EndGrid;
       } /* if */
      if (! NNG->extrap AND ! NNG->goodflag)
       surf = NNG->nuldat;
      else
       {
       surf = Surface(NNG);
       if (NNG->igrad > 0)
        surf = Meld(NNG, surf, wxd, wyd);
       if (NNG->non_neg)
        {
        if (surf < 0)
         surf = 0;
	} /* if non-neg */
       } /* else */
      if (NNG->imag && ! NNG->densi)
/* should this depend on whether it is choropleth or just density? */
       surf /= NNG->magz;
      *GridPtr = surf;
      } /* for x=0... */
     if (CheckInput_ID() == ID_BW_CLOSE)
      {
      success = 0;
      break;
      } /* if user abort */
     BusyWin_Update(BWGR, y + 1);
     } /* for y=0... */
    if (BWGR) BusyWin_Del(BWGR);

/* display map */
    if (MapWind0)
     {
     setclipbounds(MapWind0, &cb);

     Low_X = Lon_X_Convert(-NNG->xstart);
     Low_Y = Lat_Y_Convert(NNG->yterm);
     High_X = Lon_X_Convert(-NNG->xterm);
     High_Y = Lat_Y_Convert(NNG->ystart);
     LatStep = ((double)(NNG->y_nodes - 1)) / ((double)(High_Y - Low_Y));
     LonStep = ((double)(NNG->x_nodes - 1)) / ((double)(High_X - Low_X));

/* find largest data amplitudes */

     zip = 0;
     MaxAmp = -100000.0;
     MinAmp = 100000.0;
     for (y=0; y<NNG->y_nodes; y++)
      {
      for (x=0; x<NNG->x_nodes; x++, zip++)
       {
       if (NNG->Grid[zip] > MaxAmp)
        MaxAmp = NNG->Grid[zip];
       if (NNG->Grid[zip] < MinAmp)
        MinAmp = NNG->Grid[zip];
       } /* for y=... */
      } /* for x=... */
     MaxAmp += 1.0;
     MaxAmp -= 1.0;
     RangeAmp = MaxAmp - MinAmp;

/* plot color in Map View, brighter indicates higher amplitude */

     BWGR = BusyWin_New("Drawing...", High_Y - Low_Y + 1, 0, 'BWGR');

     DataRow = 0.0;
     for (y=Low_Y, k=0; y<=High_Y; y++, DataRow+=LatStep, k++)
      {
      if (y < cb.lowy)
       continue;
      if (y > cb.highy)
       break;
      Row = DataRow;
      RowZip = Row * NNG->x_nodes;
      DataCol = 0.0;
      for (x=Low_X; x<=High_X; x++, DataCol+=LonStep)
       {
       if (x < cb.lowx)
        continue;
       if (x > cb.highx)
        break;
       Col = DataCol;
       zip = RowZip + Col;

       col = 15.99 - ((NNG->Grid[zip] - MinAmp) / RangeAmp) * 7.99;
       if (col < 8)
        col = 8;
       SetAPen(MapWind0->RPort, col);
       WritePixel(MapWind0->RPort, x, y);
       } /* for x=... */
      BusyWin_Update(BWGR, k + 1);
      } /* for y=... */

     if (BWGR) BusyWin_Del(BWGR);
     } /* if MapWind0 */

/* save DEM */

    Hdr.rows = NNG->x_nodes - 1;
    Hdr.columns = NNG->y_nodes;

    Hdr.lolong = -NNG->xstart;
    Hdr.lolat = NNG->ystart;
    Hdr.steplat = (NNG->yterm - NNG->ystart) / (NNG->y_nodes - 1);
    Hdr.steplong = (NNG->xterm - NNG->xstart) / (NNG->x_nodes - 1);
    switch (Units)
     {
     case 0:
      {
      Hdr.elscale = ELSCALE_KILOM;
      break;
      } /* kilometers */
     case 1:
      {
      Hdr.elscale = ELSCALE_METERS;
      break;
      } /* meters */
     case 2:
      {
      Hdr.elscale = ELSCALE_CENTIM;
      break;
      } /* centimeters */
     case 3:
      {
      Hdr.elscale = ELSCALE_MILES;
      break;
      } /* miles */
     case 4:
      {
      Hdr.elscale = ELSCALE_FEET;
      break;
      } /* feet */
     case 5:
      {
      Hdr.elscale = ELSCALE_INCHES;
      break;
      } /* inches */
     } /* switch */

    SaveSize = NNG->GridSize / 2;
    if ((SaveMap = (short *)get_Memory(SaveSize, MEMF_ANY)) != NULL)
     {
     SavePtr = SaveMap;
     for (x=0; x<NNG->x_nodes; x++)
      {
      GridPtr = NNG->Grid + (NNG->y_nodes - 1) * NNG->x_nodes + x;
      for (y=NNG->y_nodes; y>0; y--, SavePtr++, GridPtr -= NNG->x_nodes)
       {
       *SavePtr = *GridPtr;
       } /* for y=... */
      } /* for x=... */
     strcpy(ObjName, NNG->grd_file);
     ObjName[length[0]] = 0;
     while(strlen(ObjName) < length[0])
      strcat(ObjName, " ");
     DEMFile_Save(ObjName, &Hdr, SaveMap, SaveSize);

     free_Memory(SaveMap, SaveSize);
     } /* if memory OK */

    } /* if memory allocated for grid block */

EndGrid:

   return (success);

} /* MakeGrid() */

/***********************************************************************/

short Find_Prop(struct NNGrid *NNG, double wxd, double wyd)
{
short success = 1;
int i2, i3, i4, pos_count, inside;
double xx, work3[3][3], work4[3][2];

   NNG->lastneig = NNG->rootneig;
   NNG->goodflag = 0;
   NNG->numnei = -1;
   NNG->cursimp = NNG->rootsimp;
   for (i2=0; i2<NNG->numtri; i2++)
   {  NNG->cursimp = NNG->cursimp->nextsimp;
      xx = NNG->cursimp->cent[2] - 
         SQ(wxd - NNG->cursimp->cent[0]);
      if (xx > 0)
      {  xx -= SQ(wyd - NNG->cursimp->cent[1]);
         if (xx > 0)
         {  inside = 0;
            if (NNG->cursimp->vert[0] < NNG->datcnt) inside = 1;
            for (i3=0; i3<3; i3++)
            {  for (i4=0; i4<2; i4++)
               {  work3[i4][0] = 
                     NNG->points[NNG->cursimp->vert[NNG->scor[i3][i4]]][0] - wxd;
                  work3[i4][1] = 
                     NNG->points[NNG->cursimp->vert[NNG->scor[i3][i4]]][1] - wyd;
                  work3[i4][2] = work3[i4][0] *
                     (NNG->points[NNG->cursimp->vert[NNG->scor[i3][i4]]][0] + 
                     wxd) / 2 + work3[i4][1] *
                     (NNG->points[NNG->cursimp->vert[NNG->scor[i3][i4]]][1] + 
                     wyd) / 2;
               }
               xx =  work3[0][0] * work3[1][1] - 
                  work3[1][0] * work3[0][1];
               work4[i3][0] = (work3[0][2] * 
                  work3[1][1] - work3[1][2] * 
                  work3[0][1]) / xx;
               work4[i3][1] = (work3[0][0] * 
                  work3[1][2] - work3[1][0] * 
                  work3[0][2]) / xx;
            }
            pos_count = 0;
            for (i3=0; i3<3; i3++)
            {  work3[2][i3] = 
                  ((work4[NNG->scor[i3][0]][0] - 
                  NNG->cursimp->cent[0]) *
                  (work4[NNG->scor[i3][1]][1] - 
                  NNG->cursimp->cent[1]) -
                  (work4[NNG->scor[i3][1]][0] - 
                  NNG->cursimp->cent[0]) *
                  (work4[NNG->scor[i3][0]][1] - 
                  NNG->cursimp->cent[1])) / 2;
               if (work3[2][i3]>0) pos_count++;
            }
            if (pos_count>2 AND inside) NNG->goodflag = 1;
            for (i3=0; i3<3; i3++)
            {  if (NNG->numnei>1)
               {  NNG->curneig = NNG->rootneig;
                  for (i4=0; i4<=NNG->numnei; i4++)
                  {  NNG->curneig = NNG->curneig->nextneig;
                     if (NNG->cursimp->vert[i3] EQ 
                        NNG->curneig->neinum)
                     {  NNG->curneig->narea += 
                           work3[2][i3];
                        goto GOTEM;
                     }
                  }
               }
               if (NNG->lastneig->nextneig EQ NULL) 
                  if (! (NNG->lastneig->nextneig = Neig_New()))
                  {
                     success = 0;
                     goto EndFind;
		  }
               NNG->lastneig = NNG->lastneig->nextneig;
               NNG->numnei++;
               NNG->lastneig->neinum = NNG->cursimp->vert[i3];
               NNG->lastneig->narea = work3[2][i3];
GOTEM:;       }   
         }
      }
   }

EndFind:
   return (success);

} /* Find_Prop() */

/***********************************************************************/

double Surface(struct NNGrid *NNG)
{
int i0;
double xx, asurf;

   NNG->curneig = NNG->rootneig;
   for (xx=0, i0=0; i0<=NNG->numnei; i0++) 
   {  NNG->curneig = NNG->curneig->nextneig;
      xx += NNG->curneig->narea;
   }
   NNG->curneig = NNG->rootneig;
   for (asurf=0, i0=0; i0<=NNG->numnei; i0++)
   {  NNG->curneig = NNG->curneig->nextneig;
      NNG->curneig->narea /= xx;
      asurf += NNG->curneig->narea * 
         NNG->points[NNG->curneig->neinum][2];
   }
   return asurf;
}

/***********************************************************************/

double Meld(struct NNGrid *NNG, double asurf, double wxd, double wyd)
{
int i0;
double rS, rT, rB, bD, bB, hP;

   NNG->curneig = NNG->rootneig;
   for (i0=0; i0<=NNG->numnei; i0++)
   {  NNG->curneig = NNG->curneig->nextneig;
      NNG->curneig->coord = 0.0;
      if (NNG->curneig->narea>0.00001 AND 
         NNG->curneig->narea < 2.0)
      {  if (fabs(NNG->points[NNG->curneig->neinum][5]) > 
            0.00001)
         {  rS = fabs(NNG->points[NNG->curneig->neinum][5]) + 
               NNG->bI;
            rT = rS * NNG->bJ;
            rB = 1.0 / rT;
            bD = pow(NNG->curneig->narea, rT);
            bB = bD * 2.0;
            if (bD>0.5) bB = (1.0 - bD) * 2.0;
            bB = pow(bB, rS) / 2.0;
            if (bD>0.5) bB = 1.0 - bB;
            hP = pow(bB, rB);
            NNG->curneig->coord = 
               ((NNG->points[NNG->curneig->neinum][3] *
               NNG->points[NNG->curneig->neinum][0] + 
               NNG->points[NNG->curneig->neinum][4] *
               NNG->points[NNG->curneig->neinum][1] + 
               NNG->points[NNG->curneig->neinum][2] -
               NNG->points[NNG->curneig->neinum][3] * 
               wxd -
               NNG->points[NNG->curneig->neinum][4] * 
               wyd) - asurf) * hP;
         }
      }
   }
   NNG->curneig = NNG->rootneig;
   for (i0=0; i0<=NNG->numnei; i0++) 
   {  NNG->curneig = NNG->curneig->nextneig;
      asurf += NNG->curneig->coord;
   }
   return asurf; 
}

/***********************************************************************/

short TooSteep(struct NNGrid *NNG)
{

   if (User_Message("Map View: Grid DEM",
   "The ratio of vertical to horizontal map dimensions is too large for\
 gradient estimation. Scale the data if gradients are required.\nDo you wish to\
 continue without gradient estimation?", "Continue|Cancel", "oc"))
    {
    NNG->igrad = 0;
    return (1);
    }
   return (0);

} /* TooSteep() */

/***********************************************************************/

short TooShallow(struct NNGrid *NNG)
{

   if (User_Message("Map View: Grid DEM",
   "The ratio of vertical to horizontal map dimensions is too small for\
 gradient estimation. Scale the data if gradients are required.\nDo you wish to\
 continue without gradient estimation?", "Continue|Cancel", "oc"))
    {
    NNG->igrad = 0;
    return (1);
    }
   return (0);

} /* TooShallow() */

/***********************************************************************/

short TooNarrow(struct NNGrid *NNG)
{

   if (User_Message("Map View: Grid DEM",
   "The ratio of width to length of this gridded region may be too extreme for\
 good interpolation.\nChanging the block proportions, or rescaling the\
 x or y coordinate may be a good idea.\nContinue now with the present\
 dimensions?",
 "Continue|Cancel", "oc"))
    {
    return (1);
    }
   return (0);

} /* TooNarrow() */

/***********************************************************************/

int *IntVect(int ncols)
{

   return ((int *)get_Memory(ncols * sizeof (int), MEMF_ANY));

} /* IntVect() */

/***********************************************************************/

void FreeVecti(int *vectptr, int ncols)
{

   if (vectptr)
    free_Memory(vectptr, ncols * sizeof (int));

} /* FreeVecti() */

/***********************************************************************/

double **DoubleMatrix(int nrows, int ncols)
{
int i0;
double **matptr;

 if (nrows<2) nrows = 2;
 if (ncols<2) ncols = 2;
 if ((matptr = (double **) 
      get_Memory(nrows * sizeof(double *), MEMF_ANY)) EQ NULL)
  {
  User_Message("Map View: Build DEM",
	"Out of memory Double Matrix!\nOperation terminated.", "OK", "o");
  } /* if */
 else
  {
  if ((matptr[0] = (double *) 
          get_Memory(nrows * ncols * sizeof(double), MEMF_ANY)) EQ NULL)
   {
   User_Message("Map View: Build DEM",
	"Out of memory allocating Double Matrix!\nOperation terminated.", "OK", "o");
   } /* if */
  else
   {
   for (i0=1; i0<nrows; i0++) 
             matptr[i0] = matptr[0] + i0 * ncols;
   } /* else */
  } /* else */

 return matptr;

} /* DouleMatrix() */

/***********************************************************************/

void FreeMatrixd(double **matptr, int nrows, int ncols)
{

 if (matptr)
  {
  if (matptr[0])
   free_Memory(matptr[0], nrows * ncols * sizeof (double));
  free_Memory(matptr, nrows * sizeof (double *));
  } /* if */

} /* FreeMatrixd() */

/***********************************************************************/

struct datum *Datum_New(void)
{

 return ((struct datum *)get_Memory(sizeof(struct datum), MEMF_CLEAR));

} /* Datum_New() */

/***********************************************************************/

void Datum_Del(struct datum *CD)
{
struct datum *LD;

 while (CD)
  {
  LD = CD;
  CD = CD->nextdat;
  free_Memory(LD, sizeof (struct datum));
  } /* while */

} /* Datum_Del() */

/***********************************************************************/

struct simp *Simp_New(void)
{

   return ((struct simp *)get_Memory(sizeof(struct simp), MEMF_CLEAR));

} /* Simp_New() */

/***********************************************************************/

void Simp_Del(struct simp *CS)
{
struct simp *LS;

 while (CS)
  {
  LS = CS;
  CS = CS->nextsimp;
  free_Memory(LS, sizeof (struct simp));
  } /* while */

} /* Simp_Del() */

/***********************************************************************/

struct temp *Temp_New(void)
{

 return ((struct temp *)get_Memory(sizeof(struct temp), MEMF_CLEAR));

} /* Temp_New() */

/***********************************************************************/

void Temp_Del(struct temp *CT)
{
struct temp *LT;

 while (CT)
  {
  LT = CT;
  CT = CT->nexttemp;
  free_Memory(LT, sizeof (struct temp));
  } /* while */

} /* Temp_Del() */
 
/***********************************************************************/

struct neig *Neig_New(void)
{

 return ((struct neig *)get_Memory(sizeof(struct neig), MEMF_CLEAR));

} /* Neig_New() */

/***********************************************************************/

void Neig_Del(struct neig *CN)
{
struct neig *LN;

 while (CN)
  {
  LN = CN;
  CN = CN->nextneig;
  free_Memory(LN, sizeof (struct neig));
  } /* while */

} /* Neig_Del() */
