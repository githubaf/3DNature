// GeoRaster.cpp
// Contains Raster processing and Gradient stuff for World Construction Set
// Created 5/31/97 by Gary R. Huber
// Copyright 1997 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "AppMem.h"
#include "Application.h"
#include "Joe.h"
#include "Points.h"
#include "Raster.h"
#include "Useful.h"
#include "Project.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "Log.h"
#include "VectorPolygon.h"

// allow easy switching between new & old code
#define NewPolyRasterFillByte
#undef  OldPolyRasterFillByte

//#define ABS(a) (((a)<0) ? -(a) : (a))
//#define SGN(a) (((a)<0) ? -1 : 1)

#ifdef DEBUG // we can't let anything in here make it to release builds!
// if we desire visual output for debugging
//#define DRAWTEST
#endif // DEBUG

//extern WCSApp *GlobalApp;

/*===========================================================================*/
// GeoRaster

GeoRaster::GeoRaster()
{

N = S = E = W = CellSizeNS = CellSizeEW = CellArea = 0.0;
LUTItems = LUTSize = JLUTItems = JLUTSize = 0;
GradFill = Overflow = Initialized = 0;
HighLUT = 0;
LUT = NULL;
JLUT = NULL;
LUTSet = 0;
floodmap = NULL;
vert = NULL;

} // GeoRaster::GeoRaster

/*===========================================================================*/

GeoRaster::~GeoRaster()
{

if (LUT)
	AppMem_Free(LUT, LUTSize);
LUT = NULL;
if (JLUT)
	AppMem_Free(JLUT, JLUTSize);
JLUT = NULL;

} // GeoRaster::GeoRaster

/*===========================================================================*/
// constructors for use by Effects

GeoRaster::GeoRaster(double sN, double sS, double sE, double sW, double sCellSize, long MaxMem, long &UsedMem,
	short MapType, long sLUTItems, long sJLUTItems, long AltMapMultiplier)
{
long x, NumCells;

ConstructError = GradFill = 0;

LUT = NULL;
JLUT = NULL;
Mask = 0;
LUTItems = sLUTItems;
JLUTItems = sJLUTItems;
Overflow = 0;
Initialized = 0;
HighLUT = 0;
LUTSet = 0;
N = S = W = E = CellSizeEW = CellSizeNS = CellArea = 0.0;
floodmap = NULL;
vert = NULL;

LUTSize = LUTItems * sizeof (GeneralEffect *);
JLUTSize = JLUTItems * sizeof (Joe *);
if ((UsedMem + LUTSize + JLUTSize <= MaxMem) && (! LUTSize || (LUT = (GeneralEffect **)AppMem_Alloc(LUTSize, APPMEM_CLEAR)))
	&& (! JLUTSize || (JLUT = (Joe **)AppMem_Alloc(JLUTSize, APPMEM_CLEAR))))
	{
	UsedMem += LUTSize;
	UsedMem += JLUTSize;
	Rows = quickftol((sN - sS) / sCellSize);
	if (Rows <= 0)
		Rows = 1;
	CellSizeNS = (sN - sS) / Rows;
	Cols =  quickftol((sW - sE) / sCellSize);
	if (Cols <= 0)
		Cols = 1;
	CellSizeEW = (sW - sE) / Cols;
	N = sN + CellSizeNS * 0.5;
	S = sS - CellSizeNS * 0.5;
	Rows ++;
	W = sW + CellSizeEW * 0.5;
	E = sE - CellSizeEW * 0.5;
	Cols ++;
	CellArea = CellSizeNS * CellSizeEW;
	switch (MapType)
		{
		case WCS_RASTER_BANDSET_BYTE:
			{
			ByteBandSize = Rows * Cols;
			if ((ByteBandSize + UsedMem <= MaxMem) && (ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL] = (UBYTE *)AppMem_Alloc(ByteBandSize, APPMEM_CLEAR)))
				{
				UsedMem += ByteBandSize;
				ByteBands = 1;
				} // if
			else
				ConstructError = 1;
			break;
			} // WCS_RASTER_BANDSET_BYTE
		case WCS_RASTER_BANDSET_FLOAT:
			{
			FloatBandSize = Rows * Cols * sizeof (float);
			if ((FloatBandSize + UsedMem <= MaxMem) && (FloatMap[WCS_RASTER_EFFECT_BAND_NORMAL] = (float *)AppMem_Alloc(FloatBandSize, APPMEM_CLEAR)))
				{
				UsedMem += FloatBandSize;
				FloatBands = 1;
				NumCells = Rows * Cols;
				for (x = 0; x < NumCells; x ++)
					{
					FloatMap[WCS_RASTER_EFFECT_BAND_NORMAL][x] = FLT_MIN;
					} // for
				} // else
			else
				ConstructError = 1;
			break;
			} // WCS_RASTER_BANDSET_FLOAT
		default:
			break;
		} // switch
	if (AltMapMultiplier)
		{
		AltByteBandSize = Rows * Cols * AltMapMultiplier;
		if ((AltByteBandSize + UsedMem <= MaxMem) && (AltByteMap = (UBYTE *)AppMem_Alloc(AltByteBandSize, APPMEM_CLEAR)))
			UsedMem += AltByteBandSize;
		else
			ConstructError = 1;
		} // if
	} // if
else
	ConstructError = 1;

} // GeoRaster::GeoRaster

/*===========================================================================*/

GeoRaster::GeoRaster(double sN, double sS, double sE, double sW, double sCellSize, long &UsedMem)
{

ConstructError = GradFill = 0;
LUT = NULL;
JLUT = NULL;
Mask = 0;
LUTItems = JLUTItems = LUTSize = JLUTSize = 0;
Overflow = 0;
Initialized = 0;
HighLUT = LUTSet = 0;
N = S = W = E = CellSizeEW = CellSizeNS = CellArea = 0.0;
floodmap = NULL;
vert = NULL;

Rows = quickftol((sN - sS) / sCellSize);
if (Rows <= 0)
	Rows = 1;
CellSizeNS = (sN - sS) / Rows;
Cols =  quickftol((sW - sE) / sCellSize);
if (Cols <= 0)
	Cols = 1;
CellSizeEW = (sW - sE) / Cols;
N = sN + CellSizeNS * 0.5;
S = sS - CellSizeNS * 0.5;
Rows ++;
W = sW + CellSizeEW * 0.5;
E = sE - CellSizeEW * 0.5;
Cols ++;
CellArea = CellSizeNS * CellSizeEW;

if (! AllocPolyListMap())
	ConstructError = 1;
else
	UsedMem += (sizeof(VectorPolygonList *) * Rows * Cols);

} // GeoRaster::GeoRaster

/*===========================================================================*/

GeoRaster::GeoRaster(GeoRaster *CopyRast, long MaxMem, long &UsedMem)
{
long x, NumCells;

ConstructError = 0;
N = S = E = W = CellSizeNS = CellSizeEW = CellArea = 0.0;
LUTItems = LUTSize = JLUTItems = JLUTSize = 0;
GradFill = Overflow = Initialized = 0;
Mask = 0;
HighLUT = 0;
LUT = NULL;
JLUT = NULL;
LUTSet = 0;
floodmap = NULL;
vert = NULL;

Copy(this, CopyRast);

if ((UsedMem + LUTSize + JLUTSize <= MaxMem) && (! LUTSize || (LUT = (GeneralEffect **)AppMem_Alloc(LUTSize, APPMEM_CLEAR)))
	&& (! JLUTSize || (JLUT = (Joe **)AppMem_Alloc(JLUTSize, APPMEM_CLEAR))))
	{
	UsedMem += LUTSize;
	UsedMem += JLUTSize;
	if (CopyRast->ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL])
		{
		if ((ByteBandSize + UsedMem <= MaxMem) && (ByteMap[WCS_RASTER_EFFECT_BAND_NORMAL] = (UBYTE *)AppMem_Alloc(ByteBandSize, APPMEM_CLEAR)))
			{
			UsedMem += ByteBandSize;
			ByteBands = 1;
			} // if
		else
			ConstructError = 1;
		} // WCS_RASTERTYPE_UBYTE
	else if (CopyRast->FloatMap[WCS_RASTER_EFFECT_BAND_NORMAL])
		{
		if ((FloatBandSize + UsedMem <= MaxMem) && (FloatMap[WCS_RASTER_EFFECT_BAND_NORMAL] = (float *)AppMem_Alloc(FloatBandSize, APPMEM_CLEAR)))
			{
			UsedMem += FloatBandSize;
			FloatBands = 1;
			NumCells = Rows * Cols;
			for (x = 0; x < NumCells; x ++)
				{
				FloatMap[WCS_RASTER_EFFECT_BAND_NORMAL][x] = FLT_MIN;
				} // for
			} // else
		else
			ConstructError = 1;
		} // WCS_RASTERTYPE_FLOAT
	if (CopyRast->AltByteMap)
		{
		if ((AltByteBandSize + UsedMem <= MaxMem) && (AltByteMap = (UBYTE *)AppMem_Alloc(AltByteBandSize, APPMEM_CLEAR)))
			UsedMem += AltByteBandSize;
		else
			ConstructError = 1;
		} // AltByteMap
	} // if
else
	ConstructError = 1;

} // GeoRaster::GeoRaster

/*===========================================================================*/

void GeoRaster::Copy(GeoRaster *CopyTo, GeoRaster *CopyFrom)
{

Raster::Copy((Raster *)CopyTo, (Raster *)CopyFrom);

CopyTo->N = CopyFrom->N;
CopyTo->S = CopyFrom->S;
CopyTo->E = CopyFrom->E;
CopyTo->W = CopyFrom->W;
CopyTo->CellSizeNS = CopyFrom->CellSizeNS;
CopyTo->CellSizeEW = CopyFrom->CellSizeEW;
CopyTo->CellArea = CopyFrom->CellArea;
CopyTo->LUTItems = CopyFrom->LUTItems;
CopyTo->LUTSize = CopyFrom->LUTSize;
CopyTo->JLUTItems = CopyFrom->JLUTItems;
CopyTo->JLUTSize = CopyFrom->JLUTSize;
CopyTo->GradFill = CopyFrom->GradFill;
CopyTo->Overflow = 0;
CopyTo->Initialized = 0;
CopyTo->HighLUT = 0;
CopyTo->LUT = NULL;
CopyTo->JLUT = NULL;
CopyTo->LUTSet = 0;

} // GeoRaster::Copy

/*===========================================================================*/
// Initializing

struct RasterPoint *GeoRaster::BuildRasterPointList(CoordSys *MyCoords, VectorPoint *StartPt, long &NumPts)
{
struct RasterPoint *PtList;
VectorPoint *Pt;
long Ct = 0 /*, Ctm1, Ctp1*/ /*, Iterate = 1 */;
VertexDEM MyVert;

if (PtList = (RasterPoint *)AppMem_Alloc(NumPts * sizeof (struct RasterPoint), 0))
	{
	Pt = StartPt;
	while (Ct < NumPts && Pt)
		{
		if (Pt->ProjToDefDeg(MyCoords, &MyVert))
			{
			if ((GetCell(MyVert.Lat, MyVert.Lon)) >= 0)
				{
				PtList[Ct].x = Col;
				PtList[Ct].y = Row;
				Ct ++;
				} // if
			} // if
		Pt = Pt->Next;
		} // while
	if (Ct < NumPts)
		NumPts = Ct;
// test stuff
// 6/10/2002 CXH: THis code discovered to be harmful, improperly removing points
// in the circmstance where A, B, C are all on one georaster scanline, but they
// are ordered:
//  A    C     B
// instead of
//  A    B     C
// This code is no longer necessary with the improved rasterizer anyway, so we are
// permanently disabling it.
/*
	if (NumPts > 2)
		{
		Ct = 0;
		Ctm1 = NumPts - 1;
		Ctp1 = 1;
		for (Ct = 0; Ct < NumPts && NumPts > 2; )
			{
			if (PtList[Ct].y == PtList[Ctm1].y && PtList[Ct].y == PtList[Ctp1].y)		// if two horizontal segments in a row
				{
				if (Ct < NumPts - 1)
					{
					memcpy(&PtList[Ct], &PtList[Ct + 1], (NumPts - 1 - Ct) * sizeof (struct RasterPoint));
					} // if not last point in list
				NumPts --;
				if (Ctm1 >= NumPts)
					Ctm1= NumPts - 1;
				} // if
			else
				{
				Ct ++;
				Ctm1 ++;
				Ctp1 ++;
				} // else
			if (Ctm1 >= NumPts)
				Ctm1 = 0;
			if (Ctp1 >= NumPts)
				Ctp1 = 0;
			} // for
		} // if at least three points
// end test
*/
	} // if

return (PtList);

} // GeoRaster::BuildRasterPointList

/*===========================================================================*/

struct RasterPoint *GeoRaster::BuildRasterPointList(VectorNode *StartPt, unsigned long NumPts)
{
struct RasterPoint *PtList;
VectorNode *Pt;
unsigned long Ct;

if (PtList = (RasterPoint *)AppMem_Alloc(NumPts * sizeof (struct RasterPoint), 0))
	{
	Pt = StartPt;
	for (Ct = 0; Ct < NumPts; ++Ct, Pt = Pt->NextNode)
		{
		// node coords are already default lat/lon
		if ((GetCell(Pt->Lat, Pt->Lon)) >= 0)
			{
			PtList[Ct].x = Col;
			PtList[Ct].y = Row;
			} // if
		else
			{
			FreeRasterPointList(PtList, NumPts);
			PtList = NULL;
			break;
			} // else
		} // for
	} // if

// returns NULL if any of the vertices are outside the bounds of the GeoRaster
return (PtList);

} // GeoRaster::BuildRasterPointList

/*===========================================================================*/

void GeoRaster::FreeRasterPointList(RasterPoint *PtList, long NumPts)
{

if (PtList)
	AppMem_Free(PtList, NumPts * sizeof (struct RasterPoint));

} // GeoRaster::FreeRasterPointList

/*===========================================================================*/

//#define vertex RastPt
#define HIGHSCRC	32767

void GeoRaster::ClearByte(short Band, long *ClearBounds, UBYTE ClearVal)
{
long ClearWidth, ClearStart, y;

ClearWidth = ClearBounds[3] - ClearBounds[2] + 1;

ClearStart = ClearBounds[0] * Cols + ClearBounds[2];

for (y = ClearBounds[0]; y <= ClearBounds[1]; y ++, ClearStart += Cols)
	{
	memset(&ByteMap[Band][ClearStart], ClearVal, ClearWidth);
	} // for

} // GeoRaster::ClearByte

/*===========================================================================*/

UBYTE GeoRaster::PixelRead(long x, long y)
{
long pixel;	// keep the ALPHA happy

assert((x >= 0) && (y >= 0));
pixel = (y * xsize) + x;
assert(pixel >=0 && pixel < mapsize);
assert(floodmap != NULL);
return(floodmap[pixel]);
};


/*===========================================================================*/

#define MAXFILLSTACK 40000
#define PUSH(Y, XL, XR, DY)\
	if ((sp < (stack+MAXFILLSTACK)) && (Y+(DY) >= 0) && (Y+(DY) < ysize))\
	{sp->y = Y; sp->xl = XL, sp->xr = XR; sp->dy = DY, sp++;}
#define POP(Y, XL, XR, DY)\
	{sp--; Y = sp->y+(DY = sp->dy); XL = sp->xl; XR = sp->xr;}
typedef struct {int y, xl, xr, dy;} Segment;

void GeoRaster::SeedFill(UBYTE fillval)
{

long l, x, y, x1, x2, dy;
long pixel;	// keep the Alpha happy
UBYTE ov;
Segment stack[MAXFILLSTACK], *sp = stack;

x = 0; y = 0;
ov = PixelRead(0L, 0L);
if (ov == fillval || x < 0 || x >= xsize || y < 0 || y >= ysize)
	return;
PUSH(y, x, x, 1);
PUSH((y+1), x, x, -1);
while (sp > stack)
	{
	POP(y, x1, x2, dy);
//	printf("y = %ld, x1 = %ld, x2 = %ld, dy = %ld\n", y, x1, x2, dy);
	for (x = x1; (x >= 0) && (PixelRead(x, y) == ov); x--)
		{
		assert((x >= 0) && (y >= 0));
		pixel = (y * xsize) + x;
		assert(pixel >= 0 && pixel < mapsize);
		assert(floodmap != NULL);
		floodmap[pixel] = fillval;
		} // for
	if (x >= x1) goto skip;
	l = x + 1;
	if (l < x1) PUSH(y, l, (x1-1), -dy);
	x = x1 + 1;
	do
		{
		for (; (x < xsize) && (PixelRead(x, y) == ov); x++)
			{
			assert((x >= 0) && (y >= 0));
			pixel = (y * xsize) + x;
			assert(pixel >= 0 && pixel < mapsize);
			assert(floodmap != NULL);
			floodmap[pixel] = fillval;
			} // for
		PUSH(y, l, (x-1), dy);
		if (x > x2+1) PUSH(y, (x2+1), (x-1), -dy);
skip:   for (x++; x <= x2 && (PixelRead(x, y) != ov); x++)
			{
			}	// yes, this is truly empty
		l = x;
		} while (x <= x2); // do
	} // while
};

/*===========================================================================*/
/*
void GeoRaster::DrawLine(long x1, long y1, long x2, long y2, UBYTE dotval)
{
long d, x, y, ax, ay, sx, sy, dx, dy;
long pixel;	// keep the Alpha happy

dx = x2-x1; ax = ABS(dx)<<1; sx = SGN(dx);
dy = y2-y1; ay = ABS(dy)<<1; sy = SGN(dy);

x = x1;
y = y1;
if (ax > ay)
	{
	d = ay - (ax / 2);	// (ax >> 1) may not always give desired result
	for (;;)
		{
		pixel = (y * xsize) + x;
//		if ((pixel < 0) || (pixel >= mapsize))
//			printf("Yikes\n");
		assert(pixel >= 0 && pixel < mapsize);
		assert(floodmap != NULL);
		floodmap[pixel] = dotval;
		if (x == x2) return;
		if (d >= 0)
			{
			y += sy;
			d -= ax;
			}
		x += sx;
		d += ay;
		}
	}
else
	{
	d = ax - (ay / 2);	// (ay >> 1) may not always give desired result 
	for (;;)
		{
		pixel = (y * xsize) + x;
//		if ((pixel < 0) || (pixel >= mapsize))
//			printf("Yikes\n");
		assert(pixel >= 0 && pixel < mapsize);
		assert(floodmap != NULL);
		floodmap[pixel] = dotval;
		if (y == y2) return;
		if (d >= 0)
			{
			x += sx;
			d -= ay;
			}
		y += sy;
		d += ax;
		}
	}
}
*/
/*===========================================================================*/
/*
void GeoRaster::DrawPoly(long nvert, long minx, long miny, UBYTE fillval)
{
	long i,j;
	long px1, py1, px2, py2;

	assert(vert != NULL);
	px2 = vert[0].x + 1;
	py2 = vert[0].y + 1;
	for (i = 0; i < nvert; i++)
		{
		j = (i + 1) % nvert;
		px1 = px2; py1 = py2;
		px2 = vert[j].x + 1;
		py2 = vert[j].y + 1;
		DrawLine(px1 - minx, py1 - miny, px2 - minx, py2 - miny, fillval);
		}
};
*/
/*===========================================================================*/

#ifdef NewPolyRasterFillByte
GeoRaster *GeoRaster::PolyRasterFillByte(long MaxMem, long &UsedMem, Joe *FillMe, short Band, UBYTE FillVal, short OverlapOK, short GradntFill)
{
GeoRaster *DrawRast = this, *LastRast;
long i,zip;
int Success = 0, DrawIt;
UBYTE *dstpos, *srcpos;
long nvert, x, y;
long minx, maxx, miny, maxy;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;

#ifdef DRAWTEST
static TestCnt=0;
UBYTE *TestPtrs[3];
char TestName[80];
#endif

if (MyAttr = (JoeCoordSys *)FillMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

floodmap = NULL;
vert = NULL;
mapsize = 0;
nvert = (long)FillMe->GetNumRealPoints();
if (!(vert = BuildRasterPointList(MyCoords, FillMe->GetFirstRealPoint(), nvert)))
	{
	goto F2EndPolyFill;
	}
//if (nvert == 1)
//	{
//	zip = vert[0].y * Cols + vert[0].x;
//	ByteMap[Band][zip] = FillVal;
//	return(DrawRast);
//	}
//printf("++ vert = %lx (%ld)\n", vert, nvert);
minx = HIGHSCRC;
miny = HIGHSCRC;
maxx = 0;
maxy = 0;
for (i=0; i<nvert; i++)
	{
	minx = min(minx, vert[i].x);
	maxx = max(maxx, vert[i].x);
	miny = min(miny, vert[i].y);
	maxy = max(maxy, vert[i].y);
	}

//printf("Minx = %ld, Maxx = %ld, Miny = %ld, Maxy = %ld\n", minx, maxx, miny, maxy);

// add at least 1 pixel border around bounding box (ie: +2 pixels)
// # of pixels = max-min+1
xsize = maxx - minx + 3;	// 2+max-min+1
ysize = maxy - miny + 3;

mapsize = xsize * ysize;
if (!(floodmap = (UBYTE *)AppMem_Alloc(mapsize, 0L)))
	goto F2EndPolyFill;	// function failed - can't allocate memory
//printf("++++ flood = %lx (%ld)\n", floodmap, mapsize);

// at this point, we have a bitmap to render into
memset(floodmap, FillVal, mapsize);
//mem = floodmap;
//size = 0;
//do	// initialize it to the fill value
//	{
//	*mem++ = FillVal;
//	size++;
//	} while (size < mapsize);
//DrawPoly(nvert, minx, miny, ~FillVal);	// flood boundary
// replaced DrawPoly with a more complete edge drawing algorithm that ensures all pixels touched by the edge are drawn
PolyRasterEdgeByteFloodmap(FillMe, ~FillVal, minx - 1, miny - 1, xsize, ysize);

#ifdef DRAWTEST
TestPtrs[0] = floodmap;
TestPtrs[1] = floodmap;
TestPtrs[2] = floodmap;
TestCnt++;
sprintf(TestName, "e:/debugn/F2PolyFill%1d_1.iff", TestCnt);
saveILBM(24, 0, &TestPtrs[0], NULL, 0, 1, 1, (short)xsize, (short)ysize, TestName);
#endif

SeedFill(~FillVal);	// flood exterior of poly

#ifdef DRAWTEST
sprintf(TestName, "e:/debugn/F2PolyFill%1d_2.iff", TestCnt);
saveILBM(24, 0, &TestPtrs[0], NULL, 0, 1, 1, (short)xsize, (short)ysize, TestName);
#endif

//DrawPoly(nvert, minx, miny, FillVal);	// reattach poly edges
// replaced DrawPoly with a more complete edge drawing algorithm that ensures all pixels touched by the edge are drawn
PolyRasterEdgeByteFloodmap(FillMe, FillVal, minx - 1, miny - 1, xsize, ysize);

#ifdef DRAWTEST
sprintf(TestName, "e:/debugn/F2PolyFill%1d_3.iff", TestCnt);
saveILBM(24, 0, &TestPtrs[0], NULL, 0, 1, 1, (short)xsize, (short)ysize, TestName);
#endif

// copy floodmap image to bitmap {don't copy 1 pixel border}
for (y = 0; y < (ysize - 2); y++)
	{
	// compute position to start copying scanline to
	//dstpos = bm + ((USHORT)(miny)+y)*col + (USHORT)minx;
	zip = (y + miny) * Cols + minx;
	dstpos = &ByteMap[Band][zip];
	srcpos = floodmap + (y+1)*xsize + 1;	// position past border
	for (x = 0; x < (xsize - 2); x++, dstpos++, srcpos++)
		{
		// copy filled poly only - leave rest of image untouched
		if (*srcpos == FillVal)
			{
			if (!OverlapOK)
				{
				//DrawRast->ByteMap[Band][y * Cols + x] = FillVal;
				*dstpos = FillVal;
				}
			else
				{
				zip = (y + miny) * Cols + x + minx;
				DrawRast = this;
				DrawIt = 1;
				while (DrawRast && DrawRast->ByteMap[Band][zip])
					{
					if (DrawRast->ByteMap[Band][zip] == FillVal)
						{
						DrawIt = 0;
						break;
						}
					LastRast = DrawRast;
					DrawRast = (GeoRaster *)DrawRast->Next;
					} // while
				if (DrawIt)
					{
					if (! DrawRast)
						{
						assert(LastRast != NULL);
						if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
							{
							goto F2EndPolyFill;
							} // if
						else
							{
							DrawRast = (GeoRaster *)LastRast->Next;
							} // else
						} // if ran out of maps
					DrawRast->ByteMap[Band][zip] = FillVal;
					} // if
				} // else
			} // if FillVal
		} // for x
	} // for y
AppMem_Free(floodmap, mapsize);
floodmap = NULL;
//printf("---- flood %lx (%ld)\n", floodmap, mapsize);

if (GradntFill)
	PolyRasterGradientByte(Band, FillVal);

Success = 1;

F2EndPolyFill:

if (vert)
	{
	FreeRasterPointList(vert, nvert);
//	printf("-- vert %lx (%ld)\n", vert, nvert);
	vert = NULL;
	} // if

if (! Success)
	return (NULL);

return (DrawRast);

} // PolyRasterFillByte
#endif

/*===========================================================================*/

unsigned char *GeoRaster::AllocFloodmap(Joe *FillMe, unsigned char ByteFill, long &minx, long &miny)
{
long i;
long nvert;
long maxx, maxy;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;

if (MyAttr = (JoeCoordSys *)FillMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

floodmap = NULL;
vert = NULL;
mapsize = 0;
nvert = (long)FillMe->GetNumRealPoints();
if (!(vert = BuildRasterPointList(MyCoords, FillMe->GetFirstRealPoint(), nvert)))
	{
	goto F2EndPolyFill;
	}
minx = HIGHSCRC;
miny = HIGHSCRC;
maxx = 0;
maxy = 0;
for (i=0; i<nvert; i++)
	{
	minx = min(minx, vert[i].x);
	maxx = max(maxx, vert[i].x);
	miny = min(miny, vert[i].y);
	maxy = max(maxy, vert[i].y);
	}

// add at least 1 pixel border around bounding box (ie: +2 pixels)
// # of pixels = max-min+1
xsize = maxx - minx + 3;	// 2+max-min+1
ysize = maxy - miny + 3;

mapsize = xsize * ysize;
if (floodmap = (UBYTE *)AppMem_Alloc(mapsize, 0L))
	{
	// at this point, we have a bitmap to render into
	memset(floodmap, ByteFill, mapsize);
	} // if

F2EndPolyFill:

if (vert)
	{
	FreeRasterPointList(vert, nvert);
	vert = NULL;
	} // if

return (floodmap);

} // GeoRaster::AllocFloodmap

/*===========================================================================*/

unsigned char *GeoRaster::AllocFloodmap(VectorPolygon *FillMe, unsigned char ByteFill, long &minx, long &miny)
{
long i;
long nvert;
long maxx, maxy;

floodmap = NULL;
vert = NULL;
mapsize = 0;
nvert = (long)FillMe->GetFirstPart()->NumNodes;
if (!(vert = BuildRasterPointList(FillMe->GetFirstPart()->FirstNode(), FillMe->GetFirstPart()->NumNodes)))
	{
	goto F2EndPolyFill;
	}
minx = HIGHSCRC;
miny = HIGHSCRC;
maxx = 0;
maxy = 0;
for (i=0; i<nvert; i++)
	{
	minx = min(minx, vert[i].x);
	maxx = max(maxx, vert[i].x);
	miny = min(miny, vert[i].y);
	maxy = max(maxy, vert[i].y);
	}

// add at least 1 pixel border around bounding box (ie: +2 pixels)
// # of pixels = max-min+1
xsize = maxx - minx + 3;	// 2+max-min+1
ysize = maxy - miny + 3;

mapsize = xsize * ysize;
if (floodmap = (UBYTE *)AppMem_Alloc(mapsize, 0L))
	{
	// at this point, we have a bitmap to render into
	memset(floodmap, ByteFill, mapsize);
	} // if

F2EndPolyFill:

if (vert)
	{
	FreeRasterPointList(vert, nvert);
	vert = NULL;
	} // if

return (floodmap);

} // GeoRaster::AllocFloodmap

/*===========================================================================*/

void GeoRaster::FreeFloodmap(void)
{

if (floodmap)
	AppMem_Free(floodmap, mapsize);
floodmap = NULL;
ysize = xsize = mapsize = 0;

} // GeoRaster::FreeFloodmap

/*===========================================================================*/

int GeoRaster::PolyRasterFillFloat(Joe *FillMe, short Band, float FloatFill)
{
UBYTE *srcpos, ByteFill = 1;
float *dstpos;
long MinX, MinY;
long zip, x, y;

if (! AllocFloodmap(FillMe, ByteFill, MinX, MinY))
	return (0);

PolyRasterEdgeByteFloodmap(FillMe, ~ByteFill, MinX - 1, MinY - 1, xsize, ysize);

SeedFill(~ByteFill);	// flood exterior of poly

PolyRasterEdgeByteFloodmap(FillMe, ByteFill, MinX - 1, MinY - 1, xsize, ysize);

// copy floodmap image to bitmap {don't copy 1 pixel border}
for (y = 0; y < (ysize - 2); y++)
	{
	// compute position to start copying scanline to
	//dstpos = bm + ((USHORT)(miny)+y)*col + (USHORT)minx;
	zip = (y + MinY) * Cols + MinX;
	dstpos = &FloatMap[Band][zip];
	srcpos = floodmap + (y + 1) * xsize + 1;	// position past border
	for (x = 0; x < (xsize - 2); x ++, dstpos ++, srcpos ++)
		{
		// copy filled poly only - leave rest of image untouched
		if (*srcpos == ByteFill)
			{
			*dstpos = FloatFill;
			} // if FillVal
		} // for x
	} // for y

FreeFloodmap();

return (1);

} // PolyRasterFillFloat

/*===========================================================================*/

void GeoRaster::PolyRasterTransferFloat(short Band, long MinX, long MinY, float FloatFill)
{
UBYTE *srcpos, ByteFill = 1;
float *dstpos;
long zip, x, y;

// copy floodmap image to bitmap {don't copy 1 pixel border}
for (y = 0; y < (ysize - 2); y++)
	{
	// compute position to start copying scanline to
	//dstpos = bm + ((USHORT)(miny)+y)*col + (USHORT)minx;
	zip = (y + MinY) * Cols + MinX;
	dstpos = &FloatMap[Band][zip];
	srcpos = floodmap + (y + 1) * xsize + 1;	// position past border
	for (x = 0; x < (xsize - 2); x ++, dstpos ++, srcpos ++)
		{
		// copy filled poly only - leave rest of image untouched
		if (*srcpos == ByteFill)
			{
			*dstpos = FloatFill;
			} // if FillVal
		} // for x
	} // for y

} // PolyRasterTransferFloat

/*===========================================================================*/

#ifdef OldPolyRasterFillByte
GeoRaster *GeoRaster::PolyRasterFillByte(long MaxMem, long &UsedMem, Joe *FillMe, short Band, UBYTE FillVal, short OverlapOK, short GradntFill)
{
struct RasterPoint *RastPt = NULL;
int Success = 0, DrawIt;
long minint, maxint, thisint, i, j, k, l, m, n, nvert, nedge, nint, horedge, iterations, count, Onvert, zip;
unsigned long s, t, u, y, bymax, bymin;
struct edg *edge = NULL;
unsigned long *inter = NULL;
GeoRaster *DrawRast = this, *LastRast;

#ifdef DRAWTEST
static TestCnt=0;
UBYTE *TestBM,*TestPtrs[3];
char TestName[80];

TestBM = (UBYTE *)AppMem_Alloc(Rows * Cols, APPMEM_CLEAR);	// ptr checked at time of usage
#endif

Onvert = nvert = FillMe->GetNumRealPoints();
if (! (RastPt = BuildRasterPointList(FillMe->GetFirstRealPoint(), nvert)))
	{
	goto EndPolyFill;
	} // if

if ((edge = (struct edg *)AppMem_Alloc(nvert * sizeof (struct edg), 0)) == NULL)
	{
	goto EndPolyFill;
	} // if

if ((inter = (unsigned long *)AppMem_Alloc(nvert * sizeof (unsigned long), 0)) == NULL)
	{
	goto EndPolyFill;
	} // if

bymin = HIGHSCRC;
bymax = 0;
for (i = 0; i < nvert; i++)
	{
	bymin = min(bymin, (y = vertex[i].y));
	bymax = max(bymax, y);
	} // for

horedge = FALSE;

if (nvert == 1)
	{
	ByteMap[Band][vertex[0].y * Cols + vertex[0].x] = FillVal;
	goto EndFillSuccess;
	} // if

for (i = j = 0; i < nvert; i++)
	{
	k = (i + 1) % nvert;
	if (vertex[i].y > vertex[k].y)
		{
		l = i;
		m = k;
		} // if
	else if (vertex[i].y == vertex[k].y)
		{
		horedge = TRUE;
		continue;
		} // else
	else
		{
		l = k;
		m = i;
		} // else
	edge[j].ymax = vertex[l].y;
	edge[j].ymin = vertex[m].y;
	edge[j].xpos = vertex[l].x;
	edge[j].xend = vertex[m].x;

	edge[j].slope =
		(double) ((long) edge[j].xend - (long) edge[j].xpos) /
		(double) ((long) edge[j].ymin - (long) edge[j].ymax);
	j++;
	} // for
nedge = j;
iterations = bymax - bymin;

for (y = bymax, count = 0; count <= iterations; y--, count++)
	{
	nint = 0;
	minint = HIGHSCRC;
	maxint = 0;

	for (j = 0; j < nedge; j++)
		{
		if (y <= edge[j].ymax && y >= edge[j].ymin)
			{
			if (y == edge[j].ymin)
				{
				thisint = inter[nint] = edge[j].xend;
				} // if
			else
				{
				thisint = inter[nint] = (unsigned long)((edge[j].xpos - edge[j].slope * (edge[j].ymax - y)) + 0.4999);
				} // else
			if (thisint <= minint)
				{
				minint = thisint;
				} // if
			if (thisint >= maxint)
				{
				maxint = thisint;
				} // if
			nint++;
			} // if
		} // for

	if (nint > 1)
		{
		for (i = 0; i < (nint - 1); i++)
			{
			for (j = i + 1; j < nint; j++)
				{
				if (inter[i] >= inter[j])
					{
					s = inter[i];
					inter[i] = inter[j];
					inter[j] = s;
					} // if
				} // for
			} // for
		if (nint > 2)
			{
			for (i = 0; i < (nint - 1); i++)
				{
				if (inter[i] == inter[i + 1])
					{
					for (j = 0; j < nvert; j++)
						{
						if (vertex[j].y == (long)y && vertex[j].x == (long)inter[i])
							{
							break;
							} // if
						} // for
					if (j < nvert)
						{
						s = vertex[(j == 0) ? nvert - 1 : j - 1].y;
						t = vertex[(j + 1) % nvert].y;
						u = vertex[j].y;
						if (!((u > s && u > t) || (u < s && u < t)))
							{
							for (j = i + 1; j < (nint - 1); j++)
								{
								inter[j] = inter[j + 1];
								} // for
							nint--;
							} // if
						} // if
					} // if
				} // for
			} // if
		if (horedge && (nint > 2))
			{
			for (i = 0; i < nvert; i++)
				{
				if (((long)y == vertex[i].y) && ((long)y == vertex[(i + 1) % nvert].y))
					{
					l = vertex[i].x;
					m = vertex[(i + 1) % nvert].x;
					for (k = 0; k < nint - 1; k++)
						{
						if ((((unsigned long)l == inter[k]) && ((unsigned long)m == inter[k + 1])) || (((unsigned long)m == inter[k]) && ((unsigned long)l == inter[k + 1])))
							{
							u = 0;
							s = vertex[((i + 2) % nvert)].y;
							t = vertex[((i + nvert) - 1) % nvert].y;
							if ((k & 1) == 0)
								{
								if (((s <= y) && (t >= y)) || ((s >= y) && (t <= y)))
									{
									u = 1;
									} // if
								} // if
							else
								{ // ((k & 1) == 1) 
//								assert (l == inter[k] || l == inter[k + 1])
								u = 1;
								if (((s <= y) && (t <= y)) || ((s >= y) && (t >= y)))
									{
									u++;
									} // if
								} // else
							while (u-- > 0)
								{
//								for (n = k; n < (nint - 1); n++)
								for (n = k + 1 - k % 2; n < (nint - 1); n++)
									inter[n] = inter[n + 1];
								nint--;
								} // while
							} // if
						} // for
					} // if
				} // for
			} // if horedge
		for (i = 0; i < (nint - 1); i += 2)
			{
			if (! OverlapOK)
				{
				memset(&ByteMap[Band][y * Cols + inter[i]], FillVal, inter[i + 1] - inter[i] + 1);	// try it with and without the +1
//				memset(&ByteMap[y * Cols + inter[i]], FillVal, inter[i + 1] - inter[i]);	// try it with and without the +1
				} // if no overlapping effects so just blast the pixels in there
			else
				{
				zip = y * Cols + inter[i];
				for (j = (long)inter[i]; j <= (long)inter[i + 1]; j ++, zip ++)
					{
					DrawRast = this;
					DrawIt = 1;
					while (DrawRast && DrawRast->ByteMap[Band][zip])
						{
						if (DrawRast->ByteMap[Band][zip] == FillVal)
							{
							DrawIt = 0;
							break;
							}
						LastRast = DrawRast;
						DrawRast = (GeoRaster *)DrawRast->Next;
						} // while
					if (DrawIt)
						{
						if (! DrawRast)
							{
							if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
								{
								goto EndPolyFill;
								} // if
							else
								{
								DrawRast = (GeoRaster *)LastRast->Next;
								} // else
							} // if ran out of maps
						DrawRast->ByteMap[Band][zip] = FillVal;
						} // if
					} // for
				} // else set each pixel and check to see if need to go on to next map
			} // for
		} // if nint > 1
	} // for

// draw edges

if (! (DrawRast = PolyRasterEdgeByte(MaxMem, UsedMem, FillMe, Band, FillVal, OverlapOK, TRUE)))
	goto EndPolyFill;

if (GradntFill)
	PolyRasterGradientByte(Band, FillVal);

#ifdef DRAWTEST	
// testing stuff - draw vertices
if (TestBM != NULL)
	{
	for (i = 0; i < nvert; i ++)
		{
		TestBM[vertex[i].y * Cols + vertex[i].x] = 255;
		// PhotoShop doesn't like 8 bit IFF, so fake 24 bit
		TestPtrs[0] = &TestBM[0];
		TestPtrs[1] = &TestBM[0];
		TestPtrs[2] = &TestBM[0];
		} // for
	sprintf(TestName, "e:/debugn/DrawVerts%1d.iff", TestCnt);
	TestCnt++;
	saveILBM(24, 0, &TestPtrs[0], NULL, 0, 1, 1, (short)Cols, (short)Rows, TestName);
	AppMem_Free(TestBM, Rows * Cols);
	} // if
#endif // DRAWTEST

EndFillSuccess:

Success = 1;

EndPolyFill:

if (RastPt)
	{
	FreeRasterPointList(RastPt, Onvert);
	} // if

if (edge)
	{
	AppMem_Free(edge, nvert * sizeof (struct edg));
	} // if

if (inter)
	{
	AppMem_Free(inter, nvert * sizeof (unsigned long));
	} // if

if (! Success)
	return (NULL);

return (DrawRast);

} // GeoRaster::PolyRasterFillByte
#endif // OldPolyRasterFillByte

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterCopyByte(GeoRaster *CopyFrom, long MaxMem, long &UsedMem, Joe *FillMe, GeneralEffect *Effect, short Band, long *CopyBounds, short OverlapOK)
{
long x, y, zip;
UBYTE FillVal;
GeoRaster *DrawRast, *LastRast = this;

// we're gonna trust that CopyBounds are really within limits

if (! OverlapOK)
	{
	DrawRast = this;
	while (DrawRast && DrawRast->Overflow)
		{
		LastRast = DrawRast;
		DrawRast = (GeoRaster *)DrawRast->Next;
		} // while
	while (DrawRast && DrawRast->HighLUT >= 255)
		{
		DrawRast->Overflow = 1;
		LastRast = DrawRast;
		DrawRast = (GeoRaster *)DrawRast->Next;
		} // if
	if (! DrawRast)
		{
		if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
			{
			return (NULL);
			} // if
		else
			{
			DrawRast = (GeoRaster *)LastRast->Next;
			} // else
		} // if
	DrawRast->HighLUT ++;
	DrawRast->LUT[DrawRast->HighLUT] = Effect;
	DrawRast->JLUT[DrawRast->HighLUT] = FillMe;
	FillVal = DrawRast->HighLUT;
	for (y = CopyBounds[0]; y <= CopyBounds[1]; y ++)
		{
		zip = y * DrawRast->Cols + CopyBounds[2];
		for (x = CopyBounds[2]; x <= CopyBounds[3]; x ++, zip ++)
			{
			if (CopyFrom->ByteMap[Band][zip])
				DrawRast->ByteMap[Band][zip] = FillVal;
			} // for
		} // for
	} // if no overlapping effects so just blast the pixels in there
else
	{
	DrawRast = this;
	while (DrawRast)
		{
		DrawRast->LUTSet = 0;
		DrawRast = (GeoRaster *)DrawRast->Next;
		} // while
	DrawRast = this;
	for (y = CopyBounds[0]; y <= CopyBounds[1]; y ++)
		{
		zip = y * DrawRast->Cols + CopyBounds[2];
		for (x = CopyBounds[2]; x <= CopyBounds[3]; x ++, zip ++)
			{
			if (CopyFrom->ByteMap[Band][zip])
				{
				DrawRast = this;
				while (DrawRast && (DrawRast->ByteMap[Band][zip] || DrawRast->Overflow))
					{
					LastRast = DrawRast;
					DrawRast = (GeoRaster *)DrawRast->Next;
					} // while
				while (DrawRast && DrawRast->HighLUT >= 255)
					{
					DrawRast->Overflow = 1;
					LastRast = DrawRast;
					DrawRast = (GeoRaster *)DrawRast->Next;
					} // if
				if (! DrawRast)
					{
					if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
						{
						return (NULL);
						} // if
					else
						{
						DrawRast = (GeoRaster *)LastRast->Next;
						} // else
					} // if
				if (! DrawRast->LUTSet)
					{
					DrawRast->HighLUT ++;
					DrawRast->LUT[DrawRast->HighLUT] = Effect;
					DrawRast->JLUT[DrawRast->HighLUT] = FillMe;
					DrawRast->LUTSet = 1;
					} // if
				DrawRast->ByteMap[Band][zip] = DrawRast->HighLUT;
				} // if
			} // for
		} // for
	} // else

return (DrawRast);

} // GeoRaster::PolyRasterCopyByte

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterCopyFloodByte(long MaxMem, long &UsedMem, 
	short Band, UBYTE FillVal, short OverlapOK, short GradntFill, long OffsetX, long OffsetY)
{
GeoRaster *DrawRast = this, *LastRast;
long zip;
int Success = 0, DrawIt;
UBYTE *dstpos, *srcpos;
long x, y;

// copy floodmap image to bitmap {don't copy 1 pixel border}
for (y = 0; y < (ysize - 2); y++)
	{
	// compute position to start copying scanline to
	zip = (y + OffsetY) * Cols + OffsetX;
	dstpos = &ByteMap[Band][zip];
	srcpos = floodmap + (y + 1) * xsize + 1;	// position past border
	for (x = 0; x < (xsize - 2); x ++, dstpos ++, srcpos ++)
		{
		// copy filled poly only - leave rest of image untouched
		if (*srcpos == FillVal)
			{
			if (! OverlapOK)
				{
				*dstpos = FillVal;
				}
			else
				{
				zip = (y + OffsetY) * Cols + x + OffsetX;
				DrawRast = this;
				DrawIt = 1;
				while (DrawRast && DrawRast->ByteMap[Band][zip])
					{
					if (DrawRast->ByteMap[Band][zip] == FillVal)
						{
						DrawIt = 0;
						break;
						}
					LastRast = DrawRast;
					DrawRast = (GeoRaster *)DrawRast->Next;
					} // while
				if (DrawIt)
					{
					if (! DrawRast)
						{
						assert(LastRast != NULL);
						if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
							{
							goto F2EndPolyFill;
							} // if
						else
							{
							DrawRast = (GeoRaster *)LastRast->Next;
							} // else
						} // if ran out of maps
					DrawRast->ByteMap[Band][zip] = FillVal;
					} // if
				} // else
			} // if FillVal
		} // for x
	} // for y
AppMem_Free(floodmap, mapsize);
floodmap = NULL;

if (GradntFill)
	PolyRasterGradientByte(Band, FillVal);

Success = 1;

F2EndPolyFill:

if (! Success)
	return (NULL);

return (DrawRast);

} // GeoRaster::PolyRasterCopyFloodByte

/*===========================================================================*/

bool GeoRaster::PolyRasterCopyFloodPolyList(VectorPolygon *CopyMe, long OffsetX, long OffsetY)
{
long zip;
bool Success = true;
UBYTE *srcpos;
VectorPolygonList **dstpos;
long x, y, ysizem2, xsizem2;

// copy floodmap image to bitmap {don't copy 1 pixel border}
ysizem2 = ysize - 2;
xsizem2 = xsize - 2;
for (y = 0; y < ysizem2 && Success; ++y)
	{
	// compute position to start copying scanline to
	zip = (y + OffsetY) * Cols + OffsetX;
	dstpos = &PolyListBlock[zip];
	srcpos = floodmap + (y + 1) * xsize + 1;	// position past border
	for (x = 0; x < xsizem2 && Success; ++x, ++dstpos, ++srcpos)
		{
		// copy filled poly only - leave rest of image untouched
		if (*srcpos)
			{
			Success = PlotPolygonPoint(CopyMe, dstpos, false);
			} // if
		} // for x
	} // for y
AppMem_Free(floodmap, mapsize);
floodmap = NULL;

return (Success);

} // GeoRaster::PolyRasterCopyFloodPolyList

/*===========================================================================*/

void GeoRaster::PolyRasterGradientByte(short Band, UBYTE FillVal)
{
long xIn = 0, xOut = Cols - 1, yIn = 0, yOut = Rows - 1, zip, x, y, MoreToFill = 1;
short NewFill, InlineMin, DiagMin, MaxSearch, MaxFill;

MaxSearch = 0;
MaxFill = 3;

while (MaxFill <= 255 && MoreToFill)
	{
	MoreToFill = 0;
	for (y = yIn; y <= yOut; y ++)
		{
		zip = y * Cols + xIn;
		for (x = xIn; x <= xOut; x ++, zip ++)
			{
			if (ByteMap[Band][zip] == FillVal)
				{
				InlineMin = 2 + MinInline(Band, zip, x, y, MaxSearch);
				DiagMin = 3 + MinDiag(Band, zip, x, y, MaxSearch);
				NewFill = min(InlineMin, DiagMin);
				if (NewFill <= MaxFill)
					{
					ByteMap[Band][zip] = (UBYTE)NewFill; 
					} // if
				MoreToFill = 1;
				} // if
			} // for
		} // for
	xIn ++;
	xOut --;
	yIn ++;
	yOut --;
	MaxSearch += 2;
	MaxFill += 2;
	} // for

} // GeoRaster::PolyRasterGradientByte

/*===========================================================================*/

void GeoRaster::PolyRasterFillOutByte(short Band, UBYTE FillVal, long Iterations, long *CopyBounds)
{
long zip, x, y, Passes = 0;
short NewFill, InlineMin, DiagMin, MaxSearch, MaxFill;

while (Passes < Iterations)
	{
	MaxSearch = FillVal;
	MaxFill = FillVal + 3;
	while (MaxFill <= 255 && Passes < Iterations)
		{
		for (y = CopyBounds[0]; y <= CopyBounds[1]; y ++)
			{
			zip = y * Cols + CopyBounds[2];
			for (x = CopyBounds[2]; x <= CopyBounds[3]; x ++, zip ++)
				{
				if (! ByteMap[Band][zip])
					{
					InlineMin = MinInlineFillOut(Band, zip, x, y, MaxSearch);
					DiagMin = MinDiagFillOut(Band, zip, x, y, MaxSearch);
					if (InlineMin)
						{
						InlineMin += 2;
						if (DiagMin)
							{
							DiagMin += 3;
							NewFill = min(InlineMin, DiagMin);
							} // if
						else
							NewFill = InlineMin;
						} // if
					else if (DiagMin)
						{
						DiagMin += 3;
						NewFill = DiagMin;
						} // else if
					else
						NewFill = 300;	// just some number larger than 255;
					if (NewFill <= MaxFill)
						{
						ByteMap[Band][zip] = (UBYTE)NewFill; 
						} // if
					} // if
				} // for
			} // for
		if (CopyBounds[0] > 0)
			CopyBounds[0] --;
		if (CopyBounds[1] < Rows - 1)
			CopyBounds[1] ++;
		if (CopyBounds[2] > 0)
			CopyBounds[2] --;
		if (CopyBounds[3] < Cols - 1)
			CopyBounds[3] ++;
		MaxSearch += 2;
		MaxFill += 2;
		Passes ++;
		} // while
	if (Passes < Iterations)
		{
		for (y = CopyBounds[0]; y <= CopyBounds[1]; y ++)
			{
			zip = y * Cols + CopyBounds[2];
			for (x = CopyBounds[2]; x <= CopyBounds[3]; x ++, zip ++)
				{
				if (ByteMap[Band][zip])
					{
					ByteMap[Band][zip] = 1; 
					} // if
				} // for
			} // for
		FillVal = 1;
		} // if
	} // while

} // GeoRaster::PolyRasterFillOutByte

/*===========================================================================*/

UBYTE GeoRaster::MinInline(short Band, long zip, long x, long y, short MaxSearch)
{
UBYTE MinVal = ByteMap[Band][zip], NewMin;

if (x > 0)
	{
	NewMin = ByteMap[Band][zip - 1];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (x < Cols - 1)
	{
	NewMin = ByteMap[Band][zip + 1];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (y > 0)
	{
	NewMin = ByteMap[Band][zip - Cols];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (y < Rows - 1)
	{
	NewMin = ByteMap[Band][zip + Cols];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);

return (MinVal);

} // GeoRaster::MinInline

/*===========================================================================*/

UBYTE GeoRaster::MinInlineFillOut(short Band, long zip, long x, long y, short MaxSearch)
{
UBYTE MinVal = 0, NewMin;

if (x > 0)
	{
	NewMin = ByteMap[Band][zip - 1];
	if (NewMin > 0 && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
if (x < Cols - 1)
	{
	NewMin = ByteMap[Band][zip + 1];
	if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
		MinVal = NewMin;
	} // if
if (y > 0)
	{
	NewMin = ByteMap[Band][zip - Cols];
	if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
		MinVal = NewMin;
	} // if
if (y < Rows - 1)
	{
	NewMin = ByteMap[Band][zip + Cols];
	if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
		MinVal = NewMin;
	} // if

return (MinVal);

} // GeoRaster::MinInlineFillOut

/*===========================================================================*/

UBYTE GeoRaster::MinDiag(short Band, long zip, long x, long y, short MaxSearch)
{
UBYTE MinVal = ByteMap[Band][zip], NewMin;

if (x > 0)
	{
	if (y > 0)
		{
		NewMin = ByteMap[Band][zip - 1 - Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = ByteMap[Band][zip - 1 + Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	} // if
if (x < Cols - 1)
	{
	if (y > 0)
		{
		NewMin = ByteMap[Band][zip + 1 - Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = ByteMap[Band][zip + 1 + Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	} // if

return (MinVal);

} // GeoRaster::MinDiag

/*===========================================================================*/

UBYTE GeoRaster::MinDiagFillOut(short Band, long zip, long x, long y, short MaxSearch)
{
UBYTE MinVal = 0, NewMin;

if (x > 0)
	{
	if (y > 0)
		{
		NewMin = ByteMap[Band][zip - 1 - Cols];
		if (NewMin > 0 && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = ByteMap[Band][zip - 1 + Cols];
		if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
			MinVal = NewMin;
		} // if
	} // if
if (x < Cols - 1)
	{
	if (y > 0)
		{
		NewMin = ByteMap[Band][zip + 1 - Cols];
		if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = ByteMap[Band][zip + 1 + Cols];
		if (NewMin > 0 && NewMin <= MaxSearch && (MinVal == 0 || NewMin < MinVal))
			MinVal = NewMin;
		} // if
	} // if

return (MinVal);

} // GeoRaster::MinDiag

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterEdgeByte(long MaxMem, long &UsedMem, Joe *OutlineMe, short Band, UBYTE FillVal, short OverlapOK, short ConnectBack)
{
double LastLat, LastLon;
GeoRaster *DrawRast = this;
VectorPoint *Pt, *NextPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

// draw first point unconditionally so that point style terraffectors with only one point work
if (Pt = OutlineMe->GetFirstRealPoint())
	{
	if (MyAttr = (JoeCoordSys *)OutlineMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	if (Pt->ProjToDefDeg(MyCoords, &MyVert))
		{
		if ((DrawRast = PolyRasterSegByte(MaxMem, UsedMem, MyVert.Lon, MyVert.Lat, MyVert.Lon, MyVert.Lat, Band, FillVal, OverlapOK)) == NULL)
			return (NULL);
		for ( ; Pt; Pt = Pt->Next)
			{
			LastLat = MyVert.Lat;
			LastLon = MyVert.Lon;
			NextPt = Pt->Next;
			if (! NextPt)
				{
				if (! ConnectBack)
					break;
				NextPt = OutlineMe->GetFirstRealPoint();
				} // if
			if (NextPt->ProjToDefDeg(MyCoords, &MyVert))
				{
				if ((DrawRast = PolyRasterSegByte(MaxMem, UsedMem, LastLon, LastLat, MyVert.Lon, MyVert.Lat, Band, FillVal, OverlapOK)) == NULL)
					break;
				} // if
			} // for
		} // if
	} // if

return (DrawRast);

} // GeoRaster::PolyRasterEdgeByte

/*===========================================================================*/

bool GeoRaster::PolyRasterEdgePolyList(VectorPolygon *OutlineMe)
{
double LastLat, LastLon;
unsigned long NumPts, PtCt;
bool Success = true;
VectorNode *Pt;
VectorPart *CurPart;

// draw first point unconditionally so that point style terraffectors with only one point work
// for each part
for (CurPart = OutlineMe->GetFirstPart(); CurPart && Success; CurPart = CurPart->NextPart)
	{
	NumPts = CurPart->NumNodes;
	if (Pt = CurPart->FirstNode())
		{
		if (PolyRasterSegPolyList(OutlineMe, Pt->Lon, Pt->Lat, Pt->Lon, Pt->Lat))
			{
			LastLat = Pt->Lat;
			LastLon = Pt->Lon;
			Pt = Pt->NextNode;
			for (PtCt = 0; PtCt < NumPts && Success; ++PtCt, Pt = Pt->NextNode)
				{
				Success = PolyRasterSegPolyList(OutlineMe, LastLon, LastLat, Pt->Lon, Pt->Lat);
				LastLat = Pt->Lat;
				LastLon = Pt->Lon;
				} // for
			} // if
		} // if
	} // for
	
return (Success);

} // GeoRaster::PolyRasterEdgePolyList

/*===========================================================================*/

// same code as above but with added offset for floodmap and directly filling floodmap
void GeoRaster::PolyRasterEdgeByteFloodmap(Joe *OutlineMe, UBYTE FillVal, long OffsetX, long OffsetY, long MapWidth, long MapHeight)
{
double LastLat, LastLon;
VectorPoint *Pt, *NextPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

// draw first point unconditionally so that point style terraffectors with only one point work
if (Pt = OutlineMe->GetFirstRealPoint())
	{
	if (MyAttr = (JoeCoordSys *)OutlineMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	if (Pt->ProjToDefDeg(MyCoords, &MyVert))
		{
		PolyRasterSegByteFloodmap(MyVert.Lon, MyVert.Lat, MyVert.Lon, MyVert.Lat, FillVal, OffsetX, OffsetY, MapWidth, MapHeight);
		for ( ; Pt; Pt = Pt->Next)
			{
			LastLat = MyVert.Lat;
			LastLon = MyVert.Lon;
			NextPt = Pt->Next;
			if (! NextPt)
				{
				NextPt = OutlineMe->GetFirstRealPoint();
				} // if
			if (NextPt->ProjToDefDeg(MyCoords, &MyVert))
				{
				PolyRasterSegByteFloodmap(LastLon, LastLat, MyVert.Lon, MyVert.Lat, FillVal, OffsetX, OffsetY, MapWidth, MapHeight);
				} // if
			} // for
		} // if
	} // if

} // GeoRaster::PolyRasterEdgeByteFloodmap

/*===========================================================================*/

void GeoRaster::PolyRasterEdgeByteFloodmap(VectorPart *OutlineMe, UBYTE FillVal, 
	long OffsetX, long OffsetY, long MapWidth, long MapHeight)
{
double LastLat, LastLon;
VectorNode *Pt;
unsigned long PtCt, NumNodes;

// draw first point unconditionally so that point style terraffectors with only one point work
if (Pt = OutlineMe->FirstNode())
	{
	NumNodes = OutlineMe->NumNodes;
	// plot a single point first
	PolyRasterSegByteFloodmap(Pt->Lon, Pt->Lat, Pt->Lon, Pt->Lat, FillVal, OffsetX, OffsetY, MapWidth, MapHeight);
	LastLat = Pt->Lat;
	LastLon = Pt->Lon;
	Pt = Pt->NextNode;
	// plot all the segments until we connect back to origin
	for (PtCt = 0; PtCt < NumNodes; ++PtCt, Pt = Pt->NextNode)
		{
		PolyRasterSegByteFloodmap(LastLon, LastLat, Pt->Lon, Pt->Lat, FillVal, OffsetX, OffsetY, MapWidth, MapHeight);
		LastLat = Pt->Lat;
		LastLon = Pt->Lon;
		} // for
	} // if

} // GeoRaster::PolyRasterEdgeByteFloodmap

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterEdgeBytePoint(long MaxMem, long &UsedMem, Joe *OutlineMe, short Band, UBYTE FillVal, short OverlapOK, short ConnectBack)
{
GeoRaster *DrawRast = this;
VectorPoint *Pt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

if (Pt = OutlineMe->GetFirstRealPoint())
	{
	if (MyAttr = (JoeCoordSys *)OutlineMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	for (; Pt; Pt = Pt->Next)
		{
		if (Pt->ProjToDefDeg(MyCoords, &MyVert))
			{
			if ((DrawRast = PolyRasterSegByte(MaxMem, UsedMem, MyVert.Lon, MyVert.Lat, MyVert.Lon, MyVert.Lat, Band, FillVal, OverlapOK)) == NULL)
				break;
			} // if
		} // for
	} // if

return (DrawRast);

} // GeoRaster::PolyRasterEdgeBytePoint

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterSegByte(long MaxMem, long &UsedMem, double fx, double fy, double tx, double ty, short Band, UBYTE FillVal, short OverlapOK)
{
double dx, dy, Slope, FltY, FltX, YPt, XPt;
GeoRaster *DrawRast = this;

fx = (W - fx) / CellSizeEW;		// convert to cell units
tx = (W - tx) / CellSizeEW;
fy = (N - fy) / CellSizeNS;
ty = (N - ty) / CellSizeNS;

dx = tx - fx;
dy = ty - fy;

if (fabs(dy) > fabs(dx))
	{
	if (fy > ty)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
		return (NULL);
	if (dy > 0.0)
		{
		Slope = dx / dy;
		Row ++;
		FltY = Row;
		XPt = fx + Slope * (FltY - fy);
		for ( ; FltY < ty && Row < Rows; FltY += 1.0, XPt += Slope, Row ++)
			{
			Col = quickftol(XPt);
			if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
				return (NULL);
			if (! PolyRasterFillCell(Row - 1, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
				return (NULL);
			} // for
		Row = quickftol(ty);
		Col = quickftol(tx);
		if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
			return (NULL);
		} // if
	} // if
else
	{
	if (fx > tx)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
		return (NULL);
	if (dx > 0.0)
		{
		Slope = dy / dx;
		Col ++;
		FltX = Col;
		YPt = fy + Slope * (FltX - fx);
		for ( ; FltX < tx && Col < Cols; FltX += 1.0, YPt += Slope, Col ++)
			{
			Row = quickftol(YPt);
			if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
				return (NULL);
			if (! PolyRasterFillCell(Row, Col - 1, Band, FillVal, OverlapOK, MaxMem, UsedMem))
				return (NULL);
			} // for
		Row = quickftol(ty);
		Col = quickftol(tx);
		if (! PolyRasterFillCell(Row, Col, Band, FillVal, OverlapOK, MaxMem, UsedMem))
			return (NULL);
		} // if
	} // else

return (DrawRast);

} // GeoRaster::PolyRasterSegByte

/*===========================================================================*/

bool GeoRaster::PolyRasterSegPolyList(VectorPolygon *PlotMe, double fx, double fy, double tx, double ty)
{
double dx, dy, Slope, FltY, FltX, YPt, XPt;
bool Success = true;

fx = (W - fx) / CellSizeEW;		// convert to cell units
tx = (W - tx) / CellSizeEW;
fy = (N - fy) / CellSizeNS;
ty = (N - ty) / CellSizeNS;

dx = tx - fx;
dy = ty - fy;

if (fabs(dy) > fabs(dx))
	{
	if (fy > ty)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	if (Success = PlotPolygonPoint(PlotMe, Col, Row, true))
		{
		if (dy > 0.0)
			{
			Slope = dx / dy;
			Row ++;
			FltY = Row;
			XPt = fx + Slope * (FltY - fy);
			for ( ; FltY < ty && Row < Rows && Success; FltY += 1.0, XPt += Slope, ++Row)
				{
				Col = quickftol(XPt);
				if (Success = PlotPolygonPoint(PlotMe, Col, Row, true))
					{
					Success = PlotPolygonPoint(PlotMe, Col, Row - 1, true);
					} // if
				} // for
			if (Success)
				{
				Row = quickftol(ty);
				Col = quickftol(tx);
				Success = PlotPolygonPoint(PlotMe, Col, Row, true);
				} // if
			} // if
		} // if
	} // if
else
	{
	if (fx > tx)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	if (Success = PlotPolygonPoint(PlotMe, Col, Row, true))
		{
		if (dx > 0.0)
			{
			Slope = dy / dx;
			Col ++;
			FltX = Col;
			YPt = fy + Slope * (FltX - fx);
			for ( ; FltX < tx && Col < Cols && Success; FltX += 1.0, YPt += Slope, ++Col)
				{
				Row = quickftol(YPt);
				if (Success = PlotPolygonPoint(PlotMe, Col, Row, true))
					{
					Success = PlotPolygonPoint(PlotMe, Col - 1, Row, true);
					} // if
				} // for
			if (Success)
				{
				Row = quickftol(ty);
				Col = quickftol(tx);
				Success = PlotPolygonPoint(PlotMe, Col, Row, true);
				} // if
			} // if
		} // if
	} // else

return (Success);

} // GeoRaster::PolyRasterSegPolyList

/*===========================================================================*/

// same code as above but with added offset for floodmap and directly filling floodmap
void GeoRaster::PolyRasterSegByteFloodmap(double fx, double fy, double tx, double ty, UBYTE FillVal, 
	long OffsetX, long OffsetY, long MapWidth, long MapHeight)
{
double dx, dy, Slope, FltY, FltX, YPt, XPt;

fx = (W - fx) / CellSizeEW;		// convert to cell units
tx = (W - tx) / CellSizeEW;
fy = (N - fy) / CellSizeNS;
ty = (N - ty) / CellSizeNS;

//floodmap is one pixel wider to the left and top but only encapsulates 
// one vector so is often smaller that full GeoRaster
fx -= OffsetX;
fy -= OffsetY;
tx -= OffsetX;
ty -= OffsetY;

dx = tx - fx;
dy = ty - fy;

if (fabs(dy) > fabs(dx))
	{
	if (fy > ty)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	RasterCell = Row * MapWidth + Col;
	floodmap[RasterCell] = FillVal;
	if (dy > 0.0)
		{
		Slope = dx / dy;
		Row ++;
		FltY = Row;
		XPt = fx + Slope * (FltY - fy);
		for ( ; FltY < ty && Row < MapHeight; FltY += 1.0, XPt += Slope, Row ++)
			{
			Col = quickftol(XPt);
			RasterCell = Row * MapWidth + Col;
			floodmap[RasterCell] = FillVal;
			RasterCell -= MapWidth;
			floodmap[RasterCell] = FillVal;
			} // for
		Row = quickftol(ty);
		Col = quickftol(tx);
		RasterCell = Row * MapWidth + Col;
		floodmap[RasterCell] = FillVal;
		} // if
	} // if
else
	{
	if (fx > tx)
		{
		swmem(&fx, &tx, sizeof (double));
		swmem(&fy, &ty, sizeof (double));
		dy = -dy;
		dx = -dx;
		} // if
	Row = quickftol(fy);
	Col = quickftol(fx);
	RasterCell = Row * MapWidth + Col;
	floodmap[RasterCell] = FillVal;
	if (dx > 0.0)
		{
		Slope = dy / dx;
		Col ++;
		FltX = Col;
		YPt = fy + Slope * (FltX - fx);
		for ( ; FltX < tx && Col < MapWidth; FltX += 1.0, YPt += Slope, Col ++)
			{
			Row = quickftol(YPt);
			RasterCell = Row * MapWidth + Col;
			floodmap[RasterCell] = FillVal;
			RasterCell --;
			floodmap[RasterCell] = FillVal;
			} // for
		Row = quickftol(ty);
		Col = quickftol(tx);
		RasterCell = Row * MapWidth + Col;
		floodmap[RasterCell] = FillVal;
		} // if
	} // else

} // GeoRaster::PolyRasterSegByteFloodmap

/*===========================================================================*/

GeoRaster *GeoRaster::PolyRasterFillCell(long LocalRow, long LocalCol, short Band, UBYTE FillVal, short OverlapOK, long MaxMem, long &UsedMem)
{
GeoRaster *DrawRast = this, *LastRast;
long zip, DrawIt;

if (! OverlapOK)
	{
	ByteMap[Band][LocalRow * Cols + LocalCol] = FillVal;
	} // if no overlapping effects so just blast the pixels in there
else
	{
	zip = LocalRow * Cols + LocalCol;
	DrawIt = 1;
	while (DrawRast && DrawRast->ByteMap[Band][zip])
		{
		if (DrawRast->ByteMap[Band][zip] == FillVal)
			{
			DrawIt = 0;
			break;
			}
		LastRast = DrawRast;
		DrawRast = (GeoRaster *)DrawRast->Next;
		} // while
	if (DrawIt)
		{
		if (! DrawRast)
			{
			if (! (LastRast->Next = new GeoRaster(this, MaxMem, UsedMem)) || LastRast->Next->ConstructError)
				{
				return (NULL);
				} // if
			else
				{
				DrawRast = (GeoRaster *)LastRast->Next;
				} // else
			} // if ran out of maps
		DrawRast->ByteMap[Band][zip] = FillVal;
		} // if
	} // else set each pixel and check to see if need to go on to next map

return (DrawRast);

} // GeoRaster::PolyRasterFillCell

/*===========================================================================*/

//#define FLOODMAP_DUMP

bool GeoRaster::PlotPolygon(VectorPolygon *PlotMe)
{
long MinX, MinY;
bool Success = true;
VectorPart *CurPart;
#ifdef FLOODMAP_DUMP
char TestName[80];
FILE *fDump;
#endif // FLOODMAP_DUMP

if (! PlotMe->FirstPart.NextPart)
	{
	// only one part
	// fill floodmap with 1
	if (AllocFloodmap(PlotMe, 1, MinX, MinY))
		{
#ifdef FLOODMAP_DUMP
sprintf(TestName, "d:/frames/PlotPolygon_1_%d_%d.raw", xsize, ysize);
if (fDump = fopen(TestName, "wb"))
	{
	fwrite(floodmap, 1, xsize * ysize, fDump);
	fclose(fDump);
	} // if
#endif // FLOODMAP_DUMP
		// outline polygon in 0
		PolyRasterEdgeByteFloodmap(&PlotMe->FirstPart, 0, MinX - 1, MinY - 1, xsize, ysize);
#ifdef FLOODMAP_DUMP
sprintf(TestName, "d:/frames/PlotPolygon_2_%d_%d.raw", xsize, ysize);
if (fDump = fopen(TestName, "wb"))
	{
	fwrite(floodmap, 1, xsize * ysize, fDump);
	fclose(fDump);
	} // if
#endif // FLOODMAP_DUMP
		// seed fill from edge to outer polygon in 0
		SeedFill(0);
#ifdef FLOODMAP_DUMP
sprintf(TestName, "d:/frames/PlotPolygon_3_%d_%d.raw", xsize, ysize);
if (fDump = fopen(TestName, "wb"))
	{
	fwrite(floodmap, 1, xsize * ysize, fDump);
	fclose(fDump);
	} // if
#endif // FLOODMAP_DUMP
		// copy to PolyList
		if (PolyRasterCopyFloodPolyList(PlotMe, MinX, MinY))
			{
			// outline polygon in edge value
			Success = PolyRasterEdgePolyList(PlotMe);
			} // if
		else
			Success = false;
		} // if
	else
		Success = false;
	} // if
else
	{
	// more than one part
	// fill floodmap with 0
	if (AllocFloodmap(PlotMe, 0, MinX, MinY))
		{
		for (CurPart = PlotMe->FirstPart.NextPart; CurPart; CurPart = CurPart->NextPart)
			{
			// draw hole outlines in 1
			PolyRasterEdgeByteFloodmap(CurPart, 1, MinX - 1, MinY - 1, xsize, ysize);
			} // for
		// seed fill from outer edge to outlines in 1
		SeedFill(1);
		// draw outer polygon in 0
		PolyRasterEdgeByteFloodmap(&PlotMe->FirstPart, 0, MinX - 1, MinY - 1, xsize, ysize);
		// seed fill to outer polygon in 0
		SeedFill(0);
		// copy to PolyList
		if (PolyRasterCopyFloodPolyList(PlotMe, MinX, MinY))
			{
			// outline all parts of the polygon
			if (! PolyRasterEdgePolyList(PlotMe))
				Success = false;
			} // if
		else
			Success = false;
		} // if
	else
		Success = false;
	} // else
FreeFloodmap();
	
return (Success);

} // GeoRaster::PlotPolygon

/*===========================================================================*/

bool GeoRaster::PlotPolygonPoint(VectorPolygon *PlotMe, long x, long y, bool OnEdge)
{
long CellZip = y * Cols + x;

return (PlotPolygonPoint(PlotMe, &PolyListBlock[CellZip], OnEdge));

} // GeoRaster::PlotPolygonPoint

/*===========================================================================*/

bool GeoRaster::PlotPolygonPoint(VectorPolygon *PlotMe, VectorPolygonList **PlotCell, bool OnEdge)
{
bool Success = true, Found;
VectorPolygonList *CurList;

if (! *PlotCell)
	{
	if (*PlotCell = new VectorPolygonList())
		{
		CurList = *PlotCell;
		CurList->MyPolygon = PlotMe;
		if (OnEdge)
			CurList->FlagSet(WCS_VECTORPOLYGONLIST_FLAG_EDGE);
		} // if
	else
		Success = false;
	} // if
else
	{
	CurList = *PlotCell;
	// is it already there?
	if (CurList->MyPolygon != PlotMe)
		{
		Found = false;
		while (CurList->NextPolygonList)
			{
			CurList = CurList->NextPolygonList;
			if (CurList->MyPolygon == PlotMe)
				{
				Found = true;
				break;
				} // if
			} // for
		if (! Found)
			{
			if (CurList->NextPolygonList = new VectorPolygonList())
				{
				CurList = CurList->NextPolygonList;
				CurList->MyPolygon = PlotMe;
				} // if
			else
				Success = false;
			} // if
		} // if
	if (Success && OnEdge)
		CurList->FlagSet(WCS_VECTORPOLYGONLIST_FLAG_EDGE);
	} // else

return (Success);

} // GeoRaster::PlotPolygonPoint

/*===========================================================================*/
/*===========================================================================*/

// Retrieving values

long GeoRaster::GetCell(double Lat, double Lon)
{
double dTemp;

if ((dTemp = N - Lat) > 0.0)
	{
	Row = quickftol(dTemp / CellSizeNS);
	if (Row < Rows)
		{
		if ((dTemp = W - Lon) > 0.0)
			{
			Col = quickftol(dTemp / CellSizeEW);
			if (Col < Cols)
				return (Row * Cols + Col);
			} // if
		} // if
	} // if

return (-1);

} // GeoRaster::GetCell

/*===========================================================================*/

long GeoRaster::GetValidByteCell(short Band, double Lat, double Lon)
{
double dTemp;

if ((dTemp = N - Lat) > 0.0)
	{
	Row = quickftol(dTemp / CellSizeNS);
	if (Row < Rows)
		{
		if ((dTemp = W - Lon) > 0.0)
			{
			Col = quickftol(dTemp / CellSizeEW);
			if (Col < Cols)
				{
				RasterCell = Row * Cols + Col;
				if (ByteMap[Band][RasterCell])
					return (RasterCell);
				} // if
			} // if
		} // if
	} // if

return (-1);

} // GeoRaster::GetValidByteCell

/*===========================================================================*/

long GeoRaster::GetValidFloatCell(short Band, double Lat, double Lon)
{
double dTemp;

if ((dTemp = N - Lat) > 0.0)
	{
	Row = quickftol(dTemp / CellSizeNS);
	if (Row < Rows)
		{
		if ((dTemp = W - Lon) > 0.0)
			{
			Col = quickftol(dTemp / CellSizeEW);
			if (Col < Cols)
				{
				RasterCell = Row * Cols + Col;
				if (FloatMap[Band][RasterCell] != FLT_MIN)
					return (RasterCell);
				} // if
			} // if
		} // if
	} // if

return (-1);

} // GeoRaster::GetValidFloatCell

/*===========================================================================*/

CornerMask GeoRaster::GetByteCellMask(short Band, double Lat, double Lon)
{
double dTemp;
CornerMask CMask = 0;
short NextColOK, NextRowOK;

if ((dTemp = N - Lat) > 0.0)
	{
	Row = quickftol(dTemp / CellSizeNS);
	if (Row < Rows)
		{
		if ((dTemp = W - Lon) > 0.0)
			{
			Col = quickftol(dTemp / CellSizeEW);
			if (Col < Cols)
				{
				RasterCell = Row * Cols + Col;
				NextRowOK = Row < Rows - 1;
				NextColOK = Col < Cols - 1;
				if (ByteMap[Band][RasterCell])
					 CMask |= (1 << 0);
				if (NextColOK && ByteMap[Band][RasterCell + 1])
					 CMask |= (1 << 1);
				if (NextColOK && NextRowOK && ByteMap[Band][RasterCell + 1 + Cols])
					 CMask |= (1 << 2);
				if (NextRowOK && ByteMap[Band][RasterCell + Cols])
					 CMask |= (1 << 3);
				} // if
			} // if
		} // if
	} // if

return (CMask);

} // GeoRaster::GetByteCellMask

/*===========================================================================*/

CornerMask GeoRaster::GetFloatCellMask(short Band, double Lat, double Lon)
{
double dTemp;
CornerMask CMask = 0;
short NextColOK, NextRowOK;

if ((dTemp = N - Lat) > 0.0)
	{
	Row = quickftol(dTemp / CellSizeNS);
	if (Row < Rows)
		{
		if ((dTemp = W - Lon) > 0.0)
			{
			Col = quickftol(dTemp / CellSizeEW);
			if (Col >= 0 && Col < Cols)
				{
				RasterCell = Row * Cols + Col;
				NextRowOK = Row < Rows - 1;
				NextColOK = Col < Cols - 1;
				if (FloatMap[Band][RasterCell] != FLT_MIN)
					 CMask |= (1 << 0);
				if (NextColOK && FloatMap[Band][RasterCell + 1] != FLT_MIN)
					 CMask |= (1 << 1);
				if (NextColOK && NextRowOK && FloatMap[Band][RasterCell + 1 + Cols] != FLT_MIN)
					 CMask |= (1 << 2);
				if (NextRowOK && FloatMap[Band][RasterCell + Cols] != FLT_MIN)
					 CMask |= (1 << 3);
				} // if
			} // if
		} // if
	} // if

return (CMask);

} // GeoRaster::GetFloatCellMask

/*===========================================================================*/

double GeoRaster::GetByteMaskInterp(short Band, double Lat, double Lon)
{
double LonOff, LatOff, LonInvOff, LatInvOff, Wt,  Sum = 0.0, SumWt = 0.0;
double rVal = 0.0;

if (! Mask)
	return (rVal);

LatOff = N - Row * CellSizeNS - Lat;
LonOff = W - Col * CellSizeEW - Lon;

LonInvOff = CellSizeEW - LonOff;
LatInvOff = CellSizeNS - LatOff;

if (Mask & (1 << 0))
	{
	Wt = LatInvOff * LonInvOff / CellArea;
	Sum += (ByteMap[Band][RasterCell] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 1))
	{
	Wt = LonOff * LatInvOff / CellArea;
	Sum += (ByteMap[Band][RasterCell + 1] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 2))
	{
	Wt = LatOff * LonOff / CellArea;
	Sum += (ByteMap[Band][RasterCell + 1 + Cols] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 3))
	{
	Wt = LatOff * LonInvOff / CellArea;
	Sum += (ByteMap[Band][RasterCell + Cols] * Wt);
	SumWt += Wt;
	} // if

if (SumWt > 0.0)
	rVal = Sum / SumWt;

return (rVal);

} // GeoRaster::GetByteMaskInterp

/*===========================================================================*/

double GeoRaster::GetFloatMaskInterp(short Band, double Lat, double Lon)
{
double LonOff, LatOff, LonInvOff, LatInvOff, Wt, Sum = 0.0, SumWt = 0.0;
double rVal = 0.0;

if (! Mask)
	return (rVal);

LatOff = N - Row * CellSizeNS - Lat;
LonOff = W - Col * CellSizeEW - Lon;

LonInvOff = CellSizeEW - LonOff;
LatInvOff = CellSizeNS - LatOff;

if (Mask & (1 << 0))
	{
	Wt = LatInvOff * LonInvOff / CellArea;
	Sum += (FloatMap[Band][RasterCell] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 1))
	{
	Wt = LonOff * LatInvOff / CellArea;
	Sum += (FloatMap[Band][RasterCell + 1] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 2))
	{
	Wt = LatOff * LonOff / CellArea;
	Sum += (FloatMap[Band][RasterCell + 1 + Cols] * Wt);
	SumWt += Wt;
	} // if
if (Mask & (1 << 3))
	{
	Wt = LatOff * LonInvOff / CellArea;
	Sum += (FloatMap[Band][RasterCell + Cols] * Wt);
	SumWt += Wt;
	} // if

if (SumWt > 0.0)
	rVal = Sum / SumWt;

return (rVal);

} // GeoRaster::GetFloatMaskInterp

/*===========================================================================*/
