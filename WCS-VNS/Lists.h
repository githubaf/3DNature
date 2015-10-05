// Lists.h
// Header file for all sorts of lists used by VNS
// Built from parts of EffectsLib.h on 3/7/06 by GRH
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_LISTS_H
#define WCS_LISTS_H

#include "QuantaAllocator.h"

class GeneralEffect;
class GeoRefShell;
class Joe;
class EffectsLib;
class TfxDetail;
class VectorPoint;
class VectorNode;
class EcosystemEffect;
class RenderData;
class RasterAnimHost;

class RasterAnimHostList
	{
	public:
		RasterAnimHost *Me;
		RasterAnimHostList *Next;

		RasterAnimHostList()	{Next = NULL; Me = NULL;};

	}; // class RasterAnimHostList

class RasterAnimHostBooleanList : public RasterAnimHostList
	{
	public:
		char TrueFalse;

		RasterAnimHostBooleanList()	{TrueFalse = 0;};

	}; // class RasterAnimHostBooleanList

class EffectList
	{
	public:
		GeneralEffect *Me;
		EffectList *Next;

		EffectList() {Me = NULL; Next = NULL;};
		GeneralEffect *GetEffect(void)	{return (Me);};
		void SetEffect(GeneralEffect *NewEffect)	{Me = NewEffect;};
	}; // class EffectList

class ImageList
	{
	public:
		GeoRefShell *Me;
		ImageList *Next;

		ImageList() {Me = NULL; Next = NULL;};
	}; // class ImageList

class NumberList
	{
	public:
		NumberList *Next;
		unsigned long Number;
		char TrueFalse;

		NumberList()	{TrueFalse = 0; Number = 0; Next = NULL;};
		NumberList(unsigned long NewNumber)	{TrueFalse = 0; Number = NewNumber; Next = NULL;};
	}; // class NumberList

class JoeList
	{
	public:
		Joe *Me;
		JoeList *Prev, *Next;

		JoeList();
		Joe *SimpleContained(double Lat, double Lon);
		Joe *GetVector(void)	{return (Me);};
		void SetVector(Joe *NewVector)	{Me = NewVector;};
	}; // JoeList

class RenderJoeList: public JoeList
	{
	public:
		double Area;
		GeneralEffect *Effect;
		short Priority, EvalOrder, Drawn;

		RenderJoeList()	{Effect = NULL; Priority = EvalOrder = Drawn = 0; Area = 0.0;};
		RenderJoeList *SortByAbsArea(unsigned char SortOrder);
		void SortPriority(unsigned char SortOrder);
		void SortEvalOrder(unsigned char SortOrder);
		void Swap(RenderJoeList *SwapMe);
		GeneralEffect *GetEffect(void)	{return (Effect);};
	}; // class RenderJoeList

class EffectJoeList
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(EffectJoeList), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

		Joe *MyJoe;
		GeneralEffect *MyEffect;
		TfxDetail *VSData;
		EffectJoeList *Next;

		EffectJoeList()	{MyJoe = NULL; MyEffect = NULL; VSData = NULL; Next = NULL;};
		EffectJoeList(GeneralEffect *NewEffect, Joe *NewJoe);
		EffectJoeList(EffectJoeList *CopyFrom);
		~EffectJoeList();
		void SortRenderOrder(EffectsLib *Lib);
		void Swap(EffectJoeList *SwapMe);
		GeneralEffect *GetEffect(void)	{return (MyEffect);};
		TfxDetail *GetTfxDetail(void)	{return (VSData);};
		Joe *GetVector(void)	{return (MyJoe);};
		TfxDetail *SetTfxDetail(unsigned long SetPointIndex, short SetSegment, unsigned short SetFlags);
		TfxDetail *SetTfxDetail(TfxDetail *TfxDetail);
		TfxDetail *CopyTfxDetails(TfxDetail *TfxDetailSource);
		void RemoveTfxDetails(void);
		bool Identical(EffectJoeList *CompareMe, bool &SameEffectAndVector);
		bool SupplementTfxDetails(EffectJoeList *CompareMe);
	}; // class EffectJoeList

#define WCS_TFXDETAIL_SLOPESEGMENT	(-1)

#define WCS_TFXDETAIL_FLAG_SINGLENODE	(1 << 0)
#define WCS_TFXDETAIL_FLAG_TRAILINGNODE	(1 << 1)
#define WCS_TFXDETAIL_FLAG_LEADINGNODE	(1 << 2)

class TfxDetail
	{
	// begin QuantaAllocator support
	private:
		static QuantaAllocatorPool *MyPool; // we don't clear MyPool in the constructor, because we don't want to tamper with the constructor. It should come up as 0 to start because it's static
	public:
		// customize the second sizeof in the CreatePool call (below) to specify the block alignment
		static bool PrepareAllocation(QuantaAllocator *QA, unsigned long NewPoolInitialUnits, unsigned long NewPoolGrowByUnits) 
			{MyPool = QA->CreatePool(sizeof(TfxDetail), NewPoolInitialUnits, NewPoolGrowByUnits, sizeof(unsigned long int)); return(MyPool != NULL);};
		static void CleanupAllocation(void) {MyPool = NULL;}; // probably not really needed, doesn't do much, resources only freed when the QuantaAllocator is destroyed anyway
		void *operator new(size_t _Count) {return(MyPool->AttemptAllocateNewItem(true));};
		void operator delete(void *_Ptr) {MyPool->FreeItem(_Ptr);};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolCurrentItemsInUse(void) {return(MyPool->GetPoolCurrentItemsInUse());};
		static WCS_QUANTAALLOCATOR_BIGSIZE_TYPE GetPoolMaxItemsInUse(void) {return(MyPool->GetPoolMaxItemsInUse());};
	// end QuantaAllocator support

	public:
		unsigned long Index;
		TfxDetail *Next;
		short XSectSegment;
		unsigned short Flags;

		TfxDetail()	{Index = 0; Next = NULL; XSectSegment = 0; Flags = 0;};
		TfxDetail(unsigned long SetIndex, short SetSegment, unsigned short SetFlags)
			{Index = SetIndex; XSectSegment = SetSegment; Flags = SetFlags; Next = NULL;};
		unsigned long GetIndex(void)	{return (Index);};
		short GetSegment(void)	{return (XSectSegment);};
		unsigned short GetFlags(void)	{return (Flags);};
		bool Identical(TfxDetail *CompareMe)	{return (Index == CompareMe->Index
			&& XSectSegment == CompareMe->XSectSegment && Flags == CompareMe->Flags);};
		void FlagSet(unsigned short NewFlags)	{Flags |= NewFlags;};
		void FlagClear(unsigned short ClearFlags)	{Flags &= ~ClearFlags;};
		unsigned short FlagCheck(unsigned short CheckFlags)	{return (Flags & CheckFlags);};
	}; // class TfxDetail

#define WCS_JOELIST_SORTORDER_HILO	0
#define WCS_JOELIST_SORTORDER_LOHI	1

struct RenderJoeListSort
	{
	double Area;
	RenderJoeList *Me;
	}; // struct RenderJoeListSort

// in Lists.cpp
int CompareRenderJoeListSortHiLo(const void *elem1, const void *elem2);
int CompareRenderJoeListSortLoHi(const void *elem1, const void *elem2);

#endif // WCS_LISTS_H
