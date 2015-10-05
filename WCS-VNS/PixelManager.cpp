// PixelManager.cpp
// sample code for managing bitmaps composed of multiple pixel fragments
// Written from scratch on 7/5/99 by Gary R. Huber
// Modified for use in WCS 4/2/00 by GRH.
// Copyright 1999-2000 by Questar Productions. All rights reserved.

#include "stdafx.h"

// calloc, realloc and free may be causing memory leakage.
// Try using WCS memory functions and see if there is a performance hit
//#define WCS_PIXELMGER_USE_CALLOC

#ifdef WCS_PIXELMGER_USE_CALLOC
#include "malloc.h"
#include "memory.h"
#else
#include "AppMem.h"
#endif // WCS_PIXELMGER_USE_CALLOC

#include "PixelManager.h"
#include "Raster.h"
#include "Render.h"
#include "Useful.h"
#include "EffectsLib.h"

#define	min(a, b)	(((a) < (b)) ? (a) : (b))

int FragMemFailed;

//rPixelFragment **rPixelHeader::FragLists;
//long rPixelHeader::NumFragListEntries;
//long rPixelHeader::LastListUsed;
//unsigned long rPixelHeader::LastFragUsed;
//unsigned long *rPixelHeader::FragsPerList;
//ReflectionData **rPixelHeader::ReflLists;
//long rPixelHeader::NumReflListEntries;
//long rPixelHeader::LastReflListUsed;
//unsigned long rPixelHeader::LastReflUsed;
//unsigned long *rPixelHeader::ReflsPerList;

/*===========================================================================*/

int CountBits_Std(unsigned int a)
{

#ifndef SPEED_MOD

a = (a & 0x55555555UL) + ((a >> 1) & 0x55555555UL);
a = (a & 0x33333333UL) + ((a >> 2) & 0x33333333UL);
a = (a & 0x07070707UL) + ((a >> 4) & 0x07070707UL);

return (a % 255);

#else // SPEED_MOD

/*** F2 enhanced

a = (a & 0x55555555UL) + ((a >> 1) & 0x55555555UL);	// mask odd & even bits, align & add
a = (a & 0x33333333UL) + ((a >> 2) & 0x33333333UL); // do same again with bit pairs
a = (a & 0x07070707UL) + ((a >> 4) & 0x07070707UL); // and again with nibbles

// at this point, each byte contains the count of the number of bits that were originally set in that byte

a = (a & 0x0000ffffUL) + (a >> 16);					// align high & low words & add
a = (a & 0x000000ffUL) + (a >> 8);					// align 3rd & 4th bytes & add

return a;

***/


// further optimization found on rec.puzzles via Google
// David Seal - dseal@armltd.co.uk
a = ((a >> 1) & 0x55555555) + (a & 0x55555555);
a = ((a >> 2) & 0x33333333) + (a & 0x33333333);
a = (a >> 4) + a;
a = ((a >> 8) & 0x000f000f) + (a & 0x000f000f);
a = ((a >> 16) + a) & 0x0000003f;

return a;

#endif // SPEED_MOD

} // CountBits

/*===========================================================================*/

#if _MSC_VER >= 1500
int CountBits_POPCNT(unsigned int a)
{

return (_mm_popcnt_u32(a));

} // CountBits_POPCNT
#endif // _MSC_VER

/*===========================================================================*/
/*===========================================================================*/

PixelFragment::PixelFragment()
{

Coverage = RGB[2] = RGB[1] = RGB[0] = 0;
ZBuf = FLT_MAX;

} // PixelFragment::PixelFragment

/*===========================================================================*/

PixelFragment::~PixelFragment()
{

if (WCS_PIXEL_MULTIFRAG)
	{
	PixelHeader *MyHdr = (PixelHeader *)this;

	if (MyHdr->FragList)
		#ifdef WCS_PIXELMGER_USE_CALLOC
		free(MyHdr->FragList);
		#else
		AppMem_Free(MyHdr->FragList, MyHdr->NumFrags[0]);
		#endif // WCS_PIXELMGER_USE_CALLOC
	} // if

} // PixelFragment::~PixelFragment

/*===========================================================================*/

rPixelBlockHeader *Raster::AllocPixelFragmentMap(void)
{

if (Rows > 0 && Cols > 0)
	rPixelBlock = new rPixelBlockHeader(Rows * Cols);

return (rPixelBlock);

} // Raster::AllocPixelFragmentMap

/*===========================================================================*/

void Raster::FreePixelFragmentMap(void)
{

if (rPixelBlock)
	delete rPixelBlock;
rPixelBlock = NULL;

} // Raster::FreePixelFragmentMap

/*===========================================================================*/

int PixelFragment::TestPossibleUsage(float NewZ)
{

if (! WCS_PIXEL_BUFFERFULL)
	return (1);

if (! WCS_PIXEL_MULTIFRAG)
	{
	if (NewZ <= ZBuf)
		return (1);
	} // if
else
	{
	PixelHeader *MyHdr = (PixelHeader *)this;

	if (NewZ <= MyHdr->FragList[MyHdr->NumFrags[1]].ZBuf)
		return (1);
	} // else

return (0);

} // PixelFragment::TestPossibleUsage

/*===========================================================================*/

float PixelFragment::GetNearestZ(void)
{

if (! WCS_PIXEL_MULTIFRAG)
	{
	return (ZBuf);
	} // if
else
	{
	PixelHeader *MyHdr = (PixelHeader *)this;

	return (MyHdr->FragList[0].ZBuf);
	} // else

} // PixelFragment::GetNearestZ

/*===========================================================================*/

// returns ptr to the fragment to plot into if the new pixel makes some contribution
// NearestZ will be filled in with the closest Z value of this pixel
PixelFragment *PixelFragment::PlotPixel(float NewZ, unsigned char NewCoverage)
{
PixelHeader *MyHdr = (PixelHeader *)this;
unsigned char SumCoverage, CumCoverage, LastFrag, Ct, Ct1;

if (! NewCoverage)
	return (NULL);	// no data

if (! WCS_PIXEL_MULTIFRAG)
	{
	if (NewZ < ZBuf)
		{
		if (! Coverage || (NewCoverage >= WCS_PIXEL_MAXCOVERAGE))
			{
			Coverage = NewCoverage;
			ZBuf = NewZ;
			if (NewCoverage >= WCS_PIXEL_MAXCOVERAGE)
				Coverage |= WCS_PIXEL_BUFFERFULLBITS;
			return (this);
			} // if this is the first fragment plotted to this pixel or new fragment replaces old fragment
		if (CreateMultiFragPixel(1))
			{
			MyHdr->FragList[0].Coverage = NewCoverage;
			MyHdr->FragList[0].ZBuf = NewZ;
			MyHdr->NumFrags[2] = MyHdr->FragList[0].Coverage + MyHdr->FragList[1].Coverage;
			if (MyHdr->NumFrags[2] >= WCS_PIXEL_MAXCOVERAGE)
				Coverage |= WCS_PIXEL_BUFFERFULLBITS;
			return (&MyHdr->FragList[0]);
			} // if
		return (NULL);	// failed to create multi-frag pixel
		} // if significantly closer, maybe first fragment plotted
	else if (NewZ > ZBuf)
		{
		if (WCS_PIXEL_COVERAGE < WCS_PIXEL_MAXCOVERAGE)
			{
			if (CreateMultiFragPixel(0))
				{
				MyHdr->FragList[1].Coverage = NewCoverage;
				MyHdr->FragList[1].ZBuf = NewZ;
				MyHdr->NumFrags[2] = MyHdr->FragList[0].Coverage + MyHdr->FragList[1].Coverage;
				if (MyHdr->NumFrags[2] >= WCS_PIXEL_MAXCOVERAGE)
					Coverage |= WCS_PIXEL_BUFFERFULLBITS;
				return (&MyHdr->FragList[1]);
				} // if
			} // if old fragment doesn't completely cover
		return (NULL);	// coverage is full or failed to create multi-frag pixel
		} // if
	else if (WCS_PIXEL_COVERAGE < WCS_PIXEL_MAXCOVERAGE)	// z values are identical, coverage not full
		{
		SumCoverage = Coverage + NewCoverage;
			
		Coverage = min(SumCoverage, WCS_PIXEL_MAXCOVERAGE);
		if (SumCoverage >= WCS_PIXEL_MAXCOVERAGE)
			Coverage |= WCS_PIXEL_BUFFERFULLBITS;
		} // else same distance
	return (NULL);	// don't modify color if same distance or coverage full
	} // if significantly farther
else
	{
	LastFrag = MyHdr->NumFrags[1];
	if (NewZ > MyHdr->FragList[LastFrag].ZBuf)
		{
		if (! WCS_PIXEL_BUFFERFULL)
			{
			LastFrag ++;
			if (LastFrag < 128 && (LastFrag < MyHdr->NumFrags[0] || (LastFrag = MyHdr->AddFragment() - 1)))
				{
				MyHdr->FragList[LastFrag].Coverage = NewCoverage;
				MyHdr->FragList[LastFrag].ZBuf = NewZ;
				MyHdr->NumFrags[1] = LastFrag;
				MyHdr->NumFrags[2] += NewCoverage;
				if (MyHdr->NumFrags[2] >= WCS_PIXEL_MAXCOVERAGE)
					Coverage |= WCS_PIXEL_BUFFERFULLBITS;
				return (&MyHdr->FragList[LastFrag]);
				} // if
			} // if
		return (NULL);	// buffer full or failed to add fragment
		} // if beyond last fragment
	else
		{
		CumCoverage = 0;
		for (Ct = 0; Ct <= LastFrag && CumCoverage < WCS_PIXEL_MAXCOVERAGE; Ct ++)
			{
			if (NewZ < MyHdr->FragList[Ct].ZBuf)
				{
				if (CumCoverage + NewCoverage >= WCS_PIXEL_MAXCOVERAGE)
					{
					MyHdr->FragList[Ct].Coverage = NewCoverage;
					MyHdr->FragList[Ct].ZBuf = NewZ;
					MyHdr->NumFrags[1] = Ct;
					MyHdr->NumFrags[2] = CumCoverage + NewCoverage;
					Coverage |= WCS_PIXEL_BUFFERFULLBITS;
					return (&MyHdr->FragList[Ct]);
					} // if new fragment replaces all farther away
				// need to insert a fragment
				if (LastFrag < 127)
					LastFrag ++;
				if (LastFrag < MyHdr->NumFrags[0] || (LastFrag = MyHdr->AddFragment() - 1))
					{
					memmove(&MyHdr->FragList[Ct + 1], &MyHdr->FragList[Ct], (LastFrag - Ct) * sizeof (PixelFragment));
					MyHdr->FragList[Ct].Coverage = NewCoverage;
					MyHdr->FragList[Ct].ZBuf = NewZ;
					CumCoverage += NewCoverage;
					Ct1 = Ct;
					while (CumCoverage < WCS_PIXEL_MAXCOVERAGE && Ct1 < LastFrag)
						{
						CumCoverage += MyHdr->FragList[++Ct1].Coverage;
						} // while
					MyHdr->NumFrags[1] = Ct1;
					MyHdr->NumFrags[2] = CumCoverage;
					if (MyHdr->NumFrags[2] >= WCS_PIXEL_MAXCOVERAGE)
						Coverage |= WCS_PIXEL_BUFFERFULLBITS;
					return (&MyHdr->FragList[Ct]);
					} // if
				return (NULL);	// failed to add fragment
				} // if new fragment is closer
			else if (NewZ == MyHdr->FragList[Ct].ZBuf)
				{
				SumCoverage = MyHdr->FragList[Ct].Coverage + NewCoverage;
					
				MyHdr->FragList[Ct].Coverage = min(SumCoverage, WCS_PIXEL_MAXCOVERAGE);
				CumCoverage += MyHdr->FragList[Ct].Coverage;
				Ct1 = Ct;
				while (CumCoverage < WCS_PIXEL_MAXCOVERAGE && Ct1 < LastFrag)
					{
					CumCoverage += MyHdr->FragList[++Ct1].Coverage;
					} // while
				MyHdr->NumFrags[1] = Ct1;
				MyHdr->NumFrags[2] = CumCoverage;
				if (MyHdr->NumFrags[2] >= WCS_PIXEL_MAXCOVERAGE)
					Coverage |= WCS_PIXEL_BUFFERFULLBITS;
				return (NULL);	// don't modify color if same distance
				} // else if the same distance
			CumCoverage += MyHdr->FragList[Ct].Coverage;
			} // for
		} // else
	} // else

return (NULL);

} // PixelFragment::PlotPixel

/*===========================================================================*/

PixelFragment *PixelFragment::CreateMultiFragPixel(int NewIsCloser)
{
PixelFragment *NewFragments;

#ifdef WCS_PIXELMGER_USE_CALLOC
if (NewFragments = (PixelFragment *)calloc(2, sizeof (PixelFragment)))
#else
if (NewFragments = (PixelFragment *)AppMem_Alloc(2 * sizeof (PixelFragment), 0))
#endif // WCS_PIXELMGER_USE_CALLOC
	{
	NewFragments[NewIsCloser].Coverage = (unsigned char)WCS_PIXEL_COVERAGE;
	NewFragments[NewIsCloser].ZBuf = ZBuf;
	NewFragments[NewIsCloser].RGB[0] = RGB[0];
	NewFragments[NewIsCloser].RGB[1] = RGB[1];
	NewFragments[NewIsCloser].RGB[2] = RGB[2];
	Coverage = WCS_PIXEL_MULTIFRAGBIT;		// sets the multi-fragment flag
	((PixelHeader *)this)->FragList = NewFragments;
	((PixelHeader *)this)->NumFrags[0] = 2;	// total fragments allocated
	((PixelHeader *)this)->NumFrags[1] = 1;	// index of last fragment used
	((PixelHeader *)this)->NumFrags[2] = 0;	// cumulative coverage to the last fragment used
	} // if

return (NewFragments);

} // PixelFragment::CreateMultiFragPixel

/*===========================================================================*/

unsigned char PixelHeader::AddFragment(void)
{
PixelFragment *NewFrags;

#ifdef WCS_PIXELMGER_USE_CALLOC
if (NewFrags = (PixelFragment *)realloc(FragList, (NumFrags[0] + 1) * sizeof (PixelFragment)))
	{
#else
if (NewFrags = (PixelFragment *)AppMem_Alloc((NumFrags[0] + 1) * sizeof (PixelFragment), 0))
	{
	memcpy(NewFrags, FragList, NumFrags[0] * sizeof (PixelFragment));
	AppMem_Free(FragList, NumFrags[0] * sizeof (PixelFragment));
#endif // WCS_PIXELMGER_USE_CALLOC
	FragList = NewFrags;
	NumFrags[0] ++;
	return (NumFrags[0]);
	} // if

return (0);

} // PixelHeader::AddFragment

/*===========================================================================*/

void PixelFragment::CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, float &OutZ, float &OutCoverage)
{
PixelHeader *MyHdr = (PixelHeader *)this;
long Ct, LastFrag, SumCoverage, CumCoverage, TempRed = 0, TempGreen = 0, TempBlue = 0;
float TempZ = 0.0f;

if (! WCS_PIXEL_MULTIFRAG)
	{
	OutRed = RGB[0];
	OutGreen = RGB[1];
	OutBlue = RGB[2];
	OutZ = ZBuf;
	OutCoverage = (float)((WCS_PIXEL_COVERAGE) * 0.01);  // Optimized out division. Was / 100.0
	} // if
else
	{
	LastFrag = MyHdr->NumFrags[1];
	CumCoverage = Ct = 0;
	while (CumCoverage < WCS_PIXEL_MAXCOVERAGE && Ct <= LastFrag)
		{
		SumCoverage = CumCoverage + MyHdr->FragList[Ct].Coverage;
		if (SumCoverage > WCS_PIXEL_MAXCOVERAGE)
			{
			MyHdr->FragList[Ct].Coverage = (unsigned char)(WCS_PIXEL_MAXCOVERAGE - CumCoverage);
			SumCoverage = WCS_PIXEL_MAXCOVERAGE;
			} // if
		TempRed += (MyHdr->FragList[Ct].Coverage * MyHdr->FragList[Ct].RGB[0]);
		TempGreen += (MyHdr->FragList[Ct].Coverage * MyHdr->FragList[Ct].RGB[1]);
		TempBlue += (MyHdr->FragList[Ct].Coverage * MyHdr->FragList[Ct].RGB[2]);
		TempZ += (MyHdr->FragList[Ct].Coverage * MyHdr->FragList[Ct].ZBuf);
		CumCoverage = SumCoverage;
		Ct ++;
		} // while
	if (CumCoverage)
		{
		OutRed = (unsigned char)(TempRed / CumCoverage);
		OutGreen = (unsigned char)(TempGreen / CumCoverage);
		OutBlue = (unsigned char)(TempBlue / CumCoverage);
		OutZ = (float)(TempZ / CumCoverage);
		OutCoverage = (float)(CumCoverage * 0.01);  // Optimized out division. Was / 100.0
		} // if
	else
		{
		OutRed = OutGreen = OutBlue = 0;
		OutZ = OutCoverage = 0.0f;
		} // else
	} // else

} // PixelFragment::CollapsePixel

/*===========================================================================*/

void PixelFragment::Reset(void)
{
PixelHeader *MyHdr = (PixelHeader *)this;

if (! WCS_PIXEL_MULTIFRAG)
	{
	RGB[0] = RGB[1] = RGB[2] = Coverage = 0;
	ZBuf = FLT_MAX;
	} // if
else
	{
	// leave the total number of allocated fragments alone, reset the others to 0
	MyHdr->NumFrags[1] = MyHdr->NumFrags[2] = 0;
	MyHdr->FragList[0].Coverage = 0;
	MyHdr->FragList[0].ZBuf = FLT_MAX;
	MyHdr->FragList[0].RGB[0] = MyHdr->FragList[0].RGB[1] = MyHdr->FragList[0].RGB[2] = 0;
	Coverage = WCS_PIXEL_MULTIFRAGBIT;
	} // else

} // PixelFragment::Reset

/*===========================================================================*/
/*===========================================================================*/

rPixelBlockHeader *Raster::ClearPixelFragMap(void)
{

if (rPixelBlock)
	rPixelBlock->AllocFirstFrags(Rows * Cols);

return (rPixelBlock);

} // Raster::ClearPixelFragMap

/*===========================================================================*/
/*===========================================================================*/

rPixelFragment::rPixelFragment()
{

Refl = NULL;
Next = NULL;
ZBuf = FLT_MAX;
BotCovg = TopCovg = 0;
Expon = 0;
RGB[2] = RGB[1] = RGB[0] = Flags = Alpha = 0;

} // rPixelFragment::rPixelFragment

/*===========================================================================*/

rPixelFragment::~rPixelFragment()
{

//Next = NULL;
//Refl = NULL;

} // rPixelFragment::~rPixelFragment

/*===========================================================================*/

void rPixelFragment::ExtractClippedExponentialColor(unsigned char &OneColor, unsigned short LocalExpon, int Band)
{
unsigned short TempExp;

if (Band == 0)
	{
	TempExp = (LocalExpon >> 10);
	} // if
else if (Band == 1)
	{
	TempExp = ((LocalExpon & 992) >> 5);
	} // else
else
	{
	TempExp = (LocalExpon & 31);
	} // else

if (TempExp)
	OneColor = 255;

} // rPixelFragment::ExtractClippedExponentialColor

/*===========================================================================*/

void rPixelFragment::ExtractClippedExponentialColors(unsigned char *ThreeColors, unsigned short LocalExpon)
{

if (LocalExpon >> 10)
	ThreeColors[0] = 255;

if ((LocalExpon & 992) >> 5)
	ThreeColors[1] = 255;

if (LocalExpon & 31)
	ThreeColors[2] = 255;

} // rPixelFragment::ExtractClippedExponentialColors

/*===========================================================================*/

void rPixelFragment::ExtractUnclippedExponentialColors(unsigned long *ThreeColors, unsigned short LocalExpon)
{
unsigned short TempExp;

TempExp = (LocalExpon >> 10);
if (TempExp)
	ThreeColors[0] <<= TempExp;
TempExp = ((LocalExpon & 992) >> 5);
if (TempExp)
	ThreeColors[1] <<= TempExp;
TempExp = (LocalExpon & 31);
if (TempExp)
	ThreeColors[2] <<= TempExp;

} // rPixelFragment::ExtractUnclippedExponentialColors

/*===========================================================================*/

void rPixelFragment::ExtractExponentialColors(unsigned long *ThreeColors, unsigned short &LocalExpon)
{
unsigned short TempExpon;

LocalExpon = 0;

// red
TempExpon = 0;

while (ThreeColors[0] & 0xffffff00)
	{
	TempExpon ++;
	ThreeColors[0] >>= 1;
	} // while

LocalExpon |= (TempExpon << 10);

// green
TempExpon = 0;

while (ThreeColors[1] & 0xffffff00)
	{
	TempExpon ++;
	ThreeColors[1] >>= 1;
	} // while

LocalExpon |= (TempExpon << 5);

// blue
TempExpon = 0;

while (ThreeColors[2] & 0xffffff00)
	{
	TempExpon ++;
	ThreeColors[2] >>= 1;
	} // while

LocalExpon |= (TempExpon);

} // rPixelFragment::ExtractExponentialColors

/*===========================================================================*/

void rPixelFragment::ExtractExponentialColor(unsigned long &OneColor, unsigned short &LocalExpon, int Band)
{
unsigned short TempExpon = 0;

if (Band == 0)
	{
	// red
	// clear red bits of exponent
	LocalExpon &= ~(63 << 10);
	TempExpon = 0;

	while (OneColor & 0xffffff00)
		{
		TempExpon ++;
		OneColor >>= 1;
		} // while

	LocalExpon |= (TempExpon << 10);
	} // if red
else if (Band == 1)
	{
	// green
	// clear green bits of exponent
	LocalExpon &= ~(31 << 5);
	TempExpon = 0;

	while (OneColor & 0xffffff00)
		{
		TempExpon ++;
		OneColor >>= 1;
		} // while

	LocalExpon |= (TempExpon << 5);
	} // if green
else
	{
	// blue
	// clear green bits of exponent
	LocalExpon &= ~(31);

	while (OneColor & 0xffffff00)
		{
		TempExpon ++;
		OneColor >>= 1;
		} // while

	LocalExpon |= (TempExpon);
	} // else

} // rPixelFragment::ExtractExponentialColor

/*===========================================================================*/
/*===========================================================================*/

rPixelHeader::rPixelHeader()
{

UsedFrags = 0;
FragList = NULL;

} // rPixelHeader::rPixelHeader

/*===========================================================================*/

rPixelHeader::~rPixelHeader()
{

//FragList = NULL;

} // rPixelHeader::~rPixelHeader

/*===========================================================================*/

rPixelFragment **rPixelHeader::TestPossibleUsage(float TestZ, unsigned long TestTopCovg, unsigned long TestBotCovg)
{
rPixelFragment *CurFrag, **FragPtr = &FragList;
unsigned long MaskedTop, MaskedBot, SumTop, SumBot;
int FragCt, SumCovg, TotalBits;

if (! UsedFrags)
	{
	return (FragPtr);
	} // if

SumCovg = 0;
MaskedTop = TestTopCovg;
MaskedBot = TestBotCovg;
SumTop = SumBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag; FragCt ++)
	{
	if (TestZ < CurFrag->ZBuf)
		{
		return (FragPtr);
		} // if
	// if we encounter a plot occluded flag reset coverage mask
	if (CurFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED))
		{
		SumCovg = 0;
		MaskedTop = TestTopCovg;
		MaskedBot = TestBotCovg;
		SumTop = SumBot = 0;
		} // if
	else
		{
		if (CurFrag->Alpha == 255)
			{
			MaskedTop &= (~CurFrag->TopCovg);
			MaskedBot &= (~CurFrag->BotCovg);
			if (! (MaskedTop || MaskedBot))
				return (NULL);
			} // if
		// this is intentionally conservative - better to plot extra fragments than to plot too few
		if (CurFrag->Alpha == 255)
			{
			TotalBits = (CurFrag->Alpha * (CountBits(CurFrag->TopCovg & (~SumTop)) + CountBits(CurFrag->BotCovg & (~SumBot)))) / 64;
			SumCovg += TotalBits;
			if (SumCovg >= 255)
				return (NULL);
			SumTop |= CurFrag->TopCovg;
			SumBot |= CurFrag->BotCovg;
			} // if
		} // else
	FragPtr = &CurFrag->Next;
	CurFrag = CurFrag->Next;
	} // for

return (FragPtr);

} // rPixelHeader::TestPossibleUsage

/*===========================================================================*/

rPixelFragment *rPixelHeader::GetFirstReflective(void)
{
rPixelFragment *CurFrag;

if (! UsedFrags)
	{
	return (NULL);
	} // if

for (CurFrag = FragList; CurFrag; CurFrag = CurFrag->Next)
	{
	if (CurFrag->Refl)
		return (CurFrag);
	} // for

return (NULL);

} // rPixelHeader::GetFirstReflective

/*===========================================================================*/

void rPixelHeader::SetLastFrag(rPixelFragment *LastFrag)
{
rPixelFragment *CurFrag;
int FragCt;

FragCt = 0;
for (CurFrag = FragList; CurFrag; CurFrag = CurFrag->Next)
	{
	FragCt ++;
	if (CurFrag == LastFrag)
		{
		CurFrag->Next = NULL;
		break;
		} // if
	} // for

UsedFrags = FragCt;

} // rPixelHeader::SetLastFrag

/*===========================================================================*/

rPixelFragment *rPixelHeader::PlotPixel(rPixelBlockHeader *BlockHdr, float NewZBuf, unsigned char NewAlpha, unsigned long NewTopCovg, unsigned long NewBotCovg, 
	int MaxFrags, unsigned char NewFlags)
{
rPixelFragment *CurFrag, *LastFrag, **FragPtr;
unsigned long MaskedTop, MaskedBot, SumTop, SumBot;
int FragCt, SumCovg, TotalBits;

if (! NewAlpha)
	return (NULL);

if (! UsedFrags)
	{
	UsedFrags = 1;
	FragList->Alpha = NewAlpha;
	FragList->ZBuf = NewZBuf;
	FragList->TopCovg = NewTopCovg;
	FragList->BotCovg = NewBotCovg;
	FragList->Flags = NewFlags;
	return FragList;
	} // if

#ifdef WCS_BUILD_RTX
// foliage is limited to half the fragment depth for exported imagery since it tends to occlude terrain
if ((NewFlags & WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED) && UsedFrags > MaxFrags / 2)
	return (NULL);
#endif // WCS_BUILD_RTX

SumCovg = 0;
MaskedTop = NewTopCovg;
MaskedBot = NewBotCovg;
SumTop = SumBot = 0;
for (FragCt = 0, FragPtr = &FragList, CurFrag = FragList, LastFrag = NULL; FragCt < UsedFrags && CurFrag; FragCt ++)
	{
	if (NewZBuf < CurFrag->ZBuf)
		{
		// found the place to insert new fragment
		break;
		} // if
	// F2_NOTE: Need to really benchmark whether this helps & where exactly to schedule it
	_mm_prefetch((char *)CurFrag->Next, _MM_HINT_T0);	// SSE intrinsic - requires at least SSE build support
	// if we encounter a plot occluded flag reset coverage mask
	if (CurFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED))
		{
		SumCovg = 0;
		MaskedTop = NewTopCovg;
		MaskedBot = NewBotCovg;
		SumTop = SumBot = 0;
		} // if
	else
		{
		if (CurFrag->Alpha == 255)
			{
			MaskedTop &= (~CurFrag->TopCovg);
			MaskedBot &= (~CurFrag->BotCovg);
			if (! (MaskedTop || MaskedBot))
				return (NULL);
			} // if
		// this is intentionally conservative - better to plot extra fragments than to plot too few
		if (CurFrag->Alpha == 255)
			{
			TotalBits = (CurFrag->Alpha * (CountBits(CurFrag->TopCovg & (~SumTop)) + CountBits(CurFrag->BotCovg & (~SumBot)))) / 64;
			SumCovg += TotalBits;
			if (SumCovg >= 255)
				return (NULL);
			SumTop |= CurFrag->TopCovg;
			SumBot |= CurFrag->BotCovg;
			} // if
		} // else
	FragPtr = &CurFrag->Next;
	LastFrag = CurFrag;
	CurFrag = CurFrag->Next;
	} // for

if (UsedFrags < MaxFrags)
	{
	if (CurFrag = BlockHdr->AddFragment(FragPtr))
		{
		UsedFrags ++;
		CurFrag->Alpha = NewAlpha;
		CurFrag->ZBuf = NewZBuf;
		CurFrag->TopCovg = NewTopCovg;
		CurFrag->BotCovg = NewBotCovg;
		CurFrag->Flags = NewFlags;
		return (CurFrag);
		} // if
	return (NULL);	// allocation failed
	} // if

// we're at the maximum number of fragments so we need to shuffle the data down and merge last two items' coverage
if (CurFrag)
	{
	LastFrag = NULL;
	while (CurFrag->Next)
		{
		LastFrag = CurFrag;
		CurFrag = CurFrag->Next;
		} // while
	// CurFrag now is the last in the chain, LastFrag is the one previous or NULL, FragPtr is a pointer to the insertion point
	if (LastFrag)	// CurFrag moves ahead in the chain
		{
		LastFrag->TopCovg |= CurFrag->TopCovg;
		LastFrag->BotCovg |= CurFrag->BotCovg;
		SumCovg = LastFrag->Alpha + CurFrag->Alpha;
		LastFrag->Alpha = (unsigned char)min(255, SumCovg);
		LastFrag->Next = NULL;
		CurFrag->Next = *FragPtr;
		CurFrag->Alpha = NewAlpha;
		CurFrag->TopCovg = NewTopCovg;
		CurFrag->BotCovg = NewBotCovg;
		CurFrag->Flags = NewFlags;
		*FragPtr = CurFrag;
		} // if
	else	// CurFrag stays last in chain
		{
		SumCovg = CurFrag->Alpha + NewAlpha;
		CurFrag->Alpha = (unsigned char)min(255, SumCovg);
		CurFrag->TopCovg |= NewTopCovg;
		CurFrag->BotCovg |= NewBotCovg;
		} // else
	CurFrag->ZBuf = NewZBuf;
	return (CurFrag);
	} // if there are more fragments after the point where we need to insert one

// we have no more room to add data so we return NULL but first we have to set the coverage of the pixel to the sum
if (LastFrag)
	{
	LastFrag->TopCovg |= NewTopCovg;
	LastFrag->BotCovg |= NewBotCovg;
	SumCovg = LastFrag->Alpha + NewAlpha;
	LastFrag->Alpha = (unsigned char)min(255, SumCovg);
	} // if

return (NULL);

} // rPixelHeader::PlotPixel

/*===========================================================================*/

//#define DOUBLE2ULONG(ul, d) {double t = ((d) + 6755399441055744.0); ul = *((unsigned long *)(&t));}
//#define DOUBLE2USHORT(us, d) {double t = ((d) + 103079215104.0); us = *((unsigned short *)(&t));}

void rPixelFragment::PlotPixel(rPixelBlockHeader *BlockHdr, double SourceRGB[3], double SourceReflect, double SourceNormal[3])
{
unsigned long TempRGB;
unsigned short TempExpon;

// alpha and ZBuf have already been entered in this fragment
Expon = 0;

if (SourceRGB[0] < 0.0)
	SourceRGB[0] = 0.0;
if (SourceRGB[1] < 0.0)
	SourceRGB[1] = 0.0;
if (SourceRGB[2] < 0.0)
	SourceRGB[2] = 0.0;

// red
TempRGB = (unsigned long)(SourceRGB[0] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[0] = (unsigned char)TempRGB;
Expon |= (TempExpon << 10);

// green
TempRGB = (unsigned long)(SourceRGB[1] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[1] = (unsigned char)TempRGB;
Expon |= (TempExpon << 5);

// blue
TempRGB = (unsigned long)(SourceRGB[2] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[2] = (unsigned char)TempRGB;
Expon |= (TempExpon);

// Surface Normal and reflectivity
if (SourceReflect > 0.0)
	{
	if (Refl || (Refl = BlockHdr->GetNextAvailRefl()))
		{
		SourceReflect *= 255.99;
		Refl->Reflect = (SourceReflect > USHRT_MAX) ? USHRT_MAX: (unsigned short)(SourceReflect);
		Refl->Normal[0] = (float)SourceNormal[0];
		Refl->Normal[1] = (float)SourceNormal[1];
		Refl->Normal[2] = (float)SourceNormal[2];
		} // if
	} // if
else
	Refl = NULL;

} // rPixelFragment::PlotPixel

/*===========================================================================*/

// this version only changes the color
void rPixelFragment::PlotPixel(double SourceRGB[3])
{
unsigned long TempRGB;
unsigned short TempExpon;

// alpha and ZBuf have already been entered in this fragment
Expon = 0;

if (SourceRGB[0] < 0.0)
	SourceRGB[0] = 0.0;
if (SourceRGB[1] < 0.0)
	SourceRGB[1] = 0.0;
if (SourceRGB[2] < 0.0)
	SourceRGB[2] = 0.0;

// red
TempRGB = (unsigned long)(SourceRGB[0] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[0] = (unsigned char)TempRGB;
Expon |= (TempExpon << 10);

// green
TempRGB = (unsigned long)(SourceRGB[1] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[1] = (unsigned char)TempRGB;
Expon |= (TempExpon << 5);

// blue
TempRGB = (unsigned long)(SourceRGB[2] * 255);
TempExpon = 0;

while (TempRGB & 0xffffff00)
	{
	TempExpon ++;
	TempRGB >>= 1;
	} // while

RGB[2] = (unsigned char)TempRGB;
Expon |= (TempExpon);

} // rPixelFragment::PlotPixel

/*===========================================================================*/

// fills in color as range of 0-1 or higher
void rPixelFragment::ExtractColor(double SourceRGB[3])
{
unsigned long TempRed, TempGreen, TempBlue;
unsigned short TempExp;

TempRed = RGB[0];
TempExp = (Expon >> 10);
if (TempExp)
	TempRed <<= TempExp;

TempGreen = RGB[1];
TempExp = ((Expon & 992) >> 5);
if (TempExp)
	TempGreen <<= TempExp;

TempBlue = RGB[2];
TempExp = (Expon & 31);
if (TempExp)
	TempBlue <<= TempExp;

SourceRGB[0] = TempRed * (1.0 / 255.0);
SourceRGB[1] = TempGreen * (1.0 / 255.0);
SourceRGB[2] = TempBlue * (1.0 / 255.0);

} // rPixelFragment::ExtractColor

/*===========================================================================*/

// returns double precision coverage value ranging from 0 to 1
double rPixelFragment::FetchMaskedCoverage(void)
{

//16320 = 255 * 64
return ((double)Alpha * (CountBits(TopCovg) + CountBits(BotCovg)) * (1.0 / 16320.0));

} // rPixelFragment::FetchMaskedCoverage

/*===========================================================================*/

int rPixelFragment::TestVisible(rPixelFragment *LastFrag, unsigned long &MaskTop, unsigned long &MaskBot, unsigned long &SumCovg)
{
unsigned long TempCovg, FragCovg;

// update with last fragment
if (LastFrag && SumCovg < 255)
	{
	if (LastFrag->Alpha)
		{
		FragCovg = (unsigned long)quickintceil(((CountBits(LastFrag->TopCovg & (~MaskTop)) + CountBits(LastFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* LastFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			SumCovg = TempCovg;
			if (LastFrag->Alpha == 255)
				{
				MaskTop |= LastFrag->TopCovg;
				MaskBot |= LastFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	} // if

// test this fragment
if (SumCovg < 255)
	{
	if (Alpha)
		{
		if (FragCovg = (unsigned long)quickintceil(((CountBits(TopCovg & (~MaskTop)) + CountBits(BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* Alpha))
			{
			return (1);
			} // if some coverage after masking bits
		} // if some coverage
	} // if

return (0);

} // rPixelFragment::TestVisible

/*===========================================================================*/

void rPixelHeader::CollapseMap(rPixelHeader *HeaderArray, long ArraySize, unsigned char *OutRed, unsigned char *OutGreen, unsigned char *OutBlue, 
	float *OutZ, unsigned char *OutCoverage, float *OutReflect, float *OutNormal[3], unsigned short *OutExponent)
{
long PixCt;
float TempReflect, TempNormal[3];

// you are here
for (PixCt = 0; PixCt < ArraySize; PixCt ++)
	{
	HeaderArray[PixCt].CollapsePixel(OutRed[PixCt], OutGreen[PixCt], OutBlue[PixCt],
		OutZ[PixCt], OutCoverage[PixCt], TempReflect, TempNormal, OutExponent ? &OutExponent[PixCt]: NULL, FALSE);
	if (OutReflect)
		OutReflect[PixCt] = TempReflect;
	if (TempReflect > 0.0f)
		{
		if (OutNormal[0])
			OutNormal[0][PixCt] = TempNormal[0];
		if (OutNormal[1])
			OutNormal[1][PixCt] = TempNormal[1];
		if (OutNormal[2])
			OutNormal[2][PixCt] = TempNormal[2];
		} // if
	} // for
// now you're here

} // rPixelHeader::CollapseMap

/*===========================================================================*/

void rPixelHeader::CollapseMap(rPixelHeader *HeaderArray, long ArraySize, unsigned char *OutRed, unsigned char *OutGreen, unsigned char *OutBlue, 
	float *OutZ, unsigned char *OutCoverage, float *OutReflect, float *OutNormal[3], unsigned short *OutExponent, 
	unsigned char *Out2ndRed, unsigned char *Out2ndGreen, unsigned char *Out2ndBlue, unsigned char *Out2ndCoverage, 
	int HowToCollapse, char TransparentPixelsExist)
{
long PixCt;
float TempReflect, TempNormal[3];

for (PixCt = 0; PixCt < ArraySize; PixCt ++)
	{
	if (HowToCollapse == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINFIRST)
		{
		HeaderArray[PixCt].CollapsePixel(OutRed[PixCt], OutGreen[PixCt], OutBlue[PixCt],
			OutZ[PixCt], OutCoverage[PixCt], TempReflect, TempNormal, OutExponent ? &OutExponent[PixCt]: NULL, TRUE);
		} // if foliage and terrain in first bitmap, no second bitmap
	else if (HowToCollapse == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_FOLIAGEINSECOND)
		{
		HeaderArray[PixCt].CollapsePixelTerInFirstFolInSecond(OutRed[PixCt], OutGreen[PixCt], OutBlue[PixCt],
			OutZ[PixCt], OutCoverage[PixCt], TempReflect, TempNormal, OutExponent ? &OutExponent[PixCt]: NULL,
			Out2ndRed[PixCt], Out2ndGreen[PixCt], Out2ndBlue[PixCt], Out2ndCoverage[PixCt]);
		} // else if foliage only in second bitmap, terrain only in first
	else if (HowToCollapse == WCS_EFFECTS_SCENEEXPORTER_FOLTEXTYPE_BOTHINSECOND)
		{
		HeaderArray[PixCt].CollapsePixelTerInFirstBothInSecond(OutRed[PixCt], OutGreen[PixCt], OutBlue[PixCt],
			OutZ[PixCt], OutCoverage[PixCt], TempReflect, TempNormal, OutExponent ? &OutExponent[PixCt]: NULL,
			Out2ndRed[PixCt], Out2ndGreen[PixCt], Out2ndBlue[PixCt], Out2ndCoverage[PixCt]);
		} // else if foliage and terrain in second bitmap, no foliage in first
	else
		{
		HeaderArray[PixCt].CollapsePixelNoFoliage(OutRed[PixCt], OutGreen[PixCt], OutBlue[PixCt],
			OutZ[PixCt], OutCoverage[PixCt], TempReflect, TempNormal, OutExponent ? &OutExponent[PixCt]: NULL);
		} // else no foliage in either bitmap, no second bitmap
	if (OutReflect)
		OutReflect[PixCt] = TempReflect;
	if (TempReflect > 0.0f)
		{
		if (OutNormal[0])
			OutNormal[0][PixCt] = TempNormal[0];
		if (OutNormal[1])
			OutNormal[1][PixCt] = TempNormal[1];
		if (OutNormal[2])
			OutNormal[2][PixCt] = TempNormal[2];
		} // if
	if (OutCoverage[PixCt] < 255)
		TransparentPixelsExist = 1;
	} // for

} // rPixelHeader::CollapseMap

/*===========================================================================*/

#ifdef WCS_BUILD_RTX

void rPixelHeader::CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent, int RejectVectorZ)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
float TempNormal[3], TempZ;
int FragCt; 
int FragNoZ, ZPlotted = 0;
unsigned char ObjType;
unsigned short TempExp;

OutRed = OutGreen = OutBlue = OutCoverage = 0;
OutZ = TempZ = FLT_MAX;
OutReflect = OutNormal[2] = OutNormal[1] = OutNormal[0] = 0.0f;

if (! UsedFrags)
	{
	return;
	} // if

SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && (SumCovg < 255 || ! ZPlotted); FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		ObjType = CurFrag->GetObjectType();
		FragNoZ = (RejectVectorZ && (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT || ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE));
		FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (! FragNoZ && ! ZPlotted)
				{
				TempZ = CurFrag->ZBuf;
				ZPlotted = 1;
				}
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;

				if (CurFrag->Refl)
					{
					SumReflectCovg += FragCovg;
					if (! TempReflect)	//lint !e530
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						if (! FragNoZ)
							TempZ = CurFrag->ZBuf;
						} // if
					} // if
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				if (CurFrag->Refl)
					{
					TempReflect = CurFrag->Refl->Reflect;
					TempNormal[0] = CurFrag->Refl->Normal[0];
					TempNormal[1] = CurFrag->Refl->Normal[1];
					TempNormal[2] = CurFrag->Refl->Normal[2];
					SumReflectCovg += FragCovg;
					} // if
				else
					{
					TempReflect = 0;
					TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
					} // else
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255 && ! FragNoZ)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	if (OutExponent)
		{
		*OutExponent = 0;

		// red
		TempExp = 0;
		while (SumRed & 0xffffff00)
			{
			TempExp ++;
			SumRed >>= 1;
			} // while

		OutRed = (unsigned char)SumRed;
		(*OutExponent) |= (TempExp << 10);

		// green
		TempExp = 0;
		while (SumGreen & 0xffffff00)
			{
			TempExp ++;
			SumGreen >>= 1;
			} // while

		OutGreen = (unsigned char)SumGreen;
		(*OutExponent) |= (TempExp << 5);

		// blue
		TempExp = 0;
		while (SumBlue & 0xffffff00)
			{
			TempExp ++;
			SumBlue >>= 1;
			} // while

		OutBlue = (unsigned char)SumBlue;
		(*OutExponent) |= (TempExp);
		} // if
	else
		{
		OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
		OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
		OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
		} // if
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

} // rPixelHeader::CollapsePixel

/*===========================================================================*/

#else // WCS_BUILD_RTX

void rPixelHeader::CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent, int RejectVectorZ)
{
float TempNormal[3], TempZ;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
rPixelFragment *CurFrag;
int FragCt; 
unsigned short TempExp;

OutRed = OutGreen = OutBlue = OutCoverage = 0;
OutZ = TempZ = FLT_MAX;
OutReflect = OutNormal[0] = OutNormal[1] = OutNormal[2] = 0.0f;

if (! UsedFrags)
	{
	return;
	} // if

SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && SumCovg < 255; FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		FragCovg = (int)WCS_ceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;

				if (CurFrag->Refl)
					{
					SumReflectCovg += FragCovg;
					if (! TempReflect)
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						TempZ = CurFrag->ZBuf;
						} // if
					} // if
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				TempZ = CurFrag->ZBuf;
				if (CurFrag->Refl)
					{
					TempReflect = CurFrag->Refl->Reflect;
					TempNormal[0] = CurFrag->Refl->Normal[0];
					TempNormal[1] = CurFrag->Refl->Normal[1];
					TempNormal[2] = CurFrag->Refl->Normal[2];
					SumReflectCovg += FragCovg;
					} // if
				else
					{
					TempReflect = 0;
					TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
					} // else
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	if (OutExponent)
		{
		*OutExponent = 0;

		// red
		TempExp = 0;
		while (SumRed & 0xffffff00)
			{
			TempExp ++;
			SumRed >>= 1;
			} // while

		OutRed = (unsigned char)SumRed;
		(*OutExponent) |= (TempExp << 10);

		// green
		TempExp = 0;
		while (SumGreen & 0xffffff00)
			{
			TempExp ++;
			SumGreen >>= 1;
			} // while

		OutGreen = (unsigned char)SumGreen;
		(*OutExponent) |= (TempExp << 5);

		// blue
		TempExp = 0;
		while (SumBlue & 0xffffff00)
			{
			TempExp ++;
			SumBlue >>= 1;
			} // while

		OutBlue = (unsigned char)SumBlue;
		(*OutExponent) |= (TempExp);
		} // if
	else
		{
		OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
		OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
		OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
		} // if
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

} // rPixelHeader::CollapsePixel

#endif // WCS_BUILD_RTX

/*===========================================================================*/

void rPixelHeader::CollapsePixelTerInFirstFolInSecond(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent,
	unsigned char &Out2ndRed, unsigned char &Out2ndGreen, unsigned char &Out2ndBlue, unsigned char &Out2ndCoverage)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
float TempNormal[3], TempZ;
int FragCt, FragNoZ, ZPlotted = 0;
unsigned short TempExp;
unsigned char ObjType;

OutRed = OutGreen = OutBlue = OutCoverage = 0;
Out2ndRed = Out2ndGreen = Out2ndBlue = Out2ndCoverage = 0;
OutZ = TempZ = FLT_MAX;
OutReflect = OutNormal[0] = OutNormal[1] = OutNormal[2] = 0.0f;
if (OutExponent)
	*OutExponent = 0;

if (! UsedFrags)
	{
	return;
	} // if

// first do terrain only
SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && (SumCovg < 255 || ! ZPlotted); FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		ObjType = CurFrag->GetObjectType();
		if (! (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT || ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE))
			{
			FragNoZ = (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR);
			FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
				* CurFrag->Alpha);
			if (FragCovg)
				{
				TempCovg = SumCovg + FragCovg;
				if (TempCovg > 255)
					{
					TempCovg = 255;
					FragCovg = 255 - SumCovg;
					} // if
				TempRed = CurFrag->RGB[0];
				TempExp = (CurFrag->Expon >> 10);
				if (TempExp)
					TempRed <<= TempExp;
				TempGreen = CurFrag->RGB[1];
				TempExp = ((CurFrag->Expon & 992) >> 5);
				if (TempExp)
					TempGreen <<= TempExp;
				TempBlue = CurFrag->RGB[2];
				TempExp = (CurFrag->Expon & 31);
				if (TempExp)
					TempBlue <<= TempExp;
				if (! FragNoZ && ! ZPlotted)
					{
					TempZ = CurFrag->ZBuf;
					ZPlotted = 1;
					}
				if (SumCovg)
					{
					SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
					SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
					SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
					if (CurFrag->Refl)
						{
						SumReflectCovg += FragCovg;
						if (! TempReflect)	//lint !e530
							{
							TempReflect = CurFrag->Refl->Reflect;
							TempNormal[0] = CurFrag->Refl->Normal[0];
							TempNormal[1] = CurFrag->Refl->Normal[1];
							TempNormal[2] = CurFrag->Refl->Normal[2];
							if (! FragNoZ)
								TempZ = CurFrag->ZBuf;
							} // if
						} // if
					} // if
				else
					{
					SumRed = TempRed;
					SumGreen = TempGreen;
					SumBlue = TempBlue;
					if (CurFrag->Refl)
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						SumReflectCovg += FragCovg;
						} // if
					else
						{
						TempReflect = 0;
						TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
						} // else
					} // else
				SumCovg = TempCovg;
				if (CurFrag->Alpha == 255 && ! FragNoZ)
					{
					MaskTop |= CurFrag->TopCovg;
					MaskBot |= CurFrag->BotCovg;
					} // if
				} // if some coverage after masking bits
			} // if not foliage or 3d object
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

// then do foliage only
SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && SumCovg < 255; FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		ObjType = CurFrag->GetObjectType();
		if (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT || ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE)
			{
			FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
				* CurFrag->Alpha);
			if (FragCovg)
				{
				TempCovg = SumCovg + FragCovg;
				if (TempCovg > 255)
					{
					TempCovg = 255;
					FragCovg = 255 - SumCovg;
					} // if
				TempRed = CurFrag->RGB[0];
				TempExp = (CurFrag->Expon >> 10);
				if (TempExp)
					TempRed <<= TempExp;
				TempGreen = CurFrag->RGB[1];
				TempExp = ((CurFrag->Expon & 992) >> 5);
				if (TempExp)
					TempGreen <<= TempExp;
				TempBlue = CurFrag->RGB[2];
				TempExp = (CurFrag->Expon & 31);
				if (TempExp)
					TempBlue <<= TempExp;
				if (SumCovg)
					{
					SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
					SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
					SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
					} // if
				else
					{
					SumRed = TempRed;
					SumGreen = TempGreen;
					SumBlue = TempBlue;
					} // else
				SumCovg = TempCovg;
				if (CurFrag->Alpha == 255)
					{
					MaskTop |= CurFrag->TopCovg;
					MaskBot |= CurFrag->BotCovg;
					} // if
				} // if some coverage after masking bits
			} // if not foliage or 3d object
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	Out2ndRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	Out2ndGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	Out2ndBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	Out2ndCoverage = (unsigned char)SumCovg;
	} // if

} // rPixelHeader::CollapsePixelTerInFirstFolInSecond

/*===========================================================================*/

void rPixelHeader::CollapsePixelTerInFirstBothInSecond(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent,
	unsigned char &Out2ndRed, unsigned char &Out2ndGreen, unsigned char &Out2ndBlue, unsigned char &Out2ndCoverage)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
float TempNormal[3], TempZ;
int FragCt, FragNoZ, ZPlotted = 0;
unsigned short TempExp;
unsigned char ObjType;

OutRed = OutGreen = OutBlue = OutCoverage = 0;
Out2ndRed = Out2ndGreen = Out2ndBlue = Out2ndCoverage = 0;
OutZ = TempZ = FLT_MAX;
OutReflect = OutNormal[0] = OutNormal[1] = OutNormal[2] = 0.0f;
if (OutExponent)
	*OutExponent = 0;

if (! UsedFrags)
	{
	return;
	} // if

// first do terrain plus foliage in second
SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && SumCovg < 255; FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		FragCovg = (int)WCS_ceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	Out2ndRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	Out2ndGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	Out2ndBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	Out2ndCoverage = (unsigned char)SumCovg;
	} // if

// then do terrain only in first
SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && (SumCovg < 255 || ! ZPlotted); FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		// test to see if it is foliage or 3d object
		ObjType = CurFrag->GetObjectType();
		if (! (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT || ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE))
			{
			FragNoZ = (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR);
			FragCovg = (int)WCS_ceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
				* CurFrag->Alpha);
			if (FragCovg)
				{
				TempCovg = SumCovg + FragCovg;
				if (TempCovg > 255)
					{
					TempCovg = 255;
					FragCovg = 255 - SumCovg;
					} // if
				TempRed = CurFrag->RGB[0];
				TempExp = (CurFrag->Expon >> 10);
				if (TempExp)
					TempRed <<= TempExp;
				TempGreen = CurFrag->RGB[1];
				TempExp = ((CurFrag->Expon & 992) >> 5);
				if (TempExp)
					TempGreen <<= TempExp;
				TempBlue = CurFrag->RGB[2];
				TempExp = (CurFrag->Expon & 31);
				if (TempExp)
					TempBlue <<= TempExp;
				if (! FragNoZ && ! ZPlotted)
					{
					TempZ = CurFrag->ZBuf;
					ZPlotted = 1;
					}
				if (SumCovg)
					{
					SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
					SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
					SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
					if (CurFrag->Refl)
						{
						SumReflectCovg += FragCovg;
						if (! TempReflect)	//lint !e530
							{
							TempReflect = CurFrag->Refl->Reflect;
							TempNormal[0] = CurFrag->Refl->Normal[0];
							TempNormal[1] = CurFrag->Refl->Normal[1];
							TempNormal[2] = CurFrag->Refl->Normal[2];
							if (! FragNoZ)
								TempZ = CurFrag->ZBuf;
							} // if
						} // if
					} // if
				else
					{
					SumRed = TempRed;
					SumGreen = TempGreen;
					SumBlue = TempBlue;
					if (CurFrag->Refl)
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						SumReflectCovg += FragCovg;
						} // if
					else
						{
						TempReflect = 0;
						TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
						} // else
					} // else
				SumCovg = TempCovg;
				if (CurFrag->Alpha == 255 && ! FragNoZ)
					{
					MaskTop |= CurFrag->TopCovg;
					MaskBot |= CurFrag->BotCovg;
					} // if
				} // if some coverage after masking bits
			} // if not foliage or 3d object
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

} // rPixelHeader::CollapsePixelTerInFirstBothInSecond

/*===========================================================================*/

void rPixelHeader::CollapsePixelNoFoliage(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
float TempNormal[3], TempZ;
int FragCt, FragNoZ, ZPlotted = 0;
unsigned short TempExp;
unsigned char ObjType;

OutRed = OutGreen = OutBlue = OutCoverage = 0;
OutZ = TempZ = FLT_MAX;
OutReflect = OutNormal[0] = OutNormal[1] = OutNormal[2] = 0.0f;
if (OutExponent)
	*OutExponent = 0;

if (! UsedFrags)
	{
	return;
	} // if

// first do terrain only
SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && (SumCovg < 255 || ! ZPlotted); FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		ObjType = CurFrag->GetObjectType();
		if (! (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT || ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE
			|| ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE))
			{
			FragNoZ = (ObjType == WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR);
			FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
				* CurFrag->Alpha);
			if (FragCovg)
				{
				TempCovg = SumCovg + FragCovg;
				if (TempCovg > 255)
					{
					TempCovg = 255;
					FragCovg = 255 - SumCovg;
					} // if
				TempRed = CurFrag->RGB[0];
				TempExp = (CurFrag->Expon >> 10);
				if (TempExp)
					TempRed <<= TempExp;
				TempGreen = CurFrag->RGB[1];
				TempExp = ((CurFrag->Expon & 992) >> 5);
				if (TempExp)
					TempGreen <<= TempExp;
				TempBlue = CurFrag->RGB[2];
				TempExp = (CurFrag->Expon & 31);
				if (TempExp)
					TempBlue <<= TempExp;
				if (! FragNoZ && ! ZPlotted)
					{
					TempZ = CurFrag->ZBuf;
					ZPlotted = 1;
					}
				if (SumCovg)
					{
					SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
					SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
					SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
					if (CurFrag->Refl)
						{
						SumReflectCovg += FragCovg;
						if (! TempReflect)	//lint !e530
							{
							TempReflect = CurFrag->Refl->Reflect;
							TempNormal[0] = CurFrag->Refl->Normal[0];
							TempNormal[1] = CurFrag->Refl->Normal[1];
							TempNormal[2] = CurFrag->Refl->Normal[2];
							if (! FragNoZ)
								TempZ = CurFrag->ZBuf;
							} // if
						} // if
					} // if
				else
					{
					SumRed = TempRed;
					SumGreen = TempGreen;
					SumBlue = TempBlue;
					if (CurFrag->Refl)
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						SumReflectCovg += FragCovg;
						} // if
					else
						{
						TempReflect = 0;
						TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
						} // else
					} // else
				SumCovg = TempCovg;
				if (CurFrag->Alpha == 255 && ! FragNoZ)
					{
					MaskTop |= CurFrag->TopCovg;
					MaskBot |= CurFrag->BotCovg;
					} // if
				} // if some coverage after masking bits
			} // if not foliage or 3d object
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

} // rPixelHeader::CollapsePixelNoFoliage

/*===========================================================================*/

void rPixelHeader::CollapsePixel(double *OutRGB, 
	float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, TempReflect, MaskTop, MaskBot, 
	SumCovg, TempCovg, FragCovg, SumReflectCovg;
int FragCt;
float TempNormal[3], TempZ;
unsigned short TempExp;

OutRGB[0] = OutRGB[1] = OutRGB[2] = OutCoverage = 0;
OutZ = FLT_MAX;
OutReflect = OutNormal[0] = OutNormal[1] = OutNormal[2] = 0.0f;

if (! UsedFrags)
	{
	return;
	} // if

SumCovg = SumReflectCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && SumCovg < 255; FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
				if (CurFrag->Refl)
					{
					SumReflectCovg += FragCovg;
					if (! TempReflect)	//lint !e530
						{
						TempReflect = CurFrag->Refl->Reflect;
						TempNormal[0] = CurFrag->Refl->Normal[0];
						TempNormal[1] = CurFrag->Refl->Normal[1];
						TempNormal[2] = CurFrag->Refl->Normal[2];
						TempZ = CurFrag->ZBuf;
						} // if
					} // if
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				TempZ = CurFrag->ZBuf;
				if (CurFrag->Refl)
					{
					TempReflect = CurFrag->Refl->Reflect;
					TempNormal[0] = CurFrag->Refl->Normal[0];
					TempNormal[1] = CurFrag->Refl->Normal[1];
					TempNormal[2] = CurFrag->Refl->Normal[2];
					SumReflectCovg += FragCovg;
					} // if
				else
					{
					TempReflect = 0;
					TempNormal[0] = TempNormal[1] = TempNormal[2] = 0.0f;
					} // else
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	OutRGB[0] = SumRed * (1.0 / 255.0);
	OutRGB[1] = SumGreen * (1.0 / 255.0);
	OutRGB[2] = SumBlue * (1.0 / 255.0);
	OutCoverage = (unsigned char)SumCovg;
	OutZ = TempZ;
	TempReflect = (unsigned long)(TempReflect * (double)SumReflectCovg / SumCovg);
	OutReflect = (float)(TempReflect / 255.0);
	OutNormal[0] = TempNormal[0];
	OutNormal[1] = TempNormal[1];
	OutNormal[2] = TempNormal[2];
	} // if

} // rPixelHeader::CollapsePixel

/*===========================================================================*/

// this version gets ony the RGB
void rPixelHeader::CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue)
{
rPixelFragment *CurFrag;
int FragCt;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, MaskTop, MaskBot, SumCovg, TempCovg, FragCovg;
unsigned short TempExp;

OutRed = OutGreen = OutBlue = 0;

if (! UsedFrags)
	{
	return;
	} // if

SumCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag && SumCovg < 255; FragCt ++)
	{
	if (CurFrag->Alpha)
		{
		FragCovg = quickintceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	CurFrag = CurFrag->Next;
	} // for

if (SumCovg)
	{
	OutRed = SumRed > 255 ? 255: (unsigned char)SumRed;
	OutGreen = SumGreen > 255 ? 255: (unsigned char)SumGreen;
	OutBlue = SumBlue > 255 ? 255: (unsigned char)SumBlue;
	} // if

} // rPixelHeader::CollapsePixel

/*===========================================================================*/

int rPixelHeader::CollapseFartherPixel(float CompareZ, double OutColor[3], unsigned char &OutCoverage)
{
rPixelFragment *CurFrag;
unsigned long TempRed, TempGreen, TempBlue, SumRed, SumGreen, SumBlue, MaskTop, MaskBot, SumCovg, TempCovg, FragCovg;
unsigned short TempExp;

OutCoverage = 0;
OutColor[2] = OutColor[1] = OutColor[0] = 0.0;

if (! UsedFrags)
	{
	return (0);
	} // if

for (CurFrag = FragList; CurFrag; CurFrag = CurFrag->Next)
	{
	if (CompareZ < CurFrag->ZBuf)
		{
		// found the place to start collapse
		break;
		} // if
	} // for

SumCovg = 0;
SumRed = SumGreen = SumBlue = 0;
MaskTop = MaskBot = 0;
for (; CurFrag && SumCovg < 255; CurFrag = CurFrag->Next)
	{
	if (CurFrag->Alpha)
		{
		FragCovg = (int)WCS_ceil(((CountBits(CurFrag->TopCovg & (~MaskTop)) + CountBits(CurFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
			* CurFrag->Alpha);
		if (FragCovg)
			{
			TempCovg = SumCovg + FragCovg;
			if (TempCovg > 255)
				{
				TempCovg = 255;
				FragCovg = 255 - SumCovg;
				} // if
			TempRed = CurFrag->RGB[0];
			TempExp = (CurFrag->Expon >> 10);
			if (TempExp)
				TempRed <<= TempExp;
			TempGreen = CurFrag->RGB[1];
			TempExp = ((CurFrag->Expon & 992) >> 5);
			if (TempExp)
				TempGreen <<= TempExp;
			TempBlue = CurFrag->RGB[2];
			TempExp = (CurFrag->Expon & 31);
			if (TempExp)
				TempBlue <<= TempExp;
			if (SumCovg)
				{
				SumRed = (SumRed * SumCovg + TempRed * FragCovg) / TempCovg;
				SumGreen = (SumGreen * SumCovg + TempGreen * FragCovg) / TempCovg;
				SumBlue = (SumBlue * SumCovg + TempBlue * FragCovg) / TempCovg;
				} // if
			else
				{
				SumRed = TempRed;
				SumGreen = TempGreen;
				SumBlue = TempBlue;
				} // else
			SumCovg = TempCovg;
			if (CurFrag->Alpha == 255)
				{
				MaskTop |= CurFrag->TopCovg;
				MaskBot |= CurFrag->BotCovg;
				} // if
			} // if some coverage after masking bits
		} // if some coverage
	} // for

if (SumCovg)
	{
	OutColor[0] = SumRed * (1.0 / 255.0);
	OutColor[1] = SumGreen * (1.0 / 255.0);
	OutColor[2] = SumBlue * (1.0 / 255.0);
	OutCoverage = (unsigned char)SumCovg;
	return (1);
	} // if

return (0);

} // rPixelHeader::CollapseFartherPixel

/*===========================================================================*/

void rPixelHeader::RefractMap(rPixelHeader *HeaderArray, long ArraySize)
{
long PixCt;

for (PixCt = 0; PixCt < ArraySize; ++PixCt)
	{
	HeaderArray[PixCt].RefractPixel();
	} // for

} // rPixelHeader::RefractMap

/*===========================================================================*/

void rPixelHeader::RefractPixel(void)
{
double InvOpticalDepth, FragWt, Transmission, TotalTransmitted, FragColor[3], SumColor[3];
rPixelFragment *CurFrag, *RefrFrag, *LastODFrag = NULL;
unsigned long MaskTop, MaskBot;
int FragCt, SumCovg, TempCovg, FragCovg;

// look for pixels that have reflection data

if (! UsedFrags)
	{
	return;
	} // if

for (FragCt = 0, CurFrag = FragList; FragCt < UsedFrags && CurFrag; FragCt ++, CurFrag = CurFrag->Next)
	{
	if (CurFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OPTICALDEPTH))
		{
		CurFrag->Alpha = 255 - CurFrag->Alpha;
		LastODFrag = CurFrag;
		if (CurFrag->Alpha > 0)
			{
			InvOpticalDepth = 1.0 / pow(2.0, (CurFrag->Alpha - 50) * .1);
			SumColor[0] = SumColor[1] = SumColor[2] = TotalTransmitted = 0.0;
			SumCovg = 0;
			MaskTop = MaskBot = 0;
			for (RefrFrag = CurFrag->Next; RefrFrag && SumCovg < 255; RefrFrag = RefrFrag->Next)
				{
				if (! RefrFrag->TestFlags(WCS_PIXELFRAG_FLAGBIT_OPTICALDEPTH) && RefrFrag->Alpha)
					{
					// find fragment weight
					FragCovg = quickintceil(((CountBits(RefrFrag->TopCovg & (~MaskTop)) + CountBits(RefrFrag->BotCovg & (~MaskBot))) * (1.0 / 64.0))
						* RefrFrag->Alpha);
					if (FragCovg)
						{
						TempCovg = SumCovg + FragCovg;
						if (TempCovg > 255)
							FragCovg = 255 - SumCovg;

						FragWt = FragCovg * (1.0 / 255.0);
						// accounts for bi-directional light transport through absorptive medium
						//Transmission = 2.0 * (RefrFrag->ZBuf - CurFrag->ZBuf) * 4.605 * InvOpticalDepth;
						Transmission = (RefrFrag->ZBuf - CurFrag->ZBuf) * .69315 * InvOpticalDepth;
						// if the exponent argument gets too large it causes float underflow and contributes negligibly
						// 6.9 corresponds with a transmission result of .001 which is small enough to ignore
						if (Transmission > 6.9)
							break;
						Transmission = FragWt * exp(-Transmission);
						TotalTransmitted += Transmission; 
						// multiply transmission by fragment weight and color
						// sum fragment contributions
						RefrFrag->ExtractColor(FragColor);
						SumColor[0] += FragColor[0] * Transmission;
						SumColor[1] += FragColor[1] * Transmission;
						SumColor[2] += FragColor[2] * Transmission;
						SumCovg = TempCovg;
						if (RefrFrag->Alpha == 255)
							{
							MaskTop |= RefrFrag->TopCovg;
							MaskBot |= RefrFrag->BotCovg;
							} // if
						} // if
					} // if
				} // for
			if (TotalTransmitted > 0.0)
				{
				TotalTransmitted = 1.0 - TotalTransmitted;
				CurFrag->ExtractColor(FragColor);
				// back out a proportion of the fragment color
				FragColor[0] = FragColor[0] * TotalTransmitted + SumColor[0];
				FragColor[1] = FragColor[1] * TotalTransmitted + SumColor[1];
				FragColor[2] = FragColor[2] * TotalTransmitted + SumColor[2];
				// replace with the transmitted color from farther fragments
				CurFrag->PlotPixel(FragColor);
				} // if
			} // if
		// set alpha to 255 to indicate that it is no longer transparent
		CurFrag->Alpha = 255;
		} // if
	} // for

// set flag so underwater fragments don't reflect
if (LastODFrag)
	LastODFrag->SetFlags(WCS_PIXELFRAG_FLAGBIT_LASTREFLECTABLE);

} // rPixelHeader::RefractPixel

/*===========================================================================*/
/*===========================================================================*/

rPixelBlockHeader::rPixelBlockHeader(long Size)
{

FragMap = NULL;

FragLists = NULL;
NumFragListEntries = LastListUsed = 0;
LastFragUsed = 0;
FragsPerList = NULL;
ReflLists = NULL;
NumReflListEntries = LastReflListUsed = 0;
LastReflUsed = 0;
ReflsPerList = NULL;

if (Size > 0)
	{
	if (FragMap = new rPixelHeader[Size])
		{
		if (! AllocFirstFrags(Size))
			{
			delete [] FragMap;
			FragMap = NULL;
			} // if
		} // if
	} // if

} // rPixelBlockHeader::rPixelBlockHeader

/*===========================================================================*/

rPixelBlockHeader::~rPixelBlockHeader()
{

if (FragMap)
	delete [] FragMap;
FragMap = NULL;
FreeFragLists();
FreeReflLists();

} // rPixelBlockHeader::~rPixelBlockHeader

/*===========================================================================*/

int rPixelBlockHeader::AllocFirstFrags(long NumPixels)
{
long PixelCt;

if (AllocFragList(0, NumPixels))
	{
	for (PixelCt = 0; PixelCt < NumPixels; PixelCt ++)
		{
		FragMap[PixelCt].FragList = &FragLists[0][PixelCt];
		FragMap[PixelCt].FragList->Next = NULL;
		FragMap[PixelCt].FragList->Refl = NULL;
		FragMap[PixelCt].FragList->ZBuf = FLT_MAX;
		FragMap[PixelCt].FragList->Flags = 0;
		FragMap[PixelCt].UsedFrags = 0;
		} // for
	LastFragUsed = NumPixels - 1;
	LastListUsed = 0;
	LastReflUsed = -1;
	LastReflListUsed = 0;
	return (1);
	} // if

return (0);

} // rPixelBlockHeader::AllocFirstFrags

/*===========================================================================*/

rPixelFragment *rPixelBlockHeader::AddFragment(rPixelFragment **AddAt)
{
rPixelFragment *AddMe;

#pragma omp critical (OMP_CRITICAL_ADDPIXFRAG)
if (AddMe = GetNextAvailFrag())
	{
	AddMe->Next = *AddAt;
	*AddAt = AddMe;
	} // if

return (AddMe);

} // rPixelBlockHeader::AddFragment

/*===========================================================================*/

rPixelFragment **rPixelBlockHeader::AllocFragListEntries(long NumEntries)
{

FreeFragLists();
FreeReflLists();

if (FragLists = (rPixelFragment **)AppMem_Alloc(NumEntries * sizeof (rPixelFragment *), APPMEM_CLEAR))
	{
	NumFragListEntries = NumEntries;
	if (! (FragsPerList = (long *)AppMem_Alloc(NumEntries * sizeof (long), APPMEM_CLEAR)))
		{
		AppMem_Free(FragLists, NumFragListEntries * sizeof (rPixelFragment *));
		FragLists = NULL;
		NumFragListEntries = 0;
		} // if
	} // if
LastFragUsed = 0;
LastListUsed = 0;

return (FragLists);

} // rPixelBlockHeader::AllocFragListEntries

/*===========================================================================*/

rPixelFragment **rPixelBlockHeader::AllocMoreFragListEntries(long AddEntries)
{
rPixelFragment **NewFragLists;
long *NewFragsPerList;

if (NewFragLists = (rPixelFragment **)AppMem_Alloc((AddEntries + NumFragListEntries) * sizeof (rPixelFragment *), APPMEM_CLEAR))
	{
	if (! (NewFragsPerList = (long *)AppMem_Alloc((AddEntries + NumFragListEntries) * sizeof (long), APPMEM_CLEAR)))
		{
		AppMem_Free(NewFragLists, (AddEntries + NumFragListEntries) * sizeof (rPixelFragment *));
		} // if
	else
		{
		memcpy(NewFragLists, FragLists, NumFragListEntries * sizeof (rPixelFragment *));
		memcpy(NewFragsPerList, FragsPerList, NumFragListEntries * sizeof (long));
		AppMem_Free(FragLists, NumFragListEntries * sizeof (rPixelFragment *));
		AppMem_Free(FragsPerList, NumFragListEntries * sizeof (long));
		NumFragListEntries += AddEntries;
		FragLists = NewFragLists;
		FragsPerList = NewFragsPerList;
		} // else
	} // if

return (FragLists);

} // rPixelBlockHeader::AllocMoreFragListEntries

/*===========================================================================*/

rPixelFragment *rPixelBlockHeader::AllocFragList(long ListEntry, long NumFrags)
{

if (FragListsValid() || AllocFragListEntries(100))
	{
	// see how many are allocated in first block. If not a full count, remove them and allocate new ones
	if (FragLists[ListEntry] && NumFrags > FragsPerList[ListEntry])
		{
		delete [] FragLists[ListEntry];
		FragLists[ListEntry] = NULL;
		FragsPerList[ListEntry] = 0;
		} // if
	if (! FragLists[ListEntry])
		{
		if (FragLists[ListEntry] = new rPixelFragment[NumFrags])
			FragsPerList[ListEntry] = NumFrags;
		else
			FragsPerList[ListEntry] = 0;
		} // if
	return (FragLists[ListEntry]);
	} // if

return (NULL);

} // rPixelBlockHeader::AllocFragList

/*===========================================================================*/

void rPixelBlockHeader::FreeFragLists(void)
{
long ListCt;

if (FragLists)
	{
	for (ListCt = 0; ListCt < NumFragListEntries; ++ListCt)
		{
		if (FragLists[ListCt])
			delete [] FragLists[ListCt];
		} // for
	AppMem_Free(FragLists, NumFragListEntries * sizeof (rPixelFragment *));
	FragLists = NULL;
	if (FragsPerList)
		AppMem_Free(FragsPerList, NumFragListEntries * sizeof (long));
	FragsPerList = NULL;
	NumFragListEntries = 0;
	} // if

LastFragUsed = 0;
LastListUsed = 0;

} // rPixelBlockHeader::FreeFragLists

/*===========================================================================*/

rPixelFragment *rPixelBlockHeader::GetNextAvailFrag(void)
{
long NextListToUse;
long NextFragToUse;

if (FragLists)
	{
	NextFragToUse = LastFragUsed + 1;
	if (NextFragToUse >= FragsPerList[LastListUsed])
		{
		NextListToUse = LastListUsed + 1;
		if (NextListToUse >= NumFragListEntries)
			{
			AllocMoreFragListEntries(100);
			} // if need new frag list
		if (NextListToUse < NumFragListEntries)
			{
			AllocFragList(NextListToUse, 100000);
			NextFragToUse = 0;
			} // if need new frag list
		} // if
	else
		NextListToUse = LastListUsed;

	if (NextListToUse < NumFragListEntries && NextFragToUse < FragsPerList[NextListToUse])
		{
		LastListUsed = NextListToUse;
		LastFragUsed = NextFragToUse;
		FragLists[LastListUsed][LastFragUsed].Next = NULL;
		FragLists[LastListUsed][LastFragUsed].Refl = NULL;
		FragLists[LastListUsed][LastFragUsed].Flags = 0;
		return (&FragLists[LastListUsed][LastFragUsed]);
		} // if
	} // if

FragMemFailed = 1;

return (NULL);

} // rPixelBlockHeader::GetNextAvailFrag

/*===========================================================================*/

ReflectionData **rPixelBlockHeader::AllocReflListEntries(long NumEntries)
{

FreeReflLists();

if (ReflLists = (ReflectionData **)AppMem_Alloc(NumEntries * sizeof (ReflectionData *), APPMEM_CLEAR))
	{
	NumReflListEntries = NumEntries;
	if (! (ReflsPerList = (long *)AppMem_Alloc(NumEntries * sizeof (long), APPMEM_CLEAR)))
		{
		AppMem_Free(ReflLists, NumReflListEntries * sizeof (ReflectionData *));
		ReflLists = NULL;
		NumReflListEntries = 0;
		} // if
	} // if
LastReflUsed = -1;
LastReflListUsed = 0;

return (ReflLists);

} // rPixelBlockHeader::AllocReflListEntries

/*===========================================================================*/

ReflectionData **rPixelBlockHeader::AllocMoreReflListEntries(long AddEntries)
{
ReflectionData **NewReflLists;
long *NewReflsPerList;

if (NewReflLists = (ReflectionData **)AppMem_Alloc((AddEntries + NumReflListEntries) * sizeof (ReflectionData *), APPMEM_CLEAR))
	{
	if (! (NewReflsPerList = (long *)AppMem_Alloc((AddEntries + NumReflListEntries) * sizeof (long), APPMEM_CLEAR)))
		{
		AppMem_Free(NewReflLists, (AddEntries + NumReflListEntries) * sizeof (ReflectionData *));
		} // if
	else
		{
		memcpy(NewReflLists, ReflLists, NumReflListEntries * sizeof (ReflectionData *));
		memcpy(NewReflsPerList, ReflsPerList, NumReflListEntries * sizeof (long));
		AppMem_Free(ReflLists, NumReflListEntries * sizeof (ReflectionData *));
		AppMem_Free(ReflsPerList, NumReflListEntries * sizeof (long));
		NumReflListEntries += AddEntries;
		ReflLists = NewReflLists;
		ReflsPerList = NewReflsPerList;
		} // else
	} // if

return (ReflLists);

} // rPixelBlockHeader::AllocMoreReflListEntries

/*===========================================================================*/

ReflectionData *rPixelBlockHeader::AllocReflList(long ListEntry, long NumFrags)
{

if (ReflListsValid() || AllocReflListEntries(100))
	{
	// see how many are allocated in first block. If not a full count, remove them and allocate new ones
	if (ReflLists[ListEntry] && NumFrags > ReflsPerList[ListEntry])
		{
		delete [] ReflLists[ListEntry];
		ReflLists[ListEntry] = NULL;
		ReflsPerList[ListEntry] = 0;
		} // if
	if (! ReflLists[ListEntry])
		{
		if (ReflLists[ListEntry] = new ReflectionData[NumFrags])
			ReflsPerList[ListEntry] = NumFrags;
		else
			ReflsPerList[ListEntry] = 0;
		} // if
	return (ReflLists[ListEntry]);
	} // if

return (NULL);

} // rPixelBlockHeader::AllocReflList

/*===========================================================================*/

void rPixelBlockHeader::FreeReflLists(void)
{
long ListCt;

if (ReflLists)
	{
	for (ListCt = 0; ListCt < NumReflListEntries; ++ListCt)
		{
		if (ReflLists[ListCt])
			delete [] ReflLists[ListCt];
		} // for
	AppMem_Free(ReflLists, NumReflListEntries * sizeof (ReflectionData *));
	ReflLists = NULL;
	if (ReflsPerList)
		AppMem_Free(ReflsPerList, NumReflListEntries * sizeof (long));
	ReflsPerList = NULL;
	NumReflListEntries = 0;
	} // if

LastReflUsed = 0;
LastReflListUsed = 0;

} // rPixelBlockHeader::FreeReflLists

/*===========================================================================*/

ReflectionData *rPixelBlockHeader::GetNextAvailRefl(void)
{
long NextReflListToUse;
long NextReflToUse;

if (ReflLists || AllocReflListEntries(100))
	{
	NextReflToUse = LastReflUsed + 1;
	if (NextReflToUse >= ReflsPerList[LastReflListUsed])
		{
		NextReflListToUse = ReflLists[LastReflListUsed] ? LastReflListUsed + 1: LastReflListUsed;
		if (NextReflListToUse >= NumReflListEntries)
			{
			AllocMoreReflListEntries(100);
			} // if need new refl list
		if (NextReflListToUse < NumReflListEntries)
			{
			AllocReflList(NextReflListToUse, 20000);
			NextReflToUse = 0;
			} // if need new refl list
		} // if
	else
		NextReflListToUse = LastReflListUsed;

	if (NextReflListToUse < NumReflListEntries && NextReflToUse < ReflsPerList[NextReflListToUse])
		{
		LastReflListUsed = NextReflListToUse;
		LastReflUsed = NextReflToUse;
		return (&ReflLists[LastReflListUsed][LastReflUsed]);
		} // if
	} // if

FragMemFailed = 1;

return (NULL);

} // rPixelBlockHeader::GetNextAvailRefl

/*===========================================================================*/
/*===========================================================================*/

rPixelHeader *Renderer::rAllocPixelFragmentMap(void)
{

if (Height > 0 && Width > 0)
	{
	if (rPixelBlock = new rPixelBlockHeader(Height * Width))
		{
		return (rPixelFragMap = rPixelBlock->GetFragMap());
		} // if
	} // if

return (NULL);

} // Renderer::rAllocPixelFragmentMap

/*===========================================================================*/

void Renderer::rFreePixelFragmentMap(void)
{

ReportPixelFragResources();
if (rPixelBlock)
	delete rPixelBlock;
rPixelBlock = NULL;
rPixelFragMap = NULL;

} // Renderer::rFreePixelFragmentMap

/*===========================================================================*/

void rPixelBlockHeader::CountAllocatedFragments(unsigned long &FragsAllocated, unsigned long &FragsUsed)
{
long ListCt;

FragsAllocated = 0, FragsUsed = 0;

for (ListCt = 0; ListCt < NumFragListEntries; ++ListCt)
	{
	FragsAllocated += FragsPerList[ListCt];
	if (ListCt < LastListUsed)
		FragsUsed += FragsPerList[ListCt];
	else if (ListCt == LastListUsed)
		FragsUsed += LastFragUsed + 1;
	} // for
	
} // rPixelBlockHeader::CountAllocatedFragments
