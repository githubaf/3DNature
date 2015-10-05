// PostProcessEvent.h
// Header file for PostProcessEvent.cpp
// Built from scratch on 2/19/02 by Gary R. Huber
// Copyright 2002 3D Nature LLC. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_POSTPROCEVENT_H
#define WCS_POSTPROCEVENT_H

class RenderOpt;
class BufferNode;
class EffectsLib;
class RenderData;
class EffectList;
class BufferNode;
class rPixelBlockHeader;
class RenderData;
class BusyWin;
class RenderInterface;
class rPixelFragment;
class PolygonData;
class RLASampleData;
class FontImage;
class InternalRasterFont;

#include "GraphData.h"
#include "RasterAnimHost.h"
#include "Texture.h"
#include "Render.h"
#include "FontImage.h"

#define WCS_POSTPROC_MAXNAMELENGTH	60
#define WCS_SUBCLASS_POSTPROCEVENT	145

enum
	{
	WCS_POSTPROCEVENT_TYPE_GAIN,
	WCS_POSTPROCEVENT_TYPE_GAMMA,
	WCS_POSTPROCEVENT_TYPE_LEVELS,
	WCS_POSTPROCEVENT_TYPE_LIGHTEN,
	WCS_POSTPROCEVENT_TYPE_DARKEN,
	WCS_POSTPROCEVENT_TYPE_CONTRAST,
	WCS_POSTPROCEVENT_TYPE_EXPOSURE,
	WCS_POSTPROCEVENT_TYPE_TEXTURE,
	WCS_POSTPROCEVENT_TYPE_TEXT,
	WCS_POSTPROCEVENT_TYPE_MEDIAN,
	WCS_POSTPROCEVENT_TYPE_CHROMAX,
	WCS_POSTPROCEVENT_TYPE_BOXFILTER,
	WCS_POSTPROCEVENT_TYPE_DEPTHOFFIELD,
	WCS_POSTPROCEVENT_TYPE_DISTORT,
	WCS_POSTPROCEVENT_TYPE_EDGEINK,
	WCS_POSTPROCEVENT_TYPE_POSTERIZE,
	WCS_POSTPROCEVENT_TYPE_GLOW,
	WCS_POSTPROCEVENT_TYPE_LINE,
	WCS_POSTPROCEVENT_TYPE_NEGATIVE,
	WCS_POSTPROCEVENT_TYPE_STAR,
	WCS_POSTPROCEVENT_TYPE_HALO,
	WCS_POSTPROCEVENT_TYPE_IMAGE,
	WCS_POSTPROCEVENT_TYPE_COMPOSITE,
	WCS_POSTPROCEVENT_TYPE_HEADING,
	// ADD_NEW_POSTPROC_EVENT	add a define for the new event type. The order must be the same as the name list in
	WCS_POSTPROCEVENT_NUMTYPES		// PostProcessEvent.cpp but other than that restriction, the order is unimportant.
	}; // post process event types

enum
	{
	WCS_POSTPROCEVENT_ANIMPAR_INTENSITY,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE1,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE2,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE3,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE4,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE5,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE6,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE7,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE8,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE9,
	WCS_POSTPROCEVENT_ANIMPAR_VALUE10,
	WCS_POSTPROCEVENT_NUMANIMPAR
	}; // animated PostProcessEvent params

enum
	{
	WCS_POSTPROCEVENT_TEXTURE_INTENSITY,
	WCS_POSTPROCEVENT_TEXTURE_FILTER,
	WCS_POSTPROCEVENT_TEXTURE_VALUE1,
	WCS_POSTPROCEVENT_TEXTURE_VALUE2,
	WCS_POSTPROCEVENT_TEXTURE_VALUE3,
	WCS_POSTPROCEVENT_NUMTEXTURES
	}; // PostProcessEvent textures

enum
	{
	WCS_POSTPROCEVENT_COORDTYPE_IMAGE,
	WCS_POSTPROCEVENT_COORDTYPE_PROCEDURAL
	}; // PostProcessEvent coordinate types

enum
	{
	WCS_POSTPROCEVENT_RGBORHSV_RGB,
	WCS_POSTPROCEVENT_RGBORHSV_HSV
	}; // PostProcessEvent RGBorHSV types

class PostProcessEvent : public RasterAnimHost, public RootTextureParent
	{
	public:
		AnimDoubleTime AnimPar[WCS_POSTPROCEVENT_NUMANIMPAR];
		AnimColorTime Color;
		char Enabled, AffectRed, AffectGrn, AffectBlu, RGBorHSV;
		char Name[WCS_POSTPROC_MAXNAMELENGTH];
		RootTexture *TexRoot[WCS_POSTPROCEVENT_NUMTEXTURES];
		PostProcessEvent *Next;
		static char *PostProcEventNames[WCS_POSTPROCEVENT_NUMTYPES];
		unsigned long PassNum;

		// these are variables that were local to PostProcessEvent::RenderPostProc
		// and have been moved to object scope for easy acces by virtual methods in derived objects
		unsigned char *Bitmap[3], *AABuf, *ObjTypeBuf;
		int Success;
		long Row, Col, PixZip;
		double TexOpacity, TexColor[3], Value[3], OrigValue[3], Intensity, InvIntensity;
		float *ZBuf, *LatBuf, *LonBuf, *ElevBuf, *RelElBuf, *ReflectionBuf, *IllumBuf, *SlopeBuf, *AspectBuf, *NormalBuf[3];
		unsigned short *ExponentBuf;
		RasterAnimHost **ObjectBuf;
		rPixelFragment *CurFrag;
		TextureData TexData;
		VertexDEM VertDEM;

		PostProcessEvent(RasterAnimHost *RAHost);
		virtual ~PostProcessEvent();
		static PostProcessEvent *NewEvent(RasterAnimHost *RAHost, unsigned char EventType);
		static unsigned char GetEventTypeFromName(char *EventName);
		void Copy(PostProcessEvent *CopyTo, PostProcessEvent *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		long GetNumAnimParams(void) {return (WCS_POSTPROCEVENT_NUMANIMPAR);};
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_POSTPROCEVENT_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		char *GetName(void)	{return (Name);};
		void SetName(char *NewName);
		int SetToTime(double Time);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_POSTPROCEVENT);};
		int GetRAHostAnimated(void);
		long InitImageIDs(long &ImageID);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
			RootTexture **&TexAffil);
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		void ResolveLoadLinkages(EffectsLib *Lib);
		int InitToRender(RenderOpt *Opt, BufferNode *Buffers);
		int InitFrameToRender(EffectsLib *Lib, RenderData *Rend);
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int AdvanceRow(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);

		// this can be done without NonPoint buffers as long as we're not altering the sampled buffer on the fly
		int ZSampleRelative(signed char XOff, signed char YOff, long Width, long Height, double &ZSample);
		int NormDiffSampleRelative(signed char XOff, signed char YOff, long Width, long Height, double &NormDiff, Point3d MySample);

		// this is to support multi-pass operations
		virtual unsigned char InquirePasses(void) {return (1);}; // single-pass by default

		// This is to invoke advanced editing operations
		virtual int SpecialEdit(void) {return(0);}; // base class indicates we didn't do anything

		// this is to avoid rendering things that use special multi-line buffers before reflections
		virtual int InhibitRenderBeforeReflections(void)	{return (0);};

		int BuildFileComponentsList(EffectList **Coords);
		int SaveObject(FILE *ffile);
		int LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		int UseValue(char ValNum);
		int UseTexture(char ValNum);
		virtual unsigned char GetType(void) = 0;
		virtual unsigned char NeedCoords(void) {return (1);};
		virtual unsigned char UseValue1(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue2(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue3(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue4(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue5(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue6(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue7(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue8(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue9(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValue10(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseFilter(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseColor(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValueTexture2(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual unsigned char UseValueTexture3(char *&ADTLabel) {ADTLabel = NULL; return (0);};
		virtual int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel) {ADT = NULL; ADTLabel = NULL; return (0);};
		virtual int AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers);
		virtual unsigned long SaveSpecificData(FILE *ffile);
		virtual unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		virtual void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)	{return;};

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_POSTPROCEVENT);};
		virtual char *GetRAHostTypeString(void) {return ("(Post Process Event)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual int GetRAEnabled(void)
			{return (RAParent ? (Enabled && RAParent->GetRAEnabled()): Enabled);};
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);	// return 0 means user cancelled
		virtual char *OKRemoveRaster(void);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual void GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace);

		// inherited from RootTextureParent
		virtual RootTexture *NewRootTexture(long TexNum);
		virtual char *GetTextureName(long TexNumber);
		virtual long GetNumTextures(void) {return (WCS_POSTPROCEVENT_NUMTEXTURES);};
		virtual RootTexture *GetTexRootPtr(long TexNum) {return (TexNum < WCS_POSTPROCEVENT_NUMTEXTURES ? TexRoot[TexNum]: NULL);};
		virtual RootTexture *GetEnabledTexture(long TexNum) {return (TexNum < WCS_POSTPROCEVENT_NUMTEXTURES && TexRoot[TexNum] && TexRoot[TexNum]->Enabled ? TexRoot[TexNum]: NULL);};
		virtual RootTexture **GetTexRootPtrAddr(long TexNum) {return (TexNum < WCS_POSTPROCEVENT_NUMTEXTURES ? &TexRoot[TexNum]: NULL);};
		virtual void SetTexRootPtr(long TexNum, RootTexture *NewRoot) {if (TexNum < GetNumTextures()) TexRoot[TexNum] = NewRoot;};

	}; // class PostProcessEvent

class PostProcessEventList
	{
	public:
		PostProcessEvent *Me;
		PostProcessEventList *Next;

		PostProcessEventList() {Me = NULL; Next = NULL;};
	}; // class PostProcessEventList

class PostProcGain : public PostProcessEvent
	{
	public:
		PostProcGain(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_GAIN);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcGain

class PostProcGamma : public PostProcessEvent
	{
	public:
		PostProcGamma(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_GAMMA);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcGamma

class PostProcLevels : public PostProcessEvent
	{
	public:
		PostProcLevels(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_LEVELS);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcLevels

class PostProcLighten : public PostProcessEvent
	{
	public:
		PostProcLighten(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_LIGHTEN);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcLighten

class PostProcDarken : public PostProcessEvent
	{
	public:
		PostProcDarken(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_DARKEN);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcDarken

class PostProcContrast : public PostProcessEvent
	{
	public:
		PostProcContrast(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_CONTRAST);};
		unsigned char NeedCoords(void) {return (0);};
		int FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel);
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Filter  "; return (1);};
	}; // class PostProcContrast

#define	WCS_TEXTEXPOSURE_ANIMPAR_EXPOSURE			WCS_POSTPROCEVENT_ANIMPAR_VALUE1

class PostProcExposure : public PostProcessEvent
	{
	public:
		double Exposure;

		PostProcExposure(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_EXPOSURE);};
		unsigned char NeedCoords(void) {return (0);};
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "(f-stops +/-) "; return (1);};
	}; // class PostProcExposure

class PostProcTexture : public PostProcessEvent
	{
	public:
		PostProcTexture(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_TEXTURE);};
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Texture  "; return (1);};
	}; // class PostProcTexture

#define	WCS_TEXTOVERLAY_ANIMPAR_CENTERX			WCS_POSTPROCEVENT_ANIMPAR_VALUE1
#define	WCS_TEXTOVERLAY_ANIMPAR_CENTERY			WCS_POSTPROCEVENT_ANIMPAR_VALUE2
#define	WCS_TEXTOVERLAY_ANIMPAR_WIDTH			WCS_POSTPROCEVENT_ANIMPAR_VALUE3
#define	WCS_TEXTOVERLAY_ANIMPAR_HEIGHT			WCS_POSTPROCEVENT_ANIMPAR_VALUE4
#define	WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE		WCS_POSTPROCEVENT_ANIMPAR_VALUE5
#define	WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT		WCS_POSTPROCEVENT_ANIMPAR_VALUE6
#define	WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY	WCS_POSTPROCEVENT_ANIMPAR_VALUE7
#define	WCS_TEXTOVERLAY_ANIMPAR_KERNING			WCS_POSTPROCEVENT_ANIMPAR_VALUE8
#define	WCS_TEXTOVERLAY_ANIMPAR_LEADING			WCS_POSTPROCEVENT_ANIMPAR_VALUE9
#define	WCS_TEXTOVERLAY_ANIMPAR_OUTLINE			WCS_POSTPROCEVENT_ANIMPAR_VALUE10

// Same idea as above, but different slots when used for Image
#define	WCS_IMAGEOVERLAY_ANIMPAR_CENTERX			WCS_POSTPROCEVENT_ANIMPAR_VALUE2
#define	WCS_IMAGEOVERLAY_ANIMPAR_CENTERY			WCS_POSTPROCEVENT_ANIMPAR_VALUE3
#define	WCS_IMAGEOVERLAY_ANIMPAR_WIDTH				WCS_POSTPROCEVENT_ANIMPAR_VALUE4
#define	WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT				WCS_POSTPROCEVENT_ANIMPAR_VALUE5
#define	WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE			WCS_POSTPROCEVENT_ANIMPAR_VALUE6
#define	WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT			WCS_POSTPROCEVENT_ANIMPAR_VALUE7
#define	WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY	WCS_POSTPROCEVENT_ANIMPAR_VALUE8

// These are used for TEXT and IMAGE, except for MESGTEXT, which is not used in IMAGE
#define WCS_POSTPROCTEXT_MESGTEXT			0x00210000
#define WCS_POSTPROCTEXT_ILLUMINATE			0x00220000
#define WCS_POSTPROCTEXT_APPLYVOLUMETRICS	0x00230000
#define WCS_POSTPROCTEXT_RECEIVESHADOWS		0x00240000

#define WCS_TEXTOVERLAY_MAXLEN	2048

class PostProcImage : public PostProcessEvent
	{
	public:
		char Illuminate, ApplyVolumetrics, ReceiveShadows;

		PostProcImage(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_IMAGE);};
		virtual int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		int PlotImage(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragMap, PolygonData *Poly, Raster *SourceRast, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
			int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
			double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha,
			unsigned char *EdgeAlpha);
		/*** F2 MOD
		int PlotImage(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragMap, PolygonData *Poly, Raster *SourceRast, 
			float Width, float Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
			int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
			double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha,
			unsigned char *EdgeAlpha);
		***/
		int SetRaster(Raster *NewRast);
		unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = "Image  "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "X Center (%) "; return (1);};
		unsigned char UseValue3(char *&ADTLabel) {ADTLabel = "Y Center (%) "; return (1);};
		unsigned char UseValue4(char *&ADTLabel) {ADTLabel = "Width (%) "; return (1);};
		unsigned char UseValue5(char *&ADTLabel) {ADTLabel = "Height (%) "; return (1);};
		unsigned char UseValue6(char *&ADTLabel) {ADTLabel = "Z Distance (m) "; return (1);};
		unsigned char UseValue7(char *&ADTLabel) {ADTLabel = "Back Light (%) "; return (1);};
		unsigned char UseValue8(char *&ADTLabel) {ADTLabel = "Received Shadow Intensity (%) "; return (1);};
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);
		// for troubleshooting
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Image Setup  "; return (1);};
	}; // class PostProcImage

class PostProcText : public PostProcessEvent
	{
	public:
		char Illuminate, ApplyVolumetrics, ReceiveShadows;
		char MesgText[WCS_TEXTOVERLAY_MAXLEN];

		PostProcText(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_TEXT);};
		virtual int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		void FetchPixelColor(long xpos, long ypos, unsigned char &RealR, unsigned char &RealG, unsigned char &RealB);
		int PlotText(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragMap, PolygonData *Poly, Raster *SourceRast, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
			int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
			double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha,
			unsigned char *EdgeAlpha);
		unsigned char UseColor(char *&ADTLabel) {ADTLabel = "Text Color  "; return (1);};
		unsigned char UseFilter(char *&ADTLabel) {ADTLabel = "Text Color  "; return (1);};
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "X Center (%) "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "Y Center (%) "; return (1);};
		unsigned char UseValue3(char *&ADTLabel) {ADTLabel = "Width (%) "; return (1);};
		unsigned char UseValue4(char *&ADTLabel) {ADTLabel = "Height (%) "; return (1);};
		unsigned char UseValue5(char *&ADTLabel) {ADTLabel = "Z Distance (m) "; return (1);};
		unsigned char UseValue6(char *&ADTLabel) {ADTLabel = "Back Light (%) "; return (1);};
		unsigned char UseValue7(char *&ADTLabel) {ADTLabel = "Received Shadow Intensity (%) "; return (1);};
		unsigned char UseValue8(char *&ADTLabel) {ADTLabel = "Kerning (%) "; return (1);};
		unsigned char UseValue9(char *&ADTLabel) {ADTLabel = "Leading (%) "; return (1);};
		unsigned char UseValue10(char *&ADTLabel) {ADTLabel = "Outline "; return (1);};
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);
	}; // class PostProcText

class PostProcChromax : public PostProcessEvent
	{
	public:
		PostProcChromax(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_CHROMAX);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
	}; // class PostProcChromax

class PostProcPosterize : public PostProcessEvent
	{
	public:
		double Levels, LevelsMinusOne, LevelWidth, InvLevelsMinusOne;

		PostProcPosterize(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_POSTERIZE);};
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "Levels "; return (1);};
	}; // class PostProcPosterize

// These are used for PostProcHeading
#define WCS_POSTPROCHEADING_LETTERS		0x00210000
#define WCS_POSTPROCHEADING_NUMBERS		0x00220000
#define WCS_POSTPROCHEADING_SCALE1		0x00230000
#define WCS_POSTPROCHEADING_SCALE5		0x00240000
#define WCS_POSTPROCHEADING_TICKS			0x00250000
#define WCS_POSTPROCHEADING_SELLETTERS	0x00260000
#define WCS_POSTPROCHEADING_SELNUMBERS	0x00270000
#define WCS_POSTPROCHEADING_SELNUMSTYLE	0x00280000
#define WCS_POSTPROCHEADING_SELTICKS		0x00290000

class PostProcHeading : public PostProcessEvent
	{
	public:
		double heading_delta, pixel_left, pixel_right;
		UBYTE *rbuf, *gbuf, *bbuf, *abuf;
		UBYTE *lines[3];
		float r, g, b;
		int rwidth, rheight, rsize;
		FontImage fi;
		InternalRasterFont irf;
		unsigned long zippy;
		long sel_letters, sel_numbers, sel_numstyle, sel_ticks;	// the GUI dropboxes - save with component
		char headstring[4];	// 2 or 3 digit degrees, or 3 letters (ie: NNE)
		char letters, numbers, scale1, scale5, ticks;	// the GUI checkboxes - save with component

		PostProcHeading(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_HEADING);};
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		void ComputeTick(const double angle, unsigned char intensity, bool& plot_l, bool& plot_r);
		int Doable(RenderData *Rend);
		int PlotCardinals(RenderData *Rend, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics);
		int PlotDegrees(RenderData *Rend, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics);
		int PlotScales(RenderData *Rend, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics);
		int PlotTicks(RenderData *Rend, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics);
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);
	}; // class PostProcHeading

#define WCS_POSTPROCEVENT_NONPOINT_MAXLINES	30

// Base class for post processes that are not just point-sampled
// do not try to instantiate without deriving subclasses from
// only makes sense for odd numbers of lines
class PostProcNonPoint : public PostProcessEvent
	{
	public:
		// maximum of an n*WCS_POSTPROCEVENT_NONPOINT_MAXLINES filter kernal to ease programming
		int NumLines, MidLine, NextLine;
		double *RedLines[WCS_POSTPROCEVENT_NONPOINT_MAXLINES], *GreenLines[WCS_POSTPROCEVENT_NONPOINT_MAXLINES], *BlueLines[WCS_POSTPROCEVENT_NONPOINT_MAXLINES];

		PostProcNonPoint(RasterAnimHost *RAHost, int LinesReq = 3); // defaults to a n*3 buffer setup

		// the following manage the buffered line sets.
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int AdvanceRow(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int InhibitRenderBeforeReflections(void)	{return (1);};

		// must NOT be called between PrepForPostProf and CleanupPostProc
		void SetLinesReq(int NewLinesReq);

		void TransferLineDouble(int DestLineNum, int SourceLineNum, long Width);

		int RGBSampleRelative(signed char XOff, signed char YOff, long Width, long Height,
		 double &RedSample, double &GrnSample, double &BluSample);
		int RGBSampleRelativeFrac(float XOff, float YOff, long Width, long Height,
		 double &RedSample, double &GrnSample, double &BluSample);

	}; // class PostProcNonPoint

// Base class for post processes that need a full-screen temporary output buffer for their effect
class PostProcFullBuf : public PostProcessEvent
	{
	private:
		inline int RangeCheckRelativeZip(signed char XOff, signed char YOff, long Width, long Height, unsigned long &Zip);
	public:
		float *PPFloatBuf;
		unsigned char *PPByteBuf;
		char UseFloat;

		PostProcFullBuf(RasterAnimHost *RAHost, char UseFloatBuffer); // 0=use byte, 1=use float

		// the following manage the buffer
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics);
		virtual int CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics);

		int ReadByteBufRelative(signed char XOff, signed char YOff, long Width, long Height, unsigned char &ByteSample);
		int WriteByteBufRelative(signed char XOff, signed char YOff, long Width, long Height, unsigned char ByteSample);
		int ReadFloatBufRelative(signed char XOff, signed char YOff, long Width, long Height, float &FloatSample);
		int WriteFloatBufRelative(signed char XOff, signed char YOff, long Width, long Height, float FloatSample);
	}; // class PostProcFullBuf

#define WCS_POSTPROCEVENT_MEDIAN_MAXDIM	5

struct MedianSample
	{
	double MRed, MGrn, MBlu, MSum;
	}; // MedianSample

#define WCS_POSTPROCMEDIAN_DOFIVE		0x00210000

class PostProcMedian : public PostProcNonPoint
	{
	public:
		int DoFive;

		PostProcMedian(RasterAnimHost *RAHost);
		void SetFiveByFive(int NewFive) {DoFive = NewFive;};
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_MEDIAN);};
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);
	}; // class PostProcMedian

class PostProcBoxFilter : public PostProcNonPoint
	{
	public:
		PostProcBoxFilter(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_BOXFILTER);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
	}; // class PostProcBoxFilter

class PostProcDOFFilter : public PostProcNonPoint
	{
	public:
		PostProcDOFFilter(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_DEPTHOFFIELD);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "Focal Depth "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "Max Radius (pix)"; return (1);};
	}; // class PostProcDOFFilter

class PostProcDistort : public PostProcNonPoint
	{
	public:
		PostProcDistort(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_DISTORT);};
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "X Distort (pix) "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "Y Distort (pix) "; return (1);};
		unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = "X Distort (pix)  "; return (1);};
		unsigned char UseValueTexture2(char *&ADTLabel) {ADTLabel = "Y Distort (pix)  "; return (1);};
	}; // class PostProcDistort

// These are used for PostProcEdgeInk
#define WCS_POSTPROCEDGEINK_DISTDIFF		0x00210000
#define WCS_POSTPROCEDGEINK_NORMDIFF		0x00220000
#define WCS_POSTPROCEDGEINK_INKWEIGHT		0x00230000
#define WCS_POSTPROCEDGEINK_INKCOLOR		0x00240000
#define WCS_POSTPROCEDGEINK_DISTANCE		0x00250000

// this might work out as a Z-driven gradient of some sort
struct EdgeInkParam
	{
	double Distance;
	double DistDiff, NormDiff, InkWeight, InkColor;
	}; // EdgeInkParam

class PostProcEdgeInk : public PostProcessEvent
	{
	public:
		EdgeInkParam EdgeInkParams[3]; // Near, Mid, Far

		PostProcEdgeInk(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_EDGEINK);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		void LookupInkParams(double CellDist, double &DistDiff, double &NormDiff, double &InkWeight, double &InkColor);
		virtual int AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers);
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);

	}; // class PostProcEdgeInk

// PostProcGlow is derived from PostProcFullBuf so it can be driven
// by any input channel. The Glow is not applied until it has been calculated for
// the entire image.

#define WCS_POSTPROCGLOW_DOHALO		0x00210000

class PostProcGlow : public PostProcFullBuf
	{
	private:
		int DoHalo;
	public:
		PostProcGlow(RasterAnimHost *RAHost, int AsHalo = 0);
		unsigned char GetType(void)	{return (DoHalo ? WCS_POSTPROCEVENT_TYPE_HALO : WCS_POSTPROCEVENT_TYPE_GLOW);};
		unsigned char InquirePasses(void) {return (2);}; // two-pass operation
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "Amount (%)  "; return (1);};
		unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = "Amount (%)  "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "Radius (pix) "; return (1);};
		unsigned char UseValueTexture2(const char *&ADTLabel) {ADTLabel = DoHalo ? "Halo Radius  ": "Glow Radius  "; return (1);};
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);

	}; // class PostProcGlow

#define WCS_POSTPROCSTAR_POWERCURVE		0x00210000

class PostProcStar : public PostProcFullBuf
	{
	public:
		AnimDoubleDistance PowerCurve;

		PostProcStar(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_STAR);};
		unsigned char InquirePasses(void) {return (2);}; // two-pass operation
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "Amount (%)  "; return (1);};
		unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = "Amount (%)  "; return (1);};
		unsigned char UseValue2(char *&ADTLabel) {ADTLabel = "Radius (pix) "; return (1);};
		unsigned char UseValue3(char *&ADTLabel) {ADTLabel = "Points "; return (1);};
		unsigned char UseValue4(char *&ADTLabel) {ADTLabel = "Squeeze (%) "; return (1);};
		unsigned char UseValue5(char *&ADTLabel) {ADTLabel = "Sharpness (%) "; return (1);};
		unsigned char UseValue6(char *&ADTLabel) {ADTLabel = "Rotation "; return (1);};
		unsigned char UseValue7(char *&ADTLabel) {ADTLabel = "AntiAlias "; return (1);};
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);

		int SpecialEdit(void);

	}; // class PostProcStar

class PostProcLine : public PostProcNonPoint
	{
	public:
		PostProcLine(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_LINE);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		unsigned char UseValue1(char *&ADTLabel) {ADTLabel = "Saturation (%)  "; return (1);};
		unsigned char UseValueTexture1(char *&ADTLabel) {ADTLabel = "Saturation (%)  "; return (1);};

	}; // class PostProcLine

class PostProcNegative : public PostProcessEvent
	{
	public:
		PostProcNegative(RasterAnimHost *RAHost);
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_NEGATIVE);};
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);

	}; // class PostProcNegative

// These are used for PostProcComposite
#define WCS_POSTPROCCOMPOSITE_IMAGEID		0x00210000

class PostProcComposite : public PostProcessEvent
	{
	public:
		RasterShell *Img;
		// for use during evaluation so it doesn't have to be passed during callback
		RenderData *MyRend;
		BufferNode *MyBuffers;
		rPixelBlockHeader *MyFragBlock;
		long MyWidth;
		long MyHeight;
		long MyNumPanels, MyNumSegments, MyPanel, MySegment;
		BusyWin *MyBWDE;
		RenderInterface *MyMaster;
		rPixelHeader *MyFragMap;
		unsigned char **MyOptionalBitmaps;
		long MyFrameNum;
		int MyUpdateDiagnostics;

		PostProcComposite(RasterAnimHost *RAHost);
		~PostProcComposite();
		unsigned char GetType(void)	{return (WCS_POSTPROCEVENT_TYPE_COMPOSITE);};
		int SetRaster(Raster *NewRast);
		virtual char *OKRemoveRaster(void);
		virtual void RemoveRaster(RasterShell *Shell);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual int GetDeletable(RasterAnimHost *TestMe);
		virtual int RemoveRAHost(RasterAnimHost *RemoveMe);
		virtual int RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		virtual int EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
			long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
			long FrameNum, int UpdateDiagnostics);
		int EvalOneRLASample(RLASampleData *Samp);
		void TranslateRPFMaskToPixelFrags(unsigned short InputMask, unsigned long &TopCovg, unsigned long &BotCovg);
		unsigned long SaveSpecificData(FILE *ffile);
		unsigned long LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip);
		void CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP);

	}; // class PostProcComposite


inline int PostProcFullBuf::RangeCheckRelativeZip(signed char XOff, signed char YOff, long Width, long Height, unsigned long &Zip)
{
long XLoc, YLoc;

XLoc = Col + XOff;
YLoc = Row + YOff;
if(XLoc >= 0 && XLoc < Width && YLoc >= 0 && YLoc < Height)
	{
	Zip = XLoc + (YLoc * Width);
	return(1);
	} // if
else
	{
	return(0);
	} // else
} // PostProcFullBuf::RangeCheckRelativeZip

// ADD_NEW_POSTPROC_EVENT	add the class definition, derived from PostProcessEvent

#define WCS_POSTPROCEVENT_NAME			0x00210000
#define WCS_POSTPROCEVENT_ENABLED		0x00220000
#define WCS_POSTPROCEVENT_AFFECTRED		0x00230000
#define WCS_POSTPROCEVENT_AFFECTGRN		0x00240000
#define WCS_POSTPROCEVENT_AFFECTBLU		0x00250000
#define WCS_POSTPROCEVENT_TEXCOORDTYPE	0x00260000
#define WCS_POSTPROCEVENT_INTENSITY		0x00270000
#define WCS_POSTPROCEVENT_VALUE1		0x00280000
#define WCS_POSTPROCEVENT_TEXINTENSITY	0x00290000
#define WCS_POSTPROCEVENT_TEXFILTER		0x002a0000
#define WCS_POSTPROCEVENT_RGBORHSV		0x002b0000
#define WCS_POSTPROCEVENT_VALUE2		0x002c0000
#define WCS_POSTPROCEVENT_VALUE3		0x002d0000
#define WCS_POSTPROCEVENT_TEXVALUE1		0x002e0000
#define WCS_POSTPROCEVENT_TEXVALUE2		0x002f0000
#define WCS_POSTPROCEVENT_TEXVALUE3		0x00310000
#define WCS_POSTPROCEVENT_VALUE4		0x00320000
#define WCS_POSTPROCEVENT_VALUE5		0x00330000
#define WCS_POSTPROCEVENT_VALUE6		0x00340000
#define WCS_POSTPROCEVENT_VALUE7		0x00350000
#define WCS_POSTPROCEVENT_SPECIFICDATA	0x00360000
#define WCS_POSTPROCEVENT_COLOR			0x00370000
#define WCS_POSTPROCEVENT_VALUE8		0x00380000
#define WCS_POSTPROCEVENT_VALUE9		0x00390000
#define WCS_POSTPROCEVENT_VALUE10		0x003a0000

#endif // WCS_POSTPROCEVENT_H
