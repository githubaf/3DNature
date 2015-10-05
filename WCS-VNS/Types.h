// Types.h
// 'Duh.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_TYPES_H
#define WCS_TYPES_H

#ifdef _WIN32

#undef UBYTE
#undef SBYTE
#undef USHORT
#undef SHORT
#undef ULONG
#undef LONG
#undef FLOAT
#undef DOUBLE

typedef unsigned char UBYTE;
typedef signed char SBYTE;
typedef unsigned short USHORT;
typedef signed short SHORT;
typedef unsigned long ULONG;
typedef signed long LONG;
typedef __int64 UL64;
typedef float FLOAT;
typedef double DOUBLE;
typedef void * APTR;

#endif // _WIN32

typedef double Matx3x3[3][3];
typedef double Matx4x4[4][4];
typedef double Point2d[2];
typedef double Point3d[3];
typedef double Point4d[4];
typedef float Point2f[2];
typedef float Point3f[3];
typedef float Point4f[4];
typedef char DB3LayerList[32];
typedef unsigned long NotifyTag;

//---------------------------------------------------------------------------
// Points in 3D
//---------------------------------------------------------------------------
typedef struct
{
	float x, y, z;

} eberlyPoint3;

//---------------------------------------------------------------------------
// Lines in 3D
//---------------------------------------------------------------------------
typedef struct
{
	// Line is L(t) = b+t*m for any real-valued t
	// Ray has constraint t >= 0, b is the origin of the ray
	// Line segment has constraint 0 <= t <= 1, b and b+m are end points

	eberlyPoint3 b, m;

} Line3;

#ifdef _WIN32
typedef HWND NativeControl;
typedef HWND NativeGUIWin;
typedef HWND NativeDrwWin;
typedef HWND NativeGLWin;
typedef HWND NativeAnyWin;
typedef WORD WIDGETID;
typedef HINSTANCE NativeLoadHandle;
typedef HDC  NativeDrawContext;
typedef HBITMAP NativeBitmap;
typedef HCURSOR NativeCursor;
typedef HGDIOBJ NativeFont;

#endif // _WIN32

typedef unsigned char CornerMask;

// isomorphic to windows RECT
typedef struct
	{
	int xLeft;
	int yTop;
	int xRight;
	int yBot;
	} RC;

union MultiVal
	{
	unsigned long Long;
	unsigned short Short[2];
	unsigned char Char[4];
	}; // union MultiVal

union MegaMultiVal // suitable for MMX/SSE/SSE2/3DNow! ops
	{
	double			Double[2];
	float			Float[4];
	unsigned long	ULong[4];
	long			Long[4];
	unsigned short	UShort[8];
	short			Short[8];
	unsigned char	UByte[16];
	char			Byte[16];
	}; // union MegaMultiVal

struct OldGeoPoint
	{
	double Lat, Lon;
	};

struct edg
	{
	double slope;
	unsigned long ymax, ymin, xpos, xend;
	};

struct RasterPoint
	{
	long x, y;
	}; // RasterPoint

struct RasterPointD
	{
	double x, y;
	}; // RasterPointD

struct coords
	{
	double lat, lon, alt, x, y, z, q;
	};

#endif // WCS_TYPES_H
