/*
###########################################################################
#                                                                         #
#           Copyright (C) 2000, Elumens Corporation                       #
#                                                                         #
#  These coded instructions,  statements,  and computer programs contain  #
#  unpublished    proprietary   information   of   Elumens Corporation    #
#  and are protected by Federal copyright law.  They may not be disclosed #
#  to third parties or copied or duplicated in any form, in whole  or in  #
#  part, without the prior  written consent  of Elumens Corporation.      #
#                                                                         #
###########################################################################
# Suresh Balu
# 919.816.8787
# Nov 26, 2000
*/


#ifndef SPIDLL_H
#define SPIDLL_H

#ifdef _WIN32
#define MSFTCALLCONV 
//#define MSFTCALLCONV  [__stdcall|__cdecl]
#else
#define MSFTCALLCONV
#endif

#ifdef SPILIB_API
	#define SPIDLL_API
#else
	#define SPIDLL_API __declspec(dllimport)
#endif

// w/c vs cpp extern fun.
#ifdef __cplusplus
	#define EXTERN_HEADER_MACRO extern "C" {
	#define EXTERN_FOOTER_MACRO }
#else
	#define EXTERN_HEADER_MACRO
	#define EXTERN_FOOTER_MACRO
#endif

// =-=-=-=-=-=-
// SPI Channel Specifiers
// NOTE: these are mirrored in spiEXT.h
// NOTE: dont forget to update both if any
#define SPI_1C_FRONT	1	// chan[0]

#define SPI_2C_LEFT		2	// chan[1]
#define SPI_2C_RIGHT	4	// chan[2]

#define SPI_3C_LEFT		8	// chan[3]
#define SPI_3C_RIGHT	16	// chan[4]
#define SPI_3C_TOP		32	// chan[5]

#define SPI_4C_LEFT		8	// chan[3] - aliased to SPI_3C_LEFT
#define SPI_4C_RIGHT	16	// chan[4] - aliased to SPI_3C_RIGHT
#define SPI_4C_TOP		32	// chan[5] - aliased to SPI_3C_TOP
#define SPI_4C_BOTTOM	64	// chan[6]

#define SPI_OC_FRONT	128	// chan[8]
#define SPI_2C_INSERT	128	// chan[8]
#define SPI_2C_BORDER	256	// chan[7]

// convience tokens
#define SPI_ALL_CHAN	SPI_1C_FRONT | SPI_2C_LEFT | SPI_2C_RIGHT | SPI_4C_LEFT | SPI_4C_RIGHT | SPI_4C_TOP | SPI_4C_BOTTOM | SPI_OC_FRONT | SPI_2C_INSERT | SPI_2C_BORDER
#define SPI_ALL_2_CHAN	SPI_2C_LEFT | SPI_2C_RIGHT
#define SPI_ALL_3_CHAN	SPI_3C_LEFT | SPI_3C_RIGHT | SPI_3C_TOP
#define SPI_ALL_4_CHAN	SPI_4C_LEFT | SPI_4C_RIGHT | SPI_4C_TOP | SPI_4C_BOTTOM


#define SPI_L_CHAN		1024 // if in the token string, this call is for 
							 // the left mesh.  NOTE: Left is GENERALLY assumed.
#define SPI_R_CHAN		2048 // if in the token string, this call is for 
							 // the right mesh.  NOTE: Right is ALWAYS checked for.

#define SPI_NUM_CHAN	9	// number of channels in the array in the context.

// =-=-=-=-=-=-
// =-=-=-



// =-=-=-=-=-=-
// SPI Pixel Format specifiers
#define SPI_PF_NONE				0
#define SPI_PF_BACKBUFFER		1
#define SPI_PF_NORMAL			1
#define SPI_PF_PBUFFER			2
#define SPI_PF_TEXTURE			4
#define SPI_PF_AUTO				6
#define SPI_PF_STEREO			8

// =-=-=-=-=-=-
// SPI Lens Configuration
// D's Lens options : 220 degree or default of 180 degrees.
// 
#define SPI_PF_LENS_220			16	// 220 Degree Lens
#define SPI_PF_LENS_180			0	// 180 Degree Lens

//#define EMPTY					32
//#define EMPTY					64
//#define EMPTY					128
//#define EMPTY					256

// channel combinations to use,
// spiclops, biclops, triclops or 
// qudracolps.  can use more than one
// at any given point in time.  fun fun.
#define SPI_PF_1_CHAN			512
#define SPI_PF_2_CHAN			1024
#define SPI_PF_3_CHAN			2048
#define SPI_PF_4_CHAN			4096
#define SPI_PF_O_CHAN			8192
#define SPI_PF_2_CTR_CHAN		8192*2

// =-=-=-=-=-=-
// =-=-=-


// =-=-=-=-=-=-
// SPI FlushModes

#define SPI_IMAGE			0
#define SPI_WIRE			1
#define SPI_OVERLAY			2

// =-=-=-=-=-=-
// =-=-=-


// =-=-=-=-=-=-
// spiScaleTbl indicies for each dome

#define SPI_VS	0
#define SPI_V3	1
#define SPI_V4	2
#define SPI_V5	3
#define SPI_V7	4

// =-=-=-=-=-=-
// =-=-=-

//=========================================================================
// SPI API Exported Functions
//=========================================================================
EXTERN_HEADER_MACRO

	// Functions exported
	SPIDLL_API void* MSFTCALLCONV spiInitialize  ( void* _s, int _pformat );
	SPIDLL_API void  MSFTCALLCONV spiDestroy     ( void* _s );

	SPIDLL_API void  MSFTCALLCONV spiPreRender   ( void* _s, int _wall );
	SPIDLL_API void  MSFTCALLCONV spiPostRender  ( void* _s, int _wall );

	SPIDLL_API void  MSFTCALLCONV spiFlush       ( void* _s, int _wall );
	SPIDLL_API void  MSFTCALLCONV spiBegin       ( void* _s );
	SPIDLL_API void  MSFTCALLCONV spiEnd         ( void* _s );



	// channel specific SET calls
	SPIDLL_API void	 MSFTCALLCONV spiSetChanFOV       ( void* _s, int _wall, float _fovH,  float _fovV );
	SPIDLL_API void  MSFTCALLCONV spiSetChanPosition  ( void* _s, int _wall, float _heading, float  _pitch, float _roll );
	SPIDLL_API int   MSFTCALLCONV spiSetChanSize      ( void* _s, int _wall, int _w, int _h );
	SPIDLL_API int   MSFTCALLCONV spiSetChanTextureID ( void* _s, int _wall, int _id );
	SPIDLL_API void  MSFTCALLCONV spiSetChanTessLevel ( void* _s, int _wall, int _tl );
	SPIDLL_API void  MSFTCALLCONV spiSetChanOrigin    ( void* _s, int _wall, int x, int y );
	SPIDLL_API void  MSFTCALLCONV spiSetChanEyePosition   ( void* _s, int _wall, float _x, float _y, float _z );
	SPIDLL_API void  MSFTCALLCONV spiSetChanLensPosition  ( void* _s, int _wall, float _x, float _y, float _z );

	// nonspecific SET fcns
	SPIDLL_API void	 MSFTCALLCONV spiSetLensFOV      ( void* _s, float _lensFOV );
	SPIDLL_API void  MSFTCALLCONV spiSetNearFar      ( void* _s, double _n, double _f );
	SPIDLL_API void  MSFTCALLCONV spiSetFlushMode    ( void* _s, int _m );
	SPIDLL_API void  MSFTCALLCONV spiSetPBufferSize  ( void* _s, int   _w, int   _h );
	SPIDLL_API void  MSFTCALLCONV spiSetIOD			 ( void* _s, double _iod );
	SPIDLL_API void  MSFTCALLCONV spiSetScreenOrientation( void* _s, double h, double p, double r );
	SPIDLL_API void  MSFTCALLCONV spiSetFlushFrustum ( void* _s, double _left, double _right, double _bottom, double _top, double _hither, double _yon );
	SPIDLL_API void  MSFTCALLCONV spiSetDomeType     ( void* _s, int _pfd );

	// channel specific GET calls
	SPIDLL_API void	 MSFTCALLCONV spiGetChanFOV       ( void* _s, int _wall, float* _fovH,  float* _fovV );
	SPIDLL_API void  MSFTCALLCONV spiGetChanSize      ( void* _s, int _wall, int* _w, int* _h );
	SPIDLL_API void  MSFTCALLCONV spiGetChanTextureID ( void* _s, int _wall, int* _id );
	SPIDLL_API void  MSFTCALLCONV spiGetChanTessLevel ( void* _s, int _wall, int* _tl );
	SPIDLL_API void  MSFTCALLCONV spiGetChanOrigin    ( void* _s, int _wall, int* _x, int* _y );
	SPIDLL_API void  MSFTCALLCONV spiGetChanEyePosition   ( void* _s, int _wall, float* _x, float* _y, float* _z );
	SPIDLL_API void  MSFTCALLCONV spiGetChanLensPosition  ( void* _s, int _wall, float* _x, float* _y, float* _z );
	SPIDLL_API void  MSFTCALLCONV spiGetChanPosition      ( void* _s, int _wall, float* _h, float* _p, float* _r );

	// nonspecific GET fcns
	SPIDLL_API void  MSFTCALLCONV spiGetVersion      ( void* _s, char * );
	SPIDLL_API void  MSFTCALLCONV spiGetNearFar      ( void* _s, double* _n, double* _f );
	SPIDLL_API void  MSFTCALLCONV spiGetFlushMode    ( void* _s, int* _m );
	SPIDLL_API void  MSFTCALLCONV spiGetPBufferSize  ( void* _s, int _wall, int* _w, int* _h );



	SPIDLL_API void  MSFTCALLCONV spiGetIOD          ( void* _s, double* _iod );
	SPIDLL_API int   MSFTCALLCONV spiGetLastError    ( void* _s );
	SPIDLL_API void  MSFTCALLCONV spiGetPickCoords   ( void* _s, int _walls, float _in[2], int *_iWall, float _out[2], double _prj[16], int _vp[4] );
	SPIDLL_API int   MSFTCALLCONV spiGetContextSize  ( void* _s );
	SPIDLL_API int   MSFTCALLCONV spiGetPixelFormat  ( void* _s );
	SPIDLL_API void  MSFTCALLCONV spiGetScreenOrientation( void* _s, double* h, double* p, double* r );
	SPIDLL_API void  MSFTCALLCONV spiGetFlushFrustum( void* _s, double* _left, double* _right, double* _bottom, double* _top, double* _hither, double* _yon );

	SPIDLL_API void  MSFTCALLCONV spiReleaseChanTextureID( void* _s, int _wall );

	// Projection
	SPIDLL_API void  MSFTCALLCONV spiSetDrawFunction ( void*, void (*)(int) );

	// =-=-=-
	// =-=-
	
	// =-=-=-=-=-=-
	// Research Functions
	SPIDLL_API void  MSFTCALLCONV spiSetScale		( void* _s, double _scale );
	SPIDLL_API void  MSFTCALLCONV spiSetWorldScale	( void* _s, double _scale );
	SPIDLL_API void  MSFTCALLCONV spiSetVSModel      ( void* _s, int _idx );

	SPIDLL_API void  MSFTCALLCONV spiGetScale		( void* _s, double* _scale );

	SPIDLL_API void  MSFTCALLCONV spiSetLensWorldPos( void* _s, int _wall, float _x, float _y, float _z );
	SPIDLL_API void  MSFTCALLCONV spiSetEyeWorldPos ( void* _s, int _wall, float _x, float _y, float _z );
// =-=-=-

	// =-=-=-
	// UNEXPOSED FUNCTIONS
	SPIDLL_API void MSFTCALLCONV spiSetMatrixOps( void* _s, int _m );
	SPIDLL_API int  MSFTCALLCONV spiSetDrawBuffer( void* _s, int _pfd );
	SPIDLL_API void MSFTCALLCONV spiSetChanRenderToTextureSize( void* _s, int _wall, int _w, int _h );
	SPIDLL_API int  MSFTCALLCONV spiCreateChannelGroup ( void* _s, int _cg );
	SPIDLL_API void MSFTCALLCONV spiDestroyChannelGroup( void* _s, int _cg );
	// =-=-=-
	// =-=-

EXTERN_FOOTER_MACRO

#endif

// 50, 16, 18.25


