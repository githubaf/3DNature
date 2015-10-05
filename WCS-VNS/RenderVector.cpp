// RenderVector.cpp
// Rendering vectors in World Construction Set 
// Built from scratch 12/6/99 by Gary R. Huber.
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Render.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "RenderControlGUI.h"
#include "Useful.h"
#include "AppMem.h"
#include "Database.h"
#include "Joe.h"
#include "Illustrator.h"
#include "ImageFormat.h"
#include "Log.h"
#include "PixelManager.h"

#ifdef DEBUG
// Put a big black dot at the vertex center - handy for debugging
//#define VECT_DOT_THE_MIDDLE
#endif

class ImageFormatAI;

extern ImageFormatAI *VectorOutput; // are we setup to write vectors to a file?

// image size is the total image size being passed along to the Illustrator file saver for clipping purposes
int Renderer::RenderVectors(long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, rPixelHeader *rPixelFragMap)
{
int Success = 1;
unsigned long NumObjs;
long Step;
BusyWin *BWDE = NULL;
Joe *VecWalk;
PixelData Pix;
unsigned char HazeEnabledStash, VAStash;

if ((NumObjs = DBase->HowManyObjs()) > 0)
	{
	if (IsCamView)
		{
		BWDE = new BusyWin("Vectors", NumObjs, 'BWDE', 0);
		} // if
	else
		{
		Master->ProcInit(NumObjs, "Vectors");
		} // else

	ClearCloudBuffers();
	Pix.SetDefaults();
	Pix.PixelType = WCS_PIXELTYPE_TERRAIN;
	Pix.TexData = &RendData.TexData;
	ZMergeDistance = 2.0;	// seems like a reasonable number for merging nearby vectors
	// disable atmospheres if no haze vectors
	HazeEnabledStash = Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE];
	VAStash = VolumetricAtmospheresExist;
	if (! Opt->HazeVectors)
		{
		Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = 0;
		VolumetricAtmospheresExist = 0;
		} // if

	for (Step = 0, VecWalk = DBase->GetFirst(); VecWalk && Success; VecWalk = DBase->GetNext(VecWalk))
		{
		if (! VecWalk->TestFlags(WCS_JOEFLAG_HASKIDS))
			{
			if (! VecWalk->TestFlags(WCS_JOEFLAG_ISDEM))
				{
				if (VecWalk->TestRenderFlags())
					{
					if (VecWalk->GetNumRealPoints() > 0)
						{
						Success = PrepVectorToRender(VecWalk, &Pix, TileWidth, TileHeight, TotalWidth, TotalHeight, rPixelFragMap);
						} // if
					} // if
				} // if
			} // if

		if (IsCamView)
			{
			if (BWDE)
				{
				if(BWDE->Update(Step ++))
					{
					Success = 0;
					break;
					} // if
				} // if
			} // if
		else
			{
			Master->ProcUpdate(Step ++);
			if (! Master->IsRunning())
				{
				Success = 0;
				break;
				} // if
			} // else
		} // for

	if (Master)
		Master->ProcClear();
	if (BWDE)
		delete BWDE;

	Opt->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] = HazeEnabledStash;
	VolumetricAtmospheresExist = VAStash;
	} // if

return (Success);

} // Renderer::RenderVectors

/*===========================================================================*/

static double  vwidth;			// vector width, vector width / 2
static double inside, outside;	// radii squared for Wu circle antialiasing
	
static double DeltaLatX, DeltaLatY, DeltaLonX, DeltaLonY, DeltaElevX, DeltaElevY, DeltaZX, DeltaZY, 
	DeltaXYZ_X[3], DeltaXYZ_Y[3], DeltaQX, DeltaQY;

/*===========================================================================*/

int Renderer::PrepVectorToRender(Joe *DrawMe, PixelData *Pix, long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, rPixelHeader *rPixelFragMap)
{
CoordSys *MyCoords;
JoeCoordSys *MyAttr;
VectorPoint *VectPt;
int Success = 1;	// set Success = 0 if rendering error
unsigned short Style;
VertexDEM FromVertex, ToVertex, MyVert;
#ifdef VECT_DOT_THE_MIDDLE
double tmprgb[3], tmpz;
#endif // VECT_DOT_THE_MIDDLE

if (DrawMe->GetFirstRealPoint() == NULL)	// label only - it's been known to happen!
	return Success;					// not a rendering error, just nothing to do

if (MyAttr = (JoeCoordSys *)DrawMe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
	MyCoords = MyAttr->Coord;
else
	MyCoords = NULL;

if(VectorOutput)
	{
	VectorOutput->WriteIllustratorVector(DefCoords, Cam, DefaultPlanetOpt, WCS_ILLUSTRATOR_DEFAULT_DPI, WCS_ILLUSTRATOR_APPROX_RASTER_DPI,
	  DrawMe->GetBestName(), DrawMe, DrawMe->GetNumRealPoints(), DrawMe->GetLineWidth(),
	 DrawMe->Red(), DrawMe->Green(), DrawMe->Blue(), EarthLatScaleMeters, PlanetRad, CenterPixelSize, TileWidth, TileHeight, TotalWidth, TotalHeight, rPixelFragMap);
	return(Success);
	} // if

vwidth = DrawMe->GetLineWidth();

#ifdef WCS_BUILD_RTX
if (Exporter)
	{
	vwidth *= Exporter->AnimPar[WCS_EFFECTS_SCENEEXPORTER_ANIMPAR_VECWIDTHMULT].CurValue;
	} // if
#endif // WCS_BUILD_RTX

if (vwidth == 0.0)
	return Success;					// nothing to plot

Style = DrawMe->AttribInfo.LineStyle;

inside = (vwidth - 1.0) * 0.5 - 0.5;
inside *= inside;
outside = (vwidth + 1.0) * 0.5 - 0.5;
outside *= outside;

FromVertex.Flags = 0;

Pix->Object = DrawMe;
Pix->RGB[0] = DrawMe->Red() * ( 1.0 / 255.0);
Pix->RGB[1] = DrawMe->Green() * (1.0 / 255.0);
Pix->RGB[2] = DrawMe->Blue() * (1.0 / 255.0);
if (Opt->LuminousVectors)
	Pix->Luminosity = 1.0;

VectPt = DrawMe->GetFirstRealPoint();	// skip label point
if (VectPt->ProjToDefDeg(MyCoords, &FromVertex))
	{
	FromVertex.Elev = CalcExag(FromVertex.Elev, DefaultPlanetOpt);
	Cam->ProjectVertexDEM(DefCoords, &FromVertex, EarthLatScaleMeters, PlanetRad, 1);
	if ((float)FromVertex.ScrnXYZ[2] > 0.0f && FromVertex.ScrnXYZ[2] >= MinimumZ && FromVertex.ScrnXYZ[2] <= MaximumZ)
		{
		#ifdef VECT_DOT_THE_MIDDLE
		tmprgb[0] = Pix->RGB[0];
		tmprgb[1] = Pix->RGB[1];
		tmprgb[2] = Pix->RGB[2];
		Pix->RGB[0] = Pix->RGB[1] = Pix->RGB[2] = 0.0;
		tmpz = FromVertex.ScrnXYZ[2];
		FromVertex.ScrnXYZ[2] = 1.0;	// make sure it's close so it won't get overwritten
		InverseFromZ = 1.0;
		VectorDrawWuPixel(FromVertex.ScrnXYZ[0], FromVertex.ScrnXYZ[1], 0, 0.0, 0.0, &FromVertex, Pix);
		Pix->RGB[0] = tmprgb[0];
		Pix->RGB[1] = tmprgb[1];
		Pix->RGB[2] = tmprgb[2];
		FromVertex.ScrnXYZ[2] = tmpz;
		#endif // VECT_DOT_THE_MIDDLE

		InverseFromZ = 1.0 / FromVertex.ScrnXYZ[2];

		switch (Style)
			{
			case 0:		// Point
				VectorDrawWuPixel(FromVertex.ScrnXYZ[0], FromVertex.ScrnXYZ[1], 0, 0.0, 0.0, &FromVertex, Pix);
				break;
			case 2:		// Square
				VectorWuSquare(&FromVertex, Pix);
				break;
			case 3:		// Cross
				VectorWuCross(&FromVertex, Pix);
				break;
			default:	// All line modes get circle on end
			case 1: // Circle
				VectorWuCircle(&FromVertex, Pix);
				break;
			} // switch
		} // if
	} // if

// there is similar code to this in Illustrator.cpp, ImageFormatAI::WriteIllustratorVector
for (VectPt = VectPt->Next; VectPt; VectPt = VectPt->Next)
	{
	if (VectPt->ProjToDefDeg(MyCoords, &ToVertex))
		{
		ToVertex.Elev = CalcExag(ToVertex.Elev, DefaultPlanetOpt);
		Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
		// these two tests are complimentary so that a seamless rendering can be compiled
#ifdef WCS_BUILD_VNS
		// Test to see if projected planimetric. If so, we may subdivide this vector more.
		if ((Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) && Cam->Projected)
			{
			// If distance is > sqrt(2) pixels, subdivide.
			if (fabs(FromVertex.ScrnXYZ[0] - ToVertex.ScrnXYZ[0]) + fabs(FromVertex.ScrnXYZ[1] - ToVertex.ScrnXYZ[1]) > 4.0)
				{
				double invparts, partnum = 1.0, parts = 1.0;
				double dLat, dLon, dElev;
				VertexDEM OrigToVertex = ToVertex;

				dLat = OrigToVertex.Lat - FromVertex.Lat;
				dLon = OrigToVertex.Lon - FromVertex.Lon;
				dElev = OrigToVertex.Elev - FromVertex.Elev;
				do
					{
					parts = parts * 2.0;
					if (parts > 1024.0)
						break;
					invparts = 1.0 / parts;
					ToVertex.Lat = FromVertex.Lat + dLat * invparts;
					ToVertex.Lon = FromVertex.Lon + dLon * invparts;
					ToVertex.Elev = FromVertex.Elev + dElev * invparts;
					Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
					} while (fabs(FromVertex.ScrnXYZ[0] - ToVertex.ScrnXYZ[0]) + fabs(FromVertex.ScrnXYZ[1] - ToVertex.ScrnXYZ[1]) > 4.0);

				invparts = 1.0 / parts;
				dLat = dLat * invparts;
				dLon = dLon * invparts;
				dElev = dElev * invparts;

				// now draw first n - 1 parts, then fall thru to plot last piece
				ToVertex.Lat = FromVertex.Lat + dLat;
				ToVertex.Lon = FromVertex.Lon + dLon;
				ToVertex.Elev = FromVertex.Elev + dElev;
				Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
				while (partnum < parts)
					{
					if ((FromVertex.ScrnXYZ[2] >= MinimumZ && ToVertex.ScrnXYZ[2] >= MinimumZ) && 
						(FromVertex.ScrnXYZ[2] <= MaximumZ || ToVertex.ScrnXYZ[2] <= MaximumZ))
						VectorDrawWuLine(&FromVertex, &ToVertex, Pix, Style);
					FromVertex.CopyLatLon(&ToVertex);
					FromVertex.CopyXYZ(&ToVertex);
					FromVertex.CopyScrnXYZQ(&ToVertex);
					ToVertex.Lat = FromVertex.Lat + dLat;
					ToVertex.Lon = FromVertex.Lon + dLon;
					ToVertex.Elev = FromVertex.Elev + dElev;
					Cam->ProjectVertexDEM(DefCoords, &ToVertex, EarthLatScaleMeters, PlanetRad, 1);
					partnum += 1.0;
					} // while
				} // if subdividing
			} // if projected planimetric
#endif // WCS_BUILD_VNS
		if ((FromVertex.ScrnXYZ[2] >= MinimumZ && ToVertex.ScrnXYZ[2] >= MinimumZ) && 
			(FromVertex.ScrnXYZ[2] <= MaximumZ || ToVertex.ScrnXYZ[2] <= MaximumZ))
			VectorDrawWuLine(&FromVertex, &ToVertex, Pix, Style);
		FromVertex.CopyLatLon(&ToVertex);
		FromVertex.CopyXYZ(&ToVertex);
		FromVertex.CopyScrnXYZQ(&ToVertex);
		#ifdef VECT_DOT_THE_MIDDLE
		if ((float)FromVertex.ScrnXYZ[2] > 0.0f && FromVertex.ScrnXYZ[2] >= MinimumZ && FromVertex.ScrnXYZ[2] <= MaximumZ)
			{
			tmprgb[0] = Pix->RGB[0];
			tmprgb[1] = Pix->RGB[1];
			tmprgb[2] = Pix->RGB[2];
			Pix->RGB[0] = Pix->RGB[1] = Pix->RGB[2] = 0.0;
			tmpz = FromVertex.ScrnXYZ[2];
			FromVertex.ScrnXYZ[2] = 1.0;	// make sure it's close so it won't get overwritten
			InverseFromZ = 1.0;
			VectorDrawWuPixel(FromVertex.ScrnXYZ[0], FromVertex.ScrnXYZ[1], 0, 0.0, 0.0, &FromVertex, Pix);
			Pix->RGB[0] = tmprgb[0];
			Pix->RGB[1] = tmprgb[1];
			Pix->RGB[2] = tmprgb[2];
			FromVertex.ScrnXYZ[2] = tmpz;
			} // if
		#endif // VECT_DOT_THE_MIDDLE
		} // if
	} // for

return (Success);

} // Renderer::PrepVectorToRender

/*===========================================================================*/

void Renderer::VectorDrawAAPixel(double x, double y, USHORT wu, double dx, double dy, 
	VertexDEM *FromVDEM, PixelData *Pix)
{
double fx, fy, ix, iy, mx, my, cx, cy;
double tli, tri, bli, bri;		// intensity at top left, top right, bottom left, and bottom right
double i;						// intensity
USHORT tlwu, trwu, blwu, brwu;	// wu's for the above

i = (255 - wu) * (1.0 / 255.0);
mx = x + 0.5;					// modified x
my = y + 0.5;					// modified y
fx = WCS_floor(x - 0.5);			// left coord
fy = WCS_floor(y - 0.5);			// top coord
cx = WCS_floor(x + 0.5);			// right coord
cy = WCS_floor(y + 0.5);			// bottom coord
ix = mx - WCS_floor(mx);			// intensity of right
iy = my - WCS_floor(my);			// intensity of bottom
tli = (1 - ix) * (1 - iy) * i;
tri = ix * (1 - iy) * i;
bli = (1 - ix) * iy * i;
bri = ix * iy * i;
tlwu = (USHORT)(255.0 - (tli * 255.0));
trwu = (USHORT)(255.0 - (tri * 255.0));
blwu = (USHORT)(255.0 - (bli * 255.0));
brwu = (USHORT)(255.0 - (bri * 255.0));
// now plot the pixels that make up the antialiased pixel
VectorDrawWuPixel(fx, fy, tlwu, dx, dy, FromVDEM, Pix);
VectorDrawWuPixel(fx, cy, trwu, dx, dy, FromVDEM, Pix);
VectorDrawWuPixel(cx, fy, blwu, dx, dy, FromVDEM, Pix);
VectorDrawWuPixel(cx, cy, brwu, dx, dy, FromVDEM, Pix);

} // Renderer::VectorDrawAAPixel

/*===========================================================================*/

void Renderer::VectorDrawWuPixel(double x, double y, USHORT wu, double dx, double dy, 
	VertexDEM *FromVDEM, PixelData *Pix)
{

VectorDrawWuPixel2((int)WCS_floor(x), (int)WCS_floor(y), wu, (short)dx, (short)dy, FromVDEM, Pix);

} // Renderer::VectorDrawWuPixel

/*===========================================================================*/

void Renderer::VectorDrawWuPixel2(int x, int y, USHORT wu, short dx, short dy, 
	VertexDEM *FromVDEM, PixelData *Pix)
{
double RGBStash[3];
rPixelFragment *PixFrag;
long PixVal[3],	PixZip, OldBits, NewBits, PolyBits, TotalBits, ReplaceValues;
int PixWt;
bool TexturesExist;
unsigned char FragFlags;

// renderer scope variables: LightTexturesExist and AtmoTexturesExist
TexturesExist = (LightTexturesExist || AtmoTexturesExist);

FragFlags = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR;
#ifdef WCS_BUILD_RTX
if (Exporter)
	FragFlags |= (WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED | WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED);
#endif // WCS_BUILD_RTX


if ((x < 0) || (y < 0) || (x >= Width) || (y >= Height) || (wu >= 255))
	return;

if (! CamPlanOrtho)
	{
	Pix->Zflt = (float)(InverseFromZ + dx * DeltaZX + dy * DeltaZY);
	Pix->Zflt = 1.0f / Pix->Zflt;
	} // if
else
	Pix->Zflt = (float)(FromVDEM->ScrnXYZ[2] + dx * DeltaZX + dy * DeltaZY);
if (Pix->Zflt > 0)
	{
	// Illuminate pixel will modify the color, need to restore it
	RGBStash[0] = Pix->RGB[0];
	RGBStash[1] = Pix->RGB[1];
	RGBStash[2] = Pix->RGB[2];

	if (! CamPlanOrtho)
		{
		Pix->Lat = FromVDEM->Lat * InverseFromZ + dx * DeltaLatX + dy * DeltaLatY;
		Pix->Lon = FromVDEM->Lon * InverseFromZ + dx * DeltaLonX + dy * DeltaLonY;
		Pix->Elev = FromVDEM->Elev * InverseFromZ + dx * DeltaElevX + dy * DeltaElevY;
		Pix->Q = FromVDEM->Q * InverseFromZ + dx * DeltaQX + dy * DeltaQY;
		Pix->XYZ[0] = FromVDEM->XYZ[0] * InverseFromZ + dx * DeltaXYZ_X[0] + dy * DeltaXYZ_Y[0];
		Pix->XYZ[1] = FromVDEM->XYZ[1] * InverseFromZ + dx * DeltaXYZ_X[1] + dy * DeltaXYZ_Y[1];
		Pix->XYZ[2] = FromVDEM->XYZ[2] * InverseFromZ + dx * DeltaXYZ_X[2] + dy * DeltaXYZ_Y[2];
		Pix->Lat *= Pix->Zflt;
		Pix->Lon *= Pix->Zflt;
		Pix->Elev *= Pix->Zflt;
		Pix->Q *= Pix->Zflt;
		Pix->XYZ[0] *= Pix->Zflt;
		Pix->XYZ[1] *= Pix->Zflt;
		Pix->XYZ[2] *= Pix->Zflt;
		} // if
	else
		{
		Pix->Lat = FromVDEM->Lat + dx * DeltaLatX + dy * DeltaLatY;
		Pix->Lon = FromVDEM->Lon + dx * DeltaLonX + dy * DeltaLonY;
		Pix->Elev = FromVDEM->Elev + dx * DeltaElevX + dy * DeltaElevY;
		Pix->Q = FromVDEM->Q + dx * DeltaQX + dy * DeltaQY;
		Pix->XYZ[0] = FromVDEM->XYZ[0] + dx * DeltaXYZ_X[0] + dy * DeltaXYZ_Y[0];
		Pix->XYZ[1] = FromVDEM->XYZ[1] + dx * DeltaXYZ_X[1] + dy * DeltaXYZ_Y[1];
		Pix->XYZ[2] = FromVDEM->XYZ[2] + dx * DeltaXYZ_X[2] + dy * DeltaXYZ_Y[2];
		} // else
	Pix->Normal[0] = Pix->XYZ[0];
	Pix->Normal[1] = Pix->XYZ[1];
	Pix->Normal[2] = Pix->XYZ[2];
	UnitVector(Pix->Normal);
	Pix->ViewVec[0] = Cam->CamPos->XYZ[0] - Pix->XYZ[0];
	Pix->ViewVec[1] = Cam->CamPos->XYZ[1] - Pix->XYZ[1];
	Pix->ViewVec[2] = Cam->CamPos->XYZ[2] - Pix->XYZ[2];
	UnitVector(Pix->ViewVec); // Illumination code requires this to be unitized in advance to save time later

	if (TexturesExist)
		{
		Pix->TexData->PixelX[0] = (double)x + SegmentOffsetX - Width * Cam->PanoPanels * .5;
		Pix->TexData->PixelX[1] = (double)(x + 1) + SegmentOffsetX - Width * Cam->PanoPanels * .5;
		Pix->TexData->PixelY[0] = -((double)y + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
		Pix->TexData->PixelY[1] = -((double)(y + 1) + SegmentOffsetY) + Height * Opt->RenderImageSegments * .5;
		Pix->TexData->PixelUnityX[0] = Pix->TexData->PixelX[0] / (Width * Cam->PanoPanels);
		Pix->TexData->PixelUnityX[1] = Pix->TexData->PixelX[1] / (Width * Cam->PanoPanels);
		Pix->TexData->PixelUnityY[0] = Pix->TexData->PixelY[0] / (Height * Opt->RenderImageSegments);
		Pix->TexData->PixelUnityY[1] = Pix->TexData->PixelY[1] / (Height * Opt->RenderImageSegments);
		RendData.TransferTextureData(Pix);
		} // if
	// shed some light on the subject
	IlluminatePixel(Pix);

	// offset Z value
	Pix->Zflt = Pix->Zflt - (float)Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET].CurValue;
	if (Pix->Zflt < 0.0f)
		Pix->Zflt = 0.0f;

	PixZip = y * Width + x;

	OldBits = TreeAABuf[PixZip];
	PolyBits = AABuf[PixZip];
	NewBits = 255 - wu;

	if (! rPixelFragMap)
		{
		ReplaceValues = 0;
		// clip color to allowable range
		if (Pix->RGB[0] > 1.0)
			Pix->RGB[0] = 1.0;
		if (Pix->RGB[1] > 1.0)
			Pix->RGB[1] = 1.0;
		if (Pix->RGB[2] > 1.0)
			Pix->RGB[2] = 1.0;

		// plot it
		if (Pix->Zflt >= TreeZBuf[PixZip])					// new pixel farther than old pixel
			{
			if (OldBits >= 255 || (Pix->Zflt > ZBuf[PixZip] && PolyBits >= 255))
				goto EndPlot;
			NewBits -= OldBits;
			if (NewBits > 0)
				{
				TotalBits = NewBits + OldBits;
				PixVal[0] = (long)(Pix->RGB[0] * 255.0);
				PixVal[1] = (long)(Pix->RGB[1] * 255.0);
				PixVal[2] = (long)(Pix->RGB[2] * 255.0);
				TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
				TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
				TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
				TreeAABuf[PixZip] = (unsigned char)TotalBits;
				} // if
			} // if Pix->Zflt
		else										// new pixel closer than old pixel
			{
			if (Pix->Zflt > ZBuf[PixZip] && PolyBits >= 255)
				goto EndPlot;
			OldBits -= NewBits;

			PixVal[0] = (long)(Pix->RGB[0] * 255.0);
			PixVal[1] = (long)(Pix->RGB[1] * 255.0);
			PixVal[2] = (long)(Pix->RGB[2] * 255.0);
			if (OldBits > 0)
				{
				TotalBits = NewBits + OldBits;
				TreeBitmap[0][PixZip] = (unsigned char)((TreeBitmap[0][PixZip] * OldBits + PixVal[0] * NewBits) / TotalBits);
				TreeBitmap[1][PixZip] = (unsigned char)((TreeBitmap[1][PixZip] * OldBits + PixVal[1] * NewBits) / TotalBits);
				TreeBitmap[2][PixZip] = (unsigned char)((TreeBitmap[2][PixZip] * OldBits + PixVal[2] * NewBits) / TotalBits);
				TreeAABuf[PixZip] = (unsigned char)TotalBits;
				} // if
			else
				{
				TotalBits = NewBits;
				TreeBitmap[0][PixZip] = (unsigned char)PixVal[0];
				TreeBitmap[1][PixZip] = (unsigned char)PixVal[1];
				TreeBitmap[2][PixZip] = (unsigned char)PixVal[2];
				} // else
			TreeAABuf[PixZip] = (unsigned char)TotalBits;
			TreeZBuf[PixZip] = Pix->Zflt;
			if (Pix->Zflt < ZBuf[PixZip])
				{
				ReplaceValues = 1;
				} // if
			} // else Pix->Zflt
		ScreenPixelPlotTwoBuf(Bitmap, ZBuf, AABuf, TreeBitmap, TreeZBuf, TreeAABuf, x + DrawOffsetX, y + DrawOffsetY, PixZip);
		} // if
	else
		{
		ReplaceValues = (Pix->Zflt <= rPixelFragMap[PixZip].GetFirstZ());
		} // else
	if (ReplaceValues)
		{
		if (LatBuf)
			LatBuf[PixZip] = (float)(Pix->Lat - TexRefLat);
		if (LonBuf)
			LonBuf[PixZip] = (float)(Pix->Lon - TexRefLon);
		if (IllumBuf)
			IllumBuf[PixZip] = (float)Pix->Illum;
		if (rPixelFragMap || (! ReflectionBuf || ! ReflectionBuf[PixZip] || TotalBits >= 255))
			{
			if (NormalBuf[0])
				NormalBuf[0][PixZip] = (float)Pix->Normal[0];
			if (NormalBuf[1])
				NormalBuf[1][PixZip] = (float)Pix->Normal[1];
			if (NormalBuf[2])
				NormalBuf[2][PixZip] = (float)Pix->Normal[2];
			} // if
		if (ObjectBuf)
			ObjectBuf[PixZip] = Pix->Object;
		if (ObjTypeBuf)
			ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR;
		if (ElevBuf)
			ElevBuf[PixZip] = (float)Pix->Elev;
		} // if

	if (rPixelFragMap)
		{
		PixWt = NewBits;
		if (PixWt > 255)
			PixWt = 255;
		if (PixFrag = rPixelFragMap[PixZip].PlotPixel(rPixelBlock, (float)Pix->Zflt, (unsigned char)PixWt, ~0UL, ~0UL, FragmentDepth, 
			FragFlags))
			{
			PixFrag->PlotPixel(rPixelBlock, Pix->RGB, Pix->Reflectivity, Pix->Normal);
			ScreenPixelPlotFragments(&rPixelFragMap[PixZip], x + DrawOffsetX, y + DrawOffsetY);
			} // if
		} // if

EndPlot:
	// restore pixel color since it is set only once per object
	Pix->RGB[0] = RGBStash[0];
	Pix->RGB[1] = RGBStash[1];
	Pix->RGB[2] = RGBStash[2];
	} // if

} // Renderer::VectorDrawWuPixel2

/*===========================================================================*/

void Renderer::VectorDrawWuLine(VertexDEM *FromVDEM, VertexDEM *ToVDEM, PixelData *Pix, unsigned short Style)
{
double dx, dy, dx2, dy2, hype2, px0, py0, px1, py1;
double change, InverseToZ;
//double fvwidth = vwidth;
double fvwidth2 = vwidth * 0.5;
double xx, yy;
//double tli, ti, tri, ri, bri, bi, bli, li;			// intensity %'s - top left, top, top right, right, etc.
//USHORT tlwu, twu, trwu, rwu, brwu, bwu, blwu, lwu;	// the corresponding wu values
double xl, xr, yb, yt;								// xleft, xright, ybottom, ytop
double r, t, xfac, yfac, xkj, ykj, xlk, ylk;
double denom, fvwidth2i, fvwidth2o, grrr;
USHORT wu;
SHORT xdir;

// don't try to render segments that go into negative Z, they will cross the image
if ((float)FromVDEM->ScrnXYZ[2] <= 0.0f || (float)ToVDEM->ScrnXYZ[2] <= 0.0f)
	return;

fvwidth2i = (vwidth * 0.5) - 0.5;
fvwidth2o = (vwidth * 0.5) + 0.5;
px0 = FromVDEM->ScrnXYZ[0];
py0 = FromVDEM->ScrnXYZ[1];
px1 = ToVDEM->ScrnXYZ[0];
py1 = ToVDEM->ScrnXYZ[1];

/*** F2 NOTE: Is this needed anymore? - YES!!! 04/12/01 ***/
// draw from top to bottom only
if (py0 > py1)
	{
	Swap64(py0, py1);
	Swap64(px0, px1);
	}

// initialize delta components
dx = px1 - px0;
dy = py1 - py0;
dx2 = dx * dx;
dy2 = dy * dy;
hype2 = dx2 + dy2;	// hypotenuse squared
InverseFromZ = 1.0 / FromVDEM->ScrnXYZ[2];
InverseToZ = 1.0 / ToVDEM->ScrnXYZ[2];

if (! CamPlanOrtho)
	change = FromVDEM->Lat * InverseFromZ - ToVDEM->Lat * InverseToZ;
else
	change = FromVDEM->Lat - ToVDEM->Lat;
if (dx == 0.0)
	DeltaLatX = 0.0;
else
	DeltaLatX = (dx2 / hype2 * change) / dx;	// Lat change for each dx step
if (dy == 0.0)
	DeltaLatY = 0.0;
else
	DeltaLatY = (dx2 / hype2 * change) / dy;	// Lat change for each dy step

if (! CamPlanOrtho)
	change = FromVDEM->Lon * InverseFromZ - ToVDEM->Lon * InverseToZ;
else
	change = FromVDEM->Lon - ToVDEM->Lon;
if (dx == 0.0)
	DeltaLonX = 0.0;
else
	DeltaLonX = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaLonY = 0.0;
else
	DeltaLonY = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = FromVDEM->Elev * InverseFromZ - ToVDEM->Elev * InverseToZ;
else
	change = FromVDEM->Elev - ToVDEM->Elev;
if (dx == 0.0)
	DeltaElevX = 0.0;
else
	DeltaElevX = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaElevY = 0.0;
else
	DeltaElevY = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = InverseFromZ - InverseToZ;
else
	change = FromVDEM->ScrnXYZ[2] - ToVDEM->ScrnXYZ[2];
if (dx == 0.0)
	DeltaZX = 0.0;
else
	DeltaZX = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaZY = 0.0;
else
	DeltaZY = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = FromVDEM->Q * InverseFromZ - ToVDEM->Q * InverseToZ;
else
	change = FromVDEM->Q - ToVDEM->Q;
if (dx == 0.0)
	DeltaQX = 0.0;
else
	DeltaQX = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaQY = 0.0;
else
	DeltaQY = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = FromVDEM->XYZ[0] * InverseFromZ - ToVDEM->XYZ[0] * InverseToZ;
else
	change = FromVDEM->XYZ[0] - ToVDEM->XYZ[0];
if (dx == 0.0)
	DeltaXYZ_X[0] = 0.0;
else
	DeltaXYZ_X[0] = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaXYZ_Y[0] = 0.0;
else
	DeltaXYZ_Y[0] = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = FromVDEM->XYZ[1] * InverseFromZ - ToVDEM->XYZ[1] * InverseToZ;
else
	change = FromVDEM->XYZ[1] - ToVDEM->XYZ[1];
if (dx == 0.0)
	DeltaXYZ_X[1] = 0.0;
else
	DeltaXYZ_X[1] = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaXYZ_Y[1] = 0.0;
else
	DeltaXYZ_Y[1] = (dx2 / hype2 * change) / dy;

if (! CamPlanOrtho)
	change = FromVDEM->XYZ[2] * InverseFromZ - ToVDEM->XYZ[2] * InverseToZ;
else
	change = FromVDEM->XYZ[2] - ToVDEM->XYZ[2];
if (dx == 0.0)
	DeltaXYZ_X[2] = 0.0;
else
	DeltaXYZ_X[2] = (dx2 / hype2 * change) / dx;
if (dy == 0.0)
	DeltaXYZ_Y[2] = 0.0;
else
	DeltaXYZ_Y[2] = (dx2 / hype2 * change) / dy;
// end of delta inits

// Check for non-line draw styles
if (Style == 0)	// Point
	{
	VectorDrawAAPixel(px1, py1, 0, dx, dy, ToVDEM, Pix);
	return;
	}
else if (Style == 1)
	{
	VectorWuCircle(ToVDEM, Pix);
	return;
	}
else if (Style == 2)
	{
	VectorWuSquare(ToVDEM, Pix);
	return;
	}
else if (Style == 3)
	{
	VectorWuCross(ToVDEM, Pix);
	return;
	}

if (dx >= 0)	// reinit deltas since they may have been swapped
	{
	xdir = 1;
	}
else
	{
	xdir = -1;
	dx = -dx;
	}

// draw it
yt = WCS_floor(py0 - fvwidth2) - 1;
yb = WCS_ceil(py1 + fvwidth2) + 1;
if (yt < 0.0)
	yt = 0.0;
if (yb > Height)
	yb = Height;
for (yy = yt; yy <= yb; yy++)
	{
	if (xdir > 0)	// left to right plot
		{
		xl = WCS_floor(px0 - fvwidth2) - 1;
		xr = WCS_ceil(px1 + fvwidth2) + 1;
		if (xl < 0.0)
			xl = 0.0;
		if (xr > Width)
			xr = Width;
		for (xx = xl; xx <= xr; xx++)
			{
			xkj = px0 - (xx + 0.5);	// left x coord - center of screen x coord
			ykj = py0 - (yy + 0.5);	// left y coord - center of screen y coord
			// xlk & ylk are in dx & dy
			denom = dx * dx + dy * dy;
			if (denom < 0.00001)	// coincident ends
				r = sqrt(xkj * xkj + ykj * ykj);
			else
				{
				t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
				t = min(max(t, 0.0), 1.0);			// clip to ends
				xfac = xkj + t * dx;
				yfac = ykj + t * dy;
				r = sqrt(xfac * xfac + yfac * yfac);
				}
			if (r > fvwidth2o)			// outside line?
				wu = 255;				// no intensity
			else if (r <= fvwidth2i)	// inside line?
				wu = 0;					// full intensity
			else						// anti-alias it
				{
				grrr = 1.0 - (r - fvwidth2i);
				wu = (USHORT)(255.0 - (grrr * 255.0));
				}
			VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
			} // for xx
		} // if xdir > 0
	else			// right to left plot
		{
		xl = WCS_floor(px1 - fvwidth2) - 1;
		xr = WCS_ceil(px0 + fvwidth2) + 1;
		if (xl < 0.0)
			xl = 0.0;
		if (xr > Width)
			xr = Width;
		for (xx = xl; xx <= xr; xx++)
			{
			xkj = px1 - (xx + 0.5);	// left x coord - center of screen x coord
			ykj = py1 - (yy + 0.5);	// left y coord - center of screen y coord
			xlk = px0 - px1;
			ylk = py0 - py1;
			denom = xlk * xlk + ylk * ylk;
			if (denom < 0.00001)	// coincident ends
				r = sqrt(xkj * xkj + ykj * ykj);
			else
				{
				t = -(xkj * xlk + ykj * ylk) / denom;	// compute parametric value
				t = min(max(t, 0.0), 1.0);				// clip to ends
				xfac = xkj + t * xlk;
				yfac = ykj + t * ylk;
				r = sqrt(xfac * xfac + yfac * yfac);
				}
			if (r > fvwidth2o)			// outside line?
				wu = 255;				// no intensity
			else if (r <= fvwidth2i)	// inside line?
				wu = 0;					// full intensity
			else						// anti-alias it
				{
				grrr = 1.0 - (r - fvwidth2i);
				wu = (USHORT)(255.0 - (grrr * 255.0));
				}
			VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
			} // for xx
		} // else xdir < 0
	} // for yy

return;

/*** these could speed up drawing by treating each type of line segment as special cases, but more work needs to be done

if (dy == 0.0)	// horizontal line
	{
	yt = WCS_floor(y0 - fvwidth2);								// top y position
	ti = 1.0 - ((y0 - fvwidth2) - WCS_floor(y0 - fvwidth2));	// intensity for top
	yb = WCS_ceily0 + fvwidth2);								// bottom y position
	bi = y0 + fvwidth2 - WCS_floor(y0 + fvwidth2);				// intensity for bottom
	twu = (USHORT)(255.0 - (ti * 255.0));					// convert top intensity to wu number
	bwu = (USHORT)(255.0 - (bi * 255.0));					// same for bottom
	if (xdir == 1)
		{
		}
	else
		{
		}
	// compute the corner intensities
	tli = ti * li;
	tri = ti * ri;
	bli = bi * li;
	bri = bi * ri;
	// convert intensities to wu numbers
	lwu = (USHORT)(255.0 - (li * 255.0));
	rwu = (USHORT)(255.0 - (ri * 255.0));
	twu = (USHORT)(255.0 - (ti * 255.0));
	bwu = (USHORT)(255.0 - (bi * 255.0));
	tlwu = (USHORT)(255.0 - (tli * 255.0));
	trwu = (USHORT)(255.0 - (tri * 255.0));
	blwu = (USHORT)(255.0 - (bli * 255.0));
	brwu = (USHORT)(255.0 - (bri * 255.0));
	for (yy = yt; yy <= yb; yy++)
		{
		if (yy == yt)		// top row
			{
			VectorDrawWuPixel(x0, yy, twu, dx, dy, FromVDEM, Pix);
			}
		else if (yy == yb)	// bottom row
			{
			VectorDrawWuPixel(x0, yy, bwu, dx, dy, FromVDEM, Pix);
			}
		else				// middle row
			{
			VectorDrawWuPixel(x0, yy, 0, dx, dy, FromVDEM, Pix);	// full intensity
			}
		} // for yy
	} // horizontal line
else if (dx == 0.0)	// vertical line
	{
	xl = WCS_floor(x0 - fvwidth2);
	xr = WCS_ceilx0 + fvwidth2);
	yt = WCS_floor(y0 - fvwidth2);
	yb = WCS_ceily1 + fvwidth2);
	for (yy = yt; yy <= yb; yy++)
		{
		for (xx = xl; xx <= xr; xx++)
			{
			xkj = x0 - (xx + 0.5);	// left x coord - center of screen x coord
			ykj = y0 - (yy + 0.5);	// left y coord - center of screen y coord
			// xlk & ylk are in dx & dy
			denom = dx * dx + dy * dy;
			if (denom < 0.00001)	// coincident ends
				r = sqrt(xkj * xkj + ykj * ykj);
			else
				{
				t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
				t = min(max(t, 0.0), 1.0);			// clip to ends
				xfac = xkj + t * dx;
				yfac = ykj + t * dy;
				r = sqrt(xfac * xfac + yfac * yfac);
				}
			if (r > fvwidth2o)			// outside line?
				wu = 255;				// no intensity
			else if (r <= fvwidth2i)	// inside line?
				wu = 0;					// full intensity
			else						// anti-alias it
				{
				grrr = 1.0 - (r - fvwidth2i);
				wu = (USHORT)(255.0 - (grrr * 255.0));
				}
			VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
			} // for xx
		} // for yy
	} // vertical line
else if (dx == dy)	// 45 degree diagonal
	{
	yt = WCS_floor(y0 - fvwidth2);
	yb = WCS_ceily1 + fvwidth2);
	for (yy = yt; yy <= yb; yy++)
		{
		if (xdir > 0)	// left to right plot
			{
			xl = WCS_floor(x0 - 0.5);
			xr = WCS_ceilx1 + 0.5);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x0 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y0 - (yy + 0.5);	// left y coord - center of screen y coord
				// xlk & ylk are in dx & dy
				denom = dx * dx + dy * dy;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);			// clip to ends
					xfac = xkj + t * dx;
					yfac = ykj + t * dy;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // if xdir > 0
		else			// right to left plot
			{
			xl = WCS_floor(x1 - 0.5);
			xr = WCS_ceilx0 + 0.5);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x0 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y0 - (yy + 0.5);	// left y coord - center of screen y coord
				// xlk & ylk are in dx & dy
				denom = dx * dx + dy * dy;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);			// clip to ends
					xfac = xkj + t * dx;
					yfac = ykj + t * dy;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // else xdir < 0
		} // for yy
	} // 45 degree
else if (dy > dx)	// y major
	{
	yt = WCS_floor(y0 - fvwidth2);
	yb = WCS_ceily1 + fvwidth2);
	for (yy = yt; yy <= yb; yy++)
		{
		if (xdir > 0)	// left to right plot
			{
			xl = WCS_floor(x0 - fvwidth2);
			xr = WCS_ceilx1 + fvwidth2);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x0 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y0 - (yy + 0.5);	// left y coord - center of screen y coord
				// xlk & ylk are in dx & dy
				denom = dx * dx + dy * dy;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);			// clip to ends
					xfac = xkj + t * dx;
					yfac = ykj + t * dy;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // if xdir > 0
		else			// right to left plot
			{
			xl = WCS_floor(x1 - fvwidth2);
			xr = WCS_ceilx0 + fvwidth2);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x1 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y1 - (yy + 0.5);	// left y coord - center of screen y coord
				xlk = x0 - x1;
				ylk = y0 - y1;
				denom = xlk * xlk + ylk * ylk;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * xlk + ykj * ylk) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);				// clip to ends
					xfac = xkj + t * xlk;
					yfac = ykj + t * ylk;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // else xdir < 0
		} // for yy
	} // y major
else	// x major
	{
	yt = WCS_floor(y0 - fvwidth2);
	yb = WCS_ceily1 + fvwidth2);
	for (yy = yt; yy <= yb; yy++)
		{
		if (xdir > 0)	// left to right plot
			{
			xl = WCS_floor(x0 - fvwidth2);
			xr = WCS_ceilx1 + fvwidth2);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x0 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y0 - (yy + 0.5);	// left y coord - center of screen y coord
				// xlk & ylk are in dx & dy
				denom = dx * dx + dy * dy;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * dx + ykj * dy) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);			// clip to ends
					xfac = xkj + t * dx;
					yfac = ykj + t * dy;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // if xdir > 0
		else			// right to left plot
			{
			xl = WCS_floor(x1 - fvwidth2);
			xr = WCS_ceilx0 + fvwidth2);
			for (xx = xl; xx <= xr; xx++)
				{
				xkj = x1 - (xx + 0.5);	// left x coord - center of screen x coord
				ykj = y1 - (yy + 0.5);	// left y coord - center of screen y coord
				xlk = x0 - x1;
				ylk = y0 - y1;
				denom = xlk * xlk + ylk * ylk;
				if (denom < 0.00001)	// coincident ends
					r = sqrt(xkj * xkj + ykj * ykj);
				else
					{
					t = -(xkj * xlk + ykj * ylk) / denom;	// compute parametric value
					t = min(max(t, 0.0), 1.0);				// clip to ends
					xfac = xkj + t * xlk;
					yfac = ykj + t * ylk;
					r = sqrt(xfac * xfac + yfac * yfac);
					}
				if (r > fvwidth2o)			// outside line?
					wu = 255;				// no intensity
				else if (r <= fvwidth2i)	// inside line?
					wu = 0;					// full intensity
				else						// anti-alias it
					{
					grrr = 1.0 - (r - fvwidth2i);
					wu = (USHORT)(255.0 - (grrr * 255.0));
					}
				VectorDrawWuPixel(xx, yy, wu, dx, dy, FromVDEM, Pix);	// full intensity
				} // for xx
			} // else xdir < 0
		} // for yy
	} // x major

***/

} // Renderer::VectorDrawWuLine

/*===========================================================================*/

void Renderer::VectorWuCircle(VertexDEM *FromVDEM, PixelData *Pix)
{
long xx, yy;	// x & y stepping
UBYTE wu;		// our friend
double cx, cy;	// center of circle
double distsq;	// distance squared
double r;		// radius

cx = FromVDEM->ScrnXYZ[0];
cy = FromVDEM->ScrnXYZ[1];

r = vwidth * 0.5;

for (yy = quicklongfloor(cy - r); yy <= quicklongceil(cy + r); yy++)
	{
	for (xx = quicklongfloor(cx - r); xx <= quicklongceil(cx + r); xx++)
		{
		distsq = (cx - xx) * (cx - xx) + (cy - yy) * (cy - yy);
		if (distsq >= outside)
			wu = 255;	// full off
		else if (distsq <= inside)
			wu = 0;		// full on
		else
			wu = (UBYTE)((distsq - inside) / (outside - inside) * 255.0);
		VectorDrawWuPixel((double)xx, (double)yy, wu, 0.0, 0.0, FromVDEM, Pix);
		} // for xx
	} // for yy

} // Renderer::VectorWuCircle

/*===========================================================================*/

void Renderer::VectorWuSquare(VertexDEM *FromVDEM, PixelData *Pix)
{
double cx, cy, vwidth2, xx, yy;
double tli, ti, tri, ri, bri, bi, bli, li;			// intensity %'s - top left, top, top right, right, etc.
long vwidthint;
int px1, px2, py1, py2;		// start & finish row & column values
USHORT tlwu, twu, trwu, rwu, brwu, bwu, blwu, lwu;	// the corresponding wu values

cx = FromVDEM->ScrnXYZ[0];
cy = FromVDEM->ScrnXYZ[1];

vwidthint = quicklongfloor(vwidth);
vwidth2 = vwidthint * 0.5;

// <<<>>> Frank - draw squares on non-integral size
if (vwidthint == 1)
	{
	VectorDrawAAPixel(cx, cy, 0, 0.0, 0.0, FromVDEM, Pix);
	return;
	}

if ((vwidthint % 2) == 0)	// even thickness
	{
	if (WCS_floor(cx) == cx)	// exactly centered between x screen pixels
		{
		ri = 1.0;
		li = 1.0;
		px1 = (int)(WCS_floor(cx - vwidth2));
		px2 = px1 + vwidthint - 1;
		}
	else
		{
		ri = cx - WCS_floor(cx);
		li = 1.0 - ri;
		px1 = (int)(WCS_floor(cx - vwidth2));
		px2 = px1 + vwidthint;
		}
	if (WCS_floor(cy) == cy)	// exactly centered between y screen pixels
		{
		bi = 1.0;
		ti = 1.0;
		py1 = (int)(WCS_floor(cy - vwidth2));
		py2 = py1 + vwidthint - 1;
		}
	else
		{
		bi = cy - WCS_floor(cy);
		ti = 1.0 - bi;
		py1 = (int)(WCS_floor(cy - vwidth2));
		py2 = py1 + vwidthint;
		}
	}
else	// odd thickness
	{
	if ((WCS_floor(cx) + 0.5) == cx)	// exactly centered on x screen pixel
		{
		ri = 1.0;
		li = 1.0;
		px1 = (int)(WCS_floor(cx - vwidth2));
		px2 = px1 + vwidthint - 1;
		}
	else
		{
		ri = (cx - 0.5) - WCS_floor(cx - 0.5);
		li = 1.0 - ri;
		px1 = (int)(WCS_floor(cx - vwidth2 - 0.5));
		px2 =px1 + vwidthint;
		}
	if ((WCS_floor(cy) + 0.5) == cy)	// exactly centered between y screen pixel
		{
		bi = 1.0;
		ti = 1.0;
		py1 = (int)(WCS_floor(cy - vwidth2));
		py2 = py1 + vwidthint - 1;
		}
	else
		{
		bi = (cy - 0.5) - WCS_floor(cy - 0.5);
		ti = 1.0 - bi;
		py1 = (int)(WCS_floor(cy - vwidth2 - 0.5));
		py2 = py1 + vwidthint;
		}
	}

// compute the corner intensities
tli = ti * li;
tri = ti * ri;
bli = bi * li;
bri = bi * ri;

// convert intensities to wu numbers
lwu = (USHORT)(255.0 - (li * 255.0));
rwu = (USHORT)(255.0 - (ri * 255.0));
twu = (USHORT)(255.0 - (ti * 255.0));
bwu = (USHORT)(255.0 - (bi * 255.0));
tlwu = (USHORT)(255.0 - (tli * 255.0));
trwu = (USHORT)(255.0 - (tri * 255.0));
blwu = (USHORT)(255.0 - (bli * 255.0));
brwu = (USHORT)(255.0 - (bri * 255.0));

// common drawing code
for (yy = py1; yy <= py2; yy++)
	{
	for (xx = px1; xx <= px2; xx++)
		{
		if ((xx != px1) && (xx != px2) && (yy != py1) && (yy != py2))
			VectorDrawWuPixel(xx, yy, 0, 0.0, 0.0, FromVDEM, Pix);	// interior at full intensity
		else if (xx == px1)	// left side
			{
			if (yy == py1)		// top left
				VectorDrawWuPixel(xx, yy, tlwu, 0.0, 0.0, FromVDEM, Pix);
			else if (yy == py2)	// bottom left
				VectorDrawWuPixel(xx, yy, blwu, 0.0, 0.0, FromVDEM, Pix);
			else
				VectorDrawWuPixel(xx, yy, lwu, 0.0, 0.0, FromVDEM, Pix);
			} // else if
		else if (xx == px2)	// right side
			{
			if (yy == py1)		// top right
				VectorDrawWuPixel(xx, yy, trwu, 0.0, 0.0, FromVDEM, Pix);
			else if (yy == py2)	// bottom right
				VectorDrawWuPixel(xx, yy, brwu, 0.0, 0.0, FromVDEM, Pix);
			else
				VectorDrawWuPixel(xx, yy, rwu, 0.0, 0.0, FromVDEM, Pix);
			} // else if
		else if (yy == py1)	// top side
			{
			VectorDrawWuPixel(xx, yy, twu, 0.0, 0.0, FromVDEM, Pix);
			} // else if
		else				// bottom side
			{
			VectorDrawWuPixel(xx, yy, bwu, 0.0, 0.0, FromVDEM, Pix);
			} // else
		} // for xx
	} // for yy

} // Renderer::VectorWuSquare

/*===========================================================================*/

void Renderer::VectorWuCross(VertexDEM *FromVDEM, PixelData *Pix)
{
double cx, cy, range;
double xkj, ykj, xlk, ylk, r, r1, r2, t, xfac, yfac;
double px0, py0, px1, py1, px2, py2, px3, py3;
double denom, fvwidth2i, fvwidth2o, grrr;
USHORT wu;
long x, y;

cx = FromVDEM->ScrnXYZ[0];
cy = FromVDEM->ScrnXYZ[1];

if (vwidth == 1.0)
	{
	VectorDrawAAPixel(cx, cy, 0, 0.0, 0.0, FromVDEM, Pix);
	return;
	} // if

// use a line width of 1.2, therefore 0.6 each side
fvwidth2i = 0.6 - 0.5;
fvwidth2o = 0.6 + 0.5;
range = (vwidth * 0.5) / sqrt(2.0);	// diagonal distance
px0 = px2 = cx - range;
px1 = px3 = cx + range;
py0 = py3 = cy - range;
py2 = py1 = cy + range;
range += 0.5;						// box size around center pixel

for (y = quicklongfloor(cy - range); y <= quicklongceil(cy + range); y++)
	{
	for (x = quicklongfloor(cx - range); x <= quicklongceil(cx + range); x++)
		{
		// distance to UL-LR diagonal
		xkj = px1 - (x + 0.5);	// left x coord - center of screen x coord
		ykj = py1 - (y + 0.5);	// left y coord - center of screen y coord
		xlk = px0 - px1;
		ylk = py0 - py1;
		denom = xlk * xlk + ylk * ylk;
		if (denom < 0.00001)	// coincident ends
			r1 = sqrt(xkj * xkj + ykj * ykj);
		else
			{
			t = -(xkj * xlk + ykj * ylk) / denom;	// compute parametric value
			t = min(max(t, 0.0), 1.0);				// clip to ends
			xfac = xkj + t * xlk;
			yfac = ykj + t * ylk;
			r1 = sqrt(xfac * xfac + yfac * yfac);
			} // else
		// distance to LL-UR diagonal
		xkj = px3 - (x + 0.5);	// left x coord - center of screen x coord
		ykj = py3 - (y + 0.5);	// left y coord - center of screen y coord
		xlk = px2 - px3;
		ylk = py2 - py3;
		denom = xlk * xlk + ylk * ylk;
		if (denom < 0.00001)	// coincident ends
			r2 = sqrt(xkj * xkj + ykj * ykj);
		else
			{
			t = -(xkj * xlk + ykj * ylk) / denom;	// compute parametric value
			t = min(max(t, 0.0), 1.0);				// clip to ends
			xfac = xkj + t * xlk;
			yfac = ykj + t * ylk;
			r2 = sqrt(xfac * xfac + yfac * yfac);
			} // else
		r = min(r1, r2);
		if (r > fvwidth2o)			// outside line?
			wu = 255;				// no intensity
		else if (r <= fvwidth2i)	// inside line?
			wu = 0;					// full intensity
		else						// anti-alias it
			{
			grrr = 1.0 - (r - fvwidth2i);
			wu = (USHORT)(255.0 - (grrr * 255.0));
			} // else
		VectorDrawWuPixel((double)x, (double)y, wu, 0.0, 0.0, FromVDEM, Pix);	// plot with given intensity
		} // for x
	} // for y

} // Renderer::VectorWuCross
