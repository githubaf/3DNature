/* MakeFaces.c (ne gismakefaces.c 14 Jan 1994 CXH)
** Includes routines for computing face attributes from DEM data.
** Result can be exported directly to rendering module or
** written to .face file for later recall and increased rendering speed.
** Original code written by Gary R. Huber in June, 1992. Modified July 30,
** 1993 for integration into GIS.
*/

#include "WCS.h"


static void facecompute(struct elmapheaderV101 *map, short low, short mid, short hi); // used locally only -> static, AF 19.7.2021
static void Face_Compute(struct elmapheaderV101 *map, struct FaceData *Data,
        struct faces *Face, short low, short mid, short hi); // used locally only -> static, AF 20.7.2021
static short writeface(struct elmapheaderV101 *map); // used locally only -> static, AF 20.7.2021
static short FacePt_Order(struct FaceData *Data); // used locally only -> static, AF 26.7.2021
static short minmax(void); // used locally only -> static, AF 26.7.2021



void FaceOne_Setup(struct elmapheaderV101 *map, struct faces *Face,
	long Lr, long Lc)
{
long BaseCt;
struct FaceData Data;

 BaseCt = Lr * map->columns + Lc;
 Data.El[0]  = map->lmap[BaseCt];
 Data.El[1]  = map->lmap[BaseCt + map->columns];
 Data.El[2]  = map->lmap[BaseCt + 1];
 Data.Lat[0] = map->lolat + Lc * map->steplat;
 Data.Lat[1] = Data.Lat[0];
 Data.Lat[2] = map->lolat + (Lc + 1) * map->steplat;
 Data.Lon[0] = map->lolong - Lr * map->steplong;
 Data.Lon[1] = map->lolong - (Lr + 1) * map->steplong;
 Data.Lon[2] = Data.Lon[0];

 FacePt_Switch(map, &Data, Face);

} /* FaceOne_Setup() */

/***********************************************************************/

void FaceTwo_Setup(struct elmapheaderV101 *map, struct faces *Face,
	long Lr, long Lc)
{
long BaseCt;
struct FaceData Data;

 BaseCt = (Lr + 1) * map->columns + Lc;
 Data.El[0]  = map->lmap[BaseCt];
 Data.El[1]  = map->lmap[BaseCt - map->columns];
 Data.El[2]  = map->lmap[BaseCt - 1];
 Data.Lat[0] = map->lolat + Lc * map->steplat;
 Data.Lat[1] = Data.Lat[0];
 Data.Lat[2] = map->lolat + (Lc - 1) * map->steplat;
 Data.Lon[0] = map->lolong - (Lr + 1) * map->steplong;
 Data.Lon[1] = map->lolong - Lr * map->steplong;
 Data.Lon[2] = Data.Lon[0];

 FacePt_Switch(map, &Data, Face);

} /* FaceTwo_Setup() */

/***********************************************************************/

void FractFaceOne_Setup(struct elmapheaderV101 *map, long Lr, long Lc)
{

 latface[0] = map->lolat + Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = map->lolat + (Lc + 1) * map->steplat;
 longface[0] = map->lolong - Lr * map->steplong;
 longface[1] = map->lolong - (Lr + 1) * map->steplong;
 longface[2] = longface[0];

} /* FractFaceOne_Setup() */

/***********************************************************************/

void FractFaceTwo_Setup(struct elmapheaderV101 *map, long Lr, long Lc)
{

 latface[0] = map->lolat + Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = map->lolat + (Lc - 1) * map->steplat;
 longface[0] = map->lolong - (Lr + 1) * map->steplong;
 longface[1] = map->lolong - Lr * map->steplong;
 longface[2] = longface[0];

} /* FractFaceTwo_Setup() */

/***********************************************************************/
/* these are called from the original render code as well as from eco mapping
in Map View */
short faceone(struct elmapheaderV101 *map)
{

 elface[0] = *(map->lmap + map->facept[0]);
 elface[1] = *(map->lmap + map->facept[1]);
 elface[2] = *(map->lmap + map->facept[2]);
 latface[0] = map->lolat + map->Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = map->lolat + (map->Lc + 1) * map->steplat;
 longface[0] = map->lolong - map->Lr * map->steplong;
 longface[1] = map->lolong - (map->Lr + 1) * map->steplong;
 longface[2] = longface[0];

 return(writeface(map));

} /* faceone() */

/***********************************************************************/

short facetwo(struct elmapheaderV101 *map)
{

 elface[0] = *(map->lmap + map->facept[0]);
 elface[1] = *(map->lmap + map->facept[1]);
 elface[2] = *(map->lmap + map->facept[2]);
 latface[0] = map->lolat + map->Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = map->lolat + (map->Lc - 1) * map->steplat;
 longface[0] = map->lolong - (map->Lr + 1) * map->steplong;
 longface[1] = map->lolong - map->Lr * map->steplong;
 longface[2] = longface[0];

 return(writeface(map));

} /* facetwo() */

/***********************************************************************/

void FacePt_Switch(struct elmapheaderV101 *map, struct FaceData *Data,
	struct faces *Face)
{

 switch (FacePt_Order(Data))
  {
  case 1:
   {
   Face_Compute(map, Data, Face, 0, 1, 2);
   break;
   }
  case 2:
   {
   Face_Compute(map, Data, Face, 0, 2, 1);
   break;
   }
  case 3:
   {
   Face_Compute(map, Data, Face, 2, 0, 1);
   break;
   }
  case 4:
   {
   Face_Compute(map, Data, Face, 1, 0, 2);
   break;
   }
  case 5:
   {
   Face_Compute(map, Data, Face, 1, 2, 0);
   break;
   }
  default:
   {
   Face_Compute(map, Data, Face, 2, 1, 0);
   }
  }/* switch */

} /* FacePt_Switch() */

/********************************************************************/
/* this can be eliminated when the calls to faceone, facetwo are eliminated */
static short writeface(struct elmapheaderV101 *map) // used locally only -> static, AF 20.7.2021
{
 switch (minmax()) {
  case 1:
   facecompute(map, 0, 1, 2);
   break;
  case 2:
   facecompute(map, 0, 2, 1);
   break;
  case 3:
   facecompute(map, 2, 0, 1);
   break;
  case 4:
   facecompute(map, 1, 0, 2);
   break;
  case 5:
   facecompute(map, 1, 2, 0);
   break;
  default:
   facecompute(map, 2, 1, 0);
 }
 if (render) return(0);

 face.slope=slope;
 face.aspect=aspect;
 face.diplat=diplat;
 face.diplong=diplong;

 return(0);

} /* writeface() */

/***********************************************************************/

static short FacePt_Order(struct FaceData *Data) // used locally only -> static, AF 26.7.2021
{

 if (Data->El[0] < Data->El[1])
  {
  if (Data->El[0] < Data->El[2])
   {
   if (Data->El[1] < Data->El[2])
    return (1);				/* 0 <1 <2 */
   else
    return (2);				/* 0 <2<=1 */
   }
  else
   return (3);				/* 2<=0 <1 */
  }
 else if (Data->El[1] < Data->El[2])
  {
  if (Data->El[0] < Data->El[2])
   return (4);				/* 1<=0 <2 */
  else
   return (5);				/* 1< 2<=0 */
  }
 return (6);

} /* FacePt_Order() */

/***********************************************************************/

static short minmax(void) // used locally only -> static, AF 26.7.2021
{
 short n;

 if (elface[0] < elface[1])
  {
  if (elface[0] < elface[2])
   {
   if (elface[1] < elface[2])
    n = 1;	/* 0 <1 <2 */
   else
    n = 2;				/* 0 <2<=1 */
   }
  else
   n = 3;					/* 2<=0 <1 */
  }
 else if (elface[1] < elface[2])
  {
  if (elface[0] < elface[2]) n = 4;		/* 1<=0 <2 */
  else n = 5;				/* 1< 2<=0 */
  }
 else n = 6;				/* 2<=1<=0 */
 return (n);

} /* minmax() */

/***********************************************************************/

static void Face_Compute(struct elmapheaderV101 *map, struct FaceData *Data,
	struct faces *Face, short low, short mid, short hi) // used locally only -> static, AF 20.7.2021
{
 double loel, midel, hiel, lowlat, midlat, hilat, longdistort, lowlong,
	midlong, hilong, splat, splong, elfactor, strikeaz, angledif,
	apaz, elfactor2, truedip;

 lowlat = Data->Lat[low] * LATSCALE;
 midlat = Data->Lat[mid] * LATSCALE;
 hilat = Data->Lat[hi] * LATSCALE;
 longdistort = LATSCALE * cos(Data->Lat[mid] * PiOver180);
 lowlong = Data->Lon[low] * longdistort;
 midlong = Data->Lon[mid] * longdistort;
 hilong = Data->Lon[hi] * longdistort;
 loel = Data->El[low] * map->elscale;
 midel = Data->El[mid] * map->elscale;
 hiel = Data->El[hi] * map->elscale;
 if (hiel == loel)
  {
  Face->diplat = 0.0;
  Face->diplong = 0.0;
  Face->slope = 0.0;
  Face->aspect = -.08726;
  } /* if no slope */
 else
  {
  if (midel == loel)
   {
   splat = lowlat;
   splong = lowlong;
   }
  else if (midel == hiel)
   {
   splat = hilat;
   splong = hilong;
   }
  else
   {
   elfactor = (midel - loel) / (hiel - loel);
   splat = lowlat + (hilat - lowlat) * elfactor;
   splong = lowlong + (hilong - lowlong) * elfactor;
   }
  strikeaz = findangle(midlong - splong, midlat - splat);
  angledif = findangle(midlong - lowlong, midlat - lowlat) - strikeaz;
  if (angledif == 0.0)
   {
   angledif = strikeaz - findangle(midlong - hilong, midlat - hilat);
   }
  if (angledif < -Pi)
   angledif += TwoPi;
  else if (angledif > Pi)
   angledif -= TwoPi;
  Face->aspect = angledif < 0.0 ? strikeaz - HalfPi: strikeaz + HalfPi;
  if (Face->aspect >= TwoPi)
   Face->aspect -= TwoPi;
  else if (Face->aspect < 0.0)
   Face->aspect +=TwoPi;
  apaz = findangle(hilong - lowlong, hilat - lowlat);
  elfactor  = (hilat - lowlat) * (hilat - lowlat);
  elfactor2 = (hilong - lowlong) * (hilong - lowlong);

  truedip = atan((hiel - loel)
	 / (cos(fabs((double)Face->aspect - apaz)) * sqrt(elfactor + elfactor2)));

  Face->diplat = truedip * sin(Pi - (double)Face->aspect);
  Face->diplong = truedip * sin(HalfPi - (double)Face->aspect);
  Face->slope = fabs(truedip);
  } /* else */

} /* Face_Compute() */

/***********************************************************************/
/* pseudo obsolete - this should be eliminated when writeface is eliminated */
static void facecompute(struct elmapheaderV101 *map, short low, short mid, short hi)
{
 double loel, midel, hiel, lowlat, midlat, hilat, longdistort, lowlong,
	midlong, hilong, splat, splong, elfactor, strikeaz, angledif,
	apaz, elfactor2, truedip;

 lowlat = latface[low] * LATSCALE;
 midlat = latface[mid] * LATSCALE;
 hilat = latface[hi] * LATSCALE;
 longdistort = LATSCALE * cos(latface[mid] * PiOver180);
 lowlong = longface[low] * longdistort;
 midlong = longface[mid] * longdistort;
 hilong = longface[hi] * longdistort;
 loel = elface[low] * map->elscale;
 midel = elface[mid] * map->elscale;
 hiel = elface[hi] * map->elscale;
 if (hiel == loel)
  {
  diplat = 0.0;
  diplong = 0.0;
  slope = 0.0;
  aspect = -.08726;
  }
 else
  {
  if (midel == loel)
   {
   splat = lowlat;
   splong = lowlong;
   }
  else if (midel == hiel)
   {
   splat = hilat;
   splong = hilong;
   }
  else
   {
   elfactor = (midel - loel) / (hiel - loel);
   splat = lowlat + (hilat - lowlat) * elfactor;
   splong = lowlong + (hilong - lowlong) * elfactor;
   }
  strikeaz = findangle(midlong - splong, midlat - splat);
  angledif = findangle(midlong - lowlong, midlat - lowlat) - strikeaz;
  if (angledif == 0.0)
   {
   angledif = strikeaz - findangle(midlong - hilong, midlat - hilat);
   }
  if (angledif < -Pi)
   angledif += TwoPi;
  else if (angledif > Pi)
   angledif -= TwoPi;
  aspect = angledif < 0.0 ? strikeaz - HalfPi: strikeaz + HalfPi;
  if (aspect >= TwoPi) aspect -= TwoPi;
  else if (aspect<0.0) aspect +=TwoPi;
  apaz = findangle(hilong - lowlong, hilat - lowlat);
  elfactor  = (hilat - lowlat) * (hilat - lowlat);
  elfactor2 = (hilong - lowlong) * (hilong - lowlong);

  truedip = atan((hiel - loel)
	 / (cos(fabs(aspect - apaz)) * sqrt(elfactor + elfactor2)));

  diplat = truedip * sin(Pi - aspect);
  diplong = truedip * sin(HalfPi - aspect);
  slope = fabs(truedip);
  } /* else */

} /* facecompute() */


