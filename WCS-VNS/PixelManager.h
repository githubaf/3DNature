// PixelManager.cpp
// sample code for managing bitmaps composed of multiple pixel fragments
// Written from scratch on 7/5/99 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_PIXELMANAGER_H
#define WCS_PIXELMANAGER_H

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

#undef RGB // stupid Microsoft makes macros with hazardously-common names

class rPixelBlockHeader;

#define WCS_PIXEL_BUFFERFULLBITS	(1 << 6 | 1 << 5 | 1 << 4)
#define WCS_PIXEL_MULTIFRAGBIT		(1 << 7)
#define WCS_PIXEL_COVERAGEPART		~WCS_PIXEL_MULTIFRAGBIT
#define WCS_PIXEL_MAXCOVERAGE		100

#define WCS_PIXEL_BUFFERFULL	((Coverage & WCS_PIXEL_BUFFERFULLBITS) == WCS_PIXEL_BUFFERFULLBITS)
#define WCS_PIXEL_COVERAGE		(WCS_PIXEL_BUFFERFULL ? WCS_PIXEL_MAXCOVERAGE: (Coverage & WCS_PIXEL_COVERAGEPART))
#define WCS_PIXEL_MULTIFRAG		(Coverage & WCS_PIXEL_MULTIFRAGBIT)

// DO NOT !!! use virtual functions or memcpy and realloc will probably break in nine ways from hell
class PixelBase
	{
	protected:

		unsigned char Coverage;

	}; // class PixelBase

class PixelFragment : public PixelBase
	{
	public:

		float ZBuf;
		unsigned char RGB[3];

		PixelFragment();
		~PixelFragment();
		int TestPossibleUsage(float NewZ);
		float GetNearestZ(void);
		PixelFragment *PlotPixel(float NewZ, unsigned char NewCoverage);
		PixelFragment *CreateMultiFragPixel(int NewIsCloser);
		inline int BufferFull(void)	{return (WCS_PIXEL_BUFFERFULL);};
		void CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, float &OutZ, float &OutCoverage);
		void Reset(void);

	}; // class PixelFragment

class PixelHeader : public PixelBase
	{
	public:

		PixelFragment *FragList;
		unsigned char NumFrags[3];

		unsigned char AddFragment(void);

	}; // class PixelHeader

// for use by general renderer in V6

class ReflectionData
	{
	public:
		unsigned short Reflect;
		float Normal[3];

	}; // class ReflectionData

#define WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED		0x10
#define WCS_PIXELFRAG_FLAGBIT_OPTICALDEPTH			0x20
#define WCS_PIXELFRAG_FLAGBIT_LASTREFLECTABLE		0x40
#define WCS_PIXELFRAG_FLAGBIT_HALFFRAGLIMITED		0x80
#define WCS_PIXELFRAG_FLAGBIT_OBJECTYPE				0x0f
// flag values available = 0x40, 0x80
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_TERRAIN	0x00
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SNOW		0x01
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_WATER		0x02
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_SKY		0x03
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CLOUD		0x04
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_CELESTIAL	0x05
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_STAR		0x06
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_3DOBJECT	0x07
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FOLIAGE	0x08
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_FENCE		0x09
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_BACKGROUND	0x0a
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC	0x0b
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_VECTOR		0x0c
#define WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_MASK		0x0d
// object type values available = 0x0e, 0x0f

class rPixelFragment
	{
	public:

		ReflectionData *Refl;
		rPixelFragment *Next;
		float ZBuf;
		unsigned long TopCovg, BotCovg;
		unsigned short Expon;
		unsigned char Alpha, Flags;
		unsigned char RGB[3];

		rPixelFragment();
		~rPixelFragment();
		void PlotPixel(rPixelBlockHeader *BlockHdr, double SourceRGB[3], double SourceReflect, double SourceNormal[3]);
		void PlotPixel(double SourceRGB[3]);
		void ExtractColor(double SourceRGB[3]);
		double FetchMaskedCoverage(void);
		int TestVisible(rPixelFragment *LastFrag, unsigned long &TopMask, unsigned long &BotMask, unsigned long &TestAlpha);
		void SetFlags(unsigned char FlagSet)	{Flags |= FlagSet;};
		unsigned char TestFlags(unsigned char FlagTest)	{return (FlagTest & Flags);};
		unsigned char GetObjectType(void)	{return (Flags & 0x0f);};
		// modifies one color
		static void ExtractClippedExponentialColor(unsigned char &OneColor, unsigned short LocalExpon, int Band);
		// modifies three colors
		static void ExtractClippedExponentialColors(unsigned char *ThreeColors, unsigned short LocalExpon);
		static void ExtractUnclippedExponentialColors(unsigned long *ThreeColors, unsigned short LocalExpon);
		// modifies three colors and exponent
		static void ExtractExponentialColors(unsigned long *ThreeColors, unsigned short &LocalExpon);
		// modifies one color and exponent
		static void ExtractExponentialColor(unsigned long &OneColor, unsigned short &LocalExpon, int Band);

	}; // class rPixelFragment

class rPixelHeader
	{
	public:

		rPixelFragment *FragList;
		unsigned char UsedFrags;

		//static rPixelFragment **FragLists;
		//static long NumFragListEntries, LastListUsed;
		//static unsigned long LastFragUsed, *FragsPerList;
		//static ReflectionData **ReflLists;
		//static long NumReflListEntries, LastReflListUsed;
		//static unsigned long LastReflUsed, *ReflsPerList;

		rPixelHeader();
		~rPixelHeader();
		//int AllocFirstFrags(rPixelHeader *HeaderArray, long NumPixels);
		//rPixelFragment *AddFragment(rPixelFragment **AddAt);
		rPixelFragment *PlotPixel(rPixelBlockHeader *BlockHdr, float NewZBuf, unsigned char NewAlpha, unsigned long NewTopCovg, unsigned long NewBotCovg, 
			int MaxFrags, unsigned char NewFlags);
		void SetLastFrag(rPixelFragment *LastFrag);
		void CollapseMap(rPixelHeader *HeaderArray, long ArraySize, unsigned char *OutRed, unsigned char *OutGreen, unsigned char *OutBlue, 
			float *OutZ, unsigned char *OutCoverage, float *OutReflect, float *OutNormal[3], unsigned short *OutExponent);
		void CollapseMap(rPixelHeader *HeaderArray, long ArraySize, unsigned char *OutRed, unsigned char *OutGreen, unsigned char *OutBlue, 
			float *OutZ, unsigned char *OutCoverage, float *OutReflect, float *OutNormal[3], unsigned short *OutExponent, 
			unsigned char *Out2ndRed, unsigned char *Out2ndGreen, unsigned char *Out2ndBlue, unsigned char *Out2ndCoverage, 
			int HowToCollapse, char TransparentPixelsExist);
		void CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
			float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent, int RejectVectorZ);
		// next three variants are used for texture map export where foliage and terrain might be divided among different outputs
		void CollapsePixelTerInFirstFolInSecond(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
			float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent,
			unsigned char &Out2ndRed, unsigned char &Out2ndGreen, unsigned char &Out2ndBlue, unsigned char &Out2ndCoverage);
		void CollapsePixelTerInFirstBothInSecond(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
			float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent,
			unsigned char &Out2ndRed, unsigned char &Out2ndGreen, unsigned char &Out2ndBlue, unsigned char &Out2ndCoverage);
		void CollapsePixelNoFoliage(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue, 
			float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal, unsigned short *OutExponent);
		void CollapsePixel(double *OutRGB, float &OutZ, unsigned char &OutCoverage, float &OutReflect, float *OutNormal);
		void CollapsePixel(unsigned char &OutRed, unsigned char &OutGreen, unsigned char &OutBlue);
		void RefractMap(rPixelHeader *HeaderArray, long ArraySize);
		void RefractPixel(void);
		float GetFirstZ(void)	{return (FragList ? FragList->ZBuf: FLT_MAX);};
		int CollapseFartherPixel(float CompareZ, double OutColor[3], unsigned char &OutCoverage);
		rPixelFragment **TestPossibleUsage(float TestZ, unsigned long TestTopCovg, unsigned long TestBotCovg);
		rPixelFragment *GetFirstReflective(void);

		/*
		static rPixelFragment **AllocFragListEntries(long NumEntries);
		static rPixelFragment **AllocMoreFragListEntries(long AddEntries);
		static rPixelFragment *AllocFragList(long ListEntry, unsigned long NumFrags);
		static rPixelFragment *GetNextAvailFrag(void);
		static void FreeFragLists(void);
		static int FragListsValid(void)	{return (FragLists && FragsPerList);};
		// reflection list management
		static ReflectionData **AllocReflListEntries(long NumEntries);
		static ReflectionData **AllocMoreReflListEntries(long AddEntries);
		static ReflectionData *AllocReflList(long ListEntry, unsigned long NumFrags);
		static ReflectionData *GetNextAvailRefl(void);
		static void FreeReflLists(void);
		static int ReflListsValid(void)	{return (ReflLists && ReflsPerList);};
		*/
	}; // class rPixelHeader

class rPixelBlockHeader
	{
	public:
		rPixelHeader *FragMap;

		rPixelBlockHeader(long Size);
		~rPixelBlockHeader();

		rPixelFragment **FragLists;
		long NumFragListEntries, LastListUsed;
		long LastFragUsed, *FragsPerList;
		ReflectionData **ReflLists;
		long NumReflListEntries, LastReflListUsed;
		long LastReflUsed, *ReflsPerList;

		rPixelHeader *GetFragMap(void)	{return (FragMap);};
		void CountAllocatedFragments(unsigned long &FragsAllocated, unsigned long &FragsUsed);

		// fragment list management
		rPixelFragment **AllocFragListEntries(long NumEntries);
		rPixelFragment **AllocMoreFragListEntries(long AddEntries);
		rPixelFragment *AllocFragList(long ListEntry, long NumFrags);
		rPixelFragment *GetNextAvailFrag(void);
		void FreeFragLists(void);
		int FragListsValid(void)	{return (FragLists && FragsPerList);};
		// reflection list management
		ReflectionData **AllocReflListEntries(long NumEntries);
		ReflectionData **AllocMoreReflListEntries(long AddEntries);
		ReflectionData *AllocReflList(long ListEntry, long NumFrags);
		ReflectionData *GetNextAvailRefl(void);
		void FreeReflLists(void);
		int ReflListsValid(void)	{return (ReflLists && ReflsPerList);};

		int AllocFirstFrags(long NumPixels);
		rPixelFragment *AddFragment(rPixelFragment **AddAt);

	}; // rPixelBlockHeader

// can we still use FASTCALL???
#ifndef WCSCPP
extern int (*CountBits)(unsigned int a);
#else // WCSCPP
int (*CountBits)(unsigned int a);
#endif // WCSCPP
int CountBits_Std(unsigned int a);
int CountBits_POPCNT(unsigned int a);

#endif // WCS_PIXELMANAGER_H
