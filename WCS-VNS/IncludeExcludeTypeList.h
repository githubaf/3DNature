// IncludeExcludeTypeList.h
// Header file for IncludeExcludeTypeList.cpp
// Built from scratch on 3/23/02 by Gary R. Huber
// Copyright 2002 Questar Productions. All rights reserved.

#ifndef WCS_INCLUDEEXCLUDETYPE_H
#define WCS_INCLUDEEXCLUDETYPE_H

#include "RasterAnimHost.h"
#include "PixelManager.h"

#define WCS_SUBCLASS_INCLUDEEXCLUDETYPE  145

class TypeList
	{
	public:
		char TypeNumber;
		TypeList *Next;

		TypeList()	{TypeNumber = 0; Next = NULL;};
		TypeList(char NewType);

	}; // class TypeList

class IncludeExcludeTypeList : public RasterAnimHost
	{
	public:
		char Include, Enabled;
		TypeList *ItemTypes;

		IncludeExcludeTypeList()	{Include = 0; Enabled = 0; ItemTypes = NULL;};
		IncludeExcludeTypeList(RasterAnimHost *RAHost);
		~IncludeExcludeTypeList();
		static char *GetTypeName(unsigned char MatchNumber);
		void DeleteTypeList(void);
		void Copy(IncludeExcludeTypeList *CopyTo, IncludeExcludeTypeList *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int PassTest(unsigned char TestMe);
		TypeList *AddType(unsigned char AddMe);
		int RemoveType(unsigned char RemoveMe);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_INCLUDEEXCLUDETYPE);};
		unsigned long GetRAFlags(unsigned long Mask = ~0);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_INCLUDEEXCLUDETYPE);};
		virtual char *GetRAHostTypeString(void) {return ("(Include/Exclude Type List)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};

	}; // class IncludeExcludeTypeList

#define WCS_INCLEXCLTYPE_TERRAIN	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN)
#define WCS_INCLEXCLTYPE_SNOW		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SNOW)
#define WCS_INCLEXCLTYPE_WATER		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_WATER)
#define WCS_INCLEXCLTYPE_SKY		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SKY)
#define WCS_INCLEXCLTYPE_CLOUD		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD)
#define WCS_INCLEXCLTYPE_CELESTIAL	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CELESTIAL)
#define WCS_INCLEXCLTYPE_STAR		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_STAR)
#define WCS_INCLEXCLTYPE_3DOBJECT	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT)
#define WCS_INCLEXCLTYPE_FOLIAGE	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE)
#define WCS_INCLEXCLTYPE_FENCE		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE)
#define WCS_INCLEXCLTYPE_BACKGROUND	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_BACKGROUND)
#define WCS_INCLEXCLTYPE_POSTPROC	(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC)
#define WCS_INCLEXCLTYPE_VECTOR		(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR)

// file tags
#define WCS_INCLUDEEXCLUDETYPE_ENABLED		0x00210000
#define WCS_INCLUDEEXCLUDETYPE_INCLUDE		0x00220000
#define WCS_INCLUDEEXCLUDE_TYPENUMBER		0x00230000


#endif // WCS_INCLUDEEXCLUDETYPE_H
