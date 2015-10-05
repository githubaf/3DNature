// IncludeExcludeList.h
// Header file for IncludeExcludeList.cpp
// Built from scratch on 2/13/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#ifndef WCS_INCLUDEEXCLUDE_H
#define WCS_INCLUDEEXCLUDE_H

#include "Lists.h"
#include "RasterAnimHost.h"

#define WCS_SUBCLASS_INCLUDEEXCLUDE  144

class EffectsLib;

class NameList
	{
	public:
		char *Name;
		long ItemClass;
		NameList *Next;

		NameList()	{ItemClass = 0; Name = NULL; Next = NULL;};
		NameList(char *NewName, long NewClass);
		~NameList();
		const char *FindNameOfType(long SearchClass);
		const char *FindNextNameOfType(long SearchClass, const char *CurName);
		int FindNameExists(long SearchClass, const char *CurName);

	}; // class NameList

class IncludeExcludeList : public RasterAnimHost
	{
	public:
		char Include, Enabled;
		RasterAnimHostList *RAHList;
		NameList *ItemNames;

		IncludeExcludeList()	{Include = 0; Enabled = 0; RAHList = NULL; ItemNames = NULL;};
		IncludeExcludeList(RasterAnimHost *RAHost);
		~IncludeExcludeList();
		void DeleteItemList(void);
		void DeleteNameList(void);
		void Copy(IncludeExcludeList *CopyTo, IncludeExcludeList *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void ResolveLoadLinkages(EffectsLib *Lib);
		int PassTest(RasterAnimHost *TestMe);
		RasterAnimHostList *AddRAHost(RasterAnimHost *AddMe);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_INCLUDEEXCLUDE);};
		unsigned long GetRAFlags(unsigned long Mask = ~0);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_INCLUDEEXCLUDE);};
		virtual char *GetRAHostTypeString(void) {return ("(Include/Exclude List)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);	// return 0 means user cancelled

	}; // class IncludeExcludeList

// file tags
#define WCS_INCLUDEEXCLUDE_ENABLED		0x00210000
#define WCS_INCLUDEEXCLUDE_INCLUDE		0x00220000
#define WCS_INCLUDEEXCLUDE_ITEMCLASS		0x00230000
#define WCS_INCLUDEEXCLUDE_ITEMNAME		0x00240000


#endif // WCS_INCLUDEEXCLUDE_H
