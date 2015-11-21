/********************************************************
** Copyright 1999 Earth Resource Mapping Ltd.
** This document contains proprietary source code of
** Earth Resource Mapping Ltd, and can only be used under
** one of the three licenses as described in the 
** license.txt file supplied with this distribution. 
** See separate license.txt file for license details 
** and conditions.
**
** This software is covered by US patent #6,442,298,
** #6,102,897 and #6,633,688.  Rights to use these patents 
** is included in the license agreements.
**
** FILE:   	NCSDefs.h
** CREATED:	Tue Mar 2 09:19:00 WST 1999
** AUTHOR: 	Simon Cope
** PURPOSE:	General NCS defines
** EDITS:
** [01] sjc 30Apr00 Merged Mac SDK port
** [02]  ny 03Nov00 Merge WinCE/PALM SDK changes
** [03] tfl 14Jul04 Added radians/degrees conversion factors
 *******************************************************/

#ifndef NCSDEFS_H
#define NCSDEFS_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef NCSTYPES_H
#include "NCSTypes.h"
#endif

#include <float.h>

#ifdef LINUX
#ifndef NULL
#define NULL 0
#endif
#endif

#ifdef HPUX
// This should be defined as "inline", but the compiler doenst 
// like our "static inline type function()" prototypes, probably 
// have to change them all to static type inline function()"
// Use the next line when this is sorted.
//#define __inline inline
#define __inline
#endif

#if defined(MACINTOSH)||defined(SOLARIS)||defined(IRIX)||defined(PALM)||defined(HPUX)||defined(MACOSX)
#define NCSBO_MSBFIRST
#else	// WIN32, LINUX (i386)
#define NCSBO_LSBFIRST
#endif

#if !defined(_WIN32_WCE)&&!defined(UNALIGNED)
#define UNALIGNED
#endif	/* !_WIN32_WCE */

#ifdef PALM
// FIXME
#define NCS_PALM_CREATOR_ID 'NCS1'
#endif

#ifdef HPUX
// The system headers clash, so use their versions of MAX/MIN instead.
#include <sys/param.h>

#else

#ifndef MAX
#define MAX(a, b) ((a) > (b) ? (a) : (b))
#endif
#ifndef MIN
#define MIN(a, b) ((a) > (b) ? (b) : (a))
#endif

#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifdef WIN32

#ifndef MAXDOUBLE
#define MAXDOUBLE   DBL_MAX			//1.7976931348623158e+308
#define NCS_FQNAN	_FPCLASS_QNAN	//0x0002
#define NCS_NAN		_FPCLASS_SNAN
#endif	/* !MAXDOUBLE */

#if defined(_WIN32_WCE)
#define NCS_NO_UNALIGNED_ACCESS
#endif // _WIN32_WCE

#elif defined PALM

#define NCS_FQNAN	0x0002

#elif defined MACINTOSH

#define NCS_FQNAN	0x0002
#define NCS_NAN		NAN
#ifndef MAXDOUBLE
#define MAXDOUBLE   DBL_MAX
#endif

#elif defined MACOSX
//#include <values.h>
#include <limits.h>
#include <ctype.h>

#define NCS_FQNAN	0x0002
#define NCS_NAN		NAN
#ifndef MAXDOUBLE
#define MAXDOUBLE   DBL_MAX
#endif

//FIXME: These should probably be moved somewhere else
#define HDC			CGrafPtr
#define LPRECT		Rect *
#define RECT		Rect
#define HBITMAP		PixMapHandle
#define COLORREF	DWORD
#define HRGN		RgnHandle
#define UINT		UINT32
#define HWND		DWORD
#define STDMETHOD(x)	virtual INT32 x
#define LRESULT	INT32
#define HRESULT	long
#define WPARAM	UINT32
#define LPARAM 	INT32
#define VARIANT	void *
#define HPEN		DWORD
#define HBRUSH	DWORD
#define BSTR		short *
#define BOOL		BOOLEAN
#define LPSTR	char *
#define HCURSOR	CursHandle
#define WORD		UINT16
#define STDMETHODIMP	INT32
//#define boolean	BOOLEAN
#define LOWORD(x)		((UINT16)((long)(x) & 0xffff))
#define HIWORD(x)		((UINT16)((long)(x) >> 16))
#define S_OK     ((LRESULT)0x00000000L)
#define S_FALSE  ((LRESULT)0x00000001L)
#define SUCCEEDED(Status) 	((Status) >= 0)
#define FAILED(Status) 		((Status)<0)
#define MB_OK
#define RGB(r,g,b) ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))
#define GetRValue(rgb)      ((BYTE)(rgb))
#define GetGValue(rgb)      ((BYTE)(((WORD)(rgb)) >> 8))
#define GetBValue(rgb)      ((BYTE)((rgb)>>16))
#define USES_CONVERSION
#define MK_LBUTTON	0x0001
#define MK_RBUTTON	0x0002
#define MK_SHIFT		0x0004
#define MK_CONTROL	0x0008
#define MK_MBUTTON	0x0010
#define VK_SHIFT		0x10
#define SetCursor(X) 0; if( X ) SetCursor(*X)
#define SetCapture(X)
#define ReleaseCapture(X)
#define IntersectRect(A, B, C) SectRect(B, C, A)
#define 	ZeroMemory(DATA, SIZE) memset(DATA, 0, SIZE)

#elif defined SOLARIS || defined LINUX || defined HPUX

#ifdef LINUX
#define wcsicmp wcscasecmp
#endif

#include <values.h>
#include <limits.h>
#include <ctype.h>

// Solaris can't access types on unaligned addressed
#define NCS_NO_UNALIGNED_ACCESS

#if defined(SOLARIS) || defined(HPUX)
// SPARC has slow BYTE bit ops
#define NCS_SLOW_CPU_BYTE_OPS
#endif

#else

#error DEFINE SYSTEM INCLUDES FOR TYPES

#endif	/* WIN32 */

typedef enum {
	NCSCS_RAW	= 0,
	NCSCS_UTM	= 1,
	NCSCS_LL	= 2
} NCSCoordSys;

/*Coodinate system defines*/
#define NCS_LINEAR_COORD_SYS	"linear"
#define	NCS_FEET_FACTOR			0.30480061
/*[03]*/
#define NCS_RADIANS_TO_DEGREES	57.29577951L
#define NCS_DEGREES_TO_RADIANS	1.745329252E-2L	

#if defined(WIN32)

#ifdef _MANAGED

#define NCS_EXPORT
#define NCS_IMPORT
#define NCS_GC __gc
#define NCS_VALUE __value
#define NCS_PUBLIC public
#define NCS_PROTECTED NCS_PUBLIC
#define NCS_PRIVATE NCS_PUBLIC
#ifdef __cplusplus
#using <mscorlib.dll>
#endif //__cplusplus

#else // _MANAGED

#if defined(_LIB)||defined(NCSECW_STATIC_LIBS)
#define NCS_EXPORT
#define NCS_IMPORT
#else
#define NCS_EXPORT __declspec(dllexport)
#define NCS_IMPORT __declspec(dllimport)
#endif

#define NCS_GC 
#define NCS_VALUE 
#define NCS_PUBLIC public
#define NCS_PROTECTED protected
#define NCS_PRIVATE private

#endif // _MANAGED

#ifdef _WIN32_WCE
#define NCS_CALL
#else
#define NCS_CALL __cdecl
#endif

#define NCS_CB_CALL __cdecl

#else // WIN32

#define NCS_EXPORT
#define NCS_IMPORT
#define NCS_GC
#define NCS_VALUE
#define NCS_PUBLIC public
#define NCS_PROTECTED protected
#define NCS_PRIVATE private

#define NCS_CALL
#define NCS_CB_CALL
#endif // WIN32

#ifndef MAX_PATH
#ifdef MACINTOSH

	//	Note: Verify that this is OK for all MAC/OS Platform
#define MAX_PATH	256

#elif defined PALM

#define MAX_PATH 	1024

#elif defined SOLARIS || defined LINUX || defined HPUX

#define MAX_PATH	PATH_MAX

#elif defined MACOSX

#define MAX_PATH 1024

#else	/* PALM */

#define MAX_PATH	PATHNAMELEN

#endif
#endif	/* !MAX_PATH */

#define NCSIsNullString(s) ((s) == (char *)0 || (*(s)) == '\0')

#if	__ICL >= 700
#ifdef NCS_VECTOR_CC
//Note: need /QaxMiKW /Qvec_report3 /Qrestrict ICC flags to use vectorisation
#define NCS_RESTRICT restrict
#else
#define NCS_RESTRICT
#endif
#else
#define NCS_RESTRICT
#endif

#ifdef _OPENMP
#define NCS_OPENMP
#endif // _OPENMP

#ifdef NCS_OPENMP
#define NCS_VECTOR_CC
#endif // NCS_OPENMP

#ifndef NCS_INLINE
#ifdef WIN32
#define NCS_INLINE __forceinline
#elif defined __GNUC__
#define NCS_INLINE __inline__
#else
#define NCS_INLINE __inline
#endif // WIN32
#endif // NCS_INLINE

#ifndef NCS_FASTCALL
#if defined(WIN32)&&!defined(_MANAGED)
#define NCS_FASTCALL __fastcall
#else // WIN32
#define NCS_FASTCALL
#endif // WIN32
#endif // NCS_FASTCALL

#ifdef __cplusplus
}
#endif

#endif /* NCSDEFS_H */
