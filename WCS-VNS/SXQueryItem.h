// SXQueryItem.h
// Header file for SceneExporter Click-to-Query SXQueryItem.
// Built from scratch on 12/30/04 by Gary R. Huber
// Copyright 2004 3D Nature LLC. All rights reserved.


#ifndef WCS_SXQUERYITEM_H
#define WCS_SXQUERYITEM_H

#include "stdafx.h"

class SXQueryAction;
class Joe;
class GeneralEffect;
class NameList;

enum
	{
	WCS_SXQUERYITEM_FREQ_UNKNOWN,
	WCS_SXQUERYITEM_FREQ_NORECORD,
	WCS_SXQUERYITEM_FREQ_PERACTION,
	WCS_SXQUERYITEM_FREQ_PERCOMPONENT,
	WCS_SXQUERYITEM_FREQ_PERVECTOR,
	WCS_SXQUERYITEM_FREQ_PERVECTORPERCOMPONENT,
	WCS_SXQUERYITEM_FREQ_PERINSTANCE
	};

class SXQueryActionList
	{
	public:
		long RecordNum;
		char RecordFreq;
		SXQueryAction *QueryAction;
		SXQueryActionList *Next;

		SXQueryActionList(SXQueryAction *ActionSource);

	}; // class SXQueryActionList

class SXQueryVectorList
	{
	public:
		long RecordNum;
		char RecordFreq;
		Joe *QueryVector;
		SXQueryVectorList *Next;

		SXQueryVectorList(Joe *VectorSource);
	}; // class SXQueryVectorList

class SXQueryEffectList
	{
	public:
		long RecordNum;
		char RecordFreq;
		GeneralEffect *QueryEffect;
		SXQueryEffectList *Next;

		SXQueryEffectList(GeneralEffect *EffectSource);
	}; // class SXQueryEffectList

class SXQueryVectorEffectList
	{
	public:
		long RecordNum;
		char RecordFreq;
		GeneralEffect *QueryEffect;
		Joe *QueryVector;
		SXQueryVectorEffectList *Next;

		SXQueryVectorEffectList(GeneralEffect *EffectSource, Joe *VectorSource);
	}; // class SXQueryVectorEffectList

class SXQueryItem
	{
	public:
		bool ClickQueryEnabled;
		long RecordNum;
		char RecordFreq;
		char NumActions;
		SXQueryActionList *ActionList;
		SXQueryVectorList *VectorList;
		static long ObjectFileID, ActionFileID;
		static FILE *ObjectFile, *ObjectFileRecords;
		static const char *OutputFilePath;

		SXQueryItem();
		~SXQueryItem();
		void DestroyVectorList(void);
		void DestroyActionList(void);
		bool IsClickQueryEnabled(void)	{return(ClickQueryEnabled);};
		void SetClickQueryEnabled(bool SetTo)	{ClickQueryEnabled = SetTo;};
		long GetRecordNumber(GeneralEffect *MyEffect, Joe *MyVector, NameList **FileNamesCreated);
		int SXQuerySetupForExport(SXQueryAction *AddAction, const char *OutputFilePathSource);
		void SXQueryCleanupFromExport(void);
		SXQueryVectorList *AddVectorList(Joe *AddVector);
		SXQueryVectorList *SearchVectorList(Joe *SearchVector);
		void DetermineRecordFrequency(void);
		int OpenObjectFile(NameList **FileNamesCreated);
		void CloseObjectFile(void);
		NameList *AddNewNameList(NameList **Names, char *NewName, long FileType);

	}; // class SXQueryItem

#endif // WCS_SXQUERYITEM_H
