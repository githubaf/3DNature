// Types.h
// 'Duh.

#ifndef WCS_TYPES_H
#define WCS_TYPES_H

#ifdef _WIN32
#include <windows.h>

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
typedef unsigned short int USHORT;
typedef signed short int SHORT;
typedef unsigned long int ULONG;
typedef signed long int LONG;
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
typedef unsigned long int NotifyTag;

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

#ifdef AMIGA
typedef struct Window *NativeDrwWin;
typedef struct Window *NativeGLWin;
typedef struct BitMap *NativeBitmap;
typedef void * NativeAnyWin;
typedef APTR NativeControl;
typedef struct RastPort *NativeDrawContext;
typedef APTR HICON;
typedef APTR NativeGUIWin; // MUI Window
typedef APTR NativeCursor;
typedef void * NativeFont;
typedef struct Window * NativeDrwWin; // Intuition Window
typedef unsigned short int WIDGETID; // MUI Gadget object
typedef long NativeLoadHandle; // BPTR
#endif // AMIGA

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
	long Long;
	short Short[2];
	char Char[4];
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
	unsigned long ymax, ymin, xpos, xend;
	double slope;
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
