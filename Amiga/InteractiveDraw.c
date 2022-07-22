/* InteractiveDraw.c (ne gisinteractivedraw.c 14 Jan 1994 CXH)
** Drawing and computing utility routines for GISinteractiveview.c
** Written by Gary R. Huber and Chris "Xenon" Hanson, 8/93.
*/

#include "WCS.h"
#include "GUIDefines.h"
#include <time.h>
#include "vgl/vgl.h"
#include "vgl/vgl_internals.h"


STATIC_FCN short computeview(struct elmapheaderV101 *Map); // used locally only -> static, AF 19.7.2021
STATIC_FCN void computefocprof(void); // used locally only -> static, AF 19.7.2021

/* HIDDEN_LINE_REMOVE does what it sez. */
#define HIDDEN_LINE_REMOVE

/* VGL_SPLITPOLY handles non-coplanar polygons better. */
#define VGL_SPLITPOLY
/* VGL_DRAWLATE forces that edges to be drawn _after_ the fill */
#define VGL_DRAWLATE
/* VGL_SHOW_MIDLINE _forces_ the drawing of the middle edge of the two 3-vert polys */
#undef VGL_SHOW_MIDLINE

#undef USE_MORE_UPDATES

STATIC_FCN void ClipAreaDrawVGL(PIXMAP *PM, struct vgl_coord *Poly, struct clipbounds *cb,
 char Fill, char Border); // used locally only -> static, AF 19.7.2021
void vgl_dumb_fillpoly_convex (PIXMAP * p, int n_vert, struct vgl_coord *vert, double DitherCol);

STATIC_FCN void ClipPoly4RPort(struct RastPort *RP, struct vgl_coord *Poly, struct clipbounds *cb); // used locally only -> static, AF 19.7.2021
STATIC_FCN void ClipAreaDrawRPort(struct RastPort *RP, struct vgl_coord *Poly, struct clipbounds *cb); // used locally only -> static, AF 19.7.2021
STATIC_FCN short drawinterview(void); // used locally only -> static, AF 19.7.2021

struct BusyWindow *BW;

STATIC_VAR PIXMAP *Pixie;
STATIC_VAR struct RastPort *Itchy;
char *Mask;
//char BinarySerialPlaceHolder[] = "C0DEF00F";   // unused, AF

void drawgridview(void)
{
float fDx, fDy, fDq;

  SetPointer(InterWind0, WaitPointer, 16, 16, -6, 0);
  if (EMIA_Win) SetPointer(EMIA_Win->Win, WaitPointer, 16, 16, -6, 0);
  if (EM_Win) SetPointer(EM_Win->Win, WaitPointer, 16, 16, -6, 0);
  BW = BusyWin_New("Drawing...", NoOfElMaps * 2, 1, MakeID('B','W','I','D'));
  constructview();
  if (! computeview(NULL))
   {
   SetAPen(InterWind0->RPort, 1);
   RectFill(InterWind0->RPort,
	  IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
   goto EndDraw;
   } /* if drawing aborted */
  Itchy = NULL;
  if((Pixie = vgl_makepixmap(InterWind0->Width, InterWind0->Height)))
    {
    vgl_dumb_setcur(Pixie, 3, 1);
    vgl_dumb_clear(Pixie);
    if((Itchy = ScratchRast_New(InterWind0->Width, InterWind0->Height, 4)))
    	{
     	drawinterview();
		ScratchRast_CornerTurn(Pixie, Itchy);
		WaitBOVP(&WCSScrn->ViewPort); /* BeamSync */
/*		BltMaskBitMapRastPort(Itchy->BitMap, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->RPort, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->Width - (InterWind0->BorderLeft +
		 InterWind0->BorderRight), InterWind0->Height -
		 (InterWind0->BorderTop + InterWind0->BorderBottom), 0xC0, Mask);*/
		BltBitMapRastPort(Itchy->BitMap, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->RPort, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->Width - (InterWind0->BorderLeft +
		 InterWind0->BorderRight), InterWind0->Height -
		 (InterWind0->BorderTop + InterWind0->BorderBottom), 0xC0);
		WaitBlit();
      ScratchRast_Del(Itchy);
      } /* if */
    vgl_freepixmap(Pixie);
    } /* if */
  if(!Itchy)
    {
    SetAPen(InterWind0->RPort, 1);
    RectFill(InterWind0->RPort,
	  IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
    drawinterview();
    } /* if */

 if (settings.sun)
  {
  SetAPen(InterWind0->RPort, 7);
  DP.alt = 149.0E+6;
  DP.lat = PARC_RNDR_MOTION(26);
  DP.lon = PARC_RNDR_MOTION(27);
  getviewscrncoords(&fDx, &fDy, &fDq);
  IA->SunX = fDx;
  IA->SunY = fDy;
  IA->SunRad = PARC_RNDR_MOTION(28) / IA->wscale;
  DrawHazeRPort(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad, &IA->cb);
/*  DrawCircle(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad);*/
  } /* if */

 if (settings.moon)
  {
  SetAPen(InterWind0->RPort, 2);
  DP.alt = 38.0E+4;
  DP.lat = PARC_RNDR_MOTION(29);
  DP.lon = PARC_RNDR_MOTION(30);
  getviewscrncoords(&fDx, &fDy, &fDq);
  IA->MoonX = fDx;
  IA->MoonY = fDy;
  IA->MoonRad = PARC_RNDR_MOTION(31) / IA->wscale;
  DrawHazeRPort(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad, &IA->cb);
/*  DrawCircle(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad);*/
  } /* if */


EndDraw:
  BusyWin_Del(BW);
  BW = NULL;
  ClearPointer(InterWind0);
  if (EMIA_Win) ClearPointer(EMIA_Win->Win);
  if (EM_Win) ClearPointer(EM_Win->Win);

} /* drawgridview() */

/**********************************************************************/

short drawgridpts(short erase)
{
 float *scrnx, *scrny, *scrnq;
 long zip, Lr, Lc;
 float lastq;
 struct lineseg ls;

 if (erase)
  SetAPen(InterWind0->RPort, 1);
 else
  {
  SetAPen(InterWind0->RPort, 5);
  if (! computeview(FocProf->map))
   return (1);
  } /* else */

 zip = IA_GBDens * FocProf->map->columns;

 scrnx = (float *)FocProf->map->scrnptrx;
 scrny = (float *)FocProf->map->scrnptry;
 scrnq = (float *)FocProf->map->scrnptrq;
 for (Lr=0; Lr<=FocProf->map->rows; Lr+=IA_GBDens)
  {
  lastq = *scrnq;
  for (Lc=IA_GBDens; Lc<FocProf->map->columns; Lc+=IA_GBDens)
   {
   if (scrnq[Lc] > 0.0 && lastq > 0.0)
    {
    setlineseg(&ls, scrnx[Lc - IA_GBDens] + drawoffsetX, scrny[Lc - IA_GBDens] + drawoffsetY,
	scrnx[Lc] + drawoffsetX, scrny[Lc] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
    } /* if point lies in front of viewer */
   lastq = scrnq[Lc];
   }
  scrnx += (zip);
  scrny += (zip);
  scrnq += (zip);
  } /* for Lr=0... */

 return (0);

} /* drawgridpts() */

/**********************************************************************/

STATIC_FCN short computeview(struct elmapheaderV101 *Map) // used locally only -> static, AF 19.7.2021
{
 short i, error = 0, StartMap, EndMap, Lr, Lc;
 ULONG InputID;
 short *mapptr;
 float *scrnx, *scrny, *scrnq;
 double ptelev;
 struct elmapheaderV101 *This;

 if (Map == NULL)
  {
  StartMap = 0;
  EndMap = NoOfElMaps - 1;
  }
 else
  StartMap = EndMap = 0;

 if (IA->recompute || Map)
  {

  for (i=StartMap; i<=EndMap; i++)
   {
#ifdef HIDDEN_LINE_REMOVE
   if (! Map)
    computesinglebox(i);
#endif /* HIDDEN_LINE_REMOVE */

   if (Map)
    {
    This = Map;
    }
   else
    {
    This = &elmap[i];
    }
   mapptr=(short *)This->map;
   scrnx=(float *)This->scrnptrx;
   scrny=(float *)This->scrnptry;
   scrnq=(float *)This->scrnptrq;
   for (Lr=0; Lr<=This->rows; Lr ++)
    {
    for (Lc=0; Lc<This->columns; Lc ++)
     {
     ptelev = *mapptr + (PARC_RNDR_MOTION(13) - *mapptr)
	* PARC_RNDR_MOTION(12);
     DP.alt = ptelev * PARC_RNDR_MOTION(14) * This->elscale;
     DP.lat = This->lolat + Lc * This->steplat;
     DP.lon = This->lolong - Lr * This->steplong;
     getviewscrncoords(scrnx, scrny, scrnq);
     mapptr++;
     scrnx++;
     scrny++;
     scrnq++;
     } /* for Lc=0... */
    InputID = CheckInput_ID();
    if (InputID == ID_BW_CLOSE || InputID == ID_EMTL_PLAY)
     {
     error = 1;
     break;
     } /* if user abort */
    } /* for Lr=0... */

   if (error) break;
   if (BW)
    BusyWin_Update(BW, i + 1);
   } /* for i=0; i<NoOfElMaps */

  if (error) return (0);

#ifdef HIDDEN_LINE_REMOVE
  if (! Map)
   {
   SortAltRenderList();
   findfocprof();
   IA->recompute = 0;
   } /* if all maps drawn */
#endif

  } /* if recompute */

 return (1);

} /* computeview() */

/***********************************************************************/

void makeviewcenter(short erase)
{
 short i, outpts, try = 0;
 double scalefactor = 1.0, SignA, SignB, DifLat, DifLon, DifAlt, LengthScale;
 struct lineseg ls;

 if (erase)
  {
  SetAPen(InterWind0->RPort, 1);
  if (viewctr->q[6] > 0.0 && viewctr->q[1] > 0.0)
   {
   setlineseg(&ls, viewctr->x[1] + drawoffsetX, viewctr->y[1] + drawoffsetY,
	 viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
  if (viewctr->q[6] > 0.0 && viewctr->q[0] > 0.0)
   {
   setlineseg(&ls, viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY,
	 viewctr->x[0] + drawoffsetX, viewctr->y[0] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
  for (i=2; i<6; i+=2)
   {
   if (viewctr->q[i] >0.0 && viewctr->q[i + 1] > 0.0)
    {
    setlineseg(&ls, viewctr->x[i] + drawoffsetX, viewctr->y[i] + drawoffsetY,
	 viewctr->x[i + 1] + drawoffsetX, viewctr->y[i + 1] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
    } /* if */
   } /* for i=0... */
  if (viewctr->q[6] > 0.0 && viewctr->q[7] > 0.0)
   {
   setlineseg(&ls, viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY,
	 viewctr->x[7] + drawoffsetX, viewctr->y[7] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
  } /* if erase */
ReCompute:
 try ++;

 DP.lat = PARC_RNDR_MOTION(4) - .05 / scalefactor;
 DP.lon = PARC_RNDR_MOTION(5);
 DP.alt = PARC_RNDR_MOTION(3);
 getviewscrncoords(&viewctr->x[0], &viewctr->y[0], &viewctr->q[0]);
 DP.lat = PARC_RNDR_MOTION(4) + .05 / scalefactor;
 DP.lon = PARC_RNDR_MOTION(5);
 DP.alt = PARC_RNDR_MOTION(3);
 getviewscrncoords(&viewctr->x[1], &viewctr->y[1], &viewctr->q[1]);
 DP.lat = PARC_RNDR_MOTION(4);
 DP.lon = PARC_RNDR_MOTION(5) - .05 / scalefactor;
 DP.alt = PARC_RNDR_MOTION(3);
 getviewscrncoords(&viewctr->x[2], &viewctr->y[2], &viewctr->q[2]);
 DP.lat = PARC_RNDR_MOTION(4);
 DP.lon = PARC_RNDR_MOTION(5) + .05 / scalefactor;
 DP.alt = PARC_RNDR_MOTION(3);
 getviewscrncoords(&viewctr->x[3], &viewctr->y[3], &viewctr->q[3]);
 DP.lat = PARC_RNDR_MOTION(4);
 DP.lon = PARC_RNDR_MOTION(5);
 DP.alt = (PARC_RNDR_MOTION(3) - 3.45 / scalefactor);
 getviewscrncoords(&viewctr->x[4], &viewctr->y[4], &viewctr->q[4]);
 DP.lat = PARC_RNDR_MOTION(4);
 DP.lon = PARC_RNDR_MOTION(5);
 DP.alt = (PARC_RNDR_MOTION(3) + 3.45 / scalefactor);
 getviewscrncoords(&viewctr->x[5], &viewctr->y[5], &viewctr->q[5]);
 DP.lat = PARC_RNDR_MOTION(4);
 DP.lon = PARC_RNDR_MOTION(5);
 DP.alt = PARC_RNDR_MOTION(3);
 getviewscrncoords(&viewctr->x[6], &viewctr->y[6], &viewctr->q[6]);

 if (try <= 10)
  {
  outpts = 0;
  for (i=0; i<7; i++)
   {
   if (viewctr->q[i] < 0.0)
    outpts ++;
   else if (viewctr->x[i] + drawoffsetX < IA->cb.lowx
	 || viewctr->x[i] + drawoffsetX > IA->cb.highx
	 || viewctr->y[i] + drawoffsetY < IA->cb.lowy
	 || viewctr->y[i] + drawoffsetY > IA->cb.highy)
    outpts ++;
   if (outpts > 1)
    {
    scalefactor *= 2.0;
    break;
    } /* if */
   } /* for i=0... */
  if (outpts > 1)
   goto ReCompute;
  } /* if try <= 10 */

 DifAlt = 3.45 / scalefactor;
/* Sun Lat offset*/
 SignA = 1.0;
 DifLat = PARC_RNDR_MOTION(15) - PARC_RNDR_MOTION(4);
 while (DifLat > 180.0)
  DifLat -= 360.0;
 while (DifLat < -180.0)
  DifLat += 360.0;

 if (fabs(DifLat - 90.0) < .0005)
  DifLat += .001;		/* don't take the tan of 90 */
 if (fabs(DifLat) > 90.0)
  SignA = -1.0;
 DifLat = (DifAlt * tan(DifLat * PiOver180));

/* Sun Lon offset*/
 SignB = 1.0;
 DifLon = PARC_RNDR_MOTION(16) - PARC_RNDR_MOTION(5);
 while (DifLon > 180.0)
  DifLon -= 360.0;
 while (DifLon < -180.0)
  DifLon += 360.0;

 if (fabs(DifLon - 90.0) < .0005 )
  DifLon += .001;		/* don't take the tan of 90 */
 if (fabs(DifLon) > 90.0)
  SignB = -1.0;
 DifLon = (DifAlt * tan(DifLon * PiOver180));

/* scale vector to DifAlt */
 LengthScale = DifAlt / sqrt(DifLat * DifLat + DifLon * DifLon + DifAlt * DifAlt);
 DifLat *= LengthScale;
 DifLon *= LengthScale;
 DifAlt *= LengthScale;
 DP.lat = PARC_RNDR_MOTION(4) + SignA * (DifLat / LATSCALE);
 DP.lon = PARC_RNDR_MOTION(5) + SignB * 
	(DifLon / (cos(PARC_RNDR_MOTION(4) * PiOver180) * LATSCALE));

/* Sun Alt offset*/
 SignA = (SignA < 0.0 || SignB < 0.0) ? -1.0: 1.0;
 DP.alt = PARC_RNDR_MOTION(3) + SignA * DifAlt;
 getviewscrncoords(&viewctr->x[7], &viewctr->y[7], &viewctr->q[7]);

 SetAPen(InterWind0->RPort, 1);
 if (viewctr->q[6] > 0.0 && viewctr->q[1] > 0.0)
  {
  SetAPen(InterWind0->RPort, 3);
  setlineseg(&ls, viewctr->x[1] + drawoffsetX, viewctr->y[1] + drawoffsetY,
	 viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY);
  ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
 SetAPen(InterWind0->RPort, 2);
 if (viewctr->q[6] > 0.0 && viewctr->q[0] > 0.0)
  {
  setlineseg(&ls, viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY,
	 viewctr->x[0] + drawoffsetX, viewctr->y[0] + drawoffsetY);
  ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
 for (i=2; i<6; i+=2)
  {
  if (viewctr->q[i] >0.0 && viewctr->q[i + 1] > 0.0)
   {
   setlineseg(&ls, viewctr->x[i] + drawoffsetX, viewctr->y[i] + drawoffsetY,
	 viewctr->x[i + 1] + drawoffsetX, viewctr->y[i + 1] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
  } /* for i=0... */
 if (viewctr->q[6] > 0.0 && viewctr->q[7] > 0.0)
  {
  SetAPen(InterWind0->RPort, 7);
  setlineseg(&ls, viewctr->x[6] + drawoffsetX, viewctr->y[6] + drawoffsetY,
	 viewctr->x[7] + drawoffsetX, viewctr->y[7] + drawoffsetY);
  ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */

} /* makeviewcenter() */

/***********************************************************************/

STATIC_FCN short drawinterview(void) // used locally only -> static, AF 19.7.2021
{
 short i, error = 0, Lr, Lc, Incr1, Incr2, RowLon, ColLat, ColIncr;
 char Edge;
 short RasWidth, RasHeight;
 struct lineseg ls;
 struct TmpRas TempRast;
 struct AreaInfo AreaInfo = {0};
 USHORT PolyBuffer[(8 + 1) * 5];
 PLANEPTR TempRastPtr;
 long  incrementLr;
 float lastq, *scrnx, *scrny, *scrnq, *rowmarkerx, *rowmarkery,
	*rowmarkerq, hazestart, hazeend;
 double ReducedLon;
 struct vgl_coord PolyVert[5];
 struct RastPort *DrawRast;
 DrawRast = InterWind0->RPort;
 
 if(!Itchy)
   SetAPen(DrawRast, 2);

 hazestart = PARC_RNDR_MOTION(20);
 hazeend = hazestart + PARC_RNDR_MOTION(21);

if (! IA_GridStyle) /* If hidden-line-removed */
 {

 if(!Itchy)
  {
  memset(&PolyBuffer[0], 0, sizeof (PolyBuffer));
  InitArea(&AreaInfo, PolyBuffer, 17);
  RasWidth = InterWind0->Width;
  RasHeight = InterWind0->Height;
  if ((TempRastPtr = AllocRaster(RasWidth, RasHeight)) == NULL)
   {
   User_Message((CONST_STRPTR)"Interactive Motion Module",
           (CONST_STRPTR)"Out of memory!\nHidden line removal not available.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return (1);
   } /* if out of memory initializing TmpRas */
  DrawRast->AreaInfo = &AreaInfo;
  DrawRast->TmpRas = (struct TmpRas *)
 		InitTmpRas(&TempRast, TempRastPtr, RASSIZE(RasWidth, RasHeight));
 /* SetAfPt(DrawRast, PtrnData, 1);*/
  SetAPen(DrawRast, 14);
  SetOPen(DrawRast, 2);
  } /* if */

 for (i=0; i<NoOfElMaps; i++)
  {
  ReducedLon = PARC_RNDR_MOTION(2);
  while (ReducedLon >= 360.0)
   ReducedLon -= 360.0;
  while (ReducedLon < 0.0)
   ReducedLon += 360.0;
  if (fabs(elmap[i].lolong - ReducedLon) < 180.0)
   RowLon = -1.0 + (elmap[i].lolong - ReducedLon) / elmap[i].steplong;
  else
   RowLon = -1.0 + (ReducedLon - elmap[i].lolong) / elmap[i].steplong;
  ColLat = -1.0 + (PARC_RNDR_MOTION(1) - elmap[i].lolat) / elmap[i].steplat;
  ColIncr = max(1, elmap[i].columns - 1 - ColLat); 
  Incr1 = elmap[i].columns;
  Incr2 = Incr1 + 1;

/* forward rows, forward columns */
  scrnx=(float *)elmap[i].scrnptrx;
  scrny=(float *)elmap[i].scrnptry;
  scrnq=(float *)elmap[i].scrnptrq;
  for (Lr=0; Lr<elmap[i].rows && Lr<=RowLon; Lr++)
   {
   for (Lc=0; Lc<elmap[i].columns - 1 && Lc<=ColLat; Lc++)
    {
    if (*scrnq>0.0 && *(scrnq + 1) > 0.0
	 && *(scrnq + Incr2) > 0.0 && *(scrnq + Incr1) > 0.0)
     {
     if (*scrnq > hazeend)
      Edge = 4;
     else if (*scrnq > hazestart)
      Edge = 6;
     else
      Edge = 2;
     PolyVert[0].x = PolyVert[4].x = *scrnx + drawoffsetX;
     PolyVert[1].x = *(scrnx + 1) + drawoffsetX;
     PolyVert[2].x = *(scrnx + Incr2) + drawoffsetX;
     PolyVert[3].x = *(scrnx + Incr1) + drawoffsetX;
     PolyVert[0].y = PolyVert[4].y = *scrny + drawoffsetY;
     PolyVert[1].y = *(scrny + 1) + drawoffsetY;
     PolyVert[2].y = *(scrny + Incr2) + drawoffsetY;
     PolyVert[3].y = *(scrny + Incr1) + drawoffsetY;
     if(Itchy)
       {
       vgl_dumb_setcur(Pixie, 14, 0);
       ClipAreaDrawVGL(Pixie, PolyVert, &IA->cb, 14, Edge);
       } /* if */
     else
       {
       SetOPen(DrawRast, Edge);
       ClipAreaDrawRPort(DrawRast, PolyVert, &IA->cb);
       } /* else */
     } /* if point lies to front of viewer */
    scrnx ++;
    scrny ++;
    scrnq ++;
    } /* for Lc=0... */
   scrnx += ColIncr;
   scrny += ColIncr;
   scrnq += ColIncr;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   } /* for Lr=0... */

  if (error) break;

/* forward rows, reverse columns */
  if (ColLat < elmap[i].columns - 2)
   {
   for (Lr=0; Lr<elmap[i].rows && Lr<=RowLon; Lr++)
    {
    scrnx=(float *)elmap[i].scrnptrx + Lr * elmap[i].columns + elmap[i].columns - 2;
    scrny=(float *)elmap[i].scrnptry + Lr * elmap[i].columns + elmap[i].columns - 2;
    scrnq=(float *)elmap[i].scrnptrq + Lr * elmap[i].columns + elmap[i].columns - 2;
    for (Lc=elmap[i].columns - 2; Lc>ColLat && Lc>=0; Lc--)
     {
     if (*scrnq>0.0 && *(scrnq + 1) > 0.0
	 && *(scrnq + Incr2) > 0.0 && *(scrnq + Incr1) > 0.0)
      {
      if (*scrnq > hazeend)
        Edge = 4;
      else if (*scrnq > hazestart)
        Edge = 6;
      else
        Edge = 2;
      PolyVert[0].x = PolyVert[4].x = *scrnx + drawoffsetX;
      PolyVert[1].x = *(scrnx + 1) + drawoffsetX;
      PolyVert[2].x = *(scrnx + Incr2) + drawoffsetX;
      PolyVert[3].x = *(scrnx + Incr1) + drawoffsetX;
      PolyVert[0].y = PolyVert[4].y = *scrny + drawoffsetY;
      PolyVert[1].y = *(scrny + 1) + drawoffsetY;
      PolyVert[2].y = *(scrny + Incr2) + drawoffsetY;
      PolyVert[3].y = *(scrny + Incr1) + drawoffsetY;
      if(Itchy)
        {
        vgl_dumb_setcur(Pixie, 14, 0);
        ClipAreaDrawVGL(Pixie, PolyVert, &IA->cb, 14, Edge);
        } /* if */
      else
        {
        SetOPen(DrawRast, Edge);
        ClipAreaDrawRPort(DrawRast, PolyVert, &IA->cb);
        } /* else */
      } /* if point lies to front of viewer */
     scrnx --;
     scrny --;
     scrnq --;
     } /* for Lc=elmap[i].columns... */
    if (CheckInput_ID() == ID_BW_CLOSE)
     {
     error = 1;
     break;
     } /* if user abort */
    } /* for Lr=0... */
   } /* if need to draw reverse columns */

  if (error) break;

/* reverse rows, forward columns */
  if (RowLon < elmap[i].rows - 1)
   {
   scrnx=(float *)elmap[i].scrnptrx + (elmap[i].rows - 1) * elmap[i].columns;
   scrny=(float *)elmap[i].scrnptry + (elmap[i].rows - 1) * elmap[i].columns;
   scrnq=(float *)elmap[i].scrnptrq + (elmap[i].rows - 1) * elmap[i].columns;
   for (Lr=elmap[i].rows - 1; Lr>RowLon && Lr>=0; Lr--)
    {
    for (Lc=0; Lc<elmap[i].columns - 1 && Lc<=ColLat; Lc++)
     {
     if (*scrnq>0.0 && *(scrnq + 1) > 0.0
	 && *(scrnq + Incr2) > 0.0 && *(scrnq + Incr1) > 0.0)
      {
      if (*scrnq > hazeend)
        Edge = 4;
      else if (*scrnq > hazestart)
        Edge = 6;
      else
        Edge = 2;
      PolyVert[0].x = PolyVert[4].x = *scrnx + drawoffsetX;
      PolyVert[1].x = *(scrnx + 1) + drawoffsetX;
      PolyVert[2].x = *(scrnx + Incr2) + drawoffsetX;
      PolyVert[3].x = *(scrnx + Incr1) + drawoffsetX;
      PolyVert[0].y = PolyVert[4].y = *scrny + drawoffsetY;
      PolyVert[1].y = *(scrny + 1) + drawoffsetY;
      PolyVert[2].y = *(scrny + Incr2) + drawoffsetY;
      PolyVert[3].y = *(scrny + Incr1) + drawoffsetY;
      if(Itchy)
        {
        vgl_dumb_setcur(Pixie, 14, 0);
        ClipAreaDrawVGL(Pixie, PolyVert, &IA->cb, 14, Edge);
        } /* if */
      else
        {
        SetOPen(DrawRast, Edge);
        ClipAreaDrawRPort(DrawRast, PolyVert, &IA->cb);
        } /* else */
      } /* if point lies to front of viewer */
     scrnx ++;
     scrny ++;
     scrnq ++;
     } /* for Lc=0... */
    scrnx += ColIncr;
    scrny += ColIncr;
    scrnq += ColIncr;
    scrnx -= 2 * elmap[i].columns;
    scrny -= 2 * elmap[i].columns;
    scrnq -= 2 * elmap[i].columns;
    if (CheckInput_ID() == ID_BW_CLOSE)
     {
     error = 1;
     break;
     } /* if user abort */
    } /* for Lr=0... */

   if (error) break;

/* reverse rows, reverse columns */
   if (ColLat < elmap[i].columns - 2)
    {
    for (Lr=elmap[i].rows - 1; Lr>RowLon && Lr>=0; Lr--)
     {
     scrnx=(float *)elmap[i].scrnptrx + Lr * elmap[i].columns + elmap[i].columns - 2;
     scrny=(float *)elmap[i].scrnptry + Lr * elmap[i].columns + elmap[i].columns - 2;
     scrnq=(float *)elmap[i].scrnptrq + Lr * elmap[i].columns + elmap[i].columns - 2;
     for (Lc=elmap[i].columns - 2; Lc>ColLat && Lc>=0; Lc--)
      {
      if (*scrnq>0.0 && *(scrnq + 1) > 0.0
	 && *(scrnq + Incr2) > 0.0 && *(scrnq + Incr1) > 0.0)
       {
       if (*scrnq > hazeend)
         Edge = 4;
       else if (*scrnq > hazestart)
         Edge = 6;
       else
         Edge = 2;
       PolyVert[0].x = PolyVert[4].x = *scrnx + drawoffsetX;
       PolyVert[1].x = *(scrnx + 1) + drawoffsetX;
       PolyVert[2].x = *(scrnx + Incr2) + drawoffsetX;
       PolyVert[3].x = *(scrnx + Incr1) + drawoffsetX;
       PolyVert[0].y = PolyVert[4].y = *scrny + drawoffsetY;
       PolyVert[1].y = *(scrny + 1) + drawoffsetY;
       PolyVert[2].y = *(scrny + Incr2) + drawoffsetY;
       PolyVert[3].y = *(scrny + Incr1) + drawoffsetY;
       if(Itchy)
         {
         vgl_dumb_setcur(Pixie, 14, 0);
         ClipAreaDrawVGL(Pixie, PolyVert, &IA->cb, 14, Edge);
         } /* if */
       else
         {
         SetOPen(DrawRast, Edge);
         ClipAreaDrawRPort(DrawRast, PolyVert, &IA->cb);
         } /* else */
       } /* if point lies to front of viewer */
      scrnx --;
      scrny --;
      scrnq --;
      } /* for Lc=elmap[i].columns... */
     if (CheckInput_ID() == ID_BW_CLOSE)
      {
      error = 1;
      break;
      } /* if user abort */
     } /* for Lr=0... */
    } /* if need to render some columns in reverse */
   }/* if need to render some rows in reverse */

  if (error) break;

#ifdef USE_MORE_UPDATES
		BltBitMapRastPort(Itchy->BitMap, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->RPort, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->Width - (InterWind0->BorderLeft +
		 InterWind0->BorderRight), InterWind0->Height -
		 (InterWind0->BorderTop + InterWind0->BorderBottom), 0xC0);
#endif /* USE_MORE_UPDATES */ 

  BusyWin_Update(BW, NoOfElMaps + 1 + i);
  } /* for i=0... */

 if(!Itchy)
  {
  if (TempRastPtr)
   FreeRaster(TempRastPtr, RasWidth, RasHeight);
  BNDRYOFF(DrawRast);
  } /* if */

 } /* if hidden line removal */

else
 {
 for (i=0; i<NoOfElMaps; i++)
  {
  if (showX)
   {
   scrnx=(float *)elmap[i].scrnptrx;
   scrny=(float *)elmap[i].scrnptry;
   scrnq=(float *)elmap[i].scrnptrq;
   for (Lr=0; Lr<=elmap[i].rows; Lr ++)
    {
    lastq = *scrnq;
    scrnx++;
    scrny++;
    scrnq++;
    for (Lc=1; Lc<elmap[i].columns; Lc++)
     { 
     if (*scrnq>0.0 && lastq>0.0)
      {
      if (Lr == 0 || Lr == elmap[i].rows)
        Edge = 3;
      else if (*scrnq > hazeend)
        Edge = 4;
      else if (*scrnq > hazestart)
        Edge = 6;
      else
        Edge = 2;
      if(!Itchy)
        {
        SetAPen(DrawRast, Edge);
        setlineseg(&ls, *(scrnx - 1) + drawoffsetX, *(scrny - 1) + drawoffsetY,
         *scrnx + drawoffsetX, *scrny + drawoffsetY);
        ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
        } /* if */
      else
        {
        vgl_dumb_setcur(Pixie, Edge, 0);
        vgl_dumb_line(Pixie, (int)*(scrnx - 1) + drawoffsetX, (int)*(scrny - 1) + drawoffsetY,
         (int)*scrnx + drawoffsetX, (int)*scrny + drawoffsetY);
        } /* else */
      } /* if point lies in front of viewer */
     lastq = *scrnq;
     scrnx++;
     scrny++;
     scrnq++;
     } /* for Lc=1... */
    if (CheckInput_ID() == ID_BW_CLOSE)
     {
     error = 1;
     break;
     } /* if user abort */
    } /* for Lr=0... */
   } /* if showX */

  if (error) break;

  if (showY)
   {
   rowmarkerx = (float *)elmap[i].scrnptrx;
   rowmarkery = (float *)elmap[i].scrnptry;
   rowmarkerq = (float *)elmap[i].scrnptrq;
   incrementLr = (long)elmap[i].columns;
   for (Lc=0; Lc<elmap[i].columns; Lc ++)
    {
    scrnx = rowmarkerx;
    scrny = rowmarkery;
    scrnq = rowmarkerq;
    lastq = *scrnq;
    for (Lr=1; Lr<=elmap[i].rows; Lr ++)
     { 
     scrnx += incrementLr;
     scrny += incrementLr;
     scrnq += incrementLr;
     if (*scrnq>0.0 && lastq>0.0)
      {
      if (Lc == 0 || Lc == elmap[i].columns - 1)
        Edge = 3;
      else if (*scrnq > hazeend)
        Edge = 4;
      else if (*scrnq > hazestart)
        Edge = 6;
      else
        Edge = 2;
      if(!Itchy)
        {
        SetAPen(DrawRast, Edge);
        setlineseg(&ls, *(scrnx - incrementLr) + drawoffsetX,
         *(scrny - incrementLr) + drawoffsetY,
         *scrnx + drawoffsetX, *scrny + drawoffsetY);
        ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
        } /* if */
      else
        {
        vgl_dumb_setcur(Pixie, Edge, 0);
        vgl_dumb_line(Pixie, (int)*(scrnx - incrementLr) + drawoffsetX,
        (int)*(scrny - incrementLr) + drawoffsetY,
        (int)*scrnx + drawoffsetX, (int)*scrny + drawoffsetY);
        } /* else */
      } /* if point lies in front of viewer */
     lastq = *scrnq;
     } /* for Lr=1... */
    rowmarkerx++;
    rowmarkery++;
    rowmarkerq++;
    if (CheckInput_ID() == ID_BW_CLOSE)
     {
     error = 1;
     break;
     } /* if user abort */
    } /* for Lc=0... */
   } /* if showY */
  if (error) break;
  BusyWin_Update(BW, NoOfElMaps + 1 + i);
  } /* for i=0; i<NoOfElMaps */
 } /* else no hidden line removal */

 IA->gridpresent = 1;

 return (error);

} /* drawinterview() */

/*********************************************************************/

void computequick(void)
{
 short i, j;
 double ptelev;

 for (i=0; i<NoOfElMaps; i++) {
  for (j=0; j<8; j++) {
   ptelev = BBox[i].elev[j] + (PARC_RNDR_MOTION(13) - BBox[i].elev[j])
	* PARC_RNDR_MOTION(12);
   DP.alt = ptelev * PARC_RNDR_MOTION(14) * elmap[i].elscale;
   DP.lat = BBox[i].lat[j];
   DP.lon = BBox[i].lon[j];
   getviewscrncoords(&BBox[i].scrnx[j], &BBox[i].scrny[j], &BBox[i].scrnq[j]);
  } /* for j=0... */
 } /* for i=0... */
} /* computequick() */

/*********************************************************************/

void DrawInterFresh(short drawgrid)
{

computequick();

IA->gridpresent = 0;

 SetAPen(InterWind0->RPort, 1);
 RectFill(InterWind0->RPort,
	IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);

findfocprof();
if (GridBounds)
 {
 drawgridpts(0);
 }
if (BoxBounds)
 {
 drawquick(7, 6, 3, 0);
 } /* if */
if (ProfileBounds && ! GridBounds)
 {
 drawfocprof(0);
 } /* if */
if (LandBounds)
 {
 drawveryquick(0);
 } /* if */
if (CompassBounds)
 {
 makeviewcenter((short)(! (GridBounds && drawgrid)));
 } /* if */

} /* DrawInterFresh() */

/*********************************************************************/

void drawveryquick(short undraw)
{
 short i, j;
 float fDx, fDy, fDq;
 double ptelev;
 struct lineseg ls;

 for (i=0; i<NoOfElMaps; i++)
  {
  if (undraw) {
   SetAPen(InterWind0->RPort, 1);
   if (BBox[i].scrnq[8]>0.0 && BBox[i].scrnq[9]>0.0) {
    setlineseg(&ls, BBox[i].scrnx[8] + drawoffsetX, BBox[i].scrny[8] + drawoffsetY,
	BBox[i].scrnx[9] + drawoffsetX, BBox[i].scrny[9] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
   if (BBox[i].scrnq[9]>0.0 && BBox[i].scrnq[10]>0.0) {
    setlineseg(&ls, BBox[i].scrnx[9] + drawoffsetX, BBox[i].scrny[9] + drawoffsetY,
	BBox[i].scrnx[10] + drawoffsetX, BBox[i].scrny[10] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
   if (BBox[i].scrnq[10]>0.0 && BBox[i].scrnq[11]>0.0) {
    setlineseg(&ls, BBox[i].scrnx[10] + drawoffsetX, BBox[i].scrny[10] + drawoffsetY,
	BBox[i].scrnx[11] + drawoffsetX, BBox[i].scrny[11] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
   if (BBox[i].scrnq[11]>0.0 && BBox[i].scrnq[8]>0.0) {
    setlineseg(&ls, BBox[i].scrnx[11] + drawoffsetX, BBox[i].scrny[11] + drawoffsetY,
	BBox[i].scrnx[8] + drawoffsetX, BBox[i].scrny[8] + drawoffsetY);
    ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if */
  if (settings.sun)
   {
   DP.alt = 149.0E+6;
   DP.lat = PARC_RNDR_MOTION(26);
   DP.lon = PARC_RNDR_MOTION(27);
   getviewscrncoords(&fDx, &fDy, &fDq);
   DrawHazeRPort(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad, &IA->cb);
/*   DrawCircle(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad);*/
   } /* if */

  if (settings.moon)
   {
   DP.alt = 38.0E+4;
   DP.lat = PARC_RNDR_MOTION(29);
   DP.lon = PARC_RNDR_MOTION(30);
   getviewscrncoords(&fDx, &fDy, &fDq);
   DrawHazeRPort(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad, &IA->cb);
/*   DrawCircle(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad);*/
   } /* if */
  } /* if undraw */

  for (j=8; j<12; j++) {
   ptelev = BBox[i].elev[j] + (PARC_RNDR_MOTION(13) - BBox[i].elev[j])
	* PARC_RNDR_MOTION(12);
   DP.alt = ptelev * PARC_RNDR_MOTION(14) *
		elmap[i].elscale;
   DP.lat = BBox[i].lat[j];
   DP.lon = BBox[i].lon[j];
   getviewscrncoords(&BBox[i].scrnx[j], &BBox[i].scrny[j],
	 	 &BBox[i].scrnq[j]);
  } /* for j=8... */

  SetAPen(InterWind0->RPort, 2);
  if (BBox[i].scrnq[8]>0.0 && BBox[i].scrnq[9]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[8] + drawoffsetX, BBox[i].scrny[8] + drawoffsetY,
	BBox[i].scrnx[9] + drawoffsetX, BBox[i].scrny[9] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[9]>0.0 && BBox[i].scrnq[10]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[9] + drawoffsetX, BBox[i].scrny[9] + drawoffsetY,
	BBox[i].scrnx[10] + drawoffsetX, BBox[i].scrny[10] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[10]>0.0 && BBox[i].scrnq[11]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[10] + drawoffsetX, BBox[i].scrny[10] + drawoffsetY,
	BBox[i].scrnx[11] + drawoffsetX, BBox[i].scrny[11] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[11]>0.0 && BBox[i].scrnq[8]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[11] + drawoffsetX, BBox[i].scrny[11] + drawoffsetY,
	BBox[i].scrnx[8] + drawoffsetX, BBox[i].scrny[8] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
 } /* for i=0... */

 if (settings.sun)
  {
  SetAPen(InterWind0->RPort, 7);
  DP.alt = 149.0E+6;
  DP.lat = PAR_FIRST_MOTION(26);
  DP.lon = PAR_FIRST_MOTION(27);
  getviewscrncoords(&fDx, &fDy, &fDq);
  IA->SunX = fDx;
  IA->SunY = fDy;
  IA->SunRad = PAR_FIRST_MOTION(28) / IA->wscale;
  DrawHazeRPort(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad, &IA->cb);
/*  DrawCircle(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad);*/
  } /* if */

 if (settings.moon)
  {
  SetAPen(InterWind0->RPort, 2);
  DP.alt = 38.0E+4;
  DP.lat = PAR_FIRST_MOTION(29);
  DP.lon = PAR_FIRST_MOTION(30);
  getviewscrncoords(&fDx, &fDy, &fDq);
  IA->MoonX = fDx;
  IA->MoonY = fDy;
  IA->MoonRad = PAR_FIRST_MOTION(31) / IA->wscale;
  DrawHazeRPort(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad, &IA->cb);
/*  DrawCircle(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad);*/
  } /* if */

} /* drawveryquick() */

/*********************************************************************/

void drawquick(short a, short b, short c, short Clear)
{ /* 2 4 6 */
 short i;
 struct lineseg ls;

 if(Clear)
  {
  SetAPen(InterWind0->RPort, 1);
  RectFill(InterWind0->RPort,
	IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
  } /* if */

 for (i=0; i<NoOfElMaps; i++) {
  SetAPen(InterWind0->RPort, a);
  if (BBox[i].scrnq[0]>0.0 && BBox[i].scrnq[1]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[0] + drawoffsetX, BBox[i].scrny[0] + drawoffsetY,
	BBox[i].scrnx[1] + drawoffsetX, BBox[i].scrny[1] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[1]>0.0 && BBox[i].scrnq[5]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[1] + drawoffsetX, BBox[i].scrny[1] + drawoffsetY,
	BBox[i].scrnx[5] + drawoffsetX, BBox[i].scrny[5] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[5]>0.0 && BBox[i].scrnq[4]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[5] + drawoffsetX, BBox[i].scrny[5] + drawoffsetY,
	BBox[i].scrnx[4] + drawoffsetX, BBox[i].scrny[4] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[4]>0.0 && BBox[i].scrnq[0]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[4] + drawoffsetX, BBox[i].scrny[4] + drawoffsetY,
	BBox[i].scrnx[0] + drawoffsetX, BBox[i].scrny[0] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  
  SetAPen(InterWind0->RPort, b);
  if (BBox[i].scrnq[0]>0.0 && BBox[i].scrnq[3]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[0] + drawoffsetX, BBox[i].scrny[0] + drawoffsetY,
	BBox[i].scrnx[3] + drawoffsetX, BBox[i].scrny[3] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[1]>0.0 && BBox[i].scrnq[2]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[1] + drawoffsetX, BBox[i].scrny[1] + drawoffsetY,
	BBox[i].scrnx[2] + drawoffsetX, BBox[i].scrny[2] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[5]>0.0 && BBox[i].scrnq[6]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[5] + drawoffsetX, BBox[i].scrny[5] + drawoffsetY,
	BBox[i].scrnx[6] + drawoffsetX, BBox[i].scrny[6] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[4]>0.0 && BBox[i].scrnq[7]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[4] + drawoffsetX, BBox[i].scrny[4] + drawoffsetY,
	BBox[i].scrnx[7] + drawoffsetX, BBox[i].scrny[7] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */

  SetAPen(InterWind0->RPort, c);
  if (BBox[i].scrnq[7]>0.0 && BBox[i].scrnq[3]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[7] + drawoffsetX, BBox[i].scrny[7] + drawoffsetY,
	BBox[i].scrnx[3] + drawoffsetX, BBox[i].scrny[3] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[3]>0.0 && BBox[i].scrnq[2]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[3] + drawoffsetX, BBox[i].scrny[3] + drawoffsetY,
	BBox[i].scrnx[2] + drawoffsetX, BBox[i].scrny[2] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[2]>0.0 && BBox[i].scrnq[6]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[2] + drawoffsetX, BBox[i].scrny[2] + drawoffsetY,
	BBox[i].scrnx[6] + drawoffsetX, BBox[i].scrny[6] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
  if (BBox[i].scrnq[6]>0.0 && BBox[i].scrnq[7]>0.0) {
   setlineseg(&ls, BBox[i].scrnx[6] + drawoffsetX, BBox[i].scrny[6] + drawoffsetY,
	BBox[i].scrnx[7] + drawoffsetX, BBox[i].scrny[7] + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
  } /* if */
 } /* for i=0... */

} /* drawquick() */

/*********************************************************************/

void constructview(void)
{
 double TempScale;

 initmopar();
 frame = EM_Win->Frame;
 CenterX = PAR_FIRST_MOTION(6) / IA->wscale;
 CenterY = PAR_FIRST_MOTION(7) / IA->hscale;
 TempScale = PAR_FIRST_MOTION(10) * IA->wscale;

 ralt = EARTHRAD + PARC_RNDR_MOTION(0);
 qmax = sqrt(ralt * ralt - EARTHRAD * EARTHRAD);
 horscale = (TempScale * PROJPLANE * tan(.5 * PARC_RNDR_MOTION(11) *
      PiOver180) / 3.5469) / HORSCALEFACTOR;
 vertscale = horscale * settings.picaspect;
 cosviewlat = cos(PARC_RNDR_MOTION(1) * PiOver180);
/*
 PARC_RNDR_MOTION(2) += PARC_RNDR_MOTION(9);
 if (fixfocus)
  PARC_RNDR_MOTION(5) += PARC_RNDR_MOTION(9);
 PARC_RNDR_MOTION(16) += PARC_RNDR_MOTION(9);
*/
 setquickview();

} /* constructview() */

/*********************************************************************/

void compass(double oldaz, double newaz)
{
 short x, y;

 if (! InterWind2) return;

 x = IA->CompRadius * sin(oldaz) - ROUNDING_KLUDGE;
 y = -IA->CompRadius * cos(oldaz) - ROUNDING_KLUDGE;
 SetAPen(InterWind2->RPort, 0);
 Move(InterWind2->RPort, IA->CompHCenter, IA->CompVCenter);
 Draw(InterWind2->RPort, IA->CompHCenter + x, IA->CompVCenter + y);

 x = IA->CompRadius * sin(newaz) - ROUNDING_KLUDGE;
 y = -IA->CompRadius * cos(newaz) - ROUNDING_KLUDGE;
 SetAPen(InterWind2->RPort, 3);
 Move(InterWind2->RPort, IA->CompHCenter, IA->CompVCenter);
 Draw(InterWind2->RPort, IA->CompHCenter + x, IA->CompVCenter + y);

} /* compass */

/*********************************************************************/

short make_compass(void)
{
 ULONG flags, iflags;
 UBYTE c0,c1;
 struct clipbounds cb;

 flags= SMART_REFRESH | WINDOWDRAG | WINDOWDEPTH | WINDOWCLOSE | WINDOWSIZING;
 iflags = CLOSEWINDOW  | ACTIVEWINDOW | NEWSIZE;

 c0 = 0x00;
 c1 = 0x01; 

 if (IA_CompLeft + IA_CompWidth > WCSScrn->Width)
  IA_CompWidth = 0;
 if (IA_CompTop + IA_CompHeight > WCSScrn->Height)
  IA_CompHeight = 0;

 if (! IA_CompWidth || ! IA_CompHeight)
  {
  IA_CompTop = (WCSScrn->Height - 130) / 2;
  IA_CompLeft = (WCSScrn->Width - 120) / 2;
  IA_CompWidth = 120;
  IA_CompHeight = 130;
  } /* if compass size not defined previously */

 InterWind2 = (struct Window *)
     make_window(IA_CompLeft, IA_CompTop, IA_CompWidth, IA_CompHeight,
	"Compass", flags, (ULONG)NULL, c0, c1, WCSScrn);
 if (! InterWind2) {
  return 1;
 } /* if */
 WindowLimits(InterWind2, InterWind2->BorderLeft + InterWind2->BorderRight + 20,
	InterWind2->BorderTop + InterWind2->BorderBottom + 20, -1, -1);

 ModifyIDCMP(InterWind2, iflags);
 InterWind2_Sig = 1L << InterWind2->UserPort->mp_SigBit;
 WCS_Signals |= InterWind2_Sig;

 setclipbounds(InterWind2, &cb);
 IA->CompRadius = (min(cb.highx - cb.lowx, cb.highy - cb.lowy) - 12) / 2;
 IA->CompHCenter = (cb.lowx + cb.highx) / 2;
 IA->CompVCenter = (cb.lowy + cb.highy) / 2;
 SetAPen(InterWind2->RPort, 4);
 DrawEllipse(InterWind2->RPort, IA->CompHCenter + 1, IA->CompVCenter + 1,
	 IA->CompRadius + 2, IA->CompRadius + 2);
 SetAPen(InterWind2->RPort, 2);
 DrawEllipse(InterWind2->RPort,
	 IA->CompHCenter, IA->CompVCenter,
	 IA->CompRadius + 2, IA->CompRadius + 2);
 return 0;

} /* makecompass() */

/*********************************************************************/

void computesinglebox(short i)
{
 short j;
 double ptelev;

 for (j=0; j<13; j++)
  {
  ptelev = BBox[i].elev[j] + (PARC_RNDR_MOTION(13) - BBox[i].elev[j])
	* PARC_RNDR_MOTION(12);
  DP.alt = ptelev * PARC_RNDR_MOTION(14) * elmap[i].elscale;
  DP.lat = BBox[i].lat[j];
  DP.lon = BBox[i].lon[j];
  getviewscrncoords(&BBox[i].scrnx[j], &BBox[i].scrny[j], &BBox[i].scrnq[j]);
  } /* for j=0... */

} /* computesinglebox() */

/*********************************************************************/

void drawfocprof(short erase)
{
 float lastq, *scrnx, *scrny, *scrnq;
 long zip, Lr, Lc;
 struct lineseg ls;

 if (erase)
  SetAPen(InterWind0->RPort, 1);
 else
  {
  SetAPen(InterWind0->RPort, 5);
  computefocprof();
  } /* else */

 zip = FocProf->map->columns;

 scrnx=(float *)FocProf->map->scrnptrx + FocProf->col;
 scrny=(float *)FocProf->map->scrnptry + FocProf->col;
 scrnq=(float *)FocProf->map->scrnptrq + FocProf->col;
 lastq = *scrnq;
 for (Lr=1; Lr<=FocProf->map->rows; Lr ++)
  { 
  scrnx += zip;
  scrny += zip;
  scrnq += zip;
  if (*scrnq>0.0 && lastq>0.0)
   {
   setlineseg(&ls, *(scrnx - zip) + drawoffsetX, *(scrny - zip) + drawoffsetY,
	*scrnx + drawoffsetX, *scrny + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if point lies in front of viewer */
  lastq = *scrnq;
  } /* for Lr=1... */

 zip = FocProf->row * FocProf->map->columns;

 scrnx=(float *)FocProf->map->scrnptrx + zip;
 scrny=(float *)FocProf->map->scrnptry + zip;
 scrnq=(float *)FocProf->map->scrnptrq + zip;
 lastq = *scrnq;
 for (Lc=1; Lc<FocProf->map->columns; Lc++)
  { 
  scrnx++;
  scrny++;
  scrnq++;
  if (*scrnq>0.0 && lastq>0.0)
   {
   setlineseg(&ls, *(scrnx - 1) + drawoffsetX, *(scrny - 1) + drawoffsetY,
	*scrnx + drawoffsetX, *scrny + drawoffsetY);
   ClipDrawRPort(InterWind0->RPort, &IA->cb, &ls);
   } /* if point lies in front of viewer */
  lastq = *scrnq;
  } /* for Lc=1... */

} /* drawfocprof() */

/*********************************************************************/

STATIC_FCN void computefocprof(void) // used locally only -> static, AF 19.7.2021
{
 short *mapptr;
 long zip, Lr, Lc;
 float *scrnx, *scrny, *scrnq;
 double ptelev;

 zip = FocProf->map->columns;

 mapptr = (short *)FocProf->map->map + FocProf->col;
 scrnx = (float *)FocProf->map->scrnptrx + FocProf->col;
 scrny = (float *)FocProf->map->scrnptry + FocProf->col;
 scrnq = (float *)FocProf->map->scrnptrq + FocProf->col;
 for (Lr=0; Lr<=FocProf->map->rows; Lr ++)
  {
  ptelev = *mapptr + (PARC_RNDR_MOTION(13) - *mapptr)
	* PARC_RNDR_MOTION(12);
  DP.alt = ptelev * PARC_RNDR_MOTION(14) * FocProf->map->elscale;
  DP.lat = FocProf->map->lolat + FocProf->col * FocProf->map->steplat;
  DP.lon = FocProf->map->lolong - Lr * FocProf->map->steplong;
  getviewscrncoords(scrnx, scrny, scrnq);
  mapptr += zip;
  scrnx += zip;
  scrny += zip;
  scrnq += zip;
  } /* for Lr=0... */

 zip = FocProf->row * FocProf->map->columns;

 mapptr = (short *)FocProf->map->map + zip;
 scrnx = (float *)FocProf->map->scrnptrx + zip;
 scrny = (float *)FocProf->map->scrnptry + zip;
 scrnq = (float *)FocProf->map->scrnptrq + zip;
 for (Lc=0; Lc<FocProf->map->columns; Lc ++) {
  ptelev = *mapptr + (PARC_RNDR_MOTION(13) - *mapptr)
	* PARC_RNDR_MOTION(12);
  DP.alt = ptelev * PARC_RNDR_MOTION(14) * FocProf->map->elscale;
  DP.lat = FocProf->map->lolat + Lc * FocProf->map->steplat;
  DP.lon = FocProf->map->lolong - FocProf->row * FocProf->map->steplong;
  getviewscrncoords(scrnx, scrny, scrnq);
  mapptr ++;
  scrnx ++;
  scrny ++;
  scrnq ++;
 } /* for Lc=0... */

} /* computefocprof() */


/*********************************************************************/

/* Further hacked version of ClipAreaDraw for VGL */
STATIC_FCN void ClipAreaDrawVGL(PIXMAP *PM, struct vgl_coord *Poly, struct clipbounds *cb,
 char Fill, char Border) // used locally only -> static, AF 19.7.2021
{
 short i = 0, ct = 0, Pts;

 for (; i<4; i++)
  {
  if (Poly[i].x >= cb->lowx && Poly[i].x <=cb->highx)
   {
   if (Poly[i].y >= cb->lowy && Poly[i].y <= cb->highy)
    {
    ct ++;
    } /* if y within bounds */
   } /* if x within bounds */
  } /* for i<4... */

 if (ct)
  { /* Discards trivial clips, all others will happen in scan-fill */
  vgl_dumb_setcur(PM, Fill, 0);

#ifdef VGL_SPLITPOLY
  Pts = 3;
  Poly[4].x = Poly[0].x;
  Poly[4].y = Poly[0].y;
#else
  Pts = 4;
#endif /* VGL_SPLITPOLY */


  vgl_dumb_fillpoly_convex(PM, Pts, Poly, (double)0);
  vgl_dumb_setcur(PM, Border, 0);

#ifndef VGL_DRAWLATE
  if(showX)
	{
	vgl_dumb_line(PM, Poly[0].x, Poly[0].y, Poly[1].x, Poly[1].y);
	} /* if */
  if(showY)
	{
	vgl_dumb_line(PM, Poly[1].x, Poly[1].y, Poly[2].x, Poly[2].y);
	} /* if */
#endif /* VGL_DRAWLATE */

#ifdef VGL_SPLITPOLY
  vgl_dumb_setcur(PM, Fill, 0);
  vgl_dumb_fillpoly_convex(PM, 3, &Poly[2], (double)0);
  vgl_dumb_setcur(PM, Border, 0);
#endif /* VGL_SPLITPOLY */

#ifdef VGL_DRAWLATE
  if(showX)
	{
	vgl_dumb_line(PM, Poly[0].x, Poly[0].y, Poly[1].x, Poly[1].y);
	} /* if */
  if(showY)
	{
	vgl_dumb_line(PM, Poly[1].x, Poly[1].y, Poly[2].x, Poly[2].y);
	} /* if */
#endif /* VGL_DRAWLATE */

  if(showX)
	{
	vgl_dumb_line(PM, Poly[2].x, Poly[2].y, Poly[3].x, Poly[3].y);
	} /* if */
  if(showY)
	{
	vgl_dumb_line(PM, Poly[3].x, Poly[3].y, Poly[4].x, Poly[4].y);
	} /* if */

#ifdef VGL_SHOW_MIDLINE
  if(showX && showY)
  	 vgl_dumb_line(PM, Poly[0].x, Poly[0].y, Poly[2].x, Poly[2].y);
#endif /* VGL_SHOW_MIDLINE */

  } /* if at least one point within clip bounds */

} /* ClipAreaDrawVGL() */


/*********************************************************************/

/* Hacked version of ClipAreaDraw for testing purposes */
STATIC_FCN void ClipAreaDrawRPort(struct RastPort *RP, struct vgl_coord *Poly, struct clipbounds *cb) // used locally only -> static, AF 19.7.2021
{
 short i = 0, ct = 0;

 for (; i<4; i++)
  {
  if (Poly[i].x >= cb->lowx && Poly[i].x <=cb->highx)
   {
   if (Poly[i].y >= cb->lowy && Poly[i].y <= cb->highy)
    {
    ct ++;
    } /* if y within bounds */
   } /* if x within bounds */
  } /* for i<4... */

 if (ct)
  {
  if (ct == 4)
   {
   AreaMove(RP, (long)Poly[0].x, (long)Poly[0].y);
   for (i=1; i<4; i++)
    {
    AreaDraw(RP, (long)Poly[i].x, (long)Poly[i].y);
    } /* for i=0... */
   AreaEnd(RP);
   } /* if all 4 points lie within clip region */

  else
   {
   ClipPoly4RPort(RP, Poly, cb);
   } /* else at least one point within clip region */

  } /* if at least one point within clip bounds */

} /* ClipAreaDrawRPort() */

/*********************************************************************/

STATIC_FCN void ClipPoly4RPort(struct RastPort *RP, struct vgl_coord *Poly, struct clipbounds *cb) // used locally only -> static, AF 19.7.2021
{
 short i = 0, PolyFirst = 1;
 struct lineseg ls;

 Poly[4].x = Poly[0].x;
 Poly[4].y = Poly[0].y;

 for (; i<4; i++)
  {
  setlineseg(&ls, (double)Poly[i].x, (double)Poly[i].y,
	 (double)Poly[i + 1].x, (double)Poly[i + 1].y);
  if (ClipPolySeg(cb, &ls))
   {
   if (PolyFirst)
    {
    AreaMove(RP, (long)ls.firstx, (long)ls.firsty);
    AreaDraw(RP, (long)ls.lastx, (long)ls.lasty);
    PolyFirst = 0;
    } /* if first segment found within bounds */
   else
    {
    AreaDraw(RP, (long)ls.firstx, (long)ls.firsty);
    AreaDraw(RP, (long)ls.lastx, (long)ls.lasty);
    } /* else */
   } /* if ClipLineSeg() found a segment within bounds */
  } /* for i=0... */

 if (! PolyFirst)
  {
  AreaEnd(RP);
  } /* if at least one segment */

} /* ClipPoly4RPort() */

/***********************************************************************/

void Play_Motion(struct RenderAnim *RA)
{
 char basename[32] = {0}, FrameStr[32];
 short error = 0, StartFrame, EndFrame, FrameStep, AltFieldRender;
 ULONG InputID = 0;
 float fDx, fDy, fDq;
 double TempScale;
 struct BusyWindow *BWAN = NULL;
 struct clipbounds cb;

 setclipbounds(InterWind0, &cb);
 AltFieldRender = settings.fieldrender;
 settings.fieldrender = 0;

 SetAPen(InterWind0->RPort, 1);
 RectFill(InterWind0->RPort,
	IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
 IA->gridpresent = 0;

 if (! BuildKeyTable())
  {
  User_Message((CONST_STRPTR)"Parameters Module: Path",
          (CONST_STRPTR)"Out of memory opening key frame table!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  goto EndPlay;
  } /* if no key table */

 if (RA)
  {
  strcpy(graphpath, RA->AnimPath);
  strcpy(basename, RA->AnimName);
  StartFrame = RA->StartFrame;
  if (StartFrame < 0)
   StartFrame = 0;
  if (RA->EndFrame > KT_MaxFrames)
   RA->EndFrame = KT_MaxFrames;
  if (RA->EndFrame >= StartFrame)
   EndFrame = RA->EndFrame;
  else
   EndFrame = KT_MaxFrames;
  FrameStep = RA->FrameStep;
  BWAN = BusyWin_New("Anim", EndFrame - StartFrame, 1, MakeID('B','W','A','N'));
  } /* if more than one DEM */
 else
  {
  StartFrame = 0;
  EndFrame = KT_MaxFrames;
  FrameStep = abs(settings.stepframes);
  } /* else */

 initmopar();
 initpar();

 while (InputID != ID_EMTL_PLAY)
  {
  for (frame=StartFrame; frame<=EndFrame; )
   {
   setvalues();

   CenterX = PARC_RNDR_MOTION(6) / IA->wscale;
   CenterY = PARC_RNDR_MOTION(7) / IA->hscale;
   TempScale = PAR_FIRST_MOTION(10) * IA->wscale;

   ralt = EARTHRAD + PARC_RNDR_MOTION(0);
   qmax = sqrt(ralt * ralt - EARTHRAD * EARTHRAD);
   horscale = (TempScale * PROJPLANE * tan(.5 * PARC_RNDR_MOTION(11) *
       PiOver180) / 3.5469) / HORSCALEFACTOR;
   vertscale = horscale * settings.picaspect;
   cosviewlat = cos(PARC_RNDR_MOTION(1) * PiOver180);
  
   if (! setquickview())
    {
    error = 1;
    break;
    } /* if user cancels */
   if (RA)
    {
    if (RA->OutToFile)
     {
     if (frame > 99) sprintf(graphname, "%s%d", basename, frame);
     else if (frame > 9) sprintf(graphname, "%s0%d", basename, frame);
     else sprintf(graphname, "%s00%d", basename, frame);
     } /* if bitmap file output */
    IA->recompute = 1;
    sprintf(FrameStr, "Frame %d/%d", frame - StartFrame + 1, EndFrame - StartFrame + 1);
    BW = BusyWin_New(FrameStr, NoOfElMaps * 2, 1, MakeID('B','W','I','D'));

    if (! computeview(NULL))
     {
     BusyWin_Del(BW);
     BW = NULL;
     break;
     } /* if drawing aborted */
    Itchy = NULL;
    if((Pixie = vgl_makepixmap(InterWind0->Width, InterWind0->Height)))
      {
      vgl_dumb_setcur(Pixie, 3, 1);
      vgl_dumb_clear(Pixie);
      if((Itchy = ScratchRast_New(InterWind0->Width, InterWind0->Height, 4)))
    	{
     	error = drawinterview();
	ScratchRast_CornerTurn(Pixie, Itchy);
        if (settings.sun)
         {
         SetAPen(Itchy, 7);
         DP.alt = 149.0E+6;
         DP.lat = PARC_RNDR_MOTION(26);
         DP.lon = PARC_RNDR_MOTION(27);
         getviewscrncoords(&fDx, &fDy, &fDq);
         IA->SunX = fDx;
         IA->SunY = fDy;
         IA->SunRad = PARC_RNDR_MOTION(28) / IA->wscale;
         DrawHazeRPort(Itchy, IA->SunX, IA->SunY, IA->SunRad, &IA->cb);
/*         DrawCircle(Itchy, IA->SunX, IA->SunY, IA->SunRad);*/
         } /* if */

        if (settings.moon)
         {
         SetAPen(Itchy, 2);
         DP.alt = 38.0E+4;
         DP.lat = PARC_RNDR_MOTION(29);
         DP.lon = PARC_RNDR_MOTION(30);
         getviewscrncoords(&fDx, &fDy, &fDq);
         IA->MoonX = fDx;
         IA->MoonY = fDy;
         IA->MoonRad = PARC_RNDR_MOTION(31) / IA->wscale;
         DrawHazeRPort(Itchy, IA->MoonX, IA->MoonY, IA->MoonRad, &IA->cb);
/*         DrawCircle(Itchy, IA->MoonX, IA->MoonY, IA->MoonRad);*/
         } /* if */
	WaitBOVP(&WCSScrn->ViewPort); /* BeamSync */
	BltBitMapRastPort(Itchy->BitMap, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->RPort, InterWind0->BorderLeft,
		 InterWind0->BorderTop, InterWind0->Width - (InterWind0->BorderLeft +
		 InterWind0->BorderRight), InterWind0->Height -
		 (InterWind0->BorderTop + InterWind0->BorderBottom), 0xC0);
	WaitBlit();
        if (RA->OutToFile && ! error)
         error = saveILBM(0, 0, Itchy, NULL, NULL, 0, 1, 1, InterWind0->Width, InterWind0->Height);
        ScratchRast_Del(Itchy);
        } /* if */
      vgl_freepixmap(Pixie);
      } /* if */
    if(!Itchy)
     {
     SetAPen(InterWind0->RPort, 1);
     RectFill(InterWind0->RPort,
	  IA->cb.lowx, IA->cb.lowy, IA->cb.highx, IA->cb.highy);
     error = drawinterview();
     if (settings.sun)
      {
      SetAPen(InterWind0->RPort, 7);
      DP.alt = 149.0E+6;
      DP.lat = PARC_RNDR_MOTION(26);
      DP.lon = PARC_RNDR_MOTION(27);
      getviewscrncoords(&fDx, &fDy, &fDq);
      IA->SunX = fDx;
      IA->SunY = fDy;
      IA->SunRad = PARC_RNDR_MOTION(28) / IA->wscale;
      DrawHazeRPort(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad, &IA->cb);
/*      DrawCircle(InterWind0->RPort, IA->SunX, IA->SunY, IA->SunRad);*/
      } /* if */

     if (settings.moon)
      {
      SetAPen(InterWind0->RPort, 2);
      DP.alt = 38.0E+4;
      DP.lat = PARC_RNDR_MOTION(29);
      DP.lon = PARC_RNDR_MOTION(30);
      getviewscrncoords(&fDx, &fDy, &fDq);
      IA->MoonX = fDx;
      IA->MoonY = fDy;
      IA->MoonRad = PARC_RNDR_MOTION(31) / IA->wscale;
      DrawHazeRPort(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad, &IA->cb);
/*      DrawCircle(InterWind0->RPort, IA->MoonX, IA->MoonY, IA->MoonRad);*/
      } /* if */
     if (RA->OutToFile && ! error)
      error = saveILBM(0, 0, InterWind0->RPort, NULL, NULL, 0, 1, 1, InterWind0->Width, InterWind0->Height);
     } /* if */

    BusyWin_Del(BW);
    BW = NULL;
    if (error)
     break;
    } /* if save animation */
   else
    {
    if (GridBounds)
     {
     drawgridpts(1);
     findfocprof();
     if ((error = drawgridpts(0)) != 0)
      break;
     } /* if */
    if(BoxBounds)
     {
     drawquick(1, 1, 1, 0);
     computequick();
     drawquick(7, 6, 3, 0);
     } /* if */ 
    if (ProfileBounds && ! GridBounds)
     {
     drawfocprof(1);
     findfocprof();
     drawfocprof(0);
     } /* if */
    if(LandBounds) 
     {
     drawveryquick(1);
     } /* if */
    if (CompassBounds)
     {
     makeviewcenter(1);
     } /* if */
    }/* else not RA */

   if (EMTL_Win)
    {
    sprintf(str, "%1d", frame);
    set(EMTL_Win->FrameTxt, MUIA_Text_Contents, (ULONG)str);
    if ((InputID = CheckInput_ID()) == ID_EMTL_PLAY) break;
    } /* check for stop */
   if (RA)
    {
    if ((InputID = CheckInput_ID()) == ID_BW_CLOSE)
     {
     break;
     }
    } /* check abort */
   if ((InputID & 0xffff0000) == ID_EMIA_WINDOW)
    {
    Handle_EMIA_Window(InputID);
    if (! EMIA_Win)
     {
     InputID = ID_EMTL_PLAY;
     break;
     } /* if user closed Cam View Control */
    } /* if Camera View Control ID */
   frame += FrameStep;
   if (BWAN)
    {
    BusyWin_Update(BWAN, frame - StartFrame + 1);
    } /* if Image Busy Window open */

   } /* for frame=... */

  if (RA || error)
   break;

  } /* while */

EndPlay:

 if (BWAN)
  BusyWin_Del(BWAN);
 if (KT)
  FreeKeyTable();
 if (IA)
  IA->recompute = 1;
 if (RA && RA->OutToFile)
  strcpy(graphname, basename);
 settings.fieldrender = AltFieldRender;

} /* Play_Motion() */
