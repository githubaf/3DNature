// Ecotype.h
// Header for WCS foliage
// Built from scratch on 04/10/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_FOLIAGE_H
#define WCS_FOLIAGE_H

class FoliageShellLink;
class FoliageShell;
//class Object3DEffect;
class RootTexture;
class PixelData;
class FoliageLink;
class FoliageChainList;
//class RenderData;
class ThematicMap;
class Ecotype;
class ForestryRenderData;
class FoliagePreviewData;
struct pEcotype;
struct pFoliageGroup;
struct pFoliage;

#include "GraphData.h"
#include "Texture.h"
#include "Random.h"
#include "ThematicOwner.h"

enum
	{
	WCS_FOLIAGE_TYPE_RASTER,
	WCS_FOLIAGE_TYPE_OBJECT3D
	}; // foliage texture types

enum
	{
	WCS_FOLIAGE_DENSITY_VARIABLE,
	WCS_FOLIAGE_DENSITY_CONSTANT
	}; // foliage density types

enum
	{
	WCS_FOLIAGE_DENSITY_HECTARE,
	WCS_FOLIAGE_DENSITY_ACRE,
	WCS_FOLIAGE_DENSITY_SQMETER,
	WCS_FOLIAGE_DENSITY_SQFOOT
	}; // foliage density units

enum
	{
	WCS_FOLIAGE_BASALAREAUNITS_SQMETER,
	WCS_FOLIAGE_BASALAREAUNITS_SQFOOT
	}; // basal area units

enum
	{
	WCS_FOLIAGE_DENSITYMETHOD_STEMSPERAREA,
	WCS_FOLIAGE_DENSITYMETHOD_BASALAREA,
	WCS_FOLIAGE_DENSITYMETHOD_CLOSURE
	}; // density methods

enum
	{
	WCS_FOLIAGE_SIZEMETHOD_HEIGHT,
	WCS_FOLIAGE_SIZEMETHOD_DBH,
	WCS_FOLIAGE_SIZEMETHOD_AGE,
	WCS_FOLIAGE_SIZEMETHOD_CLOSURE
	}; // size methods


// misc
#define	WCS_EFFECT_MAXNAMELENGTH	60

#define WCS_SEEDOFFSET_UNDERSTORY0		2145
#define WCS_SEEDOFFSET_UNDERSTORY1		8932
#define WCS_SEEDOFFSET_OVERSTORY0		6537
#define WCS_SEEDOFFSET_OVERSTORY1		3989
#define WCS_SEEDOFFSET_FOLIAGEEFFECT	1792
// notify subclass
enum
	{
	WCS_SUBCLASS_ECOTYPE = 160,
	WCS_SUBCLASS_FOLIAGEGRP,
	WCS_SUBCLASS_FOLIAGE
	}; // subclasses contained in effects that are not effects per se

enum
	{
	WCS_FOLIAGE_ANIMPAR_HEIGHT,
	WCS_FOLIAGE_ANIMPAR_DENSITY,
	WCS_FOLIAGE_ANIMPAR_ORIENTATIONSHADING,
	WCS_FOLIAGE_NUMANIMPAR
	}; //  foliage anim params

enum
	{
	WCS_FOLIAGE_TEXTURE_HEIGHT,
	WCS_FOLIAGE_TEXTURE_DENSITY,
	WCS_FOLIAGE_TEXTURE_COLOR,
	WCS_FOLIAGE_TEXTURE_ORIENTATIONSHADING,
	WCS_FOLIAGE_NUMTEXTURES
	}; //  foliage textures

enum
	{
	WCS_FOLIAGE_THEME_HEIGHT,
	WCS_FOLIAGE_THEME_DENSITY,
	WCS_FOLIAGE_THEME_COLOR,
	WCS_FOLIAGE_NUMTHEMES
	}; //  foliage themes

class Foliage : public RasterAnimHost, public RootTextureParent, public ThematicOwner
	{
	public:
		AnimDoubleTime AnimPar[WCS_FOLIAGE_NUMANIMPAR];
		AnimColorTime Color;
		Foliage *Next;
		FoliageShell *Img;
		Object3DEffect *Obj;
		ThematicMap *Theme[WCS_FOLIAGE_NUMTHEMES];
		RootTexture *TexRoot[WCS_FOLIAGE_NUMTEXTURES];	// for height pct., density pct., and color
		char Enabled, FoliageType, PosShade, Shade3D, RandomRotate[3], FlipX;
		double Rotate[3];

		#ifdef WCS_FORESTRY_WIZARD
		int ColorImage, RenderOccluded;
		Raster *Rast;
		double FoliagePercentage, RelativeFoliageDensity, RelativeFoliageSize, ImageWidthFactor;
		RootTexture *FoliageDensityTex, *FoliageSizeTex, *FoliageColorTex, *FoliageBacklightTex;
		ThematicMap *FoliageDensityTheme, *FoliageSizeTheme, *FoliageColorTheme;
		#endif // WCS_FORESTRY_WIZARD

		Foliage(RasterAnimHost *RAHost);
		~Foliage();
		void Copy(Foliage *CopyTo, Foliage *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int SetRaster(Raster *NewRast);
		int SetObject(Object3DEffect *NewObj);
		int AnimateShadows(void);
		long GetNumAnimParams(void) {return (WCS_FOLIAGE_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_FOLIAGE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		int GetRAHostAnimated(void);
		bool AnimateMaterials(void);
		virtual unsigned long GetRAFlags(unsigned long Mask = ~0);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_FOLIAGE);};
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB);
		int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		AnimColorTime *GetReplacementColor(void)	{return (&Color);};
		char GetShade3D(void)	{return (Shade3D);};

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_FOLIAGE_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_FOLIAGE_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum);
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_FOLIAGE_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual int SetTheme(long ThemeNum, ThematicMap *NewTheme);
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};
		virtual int GetThemeType(long ThemeNum);	// return 0 for area and 1 to make it point type

		// inherited from RootTextureParent
		virtual char *GetTextureName(long TexNumber);
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual long GetNumTextures(void) {return (WCS_FOLIAGE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_FOLIAGE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum) {return (TexNum < WCS_FOLIAGE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_FOLIAGE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_FOLIAGE);};
		virtual char *GetRAHostTypeString(void) {return ("(Foliage)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual void EditRAHost(void);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// in ParamConvert.cpp
		void TransferParamFoliageData(struct pFoliage *ParamFol);

		// in RenderForestry.cpp
		void ComputeFoliageRelativeDensity(ForestryRenderData *ForDat);
		void ComputeFoliageRelativeSize(ForestryRenderData *ForDat);

	}; // class Foliage

enum
	{
	WCS_FOLIAGEGRP_ANIMPAR_MAXHEIGHT,
	WCS_FOLIAGEGRP_ANIMPAR_MINHEIGHT,
	WCS_FOLIAGEGRP_ANIMPAR_DENSITY,
	WCS_FOLIAGEGRP_ANIMPAR_DBH,
	WCS_FOLIAGEGRP_ANIMPAR_BASALAREA,
	WCS_FOLIAGEGRP_ANIMPAR_CROWNCLOSURE,
	WCS_FOLIAGEGRP_ANIMPAR_AGE,
	WCS_FOLIAGEGRP_NUMANIMPAR
	}; //  foliage group anim params

enum
	{
	WCS_FOLIAGEGRP_TEXTURE_HEIGHT,
	WCS_FOLIAGEGRP_TEXTURE_DENSITY,
	WCS_FOLIAGEGRP_TEXTURE_DBH,
	WCS_FOLIAGEGRP_TEXTURE_BASALAREA,
	WCS_FOLIAGEGRP_TEXTURE_CROWNCLOSURE,
	WCS_FOLIAGEGRP_TEXTURE_AGE,
	WCS_FOLIAGEGRP_NUMTEXTURES
	}; //  foliage group textures

enum
	{
	WCS_FOLIAGEGRP_THEME_HEIGHT,
	WCS_FOLIAGEGRP_THEME_MINHEIGHT,
	WCS_FOLIAGEGRP_THEME_DENSITY,
	WCS_FOLIAGEGRP_THEME_DBH,
	WCS_FOLIAGEGRP_THEME_BASALAREA,
	WCS_FOLIAGEGRP_THEME_CROWNCLOSURE,
	WCS_FOLIAGEGRP_THEME_AGE,
	WCS_FOLIAGEGRP_NUMTHEMES
	}; //  foliage group themes

class FoliageGroup : public RasterAnimHost, public RootTextureParent, public ThematicOwner
	{
	public:
		AnimDoubleTime AnimPar[WCS_FOLIAGEGRP_NUMANIMPAR];
		FoliageGroup *Next;
		Foliage *Fol;
		ThematicMap *Theme[WCS_FOLIAGEGRP_NUMTHEMES];
		RootTexture *TexRoot[WCS_FOLIAGEGRP_NUMTEXTURES];	// for height pct., density pct.
		char Enabled, Name[WCS_EFFECT_MAXNAMELENGTH];
		AnimDoubleCurve DBHCurve, AgeCurve;

		#ifdef WCS_FORESTRY_WIZARD
		double GroupPercentage, RelativeGroupDensity, RelativeGroupSize, AvgImageHeightWidthRatio,
			HeightRange, MinHeight, MaxHeight, AgvHeight, NormalizedDensity, RawDensity, MaxDbh, MaxAge, MaxClosure, 
			MaxStems, MaxBasalArea;
		RootTexture *GroupSizeTex, *GroupDensityTex, *GroupHeightTex, *GroupStemsTex, *GroupDbhTex, *GroupClosureTex, *GroupAgeTex, 
			*GroupBasalAreaTex;
		ThematicMap *GroupSizeTheme, *GroupDensityTheme, *GroupHeightTheme, *GroupMinHeightTheme, *GroupStemsTheme, *GroupDbhTheme, *GroupClosureTheme, 
			*GroupAgeTheme, *GroupBasalAreaTheme;
		char GroupSizeComputed, GroupDensityComputed;
		#endif // WCS_FORESTRY_WIZARD

		FoliageGroup(RasterAnimHost *RAHost);
		~FoliageGroup();
		void SetAnimDefaults(Ecotype *Master);
		void Copy(FoliageGroup *CopyTo, FoliageGroup *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		Foliage *FindFoliage(char *FindName);
		Foliage *AddFoliage(Foliage *CopyFol);
		int AnimateShadows(void);
		long GetNumAnimParams(void) {return (WCS_FOLIAGEGRP_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_FOLIAGEGRP_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		bool AnimateMaterials(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_FOLIAGEGROUP);};
		long InitImageIDs(long &ImageID);
		int InitToRender(void);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB);
		int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		double CalcAvgImageHeightWidthRatio(void);

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_FOLIAGEGRP_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_FOLIAGEGRP_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum);
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_FOLIAGEGRP_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual int SetTheme(long ThemeNum, ThematicMap *NewTheme);
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};
		virtual int GetThemeType(long ThemeNum);	// return 0 for area and 1 to make it point type

		// inherited from RootTextureParent
		virtual char *GetTextureName(long TexNumber);
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual long GetNumTextures(void) {return (WCS_FOLIAGEGRP_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_FOLIAGEGRP_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum) {return (TexNum < WCS_FOLIAGEGRP_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_FOLIAGEGRP_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_FOLIAGEGRP);};
		virtual char *GetRAHostTypeString(void) {return ("(Foliage Group)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual void EditRAHost(void);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// in ParamConvert.cpp
		void TransferParamFoliageGroupData(struct pFoliageGroup *ParamFolGrp);

		// in RenderForestry.cpp
		void ComputeGroupRelativeDensity(ForestryRenderData *ForDat);
		void ComputeGroupRelativeSize(ForestryRenderData *ForDat);
		void ComputeGroupSize(ForestryRenderData *ForDat);
		void ComputeGroupDensity(ForestryRenderData *ForDat);
		void ComputeGroupSizeFromHt(ForestryRenderData *ForDat);
		void ComputeGroupSizeFromDbh(ForestryRenderData *ForDat);
		void ComputeGroupSizeFromAge(ForestryRenderData *ForDat);
		void ComputeGroupSizeFromClosure(ForestryRenderData *ForDat);
		void ComputeGroupDensityFromStems(ForestryRenderData *ForDat);
		void ComputeGroupDensityFromBasalArea(ForestryRenderData *ForDat);
		void ComputeGroupDensityFromClosure(ForestryRenderData *ForDat);
		void ComputeGroupHeight(ForestryRenderData *ForDat);
		void ComputeGroupDbh(ForestryRenderData *ForDat);
		void ComputeGroupAge(ForestryRenderData *ForDat);
		void ComputeGroupClosure(ForestryRenderData *ForDat);
		void ComputeGroupStems(ForestryRenderData *ForDat);
		void ComputeGroupBasalArea(ForestryRenderData *ForDat);

	}; // class FoliageGroup

enum
	{
	WCS_ECOTYPE_ANIMPAR_MAXHEIGHT,
	WCS_ECOTYPE_ANIMPAR_MINHEIGHT,
	WCS_ECOTYPE_ANIMPAR_DENSITY,
	WCS_ECOTYPE_ANIMPAR_DBH,
	WCS_ECOTYPE_ANIMPAR_BASALAREA,
	WCS_ECOTYPE_ANIMPAR_CROWNCLOSURE,
	WCS_ECOTYPE_ANIMPAR_AGE,
	WCS_ECOTYPE_NUMANIMPAR
	}; // animated Ecotype params

enum
	{
	WCS_ECOTYPE_TEXTURE_DISSOLVECOLOR,
	WCS_ECOTYPE_TEXTURE_HEIGHT,
	WCS_ECOTYPE_TEXTURE_DENSITY,
	WCS_ECOTYPE_TEXTURE_DBH,
	WCS_ECOTYPE_TEXTURE_BASALAREA,
	WCS_ECOTYPE_TEXTURE_CROWNCLOSURE,
	WCS_ECOTYPE_TEXTURE_AGE,
	WCS_ECOTYPE_NUMTEXTURES
	}; //  Ecotype textures

enum
	{
	WCS_ECOTYPE_THEME_HEIGHT,
	WCS_ECOTYPE_THEME_MINHEIGHT,
	WCS_ECOTYPE_THEME_DENSITY,
	WCS_ECOTYPE_THEME_DBH,
	WCS_ECOTYPE_THEME_BASALAREA,
	WCS_ECOTYPE_THEME_CROWNCLOSURE,
	WCS_ECOTYPE_THEME_AGE,
	WCS_ECOTYPE_NUMTHEMES
	}; //  Ecotype themes

enum
	{
	WCS_ECOTYPE_ABSRESIDENT_ECOTYPE,
	WCS_ECOTYPE_ABSRESIDENT_FOLGROUP
	}; //  where are absolute values

enum
	{
	WCS_ECOTYPE_SECONDHT_MINABS,
	WCS_ECOTYPE_SECONDHT_MINPCT,
	WCS_ECOTYPE_SECONDHT_RANGEPCT
	}; //  where are absolute values

class Ecotype : public RasterAnimHost, public RootTextureParent, public ThematicOwner
	{
	public:
		AnimDoubleTime AnimPar[WCS_ECOTYPE_NUMANIMPAR];
		AnimColorTime DissolveColor;	// includes color intensity
		FoliageGroup *FolGrp;
		char Enabled, ConstDensity, DensityUnits, BasalAreaUnits, DissolveEnabled, AbsHeightResident, AbsDensResident,
			SecondHeightType, RenderOccluded, SizeMethod, DensityMethod, PrevSizeMethod, PixelTexturesExist;		// set when rendering
		double DissolvePixelHeight;	// user-settable
		double DensityPerSqMeter,	// calc for rendering
			DissolveDistance,	// calc by renderer
			HeightRange, DensityNormalizer;						// calc by renderer
		#ifdef WCS_USE_OLD_FOLIAGE
		FoliageChainList *ChainList;
		#endif // WCS_USE_OLD_FOLIAGE
		RootTexture *TexRoot[WCS_ECOTYPE_NUMTEXTURES];	// for dissolve color
		ThematicMap *Theme[WCS_ECOTYPE_NUMTHEMES];
		PRNGX Rand;
		AnimDoubleCurve DBHCurve, AgeCurve;

		#ifdef WCS_FORESTRY_WIZARD
		double AvgImageHeightWidthRatio, BasalAreaUnitConversion;
		RootTexture *EcoDensityTex, *EcoSizeTex;
		ThematicMap *EcoHeightTheme, *EcoMinHeightTheme, *EcoDbhTheme, *EcoAgeTheme, *EcoClosureTheme, 
			*EcoStemsTheme, *EcoBasalAreaTheme;
		#endif // WCS_FORESTRY_WIZARD

		Ecotype();
		Ecotype(RasterAnimHost *RAHost);
		~Ecotype();
		void SetDefaults(void);
		void SetAnimDefaults(void);
		void Copy(Ecotype *CopyTo, Ecotype *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		FoliageGroup *FindFoliageGroup(char *FindName);
		FoliageGroup *AddFoliageGroup(FoliageGroup *CopyFolGrp, char *NewName);
		int AnimateShadows(void);
		long GetNumAnimParams(void) {return (WCS_ECOTYPE_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_ECOTYPE_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		int GetRAHostAnimated(void);
		bool AnimateMaterials(void);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_ECOTYPE);};
		long InitImageIDs(long &ImageID);
		void EvaluateDissolve(PixelData *Pix, double DissolveWeight);
		int BuildFoliageChain(RenderData *Rend);
		int BuildFoliageChainDensAndHtByEcotype(RenderData *Rend);
		int BuildFoliageChainDensAndHtByGroup(RenderData *Rend);
		int BuildFoliageChainDensByGroup(RenderData *Rend);
		int BuildFoliageChainHtByGroup(RenderData *Rend);
		#ifdef WCS_USE_OLD_FOLIAGE
		void DeleteFoliageChain(void);
		#endif // WCS_USE_OLD_FOLIAGE
		int InitToRender(void);
		int FindnRemove3DObjects(Object3DEffect *RemoveMe);
		int BuildFileComponentsList(EffectList **Material3Ds, EffectList **Object3Ds, EffectList **Waves, 
			EffectList **Queries, EffectList **Themes, EffectList **Coords);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil, ThematicMap **&ThemeAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB);
		int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		void ChangeSizeMethod(void);
		void ChangeAbsHtResident(void);
		void ChangeAbsDensResident(void);
		void ChangeSecondHtType(char LastType);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		double CalcAvgImageHeightWidthRatio(void);
		int SelectForestryImageOrObject(RenderData *Rend, PolygonData *Poly, VertexData *Vtx[3], unsigned long SeedOffset, 
			int FolEffect, FoliagePreviewData *PointData);
		int SelectForestryImageOrObjectStage2(RenderData *Rend, ForestryRenderData *ForDat, FoliagePreviewData *PointData);
		int SelectForestryImageOrObjectStage3(RenderData *Rend, ForestryRenderData *ForDat, FoliagePreviewData *PointData,
			Foliage *FolToRender, VertexDEM *FolVtx, double *Random, double FolHeight);

		// inherited from ThematicOwner
		virtual char *GetThemeName(long ThemeNum);
		virtual long GetNumThemes(void) {return (WCS_ECOTYPE_NUMTHEMES);};
		virtual ThematicMap *GetTheme(long ThemeNum) {return (ThemeNum < WCS_ECOTYPE_NUMTHEMES ? Theme[ThemeNum]: NULL);};
		virtual ThematicMap *GetEnabledTheme(long ThemeNum);
		virtual ThematicMap **GetThemeAddr(long ThemeNum) {return (ThemeNum < WCS_ECOTYPE_NUMTHEMES ? &Theme[ThemeNum]: NULL);};
		virtual int SetTheme(long ThemeNum, ThematicMap *NewTheme);
		virtual ThematicMap *SetThemePtr(long ThemeNum, ThematicMap *NewTheme)	{return (Theme[ThemeNum] = NewTheme);};
		virtual int GetThemeType(long ThemeNum);	// return 0 for area and 1 to make it point type

		// inherited from RootTextureParent
		virtual char *GetTextureName(long TexNumber);
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual long GetNumTextures(void) {return (WCS_ECOTYPE_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_ECOTYPE_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum) {return (TexNum < WCS_ECOTYPE_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_ECOTYPE_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_ECOTYPE);};
		virtual char *GetRAHostTypeString(void) {return ("(Ecotype)");};
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual char *OKRemoveRaster(void);
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual void EditRAHost(void);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);

		// in ParamConvert.cpp
		void TransferParamEcotypeData(struct Ecosystem *ParamEco, short EcoNumber, struct pEcotype *ParamEcotype);

		// in RenderForestry.cpp
		void ComputeEcoSize(ForestryRenderData *ForDat);
		void ComputeEcoDensity(ForestryRenderData *ForDat);
		void ComputeEcoSizeFromHt(ForestryRenderData *ForDat);
		void ComputeEcoSizeFromDbh(ForestryRenderData *ForDat);
		void ComputeEcoSizeFromAge(ForestryRenderData *ForDat);
		void ComputeEcoSizeFromClosure(ForestryRenderData *ForDat);
		void ComputeEcoDensityFromStems(ForestryRenderData *ForDat);
		void ComputeEcoDensityFromBasalArea(ForestryRenderData *ForDat);
		void ComputeEcoDensityFromClosure(ForestryRenderData *ForDat);
		void ComputeEcoHeight(ForestryRenderData *ForDat);
		void ComputeEcoDbh(ForestryRenderData *ForDat);
		void ComputeEcoAge(ForestryRenderData *ForDat);
		void ComputeEcoClosure(ForestryRenderData *ForDat);
		void ComputeEcoStems(ForestryRenderData *ForDat);
		void ComputeEcoBasalArea(ForestryRenderData *ForDat);

	}; // class Ecotype

// this class is used as a linked chain by the renderer
class FoliageLink
	{
	public:
		Foliage *Fol;
		FoliageGroup *Grp;
		Raster *Rast;
		Object3DEffect *Obj;
		FoliageLink *Next;
		RootTexture *EcoDensTex, *EcoHtTex, *GrpDensTex, *GrpHtTex, *FolDensTex, 
			*FolHtTex, *FolColorTex, *FolBacklightTex;
		ThematicMap *EcoHtTheme, *GrpDensTheme, *GrpHtTheme, *FolDensTheme, 
			*FolHtTheme, *FolColorTheme;
		char Enabled, ColorImage, PosShade, Shade3D, RenderOccluded;
		double Density, ImageWidthFactor;

		FoliageLink();

	}; // class FoliageLink

#ifdef WCS_USE_OLD_FOLIAGE
class FoliageChainList
	{
	public:
		double DensityPerSqMeter, HeightRange, MaxHeight, EcoThemeDensFactor;
		FoliageLink *FoliageChain;
		ThematicMap *DensityTheme;
		FoliageChainList *Next;

		FoliageChainList()	{FoliageChain = NULL; DensityTheme = NULL, Next = NULL; DensityPerSqMeter = HeightRange = MaxHeight = 0.0; EcoThemeDensFactor = 1.0;};
	}; // class FoliageChainList
#endif // WCS_USE_OLD_FOLIAGE


// file codes
#define WCS_PARAM_DONE						0xffff0000

#define WCS_FOLIAGE_HEIGHTPCT			0x00010000
#define WCS_FOLIAGE_DENSPCT				0x00020000
#define WCS_FOLIAGE_COLOR				0x00030000
#define WCS_FOLIAGE_FOLIAGETYPE			0x00040000
#define WCS_FOLIAGE_POSSHADE			0x00050000
#define WCS_FOLIAGE_SHADE3D				0x00060000
#define WCS_FOLIAGE_RANDOMROTATEX		0x00070000
#define WCS_FOLIAGE_RANDOMROTATEY		0x00080000
#define WCS_FOLIAGE_RANDOMROTATEZ		0x00090000
#define WCS_FOLIAGE_ROTATEX				0x000a0000
#define WCS_FOLIAGE_ROTATEY				0x000b0000
#define WCS_FOLIAGE_ROTATEZ				0x000c0000
#define WCS_FOLIAGE_FLIPX				0x000d0000
#define WCS_FOLIAGE_IMAGEID				0x000e0000
#define WCS_FOLIAGE_OBJECTNAME			0x000f0000
#define WCS_FOLIAGE_ENABLED				0x00210000
#define WCS_FOLIAGE_ORIENTATIONSHADING	0x00220000
#define WCS_FOLIAGE_TEXTUREROOTNUM		0x00310000	// used only in early v5 beta
#define WCS_FOLIAGE_TEXTUREROOT			0x00320000	// used only in early v5 beta
#define WCS_FOLIAGE_TEXHEIGHT			0x00330000
#define WCS_FOLIAGE_TEXDENSITY			0x00340000
#define WCS_FOLIAGE_TEXCOLOR			0x00350000
#define WCS_FOLIAGE_THEMEHEIGHT			0x00360000
#define WCS_FOLIAGE_THEMEDENSITY		0x00370000
#define WCS_FOLIAGE_THEMECOLOR			0x00380000
#define WCS_FOLIAGE_TEXORIENTSHADING	0x00390000

#define WCS_FOLIAGEGRP_NAME					0x00010000
#define WCS_FOLIAGEGRP_V5HEIGHTPCT			0x00020000	// used in v5 and early VNS beta
#define WCS_FOLIAGEGRP_V5DENSPCT			0x00030000	// used in v5 and early VNS beta
#define WCS_FOLIAGEGRP_ENABLED				0x00040000
#define WCS_FOLIAGEGRP_BROWSEDATA			0x00050000
#define WCS_FOLIAGEGRP_MAXHEIGHT			0x00060000
#define WCS_FOLIAGEGRP_MINHEIGHT			0x00070000
#define WCS_FOLIAGEGRP_DENSITY				0x00080000
#define WCS_FOLIAGEGRP_FOLIAGE				0x00210000
#define WCS_FOLIAGEGRP_TEXTUREROOTNUM		0x00310000	// used only in early v5 beta
#define WCS_FOLIAGEGRP_TEXTUREROOT			0x00320000	// used only in early v5 beta
#define WCS_FOLIAGEGRP_TEXHEIGHT			0x00330000
#define WCS_FOLIAGEGRP_TEXDENSITY			0x00340000
#define WCS_FOLIAGEGRP_THEMEHEIGHT			0x00350000
#define WCS_FOLIAGEGRP_THEMEDENSITY			0x00360000
#define WCS_FOLIAGEGRP_DBH					0x00380000
#define WCS_FOLIAGEGRP_BASALAREA			0x00390000
#define WCS_FOLIAGEGRP_CROWNCLOSURE			0x003b0000
#define WCS_FOLIAGEGRP_AGE					0x003a0000
#define WCS_FOLIAGEGRP_TEXDBH				0x003c0000
#define WCS_FOLIAGEGRP_TEXBASALAREA			0x003d0000
#define WCS_FOLIAGEGRP_TEXCROWNCLOSURE		0x003e0000
#define WCS_FOLIAGEGRP_TEXAGE				0x003f0000
#define WCS_FOLIAGEGRP_THEMEDBH				0x00410000
#define WCS_FOLIAGEGRP_THEMEBASALAREA		0x00420000
#define WCS_FOLIAGEGRP_THEMECROWNCLOSURE	0x00430000
#define WCS_FOLIAGEGRP_THEMEAGE				0x00440000
#define WCS_FOLIAGEGRP_AGECURVE				0x00450000
#define WCS_FOLIAGEGRP_DBHCURVE				0x00460000
#define WCS_FOLIAGEGRP_THEMEMINHEIGHT		0x00470000

#define WCS_ECOTYPE_DISSOLVECOLOR			0x00010000
#define WCS_ECOTYPE_CONSTDENSITY			0x00020000
#define WCS_ECOTYPE_DENSITYUNITS			0x00030000
#define WCS_ECOTYPE_DISSOLVEENABLED			0x00040000
#define WCS_ECOTYPE_DISSOLVEPIXHT			0x00050000
#define WCS_ECOTYPE_MAXHEIGHT				0x00060000
#define WCS_ECOTYPE_MINHEIGHT				0x00070000
#define WCS_ECOTYPE_FOLDENSITY				0x00080000
#define WCS_ECOTYPE_ENABLED					0x00090000
#define WCS_ECOTYPE_BROWSEDATA				0x000a0000
#define WCS_ECOTYPE_ABSHEIGHTRESIDENT		0x000b0000
#define WCS_ECOTYPE_ABSDENSRESIDENT			0x000c0000
#define WCS_ECOTYPE_SECONDHEIGHTTYPE		0x000d0000
#define WCS_ECOTYPE_FOLIAGEGROUP			0x00210000
#define WCS_ECOTYPE_TEXTUREROOTNUM			0x00310000	// used only in early v5 beta
#define WCS_ECOTYPE_TEXTUREROOT				0x00320000	// used only in early v5 beta
#define WCS_ECOTYPE_TEXDISSOLVECOLOR		0x00330000
#define WCS_ECOTYPE_TEXHEIGHT				0x00340000
#define WCS_ECOTYPE_TEXDENSITY				0x00350000
#define WCS_ECOTYPE_THEMEHEIGHT				0x00360000
#define WCS_ECOTYPE_THEMEDENSITY			0x00370000
#define WCS_ECOTYPE_RENDEROCCLUDED			0x00380000
#define WCS_ECOTYPE_SIZEMETHOD				0x00390000
#define WCS_ECOTYPE_DENSITYMETHOD			0x003a0000
#define WCS_ECOTYPE_DBH						0x003b0000
#define WCS_ECOTYPE_BASALAREA				0x003c0000
#define WCS_ECOTYPE_CROWNCLOSURE			0x003d0000
#define WCS_ECOTYPE_AGE						0x003e0000
#define WCS_ECOTYPE_TEXDBH					0x003f0000
#define WCS_ECOTYPE_TEXBASALAREA			0x00410000
#define WCS_ECOTYPE_TEXCROWNCLOSURE			0x00420000
#define WCS_ECOTYPE_TEXAGE					0x00430000
#define WCS_ECOTYPE_THEMEDBH				0x00440000
#define WCS_ECOTYPE_THEMEBASALAREA			0x00450000
#define WCS_ECOTYPE_THEMECROWNCLOSURE		0x00460000
#define WCS_ECOTYPE_THEMEAGE				0x00470000
#define WCS_ECOTYPE_AGECURVE				0x00480000
#define WCS_ECOTYPE_DBHCURVE				0x00490000
#define WCS_ECOTYPE_BASALAREAUNITS			0x004a0000
#define WCS_ECOTYPE_THEMEMINHEIGHT			0x004b0000


#endif // WCS_FOLIAGE_H
