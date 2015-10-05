// EffectMaterial.cpp
// For managing Material Effects
// Built from EffectMaterial.cpp on 03/5/98 by Gary R. Huber
// Copyright 1998 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "MaterialEditGUI.h"
#include "Project.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Texture.h"
#include "TextureEditGUI.h"
#include "Requester.h"
#include "Raster.h"
#include "Ecotype.h"
#include "Render.h"
#include "Toolbar.h"
#include "DragnDropListGUI.h"
#include "Database.h"
#include "Security.h"
#include "KeyScaleDeleteGUI.h"
#include "AppMem.h"
#include "Lists.h"
#include "FeatureConfig.h"

#ifndef min
#define   min(a,b)    ((a) <= (b) ? (a) : (b))
#endif

unsigned char RemapV4toV5MaterialTextureNumbers[WCS_EFFECTS_MATERIAL_V4NUMTEXTURES] = 
	{
	WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR,
	WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY,
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY,
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY,
	WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP,
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE,
	WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP,
	WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT
	}; // Remap V4 to V5 texture numbers

unsigned char MaterialEffect::ParamAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMANIMPAR] = 
	{
//	L, T, S,	E, P, S2,E2,P2,T, E, D,		B, R, W, F,		O, D
	1, 1, 1,	1, 1, 1, 1, 1, 1, 1, 0,		1, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_OBJECT3D,
	1, 1, 1,	1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM,
	1, 1, 1,	1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_GROUND,
	1, 1, 1,	1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_SNOW,
	1, 0, 1,	1, 0, 0, 0, 0, 1, 1, 1,		1, 1, 1, 1,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_WATER,
	1, 0, 1,	1, 0, 0, 0, 0, 1, 1, 0,		0, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_FOAM,
	1, 1, 1,	1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		0, 1,	// WCS_EFFECTS_MATERIALTYPE_BEACH,
	1, 1, 1,	1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		0, 1	// WCS_EFFECTS_MATERIALTYPE_FENCE,
	};

unsigned char MaterialEffect::TextureAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMTEXTURES] = 
	{
//	D, S, S2,L, T, S,		E, P, S2,E2,P2,T, E, D,		B, R, F, O,		D, B
	1, 1, 1, 1, 1, 1,		1, 1, 1, 1, 1, 1, 1, 0,		1, 1, 0, 0,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_OBJECT3D,
	1, 0, 0, 1, 1, 1,		1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM,
	1, 0, 0, 1, 1, 1,		1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_GROUND,
	1, 0, 0, 1, 1, 1,		1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_SNOW,
	1, 0, 0, 1, 0, 1,		1, 0, 0, 0, 0, 1, 1, 1,		1, 1, 1, 1,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_WATER,
	1, 0, 0, 1, 0, 1,		1, 0, 0, 0, 0, 1, 1, 0,		0, 1, 0, 0,		1, 0,	// WCS_EFFECTS_MATERIALTYPE_FOAM,
	1, 0, 0, 1, 1, 1,		1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		1, 1,	// WCS_EFFECTS_MATERIALTYPE_BEACH,
	1, 0, 0, 1, 1, 1,		1, 0, 0, 0, 0, 0, 0, 0,		1, 1, 0, 0,		1, 1	// WCS_EFFECTS_MATERIALTYPE_FENCE,
	};

unsigned char MaterialEffect::MiscEntityAllowed[WCS_EFFECTS_NUMMATERIALTYPES][WCS_EFFECTS_MATERIAL_NUMMISCENTITIES] = 
	{
	0, 0, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_OBJECT3D,
	1, 1, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM,
	0, 1, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_GROUND,
	0, 0, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_SNOW,
	0, 0, 1, 1,	// WCS_EFFECTS_MATERIALTYPE_WATER,
	0, 0, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_FOAM,
	1, 1, 0, 0,	// WCS_EFFECTS_MATERIALTYPE_BEACH,
	0, 1, 0, 0	// WCS_EFFECTS_MATERIALTYPE_FENCE,
	};

MaterialEffect::MaterialEffect(char NewMatType)
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_MATERIAL;
MaterialType = NewMatType;
SetDefaults();

} // MaterialEffect::MaterialEffect

/*===========================================================================*/

MaterialEffect::MaterialEffect(RasterAnimHost *RAHost, char NewMatType)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_MATERIAL;
MaterialType = NewMatType;
SetDefaults();

} // MaterialEffect::MaterialEffect

/*===========================================================================*/

MaterialEffect::MaterialEffect(RasterAnimHost *RAHost, EffectsLib *Library, MaterialEffect *Proto, char NewMatType)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_MATERIAL;
if (Library)
	{
	Prev = Library->LastMaterial;
	if (Library->LastMaterial)
		{
		Library->LastMaterial->Next = this;
		Library->LastMaterial = this;
		} // if
	else
		{
		Library->Material = Library->LastMaterial = this;
		} // else
	} // if
MaterialType = NewMatType;
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
	strcpy(NameBase, "Material");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // MaterialEffect::MaterialEffect

/*===========================================================================*/

MaterialEffect::~MaterialEffect()
{
long Ct;
EffectList *CurWave;
RootTexture *DelTex;
MaterialStrata *TempStrata;
MaterialEffect *TempFoam;
Ecotype *TempFol;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->MAG && GlobalApp->GUIWins->MAG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->MAG;
		GlobalApp->GUIWins->MAG = NULL;
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
for (Ct = 0; Ct < 2; Ct ++)
	{
	if (TempFol = EcoFol[Ct])
		{
		EcoFol[Ct] = NULL;
		delete TempFol;
		} // if
	} // for
if (TempStrata = Strata)
	{
	Strata = NULL;
	delete TempStrata;
	} // if
if (TempFoam = Foam)
	{
	Foam = NULL;
	delete TempFoam;
	} // if

while (Waves)
	{
	CurWave = Waves;
	Waves = Waves->Next;
	delete CurWave;
	} // while

} // MaterialEffect::~MaterialEffect

/*===========================================================================*/

void MaterialEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_MATERIAL_NUMANIMPAR] = {0.0, 0.0, 0.0, 5.0, 0.0, 0.0, 5.0, 0.0, 0.0, 1.0, 0.0, 1.0, 0.0, 0.0, .1, 1.0, 1.0};
double RangeDefaults[WCS_EFFECTS_MATERIAL_NUMANIMPAR][3] = {100.0, -100.0, .01,		// luminosity (%)
															1.0, 0.0, .01,			// transparency (%)
															100.0, 0.0, .01,		// specularity (%)
															200.0, 0.0, 1.0,		// specular exponent 
															1.0, 0.0, .01,			// specular color (%)
															100.0, 0.0, .01,		// specularity 2 (%)
															200.0, 0.0, 1.0,		// specular exponent 2
															1.0, 0.0, .01,			// specular color 2 (%)
															10.0, 0.0, .01,			// transluminance (%)
															200.0, 0.0, 1.0,		// translum exponent 
															1000.0, 0.0, .1,		// displacement (m)
															10.0, -10.0, .01,		// bump intensity (%)
															100.0, 0.0, .01,		// reflectivity (%)
															FLT_MAX, -FLT_MAX, .1,	// water depth (m)
															1.0, 0.0, .01,			// foam coverage (%)
															FLT_MAX, 0.0, 1.0,		// optical depth
															100.0, 0.0, .01};		// diffuse intensity
long Ct;

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
DiffuseColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR);
SpecularColor.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 1);
SpecularColor2.SetDefaults(this, WCS_EFFECTS_MATERIAL_NUMANIMPAR + 2);
if (MaterialType == WCS_EFFECTS_MATERIALTYPE_FOAM)
	{
	DiffuseColor.SetValue3(.9, .9, .9);
	SpecularColor.SetValue3(.9, .9, .9);
	SpecularColor2.SetValue3(.9, .9, .9);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].SetValue(.25);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetValue(.5);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].SetValue(2.0);
	} // if
else if (MaterialType == WCS_EFFECTS_MATERIALTYPE_WATER)
	{
	DiffuseColor.SetValue3(.15, .2, .3);
	SpecularColor.SetValue3(.15, .2, .3);
	SpecularColor2.SetValue3(.15, .2, .3);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].SetValue(1.0);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetValue(.5);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].SetValue(10.0);
	} // if
else if (MaterialType == WCS_EFFECTS_MATERIALTYPE_SNOW)
	{
	DiffuseColor.SetValue3(1.0, 1.0, 1.0);
	SpecularColor.SetValue3(1.0, 1.0, 1.0);
	SpecularColor2.SetValue3(1.0, 1.0, 1.0);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetValue(.5);
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].SetValue(2.0);
	} // if
else if (MaterialType == WCS_EFFECTS_MATERIALTYPE_BEACH)
	{
	// beach colors 227, 190, 133 from Chris
	DiffuseColor.SetValue3(227.0 / 255.0, 190.0 / 255.0, 133.0 / 255.0);
	SpecularColor.SetValue3(227.0 / 255.0, 190.0 / 255.0, 133.0 / 255.0);
	SpecularColor2.SetValue3(227.0 / 255.0, 190.0 / 255.0, 133.0 / 255.0);
	} // if
else
	{
	SpecularColor.SetValue3(DiffuseColor.CurValue[0], DiffuseColor.CurValue[1], DiffuseColor.CurValue[2]);
	SpecularColor2.SetValue3(DiffuseColor.CurValue[0], DiffuseColor.CurValue[1], DiffuseColor.CurValue[2]);
	} // else
Shading = WCS_EFFECT_MATERIAL_SHADING_FLAT;
DoubleSided = TRUE;
FlipNormal = FALSE;
SmoothAngle = 45.0;
CosSmoothAngle = cos(SmoothAngle * PiOver180);
EcoFol[0] = EcoFol[1] = NULL;
Strata = NULL;
Foam = NULL;
Waves = NULL;
PixelTexturesExist = TransparencyTexturesExist = 0;
MaxWaveAmp = MinWaveAmp = WaveAmpRange = OverstoryDissolve = UnderstoryDissolve = 0.0;
strcpy(Name, "Material");
ShortName[0] = 0;

AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY].SetMultiplier(100.0);

} // MaterialEffect::SetDefaults

/*===========================================================================*/

void MaterialEffect::Copy(MaterialEffect *CopyTo, MaterialEffect *CopyFrom)
{
long Ct, Result = -1;
EffectList *NextWave, **ToWave;
NotifyTag Changes[2];

for (Ct = 0; Ct < 2; Ct ++)
	{
	if (CopyTo->EcoFol[Ct])
		delete CopyTo->EcoFol[Ct];
	CopyTo->EcoFol[Ct] = NULL;
	if (CopyTo->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE) && 
		CopyFrom->EcoFol[Ct] && (CopyTo->EcoFol[Ct] = new Ecotype(CopyTo)))
		{
		CopyTo->EcoFol[Ct]->Copy(CopyTo->EcoFol[Ct], CopyFrom->EcoFol[Ct]);
		} // if
	} // for

if (CopyTo->Strata)
	delete CopyTo->Strata;
CopyTo->Strata = NULL;
if (CopyTo->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && CopyFrom->Strata)
	{
	if (CopyTo->Strata = new MaterialStrata(CopyTo))
		{
		CopyTo->Strata->Copy(CopyTo->Strata, CopyFrom->Strata);
		} // if
	} // if

if (CopyTo->Foam)
	delete CopyTo->Foam;
CopyTo->Foam = NULL;
if (CopyTo->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_FOAM) && CopyFrom->Foam)
	{
	if (CopyTo->Foam = new MaterialEffect(CopyTo, WCS_EFFECTS_MATERIALTYPE_FOAM))
		{
		CopyTo->Foam->Copy(CopyTo->Foam, CopyFrom->Foam);
		} // if
	} // if

while (CopyTo->Waves)
	{
	NextWave = CopyTo->Waves;
	CopyTo->Waves = CopyTo->Waves->Next;
	delete NextWave;
	} // if
if (CopyTo->ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_WAVES))
	{
	NextWave = CopyFrom->Waves;
	ToWave = &CopyTo->Waves;
	while (NextWave)
		{
		if (NextWave->Me)
			{
			if (*ToWave = new EffectList())
				{
				if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
					{
					(*ToWave)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextWave->Me);
					} // if no need to make another copy, its all in the family
				else
					{
					if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextWave->Me->EffectType, NextWave->Me->Name))
						{
						Result = UserMessageCustom("Copy Material", "How do you wish to resolve Wave name collisions?\n\nLink to existing Waves, replace existing Waves, or create new Waves?",
							"Link", "Create", "Overwrite", 0);
						} // if
					if (Result <= 0)
						{
						(*ToWave)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextWave->Me->EffectType, NULL, NextWave->Me);
						} // if create new
					else if (Result == 1)
						{
						(*ToWave)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextWave->Me);
						} // if link to existing
					else if ((*ToWave)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextWave->Me->EffectType, NextWave->Me->Name))
						{
						((WaveEffect *)(*ToWave)->Me)->Copy((WaveEffect *)(*ToWave)->Me, (WaveEffect *)NextWave->Me);
						Changes[0] = MAKE_ID((*ToWave)->Me->GetNotifyClass(), (*ToWave)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
						Changes[1] = NULL;
						GlobalApp->AppEx->GenerateNotify(Changes, (*ToWave)->Me);
						} // else if found and overwrite
					else
						{
						(*ToWave)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextWave->Me->EffectType, NULL, NextWave->Me);
						} // else
					} // else better copy or overwrite it since its important to get just the right wave
				if ((*ToWave)->Me)
					ToWave = &(*ToWave)->Next;
				else
					{
					delete *ToWave;
					*ToWave = NULL;
					} // if
				} // if
			} // if
		NextWave = NextWave->Next;
		} // while
	} // if

CopyTo->DiffuseColor.Copy(&CopyTo->DiffuseColor, &CopyFrom->DiffuseColor);
CopyTo->SpecularColor.Copy(&CopyTo->SpecularColor, &CopyFrom->SpecularColor);
CopyTo->SpecularColor2.Copy(&CopyTo->SpecularColor2, &CopyFrom->SpecularColor2);
CopyTo->Shading = CopyFrom->Shading;
CopyTo->DoubleSided = CopyFrom->DoubleSided;
CopyTo->FlipNormal = CopyFrom->FlipNormal;
CopyTo->SmoothAngle = CopyFrom->SmoothAngle;
CopyTo->CosSmoothAngle = CopyFrom->CosSmoothAngle;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

// textures may have been copied which are not appropriate to this texture type
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->GetTexRootPtr(Ct) && ! CopyTo->ApproveTexture(Ct))
		{
		delete CopyTo->GetTexRootPtr(Ct);		// removes links to images
		CopyTo->SetTexRootPtr(Ct, NULL);
		} // if
	} // for

} // MaterialEffect::Copy

/*===========================================================================*/

short MaterialEffect::AnimateShadow3D(void)
{
RootTexture *MyRoot;

//test transparency and translucency
if (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].GetNumNodes(0) > 1 || 
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].GetNumNodes(0) > 1 ||
	AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].GetNumNodes(0) > 1)
	return (1);
// test any transparency or translucency image objects which may have sequences, disolves or animated parameters
if (MyRoot = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
	{
	if (MyRoot->GetEnabled() && MyRoot->IsAnimated())
		return (1);
	} // if
if (MyRoot = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE))
	{
	if (MyRoot->GetEnabled() && MyRoot->IsAnimated())
		return (1);
	} // if
if (MyRoot = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP))
	{
	if (MyRoot->GetEnabled() && MyRoot->IsAnimated())
		return (1);
	} // if
	
return (0);

} // MaterialEffect::AnimateShadow3D

/*===========================================================================*/

short MaterialEffect::AnimateEcoShadows(void)
{
long Ct;

for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct] && EcoFol[Ct]->AnimateShadows())
		return (1);
	} // for
if (Strata && Strata->AnimateShadows())
	return (1);
if (Foam && Foam->AnimateEcoShadows())
	return (1);

return (0);

} // MaterialEffect::AnimateEcoShadows

/*===========================================================================*/

int MaterialEffect::GetRAHostAnimated(void)
{
long Ct;
EffectList *CurWave;

if (GeneralEffect::GetRAHostAnimated())
	return (1);
for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct] && EcoFol[Ct]->GetRAHostAnimated())
		return (1);
	} // for
for (CurWave = Waves; CurWave; CurWave = CurWave->Next)
	{
	if (CurWave->Me->GetRAHostAnimated())
		return (1);
	} // for
if (DiffuseColor.GetRAHostAnimated())
	return (1);
if (SpecularColor.GetRAHostAnimated())
	return (1);
if (SpecularColor2.GetRAHostAnimated())
	return (1);
if (Strata && Strata->GetRAHostAnimated())
	return (1);
if (Foam && Foam->GetRAHostAnimated())
	return (1);

return (0);

} // MaterialEffect::GetRAHostAnimated

/*===========================================================================*/

bool MaterialEffect::AnimateMaterials(void)
{
long Ct;
EffectList *CurWave;

if (GeneralEffect::AnimateMaterials())
	return (true);
for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct] && EcoFol[Ct]->AnimateMaterials())
		return (true);
	} // for
for (CurWave = Waves; CurWave; CurWave = CurWave->Next)
	{
	if (CurWave->Me->AnimateMaterials())
		return (true);
	} // for
if (Foam && Foam->AnimateMaterials())
	return (true);

return (false);

} // MaterialEffect::AnimateMaterials

/*===========================================================================*/

unsigned long MaterialEffect::GetRAFlags(unsigned long Mask)
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
	Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_MATERIAL | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // MaterialEffect::GetRAFlags

/*===========================================================================*/

int MaterialEffect::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;
MaterialStrata **StrataAffil = NULL;
MaterialEffect **FoamAffil = NULL;
Ecotype **EcoAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil, ThemeAffil, StrataAffil, FoamAffil, EcoAffil))
	{
	return (GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil, StrataAffil, FoamAffil, EcoAffil));
	} // if

return (0);

} // MaterialEffect::GetPopClassFlags

/*===========================================================================*/

int MaterialEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil, MaterialStrata **&StrataAffil, 
	MaterialEffect **&FoamAffil, Ecotype **&EcoAffil)
{

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;
StrataAffil = NULL;
FoamAffil = NULL;
EcoAffil = NULL;

if (ChildB)
	{
	if (ChildB == (RasterAnimHost **)&Strata)
		{
		StrataAffil = &Strata;
		return (1);
		} // if
	if (ChildB == (RasterAnimHost **)&Foam)
		{
		FoamAffil = &Foam;
		return (1);
		} // if
	if (ChildB == (RasterAnimHost **)&EcoFol[0])
		{
		EcoAffil = &EcoFol[0];
		return (1);
		} // if
	if (ChildB == (RasterAnimHost **)&EcoFol[1])
		{
		EcoAffil = &EcoFol[1];
		return (1);
		} // if
	} // else if

return (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil));

} // MaterialEffect::GetAffiliates

/*===========================================================================*/

int MaterialEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
				case WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	if (ChildA == &DiffuseColor)
		{
		AnimAffil = &DiffuseColor;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR);
		ThemeAffil = GetThemeAddr(WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR);
		return (1);
		} // if
	else if (ChildA == &SpecularColor)
		{
		AnimAffil = &SpecularColor;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR);
		return (1);
		} // if
	else if (ChildA == &SpecularColor2)
		{
		AnimAffil = &SpecularColor2;
		TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2);
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
				case WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR:
					{
					AnimAffil = &DiffuseColor;
					ThemeAffil = GetThemeAddr(WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR);
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR:
					{
					AnimAffil = &SpecularColor;
					break;
					} // 
				case WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2:
					{
					AnimAffil = &SpecularColor2;
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
				case WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR:
					{
					AnimAffil = &DiffuseColor;
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // MaterialEffect::GetAffiliates

/*===========================================================================*/

int MaterialEffect::GetPopClassFlags(RasterAnimHostProperties *Prop, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil, MaterialStrata **&StrataAffil, 
	MaterialEffect **&FoamAffil, Ecotype **&EcoAffil)
{

if (StrataAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_STRATA;
	if (*StrataAffil)
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_STRATA;
		if ((*StrataAffil)->Enabled)
			Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_STRATA;
		} // if
	} // if
if (FoamAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_FOAM;
	if (*FoamAffil)
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_FOAM;
		if ((*FoamAffil)->Enabled)
			Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_FOAM;
		} // if
	} // if
if (EcoAffil)
	{
	Prop->PopClassFlags |= WCS_RAH_POPMENU_CLASS_ECO;
	if (*EcoAffil)
		{
		Prop->PopExistsFlags |= WCS_RAH_POPMENU_CLASS_ECO;
		if ((*EcoAffil)->Enabled)
			Prop->PopEnabledFlags |= WCS_RAH_POPMENU_CLASS_ECO;
		} // if
	} // if

RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, ThemeAffil);

return (Prop->PopClassFlags ? 1: 0);

} // MaterialEffect::GetPopClassFlags

/*===========================================================================*/

int MaterialEffect::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;
MaterialStrata **StrataAffil = NULL;
MaterialEffect **FoamAffil = NULL;
Ecotype **EcoAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil, StrataAffil, FoamAffil, EcoAffil))
	{
	return (AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil, StrataAffil, FoamAffil, EcoAffil));
	} // if

return(0);

} // MaterialEffect::AddSRAHBasePopMenus

/*===========================================================================*/

int MaterialEffect::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
	RasterAnimHost **ChildB, AnimCritter *&AnimAffil, RootTexture **&TexAffil, ThematicMap **&ThemeAffil, 
	MaterialStrata **&StrataAffil, MaterialEffect **&FoamAffil, Ecotype **&EcoAffil)
{
int MenuAdded = 0, PasteStrataOK = 0, PasteFoamOK = 0, PasteEcoOK = 0;
double CurTime;
RasterAnimHostProperties Prop;

if (StrataAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_STRATA)
	{ // strata related
	if (RasterAnimHost::GetCopyOfRAHost())
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_MATERIALSTRATA)
			PasteStrataOK = 1;
		} // if
	if (! *StrataAffil)
		{ //no texture
		if (PasteStrataOK)
			PMA->AddPopMenuItem("Paste Strata", "STRATA_PASTE", 1, 1);
		PMA->AddPopMenuItem("Create Strata", "STRATA_CREATE", 1, 1);
		} // if
	else
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_KEYRANGE;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
		(*StrataAffil)->GetRAHostProperties(&Prop);
		PMA->AddPopMenuItem("Edit Strata", "STRATA_EDIT", 1, 1);
		PMA->AddPopMenuItem("Copy Strata", "STRATA_COPY", 1, 1);
		if (PasteStrataOK)
			PMA->AddPopMenuItem("Paste Strata", "STRATA_PASTE", 1, 1);
		PMA->AddPopMenuItem("Delete Strata", "STRATA_DELETE", 1, 1);
		if ((*StrataAffil)->Enabled)
			PMA->AddPopMenuItem("Disable Strata", "STRATA_DISABLE", 1, 1);
		else
			PMA->AddPopMenuItem("Enable Strata", "STRATA_ENABLE", 1, 1);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			{
			PMA->AddPopMenuItem("Delete Strata Key(s)", "STRATA_DELETEKEY", 1, 1);
			PMA->AddPopMenuItem("Scale Strata Keyframe(s)", "STRATA_SCALEKEY", 1, 1);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			if (CurTime > Prop.KeyNodeRange[0])
				PMA->AddPopMenuItem("Previous Strata Key", "STRATA_PREVKEY", 1, 1);
			if (CurTime < Prop.KeyNodeRange[1])
				PMA->AddPopMenuItem("Next Strata Key", "STRATA_NEXTKEY", 1, 1);
			} // if
		PMA->AddPopMenuItem("Activate Strata", "STRATA_ACTIVATE", 1, 1);
		} // else
	MenuAdded = 1;
	} // if

if (FoamAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_FOAM)
	{ // Foam related
	if (RasterAnimHost::GetCopyOfRAHost())
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_MATERIAL && ((MaterialEffect *)RasterAnimHost::GetCopyOfRAHost())->MaterialType == WCS_EFFECTS_MATERIALTYPE_FOAM)
			PasteFoamOK = 1;
		} // if
	if (! *FoamAffil)
		{ //no texture
		if (PasteFoamOK)
			PMA->AddPopMenuItem("Paste Foam", "FOAM_PASTE", 1, 1);
		PMA->AddPopMenuItem("Create Foam", "FOAM_CREATE", 1, 1);
		} // if
	else
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_KEYRANGE;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
		(*FoamAffil)->GetRAHostProperties(&Prop);
		PMA->AddPopMenuItem("Copy Foam", "FOAM_COPY", 1, 1);
		if (PasteFoamOK)
			PMA->AddPopMenuItem("Paste Foam", "FOAM_PASTE", 1, 1);
		PMA->AddPopMenuItem("Delete Foam", "FOAM_DELETE", 1, 1);
		if ((*FoamAffil)->Enabled)
			PMA->AddPopMenuItem("Disable Foam", "FOAM_DISABLE", 1, 1);
		else
			PMA->AddPopMenuItem("Enable Foam", "FOAM_ENABLE", 1, 1);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			{
			PMA->AddPopMenuItem("Delete Foam Key(s)", "FOAM_DELETEKEY", 1, 1);
			PMA->AddPopMenuItem("Scale Foam Keyframe(s)", "FOAM_SCALEKEY", 1, 1);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			if (CurTime > Prop.KeyNodeRange[0])
				PMA->AddPopMenuItem("Previous Foam Key", "FOAM_PREVKEY", 1, 1);
			if (CurTime < Prop.KeyNodeRange[1])
				PMA->AddPopMenuItem("Next Foam Key", "FOAM_NEXTKEY", 1, 1);
			} // if
		PMA->AddPopMenuItem("Activate Foam", "FOAM_ACTIVATE", 1, 1);
		} // else
	MenuAdded = 1;
	} // if

if (EcoAffil && MenuClassFlags & WCS_RAH_POPMENU_CLASS_ECO)
	{ // Ecotype related
	if (RasterAnimHost::GetCopyOfRAHost())
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE)
			PasteEcoOK = 1;
		} // if
	if (! *EcoAffil)
		{ // no Ecotype
		if (PasteEcoOK)
			PMA->AddPopMenuItem("Paste Ecotype", "ECO_PASTE", 1, 1);
		PMA->AddPopMenuItem("Create Ecotype", "ECO_CREATE", 1, 1);
		} // if
	else
		{
		Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_KEYRANGE;
		Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_ANIMATED;
		(*EcoAffil)->GetRAHostProperties(&Prop);
		PMA->AddPopMenuItem("Edit Ecotype", "ECO_EDIT", 1, 1);
		PMA->AddPopMenuItem("Copy Ecotype", "ECO_COPY", 1, 1);
		if (PasteEcoOK)
			PMA->AddPopMenuItem("Paste Ecotype", "ECO_PASTE", 1, 1);
		PMA->AddPopMenuItem("Delete Ecotype", "ECO_DELETE", 1, 1);
		if ((*EcoAffil)->Enabled)
			PMA->AddPopMenuItem("Disable Ecotype", "ECO_DISABLE", 1, 1);
		else
			PMA->AddPopMenuItem("Enable Ecotype", "ECO_ENABLE", 1, 1);
		if (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED)
			{
			PMA->AddPopMenuItem("Delete Ecotype Key(s)", "ECO_DELETEKEY", 1, 1);
			PMA->AddPopMenuItem("Scale Ecotype Keyframe(s)", "ECO_SCALEKEY", 1, 1);
			CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
			if (CurTime > Prop.KeyNodeRange[0])
				PMA->AddPopMenuItem("Previous Ecotype Key", "ECO_PREVKEY", 1, 1);
			if (CurTime < Prop.KeyNodeRange[1])
				PMA->AddPopMenuItem("Next Ecotype Key", "ECO_NEXTKEY", 1, 1);
			} // if
		PMA->AddPopMenuItem("Activate Ecotype", "ECO_ACTIVATE", 1, 1);
		} // else
	MenuAdded = 1;
	} // if

return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil) || MenuAdded);

} // MaterialEffect::AddSRAHBasePopMenus

/*===========================================================================*/

int MaterialEffect::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
int Handled = 0, EcoNumber, RemoveAll = 0;
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
ThematicMap **ThemeAffil = NULL;
MaterialStrata **StrataAffil = NULL;
MaterialEffect **FoamAffil = NULL;
Ecotype **EcoAffil = NULL;
RasterAnimHost *PasteHost;
RasterAnimHostProperties Prop;
char CopyMsg[256];
NotifyTag Changes[2];

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil, ThemeAffil, StrataAffil, FoamAffil, EcoAffil))
	{
	if (! strncmp((char *)Action, "STRATA", 6))
		{
		if (! strcmp((char *)Action, "STRATA_ACTIVATE") && StrataAffil && (*StrataAffil))
			{
			RasterAnimHost::SetActiveRAHost((RasterAnimHost *)(*StrataAffil), 1);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_CREATE") && StrataAffil && ! (*StrataAffil))
			{
			if (NewStrata())
				{
				GetStrataPtr()->EditRAHost();
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_EDIT") && StrataAffil && (*StrataAffil))
			{
			(*StrataAffil)->EditRAHost();
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_DELETE") && StrataAffil && (*StrataAffil))
			{
			FindnRemoveRAHostChild(*StrataAffil, RemoveAll);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_ENABLE") && StrataAffil && (*StrataAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
			(*StrataAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_DISABLE") && StrataAffil && (*StrataAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = 0;
			(*StrataAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_COPY") && StrataAffil && (*StrataAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			(*StrataAffil)->GetRAHostProperties(&Prop);
			if (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
				{
				RasterAnimHost::SetCopyOfRAHost(*StrataAffil);
				sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK("Copy", "Selected item cannot be copied.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_PASTE") && StrataAffil)
			{
			if (RasterAnimHost::GetCopyOfRAHost())
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
				RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
				if (*StrataAffil)
					PasteHost = (*StrataAffil);
				else
					PasteHost = this;
				PasteHost->GetRAHostProperties(&Prop);
				if (Prop.DropOK)
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
					// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
					//  eg. still in progress through a DragnDropListGUI
					PasteHost->SetRAHostProperties(&Prop);
					sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
					} // else
				} // if
			else
				{
				UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_DELETEKEY") && StrataAffil && (*StrataAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*StrataAffil), WCS_KEYOPERATION_DELETE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_SCALEKEY") && StrataAffil && (*StrataAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*StrataAffil), WCS_KEYOPERATION_SCALE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_PREVKEY") && StrataAffil && (*StrataAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*StrataAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
				Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
			if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "STRATA_NEXTKEY") && StrataAffil && (*StrataAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*StrataAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
				Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
			Handled = 1;
			} // if
		} // if strata
	else if (! strncmp((char *)Action, "FOAM", 4))
		{
		if (! strcmp((char *)Action, "FOAM_ACTIVATE") && StrataAffil && (*FoamAffil))
			{
			RasterAnimHost::SetActiveRAHost((RasterAnimHost *)(*FoamAffil), 1);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_CREATE") && FoamAffil && ! (*FoamAffil))
			{
			if (NewFoam())
				{
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_DELETE") && FoamAffil && (*FoamAffil))
			{
			FindnRemoveRAHostChild(*FoamAffil, RemoveAll);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_ENABLE") && FoamAffil && (*FoamAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
			(*FoamAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_DISABLE") && FoamAffil && (*FoamAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = 0;
			(*FoamAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_COPY") && FoamAffil && (*FoamAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			(*FoamAffil)->GetRAHostProperties(&Prop);
			if (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
				{
				RasterAnimHost::SetCopyOfRAHost(*FoamAffil);
				sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK("Copy", "Selected item cannot be copied.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_PASTE") && FoamAffil)
			{
			if (RasterAnimHost::GetCopyOfRAHost())
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
				RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
				if (*FoamAffil)
					PasteHost = (*FoamAffil);
				else
					PasteHost = this;
				PasteHost->GetRAHostProperties(&Prop);
				if (Prop.DropOK)
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
					// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
					//  eg. still in progress through a DragnDropListGUI
					PasteHost->SetRAHostProperties(&Prop);
					sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
					} // else
				} // if
			else
				{
				UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_DELETEKEY") && FoamAffil && (*FoamAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*FoamAffil), WCS_KEYOPERATION_DELETE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_SCALEKEY") && FoamAffil && (*FoamAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*FoamAffil), WCS_KEYOPERATION_SCALE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_PREVKEY") && FoamAffil && (*FoamAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*FoamAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
				Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
			if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "FOAM_NEXTKEY") && FoamAffil && (*FoamAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*FoamAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
				Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
			Handled = 1;
			} // if
		} // else if foam
	else if (! strncmp((char *)Action, "ECO", 3))
		{
		if (! strcmp((char *)Action, "ECO_ACTIVATE") && EcoAffil && (*EcoAffil))
			{
			RasterAnimHost::SetActiveRAHost((RasterAnimHost *)(*EcoAffil), 1);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_CREATE") && EcoAffil && ! (*EcoAffil))
			{
			EcoNumber = EcoAffil == &EcoFol[0] ? 0: 1;
			if (NewEcotype(EcoNumber))
				{
				GetEcotypePtr(EcoNumber)->EditRAHost();
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_EDIT") && EcoAffil && (*EcoAffil))
			{
			(*EcoAffil)->EditRAHost();
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_DELETE") && EcoAffil && (*EcoAffil))
			{
			FindnRemoveRAHostChild(*EcoAffil, RemoveAll);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_ENABLE") && EcoAffil && (*EcoAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = WCS_RAHOST_FLAGBIT_ENABLED;
			(*EcoAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_DISABLE") && EcoAffil && (*EcoAffil))
			{
			Prop.PropMask = (WCS_RAHOST_MASKBIT_FLAGS);
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ENABLED;
			Prop.Flags = 0;
			(*EcoAffil)->SetRAHostProperties(&Prop);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_COPY") && EcoAffil && (*EcoAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			(*EcoAffil)->GetRAHostProperties(&Prop);
			if (Prop.Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
				{
				RasterAnimHost::SetCopyOfRAHost(*EcoAffil);
				sprintf(CopyMsg, "%s %s copied to clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
				} // if
			else
				{
				UserMessageOK("Copy", "Selected item cannot be copied.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_PASTE") && EcoAffil)
			{
			if (RasterAnimHost::GetCopyOfRAHost())
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
				RasterAnimHost::GetCopyOfRAHost()->GetRAHostProperties(&Prop);
				Prop.PropMask = WCS_RAHOST_MASKBIT_DROPOK;
				if (*EcoAffil)
					PasteHost = (*EcoAffil);
				else
					PasteHost = this;
				PasteHost->GetRAHostProperties(&Prop);
				if (Prop.DropOK)
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
					Prop.DropSource = RasterAnimHost::GetCopyOfRAHost();
					// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
					//  eg. still in progress through a DragnDropListGUI
					PasteHost->SetRAHostProperties(&Prop);
					sprintf(CopyMsg, "%s %s pasted from clipboard", Prop.Name ? Prop.Name: "", Prop.Type ? Prop.Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(Prop.Name ? Prop.Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
					} // else
				} // if
			else
				{
				UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
				} // else
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_DELETEKEY") && EcoAffil && (*EcoAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*EcoAffil), WCS_KEYOPERATION_DELETE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_SCALEKEY") && EcoAffil && (*EcoAffil))
			{
			if (GlobalApp->GUIWins->DKG)
				{
				delete GlobalApp->GUIWins->DKG;
				GlobalApp->GUIWins->DKG = NULL;
				} // if
			if (GlobalApp->GUIWins->DKG = new KeyScaleDeleteGUI(GlobalApp->MainProj, GlobalApp->AppEffects, (*EcoAffil), WCS_KEYOPERATION_SCALE))
				{
				if (GlobalApp->GUIWins->DKG->ConstructError)
					{
					delete GlobalApp->GUIWins->DKG;
					GlobalApp->GUIWins->DKG = NULL;
					} // if
				} // if
			if (GlobalApp->GUIWins->DKG)
				{
				GlobalApp->GUIWins->DKG->Open(GlobalApp->MainProj);
				GlobalApp->GUIWins->DKG->SetModalInhibit(TRUE);
				} // if
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_PREVKEY") && EcoAffil && (*EcoAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*EcoAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[0] > Prop.KeyNodeRange[0] || Prop.NewKeyNodeRange[0] == -DBL_MAX)
				Prop.NewKeyNodeRange[0] = Prop.KeyNodeRange[0];
			if (Prop.NewKeyNodeRange[0] < Prop.KeyNodeRange[0])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[0]);
			Handled = 1;
			} // if
		else if (! strcmp((char *)Action, "ECO_NEXTKEY") && EcoAffil && (*EcoAffil))
			{
			Prop.PropMask = WCS_RAHOST_MASKBIT_NEXTKEY;
			Prop.KeyNodeRange[0] = Prop.KeyNodeRange[1] = GlobalApp->MainProj->Interactive->GetActiveTime();
			Prop.NewKeyNodeRange[0] = -DBL_MAX;
			Prop.NewKeyNodeRange[1] = DBL_MAX;
			Prop.ItemOperator = WCS_KEYOPERATION_CUROBJ;
			(*EcoAffil)->GetRAHostProperties(&Prop);
			if (Prop.NewKeyNodeRange[1] < Prop.KeyNodeRange[1] || Prop.NewKeyNodeRange[1] == DBL_MAX)
				Prop.NewKeyNodeRange[1] = Prop.KeyNodeRange[1];
			if (Prop.NewKeyNodeRange[1] > Prop.KeyNodeRange[1])
				GlobalApp->MainProj->Interactive->SetActiveTime(Prop.NewKeyNodeRange[1]);
			Handled = 1;
			} // if
		} // else if ecotype
	else
		Handled = GeneralEffect::HandleSRAHPopMenuSelection(Action, ChildA, ChildB);
	} // if

return (Handled);

} // MaterialEffect::HandleSRAHPopMenuSelection

/*===========================================================================*/

unsigned long MaterialEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1, TexEnabled = -1;
char MatchName[WCS_EFFECT_MAXNAMELENGTH];
EffectList **CurWave = &Waves;

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
						if (! Name[0])
							strcpy(Name, "Material");
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
					case WCS_EFFECTS_MATERIAL_LUMINOSITYx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_MATERIAL_LUMINOSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_LUMINOSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TRANSPARENCYx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TRANSPARENCY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSPARENCY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARITYx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULAREXP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].Load(ffile, Size, ByteFlip);
						if (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue > 100.0)
							AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].ScaleValues(100.0 / AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP].CurValue);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARITY2:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARITY2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULAREXP2:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULAREXP2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARCOLORPCT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARCOLORPCT2:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TRANSLUMINANCEx100:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].Load(ffile, Size, ByteFlip);
						AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].ScaleValues(.01);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TRANSLUMINANCE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMINANCE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TRANSLUMEXP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].Load(ffile, Size, ByteFlip);
						if (AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].CurValue > 100.0)
							AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].ScaleValues(100.0 / AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_TRANSLUMEXP].CurValue);
						break;
						}
					case WCS_EFFECTS_MATERIAL_DISPLACEMENT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_REFLECTIVITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_REFLECTIVITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_BUMPINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_BUMPINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_FOAMCOVERAGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_FOAMCOVERAGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_OPTICALDEPTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_OPTICALDEPTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_WATERDEPTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_DIFFUSEINTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DIFFUSEINTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_DIFFUSECOLOR:
						{
						BytesRead = DiffuseColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARCOLOR:
						{
						BytesRead = SpecularColor.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SPECULARCOLOR2:
						{
						BytesRead = SpecularColor2.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_V4SHADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Shading, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						// in v3 & v4 Gouraud was Shading = 2. Gouraud was removed in v5 making Phong = 2.
						if (Shading >= WCS_EFFECT_MATERIAL_SHADING_PHONG)
							Shading = WCS_EFFECT_MATERIAL_SHADING_PHONG;
						break;
						}
					case WCS_EFFECTS_MATERIAL_SHADING:
						{
						BytesRead = ReadBlock(ffile, (char *)&Shading, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_DOUBLESIDED:
						{
						BytesRead = ReadBlock(ffile, (char *)&DoubleSided, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_FLIPNORMAL:
						{
						BytesRead = ReadBlock(ffile, (char *)&FlipNormal, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_SMOOTHANGLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&SmoothAngle, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_COSSMOOTHANGLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CosSmoothAngle, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < WCS_EFFECTS_MATERIAL_V4NUMTEXTURES && ApproveTexture(RemapV4toV5MaterialTextureNumbers[TexRootNumber]))
							{
							TexRootNumber = RemapV4toV5MaterialTextureNumbers[TexRootNumber];
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						else
							TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXENABLED:
						{
						if (TexRootNumber >= 0 && TexRoot[TexRootNumber])
							BytesRead = ReadBlock(ffile, (char *)&TexEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRoot[TexRootNumber])
							{
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
							if (TexEnabled >= 0)
								TexRoot[TexRootNumber]->Enabled = TexEnabled;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						TexEnabled = -1;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXDIFFUSECOLOR:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARCOLOR:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARCOLOR2:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXLUMINOSITY:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXTRANSPARENCY:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARITY:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULAREXP:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARITY2:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARITY2]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULAREXP2:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULAREXP2]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARCOLORPCT:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXSPECULARCOLORPCT2:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLORPCT2]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXTRANSLUMINANCE:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMINANCE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXTRANSLUMEXP:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_TRANSLUMEXP]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXDISPLACEMENT:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXBUMPINTENSITY:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMPINTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXBUMP:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_BUMP) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_BUMP]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXREFLECTIVITY:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_REFLECTIVITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXFOAMCOVERAGE:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_FOAMCOVERAGE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXOPTICALDEPTH:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_OPTICALDEPTH]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_TEXDIFFUSEINTENS:
						{
						if (ApproveTexture(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY) && (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY] = new RootTexture(this, 0, 0, 0)))
							{
							BytesRead = TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSEINTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#ifdef WCS_THEMATIC_MAP
					case WCS_EFFECTS_MATERIAL_THEMEDIFFUSECOLOR:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (MatchName[0])
							{
							Theme[WCS_EFFECTS_MATERIAL_THEME_DIFFUSECOLOR] = (ThematicMap *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_THEMATICMAP, MatchName);
							} // if
						break;
						}
					#endif // WCS_THEMATIC_MAP
					case WCS_EFFECTS_MATERIAL_ECOTYPEOVER:
						{
						if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE) && (EcoFol[0] = new Ecotype(this)))
							BytesRead = EcoFol[0]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_ECOTYPEUNDER:
						{
						if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE) && (EcoFol[1] = new Ecotype(this)))
							BytesRead = EcoFol[1]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_FOAM:
						{
						if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_FOAM) && (Foam = new MaterialEffect(this, WCS_EFFECTS_MATERIALTYPE_FOAM)))
							BytesRead = Foam->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_STRATA:
						{
						if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && (Strata = new MaterialStrata(this)))
							BytesRead = Strata->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_MATERIAL_WAVENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)MatchName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_WAVES) && MatchName[0])
							{
							if (*CurWave = new EffectList())
								{
								if ((*CurWave)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_WAVE, MatchName))
									CurWave = &(*CurWave)->Next;
								else
									{
									delete *CurWave;
									*CurWave = NULL;
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

return (TotalRead);

} // MaterialEffect::Load

/*===========================================================================*/

unsigned long MaterialEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char Ct;
unsigned long AnimItemTag[WCS_EFFECTS_MATERIAL_NUMANIMPAR] = {WCS_EFFECTS_MATERIAL_LUMINOSITY,
																	WCS_EFFECTS_MATERIAL_TRANSPARENCY,
																	WCS_EFFECTS_MATERIAL_SPECULARITY,
																	WCS_EFFECTS_MATERIAL_SPECULAREXP,
																	WCS_EFFECTS_MATERIAL_SPECULARCOLORPCT,
																	WCS_EFFECTS_MATERIAL_SPECULARITY2,
																	WCS_EFFECTS_MATERIAL_SPECULAREXP2,
																	WCS_EFFECTS_MATERIAL_SPECULARCOLORPCT2,
																	WCS_EFFECTS_MATERIAL_TRANSLUMINANCE,
																	WCS_EFFECTS_MATERIAL_TRANSLUMEXP,
																	WCS_EFFECTS_MATERIAL_DISPLACEMENT,
																	WCS_EFFECTS_MATERIAL_BUMPINTENSITY,
																	WCS_EFFECTS_MATERIAL_REFLECTIVITY,
																	WCS_EFFECTS_MATERIAL_WATERDEPTH,
																	WCS_EFFECTS_MATERIAL_FOAMCOVERAGE,
																	WCS_EFFECTS_MATERIAL_OPTICALDEPTH,
																	WCS_EFFECTS_MATERIAL_DIFFUSEINTENSITY,
																	};
unsigned long TextureItemTag[WCS_EFFECTS_MATERIAL_NUMTEXTURES] = {WCS_EFFECTS_MATERIAL_TEXDIFFUSECOLOR,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARCOLOR,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARCOLOR2,
																	WCS_EFFECTS_MATERIAL_TEXLUMINOSITY,
																	WCS_EFFECTS_MATERIAL_TEXTRANSPARENCY,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARITY,
																	WCS_EFFECTS_MATERIAL_TEXSPECULAREXP,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARCOLORPCT,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARITY2,
																	WCS_EFFECTS_MATERIAL_TEXSPECULAREXP2,
																	WCS_EFFECTS_MATERIAL_TEXSPECULARCOLORPCT2,
																	WCS_EFFECTS_MATERIAL_TEXTRANSLUMINANCE,
																	WCS_EFFECTS_MATERIAL_TEXTRANSLUMEXP,
																	WCS_EFFECTS_MATERIAL_TEXDISPLACEMENT,
																	WCS_EFFECTS_MATERIAL_TEXBUMPINTENSITY,
																	WCS_EFFECTS_MATERIAL_TEXREFLECTIVITY,
																	WCS_EFFECTS_MATERIAL_TEXFOAMCOVERAGE,
																	WCS_EFFECTS_MATERIAL_TEXOPTICALDEPTH,
																	WCS_EFFECTS_MATERIAL_TEXDIFFUSEINTENS,
																	WCS_EFFECTS_MATERIAL_TEXBUMP};
unsigned long ThemeItemTag[WCS_EFFECTS_MATERIAL_NUMTHEMES] = {WCS_EFFECTS_MATERIAL_THEMEDIFFUSECOLOR};
EffectList *CurWave;

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

ItemTag = WCS_EFFECTS_MATERIAL_DIFFUSECOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = DiffuseColor.Save(ffile))
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

ItemTag = WCS_EFFECTS_MATERIAL_SPECULARCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SpecularColor.Save(ffile))
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

ItemTag = WCS_EFFECTS_MATERIAL_SPECULARCOLOR2 + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SpecularColor2.Save(ffile))
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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_SHADING, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Shading)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_DOUBLESIDED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&DoubleSided)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_FLIPNORMAL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&FlipNormal)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_SMOOTHANGLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SmoothAngle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_COSSMOOTHANGLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&CosSmoothAngle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

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
					} // if anim param saved
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

for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct])
		{
		if (Ct == 0)
			ItemTag = WCS_EFFECTS_MATERIAL_ECOTYPEOVER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		else
			ItemTag = WCS_EFFECTS_MATERIAL_ECOTYPEUNDER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;

		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = EcoFol[Ct]->Save(ffile))
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
					} /* if ecotype saved */
				else
					goto WriteError;
				} /* if size written */
			else
				goto WriteError;
			} /* if tag written */
		else
			goto WriteError;
		} // if
	} // for

if (Strata)
	{
	ItemTag = WCS_EFFECTS_MATERIAL_STRATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Strata->Save(ffile))
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
				} /* if strata saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // if

if (Foam)
	{
	ItemTag = WCS_EFFECTS_MATERIAL_FOAM + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Foam->Save(ffile))
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
				} /* if strata saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // if

CurWave = Waves;
while (CurWave)
	{
	if (CurWave->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_MATERIAL_WAVENAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurWave->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurWave->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurWave = CurWave->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // MaterialEffect::Save

/*===========================================================================*/

void MaterialEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->MAG)
	{
	delete GlobalApp->GUIWins->MAG;
	}
GlobalApp->GUIWins->MAG = new MaterialEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->MAG)
	{
	GlobalApp->GUIWins->MAG->Open(GlobalApp->MainProj);
	}

} // MaterialEffect::Edit

/*===========================================================================*/

char *MaterialEffectCritterNames[WCS_EFFECTS_MATERIAL_NUMANIMPAR] = {"Luminosity (%)", "Transparency (%)", "Specularity (%)", "Specular Exponent", "Alt. Specular Color (%)", "2nd Specularity (%)", "2nd Specular Exponent", "2nd Alt. Specular Color (%)", 
	"Translucency (%)", "Translucency Exponent", "Roughness (m)", "Bump Intensity (%)", "Reflectivity (%)", "Water Depth (m)", "Foam Coverage (%)",
	"Optical Depth (m)", "Diffuse Intensity (%)"};
char *MaterialEffectTextureNames[WCS_EFFECTS_MATERIAL_NUMTEXTURES] = {"Diffuse Color", "Specular Color", "2nd Specular Color", "Luminosity (%)", "Transparency (%)", "Specularity (%)",
	"Specular Exponent", "Alt. Specular Color (%)", "2nd Specularity (%)", "2nd Specular Exponent", "2nd Alt. Specular Color (%)", "Translucency (%)", "Translucency Exponent", "Roughness (%)", "Bump Intensity (%)", "Reflectivity (%)",
	"Foam Coverage (%)", "Optical Depth (%)", "Diffuse Intensity (%)", "Bump Map"};
char *MaterialEffectThemeNames[WCS_EFFECTS_MATERIAL_NUMTHEMES] = {"Diffuse Color"};

char *MaterialEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (MaterialEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (MaterialEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &DiffuseColor)
	return ("Diffuse Color");
if (Test == &SpecularColor)
	return ("Specular Color");
if (Test == &SpecularColor2)
	return ("2nd Specular Color");
if (Test == EcoFol[0])
	return ("Overstory");
if (Test == EcoFol[1])
	return ("Understory");
if (Test == Strata)
	return ("Strata");
if (Test == Foam)
	return ("Foam");

return ("");

} // MaterialEffect::GetCritterName

/*===========================================================================*/

char *MaterialEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Material Texture! Remove anyway?");

} // MaterialEffect::OKRemoveRaster

/*===========================================================================*/

int MaterialEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurWave = Waves, *PrevWave = NULL;
NotifyTag Changes[2];
int Removed = 0;

if (RemoveMe == EcoFol[0])
	{
	delete EcoFol[0];
	EcoFol[0] = NULL;
	Removed = 1;
	}
if (RemoveMe == EcoFol[1])
	{
	delete EcoFol[1];
	EcoFol[1] = NULL;
	Removed = 1;
	}
if (RemoveMe == Strata)
	{
	delete Strata;
	Strata = NULL;
	Removed = 1;
	}
if (RemoveMe == Foam)
	{
	delete Foam;
	Foam = NULL;
	Removed = 1;
	}

if (! Removed)
	{
	while (CurWave)
		{
		if (CurWave->Me == (GeneralEffect *)RemoveMe)
			{
			if (PrevWave)
				PrevWave->Next = CurWave->Next;
			else
				Waves = CurWave->Next;

			delete CurWave;

			Removed = 1;
			break;
			} // if
		PrevWave = CurWave;
		CurWave = CurWave->Next;
		} // while
	} // if

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // MaterialEffect::RemoveRAHost

/*===========================================================================*/

int MaterialEffect::FindnRemove3DObjects(Object3DEffect *RemoveMe)
{

if (EcoFol[0] && ! EcoFol[0]->FindnRemove3DObjects(RemoveMe))
	return (0);
if (EcoFol[1] && ! EcoFol[1]->FindnRemove3DObjects(RemoveMe))
	return (0);

return (1);

} // MaterialEffect::FindnRemove3DObjects

/*===========================================================================*/

int MaterialEffect::SetToTime(double Time)
{
long Found = 0, Ct;

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
for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct] && EcoFol[Ct]->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (DiffuseColor.SetToTime(Time))
	Found = 1;
if (SpecularColor.SetToTime(Time))
	Found = 1;
if (SpecularColor2.SetToTime(Time))
	Found = 1;
if (Strata && Strata->SetToTime(Time))
	{
	Found = 1;
	} // if
if (Foam && Foam->SetToTime(Time))
	{
	Found = 1;
	} // if

return (Found);

} // MaterialEffect::SetToTime

/*===========================================================================*/

long MaterialEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (DiffuseColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (SpecularColor.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (SpecularColor2.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (EcoFol[0] && EcoFol[0]->GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (EcoFol[1] && EcoFol[1]->GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Strata && Strata->GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (Foam && Foam->GetKeyFrameRange(MinDist, MaxDist))
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

} // MaterialEffect::GetKeyFrameRange

/*===========================================================================*/

char MaterialEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	return (1);

if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_WAVES) && DropType == WCS_EFFECTSSUBCLASS_WAVE)
	return (1);
if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA) && DropType == WCS_RAHOST_OBJTYPE_MATERIALSTRATA)
	return (1);
if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE) && 
	(DropType == WCS_RAHOST_OBJTYPE_ECOTYPE
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropType == WCS_RAHOST_OBJTYPE_FOLIAGE))
	return (1);

return (0);

} // MaterialEffect::GetRAHostDropOK

/*===========================================================================*/

int MaterialEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128], OldName[WCS_EFFECT_MAXNAMELENGTH];
int QueryResult, Success = 0;
long Ct, ApprovedCt, NumListItems = 0;
EffectList **LoadTo = &Waves;
RasterAnimHost *TargetList[30];
long TargetListItems[30];
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (MaterialEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			strcpy(OldName, Name);
			Copy(this, (MaterialEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			if (GlobalApp->AppEffects->IsEffectValid(this, EffectType, 0))
				GlobalApp->AppEffects->MaterialNameChanging(OldName, Name);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Color");
	QueryResult = UserMessageCustom(NameStr, QueryStr, "Diffuse", "2nd Specular", "Specular", 0);
	if (QueryResult == 0)
		{
		Success = SpecularColor2.ProcessRAHostDragDrop(DropSource);
		} // if
	if (QueryResult == 1)
		{
		Success = DiffuseColor.ProcessRAHostDragDrop(DropSource);
		} // if
	else
		{
		Success = SpecularColor.ProcessRAHostDragDrop(DropSource);
		} // else
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ECOTYPE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGEGROUP
	|| DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_FOLIAGE)
	{
	if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE))
		{
		Success = -1;
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Ecotype");
		if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Overstory", "Cancel", "Understory", 0))
			{
			if (QueryResult == 1)
				{
				if (! EcoFol[0])
					{
					EcoFol[0] = new Ecotype(this);
					Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					} // if
				if (EcoFol[0])
					Success = EcoFol[0]->ProcessRAHostDragDrop(DropSource);
				} // if
			else if (QueryResult == 2)
				{
				if (! EcoFol[1])
					{
					EcoFol[1] = new Ecotype(this);
					Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
					} // if
				if (EcoFol[1])
					Success = EcoFol[1]->ProcessRAHostDragDrop(DropSource);
				} // else if
			} // if
		} // if
	else
		Success = 0;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_MATERIALSTRATA)
	{
	if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA))
		{
		Success = -1;
		if (! Strata)
			{
			Strata = new MaterialStrata(this);
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		if (Strata)
			Success = Strata->ProcessRAHostDragDrop(DropSource);
		} // if
	else
		Success = 0;
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_WAVE)
	{
	if (ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_WAVES))
		{
		Success = -1;
		sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			while (*LoadTo)
				{
				LoadTo = &(*LoadTo)->Next;
				} // if
			if (*LoadTo = new EffectList())
				{
				(*LoadTo)->Me = (GeneralEffect *)DropSource->DropSource;
				Success = 1;
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if
			} // if
		} // if
	else
		Success = 0;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = ApprovedCt = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ApproveTexture(Ct))
			{
			TargetListItems[ApprovedCt] = Ct;
			TargetList[ApprovedCt ++] = GetTexRootPtr(Ct);
			} // if
		} // for
	NumListItems = ApprovedCt;
	} // else if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL, TargetListItems);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // MaterialEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *MaterialEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? MaterialEffectTextureNames[TexNumber]: (char*)"");

} // MaterialEffect::GetTextureName

/*===========================================================================*/

void MaterialEffect::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = (Test == GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR) || Test == GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR)
	 || Test == GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2));
ApplyToDisplace = (Test == GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT));

} // MaterialEffect::GetTextureApplication

/*===========================================================================*/

RootTexture *MaterialEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = (TexNumber == WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR || TexNumber == WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR
	 || TexNumber == WCS_EFFECTS_MATERIAL_TEXTURE_SPECULARCOLOR2);
char ApplyToDisplace = (TexNumber == WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT);
char ApplyToEcosys = RAParent ? 1: 0;

if (TexNumber < GetNumTextures() && ApproveTexture(TexNumber))
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // MaterialEffect::NewRootTexture

/*===========================================================================*/

char *MaterialEffect::GetThemeName(long ThemeNum)
{

return (ThemeNum < GetNumThemes() ? MaterialEffectThemeNames[ThemeNum]: (char*)"");

} // MaterialEffect::GetThemeName

/*===========================================================================*/

Ecotype *MaterialEffect::NewEcotype(long EcoNumber)
{

if (EcoNumber < 2 && ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_ECOTYPE))
	return (EcoFol[EcoNumber] ? EcoFol[EcoNumber]:
		(EcoFol[EcoNumber] = new Ecotype(this)));

return (NULL);

} // MaterialEffect::NewEcotype

/*===========================================================================*/

MaterialStrata *MaterialEffect::NewStrata(void)
{

return ((! ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_STRATA)) ? NULL: Strata ? Strata:
	(Strata = new MaterialStrata(this)));

} // MaterialEffect::NewStrata

/*===========================================================================*/

MaterialEffect *MaterialEffect::NewFoam(void)
{

return ((! ApproveMiscEntity(WCS_EFFECTS_MATERIAL_MISCENTITY_FOAM)) ? NULL: Foam ? Foam:
	(Foam = new MaterialEffect(this, WCS_EFFECTS_MATERIALTYPE_FOAM)));

} // MaterialEffect::NewStrata

/*===========================================================================*/

int MaterialEffect::VertexColorsAvailable(void)
{
Object3DEffect *CurObj;

CurObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
while (CurObj)
	{
	if (CurObj->MatchMaterialName(Name) && CurObj->VertexColorsAvailable)
		return (1);
	CurObj = (Object3DEffect *)CurObj->Next;
	} // while

return (0);

} // MaterialEffect::VertexColorsAvailable

/*===========================================================================*/

int MaterialEffect::VertexUVWAvailable(void)
{
Object3DEffect *CurObj;

CurObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
while (CurObj)
	{
	if (CurObj->MatchMaterialName(Name) && CurObj->VertexUVWAvailable)
		return (1);
	CurObj = (Object3DEffect *)CurObj->Next;
	} // while

return (0);

} // MaterialEffect::VertexUVWAvailable

/*===========================================================================*/

char **MaterialEffect::GetVertexMapNames(int MapType)	// 0 = UVW, 1 = ColorPerVertex
{
Object3DEffect *CurObj;
long NumMaps = 0, Ct, MapCt;
char **MapNames = NULL;

// count maps
CurObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
while (CurObj)
	{
	if (MapType == 0)
		{
		if (CurObj->MatchMaterialName(Name) && CurObj->VertexUVWAvailable && CurObj->UVWTable)
			{
			NumMaps += CurObj->NumUVWMaps;
			} // if
		} // if
	else
		{
		if (CurObj->MatchMaterialName(Name) && CurObj->VertexColorsAvailable && CurObj->CPVTable)
			{
			NumMaps += CurObj->NumCPVMaps;
			} // if
		} // else
	CurObj = (Object3DEffect *)CurObj->Next;
	} // while

// allocate one extra set to NULL to signal end of list
if (NumMaps > 0 && (MapNames = (char **)AppMem_Alloc((NumMaps + 1) * sizeof (char *), APPMEM_CLEAR)))
	{
	// fill map names
	MapCt = 0;
	CurObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
	while (CurObj)
		{
		if (MapType == 0)
			{
			if (CurObj->MatchMaterialName(Name) && CurObj->VertexUVWAvailable && CurObj->UVWTable)
				{
				for (Ct = 0; Ct < CurObj->NumUVWMaps; Ct ++)
					MapNames[MapCt ++] = CurObj->UVWTable[Ct].Name;
				} // if
			} // if
		else
			{
			if (CurObj->MatchMaterialName(Name) && CurObj->VertexColorsAvailable && CurObj->CPVTable)
				{
				for (Ct = 0; Ct < CurObj->NumCPVMaps; Ct ++)
					MapNames[MapCt ++] = CurObj->CPVTable[Ct].Name;
				} // if
			} // else
		CurObj = (Object3DEffect *)CurObj->Next;
		} // while
	} // if

return (MapNames);

} // MaterialEffect::GetVertexMapNames

/*===========================================================================*/

int MaterialEffect::AreTexturesTileable(double &TileWidth, double &TileHeight, double &TileCenterX, double &TileCenterY)
{
double CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY;
int Tileable = 1, OneTileDone = 0;

TileWidth = TileHeight = 1.0;
TileCenterX = TileCenterY = 0.0;

// we are just going to worry about diffuse color textures unless that proves to be a problem
if (GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR))
	{
	if (Tileable = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_DIFFUSECOLOR)->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY))
		{
		TileCenterX = CurTileCenterX;
		TileCenterY = CurTileCenterY;
		TileWidth = CurTileWidth;
		TileHeight = CurTileHeight;
		OneTileDone = 1;
		} // if
	} // if
if (Tileable && GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY))
	{
	if (Tileable = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_LUMINOSITY)->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY))
		{
		if (OneTileDone)
			{
			if (fabs(CurTileCenterX - TileCenterX) <= .01 && fabs(CurTileCenterY - TileCenterY) <= .01)
				{
				// find common size
				if (fabs(CurTileWidth - TileWidth) > .01 || fabs(CurTileHeight - TileHeight) > .01)
					{
					TileWidth = WCS_lcm(TileWidth, CurTileWidth, .05);
					TileHeight = WCS_lcm(TileHeight, CurTileHeight, .05);
					} // if
				} // if
			else
				Tileable = 0;
			} // if
		else
			{
			TileCenterX = CurTileCenterX;
			TileCenterY = CurTileCenterY;
			TileWidth = CurTileWidth;
			TileHeight = CurTileHeight;
			OneTileDone = 1;
			} // else
		} // if
	} // if
if (Tileable && GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY))
	{
	if (Tileable = GetTexRootPtr(WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY)->IsTextureTileable(CurTileWidth, CurTileHeight, CurTileCenterX, CurTileCenterY))
		{
		if (OneTileDone)
			{
			if (fabs(CurTileCenterX - TileCenterX) <= .01 && fabs(CurTileCenterY - TileCenterY) <= .01)
				{
				// find common size
				if (fabs(CurTileWidth - TileWidth) > .01 || fabs(CurTileHeight - TileHeight) > .01)
					{
					TileWidth = WCS_lcm(TileWidth, CurTileWidth, .05);
					TileHeight = WCS_lcm(TileHeight, CurTileHeight, .05);
					} // if
				} // if
			else
				Tileable = 0;
			} // if
		else
			{
			TileCenterX = CurTileCenterX;
			TileCenterY = CurTileCenterY;
			TileWidth = CurTileWidth;
			TileHeight = CurTileHeight;
			OneTileDone = 1;
			} // else
		} // if
	} // if

return (Tileable);

} // MaterialEffect::AreTexturesTileable

/*===========================================================================*/

void MaterialEffect::GetWaterDepthAndWaveRange(double &MaximumMod, double &MinimumMod)
{
double Displacement;

MaximumMod = GetMaxWaveAmp(Displacement);

// MaximumMod includes displacement
// ActualWaveAmp = MaximumMod - Displacement;
MinimumMod = Displacement - (MaximumMod - Displacement);

// water depth is a constant at a given animation time
MaximumMod += AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue;
MinimumMod += AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_WATERDEPTH].CurValue;

} // MaterialEffect::GetWaterDepthAndWaveRange

/*===========================================================================*/

// this includes displacement
double MaterialEffect::GetMaxWaveAmp(double &Displacement)
{
double TempMaxWaveAmp = 0.0;
EffectList *CurWave = Waves;

while (CurWave)
	{
	if (CurWave->Me && CurWave->Me->Enabled)
		TempMaxWaveAmp += ((WaveEffect *)CurWave->Me)->GetMaxWaveAmp();
	CurWave = CurWave->Next;
	} // while

if (TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT] && TexRoot[WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT]->Enabled)
	{
	TempMaxWaveAmp += (Displacement = AnimPar[WCS_EFFECTS_MATERIAL_ANIMPAR_DISPLACEMENT].CurValue);
	} // if
else
	Displacement = 0.0;

return (TempMaxWaveAmp);

} // MaterialEffect::GetMaxWaveAmp

/*===========================================================================*/

// maximum is in array elements [0], minimum in [1]
int MaterialEffect::GetMaterialBoundsXYZ(double XRange[2], double YRange[2], double ZRange[2])
{
int Found = 0;
double HighX, LowX, HighY, LowY, HighZ, LowZ;
Object3DEffect *CurObj;

XRange[0] = YRange[0] = ZRange[0] = -FLT_MAX;
XRange[1] = YRange[1] = ZRange[1] = FLT_MAX;

CurObj = (Object3DEffect *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D);
while (CurObj)
	{
	if (CurObj->MatchMaterialName(Name))
		{
		CurObj->GetMaterialBoundsXYZ(Name, HighX, LowX, HighY, LowY, HighZ, LowZ);
		if (HighX > XRange[0])
			{
			XRange[0] = HighX;
			Found = 1;
			} // if
		if (LowX < XRange[1])
			XRange[1] = LowX;
		if (HighY > YRange[0])
			YRange[0] = HighY;
		if (LowY < YRange[1])
			YRange[1] = LowY;
		if (HighZ > ZRange[0])
			ZRange[0] = HighZ;
		if (LowZ < ZRange[1])
			ZRange[1] = LowZ;
		} // if
	CurObj = (Object3DEffect *)CurObj->Next;
	} // while

if (! Found)
	{
	XRange[0] = XRange[1] = 0.0;
	YRange[0] = YRange[1] = 0.0;
	ZRange[0] = ZRange[1] = 0.0;
	} // if

return (Found);

} // MaterialEffect::GetMaterialBoundsXYZ

/*===========================================================================*/

long MaterialEffect::InitImageIDs(long &ImageID)
{
char Ct;
long NumImages = 0;
EffectList *CurWave = Waves;

for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct])
		{
		NumImages += EcoFol[Ct]->InitImageIDs(ImageID);
		} // if
	} // for
if (Strata)
	NumImages += Strata->InitImageIDs(ImageID);
if (Foam)
	NumImages += Foam->InitImageIDs(ImageID);
while (CurWave)
	{
	if (CurWave->Me)
		NumImages += CurWave->Me->InitImageIDs(ImageID);
	CurWave = CurWave->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // MaterialEffect::InitImageIDs

/*===========================================================================*/

int MaterialEffect::BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **WaveList, EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
char Ct;
EffectList **ListPtr, *CurWave = Waves;

for (Ct = 0; Ct < 2; Ct ++)
	{
	if (EcoFol[Ct])
		{
		if (! EcoFol[Ct]->BuildFileComponentsList(Material3Ds, Object3Ds, WaveList, Queries, Themes, Coords))
			return (0);
		} // if
	} // for
while (CurWave)
	{
	if (CurWave->Me)
		{
		ListPtr = WaveList;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurWave->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurWave->Me;
			else
				return (0);
			} // if
		if (! CurWave->Me->BuildFileComponentsList(Queries, Themes, Coords))
			return (0);
		} // if
	CurWave = CurWave->Next;
	} // while
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for

return (GeneralEffect::BuildFileComponentsList(Queries, Themes, Coords));

} // MaterialEffect::BuildFileComponentsList

/*===========================================================================*/

int MaterialEffect::ConfigureTextureForEcosystem(void)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && ! GetTexRootPtr(Ct)->ConfigureForEcosystem())
		{
		return (0);
		} // if
	} // for

return (1);

} // MaterialEffect::ConfigureTextureForEcosystem

/*===========================================================================*/

int MaterialEffect::ConfigureTextureForObject3D(void)
{
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && ! GetTexRootPtr(Ct)->ConfigureForObject3D())
		{
		return (0);
		} // if
	} // for

return (1);

} // MaterialEffect::ConfigureTextureForObject3D

/*===========================================================================*/

RasterAnimHost *MaterialEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
EffectList *CurWave;
char Ct, Found = 0;

if (! Current)
	return (&DiffuseColor);
if (Current == &DiffuseColor)
	Found = 1;
if (Found && ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT))
	return (&SpecularColor);
if (Current == &SpecularColor)
	Found = 1;
if (Found && ApproveParam(WCS_EFFECTS_MATERIAL_ANIMPAR_SPECULARCOLORPCT2))
	return (&SpecularColor2);
if (Current == &SpecularColor2)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found && ApproveParam(Ct))
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct) && ApproveTexture(Ct))
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
for (Ct = 0; Ct < 2; Ct ++)
	{
	if (Found && EcoFol[Ct])
		return (EcoFol[Ct]);
	if (Current == EcoFol[Ct])
		Found = 1;
	} // for
if (Found && Strata)
	return (Strata);
if (Current == Strata)
	Found = 1;
if (Found && Foam)
	return (Foam);
if (Current == Foam)
	Found = 1;
CurWave = Waves;
while (CurWave)
	{
	if (Found)
		return (CurWave->Me);
	if (Current == CurWave->Me)
		Found = 1;
	CurWave = CurWave->Next;
	} // while

return (NULL);

} // MaterialEffect::GetRAHostChild

/*===========================================================================*/

int MaterialEffect::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
if (Test == EcoFol[0])
	return (1);
if (Test == EcoFol[1])
	return (1);
if (Test == Strata)
	return (1);
if (Test == Foam)
	return (1);

return (0);

} // MaterialEffect::GetDeletable

/*===========================================================================*/

int MaterialEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;

PixelTexturesExist = TransparencyTexturesExist = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		if (Ct != WCS_EFFECTS_MATERIAL_TEXTURE_DISPLACEMENT) 
			PixelTexturesExist = 1;
		if (Ct == WCS_EFFECTS_MATERIAL_TEXTURE_TRANSPARENCY) 
			TransparencyTexturesExist = 1;
		} // if
	} // for
if (EcoFol[0])
	{
	if (! EcoFol[0]->InitToRender())
		return (0);
	} // if
if (EcoFol[1])
	{
	if (! EcoFol[1]->InitToRender())
		return (0);
	} // if
if (Strata)
	{
	if (! Strata->InitToRender())
		return (0);
	PixelTexturesExist = 1;
	} // if
if (Foam)
	{
	if (! Foam->InitToRender(Opt, Buffers))
		return (0);
	if (Foam->PixelTexturesExist)
		PixelTexturesExist = 1;
	} // if
if (Waves)
	{
	PixelTexturesExist = 1;
	} // if

return (1);

} // MaterialEffect::InitToRender

/*===========================================================================*/

int MaterialEffect::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
double Displacement;

if (EcoFol[0])
	{
	if (EcoFol[0]->Enabled)
		{
		if (! EcoFol[0]->BuildFoliageChain(Rend) || ! EcoFol[0]->InitFrameToRender(Lib, Rend))
			return (0);
		} // if
	else
		{
		#ifdef WCS_USE_OLD_FOLIAGE
		EcoFol[0]->DeleteFoliageChain();
		#endif // WCS_USE_OLD_FOLIAGE
		} // else if
	} // if
if (EcoFol[1])
	{
	if (EcoFol[1]->Enabled)
		{
		if (! EcoFol[1]->BuildFoliageChain(Rend) || ! EcoFol[1]->InitFrameToRender(Lib, Rend))
			return (0);
		} // if
	else
		{
		#ifdef WCS_USE_OLD_FOLIAGE
		EcoFol[1]->DeleteFoliageChain();
		#endif // WCS_USE_OLD_FOLIAGE
		} // else if
	} // if

if (Strata)
	{
	if (! Strata->InitFrameToRender(Lib, Rend))
		return (0);
	} // if
if (Foam)
	{
	if (! Foam->InitFrameToRender(Lib, Rend))
		return (0);
	} // if

MaxWaveAmp = fabs(GetMaxWaveAmp(Displacement));
MinWaveAmp = - (MaxWaveAmp - Displacement);
WaveAmpRange = MaxWaveAmp - MinWaveAmp;
OverstoryDissolve = UnderstoryDissolve = 0.0;

return (GeneralEffect::InitFrameToRender(Lib, Rend));

} // MaterialEffect::InitFrameToRender

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int MaterialEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH], OldName[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
MaterialEffect *CurrentMaterial = NULL, *CurrentSubMaterial = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
Object3DEffect *CurrentObj = NULL;
WaveEffect *CurrentWave = NULL;
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
					else if (! strnicmp(ReadBuf, "SubMat3D", 8))
						{
						if (CurrentSubMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = CurrentSubMaterial->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentWave->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "Object3D", 8))
						{
						if (CurrentObj = new Object3DEffect(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentObj->Load(ffile, Size, ByteFlip);
							}
						} // if 3d object
					else if (! strnicmp(ReadBuf, "Matl3D", 8))
						{
						if (CurrentMaterial = new MaterialEffect(NULL, LoadToEffects, NULL, MaterialType))
							{
							if ((BytesRead = CurrentMaterial->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if 3d object
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

if (Success == 1 && CurrentMaterial)
	{
	strcpy(OldName, Name);
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentMaterial);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
	if (GlobalApp->AppEffects->IsEffectValid(this, EffectType, 0))
		GlobalApp->AppEffects->MaterialNameChanging(OldName, Name);
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

} // MaterialEffect::LoadObject

/*===========================================================================*/

int MaterialEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
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
			strcpy(StrBuf, "SubMat3D");
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

// MaterialEffect
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
			} // if MaterialEffect saved 
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

} // MaterialEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::MaterialEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
MaterialEffect *Current;

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
						if (Current = new MaterialEffect(NULL, this, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (MaterialEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::MaterialEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::MaterialEffect_Save(FILE *ffile)
{
MaterialEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Material;
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
					} // if material effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (MaterialEffect *)Current->Next;
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

} // EffectsLib::MaterialEffect_Save()
