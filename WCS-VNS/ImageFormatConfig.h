// ImageFormatConfig.h
// Container for managing defines for various optionally-supported image formats
// to declutter our overloaded defines window

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMAGE_FORMAT_CONFIG_H
#define WCS_IMAGE_FORMAT_CONFIG_H

#ifdef WCS_BUILD_VNS
	#define WCS_BUILD_IMAGEFORMATBIL_SUPPORT
	#define WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT
	#define WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT
	// We define this in Visual's Project Settings now
	// since not everyone has the QuickTime SDK installed
	//#define WCS_BUILD_QUICKTIME_SUPPORT
	#define WCS_BUILD_JPEG_SUPPORT
	#define WCS_BUILD_PNG_SUPPORT
	#define WCS_BUILD_TIFF_SUPPORT
	#define WCS_BUILD_GEOTIFF_SUPPORT
	#define WCS_BUILD_WORLDFILE_SUPPORT
	#define WCS_BUILD_AVI_SUPPORT
	#ifndef WCS_BUILD_GARY // Gary can't link ECW functions for some reason
	#define WCS_BUILD_ECW_SUPPORT
	//#define WCS_BUILD_ECWDEM_SUPPORT
	#endif // WCS_BUILD_GARY
	#define WCS_BUILD_JP2_SUPPORT

	// Currently not fully implemented
	#define WCS_BUILD_MRSID_SUPPORT
	//WCS_BUILD_MRSID__WRITE_SUPPORT

	#if defined(WCS_BUILD_ECW_SUPPORT) || defined(WCS_BUILD_MRSID_SUPPORT)
	#define WCS_BUILD_REMOTEFILE_SUPPORT
	#endif // WCS_BUILD_ECW_SUPPORT

#endif // WCS_BUILD_VNS

// WCS 6
#ifdef WCS_BUILD_W6
	#define WCS_BUILD_HDR_SUPPORT
	#define WCS_BUILD_JPEG_SUPPORT
	#define WCS_BUILD_PNG_SUPPORT
	#define WCS_BUILD_TIFF_SUPPORT
	#define WCS_BUILD_AVI_SUPPORT
#endif // WCS_BUILD_W6

// VNS 2
#ifdef WCS_BUILD_V2
	#define WCS_BUILD_HDR_SUPPORT
#endif // WCS_BUILD_V2

// Scene Express
#ifdef WCS_BUILD_RTX
	#define WCS_BUILD_SGIRGB_SUPPORT
#endif // WCS_BUILD_RTX

#endif // !WCS_IMAGE_FORMAT_CONFIG_H
