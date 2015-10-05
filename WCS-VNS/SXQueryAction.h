// SXQueryAction.h
// Header file for SceneExporter Click-to-Query Action.
// Built from scratch on 12/10/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.

#include "stdafx.h"

#ifndef WCS_SXQUERYACTION_H
#define WCS_SXQUERYACTION_H

class RasterAnimHostList;
class NumberList;
class Database;
class Joe;
class EffectsLib;
class RasterAnimHost;
class RasterAnimHostProperties;
class GeneralEffect;
class SXQueryVectorList;
class SXQueryEffectList;
class SXQueryVectorEffectList;
class NameList;
class SearchQuery;

enum
	{
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE,
	WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET, // or Highlight Object, perhaps
	WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE,
	WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND, // NVX only
	WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN, // NVX only
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL,
	WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE,
	WCS_SXQUERYACTION_NUMACTIONTYPES
	}; // ActionTypes


class SXQueryAction
	{
	public:
		char *ActionText;
		const char *OutputFilePath;
		RasterAnimHostList *Items;
		NumberList *DBItemNumbers;
		SXQueryVectorList *VectorList;
		SXQueryEffectList *EffectList;
		SXQueryVectorEffectList *VectorEffectList;
		SXQueryAction *Next;
		static FILE *ActionFileRecords;
		static FILE *ActionFile;
		long RecordNum;
		char ActionType, RecordFreq;


		SXQueryAction();
		~SXQueryAction();
		void Copy(SXQueryAction *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		void SetActionType(char NewType)	{if (NewType >= 0 && NewType < WCS_SXQUERYACTION_NUMACTIONTYPES) ActionType = NewType;};
		void RemoveStrings(void);
		void RemoveItems(void);
		void RemoveDBItemNumbers(void);
		char *CreateString(char *NewString);
		void ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID);
		RasterAnimHostList *AddItem(RasterAnimHost *AddMe, int GenNotify, RasterAnimHost *NotifyObj);
		long AddDBItem(RasterAnimHost *NotifyObj);
		long AddItemsByClass(EffectsLib *HostEffects, Database *HostDB, long ItemClass, RasterAnimHost *NotifyObj);
		long AddItemsByQuery(SearchQuery *TestMe, int GenNotify, RasterAnimHost *NotifyObj);
		int RemoveRAHost(RasterAnimHost *RemoveMe, RasterAnimHost *NotifyObj);
		int InitToExport(const char *OutputFilePathSource);
		void CleanupFromExport(void);
		bool AcceptItemType(long DropID);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource, int GenNotify, RasterAnimHost *NotifyObj);
		const char *GetActionText()	{return (ActionText);};
		char GetRecordFrequency(void);
		long GetRecordNumber(GeneralEffect *MyEffect, Joe *MyVector, long &ActionFileID, NameList **FileNamesCreated);
		bool TestTextForInstanceInfo(const char *TestText);
		bool TestTextForVectorInfo(const char *TestText);
		bool TestTextForComponentInfo(const char *TestText);
		int CreateActionRecord(GeneralEffect *MyEffect, Joe *MyVector, long &ActionFileID, NameList **FileNamesCreated);
		int WriteActionRecord(char *ActionString, NameList **FileNamesCreated);
		void DestroyVectorList(void);
		SXQueryVectorList *AddVectorList(Joe *AddVector);
		SXQueryVectorList *SearchVectorList(Joe *SearchVector);
		SXQueryEffectList *AddEffectList(GeneralEffect *AddEffect);
		SXQueryEffectList *SearchEffectList(GeneralEffect *SearchEffect);
		SXQueryVectorEffectList *AddVectorEffectList(GeneralEffect *AddEffect, Joe *AddVector);
		SXQueryVectorEffectList *SearchVectorEffectList(GeneralEffect *SearchEffect, Joe *SearchVector);
		int OpenActionFile(NameList **FileNamesCreated);
		void CloseActionFile(void);
		void ReplaceStringTokens(char *MesgCopy, const char *OrigMesg, GeneralEffect *CurEffect, Joe *CurVec);
		NameList *AddNewNameList(NameList **Names, char *NewName, long FileType);

	}; // class SXQueryAction;

#define WCS_EFFECTS_SXACTIONQUERY_ACTIONTYPE		0x00210000
#define WCS_EFFECTS_SXACTIONQUERY_ACTIONTEXT		0x00220000
#define WCS_EFFECTS_SXACTIONQUERY_EFFECTTYPE		0x00230000
#define WCS_EFFECTS_SXACTIONQUERY_EFFECTNAME		0x00240000
#define WCS_EFFECTS_SXACTIONQUERY_JOENUMBER			0x00250000


#endif // WCS_SXQUERYACTION_H
