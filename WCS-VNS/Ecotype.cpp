// Ecotype.cpp
// For managing WCS foliage
// Built from scratch on 04/10/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Ecotype.h"
#include "Texture.h"
#include "Raster.h"
#include "Useful.h"
#include "Application.h"
#include "EffectsLib.h"
#include "requester.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "Toolbar.h"
#include "LakeEditGUI.h"
#include "StreamEditGUI.h"
#include "EcosystemEditGUI.h"
#include "FoliageEffectEditGUI.h"
#include "Render.h"
#include "Database.h"
#include "Project.h"
#include "Security.h"
#include "KeyScaleDeleteGUI.h"
#include "Lists.h"
#include "ProjUpdateGUI.h"
#include "FeatureConfig.h"

extern int EngineOnly;

Ecotype::Ecotype()
: RasterAnimHost(NULL)
{

SetDefaults();

} // Ecotype::Ecotype

/*===========================================================================*/

Ecotype::Ecotype(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{

SetDefaults();

} // Ecotype::Ecotype

/*===========================================================================*/

Ecotype::~Ecotype()
{
FoliageGroup *NextFolGrp;
RootTexture *DelTex;
long Ct;

//if (GlobalApp->GUIWins)
//	{
//	if (GlobalApp->GUIWins->FEG && GlobalApp->GUIWins->FEG->GetActive() == this)
//		{
//		delete GlobalApp->GUIWins->FEG;
//		GlobalApp->GUIWins->FEG = NULL;
//		} // if
//	} // if

while (FolGrp)
	{
	NextFolGrp = FolGrp;
	FolGrp = FolGrp->Next;
	delete NextFolGrp;
	} // while
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

// delete foliage chain
#ifdef WCS_USE_OLD_FOLIAGE
DeleteFoliageChain();
#endif // WCS_USE_OLD_FOLIAGE

} // Ecotype::~Ecotype

/*===========================================================================*/

void Ecotype::SetDefaults(void)
{
double EffectDefault[WCS_ECOTYPE_NUMANIMPAR] = {10.0, 0.0, 50.0, .2, 100.0, .2, 20.0};
double RangeDefaults[WCS_ECOTYPE_NUMANIMPAR][3] = {100000.0, 0.0, 1.0,		// max height
													100000.0, 0.0, 1.0,		// min height
													100000.0, 0.0, 1.0,		// density
													100000.0, 0.0, .1,		// dbh
													FLT_MAX, 0.0, 1.0,		// basal area
													5.0, 0.0, .01,		// crown closure
													100000.0, 0.0, 1.0		// age
													};
long Ct;
GraphNode *Node;

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
DissolveColor.SetDefaults(this, (char)Ct ++);
DBHCurve.SetDefaults(this, (char)Ct ++);
AgeCurve.SetDefaults(this, (char)Ct ++);
FolGrp = NULL;
#ifdef WCS_USE_OLD_FOLIAGE
ChainList = NULL;
#endif // WCS_USE_OLD_FOLIAGE
DensityPerSqMeter = DensityNormalizer = 0.0;
DissolveDistance = 0.0;
ConstDensity = WCS_FOLIAGE_DENSITY_CONSTANT;
DensityUnits = WCS_FOLIAGE_DENSITY_HECTARE;
BasalAreaUnits = WCS_FOLIAGE_BASALAREAUNITS_SQFOOT;
DissolvePixelHeight = 2.0;
DissolveEnabled = 0;
RenderOccluded = 0;
Enabled = 1;
HeightRange = 0.0;
PixelTexturesExist = 0;
AbsHeightResident = AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;
SecondHeightType = WCS_ECOTYPE_SECONDHT_MINABS;
PrevSizeMethod = SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;

DBHCurve.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AgeCurve.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

DBHCurve.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
DBHCurve.SetHorizontalMetric(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AgeCurve.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

strcpy(AgeCurve.XLabel, "Age");
strcpy(AgeCurve.YLabel, "Ht.");

strcpy(DBHCurve.XLabel, "DBH");
strcpy(DBHCurve.YLabel, "Ht.");

AgeCurve.RemoveNode(10.0, 0.1);
AgeCurve.AddNode(12.5, 7.0, 0.0);
AgeCurve.AddNode(25.0, 12.0, 0.0);
AgeCurve.AddNode(50.0, 17.0, 0.0);
if (Node = AgeCurve.AddNode(100.0, 20.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

DBHCurve.RemoveNode(10.0, 0.1);
DBHCurve.AddNode(.125, 7.0, 0.0);
DBHCurve.AddNode(.25, 12.0, 0.0);
DBHCurve.AddNode(.5, 17.0, 0.0);
if (Node = DBHCurve.AddNode(1.0, 20.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

DBHCurve.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AgeCurve.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

SetAnimDefaults();

#ifdef WCS_FORESTRY_WIZARD
AvgImageHeightWidthRatio = BasalAreaUnitConversion = 1.0;
EcoHeightTheme = EcoMinHeightTheme = EcoDbhTheme = EcoAgeTheme = EcoClosureTheme = EcoStemsTheme = EcoBasalAreaTheme = NULL;
EcoDensityTex = EcoSizeTex = NULL;
#endif // WCS_FORESTRY_WIZARD

} // Ecotype::SetDefaults

/*===========================================================================*/

void Ecotype::SetAnimDefaults(void)
{
FoliageGroup *Current;
double RangeDefaults[3][3] = {100000.0, 0.0, 1.0, 100000.0, 0.0, .1, 1.0, 0.0, .01};

Current = FolGrp;
while (Current)
	{
	Current->SetAnimDefaults(this);
	Current = Current->Next;
	} // while

AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
	{
	if (! GlobalApp->ForestryAuthorized || SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
		{
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
		} // if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[1]);
		} // if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
		} // if
	else	// crown closure
		{
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMultiplier(100.0);
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[2]);
		} // if
	} // if
else
	{
	AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
	AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
	AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
	} // else
AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].SetMultiplier(100.0);

} // Ecotype::SetAnimDefaults

/*===========================================================================*/

void Ecotype::Copy(Ecotype *CopyTo, Ecotype *CopyFrom)
{
FoliageGroup *CurrentFrom = CopyFrom->FolGrp, **ToPtr, *NextFolGrp;
RasterAnimHost *Root;
RasterAnimHostProperties Prop;
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
AgeCurve.Copy(&CopyTo->AgeCurve, &CopyFrom->AgeCurve);
DBHCurve.Copy(&CopyTo->DBHCurve, &CopyFrom->DBHCurve);

// delete existing foliage groups
while (CopyTo->FolGrp)
	{
	NextFolGrp = CopyTo->FolGrp;
	CopyTo->FolGrp = CopyTo->FolGrp->Next;
	delete NextFolGrp;
	} // while

ToPtr = &CopyTo->FolGrp;

while (CurrentFrom)
	{
	if (*ToPtr = new FoliageGroup(CopyTo))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

Root = CopyTo->GetRAHostRoot();
CopyTo->DissolveColor.Copy(&CopyTo->DissolveColor, &CopyFrom->DissolveColor);
CopyTo->DissolveDistance = CopyFrom->DissolveDistance;
CopyTo->ConstDensity = CopyFrom->ConstDensity;
CopyTo->DensityUnits = CopyFrom->DensityUnits;
CopyTo->BasalAreaUnits = CopyFrom->BasalAreaUnits;
CopyTo->DissolvePixelHeight = CopyFrom->DissolvePixelHeight;
CopyTo->DissolveEnabled = CopyFrom->DissolveEnabled;
CopyTo->RenderOccluded = CopyFrom->RenderOccluded;
CopyTo->AbsHeightResident = CopyFrom->AbsHeightResident;
CopyTo->SecondHeightType = CopyFrom->SecondHeightType;
CopyTo->SizeMethod = CopyFrom->SizeMethod;
CopyTo->DensityMethod = CopyFrom->DensityMethod;
if (Root)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		CopyTo->AbsDensResident = WCS_ECOTYPE_ABSRESIDENT_ECOTYPE;
		CopyTo->DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
		if (CopyTo->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)	// crown closure requires density and area
			CopyTo->SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;
		CopyTo->Enabled = 1;	// not adjustable from interface in Foliage Effects
		} // if
	else
		CopyTo->AbsDensResident = CopyFrom->AbsDensResident;
	} // if
else
	CopyTo->AbsDensResident = CopyFrom->AbsDensResident;
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->PrevSizeMethod = CopyFrom->PrevSizeMethod;
CopyTo->SetAnimDefaults();
RootTextureParent::Copy(CopyTo, CopyFrom);
ThematicOwner::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // Ecotype::Copy

/*===========================================================================*/

FoliageGroup *Ecotype::FindFoliageGroup(char *FindName)
{
FoliageGroup *CurFolGrp = FolGrp;

while (CurFolGrp)
	{
	if (! stricmp(CurFolGrp->Name, FindName))
		break;
	CurFolGrp = CurFolGrp->Next;
	} // while

return (CurFolGrp);

} // Ecotype::FindFoliageGroup

/*===========================================================================*/

FoliageGroup *Ecotype::AddFoliageGroup(FoliageGroup *CopyFolGrp, char *NewName)
{
FoliageGroup **LoadTo = &FolGrp;
NotifyTag Changes[2];

while (*LoadTo)
	{
	LoadTo = &(*LoadTo)->Next;
	} // while
if (*LoadTo = new FoliageGroup(this))
	{
	if (NewName)
		{
		strncpy((*LoadTo)->Name, NewName, WCS_EFFECT_MAXNAMELENGTH);
		(*LoadTo)->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
		} // if
	(*LoadTo)->SetAnimDefaults(this);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), (*LoadTo)->GetNotifySubclass(), WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	if (CopyFolGrp)
		{
		(*LoadTo)->Copy(*LoadTo, CopyFolGrp);
		Changes[0] = MAKE_ID((*LoadTo)->GetNotifyClass(), (*LoadTo)->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, (*LoadTo)->GetRAHostRoot());
		} // if
	} // if

return (*LoadTo);

} // Ecotype::AddFoliageGroup

/*===========================================================================*/

int Ecotype::SetToTime(double Time)
{
FoliageGroup *Current = FolGrp;
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (DissolveColor.SetToTime(Time))
	{
	Found = 1;
	} // if
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while

return (Found);

} // Ecotype::SetToTime

/*===========================================================================*/

int Ecotype::AnimateShadows(void)
{
FoliageGroup *Current = FolGrp;
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0))
		{
		return (1);
		} // if
	} // for
for (Ct = WCS_ECOTYPE_TEXTURE_HEIGHT; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled && TexRoot[Ct]->IsAnimated())
		{
		return (1);
		} // if
	} // for

while (Current)
	{
	if (Current->Enabled && Current->AnimateShadows())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // Ecotype::AnimateShadows

/*===========================================================================*/

void Ecotype::EditRAHost(void)
{
RasterAnimHost *Root;
RasterAnimHostProperties Prop;
bool OpenEd = true;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;

DONGLE_INLINE_CHECK()
if (Root = GetRAHostRoot())
	{
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			OpenEd = false;
		} // if
	if (OpenEd)
		Root->EditRAHost();
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->FLG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->FLG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->ECG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->ECG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->LEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->LEG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->SEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->SEG->ActivateFoliageItem(this);
			} // if
		} // if
	/*
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		Root->EditRAHost();
		} // if
	else
		{
		if (GlobalApp->GUIWins->FEG)
			{
			delete GlobalApp->GUIWins->FEG;
			}
		GlobalApp->GUIWins->FEG = new EcotypeEditGUI(GlobalApp->AppImages, GlobalApp->AppEffects, this);
		if (GlobalApp->GUIWins->FEG)
			{
			GlobalApp->GUIWins->FEG->Open(GlobalApp->MainProj);
			}
		} // else
	*/
	} // if

} // Ecotype::EditRAHost

/*===========================================================================*/

void Ecotype::ChangeSizeMethod(void)
{
double PrevRatio;
FoliageGroup *Current;

if (PrevSizeMethod != SizeMethod)
	{
	SetAnimDefaults();
	// what is the second height type
	if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		// what was ratio of previous method's second to first sizes
		if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			PrevRatio = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue > 0.0 ? AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue: 0.0;
			} // if
		else if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			PrevRatio = AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue > 0.0 ? AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue: 0.0;
			} // if
		else if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			PrevRatio = AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue > 0.0 ? AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue: 0.0;
			} // if
		else	// crown closure
			{
			PrevRatio = AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue > 0.0 ? AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue: 0.0;
			} // if
		// apply ratio to new method
		if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue);
			} // if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue);
			} // if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue);
			} // if
		else	// crown closure
			{
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue);
			} // if
		Current = FolGrp;
		while (Current)
			{
			// what is the second height type
			if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
				{
				// what was ratio of previous method's second to first sizes
				if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
					{
					PrevRatio = Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue > 0.0 ? Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue: 0.0;
					} // if
				else if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
					{
					PrevRatio = Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue > 0.0 ? Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue: 0.0;
					} // if
				else if (PrevSizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
					{
					PrevRatio = Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue > 0.0 ? Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue: 0.0;
					} // if
				else	// crown closure
					{
					PrevRatio = Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue > 0.0 ? Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue: 0.0;
					} // if
				// apply ratio to new method
				if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
					{
					Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue);
					} // if
				else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
					{
					Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue);
					} // if
				else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
					{
					Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue);
					} // if
				else	// crown closure
					{
					Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetCurValue(PrevRatio * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue);
					} // if
				} // if
			Current = Current->Next;
			} // while
		} // if
	PrevSizeMethod = SizeMethod;
	} // if

} // Ecotype::ChangeSizeMethod

/*===========================================================================*/

void Ecotype::ChangeAbsHtResident(void)
{
double RestoreValue;
FoliageGroup *Current;
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;

SetAnimDefaults();
if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	if (AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue > 0.0)
		{
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // while
	} // if
else
	{
	Current = FolGrp;
	while (Current)
		{
		RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
		Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
		Current = Current->Next;
		} // while
	} // else

} // Ecotype::ChangeAbsHtResident

/*===========================================================================*/

void Ecotype::ChangeAbsDensResident(void)
{
double RestoreValue;
FoliageGroup *Current;
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;

SetAnimDefaults();
if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	if (AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue > 0.0)
		{
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // while
	} // if
else
	{
	Current = FolGrp;
	while (Current)
		{
		RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue;
		Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
		Current = Current->Next;
		} // while
	} // else

} // Ecotype::ChangeAbsDensResident

/*===========================================================================*/

void Ecotype::ChangeSecondHtType(char LastType)
{
double RestoreValue;
FoliageGroup *Current;
NotifyTag Changes[2];

Changes[0] = MAKE_ID(GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Changes[1] = NULL;

SetAnimDefaults();
if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS && (LastType == WCS_ECOTYPE_SECONDHT_MINPCT || LastType == WCS_ECOTYPE_SECONDHT_RANGEPCT))
	{
	if (! GlobalApp->ForestryAuthorized || SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
		{
		RestoreValue = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		RestoreValue = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue;
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // else if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		RestoreValue = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue;
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // else if
	else	// crown closure
		{
		RestoreValue = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue * AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue;
		AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		Current = FolGrp;
		while (Current)
			{
			RestoreValue = .01 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue;
			Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
			Current = Current->Next;
			} // while
		} // else if
	} // if
else if ((SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT || SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT) &&
	(LastType == WCS_ECOTYPE_SECONDHT_MINABS))
	{
	if (! GlobalApp->ForestryAuthorized || SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
		{
		if (AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue > 0.0)
			{
			RestoreValue = 100.0 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Current = FolGrp;
		while (Current)
			{
			if (Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue > 0.0)
				{
				RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
				Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
				GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
				} // if
			Current = Current->Next;
			} // while
		} // if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
		{
		if (AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue > 0.0)
			{
			RestoreValue = 100.0 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].CurValue;
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Current = FolGrp;
		while (Current)
			{
			if (Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue > 0.0)
				{
				RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].CurValue;
				Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
				GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
				} // if
			Current = Current->Next;
			} // while
		} // if
	else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
		{
		if (AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue > 0.0)
			{
			RestoreValue = 100.0 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].CurValue;
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Current = FolGrp;
		while (Current)
			{
			if (Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue > 0.0)
				{
				RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].CurValue;
				Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
				GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
				} // if
			Current = Current->Next;
			} // while
		} // if
	else	// crown closure
		{
		if (AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue > 0.0)
			{
			RestoreValue = 100.0 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue / AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].CurValue;
			AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Current = FolGrp;
		while (Current)
			{
			if (Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue > 0.0)
				{
				RestoreValue = 100.0 * Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue / Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].CurValue;
				Current->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetValue(RestoreValue);
				GlobalApp->AppEx->GenerateNotify(Changes, Current->GetRAHostRoot());
				} // if
			Current = Current->Next;
			} // while
		} // if

	} // else

} // Ecotype::ChangeSecondHtType

/*===========================================================================*/

unsigned long Ecotype::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
FoliageGroup **LoadTo;
union MultiVal MV;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
#ifdef WCS_THEMATIC_MAP
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_THEMATIC_MAP
char TexRootNumber = -1, WarnAboutDensByPolygon = 0;

LoadTo = &FolGrp;

#ifdef WCS_FORESTRY_WIZARD
{
int InhibitForestry = 0;

if (GlobalApp->MainProj->Prefs.PublicQueryConfigOptTrue("inhibit_forestry"))
	{
	InhibitForestry = 1;
	GlobalApp->ForestryAuthorized = 0;
	} // if
if (! GlobalApp->ForestryAuthorized)
	GlobalApp->ForestryAuthorized = (! InhibitForestry && (GlobalApp->Sentinal->CheckAuthFieldForestry() ||
		(GlobalApp->Sentinal->CheckRenderEngineQuick() && GlobalApp->LoadToEffectsLib->ProjectFileSavedWithForestEd))) ? 1: 0;
} // temp scope
#endif // WCS_FORESTRY_WIZARD

DensityMethod = WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA;
SizeMethod = WCS_FOLIAGE_SIZEMETHOD_HEIGHT;

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
					case WCS_ECOTYPE_BROWSEDATA:
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
					case WCS_ECOTYPE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_CONSTDENSITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ConstDensity, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (! ConstDensity)
							WarnAboutDensByPolygon = 1;
						break;
						}
					case WCS_ECOTYPE_DENSITYUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&DensityUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_BASALAREAUNITS:
						{
						BytesRead = ReadBlock(ffile, (char *)&BasalAreaUnits, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_DISSOLVEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&DissolveEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_RENDEROCCLUDED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RenderOccluded, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_ABSHEIGHTRESIDENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&AbsHeightResident, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_ABSDENSRESIDENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&AbsDensResident, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_SECONDHEIGHTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&SecondHeightType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_SIZEMETHOD:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = ReadBlock(ffile, (char *)&SizeMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						PrevSizeMethod = SizeMethod;
						break;
						}
					case WCS_ECOTYPE_DENSITYMETHOD:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = ReadBlock(ffile, (char *)&DensityMethod, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_DISSOLVEPIXHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&DissolvePixelHeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_DISSOLVECOLOR:
						{
						BytesRead = DissolveColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_MAXHEIGHT:
						{
						BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_MINHEIGHT:
						{
						BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_FOLDENSITY:
						{
						BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_ECOTYPE_DBH:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_DBH].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_BASALAREA:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_BASALAREA].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_CROWNCLOSURE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_AGE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AnimPar[WCS_ECOTYPE_ANIMPAR_AGE].Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_AGECURVE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AgeCurve.Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_DBHCURVE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = DBHCurve.Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_FOLIAGEGROUP:
						{
						if (*LoadTo = new FoliageGroup(this))
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_ECOTYPE_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_ECOTYPE_TEXDISSOLVECOLOR:
						{
						if (TexRoot[WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXHEIGHT:
						{
						if (TexRoot[WCS_ECOTYPE_TEXTURE_HEIGHT] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_HEIGHT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXDENSITY:
						{
						if (TexRoot[WCS_ECOTYPE_TEXTURE_DENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_DENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXDBH:
						{
						if (GlobalApp->ForestryAuthorized && (TexRoot[WCS_ECOTYPE_TEXTURE_DBH] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_DBH]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXBASALAREA:
						{
						if (GlobalApp->ForestryAuthorized && (TexRoot[WCS_ECOTYPE_TEXTURE_BASALAREA] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_BASALAREA]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXCROWNCLOSURE:
						{
						if (GlobalApp->ForestryAuthorized && (TexRoot[WCS_ECOTYPE_TEXTURE_CROWNCLOSURE] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_CROWNCLOSURE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_TEXAGE:
						{
						if (GlobalApp->ForestryAuthorized && (TexRoot[WCS_ECOTYPE_TEXTURE_AGE] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_ECOTYPE_TEXTURE_AGE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_ECOTYPE_THEMEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_ECOTYPE_THEME_HEIGHT] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_ECOTYPE_THEMEMINHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_ECOTYPE_THEME_MINHEIGHT] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_ECOTYPE_THEMEDENSITY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_ECOTYPE_THEME_DENSITY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_ECOTYPE_THEMEDBH:
						{
						if (GlobalApp->ForestryAuthorized)
							{
							BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (MatchName[0])
								{
								Theme[WCS_ECOTYPE_THEME_DBH] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_THEMEBASALAREA:
						{
						if (GlobalApp->ForestryAuthorized)
							{
							BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (MatchName[0])
								{
								Theme[WCS_ECOTYPE_THEME_BASALAREA] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_THEMECROWNCLOSURE:
						{
						if (GlobalApp->ForestryAuthorized)
							{
							BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (MatchName[0])
								{
								Theme[WCS_ECOTYPE_THEME_CROWNCLOSURE] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_ECOTYPE_THEMEAGE:
						{
						if (GlobalApp->ForestryAuthorized)
							{
							BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
							if (MatchName[0])
								{
								Theme[WCS_ECOTYPE_THEME_AGE] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
								} // if
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_THEMATIC_MAP
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

SetAnimDefaults();

if (!EngineOnly && WarnAboutDensByPolygon)
	{
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD, WCS_TOOLBAR_ITEM_PUW, 0);
	if (GlobalApp->GUIWins->PUG)
		{
		GlobalApp->GUIWins->PUG->SetEcotypeMessagesVisible(WarnAboutDensByPolygon ? true : false);
		} // if
	} // if
if (!EngineOnly && WarnAboutDensByPolygon)
	{
	ConstDensity = 1;
	} // if

return (TotalRead);

} // Ecotype::Load

/*===========================================================================*/

unsigned long Ecotype::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
FoliageGroup *Current;
unsigned long AnimItemTag[WCS_ECOTYPE_NUMANIMPAR] = {WCS_ECOTYPE_MAXHEIGHT,
														WCS_ECOTYPE_MINHEIGHT,
														WCS_ECOTYPE_FOLDENSITY,
														WCS_ECOTYPE_DBH,
														WCS_ECOTYPE_BASALAREA,
														WCS_ECOTYPE_CROWNCLOSURE,
														WCS_ECOTYPE_AGE
														};
unsigned long TextureItemTag[WCS_ECOTYPE_NUMTEXTURES] = {WCS_ECOTYPE_TEXDISSOLVECOLOR,
														WCS_ECOTYPE_TEXHEIGHT,
														WCS_ECOTYPE_TEXDENSITY,
														WCS_ECOTYPE_TEXDBH,
														WCS_ECOTYPE_TEXBASALAREA,
														WCS_ECOTYPE_TEXCROWNCLOSURE,
														WCS_ECOTYPE_TEXAGE
														};
unsigned long ThemeItemTag[WCS_ECOTYPE_NUMTHEMES] = {WCS_ECOTYPE_THEMEHEIGHT,
														WCS_ECOTYPE_THEMEMINHEIGHT,
														WCS_ECOTYPE_THEMEDENSITY,
														WCS_ECOTYPE_THEMEDBH,
														WCS_ECOTYPE_THEMEBASALAREA,
														WCS_ECOTYPE_THEMECROWNCLOSURE,
														WCS_ECOTYPE_THEMEAGE
														};

if (BrowseInfo)
	{
	ItemTag = WCS_ECOTYPE_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_CONSTDENSITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ConstDensity)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DENSITYUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DensityUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_BASALAREAUNITS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BasalAreaUnits)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DISSOLVEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DissolveEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_RENDEROCCLUDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RenderOccluded)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_ABSHEIGHTRESIDENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AbsHeightResident)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_ABSDENSRESIDENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AbsDensResident)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_SECONDHEIGHTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SecondHeightType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_SIZEMETHOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SizeMethod)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DENSITYMETHOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DensityMethod)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_ECOTYPE_DISSOLVEPIXHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&DissolvePixelHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_ECOTYPE_DISSOLVECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = DissolveColor.Save(ffile))
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
			} // if color saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

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
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_ECOTYPE_DBHCURVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = DBHCurve.Save(ffile))
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
			} // if DBHCurve saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_ECOTYPE_AGECURVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = AgeCurve.Save(ffile))
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
			} // if AgeCurve saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

Current = FolGrp;
while (Current)
	{
	ItemTag = WCS_ECOTYPE_FOLIAGEGROUP + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
				} // if foliage group saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	Current = Current->Next;
	} // while

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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Ecotype::Save

/*===========================================================================*/

char *EcotypeCritterNames[WCS_ECOTYPE_NUMANIMPAR] = {"Foliage Max Ht (m)", "Foliage Min Ht (m)",
	"Foliage Density", "DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};
char *EcotypeTextureNames[WCS_ECOTYPE_NUMTEXTURES] = {"Dissolve Color", "Foliage Height (%)",
	"Foliage Density (%)", "DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};
char *EcotypeThemeNames[WCS_ECOTYPE_NUMTHEMES] = {"Foliage Height (m)", "Foliage Min Ht (m)",
	"Foliage Density", "DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};

char *Ecotype::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (EcotypeCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (EcotypeTextureNames[Ct]);
		} // if
	} // for
if (Test == &DissolveColor)
	return ("Dissolve Color");
if (Test == &DBHCurve)
	return ("DBH Curve");
if (Test == &AgeCurve)
	return ("Age Curve");

return ("");

} // Ecotype::GetCritterName

/*===========================================================================*/

char *Ecotype::OKRemoveRaster(void)
{

return ("Image Object is used as an Ecotype Texture! Remove anyway?");

} // Ecotype::OKRemoveRaster

/*===========================================================================*/

int Ecotype::GetRAHostAnimated(void)
{
long Ct;
FoliageGroup *Current = FolGrp;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // Ecotype::GetRAHostAnimated

/*===========================================================================*/

bool Ecotype::AnimateMaterials(void)
{
long Ct;
FoliageGroup *Current = FolGrp;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (true);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for

while (Current)
	{
	if (Current->AnimateMaterials())
		return (true);
	Current = Current->Next;
	} // while

return (false);

} // Ecotype::AnimateMaterials

/*===========================================================================*/

long Ecotype::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0, Ct;
FoliageGroup *CurFolGrp = FolGrp;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // while
if (DissolveColor.GetMinMaxDist(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
while (CurFolGrp)
	{
	if (CurFolGrp->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurFolGrp = CurFolGrp->Next;
	} // while

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

} // Ecotype::GetKeyFrameRange

/*===========================================================================*/

char Ecotype::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGE
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER
	|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D)
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE && GlobalApp->ForestryAuthorized)
	return (1);

return (0);

} // Ecotype::GetRAHostDropOK

/*===========================================================================*/

int Ecotype::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0, QueryResult;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
FoliageGroup *CurFolGrp;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Ecotype *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Ecotype *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = DissolveColor.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP)
	{
	Success = AddFoliageGroup((FoliageGroup *)DropSource->DropSource, NULL) ? 1: 0;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = -1;
	for (Ct = 0, CurFolGrp = FolGrp; CurFolGrp && Ct < 30; Ct ++, CurFolGrp = CurFolGrp->Next)
		{
		TargetList[Ct] = CurFolGrp;
		} // for
	NumListItems = Ct;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	if (GlobalApp->ForestryAuthorized)
		{
		Success = -1;
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Curve");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "DBH", "Cancel", "Age", 0))
			{
			if (QueryResult == 1)
				{
				Success = DBHCurve.ProcessRAHostDragDrop(DropSource);
				} // if
			else if (QueryResult == 2)
				{
				Success = AgeCurve.ProcessRAHostDragDrop(DropSource);
				} // else if
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	#ifdef WCS_FORESTRY_WIZARD
	for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumTextures(): WCS_ECOTYPE_TEXTURE_DBH); Ct ++)
	#endif // WCS_FORESTRY_WIZARD
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // Ecotype::ProcessRAHostDragDrop

/*===========================================================================*/

char *Ecotype::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? EcotypeTextureNames[TexNumber]: (char*)"");

} // Ecotype::GetTextureName

/*===========================================================================*/

void Ecotype::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = (Test == GetTexRootPtr(WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR));
ApplyToDisplace = 0;

} // Ecotype::GetTextureApplication

/*===========================================================================*/

RootTexture *Ecotype::NewRootTexture(long TexNumber)
{
char ApplyToColor = (TexNumber == WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR);
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Ecotype::NewRootTexture

/*===========================================================================*/

int Ecotype::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;
unsigned long ItemRemoved;
FoliageGroup *CurFolGrp = FolGrp, *PrevFolGrp = NULL;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		ItemRemoved = WCS_SUBCLASS_ROOTTEXTURE;
		} // if
	} // for

if (! Removed)
	{
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (RemoveMe == GetTheme(Ct))
			{
			SetThemePtr(Ct, NULL);
			Removed = 1;
			ItemRemoved = WCS_EFFECTSSUBCLASS_THEMATICMAP;
			} // if
		} // for
	} // if

if (! Removed)
	{
	while (CurFolGrp)
		{
		if (CurFolGrp == (FoliageGroup *)RemoveMe)
			{
			if (PrevFolGrp)
				PrevFolGrp->Next = CurFolGrp->Next;
			else
				FolGrp = CurFolGrp->Next;

			delete CurFolGrp;
			Removed = 1;
			ItemRemoved = WCS_SUBCLASS_FOLIAGEGRP;
			break;
			} // if
		PrevFolGrp = CurFolGrp;
		CurFolGrp = CurFolGrp->Next;
		} // while
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), ItemRemoved, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // Ecotype::RemoveRAHost

/*===========================================================================*/

int Ecotype::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
FoliageGroup *CurFolGrp = FolGrp;

while (CurFolGrp)
	{
	if (! CurFolGrp->FindnRemove3DObjects(RemoveMe))
		return (0);
	CurFolGrp = CurFolGrp->Next;
	} // while

return (1);

} // Ecotype::FindnRemove3DObjects

/*===========================================================================*/

ThematicMap *Ecotype::GetEnabledTheme(long ThemeNum)
{

return (ThemeNum < WCS_ECOTYPE_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);

} // Ecotype::GetEnabledTheme

/*===========================================================================*/

int Ecotype::SetTheme(long ThemeNum, ThematicMap *NewTheme)
{
NotifyTag Changes[2];

if (ThemeNum < GetNumThemes())
	{
	SetThemePtr(ThemeNum, NewTheme);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_EFFECTSSUBCLASS_THEMATICMAP, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if

return (0);

} // Ecotype::SetTheme

/*===========================================================================*/

char *Ecotype::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? EcotypeThemeNames[ThemeNum]: (char*)"");

} // Ecotype::GetThemeName

/*===========================================================================*/

unsigned long Ecotype::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_ECOTYPE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Ecotype::GetRAFlags

/*===========================================================================*/

void Ecotype::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_ECOSYSTEM];
	Prop->Ext = "etp";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // Ecotype::GetRAHostProperties

/*===========================================================================*/

int Ecotype::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // Ecotype::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *Ecotype::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
FoliageGroup *CurFolGrp;

if (! Current)
	return (&DissolveColor);
if (Current == &DissolveColor)
	Found = 1;
#ifdef WCS_FORESTRY_WIZARD
if (GlobalApp->ForestryAuthorized)
	{
	if (Found)
		return (&DBHCurve);
	if (Current == &DBHCurve)
		return (&AgeCurve);
	if (Current == &AgeCurve)
		Found = 1;
	} // if
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumAnimParams(): WCS_ECOTYPE_ANIMPAR_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_FORESTRY_WIZARD
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumTextures(): WCS_ECOTYPE_TEXTURE_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_FORESTRY_WIZARD
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumThemes(): WCS_ECOTYPE_THEME_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found && GetTheme(Ct) && GetThemeUnique(Ct))
		return (GetTheme(Ct));
	if (Current == GetTheme(Ct) && GetThemeUnique(Ct))
		Found = 1;
	} // for
CurFolGrp = FolGrp;
while (CurFolGrp)
	{
	if (Found)
		return (CurFolGrp);
	if (Current == CurFolGrp)
		Found = 1;
	CurFolGrp = CurFolGrp->Next;
	} // while

return (NULL);

} // Ecotype::GetRAHostChild

/*===========================================================================*/

int Ecotype::GetDeletable(RasterAnimHost *Test)
{
char Ct;
FoliageGroup *CurFolGrp;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
CurFolGrp = FolGrp;
while (CurFolGrp)
	{
	if (Test == CurFolGrp)
		return (1);
	CurFolGrp = CurFolGrp->Next;
	} // while

return (0);

} // Ecotype::GetDeletable

/*===========================================================================*/

int Ecotype::GetThemeType(long ThemeNum)
{
// return 0 for area and 1 to make it point type
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

if (Root = GetRAHostRoot())
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		return (1);
		} // if
	} // if

return (0);

} // Ecotype::GetThemeType

/*===========================================================================*/

int Ecotype::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_ECOTYPE_ANIMPAR_MAXHEIGHT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_HEIGHT);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_HEIGHT);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_MINHEIGHT:
					{
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_MINHEIGHT);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_DENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_DENSITY);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_DBH:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_DBH);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_DBH);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_BASALAREA:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_BASALAREA);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_BASALAREA);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_CROWNCLOSURE);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_CROWNCLOSURE);
					break;
					} // 
				case WCS_ECOTYPE_ANIMPAR_AGE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_AGE);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_AGE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	if (ChildA == &DissolveColor)
		{
		AnimAffil = &DissolveColor;
		TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR);
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
				case WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR:
					{
					AnimAffil = &DissolveColor;
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_MAXHEIGHT);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_HEIGHT);
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_DENSITY);
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_DBH:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_DBH);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_DBH);
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_BASALAREA:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_BASALAREA);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_BASALAREA);
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_CROWNCLOSURE:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_CROWNCLOSURE);
					break;
					} // 
				case WCS_ECOTYPE_TEXTURE_AGE:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_AGE);
					ThemeAffil = GetThemeAddr(WCS_ECOTYPE_THEME_AGE);
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
				case WCS_ECOTYPE_THEME_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_MAXHEIGHT);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_HEIGHT);
					break;
					} // 
				case WCS_ECOTYPE_THEME_MINHEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_MINHEIGHT);
					break;
					} // 
				case WCS_ECOTYPE_THEME_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_DENSITY);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_DENSITY);
					break;
					} // 
				case WCS_ECOTYPE_THEME_DBH:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_DBH);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_DBH);
					break;
					} // 
				case WCS_ECOTYPE_THEME_BASALAREA:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_BASALAREA);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_BASALAREA);
					break;
					} // 
				case WCS_ECOTYPE_THEME_CROWNCLOSURE:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_CROWNCLOSURE);
					break;
					} // 
				case WCS_ECOTYPE_THEME_AGE:
					{
					AnimAffil = GetAnimPtr(WCS_ECOTYPE_ANIMPAR_AGE);
					TexAffil = GetTexRootPtrAddr(WCS_ECOTYPE_TEXTURE_AGE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Ecotype::GetAffiliates

/*===========================================================================*/

int Ecotype::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil));
	} // if

return (0);

} // Ecotype::GetPopClassFlags

/*===========================================================================*/

int Ecotype::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil));
	} // if

return(0);

} // Ecotype::AddSRAHBasePopMenus

/*===========================================================================*/

int Ecotype::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, ThemeAffil, this, this));
	} // if

return(0);

} // Ecotype::HandleSRAHPopMenuSelection

/*===========================================================================*/

long Ecotype::InitImageIDs(long &ImageID)
{
long Ct, ImagesFound = 0;
FoliageGroup *CurGrp = FolGrp;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		ImagesFound += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for
while (CurGrp)
	{
	ImagesFound += CurGrp->InitImageIDs(ImageID);
	CurGrp = CurGrp->Next;
	} // while

return (ImagesFound);

} // Ecotype::InitImageIDs

/*===========================================================================*/

int Ecotype::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
FoliageGroup *CurGrp = FolGrp;
long Ct;

while (CurGrp)
	{
	if (! CurGrp->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
		return (0);
	CurGrp = CurGrp->Next;
	} // while

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for

return (ThematicOwner::BuildFileComponentsList(Themes));

} // Ecotype::BuildFileComponentsList

/*===========================================================================*/

int Ecotype::InitToRender(void)
{
char Ct;
FoliageGroup *CurGrp = FolGrp;

PixelTexturesExist = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		if (Ct == WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR) 
			PixelTexturesExist = 1;
		} // if
	} // for
while (CurGrp)
	{
	if (! CurGrp->InitToRender())
		return (0);
	CurGrp = CurGrp->Next;
	} // while

return (1);

} // Ecotype::InitToRender

/*===========================================================================*/

int Ecotype::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
char Ct;
FoliageGroup *CurGrp = FolGrp;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitFrameToRender(Lib, Rend))
			{
			return (0);
			} // if
		} // if
	} // for
while (CurGrp)
	{
	if (! CurGrp->InitFrameToRender(Lib, Rend))
		return (0);
	CurGrp = CurGrp->Next;
	} // while

return (1);

} // Ecotype::InitFrameToRender

/*===========================================================================*/

double Ecotype::CalcAvgImageHeightWidthRatio(void)
{
double Width, Height, Ratio = 0.0, ObjX, ObjY, ObjZ;
long NumItems = 0;
FoliageGroup *CurGrp = FolGrp;
Foliage *CurFol;

while (CurGrp)
	{
	if (CurGrp->Enabled)
		{
		CurFol = CurGrp->Fol;
		while (CurFol)
			{
			if (CurFol->Enabled)
				{
				if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
					{
					if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
						{
						Width = CurFol->Img->GetRaster()->Cols;
						Height = CurFol->Img->GetRaster()->Rows;
						if (Width > 0.0)
							{
							Ratio += (Height / Width);
							NumItems ++;
							} // if
						} // if
					} // if
				else	// CurFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D
					{
					if (CurFol->Obj && CurFol->Enabled)
						{
						ObjX = fabs(CurFol->Obj->ObjectBounds[0] - CurFol->Obj->ObjectBounds[1]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
						ObjY = fabs(CurFol->Obj->ObjectBounds[2] - CurFol->Obj->ObjectBounds[3]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
						ObjZ = fabs(CurFol->Obj->ObjectBounds[4] - CurFol->Obj->ObjectBounds[5]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
						Width = max(ObjX, ObjZ);
						Height = ObjY;
						if (Width > 0.0)
							{
							Ratio += (Height / Width);
							NumItems ++;
							} // if
						} // if
					} // else
				} // if
			CurFol = CurFol->Next;
			} // while
		} // if
	CurGrp = CurGrp->Next;
	} // while

if (NumItems > 0)
	{
	return (Ratio / NumItems);
	} // if
else
	return (1.0);

} // Ecotype::CalcAvgImageHeightWidthRatio

/*===========================================================================*/

int Ecotype::BuildFoliageChain(RenderData *Rend)
{
int Success = 1;
#ifdef WCS_FORESTRY_WIZARD
FoliageGroup *CurGrp;
Foliage *CurFol;
#endif // WCS_FORESTRY_WIZARD

// delete existing chain
#ifdef WCS_USE_OLD_FOLIAGE
DeleteFoliageChain();
#endif // WCS_USE_OLD_FOLIAGE

#ifdef WCS_FORESTRY_WIZARD
//if (GlobalApp->ForestryAuthorized)
	{
	AvgImageHeightWidthRatio = CalcAvgImageHeightWidthRatio();

	if (BasalAreaUnits == WCS_FOLIAGE_BASALAREAUNITS_SQMETER)
		BasalAreaUnitConversion = 1.0;
	else	// WCS_FOLIAGE_BASALAREAUNITS_SQFOOT
		{
		BasalAreaUnitConversion = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
		BasalAreaUnitConversion *= BasalAreaUnitConversion;
		} // else feet

	EcoHeightTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_HEIGHT);
	EcoMinHeightTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_MINHEIGHT);
	if (GlobalApp->ForestryAuthorized)
		{
		EcoDbhTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_DBH);
		EcoAgeTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_AGE);
		EcoClosureTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_CROWNCLOSURE);
		EcoBasalAreaTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_BASALAREA);
		} // if
	else
		EcoDbhTheme = EcoAgeTheme = EcoClosureTheme = EcoBasalAreaTheme = NULL;
	EcoStemsTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_DENSITY);

	EcoSizeTex = NULL;
	EcoDensityTex = NULL;

	if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
		{
		if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
			{
			EcoDensityTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_DENSITY);
			} // if
		else if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			EcoDensityTex = GlobalApp->ForestryAuthorized ? GetEnabledTexture(WCS_ECOTYPE_TEXTURE_BASALAREA): NULL;
			} // else if
		else if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			EcoDensityTex = GlobalApp->ForestryAuthorized ? GetEnabledTexture(WCS_ECOTYPE_TEXTURE_CROWNCLOSURE): NULL;
			} // else if
		} // if

	if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
		{
		if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			EcoSizeTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_HEIGHT);
			} // if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			EcoSizeTex = GlobalApp->ForestryAuthorized ? GetEnabledTexture(WCS_ECOTYPE_TEXTURE_DBH): NULL;
			} // else if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			EcoSizeTex = GlobalApp->ForestryAuthorized ? GetEnabledTexture(WCS_ECOTYPE_TEXTURE_AGE): NULL;
			} // else if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			EcoSizeTex = GlobalApp->ForestryAuthorized ? GetEnabledTexture(WCS_ECOTYPE_TEXTURE_CROWNCLOSURE): NULL;
			} // else if
		} // if

	for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
		{
		CurGrp->GroupSizeTex = CurGrp->GroupDensityTex = NULL;
		CurGrp->GroupSizeTheme = CurGrp->GroupDensityTheme = NULL;

		if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE || DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA)
			{
			CurGrp->GroupDensityTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
			CurGrp->GroupDensityTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
			} // if
		else if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_BASALAREA)
			{
			CurGrp->GroupDensityTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_BASALAREA): NULL;
			CurGrp->GroupDensityTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_BASALAREA): NULL;
			} // else if
		else if (DensityMethod == WCS_FOLIAGE_DENSITYMETHOD_CLOSURE)
			{
			CurGrp->GroupDensityTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE): NULL;
			CurGrp->GroupDensityTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE): NULL;
			} // else if

		if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE || SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			CurGrp->GroupSizeTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
			CurGrp->GroupSizeTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);
			} // if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			CurGrp->GroupSizeTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DBH): NULL;
			CurGrp->GroupSizeTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DBH): NULL;
			} // else if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			CurGrp->GroupSizeTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_AGE): NULL;
			CurGrp->GroupSizeTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_AGE): NULL;
			} // else if
		else if (SizeMethod == WCS_FOLIAGE_SIZEMETHOD_CLOSURE)
			{
			CurGrp->GroupSizeTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE): NULL;
			CurGrp->GroupSizeTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE): NULL;
			} // else if

		CurGrp->GroupHeightTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
		CurGrp->GroupDbhTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DBH): NULL;
		CurGrp->GroupAgeTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_AGE): NULL;
		CurGrp->GroupClosureTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE): NULL;
		CurGrp->GroupStemsTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
		CurGrp->GroupBasalAreaTex = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_BASALAREA): NULL;

		CurGrp->GroupHeightTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);
		CurGrp->GroupMinHeightTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_MINHEIGHT);
		CurGrp->GroupDbhTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DBH): NULL;
		CurGrp->GroupAgeTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_AGE): NULL;
		CurGrp->GroupClosureTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE): NULL;
		CurGrp->GroupStemsTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
		CurGrp->GroupBasalAreaTheme = GlobalApp->ForestryAuthorized ? CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_BASALAREA): NULL;

		CurGrp->AvgImageHeightWidthRatio = CurGrp->CalcAvgImageHeightWidthRatio();

		for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
			{
			if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
				{
				CurFol->Rast = CurFol->Img->GetRaster();
				CurFol->ColorImage = CurFol->Rast->GetIsColor();
				CurFol->RenderOccluded = RenderOccluded;
				CurFol->ImageWidthFactor = (double)CurFol->Rast->Cols / CurFol->Rast->Rows;
				} // if
			CurFol->FoliageSizeTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_HEIGHT);
			CurFol->FoliageDensityTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_DENSITY);
			CurFol->FoliageColorTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_COLOR);
			CurFol->FoliageBacklightTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
			CurFol->FoliageSizeTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_HEIGHT);
			CurFol->FoliageDensityTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_DENSITY);
			CurFol->FoliageColorTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_COLOR);
			} // for
		} // for
	} // if
#endif // WCS_FORESTRY_WIZARD

// for back-compatibility
//#ifdef WCS_USE_OLD_FOLIAGE
if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE && AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	Success = BuildFoliageChainDensAndHtByEcotype(Rend);
else if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP && AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	Success = BuildFoliageChainDensAndHtByGroup(Rend);
else if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	Success = BuildFoliageChainDensByGroup(Rend);
else if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	Success = BuildFoliageChainHtByGroup(Rend);
//#endif // #WCS_USE_OLD_FOLIAGE

return (Success);

} // Ecotype::BuildFoliageChain

/*===========================================================================*/

int Ecotype::BuildFoliageChainDensAndHtByEcotype(RenderData *Rend)
{
//FoliageChainList *CurChain, **ChainPtr = &ChainList;
//FoliageLink *CurLink, **FolLinkPtr;
//FoliageGroup *CurGrp;
//Foliage *CurFol;
//double ProjPlaneDissolveHt, SumDens, CurDens, MaxxHeight, MinHeight;
double ProjPlaneDissolveHt, MaxxHeight;
//RootTexture *EcoDensTex, *EcoHtTex, *GrpDensTex, *GrpHtTex;
//ThematicMap *EcoDensTheme, *EcoHtTheme, *GrpDensTheme, *GrpHtTheme;
VertexDEM FolVtx1, FolVtx2;

if (ConstDensity)
	{
	switch (DensityUnits)
		{
		case WCS_FOLIAGE_DENSITY_HECTARE:
			{
			DensityNormalizer = 1.0 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_HECTARE
		case WCS_FOLIAGE_DENSITY_ACRE:
			{
			DensityNormalizer = 2.471 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_ACRE
		case WCS_FOLIAGE_DENSITY_SQFOOT:
			{
			DensityNormalizer = 10.76;
			break;
			} // WCS_FOLIAGE_DENSITY_SQFOOT
		//case WCS_FOLIAGE_DENSITY_SQMETER:
		default:
			{
			DensityNormalizer = 1.0;
			break;
			} // WCS_FOLIAGE_DENSITY_SQMETER
		} // switch
	} // if
else
	DensityNormalizer = 1.0;

// set height range
MaxxHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;

// determine dissolve distances [0] = near, [1] = far
FolVtx1.ScrnXYZ[0] = Rend->Width * 0.5;
FolVtx1.ScrnXYZ[1] = Rend->Height * 0.5;
FolVtx1.ScrnXYZ[2] = FolVtx1.Q = 1.0;
FolVtx2.CopyScrnXYZQ(&FolVtx1);
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx1, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
if (GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize)
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight * Rend->SetupHeight / GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt;
else
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight;
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx2, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
FolVtx1.GetPosVector(&FolVtx2);
ProjPlaneDissolveHt = sqrt(FolVtx1.XYZ[0] * FolVtx1.XYZ[0] + FolVtx1.XYZ[1] * FolVtx1.XYZ[1] + FolVtx1.XYZ[2] * FolVtx1.XYZ[2]);

if (ProjPlaneDissolveHt > 0.0)
	DissolveDistance = MaxxHeight / ProjPlaneDissolveHt;
else
	DissolveDistance = 1.0;

#ifdef WCS_USE_OLD_FOLIAGE
// determine if there are ecotype textures and thematic maps to be evaluated during rendering
EcoDensTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_DENSITY);
EcoDensTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_DENSITY);

EcoHtTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_HEIGHT);
EcoHtTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_HEIGHT);

// if absolute density and height are resident in ecotype then we only need one chain
SumDens = 0.0;
// find total density for all groups and foliage objects
CurGrp = FolGrp;
while (CurGrp)
	{
	if (CurGrp->Enabled)
		{
		CurFol = CurGrp->Fol;
		while (CurFol)
			{
			if (CurFol->Enabled)
				{
				SumDens += .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;
				} // if enabled
			CurFol = CurFol->Next;
			} // while
		} // if enabled
	CurGrp = CurGrp->Next;
	} // while

// allocate a link for each foliage object and fill in values
if (SumDens > 0.0)
	{
	if (*ChainPtr = new FoliageChainList())
		{
		CurChain = *ChainPtr;
		ChainPtr = &CurChain->Next;
		FolLinkPtr = &CurChain->FoliageChain;
		CurChain->EcoThemeDensFactor =  1.0;
		// normalize constant density
		if (ConstDensity)
			CurChain->DensityPerSqMeter = DensityNormalizer * AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue;
		else
			CurChain->DensityPerSqMeter = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue;

		CurDens = 0.0;
		CurGrp = FolGrp;
		while (CurGrp)
			{
			if (CurGrp->Enabled)
				{
				GrpDensTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
				GrpHtTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);

				GrpDensTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
				GrpHtTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);

				// set current chain's density theme
				CurChain->DensityTheme = EcoDensTheme;

				// set absolute height range for current chain
				CurChain->MaxHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
				MinHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
				if (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
					{
					MinHeight = CurChain->MaxHeight - .01 * CurChain->MaxHeight * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
					CurChain->MaxHeight += (.01 * CurChain->MaxHeight * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue);
					if (MinHeight < 0.0)
						MinHeight = 0.0;
					} // if
				else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
					{
					MinHeight = CurChain->MaxHeight * .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
					if (MinHeight < 0.0)
						MinHeight = 0.0;
					} // if

				if (MinHeight > CurChain->MaxHeight)
					MinHeight = CurChain->MaxHeight;
				CurChain->HeightRange = CurChain->MaxHeight - MinHeight;

				CurFol = CurGrp->Fol;
				while (CurFol)
					{
					if (CurFol->Enabled)
						{
						if (*FolLinkPtr = new FoliageLink())
							{
							CurLink = *FolLinkPtr;
							FolLinkPtr = &CurLink->Next;
							CurLink->Density = CurDens + .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue / SumDens;
							CurDens = CurLink->Density;
							if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
								{
								if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
									{
									CurLink->Enabled = 1;
									CurLink->Rast = CurFol->Img->GetRaster();
									CurLink->ColorImage = CurFol->Img->GetRaster()->GetIsColor();
									CurLink->PosShade = CurFol->PosShade;
									CurLink->Shade3D = CurFol->Shade3D;
									CurLink->RenderOccluded = RenderOccluded;
									CurLink->ImageWidthFactor = (double)CurLink->Rast->Cols / CurLink->Rast->Rows;
									CurLink->Fol = CurFol;
									CurLink->Grp = CurGrp;
									} // if
								} // if
							else
								{
								if (CurFol->Obj && CurFol->Obj->GetEnabled())
									{
									CurLink->Enabled = 1;
									CurLink->Obj = CurFol->Obj;
									CurLink->PosShade = CurFol->PosShade;
									CurLink->Fol = CurFol;
									CurLink->Grp = CurGrp;
									} // if
								} // else
							CurLink->EcoDensTex = EcoDensTex;
							CurLink->EcoHtTex = EcoHtTex;
							CurLink->GrpDensTex = GrpDensTex;
							CurLink->GrpHtTex = GrpHtTex;
							// EcoDensTheme is found in FoliageChain, not FoliageLink to facillitate renderer density calculation
							CurLink->EcoHtTheme = EcoHtTheme;	// NULL if controlled abs height in group
							CurLink->GrpDensTheme = GrpDensTheme;
							CurLink->GrpHtTheme = GrpHtTheme;
							CurLink->FolDensTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_DENSITY);
							CurLink->FolHtTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_HEIGHT);
							CurLink->FolBacklightTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
							CurLink->FolColorTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_COLOR);
							CurLink->FolDensTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_DENSITY);
							CurLink->FolHtTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_HEIGHT);
							CurLink->FolColorTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_COLOR);
							} // if new FoliageLink
						else
							return (0);
						} // if enabled
					CurFol = CurFol->Next;
					} // while CurFol
				} // if group enabled
			CurGrp = CurGrp->Next;
			} // while CurGrp
		} // if new FoliageChainList
	else
		return (0);
	} // if
#endif // WCS_USE_OLD_FOLIAGE

return (1);

} // Ecotype::BuildFoliageChainDensAndHtByEcotype

/*===========================================================================*/

int Ecotype::BuildFoliageChainDensAndHtByGroup(RenderData *Rend)
{
//FoliageChainList *CurChain, **ChainPtr = &ChainList;
//FoliageLink *CurLink, **FolLinkPtr;
FoliageGroup *CurGrp;//, *LoopGroup = FolGrp;
//Foliage *CurFol;
//double ProjPlaneDissolveHt, SumDens, CurDens, MaxxHeight, MinHeight;
double ProjPlaneDissolveHt, MaxxHeight;
//RootTexture *EcoDensTex, *EcoHtTex, *GrpDensTex, *GrpHtTex;
//ThematicMap *EcoDensTheme, *EcoHtTheme, *GrpDensTheme, *GrpHtTheme;
VertexDEM FolVtx1, FolVtx2;

if (ConstDensity)
	{
	switch (DensityUnits)
		{
		case WCS_FOLIAGE_DENSITY_HECTARE:
			{
			DensityNormalizer = 1.0 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_HECTARE
		case WCS_FOLIAGE_DENSITY_ACRE:
			{
			DensityNormalizer = 2.471 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_ACRE
		case WCS_FOLIAGE_DENSITY_SQFOOT:
			{
			DensityNormalizer = 10.76;
			break;
			} // WCS_FOLIAGE_DENSITY_SQFOOT
		//case WCS_FOLIAGE_DENSITY_SQMETER:
		default:
			{
			DensityNormalizer = 1.0;
			break;
			} // WCS_FOLIAGE_DENSITY_SQMETER
		} // switch
	} // if
else
	DensityNormalizer = 1.0;

// set height range
MaxxHeight = 0.0;
CurGrp = FolGrp;
while (CurGrp)
	{
	if (CurGrp->Enabled)
		{
		if (CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue > MaxxHeight)
			MaxxHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
		} // if enabled
	CurGrp = CurGrp->Next;
	} // while

// determine dissolve distances [0] = near, [1] = far
FolVtx1.ScrnXYZ[0] = Rend->Width * 0.5;
FolVtx1.ScrnXYZ[1] = Rend->Height * 0.5;
FolVtx1.ScrnXYZ[2] = FolVtx1.Q = 1.0;
FolVtx2.CopyScrnXYZQ(&FolVtx1);
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx1, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
if (GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize)
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight * Rend->SetupHeight / GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt;
else
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight;
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx2, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
FolVtx1.GetPosVector(&FolVtx2);
ProjPlaneDissolveHt = sqrt(FolVtx1.XYZ[0] * FolVtx1.XYZ[0] + FolVtx1.XYZ[1] * FolVtx1.XYZ[1] + FolVtx1.XYZ[2] * FolVtx1.XYZ[2]);

if (ProjPlaneDissolveHt > 0.0)
	DissolveDistance = MaxxHeight / ProjPlaneDissolveHt;
else
	DissolveDistance = 1.0;

#ifdef WCS_USE_OLD_FOLIAGE
// determine if there are ecotype textures and thematic maps to be evaluated during rendering
EcoDensTex = NULL;
EcoDensTheme = NULL;

EcoHtTex = NULL;
EcoHtTheme = NULL;

// if absolute density and height are resident in ecotype then we only need one chain, but if they are in the groups then
// we will need a linked list of foliage link lists
while (LoopGroup)
	{
	SumDens = 0.0;
	// find total density for all foliage objects
	if (LoopGroup->Enabled)
		{
		CurFol = LoopGroup->Fol;
		while (CurFol)
			{
			if (CurFol->Enabled)
				{
				SumDens += CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;
				} // if enabled
			CurFol = CurFol->Next;
			} // while
		} // if enabled

	// allocate a link for each foliage object and fill in values
	if (SumDens > 0.0)
		{
		if (*ChainPtr = new FoliageChainList())
			{
			CurChain = *ChainPtr;
			ChainPtr = &CurChain->Next;
			FolLinkPtr = &CurChain->FoliageChain;
			CurChain->EcoThemeDensFactor =  1.0;
			// normalize constant density
			if (ConstDensity)
				CurChain->DensityPerSqMeter = DensityNormalizer * LoopGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue;
			else
				CurChain->DensityPerSqMeter = .01 * LoopGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue;

			CurDens = 0.0;
			CurGrp = LoopGroup;

			GrpDensTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
			GrpHtTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
			GrpDensTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
			GrpHtTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);

			// set current chain's density theme
			CurChain->DensityTheme = GrpDensTheme;
			GrpDensTheme = NULL;

			// set absolute height range for current chain
			CurChain->MaxHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
			MinHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
			if (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
				{
				MinHeight = CurChain->MaxHeight - .01 * CurChain->MaxHeight * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
				CurChain->MaxHeight += (.01 * CurChain->MaxHeight * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue);
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if
			else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				MinHeight = CurChain->MaxHeight * .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if

			if (MinHeight > CurChain->MaxHeight)
				MinHeight = CurChain->MaxHeight;
			CurChain->HeightRange = CurChain->MaxHeight - MinHeight;

			CurFol = CurGrp->Fol;
			while (CurFol)
				{
				if (CurFol->Enabled)
					{
					if (*FolLinkPtr = new FoliageLink())
						{
						CurLink = *FolLinkPtr;
						FolLinkPtr = &CurLink->Next;
						CurLink->Density = CurDens + CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue / SumDens;
						CurDens = CurLink->Density;
						if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
							{
							if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Rast = CurFol->Img->GetRaster();
								CurLink->ColorImage = CurFol->Img->GetRaster()->GetIsColor();
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Shade3D = CurFol->Shade3D;
								CurLink->RenderOccluded = RenderOccluded;
								CurLink->ImageWidthFactor = (double)CurLink->Rast->Cols / CurLink->Rast->Rows;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // if
						else
							{
							if (CurFol->Obj && CurFol->Obj->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Obj = CurFol->Obj;
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // else
						CurLink->EcoDensTex = EcoDensTex;	// NULL if controlled abs density in group
						CurLink->EcoHtTex = EcoHtTex;	// NULL if controlled abs height in group
						CurLink->GrpDensTex = GrpDensTex;
						CurLink->GrpHtTex = GrpHtTex;
						// EcoDensTheme is found in FoliageChain, not FoliageLink to facillitate renderer density calculation
						CurLink->EcoHtTheme = EcoHtTheme;	// NULL if controlled abs height in group
						CurLink->GrpDensTheme = GrpDensTheme;
						CurLink->GrpHtTheme = GrpHtTheme;
						CurLink->FolDensTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_DENSITY);
						CurLink->FolHtTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_HEIGHT);
						CurLink->FolBacklightTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
						CurLink->FolColorTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_COLOR);
						CurLink->FolDensTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_DENSITY);
						CurLink->FolHtTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_HEIGHT);
						CurLink->FolColorTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_COLOR);
						} // if new FoliageLink
					else
						return (0);
					} // if enabled
				CurFol = CurFol->Next;
				} // while CurFol
			} // if new FoliageChainList
		else
			return (0);
		} // if
	LoopGroup = LoopGroup->Next;
	} // while LoopGroup
#endif // WCS_USE_OLD_FOLIAGE

return (1);

} // Ecotype::BuildFoliageChainDensAndHtByGroup

/*===========================================================================*/

int Ecotype::BuildFoliageChainDensByGroup(RenderData *Rend)
{
//FoliageChainList *CurChain, **ChainPtr = &ChainList;
//FoliageLink *CurLink, **FolLinkPtr;
//FoliageGroup *CurGrp, *LoopGroup = FolGrp;
//Foliage *CurFol;
//double ProjPlaneDissolveHt, SumDens, CurDens, MaxxHeight, MinHeight;
double ProjPlaneDissolveHt, MaxxHeight;
//RootTexture *EcoDensTex, *EcoHtTex, *GrpDensTex, *GrpHtTex;
//ThematicMap *EcoDensTheme, *EcoHtTheme, *GrpDensTheme, *GrpHtTheme;
VertexDEM FolVtx1, FolVtx2;

if (ConstDensity)
	{
	switch (DensityUnits)
		{
		case WCS_FOLIAGE_DENSITY_HECTARE:
			{
			DensityNormalizer = 1.0 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_HECTARE
		case WCS_FOLIAGE_DENSITY_ACRE:
			{
			DensityNormalizer = 2.471 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_ACRE
		case WCS_FOLIAGE_DENSITY_SQFOOT:
			{
			DensityNormalizer = 10.76;
			break;
			} // WCS_FOLIAGE_DENSITY_SQFOOT
		//case WCS_FOLIAGE_DENSITY_SQMETER:
		default:
			{
			DensityNormalizer = 1.0;
			break;
			} // WCS_FOLIAGE_DENSITY_SQMETER
		} // switch
	} // if
else
	DensityNormalizer = 1.0;

// set height range
MaxxHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;

// determine dissolve distances [0] = near, [1] = far
FolVtx1.ScrnXYZ[0] = Rend->Width * 0.5;
FolVtx1.ScrnXYZ[1] = Rend->Height * 0.5;
FolVtx1.ScrnXYZ[2] = FolVtx1.Q = 1.0;
FolVtx2.CopyScrnXYZQ(&FolVtx1);
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx1, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
if (GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize)
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight * Rend->SetupHeight / GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt;
else
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight;
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx2, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
FolVtx1.GetPosVector(&FolVtx2);
ProjPlaneDissolveHt = sqrt(FolVtx1.XYZ[0] * FolVtx1.XYZ[0] + FolVtx1.XYZ[1] * FolVtx1.XYZ[1] + FolVtx1.XYZ[2] * FolVtx1.XYZ[2]);

if (ProjPlaneDissolveHt > 0.0)
	DissolveDistance = MaxxHeight / ProjPlaneDissolveHt;
else
	DissolveDistance = 1.0;

#ifdef WCS_USE_OLD_FOLIAGE
// determine if there are ecotype textures and thematic maps to be evaluated during rendering
EcoDensTex = NULL;
EcoDensTheme = NULL;

EcoHtTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_HEIGHT);
EcoHtTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_HEIGHT);

// if absolute density and height are resident in ecotype then we only need one chain, but if they are in the groups then
// we will need a linked list of foliage link lists
while (LoopGroup)
	{
	SumDens = 0.0;
	// find total density for all foliage objects
	if (LoopGroup->Enabled)
		{
		CurFol = LoopGroup->Fol;
		while (CurFol)
			{
			if (CurFol->Enabled)
				{
				SumDens += CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;
				} // if enabled
			CurFol = CurFol->Next;
			} // while
		} // if enabled

	// allocate a link for each foliage object and fill in values
	if (SumDens > 0.0)
		{
		if (*ChainPtr = new FoliageChainList())
			{
			CurChain = *ChainPtr;
			ChainPtr = &CurChain->Next;
			FolLinkPtr = &CurChain->FoliageChain;
			CurChain->EcoThemeDensFactor =  1.0;
			// normalize constant density
			if (ConstDensity)
				CurChain->DensityPerSqMeter = DensityNormalizer * LoopGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue;
			else
				CurChain->DensityPerSqMeter = .01 * LoopGroup->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue;

			CurDens = 0.0;
			CurGrp = LoopGroup;

			GrpDensTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
			GrpHtTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
			GrpDensTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
			GrpHtTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);

			// set current chain's density theme
			CurChain->DensityTheme = GrpDensTheme;
			GrpDensTheme = NULL;

			// set absolute height range for current chain
			CurChain->MaxHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MAXHEIGHT].CurValue;
			MinHeight = AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
			if (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
				{
				MinHeight = CurChain->MaxHeight - .01 * CurChain->MaxHeight * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
				CurChain->MaxHeight += (.01 * CurChain->MaxHeight * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue);
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if
			else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				MinHeight = CurChain->MaxHeight * .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_MINHEIGHT].CurValue;
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if

			if (MinHeight > CurChain->MaxHeight)
				MinHeight = CurChain->MaxHeight;
			CurChain->HeightRange = CurChain->MaxHeight - MinHeight;

			CurFol = CurGrp->Fol;
			while (CurFol)
				{
				if (CurFol->Enabled)
					{
					if (*FolLinkPtr = new FoliageLink())
						{
						CurLink = *FolLinkPtr;
						FolLinkPtr = &CurLink->Next;
						CurLink->Density = CurDens + CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue / SumDens;
						CurDens = CurLink->Density;
						if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
							{
							if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Rast = CurFol->Img->GetRaster();
								CurLink->ColorImage = CurFol->Img->GetRaster()->GetIsColor();
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Shade3D = CurFol->Shade3D;
								CurLink->RenderOccluded = RenderOccluded;
								CurLink->ImageWidthFactor = (double)CurLink->Rast->Cols / CurLink->Rast->Rows;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // if
						else
							{
							if (CurFol->Obj && CurFol->Obj->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Obj = CurFol->Obj;
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // else
						CurLink->EcoDensTex = EcoDensTex;	// NULL if controlled abs density in group
						CurLink->EcoHtTex = EcoHtTex;	// NULL if controlled abs height in group
						CurLink->GrpDensTex = GrpDensTex;
						CurLink->GrpHtTex = GrpHtTex;
						// EcoDensTheme is found in FoliageChain, not FoliageLink to facillitate renderer density calculation
						CurLink->EcoHtTheme = EcoHtTheme;	// NULL if controlled abs height in group
						CurLink->GrpDensTheme = GrpDensTheme;
						CurLink->GrpHtTheme = GrpHtTheme;
						CurLink->FolDensTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_DENSITY);
						CurLink->FolHtTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_HEIGHT);
						CurLink->FolBacklightTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
						CurLink->FolColorTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_COLOR);
						CurLink->FolDensTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_DENSITY);
						CurLink->FolHtTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_HEIGHT);
						CurLink->FolColorTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_COLOR);
						} // if new FoliageLink
					else
						return (0);
					} // if enabled
				CurFol = CurFol->Next;
				} // while CurFol
			} // if new FoliageChainList
		else
			return (0);
		} // if
	LoopGroup = LoopGroup->Next;
	} // while LoopGroup
#endif // WCS_USE_OLD_FOLIAGE

return (1);

} // Ecotype::BuildFoliageChainDensByGroup

/*===========================================================================*/

int Ecotype::BuildFoliageChainHtByGroup(RenderData *Rend)
{
//FoliageChainList *CurChain, **ChainPtr = &ChainList;
//FoliageLink *CurLink, **FolLinkPtr;
FoliageGroup *CurGrp;//, *LoopGroup = FolGrp;
//Foliage *CurFol;
//double ProjPlaneDissolveHt, SumDens, CurDens, SumCurGroupDens, MaxxHeight, MinHeight;
double ProjPlaneDissolveHt, MaxxHeight;
//RootTexture *EcoDensTex, *EcoHtTex, *GrpDensTex, *GrpHtTex;
//ThematicMap *EcoDensTheme, *EcoHtTheme, *GrpDensTheme, *GrpHtTheme;
VertexDEM FolVtx1, FolVtx2;

if (ConstDensity)
	{
	switch (DensityUnits)
		{
		case WCS_FOLIAGE_DENSITY_HECTARE:
			{
			DensityNormalizer = 1.0 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_HECTARE
		case WCS_FOLIAGE_DENSITY_ACRE:
			{
			DensityNormalizer = 2.471 / 10000.0;
			break;
			} // WCS_FOLIAGE_DENSITY_ACRE
		case WCS_FOLIAGE_DENSITY_SQFOOT:
			{
			DensityNormalizer = 10.76;
			break;
			} // WCS_FOLIAGE_DENSITY_SQFOOT
		//case WCS_FOLIAGE_DENSITY_SQMETER:
		default:
			{
			DensityNormalizer = 1.0;
			break;
			} // WCS_FOLIAGE_DENSITY_SQMETER
		} // switch
	} // if
else
	DensityNormalizer = 1.0;

// set height range
MaxxHeight = 0.0;
CurGrp = FolGrp;
while (CurGrp)
	{
	if (CurGrp->Enabled)
		{
		if (CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue > MaxxHeight)
			MaxxHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
		} // if enabled
	CurGrp = CurGrp->Next;
	} // while

// determine dissolve distances [0] = near, [1] = far
FolVtx1.ScrnXYZ[0] = Rend->Width * 0.5;
FolVtx1.ScrnXYZ[1] = Rend->Height * 0.5;
FolVtx1.ScrnXYZ[2] = FolVtx1.Q = 1.0;
FolVtx2.CopyScrnXYZQ(&FolVtx1);
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx1, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
if (GlobalApp->AppEffects->EcosystemBase.DissolveByImageSize)
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight * Rend->SetupHeight / GlobalApp->AppEffects->EcosystemBase.DissolveRefImageHt;
else
	FolVtx2.ScrnXYZ[1] -= DissolvePixelHeight;
Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &FolVtx2, Rend->EarthLatScaleMeters, Rend->PlanetRad, 0);
FolVtx1.GetPosVector(&FolVtx2);
ProjPlaneDissolveHt = sqrt(FolVtx1.XYZ[0] * FolVtx1.XYZ[0] + FolVtx1.XYZ[1] * FolVtx1.XYZ[1] + FolVtx1.XYZ[2] * FolVtx1.XYZ[2]);

if (ProjPlaneDissolveHt > 0.0)
	DissolveDistance = MaxxHeight / ProjPlaneDissolveHt;
else
	DissolveDistance = 1.0;

#ifdef WCS_USE_OLD_FOLIAGE
// determine if there are ecotype textures and thematic maps to be evaluated during rendering
EcoDensTex = GetEnabledTexture(WCS_ECOTYPE_TEXTURE_DENSITY);
EcoDensTheme = GetEnabledTheme(WCS_ECOTYPE_THEME_DENSITY);

EcoHtTex = NULL;
EcoHtTheme = NULL;

// if absolute density and height are resident in ecotype then we only need one chain, but if they are in the groups then
// we will need a linked list of foliage link lists
while (LoopGroup)
	{
	SumDens = SumCurGroupDens = 0.0;
	// find total density for all groups and foliage objects
	CurGrp = FolGrp;
	while (CurGrp)
		{
		if (CurGrp->Enabled)
			{
			CurFol = CurGrp->Fol;
			while (CurFol)
				{
				if (CurFol->Enabled)
					{
					if (CurGrp == LoopGroup)
						SumCurGroupDens += .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;
					else
						SumDens += .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue;
					} // if enabled
				CurFol = CurFol->Next;
				} // while
			} // if enabled
		CurGrp = CurGrp->Next;
		} // while
	SumDens += SumCurGroupDens;

	// allocate a link for each foliage object and fill in values
	if (SumCurGroupDens > 0.0)
		{
		if (*ChainPtr = new FoliageChainList())
			{
			CurChain = *ChainPtr;
			ChainPtr = &CurChain->Next;
			FolLinkPtr = &CurChain->FoliageChain;
			// an amount to multiply times the themed ecotype density to compensate for the fact 
			// that each group (chain) gets only a fraction of the total stems
			CurChain->EcoThemeDensFactor =  SumCurGroupDens / SumDens;
			// normalize constant density
			if (ConstDensity)
				CurChain->DensityPerSqMeter = DensityNormalizer * AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue * CurChain->EcoThemeDensFactor;
			else
				CurChain->DensityPerSqMeter = .01 * AnimPar[WCS_ECOTYPE_ANIMPAR_DENSITY].CurValue * CurChain->EcoThemeDensFactor;

			CurDens = 0.0;
			CurGrp = LoopGroup;

			GrpDensTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
			GrpHtTex = CurGrp->GetEnabledTexture(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
			GrpDensTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_DENSITY);
			GrpHtTheme = CurGrp->GetEnabledTheme(WCS_FOLIAGEGRP_THEME_HEIGHT);

			// set current chain's density theme
			CurChain->DensityTheme = EcoDensTheme;

			// set absolute height range for current chain
			CurChain->MaxHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].CurValue;
			MinHeight = CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
			if (SecondHeightType == WCS_ECOTYPE_SECONDHT_RANGEPCT)
				{
				MinHeight = CurChain->MaxHeight - .01 * CurChain->MaxHeight * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
				CurChain->MaxHeight += (.01 * CurChain->MaxHeight * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue);
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if
			else if (SecondHeightType == WCS_ECOTYPE_SECONDHT_MINPCT)
				{
				MinHeight = CurChain->MaxHeight * .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].CurValue;
				if (MinHeight < 0.0)
					MinHeight = 0.0;
				} // if

			if (MinHeight > CurChain->MaxHeight)
				MinHeight = CurChain->MaxHeight;
			CurChain->HeightRange = CurChain->MaxHeight - MinHeight;

			CurFol = CurGrp->Fol;
			while (CurFol)
				{
				if (CurFol->Enabled)
					{
					if (*FolLinkPtr = new FoliageLink())
						{
						CurLink = *FolLinkPtr;
						FolLinkPtr = &CurLink->Next;
						CurLink->Density = CurDens + .01 * CurGrp->AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].CurValue * CurFol->AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].CurValue / SumCurGroupDens;
						CurDens = CurLink->Density;
						if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
							{
							if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Rast = CurFol->Img->GetRaster();
								CurLink->ColorImage = CurFol->Img->GetRaster()->GetIsColor();
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Shade3D = CurFol->Shade3D;
								CurLink->RenderOccluded = RenderOccluded;
								CurLink->ImageWidthFactor = (double)CurLink->Rast->Cols / CurLink->Rast->Rows;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // if
						else
							{
							if (CurFol->Obj && CurFol->Obj->GetEnabled())
								{
								CurLink->Enabled = 1;
								CurLink->Obj = CurFol->Obj;
								CurLink->PosShade = CurFol->PosShade;
								CurLink->Fol = CurFol;
								CurLink->Grp = CurGrp;
								} // if
							} // else
						CurLink->EcoDensTex = EcoDensTex;	// NULL if controlled abs density in group
						CurLink->EcoHtTex = EcoHtTex;	// NULL if controlled abs height in group
						CurLink->GrpDensTex = GrpDensTex;
						CurLink->GrpHtTex = GrpHtTex;
						// EcoDensTheme is found in FoliageChain, not FoliageLink to facillitate renderer density calculation
						CurLink->EcoHtTheme = EcoHtTheme;	// NULL if controlled abs height in group
						CurLink->GrpDensTheme = GrpDensTheme;
						CurLink->GrpHtTheme = GrpHtTheme;
						CurLink->FolDensTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_DENSITY);
						CurLink->FolHtTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_HEIGHT);
						CurLink->FolBacklightTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
						CurLink->FolColorTex = CurFol->GetEnabledTexture(WCS_FOLIAGE_TEXTURE_COLOR);
						CurLink->FolDensTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_DENSITY);
						CurLink->FolHtTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_HEIGHT);
						CurLink->FolColorTheme = CurFol->GetEnabledTheme(WCS_FOLIAGE_THEME_COLOR);
						} // if new FoliageLink
					else
						return (0);
					} // if enabled
				CurFol = CurFol->Next;
				} // while CurFol
			} // if new FoliageChainList
		else
			return (0);
		} // if
	LoopGroup = LoopGroup->Next;
	} // while LoopGroup
#endif // WCS_USE_OLD_FOLIAGE

return (1);

} // Ecotype::BuildFoliageChainHtByGroup

/*===========================================================================*/

#ifdef WCS_USE_OLD_FOLIAGE
void Ecotype::DeleteFoliageChain(void)
{
FoliageChainList *CurChain;
FoliageLink *CurLink;

while (ChainList)
	{
	CurChain = ChainList;
	ChainList = ChainList->Next;
	while (CurChain->FoliageChain)
		{
		CurLink = CurChain->FoliageChain;
		CurChain->FoliageChain = CurChain->FoliageChain->Next;
		delete CurLink;
		} // while
	delete CurChain;
	} // while

} // Ecotype::DeleteFoliageChain
#endif // WCS_USE_OLD_FOLIAGE

/*===========================================================================*/

#ifdef WCS_FORESTRY_WIZARD
int Ecotype::SelectForestryImageOrObject(RenderData *Rend, PolygonData *Poly, VertexData *Vtx[3], unsigned long SeedOffset, 
	int FolEffect, FoliagePreviewData *PointData)
{
ForestryRenderData ForDat;

ForDat.Eco = this;
ForDat.Poly = Poly;
ForDat.Vtx = Vtx;
ForDat.SeedOffset = SeedOffset;
ForDat.FolEffect = FolEffect ? true: false;
ForDat.RenderOpacity = 1.0;
Rand.Seed64BitShift(Poly->LatSeed + SeedOffset, Poly->LonSeed + SeedOffset);

if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	ComputeEcoSize(&ForDat);
	} // if
if (! FolEffect)
	{
	if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
		{
		// compute ForDat.NormalizedDensity
		if (! ForDat.EcoDensityComputed)
			{
			ComputeEcoDensity(&ForDat);
			} // if
		// average number of stems per polygon of this size
		if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
			ForDat.AvgNumStems = ForDat.NormalizedDensity * Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
		else
			ForDat.AvgNumStems = ForDat.NormalizedDensity;
		} // if
	} // if
else
	ForDat.AvgNumStems = 1.0;

return (SelectForestryImageOrObjectStage2(Rend, &ForDat, PointData));

} // Ecotype::SelectForestryImageOrObject

/*===========================================================================*/

/*
// usage of random numbers for foliage
Random[0] - added to GroupSum to determine if a stem should be drawn
Random[1] - vertex location randomization, used again for determining if another stem should be drawn
Random[2] - vertex location randomization
Random[3] - tests to see if textures have elliminated a stem
Random[4] - determines which foliage in a group to draw
Random[5] - determines height within minimum and maximum ends of height range
Random[6] - image FlipX state, random 3d object X rotation
Random[7] - random 3d object Y rotation
Random[8] - random 3d object Z rotation
*/

int Ecotype::SelectForestryImageOrObjectStage2(RenderData *Rend, ForestryRenderData *ForDat, FoliagePreviewData *PointData)
{
double Random[9], GroupSum = 0.0, SumGroupDensity = 0.0, SumFoliageDensity, InstanceTexturedDensity, MaxStemsThisGroup, 
	ThirdVtxWt, SumImageFinder, TexOpacity, FolHeight, Value[3];
FoliageGroup *CurGrp, *StrtGrp;
Foliage *CurFol;
int Success = 1, GroupNum = 1;
VertexDEM FolVtx;

Value[0] = Value[1] = Value[2] = 0.0;

// compute a group percentage for each group taking into account thematic maps but not textures
// added to replace below 3/27/08 GRH
for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
	{
	if (CurGrp->Enabled)
		{
		CurGrp->GroupSizeComputed = CurGrp->GroupDensityComputed = 0;
		if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			CurGrp->ComputeGroupSize(ForDat);
			// calculated ForDat->HeightRange, ForDat->MinHeight, ForDat->MaxHeight, ForDat->AgvHeight
			} // if
		else
			{
			CurGrp->ComputeGroupRelativeSize(ForDat);
			// calculated CurGrp->RelativeGroupSize
			} // else
		if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			CurGrp->ComputeGroupDensity(ForDat);	
			// calculated ForDat->NormalizedDensity, ForDat->RawDensity
			CurGrp->GroupPercentage = 1.0;
			} // if
		else
			{
			CurGrp->ComputeGroupRelativeDensity(ForDat);
			// calculated CurGrp->RelativeGroupDensity
			SumGroupDensity += CurGrp->RelativeGroupDensity;
			} // else
		} // if
	else
		{
		CurGrp->GroupPercentage = 0.0;
		} // else
	} // for
if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	if (SumGroupDensity > 0.0)
		{
		SumGroupDensity = 1.0 / SumGroupDensity;
		for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
			{
			if (CurGrp->Enabled)
				CurGrp->GroupPercentage = CurGrp->RelativeGroupDensity * SumGroupDensity;
			} // for
		} // if
	else
		return (1);
	} // if
/* removed and replaced by above 7/1/08 GRH
// For ecotypes that have density by group just set each percentage to 1
if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
	{
	for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
		{
		if (CurGrp->Enabled)
			{
			CurGrp->GroupPercentage = 1.0;
			CurGrp->ComputeGroupRelativeSize(ForDat);
			} // if
		else
			CurGrp->GroupPercentage = 0.0;
		} // for
	} // if
else
	{
	// if there are thematic maps for any of the group %'s then they need to be evaluated and new
	// relative percentages calculated for each group
	for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
		{
		if (CurGrp->Enabled)
			{
			CurGrp->ComputeGroupRelativeDensity(ForDat);
			SumGroupDensity += CurGrp->RelativeGroupDensity;
			CurGrp->ComputeGroupRelativeSize(ForDat);
			} // if
		} // for
	if (SumGroupDensity > 0.0)
		{
		SumGroupDensity = 1.0 / SumGroupDensity;
		for (CurGrp = FolGrp; CurGrp; CurGrp = CurGrp->Next)
			{
			if (CurGrp->Enabled)
				CurGrp->GroupPercentage = CurGrp->RelativeGroupDensity * SumGroupDensity;
			else
				CurGrp->GroupPercentage = 0.0;
			} // for
		} // if
	else
		return (1);
	} // else
*/
// if foliage effect, choose a group to render from. Only going to render one unit.
if (ForDat->FolEffect)
	{
	Random[0] = Rand.GenPRN();
	// the granularity of the random numbers is .00001 - 
	// this prohibits fine control over foliage density at low densities per polygon.
	// The solution is to add a fraction of that granularity if the random # is very small.
	if (Random[0] < .0001)
		Random[0] += (.00001 * Rand.GenPRN());
	// find the Foliage to draw
	SumImageFinder = 0.0;
	for (StrtGrp = FolGrp; StrtGrp && StrtGrp->Next; StrtGrp = StrtGrp->Next, GroupNum ++)
		{
		SumImageFinder += StrtGrp->GroupPercentage;
		if (SumImageFinder >= Random[0])
			break;
		} // while
	} // if
else
	StrtGrp = FolGrp;

for (CurGrp = StrtGrp; CurGrp && Success; CurGrp = CurGrp->Next, GroupNum ++)
	{
	if (CurGrp->GroupPercentage > 0.0)
		{
		// copy data over from group into common data structure for ready access. Added 7/1/08 GRH
		if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			ForDat->NormalizedDensity = CurGrp->NormalizedDensity;
			ForDat->RawDensity = CurGrp->RawDensity;
			if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
				ForDat->AvgNumStems = ForDat->NormalizedDensity * ForDat->Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
			else
				ForDat->AvgNumStems = ForDat->NormalizedDensity;
			} // if
		if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
			{
			ForDat->HeightRange = CurGrp->HeightRange;
			ForDat->MinHeight = CurGrp->MinHeight;
			ForDat->MaxHeight = CurGrp->MaxHeight;
			ForDat->AgvHeight = CurGrp->AgvHeight;
			} // if
		// removed and replaced by above 7/1/08 GRH
		//ForDat->GroupDensityComputed = ForDat->GroupSizeComputed = false;
		// if there are thematic maps for any of the folaige %'s then they need to be evaluated and new
		// relative percentages calculated for each foliage
		SumFoliageDensity = 0.0;
		for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
			{
			if (CurFol->Enabled)
				{
				CurFol->ComputeFoliageRelativeDensity(ForDat);
				SumFoliageDensity += CurFol->RelativeFoliageDensity;
				CurFol->ComputeFoliageRelativeSize(ForDat);
				} // if
			} // for
		if (SumFoliageDensity > 0.0)
			{
			SumFoliageDensity = 1.0 / SumFoliageDensity;
			for (CurFol = CurGrp->Fol; CurFol; CurFol = CurFol->Next)
				{
				if (CurFol->Enabled)
					CurFol->FoliagePercentage = CurFol->RelativeFoliageDensity * SumFoliageDensity;
				else
					CurFol->FoliagePercentage = 0.0;
				} // for

			if (! ForDat->FolEffect)
				{
				// by reseeding the PRNG for each group with a unique value we can get the same trees drawn
				// even if an earlier group becomes disabled or the number of instances of an earlier group changes
				if (GroupNum > 1)
					Rand.Seed64BitShift(ForDat->Poly->LatSeed + ForDat->SeedOffset * GroupNum, ForDat->Poly->LonSeed + ForDat->SeedOffset * GroupNum);
				// if density is in groups we need to compute the untextured density now
				// if textures apply they will be calculated below using vertex locations of actual stems
				// removed and replaced by above 7/1/08 GRH
				//if (AbsDensResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
				//	{
				//	CurGrp->ComputeGroupDensity(ForDat);
				//	// average number of stems per polygon of this size
				//	if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
				//		ForDat->AvgNumStems = ForDat->NormalizedDensity * ForDat->Poly->Area;	// Area is always positive calculated by SolveTriangleArea()
				//	else
				//		ForDat->AvgNumStems = ForDat->NormalizedDensity;
				//	} // if
				Random[0] = Rand.GenPRN();
				// the granularity of the random numbers is .00001 - 
				// this prohibits fine control over foliage density at low densities per polygon.
				// The solution is to add a fraction of that granularity if the random # is very small.
				if (Random[0] < .0001)
					Random[0] += (.00001 * Rand.GenPRN());

				/* <<<>>> removed and replaced 7/1/08 to match RenderForestry.cpp
				if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
					{
					// render a number of stems of that group until random numbers exceed the average per group
					// for each group - multiply the group percentage times the average number of stems
					MaxStemsThisGroup = CurGrp->GroupPercentage * ForDat->AvgNumStems * .5;
					} // if
				else
					{
					// variable density requres a slightly different method to make it come out with the right density
					MaxStemsThisGroup = CurGrp->GroupPercentage * ForDat->AvgNumStems;
					} // else
				*/
				// this is the old method from VNS 1 and it seems to work very well. Tested it in renderer on 7/22/04
				MaxStemsThisGroup = ((Rand.GenGauss() * (1.0 / 6.9282)) + 1.0) * CurGrp->GroupPercentage * ForDat->AvgNumStems;

				GroupSum = Random[0];
				} // if
			else
				{
				GroupSum = 0.0;
				MaxStemsThisGroup = 1.0;
				} // else
			while (GroupSum <= MaxStemsThisGroup)
				{
				// compute group size if needed before textures are initialized for this foliage vertex location
				// removed and replaced by above 7/1/08 GRH
				//if (! ForDat->GroupSizeComputed)
				//	{
				//	if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_FOLGROUP)
				//		{
				//		CurGrp->ComputeGroupSize(ForDat);
				//		} // if
				//	ForDat->GroupSizeComputed = 1;
				//	} // if

				Rand.GenMultiPRN(8, &Random[1]);
				// calculate a random position for this foliage stem within the polygon
				if (! ForDat->FolEffect && ForDat->Vtx)
					{
					// method for evenly distributing points over a triangle adopted for VNS 2/WCS 6
					if (Random[1] + Random[2] > 1.0)
						{
						Random[1] = 1.0 - Random[1];
						Random[2] = 1.0 - Random[2];
						} // if
					ThirdVtxWt = 1 - Random[1] - Random[2];
					FolVtx.Lat = ForDat->Vtx[0]->Lat * Random[1] + ForDat->Vtx[1]->Lat * Random[2] + ForDat->Vtx[2]->Lat * ThirdVtxWt;
					FolVtx.Lon = ForDat->Vtx[0]->Lon * Random[1] + ForDat->Vtx[1]->Lon * Random[2] + ForDat->Vtx[2]->Lon * ThirdVtxWt;
					FolVtx.Elev = ForDat->Vtx[0]->Elev * Random[1] + ForDat->Vtx[1]->Elev * Random[2] + ForDat->Vtx[2]->Elev * ThirdVtxWt;
					} // if
				else
					{
					FolVtx.Lat = ForDat->Poly->Lat;
					FolVtx.Lon = ForDat->Poly->Lon;
					FolVtx.Elev = ForDat->Poly->Elev;
					} // else

				// project the foliage base position
				Rend->Cam->ProjectVertexDEM(Rend->DefCoords, &FolVtx, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);	// 1 means convert vertex to cartesian

				// in case textures exist
				Rend->TransferTextureData(ForDat->Poly);
				// set foliage coordinates
				Rend->TexData.Elev = FolVtx.Elev;
				Rend->TexData.Latitude = FolVtx.Lat;
				Rend->TexData.Longitude = FolVtx.Lon;
				Rend->TexData.ZDist = FolVtx.ScrnXYZ[2];
				Rend->TexData.QDist = FolVtx.Q;
				Rend->TexData.VDEM[0] = &FolVtx;
				Rend->TexData.WaterDepth = ForDat->Poly->WaterElev - FolVtx.Elev;
				Rend->TexData.VDataVecOffsetsValid = 0;

				// if there are ecotype or group textures calculate them and multiply together
				// test to see if the second RN falls below the product
				InstanceTexturedDensity = 1.0;
				if (! ForDat->FolEffect)
					{
					if (EcoDensityTex)
						{
						if ((TexOpacity = EcoDensityTex->Eval(Value, &Rend->TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								// Value[0] has already been diminished by the texture's opacity
								InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
								} // if
							else
								InstanceTexturedDensity *= Value[0];
							} // if
						} // if
					if (CurGrp->GroupDensityTex)
						{
						if ((TexOpacity = CurGrp->GroupDensityTex->Eval(Value, &Rend->TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								// Value[0] has already been diminished by the texture's opacity
								InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
								} // if
							else
								InstanceTexturedDensity *= Value[0];
							} // if
						} // if
					} // if

				// test to see if ecotype or group textures wiped out this instance
				if (Random[3] <= InstanceTexturedDensity)
					{
					// find the Foliage to draw
					SumImageFinder = 0.0;
					for (CurFol = CurGrp->Fol; CurFol && CurFol->Next; CurFol = CurFol->Next)
						{
						SumImageFinder += CurFol->FoliagePercentage;
						if (SumImageFinder >= Random[4])
							break;
						} // while

					if (CurFol)
						{
						// evaluate foliage density texture
						if (! ForDat->FolEffect && CurFol->FoliageDensityTex)
							{
							if ((TexOpacity = CurFol->FoliageDensityTex->Eval(Value, &Rend->TexData)) > 0.0)
								{
								if (TexOpacity < 1.0)
									{
									// Value[0] has already been diminished by the texture's opacity
									InstanceTexturedDensity *= (1.0 - TexOpacity + Value[0]);
									} // if
								else
									InstanceTexturedDensity *= Value[0];
								} // if
							} // if

						// test it again taking foliage texture into account
						if (Random[3] <= InstanceTexturedDensity)
							{
							// looks like we're really going to render this thing so now we have to figure out how big
							FolHeight = ForDat->MinHeight + ForDat->HeightRange * Random[5];
							// if absolute size is in ecotype, reduce by group size
							if (AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
								FolHeight *= CurGrp->RelativeGroupSize;
							// reduce by individual foliage size
							FolHeight *= CurFol->RelativeFoliageSize;

							if (EcoSizeTex)
								{
								if ((TexOpacity = EcoSizeTex->Eval(Value, &Rend->TexData)) > 0.0)
									{
									if (TexOpacity < 1.0)
										{
										// Value[0] has already been diminished by the texture's opacity
										FolHeight *= (1.0 - TexOpacity + Value[0]);
										} // if
									else
										FolHeight *= Value[0];
									} // if
								} // if
							if (CurGrp->GroupSizeTex)
								{
								if ((TexOpacity = CurGrp->GroupSizeTex->Eval(Value, &Rend->TexData)) > 0.0)
									{
									if (TexOpacity < 1.0)
										{
										// Value[0] has already been diminished by the texture's opacity
										FolHeight *= (1.0 - TexOpacity + Value[0]);
										} // if
									else
										FolHeight *= Value[0];
									} // if
								} // if
							if (CurFol->FoliageSizeTex)
								{
								if ((TexOpacity = CurFol->FoliageSizeTex->Eval(Value, &Rend->TexData)) > 0.0)
									{
									if (TexOpacity < 1.0)
										{
										// Value[0] has already been diminished by the texture's opacity
										FolHeight *= (1.0 - TexOpacity + Value[0]);
										} // if
									else
										FolHeight *= Value[0];
									} // if
								} // if

							// textured height has been found
							if (FolHeight > 0.0)
								{
								if (! (Success = SelectForestryImageOrObjectStage3(Rend, ForDat, PointData,
									CurFol, &FolVtx, Random, FolHeight)))
									break;
								} // if
							} // if
						} // if
					} // if

				// see if we need to draw another one
				if (ForDat->FolEffect || GroupSum >= MaxStemsThisGroup)
					break;
				if (ConstDensity == WCS_FOLIAGE_DENSITY_CONSTANT)
					{
					// this replaced below 7/22/04 in the Renderer, 7/1/08 here
					MaxStemsThisGroup -= 1.0;
					/*
					Random[0] = Rand.GenPRN();
					// see note near top of function for explanation
					if (Random[0] < .0001)
					Random[0] += (.00001 * Rand.GenPRN());
					GroupSum += Random[0];
					if (GroupSum > MaxStemsThisGroup)
						{
						Diff = (GroupSum - MaxStemsThisGroup);
						Random[1] = Rand.GenPRN();
						if (Random[1] <= Diff)
							GroupSum = MaxStemsThisGroup;
						} // if
					*/
					} // if
				else if (MaxStemsThisGroup > 1.0)
					{
					MaxStemsThisGroup -= 1.0;
					/* not needed anymore 7/22/04 in the Renderer, 7/1/08 here
					Random[0] = Rand.GenPRN();
					// see note near top of function for explanation
					if (Random[0] < .0001)
						Random[0] += (.00001 * Rand.GenPRN());
					GroupSum = Random[0];
					*/
					} // else
				else
					break;
				} // while
			} // if
		} // if
	if (ForDat->FolEffect)
		break;
	} // for

return (Success);

} // Ecotype::SelectForestryImageOrObjectStage2

/*===========================================================================*/

int Ecotype::SelectForestryImageOrObjectStage3(RenderData *Rend, ForestryRenderData *ForDat, FoliagePreviewData *PointData,
	Foliage *FolToRender, VertexDEM *FolVtx, double *Random, double FolHeight)
{
double EnvHeightFactor, Value[3], TexOpacity;
int Success = 1, FlipX;

Value[0] = Value[1] = Value[2] = 0.0;

// Use factor in default Environment if no Environment specified for this polygon (could be Foliage or Ecosystem Effect).
EnvHeightFactor = ForDat->Poly->Env ? ForDat->Poly->Env->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue: 
	Rend->DefaultEnvironment->AnimPar[WCS_EFFECTS_ENVIRONMENT_ANIMPAR_FOLIAGEHTFACT].CurValue;
FolHeight *= EnvHeightFactor;
// now paths diverge between Image Objects and 3D Objects
if (FolToRender->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
	{
	PointData->Rotate[0] = FolToRender->RandomRotate[0] ? FolToRender->Rotate[0] * 2.0 * (Random[6] - .5): 0.0;
	PointData->Rotate[1] = FolToRender->RandomRotate[1] ? FolToRender->Rotate[1] * 2.0 * (Random[7] - .5): 0.0;
	PointData->Rotate[2] = FolToRender->RandomRotate[2] ? FolToRender->Rotate[2] * 2.0 * (Random[8] - .5): 0.0;
	// render the object, pass the rotation set, height, shading options, position of base
	if (FolToRender->Obj)
		FolToRender->Obj->Rand.Copy(&Rand);

	PointData->Object3D = FolToRender->Obj;
	PointData->Height = FolHeight;
	PointData->Width = FolHeight * FolToRender->ImageWidthFactor;
	} // if 3D Object
else
	{
	FlipX = (FolToRender->FlipX && Random[6] >= .5);
	// Determine if there is a color texture and evaluate it.
	// If the opacity of the texture is less than 100% then the 
	// texture color is either blended with the replacement color
	// or blended with the image color during rendering.

	// To apply the color Value[3] to a foliage:
	//	if gray image
	//		RGB = Luminosity * Value[]
	//  if color image
	//		RGB = Luminosity * Value[] + ImageRGB * (1 - Texture Opacity)
	if (ForDat->Poly->TintFoliage)
		{
		Value[0] = ForDat->Poly->RGB[0];
		Value[1] = ForDat->Poly->RGB[1];
		Value[2] = ForDat->Poly->RGB[2];
		TexOpacity = 1.0;
		} // if
	else if (FolToRender->FoliageColorTheme && FolToRender->FoliageColorTheme->Eval(Value, ForDat->Poly->Vector))
		{
		// colors will be in range 0-255 hopefully so we need to reduce to 0-1 and elliminate negatives
		if (Value[0] <= 0.0)
			Value[0] = 0.0;
		else
			Value[0] *= (1.0 / 255.0);
		if (Value[1] <= 0.0)
			Value[1] = 0.0;
		else
			Value[1] *= (1.0 / 255.0);
		if (Value[2] <= 0.0)
			Value[2] = 0.0;
		else
			Value[2] *= (1.0 / 255.0);
		TexOpacity = 1.0;
		} // else if
	else if (FolToRender->FoliageColorTex)
		{
		if ((TexOpacity = FolToRender->FoliageColorTex->Eval(Value, &Rend->TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// if gray image blend texture color with replacement color
				if (! FolToRender->ColorImage)
					{
					Value[0] += FolToRender->Color.GetCompleteValue(0) * (1.0 - TexOpacity);
					Value[1] += FolToRender->Color.GetCompleteValue(1) * (1.0 - TexOpacity);
					Value[2] += FolToRender->Color.GetCompleteValue(2) * (1.0 - TexOpacity);
					} // else
				} // if
			} // if
		else if (! FolToRender->ColorImage)
			{
			Value[0] = FolToRender->Color.GetCompleteValue(0);
			Value[1] = FolToRender->Color.GetCompleteValue(1);
			Value[2] = FolToRender->Color.GetCompleteValue(2);
			} // else if gray image
		// else Value[] already set to 0's by texture evaluator
		} // if
	else
		{
		TexOpacity = 0.0;
		if (! FolToRender->ColorImage)
			{
			Value[0] = FolToRender->Color.GetCompleteValue(0);
			Value[1] = FolToRender->Color.GetCompleteValue(1);
			Value[2] = FolToRender->Color.GetCompleteValue(2);
			} // if gray image
		else
			{
			Value[0] = Value[1] = Value[2] = 0.0;
			} // else need to set Value[] to 0's
		} // else

	PointData->Height = FolHeight;
	PointData->Width = FolHeight * FolToRender->ImageWidthFactor;

	PointData->CurRast = FolToRender->Rast;
	PointData->ColorImageOpacity = 1.0 - TexOpacity;
	PointData->RGB[0] = Value[0];
	PointData->RGB[1] = Value[1];
	PointData->RGB[2] = Value[2];
	PointData->Shade3D = FolToRender->Shade3D;
	PointData->FlipX = FlipX;
	} // else it's an image Object

return (Success);

} // Ecotype::SelectForestryImageOrObjectStage3
#endif // WCS_FORESTRY_WIZARD

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Ecotype::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Ecotype *CurrentEcotype = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
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
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if Matl3D
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if Wave
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentObj->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
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
					else if (! strnicmp(ReadBuf, "Ecotype", 8))
						{
						if (CurrentEcotype = new Ecotype(NULL))
							{
							if ((BytesRead = CurrentEcotype->Load(ffile, Size, ByteFlip)) == Size)
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
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentEcotype)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentEcotype);
	delete CurrentEcotype;
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

} // Ecotype::LoadObject

/*===========================================================================*/

int Ecotype::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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

if (BuildFileComponentsList(&Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
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
	while (Waves)
		{
		CurEffect = Waves;
		Waves = Waves->Next;
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
	} // if

// Ecotype
strcpy(StrBuf, "Ecotype");
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
			} // if Ecotype saved 
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

} // Ecotype::SaveObject

/*===========================================================================*/
/*===========================================================================*/

FoliageGroup::FoliageGroup(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
double EffectDefault[WCS_FOLIAGEGRP_NUMANIMPAR] = {100.0, 0.0, 100.0, .2, 100.0, .2, 20.0};
double RangeDefaults[WCS_FOLIAGEGRP_NUMANIMPAR][3] = {100000.0, 0.0, 1.0,	// max height
													100000.0, 0.0, 1.0,		// min height
													100000.0, 0.0, 1.0,		// density
													100000.0, 0.0, .1,		// dbh
													FLT_MAX, 0.0, 1.0,		// basal area
													5.0, 0.0, .01,			// crown closure
													100000.0, 0.0, 1.0		// age
													};
long Ct;
GraphNode *Node;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

Name[0] = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for
DBHCurve.SetDefaults(this, (char)Ct ++);
AgeCurve.SetDefaults(this, (char)Ct ++);

Next = NULL;
Fol = NULL;
Enabled = 1;

DBHCurve.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AgeCurve.SetFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

DBHCurve.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
DBHCurve.SetHorizontalMetric(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AgeCurve.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);

strcpy(AgeCurve.XLabel, "Age");
strcpy(AgeCurve.YLabel, "Ht.");

strcpy(DBHCurve.XLabel, "DBH");
strcpy(DBHCurve.YLabel, "Ht.");

AgeCurve.RemoveNode(10.0, 0.1);
AgeCurve.AddNode(12.5, 7.0, 0.0);
AgeCurve.AddNode(25.0, 12.0, 0.0);
AgeCurve.AddNode(50.0, 17.0, 0.0);
if (Node = AgeCurve.AddNode(100.0, 20.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

DBHCurve.RemoveNode(10.0, 0.1);
DBHCurve.AddNode(.125, 7.0, 0.0);
DBHCurve.AddNode(.25, 12.0, 0.0);
DBHCurve.AddNode(.5, 17.0, 0.0);
if (Node = DBHCurve.AddNode(1.0, 20.0, 0.0))
	{
	Node->TCB[0] = 1.0;
	} // if

DBHCurve.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AgeCurve.ClearFlags(WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

#ifdef WCS_FORESTRY_WIZARD
GroupPercentage = RelativeGroupDensity = RelativeGroupSize = HeightRange = MinHeight = MaxHeight = AgvHeight =
	NormalizedDensity = RawDensity = MaxDbh = MaxAge = MaxClosure = MaxStems = MaxBasalArea = 0.0;
AvgImageHeightWidthRatio = 1.0;
GroupSizeTex = GroupDensityTex = GroupHeightTex = GroupStemsTex = GroupDbhTex = GroupClosureTex = GroupAgeTex = 
	GroupBasalAreaTex = NULL;
GroupSizeTheme = GroupDensityTheme = GroupHeightTheme = GroupMinHeightTheme = GroupStemsTheme = GroupDbhTheme = GroupClosureTheme = GroupAgeTheme = 
	GroupBasalAreaTheme = NULL;
GroupSizeComputed = GroupDensityComputed = false;
#endif // WCS_FORESTRY_WIZARD

} // FoliageGroup::FoliageGroup

/*===========================================================================*/

FoliageGroup::~FoliageGroup()
{
Foliage *NextFol;
long Ct;
RootTexture *DelTex;

while (Fol)
	{
	NextFol = Fol;
	Fol = Fol->Next;
	delete NextFol;
	} // while

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // FoliageGroup::~FoliageGroup

/*===========================================================================*/

void FoliageGroup::SetAnimDefaults(Ecotype *Master)
{
double RangeDefaults[3][3] = {100000.0, 0.0, 1.0, 100000.0, 0.0, .1, 1.0, 0.0, .01};

if (Master->AbsHeightResident == WCS_ECOTYPE_ABSRESIDENT_ECOTYPE)
	{
	AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
	AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
	AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
	AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
	} // if
else
	{
	AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
	if (Master->SecondHeightType == WCS_ECOTYPE_SECONDHT_MINABS)
		{
		if (! GlobalApp->ForestryAuthorized || Master->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_HEIGHT)
			{
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
			} // if
		else if (Master->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_DBH)
			{
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[1]);
			} // if
		else if (Master->SizeMethod == WCS_FOLIAGE_SIZEMETHOD_AGE)
			{
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
			} // if
		else	// crown closure
			{
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(100.0);
			AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[2]);
			} // if
		} // if
	else
		{
		AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
		AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetMultiplier(1.0);
		AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].SetRangeDefaults(RangeDefaults[0]);
		} // else
	} // if

AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].SetMultiplier(100.0);

} // FoliageGroup::SetAnimDefaults

/*===========================================================================*/

void FoliageGroup::Copy(FoliageGroup *CopyTo, FoliageGroup *CopyFrom)
{
Foliage *CurrentFrom = CopyFrom->Fol, **ToPtr, *NextFol;
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
AgeCurve.Copy(&CopyTo->AgeCurve, &CopyFrom->AgeCurve);
DBHCurve.Copy(&CopyTo->DBHCurve, &CopyFrom->DBHCurve);

// delete existing foliage 
while (CopyTo->Fol)
	{
	NextFol = CopyTo->Fol;
	CopyTo->Fol = CopyTo->Fol->Next;
	delete NextFol;
	} // while

ToPtr = &CopyTo->Fol;

while (CurrentFrom)
	{
	if (*ToPtr = new Foliage(CopyTo))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

strcpy(CopyTo->Name, CopyFrom->Name);

CopyTo->Enabled = CopyFrom->Enabled;
RootTextureParent::Copy(CopyTo, CopyFrom);
ThematicOwner::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

// see what can be done if heights or densities are stored according to different rules
if (CopyTo->RAParent && CopyFrom->RAParent)
	{
	if (((Ecotype *)CopyTo->RAParent)->AbsHeightResident != ((Ecotype *)CopyFrom->RAParent)->AbsHeightResident)
		{
		CopyTo->SetAnimDefaults((Ecotype *)CopyTo->RAParent);
		} // if
	} // if

} // FoliageGroup::Copy

/*===========================================================================*/

Foliage *FoliageGroup::FindFoliage(char *FindName)
{
Foliage *CurFol = Fol;

while (CurFol)
	{
	if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
		{
		if (CurFol->Img && CurFol->Img->GetRaster())
			{
			if (! stricmp(FindName, CurFol->Img->GetRaster()->GetUserName()))
				break;
			if (! stricmp(FindName, CurFol->Img->GetRaster()->GetName()))
				break;
			} // if
		} // if
	else if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D)
		{
		if (CurFol->Obj)
			{
			if (! stricmp(FindName, CurFol->Obj->GetName()))
				break;
			} // if
		} // if
	CurFol = CurFol->Next;
	} // while

return (CurFol);

} // FoliageGroup::FindFoliage

/*===========================================================================*/

Foliage *FoliageGroup::AddFoliage(Foliage *CopyFol)
{
Foliage **LoadTo = &Fol;
NotifyTag Changes[2];

while (*LoadTo)
	{
	LoadTo = &(*LoadTo)->Next;
	} // while
if (*LoadTo = new Foliage(this))
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), (*LoadTo)->GetNotifySubclass(), WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	if (CopyFol)
		{
		(*LoadTo)->Copy(*LoadTo, CopyFol);
		Changes[0] = MAKE_ID((*LoadTo)->GetNotifyClass(), (*LoadTo)->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, (*LoadTo)->GetRAHostRoot());
		} // if
	} // if

return (*LoadTo);

} // FoliageGroup::AddFoliage

/*===========================================================================*/

int FoliageGroup::SetToTime(double Time)
{
long Found = 0, Ct;
Foliage *Current = Fol;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while

return (Found);

} // FoliageGroup::SetToTime

/*===========================================================================*/

int FoliageGroup::AnimateShadows(void)
{
long Ct;
Foliage *Current = Fol;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0) > 1)
		{
		return (1);
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled && TexRoot[Ct]->IsAnimated())
		{
		return (1);
		} // if
	} // for

while (Current)
	{
	if (Current->Enabled && Current->AnimateShadows())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // FoliageGroup::AnimateShadows

/*===========================================================================*/

void FoliageGroup::EditRAHost(void)
{
RasterAnimHost *Root;
RasterAnimHostProperties Prop;
bool OpenEd = true;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;

DONGLE_INLINE_CHECK()
if (Root = GetRAHostRoot())
	{
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			OpenEd = false;
		} // if
	if (OpenEd)
		Root->EditRAHost();
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->FLG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->FLG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->ECG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->ECG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->LEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->LEG->ActivateFoliageItem(this);
			} // if
		} // if
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->SEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->SEG->ActivateFoliageItem(this);
			} // if
		} // if
	} // if

} // FoliageGroup::EditRAHost

/*===========================================================================*/

unsigned long FoliageGroup::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
#ifdef WCS_THEMATIC_MAP
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
#endif // WCS_THEMATIC_MAP
char TexRootNumber = -1;
Foliage **LoadTo;

LoadTo = &Fol;

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
					case WCS_FOLIAGEGRP_BROWSEDATA:
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
					case WCS_FOLIAGEGRP_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_V5HEIGHTPCT:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].ScaleValues(100.0);
						break;
						}
					case WCS_FOLIAGEGRP_MAXHEIGHT:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_MINHEIGHT:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_V5DENSPCT:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].ScaleValues(100.0);
						break;
						}
					case WCS_FOLIAGEGRP_DENSITY:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_DBH:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_DBH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_BASALAREA:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_BASALAREA].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_CROWNCLOSURE:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_AGE:
						{
						BytesRead = AnimPar[WCS_FOLIAGEGRP_ANIMPAR_AGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGEGRP_AGECURVE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = AgeCurve.Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_DBHCURVE:
						{
						if (GlobalApp->ForestryAuthorized)
							BytesRead = DBHCurve.Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_FOLIAGE:
						{
						if (*LoadTo = new Foliage(this))
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_FOLIAGEGRP_TEXHEIGHT:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_HEIGHT] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_HEIGHT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXDENSITY:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_DENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_DENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXDBH:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_DBH] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_DBH]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXBASALAREA:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_BASALAREA] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_BASALAREA]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXCROWNCLOSURE:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGEGRP_TEXAGE:
						{
						if (TexRoot[WCS_FOLIAGEGRP_TEXTURE_AGE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGEGRP_TEXTURE_AGE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_FOLIAGEGRP_THEMEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_HEIGHT] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMEMINHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_MINHEIGHT] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMEDENSITY:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_DENSITY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMEDBH:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_DBH] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMEBASALAREA:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_BASALAREA] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMECROWNCLOSURE:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_CROWNCLOSURE] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					case WCS_FOLIAGEGRP_THEMEAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_FOLIAGEGRP_THEME_AGE] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
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

} // FoliageGroup::Load

/*===========================================================================*/

unsigned long FoliageGroup::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
Foliage *Current;
unsigned long AnimItemTag[WCS_FOLIAGEGRP_NUMANIMPAR] = {WCS_FOLIAGEGRP_MAXHEIGHT,
														WCS_FOLIAGEGRP_MINHEIGHT,
														WCS_FOLIAGEGRP_DENSITY,
														WCS_FOLIAGEGRP_DBH,
														WCS_FOLIAGEGRP_BASALAREA,
														WCS_FOLIAGEGRP_CROWNCLOSURE,
														WCS_FOLIAGEGRP_AGE
														};
unsigned long TextureItemTag[WCS_FOLIAGEGRP_NUMTEXTURES] = {WCS_FOLIAGEGRP_TEXHEIGHT,
														WCS_FOLIAGEGRP_TEXDENSITY,
														WCS_FOLIAGEGRP_TEXDBH,
														WCS_FOLIAGEGRP_TEXBASALAREA,
														WCS_FOLIAGEGRP_TEXCROWNCLOSURE,
														WCS_FOLIAGEGRP_TEXAGE
														};
unsigned long ThemeItemTag[WCS_FOLIAGEGRP_NUMTHEMES] = {WCS_FOLIAGEGRP_THEMEHEIGHT,
														WCS_FOLIAGEGRP_THEMEMINHEIGHT,
														WCS_FOLIAGEGRP_THEMEDENSITY,
														WCS_FOLIAGEGRP_THEMEDBH,
														WCS_FOLIAGEGRP_THEMEBASALAREA,
														WCS_FOLIAGEGRP_THEMECROWNCLOSURE,
														WCS_FOLIAGEGRP_THEMEAGE
														};

if (BrowseInfo)
	{
	ItemTag = WCS_FOLIAGEGRP_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGEGRP_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGEGRP_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
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

ItemTag = WCS_FOLIAGEGRP_DBHCURVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = DBHCurve.Save(ffile))
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
			} // if DBHCurve saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_FOLIAGEGRP_AGECURVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = AgeCurve.Save(ffile))
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
			} // if AgeCurve saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

Current = Fol;
while (Current)
	{
	ItemTag = WCS_FOLIAGEGRP_FOLIAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if foliage saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	Current = Current->Next;
	} // while

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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // FoliageGroup::Save

/*===========================================================================*/

char *FoliageGroupCritterNames[WCS_FOLIAGEGRP_NUMANIMPAR] = {"Group Max Height", "Group Min Height", 
	"Group Density", "DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};
char *FoliageGroupTextureNames[WCS_FOLIAGEGRP_NUMTEXTURES] = {"Group Height (%)", "Group Density (%)", 
	"DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};
char *FoliageGroupThemeNames[WCS_FOLIAGEGRP_NUMTHEMES] = {"Group Height", "Group Density", 
	"DBH (m)", "Basal Area (sq m)", "Crown Closure (%)", "Age"};

char *FoliageGroup::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (FoliageGroupCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (FoliageGroupTextureNames[Ct]);
		} // if
	} // for
if (Test == &DBHCurve)
	return ("DBH Curve");
if (Test == &AgeCurve)
	return ("Age Curve");

return ("");

} // FoliageGroup::GetCritterName

/*===========================================================================*/

char *FoliageGroup::OKRemoveRaster(void)
{

return ("Image Object is used as a Foliage Group Texture! Remove anyway?");

} // FoliageGroup::OKRemoveRaster

/*===========================================================================*/

int FoliageGroup::GetRAHostAnimated(void)
{
long Ct;
Foliage *Current = Fol;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

while (Current)
	{
	if (Current->GetRAHostAnimated())
		return (1);
	Current = Current->Next;
	} // while

return (0);

} // FoliageGroup::GetRAHostAnimated

/*===========================================================================*/

bool FoliageGroup::AnimateMaterials(void)
{
long Ct;
Foliage *Current = Fol;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (true);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for

while (Current)
	{
	if (Current->AnimateMaterials())
		return (true);
	Current = Current->Next;
	} // while

return (false);

} // FoliageGroup::AnimateMaterials

/*===========================================================================*/

long FoliageGroup::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0, Ct;
Foliage *CurFol = Fol;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // while
while (CurFol)
	{
	if (CurFol->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurFol = CurFol->Next;
	} // while

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

} // FoliageGroup::GetKeyFrameRange

/*===========================================================================*/

char FoliageGroup::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGE
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER
	|| DropType == WCS_EFFECTSSUBCLASS_OBJECT3D)
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE && GlobalApp->ForestryAuthorized)
	return (1);

return (0);

} // FoliageGroup::GetRAHostDropOK

/*===========================================================================*/

int FoliageGroup::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0, QueryResult;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
Foliage *CurFol;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (FoliageGroup *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (FoliageGroup *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
	{
	Success = AddFoliage((Foliage *)DropSource->DropSource) ? 1: 0;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER
	|| DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = -1;
	for (Ct = 0, CurFol = Fol; CurFol && Ct < 30; Ct ++, CurFol = CurFol->Next)
		{
		TargetList[Ct] = CurFol;
		} // for
	NumListItems = Ct;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	if (GlobalApp->ForestryAuthorized)
		{
		Success = -1;
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Curve");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "DBH", "Cancel", "Age", 0))
			{
			if (QueryResult == 1)
				{
				Success = DBHCurve.ProcessRAHostDragDrop(DropSource);
				} // if
			else if (QueryResult == 2)
				{
				Success = AgeCurve.ProcessRAHostDragDrop(DropSource);
				} // else if
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // FoliageGroup::ProcessRAHostDragDrop

/*===========================================================================*/

char *FoliageGroup::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? FoliageGroupTextureNames[TexNumber]: (char*)"");

} // FoliageGroup::GetTextureName

/*===========================================================================*/

RootTexture *FoliageGroup::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // FoliageGroup::NewRootTexture

/*===========================================================================*/

int FoliageGroup::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;
unsigned long ItemRemoved;
Foliage *CurFol = Fol, *PrevFol = NULL;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		ItemRemoved = WCS_SUBCLASS_ROOTTEXTURE;
		} // if
	} // for

if (! Removed)
	{
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (RemoveMe == GetTheme(Ct))
			{
			SetThemePtr(Ct, NULL);
			Removed = 1;
			ItemRemoved = WCS_EFFECTSSUBCLASS_THEMATICMAP;
			} // if
		} // for
	} // if

if (! Removed)
	{
	while (CurFol)
		{
		if (CurFol == (Foliage *)RemoveMe)
			{
			if (PrevFol)
				PrevFol->Next = CurFol->Next;
			else
				Fol = CurFol->Next;

			delete CurFol;
			Removed = 1;
			ItemRemoved = WCS_SUBCLASS_FOLIAGE;
			break;
			} // if
		PrevFol = CurFol;
		CurFol = CurFol->Next;
		} // while
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), ItemRemoved, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // FoliageGroup::RemoveRAHost

/*===========================================================================*/

int FoliageGroup::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
Foliage *CurFol = Fol;

while (CurFol)
	{
	if (! CurFol->FindnRemove3DObjects(RemoveMe))
		return (0);
	CurFol = CurFol->Next;
	} // while

return (1);

} // FoliageGroup::FindnRemove3DObjects

/*===========================================================================*/

ThematicMap *FoliageGroup::GetEnabledTheme(long ThemeNum)
{

return (ThemeNum < WCS_FOLIAGEGRP_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);

} // FoliageGroup::GetEnabledTheme

/*===========================================================================*/

int FoliageGroup::SetTheme(long ThemeNum, ThematicMap *NewTheme)
{
NotifyTag Changes[2];

if (ThemeNum < GetNumThemes())
	{
	SetThemePtr(ThemeNum, NewTheme);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_EFFECTSSUBCLASS_THEMATICMAP, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if

return (0);

} // FoliageGroup::SetTheme

/*===========================================================================*/

char *FoliageGroup::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? FoliageGroupThemeNames[ThemeNum]: (char*)"");

} // FoliageGroup::GetThemeName

/*===========================================================================*/

unsigned long FoliageGroup::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_FOLIAGEGROUP | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | WCS_RAHOST_FLAGBIT_EDITNAME | Flags);

return (Mask);

} // FoliageGroup::GetRAFlags

/*===========================================================================*/

void FoliageGroup::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = Name;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_ECOSYSTEM];
	Prop->Ext = "fgp";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // FoliageGroup::GetRAHostProperties

/*===========================================================================*/

int FoliageGroup::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(Name, Prop->Name, WCS_EFFECT_MAXNAMELENGTH);
	Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // FoliageGroup::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *FoliageGroup::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
Foliage *CurFol;

if (! Current)
	Found = 1;
#ifdef WCS_FORESTRY_WIZARD
if (GlobalApp->ForestryAuthorized)
	{
	if (Found)
		return (&DBHCurve);
	if (Current == &DBHCurve)
		return (&AgeCurve);
	if (Current == &AgeCurve)
		Found = 1;
	} // if
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumAnimParams(): WCS_FOLIAGEGRP_ANIMPAR_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_FORESTRY_WIZARD
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumTextures(): WCS_FOLIAGEGRP_TEXTURE_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_FORESTRY_WIZARD
for (Ct = 0; Ct < (GlobalApp->ForestryAuthorized ? GetNumThemes(): WCS_FOLIAGEGRP_THEME_DBH); Ct ++)
#endif // WCS_FORESTRY_WIZARD
	{
	if (Found && GetTheme(Ct) && GetThemeUnique(Ct))
		return (GetTheme(Ct));
	if (Current == GetTheme(Ct) && GetThemeUnique(Ct))
		Found = 1;
	} // for
CurFol = Fol;
while (CurFol)
	{
	if (Found)
		return (CurFol);
	if (Current == CurFol)
		Found = 1;
	CurFol = CurFol->Next;
	} // while

return (NULL);

} // FoliageGroup::GetRAHostChild

/*===========================================================================*/

int FoliageGroup::GetDeletable(RasterAnimHost *Test)
{
char Ct;
Foliage *CurFol;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
CurFol = Fol;
while (CurFol)
	{
	if (Test == CurFol)
		return (1);
	CurFol = CurFol->Next;
	} // while

return (0);

} // FoliageGroup::GetDeletable

/*===========================================================================*/

int FoliageGroup::GetThemeType(long ThemeNum)
{
// return 0 for area and 1 to make it point type
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

if (Root = GetRAHostRoot())
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		return (1);
		} // if
	} // if

return (0);

} // FoliageGroup::GetThemeType

/*===========================================================================*/

int FoliageGroup::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT:
					{
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_MINHEIGHT);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_DENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_DENSITY);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_DBH:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_DBH);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_DBH);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_BASALAREA:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_BASALAREA);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_BASALAREA);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE);
					break;
					} // 
				case WCS_FOLIAGEGRP_ANIMPAR_AGE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_AGE);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_AGE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
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
				case WCS_FOLIAGEGRP_TEXTURE_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGEGRP_TEXTURE_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_DENSITY);
					break;
					} // 
				case WCS_FOLIAGEGRP_TEXTURE_DBH:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_DBH);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_DBH);
					break;
					} // 
				case WCS_FOLIAGEGRP_TEXTURE_BASALAREA:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_BASALAREA);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_BASALAREA);
					break;
					} // 
				case WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_CROWNCLOSURE);
					break;
					} // 
				case WCS_FOLIAGEGRP_TEXTURE_AGE:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_AGE);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGEGRP_THEME_AGE);
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
				case WCS_FOLIAGEGRP_THEME_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_MINHEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_DENSITY);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_DENSITY);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_DBH:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_DBH);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_DBH);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_BASALAREA:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_BASALAREA);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_BASALAREA);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_CROWNCLOSURE:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE);
					break;
					} // 
				case WCS_FOLIAGEGRP_THEME_AGE:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGEGRP_ANIMPAR_AGE);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGEGRP_TEXTURE_AGE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // FoliageGroup::GetAffiliates

/*===========================================================================*/

int FoliageGroup::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil));
	} // if

return (0);

} // FoliageGroup::GetPopClassFlags

/*===========================================================================*/

int FoliageGroup::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil));
	} // if

return(0);

} // FoliageGroup::AddSRAHBasePopMenus

/*===========================================================================*/

int FoliageGroup::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, ThemeAffil, this, this));
	} // if

return(0);

} // FoliageGroup::HandleSRAHPopMenuSelection

/*===========================================================================*/

long FoliageGroup::InitImageIDs(long &ImageID)
{
long Ct, ImagesFound = 0;
Foliage *CurFol = Fol;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		ImagesFound += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for
while (CurFol)
	{
	ImagesFound += CurFol->InitImageIDs(ImageID);
	CurFol = CurFol->Next;
	} // while

return (ImagesFound);

} // FoliageGroup::InitImageIDs

/*===========================================================================*/

int FoliageGroup::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
Foliage *CurFol = Fol;
long Ct;

while (CurFol)
	{
	if (! CurFol->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
		return (0);
	CurFol = CurFol->Next;
	} // while

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for

return (ThematicOwner::BuildFileComponentsList(Themes));

} // FoliageGroup::BuildFileComponentsList

/*===========================================================================*/

int FoliageGroup::InitToRender(void)
{
char Ct;
Foliage *CurFol = Fol;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for
while (CurFol)
	{
	if (! CurFol->InitToRender())
		return (0);
	CurFol = CurFol->Next;
	} // while

return (1);

} // FoliageGroup::InitToRender

/*===========================================================================*/

int FoliageGroup::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
char Ct;
Foliage *CurFol = Fol;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitFrameToRender(Lib, Rend))
			{
			return (0);
			} // if
		} // if
	} // for
while (CurFol)
	{
	if (! CurFol->InitFrameToRender(Lib, Rend))
		return (0);
	CurFol = CurFol->Next;
	} // while

return (1);

} // FoliageGroup::InitFrameToRender

/*===========================================================================*/

double FoliageGroup::CalcAvgImageHeightWidthRatio(void)
{
double Width, Height, Ratio = 0.0, ObjX, ObjY, ObjZ;
long NumItems = 0;
Foliage *CurFol;

CurFol = Fol;
while (CurFol)
	{
	if (CurFol->Enabled)
		{
		if (CurFol->FoliageType == WCS_FOLIAGE_TYPE_RASTER)
			{
			if (CurFol->Img && CurFol->Img->GetRaster() && CurFol->Img->GetRaster()->GetEnabled())
				{
				Width = CurFol->Img->GetRaster()->Cols;
				Height = CurFol->Img->GetRaster()->Rows;
				if (Width > 0.0)
					{
					Ratio += (Height / Width);
					NumItems ++;
					} // if
				} // if
			} // if
		else	// CurFol->FoliageType == WCS_FOLIAGE_TYPE_OBJECT3D
			{
			if (CurFol->Obj && CurFol->Enabled)
				{
				ObjX = fabs(CurFol->Obj->ObjectBounds[0] - CurFol->Obj->ObjectBounds[1]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
				ObjY = fabs(CurFol->Obj->ObjectBounds[2] - CurFol->Obj->ObjectBounds[3]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
				ObjZ = fabs(CurFol->Obj->ObjectBounds[4] - CurFol->Obj->ObjectBounds[5]) * CurFol->Obj->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
				Width = max(ObjX, ObjZ);
				Height = ObjY;
				if (Width > 0.0)
					{
					Ratio += (Height / Width);
					NumItems ++;
					} // if
				} // if
			} // else
		} // if
	CurFol = CurFol->Next;
	} // while

if (NumItems > 0)
	{
	return (Ratio / NumItems);
	} // if
else
	return (1.0);

} // FoliageGroup::CalcAvgImageHeightWidthRatio

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int FoliageGroup::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
FoliageGroup *CurrentFoliageGroup = NULL;
Object3DEffect *CurrentObj = NULL;
MaterialEffect *CurrentMaterial = NULL;
WaveEffect *CurrentWave = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
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
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if Matl3D
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if Wave
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentObj->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
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
					else if (! strnicmp(ReadBuf, "FolagGrp", 8))
						{
						if (CurrentFoliageGroup = new FoliageGroup(NULL))
							{
							if (RAParent)
								{
								// make sure parent is an Ecotype
								RasterAnimHostProperties Prop;
								Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
								RAParent->GetRAHostProperties(&Prop);
								if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
									CurrentFoliageGroup->SetAnimDefaults((Ecotype *)RAParent);
								} // if
							if ((BytesRead = CurrentFoliageGroup->Load(ffile, Size, ByteFlip)) == Size)
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
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentFoliageGroup)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentFoliageGroup);
	delete CurrentFoliageGroup;
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

} // FoliageGroup::LoadObject

/*===========================================================================*/

int FoliageGroup::SaveObject(FILE *ffile)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Material3Ds = NULL, *Object3Ds = NULL, *Waves = NULL, *Queries = NULL, *Themes = NULL, *Coords = NULL;
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

if (BuildFileComponentsList(&Material3Ds, &Object3Ds, &Waves, &Queries, &Themes, &Coords))
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
	while (Waves)
		{
		CurEffect = Waves;
		Waves = Waves->Next;
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
	} // if

// FoliageGroup
strcpy(StrBuf, "FolagGrp");
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
			} // if FoliageGroup saved 
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

} // FoliageGroup::SaveObject

/*===========================================================================*/
/*===========================================================================*/

Foliage::Foliage(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
double EffectDefault[WCS_FOLIAGE_NUMANIMPAR] = {1.0, 1.0, 0.25};
double RangeDefaults[WCS_FOLIAGE_NUMANIMPAR][3] = {10.0, 0.0, .01, 10.0, 0.0, .01, 2.0, 0.0, .01};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for

Color.SetDefaults(this, WCS_FOLIAGE_NUMANIMPAR);

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
for (Ct = 0; Ct < GetNumThemes(); Ct ++)
	{
	Theme[Ct] = NULL;
	} // for

Next = NULL;
Img = NULL;
Obj = NULL;
FoliageType = WCS_FOLIAGE_TYPE_RASTER; 
PosShade = 1; 
Shade3D = 1; 
RandomRotate[0] = RandomRotate[2] = 0;
RandomRotate[1] = 1;
Rotate[0] = Rotate[2] = 5.0;
Rotate[1] = 180.0;
FlipX = 1;
Enabled = 1;

AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].SetMultiplier(100.0);
AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].SetMultiplier(100.0);
AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING].SetMultiplier(100.0);

#ifdef WCS_FORESTRY_WIZARD
ColorImage = RenderOccluded = 0;
ImageWidthFactor = 1.0;
FoliagePercentage = RelativeFoliageDensity = RelativeFoliageSize = 0.0;
FoliageSizeTex = FoliageDensityTex = FoliageColorTex = FoliageBacklightTex = NULL;
FoliageSizeTheme = FoliageDensityTheme = FoliageColorTheme = NULL;
Rast = NULL;
#endif // WCS_FORESTRY_WIZARD

} // Foliage::Foliage

/*===========================================================================*/

Foliage::~Foliage()
{
long Ct;
RootTexture *DelTex;

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
	#ifdef WCS_FORESTRY_WIZARD
	Rast = NULL;
	#endif // WCS_FORESTRY_WIZARD
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // Foliage::~Foliage

/*===========================================================================*/

void Foliage::Copy(Foliage *CopyTo, Foliage *CopyFrom)
{
long Ct;
Raster *NewRast;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for

Color.Copy(&CopyTo->Color, &CopyFrom->Color);
CopyTo->FoliageType = CopyFrom->FoliageType;
CopyTo->PosShade = CopyFrom->PosShade;
CopyTo->Shade3D = CopyFrom->Shade3D;
CopyTo->RandomRotate[0] = CopyFrom->RandomRotate[0];
CopyTo->RandomRotate[1] = CopyFrom->RandomRotate[1];
CopyTo->RandomRotate[2] = CopyFrom->RandomRotate[2];
CopyTo->Rotate[0] = CopyFrom->Rotate[0];
CopyTo->Rotate[1] = CopyFrom->Rotate[1];
CopyTo->Rotate[2] = CopyFrom->Rotate[2];
CopyTo->FlipX = CopyFrom->FlipX;
CopyTo->Enabled = CopyFrom->Enabled;

if (CopyTo->Img)
	{
	if (CopyTo->Img->GetRaster())
		{
		CopyTo->Img->GetRaster()->RemoveAttribute(CopyTo->Img);
		} // if
	if (CopyTo->Img)
		delete CopyTo->Img;
	#ifdef WCS_FORESTRY_WIZARD
	CopyTo->Rast = NULL;
	#endif // WCS_FORESTRY_WIZARD
	} // if
CopyTo->Img = NULL;
if (CopyFrom->Img)
	{
	if (CopyTo->Img = new FoliageShell())
		{
		if (CopyFrom->Img->GetRaster())
			{
			if (NewRast = GlobalApp->CopyToImageLib->MatchNameMakeRaster(CopyFrom->Img->GetRaster()))
				{
				NewRast->AddAttribute(CopyTo->Img->GetType(), CopyTo->Img, CopyTo);
				} // if
			} // if
		} // if
	} // if
if (CopyFrom->Obj)
	CopyTo->Obj = (Object3DEffect *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Obj);
else
	CopyTo->Obj = NULL;
ThematicOwner::Copy(CopyTo, CopyFrom);
RootTextureParent::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // Foliage::Copy

/*===========================================================================*/

int Foliage::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (Color.SetToTime(Time))
	{
	Found = 1;
	} // if
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // Foliage::SetToTime

/*===========================================================================*/

int Foliage::AnimateShadows(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetNumNodes(0))
		{
		return (1);
		} // if
	} // for
for (Ct = 0; Ct < WCS_FOLIAGE_TEXTURE_COLOR; Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled && TexRoot[Ct]->IsAnimated())
		{
		return (1);
		} // if
	} // for
if (Img && Img->GetRaster() && Img->GetRaster()->GetEnabled() && (Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) || Img->GetRaster()->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)))
	return (1);
if (Obj && Obj->AnimateShadow3D())
	return (1);
return (0);

} // Foliage::AnimateShadows

/*===========================================================================*/

void Foliage::EditRAHost(void)
{
RasterAnimHost *Root;
RasterAnimHostProperties Prop;
bool OpenEd = true;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;

DONGLE_INLINE_CHECK()
if (Root = GetRAHostRoot())
	{
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			OpenEd = false;
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			OpenEd = false;
		} // if
	if (OpenEd)
		Root->EditRAHost();
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		if (GlobalApp->GUIWins->FLG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->FLG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->FLG->ActivateFoliageItem(this);
			} // if
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM)
		{
		if (GlobalApp->GUIWins->ECG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->ECG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->ECG->ActivateFoliageItem(this);
			} // if
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE)
		{
		if (GlobalApp->GUIWins->LEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->LEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->LEG->ActivateFoliageItem(this);
			} // if
		} // if
	else if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM)
		{
		if (GlobalApp->GUIWins->SEG)
			{
			// set the page to foliage
			GlobalApp->GUIWins->SEG->SetToFoliagePage();
			// set the active item in the foliage page
			GlobalApp->GUIWins->SEG->ActivateFoliageItem(this);
			} // if
		} // if
	} // if

} // Foliage::EditRAHost

/*===========================================================================*/

unsigned long Foliage::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID;
char TexRootNumber = -1, ObjectName[256];

ObjectName[0] = 0;

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
					case WCS_FOLIAGE_HEIGHTPCT:
						{
						BytesRead = AnimPar[WCS_FOLIAGE_ANIMPAR_HEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_DENSPCT:
						{
						BytesRead = AnimPar[WCS_FOLIAGE_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_ORIENTATIONSHADING:
						{
						BytesRead = AnimPar[WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_COLOR:
						{
						BytesRead = Color.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_FOLIAGETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_POSSHADE:
						{
						BytesRead = ReadBlock(ffile, (char *)&PosShade, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_SHADE3D:
						{
						BytesRead = ReadBlock(ffile, (char *)&Shade3D, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_RANDOMROTATEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomRotate[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_RANDOMROTATEY:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomRotate[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_RANDOMROTATEZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomRotate[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_ROTATEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rotate[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_ROTATEY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rotate[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_ROTATEZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rotate[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_FLIPX:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlipX, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_FOLIAGE_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (Img = new FoliageShell))
							{
							GlobalApp->LoadToImageLib->MatchRasterSetShell(ImageID, Img, this);
							#ifdef WCS_FORESTRY_WIZARD
							Rast = NULL;
							#endif // WCS_FORESTRY_WIZARD
							} // if
						break;
						}
					case WCS_FOLIAGE_OBJECTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)ObjectName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ObjectName[0])
							{
							Obj = (Object3DEffect *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_OBJECT3D, ObjectName);
							} // if
						break;
						}
					case WCS_FOLIAGE_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_FOLIAGE_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_FOLIAGE_TEXHEIGHT:
						{
						if (TexRoot[WCS_FOLIAGE_TEXTURE_HEIGHT] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGE_TEXTURE_HEIGHT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGE_TEXDENSITY:
						{
						if (TexRoot[WCS_FOLIAGE_TEXTURE_DENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGE_TEXTURE_DENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGE_TEXORIENTSHADING:
						{
						if (TexRoot[WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_FOLIAGE_TEXCOLOR:
						{
						if (TexRoot[WCS_FOLIAGE_TEXTURE_COLOR] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_FOLIAGE_TEXTURE_COLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_FOLIAGE_THEMEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)ObjectName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ObjectName[0])
							{
							Theme[WCS_FOLIAGE_THEME_HEIGHT] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, ObjectName);
							} // if
						break;
						}
					case WCS_FOLIAGE_THEMEDENSITY:
						{
						BytesRead = ReadBlock(ffile, (char *)ObjectName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ObjectName[0])
							{
							Theme[WCS_FOLIAGE_THEME_DENSITY] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, ObjectName);
							} // if
						break;
						}
					case WCS_FOLIAGE_THEMECOLOR:
						{
						BytesRead = ReadBlock(ffile, (char *)ObjectName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ObjectName[0])
							{
							Theme[WCS_FOLIAGE_THEME_COLOR] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, ObjectName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
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

} // Foliage::Load

/*===========================================================================*/

unsigned long Foliage::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long ImageID;
unsigned long AnimItemTag[WCS_FOLIAGE_NUMANIMPAR] = {WCS_FOLIAGE_HEIGHTPCT,
														WCS_FOLIAGE_DENSPCT,
														WCS_FOLIAGE_ORIENTATIONSHADING};
unsigned long TextureItemTag[WCS_FOLIAGE_NUMTEXTURES] = {WCS_FOLIAGE_TEXHEIGHT,
														WCS_FOLIAGE_TEXDENSITY,
														WCS_FOLIAGE_TEXCOLOR,
														WCS_FOLIAGE_TEXORIENTSHADING};
unsigned long ThemeItemTag[WCS_FOLIAGE_NUMTHEMES] = {WCS_FOLIAGE_THEMEHEIGHT,
														WCS_FOLIAGE_THEMEDENSITY,
														WCS_FOLIAGE_THEMECOLOR};

if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_FOLIAGETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FoliageType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_POSSHADE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PosShade)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_SHADE3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Shade3D)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_FLIPX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FlipX)) == NULL)
   goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_RANDOMROTATEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomRotate[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_RANDOMROTATEY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomRotate[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_RANDOMROTATEZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomRotate[2])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_ROTATEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Rotate[0])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_ROTATEY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Rotate[1])) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_ROTATEZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Rotate[2])) == NULL)
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

ItemTag = WCS_FOLIAGE_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Color.Save(ffile))
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

if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_IMAGEID, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (Obj)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_FOLIAGE_OBJECTNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Obj->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Obj->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // Foliage::Save

/*===========================================================================*/

char *FoliageCritterNames[WCS_FOLIAGE_NUMANIMPAR] = {"Specimen Height (%)", "Specimen Density (%)", "Back Light (%)"};
char *FoliageTextureNames[WCS_FOLIAGE_NUMTEXTURES] = {"Specimen Height (%)", "Specimen Density (%)", "Gray Replacement Color", "Backlight %"};
char *FoliageThemeNames[WCS_FOLIAGE_NUMTHEMES] = {"Specimen Height (%)", "Specimen Density (%)", "Gray Replacement Color"};

char *Foliage::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (FoliageCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (FoliageTextureNames[Ct]);
		} // if
	} // for
if (Test == &Color)
	return ("Replacement Color");

return ("");

} // Foliage::GetCritterName

/*===========================================================================*/

char *Foliage::OKRemoveRaster(void)
{

return ("Image Object is used in a Foliage Object! Remove anyway?");

} // Foliage::OKRemoveRaster

/*===========================================================================*/

void Foliage::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img && Img == Shell)
	{
	Img = NULL;
	#ifdef WCS_FORESTRY_WIZARD
	Rast = NULL;
	#endif // WCS_FORESTRY_WIZARD
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // Foliage::RemoveRaster

/*===========================================================================*/

int Foliage::SetRaster(Raster *NewRast)
{
NotifyTag Changes[2];

#ifdef WCS_FORESTRY_WIZARD
Rast = NULL;
#endif // WCS_FORESTRY_WIZARD

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	} // if
if (! Img)
	{
	Img = new FoliageShell;
	} // else
if (Img && NewRast)
	{
	NewRast->AddAttribute(Img->GetType(), Img, this);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_RAHOST_OBJTYPE_RASTER, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
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

} // Foliage::SetRaster

/*===========================================================================*/

int Foliage::SetObject(Object3DEffect *NewObj)
{
NotifyTag Changes[2];

Obj = NewObj;
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_EFFECTSSUBCLASS_OBJECT3D, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // Foliage::SetObject

/*===========================================================================*/

int Foliage::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // Foliage::GetRAHostAnimated

/*===========================================================================*/

bool Foliage::AnimateMaterials(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (true);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for

return (false);

} // Foliage::AnimateMaterials

/*===========================================================================*/

long Foliage::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // while
if (Color.GetMinMaxDist(MinDist, MaxDist))
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

} // Foliage::GetKeyFrameRange

/*===========================================================================*/

char Foliage::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER)
	return (1);

return (0);

} // Foliage::GetRAHostDropOK

/*===========================================================================*/

int Foliage::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Foliage *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Foliage *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = Color.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = SetRaster((Raster *)DropSource->DropSource);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_OBJECT3D)
	{
	Success = SetObject((Object3DEffect *)DropSource->DropSource);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // Foliage::ProcessRAHostDragDrop

/*===========================================================================*/

char *Foliage::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? FoliageTextureNames[TexNumber]: (char*)"");

} // Foliage::GetTextureName

/*===========================================================================*/

void Foliage::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = (Test == GetTexRootPtr(WCS_FOLIAGE_TEXTURE_COLOR));
ApplyToDisplace = 0;

} // Foliage::GetTextureApplication

/*===========================================================================*/

RootTexture *Foliage::NewRootTexture(long TexNumber)
{
char ApplyToColor = (TexNumber == WCS_FOLIAGE_TEXTURE_COLOR);
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // Foliage::NewRootTexture

/*===========================================================================*/

int Foliage::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;
unsigned long ItemRemoved;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		ItemRemoved = WCS_SUBCLASS_ROOTTEXTURE;
		} // if
	} // for

if (! Removed)
	{
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (RemoveMe == GetTheme(Ct))
			{
			SetThemePtr(Ct, NULL);
			Removed = 1;
			ItemRemoved = WCS_EFFECTSSUBCLASS_THEMATICMAP;
			} // if
		} // for
	} // if

if (Obj == (Object3DEffect *)RemoveMe)
	{
	Obj = NULL;
	Removed = 1;
	ItemRemoved = WCS_EFFECTSSUBCLASS_OBJECT3D;
	} // if

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	#ifdef WCS_FORESTRY_WIZARD
	Rast = NULL;
	#endif // WCS_FORESTRY_WIZARD
	Removed = -1;
	ItemRemoved = WCS_SUBCLASS_RASTER;
	} // if

if (Removed > 0)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), ItemRemoved, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (abs(Removed));

} // Foliage::RemoveRAHost

/*===========================================================================*/

int Foliage::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{
NotifyTag Changes[2];

if (Obj == RemoveMe)
	{
	if (RemoveRAHostQuery(RemoveMe))
		{
		Obj = NULL;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_EFFECTSSUBCLASS_OBJECT3D, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	else
		return (0);
	} // if

return (1);

} // Foliage::FindnRemove3DObjects

/*===========================================================================*/

ThematicMap *Foliage::GetEnabledTheme(long ThemeNum)
{

return (ThemeNum < WCS_FOLIAGE_NUMTHEMES && Theme[ThemeNum] && Theme[ThemeNum]->Enabled ? Theme[ThemeNum]: NULL);

} // Foliage::GetEnabledTheme

/*===========================================================================*/

int Foliage::SetTheme(long ThemeNum, ThematicMap *NewTheme)
{
NotifyTag Changes[2];

if (ThemeNum < GetNumThemes())
	{
	SetThemePtr(ThemeNum, NewTheme);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), WCS_EFFECTSSUBCLASS_THEMATICMAP, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if

return (0);

} // Foliage::SetTheme

/*===========================================================================*/

char *Foliage::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? FoliageThemeNames[ThemeNum]: (char*)"");

} // Foliage::GetThemeName

/*===========================================================================*/

unsigned long Foliage::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_FOLIAGE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Foliage::GetRAFlags

/*===========================================================================*/

void Foliage::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = (Img && Img->GetRaster()) ? Img->GetRaster()->GetUserName(): Obj ? Obj->GetName(): (char*)"";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = NULL;
	Prop->Ext = NULL;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // Foliage::GetRAHostProperties

/*===========================================================================*/

int Foliage::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return (-2);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return (-2);
	} // if

return (Success);

} // Foliage::SetRAHostProperties

/*===========================================================================*/

RasterAnimHost *Foliage::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&Color);
if (Current == &Color)
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
if (Found && Obj)
	return (Obj);
if (Current == Obj)
	Found = 1;
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());

return (NULL);

} // Foliage::GetRAHostChild

/*===========================================================================*/

int Foliage::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for

return (0);

} // Foliage::GetDeletable

/*===========================================================================*/

int Foliage::GetThemeType(long ThemeNum)
{
// return 0 for area and 1 to make it point type
RasterAnimHost *Root;
RasterAnimHostProperties Prop;

if (Root = GetRAHostRoot())
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
	Root->GetRAHostProperties(&Prop);
	if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE)
		{
		return (1);
		} // if
	} // if

return (0);

} // Foliage::GetThemeType

/*===========================================================================*/

int Foliage::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_FOLIAGE_ANIMPAR_HEIGHT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_HEIGHT);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGE_ANIMPAR_DENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_DENSITY);
					break;
					} // 
				case WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING:
					{
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	if (ChildA == &Color)
		{
		AnimAffil = &Color;
		TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_COLOR);
		ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_COLOR);
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
				case WCS_FOLIAGE_TEXTURE_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGE_ANIMPAR_HEIGHT);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGE_TEXTURE_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGE_ANIMPAR_DENSITY);
					ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_DENSITY);
					break;
					} // 
				case WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING);
					break;
					} // 
				case WCS_FOLIAGE_TEXTURE_COLOR:
					{
					AnimAffil = &Color;
					ThemeAffil = GetThemeAddr(WCS_FOLIAGE_THEME_COLOR);
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
				case WCS_FOLIAGE_THEME_HEIGHT:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGE_ANIMPAR_HEIGHT);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_HEIGHT);
					break;
					} // 
				case WCS_FOLIAGE_THEME_DENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_FOLIAGE_ANIMPAR_DENSITY);
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_DENSITY);
					break;
					} // 
				case WCS_FOLIAGE_THEME_COLOR:
					{
					AnimAffil = &Color;
					TexAffil = GetTexRootPtrAddr(WCS_FOLIAGE_TEXTURE_COLOR);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // Foliage::GetAffiliates

/*===========================================================================*/

int Foliage::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil));
	} // if

return (0);

} // Foliage::GetPopClassFlags

/*===========================================================================*/

int Foliage::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil));
	} // if

return(0);

} // Foliage::AddSRAHBasePopMenus

/*===========================================================================*/

int Foliage::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, ThemeAffil, this, this));
	} // if

return(0);

} // Foliage::HandleSRAHPopMenuSelection

/*===========================================================================*/

long Foliage::InitImageIDs(long &ImageID)
{
long Ct, ImagesFound = 0;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	ImagesFound = 1;
	} // if
if (Obj)
	{
	ImagesFound += Obj->InitImageIDs(ImageID);
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		ImagesFound += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (ImagesFound);

} // Foliage::InitImageIDs

/*===========================================================================*/

int Foliage::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves,
	EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
EffectList **ListPtr;
long Ct;

if (Obj)
	{
	ListPtr = Object3Ds;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Obj)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Obj;
		else
			return (0);
		} // if
	if (! Obj->BuildFileComponentsList(Material3Ds, Object3Ds, Waves, Queries, Themes, Coords))
		return (0);
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for

return (ThematicOwner::BuildFileComponentsList(Themes));

} // Foliage::BuildFileComponentsList

/*===========================================================================*/

int Foliage::InitToRender(void)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // Foliage::InitToRender

/*===========================================================================*/

int Foliage::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetEnabledTexture(Ct))
		{
		if (! TexRoot[Ct]->InitFrameToRender(Lib, Rend))
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // Foliage::InitFrameToRender

/*===========================================================================*/
/*===========================================================================*/

FoliageLink::FoliageLink()
{

Fol = NULL;
Grp = NULL;
Rast = NULL;
Obj = NULL;
Next = NULL;
Enabled = ColorImage = PosShade = Shade3D = RenderOccluded = 0;
EcoHtTheme = GrpDensTheme = GrpHtTheme = FolDensTheme = FolHtTheme = FolColorTheme = NULL;
EcoDensTex = EcoHtTex = GrpDensTex = GrpHtTex = FolDensTex = FolHtTex = FolColorTex = FolBacklightTex = NULL;
Density = 0.0;
ImageWidthFactor = 1.0;

} // FoliageLink::FoliageLink
