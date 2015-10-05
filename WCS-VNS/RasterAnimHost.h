// RasterAnimHost.h
// Header file for EffectsLib.cpp
// Built from scratch on 04/30/97 by Gary R. Huber
// Copyright 1997 Questar Productions. All rights reserved.


#ifndef WCS_RASTERANIMHOST_H
#define WCS_RASTERANIMHOST_H

#include <vector>
#include <string>

class RasterShell;
class RootTexture;
class AnimCritter;
class AnimDoubleTime;
class RasterAnimHost;
class Thumbnail;
class DiagnosticData;
class EffectsLib;
class PopMenuAdder;
class ThematicMap;
class RootTextureParent;
class ThematicOwner;

#define WCS_RAHOST_MASKBIT_FLAGS		(1 << 0)
#define WCS_RAHOST_MASKBIT_NAME			(1 << 1)
#define WCS_RAHOST_MASKBIT_TYPE			(1 << 2)
#define WCS_RAHOST_MASKBIT_KEYRANGE		(1 << 3)
#define WCS_RAHOST_MASKBIT_NEXTKEY		(1 << 4)
#define WCS_RAHOST_MASKBIT_TYPENUMBER	(1 << 5)
#define WCS_RAHOST_MASKBIT_DROPOK		(1 << 6)
#define WCS_RAHOST_MASKBIT_DROPSOURCE	(1 << 7)
#define WCS_RAHOST_MASKBIT_FILEINFO		(1 << 8)
#define WCS_RAHOST_MASKBIT_LOADFILE		(1 << 9)
#define WCS_RAHOST_MASKBIT_SAVEFILE		(1 << 10)
#define WCS_RAHOST_MASKBIT_INTERFLAGS	(1 << 11)
#define WCS_RAHOST_MASKBIT_POPCLASS		(1 << 12)

#define WCS_RAHOST_FLAGBITS_ICONTYPE	(0xff)			// from the enum list above
#define WCS_RAHOST_FLAGBIT_ENABLED		(1 << 8)		// if it and all ancesors are enabled
#define WCS_RAHOST_FLAGBIT_ANIMATABLE	(1 << 9)		// if anim graph can be opened
#define WCS_RAHOST_FLAGBIT_ANIMATED		(1 << 10)		// if it or children has keyframes
#define WCS_RAHOST_FLAGBIT_CHILDREN		(1 << 11)		// if there are children
#define WCS_RAHOST_FLAGBIT_DRAGGABLE	(1 << 12)		// if item can be dragged in S@G
#define WCS_RAHOST_FLAGBIT_DRAGTARGET	(1 << 13)		// if item can potentially be a drag target
#define WCS_RAHOST_FLAGBIT_EDITNAME		(1 << 14)		// if item has a user-editable name
#define WCS_RAHOST_FLAGBIT_EXPANDED1	(1 << 15)		// if item is expanded in list 1
#define WCS_RAHOST_FLAGBIT_EXPANDED2	(1 << 16)		// if item is expanded in list 2
#define WCS_RAHOST_FLAGBIT_EXPANDED		((1 << 15) | (1 << 16))		// if item is expanded in list 1 or 2
#define WCS_RAHOST_FLAGBIT_DELETABLE	(1 << 17)		// if it and all ancesors are enabled

#define WCS_RAHOST_INTERBIT_CLICKTOPOS	(1 << 0)
#define WCS_RAHOST_INTERBIT_MOVEX		(1 << 1)
#define WCS_RAHOST_INTERBIT_MOVEY		(1 << 2)
#define WCS_RAHOST_INTERBIT_MOVEZ		(1 << 3)
#define WCS_RAHOST_INTERBIT_MOVEELEV	(1 << 4)
#define WCS_RAHOST_INTERBIT_SCALEX		(1 << 5)
#define WCS_RAHOST_INTERBIT_SCALEY		(1 << 6)
#define WCS_RAHOST_INTERBIT_SCALEZ		(1 << 7)
#define WCS_RAHOST_INTERBIT_ROTATEX		(1 << 8)
#define WCS_RAHOST_INTERBIT_ROTATEY		(1 << 9)
#define WCS_RAHOST_INTERBIT_ROTATEZ		(1 << 10)
#define WCS_RAHOST_INTERBIT_POINTS		(1 << 11)

class RasterAnimHostProperties
	{
	public:
		double KeyNodeRange[2], NewKeyNodeRange[2];
		short ByteFlip;
		unsigned long Flags, InterFlags, PropMask, FlagsMask, PopClassFlags, PopEnabledFlags, PopExistsFlags;
		long TypeNumber, ChildTypeFilter;
		RasterAnimHost *DropSource, *ChildA, **ChildB;
		char *Name, *Type, *Path, *Ext;
		FILE *fFile;
		unsigned char FileVersion, FileRevision;
		char ItemOperator, FrameOperator, KeyframeOperation, DropOK, Queries;

		// in EffectsLib.cpp
		RasterAnimHostProperties();
		unsigned long TestInterFlags(unsigned long FlagTest) {return(FlagTest & InterFlags);};
		void Copy(RasterAnimHostProperties *CopyFrom);
	};

class BrowseData
	{
	public:
		Thumbnail *Thumb;
		char *Author, *Author2, *Comment, *Date, *Address, *Category;

		// in EffectsLib.cpp
		BrowseData();
		~BrowseData();
		void FreeAll(void);
		void Copy(BrowseData *CopyTo, BrowseData *CopyFrom);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		unsigned long Save(FILE *ffile);
		void FreeAuthor(void);
		void FreeAuthor2(void);
		void FreeDate(void);
		void FreeAddress(void);
		void FreeComment(void);
		void FreeCategory(void);
		char *NewAuthor(long NewLength);
		char *NewAuthor2(long NewLength);
		char *NewDate(long NewLength);
		char *NewAddress(long NewLength);
		char *NewComment(long NewLength);
		char *NewCategory(long NewLength);
		void FreeThumb(void);
		Thumbnail *NewThumb(void);
		Thumbnail *GetThumb(void) {return (Thumb);};
		Thumbnail *SetThumb(Thumbnail *NewThumbData) {Thumb = NewThumbData; return (Thumb);};
		int LoadBrowseInfo(char *FileName);
	}; // class BrowseData

class RasterAnimHost
	{
	public:
		RasterAnimHost *RAParent;
		BrowseData *BrowseInfo;
		unsigned char Expanded, TemplateItem;
		static int ActiveLock;
		static RasterAnimHost *ActiveRAHost, *BackupRAHost, *CopyOfRAHost;
		static void SetActiveRAHost(RasterAnimHost *NewActive, int CheckSAG = 0);
		static void SetActiveRAHostNoBackup(RasterAnimHost *NewActive, int CheckSAG = 0);
		static void SetBackupRAHost(RasterAnimHost *NewActive, int CheckSAG = 0);
		static void RestoreBackupRAHost(void);
		static RasterAnimHost *GetActiveRAHost(void)	{return (ActiveRAHost);};
		static RasterAnimHost *GetBackupRAHost(void)	{return (BackupRAHost);};
		static void SetCopyOfRAHost(RasterAnimHost *NewCopyTo)	{CopyOfRAHost = NewCopyTo;};
		static RasterAnimHost *GetCopyOfRAHost(void)	{return (CopyOfRAHost);};
		static int IsDigitizeLegal(RasterAnimHost *TestMe, long CategoryOnly);

		static int GetRADropVectorOK(long TypeNumber);
		static void SetActiveLock(int SetLock) {ActiveLock = SetLock;};
		static int GetActiveLock(void) {return(ActiveLock);};

		RasterAnimHost()										{RAParent = NULL; Expanded = TemplateItem = 0; BrowseInfo = NULL;};
		RasterAnimHost(RasterAnimHost *RAHost)					{RAParent = RAHost; Expanded = TemplateItem = 0; BrowseInfo = NULL;};
		virtual ~RasterAnimHost();
		RasterAnimHost *GetRAHostRoot(void)						{return (RAParent ? RAParent->GetRAHostRoot(): this);};
		void Copy(RasterAnimHost *CopyTo, RasterAnimHost *CopyFrom);
		void SetExpansionFlags(unsigned long Mask, unsigned long Flags)
			{
			if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED1)
				{
				if (Flags & WCS_RAHOST_FLAGBIT_EXPANDED1)
					Expanded |= 0x01;
				else
					Expanded &= ~0x01;
				} // if
			if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED2)
				{
				if (Flags & WCS_RAHOST_FLAGBIT_EXPANDED2)
					Expanded |= 0x02;
				else
					Expanded &= ~0x02;
				} // if
			}; // SetExpansionFlags
		void GetExpansionFlags(unsigned long Mask, unsigned long &Flags)
			{
			if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED1)
				{
				if (Expanded & 0x01)
					Flags |= WCS_RAHOST_FLAGBIT_EXPANDED1;
				else
					Flags &= ~WCS_RAHOST_FLAGBIT_EXPANDED1;
				} // if
			if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED2)
				{
				if (Expanded & 0x02)
					Flags |= WCS_RAHOST_FLAGBIT_EXPANDED2;
				else
					Flags &= ~WCS_RAHOST_FLAGBIT_EXPANDED2;
				} // if
			}; // GetExpansionFlags

		// in EffectsLib.cpp
		int FindnRemoveRAHostChild(RasterAnimHost *RemoveMe, int &RemoveAll);	// return 0 means user cancelled
		int LoadFilePrep(RasterAnimHostProperties *Prop);
		int SaveFilePrep(RasterAnimHostProperties *Prop);
		void OpenGallery(EffectsLib *EffectsHost);
		void OpenBrowseData(EffectsLib *EffectsHost);
		int LoadComponentFile(char *NameSupplied, char Queries = 1);
		int GetPopClassFlags(RasterAnimHostProperties *Prop, AnimCritter *AnimAffil,
			RootTexture **TexAffil, ThematicMap **ThemeAffil);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, 
			RasterAnimHost **ChildB, AnimCritter *AnimAffil, RootTexture **TexAffil, ThematicMap **ThemeAffil);
		int HandleSRAHPopMenuSelection(void *Action, AnimCritter *AnimAffil, RootTexture **TexAffil, 
			ThematicMap **ThemeAffil, RootTextureParent *TexParent, ThematicOwner *ThemeOwner);
		void Embed(void);

		// virtual functions, la raison d'etre de la RasterAnimHost
		virtual int GetRAEnabled(void)							{return (RAParent ? RAParent->GetRAEnabled(): 1);};
		virtual char *OKRemoveRaster(void)						{return (NULL);};
		virtual void RemoveRaster(RasterShell *Shell)			{return;};
		virtual void EditRAHost(void)							{if (RAParent) RAParent->EditRAHost(); return;};
		virtual char *GetRAHostName(void)						{return (RAParent ? RAParent->GetRAHostName(): (char *)"");};
		virtual char *GetRAHostTypeString(void)					{return ("");};
		virtual char *GetCritterName(RasterAnimHost *TestMe)	{return ("");};
		virtual int GetDeletable(RasterAnimHost *TestMe)		{return (0);};
		virtual int RemoveRAHostQuery(RasterAnimHost *RemoveMe);
		virtual int GetNextAnimNode(RasterAnimHostProperties *Prop);
		virtual int ScaleDeleteAnimNodes(RasterAnimHostProperties *Prop);
		virtual unsigned char GetNotifyClass(void)				{return (RAParent ? RAParent->GetNotifyClass(): GetNotifySubclass());};
		virtual unsigned char GetNotifySubclass(void) = 0;
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop) = 0;
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)	{return (NULL);};
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace) {ApplyToColor = 0; ApplyToDisplace = 0;};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe)		{return (1);};	// return 1 means object found and deleted
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother)	{return (NULL);};
		virtual void SetFloating(char NewFloating)				{return;};
		virtual void GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)	{if (RAParent) RAParent->GetInterFlags(Prop, FlagMe); else Prop->InterFlags = 0; return;};
		virtual int ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation) 
			{return (RAParent ? RAParent->ScaleMoveRotate(MoveMe, Data, Operation): 0);};
		virtual int ComponentOverwriteOK(RasterAnimHostProperties *Prop) {return (1);};
		virtual void HeresYourNewFilePathIfYouCare(char *NewFullPath) {return;};

		// S@G Popup Context Menu-handling methods
		int AddBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags); 		// in EffectsLib.cpp
		virtual int AddDerivedPopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags) {return(0);}; // in each effect type
		virtual int HandlePopMenuSelection(void *Action) {return(0);}; // in each effect type

		// SRAH Popup Context Menu-handling methods
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB); // GeneralEffect implementation in EffectsLib.cpp
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB) {return(0);};
	}; // class RasterAnimHost


enum
	{
	WCS_RAHOST_ICONTYPE_MISC,
	WCS_RAHOST_ICONTYPE_VECTOR,
	WCS_RAHOST_ICONTYPE_DEM,
	WCS_RAHOST_ICONTYPE_CONTROLPT,
	WCS_RAHOST_ICONTYPE_MATERIAL,

	WCS_RAHOST_ICONTYPE_TERRAINPAR,
	WCS_RAHOST_ICONTYPE_CLOUD, // cloud
	WCS_RAHOST_ICONTYPE_LIGHT,
	WCS_RAHOST_ICONTYPE_LAKE,
	WCS_RAHOST_ICONTYPE_ECOTYPE,

	WCS_RAHOST_ICONTYPE_FOLIAGEGROUP,
	WCS_RAHOST_ICONTYPE_FOLIAGE,
	WCS_RAHOST_ICONTYPE_ROOTTEXTURE,
	WCS_RAHOST_ICONTYPE_TEXTURE,
	WCS_RAHOST_ICONTYPE_RASTER,

	WCS_RAHOST_ICONTYPE_GEOREGISTER,
	WCS_RAHOST_ICONTYPE_WAVESOURCE,
	WCS_RAHOST_ICONTYPE_ANIMDOUBLETIME,
	WCS_RAHOST_ICONTYPE_ANIMDOUBLEDISTANCE,
	WCS_RAHOST_ICONTYPE_ANIMCOLORTIME,

	WCS_RAHOST_ICONTYPE_ANIMCOLORGRADIENT,
	WCS_RAHOST_ICONTYPE_ANIMMATERIALGRADIENT,
	WCS_RAHOST_ICONTYPE_COLORTEXTURE,
	WCS_RAHOST_ICONTYPE_RENDER,
	WCS_RAHOST_ICONTYPE_ENVIRONMENT,

	WCS_RAHOST_ICONTYPE_CMAP,
	WCS_RAHOST_ICONTYPE_FOLEFFECT,
	WCS_RAHOST_ICONTYPE_3DOBJECT,
	WCS_RAHOST_ICONTYPE_PLANET,
	WCS_RAHOST_ICONTYPE_SNOW,

	WCS_RAHOST_ICONTYPE_STAR,
	WCS_RAHOST_ICONTYPE_TERRAFFECTOR,
	WCS_RAHOST_ICONTYPE_RASTERTA,
	WCS_RAHOST_ICONTYPE_SHADOW,
	WCS_RAHOST_ICONTYPE_WAVE,

	WCS_RAHOST_ICONTYPE_STREAM,
	WCS_RAHOST_ICONTYPE_ATMOSPHERE,
	WCS_RAHOST_ICONTYPE_SKY,
	WCS_RAHOST_ICONTYPE_CELESTIAL,
	WCS_RAHOST_ICONTYPE_ILLUMINATION,

	WCS_RAHOST_ICONTYPE_MATERIALSTRATA,
	WCS_RAHOST_ICONTYPE_GRADIENTPROFILE,
	WCS_RAHOST_ICONTYPE_ENVELOPE,
	WCS_RAHOST_ICONTYPE_CROSSSECTION,
	WCS_RAHOST_ICONTYPE_ECOSYSTEM,

	WCS_RAHOST_ICONTYPE_FENCE,
	WCS_RAHOST_ICONTYPE_LABEL,
	WCS_RAHOST_ICONTYPE_COORDSYS,
	WCS_RAHOST_ICONTYPE_DEMMERGE,
	WCS_RAHOST_ICONTYPE_THEMATIC,
	WCS_RAHOST_ICONTYPE_RENDJOB,
	WCS_RAHOST_ICONTYPE_RENDOPT,
	WCS_RAHOST_ICONTYPE_RENDSCEN,
	WCS_RAHOST_ICONTYPE_SCHQY,
	WCS_RAHOST_ICONTYPE_TERRAINGEN,
	WCS_RAHOST_ICONTYPE_TERRAINGRID,
	WCS_RAHOST_ICONTYPE_SCENEEXP,
	WCS_RAHOST_ICONTYPE_POSTPROC,
	WCS_RAHOST_ICONTYPE_GROUND,
	WCS_RAHOST_ICONTYPE_KEYOVERLAY,
	WCS_RAHOST_ICONTYPE_LAYER,
// <<<>>> ADD_NEW_EFFECTS unless the effect will re-use one of the existing icons in S@G
// <<<>>> WCS_ADD_RAHOST	add an item here unless the new RAHost will use an existing icon, add icon to .bmp icon strip
	WCS_RAHOST_NUMICONTYPES
	}; // icon type - limited to 256 types

// object types for drag 'n drop control
// effects will use their effect type
enum
	{
	WCS_RAHOST_OBJTYPE_VECTOR = 100,
	WCS_RAHOST_OBJTYPE_DEM,
	WCS_RAHOST_OBJTYPE_CONTROLPT,
	WCS_RAHOST_OBJTYPE_ECOTYPE,
	WCS_RAHOST_OBJTYPE_FOLIAGEGROUP,
	WCS_RAHOST_OBJTYPE_FOLIAGE,
	WCS_RAHOST_OBJTYPE_ROOTTEXTURE,
	WCS_RAHOST_OBJTYPE_TEXTURE,
	WCS_RAHOST_OBJTYPE_RASTER,
	WCS_RAHOST_OBJTYPE_GEOREGISTER,
	WCS_RAHOST_OBJTYPE_MATERIALSTRATA,
	WCS_RAHOST_OBJTYPE_WAVESOURCE,
	WCS_RAHOST_OBJTYPE_ATMOCOMPONENT,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE,
	WCS_RAHOST_OBJTYPE_ANIMCOLORTIME,
	WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT,
	WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT,
	WCS_RAHOST_OBJTYPE_COLORTEXTURE,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION,
	WCS_RAHOST_OBJTYPE_TERRAINTYPE,
	WCS_RAHOST_OBJTYPE_PROJMETHOD,
	WCS_RAHOST_OBJTYPE_GEODATUM,
	WCS_RAHOST_OBJTYPE_GEOELLIPSOID,
	WCS_RAHOST_OBJTYPE_INCLUDEEXCLUDE,
	WCS_RAHOST_OBJTYPE_POSTPROCEVENT,
	WCS_RAHOST_OBJTYPE_INCLUDEEXCLUDETYPE,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLEBOOLEAN,
	WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE,
// <<<>>> WCS_ADD_RAHOST	add an item here if the new RAHost type is not in the Effects Library
	WCS_RAHOST_OBJTYPE_PROJECT
	}; // object types for drag 'n drop control

enum
	{
	WCS_RAHOST_INTERACTIVEOP_SETPOS,
	WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY,
	WCS_RAHOST_INTERACTIVEOP_SETSIZE,
	WCS_RAHOST_INTERACTIVEOP_SETROTATION,
	WCS_RAHOST_INTERACTIVEOP_MOVEXYZ,
	WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV,
	WCS_RAHOST_INTERACTIVEOP_SCALE,
	WCS_RAHOST_INTERACTIVEOP_ROTATE,
	WCS_RAHOST_INTERACTIVEOP_MOVEPOINTS,
	WCS_RAHOST_INTERACTIVEOP_SCALEPOINTS,
	WCS_RAHOST_INTERACTIVEOP_ROTATEPOINTS
	}; // interactive operations

#define WCS_BROWSEDATA_AUTHOR			0x00210000
#define WCS_BROWSEDATA_AUTHOR2			0x00220000
#define WCS_BROWSEDATA_ADDRESS			0x00230000
#define WCS_BROWSEDATA_DATE				0x00240000
#define WCS_BROWSEDATA_COMMENT			0x00250000
#define WCS_BROWSEDATA_CATEGORY			0x00260000
#define WCS_BROWSEDATA_TNAIL			0x00270000

// PopMenuAdder is implemented in WCSWidgets.cpp

#define WCS_RAH_POPMENU_MAX_STACK	5

class PopMenuAdder
	{
	private:
		std::vector <std::string> Actions;
		int CurItem, MenuStackCurrent;
		void *MenuStack[WCS_RAH_POPMENU_MAX_STACK];
		
	public:
		// Effective 12/2008 the Action parameter is now a string, and is copied to the
		// Action container, so as to support dynamically-formatted Actions.
		PopMenuAdder(void *InitialMenu) {CurItem = 0; MenuStackCurrent = 0; MenuStack[0] = NULL; MenuStack[0] = InitialMenu;};
		int AddPopMenuItem(const char *DisplayText, const char *Action, int Derived, int Enabled = 1, int Checked = 0);
		int BeginPopSubMenu(const char *DisplayText, int Enabled = 1, int Checked = 0);
		int EndPopSubMenu(void);
		int AddPopMenuDivider(void);
		const char *GetAction(int ActionID) {return(Actions[ActionID].c_str());};
	}; // PopMenuAdder

// flag bits for what classes to add
#define WCS_RAH_POPMENU_CLASS_ANIM		(1 << 0)
#define WCS_RAH_POPMENU_CLASS_TEX		(1 << 1)
#define WCS_RAH_POPMENU_CLASS_ECO		(1 << 2)
#define WCS_RAH_POPMENU_CLASS_THEME		(1 << 3)
#define WCS_RAH_POPMENU_CLASS_STRATA	(1 << 4)
#define WCS_RAH_POPMENU_CLASS_FOAM		(1 << 5)

#define WCS_RAH_POPMENU_CLASS_GLOBAL	(1 << 31)

#define WCS_RAH_POPMENU_CLASS_ALL		0xffffffff

// This is a macro only because it explains what it is doing
#define WCS_RAH_POPMENU_CHECK_APPLICABLE(a, b) (a & b)

#endif // WCS_RASTERANIMHOST_H

#ifdef WCS_SAMPLE_CODE

/*******************************************************************************/

unsigned long Foliage::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

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

Mask &= (WCS_RAHOST_ICONTYPE_FOLIAGE | WCS_RAHOST_FLAGBIT_ANIMATABLE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | WCS_RAHOST_FLAGBIT_EDITNAME | Flags);

return (Mask);

} // Foliage::GetRAFlags

#endif // WCS_SAMPLE_CODE
