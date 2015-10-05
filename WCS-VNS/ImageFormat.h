// ImageFormat.h
// header for the output image formats
// Created from scratch on 11/29/99 by Chris "Xenon" Hanson"
// Copyright 1999 by 3DNature. All rights reserved.

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMAGE_FORMAT_H
#define WCS_IMAGE_FORMAT_H

#include "ImageFormatConfig.h"

#ifdef WCS_BUILD_AVI_SUPPORT
#include <vfw.h> // Video for Windows, not Veterans of Foreign Wars
#endif // WCS_BUILD_AVI_SUPPORT

#ifdef WCS_BUILD_QUICKTIME_SUPPORT
#include "ImageFormatQT.h"
#endif // WCS_BUILD_QUICKTIME_SUPPORT

class ImageFormat;
class ImageOutputEvent;
class BufferNode;
class Camera;
class Joe;
class PlanetOpt;
class RasterBounds;

#include "Raster.h"
#include "ImageOutputEvent.h"
#ifdef WCS_BUILD_ECW_SUPPORT
// pick up ECW defines from here.
#include "ImageInputFormat.h"
#endif // WCS_BUILD_ECW_SUPPORT
#include "PixelManager.h"
#include "FeatureConfig.h"

#define WCS_IMAGE_FORMAT_CHAR_CHANNELS_MAX	WCS_MAX_IMAGEOUTBUFFERS
#define WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX	WCS_MAX_IMAGEOUTBUFFERS
#define WCS_IMAGE_FORMAT_SHORT_CHANNELS_MAX	WCS_MAX_IMAGEOUTBUFFERS

// set WCS_ECW_MAXVALUE & WCS_ECW_MINVALUE for the value range acceptable to ECW. Used in ECWDEM elevation file saver
#define WCS_ECW_MINVALUE   ((float)0)
#define WCS_ECW_MAXVALUE8  ((float)255)
#define WCS_ECW_MAXVALUE16 ((float)65535)
#define WCS_ECW_MAXVALUE28 ((float)0x0fffffff)
#define WCS_ECW_MAXVALUE32 ((float)4294967295)

class ImageFormat
	{
	private:
		void ConstructClear(void);
	protected:
		ImageOutputEvent *IOE;
		FILE *fHandle;
		unsigned char *TransferBuf, FormatIndex;
		char *CompletePath;
		// These are just temporary working pointers
		BufferNode *CharChannelNode[WCS_IMAGE_FORMAT_CHAR_CHANNELS_MAX],
		 *FloatChannelNode[WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX], *ShortChannelNode[WCS_IMAGE_FORMAT_SHORT_CHANNELS_MAX];
		unsigned char *CharChannelData[WCS_IMAGE_FORMAT_CHAR_CHANNELS_MAX];
		float *FloatChannelData[WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX];
		unsigned short *ShortChannelData[WCS_IMAGE_FORMAT_SHORT_CHANNELS_MAX];
		float FCDMax[WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX],
		 FCDMin[WCS_IMAGE_FORMAT_FLOAT_CHANNELS_MAX];

	public:
		ImageFormat();
		// if derived classes have any dynamic data, this needs to be virtual.
		// They don't yet, so it isn't.
		~ImageFormat();

		void SetIOE(ImageOutputEvent *NewIOE) {IOE = NewIOE;};
		void SetCompleteOutputPath(char *OutPath);
		char *GetCompleteOutputPath(void);
		unsigned char GetFormatIndex(void);
		char *GetName(void);
		virtual int StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime) {return(1);};
		virtual int StartVectorFrame(long BufWidth, long BufHeight, long Frame) {return(1);};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame) = 0;
		virtual int EndVectorFrame(void) {return(1);};
		virtual int StartField(char Even) {return(1);};
		virtual int EndAnim(void) {return(1);};
		virtual void SetDataRange(float MaxValue, float MinValue)	{return;};
	}; // class ImageFormat

class ImageFormatIFF: public ImageFormat
	{
	public:
		char ZAsGrey;
		ImageFormatIFF() {FormatIndex = WCS_IMAGEFILEFORMAT_IFF; ZAsGrey = 0;};
		void SetZAsGrey(char State) {ZAsGrey = State;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatIFF

class ImageFormatBMP: public ImageFormat
	{
	public:
		ImageFormatBMP() {FormatIndex = WCS_IMAGEFILEFORMAT_BMP;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatBMP

class ImageFormatBT: public ImageFormat
	{
	public:
		ImageFormatBT() {FormatIndex = WCS_IMAGEFILEFORMAT_BT;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatBT

class ImageFormatWCSELEV: public ImageFormat
	{
	public:
		ImageFormatWCSELEV() {FormatIndex = WCS_IMAGEFILEFORMAT_WCSELEV;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatWCSELEV

#ifdef WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT
class ImageFormatARCASCII: public ImageFormat
	{
	public:
		ImageFormatARCASCII() {FormatIndex = WCS_IMAGEFILEFORMAT_ARCASCII;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatARCASCII
#endif // WCS_BUILD_IMAGEFORMATARCASCII_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATSTL_SUPPORT
class ImageFormatSTL: public ImageFormat
	{
	public:
		ImageFormatSTL() {FormatIndex = WCS_IMAGEFILEFORMAT_STL;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatSTL
#endif // WCS_BUILD_IMAGEFORMATSTL_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATBIL_SUPPORT
class ImageFormatBIL: public ImageFormat
	{
	public:
		ImageFormatBIL() {FormatIndex = WCS_IMAGEFILEFORMAT_BIL;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatBIL
#endif // WCS_BUILD_IMAGEFORMATBIL_SUPPORT

#ifdef WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT
class ImageFormatGRIDFLOAT: public ImageFormat
	{
	public:
		ImageFormatGRIDFLOAT() {FormatIndex = WCS_IMAGEFILEFORMAT_GRIDFLOAT;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatGRIDFLOAT
#endif // WCS_BUILD_IMAGEFORMATGRIDFLOAT_SUPPORT

class ImageFormatTGA: public ImageFormat
	{
	private:
		char TGA_ID[70], TGA_Dummy[5];
	public:
		ImageFormatTGA() {FormatIndex = WCS_IMAGEFILEFORMAT_TGA;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatTGA

#ifdef WCS_BUILD_SGIRGB_SUPPORT
class ImageFormatSGIRGB: public ImageFormat
	{
	public:
		ImageFormatSGIRGB() {FormatIndex = WCS_IMAGEFILEFORMAT_SGIRGB;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatSGIRGB
#endif // WCS_BUILD_SGIRGB_SUPPORT

#ifdef WCS_BUILD_HDR_SUPPORT
class ImageFormatRGBE: public ImageFormat
	{
	public:
		ImageFormatRGBE() {FormatIndex = WCS_IMAGEFILEFORMAT_RGBE;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatRGBE

class ImageFormatFLX: public ImageFormat
	{
	public:
		ImageFormatFLX() {FormatIndex = WCS_IMAGEFILEFORMAT_FLX;};
		virtual int StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime);
		virtual int EndAnim(void);
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatHDR
#endif // WCS_BUILD_HDR_SUPPORT

class ImageFormatPICT: public ImageFormat
	{
	public:
		ImageFormatPICT() {FormatIndex = WCS_IMAGEFILEFORMAT_PICT;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatPICT

#ifdef WCS_BUILD_ECW_SUPPORT
class ImageFormatECW: public ImageFormat
	{
	public:
		unsigned int ECWBands, ECWWidth;
		ImageFormatECW() {FormatIndex = WCS_IMAGEFILEFORMAT_ECW;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		BOOLEAN ECWWriteReadCallbackMethod(unsigned int nNextLine, float **ppInputArray);
		void CreateECWGeoRef(CoordSys *CS, NCSEcwCompressClient *pClient, RasterBounds *RBounds);
		}; // ImageFormatECW
#endif // WCS_BUILD_ECW_SUPPORT

#if defined WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_ECWDEM_SUPPORT && defined WCS_BUILD_SX2
class ECWDEMCompressor;

class ImageFormatECWDEM: public ImageFormatECW
	{
	public:
		float HighElev, LowElev, ElevScale, MaxValue;
		char BitDepth, JP2;
		ImageFormatECWDEM() {FormatIndex = WCS_IMAGEFILEFORMAT_ECWDEM; HighElev = LowElev = ElevScale = 1.0f; MaxValue = WCS_ECW_MAXVALUE8; BitDepth = 8; JP2 = 0;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		void CreateECWGeoRefNewAPI(CoordSys *CS, NCSFileViewFileInfoEx *pClient, RasterBounds *RBounds);
		virtual void SetDataRange(float MaxValue, float MinValue)	{HighElev = MaxValue; LowElev = MinValue;};
		void SetJP2(char NewJP2) {JP2 = NewJP2;};
		char GetJP2(void) {return(JP2);};
		BOOLEAN WriteReadLineCallback(UINT32 nNextLine, void **ppInputArray, UINT32 Width, NCSEcwCellType CellType);
		}; // ImageFormatECWDEM
#endif // WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_ECWDEM_SUPPORT && defined WCS_BUILD_SX2

// WCS_BUILD_JP2_SUPPORT
// There is no JP2 saver, we just use the ECW saver

#ifdef WCS_BUILD_MRSID_WRITE_SUPPORT
class ImageFormatMRSID: public ImageFormat
	{
	public:
		ImageFormatMRSID() {FormatIndex = WCS_IMAGEFILEFORMAT_MRSID;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatMRSID
#endif // WCS_BUILD_MRSID_WRITE_SUPPORT


#ifdef WCS_BUILD_JPEG_SUPPORT
class ImageFormatJPEG: public ImageFormat
	{
	public:
		ImageFormatJPEG() {FormatIndex = WCS_IMAGEFILEFORMAT_JPEG;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatJPEG
#endif // WCS_BUILD_JPEG_SUPPORT

#ifdef WCS_BUILD_TIFF_SUPPORT
class ImageFormatTIFF: public ImageFormat
	{
	public:
		ImageFormatTIFF() {FormatIndex = WCS_IMAGEFILEFORMAT_TIFF;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatTIFF
#endif // WCS_BUILD_TIFF_SUPPORT

#ifdef WCS_BUILD_PNG_SUPPORT
class ImageFormatPNG: public ImageFormat
	{
	public:
		ImageFormatPNG() {FormatIndex = WCS_IMAGEFILEFORMAT_PNG;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatPNG
#endif // WCS_BUILD_PNG_SUPPORT

class ImageFormatRAW: public ImageFormat
	{
	char Interleave, ForceByte;
	public:
		ImageFormatRAW() {Interleave = ForceByte = 0; FormatIndex = WCS_IMAGEFILEFORMAT_PICT;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		void SetInterleave(char State) {Interleave = State;};
		void SetForceByte (char State) {ForceByte  = State;};
	}; // ImageFormatRAW

class ImageFormatZBUF: public ImageFormat
	{
	public:
		ImageFormatZBUF() {FormatIndex = WCS_IMAGEFILEFORMAT_IFFZBUF;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatZBUF

class ImageFormatZPIC: public ImageFormat
	{
	public:
		ImageFormatZPIC() {FormatIndex = WCS_IMAGEFILEFORMAT_PICT;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatZPIC

class ImageFormatAI: public ImageFormat
	{
	public:
		ImageFormatAI() {FormatIndex = WCS_IMAGEFILEFORMAT_AI;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		virtual int StartVectorFrame(long BufWidth, long BufHeight, long Frame);
		long IllustratorInit(double VecDPI, double RasDPI, long screenwidth, long screenheight, long Frame);
		long WriteIllustratorVector(CoordSys *DefCoords, Camera *Cam, PlanetOpt *PO, double VecDPI, double RasDPI, const char *obj_id, Joe *DrawMe,
			unsigned long numpts, unsigned long weight, unsigned char R, unsigned char G, unsigned char B, double EarthLatScaleMeters,
			double PlanetRad, double CenterPixelSize, long TileWidth, long TileHeight, long TotalWidth, long TotalHeight, rPixelHeader *rPixelFragMap);
		virtual int EndVectorFrame(void);
		long IllustratorEnd(void);
	}; // ImageFormatAI

class ImageFormatRLA: public ImageFormat
	{
	public:
		ImageFormatRLA() {FormatIndex = WCS_IMAGEFILEFORMAT_RLA;};
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
	}; // ImageFormatRLA

#ifdef WCS_BUILD_AVI_SUPPORT
class ImageFormatAVI: public ImageFormat
	{
	private:
		PAVIFILE pfile;
		AVISTREAMINFO      avisi;
		PAVISTREAM         pavi, pcomp;
		AVICOMPRESSOPTIONS compOptions;
		COMPVARS			cv;
		BITMAPINFO *bmi;
		int BMISize, LibRef, rowbytes, FramesWritten;
	public:
		ImageFormatAVI() {FormatIndex = WCS_IMAGEFILEFORMAT_AVI;pfile = NULL;pavi=pcomp=NULL;FramesWritten = BMISize = rowbytes = LibRef = 0;bmi = NULL;};
		virtual int StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime);
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		virtual int EndAnim(void);
	}; // ImageFormatAVI
#endif // WCS_BUILD_AVI_SUPPORT

#ifdef WCS_BUILD_QUICKTIME_SUPPORT
class ImageFormatQT: public ImageFormat
	{
	private:
		double LocalFrameRate;
		int FramesWritten, LocalFrameStep, FirstFrame;
		Movie theMovie;
		FSSpec mySpec;
		short resRefNum;
		Track theTrack;
		Media theMedia;
		Rect trackFrame;
		ComponentInstance ci;
		ImageDescriptionHandle imageDesc;
		GWorldPtr theGWorld;

	public:
		ImageFormatQT() {FormatIndex = WCS_IMAGEFILEFORMAT_QT; theMovie = nil; LocalFrameRate = 1.0; FramesWritten = 0; resRefNum = 0; FirstFrame = -1; theGWorld = nil;};
		virtual int StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime);
		virtual int StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame);
		virtual int EndAnim(void);
	}; // ImageFormatQT
#endif // WCS_BUILD_QUICKTIME_SUPPORT

#endif // WCS_IMAGE_FORMAT_H
