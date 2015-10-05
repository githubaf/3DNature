// Label.cpp
// For managing Label
// Built from EffectFoliage.cpp on 4/14/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "LabelEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "Points.h"
#include "MathSupport.h"
#include "Render.h"
#include "requester.h"
#include "Database.h"
#include "Security.h"
#include "DEM.h"
#include "Lists.h"

#include "FeatureConfig.h"

char *Label_TextLabels[] = {"Camera Name", "Camera FOV", "Camera Heading Param", "Camera Compass Bearing", 

 "Camera Pitch Param", "Camera Tilt", "Camera Bank", "Camera Longitude", "Camera Latitude",

 "Camera Elevation",

 "Target Longitude", "Target Latitude", "Target Elevation", "Target Distance",

 "Vertex Longitude", "Vertex Latitude", "Vertex Elevation", "Vertex Distance",

 "Ground Elevation",

 "Vector Name", "Vector Label",
 
 "Frame Number", "Frame Number SMPTE",
 
 "Project Name", "Render Options Name", "Render Date & Time", "Render Date Only", "Render Time Only",

 "User Name", "User Email",

 "Italics Toggle",

 NULL
 };

char *Label_TextSymbols[] = {"&CN", "&CF%1", "&CH%1", "&CC%1",

 "&CP%1", "&CT%1", "&CB%1", "&CX%3", "&CY%3",

 "&CZ%0m",

 "&TX%3", "&TY%3", "&TZ%0m", "&TD%0m",

 "&VX%3", "&VY%3", "&VZ%0m", "&VD%0m",

 "&GZ%0",

 "&VN", "&VL", 

 "&FN4", "&FS",

 "&PN", "&RN", "&RD", "&RE", "&RT",

 "&UN", "&UE",

 "&I",

 NULL
 };



LabelBase::LabelBase(void)
{

SetDefaults();

} // LabelBase::LabelBase

/*===========================================================================*/

void LabelBase::SetDefaults(void)
{

VerticesToRender = 0;
VertList = NULL;

} // LabelBase::SetDefaults

/*===========================================================================*/

int LabelBase::Init(RenderJoeList *JL)
{
long Ct;
RenderJoeList *CurJL;
VectorPoint *CurPt;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;
CurJL = JL;
while (CurJL)
	{
	if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
		VerticesToRender += (CurJL->Me->GetNumRealPoints());
	CurJL = (RenderJoeList *)CurJL->Next;
	} // while
if (VerticesToRender > 0)
	{
	if (VertList = new LabelVertexList[VerticesToRender])
		{
		CurJL = JL;
		Ct = 0;
		while (CurJL)
			{
			if (CurJL->Me && CurJL->Me->GetNumRealPoints() > 0)
				{
				if (MyAttr = (JoeCoordSys *)CurJL->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
					MyCoords = MyAttr->Coord;
				else
					MyCoords = NULL;
				for (CurPt = CurJL->Me->GetFirstRealPoint(); CurPt; CurPt = CurPt->Next)
					{
					if (CurPt->ProjToDefDeg(MyCoords, &MyVert))
						{
						VertList[Ct].Labl = (Label *)CurJL->Effect;
						VertList[Ct].Vec = CurJL->Me;
						VertList[Ct].Lat = MyVert.Lat;
						VertList[Ct].Lon = MyVert.Lon;
						VertList[Ct ++].Point = CurPt;
						} // if
					else
						VerticesToRender --;
					} // for
				} // if
			CurJL = (RenderJoeList *)CurJL->Next;
			} // while
		return (1);
		} // if
	} // if

return (0);

} // LabelBase::Init

/*===========================================================================*/

void LabelBase::Destroy(void)
{

if (VertList)
	delete [] VertList;
VertList = NULL;
VerticesToRender = 0;

} // LabelBase::Destroy

/*===========================================================================*/

void LabelBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, DEM *CurDEM)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

if (! CurDEM)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{ 
		if (CurDEM->GeographicPointContained(Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon, TRUE))
			{
			VertList[VertCt].Labl->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			Rend->Cam->ProjectVertexDEM(Rend->DefCoords, &ObjVert, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_LABEL;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = (float)(ObjVert.ScrnXYZ[2] - Rend->ShadowMapDistanceOffset);
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // LabelBase::FillRenderPolyArray

/*===========================================================================*/

void LabelBase::FillRenderPolyArray(RenderData *Rend, struct TerrainPolygonSort *PolyArray, unsigned long &PolyCt, 
	CoordSys *MyCoords, GeoRegister *MyBounds)
{
long VertCt;
VertexData ObjVert;
PolygonData Poly;

if (! MyBounds)
	return;

for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
	{
	if (! VertList[VertCt].Rendered)
		{ 
		if (MyBounds->GeographicPointContained(MyCoords, Rend->DefCoords, VertList[VertCt].Lat, VertList[VertCt].Lon))
			{
			VertList[VertCt].Labl->FindBasePosition(Rend, &ObjVert, &Poly, VertList[VertCt].Vec, VertList[VertCt].Point);
			#ifdef WCS_BUILD_VNS
			Rend->DefCoords->DegToCart(&ObjVert);
			#else // WCS_BUILD_VNS
			ObjVert.DegToCart(Rend->PlanetRad);
			#endif // WCS_BUILD_VNS
			PolyArray[PolyCt].PolyType = WCS_POLYSORTTYPE_LABEL;
			PolyArray[PolyCt].PolyNumber = (unsigned long)&VertList[VertCt];
			PolyArray[PolyCt ++].PolyQ = 1.0f;
			VertList[VertCt].Rendered = 1;
			} // if
		} // if
	} // for

} // LabelBase::FillRenderPolyArray

/*===========================================================================*/

int LabelBase::InitFrameToRender(void)
{
long VertCt;

if (VertList)
	{
	for (VertCt = 0; VertCt < VerticesToRender; VertCt ++)
		{
		VertList[VertCt].Rendered = 0;
		} // for
	} // if

return (1);

} // LabelBase::InitFrameToRender

/*===========================================================================*/
/*===========================================================================*/

Label::Label()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LABEL;
SetDefaults();

} // Label::Label

/*===========================================================================*/

Label::Label(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_LABEL;
SetDefaults();

} // Label::Label

/*===========================================================================*/

Label::Label(RasterAnimHost *RAHost, EffectsLib *Library, Label *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_LABEL;
if (Library)
	{
	Prev = Library->LastLabel;
	if (Library->LastLabel)
		{
		Library->LastLabel->Next = this;
		Library->LastLabel = this;
		} // if
	else
		{
		Library->Labels = Library->LastLabel = this;
		} // else
	} // if
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "Label");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Label::Label

/*===========================================================================*/

Label::~Label(void)
{
long Ct;
RootTexture *DelTex;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->LBL && GlobalApp->GUIWins->LBL->GetActive() == this)
		{
		delete GlobalApp->GUIWins->LBL;
		GlobalApp->GUIWins->LBL = NULL;
		} // if
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // Label::~Label

/*===========================================================================*/

void Label::SetDefaults(void)
{
long Ct;
double EffectDefault[WCS_EFFECTS_LABEL_NUMANIMPAR] = {0.0,		// WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV
														1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE
														10.0,	// WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT
														.5,		// WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH
														10.0,	// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH
														10.0,	// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT
														.5,		// WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH
														.1,		// WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH
														1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT
														0.0,		// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE
														0.0,		// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE
														0.0,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR,
														0.0,	// WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR,
														0.0,	// WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR,
														0.0,	// WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR,
														0.0		// WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR,
														};
double RangeDefaults[WCS_EFFECTS_LABEL_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV
														100.0, 0.0, 0.01,		// WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE
														FLT_MAX, 0.0, 1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT
														FLT_MAX, 0.0, 0.1,	// WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH
														FLT_MAX, 0.0, 1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH
														FLT_MAX, 0.0, 1.0,	// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT
														FLT_MAX, 0.0, 0.1,	// WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH
														FLT_MAX, 0.0, 0.05,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH
														FLT_MAX, 0.0, 0.1,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT
														FLT_MAX, 0.0, 0.1,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE
														FLT_MAX, 0.0, 0.1,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE
														1.0, 0.0, 0.01,	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR
														1.0, 0.0, 0.01,	// WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR
														1.0, 0.0, 0.01,	// WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR
														1.0, 0.0, 0.01,	// WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR
														1.0, 0.0, 0.01	// WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR
														};

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for
TextColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR);
OutlineColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 1);
FlagColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 2);
BorderColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 3);
PoleColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 4);
PreviewEnabled = 1;
Justification = WCS_EFFECTS_LABEL_JUSTIFY_CENTER;
FlagWidthStyle = WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING;
FlagHeightStyle = WCS_EFFECTS_LABEL_FLAGSIZE_FLOATING;
WordWrapEnabled = 1;
PoleStyle = WCS_EFFECTS_LABEL_POLESTYLE_VERTICAL;
PoleBaseStyle = WCS_EFFECTS_LABEL_POLEBASESTYLE_SQUARE;
PoleFullWidth = 0;
PoleFullHeight = 0;
PolePosition = WCS_EFFECTS_LABEL_POLEPOSITION_LEFT;
AnchorPoint = WCS_EFFECTS_LABEL_ANCHORPOINT_CENTER;
TextEnabled = OutlineEnabled = FlagEnabled = BorderEnabled = PoleEnabled = OverheadViewPole = 1;
HiResFont = WCS_EFFECTS_LABEL_HIRESFONT_SOMETIMES;
strcpy(MesgText, "Type the label text here.");

TextColor.SetValue3(1.0, 1.0, 1.0);
OutlineColor.SetValue3(0.25, 0.25, 0.25);

CompleteTextColor[0] = CompleteTextColor[1] = CompleteTextColor[2] = 0.0;
CompleteOutlineColor[0] = CompleteOutlineColor[1] = CompleteOutlineColor[2] = 0.0;
CompleteFlagColor[0] = CompleteFlagColor[1] = CompleteFlagColor[2] = 0.0;
CompleteBorderColor[0] = CompleteBorderColor[1] = CompleteBorderColor[2] = 0.0;
CompletePoleColor[0] = CompletePoleColor[1] = CompletePoleColor[2] = 0.0;

AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR].SetMultiplier(100.0);

} // Label::SetDefaults

/*===========================================================================*/

void Label::Copy(Label *CopyTo, Label *CopyFrom)
{

CopyTo->PreviewEnabled = CopyFrom->PreviewEnabled;
CopyTo->Justification = CopyFrom->Justification;
CopyTo->FlagWidthStyle = CopyFrom->FlagWidthStyle;
CopyTo->FlagHeightStyle = CopyFrom->FlagHeightStyle;
CopyTo->WordWrapEnabled = CopyFrom->WordWrapEnabled;
CopyTo->PoleStyle = CopyFrom->PoleStyle;
CopyTo->PoleBaseStyle = CopyFrom->PoleBaseStyle;
CopyTo->PoleFullWidth = CopyFrom->PoleFullWidth;
CopyTo->PoleFullHeight = CopyFrom->PoleFullHeight;
CopyTo->PolePosition = CopyFrom->PolePosition;
CopyTo->AnchorPoint = CopyFrom->AnchorPoint;
CopyTo->TextEnabled = CopyFrom->TextEnabled;
CopyTo->OutlineEnabled = CopyFrom->OutlineEnabled;
CopyTo->FlagEnabled = CopyFrom->FlagEnabled;
CopyTo->BorderEnabled = CopyFrom->BorderEnabled;
CopyTo->PoleEnabled = CopyFrom->PoleEnabled;
CopyTo->OverheadViewPole = CopyFrom->OverheadViewPole;
CopyTo->HiResFont = CopyFrom->HiResFont;
CopyTo->TextColor.Copy(&CopyTo->TextColor, &CopyFrom->TextColor);
CopyTo->OutlineColor.Copy(&CopyTo->OutlineColor, &CopyFrom->OutlineColor);
CopyTo->FlagColor.Copy(&CopyTo->FlagColor, &CopyFrom->FlagColor);
CopyTo->BorderColor.Copy(&CopyTo->BorderColor, &CopyFrom->BorderColor);
CopyTo->PoleColor.Copy(&CopyTo->PoleColor, &CopyFrom->PoleColor);
strcpy(CopyTo->MesgText, CopyFrom->MesgText);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Label::Copy

/*===========================================================================*/

unsigned long int Label::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
#if defined (WCS_BUILD_VNS) || defined(WCS_THEMATIC_MAP)
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // defined (WCS_BUILD_VNS) || defined(WCS_THEMATIC_MAP)

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_HIRESEDGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&HiResEdge, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_USEGRADIENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseGradient, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ABSOLUTE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Absolute, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_RENDEROCCLUDED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderOccluded, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_PREVIEWENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PreviewEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_JUSTIFICATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&Justification, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_FLAGWIDTHSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlagWidthStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_FLAGHEIGHTSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlagHeightStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_WORDWRAPENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&WordWrapEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLESTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PoleStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEBASESTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PoleBaseStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEFULLWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&PoleFullWidth, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEFULLHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&PoleFullHeight, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEPOSITION:
						{
						BytesRead = ReadBlock(ffile, (char *)&PolePosition, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_ANCHORPOINT:
						{
						BytesRead = ReadBlock(ffile, (char *)&AnchorPoint, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&TextEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_OUTLINEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&OutlineEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_FLAGENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlagEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_BORDERENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&BorderEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&PoleEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_OVERHEADVIEWPOLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&OverheadViewPole, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_HIRESFONT:
						{
						BytesRead = ReadBlock(ffile, (char *)&HiResFont, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_MESGTEXT:
						{
						BytesRead = ReadBlock(ffile, (char *)MesgText, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_BASEELEV:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_MASTERSIZE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEHEIGHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLEWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_MAXFLAGWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_MAXFLAGHEIGHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_BORDERWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTOUTLINEWIDTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTLINEHEIGHT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTLINESPACE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTLETTERSPACE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTTRANSPAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_OUTLINETRANSPAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_FLAGTRANSPAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_BORDERTRANSPAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLETRANSPAR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_TEXTCOLOR:
						{
						BytesRead = TextColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_OUTLINECOLOR:
						{
						BytesRead = OutlineColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_FLAGCOLOR:
						{
						BytesRead = FlagColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_BORDERCOLOR:
						{
						BytesRead = BorderColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_LABEL_POLECOLOR:
						{
						BytesRead = PoleColor.Load(ffile, Size, ByteFlip);
						break;
						}

					case WCS_EFFECTS_LABEL_TEXMASTERSIZE:
						{
						if (TexRoot[WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_LABEL_THEMEMASTERSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_LABEL_THEME_MASTERSIZE] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_EFFECTS_LABEL_THEMEBASEELEV:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_LABEL_THEME_BASEELEV] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
					#ifdef WCS_BUILD_VNS
					case WCS_EFFECTS_QUERY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Search = (SearchQuery *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_SEARCHQUERY, MatchName);
							} // if
						break;
						}
					#endif // WCS_BUILD_VNS
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // Label::Load

/*===========================================================================*/

unsigned long int Label::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_LABEL_NUMANIMPAR] = {WCS_EFFECTS_LABEL_BASEELEV,
																WCS_EFFECTS_LABEL_MASTERSIZE,
																WCS_EFFECTS_LABEL_POLEHEIGHT,
																WCS_EFFECTS_LABEL_POLEWIDTH,
																WCS_EFFECTS_LABEL_MAXFLAGWIDTH,
																WCS_EFFECTS_LABEL_MAXFLAGHEIGHT,
																WCS_EFFECTS_LABEL_BORDERWIDTH,
																WCS_EFFECTS_LABEL_TEXTOUTLINEWIDTH,
																WCS_EFFECTS_LABEL_TEXTLINEHEIGHT,
																WCS_EFFECTS_LABEL_TEXTLINESPACE,
																WCS_EFFECTS_LABEL_TEXTLETTERSPACE,
																WCS_EFFECTS_LABEL_TEXTTRANSPAR,
																WCS_EFFECTS_LABEL_OUTLINETRANSPAR,
																WCS_EFFECTS_LABEL_FLAGTRANSPAR,
																WCS_EFFECTS_LABEL_BORDERTRANSPAR,
																WCS_EFFECTS_LABEL_POLETRANSPAR};
unsigned long int TextureItemTag[WCS_EFFECTS_LABEL_NUMTEXTURES] = {WCS_EFFECTS_LABEL_TEXMASTERSIZE};
unsigned long int ThemeItemTag[WCS_EFFECTS_LABEL_NUMTHEMES] = {WCS_EFFECTS_LABEL_THEMEMASTERSIZE,
																WCS_EFFECTS_LABEL_THEMEBASEELEV};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_HIRESEDGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&HiResEdge)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_USEGRADIENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseGradient)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ABSOLUTE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Absolute)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_RENDEROCCLUDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RenderOccluded)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_PREVIEWENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PreviewEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_JUSTIFICATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Justification)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_FLAGWIDTHSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlagWidthStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_FLAGHEIGHTSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlagHeightStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_WORDWRAPENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WordWrapEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLESTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PoleStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLEBASESTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PoleBaseStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLEFULLWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PoleFullWidth)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLEFULLHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PoleFullHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLEPOSITION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PolePosition)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_ANCHORPOINT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AnchorPoint)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_TEXTENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TextEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_OUTLINEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OutlineEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_FLAGENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlagEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_BORDERENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BorderEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_POLEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PoleEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_OVERHEADVIEWPOLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OverheadViewPole)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_HIRESFONT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&HiResFont)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_LABEL_MESGTEXT, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MesgText) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MesgText)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if anim param saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // for

ItemTag = WCS_EFFECTS_LABEL_TEXTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = TextColor.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_LABEL_OUTLINECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = OutlineColor.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_LABEL_FLAGCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = FlagColor.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_LABEL_BORDERCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = BorderColor.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

ItemTag = WCS_EFFECTS_LABEL_POLECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PoleColor.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block
					else
						goto WriteError;
					} // if texture saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

#ifdef WCS_THEMATIC_MAP
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (GetTheme(Ct))
		{
		if ((BytesWritten = PrepWriteBlock(ffile, ThemeItemTag[Ct], WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(GetTheme(Ct)->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)GetTheme(Ct)->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	} // for
#endif // WCS_THEMATIC_MAP

#ifdef WCS_BUILD_VNS
if (Search)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_QUERY, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Search->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Search->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Label::Save

/*===========================================================================*/

void Label::Edit(void)
{

#ifdef WCS_LABEL
DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->LBL)
	{
	delete GlobalApp->GUIWins->LBL;
	}
GlobalApp->GUIWins->LBL = new LabelEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->LBL)
	{
	GlobalApp->GUIWins->LBL->Open(GlobalApp->MainProj);
	}
#endif // WCS_LABEL

} // Label::Edit

/*===========================================================================*/

char *LabelCritterNames[WCS_EFFECTS_LABEL_NUMANIMPAR] = {"Base Elevation (m)",		// WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV,
														"Master Size (%)",			// WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE,
														"Pole Height (m)",			// WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT,
														"Pole Width (m)",			// WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH,
														"Text Width (m)",			// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH,
														"Text Height (m)",			// WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT,
														"Border Width (m)",			// WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH,
														"Text Outline Width (m)",	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH,
														"Text Line Height (m)",		// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT,
														"Text Line Space (m)",		// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE,
														"Text Letter Space (m)",	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE,
														"Text Transparency (%)",	// WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR,
														"Outline Transparency (%)",	// WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR,
														"Flag Transparency (%)",	// WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR,
														"Border Transparency (%)",	// WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR,
														"Pole Transparency (%)"		// WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR,
														};
char *LabelTextureNames[WCS_EFFECTS_LABEL_NUMTEXTURES] = {"Master Size (%)"};
char *LabelThemeNames[WCS_EFFECTS_LABEL_NUMTHEMES] = {"Master Size (%)", "Base Elevation (m)"};

char *Label::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (LabelCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (LabelTextureNames[Ct]);
		} // if
	} // for
if (Test == &TextColor)
	return ("Text Color");
if (Test == &OutlineColor)
	return ("Outline Color");
if (Test == &FlagColor)
	return ("Flag Color");
if (Test == &BorderColor)
	return ("Border Color");
if (Test == &PoleColor)
	return ("Pole Color");

return ("");

} // Label::GetCritterName

/*===========================================================================*/

int Label::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (TextColor.GetRAHostAnimated())
	return (1);
if (OutlineColor.GetRAHostAnimated())
	return (1);
if (FlagColor.GetRAHostAnimated())
	return (1);
if (BorderColor.GetRAHostAnimated())
	return (1);
if (PoleColor.GetRAHostAnimated())
	return (1);

return (0);

} // Label::GetRAHostAnimated

/*===========================================================================*/

int Label::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (TextColor.SetToTime(Time))
	Found = 1;
if (OutlineColor.SetToTime(Time))
	Found = 1;
if (FlagColor.SetToTime(Time))
	Found = 1;
if (BorderColor.SetToTime(Time))
	Found = 1;
if (PoleColor.SetToTime(Time))
	Found = 1;

return (Found);

} // Label::SetToTime

/*===========================================================================*/

long Label::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (TextColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (OutlineColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (FlagColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (BorderColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (PoleColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // Label::GetKeyFrameRange

/*===========================================================================*/

int Label::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LABEL_THEME_MASTERSIZE);
					break;
					} // 
				case WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV:
					{
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LABEL_THEME_BASEELEV);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	if (ChildA == &TextColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	else if (ChildA == &OutlineColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	else if (ChildA == &FlagColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	else if (ChildA == &BorderColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	else if (ChildA == &PoleColor)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE);
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_LABEL_THEME_MASTERSIZE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetThemeAddr(Ct))
			{
			ThemeAffil = (ThematicMap **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_LABEL_THEME_MASTERSIZE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MASTERSIZE);
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_LABEL_TEXTURE_MASTERSIZE);
					break;
					} // 
				case WCS_EFFECTS_LABEL_THEME_BASEELEV:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Label::GetAffiliates

/*===========================================================================*/

char Label::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	#ifdef WCS_BUILD_VNS
	|| DropType == WCS_EFFECTSSUBCLASS_SEARCHQUERY
	#endif // WCS_BUILD_VNS
	)
	return (1);

return (0);

} // Label::GetRAHostDropOK

/*===========================================================================*/

int Label::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Label *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Label *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_SEARCHQUERY)
	{
	Success = -1;
	sprintf(QueryStr, "Apply %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		Success = SetQuery((SearchQuery *)DropSource->DropSource);
		} // if
	} // else if
#endif // WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (((Joe *)DropSource->DropSource)->AddEffect(this, -1))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // Label::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Label::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_LABEL | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Label::GetRAFlags

/*===========================================================================*/

RasterAnimHost *Label::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
JoeList *CurJoe = Joes;
char Ct, Found = 0;

if (! Current)
	return (&TextColor);
if (Current == &TextColor)
	return (&OutlineColor);
if (Current == &OutlineColor)
	return (&FlagColor);
if (Current == &FlagColor)
	return (&BorderColor);
if (Current == &BorderColor)
	return (&PoleColor);
if (Current == &PoleColor)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	if (Found && GetTheme(Ct) && GetThemeUnique(Ct))
		return (GetTheme(Ct));
	if (Current == GetTheme(Ct) && GetThemeUnique(Ct))
		Found = 1;
	} // for
#ifdef WCS_BUILD_VNS
if (Found && Search)
	return (Search);
if (Search && Current == Search)
	Found = 1;
#endif // WCS_BUILD_VNS
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // Label::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *Label::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_OUTLINETRANSPAR))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_FLAGTRANSPAR))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_BORDERTRANSPAR))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLETRANSPAR))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_TEXTTRANSPAR));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH));

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT))
	return (GetAnimPtr(WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH));

return (NULL);

} // Label::GetNextGroupSibling

/*===========================================================================*/

char *Label::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? LabelTextureNames[TexNumber]: (char*)"");

} // Label::GetTextureName

/*===========================================================================*/

RootTexture *Label::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Label::NewRootTexture

/*===========================================================================*/

char *Label::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? LabelThemeNames[ThemeNum]: (char*)"");

} // Label::GetThemeName

/*===========================================================================*/

void Label::ScaleSizes(double ScaleFactor)
{

AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEHEIGHT].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_POLEWIDTH].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGWIDTH].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_MAXFLAGHEIGHT].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BORDERWIDTH].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTOUTLINEWIDTH].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINEHEIGHT].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLINESPACE].ScaleValues(ScaleFactor);
AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_TEXTLETTERSPACE].ScaleValues(ScaleFactor);

} // Label::ScaleSizes

/*===========================================================================*/

int Label::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{

CompleteTextColor[0] = TextColor.GetCompleteValue(0);
CompleteTextColor[1] = TextColor.GetCompleteValue(1);
CompleteTextColor[2] = TextColor.GetCompleteValue(2);

CompleteOutlineColor[0] = OutlineColor.GetCompleteValue(0);
CompleteOutlineColor[1] = OutlineColor.GetCompleteValue(1);
CompleteOutlineColor[2] = OutlineColor.GetCompleteValue(2);

CompleteFlagColor[0] = FlagColor.GetCompleteValue(0);
CompleteFlagColor[1] = FlagColor.GetCompleteValue(1);
CompleteFlagColor[2] = FlagColor.GetCompleteValue(2);

CompleteBorderColor[0] = BorderColor.GetCompleteValue(0);
CompleteBorderColor[1] = BorderColor.GetCompleteValue(1);
CompleteBorderColor[2] = BorderColor.GetCompleteValue(2);

CompletePoleColor[0] = PoleColor.GetCompleteValue(0);
CompletePoleColor[1] = PoleColor.GetCompleteValue(1);
CompletePoleColor[2] = PoleColor.GetCompleteValue(2);

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // Label::InitFrameToRender

/*===========================================================================*/

int Label::FindBasePosition(RenderData *Rend, VertexDEM *Vert, PolygonData *Poly, Joe *Vector, VectorPoint *CurVtx)
{
unsigned long Flags;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexData VertData;

if (CurVtx)
	{
	if (Vector && (MyAttr = (JoeCoordSys *)Vector->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
		MyCoords = MyAttr->Coord;
	else
		MyCoords = NULL;
	if (CurVtx->ProjToDefDeg(MyCoords, &VertData))
		{
		if (Absolute == WCS_EFFECT_RELATIVETOJOE)			// relative to Joe - apply exaggeration to vector elevation
			{
			Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
				WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
				WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
			Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
			VertData.Elev = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV].CurValue + 
				Rend->ElevDatum + (CurVtx->Elevation - Rend->ElevDatum) * Rend->Exageration;
			} // if relative to Joe
		else if (Absolute == WCS_EFFECT_ABSOLUTE)		// absolute - no exaggeration
			{
			Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | 
				WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
				WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
			Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
			VertData.Elev = AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV].CurValue;
			} // if absolute
		else
			{						// relative to ground - exageration applied in VertexDataPoint() to terrain elevation
			Flags = (WCS_VERTEXDATA_FLAG_ELEVATION | WCS_VERTEXDATA_FLAG_RELEL | WCS_VERTEXDATA_FLAG_PRERASTERTAAPPLIED |
				WCS_VERTEXDATA_FLAG_POSTRASTERTAAPPLIED | WCS_VERTEXDATA_FLAG_TERRAFFECTORAPPLIED | 
				WCS_VERTEXDATA_FLAG_WATERELEV | WCS_VERTEXDATA_FLAG_WATERDEPTH | WCS_VERTEXDATA_FLAG_LAKEAPPLIED |
				WCS_VERTEXDATA_FLAG_STREAMAPPLIED | WCS_VERTEXDATA_FLAG_WAVEHEIGHT);
			Rend->Interactive->VertexDataPoint(Rend, &VertData, Poly, Flags);
			VertData.Elev += AnimPar[WCS_EFFECTS_LABEL_ANIMPAR_BASEELEV].CurValue;
			} // else relative to ground
		// copy relevant data to vertex and polygon
		Vert->Lat = VertData.Lat;
		Vert->Lon = VertData.Lon;
		Vert->Elev = VertData.Elev;
		Poly->Lat = VertData.Lat;
		Poly->Lon = VertData.Lon;
		Poly->Elev = VertData.Elev;
		Poly->RelEl = VertData.RelEl;
		Poly->WaterElev = VertData.WaterElev;
		Poly->Object = this;
		Poly->Vector = Vector;
		Poly->VectorType = WCS_TEXTURE_VECTOREFFECTTYPE_LINE;

		Poly->LonSeed = (ULONG)((Vert->Lon - WCS_floor(Vert->Lon)) * ULONG_MAX);
		Poly->LatSeed = (ULONG)((Vert->Lat - WCS_floor(Vert->Lat)) * ULONG_MAX);
		return (1);
		} // if
	} // if

return (0);

} // Label::FindBasePosition

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Label::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
Label *CurrentLabel = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
DEMBounds OldBounds, CurBounds;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	// set some global pointers so that things know what libraries to link to
	GlobalApp->LoadToEffectsLib = LoadToEffects;

	while (BytesRead && Success)
		{
		// read block descriptor tag from file 
		if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
			WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
			{
			TotalRead += BytesRead;
			ReadBuf[8] = 0;
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&Size,
				WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				if (! strnicmp(ReadBuf, "DEMBnds", 8))
					{
					if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
						OldBoundsLoaded = 1;
					} // if material
				else if (! strnicmp(ReadBuf, "Search", 8))
					{
					if (CurrentQuery = new SearchQuery(NULL, LoadToEffects, NULL))
						{
						BytesRead = CurrentQuery->Load(ffile, Size, ByteFlip);
						}
					} // if search query
				else if (! strnicmp(ReadBuf, "ThemeMap", 8))
					{
					if (CurrentTheme = new ThematicMap(NULL, LoadToEffects, NULL))
						{
						BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip);
						}
					} // if thematic map
				else if (! strnicmp(ReadBuf, "Label", 8))
					{
					if (CurrentLabel = new Label(NULL, LoadToEffects, NULL))
						{
						if ((BytesRead = CurrentLabel->Load(ffile, Size, ByteFlip)) == Size)
							Success = 1;	// we got our man
						}
					} // if eco
				else if (! fseek(ffile, Size, SEEK_CUR))
					BytesRead = Size;
				TotalRead += BytesRead;
				if (BytesRead != Size)
					{
					Success = 0;
					break;
					} // if error
				} // if size block read 
			else
				break;
			} // if tag block read 
		else
			break;
		} // while 
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentLabel)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	Copy(this, CurrentLabel);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;

return (Success);

} // Label::LoadObject

/*===========================================================================*/

int Label::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
EffectList *CurEffect, *Queries = NULL, *Themes = NULL, *Coords = NULL;
DEMBounds CurBounds;
char StrBuf[12];

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

if (GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while

	CurEffect = Queries;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Search");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SearchQuery *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if SearchQuery saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	#ifdef WCS_THEMATIC_MAP
	CurEffect = Themes;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "ThemeMap");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ThematicMap *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if ThemeMap saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_THEMATIC_MAP

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	while (Queries)
		{
		CurEffect = Queries;
		Queries = Queries->Next;
		delete CurEffect;
		} // while
	while (Themes)
		{
		CurEffect = Themes;
		Themes = Themes->Next;
		delete CurEffect;
		} // while
	} // if

// Label
strcpy(StrBuf, "Label");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Label saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // Label::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Label_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Label *Current;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Label(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Label *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // EffectsLib::Label_Load()

/*===========================================================================*/

ULONG EffectsLib::Label_Save(FILE *ffile)
{
Label *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Labels;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block 
					else
						goto WriteError;
					} // if Label saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Label *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // EffectsLib::Label_Save()

/*===========================================================================*/
