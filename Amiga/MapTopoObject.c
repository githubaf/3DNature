/* MapTopoObject.c (ne gismaptopoobject.c 14 Jan 1994 CXH)
**
** Built, Shredded, Ripped, Mangled, Puree'd, Diced, Chopped, Julienned,
** and otherwise downsized, reorganized (you forgot galvanized!)
** galvanized and Simonized from gisam.c into lots of little pieces on
** 24 Jul 1993 by Chris "Xenon" Hanson.
** Original masterpiece the loving work of Gary R. Huber, 1992-93.
*/

#include "WCS.h"
#include "GUIDefines.h"

STATIC_FCN short setfaceone(struct elmapheaderV101 *map);  // AF static 16.July2021
STATIC_FCN short setfacetwo(struct elmapheaderV101 *map);  // AF static 16.July2021
STATIC_FCN void VertexIndex_Del(struct VertexIndex *Vtx);  // used locally only -> static, AF 19.7.2021
STATIC_FCN short VertexIndex_New(struct VertexIndex *Vtx, long MaxFract); // used locally only -> static, AF 23.7.2021
STATIC_FCN void rendercloud(struct Window *win, short *CloudVal, short *IllumVal, short Elev); // used locally only -> static, AF 23.7.2021
STATIC_FCN void Face_Render(struct elmapheaderV101 *map, struct faces *Vertex,
        struct Window *win, struct CloudData *CD); // used locally only -> static, AF 23.7.2021
STATIC_FCN void FractFace_Render(struct elmapheaderV101 *map,
        struct Window *win, short WhichFace, struct CloudData *CD); // used locally only -> static, AF 23.7.2021
STATIC_FCN short MakeFractalMap(struct elmapheaderV101 *map,
        BYTE *FractalMap, long FractalMapSize); // used locally only -> static, AF 23.7.2021
STATIC_FCN void CloudPointSort(short *CloudVal, short *IllumVal); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setcloudfaceone(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
STATIC_FCN short FractalLevel(short MaxSize); // used locally only -> static, AF 23.7.2021
STATIC_FCN short MapCloudLayer(struct elmapheaderV101 *map,
        struct CloudData *CD,  struct CloudLayer *CL, struct CloudLayer *More,
        double MinAmp, double MaxAmp, short j, struct Window *win/*, UBYTE *CldMap*/); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setquickfaceone(struct elmapheaderV101 *map, long Lr, long Lc); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setcloudfacetwo(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setquickface(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setquickfacetwo(struct elmapheaderV101 *map, long Lr, long Lc); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setfacetwo(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
STATIC_FCN short setface(struct elmapheaderV101 *map); // used locally only -> static, AF 23.7.2021
STATIC_FCN void renderface(struct elmapheaderV101 *map, struct Window *win, struct CloudData *CD); // used locally only -> static, AF 23.7.2021


short maptopoobject(struct elmapheaderV101 *map, struct Window *win,
	short DEMnum, short NumDEMs, struct CloudData *CD)
{
 short y, z, j, k, error = 0, Smoothing = 0;
 long stepct, RowLon, ColLat, FaceColSize;
 struct faces **FaceIndex, *Face[3], *MaxBase;
 float ppp;
 double ptelev, ReducedLon;
 struct faces Vertex[3];
 struct BusyWindow *BWDE;
 char DEMstr[16];
/*
StudyVertex = 0;
*/
 dir=10;
 Random=0.0;
 cloudcover = 0.0;
 DMod = 0;
 map->LatRange = (map->columns - 1) * map->steplat;
 map->LonRange = map->rows * map->steplong;

 fractperturb[0] = settings.displacement;
 for (j=1; j<10; j++)
  fractperturb[j] = fractperturb[j - 1] / 2.0;

 if (map->face)
  {
RetrySmooth:
  if ((FaceIndex = (struct faces **)get_Memory(16 * sizeof (struct faces *), MEMF_ANY)) == NULL)
   {
   if (User_Message_Def((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Out of memory allocating\
 Smoothing Index array!", (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
    goto RetrySmooth;
   else
    return (1);
   } /* if */
  FaceColSize = 2 * map->columns;
  MaxBase = map->face + 2 * FaceColSize;
  Smoothing = 1;
  } /* if */

 else if (settings.displace)
  {
  if (! map->fractal)
   settings.fixfract = 1;
/* no tool files for levels greater than 6 */
  if (settings.fractal > 6)
   settings.fractal = 6;
  for (j=1; j<=settings.fractal; j++)
   {
   if (! VertexIndex_New(&Vtx[j], (long)j))
    {
    if (User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Error allocating or reading Fractal Index arrays!\n\
Continue without Fractal Displacement Mapping?", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
     {
     settings.displace = 0;
     for (k=1; k<=j; k++)
      VertexIndex_Del(&Vtx[k]);
     break;
     } /* if */
    else
     {
     error = 1;
     goto EndMap;
     }  /* else */
    } /* if no vertex index stuff */
   } /* for j=1... */
  } /* if displacement mapping */

 sprintf(DEMstr, "DEM %d/%d", DEMnum, NumDEMs);
 BWDE = BusyWin_New(DEMstr, (map->rows + 1) * 2, 0, MakeID('B','W','D','E'));

/* compute screen coordinates */
 map->facect = 0;
 for (map->Lr=0; map->Lr<=map->rows; map->Lr++)
  {
  for (map->Lc=0; map->Lc<map->columns; map->Lc++)
   {
   ptelev = *(map->map + map->facect) * map->elscale / ELSCALE_METERS;
   ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
   *(map->lmap + map->facect) = (long)ptelev;
   if (ptelev <= SeaLevel && ! map->ForceBath)
    ptelev = SeaLevel;
   DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
   DP.lat = map->lolat + map->Lc * map->steplat;
   DP.lon = map->lolong - map->Lr * map->steplong;
   getscrncoords(map->scrnptrx + map->facect, map->scrnptry + map->facect,
		map->scrnptrq + map->facect, NULL, NULL);
   map->facect ++;
   } /* for map->Lc=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, map->Lr + 1);
  } /* for map->Lr=0... */

 if (map->fractal && ! settings.fixfract)
  MakeFractalMap(map, map->fractal, map->fractalsize);
 
 treehtfact = 0.0;

 if (error)
  goto EndMap;

/* draw topo object */
 ReducedLon = PARC_RNDR_MOTION(2);
 while (ReducedLon >= 360.0)
  ReducedLon -= 360.0;
 while (ReducedLon < 0.0)
  ReducedLon += 360.0;
 if (abs(map->lolong - ReducedLon) < 180.0)
  RowLon = /* -1.0 +*/ (map->lolong - ReducedLon) / map->steplong;
 else
  RowLon = /* -1.0 +*/ (ReducedLon - map->lolong) / map->steplong;

 ColLat = /* -1.0 +*/ (PARC_RNDR_MOTION(1) - map->lolat) / map->steplat;
 if (RowLon < 0) RowLon = 0;
 else if (RowLon > map->rows) RowLon = map->rows;
 if (ColLat < 0) ColLat = 0;
 else if (ColLat > map->columns - 1) ColLat = map->columns - 1;

 stepct = map->rows + 1;

 if (Smoothing)
  {
  OldLightingModel = 1;
  SmoothFace_ColSet(map, Face, FaceColSize);
  for (map->Lr = RowLon; map->Lr < map->rows; map->Lr++)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace1;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 0);
      VertexOne_Set(map, FaceIndex, Vertex);
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
NextFace1:
    map->Lc ++;
    map->facect ++;
    map->fracct++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 1);
      VertexTwo_Set(map, FaceIndex, Vertex);
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace2;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 1);
      VertexTwo_Set(map, FaceIndex, Vertex);
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
NextFace2:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 0);
      VertexOne_Set(map, FaceIndex, Vertex);
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   SmoothFace_IncrColSwap(map, Face, MaxBase, FaceColSize);
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... forward rows */

  if (error)
   goto EndMap;

  SmoothFace_ColSet(map, Face, FaceColSize);
  for (map->Lr = RowLon - 1; map->Lr >= 0; map->Lr--)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace3;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 0);
      VertexOne_Set(map, FaceIndex, Vertex);
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
NextFace3:
    map->Lc ++;
    map->facect ++;
    map->fracct ++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 1);
      VertexTwo_Set(map, FaceIndex, Vertex);
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace4;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 1);
      VertexTwo_Set(map, FaceIndex, Vertex);
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
NextFace4:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FaceIndex_Set(Face, FaceIndex, map->Lc, 0);
      VertexOne_Set(map, FaceIndex, Vertex);
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      Face_Render(map, Vertex, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   SmoothFace_DecrColSwap(map, Face, MaxBase, FaceColSize);
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... reverse rows */
  }

/* Fractal displacement */

 else if (settings.displace)
  {
  OldLightingModel = 0;
  for (map->Lr = RowLon; map->Lr < map->rows; map->Lr++)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace5;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      FractFace_Render(map, win, 0, CD);
      } /* if */
     } /* if */
NextFace5:
    map->Lc ++;
    map->facect ++;
    map->fracct ++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      FractFace_Render(map, win, 1, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace6;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      FractFace_Render(map, win, 1, CD);
      } /* if */
     } /* if */
NextFace6:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      FractFace_Render(map, win, 0, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... forward rows */

  if (error)
   goto EndMap;

  for (map->Lr = RowLon - 1; map->Lr >= 0; map->Lr--)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace7;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      FractFace_Render(map, win, 0, CD);
      } /* if */
     } /* if */
NextFace7:
    map->Lc ++;
    map->facect ++;
    map->fracct ++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      FractFace_Render(map, win, 1, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace8;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      FractFaceTwo_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 120);
      FractFace_Render(map, win, 1, CD);
      } /* if */
     } /* if */
NextFace8:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      FractFaceOne_Setup(map, map->Lr, map->Lc);
      hseed = map->facect + map->facect * (map->facect % 191);
      FractFace_Render(map, win, 0, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... reverse rows */
  } /* else if fractal displacement */

/* Normal, old-fashioned style mapping */

 else
  {
  OldLightingModel = 1;
  for (map->Lr = RowLon; map->Lr < map->rows; map->Lr++)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace9;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      faceone(map);
      hseed = map->facect + map->facect * (map->facect % 191);
      renderface(map, win, CD);
      } /* if */
     } /* if */
NextFace9:
    map->Lc ++;
    map->facect ++;
    map->fracct ++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      facetwo(map);
      hseed = map->facect + map->facect * (map->facect % 120);
      renderface(map, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace10;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      facetwo(map);
      hseed = map->facect + map->facect * (map->facect % 120);
      renderface(map, win, CD);
      } /* if */
     } /* if */
NextFace10:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      faceone(map);
      hseed = map->facect + map->facect * (map->facect % 191);
      renderface(map, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... forward rows */

  if (error)
   goto EndMap;

  for (map->Lr = RowLon - 1; map->Lr >= 0; map->Lr--)
   {
   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2;
   for (map->Lc = ColLat; map->Lc < map->columns - 1; map->fracct++)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace11;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      faceone(map);
      hseed = map->facect + map->facect * (map->facect % 191);
      renderface(map, win, CD);
      } /* if */
     } /* if */
NextFace11:
    map->Lc ++;
    map->facect ++;
    map->fracct ++;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      facetwo(map);
      hseed = map->facect + map->facect * (map->facect % 120);
      renderface(map, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... forward columns */

   map->facect = map->Lr * map->columns + ColLat;
   map->fracct = (map->facect - map->Lr) * 2 - 1;
   for (map->Lc = ColLat; map->Lc > 0; map->fracct--)
    {
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      goto NextFace12;
    if (setfacetwo(map))
     {
     if (qqq > qmin)
      {
      facetwo(map);
      hseed = map->facect + map->facect * (map->facect % 120);
      renderface(map, win, CD);
      } /* if */
     } /* if */
NextFace12:
    map->Lc --;
    map->facect --;
    map->fracct --;
    if (map->fractal)
     if (map->fractal[map->fracct] < 0)
      continue;
    if (setfaceone(map))
     {
     if (qqq > qmin)
      {
      faceone(map);
      hseed = map->facect + map->facect * (map->facect % 191);
      renderface(map, win, CD);
      } /* if */
     } /* if */
    } /* for map->Lc=... reverse columns */
   stepct ++;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 1;
    break;
    } /* if user abort */
   BusyWin_Update(BWDE, stepct);
   } /* for map->Lr=... reverse rows */
  } /* else no face smoothing */

EndMap:

 if (Smoothing)
  {
  if (FaceIndex)
   free_Memory(FaceIndex, 16 * sizeof (struct faces *));
  } /* if */

 else if (settings.displace)
  {
  for (j=1; j<=settings.fractal; j++)
   VertexIndex_Del(&Vtx[j]);
  } /* if displacement fractal mapping */

 if (error || (! strcmp(DBase[OBN].Special, "TOP") && ! map->MapAsSFC))
  {
  if (BWDE) BusyWin_Del(BWDE);
  return(error);
  }

/* draw surface grid */

 if (settings.drawgrid)
  {
  if (settings.linetoscreen == 0)
   {
SaveRepeat:
   error = writelinefile(map, 1);
   if (error)
    {
    fclose(fvector);
    User_Message((CONST_STRPTR)"Render Module",
            (CONST_STRPTR)"Error saving vector vertices to file!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
NewFileRequest:
    if (getfilename(1, "New Line Save Path", linepath, linefile))
     {
     char filename[255];

     strmfp(str, linepath, linefile);
     sprintf(filename, "%s%d", str, frame);
     if ((fvector = fopen(filename, "w")) == NULL)
      {
      User_Message((CONST_STRPTR)"Render Module", (CONST_STRPTR)"Can't open vector file for output!", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
      goto NewFileRequest;
      } /* if */
     else goto SaveRepeat;
     } /* if getfilename */
    settings.linetoscreen = -1;
    if (BWDE) BusyWin_Del(BWDE);
    return (1);
    } /* if error */
   if (BWDE) BusyWin_Del(BWDE);
   return (0);
   } /* if settings.linetoscreen == 0 */
  sunangle = sqrt(sunlong * sunlong + sunlat * sunlat) * .7071;
  if (sunangle > 1.57079) sunangle = 1.57079;
  sunfactor = 1.0 - cos(sunangle);
  sunshade = sunfactor * PARC_RNDR_MOTION(22);
  altred = PARC_RNDR_COLOR(5, 0);
  altgreen = PARC_RNDR_COLOR(5, 1);
  altblue = PARC_RNDR_COLOR(5, 2);
  if (settings.linefade)
   {
   altred -= sunshade * altred;
   altgreen -= sunshade * altgreen;
   altblue -= sunshade * altblue;
   } /* if */

  if (showX)
   {
   for (map->Lr=0; map->Lr<=map->rows; map->Lr+=settings.gridsize)
    {
    for (z=j=k=0; z<map->columns-1; z++,j=k=0)
     {
     map->facept[0] = z + map->Lr * map->columns;
     map->facept[1] = z + 1 + map->Lr * map->columns;
     for (y=0; y<2; y++)
      {
      if (*(map->scrnptrq + map->facept[y]) < 0.0)
       {
       j = 2;
       break;
       } /* if */
      xx[y] = *(map->scrnptrx + map->facept[y]);
      yy[y] = *(map->scrnptry + map->facept[y]);
      if (!y) qqq = *(map->scrnptrq + map->facept[y]);
      else
       {
       ppp = *(map->scrnptrq + map->facept[y]);
       if (ppp < qqq) qqq = ppp;
       qqq -= settings.lineoffset;
       if (qqq < 0.0) qqq = 0.0;
       } /* else */
      if (xx[y] < 0) j += 1;
      else if (xx[y] > wide) j += 1;
      if (yy[y] < 0) k +=1;
      else if (yy[y] > high) k += 1;
      } /* for y<2 */
     if (j < 2 && k < 2)
      {
      FadeLine(map->lmap[map->facept[0]]);
      EndDrawLine(win, 1, 1);
      } /* if */
     } /* for z<map->columns */
    } /* for map->Lr<=map->rows */
   } /* if showX */
  if (showY)
   {
   for (map->Lc=0; map->Lc<map->columns; map->Lc+=settings.gridsize)
    {
    for (z=j=k=0; z<map->rows; z++,j=k=0)
     {
     map->facept[0] = map->Lc + z * map->columns;
     map->facept[1] = map->Lc + (z+1) * map->columns;
     for (y=0; y<2; y++)
      {
      if (*(map->scrnptrq + map->facept[y]) < 0.0)
       {
       j = 2;
       break;
       } /* if */
      xx[y] = *(map->scrnptrx + map->facept[y]);
      yy[y] = *(map->scrnptry + map->facept[y]);
      if (!y) qqq = *(map->scrnptrq + map->facept[y]);
      else
       {
       ppp = *(map->scrnptrq + map->facept[y]);
       if (ppp < qqq) qqq = ppp;
       qqq -= settings.lineoffset;
       if (qqq < 0.0) qqq = 0.0;
       } /* else */
      if (xx[y] < 0) j += 1;
      else if (xx[y] > wide) j += 1;
      if (yy[y] < 0) k += 1;
      else if (yy[y] > high) k += 1;
      } /* for y<2 */
     if (j<2 && k<2)
      {
      FadeLine(map->lmap[map->facept[0]]);
      EndDrawLine(win, 1, 1);
      } /* if */
     } /* for z<map->rows */
    } /* for map->Lc<map->columns */
   } /* if showY */
  } /* if settings.drawgrid */

 if (BWDE) BusyWin_Del(BWDE);
 return (0);

} /* maptopoobject() */

/*********************************************************************/

STATIC_FCN void Face_Render(struct elmapheaderV101 *map, struct faces *Vertex,
	struct Window *win, struct CloudData *CD) // used locally only -> static, AF 23.7.2021
{
 long fract;
 struct FaceData Data;

 aspect = (Vertex[0].aspect + Vertex[1].aspect + Vertex[2].aspect) / 3.0;

 for (b=0; b<3; b++)
  {
  polyx[0][b] = ptx[b];    
  polyy[0][b] = pty[b];  
  polyel[0][b] = map->lmap[map->facept[b]];
  polyq[0][b] = ptqq[b];
  polyslope[0][b] = Vertex[b].slope;
  polydiplat[0][b] = Vertex[b].diplat;
  polydiplon[0][b] = Vertex[b].diplong;
  polylat[0][b] = latface[b];
  polylon[0][b] = longface[b];
  polycld[0][b] = CD ? CloudCover_Set(CD, polylat[0][b], polylon[0][b]): 0.0;
  } /* for */
 if (relelev.map)
  {
  polyrelel[0][0] = relelev.map[map->facept[0]];
  polyrelel[0][1] = relelev.map[map->facept[1]];
  polyrelel[0][2] = relelev.map[map->facept[2]];
  }
 else
  {
  polyrelel[0][0] = 0.0;
  polyrelel[0][1] = 0.0;
  polyrelel[0][2] = 0.0;
  }
 if (settings.fixfract)
  {
  maxfract = settings.fractal < DBase[OBN].MaxFract ?
	settings.fractal: DBase[OBN].MaxFract;
  if (map->fractal)
   {
   if (map->fractal[map->fracct] < maxfract)
    maxfract = map->fractal[map->fracct];
   } /* if fractal map */
  } /* if fixed fractal level */
 else
  {
  fract = max(abs(ptx[0] - ptx[1]), pty[1] - pty[0]);
  fract = max(fract, abs(ptx[0] - ptx[2]));
  fract = max(fract, pty[2] - pty[0]);
  fract = max(fract, abs(ptx[1] - ptx[2]));
  fract = max(fract, pty[2] - pty[1]);
  maxfract=0;
  while (fract > 2 && maxfract < settings.fractal
	&& maxfract < DBase[OBN].MaxFract)
   {
   fract >>= 1;
   maxfract ++;
   } /* while */
  } /* else not fixed fractal level */

 fracount[maxfract] ++;

 srand48(hseed);
 dir = 3.99 * drand48() + ROUNDING_KLUDGE;

 PtrnOffset = map->steplat * LATSCALE / (maxfract + 1);
 HalfPtrnOffset = PtrnOffset * .5;	/* this used to be .75 */
 PtrnOffset *= 2.0;

 ZeroCoords(&PP[3]);
 ZeroCoords(&PP[4]);

 if (maxfract == 0)
  {
  b = 0;
  Point_Sort();
  xx[0] = polyx[0][0]; yy[0] = polyy[0][0];
  xx[1] = polyx[0][1]; yy[1] = polyy[0][1];
  xx[2] = polyx[0][2]; yy[2] = polyy[0][2];
  el = (polyel[0][0] + polyel[0][1] + polyel[0][2]) / 3.0;
  qqq = (polyq[0][0] + polyq[0][1] + polyq[0][2]) / 3.0;
  facelat = (polylat[0][0] + polylat[0][1] + polylat[0][2]) / 3.0;
  facelong = (polylon[0][0] + polylon[0][1] + polylon[0][2]) / 3.0;
  slope = (polyslope[0][0] + polyslope[0][1] + polyslope[0][2]) / 3.0;
  relel = polyrelel[0][0] + polyrelel[0][1] + polyrelel[0][2];
  diplat = (polydiplat[0][0] + polydiplat[0][1] + polydiplat[0][2]) / 3.0;
  diplong = (polydiplon[0][0] + polydiplon[0][1] + polydiplon[0][2]) / 3.0;
  cloudcover = (polycld[0][0] + polycld[0][1] + polycld[0][2]) / 3.0;
  treerand4 = drand48();
  treerand3 = drand48();
  treerand2 = drand48();
  treerand = drand48();
  Random = -.2 * treerand + .1;
  Data.El[0] = polyel[0][0];
  Data.El[1] = polyel[0][1];
  Data.El[2] = polyel[0][2];

  MapTopo(map, win, map->MapAsSFC, 0, 1, &Data.El[0]);
  } /* if no recursion necessary, fractal level 0 */
 else
  {
  b = 1;
/* begin perturbation stuff */
  if (settings.perturb)
   {
   dlat = diplat;
   dlong = diplong;
   dslope = slope;

   maxdlat = diplat * 1.5;
   mindlat = diplat * .5;
   if (maxdlat < mindlat) swmem(&maxdlat, &mindlat, 8);
   maxdlat = maxdlat > 1.2 ? 1.5: maxdlat + .3;
   mindlat = mindlat < -1.2 ? -1.5: mindlat - .3;
   maxdlong = diplong * 1.5;
   mindlong = diplong * .5;
   if (maxdlong < mindlong) swmem(&maxdlong, &mindlong, 8);
   maxdlong = maxdlong > 1.2 ? 1.5: maxdlong + .3;
   mindlong = mindlong < -1.2 ? -1.5: mindlong - .3;
   } /* if */
/* end perturbation stuff */

  Recurse(map, win, map->MapAsSFC, CD, &Data);
  } /* else use recursion, fractal level > 0 */

} /* Face_Render() */

/********************************************************************/

STATIC_FCN void FractFace_Render(struct elmapheaderV101 *map,
	struct Window *win, short WhichFace, struct CloudData *CD) // used locally only -> static, AF 23.7.2021
{
 struct FaceData Data;
 struct faces Face;
/*
 ULONG Seed;
 char *SeedBytes;
 double OffsetLat, OffsetLon;

 OffsetLat = map->steplat / 2.0;
 OffsetLon = map->steplong / 2.0;
*/
 for (b=0; b<3; b++)
  {
  polyx[0][b] = ptx[b];    
  polyy[0][b] = pty[b];  
  polyel[0][b] = map->lmap[map->facept[b]];
  polyq[0][b] = ptqq[b];

/* testing polygon vertex offset
  Seed = (fabs((map->lolong - longface[b]) / map->LonRange) * USHRT_MAX);
  Seed <<= 16;
  Seed += fabs((latface[b] - map->lolat) / map->LatRange) * USHRT_MAX;
  SeedBytes = (char *)&Seed;
  swmem(&SeedBytes[0], &SeedBytes[1], 1);
  swmem(&SeedBytes[2], &SeedBytes[3], 1);
  srand48(Seed);
  latface[b] += ((drand48() - .5) * OffsetLat);
  longface[b] += ((drand48() - .5) * OffsetLon);
 end test */
  polylat[0][b] = latface[b];
  polylon[0][b] = longface[b];
  polycld[0][b] = CD ? CloudCover_Set(CD, polylat[0][b], polylon[0][b]): 0.0;
  } /* for */
 if (relelev.map)
  {
  polyrelel[0][0] = relelev.map[map->facept[0]];
  polyrelel[0][1] = relelev.map[map->facept[1]];
  polyrelel[0][2] = relelev.map[map->facept[2]];
  }
 else
  {
  polyrelel[0][0] = 0.0;
  polyrelel[0][1] = 0.0;
  polyrelel[0][2] = 0.0;
  }
 maxfract = settings.fractal < DBase[OBN].MaxFract ?
	settings.fractal: DBase[OBN].MaxFract;
 if (map->fractal)
  {
  if (map->fractal[map->fracct] < maxfract)
   maxfract = map->fractal[map->fracct];
  } /* if fractal map */

 fracount[maxfract] ++;

 srand48(hseed);
 dir = 3.99 * drand48() + ROUNDING_KLUDGE;

 PtrnOffset = map->steplat * LATSCALE / (maxfract + 1);
 HalfPtrnOffset = PtrnOffset * .5;	/* this used to be .75 */
 PtrnOffset *= 2.0;

 if (maxfract == 0)
  {
  short i = 0, MakeWater = 0, MakeBeach = 0/*, Visible*/;
  double Elev[3];

/* 	PP[0] = target point vertex 0
	PP[1] = target point vertex 1
	PP[2] = target point vertex 2
	PP[3] = sum of view to vertex vectors
	PP[4] = copy of raw vertex 0 for tree shading
*/
  b = 0;

  ZeroCoords(&PP[3]);

  for (; i<3; i++)
   {
   if (polyel[0][i] <= MaxSeaLevel)
    {
    WaveAmp_Compute(&Elev[i], &DP.alt, &polyel[0][i], &polylat[0][i],
		&polylon[0][i], Tsunami, &MakeWater, &MakeBeach,
		(double)frame / (double)(settings.fieldrender + 1));
    } /* if point at or below sea level */
   else
    {
    Elev[i] = polyel[0][i];
    DP.alt = polyel[0][i] * PARC_RNDR_MOTION(14) * .001;
    } /* else */
   DP.lat = polylat[0][i];
   DP.lon = polylon[0][i];
   getdblscrncoords(&polyx[0][i], &polyy[0][i], &polyq[0][i], &PP[i], &PP[3]);
   } /* for i=0... */

/* for use in tree shading */

  memcpy(&PP[4], &PP[0], sizeof (struct coords));

  findposvector(&PP[1], &PP[0]);
  findposvector(&PP[2], &PP[0]);
  SurfaceNormal(&PP[0], &PP[2], &PP[1]);

  if (MakeWater >= 2)
   {
   Data.El[0]  = Elev[0];
   Data.El[1]  = Elev[1];
   Data.El[2]  = Elev[2];
   } /* if */
  else
   {
   Data.El[0]  = polyel[0][0];
   Data.El[1]  = polyel[0][1];
   Data.El[2]  = polyel[0][2];
   } /* else */
  Data.Lat[0] = polylat[0][0];
  Data.Lat[1] = polylat[0][1];
  Data.Lat[2] = polylat[0][2];
  Data.Lon[0] = polylon[0][0];
  Data.Lon[1] = polylon[0][1];
  Data.Lon[2] = polylon[0][2];

  FacePt_Switch(map, &Data, &Face);
   
  if (Tsunami)
   {
   if (MakeWater >= 3)
    MakeWater = 1;
   else if (MakeWater + MakeBeach >= 2)
    MakeWater = 2;
   else
    MakeWater = 0;
   }
  else
   MakeWater = 0;
   
  for (i=0; i<3; i++)
   {
   if (polyx[b][i] < 0.0)
    polyx[b][i] = 0.0;
   else if (polyx[b][i] > wide)
    polyx[b][i] = wide;
   if (polyy[b][i] < -30.0)
    polyy[b][i] = -30.0;
   else if (polyy[b][i] > oshigh)
    polyy[b][i] = oshigh;
   } /* for i=0... */

  FractPoint_Sort(&Data.El[0]);

  xx[0] = polyx[0][0]; yy[0] = polyy[0][0];
  xx[1] = polyx[0][1]; yy[1] = polyy[0][1];
  xx[2] = polyx[0][2]; yy[2] = polyy[0][2];
  el = (Elev[0] + Elev[1] + Elev[2]) / 3.0;
  qqq = (polyq[0][0] + polyq[0][1] + polyq[0][2]) / 3.0;
  facelat = (polylat[0][0] + polylat[0][1] + polylat[0][2]) / 3.0;
  facelong = (polylon[0][0] + polylon[0][1] + polylon[0][2]) / 3.0;
  relel = polyrelel[0][0] + polyrelel[0][1] + polyrelel[0][2];
  cloudcover = (polycld[0][0] + polycld[0][1] + polycld[0][2]) / 3.0;
  slope = Face.slope;
  aspect = Face.aspect;
  diplat = Face.diplat;
  diplong = Face.diplong;
  treerand4 = drand48();
  treerand3 = drand48();
  treerand2 = drand48();
  treerand = drand48();
  Random = -.2 * treerand + .1;

  sunangle = VectorAngle(&PP[0], &SP);
  if (sunangle < 0.0)
   sunangle = 0.0;
  else if (sunangle > 0.0)
   {
   double LonDiff;

   LonDiff = fabs(facelong - PARC_RNDR_MOTION(16));
   while (LonDiff > 180.0)
    {
    LonDiff -= 360.0;
    LonDiff = fabs(LonDiff);
    } /* while */
   if (LonDiff > 85.0)
    {
    if (LonDiff > 105.0)
     LonDiff = 105.0;
    sunangle -= (sunangle * ((LonDiff - 85.0) / 30.0));
    } /* if surface is positioned near 90 from the sun in longitude */
   } /* if surface is lit at all */
  sunfactor = 1.0 - sunangle;
  sunshade = sunfactor * PARC_RNDR_MOTION(22);
  sunshade += ((1.0 - sunshade) * cloudcover);
  sunangle = ACos_Table(sunangle);

  MapTopo(map, win, map->MapAsSFC, MakeWater, 1, &Data.El[0]);

  } /* if no recursion necessary, fractal level 0 */
 else
  {
  b = 1;

  Vtx[maxfract].Lat[0] = polylat[0][0];
  Vtx[maxfract].Lon[0] = polylon[0][0];
  Vtx[maxfract].El[0]  = polyel[0][0];
  Vtx[maxfract].RelEl[0]  = polyrelel[0][0];
  Vtx[maxfract].Cld[0]  = polycld[0][0];
  Vtx[maxfract].Lat[1] = polylat[0][1];
  Vtx[maxfract].Lon[1] = polylon[0][1];
  Vtx[maxfract].El[1]  = polyel[0][1];
  Vtx[maxfract].RelEl[1]  = polyrelel[0][1];
  Vtx[maxfract].Cld[1]  = polycld[0][1];
  Vtx[maxfract].Lat[2] = polylat[0][2];
  Vtx[maxfract].Lon[2] = polylon[0][2];
  Vtx[maxfract].El[2]  = polyel[0][2];
  Vtx[maxfract].RelEl[2]  = polyrelel[0][2];
  Vtx[maxfract].Cld[2]  = polycld[0][2];
  VtxNum = 3;

  if (WhichFace)
   VertexIndexFaceTwo_EdgeSet(map, &Vtx[maxfract], maxfract);
  else
   VertexIndexFaceOne_EdgeSet(map, &Vtx[maxfract], maxfract);

  FractRecurse(win, map, map->MapAsSFC, &Data, &Face, &Vtx[maxfract], CD);

#ifdef JSHKJHDKASHDK
// stuff for testing consistency
  if (StudyVertex == 0 || StudyVertex == 15 || StudyVertex == 207)
   {
   long fract, sum = 0;

   Vtx[maxfract].Edge[0] = 0;
   Vtx[maxfract].Edge[1] = 1;
   Vtx[maxfract].Edge[2] = 2;

   printf("\nBegin Vertex %d\n", StudyVertex);
   for (fract=0; fract<VtxNum; fract++)
    printf("%5d %2d %2d\n", Vtx[maxfract].Use[fract],
	Vtx[maxfract].Pert[fract], Vtx[maxfract].Edge[fract]);

   for (fract=0; fract<VtxNum; fract++)
    {
    if (Vtx[maxfract].Edge[fract] == 5)
     sum++;
    }
   printf("total number of 5's = %d\n", sum);
   } /* if */

  StudyVertex ++;
#endif

#ifdef GJHSGJAHSDGJ
// Stuff for recording vertex info in a special tool file

  if (StudyVertex == 0)
   {
   long fract;
   FILE *fVtx;
   char *Title = "WCSFractalIndex";

   sprintf(str, "WCS:Tools/Fract%d", maxfract);
   fVtx = fopen(str, "wb");
   if (fVtx)
    {
    fwrite((char *)Title, 16, 1, fVtx);
    fwrite((char *)&maxfract, 4, 1, fVtx);
    fwrite((char *)&VtxNum, 4, 1, fVtx);
    for (fract=0; fract<VtxNum; fract++)
     fwrite((char *)&Vtx[maxfract].Use[fract], 4, 1, fVtx);
    for (fract=0; fract<VtxNum; fract++)
     fwrite((char *)&Vtx[maxfract].Pert[fract], 1, 1, fVtx);
    for (fract=0; fract<VtxNum; fract++)
     fwrite((char *)&Vtx[maxfract].Edge[fract], 1, 1, fVtx);
    fclose (fVtx);
    }
   } /* if */
  StudyVertex ++;
#endif
  } /* else use recursion, fractal level > 0 */

} /* FractFace_Render() */

/********************************************************************/

STATIC_FCN void renderface(struct elmapheaderV101 *map, struct Window *win, struct CloudData *CD) // used locally only -> static, AF 23.7.2021
{
 long fract;
 struct FaceData Data;

 if (relelev.map)
  relel = *(relelev.map + map->facept[0]) + *(relelev.map + map->facept[1])
	+ *(relelev.map + map->facept[2]);

 for (b=0; b<3; b++)
  {
  polyx[0][b] = ptx[b];    
  polyy[0][b] = pty[b];  
  polyel[0][b] = map->lmap[map->facept[b]];
  polyq[0][b] = ptqq[b];
  polylat[0][b] = latface[b];
  polylon[0][b] = longface[b];
  polycld[0][b] = CD ? CloudCover_Set(CD, polylat[0][b], polylon[0][b]): 0.0;
  } /* for */
 if (settings.fixfract)
  {
  maxfract = settings.fractal < DBase[OBN].MaxFract ?
	settings.fractal: DBase[OBN].MaxFract;
  if (map->fractal)
   {
   if ((unsigned int)map->fractal[map->fracct] < maxfract)
    maxfract = (unsigned int)map->fractal[map->fracct];
   } /* if fractal map */
  } /* if fixed fractal level */
 else
  {
  fract = max(abs(ptx[0] - ptx[1]), pty[1] - pty[0]);
  fract = max(fract, abs(ptx[0] - ptx[2]));
  fract = max(fract, pty[2] - pty[0]);
  fract = max(fract, abs(ptx[1] - ptx[2]));
  fract = max(fract, pty[2] - pty[1]);
  maxfract=0;
  while (fract > 2 && maxfract < settings.fractal
	&& maxfract < DBase[OBN].MaxFract)
   {
   fract >>= 1;
   maxfract ++;
   } /* while */
  } /* else not fixed fractal level */

 fracount[maxfract] ++;

 srand48(hseed);
 dir = 3.99 * drand48() + ROUNDING_KLUDGE;
 PtrnOffset = map->steplat * LATSCALE / (maxfract + 1);	/* used to not add 1 */
 HalfPtrnOffset = PtrnOffset * .5;	/* this used to be .75 */
 PtrnOffset *= 2.0;

 if (maxfract == 0)
  {
  b = 0;
  PointSort2();
  xx[0] = polyx[0][0]; yy[0] = polyy[0][0];
  xx[1] = polyx[0][1]; yy[1] = polyy[0][1];
  xx[2] = polyx[0][2]; yy[2] = polyy[0][2];
  el = (polyel[0][0] + polyel[0][1] + polyel[0][2]) / 3.0;
  qqq = (polyq[0][0] + polyq[0][1] + polyq[0][2]) / 3.0;
  facelat = (polylat[0][0] + polylat[0][1] + polylat[0][2]) / 3.0;
  facelong = (polylon[0][0] + polylon[0][1] + polylon[0][2]) / 3.0;
  cloudcover = (polycld[0][0] + polycld[0][1] + polycld[0][2]) / 3.0;
  treerand4 = drand48();
  treerand3 = drand48();
  treerand2 = drand48();
  treerand = drand48();
  Random = -.2 * treerand + .1;
  Data.El[0] = polyel[0][0];
  Data.El[1] = polyel[0][1];
  Data.El[2] = polyel[0][2];

  MapTopo(map, win, map->MapAsSFC, 0, 1, &Data.El[0]);
  } /* if no recursion necessary, fractal level 0 */
 else
  {
  b = 1;
  if (settings.perturb)
   {
   dlat = diplat;
   dlong = diplong;
   dslope = slope;
   maxdlat = diplat * 1.5;
   mindlat = diplat * .5;
   if (maxdlat < mindlat) swmem(&maxdlat, &mindlat, 8);
   maxdlat = maxdlat > 1.2 ? 1.5: maxdlat + .3;
   mindlat = mindlat < -1.2 ? -1.5: mindlat - .3;
   maxdlong = diplong * 1.5;
   mindlong = diplong * .5;
   if (maxdlong < mindlong) swmem(&maxdlong, &mindlong, 8);
   maxdlong = maxdlong > 1.2 ? 1.5: maxdlong + .3;
   mindlong = mindlong < -1.2 ? -1.5: mindlong - .3;
   } /* if */

  recurse(map, win, map->MapAsSFC, CD, &Data);
  } /* else use recursion, fractal level > 0 */

} /* renderface() */

/*********************************************************************/

STATIC_FCN short setfaceone(struct elmapheaderV101 *map)
{
 map->facept[0] = map->facect;
 map->facept[1] = map->facect + map->columns;
 map->facept[2] = map->facect + 1;

 return (setface(map));

} /* setfaceone() */

/*********************************************************************/

STATIC_FCN short setfacetwo(struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 map->facept[0] = map->facect + map->columns;
 map->facept[1] = map->facect;		
 map->facept[2] = map->facept[0] - 1;

 return (setface(map));

} /* setfacetwo() */

/*********************************************************************/

STATIC_FCN short setface(struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 short y, j = 0, k = 0, avgX, avgY, offsetY, width;
 float *zbufbase;

 qqq = LARGENUM; 
 for (y=0; y<3; y++)
  {
  ptqq[y] = map->scrnptrq[map->facept[y]];
  if (ptqq[y] < 0.0)
   {
   j = 3;
   break;
   } /* if */
  ptx[y] = map->scrnptrx[map->facept[y]];
  pty[y] = map->scrnptry[map->facept[y]];
  if (ptqq[y] < qqq)
   qqq = ptqq[y];
  if (ptx[y] < 0)
   {
   ptx[y] = 0;
   j += 1;
   }
  else if (ptx[y] > wide)
   {
   ptx[y] = wide;
   j += 1;
   }
  if (pty[y] < -10)
   {
   pty[y] = -10;
   k += 1;
   }
  else if (pty[y] > oshigh)
   {
   pty[y] = oshigh;
   k +=1;
   }
  } /* for y=0... */

 if (j<3 && k<3)
  {
/* I think this is OK now, uncomment this if undrawn polygons appear in SFC's
  if (map->MapAsSFC)
   return (1);
*/
  treehtfact = settings.treefactor / (PARC_RNDR_MOTION(10) * qqq
	* PARC_RNDR_MOTION(11) / 90.0);
  avgY = (pty[0] + pty[1] + pty[2]) / 3.0;
  offsetY = 4.0 * (treehtfact + 2.0);
  if (avgY - offsetY < 0)
   return (1);
  avgX = (ptx[0] + ptx[1] + ptx[2]) / 3.0;
  if (avgX < 0)
   avgX = 0;
  else if (avgX > wide)
   avgX = wide;
  zbufbase = zbuf + avgX;
  width = wide + 1;
  if (qqq < .1 + zbufbase[(avgY - offsetY) * width])
   return (1);
  if (avgY + offsetY > oshigh)
   return (1);
  if (qqq < .1 + zbufbase[(avgY + offsetY) * width])
   return (1);
  zbufbase += (avgY * width); 
  if (qqq < .1 + *(zbufbase))
   return (1);
  if (avgX + offsetY > wide)
   return (1);
  if (qqq < .1 + zbufbase[offsetY])
   return (1);
  if (avgX - offsetY < 0)
   return (1);
  if (qqq < .1 + *(zbufbase - offsetY))
   return (1);
  } /* if */

 return (0);

} /* setface() */

/**********************************************************************/

short CloudShadow_Init(struct elmapheaderV101 *map, struct CloudData *CD)
{
short *CloudPtr, ShadowVal;
long x, y, zip;
float *PlanePtr, ShadowLowCutoff, ShadowHighCutoff, ShadowCutoffRange, Shadow;
double MaxAmp, MinAmp, CloudMinAmp, CloudRangeAmp, Density;
struct CloudLayer *CL, *CM, *CE;

/* find ampl. range */

 zip = 0;
 MaxAmp = -10000.0;
 MinAmp = 10000.0;
 for (y=0; y<CD->Rows; y++)
  {
  for (x=0; x<CD->Cols; x++, zip++)
   {
   if (CD->CloudPlane[zip] > MaxAmp)
    MaxAmp = CD->CloudPlane[zip];
   if (CD->CloudPlane[zip] < MinAmp)
    MinAmp = CD->CloudPlane[zip];
   } /* for y=... */
  } /* for x=... */

/* determine which layer has maximum coverage */

 CM = CE = CD->Layer;
 CL = CM->Next;
 while (CL)
  {
  if (CL->Covg > CM->Covg)
   CM = CL;
  if (CL->Dens > CE->Dens)
   CE = CL;
  CL = CL->Next;
  } /* while */

 CloudMinAmp = MinAmp + (MaxAmp - MinAmp) *
	(1.0 - (CD->Coverage * CM->Covg / 10000.0));
 CloudRangeAmp = MaxAmp - CloudMinAmp;
 Density = CD->Density * CE->Dens / 10000.0;

/* convert cloudmap into 8 bit w/ DEM orientation */

 switch (CD->CloudType)
  {
  case 0:
   {
   ShadowLowCutoff = .7;
   ShadowHighCutoff = 1.0;
   ShadowCutoffRange = .3;
   ShadowVal = 100;
   break;
   } /*  */
  case 1:
   {
   ShadowLowCutoff = .5;
   ShadowHighCutoff = .75;
   ShadowCutoffRange = .25;
   ShadowVal = 150;
   break;
   } /*  */
  case 2:
   {
   ShadowLowCutoff = .37;
   ShadowHighCutoff = .52;
   ShadowCutoffRange = .15;
   ShadowVal = 200;
   break;
   } /*  */
  case 3:
   {
   ShadowLowCutoff = .3;
   ShadowHighCutoff = .4;
   ShadowCutoffRange = .1;
   ShadowVal = 255;
   break;
   } /*  */
  } /* switch */

 MinAmp = 10000.0;
 MaxAmp = -10000.0;
 CloudPtr = map->map;
 for (x=0; x<CD->Cols; x++)
  {
  PlanePtr = CD->CloudPlane + (CD->Rows - 1) * CD->Cols + x;
  for (y=CD->Rows-1; y>=0; y--, CloudPtr++, PlanePtr-=CD->Cols)
   {
   if (*PlanePtr > CloudMinAmp)
    {
/* This creates feathered edges - not very realistic
    *CloudPtr = (Density * (*PlanePtr - CloudMinAmp)
	/ CloudRangeAmp) * 255.0;
    if (*CloudPtr > 255)
     *CloudPtr = 255;
    else if (*CloudPtr < 0)
     *CloudPtr = 0;
*/
    Shadow = (Density * (*PlanePtr - CloudMinAmp) / CloudRangeAmp);
    if (Shadow > ShadowLowCutoff)
     {
     if (Shadow >= ShadowHighCutoff)
      *CloudPtr = ShadowVal;
     else
      *CloudPtr = ShadowVal * (Shadow - ShadowLowCutoff) / ShadowCutoffRange;
     } /* if */
    else
     *CloudPtr = 0;
    } /* if */
   else
    *CloudPtr = 0;
   } /* for y=... */
  } /* for x=... */

return (0);

} /* CloudShadow_Init() */

/**********************************************************************/

short MapCloudObject(struct elmapheaderV101 *map, struct CloudData *CD,
	struct Window *win)
{
 short j, error = 0, *CloudPtr, Diff;
/* UBYTE *CldMap;*/
 long x, y, zip;
 double MinAmp, MaxAmp, h, d, sunshade;
 struct BusyWindow *BWCL;
 struct CloudLayer *CL;

/* CldMap = (UBYTE *)get_Memory(CD->Map.size / 2, MEMF_ANY);
 if (! CldMap)
  {
  if (! User_Message("Render Module: Clouds",
	"Out of memory initializing Cloud Map!\nContinue without clouds or cancel rendering?",
	"Continue|Cancel", "oc"))
   error = 1;
  goto EndCloud;
  } / if no memory
*/
 CL = CD->Layer;

 BWCL = BusyWin_New("Clouds", CD->NumLayers, 1, MakeID('B','W','C','L'));

/* find ampl. range */

 zip = 0;
 MaxAmp = -10000.0;
 MinAmp = 10000.0;
 for (y=0; y<CD->Rows; y++)
  {
  for (x=0; x<CD->Cols; x++, zip++)
   {
   if (CD->CloudPlane[zip] > MaxAmp)
    MaxAmp = CD->CloudPlane[zip];
   if (CD->CloudPlane[zip] < MinAmp)
    MinAmp = CD->CloudPlane[zip];
   } /* for y=... */
  } /* for x=... */

 CloudPtr = map->map;
 for (x=0, facelong=map->lolong; x<=map->rows; x++, facelong-=map->steplong)
  {
  for (y=0, facelat=map->lolat; y<map->columns; y++, CloudPtr++,
	facelat+=map->steplat)
   {
   h = sunlong - facelong * PiOver180;
   if (h > Pi) h -= TwoPi;
   else if (h < -Pi) h += TwoPi;
   d = sunlat - facelat * PiOver180;
   if (d > Pi) d -= TwoPi;
   else if (d < -Pi) d += TwoPi;
   sunshade = sqrt(h * h + d * d) * .7071;
   if (sunshade > HalfPi) sunshade = HalfPi;
   sunshade = (1.0 - cos(sunshade)) * PARC_RNDR_MOTION(22);

   Diff = 255.0 - 255.0 * pow(sunshade, 5.0);

   *CloudPtr = (Diff << 8);
   } /* for y=... */
  } /* for x=... */

 j = 0;

 while (CL)
  {
  if (CL->Alt * PARC_RNDR_MOTION(14) >= PARC_RNDR_MOTION(0))
   break;
/* render this layer */
  if ((error = MapCloudLayer(map, CD, CL, CL->Next, MinAmp, MaxAmp, j, win/*,
	CldMap*/)))
   break;
  CL = CL->Next;
  j ++;
  BusyWin_Update(BWCL, j);
  } /* while */

 if (! error)
  {
  while (CL->Next)
   CL = CL->Next;
  
  while (CL)
   {
   if (CL->Alt * PARC_RNDR_MOTION(14) < PARC_RNDR_MOTION(0))
    break;
/* render this layer */
   if ((error = MapCloudLayer(map, CD, CL, CL->Prev, MinAmp, MaxAmp, j, win/*,
	CldMap*/)))
    break;
   CL = CL->Prev;
   j ++;
   BusyWin_Update(BWCL, j);
   } /* while */
  } /* if no error */

 if (BWCL) BusyWin_Del(BWCL);

//EndCloud:
/*
 if (CldMap)
  free_Memory(CldMap, CD->Map.size / 2);
*/

 return (error);

} /* MapCloudObject() */

/*********************************************************************/

STATIC_FCN short MapCloudLayer(struct elmapheaderV101 *map,
	struct CloudData *CD,  struct CloudLayer *CL, struct CloudLayer *More,
	double MinAmp, double MaxAmp, short j, struct Window *win/*, UBYTE *CldMap*/) // used locally only -> static, AF 23.7.2021
{
char BusyWinStr[16];
short i, CloudVal[3], IllumVal[3], ColDif, *CloudPtr, error = 0, Cld,
	Intensity, Illumination;
long x, y, RowLon, ColLat, stepct, zip, Elev;
float *PlanePtr;
double CloudMinAmp, CloudRangeAmp, Density, ReducedLon, fade;
struct ColorComponents CC, CC1;
struct BusyWindow *BWDE;
/*
UBYTE *CldPtr;
short Diff, NewDiff, FirstDiff, Sign;
double h, d, sunshade, Azim, Dip;
*/
 sprintf(BusyWinStr, "Cloud %d", j);
 BWDE = BusyWin_New(BusyWinStr, (map->rows + 1) * 2, 0, MakeID('B','W','D','E'));

 CC.Red = PARC_RNDR_COLOR(21, 0) * CL->Illum;
 CC.Grn = PARC_RNDR_COLOR(21, 1) * CL->Illum;
 CC.Blu = PARC_RNDR_COLOR(21, 2) * CL->Illum;

/* convert cloudmap into 8 bit w/ DEM orientation */

/* create density map in low byte, illumination map in high byte */

 CloudMinAmp = MinAmp + (MaxAmp - MinAmp) *
	(1.0 - (CD->Coverage * CL->Covg / 10000.0));
 CloudRangeAmp = MaxAmp - CloudMinAmp;
 Density = CD->Density * CL->Dens / 10000.0;

 CloudPtr = map->map;
 for (x=0; x<CD->Cols; x++)
  {
  PlanePtr = CD->CloudPlane + (CD->Rows - 1) * CD->Cols + x;
  for (y=CD->Rows-1; y>=0; y--, CloudPtr++, PlanePtr-=CD->Cols)
   {
   *CloudPtr &= 0xff00;
   
   if (*PlanePtr > CloudMinAmp)
    {
    Cld = (Density * (*PlanePtr - CloudMinAmp)
	/ CloudRangeAmp) * 255.0;
    if (Cld > 255)
     Cld = 255;
    else if (Cld < 0)
     Cld = 0;
    }
   else
    Cld = 0;
   *CloudPtr += Cld;
   } /* for y=... */
  } /* for x=... */

#ifdef GHGJHGJHGJG
 if (CH)
  {
  CloudMinAmp = MinAmp + (MaxAmp - MinAmp) *
	(1.0 - (CD->Coverage * CH->Covg / 10000.0));
  CloudRangeAmp = MaxAmp - CloudMinAmp;
  Density = CD->Density * CH->Dens / 10000.0;

  CldPtr = CldMap;
  for (x=0; x<CD->Cols; x++)
   {
   PlanePtr = CD->CloudPlane + (CD->Rows - 1) * CD->Cols + x;
   for (y=CD->Rows-1; y>=0; y--, CldPtr++, PlanePtr-=CD->Cols)
    {
    Cld = (Density * (*PlanePtr - CloudMinAmp)
	/ CloudRangeAmp) * 255.0;
    if (Cld > 255)
     Cld = 255;
    else if (Cld < 0)
     Cld = 0;
    *CldPtr = Cld;
    } /* for y=... */
   } /* for x=... */
  } /* if CH */

 CloudMinAmp = MinAmp + (MaxAmp - MinAmp) *
	(1.0 - (CD->Coverage * CL->Covg / 10000.0));
 CloudRangeAmp = MaxAmp - CloudMinAmp;
 Density = CD->Density * CL->Dens / 10000.0;

 CloudPtr = map->map;
 for (x=0; x<CD->Cols; x++)
  {
  PlanePtr = CD->CloudPlane + (CD->Rows - 1) * CD->Cols + x;
  for (y=CD->Rows-1; y>=0; y--, CloudPtr++, PlanePtr-=CD->Cols)
   {
   Cld = (Density * (*PlanePtr - CloudMinAmp)
	/ CloudRangeAmp) * 255.0;
   if (Cld > 255)
    Cld = 255;
   else if (Cld < 0)
    Cld = 0;
   *CloudPtr = Cld;
   } /* for y=... */
  } /* for x=... */
 
 CloudPtr = map->map;
 CldPtr = CldMap;
 for (x=0, facelong=map->lolong; x<=map->rows; x++, facelong-=map->steplong)
  {
  for (y=0, facelat=map->lolat; y<map->columns; y++, CloudPtr++, CldPtr++,
	facelat+=map->steplat)
   {
   if (CH)
    {
    Diff = (*CloudPtr - (unsigned int)*CldPtr);
    if (Diff > 0)
     Sign = -1;
    else
     Sign = 1;
    Diff = FirstDiff = iabs(Diff);

    Azim = 0.0;
    if (y < map->columns - 1)
     {
     if (x > 0)
      {
      NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr + 1 - map->columns));
      if (NewDiff < Diff)
       {
       Azim = 1.75 * Pi;
       Diff = NewDiff;
       }
      }
     NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr + 1));
     if (NewDiff < Diff)
      {
      Azim = 0.0;
      Diff = NewDiff;
      }
     if (x < map->rows)
      {
      NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr + 1 + map->columns));
      if (NewDiff < Diff)
       {
       Azim = .25 * Pi;
       Diff = NewDiff;
       }
      }
     }
    if (x < map->rows)
     {
     NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr + map->columns));
     if (NewDiff < Diff)
      {
      Azim = .5 * Pi;
      Diff = NewDiff;
      }
     }
    if (y > 0)
     {
     if (x < map->rows)
      {
      NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr - 1 + map->columns));
      if (NewDiff < Diff)
       {
       Azim = .75 * Pi;
       Diff = NewDiff;
       }
      }
     NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr - 1));
     if (NewDiff < Diff)
      {
      Azim = Pi;
      Diff = NewDiff;
      }
     if (x > 0)
      {
      NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr - 1 - map->columns));
      if (NewDiff < Diff)
       {
       Azim = 1.25 * Pi;
       Diff = NewDiff;
       }
      }
     }
    if (x > 0)
     {
     NewDiff = iabs(*CloudPtr - (unsigned int)*(CldPtr - map->columns));
     if (NewDiff < Diff)
      {
      Azim = 1.5 * Pi;
      Diff = NewDiff;
      }
     }

    Dip = HalfPi - HalfPi * FirstDiff / 255.0;

    if (Sign < 0)
     Azim += Pi;
    if (Azim > TwoPi)
     Azim -= TwoPi;
    diplat = Dip * sin(Pi - Azim);
    diplong = Dip * sin(HalfPi - Azim);
    if (Sign < 0)
     diplong += Pi;
    if (diplong > TwoPi)
     diplong -= TwoPi;
    } /* if CH */
   else
    {
    diplat = diplong = 0.0;
    } /* else topmost layer */

   h = sunlong - facelong * PiOver180 - diplong;
   if (h > Pi) h -= TwoPi;
   else if (h < -Pi) h += TwoPi;
   d = sunlat - facelat * PiOver180 - diplat;
   if (d > Pi) d -= TwoPi;
   else if (d < -Pi) d += TwoPi;
   sunshade = sqrt(h * h + d * d) * .7071;
   if (sunshade > HalfPi) sunshade = HalfPi;
   sunshade = (1.0 - cos(sunshade)) * PARC_RNDR_MOTION(22);

   Diff = 255.0 - 255.0 * pow(sunshade, 5.0);

   *CloudPtr += (Diff << 8);
   } /* for y=... */
  } /* for x=... */
#endif
/* clear byte maps */

 memset(bytemap, 0, bmapsize * 2);

/* compute screen coordinates */

 map->facect = 0;
 if (settings.flatteneco)
  {
  DP.alt = CL->Alt * 1000.0;
  Elev = DP.alt;
  DP.alt = (DP.alt + (PARC_RNDR_MOTION(13) - DP.alt) * PARC_RNDR_MOTION(12)) * .001;
  }
 else
  {
  Elev = CL->Alt * 1000.0;
  DP.alt = CL->Alt;
  }
 DP.alt *= PARC_RNDR_MOTION(14);
 for (map->Lr=0; map->Lr<=map->rows; map->Lr++)
  {
  for (map->Lc=0; map->Lc<map->columns; map->Lc++)
   {
   DP.lat = map->lolat + map->Lc * map->steplat;
   DP.lon = map->lolong - map->Lr * map->steplong;
   getscrncoords(map->scrnptrx + map->facect, map->scrnptry + map->facect,
		map->scrnptrq + map->facect, NULL, NULL);
   map->facect ++;
   } /* for map->Lc=0... */
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, map->Lr + 1);
  } /* for map->Lr=0... */

 if (error)
  goto EndMap;

/* draw cloud object into bytemap */

 ReducedLon = PARC_RNDR_MOTION(2);
 while (ReducedLon >= 360.0)
  ReducedLon -= 360.0;
 while (ReducedLon < 0.0)
  ReducedLon += 360.0;
 if (abs(map->lolong - ReducedLon) < 180.0)
  RowLon = /* -1.0 +*/ (map->lolong - ReducedLon) / map->steplong;
 else
  RowLon = /* -1.0 +*/ (ReducedLon - map->lolong) / map->steplong;

 ColLat = /* -1.0 +*/ (PARC_RNDR_MOTION(1) - map->lolat) / map->steplat;
 if (RowLon < 0) RowLon = 0;
 else if (RowLon > map->rows) RowLon = map->rows;
 if (ColLat < 0) ColLat = 0;
 else if (ColLat > map->columns - 1) ColLat = map->columns - 1;

 stepct = map->rows + 1;

 for (map->Lr = RowLon; map->Lr < map->rows; map->Lr++)
  {
  map->facect = map->Lr * map->columns + ColLat;
  for (map->Lc = ColLat; map->Lc < map->columns - 1;)
   {
   if (setcloudfaceone(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   map->Lc ++;
   map->facect ++;
   if (setcloudfacetwo(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   } /* for map->Lc=... forward columns */

  map->facect = map->Lr * map->columns + ColLat;
  for (map->Lc = ColLat; map->Lc > 0;)
   {
   if (setcloudfacetwo(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   map->Lc --;
   map->facect --;
   if (setcloudfaceone(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   } /* for map->Lc=... reverse columns */
  stepct ++;
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, stepct);
  } /* for map->Lr=... forward rows */

 if (error)
  goto EndMap;

 for (map->Lr = RowLon - 1; map->Lr >= 0; map->Lr--)
  {
  map->facect = map->Lr * map->columns + ColLat;
  for (map->Lc = ColLat; map->Lc < map->columns - 1;)
   {
   if (setcloudfaceone(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   map->Lc ++;
   map->facect ++;
   if (setcloudfacetwo(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   } /* for map->Lc=... forward columns */

  map->facect = map->Lr * map->columns + ColLat;
  for (map->Lc = ColLat; map->Lc > 0;)
   {
   if (setcloudfacetwo(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   map->Lc --;
   map->facect --;
   if (setcloudfaceone(map) > 0 || ! More)
    {
    if (setface(map))
     {
     if (qqq > qmin)
      {
      for (i=0; i<3; i++)
       {
       CloudVal[i] = (map->map[map->facept[i]] & 0xff);
       IllumVal[i] = ((map->map[map->facept[i]] & 0xff00) >> 8);
       } /* for i=0... */
      rendercloud(win, CloudVal, IllumVal, Elev);
      } /* if */
     } /* if */
    } /* if */
   } /* for map->Lc=... reverse columns */
  stepct ++;
  if (CheckInput_ID() == ID_BW_CLOSE)
   {
   error = 1;
   break;
   } /* if user abort */
  BusyWin_Update(BWDE, stepct);
  } /* for map->Lr=... reverse rows */

/* merge bytemap with bitmaps */
 if (! error )
  { 
  for (zip=0, x=0, y=0; zip<bmapsize; zip++, x++)
   {
   if (x == settings.scrnwidth)
    {
    x = 0;
    y ++;
    }
   Intensity = (bytemap[zip] & 0xff);
   if (Intensity)
    {
    fade = (zbuf[zip] - PARC_RNDR_MOTION(20)) / PARC_RNDR_MOTION(21);
    if (fade > 1.0)
     fade = 1.0;
    else if (fade < 0.0)
     fade = 0.0;
    Illumination = ((bytemap[zip] & 0xff00) >> 8);
    CC1.Red = (Illumination * CC.Red) / 255;
    CC1.Grn = (Illumination * CC.Grn) / 255;
    CC1.Blu = (Illumination * CC.Blu) / 255;
    CC1.Red += (PARC_RNDR_COLOR(2, 0) - CC1.Red) * fade;
    CC1.Grn += (PARC_RNDR_COLOR(2, 1) - CC1.Grn) * fade;
    CC1.Blu += (PARC_RNDR_COLOR(2, 2) - CC1.Blu) * fade;
    CC1.Red *= redsun;
    CC1.Grn *= greensun;
    CC1.Blu *= bluesun;
    if (CC1.Red > 255) CC1.Red = 255;
    else if (CC1.Red < 0) CC1.Red = 0;
    if (CC1.Grn > 255) CC1.Grn = 255;
    else if (CC1.Grn < 0) CC1.Grn = 0;
    if (CC1.Blu > 255) CC1.Blu = 255;
    else if (CC1.Blu < 0) CC1.Blu = 0;
    
    ColDif = 255 - Intensity;
    *(bitmap[0] + zip) = (CC1.Red * Intensity
	 + *(bitmap[0] + zip) * ColDif) / 255;
    *(bitmap[1] + zip) = (CC1.Grn * Intensity
	 + *(bitmap[1] + zip) * ColDif) / 255;
    *(bitmap[2] + zip) = (CC1.Blu * Intensity
	 + *(bitmap[2] + zip) * ColDif) / 255;

    if (render & 0x10)
     {
     if (render & 0x01)
      {
      ScreenPixelPlot(win, bitmap, x + drawoffsetX, y + drawoffsetY, zip);
      } /* if bitmaps */
     } /* if bitmaps */
    } /* if render to screen */
   } /* for zip=0... */
  } /* if no error */

EndMap:

 if (BWDE) BusyWin_Del(BWDE);

 return (error);

} /* MapCloudLayer() */

/*********************************************************************/

STATIC_FCN short setcloudfaceone(struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 map->facept[0] = map->facect;
 map->facept[1] = map->facect + map->columns;
 map->facept[2] = map->facect + 1;

 return ((short)((*(map->map + map->facept[0]) & 0xff) + 
	(*(map->map + map->facept[1]) & 0xff) +
	(*(map->map + map->facept[2]) & 0xff)));

} /* setcloudfaceone() */

/*********************************************************************/

STATIC_FCN short setcloudfacetwo(struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 map->facept[0] = map->facect + map->columns;
 map->facept[1] = map->facect;		
 map->facept[2] = map->facept[0] - 1;

 return ((short)((*(map->map + map->facept[0]) & 0xff) + 
	(*(map->map + map->facept[1]) & 0xff) +
	(*(map->map + map->facept[2]) & 0xff)));

} /* setcloudfacetwo() */

/************************************************************************/

STATIC_FCN void rendercloud(struct Window *win, short *CloudVal, short *IllumVal,
	short Elev) // used locally only -> static, AF 23.7.2021
{

 CloudPointSort(CloudVal, IllumVal);

 xx[0] = ptx[0];
 yy[0] = pty[0];
 xx[1] = ptx[1];
 yy[1] = pty[1];
 xx[2] = ptx[2];
 yy[2] = pty[2];

 MapCloud(win, CloudVal, IllumVal, Elev);

} /* rendercloud() */

/*********************************************************************/

STATIC_FCN void CloudPointSort(short *CloudVal, short *IllumVal) // used locally only -> static, AF 23.7.2021
{

 if (pty[1] < pty[0])
  {
  swmem(&pty[0], &pty[1], 8);
  swmem(&ptx[0], &ptx[1], 8);
  swmem(&ptqq[0], &ptqq[1], 8);
  swmem(&CloudVal[0], &CloudVal[1], sizeof (short));
  swmem(&IllumVal[0], &IllumVal[1], sizeof (short));
  } /* if */
 if (pty[2] < pty[1])
  {
  swmem(&pty[1], &pty[2], 8);
  swmem(&ptx[1], &ptx[2], 8);
  swmem(&ptqq[1], &ptqq[2], 8);
  swmem(&CloudVal[1], &CloudVal[2], sizeof (short));
  swmem(&IllumVal[1], &IllumVal[2], sizeof (short));
  if (pty[1] < pty[0])
   {
   swmem(&pty[0], &pty[1], 8);
   swmem(&ptx[0], &ptx[1], 8);
   swmem(&ptqq[0], &ptqq[1], 8);
   swmem(&CloudVal[0], &CloudVal[1], sizeof (short));
   swmem(&IllumVal[0], &IllumVal[1], sizeof (short));
   } /* if */
  } /* if */

} /* CloudPointSort() */

/*********************************************************************/
/* Includes reading Edge values */

STATIC_FCN short VertexIndex_New(struct VertexIndex *Vtx, long MaxFract)
{
char Title[16];
short error = 0;
long Depth, Vertices;
FILE *fVtx;

if (MaxFract == 0)		/* don't need an index */
{
    return (1);
}

 sprintf(str, "Tools/Fract%ld", MaxFract);
 if ((fVtx = fopen(str, "rb")))
  {
  fread((char *)Title, 16, 1, fVtx);
  if (! strcmp(Title, "WCSFractalIndex"))
   {
   fread((char *)&Depth, sizeof (long), 1, fVtx);
   if (Depth == MaxFract)
    {
    fread((char *)&Vertices, sizeof (long), 1, fVtx);
    Vtx->PSize = Vertices;
    if ((Vtx->Use = (long *)get_Memory(Vertices * sizeof (long), MEMF_ANY)) != NULL)
     {
     if ((Vtx->Pert = (UBYTE *)get_Memory(Vertices * sizeof (char), MEMF_ANY)) != NULL)
      {
      if ((Vtx->Edge = (UBYTE *)get_Memory(Vertices * sizeof (char), MEMF_ANY)) != NULL)
       {
       if ((fread((char *)Vtx->Use, Vertices * sizeof (long), 1, fVtx)) == 1)
        {
        if ((fread((char *)Vtx->Pert, Vertices, 1, fVtx)) == 1)
         {
         if ((fread((char *)Vtx->Edge, Vertices, 1, fVtx)) == 1)
          {
          if ((Vtx->El = (double *)get_Memory(Vertices * sizeof (double), MEMF_ANY)) != NULL)
           {
           if ((Vtx->Lat = (double *)get_Memory(Vertices * sizeof (double), MEMF_ANY)) != NULL)
            {
            if ((Vtx->Lon = (double *)get_Memory(Vertices * sizeof (double), MEMF_ANY)) != NULL)
             {
             if ((Vtx->RelEl = (double *)get_Memory(Vertices * sizeof (double), MEMF_ANY)) != NULL)
              {
              if ((Vtx->Cld = (double *)get_Memory(Vertices * sizeof (double), MEMF_ANY)) == NULL)
               error = 1;
	      } /* if Cloudcover memory */
             else
              error = 1;
             } /* if Lon memory */
            else
             error = 1;
            } /* if Lat memory */
           else
            error = 1;
           } /* if El memory */
          else
           error = 1;
          } /* if Edge values read */
         else
          error = 1;
         } /* if Perturbation values read */
        else
         error = 1;
        } /* if Use values read */
       else
        error = 1;
       } /* if Edge memory */
      else
       error = 1;
      } /* if Perturbation memory */
     else
      error = 1;
     } /* if Use memory */
    else
     error = 1;
    } /* if correct fractal depth */
   else
    error = 1;
   } /* if correct file type */
  else
   error = 1;
  fclose (fVtx);
  } /* if file opened */
 else
  error = 1;

 if (error)
  return (0);
 return (1);

} /* VertexIndex_New() */

/*********************************************************************/

STATIC_FCN void VertexIndex_Del(struct VertexIndex *Vtx) // used locally only -> static, AF 19.7.2021
{

 if (Vtx)
  {
  if (Vtx->El)
   free_Memory(Vtx->El, Vtx->PSize * sizeof (double));
  if (Vtx->Lat)
   free_Memory(Vtx->Lat, Vtx->PSize * sizeof (double));
  if (Vtx->Lon)
   free_Memory(Vtx->Lon, Vtx->PSize * sizeof (double));
  if (Vtx->RelEl)
   free_Memory(Vtx->RelEl, Vtx->PSize * sizeof (double));
  if (Vtx->Cld)
   free_Memory(Vtx->Cld, Vtx->PSize * sizeof (double));
  if (Vtx->Use)
   free_Memory(Vtx->Use, Vtx->PSize * sizeof (long));
  if (Vtx->Pert)
   free_Memory(Vtx->Pert, Vtx->PSize * sizeof (char));
  if (Vtx->Edge)
   free_Memory(Vtx->Edge, Vtx->PSize * sizeof (char));
  Vtx->El = Vtx->Lat = Vtx->Lon = Vtx->RelEl = NULL;
  Vtx->Use = NULL;
  Vtx->Pert = Vtx->Edge = NULL;
  } /* if */

} /* VertexIndex_Del() */

/*********************************************************************/

short FractalDepth_Preset(void)
{
char FrameStr[16], DEMstr[16], filename[256], FractalFile[32], FractalPath[256];
short i, OrigOBN, success = 1, FrameInt, FirstFrame, LastFrame, objectcount,
	objectlimit, OpenOK, error = 0, MaxFract, MaxSize, MapAsSFC, UnderWater;
long j, poly, Lr, Lc, FractalMapSize, MaxCol, stepct;
float dummy;
double ptelev;
struct elmapheaderV101 map;
struct BusyWindow *BWAN, *BWIM, *BWDE = NULL;
struct DirList *DLItem;
struct coords PP[4];
BYTE *FractalMap = NULL;
FILE *fFrd;
 
 OrigOBN = OBN;

 if (BuildKeyTable())
  {
/* determine active frame range to explore - camera, focus, view arc, scale,
	datum, vert exag, flattening, center X, center Y are the critical
	parameters */

  FirstFrame = LastFrame = -1;
  for (i=0; i<ParHdr.KeyFrames; i++)
   {
   if (KF[i].MoKey.Group == 0)
    {
    if (KF[i].MoKey.Item <= 14)
     {
     if (FirstFrame < 0)
      FirstFrame = KF[i].MoKey.KeyFrame;
     if (KF[i].MoKey.KeyFrame > LastFrame)
      LastFrame = KF[i].MoKey.KeyFrame;
     } /* if one of the critical parameters */
    } /* if motion group key frame */
   } /* for i=0... */
  if (FirstFrame < 0)
   FirstFrame = LastFrame = 1;

/* determine frame interval from user */

  sprintf(str, "%d", 1);
  if (GetInputString("Enter the maximum pixel size for a polygon. The smaller the number the longer image rendering will take!",
 	"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", str))
   {
   MaxSize = 2.0 * atof(str);

   sprintf(str, "%d", FirstFrame);
   if (GetInputString("Enter the first frame to scan.",
 	"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", str))
    {
    FirstFrame = atoi(str);

    sprintf(str, "%d", LastFrame);
    if (GetInputString("Enter the last frame to scan.",
 	"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", str))
     {
     LastFrame = atoi(str);

     sprintf(str, "%d", 5);
     if (GetInputString("Enter the frame interval to scan. The smaller the number the longer this process will take!",
 	"abcdefghijklmnopqrstuvwxyz-:;*/?`#%", str))
      {
      FrameInt = atoi(str);
      if (FrameInt <= 0)
       FrameInt = 1;

      initmopar();
      initpar();
      high = settings.scrnheight - 1;
      wide = settings.scrnwidth - 1;

/* delete all the old files */

      for (OBN=0; OBN<NoOfObjects; OBN++)
       {
       if ((! strcmp(DBase[OBN].Special, "TOP") ||
		! strcmp(DBase[OBN].Special, "SFC")) && (DBase[OBN].Flags & 2))
        {
        strmfp(filename, dirname, DBase[OBN].Name);
        strcat(filename, ".frd");
        if ((fFrd = fopen(filename, "rb")) != NULL)
         {
         fclose(fFrd);
         remove(filename);
         break;
         } /* else open succeeds */
	} /* if */
       } /* for OBN=0... */

      if ((BWAN = BusyWin_New("Animation", LastFrame-FirstFrame+FrameInt, 1, MakeID('B','W','A','N'))))
       {
       for (frame=FirstFrame; frame<=LastFrame; frame+=FrameInt)
        {
        setvalues();
        ralt = EARTHRAD + PARC_RNDR_MOTION(0);
        qmax = sqrt(ralt * ralt - EARTHRAD * EARTHRAD);
        horscale = (PARC_RNDR_MOTION(10) * PROJPLANE * tan(.5 * PARC_RNDR_MOTION(11) *
   	     PiOver180) / 3.5469) / HORSCALEFACTOR;
        vertscale = horscale * settings.picaspect;
        cosviewlat = cos(PARC_RNDR_MOTION(1) * PiOver180);
        setview();
        CenterX = PARC_RNDR_MOTION(6);
        CenterY = PARC_RNDR_MOTION(7); 
        autoactivate();
        objectlimit = RenderObjects;

        sprintf(FrameStr, "Frame %d/%d", frame, LastFrame);
        if ((BWIM = BusyWin_New(FrameStr, objectlimit, 1, MakeID('B','W','I','M'))))
         {
         for (objectcount=0; objectcount<objectlimit; objectcount++)
          {
          OBN = RenderList[objectcount][0];
          if ((! strcmp(DBase[OBN].Special, "TOP") ||
		! strcmp(DBase[OBN].Special, "SFC")) && (DBase[OBN].Flags & 2))
           {
RepeatLoad:
           OpenOK = 0;

           DLItem = DL;
           while (DLItem)
            {
            strmfp(filename, DLItem->Name, DBase[OBN].Name);
            strcat(filename, ".elev");
            if (readDEM(filename, &map) != 0)
             {
             DLItem = DLItem->Next;
             } /* if open/read fails */
            else
             {
             OpenOK = 1;
             strcpy(FractalPath, DLItem->Name);
             strcpy(FractalFile, DBase[OBN].Name);
             break;
             } /* else open succeeds */
            } /* while */

           if (! OpenOK)
            {
            error = 1;
            Log(ERR_OPEN_FAIL, (CONST_STRPTR)DBase[OBN].Name);
            break;
            } /* if file not found */

           MapAsSFC = (settings.mapassfc || ! strcmp(DBase[OBN].Special, "SFC"))
		? 1: 0;
            
           FractalMapSize = map.rows * (map.columns - 1) * 2;
           FractalMap  = (BYTE *)get_Memory (FractalMapSize, MEMF_ANY);
           map.scrnptrx = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
           map.scrnptry = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
           map.scrnptrq = (float *)get_Memory (map.scrnptrsize, MEMF_ANY);
           if (! map.scrnptrx || ! map.scrnptry || ! map.scrnptrq || ! FractalMap)
            {
            sprintf(str, "Out of memory reading map %s!", DBase[OBN].Name);
            if (User_Message_Def((CONST_STRPTR)"Render Module: Topo", (CONST_STRPTR)str, (CONST_STRPTR)"Retry|Cancel", (CONST_STRPTR)"rc", 1))
             {
             error = -1;
             goto MapCleanup;
             } /* if try again */
            error = 1;
            goto MapCleanup;
            } /* if */
           strcat(FractalFile, ".frd");
           strmfp(filename, dirname, FractalFile);
           if ((fFrd = fopen(filename, "rb")))
            {
            if (fread(FractalMap, FractalMapSize, 1, fFrd) != 1)
             {
             error = 1;
             Log(ERR_READ_FAIL, (CONST_STRPTR)FractalFile);
             fclose(fFrd);
             goto MapCleanup;
             } /* if read error */
            fclose(fFrd);
	    } /* if file exists */
           else
            memset(FractalMap, -1, FractalMapSize);

           sprintf(DEMstr, "DEM %d/%d", objectcount + 1, objectlimit);
           BWDE = BusyWin_New(DEMstr, (map.rows + 1) * 2, 0, MakeID('B','W','D','E'));

/* compute screen coordinates */
           for (j=0, Lr=0; Lr<=map.rows; Lr++)
            {
            for (Lc=0; Lc<map.columns; Lc++, j++)
             {
             ptelev = *(map.map + j) * map.elscale / ELSCALE_METERS;
             ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
             if (ptelev <= SeaLevel && ! MapAsSFC)
              ptelev = SeaLevel;
             DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
             DP.lat = map.lolat + Lc * map.steplat;
             DP.lon = map.lolong - Lr * map.steplong;
             getscrncoords(map.scrnptrx + j, map.scrnptry + j,
		map.scrnptrq + j, NULL, NULL);
             } /* for Lc=0... */
            if (CheckInput_ID() == ID_BW_CLOSE)
             {
             error = 1;
             break;
             } /* if user abort */
            if (BWDE)
             BusyWin_Update(BWDE, Lr + 1);
            } /* for Lr=0... */

           if (error)
            goto MapCleanup;

           stepct = map.rows + 1;
           MaxCol = map.columns - 1;
           for (map.facect=0, poly=0, Lr=0; Lr<map.rows; Lr++, map.facect++, stepct++)
            {
            for (Lc=0; Lc<MaxCol; poly++, UnderWater=0)
             {
             if (setquickfaceone(&map, Lr, Lc))
              {
              PP[3].x = PP[3].y = PP[3].z = PP[3].q = 0;
              for (i=0; i<3; i++)
               {
               ptelev = *(map.map + map.facept[i]) * map.elscale / ELSCALE_METERS;
               ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
               if (ptelev <= SeaLevel && ! MapAsSFC)
                {
                UnderWater = 1;
                ptelev = SeaLevel;
		}
               DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
               DP.lat = latface[i];
               DP.lon = longface[i];
               getscrncoords(&dummy, &dummy, &dummy, &PP[i], &PP[3]);
	       }
              if (! UnderWater)
               {
               findposvector(&PP[1], &PP[0]);
               findposvector(&PP[2], &PP[0]);
               VectorMagnitude(&PP[3]);
               UnitVector(&PP[1]);
               UnitVector(&PP[2]);
               UnitVector(&PP[3]);
               SurfaceNormal(&PP[0], &PP[1], &PP[2]);
               if (SurfaceVisible(&PP[0], &PP[3], 0))
                {
                if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
                 FractalMap[poly] = (BYTE)MaxFract;
	        } /* if visible */
	       } /* if */
              else
               {
               if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
                FractalMap[poly] = (BYTE)MaxFract;
	       } /* else */
              } /* if */
             Lc ++;
             map.facect ++;
             poly ++;
             UnderWater = 0;
             if (setquickfacetwo(&map, Lr, Lc))
              {
              PP[3].x = PP[3].y = PP[3].z = PP[3].q = 0;
              for (i=0; i<3; i++)
               {
               ptelev = *(map.map + map.facept[i]) * map.elscale / ELSCALE_METERS;
               ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
               if (ptelev <= SeaLevel && ! MapAsSFC)
                {
                UnderWater = 1;
                ptelev = SeaLevel;
		}
               DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
               DP.lat = latface[i];
               DP.lon = longface[i];
               getscrncoords(&dummy, &dummy, &dummy, &PP[i], &PP[3]);
	       }
              if (! UnderWater)
               {
               findposvector(&PP[1], &PP[0]);
               findposvector(&PP[2], &PP[0]);
               VectorMagnitude(&PP[3]);
               UnitVector(&PP[1]);
               UnitVector(&PP[2]);
               UnitVector(&PP[3]);
               SurfaceNormal(&PP[0], &PP[1], &PP[2]);
               if (SurfaceVisible(&PP[0], &PP[3], 0))
                {
                if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
                 FractalMap[poly] = (BYTE)MaxFract;
	        } /* if */
	       } /* if */
              else
               {
               if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
                FractalMap[poly] = (BYTE)MaxFract;
	       } /* else */
	      } /* if */
	     } /* for Lc=0... */
            if (CheckInput_ID() == ID_BW_CLOSE)
             {
             error = 1;
             break;
             } /* if user abort */
            if (BWDE)
             BusyWin_Update(BWDE, stepct + 1);
 	    } /* for Lr=0; */

           if (error)
            goto MapCleanup;

           if ((fFrd = fopen(filename, "wb")))
            {
            if (fwrite(FractalMap, FractalMapSize, 1, fFrd) != 1)
             {
             error = 1;
             Log(ERR_WRITE_FAIL, (CONST_STRPTR)FractalFile);
	     } /* if read error */
            fclose(fFrd);
	    } /* if file exists */
           else
            {
            error = 1;
            Log(ERR_OPEN_FAIL, (CONST_STRPTR)FractalFile);
	    } /* else */

MapCleanup:
           if (BWDE)
            BusyWin_Del(BWDE);
           BWDE = NULL;
           if (map.map) free_Memory(map.map, map.size);
           if (FractalMap) free_Memory(FractalMap, FractalMapSize);
           if (map.scrnptrx) free_Memory(map.scrnptrx, map.scrnptrsize);
           if (map.scrnptry) free_Memory(map.scrnptry, map.scrnptrsize);
           if (map.scrnptrq) free_Memory(map.scrnptrq, map.scrnptrsize);
           map.map = NULL;
           map.scrnptrx = map.scrnptry = map.scrnptrq = NULL;
           FractalMap = NULL;
           if (error == -1)
            {
            error = 0;
            goto RepeatLoad;
            } /* if */
           } /* if topo object */
          if (error)
           break;
          BusyWin_Update(BWIM, objectcount + 1);
	  } /* for objectcount=... */
         BusyWin_Del(BWIM);
         } /* if */
        if (error)
         break;
        BusyWin_Update(BWAN, frame - FirstFrame + FrameInt);
        } /* for frame=... */
       BusyWin_Del(BWAN);
       } /* if */
      } /* if */
     } /* if */
    } /* if */
   } /* if frame interval entered */
  FreeKeyTable();
  } /* if key table built */
 else
  {
  User_Message((CONST_STRPTR)"Render Module",
          (CONST_STRPTR)"Out of memory opening key frame table!\nOperation terminated.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  success = 0;
  } /* else */

 OBN = OrigOBN;

 return (success);

} /* FractalDepth_Preset() */

/*********************************************************************/

STATIC_FCN short setquickfaceone(struct elmapheaderV101 *map, long Lr, long Lc) // used locally only -> static, AF 23.7.2021
{
 map->facept[0] = map->facect;
 map->facept[1] = map->facect + map->columns;
 map->facept[2] = map->facect + 1;
 latface[0] = map->lolat + Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = latface[1] + map->steplat;
 longface[0] = map->lolong - Lr * map->steplong;
 longface[1] = longface[0] - map->steplong;
 longface[2] = longface[0];

 return (setquickface(map));

} /* setquickfaceone() */

/*************************************************************************/

STATIC_FCN short setquickfacetwo(struct elmapheaderV101 *map, long Lr, long Lc) // used locally only -> static, AF 23.7.2021
{
 map->facept[0] = map->facect + map->columns;
 map->facept[1] = map->facect;		
 map->facept[2] = map->facept[0] - 1;
 latface[0] = map->lolat + Lc * map->steplat;
 latface[1] = latface[0];
 latface[2] = latface[0] - map->steplat;
 longface[0] = map->lolong - (Lr + 1) * map->steplong;
 longface[1] = longface[0] + map->steplong;
 longface[2] = longface[0];

 return (setquickface(map));

} /* setquickfacetwo() */

/*********************************************************************/

short setquickface(struct elmapheaderV101 *map) // used locally only -> static, AF 23.7.2021
{
 short y, j = 0, k = 0, l = 0;
 long WayWide, WayHigh;

 WayWide = wide + 100;
 WayHigh = high + 100;

 for (y=0; y<3; y++)
  {
  ptqq[y] = *(map->scrnptrq + map->facept[y]);
  if (ptqq[y] < 0.0)
   {
   j = 3;
   break;
   } /* if */
  ptx[y] = *(map->scrnptrx + map->facept[y]);
  pty[y] = *(map->scrnptry + map->facept[y]);
  if (ptqq[y] < 0.0)
   l ++;
  if (ptx[y] < -100)
   j ++;
  else if (ptx[y] > WayWide)
   j ++;
  if (pty[y] < -100)
   k ++;
  else if (pty[y] > WayHigh)
   k ++;
  } /* for y=0... */

 return ((short)(j < 3 && k < 3 && l < 3));

} /* setquickface() */

/***********************************************************************/

STATIC_FCN short FractalLevel(short MaxSize) // used locally only -> static, AF 23.7.2021
{
 short MaxFract = 0;
 long dif[2][3], Fract;

 dif[0][0] = ptx[0] - ptx[1];
 dif[0][1] = ptx[0] - ptx[2];
 dif[0][2] = ptx[1] - ptx[2];
 dif[1][0] = pty[0] - pty[1];
 dif[1][1] = pty[0] - pty[2];
 dif[1][2] = pty[1] - pty[2];

 Fract = max(abs(dif[0][0]), abs(dif[0][1]));
 Fract = max(Fract, abs(dif[0][2]));
 Fract = max(Fract, abs(dif[1][0]));
 Fract = max(Fract, abs(dif[1][1]));
 Fract = max(Fract, abs(dif[1][2]));

 while (Fract > MaxSize && MaxFract < 255)
  {
  Fract >>= 1;
  MaxFract ++;
  } /* while */

 return (MaxFract);

} /* FractalLevel() */

/***********************************************************************/

STATIC_FCN short MakeFractalMap(struct elmapheaderV101 *map,
	BYTE *FractalMap, long FractalMapSize) // used locally only -> static, AF 23.7.2021
{
short i, MaxFract, MaxSize = 2, UnderWater;
long poly, Lr, Lc, MaxCol, stepct;
float dummy;
double ptelev;
struct coords PP[4];

 memset(FractalMap, -1, FractalMapSize);

 stepct = map->rows + 1;
 MaxCol = map->columns - 1;
 for (map->facect=0, poly=0, Lr=0; Lr<map->rows; Lr++, map->facect++, stepct++)
  {
  for (Lc=0; Lc<MaxCol; poly++, UnderWater=0)
   {
   if (setquickfaceone(map, Lr, Lc))
    {
    PP[3].x = PP[3].y = PP[3].z = PP[3].q = 0;
    for (i=0; i<3; i++)
     {
     ptelev = *(map->map + map->facept[i]) * map->elscale / ELSCALE_METERS;
     ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
     if (ptelev <= SeaLevel && ! map->ForceBath)
      {
      UnderWater = 1;
      ptelev = SeaLevel;
      } /* if */
     DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
     DP.lat = latface[i];
     DP.lon = longface[i];
     getscrncoords(&dummy, &dummy, &dummy, &PP[i], &PP[3]);
     } /* for i=0... */
    if (! UnderWater)
     {
     findposvector(&PP[1], &PP[0]);
     findposvector(&PP[2], &PP[0]);
     VectorMagnitude(&PP[3]);
     UnitVector(&PP[1]);
     UnitVector(&PP[2]);
     UnitVector(&PP[3]);
     SurfaceNormal(&PP[0], &PP[1], &PP[2]);
     if (SurfaceVisible(&PP[0], &PP[3], 0))
      {
      if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
       FractalMap[poly] = (BYTE)MaxFract;
      } /* if visible */
     } /* if */
    else
     {
     if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
      FractalMap[poly] = (BYTE)MaxFract;
     } /* else */
    } /* if */
   Lc ++;
   map->facect ++;
   poly ++;
   UnderWater = 0;
   if (setquickfacetwo(map, Lr, Lc))
    {
    PP[3].x = PP[3].y = PP[3].z = PP[3].q = 0;
    for (i=0; i<3; i++)
     {
     ptelev = *(map->map + map->facept[i]) * map->elscale / ELSCALE_METERS;
     ptelev += ((PARC_RNDR_MOTION(13) - ptelev) * PARC_RNDR_MOTION(12));
     if (ptelev <= SeaLevel && ! map->ForceBath)
      {
      UnderWater = 1;
      ptelev = SeaLevel;
      } /* if */
     DP.alt = ptelev * PARC_RNDR_MOTION(14) * .001;
     DP.lat = latface[i];
     DP.lon = longface[i];
     getscrncoords(&dummy, &dummy, &dummy, &PP[i], &PP[3]);
     }
    if (! UnderWater)
     {
     findposvector(&PP[1], &PP[0]);
     findposvector(&PP[2], &PP[0]);
     VectorMagnitude(&PP[3]);
     UnitVector(&PP[1]);
     UnitVector(&PP[2]);
     UnitVector(&PP[3]);
     SurfaceNormal(&PP[0], &PP[1], &PP[2]);
     if (SurfaceVisible(&PP[0], &PP[3], 0))
      {
      if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
       FractalMap[poly] = (BYTE)MaxFract;
      } /* if */
     } /* if */
    else
     {
     if ((MaxFract = FractalLevel(MaxSize)) > FractalMap[poly])
      FractalMap[poly] = (BYTE)MaxFract;
     } /* else */
    } /* if */
   } /* for Lc=0... */
  } /* for Lr=0; */

 return (1);

} /* MakeFractalMap() */
