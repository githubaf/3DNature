// ImageFormatQT.cpp
// Built from bits of ImageFormat.cpp and Apple's CreateMovie.c and QTVideo.c
// on Sept 24th 2001

#include "stdafx.h"

#include <float.h>
#include <limits.h>
#include <time.h>
#include "ImageFormat.h"
#include "ImageFormatQT.h"
#include "Useful.h"
#include "Project.h"
#include "ImageOutputEvent.h"
#include "Application.h"
#include "requester.h"
#include "AppMem.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "RLA.h"
#include "ImageFormatIFF.h"
#include "Illustrator.h"
#include "Exports.h"
#include "DEM.h"
#include "WCSVersion.h"
#include "PixelManager.h"

int QTInited;

#if defined( _MAC ) && !defined (__MACH__)

#include <macname1.h>

#include "ImageCompression.h"
#include <script.h>
#include <ToolUtil.h>

#include <macname2.h>
#endif // _MAC

#ifdef __MACH__
#include <QuickDraw.h>
#endif // __MACH__

#ifdef _MAC
//#include "Movies.h"
#define createMovieFileDontCreateResFile (1L << 28)

	static void InitMacToolbox (void);

// Code from CoreTek CPL
OSErr FSpChangeCreatorType (const FSSpec *fp, OSType creator, OSType fileType);
static OSErr _changeCreatorType(short vRefNum, long dirID, ConstStr255Param name, OSType creator, OSType fileType);
OSErr FSpLocationFromFullPath (short flen, const void *fpp, FSSpec *fp);


#else // !_MAC
#endif // !_MAC

static void QuickTimeInit(void);
static void QuickTimeUnInit(void);
//void CheckError(OSErr error, char *msg);

/************************************************************
*                                                           *
*    CONSTANTS                                              *
*                                                           *
*************************************************************/

#define		kMgrChoose			0
#define		kMediaStart			0
#define		kMsgDialogRsrcID	129
#define		kMsgItemID			3	

/************************************************************
*                                                           *
*    InitMacToolbox()                                       *
*                                                           *
*    Initializes the various Mac Toolbox Managers           *
*                                                           *
*************************************************************/


#if defined( _MAC) && !defined (__MACH__)
static void InitMacToolbox(void)
{
	InitGraf (&qd.thePort);
	InitFonts ();
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (nil);
} // InitMacToolbox

#endif // _MAC



static void QuickTimeInit(void)
{
#ifndef _MAC // WIN32
	if(InitializeQTML(0L) != noErr) return;
#endif // !_MAC

if(EnterMovies() == noErr)
	{
	QTInited = 1;
	} // if
} // QuickTimeInit


static void QuickTimeUnInit(void)
{
if(QTInited)
	{
	ExitMovies();
	#ifndef _MAC
	TerminateQTML();
	#endif // _MAC
	QTInited = 0;
	} // if
} // QuickTimeUnInit

char QTAnimOutputPath[1024];



/************************************************************
*                                                           *
*    QTVideo_AddVideoSamplesToMedia()                       *
*                                                           *
*    Creates video samples for the media in a track         *
*                                                           *
*************************************************************/


#ifdef WCS_BUILD_QUICKTIME_SUPPORT
int ImageFormatQT::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
OSErr err = noErr;
PixMapHandle pm;
unsigned char *line, *linebase;
int rowbytes, Channels = 3;
long xscan, scanrow = 0;
unsigned char AllIsWell = 0, SampRGB[3];

if(QTInited)
	{
	// we know we'll need RGB bands so look for them specifically
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
	ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
	ShortChannelData[0] = NULL;
	if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
		{
		CharChannelNode[3] = Buffers->FindBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE);
		Channels = 4;
		} // if

	if (CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
		{
		// begin for-each-frame
		pm = GetGWorldPixMap(theGWorld);
		LockPixels (pm);
		linebase = (unsigned char *) GetPixBaseAddr (pm);
		#ifndef __MACH__
		#ifndef _MAC
		rowbytes = (**theGWorld->portPixMap).rowBytes & 0x3FFF;
		#else // _MAC
		rowbytes = (**theGWorld->portPixMap).rowBytes & 0x3FFF;
		#endif // _MAC
		#else
		rowbytes = (**(GetPortPixMap(GetQDGlobalsThePort()))).rowBytes & 0x3FFF;
		#endif
		if(theMovie && theGWorld && theTrack && theMedia && imageDesc)
			{
			for(scanrow = 0; scanrow < BufHeight; scanrow++)
				{
				CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
				if (ShortChannelNode[0])
					ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(scanrow, WCS_RASTER_BANDSET_SHORT);
				if(Channels == 4)
					{
					CharChannelData[3] = (unsigned char *)CharChannelNode[3]->GetDataLine(scanrow, WCS_RASTER_BANDSET_BYTE);
					} // if

				line = linebase + (scanrow * rowbytes);
				if(CharChannelData[0] && CharChannelData[1] && CharChannelData[2])
					{
					// Interleave data into QuickTime's (A)RGB format
					for(xscan = 0; xscan < BufWidth; xscan++)
						{
						SampRGB[0] = CharChannelData[0][xscan];
						SampRGB[1] = CharChannelData[1][xscan];
						SampRGB[2] = CharChannelData[2][xscan];
						if (ShortChannelData[0] && ShortChannelData[0][xscan])
							rPixelFragment::ExtractClippedExponentialColors(SampRGB, ShortChannelData[0][xscan]);
						
						// Mac seems to want the extra fourth (alpha?) byte, Windows doesn't...
						// We can tell by the pixelSize in the Pixmap being 24 or 32.
						#ifndef __MACH__
						if((**theGWorld->portPixMap).pixelSize == 32)
						#else
						if((**(GetPortPixMap(GetQDGlobalsThePort()))).pixelSize == 32 )
						#endif
							{
							if(Channels == 4)
								{
								*line++ = CharChannelData[3][xscan];
								} // if
							else
								{
								*line++ = SampRGB[0]; // use red as dummy alpha
								} // else
							} // if
						*line++ = SampRGB[0];
						*line++ = SampRGB[1];
						*line++ = SampRGB[2];
						} // for
					} // if
				else
					{
					break;
					} // else
				} // for
			} // if
		UnlockPixels (pm);

		if(scanrow == BufHeight)
			{
			Handle			 compressedFrame = NULL;
			long			 compressedSize;
			short			 syncFlag;
			
			err = (int)SCCompressSequenceFrame (ci, pm, NULL, &compressedFrame, &compressedSize, &syncFlag);
			err = AddMediaSample (theMedia, compressedFrame, 0, compressedSize, LocalFrameStep, 
			 (SampleDescriptionHandle) imageDesc, 1, syncFlag, NULL);

			FramesWritten++;
			if(FirstFrame == -1)
				{
				FirstFrame = Frame;
				} // if
			AllIsWell = 1;
			} // if
		if(!AllIsWell)
			{
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
			return(1);
			} // if
		// end for-each-frame

		} // if
	} // if
return(0);
#endif // !DEMO
} // ImageFormatQT::StartFrame

int ImageFormatQT::StartAnim(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, double FrameRate, double AnimTime)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
int Channels = 3;

#if defined( _MAC) && !defined (__MACH__)
	InitMacToolbox ();
#endif // MAC

if(!QTInited)
	{
	QuickTimeInit();
	} // if

LocalFrameRate = FrameRate;
LocalFrameStep = 1;
FirstFrame = -1;

if (IOE && IOE->SaveEnabledBufferQuery("ANTIALIAS"))
	{
	Channels = 4;
	} // if

if(IOE && IOE->SaveOpts)
	{
	LocalFrameStep = IOE->SaveOpts->FrameStep;
	} // if

if(QTInited)
	{
	SCSpatialSettings CurSpSet;
	SCTemporalSettings CurTemSet;
	OSErr err = noErr;

	IOE->PAF.GetFramePathAndName(QTAnimOutputPath, IOE->AutoExtension ? ImageSaverLibrary::GetDefaultExtension(IOE->FileType) : NULL, 0, 1000, 0);



	#ifdef _MAC
	FSpLocationFromFullPath(strlen(GlobalApp->MainProj->MungPath(QTAnimOutputPath)), GlobalApp->MainProj->MungPath(QTAnimOutputPath), &mySpec);
	#else // !MAC: Quicktime-Windows
	if(NativePathNameToFSSpec(GlobalApp->MainProj->MungPath(QTAnimOutputPath), &mySpec, 0) == noErr)
	#endif // !MAC: Quicktime-Windows
		{
		ci = OpenDefaultComponent(StandardCompressionType, StandardCompressionSubType);

		SCGetInfo(ci, scTemporalSettingsType, &CurTemSet);
		CurTemSet.frameRate = X2Fix (LocalFrameRate);
		CurTemSet.keyFrameRate = 1;
		CurTemSet.temporalQuality = codecLosslessQuality;
		
		SCGetInfo(ci, scSpatialSettingsType, &CurSpSet);
		CurSpSet.codecType = FOUR_CHAR_CODE('raw ');
		CurSpSet.codec = bestFidelityCodec;
		CurSpSet.depth = codecInfoDepth32;
		CurSpSet.spatialQuality = codecLosslessQuality;

		SCSetInfo (ci, scTemporalSettingsType, &CurTemSet);
		SCSetInfo (ci, scSpatialSettingsType, &CurSpSet);

		SCRequestSequenceSettings (ci);

		if(CreateMovieFile(&mySpec, FOUR_CHAR_CODE('TVOD'), -2 /* smCurrentScript */,
		 createMovieFileDeleteCurFile | createMovieFileDontCreateResFile,
		 &resRefNum, &theMovie) == noErr)
			{
			trackFrame.left   = 0;
			trackFrame.top    = 0;
			trackFrame.right  = (short)BufWidth;
			trackFrame.bottom = (short)BufHeight;

			if(NewGWorld (&theGWorld, 8 * Channels, &trackFrame, nil, nil, (GWorldFlags) 0 ) == noErr)
				{
				theTrack = NewMovieTrack(theMovie, FixRatio(trackFrame.right,1), FixRatio(trackFrame.bottom,1), kNoVolume);
				if(GetMoviesError()== noErr)
					{
					theMedia = NewTrackMedia(theTrack, VideoMediaType, (long)LocalFrameRate /* Video Time Scale */, nil, 0);
					if(GetMoviesError()== noErr)
						{
						if(BeginMediaEdits(theMedia) == noErr)
							{
							SCCompressSequenceBegin (ci, GetGWorldPixMap(theGWorld), NULL, &imageDesc);
							return(1);
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if
else
	{
	UserMessageOK("QuickTime Saver", "QuickTime unavailable. QuickTime may not be installed on this computer.");
	} // else


return(0);
#endif // !DEMO
} // ImageFormatQT::StartAnim

int ImageFormatQT::EndAnim(void)
{
int TrackBeginTime = 0;
if(QTInited)
	{
	// Prevent stupidity.
	if(FirstFrame == -1) FirstFrame = 0;

// <<<>>> Disabled until we can figure out how this is supposed to work
/*	if(!strcmp(IOE->Codec, "Start Track at Proper Time"))
		{
		TrackBeginTime = FirstFrame;
		} // if
*/
	SCCompressSequenceEnd (ci);

	if(theMedia)
		{
		short resId = movieInDataForkResID;

		EndMediaEdits(theMedia);
		if(theTrack)
			{
			InsertMediaIntoTrack(theTrack, TrackBeginTime/* track start time */, 0, /* media start time */
			 GetMediaDuration(theMedia), fixed1);
			} // if
		AddMovieResource(theMovie, resRefNum, &resId, NULL);
		} // if

	if(resRefNum)
		{
		CloseMovieFile (resRefNum);
		} // if

	if(ci)
		{
		CloseComponent(ci);
		} // if

	if(theGWorld)
		{
		DisposeGWorld(theGWorld);
		} // if
	if(theMovie)
		{
		DisposeMovie(theMovie);
		} // if
	QuickTimeUnInit();
	} // if
return(1);
} // ImageFormatQT::EndAnim

#endif // WCS_BUILD_QUICKTIME_SUPPORT
