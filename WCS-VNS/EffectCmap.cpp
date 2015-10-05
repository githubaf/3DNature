// EffectCmap.cpp
// For managing Cmap Effects
// Built from scratch CmapEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "Conservatory.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Raster.h"
#include "requester.h"
#include "CmapEditGUI.h"
#include "Toolbar.h"
#include "AppMem.h"
#include "Render.h"
#include "DEM.h"
#include "Database.h"
#include "Project.h"
#include "Interactive.h"
#include "Lists.h"
#include "FeatureConfig.h"

CmapEffectBase::CmapEffectBase(void)
{

SetDefaults();

} // CmapEffectBase::CmapEffectBase

/*===========================================================================*/

void CmapEffectBase::SetDefaults(void)
{

DEMSpecificCmaps = NULL;

} // CmapEffectBase::SetDefaults

/*===========================================================================*/

void CmapEffectBase::Destroy(void)
{
EffectList *CurCmap = DEMSpecificCmaps;

while (CurCmap)
	{
	DEMSpecificCmaps = CurCmap->Next;
	delete CurCmap;
	CurCmap = DEMSpecificCmaps;
	} // while

} // CmapEffectBase::Destroy

/*===========================================================================*/

short CmapEffectBase::AreThereEdges(GeneralEffect *EffectList)
{

while (EffectList)
	{
	if (EffectList->Enabled && EffectList->HiResEdge)
		return (1);
	EffectList = EffectList->Next;
	} // while

return (0);

} // CmapEffectBase::AreThereEdges

/*===========================================================================*/

int CmapEffectBase::BuildDEMSpecificCmapList(EffectsLib *Lib, DEM *MyDEM)
{
int Sorted = 0, Success = 1;
EffectList *CurCmap = DEMSpecificCmaps, **CurCmapPtr = &DEMSpecificCmaps;
CmapEffect *CurCmapEffect;

while (CurCmap)
	{
	DEMSpecificCmaps = CurCmap->Next;
	delete CurCmap;
	CurCmap = DEMSpecificCmaps;
	} // while

for (CurCmapEffect = Lib->Cmap; CurCmapEffect; CurCmapEffect = (CmapEffect *)CurCmapEffect->Next)
	{
	if (CurCmapEffect->Rast)
		{
		// check bounds

//		if (CurCmapEffect->South < MyDEM->Northest() &&
//			CurCmapEffect->North > MyDEM->Southest() &&
//			CurCmapEffect->East < MyDEM->Westest() &&
//			CurCmapEffect->West > MyDEM->Eastest())
			{
			// add a link
			if (*CurCmapPtr = new EffectList())
				{
				(*CurCmapPtr)->Me = CurCmapEffect;
				CurCmapPtr = &(*CurCmapPtr)->Next;
				} // if
			else
				Success = 0;
			} // if
		} // if
	} // for

// sort by priority
while (! Sorted)
	{
	Sorted = 1;
	CurCmap = DEMSpecificCmaps;
	while (CurCmap && CurCmap->Next)
		{
		if (CurCmap->Next->Me->Priority > CurCmap->Me->Priority)
			{
			swmem(&CurCmap->Next->Me, &CurCmap->Me, sizeof (CmapEffect *));
			Sorted = 0;
			} // if
		CurCmap = CurCmap->Next;
		} // while
	} // while

return (Success);

} // CmapEffectBase::BuildDEMSpecificCmapList

/*===========================================================================*/

int CmapEffectBase::Eval(RenderData *Rend, PolygonData *Poly)
{
double Lat, Lon, RandomLatShift, RandomLonShift;
EffectList *CurCmap = DEMSpecificCmaps, *CurEco;
CmapEffect *Cmap;
EcosystemEffect *Eco;
unsigned char SampleRGB[3];
int Done = 0, Abort = 0, RandomInit = 0, EcoFailed, InBounds, FoundEco;
long CurRow, CurCol, MaxRow, MaxCol;
VertexDEM Vert;

Lat = Poly->Lat;
Lon = Poly->Lon;

while (CurCmap && ! Done)
	{
	Cmap = (CmapEffect *)CurCmap->Me;
	if (Cmap->Enabled)	// it can become disabled if image is not found
		{
		Vert.Lat = Vert.xyz[1] = Lat;
		Vert.Lon = Vert.xyz[0] = Lon;

		// convert point to appropriate coord system
		if (Cmap->Coords)
			{
			// convert point to global cartesian
			if (! Cmap->Coords->DefDegToProj(&Vert))
				{
				CurCmap = CurCmap->Next;
				continue;
				} // if projection error
			} // if

		InBounds = 0;
		// evaluate point to see if it is in
		if (Vert.xyz[1] >= Cmap->NSLow && Vert.xyz[1] <= Cmap->NSHigh)
			{
			if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
				InBounds = 1;
			else if ((! Cmap->Coords) || Cmap->Coords->GetGeographic())
				{
				Vert.xyz[0] += 360.0;
				if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
					InBounds = 1;
				else
					{
					Vert.xyz[0] -= 720.0;
					if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
						InBounds = 1;
					} // else
				} // else
			if (InBounds)
				{
				// if it is an image drape 
				if (Cmap->EvalByPixel && ! Rend->IsTextureMap)
					{
					Poly->Ground = Cmap->ByPixelGround;
					Poly->Cmap = Cmap;
					Done = 1;
					break;
					} // if
				// find out what row and column it's in
				MaxRow = Cmap->Rast->Rows - 1;
				MaxCol = Cmap->Rast->Cols - 1;
				if (Cmap->Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
					{
					CurRow = (long)((Cmap->North - Vert.xyz[1]) / Cmap->DegLatPerCell);
					CurCol = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
					} // if
				else
					{
					CurCol = (long)((Vert.xyz[1] - Cmap->South) / Cmap->DegLatPerCell);
					CurRow = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
					} // else west at top
				if (CurRow > MaxRow)
					CurRow = MaxRow;
				if (CurCol > MaxCol)
					CurCol = MaxCol;
				if (Cmap->RandomizeEdges)
					{
					if (! RandomInit)
						{
						Rand.Seed64BitShift(Poly->LatSeed + WCS_SEEDOFFSET_CMAP, Poly->LonSeed + WCS_SEEDOFFSET_CMAP);
						RandomLatShift = Rand.GenPRN() - .5;
						RandomLonShift = Rand.GenPRN() - .5;
						RandomInit = 1;
						} // if
					if (Cmap->Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
						{
						Vert.xyz[1] += (RandomLatShift * Cmap->DegLatPerCell);
						Vert.xyz[0] += (RandomLonShift * Cmap->DegLonPerCell);
						CurRow = (long)((Cmap->North - Vert.xyz[1]) / Cmap->DegLatPerCell);
						CurCol = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
						} // if
					else
						{
						Vert.xyz[0] += (RandomLonShift * Cmap->DegLonPerCell);
						Vert.xyz[1] += (RandomLatShift * Cmap->DegLatPerCell);
						CurCol = (long)((Vert.xyz[1] - Cmap->South) / Cmap->DegLatPerCell);
						CurRow = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
						} // else
					if (CurRow > MaxRow)
						CurRow = MaxRow;
					else if (CurRow < 0)
						CurRow = 0;
					if (CurCol > MaxCol)
						CurCol = MaxCol;
					else if (CurCol < 0)
						CurCol = 0;
					} // if
				// sample color map, no antialiasing
				if (Cmap->Rast->SampleByteCell3(SampleRGB, CurCol, CurRow, Abort))
					{
					EcoFailed = 0;
					if (! Cmap->EvalByPixel)
						{
						for (CurEco = Cmap->Ecosystems; CurEco; CurEco = CurEco->Next)
							{
							FoundEco = 0;
							Eco = (EcosystemEffect *)CurEco->Me;
							if (Eco->CmapMatch && Eco->Enabled)
								{
								if (Eco->CmapMatchRange)
									{
									if (SampleRGB[0] >= Eco->MatchColor[0] && SampleRGB[1] >= Eco->MatchColor[1] && SampleRGB[2] >= Eco->MatchColor[2] &&
										SampleRGB[0] <= Eco->MatchColor[3] && SampleRGB[1] <= Eco->MatchColor[4] && SampleRGB[2] <= Eco->MatchColor[5])
										FoundEco = 1;
									} // if
								else if (SampleRGB[0] == Eco->MatchColor[0] && SampleRGB[1] == Eco->MatchColor[1] && SampleRGB[2] == Eco->MatchColor[2])
									FoundEco = 1;
								if (FoundEco)
									{
									if (Poly->Slope >= Eco->MinSlope &&	// both in degrees
										Poly->Slope <= Eco->MaxSlope)
										{
										if (Poly->RelEl >= Eco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue &&
											Poly->RelEl <= Eco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue)
											{
											Poly->Eco = Eco;
											EcoFailed = 0;
											break;
											} // if
										else
											EcoFailed = 1;
										} // if
									else
										EcoFailed = 1;
									} // if
								} // if
							} // for
						} // if
					if (! EcoFailed)
						{
						Done = 1;
						Poly->Cmap = Cmap;
						//Poly->TintFoliage = (char)Cmap->TintFoliage;
						Poly->RGB[0] = SampleRGB[0] / 255.0;
						Poly->RGB[1] = SampleRGB[1] / 255.0;
						Poly->RGB[2] = SampleRGB[2] / 255.0;
						if (! Poly->Eco)
							{
							Poly->RenderCmapColor = 1;
							Poly->LuminousColor = (char)Cmap->LuminousColors;
							} // if
						} // if
					} // if colored pixel
				else if (Abort)
					{
					Cmap->Enabled = 0;
					} // else
				} // if in bounds
			} // if in lat bounds
		} // if enabled
	CurCmap = CurCmap->Next;
	} // while

return (Done);

} // CmapEffectBase::Eval

/*===========================================================================*/

bool CmapEffectBase::Eval(RenderData *Rend, VectorNode *CurNode, double &SumOfAllCoverages, double &PlowedSnow)
{
unsigned char SampleRGB[3];
int Abort = 0, RandomInit = 0, InBounds, FoundEco;
long CurRow, CurCol, MaxRow, MaxCol;
unsigned long LonSeed, LatSeed;
EffectList *CurCmap = DEMSpecificCmaps, *CurEco;
double RandomLatShift, RandomLonShift;
CmapEffect *Cmap;
EcosystemEffect *Eco;
VertexDEM Vert;
TwinMaterials MatTwin;
bool Done = false, Success = true;

while (CurCmap && ! Done && Success)
	{
	Cmap = (CmapEffect *)CurCmap->Me;
	if (Cmap->Enabled)	// it can become disabled if image is not found
		{
		Vert.Lat = Vert.xyz[1] = CurNode->Lat;
		Vert.Lon = Vert.xyz[0] = CurNode->Lon;

		// convert point to appropriate coord system
		if (Cmap->Coords)
			{
			// convert point to global cartesian
			if (! Cmap->Coords->DefDegToProj(&Vert))
				{
				CurCmap = CurCmap->Next;
				continue;
				} // if projection error
			} // if

		InBounds = 0;
		// evaluate point to see if it is in
		if (Vert.xyz[1] >= Cmap->NSLow && Vert.xyz[1] <= Cmap->NSHigh)
			{
			if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
				InBounds = 1;
			else if ((! Cmap->Coords) || Cmap->Coords->GetGeographic())
				{
				Vert.xyz[0] += 360.0;
				if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
					InBounds = 1;
				else
					{
					Vert.xyz[0] -= 720.0;
					if (Vert.xyz[0] >= Cmap->WELow && Vert.xyz[0] <= Cmap->WEHigh)
						InBounds = 1;
					} // else
				} // else
			if (InBounds)
				{
				// if it is an image drape 
				if (Cmap->EvalByPixel && ! Rend->IsTextureMap)
					{
					MaterialEffect *Matthew;
					GradientCritter *MatNode;
					GeneralEffect *SetEffect;
					
					if (MatNode = Cmap->ByPixelGround->EcoMat.FindNode(0.0, 0.0))
						{
						if (Matthew = (MaterialEffect *)MatNode->GetThing())
							{
							SetEffect = Cmap->ByPixelGround;
							SumOfAllCoverages += 1.0;
							if (CurNode->NodeData->AddMaterial(Matthew, SetEffect, NULL, 1.0f))
								{
								Done = true;
								break;
								} // if
							else
								{
								Success = false;
								break;
								} // else
							} // if
						} // if
					} // if
				// find out what row and column it's in
				MaxRow = Cmap->Rast->Rows - 1;
				MaxCol = Cmap->Rast->Cols - 1;
				if (Cmap->Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
					{
					CurRow = (long)((Cmap->North - Vert.xyz[1]) / Cmap->DegLatPerCell);
					CurCol = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
					} // if
				else
					{
					CurCol = (long)((Vert.xyz[1] - Cmap->South) / Cmap->DegLatPerCell);
					CurRow = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
					} // else west at top
				if (CurRow > MaxRow)
					CurRow = MaxRow;
				if (CurCol > MaxCol)
					CurCol = MaxCol;
				if (Cmap->RandomizeEdges)
					{
					if (! RandomInit)
						{
					
						LonSeed = (ULONG)((CurNode->Lon - WCS_floor(CurNode->Lon)) * ULONG_MAX);
						LatSeed = (ULONG)((CurNode->Lat - WCS_floor(CurNode->Lat)) * ULONG_MAX);
						Rand.Seed64BitShift(LatSeed + WCS_SEEDOFFSET_CMAP, LonSeed + WCS_SEEDOFFSET_CMAP);
						RandomLatShift = Rand.GenPRN() - .5;
						RandomLonShift = Rand.GenPRN() - .5;
						RandomInit = 1;
						} // if
					if (Cmap->Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
						{
						Vert.xyz[1] += (RandomLatShift * Cmap->DegLatPerCell);
						Vert.xyz[0] += (RandomLonShift * Cmap->DegLonPerCell);
						CurRow = (long)((Cmap->North - Vert.xyz[1]) / Cmap->DegLatPerCell);
						CurCol = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
						} // if
					else
						{
						Vert.xyz[0] += (RandomLonShift * Cmap->DegLonPerCell);
						Vert.xyz[1] += (RandomLatShift * Cmap->DegLatPerCell);
						CurCol = (long)((Vert.xyz[1] - Cmap->South) / Cmap->DegLatPerCell);
						CurRow = (long)((Cmap->West - Vert.xyz[0]) / Cmap->DegLonPerCell);
						} // else
					if (CurRow > MaxRow)
						CurRow = MaxRow;
					else if (CurRow < 0)
						CurRow = 0;
					if (CurCol > MaxCol)
						CurCol = MaxCol;
					else if (CurCol < 0)
						CurCol = 0;
					} // if
				// sample color map, no antialiasing
				if (Cmap->Rast->SampleByteCell3(SampleRGB, CurCol, CurRow, Abort))
					{
					if (! Cmap->EvalByPixel)
						{
						for (CurEco = Cmap->Ecosystems; CurEco; CurEco = CurEco->Next)
							{
							FoundEco = 0;
							Eco = (EcosystemEffect *)CurEco->Me;
							if (Eco->CmapMatch && Eco->Enabled)
								{
								if (Eco->CmapMatchRange)
									{
									if (SampleRGB[0] >= Eco->MatchColor[0] && SampleRGB[1] >= Eco->MatchColor[1] && SampleRGB[2] >= Eco->MatchColor[2] &&
										SampleRGB[0] <= Eco->MatchColor[3] && SampleRGB[1] <= Eco->MatchColor[4] && SampleRGB[2] <= Eco->MatchColor[5])
										FoundEco = 1;
									} // if
								else if (SampleRGB[0] == Eco->MatchColor[0] && SampleRGB[1] == Eco->MatchColor[1] && SampleRGB[2] == Eco->MatchColor[2])
									FoundEco = 1;
								if (FoundEco)
									{
									if (CurNode->NodeData->Slope >= Eco->MinSlope &&	// both in degrees
										CurNode->NodeData->Slope <= Eco->MaxSlope)
										{
										if (CurNode->NodeData->RelEl >= Eco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MINRELEL].CurValue &&
											CurNode->NodeData->RelEl <= Eco->AnimPar[WCS_EFFECTS_ECOSYSTEM_ANIMPAR_MAXRELEL].CurValue)
											{
											if (Eco->EcoMat.GetRenderMaterial(&MatTwin, Eco->GetRenderMatGradientPos(Rend, NULL, CurNode, NULL)))
												{
												Done = true;
												if (MatTwin.Mat[0])
													{
													if (Eco->PlowSnow)
														PlowedSnow += MatTwin.Covg[0];
													SumOfAllCoverages += MatTwin.Covg[0] * (1.0 - MatTwin.Mat[0]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
													if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[0], Eco, NULL, (float)(MatTwin.Covg[0])))
														{
														Success = false;
														break;
														} // if
													} // if
												if (MatTwin.Mat[1])
													{
													if (Eco->PlowSnow)
														PlowedSnow += MatTwin.Covg[1];
													SumOfAllCoverages += MatTwin.Covg[1] * (1.0 - MatTwin.Mat[1]->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].CurValue);
													if (! CurNode->NodeData->AddMaterial(MatTwin.Mat[1], Eco, NULL, (float)(MatTwin.Covg[1])))
														{
														Success = false;
														break;
														} // if
													} // if
												} // if
											break;
											} // if
										} // if
									} // if
								} // if
							} // for
						} // if
					} // if colored pixel
				else if (Abort)
					{
					Cmap->Enabled = 0;
					} // else
				} // if in bounds
			} // if in lat bounds
		} // if enabled
	CurCmap = CurCmap->Next;
	} // while

return (Success);

} // CmapEffectBase::Eval

/*===========================================================================*/
/*===========================================================================*/

CmapEffect::CmapEffect()
: GeneralEffect(NULL)//, GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CMAP;
SetDefaults();

} // CmapEffect::CmapEffect

/*===========================================================================*/

CmapEffect::CmapEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)//, GeoReg(this)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CMAP;
SetDefaults();

} // CmapEffect::CmapEffect

/*===========================================================================*/

CmapEffect::CmapEffect(RasterAnimHost *RAHost, EffectsLib *Library, CmapEffect *Proto)
: GeneralEffect(RAHost)//, GeoReg(this)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_CMAP;
Prev = Library->LastCmap;
if (Library->LastCmap)
	{
	Library->LastCmap->Next = this;
	Library->LastCmap = this;
	} // if
else
	{
	Library->Cmap = Library->LastCmap = this;
	} // else
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
	strcpy(NameBase, "Cmap");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // CmapEffect::CmapEffect

/*===========================================================================*/

CmapEffect::~CmapEffect()
{
EffectList *NextEco;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->CMG && GlobalApp->GUIWins->CMG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->CMG;
		GlobalApp->GUIWins->CMG = NULL;
		} // if
	} // if

while (Ecosystems)
	{
	NextEco = Ecosystems;
	Ecosystems = Ecosystems->Next;
	delete NextEco;
	} // while

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	// Img may be NULLed out by RemoveAttribute()
	if (Img)
		delete Img;
	Img = NULL;
	} // if
	
} // CmapEffect::~CmapEffect

/*===========================================================================*/

void CmapEffect::SetDefaults(void)
{

Img = NULL;
RandomizeEdges = LuminousColors = 0;
EvalByPixel = WCS_CMAP_EVAL_BYPIXEL;
Orientation = WCS_CMAP_ORIENTATION_TOPNORTH;
Ecosystems = NULL;
ByPixelGround = NULL;
Rast = NULL;
Coords = NULL;
North = South = West = East = NSLow = NSHigh = WELow = WEHigh = DegLatPerCell = DegLonPerCell = 0.0;	// for use by renderer

} // CmapEffect::SetDefaults

/*===========================================================================*/

void CmapEffect::Copy(CmapEffect *CopyTo, CmapEffect *CopyFrom)
{
long Result = -1;
Raster *NewRast;
EffectList *NextEco, **ToEco;
RasterAttribute *MyAttr;
CoordSys *MyCoords;
NotifyTag Changes[2];

while (CopyTo->Ecosystems)
	{
	NextEco = CopyTo->Ecosystems;
	CopyTo->Ecosystems = CopyTo->Ecosystems->Next;
	delete NextEco;
	} // if
NextEco = CopyFrom->Ecosystems;
ToEco = &CopyTo->Ecosystems;
while (NextEco)
	{
	if (NextEco->Me)
		{
		if (*ToEco = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
				{
				(*ToEco)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEco->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextEco->Me->EffectType, NextEco->Me->Name))
					{
					Result = UserMessageCustom("Copy Color Map", "How do you wish to resolve Ecosystem name collisions?\n\nLink to existing Ecosystems, replace existing Ecosystems, or create new Ecosystems?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEco->Me->EffectType, NULL, NextEco->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextEco->Me);
					} // if link to existing
				else if ((*ToEco)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextEco->Me->EffectType, NextEco->Me->Name))
					{
					((EcosystemEffect *)(*ToEco)->Me)->Copy((EcosystemEffect *)(*ToEco)->Me, (EcosystemEffect *)NextEco->Me);
					Changes[0] = MAKE_ID((*ToEco)->Me->GetNotifyClass(), (*ToEco)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToEco)->Me);
					} // else if found and overwrite
				else
					{
					(*ToEco)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextEco->Me->EffectType, NULL, NextEco->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToEco)->Me)
				ToEco = &(*ToEco)->Next;
			else
				{
				delete *ToEco;
				*ToEco = NULL;
				} // if
			} // if
		} // if
	NextEco = NextEco->Next;
	} // while

if (CopyTo->Img)
	{
	if (CopyTo->Img->GetRaster())
		{
		CopyTo->Img->GetRaster()->RemoveAttribute(CopyTo->Img);
		} // if
	if (CopyTo->Img)
		delete CopyTo->Img;
	} // if
CopyTo->Img = NULL;
if (CopyFrom->Img)
	{
	if (CopyTo->Img = new RasterShell())
		{
		if (CopyFrom->Img->GetRaster())
			{
			if (NewRast = GlobalApp->CopyToImageLib->MatchNameMakeRaster(CopyFrom->Img->GetRaster()))
				{
				NewRast->AddAttribute(CopyTo->Img->GetType(), CopyTo->Img, CopyTo);
				// add georeference attribute
				if ((MyAttr = CopyFrom->Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
					{
					if (((GeoRefShell *)MyAttr->GetShell())->Host)
						{
						if (MyCoords = GlobalApp->CopyToEffectsLib->CompareMakeCoordSys((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host, TRUE))
							MyCoords->AddRaster(NewRast);

						/*
						if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
							{
							MyCoords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
							} // if no need to make another copy, its all in the family
						else
							{
							if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, ((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host)->Name))
								{
								Result = UserMessageCustom("Copy Color Map", "How do you wish to resolve Coordinate System name collision?\n\nLink to existing Coordinate System, replace existing Coordinate System, or create new Coordinate System?",
									"Link", "Create", "Overwrite", 1);
								} // if
							if (Result <= 0)
								{
								MyCoords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
								} // if create new
							else if (Result == 1)
								{
								MyCoords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
								} // if link to existing
							else if (MyCoords = (CoordSys *)GlobalApp->CopyToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, ((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host)->Name))
								{
								MyCoords->Copy(MyCoords, (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
								Changes[0] = MAKE_ID(MyCoords->GetNotifyClass(), MyCoords->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
								Changes[1] = NULL;
								GlobalApp->AppEx->GenerateNotify(Changes, MyCoords);
								} // else if found and overwrite
							else
								{
								MyCoords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
								} // else
							} // else better copy or overwrite it since its important to get just the right coordinate system
						if (MyCoords)
							MyCoords->AddRaster(NewRast);
						*/
						} // if
					}  // if
				} // if
			} // if
		} // if
	} // if
//GeoReg.Copy(&CopyTo->GeoReg, &CopyFrom->GeoReg);
CopyTo->RandomizeEdges = CopyFrom->RandomizeEdges;
CopyTo->LuminousColors = CopyFrom->LuminousColors;
CopyTo->Orientation = CopyFrom->Orientation;
CopyTo->EvalByPixel = CopyFrom->EvalByPixel;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // CmapEffect::Copy

/*===========================================================================*/

char *CmapEffect::GetCritterName(RasterAnimHost *Test)
{

//if (Test == &GeoReg)
//	return ("Map Bounds");

return ("");

} // CmapEffect::GetCritterName

/*===========================================================================*/

short CmapEffect::AnimateShadows(void)
{
RasterAttribute *MyAttr;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		return (((GeoRefShell *)MyAttr->GetShell())->GeoReg.AnimateShadows());
		} // if
	} // if

return (0);

} // CmapEffect::AnimateShadows

/*===========================================================================*/

char *CmapEffect::OKRemoveRaster()
{

if (Img)
	{
	return("Image Object is used as a Color Map! Remove anyway?");
	} // if
return (NULL);

} // CmapEffect::OKRemoveRaster

/*===========================================================================*/

void CmapEffect::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img == Shell)
	Img = NULL;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // CmapEffect::RemoveRaster

/*===========================================================================*/

int CmapEffect::SetRaster(Raster *NewRast)
{
NotifyTag Changes[2];

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	} // if
if (! Img)
	{
	Img = new RasterShell;
	} // else
if (Img && NewRast)
	{
	NewRast->AddAttribute(Img->GetType(), Img, this);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if
else
	{
	delete Img;		// delete it here since it wasn't added to a raster
	Img = NULL;
	return (0);
	} // else

} // CmapEffect::SetRaster

/*===========================================================================*/

ULONG CmapEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID = 0, GeoRegLoaded = 0;
short MatchEcoDummy = 1;
char EcoName[256];
EffectList **CurEco = &Ecosystems;
RasterAttribute *MyAttr;
GeoRegister GeoReg(NULL);

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
					case WCS_EFFECTS_CMAP_RANDOMEDGES:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeEdges, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CMAP_LUMINOUSCOLORS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LuminousColors, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CMAP_MATCHECO:
						{
						BytesRead = ReadBlock(ffile, (char *)&MatchEcoDummy, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CMAP_ORIENTATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&Orientation, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CMAP_EVALBYPIXEL:
						{
						BytesRead = ReadBlock(ffile, (char *)&EvalByPixel, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CMAP_GEOREG:
						{
						BytesRead = GeoReg.Load(ffile, Size, ByteFlip);
						GeoRegLoaded = 1;
						break;
						}
					case WCS_EFFECTS_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (Img = new RasterShell))
							{
							GlobalApp->LoadToImageLib->MatchRasterSetShell(ImageID, Img, this);
							if (GeoRegLoaded && Img->GetRaster())
								{
								// add GeoRef attribute to Raster if needed and set coords
								// since this is an old version from a time when we only supported geographic coord
								// it isn't necessary to attach a coordinate system.
								if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) ||
									(MyAttr = Img->GetRaster()->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF, NULL, NULL)))
									{
									GeoReg.Copy(&((GeoRefShell *)MyAttr->GetShell())->GeoReg, &GeoReg);
									((GeoRefShell *)MyAttr->GetShell())->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_EDGES;
									} // if
								} // if
							} // if
						break;
						}
					case WCS_EFFECTS_CMAP_ECONAME:
						{
						BytesRead = ReadBlock(ffile, (char *)EcoName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (EcoName[0])
							{
							if (*CurEco = new EffectList())
								{
								if ((*CurEco)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_ECOSYSTEM, EcoName))
									CurEco = &(*CurEco)->Next;
								else
									{
									delete *CurEco;
									*CurEco = NULL;
									} // else
								} // if
							} // if
						break;
						}
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

// MatchEcoDummy is only read from VNS 2 or earlier files and can only be 0 if read from file
// This is used to turn off per-polygon rendering in old projects if eco matching was not enabled.
if (! EvalByPixel && ! MatchEcoDummy)
	EvalByPixel = 1;
	
return (TotalRead);

} // CmapEffect::Load

/*===========================================================================*/

unsigned long int CmapEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long ImageID;
EffectList *CurEco;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CMAP_RANDOMEDGES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RandomizeEdges)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CMAP_LUMINOUSCOLORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&LuminousColors)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CMAP_ORIENTATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Orientation)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CMAP_EVALBYPIXEL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&EvalByPixel)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
/*
ItemTag = WCS_EFFECTS_CMAP_GEOREG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoReg.Save(ffile))
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
			} // if registration saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;
*/
if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_IMAGEID, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

CurEco = Ecosystems;
while (CurEco)
	{
	if (CurEco->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CMAP_ECONAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurEco->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurEco->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurEco = CurEco->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // CmapEffect::Save

/*===========================================================================*/

void CmapEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->CMG)
	{
	delete GlobalApp->GUIWins->CMG;
	}
GlobalApp->GUIWins->CMG = new CmapEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->CMG)
	{
	GlobalApp->GUIWins->CMG->Open(GlobalApp->MainProj);
	}

} // CmapEffect::Edit

/*===========================================================================*/

int CmapEffect::GetRAHostAnimated(void)
{
RasterAttribute *MyAttr;

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		if (((GeoRefShell *)MyAttr->GetShell())->GeoReg.GetRAHostAnimated())
			return (1);
		} // if
	} // if

return (0);

} // CmapEffect::GetRAHostAnimated

/*===========================================================================*/

int CmapEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (ByPixelGround)
	ByPixelGround->SetToTime(Time);		// only exists during rendering, copy of GeoReg contained

return (Found);

} // CmapEffect::SetToTime

/*===========================================================================*/

long CmapEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurEco = Ecosystems;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	NumImages ++;
	} // if
while (CurEco)
	{
	if (CurEco->Me)
		NumImages += CurEco->Me->InitImageIDs(ImageID);
	CurEco = CurEco->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // CmapEffect::InitImageIDs

/*===========================================================================*/

int CmapEffect::BuildFileComponentsList(EffectList **Ecosys, EffectList **Material3Ds, EffectList **Object3Ds, 
	EffectList **Waves, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr, *CurEco = Ecosystems;
RasterAttribute *MyAttr;

while (CurEco)
	{
	if (CurEco->Me)
		{
		ListPtr = Ecosys;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurEco->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurEco->Me;
			else
				return (0);
			} // if
		if (! ((EcosystemEffect *)CurEco->Me)->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
			return (0);
		} // if
	CurEco = CurEco->Next;
	} // while

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		if (((GeoRefShell *)MyAttr->GetShell())->Host)
			{
			ListPtr = Coords;
			while (*ListPtr)
				{
				if ((*ListPtr)->Me == ((GeoRefShell *)MyAttr->GetShell())->Host)
					break;
				ListPtr = &(*ListPtr)->Next;
				} // if
			if (! (*ListPtr))
				{
				if (*ListPtr = new EffectList())
					(*ListPtr)->Me = (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host;
				else
					return (0);
				} // if
			} // if
		}  // if
	} // if

return (1);

} // CmapEffect::BuildFileComponentsList

/*===========================================================================*/

long CmapEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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

} // CmapEffect::GetKeyFrameRange

/*===========================================================================*/

EffectList *CmapEffect::AddEcosystem(GeneralEffect *AddMe)
{
EffectList **CurEco = &Ecosystems;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurEco)
		{
		if ((*CurEco)->Me == AddMe)
			return (NULL);
		CurEco = &(*CurEco)->Next;
		} // while
	if (*CurEco = new EffectList())
		{
		(*CurEco)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurEco);
	} // if

return (NULL);

} // CmapEffect::AddEcosystem

/*===========================================================================*/

int CmapEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurEco = Ecosystems, *PrevEco = NULL;
NotifyTag Changes[2];

while (CurEco)
	{
	if (CurEco->Me == (GeneralEffect *)RemoveMe)
		{
		if (PrevEco)
			PrevEco->Next = CurEco->Next;
		else
			Ecosystems = CurEco->Next;

		delete CurEco;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevEco = CurEco;
	CurEco = CurEco->Next;
	} // while

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // CmapEffect::RemoveRAHost

/*===========================================================================*/

char CmapEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (/*DropType == WCS_RAHOST_OBJTYPE_GEOREGISTER
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME
	|| */DropType == WCS_RAHOST_OBJTYPE_RASTER
	|| DropType == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
	return (1);

return (0);

} // CmapEffect::GetRAHostDropOK

/*===========================================================================*/

int CmapEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (CmapEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (CmapEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
/*
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_GEOREGISTER ||
	DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = GeoReg.ProcessRAHostDragDrop(DropSource);
	} // else if
*/
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = SetRaster((Raster *)DropSource->DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddEcosystem((GeneralEffect *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if

return (Success);

} // CmapEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long CmapEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_CMAP | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // CmapEffect::GetRAFlags

/*===========================================================================*/

RasterAnimHost *CmapEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;
EffectList *CurEco;
JoeList *CurJoe = Joes;

if (! Current)
	Found = 1;
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());
if (Img && Current == Img->GetRaster())
	Found = 1;
CurEco = Ecosystems;
while (CurEco)
	{
	if (Found)
		return (CurEco->Me);
	if (Current == CurEco->Me)
		Found = 1;
	CurEco = CurEco->Next;
	} // while
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // CmapEffect::GetRAHostChild

/*===========================================================================*/

// this function is verrrrry slow due to sorting the image by color
// There are no calls to this function from anywhere currently. 
// The only one there was was removed 4/11/00 by GRH. It needs more sophistication 
// in looking for range matches and giving the user more options as to what order to put things in.
int CmapEffect::SortEcosystems(void)
{
TriStimulus *Sorted;
unsigned long Elements, LowElement, HighElement, NumEcos = 0, EcoNum, 
	SumMatchedPixels, SumBlackPixels, UnmatchedEcos, *EcoMatchesFound;
int Success = 0, Done;
EffectList *CurEco;
EcosystemEffect **EcoEffectList;
short MatchColor[3];
char Str[256];
NotifyTag Changes[2];

if (CurEco = Ecosystems)
	{
	if (Img && Img->GetRaster())
		{
		if (Sorted = Img->GetRaster()->SortByteMaps(Elements))
			{
			while (CurEco)
				{
				if (CurEco->Me && CurEco->Me->Enabled && ((EcosystemEffect *)CurEco->Me)->CmapMatch)
					NumEcos ++;
				CurEco = CurEco->Next;
				} // while
			if (NumEcos && (EcoMatchesFound = (unsigned long *)AppMem_Alloc(NumEcos * sizeof (unsigned long), APPMEM_CLEAR)))
				{
				if (EcoEffectList = (EcosystemEffect **)AppMem_Alloc(NumEcos * sizeof (EcosystemEffect *), APPMEM_CLEAR))
					{
					CurEco = Ecosystems;
					EcoNum = 0;
					while (CurEco)
						{
						if (CurEco->Me && CurEco->Me->Enabled && ((EcosystemEffect *)CurEco->Me)->CmapMatch)
							{
							EcoEffectList[EcoNum] = (EcosystemEffect *)CurEco->Me;
							EcoMatchesFound[EcoNum] = 0;
							MatchColor[0] = EcoEffectList[EcoNum]->MatchColor[0];
							MatchColor[1] = EcoEffectList[EcoNum]->MatchColor[1];
							MatchColor[2] = EcoEffectList[EcoNum]->MatchColor[2];

							// find lower element of red match range
							for (LowElement = 0; LowElement < Elements && Sorted[LowElement].RGB[0] != MatchColor[0]; LowElement ++);	//lint !e722

							if (LowElement < Elements)
								{
								// find upper element of red match range
								for (HighElement = LowElement + 1; HighElement < Elements && Sorted[HighElement].RGB[0] == MatchColor[0]; HighElement ++);	//lint !e722

								if (HighElement >= Elements)
									{
									HighElement = Elements;
									} // if ran off the end of the array

								// find lower element of green match range
								for ( ; LowElement <= HighElement && Sorted[LowElement].RGB[1] != MatchColor[1]; LowElement ++);	//lint !e722

								if (LowElement <= HighElement)
									{
									// find upper element of green match range
									for ( ; HighElement >= LowElement && Sorted[HighElement].RGB[1] != MatchColor[1]; HighElement --);	//lint !e722

									if (HighElement > LowElement)
										{
										// find lower element of blue match range
										for ( ; LowElement <= HighElement && Sorted[LowElement].RGB[2] != MatchColor[2]; LowElement ++);	//lint !e722

										if (LowElement <= HighElement)
											{
											// find upper element of green match range
											for ( ; HighElement >= LowElement && Sorted[HighElement].RGB[2] != MatchColor[2]; HighElement --);	//lint !e722

											EcoMatchesFound[EcoNum] = HighElement - LowElement + 1;
											} // if found at least one blue match
										} // if
									// else only one match possible
									else if (Sorted[LowElement].RGB[2] == MatchColor[2])
										EcoMatchesFound[EcoNum] = 1;
									// else no matches found
									} // if found at least one green match
								} // if found at least one red match
							EcoNum ++;
							} // if
						CurEco = CurEco->Next;
						} // while

					// now do something with this information, like sort the ecosystem list, advise the user, etc.
					SumMatchedPixels = 0;
					UnmatchedEcos = 0;
					for (EcoNum = 0; EcoNum < NumEcos; EcoNum ++)
						{
						if (! EcoMatchesFound[EcoNum])
							{
							UnmatchedEcos ++;
							sprintf(Str, "%d matches found for %s Ecosystem", EcoMatchesFound[EcoNum], EcoEffectList[EcoNum]->GetName());
							UserMessageOK("Color Map: Sort Ecosystems", Str);
							} // if
						else
							Success = 1;
						SumMatchedPixels += EcoMatchesFound[EcoNum];
						} // for
					// subtract black pixels from unmatched total
					SumBlackPixels = 0;
					for (LowElement = 0; LowElement < Elements; LowElement ++)
						{
						if (! Sorted[LowElement].RGB[0] && ! Sorted[LowElement].RGB[1] && ! Sorted[LowElement].RGB[2])
							SumBlackPixels ++;
						} // for
					sprintf(Str, "%d%% of the image is black.", (SumBlackPixels * 100) / Elements);
					UserMessageOK("Color Map: Sort Ecosystems", Str);
					sprintf(Str, "%d unmatched ecosystems and %d unmatched pixels were found.", UnmatchedEcos, Elements - SumBlackPixels - SumMatchedPixels);
					UserMessageOK("Color Map: Sort Ecosystems", Str);
					if (Success)
						{
						// remove the old list and send notify
						while (CurEco = Ecosystems)
							{
							Ecosystems = Ecosystems->Next;
							delete CurEco;
							} // while
						Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

						// re-sort the ecosystem order by swapping EcoEffect pointers in the EffectList
						Done = 0;
						while (! Done)
							{
							Done = 1;
							for (EcoNum = 0; EcoNum < NumEcos - 1; EcoNum ++)
								{
								if (EcoMatchesFound[EcoNum] < EcoMatchesFound[EcoNum + 1])
									{
									Done = 0;
									swmem(&EcoMatchesFound[EcoNum], &EcoMatchesFound[EcoNum + 1], sizeof (unsigned long));
									swmem(&EcoEffectList[EcoNum], &EcoEffectList[EcoNum + 1], sizeof (EcosystemEffect *));
									} // if
								} // for
							} // while

						// add the ecosystems
						for (EcoNum = 0; EcoNum < NumEcos; EcoNum ++)
							{
							if (EcoMatchesFound[EcoNum])
								{
								// this will generate a notify
								AddEcosystem(EcoEffectList[EcoNum]);
								} // if
							} // for
						} // if

					AppMem_Free(EcoEffectList, NumEcos * sizeof (EcosystemEffect *));
					} // if effect list allocated
				AppMem_Free(EcoMatchesFound, NumEcos * sizeof (unsigned long));
				} // if list allocated
			delete [] Sorted;
			} // if raster sorted
		else
			{
			UserMessageOK("Color Map: Sort Ecosystems", "Error sorting Image Object for this Color Map!");
			} // else
		} // if
	else
		{
		UserMessageOK("Color Map: Sort Ecosystems", "There is no Image Object for this Color Map!");
		} // else
	} // if
else
	{
	UserMessageOK("Color Map: Sort Ecosystems", "There are no Ecosystems to sort!");
	} // else

return (Success);

} // CmapEffect::SortEcosystems

/*===========================================================================*/

int CmapEffect::GrabAllEcosystems(void)
{
int Success = 0;
EffectList *CurEco;
EcosystemEffect *CurEffect;
NotifyTag Changes[2];

while (CurEco = Ecosystems)
	{
	Ecosystems = Ecosystems->Next;
	delete CurEco;
	} // while
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

if (CurEffect = (EcosystemEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM))
	{
	while (CurEffect)
		{
		if (CurEffect->CmapMatch)
			{
			AddEcosystem(CurEffect);	// sends its own notify
			Success = 1;
			} // if
		CurEffect = (EcosystemEffect *)CurEffect->Next;
		} // while
	} // if

return (Success);

} // CmapEffect::GrabAllEcosystems

/*===========================================================================*/

void CmapEffect::SetBounds(double LatRange[2], double LonRange[2])
{
RasterAttribute *MyAttr;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		((GeoRefShell *)MyAttr->GetShell())->SetBounds(LatRange, LonRange);
		} // if
	} // if

/*
NotifyTag Changes[2];

// this will set new color map lat/lon bounds

// ensure that latitude is within bounds
if (LatRange[0] > 90.0)
	LatRange[0] = 90.0;
if (LatRange[0] < -90.0)
	LatRange[0] = -90.0;
if (LatRange[1] > 90.0)
	LatRange[1] = 90.0;
if (LatRange[1] < -90.0)
	LatRange[1] = -90.0;
// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(LatRange[0]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(LonRange[0]);
	Changes[0] = MAKE_ID(GeoReg.GetNotifyClass(), GeoReg.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GeoReg.GetRAHostRoot());
	} // if
*/
} // CmapEffect::SetBounds

/*===========================================================================*/

void CmapEffect::SetFloating(char NewFloating)
{
DEMBounds CurBounds;
double LatRange[2], LonRange[2];
RasterAttribute *MyAttr;

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	LatRange[0] = CurBounds.South;
	LatRange[1] = CurBounds.North;
	LonRange[0] = CurBounds.East;
	LonRange[1] = CurBounds.West;
	if (Img && Img->GetRaster())
		{
		if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			((GeoRefShell *)MyAttr->GetShell())->SetBounds(LatRange, LonRange);
			} // if
		} // if
	} // if

} // CmapEffect::SetFloating

/*===========================================================================*/
/*
int CmapEffect::CreateImageObject(Database *DBHost, ImageLib *ImageHost)
{
int Result, DEMsExist, Retry = 1;
long CellsNS, CellsWE, LatCt, LonCt, PtCt;
float MaxEl, MinEl;
double CellSize, CellSizeNS, CellSizeWE, MetersPerDegLat, MetersPerDegLon, AvgLat, LatRange, LonRange, LatPt, LonPt, 
	CurElev, ElevRange, LowElev;
char CellSizeStr[64], CellSizeMessage[128];
PlanetOpt *DefPlanetOpt;
NotifyTag Changes[2];

if (DEMsExist = DBHost->GetMinDEMCellSizeMeters(CellSizeNS, CellSizeWE))
	{
	CellSize = CellSizeNS;
	} // if
else
	{
	CellSize = 30.0;
	} // else

// ask user what cell size
sprintf(CellSizeStr, "%f", CellSize);
TrimZeros(CellSizeStr);

while (Retry)
	{
	Retry = 0;
	if (GetInputString("Enter the desired North-South pixel size in meters. West-East pixel size will be smaller.", WCS_REQUESTER_POSDIGITS_ONLY, CellSizeStr))
		{
		CellSize = atof(CellSizeStr);
		if (CellSize > 0.0)
			{
			GeoReg.ValidateBoundsOrder();
			LatRange = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue -
				GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
			LonRange = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue -
				GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
			AvgLat = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue +
				GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) / 2.0;
			if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
				{
				MetersPerDegLat = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
				MetersPerDegLon = MetersPerDegLat * cos(AvgLat * PiOver180);
				CellsNS = (long)WCS_ceil(LatRange * MetersPerDegLat / CellSize);
				CellsWE = (long)WCS_ceil(LonRange * MetersPerDegLon / CellSize);
				CellSizeNS = LatRange / CellsNS;
				CellSizeWE = LonRange / CellsWE;
				sprintf(CellSizeMessage, "This will result in an image %d columns by %d rows.", CellsWE, CellsNS);
				if ((Result = UserMessageCustom("Create Image Object", CellSizeMessage, "OK", "Cancel", "Reset Size", 1)) == 0)
					return (0);
				else if (Result == 2)
					Retry = 1;
				else
					{
					if (Rast = new Raster())
						{
						Rast->Rows = CellsNS;
						Rast->Cols = CellsWE;
						if (Rast->AllocCmapBands())
							{
							if (DEMsExist && UserMessageYN("Create Image Object", "Plot elevation of terrain into new Color Map Image?"))
								{
								DBHost->GetDEMElevRange(MaxEl, MinEl);
								LowElev = MinEl;
								if ((ElevRange = MaxEl - MinEl) > 0.0)
									{
									for (LatCt = PtCt = 0, LatPt = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - CellSizeNS / 2.0; LatCt < CellsNS; LatCt ++, LatPt -= CellSizeNS)
										{
										for (LonCt = 0, LonPt = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - CellSizeWE / 2.0; LonCt < CellsWE; LonCt ++, PtCt ++, LonPt -= CellSizeWE)
											{
											CurElev = GlobalApp->MainProj->Interactive->ElevationPoint(LatPt, LonPt);
											CurElev = (CurElev - LowElev) / ElevRange;
											if (CurElev < 0.0)
												CurElev = 0.0;
											if (CurElev > 1.0)
												CurElev = 1.0;
											Rast->Red[PtCt] = Rast->Green[PtCt] = Rast->Blue[PtCt] = (unsigned char)(CurElev * 255.99);
											} // for
										} // for
									} // if
								} // if
							if (UserMessageYN("Create Image Object", "Plot enabled vectors into new Color Map Image?"))
								{
								PlotVectors(DBHost, Rast, CellSizeNS, CellSizeWE);
								} // if
							// get file name
							Rast->PAF.SetPath(GlobalApp->MainProj->dirname);
							Rast->PAF.SetName(Name);
							
							if (Rast->SaveImage(0))
								{

								// add raster to image lib
								ImageHost->AddRaster(Rast);

								// set north at top
								Orientation = WCS_CMAP_ORIENTATION_TOPNORTH;

								// generate value change notifications
								Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
								Changes[1] = NULL;
								GlobalApp->AppEx->GenerateNotify(Changes, NULL);

								// set color map image object
								SetRaster(Rast);

								} // if
							return (1);
							} // if
						} // if
					} // else
				} // if
			} // if
		} // if
	} // while

return (0);

} // CmapEffect::CreateImageObject
*/
/*===========================================================================*/
/*
void CmapEffect::PlotVectors(Database *DBHost, Raster *DrawRast, double CellSizeNS, double CellSizeWE)
{
unsigned char Red, Grn, Blu;
short a, b, LWidth, UWidth;
long x, y, CurRow, Zip;
double m, XS, XE, YS, YE, LastLat, LastLon;
Joe *BillyBob;
VectorPoint *Point;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;
VectorClipper Clipper;

DBHost->SetGeoClip(GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue, 
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue, 
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue, 
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue);

Clipper.LowX = Clipper.LowY = 0;
Clipper.HighX = (short)(DrawRast->Cols - 1);
Clipper.HighY = (short)(DrawRast->Rows - 1);

for (BillyBob = DBHost->GetFirst(); BillyBob; BillyBob = DBHost->GetNext(BillyBob))
	{
	if(BillyBob->TestFlags(WCS_JOEFLAG_ACTIVATED) && BillyBob->TestFlags(WCS_JOEFLAG_DRAWENABLED))
		{
		if(! BillyBob->TestFlags(WCS_JOEFLAG_ISDEM) && BillyBob->Points && BillyBob->NumPoints > 1)
			{
			if (MyAttr = (JoeCoordSys *)BillyBob->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
				MyCoords = MyAttr->Coord;
			else
				MyCoords = NULL;

			Red = BillyBob->Red();
			Grn = BillyBob->Green();
			Blu = BillyBob->Blue();

			LWidth = BillyBob->GetLineWidth() / 2;
			UWidth = BillyBob->GetLineWidth() / 2 + BillyBob->GetLineWidth() % 2;
			Point = BillyBob->Points->Next;
			if (Point && Point->ProjToDefDeg(MyCoords, &MyVert))
				{
				for (; Point; Point = Point->Next)
					{
					LastLat = MyVert.Lat;
					LastLon = MyVert.Lon;
					YS = YE = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - LastLat) / CellSizeNS;
					XS = XE = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - LastLon) / CellSizeWE;
					if (BillyBob->GetLineStyle() >= 4)
						{
						if (Point->Next && Point->Next->ProjToDefDeg(MyCoords, &MyVert))
							{ // if
							YE = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - MyVert.Lat) / CellSizeNS;
							XE = (GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - MyVert.Lon) / CellSizeWE;
							} // if not last point
						else
							{
							break;
							} // else done with this object
						} // if line style
					else if (Point->Next && ! Point->Next->ProjToDefDeg(MyCoords, &MyVert))
						{
						break;
						} // else if

					if (Clipper.ClipSeg(XS, YS, XE, YE))
						{
						// if line draw style 

						if (BillyBob->GetLineStyle() >= 4)
							{
							if (YE < YS)
								{
								swmem(&XE, &XS, 8);
								swmem(&YE, &YS, 8);
								}
							if (YE == YS)
								m = 0.0;
							else
								m = (XE - XS) / (YE - YS);
							for (a = -LWidth; a < UWidth; a ++)
								{
								for (b = -LWidth; b < UWidth; b ++)
									{
									CurRow = b + (long)YS;
									for (y = 0; y <= (long)(YE - YS); y ++, CurRow ++)
										{
										if (CurRow >= DrawRast->Rows)
											break;
										if (CurRow < 0)
											continue;
										x = (long)(m * y + XS + a);
										if (x < 0 || x >= DrawRast->Cols)
											continue;
										Zip = DrawRast->RowZip[CurRow] + x;
										DrawRast->Red[Zip] = Red;
										DrawRast->Green[Zip] = Grn;
										DrawRast->Blue[Zip] = Blu;
										} // for y=0... 
									} // for b=... 
								} // for a=... 

							if (XE < XS)
								{
								swmem(&XS, &XE, 8);
								swmem(&YS, &YE, 8);
								}
							if (XE == XS)
								m = 0.0;
							else
								m = (YE - YS) / (XE - XS);
							for (a = -LWidth; a < UWidth; a ++)
								{
								for (b = -LWidth; b < UWidth; b ++)
									{
									CurRow = b + (long)XS;
									for (x = 0; x <= (long)(XE - XS); x ++, CurRow ++)
										{
										if (CurRow >DrawRast->Cols)
											break;
										if (CurRow < 0)
											continue;
										y = (long)(m * x + YS + a);
										if (y < 0 || y >DrawRast->Rows)
											continue;
										Zip = DrawRast->RowZip[y] + CurRow;
										DrawRast->Red[Zip] = Red;
										DrawRast->Green[Zip] = Grn;
										DrawRast->Blue[Zip] = Blu;
										} // for x=0...
									} // for b=... 
								} // for a=... 

							} // if line pattern 

						else
							{
							for (a = -LWidth; a < UWidth; a ++)
								{
								x = (long)(XS + a);
								if (x >= DrawRast->Cols)
									break;
								if (x < 0)
									continue;
								for (b = -LWidth; b < UWidth; b ++)
									{
									CurRow = (long)(b + YS);
									if (CurRow >= DrawRast->Rows)
										break;
									if (CurRow < 0)
										continue;
									Zip = DrawRast->RowZip[CurRow] + x;
									DrawRast->Red[Zip] = Red;
									DrawRast->Green[Zip] = Grn;
									DrawRast->Blue[Zip] = Blu;
									} // for b=... 
								} // for a=... 
							} // else point pattern 
						} // if in bounds
					} // for point
				} // if
			} // if not DEM
		} // if enabled
	} // for BillyBob... objects 

} // CmapEffect::PlotVectors
*/
/*===========================================================================*/

int CmapEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
int Success = 1;
GradientCritter *MatNode;
MaterialEffect *Mat;
RootTexture *Root;
Texture *Tex;
RasterAttribute *MyAttr;

Rast = NULL;
Coords = NULL;

if (Img && Img->GetRaster() && (MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	Rast = Img->GetRaster();
	if (EvalByPixel)
		{
		if (ByPixelGround = new GroundEffect(this))
			{
			// create texture
			if (MatNode = ByPixelGround->EcoMat.FindNode(0.0, 0.0))
				{
				if (Mat = (MaterialEffect *)MatNode->GetThing())
					{
					if (LuminousColors)
						Mat->AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetValue (1.0);
					// create root texture
					if (Root = Mat->NewRootTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
						{
						// set texture type to planar image
						if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_PLANARIMAGE))
							{
							// install image
							Tex->SetRaster(Img->GetRaster());
							Tex->CoordSpace = WCS_TEXTURE_COORDSPACE_IMAGE_GEOREFERENCED;
							// initialize
							if (! ByPixelGround->InitToRender(Opt, Buffers))
								Success = 0;
							} // if
						else
							Success = 0;
						} // if
					else
						Success = 0;
					} // if
				else
					Success = 0;
				} // if
			else
				Success = 0;
			} // if
		else
			Success = 0;
		} // if
	} // if

return (Success);

} // CmapEffect::InitToRender

/*===========================================================================*/

int CmapEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
int BoundsType;
GeoRegister *GeoReg;
RasterAttribute *MyAttr;

Rast = NULL;
Coords = NULL;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		GeoReg = &((GeoRefShell *)MyAttr->GetShell())->GeoReg;
		Coords = (CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host;
		BoundsType = ((GeoRefShell *)MyAttr->GetShell())->BoundsType;

		North = NSHigh = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
		South = NSLow = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		West = WEHigh = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
		East = WELow = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;

		if (Rast = Img->GetRaster()->Rows && Img->GetRaster()->Cols ? Img->GetRaster(): NULL)
			{
			if (BoundsType == WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS)
				{
				if (Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
					{
					// these values are in units of the projection, either degrees or meters
					// measured from south and east
					DegLatPerCell = (North - South) / (Rast->Rows - 1); //lint !e413
					DegLonPerCell = (West - East) / (Rast->Cols - 1); //lint !e413
					} // if
				else
					{
					DegLatPerCell = (North - South) / (Rast->Cols - 1); //lint !e413
					DegLonPerCell = (West - East) / (Rast->Rows - 1); //lint !e413
					} // else
				South -= DegLatPerCell * .5;
				North += DegLatPerCell * .5;
				East -= DegLonPerCell * .5;
				West += DegLonPerCell * .5;
				} // if
			else
				{
				if (Orientation == WCS_CMAP_ORIENTATION_TOPNORTH)
					{
					// these values are in units of the projection, either degrees or meters
					// measured from south and east
					DegLatPerCell = (North - South) / Rast->Rows; //lint !e413
					DegLonPerCell = (West - East) / Rast->Cols; //lint !e413
					} // if
				else
					{
					DegLatPerCell = (North - South) / Rast->Cols; //lint !e413
					DegLonPerCell = (West - East) / Rast->Rows; //lint !e413
					} // else
				} // else

			if (NSHigh < NSLow)
				swmem(&NSHigh, &NSLow, sizeof (double));
			if (WEHigh < WELow)
				swmem(&WEHigh, &WELow, sizeof (double));

			if (ByPixelGround)
				{
				if (! ByPixelGround->InitFrameToRender(Lib, Rend))
					return (0);
				} // if
			} // if

		} // if
	} // if

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // CmapEffect::InitFrameToRender

/*===========================================================================*/

void CmapEffect::CleanupFromRender(void)
{

if (ByPixelGround)
	{
	delete ByPixelGround;
	ByPixelGround = NULL;
	} // if
Rast = NULL;
Coords = NULL;

} // CmapEffect::CleanupFromRender

/*===========================================================================*/

void CmapEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ScaleWE, ScaleNS, TempVal;
GraphNode *CurNode;
GeoRegister *GeoReg;
RasterAttribute *MyAttr;

if (OldBounds->West > OldBounds->East)
	ScaleWE = (CurBounds->West - CurBounds->East) / (OldBounds->West - OldBounds->East);
else
	ScaleWE = 1.0;
if (OldBounds->North > OldBounds->South)
	ScaleNS = (CurBounds->North - CurBounds->South) / (OldBounds->North - OldBounds->South);
else
	ScaleNS = 1.0;

if (Img && Img->GetRaster())
	{
	if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
		{
		GeoReg = &((GeoRefShell *)MyAttr->GetShell())->GeoReg;

		GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(
			(GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - OldBounds->South) * ScaleNS + CurBounds->South);
		if (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetFirstNode(0))
			{
			TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
			if (TempVal > 90.0)
				TempVal = 90.0;
			if (TempVal < -90.0)
				TempVal = -90.0;
			CurNode->SetValue(TempVal);
			while (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].GetNextNode(0, CurNode))
				{
				TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
				if (TempVal > 90.0)
					TempVal = 90.0;
				if (TempVal < -90.0)
					TempVal = -90.0;
				CurNode->SetValue(TempVal);
				} // while
			} // if
		GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(
			(GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue - OldBounds->South) * ScaleNS + CurBounds->South);
		if (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetFirstNode(0))
			{
			TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
			if (TempVal > 90.0)
				TempVal = 90.0;
			if (TempVal < -90.0)
				TempVal = -90.0;
			CurNode->SetValue(TempVal);
			while (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].GetNextNode(0, CurNode))
				{
				TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
				if (TempVal > 90.0)
					TempVal = 90.0;
				if (TempVal < -90.0)
					TempVal = -90.0;
				CurNode->SetValue(TempVal);
				} // while
			} // if
		GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(
			(GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - OldBounds->East) * ScaleWE + CurBounds->East);
		if (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetFirstNode(0))
			{
			CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
			while (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].GetNextNode(0, CurNode))
				{
				CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
				} // while
			} // if
		GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(
			(GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - OldBounds->East) * ScaleWE + CurBounds->East);
		if (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetFirstNode(0))
			{
			CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
			while (CurNode = GeoReg->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].GetNextNode(0, CurNode))
				{
				CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
				} // while
			} // if
		} // if
	} // if

} // CmapEffect::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int CmapEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
CmapEffect *CurrentCmap = NULL;
EcosystemEffect *CurrentEco = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
RasterAttribute *MyAttr;
DEMBounds OldBounds, CurBounds;

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

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
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if material
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
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
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if 3d material
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if wave
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentObj->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "Ecosys", 8))
						{
						if (CurrentEco = new EcosystemEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM))
							{
							if ((BytesRead = CurrentEco->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;
							}
						} // if eco
					else if (! strnicmp(ReadBuf, "ColorMap", 8))
						{
						if (CurrentCmap = new CmapEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentCmap->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Cmap
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
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentCmap)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM) && UserMessageYN("Load Color Map", "Do you wish the loaded Color Map's Ecosystem\n elevation lines to be scaled to current DEM elevations?"))
			{
			for (CurrentEco = (EcosystemEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); CurrentEco; CurrentEco = (EcosystemEffect *)CurrentEco->Next)
				{
				CurrentEco->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE) && UserMessageYN("Load Color Map", "Do you wish the loaded Color Map's Wave positions\n to be scaled to current DEM bounds?"))
			{
			for (CurrentWave = (WaveEffect *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_WAVE); CurrentWave; CurrentWave = (WaveEffect *)CurrentWave->Next)
				{
				CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (Img && Img->GetRaster())
			{
			if ((MyAttr = Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
				{
				// don't scale bounds if there is a CoordSys attached
				if (! ((GeoRefShell *)MyAttr->GetShell())->Host)
					{
					if (UserMessageYN("Load Color Map", "Do you wish the loaded Color Map's bounds\n to be scaled to current DEM bounds?"))
						{
						CurrentCmap->ScaleToDEMBounds(&OldBounds, &CurBounds);
						} // if
					} // if
				}  // if
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentCmap);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // CmapEffect::LoadObject

/*===========================================================================*/

int CmapEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *EcoList = NULL, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
DEMBounds CurBounds;

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

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
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
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

if (BuildFileComponentsList(&EcoList, &Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords)
	&& GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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

	CurEffect = Waves;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Wave");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((WaveEffect *)CurEffect->Me)->Save(ffile))
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
						} // if wave saved 
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

	CurEffect = Material3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Matl3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((MaterialEffect *)CurEffect->Me)->Save(ffile))
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
						} // if material saved 
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

	CurEffect = Object3Ds;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Object3D");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((Object3DEffect *)CurEffect->Me)->Save(ffile))
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
						} // if 3D Object saved 
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

	CurEffect = EcoList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Ecosys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((EcosystemEffect *)CurEffect->Me)->Save(ffile))
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
						} // if 3D Object saved 
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
	while (EcoList)
		{
		CurEffect = EcoList;
		EcoList = EcoList->Next;
		delete CurEffect;
		} // while
	while (Material3Ds)
		{
		CurEffect = Material3Ds;
		Material3Ds = Material3Ds->Next;
		delete CurEffect;
		} // while
	while (Object3Ds)
		{
		CurEffect = Object3Ds;
		Object3Ds = Object3Ds->Next;
		delete CurEffect;
		} // while
	while (Waves)
		{
		CurEffect = Waves;
		Waves = Waves->Next;
		delete CurEffect;
		} // while
	} // if

// ColorMap
strcpy(StrBuf, "ColorMap");
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
			} // if ColorMap saved 
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

} // CmapEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::CmapEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
CmapEffect *Current;

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
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new CmapEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (CmapEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::CmapEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::CmapEffect_Save(FILE *ffile)
{
CmapEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Cmap;
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
					} // if Cmap effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (CmapEffect *)Current->Next;
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

} // EffectsLib::CmapEffect_Save()

/*===========================================================================*/
