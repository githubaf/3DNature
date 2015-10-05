// Lists.cpp
// Source file for all sorts of lists used by VNS
// Built from parts of EffectsLib.cpp on 3/7/06 by GRH
// Copyright 2006 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "Lists.h"
#include "AppMem.h"
#include "UsefulSwap.h"
#include "Joe.h"
#include "EffectsLib.h"

#ifdef _DEBUG
unsigned long NumberOfEffectLists;
#endif // _DEBUG

QuantaAllocatorPool *EffectJoeList::MyPool;
QuantaAllocatorPool *TfxDetail::MyPool;

JoeList::JoeList(void)
{

Me = NULL;
Next = Prev = NULL;

} // JoeList::JoeList

/*===========================================================================*/

RenderJoeList *RenderJoeList::SortByAbsArea(unsigned char SortOrder)
{
short MoreToSort = 1;
long NumJLs = 0, JLCt, JLCtLimit;
RenderJoeList *JL, *HeadChain = this;
struct RenderJoeListSort *Sordid;

// create an array of RJL ptrs with associated area variable to sort by
// count elements
JL = this;
while (JL)
	{
	NumJLs ++;
	JL = (RenderJoeList *)JL->Next;
	} // while

if (NumJLs > 1)
	{
	if (Sordid = (struct RenderJoeListSort *)AppMem_Alloc(NumJLs * sizeof (struct RenderJoeListSort), 0))
		{
		JLCt = 0;
		JL = this;
		while (JL)
			{
			Sordid[JLCt].Me = JL;
			Sordid[JLCt].Area = JL->Area;
			JLCt ++;
			JL = (RenderJoeList *)JL->Next;
			} // while

		// qsort list by area according to the appropriate comparison rule
		if (SortOrder == WCS_JOELIST_SORTORDER_HILO)
			{
			qsort((void *)Sordid, (size_t)NumJLs, (size_t)(sizeof (struct RenderJoeListSort)), CompareRenderJoeListSortHiLo);
			} // if
		else
			{
			qsort((void *)Sordid, (size_t)NumJLs, (size_t)(sizeof (struct RenderJoeListSort)), CompareRenderJoeListSortLoHi);
			} // else

		JLCtLimit = NumJLs - 1;
		for (JLCt = 0; JLCt < JLCtLimit; JLCt ++)
			{
			Sordid[JLCt].Me->Next = Sordid[JLCt + 1].Me;
			} // for
		Sordid[JLCtLimit].Me->Next = NULL;
		HeadChain = Sordid[0].Me;

		AppMem_Free(Sordid, NumJLs * sizeof (struct RenderJoeListSort));
		} // if
	} // if

return (HeadChain);

} // RenderJoeList::SortByAbsArea

/*===========================================================================*/

int CompareRenderJoeListSortHiLo(const void *elem1, const void *elem2)
{

return (
	fabs(((struct RenderJoeListSort *)elem1)->Area) < fabs(((struct RenderJoeListSort *)elem2)->Area) ? 1:
	(fabs(((struct RenderJoeListSort *)elem1)->Area) > fabs(((struct RenderJoeListSort *)elem2)->Area) ? -1: 0)
	);

} // CompareRenderJoeListSortHiLo

/*===========================================================================*/

int CompareRenderJoeListSortLoHi(const void *elem1, const void *elem2)
{

return (
	fabs(((struct RenderJoeListSort *)elem1)->Area) > fabs(((struct RenderJoeListSort *)elem2)->Area) ? 1:
	(fabs(((struct RenderJoeListSort *)elem1)->Area) < fabs(((struct RenderJoeListSort *)elem2)->Area) ? -1: 0)
	);

} // CompareRenderJoeListSortLoHi

/*===========================================================================*/

void RenderJoeList::SortPriority(unsigned char SortOrder)
{
short MoreToSort = 1;
RenderJoeList *NextJL, *JL;

if (SortOrder == WCS_JOELIST_SORTORDER_HILO)
	{
	while (MoreToSort)
		{
		MoreToSort = 0;
		JL = this;
		NextJL = (RenderJoeList *)this->Next;
		while (NextJL)
			{
			if (NextJL->Priority > JL->Priority)
				{
				JL->Swap(NextJL);
				MoreToSort = 1;
				} // if
			JL = NextJL;
			NextJL = (RenderJoeList *)NextJL->Next;
			} // while
		} // while
	} // if
else
	{
	while (MoreToSort)
		{
		MoreToSort = 0;
		JL = this;
		NextJL = (RenderJoeList *)this->Next;
		while (NextJL)
			{
			if (NextJL->Priority < JL->Priority)
				{
				JL->Swap(NextJL);
				MoreToSort = 1;
				} // if
			JL = NextJL;
			NextJL = (RenderJoeList *)NextJL->Next;
			} // while
		} // while
	} // else

} // RenderJoeList::SortPriority

/*===========================================================================*/

void RenderJoeList::SortEvalOrder(unsigned char SortOrder)
{
short MoreToSort = 1;
RenderJoeList *NextJL, *JL;

if (SortOrder == WCS_JOELIST_SORTORDER_HILO)
	{
	while (MoreToSort)
		{
		MoreToSort = 0;
		JL = this;
		NextJL = (RenderJoeList *)this->Next;
		while (NextJL)
			{
			if (NextJL->Priority == JL->Priority)
				{
				if (NextJL->EvalOrder > JL->EvalOrder)
					{
					JL->Swap(NextJL);
					MoreToSort = 1;
					} // if
				} // if
			JL = NextJL;
			NextJL = (RenderJoeList *)NextJL->Next;
			} // while
		} // while
	} // if
else
	{
	while (MoreToSort)
		{
		MoreToSort = 0;
		JL = this;
		NextJL = (RenderJoeList *)this->Next;
		while (NextJL)
			{
			if (NextJL->Priority == JL->Priority)
				{
				if (NextJL->EvalOrder < JL->EvalOrder)
					{
					JL->Swap(NextJL);
					MoreToSort = 1;
					} // if
				} // if
			JL = NextJL;
			NextJL = (RenderJoeList *)NextJL->Next;
			} // while
		} // while
	} // else

} // RenderJoeList::SortEvalOrder

/*===========================================================================*/

void RenderJoeList::Swap(RenderJoeList *SwapMe)
{

/***
swmem(&Me, &SwapMe->Me, sizeof (Joe *));
swmem(&Drawn, &SwapMe->Drawn, sizeof (short));
swmem(&Priority, &SwapMe->Priority, sizeof (short));
swmem(&EvalOrder, &SwapMe->EvalOrder, sizeof (short));
swmem(&Area, &SwapMe->Area, sizeof (double));
swmem(&Effect, &SwapMe->Effect, sizeof (GeneralEffect *));
***/

SwapV((void **)&Me, (void **)&SwapMe->Me);
Swap16S(Drawn, SwapMe->Drawn);
Swap16S(Priority, SwapMe->Priority);
Swap16S(EvalOrder, SwapMe->EvalOrder);
Swap64(Area, SwapMe->Area);
SwapV((void **)&Effect, (void **)&SwapMe->Effect);

} // RenderJoeList::Swap

/*===========================================================================*/

Joe *JoeList::SimpleContained(double Lat, double Lon)
{
JoeList *JL;
Joe *Found = NULL;

JL = this;
while (JL)
	{
	if (JL->Me->SimpleContained(Lat, Lon))
		{
		Found = JL->Me;
		break;
		} // if
	JL = JL->Next;
	} // while

return (Found);

} // JoeList::SimpleContained

/*===========================================================================*/
/*===========================================================================*/

EffectJoeList::EffectJoeList(GeneralEffect *NewEffect, Joe *NewJoe)
{

MyEffect = NewEffect; 
MyJoe = NewJoe; 
VSData = NULL; 
Next = NULL;
#ifdef _DEBUG
++NumberOfEffectLists;
#endif // _DEBUG

} // EffectJoeList::EffectJoeList

/*===========================================================================*/

EffectJoeList::EffectJoeList(EffectJoeList *CopyFrom)
{

MyEffect = CopyFrom->MyEffect;
MyJoe = CopyFrom->MyJoe;
VSData = NULL;
if (CopyFrom->VSData)
	CopyTfxDetails(CopyFrom->VSData);
Next = NULL;
#ifdef _DEBUG
++NumberOfEffectLists;
#endif // _DEBUG

} // EffectJoeList::EffectJoeList

/*===========================================================================*/

EffectJoeList::~EffectJoeList()
{

RemoveTfxDetails();

} // EffectJoeList::~EffectJoeList

/*===========================================================================*/

void EffectJoeList::RemoveTfxDetails(void)
{

for (TfxDetail *CurVS = VSData; CurVS; CurVS = VSData)
	{
	VSData = VSData->Next;
	delete CurVS;
	} // for

} // EffectJoeList::RemoveTfxDetails

/*===========================================================================*/

bool EffectJoeList::Identical(EffectJoeList *CompareMe, bool &SameEffectAndVector)
{
bool Found;
TfxDetail *CurVS, *SearchVS;

SameEffectAndVector = false;

if (MyEffect != CompareMe->MyEffect)
	return (false);
if (MyJoe != CompareMe->MyJoe)
	return (false);

// we know this now
SameEffectAndVector = true;

if (VSData && ! CompareMe->VSData)
	return (false);
if (! VSData && CompareMe->VSData)
	return (false);

// compare both sets to each other so none are missed
for (CurVS = VSData; CurVS; CurVS = CurVS->Next)
	{
	Found = false;
	for (SearchVS = CompareMe->VSData; SearchVS; SearchVS = SearchVS->Next)
		{
		if (CurVS->Identical(SearchVS))
			{
			Found = true;
			break;
			} // if
		} // for
	if (! Found)
		return (false);
	} // for
for (CurVS = CompareMe->VSData; CurVS; CurVS = CurVS->Next)
	{
	Found = false;
	for (SearchVS = VSData; SearchVS; SearchVS = SearchVS->Next)
		{
		if (CurVS->Identical(SearchVS))
			{
			Found = true;
			break;
			} // if
		} // for
	if (! Found)
		return (false);
	} // for

return (true);

} // EffectJoeList::Identical

/*===========================================================================*/

void EffectJoeList::SortRenderOrder(EffectsLib *Lib)
{
int MoreToSort, EJLPriority, NextEJLPriority;
EffectJoeList *NextEJL, *EJL;

// sort by effect type first in the order they will be evaluated by the renderer
// sort by decreasing effect type precedence
MoreToSort = 1;
while (MoreToSort)
	{
	MoreToSort = 0;
	EJL = this;
	EJLPriority = Lib->GetEffectTypePrecedence(EJL->MyEffect->EffectType);
	NextEJL = this->Next;
	while (NextEJL)
		{
		NextEJLPriority = Lib->GetEffectTypePrecedence(NextEJL->MyEffect->EffectType);
		if (NextEJLPriority > EJLPriority)
			{
			EJL->Swap(NextEJL);
			NextEJLPriority = EJLPriority;
			MoreToSort = 1;
			} // if
		EJL = NextEJL;
		EJLPriority = NextEJLPriority;
		NextEJL = NextEJL->Next;
		} // while
	} // while

// sort by decreasing priority
MoreToSort = 1;
while (MoreToSort)
	{
	MoreToSort = 0;
	EJL = this;
	NextEJL = this->Next;
	while (NextEJL)
		{
		if (NextEJL->MyEffect->EffectType == EJL->MyEffect->EffectType)
			{
			if (NextEJL->MyEffect->Priority > EJL->MyEffect->Priority)
				{
				EJL->Swap(NextEJL);
				MoreToSort = 1;
				} // if
			} // if
		EJL = NextEJL;
		NextEJL = NextEJL->Next;
		} // while
	} // while

// sort by increasing evaluation order
MoreToSort = 1;
while (MoreToSort)
	{
	MoreToSort = 0;
	EJL = this;
	NextEJL = this->Next;
	while (NextEJL)
		{
		if (NextEJL->MyEffect->EffectType == EJL->MyEffect->EffectType && NextEJL->MyEffect->Priority == EJL->MyEffect->Priority)
			{
			// if it is a terraffector of one kind or another
			if (NextEJL->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_RASTERTA)
				{
				if (((RasterTerraffectorEffect *)NextEJL->MyEffect)->EvalOrder < ((RasterTerraffectorEffect *)EJL->MyEffect)->EvalOrder)
					{
					EJL->Swap(NextEJL);
					MoreToSort = 1;
					} // if
				} // if
			else if (NextEJL->MyEffect->EffectType == WCS_EFFECTSSUBCLASS_TERRAFFECTOR)
				{
				if (((TerraffectorEffect *)NextEJL->MyEffect)->EvalOrder < ((TerraffectorEffect *)EJL->MyEffect)->EvalOrder)
					{
					EJL->Swap(NextEJL);
					MoreToSort = 1;
					} // if
				} // else if
			} // if
		EJL = NextEJL;
		NextEJL = NextEJL->Next;
		} // while
	} // while

} // EffectJoeList::SortRenderOrder

/*===========================================================================*/

void EffectJoeList::Swap(EffectJoeList *SwapMe)
{

SwapV((void **)&MyJoe, (void **)&SwapMe->MyJoe);
SwapV((void **)&MyEffect, (void **)&SwapMe->MyEffect);
SwapV((void **)&VSData, (void **)&SwapMe->VSData);

} // EffectJoeList::Swap

/*===========================================================================*/

TfxDetail *EffectJoeList::SetTfxDetail(unsigned long SetIndex, short SetSegment, unsigned short SetFlags)
{
TfxDetail **VSDPtr;

for (VSDPtr = &VSData; *VSDPtr; VSDPtr = &(*VSDPtr)->Next)
	{
	if ((*VSDPtr)->GetIndex() == SetIndex && 
		(*VSDPtr)->GetSegment() == SetSegment &&
		(*VSDPtr)->GetFlags() == SetFlags)
		return (*VSDPtr);
	} // for

return (*VSDPtr = new TfxDetail(SetIndex, SetSegment, SetFlags));

} // EffectJoeList::SetTfxDetail

/*===========================================================================*/

TfxDetail *EffectJoeList::SetTfxDetail(TfxDetail *SegmentSource)
{

if (SegmentSource)
	{
	return (SetTfxDetail(SegmentSource->GetIndex(), SegmentSource->GetSegment(), SegmentSource->GetFlags()));
	} // if

return (NULL);

} // EffectJoeList::SetTfxDetail

/*===========================================================================*/

TfxDetail *EffectJoeList::CopyTfxDetails(TfxDetail *TfxDetailSource)
{

for (TfxDetail *CurTfxDetail = TfxDetailSource; CurTfxDetail; CurTfxDetail = CurTfxDetail->Next)
	{
	if (! SetTfxDetail(CurTfxDetail))
		return (NULL);
	} // for

return (VSData);

} // EffectJoeList::CopyTfxDetails

/*===========================================================================*/

bool EffectJoeList::SupplementTfxDetails(EffectJoeList *CompareMe)
{

return (CopyTfxDetails(CompareMe->GetTfxDetail()) ? true: false);

} // EffectJoeList::SupplementTfxDetails

/*===========================================================================*/
