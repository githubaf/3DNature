/* MapUtil.c (ne gismaputil.c 14 Jan 1994 CXH)
** Some utility functions for mapping in WCS renderer.
** Built/ripped/hacked from gisam.c on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent clever modifications by Gary R. Huber.
*/

//PrintIt = 1;

#include "WCS.h"

STATIC_FCN void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C); // used locally only -> static, AF 23.7.2021
STATIC_FCN void ZeroMatrix3x3(Matx3x3 A); // used locally only -> static, AF 23.7.2021
STATIC_FCN void sortrenderlist(void); // used locally only -> static, AF 24.7.2021
STATIC_FCN double DegMinSecToDegrees2(double Val); // used locally only -> static, AF 24.7.2021
//void ClipAreaDraw(struct Window *win, struct poly4 *Poly, struct clipbounds *cb); // AF, not used 26.July 2021
// void ClipPoly4(struct Window *win, struct poly4 *Poly, struct clipbounds *cb); // AF, not used 26.July 2021
STATIC_FCN void RotationMatrix3D(short m, double theta, Matx3x3 A); // used locally only -> static, AF 24.7.2021


void getscrnpt(struct coords *TP, struct coords *SP)
{
/* double newpta;*/

 convertpt(&DP);
 if (TP)
  {
  TP->x = DP.x;
  TP->y = DP.y;
  TP->z = DP.z;
/*  TP->q = DP.q; This doesn't have anything worthwhile in it at this stage */
  }
 findposvector(&DP, &VP);
 if (SP)
  {
  SP->x += DP.x;
  SP->y += DP.y;
  SP->z += DP.z;
  }

 RotatePoint(&DP, ScrRotMatx);

/*
 newpta = DP.x * cosyrot + DP.z * sinyrot;
 DP.z = - DP.x * sinyrot + DP.z * cosyrot;
 DP.x = newpta;

 newpta = DP.y * cosxrot + DP.z * sinxrot;
 DP.z = - DP.y * sinxrot + DP.z * cosxrot;
 DP.y = newpta;

 newpta = DP.x * coszrot + DP.y * sinzrot;
 DP.y = - DP.x * sinzrot + DP.y * coszrot;
 DP.x = newpta;
*/

} /* getscrnpt() */

/***********************************************************************/

void getscrncoords(float *scrnx, float *scrny, float *scrnq,
	struct coords *TP, struct coords *SP)
{

 getscrnpt(TP, SP);
 *scrnx = CenterX + (DP.x * PROJPLANE / DP.z) / horscale;
 *scrny = CenterY - (DP.y * PROJPLANE / DP.z) / vertscale;
 if (settings.alternateq)
  {
  facelat = (DP.lat * PiUnder180 - settings.altqlat) * LATSCALE;
  facelong = (DP.lon * PiUnder180 - settings.altqlon) * LATSCALE * cosviewlat;
  *scrnq = sqrt(facelat * facelat + facelong * facelong);
  } /* if */
 else if (DP.z > 0.0) *scrnq = DP.q;	/*sqrt(DP.x*DP.x+DP.y*DP.y+DP.z*DP.z)*/
 else *scrnq = -DP.q;

} /* getscrncoords() */

/***********************************************************************/

void getdblscrncoords(double *scrnx, double *scrny, double *scrnq,
	struct coords *TP, struct coords *SP)
{

 getscrnpt(TP, SP);
 *scrnx = CenterX + (DP.x * PROJPLANE / DP.z) / horscale;
 *scrny = CenterY - (DP.y * PROJPLANE / DP.z) / vertscale;
 if (settings.alternateq)
  {
  facelat = (DP.lat * PiUnder180 - settings.altqlat) * LATSCALE;
  facelong = (DP.lon * PiUnder180 - settings.altqlon) * LATSCALE * cosviewlat;
  *scrnq = sqrt(facelat * facelat + facelong * facelong);
  } /* if */
 else if (DP.z > 0.0) *scrnq = DP.q;	/*sqrt(DP.x*DP.x+DP.y*DP.y+DP.z*DP.z)*/
 else *scrnq = -DP.q;

} /* getdblscrncoords() */

/***********************************************************************/

void getviewscrncoords(float *scrnx, float *scrny, float *scrnq)
{
 getscrnpt(NULL, NULL);
 *scrnx = CenterX + (DP.x * PROJPLANE / DP.z) / horscale;
 *scrny = CenterY - (DP.y * PROJPLANE / DP.z) / vertscale;
 if (DP.z > 0.0) *scrnq = DP.q;
 else *scrnq = -DP.q;

} /* getviewscrncoords() */

/***********************************************************************/

void convertpt(struct coords *PT)
{
 long z = 0;
 double radius, zfact;

 while (PT->lon > 180.0)
  {
  PT->lon -= 360.0;
  } /* while */
 if (fabs(PT->lon) > 90.0) z = 1;
 else if (fabs(PT->lon) < 90.0) z = -1;
 else PT->z = 0.0;
 PT->lon *= PiOver180;
 PT->lat *= PiOver180;
 radius = EARTHRAD + PT->alt;
 PT->x = -radius * sin(PT->lon) * cos(PT->lat);
 PT->y = radius * sin(PT->lat);
 if (z)
  {
  zfact = radius * radius - PT->x * PT->x - PT->y * PT->y;
  PT->z = zfact >= 0.0 ? z * sqrt(zfact): 0.0;
/*  PT->z = z * sqrt(radius * radius - PT->x * PT->x - PT->y * PT->y);*/
  } /* if */

} /* convertpt() */

/***********************************************************************/

void findposvector(struct coords *EP, struct coords *SP)
{
/* 
** subtract starting vector from ending vector
**result is position vector for ending vector (a data point)
** with the starting pt at the origin. q contains the magnitude of the vector.
*/
 EP->x -= SP->x;
 EP->y -= SP->y;
 EP->z -= SP->z;
 EP->q = sqrt(EP->x * EP->x + EP->y * EP->y + EP->z * EP->z); 

} /* findposvector() */

/***********************************************************************/

void VectorMagnitude(struct coords *TP)
{

 TP->q = sqrt(TP->x * TP->x + TP->y * TP->y + TP->z * TP->z); 

} /* VectorMagnitude() */

/***********************************************************************/

void SurfaceNormal(struct coords *NP, struct coords *FP, struct coords *LP)
{

 NP->x = FP->y * LP->z - FP->z * LP->y;
 NP->y = FP->z * LP->x - FP->x * LP->z;
 NP->z = FP->x * LP->y - FP->y * LP->x;
 VectorMagnitude(NP);
 UnitVector(NP);

 //AF_DEBUG_f_f_f("x y z",NP->x,NP->y,NP->z);
 //AF_DEBUG_f_f_f("lat lon alt y z",NP->lat,NP->lon,NP->alt);
} /* SurfaceNormal() */

/***********************************************************************/

void UnitVector(struct coords *UV)
{

 if (UV->q >= 0.0)
  {
  UV->x /= UV->q;
  UV->y /= UV->q;
  UV->z /= UV->q;
  UV->q = 1.0;
  } /* if */

} /* UnitVector() */

/***********************************************************************/

short SurfaceVisible(struct coords *SN, struct coords *VP, short Face)
{

 if (! Face)
  return ((short)
	((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z) >= 0.0));
 else
  return ((short)
	((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z) <= 0.0));

} /* SurfaceVisible() */

/***********************************************************************/

double VectorAngle(struct coords *SN, struct coords *VP)
{

 if (SN->q <= 0.0 || VP->q <= 0.0)
  return (1.0);

/* returns the cosine of the angle */

 return ((SN->x * VP->x + SN->y * VP->y + SN->z * VP->z)
	/ (SN->q * VP->q));

} /* VectorAngle() */

/**********************************************************************/

double SignedVectorAngle2D(struct coords *From, struct coords *To, short Axis)
{
double LFrom, LTo, Sign;

/* clockwise angle returns positive */
/* return value is the actual angle in radians */

 switch (Axis)
  {
  case 1:
   {
   LFrom = sqrt(From->y * From->y + From->z * From->z);
   LTo   = sqrt(To->y * To->y + To->z * To->z);
   Sign = (From->y * To->z - From->z * To->y > 0.0) ? 1.0: -1.0;
   return (Sign * ACos_Table((From->y * To->y + From->z * To->z) / (LFrom * LTo)));
   break;
   } /* X */
  case 2:
   {
   LFrom = sqrt(From->x * From->x + From->z * From->z);
   LTo   = sqrt(To->x * To->x + To->z * To->z);
   Sign = (From->z * To->x - From->x * To->z > 0.0) ? 1.0: -1.0;
   return (Sign * ACos_Table((From->x * To->x + From->z * To->z) / (LFrom * LTo)));
   break;
   } /* Y */
  case 3:
   {
   LFrom = sqrt(From->x * From->x + From->y * From->y);
   LTo   = sqrt(To->x * To->x + To->y * To->y);
   Sign = (From->x * To->y - From->y * To->x > 0.0) ? 1.0: -1.0;
   return (Sign * ACos_Table((From->x * To->x + From->y * To->y) / (LFrom * LTo)));
   break;
   } /* Z */
  } /* switch */
  return 0;
} /* SignedVectorAngle2D() */

/**********************************************************************/

void BuildRotationMatrix(double Rx, double Ry, double Rz, Matx3x3 RMatx)
{
Matx3x3 XRot, YRot, ZRot, Temp;

 RotationMatrix3D(1, Rx, XRot);
 RotationMatrix3D(2, Ry, YRot);
 RotationMatrix3D(3, Rz, ZRot);
 Multiply3x3Matrices(XRot, YRot, Temp);
 Multiply3x3Matrices(ZRot, Temp, RMatx);

} /* BuildRotationMatrix() */

/**********************************************************************/

STATIC_FCN void RotationMatrix3D(short m, double theta, Matx3x3 A) // used locally only -> static, AF 24.7.2021
{

 short m1, m2;
 double c, s;

 ZeroMatrix3x3(A);
 A[m - 1][m - 1] = 1.0;
 m1 = (m % 3) + 1;
 m2 = m1 % 3;
 m1 --;
 c = cos(theta);
 s = sin(theta);
/*
 A[m1][m1] = c;
 A[m1][m2] = s;
 A[m2][m2] = c;
 A[m2][m1] = -s;
*/
 A[m1][m1] = -c;
 A[m1][m2] = s;
 A[m2][m2] = -c;
 A[m2][m1] = -s;

} /* RotationMatrix3D() */

/***********************************************************************/

STATIC_FCN void ZeroMatrix3x3(Matx3x3 A) // used locally only -> static, AF 23.7.2021
{
short i, j;

 for (i=0; i<3; i++)
  {
  for (j=0; j<3; j++)
   A[i][j] = 0.0;
  } /* for i=0... */

} /* ZeroMatx3x3() */

/***********************************************************************/

void ZeroCoords(struct coords *A)
{

 A->x = A->y = A->z = A->q = 0.0;

} /* ZeroCoords() */

/***********************************************************************/

void NegateVector(struct coords *A)
{

 A->x = -A->x;
 A->y = -A->y;
 A->z = -A->z;

} /* NegateVector() */

/***********************************************************************/

STATIC_FCN void Multiply3x3Matrices(Matx3x3 A, Matx3x3 B, Matx3x3 C) // used locally only -> static, AF 23.7.2021
{
short i, j, k;
double ab;

 for (i=0; i<3; i++)
  {
  for (j=0; j<3; j++)
   {
   ab = 0.0;
   for (k=0; k<3; k++)
    ab += A[i][k] * B[k][j];
   C[i][j] = ab;
   } /* for j=0... */
  } /* for i=0... */

} /* Multiply3x3Matrices() */

/**********************************************************************/

void RotatePoint(struct coords *A, Matx3x3 M)
{
struct coords B;

 B.x = A->x * M[0][0] + A->y * M[0][1] + A->z * M[0][2];
 B.y = A->x * M[1][0] + A->y * M[1][1] + A->z * M[1][2];
 B.z = A->x * M[2][0] + A->y * M[2][1] + A->z * M[2][2];

 A->x = B.x;
 A->y = B.y;
 A->z = B.z;
 VectorMagnitude(A);

} /* RotatePoint() */

/**********************************************************************/

double findangle(double pta, double ptb)
{
 double angle;

 if (pta == 0.0)
  {
  angle = ptb > 0.0 ? OneAndHalfPi: HalfPi;
  } /* if */
 else
  {
  angle = atan(ptb / pta);
  if (pta > 0.0) angle += Pi;
  else if (ptb > 0.0) angle += TwoPi;
  } /* else */

 return(angle);

} /* findangle() */

/***********************************************************************/

double findangle2(double pta, double ptb)
{
 double angle;

 if (ptb == 0.0)
  {
  angle = pta > 0.0 ? HalfPi: OneAndHalfPi;
  } /* if */
 else
  {
  angle = atan(pta / ptb);
  if (ptb < 0.0) angle += Pi;
  } /* else */

 return angle;

} /* findangle2() */

/***********************************************************************/

double findangle3(double pta, double ptb)
{
 double angle;

 if (pta == 0.0) {
  if (ptb > 0.0) angle = HalfPi;
  else angle = OneAndHalfPi;
 }
 else {
  angle = atan(ptb / pta);
  if (pta < 0.0) angle += Pi;
 }
 return angle;

} /* findangle3() */

/************************************************************************/

void rotate(double *pta, double *ptb, double rotangle, double angle)
{
 double length, newangle;

 newangle = angle - rotangle;
 length = sqrt((*pta) * (*pta) + (*ptb) * (*ptb));
 *pta = length * sin(newangle);
 *ptb = length * cos(newangle);

} /* rotate() */

/************************************************************************/

void rotate2(double *pta, double *ptb, double rotangle, double angle)
{
 double length,newangle;

 newangle = angle - rotangle;
 length = sqrt((*pta) * (*pta) + (*ptb) * (*ptb));
 *pta = length * cos(newangle);
 *ptb = length * sin(newangle);

} /* rotate2() */

/***********************************************************************/

short autoactivate(void)
{
 short error = 0, i, result;
 struct elmapheaderV101 *OldElMap = NULL;
 struct boundingbox *OldBBox = NULL;
 long OldElmapSize = 0, OldBBoxSize = 0, OldNoOfElMaps = 0;

 if (elmap && ElmapSize > 0 && BBoxSize > 0)
  {
  OldElMap = (struct elmapheaderV101 *)get_Memory(ElmapSize, MEMF_ANY);
  OldBBox = (struct boundingbox *)get_Memory(BBoxSize, MEMF_ANY);
  if (! OldElMap || ! OldBBox)
   {
   error = 1;
   goto Cleanup;
   } /* if no memory */
  memcpy(OldElMap, elmap, ElmapSize);
  memcpy(OldBBox, BBox, BBoxSize);
  OldElmapSize = ElmapSize;
  OldBBoxSize = BBoxSize;
  OldNoOfElMaps = NoOfElMaps;
  } /* if elmap array already in use (by Camera View) */

 result = initinterview(1);
 if (result == 1)
  {
  error = 1;
  RenderObjects = 0;
  goto Cleanup;
  } /* if out of memory */
 else if (result == 2)
  {
  Log(MSG_NULL, (CONST_STRPTR)"Render List: No maps found");
  error = 1;
  RenderObjects = 0;
  goto Cleanup;
  } /* else if no maps found */

 RenderObjects = NoOfElMaps;

 sortrenderlist();

Cleanup:
 if (elmap)
  {
  for (i=0; i<NoOfElMaps; i++)
   {
   if (elmap[i].scrnptrx) free_Memory(elmap[i].scrnptrx, elmap[i].scrnptrsize);
   if (elmap[i].scrnptry) free_Memory(elmap[i].scrnptry, elmap[i].scrnptrsize);
   if (elmap[i].scrnptrq) free_Memory(elmap[i].scrnptrq, elmap[i].scrnptrsize);
   } /* for i=0... */
  free_Memory(elmap, ElmapSize);
  } /* if elmap */
 elmap = OldElMap;
 if (BBox) free_Memory(BBox, BBoxSize);
 BBox = OldBBox;
 NoOfElMaps = OldNoOfElMaps;
 ElmapSize = OldElmapSize;
 BBoxSize = OldBBoxSize;

 Log(DTA_NULL, (CONST_STRPTR)"Render List:");
 for (i=0; i<RenderObjects; i++)
  {
  sprintf(str, "%hd. %s Dir=%s", i, DBase[RenderList[i][0]].Name,
	RenderList[i][1] ? "E-W": "W-E");
  Log(MSG_UTIL_TAB, (CONST_STRPTR)str);
  } /* for i=0... */

 return error;

} /* autoactivate() */

/***********************************************************************/

STATIC_FCN void sortrenderlist(void) // used locally only -> static, AF 24.7.2021
{
 short i, j, more;
 long Qsize;
 float *Qavg;

 Qsize = 4 * RenderObjects;
 if ((Qavg = (float *)get_Memory(Qsize, MEMF_CLEAR)) == NULL)
  {
  Log(ERR_MEM_FAIL, (CONST_STRPTR)"No render list!");
  return;
  } /* if */
 for (i=0; i<RenderObjects; i++)
  {
  for (j=4; j<8; j++)
   {
   Qavg[i] += fabs(BBox[i].scrnq[j]);
   } /* for j=4... */
  } /* for i=0... */

 do
  {
   more = 0;
   for (i=0; i<RenderObjects-1; i++)
    {
    if (Qavg[i] > Qavg[i+1])
     {
     swmem(&Qavg[i], &Qavg[i+1], 4);
     swmem(&RenderList[i], &RenderList[i+1], 4);
     more ++;
     } /* if wrong order */
    } /* for i=0... */
   } while (more); /* while not fully sorted */

 free_Memory(Qavg, Qsize);

} /* sortrenderlist() */

/***********************************************************************/

void SortAltRenderList(void)
{
 short i, j, more;
 long Qsize;
 float *Qavg;

 Qsize = 4 * NoOfElMaps;
 if ((Qavg = (float *)get_Memory(Qsize, MEMF_CLEAR)) == NULL)
  {
  Log(ERR_MEM_FAIL, (CONST_STRPTR)"No render list!");
  return;
  } /* if get memory failed */

 for (i=0; i<NoOfElMaps; i++)
  {
  for (j=4; j<8; j++)
   {
   Qavg[i] += fabs(BBox[i].scrnq[j]);
   } /* for j=4... */
  } /* for i=0... */

 do
  {
   more = 0;
   for (i=0; i<NoOfElMaps-1; i++)
    {
    if (Qavg[i] < Qavg[i+1])
     {
     swmem(&Qavg[i], &Qavg[i+1], 4);
     swmem((AltRenderList + (i * 2)), (AltRenderList + (i + 1) * 2), 4);
     swmem(&BBox[i], &BBox[i + 1], sizeof (struct boundingbox));
     swmem(&elmap[i], &elmap[i + 1], sizeof (struct elmapheaderV101));
     more ++;
     } /* if wrong order */
    } /* for i=0... */
  } while (more); /* while not fully sorted */

 free_Memory(Qavg, Qsize);

} /* SortAltRenderList() */

/***********************************************************************/
#ifdef HGJGJHGJGJHGSD
// No longer needed
void reversemap(struct elmapheaderV101 *map)
{
 short *lowptr, *highptr;
 short i;

 lowptr = highptr = map->map;
 highptr += map->rows * map->columns;
 for (i=0; i<(map->rows+1)/2; i++)
  {
  swmem(lowptr, highptr, map->columns * 2);
  lowptr += map->columns;
  highptr -= map->columns;
  } /* for i=0... */
 map->lolong -= map->rows * map->steplong;
 map->steplong *= (-1);

} /* reversemap() */

/***********************************************************************/

void reverseimage(UBYTE *image, long rows, long columns)
{
 UBYTE *lowptr, *highptr;
 short i;

 lowptr = highptr = image;
 highptr += rows * columns;
 for (i=0; i<(rows+1)/2; i++)
  {
  swmem(lowptr, highptr, columns);
  lowptr += columns;
  highptr -= columns;
  } /* for i=0... */

} /* reverseimage */
#endif
/***********************************************************************/

void setclipbounds(struct Window *win, struct clipbounds *cb)
{
 cb->lowx = win->BorderLeft;
 cb->highx = win->Width - win->BorderRight - 1;
 cb->lowy = win->BorderTop;
 cb->highy = win->Height - win->BorderBottom - 1;

} /* setclipbounds() */

/***********************************************************************/

void setlineseg(struct lineseg *ls, double firstx, double firsty,
	double lastx, double lasty)
{
 ls->oldx = ls->lastx;
 ls->oldy = ls->lasty;
 ls->firstx = firstx;
 ls->firsty = firsty;
 ls->lastx = lastx;
 ls->lasty = lasty;

} /* setlineseg() */

/***********************************************************************/

void ClipDraw(struct Window *win, struct clipbounds *cb, struct lineseg *ls)
{

 ClipDrawRPort(win->RPort, cb, ls);

} /* ClipDraw() */

/***********************************************************************/

void ClipDrawRPort(struct RastPort *Rast, struct clipbounds *cb, struct lineseg *ls)
{
 short outxlow, outxhigh, outylow, outyhigh, firstin, lastin;
 double m;

 outxlow = outxhigh = outylow = outyhigh = 0;
 firstin = lastin = 1;

/*
** Determine which pts are in draw region. Reject segment if both are on one
**   side and outside of draw region.
*/

 if (ls->firstx < cb->lowx)
  {
  firstin = 0;
  outxlow ++;
  }
 else if (ls->firstx > cb->highx)
  {
  firstin = 0;
  outxhigh ++;
  }
 if (ls->lastx < cb->lowx)
  {
  lastin = 0;
  outxlow ++;
  }
 else if (ls->lastx > cb->highx)
  {
  lastin = 0;
  outxhigh ++;
  }
 if (outxlow == 2 || outxhigh == 2) return;

 if (ls->firsty < cb->lowy)
  {
  firstin = 0;
  outylow ++;
  }
 else if (ls->firsty > cb->highy)
  {
  firstin = 0;
  outyhigh ++;
  }
 if (ls->lasty < cb->lowy)
  {
  lastin = 0;
  outylow ++;
  }
 else if (ls->lasty > cb->highy)
  {
  lastin = 0;
  outyhigh ++;
  }
 if (outylow == 2 || outyhigh == 2) return;

/*
** If both pts are in the draw region draw the segment. Otherwise
**   compute bounds intercept(s).
*/

 if (firstin && lastin)
  {
  if (ls->firstx != ls->oldx || ls->firsty != ls->oldy)
   Move(Rast, (long)ls->firstx, (long)ls->firsty);
  Draw(Rast, (long)ls->lastx, (long)ls->lasty);
  return;
  } /* if both pts within draw region */

 if (lastin) swmem(&ls->firstx, &ls->lastx, 16);

 m = (ls->lasty - ls->firsty) / (ls->lastx - ls->firstx);

 if (firstin || lastin)
  {
  if ((outxhigh + outxlow) && !(outyhigh + outylow))
   {
   if (outxhigh) ls->lastx = cb->highx;
   else ls->lastx = cb->lowx;
   ls->lasty = ls->firsty + m * (ls->lastx - ls->firstx);
   } /* if last pt x value is outside bounds */
  else if ((outyhigh + outylow) && !(outxhigh + outxlow))
   {
   if (outyhigh) ls->lasty = cb->highy;
   else ls->lasty = cb->lowy;
   ls->lastx = ls->firstx + (ls->lasty - ls->firsty) / m;
   } /* if last pt y value is outside bounds */
  else
   {
   if (outxhigh) ls->lastx = cb->highx;
   else ls->lastx = cb->lowx;
   ls->lasty = ls->firsty + m * (ls->lastx - ls->firstx);
   if (ls->lasty < cb->lowy || ls->lasty > cb->highy)
    {
    if (outyhigh) ls->lasty = cb->highy;
    else ls->lasty = cb->lowy;
    ls->lastx = ls->firstx + (ls->lasty - ls->firsty) / m;
    }
   } /* if last pt x and y values are outside bounds */
  } /* if first pt only is within draw region */

 else
  {
  short found = 0;
  double icptx[4], icpty[4];

  icptx[0] = cb->lowx;
  icpty[0] = ls->firsty + m * (icptx[0] - ls->firstx);
  icptx[1] = cb->highx;
  icpty[1] = ls->firsty + m * (icptx[1] - ls->firstx);

  if (icpty[0] >= cb->lowy && icpty[0] <= cb->highy) found +=1;
  if (icpty[1] >= cb->lowy && icpty[1] <= cb->highy) found +=2;

  if (found < 3)
   {
   icpty[2] = cb->lowy;
   icptx[2] = ls->firstx + (icpty[2] - ls->firsty) / m;
   icpty[3] = cb->highy;
   icptx[3] = ls->firstx + (icpty[3] - ls->firsty) / m;

   if (icptx[2] >= cb->lowx && icptx[2] <= cb->highx) found +=4;
   if (icptx[3] >= cb->lowx && icptx[3] <= cb->highx) found +=8;
   } /* if one or both of the y intercepts was out of bounds */

  switch (found)
   {
   case 3:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[1];
    ls->lasty = icpty[1];
    break;
   case 5:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[2];
    ls->lasty = icpty[2];
    break;
   case 6:
    ls->firstx = icptx[1];
    ls->firsty = icpty[1];
    ls->lastx = icptx[2];
    ls->lasty = icpty[2];
    break;
   case 9:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   case 10:
    ls->firstx = icptx[1];
    ls->firsty = icpty[1];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   case 12:
    ls->firstx = icptx[2];
    ls->firsty = icpty[2];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   default:
    return;
   } /* switch */
  } /* else both pts outside draw region */

/*
** Draw the segment.
*/

 Move(Rast, (long)ls->firstx, (long)ls->firsty);
 Draw(Rast, (long)ls->lastx, (long)ls->lasty);

}/* ClipDraw() */

/***********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void ClipAreaDraw(struct Window *win, struct poly4 *Poly, struct clipbounds *cb)
{
 short i = 0, ct = 0;

 for (; i<4; i++)
  {
  if (Poly->X[i] >= cb->lowx && Poly->X[i] <=cb->highx)
   {
   if (Poly->Y[i] >= cb->lowy && Poly->Y[i] <= cb->highy)
    {
    ct ++;
    } /* if y within bounds */
   } /* if x within bounds */
  } /* for i<4... */

 if (ct)
  {
  if (ct == 4)
   {
   AreaMove(win->RPort, (long)Poly->X[0], (long)Poly->Y[0]);
   for (i=1; i<4; i++)
    {
    AreaDraw(win->RPort, (long)Poly->X[i], (long)Poly->Y[i]);
    } /* for i=0... */
   AreaEnd(win->RPort);
   } /* if all 4 points lie within clip region */

  else
   {
   ClipPoly4(win, Poly, cb);
   } /* else at least one point within clip region */

  } /* if at least one point within clip bounds */

} /* ClipAreaDraw() */
#endif
/***********************************************************************/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void ClipPoly4(struct Window *win, struct poly4 *Poly, struct clipbounds *cb)
{
 short i = 0, PolyFirst = 1;
 struct lineseg ls;

 Poly->X[4] = Poly->X[0];
 Poly->Y[4] = Poly->Y[0];

 for (; i<4; i++)
  {
  setlineseg(&ls, (double)Poly->X[i], (double)Poly->Y[i],
	 (double)Poly->X[i + 1], (double)Poly->Y[i + 1]);
  if (ClipPolySeg(cb, &ls))
   {
   if (PolyFirst)
    {
    AreaMove(win->RPort, (long)ls.firstx, (long)ls.firsty);
    AreaDraw(win->RPort, (long)ls.lastx, (long)ls.lasty);
    PolyFirst = 0;
    } /* if first segment found within bounds */
   else
    {
    AreaDraw(win->RPort, (long)ls.firstx, (long)ls.firsty);
    AreaDraw(win->RPort, (long)ls.lastx, (long)ls.lasty);
    } /* else */
   } /* if ClipLineSeg() found a segment within bounds */
  } /* for i=0... */

 if (! PolyFirst)
  {
  AreaEnd(win->RPort);
  } /* if at least one segment */

} /* ClipPoly4() */
#endif
/***********************************************************************/

short ClipPolySeg(struct clipbounds *cb, struct lineseg *ls)
{
 short outxlow, outxhigh, outylow, outyhigh, firstin, lastin, swap = 0;
 double m;

 outxlow = outxhigh = outylow = outyhigh = 0;
 firstin = lastin = 1;

/*
** Determine which pts are in draw region. Reject segment if both are on one
**   side and outside of draw region.
*/

 if (ls->firstx < cb->lowx)
  {
  firstin = 0;
  outxlow ++;
  }
 else if (ls->firstx > cb->highx)
  {
  firstin = 0;
  outxhigh ++;
  }
 if (ls->lastx < cb->lowx)
  {
  lastin = 0;
  outxlow ++;
  }
 else if (ls->lastx > cb->highx)
  {
  lastin = 0;
  outxhigh ++;
  }
 if (outxlow == 2 || outxhigh == 2) return (0);

 if (ls->firsty < cb->lowy)
  {
  firstin = 0;
  outylow ++;
  }
 else if (ls->firsty > cb->highy)
  {
  firstin = 0;
  outyhigh ++;
  }
 if (ls->lasty < cb->lowy)
  {
  lastin = 0;
  outylow ++;
  }
 else if (ls->lasty > cb->highy)
  {
  lastin = 0;
  outyhigh ++;
  }
 if (outylow == 2 || outyhigh == 2) return (0);

/*
** If both pts are in the draw region draw the segment. Otherwise
**   compute bounds intercept(s).
*/

 if (firstin && lastin)
  {
  return (1);
  } /* if both pts within draw region */

 if (lastin)
  {
  swmem(&ls->firstx, &ls->lastx, 16);
  swap = 1;
  } /* keep track of position swapping to unswap later */

 m = (ls->lasty - ls->firsty) / (ls->lastx - ls->firstx);

 if (firstin || lastin)
  {
  if ((outxhigh + outxlow) && !(outyhigh + outylow))
   {
   if (outxhigh) ls->lastx = cb->highx;
   else ls->lastx = cb->lowx;
   ls->lasty = ls->firsty + m * (ls->lastx - ls->firstx);
   } /* if last pt x value is outside bounds */
  else if ((outyhigh + outylow) && !(outxhigh + outxlow))
   {
   if (outyhigh) ls->lasty = cb->highy;
   else ls->lasty = cb->lowy;
   ls->lastx = ls->firstx + (ls->lasty - ls->firsty) / m;
   } /* if last pt y value is outside bounds */
  else
   {
   if (outxhigh) ls->lastx = cb->highx;
   else ls->lastx = cb->lowx;
   ls->lasty = ls->firsty + m * (ls->lastx - ls->firstx);
   if (ls->lasty < cb->lowy || ls->lasty > cb->highy)
    {
    if (outyhigh) ls->lasty = cb->highy;
    else ls->lasty = cb->lowy;
    ls->lastx = ls->firstx + (ls->lasty - ls->firsty) / m;
    }
   } /* if last pt x and y values are outside bounds */
  } /* if first pt only is within draw region */

 else
  {
  short found = 0;
  double icptx[4], icpty[4];

  icptx[0] = cb->lowx;
  icpty[0] = ls->firsty + m * (icptx[0] - ls->firstx);
  icptx[1] = cb->highx;
  icpty[1] = ls->firsty + m * (icptx[1] - ls->firstx);

  if (icpty[0] >= cb->lowy && icpty[0] <= cb->highy) found +=1;
  if (icpty[1] >= cb->lowy && icpty[1] <= cb->highy) found +=2;

  if (found < 3)
   {
   icpty[2] = cb->lowy;
   icptx[2] = ls->firstx + (icpty[2] - ls->firsty) / m;
   icpty[3] = cb->highy;
   icptx[3] = ls->firstx + (icpty[3] - ls->firsty) / m;

   if (icptx[2] >= cb->lowx && icptx[2] <= cb->highx) found +=4;
   if (icptx[3] >= cb->lowx && icptx[3] <= cb->highx) found +=8;
   } /* if one or both of the y intercepts was out of bounds */

  switch (found)
   {
   case 3:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[1];
    ls->lasty = icpty[1];
    break;
   case 5:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[2];
    ls->lasty = icpty[2];
    break;
   case 6:
    ls->firstx = icptx[1];
    ls->firsty = icpty[1];
    ls->lastx = icptx[2];
    ls->lasty = icpty[2];
    break;
   case 9:
    ls->firstx = icptx[0];
    ls->firsty = icpty[0];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   case 10:
    ls->firstx = icptx[1];
    ls->firsty = icpty[1];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   case 12:
    ls->firstx = icptx[2];
    ls->firsty = icpty[2];
    ls->lastx = icptx[3];
    ls->lasty = icpty[3];
    break;
   default:
    return (0);
   } /* switch */
  } /* else both pts outside draw region */

 if (swap) 
  {
  swmem(&ls->firstx, &ls->lastx, 16);
  } /* swap back so points are in correct drawing order */

 return (1);

} /* ClipPolySeg() */

/************************************************************************/

void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone)
{

  Coords->a 	= 6378206.4;
  Coords->e_sq 	= 0.00676866;
  Coords->k_0 	= 0.9996;
  Coords->M_0 	= 0.0;
  Coords->lam_0	= (double)(183 - UTMZone * 6);

  Coords->e_pr_sq 	= Coords->e_sq / (1.0 - Coords->e_sq);
  Coords->e_sq_sq 	= Coords->e_sq * Coords->e_sq;
  Coords->e_1 		= (1.0 - sqrt(1.0 - Coords->e_sq)) / (1.0 + sqrt(1.0 - Coords->e_sq));
  Coords->e_1_sq 	= Coords->e_1 * Coords->e_1;

} /* UTMLatLonCoords_Init() */

/************************************************************************/

void UTM_LatLon(struct UTMLatLonCoords *Coords)
{
double x, y, M, mu, phi_1, C_1, T_1, N_1, R_1, D_1,
	cos_phi_1, tan_phi_1, sin_phi_1, sin_sq_phi_1, C_1_sq, T_1_sq, D_1_sq;


 x 		= Coords->East;
 y 		= Coords->North;
 x 		-= 500000.0;
 M 		= Coords->M_0 + y / Coords->k_0;
 mu = M / (Coords->a * (1.0 - Coords->e_sq / 4.0 - 3.0 * Coords->e_sq_sq
	/ 64 - 5.0 * Coords->e_sq * Coords->e_sq_sq / 256.0));
 phi_1 = (mu + (3.0 * Coords->e_1 / 2.0 - 27.0 * Coords->e_1 * Coords->e_1_sq / 32.0)
	* sin(2.0 * mu)
	+ (21.0 * Coords->e_1_sq / 16.0 - 55.0 * Coords->e_1_sq * Coords->e_1_sq / 32.0)
	* sin(4.0 * mu)
	+ (151.0 * Coords->e_1 * Coords->e_1_sq / 96.0) * sin(6.0 * mu)); /* radians */
 cos_phi_1 	= cos(phi_1);
 tan_phi_1 	= tan(phi_1);
 sin_phi_1 	= sin(phi_1);
 sin_sq_phi_1	= sin_phi_1 * sin_phi_1;
 phi_1 		*= PiUnder180;				/* degrees */
 C_1 		= Coords->e_pr_sq * cos_phi_1 * cos_phi_1;
 C_1_sq 	= C_1 * C_1;
 T_1 		= tan_phi_1 * tan_phi_1;
 T_1_sq 	= T_1 * T_1;
 N_1 		= Coords->a / sqrt(1.0 - Coords->e_sq * sin_sq_phi_1);
 R_1 		= Coords->a * (1.0 - Coords->e_sq) / pow((1.0 - Coords->e_sq * sin_sq_phi_1), 1.5);
 D_1		= x / (N_1 * Coords->k_0);
 D_1_sq 	= D_1 * D_1;

 Coords->Lat = phi_1 - (N_1 * tan_phi_1 / R_1) *
	(D_1_sq / 2.0 
	- (5.0 + 3.0 * T_1 + 10.0 * C_1 - 4.0 * C_1_sq - 9.0 * Coords->e_pr_sq)
	* D_1_sq * D_1_sq / 24.0
	+ (61.0 + 90.0 * T_1 + 298.0 * C_1 + 45.0 * T_1_sq - 252.0 * Coords->e_pr_sq
		- 3.0 * C_1_sq)
	* D_1_sq * D_1_sq * D_1_sq / 720.0)
	* PiUnder180;		/* degrees */

/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */

 Coords->Lon = Coords->lam_0 - ((D_1 - (1.0 + 2.0 * T_1 + C_1) * D_1 * D_1_sq / 6.0
	+ (5.0 - 2.0 * C_1 + 28.0 * T_1 - 3.0 * C_1_sq + 8.0 * Coords->e_pr_sq
		+ 24.0 * T_1_sq)
	* D_1 * D_1_sq * D_1_sq / 120.0) / cos_phi_1) * PiUnder180; /* degrees */

} /* UTM_LatLon() */

/***********************************************************************/

void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone)
{
 double a, e_sq, k_0, lam_0, M_0, x, y, e_pr_sq, M,
	C, T, N, phi, lam, phi_rad,
	cos_phi, sin_phi, tan_phi, A, A_sq, A_cu, A_fo, A_fi, A_si, T_sq;

 a 	= 6378206.4;
 e_sq 	= 0.00676866;
 k_0 	= 0.9996;
 M_0 	= 0.0;
 phi 	= Coords->Lat;
 lam 	= -Coords->Lon;	/* change sign to conform with GIS standard */
 lam_0 	= -(double)(183 - UTMZone * 6);
 phi_rad = phi * PiOver180;

 e_pr_sq = e_sq / (1.0 - e_sq);
 sin_phi = sin(phi_rad);

/* use a very large number for tan_phi if tan(phi) is undefined */

 tan_phi = fabs(phi) == 90.0 ? (phi >= 0.0 ? 1.0: -1.0) * FLT_MAX / 100.0:
	 tan(phi_rad);
 cos_phi = cos(phi_rad);

 N	= a / sqrt(1.0 - e_sq * sin_phi * sin_phi);
 T	= tan_phi * tan_phi;
 T_sq	= T * T;
 C	= e_pr_sq * cos_phi * cos_phi;
 A	= cos_phi * (lam - lam_0) * PiOver180;
 A_sq	= A * A;
 A_cu	= A * A_sq;
 A_fo	= A * A_cu;
 A_fi	= A * A_fo;
 A_si	= A * A_fi;
 M	= 111132.0894 * phi - 16216.94 * sin (2.0 * phi_rad)
	 + 17.21 * sin(4.0 * phi_rad) - .02 * sin(6.0 * phi_rad);
 x	= k_0 * N * (A + ((1.0 - T + C) * A_cu / 6.0)
	  + ((5.0 - 18.0 * T + T_sq + 72.0 * C - 58.0 * e_pr_sq) * A_fi / 120.0)
	  ) + 500000;
 y	= k_0 * (M - M_0 + N * tan_phi *
	   (A_sq / 2.0 + ((5.0 - T + 9.0 * C + 4.0 * C * C) * A_fo / 24.0)
	   + ((61.0 - 58.0 * T + T_sq + 600.0 * C - 330.0 * e_pr_sq) * A_si / 720.0)
	  ));


/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */


 Coords->North = y;
 Coords->East  = x;

} /* UTM_LatLon() */

/************************************************************************/

void AlbLatLonCoords_Init(struct AlbLatLonCoords *Coords)
{
double phi_0, phi_1, phi_2, m_1, m_2, q_1, q_2, q_0,
	sin_phi_0, sin_phi_1, sin_phi_2, sin_sq_phi_0, sin_sq_phi_1, sin_sq_phi_2;

  Coords->a 	= Coords->ProjPar[0];
  Coords->e_sq 	= Coords->ProjPar[1];
  Coords->e 	= sqrt(Coords->e_sq);
  Coords->two_e	= 2.0 * Coords->e;
  Coords->lam_0	= DegMinSecToDegrees2(Coords->ProjPar[4]);
  phi_0		= DegMinSecToDegrees2(Coords->ProjPar[5]) * PiOver180;
  phi_1		= DegMinSecToDegrees2(Coords->ProjPar[2]) * PiOver180;
  phi_2		= DegMinSecToDegrees2(Coords->ProjPar[3]) * PiOver180;

  sin_phi_0 = sin(phi_0);  
  sin_phi_1 = sin(phi_1);  
  sin_phi_2 = sin(phi_2);  
  sin_sq_phi_0 = sin_phi_0 * sin_phi_0;
  sin_sq_phi_1 = sin_phi_1 * sin_phi_1;
  sin_sq_phi_2 = sin_phi_2 * sin_phi_2;

  m_1 = cos(phi_1) / sqrt(1.0 - Coords->e_sq * sin_sq_phi_1);
  m_2 = cos(phi_2) / sqrt(1.0 - Coords->e_sq * sin_sq_phi_2);

  q_0 = (1.0 - Coords->e_sq) * (sin_phi_0 / (1.0 - Coords->e_sq * sin_sq_phi_0)
	- (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_0)
	/ (1.0 + Coords->e * sin_phi_0)));
  q_1 = (1.0 - Coords->e_sq) * (sin_phi_1 / (1.0 - Coords->e_sq * sin_sq_phi_1)
	- (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_1)
	/ (1.0 + Coords->e * sin_phi_1)));
  q_2 = (1.0 - Coords->e_sq) * (sin_phi_2 / (1.0 - Coords->e_sq * sin_sq_phi_2)
	- (1.0 / (2.0 * Coords->e)) * log((1.0 - Coords->e * sin_phi_2)
	/ (1.0 + Coords->e * sin_phi_2)));
  Coords->n 	= (m_1 * m_1 - m_2 * m_2) / (q_2 - q_1);
  Coords->C	= m_1 * m_1 + Coords->n * q_1;
  Coords->rho_0	= Coords->a * sqrt(Coords->C - Coords->n * q_0) / Coords->n;

} /* AlbLatLonCoords_Init() */

/************************************************************************/

void Alb_LatLon(struct AlbLatLonCoords *Coords)
{
short i;
double x, y, phi, theta, rho, q, phi_rad, sin_phi, sin_sq_phi, delta_phi;

 x = Coords->East;
 y = Coords->North;

 if (Coords->n < 0.0)
  theta = atan(-x / (y - Coords->rho_0));
 else
  theta = atan(x / (Coords->rho_0 - y));

 rho = sqrt(x * x + (Coords->rho_0 - y) * (Coords->rho_0 - y));

 q = (Coords->C - (rho * rho * Coords->n * Coords->n)
	/ (Coords->a * Coords->a)) / Coords->n;

 phi_rad = asin(q / 2.0);
 phi = phi_rad * PiUnder180;

 for (i=0, delta_phi=10.0; fabs(delta_phi)>.00001 && i<10; i++)
  {
  sin_phi = sin(phi_rad);
  sin_sq_phi = sin_phi * sin_phi;
  delta_phi = ((1.0 - Coords->e_sq * sin_sq_phi) * (1.0 - Coords->e_sq * sin_sq_phi) 
	/ (2.0 * cos(phi_rad))) 
	* (q / (1.0 - Coords->e_sq) 
	- sin_phi / (1.0 - Coords->e_sq * sin_sq_phi) 
	+ (1.0 / Coords->two_e) 
	* log((1.0 - Coords->e * sin_phi) / (1.0 + Coords->e * sin_phi))) * PiUnder180;
  phi += delta_phi;
  phi_rad = phi * PiOver180;
  }
 if (i >= 10)
  phi = 90.0 * (q >= 0.0 ? 1.0: -1.0);

 Coords->Lat = phi;

/* change sign for WCS convention */
 Coords->Lon = -(Coords->lam_0 + PiUnder180 * theta / Coords->n);


} /* Alb_LatLon() */

/***********************************************************************/

/* For converting a coordinate string of the form dddmmss to decimal degrees */

STATIC_FCN double DegMinSecToDegrees2(double Val) // used locally only -> static, AF 24.7.2021
{
long Deg, Min, DegMinSec;

 DegMinSec = Val;

 Deg = DegMinSec / 1000000;
 DegMinSec %= 1000000;
 Min = DegMinSec / 1000;
 DegMinSec %= 1000;

 return ((double)Deg + (double)Min / 60.0 + (double)DegMinSec / 3600.0);
 
} /* DegMinSecToDegrees2() */

/************************************************************************/

double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, short *Data, long Rows, long Cols)
{
 long Row, Col, Col_p, Row_p, TrueRow, TrueCol;
 double RemX = 0.0, RemY = 0.0, P1, P2;

 Row = TrueRow = ((Y - MinY) / IntY);
 Col = TrueCol = ((X - MinX) / IntX);
 if (Row < 0)
  Row = 0;
 else if (Row >= Rows)
  Row = Rows - 1;
 if (Col < 0)
  Col = 0;
 if (Col >= Cols)
  Col = Cols - 1;

 Row_p = Row;
 Col_p = Col;

 if (Row < Rows - 1 && TrueRow >= 0)
  {
  RemY = (Y - (Row * IntY + MinY)) / IntY;
  if (RemY < 0.0)
   RemY = 0.0;
  Row_p ++;
  } /* if not last row */
 if (Col < Cols - 1 && TrueCol >= 0)
  { 
  RemX = (X - (Col * IntX + MinX)) / IntX;
  if (RemX < 0.0)
   RemX = 0.0;
  Col_p ++;
  } /* if not last column */

 P1 = RemX * (Data[Col_p * Rows + Row] - Data[Col * Rows + Row])
	 + Data[Col * Rows + Row];
 P2 = RemX * (Data[Col_p * Rows + Row_p] - Data[Col * Rows + Row_p])
	 + Data[Col * Rows + Row_p];

 return ( RemY * (P2 - P1) + P1 );

} /* Point_Extract() */

