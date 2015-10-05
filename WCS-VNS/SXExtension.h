// SXExtension.h
// Header file for SceneExporter Extensions.
// Built from scratch on 11/10/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.


#ifndef WCS_SXEXTENSION_H
#define WCS_SXEXTENSION_H

#include "PathAndFile.h"

class SXQueryAction;
class Database;
class Joe;
class RasterAnimHost;
class RasterAnimHostProperties;
class KMLScreenOverlay;
class SceneExporter;

enum
	{
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_VRML,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_VTP,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_3DS,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_MAYA,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_SOFTIMAGE,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_LW,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX,
	WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE
	}; // SXExtension types

class SXExtension
	{
	public:
		SXExtension();
		virtual ~SXExtension();
		virtual int GetType(void) = 0;
		virtual void Copy(SXExtension *SXCopyFrom) = 0;
		virtual unsigned long Save(FILE *ffile) = 0;
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip) = 0;
		virtual void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)	{return;};
		virtual int InitToExport(const char *OutputFilePath)			{return (1);};
		virtual void CleanupFromExport(SceneExporter *MyExporter)	{return;};
		virtual void CloseQueryFiles(void)	{return;};
		virtual bool AcceptDragDrop(long DropID)	{return (0);};
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource, int GenNotify, RasterAnimHost *NotifyObj)	{return (0);};
		
		virtual bool GetSupportsQueryAction(void) {return(false);};
		virtual bool ApproveActionAvailable(int TestType) {return(false);};
		virtual bool ApproveActionItemAvailable(int TestItem) {return(false);};

	}; // class SXExtension

enum
	{
	WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOFIT,
	WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOSCALE
	}; // SXExtensionSTL Build Modes

enum
	{
	WCS_EFFECTS_SXEXTENSION_BUILDUNIT_MM,
	WCS_EFFECTS_SXEXTENSION_BUILDUNIT_INCHES
	}; // SXExtensionSTL Units of Measure

class SXExtensionSTL : public SXExtension
	{
	public:
		double MaxDimX, MaxDimY, MaxDimZ, ActualDimX, ActualDimY, ActualDimZ, VertExag, MinThickness, BuildScale;
		char BuildMode, UnitOfMeasure;
		
		SXExtensionSTL();
		virtual ~SXExtensionSTL();
		virtual int GetType(void)	{return(WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL);};
		virtual void Copy(SXExtension *SXCopyFrom);
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
	}; // class SXExtensionSTL

class SXExtensionOF : public SXExtension
	{
	public:
		char CreateFoliage, IndividualFolLOD;
		
		SXExtensionOF();
		virtual ~SXExtensionOF();
		virtual int GetType(void)	{return(WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT);};
		virtual void Copy(SXExtension *SXCopyFrom);
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
	}; // class SXExtensionOF

enum
	{
	WCS_SXEXTENSIONNVE_NAVSTYLE_SIMPLE
	}; // SXExtensionNVE navigation types

// should be pure virtual -- must derive from this in order to use
class SXExtensionActionable : public SXExtension
	{
	public:
		SXQueryAction *ActionList, *ActiveAction;
		
		SXExtensionActionable();
		virtual ~SXExtensionActionable();
		// because GetType(void) and Copy() are not implemented here, you must derive a class an instantiate that class, SXExtensionActionable is still purely virtual
		void CopyActions(SXExtension *SXCopyFrom);
		unsigned long SaveActions(FILE *ffile);
		// Action load functionality is basically in SXQueryAct->Load
		// and is called from Load method of base class SXExtensionNVE or SXExtensionGE
		virtual void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID);
		virtual int InitToExport(const char *OutputFilePath);
		virtual void CleanupFromExport(SceneExporter *MyExporter);
		virtual void CloseQueryFiles(void);
		virtual bool AcceptDragDrop(long DropID);
		virtual int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource, int GenNotify, RasterAnimHost *NotifyObj);

		SXQueryAction *GetActionList(void)	{return (ActionList);};
		SXQueryAction *AddQueryAction(SXQueryAction *CopyFrom);
		SXQueryAction *RemoveQueryAction(SXQueryAction *RemoveMe);
		SXQueryAction *VerifyActiveAction(SXQueryAction *VerifyMe);

		int RemoveRAHost(RasterAnimHost *RemoveMe, RasterAnimHost *NotifyObj);
		void RemoveActions(void);
		void SetActiveAction(SXQueryAction *NewAction)	{ActiveAction = NewAction;};

		virtual bool GetSupportsQueryAction(void) {return(true);};
		virtual bool ApproveActionAvailable(int TestType) = 0;	// force implementation
		virtual bool ApproveActionItemAvailable(int TestItem) = 0;	// force implementation
		virtual char GetDefaultActionType(void) = 0;
	}; // class SXExtensionActionable


class SXExtensionNVE : public SXExtensionActionable
	{
	public:
		double NavFollowTerrainHeight, NavFollowTerrainMaxHeight, NavFriction, NavSpeed, NavAcceleration, NavInertia;
		long OverlayNumLines, LODMinFeatureSizePixels, LODMaxFoliageStems;
		char *OverlayLogoText, *WatermarkText, *MetaName, *MetaCopyright, *MetaAuthor, *MetaEmail, *MetaUser1,
			*MetaUser2, *MetaUser3, *MetaUser4, *MetaUser5;
		char NavStyle, NavConstrain, NavMaxHtConstrain, NavUseDefaultSpeed, OverlayShowMap, LODCompressTerrainTex, LODCompressFoliageTex, LODOptimizeMove;
		PathAndFile OverlayLogoFileName;
		
		SXExtensionNVE();
		virtual ~SXExtensionNVE();
		virtual int GetType(void)	{return(WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE);};
		virtual void Copy(SXExtension *SXCopyFrom);
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		void RemoveStrings(void);
		char *CreateString(char **StringLoc, char *NewString);
		int RemoveRAHost(RasterAnimHost *RemoveMe, RasterAnimHost *NotifyObj);
		virtual bool ApproveActionAvailable(int TestType);
		virtual bool ApproveActionItemAvailable(int TestItem);
		virtual char GetDefaultActionType(void);
	}; // class SXExtensionNVE

class SXExtensionFBX : public SXExtension
	{
	public:
		char SaveV5, SaveBinary, EmbedMedia, UsePassword;
		char Password[32];
		
		SXExtensionFBX();
		virtual ~SXExtensionFBX();
		virtual int GetType(void)	{return(WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX);};
		virtual void Copy(SXExtension *SXCopyFrom);
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void SetPassword(char *NewPassword);
	}; // class SXExtensionFBX

class SXExtensionGE : public SXExtensionActionable
	{
	public:
		double FoliageRescale, LabelRescale, overlayX, overlayY;
		PathAndFile overlayFilename;
		char Reverse3DNormals, DrawOrder;
		char Message[81];
		
		SXExtensionGE();
		virtual ~SXExtensionGE();
		virtual int GetType(void)	{return(WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE);};
		virtual void Copy(SXExtension *SXCopyFrom);
		virtual unsigned long Save(FILE *ffile);
		virtual unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void SetMessage(char *NewMessage);
		virtual bool ApproveActionAvailable(int TestType);
		virtual bool ApproveActionItemAvailable(int TestItem);
		virtual char GetDefaultActionType(void);
		virtual void CleanupFromExport(SceneExporter *MyExporter);
	}; // class SXExtensionGE


#endif // WCS_SXEXTENSION_H
