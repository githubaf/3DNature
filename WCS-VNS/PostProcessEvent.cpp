// PostProcessEvent.cpp
// Code module for PostProcessEvent
// Built from scratch on 2/19/02 by Gary R. Huber
// Copyright 2002 3D Nature LLC. All rights reserved.

//#define WCS_POSTPROCESSEVENT_CREATE_ZLIBFONT

#include "stdafx.h"
#include "AppMem.h"
#include "Application.h"
#include "PostProcessEvent.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "Requester.h"
#include "Toolbar.h"
#include "Database.h"
#include "Raster.h"
#include "Render.h"
#include "Project.h"
#include "PixelManager.h"
#include "RenderControlGUI.h"
#include "FontImage.h"
#include "RLA.h"
//#include "Interactive.h"
#ifdef WCS_POSTPROCESSEVENT_CREATE_ZLIBFONT
#include "zlib.h"
#endif // WCS_POSTPROCESSEVENT_CREATE_ZLIBFONT
#include "Lists.h"
#include "Log.h"
#include "RasterResampler.h"

using namespace std;

// static methods and variables
char *PostProcessEvent::PostProcEventNames[WCS_POSTPROCEVENT_NUMTYPES] = {"Gain", "Gamma", "Levels", "Lighten", "Darken",
	"Contrast", "Exposure", "Texture Overlay", "Text Overlay", "Median Filter", "Chromax", "Box Filter", "Depth of Field",
	"Distort", "Edge Ink", "Posterize", "Glow", "Line", "Negative", "Star", "Halo", "Image", "Composite", "Heading"};
// ADD_NEW_POSTPROC_EVENT	add the name that will appear in the interface, be the default name for the event and 
//							the name saved in the project file - do not change the name once it is in general circulation
//							or events of that type in existing project files will no longer be recognized.
//							Place the event in the same order as the enum list in the header. New event types can be
//							inserted in the middle of the list as long as the enum list matches.

unsigned char PostProcessEvent::GetEventTypeFromName(char *EventName)
{
unsigned char Ct;

for (Ct = 0; Ct < WCS_POSTPROCEVENT_NUMTYPES; ++Ct)
	{
	if (! stricmp(EventName, PostProcEventNames[Ct]))
		return (Ct);
	} // for

return (0);

} // PostProcessEvent::GetEventTypeFromName

/*===========================================================================*/

PostProcessEvent *PostProcessEvent::NewEvent(RasterAnimHost *RAHost, unsigned char EventType)
{

switch (EventType)
	{
	case WCS_POSTPROCEVENT_TYPE_GAIN:
		{
		return (new PostProcGain(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_GAIN
	case WCS_POSTPROCEVENT_TYPE_GAMMA:
		{
		return (new PostProcGamma(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_GAMMA
	case WCS_POSTPROCEVENT_TYPE_LEVELS:
		{
		return (new PostProcLevels(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_LEVELS
	case WCS_POSTPROCEVENT_TYPE_LIGHTEN:
		{
		return (new PostProcLighten(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_LIGHTEN
	case WCS_POSTPROCEVENT_TYPE_DARKEN:
		{
		return (new PostProcDarken(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_DARKEN
	case WCS_POSTPROCEVENT_TYPE_CONTRAST:
		{
		return (new PostProcContrast(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_CONTRAST
	case WCS_POSTPROCEVENT_TYPE_EXPOSURE:
		{
		return (new PostProcExposure(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_EXPOSURE
	case WCS_POSTPROCEVENT_TYPE_TEXTURE:
		{
		return (new PostProcTexture(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_TEXTURE
	case WCS_POSTPROCEVENT_TYPE_TEXT:
		{
		return (new PostProcText(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_TEXT
	case WCS_POSTPROCEVENT_TYPE_MEDIAN:
		{
		return (new PostProcMedian(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_MEDIAN
	case WCS_POSTPROCEVENT_TYPE_CHROMAX:
		{
		return (new PostProcChromax(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_CHROMAX
	case WCS_POSTPROCEVENT_TYPE_BOXFILTER:
		{
		return (new PostProcBoxFilter(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_BOXFILTER
	case WCS_POSTPROCEVENT_TYPE_DEPTHOFFIELD:
		{
		return (new PostProcDOFFilter(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_DEPTHOFFIELD
	case WCS_POSTPROCEVENT_TYPE_DISTORT:
		{
		return (new PostProcDistort(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_DISTORT
	case WCS_POSTPROCEVENT_TYPE_EDGEINK:
		{
		return (new PostProcEdgeInk(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_EDGEINK
	case WCS_POSTPROCEVENT_TYPE_POSTERIZE:
		{
		return (new PostProcPosterize(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_POSTERIZE
	case WCS_POSTPROCEVENT_TYPE_GLOW:
		{
		return (new PostProcGlow(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_GLOW
	case WCS_POSTPROCEVENT_TYPE_HALO:
		{ // Halo is really just a variation of Glow with a different power function
		return (new PostProcGlow(RAHost, 1));
		} // WCS_POSTPROCEVENT_TYPE_HALO
	case WCS_POSTPROCEVENT_TYPE_LINE:
		{
		return (new PostProcLine(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_LINE
	case WCS_POSTPROCEVENT_TYPE_NEGATIVE:
		{
		return (new PostProcNegative(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_NEGATIVE
	case WCS_POSTPROCEVENT_TYPE_STAR:
		{
		return (new PostProcStar(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_STAR
	case WCS_POSTPROCEVENT_TYPE_IMAGE:
		{
		return (new PostProcImage(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_IMAGE
	case WCS_POSTPROCEVENT_TYPE_COMPOSITE:
		{
		return (new PostProcComposite(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_COMPOSITE
	case WCS_POSTPROCEVENT_TYPE_HEADING:
		{
		return (new PostProcHeading(RAHost));
		} // WCS_POSTPROCEVENT_TYPE_IMAGE
// ADD_NEW_POSTPROC_EVENT	add a case to create a new instance of the class
	} // switch

return (NULL);

} // PostProcessEvent::NewEvent

/*===========================================================================*/
/*===========================================================================*/

PostProcessEvent::PostProcessEvent(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
double EffectDefault[WCS_POSTPROCEVENT_NUMANIMPAR] = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0, 1.0};
double RangeDefaults[WCS_POSTPROCEVENT_NUMANIMPAR][3] = {1.0, 0.0, .01, 1.0, 0.0, .01, 1.0, 0.0, .01, 1.0, 0.0, .01,
														1.0, 0.0, .01, 1.0, 0.0, .01, 1.0, 0.0, .01, 1.0, 0.0, .01, 
														1.0, 0.0, .01, 1.0, 0.0, .01, 1.0, 0.0, .01};
long Ct;

Enabled = 1;
AffectRed = AffectGrn = AffectBlu = 1;
Name[0] = 0;
//TexCoordType = WCS_POSTPROCEVENT_COORDTYPE_IMAGE;
RGBorHSV = WCS_POSTPROCEVENT_RGBORHSV_RGB;

Next = NULL;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
Color.SetDefaults(this, (char)Ct);
Color.SetValue3(1.0, 1.0, 1.0);
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	TexRoot[Ct] = NULL;
	} // for

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].SetMultiplier(100.0);

} // PostProcessEvent::PostProcessEvent

/*===========================================================================*/

PostProcessEvent::~PostProcessEvent()
{
RootTexture *DelTex;
long Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for
/*
switch (EventType)
	{
	case WCS_POSTPROCEVENT_TYPE_COMPOSITE:
		{
		((PostProcComposite *)this)->DeleteSpecificResources();
		break;
		} // WCS_POSTPROCEVENT_TYPE_COMPOSITE
	} // switch
*/
} // PostProcessEvent::~PostProcessEvent

/*===========================================================================*/

void PostProcessEvent::Copy(PostProcessEvent *CopyTo, PostProcessEvent *CopyFrom)
{
long Ct;

CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->AffectRed = CopyFrom->AffectRed;
CopyTo->AffectGrn = CopyFrom->AffectGrn;
CopyTo->AffectBlu = CopyFrom->AffectBlu;
//CopyTo->TexCoordType = CopyFrom->TexCoordType;
CopyTo->RGBorHSV = CopyFrom->RGBorHSV;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	GetAnimPtr(Ct)->Copy((AnimCritter *)CopyTo->GetAnimPtr(Ct), (AnimCritter *)CopyFrom->GetAnimPtr(Ct));
	} // for

CopyTo->Color.Copy(&CopyTo->Color, &CopyFrom->Color);
strcpy(CopyTo->Name, CopyFrom->Name);
RootTextureParent::Copy(CopyTo, CopyFrom);
CopySpecificData(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // PostProcessEvent::Copy

/*===========================================================================*/

void PostProcessEvent::SetName(char *NewName)
{
NotifyTag Changes[2];

strcpy(Name, NewName);
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // PostProcessEvent::SetName

/*===========================================================================*/

ULONG PostProcessEvent::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
long Ct;
char TexCoordType, SetTexTypeToPixels = 0;
union MultiVal MV;
RootTexture *DelTex;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTS_BROWSEDATA:
						{
						if (BrowseInfo)
							BrowseInfo->FreeAll();
						else
							BrowseInfo = new BrowseData();
						if (BrowseInfo)
							BytesRead = BrowseInfo->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_AFFECTRED:
						{
						BytesRead = ReadBlock(ffile, (char *)&AffectRed, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_AFFECTGRN:
						{
						BytesRead = ReadBlock(ffile, (char *)&AffectGrn, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_AFFECTBLU:
						{
						BytesRead = ReadBlock(ffile, (char *)&AffectBlu, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_TEXCOORDTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexCoordType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexCoordType == WCS_POSTPROCEVENT_COORDTYPE_PROCEDURAL)
							SetTexTypeToPixels = 1;
						break;
						}
					case WCS_POSTPROCEVENT_RGBORHSV:
						{
						BytesRead = ReadBlock(ffile, (char *)&RGBorHSV, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_INTENSITY:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE1:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE2:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE3:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE4:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE5:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE6:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE6].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE7:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE7].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE8:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE8].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE9:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE9].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_VALUE10:
						{
						BytesRead = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE10].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_COLOR:
						{
						BytesRead = Color.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEVENT_TEXINTENSITY:
						{
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_TEXFILTER:
						{
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_TEXVALUE1:
						{
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_TEXVALUE2:
						{
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_TEXVALUE3:
						{
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE3] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE3]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_POSTPROCEVENT_SPECIFICDATA:
						{
						BytesRead = LoadSpecificData(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

if (SetTexTypeToPixels)
	{
	for (Ct = 0; Ct < WCS_POSTPROCEVENT_NUMTEXTURES; ++Ct)
		{
		if (TexRoot[Ct])
			{
			TexRoot[Ct]->PropagateCoordSpace(WCS_TEXTURE_COORDSPACE_IMAGE_PIXELUNITS);
			} // if
		} // for
	} // if

return (TotalRead);

} // PostProcessEvent::Load

/*===========================================================================*/

unsigned long PostProcessEvent::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long AnimItemTag[WCS_POSTPROCEVENT_NUMANIMPAR] = {WCS_POSTPROCEVENT_INTENSITY, 
																	WCS_POSTPROCEVENT_VALUE1,
																	WCS_POSTPROCEVENT_VALUE2, 
																	WCS_POSTPROCEVENT_VALUE3,
																	WCS_POSTPROCEVENT_VALUE4,
																	WCS_POSTPROCEVENT_VALUE5,
																	WCS_POSTPROCEVENT_VALUE6,
																	WCS_POSTPROCEVENT_VALUE7,
																	WCS_POSTPROCEVENT_VALUE8,
																	WCS_POSTPROCEVENT_VALUE9,
																	WCS_POSTPROCEVENT_VALUE10};
unsigned long TextureItemTag[WCS_POSTPROCEVENT_NUMTEXTURES] = {WCS_POSTPROCEVENT_TEXINTENSITY,
																	WCS_POSTPROCEVENT_TEXFILTER,
																	WCS_POSTPROCEVENT_TEXVALUE1,
																	WCS_POSTPROCEVENT_TEXVALUE2,
																	WCS_POSTPROCEVENT_TEXVALUE3};

if (BrowseInfo)
	{
	ItemTag = WCS_EFFECTS_BROWSEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = BrowseInfo->Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if browse data saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_AFFECTRED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AffectRed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_AFFECTGRN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AffectGrn)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_AFFECTBLU, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AffectBlu)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
//if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_TEXCOORDTYPE, WCS_BLOCKSIZE_CHAR,
//	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
//	WCS_BLOCKTYPE_CHAR, (char *)&TexCoordType)) == NULL)
//	goto WriteError;
//TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEVENT_RGBORHSV, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RGBorHSV)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_POSTPROCEVENT_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Color.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if Color saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (TexRoot[Ct])
		{
		ItemTag = TextureItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = TexRoot[Ct]->Save(ffile))
					{
					TotalWritten += BytesWritten;
					fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
					if (WriteBlock(ffile, (char *)&BytesWritten,
						WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
						{
						fseek(ffile, 0, SEEK_END);
						} // if wrote size of block
					else
						goto WriteError;
					} // if texture saved
				else
					goto WriteError;
				} // if size written
			else
				goto WriteError;
			} // if tag written
		else
			goto WriteError;
		} // if
	} // for

ItemTag = WCS_POSTPROCEVENT_SPECIFICDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SaveSpecificData(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if specific data saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcessEvent::Save

/*===========================================================================*/

ULONG PostProcessEvent::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, BytesRead, TotalRead = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcessEvent::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcessEvent::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcessEvent::SaveSpecificData

/*===========================================================================*/

char *PostProcessEventCritterNames[WCS_POSTPROCEVENT_NUMANIMPAR] = {"Intensity (%)", "Value 1", "Value 2", "Value 3", "Value 4", "Value 5", "Value 6", "Value 7", "Value 8", "Value 9", "Value 10"};
char *PostProcessEventTextureNames[WCS_POSTPROCEVENT_NUMTEXTURES] = {"Intensity (%)", "Filter", "Value 1", "Value 2", "Value 3"};

char *PostProcessEvent::GetCritterName(RasterAnimHost *Test)
{
char Ct;
char *ValName;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (Test == GetAnimPtr(Ct))
		{
		if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE1 && UseValue1(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE2 && UseValue2(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE3 && UseValue3(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE4 && UseValue4(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE5 && UseValue5(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE6 && UseValue6(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE7 && UseValue7(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE8 && UseValue8(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE9 && UseValue9(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_ANIMPAR_VALUE10 && UseValue10(ValName) && ValName)
			return (ValName);
		else
			return (PostProcessEventCritterNames[Ct]);
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		if (Ct == WCS_POSTPROCEVENT_TEXTURE_FILTER && UseFilter(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_TEXTURE_VALUE1 && UseValueTexture1(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_TEXTURE_VALUE2 && UseValueTexture2(ValName) && ValName)
			return (ValName);
		else if (Ct == WCS_POSTPROCEVENT_TEXTURE_VALUE3 && UseValueTexture3(ValName) && ValName)
			return (ValName);
		else
			return (PostProcessEventTextureNames[Ct]);
		} // if
	} // for
if (Test == &Color)
	{
	if (UseColor(ValName) && ValName)
		return (ValName);
	else
		return ("Color");
	} // if

return ("");

} // PostProcessEvent::GetCritterName

/*===========================================================================*/

char *PostProcessEvent::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as an Post Process Event Texture! Remove anyway?");

} // PostProcessEvent::OKRemoveRaster

/*===========================================================================*/

char *PostProcessEvent::GetTextureName(long TexNumber)
{
RootTexture *Tex;
char *ADTLabel;

if (TexNumber < GetNumTextures())
	{
	if (Tex = GetTexRootPtr(TexNumber))
		{
		return (GetCritterName(Tex));
		} // if
	if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_FILTER && UseFilter(ADTLabel) && ADTLabel)
		return (ADTLabel);
	if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE1 && UseValueTexture1(ADTLabel) && ADTLabel)
		return (ADTLabel);
	if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE2 && UseValueTexture2(ADTLabel) && ADTLabel)
		return (ADTLabel);
	if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE3 && UseValueTexture3(ADTLabel) && ADTLabel)
		return (ADTLabel);
	return (PostProcessEventTextureNames[TexNumber]);
	} // if

return ("");

} // PostProcessEvent::GetTextureName

/*===========================================================================*/

void PostProcessEvent::GetTextureApplication(RasterAnimHost *Test, unsigned char &ApplyToColor, unsigned char &ApplyToDisplace)
{

ApplyToColor = (Test == GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER));
ApplyToDisplace = 0;

} // PostProcessEvent::GetTextureApplication

/*===========================================================================*/

RootTexture *PostProcessEvent::NewRootTexture(long TexNumber)
{
char *ADTLabel;
int DoIt;
char ApplyToColor = TexNumber == WCS_POSTPROCEVENT_TEXTURE_FILTER ? 1: 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 0;

if (TexNumber == 0)
	DoIt = 1;
else if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_FILTER && UseFilter(ADTLabel) && ADTLabel)
	DoIt = 1;
else if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE1 && UseValueTexture1(ADTLabel) && ADTLabel)
	DoIt = 1;
else if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE2 && UseValueTexture2(ADTLabel) && ADTLabel)
	DoIt = 1;
else if (TexNumber == WCS_POSTPROCEVENT_TEXTURE_VALUE3 && UseValueTexture3(ADTLabel) && ADTLabel)
	DoIt = 1;
else
	DoIt = 0;

if (DoIt && TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // PostProcessEvent::NewRootTexture

/*===========================================================================*/

char PostProcessEvent::GetRAHostDropOK(long DropType)
{
char *ValName;

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME && UseColor(ValName))
	return (1);
return (0);

} // PostProcessEvent::GetRAHostDropOK

/*===========================================================================*/

int PostProcessEvent::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
RasterAnimHost *TargetList[30];
int Success = 0;
long Ct, NumListItems = 0;
char WinNum;

if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = Color.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); ++Ct)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if (GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // PostProcessEvent::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long PostProcessEvent::GetRAFlags(unsigned long Mask)
{
unsigned long Flags = 0;

if (Mask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(Mask, Flags);
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (Mask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if

Mask &= (WCS_RAHOST_ICONTYPE_POSTPROC | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // PostProcessEvent::GetRAFlags

/*===========================================================================*/

RasterAnimHost *PostProcessEvent::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, *ValName, Found = 0;

if (! Current)
	Found = 1;
if (Found && UseColor(ValName))
	return (&Color);
if (Current == &Color)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (Found && UseValue(Ct))
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (Found && GetTexRootPtr(Ct) && UseTexture(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // PostProcessEvent::GetRAHostChild

/*===========================================================================*/

int PostProcessEvent::UseValue(char ValNum)
{
char *TestStr;

switch (ValNum)
	{
	case WCS_POSTPROCEVENT_ANIMPAR_INTENSITY:
		{
		return (1);
		} // WCS_POSTPROCEVENT_ANIMPAR_INTENSITY
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE1:
		{
		return (UseValue1(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE1
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE2:
		{
		return (UseValue2(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE2
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE3:
		{
		return (UseValue3(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE3
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE4:
		{
		return (UseValue4(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE4
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE5:
		{
		return (UseValue5(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE5
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE6:
		{
		return (UseValue6(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE6
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE7:
		{
		return (UseValue7(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE7
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE8:
		{
		return (UseValue8(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE8
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE9:
		{
		return (UseValue9(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE9
	case WCS_POSTPROCEVENT_ANIMPAR_VALUE10:
		{
		return (UseValue10(TestStr));
		} // WCS_POSTPROCEVENT_ANIMPAR_VALUE10
	} // switch

return (0);

} // PostProcessEvent::UseValue

/*===========================================================================*/

int PostProcessEvent::UseTexture(char ValNum)
{
char *TestStr;

switch (ValNum)
	{
	case WCS_POSTPROCEVENT_TEXTURE_INTENSITY:
		{
		return (1);
		} // WCS_POSTPROCEVENT_TEXTURE_INTENSITY
	case WCS_POSTPROCEVENT_TEXTURE_FILTER:
		{
		return (UseFilter(TestStr));
		} // WCS_POSTPROCEVENT_TEXTURE_FILTER
	case WCS_POSTPROCEVENT_TEXTURE_VALUE1:
		{
		return (UseValueTexture1(TestStr));
		} // WCS_POSTPROCEVENT_TEXTURE_VALUE1
	case WCS_POSTPROCEVENT_TEXTURE_VALUE2:
		{
		return (UseValueTexture2(TestStr));
		} // WCS_POSTPROCEVENT_TEXTURE_VALUE2
	case WCS_POSTPROCEVENT_TEXTURE_VALUE3:
		{
		return (UseValueTexture3(TestStr));
		} // WCS_POSTPROCEVENT_TEXTURE_VALUE3
	} // switch

return (0);

} // PostProcessEvent::UseTexture

/*===========================================================================*/

int PostProcessEvent::RemoveRAHost(RasterAnimHost *RemoveMe)
{
int Removed = 0;
NotifyTag Changes[2];
char Ct;

if (RemoveMe)
	{
	for (Ct = 0; Ct < GetNumTextures(); ++Ct)
		{
		if (RemoveMe == GetTexRootPtr(Ct))
			{
			SetTexRootPtr(Ct, NULL);
			delete (RootTexture *)RemoveMe;
			Removed = 1;
			} // if
		} // for

	if (Removed)
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if

return (Removed);

} // PostProcessEvent::RemoveRAHost

/*===========================================================================*/

int PostProcessEvent::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (Test == GetTexRootPtr(Ct))
		return (1);
	} // for

return (0);

} // PostProcessEvent::GetDeletable

/*===========================================================================*/

int PostProcessEvent::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
if (Color.GetRAHostAnimated())
	return (1);

return (0);

} // PostProcessEvent::GetRAHostAnimated

/*===========================================================================*/

long PostProcessEvent::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (GetAnimPtr(Ct)->GetMinMaxDist(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	} // for

if (Found)
	{
	FirstKey = TestFirst;
	LastKey = TestLast;
	} // if
else
	{
	FirstKey = LastKey = 0;
	} // else

return (Found);

} // PostProcessEvent::GetKeyFrameRange

/*===========================================================================*/

void PostProcessEvent::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = GetName();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPE)
	{
	Prop->Type = GetRAHostTypeString();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_TYPENUMBER)
	{
	Prop->TypeNumber = GetRAHostTypeNumber();
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPOK)
	{
	Prop->DropOK = GetRAHostDropOK(Prop->TypeNumber);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	GetKeyFrameRange(Prop->KeyNodeRange[0], Prop->KeyNodeRange[1]);
	} // if
else if (Prop->PropMask & WCS_RAHOST_MASKBIT_NEXTKEY)
	{
	GetNextAnimNode(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_FILEINFO)
	{
	Prop->Path = EffectsLib::DefaultPaths[WCS_EFFECTSSUBCLASS_RENDEROPT];
	Prop->Ext = "ppe";
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // PostProcessEvent::GetRAHostProperties

/*===========================================================================*/

int PostProcessEvent::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
NotifyTag Changes[2];
int Success = 0;
char NameStr[WCS_POSTPROC_MAXNAMELENGTH];

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_ENABLED)
		{
		Enabled = (Prop->Flags & WCS_RAHOST_FLAGBIT_ENABLED) ? 1: 0;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	if (Prop->FlagsMask & WCS_RAHOST_FLAGBIT_EXPANDED)
		SetExpansionFlags(Prop->FlagsMask, Prop->Flags);
	Success = 1;
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	strncpy(NameStr, Prop->Name, WCS_POSTPROC_MAXNAMELENGTH);
	NameStr[WCS_POSTPROC_MAXNAMELENGTH - 1] = 0;
	Success = 1;
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_DROPSOURCE)
	{
	Success = ProcessRAHostDragDrop(Prop);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_KEYRANGE)
	{
	if (Success = ScaleDeleteAnimNodes(Prop))
		{
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_LOADFILE)
	{
	return(LoadObject(Prop->fFile, 0, Prop->ByteFlip));
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_SAVEFILE)
	{
	return(SaveObject(Prop->fFile));
	} // if

return (Success);

} // PostProcessEvent::SetRAHostProperties

/*===========================================================================*/

int PostProcessEvent::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
if (Color.SetToTime(Time))
	Found = 1;

return (Found);

} // PostProcessEvent::SetToTime

/*===========================================================================*/

int PostProcessEvent::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil)
{
char *ValName;
long Ct;

AnimAffil = NULL;
TexAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); ++Ct)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_POSTPROCEVENT_ANIMPAR_INTENSITY:
					{
					TexAffil = GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_INTENSITY);
					break;
					} // 
				case WCS_POSTPROCEVENT_ANIMPAR_VALUE1:
					{
					if (UseTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE1))
						TexAffil = GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_VALUE1);
					break;
					} // 
				case WCS_POSTPROCEVENT_ANIMPAR_VALUE2:
					{
					if (UseTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE2))
						TexAffil = GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_VALUE2);
					break;
					} // 
				case WCS_POSTPROCEVENT_ANIMPAR_VALUE3:
					{
					if (UseTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE3))
						TexAffil = GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_VALUE3);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	if (ChildA == &Color)
		{
		AnimAffil = (AnimCritter *)ChildA;
		if (UseFilter(ValName))
			TexAffil = GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_FILTER);
		return (1);
		} // if
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); ++Ct)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_POSTPROCEVENT_TEXTURE_INTENSITY:
					{
					AnimAffil = GetAnimPtr(WCS_POSTPROCEVENT_ANIMPAR_INTENSITY);
					break;
					} // 
				case WCS_POSTPROCEVENT_TEXTURE_FILTER:
					{
					if (UseColor(ValName))
						AnimAffil = &Color;
					break;
					} // 
				case WCS_POSTPROCEVENT_TEXTURE_VALUE1:
					{
					if (UseValue(WCS_POSTPROCEVENT_ANIMPAR_VALUE1))
						AnimAffil = GetAnimPtr(WCS_POSTPROCEVENT_ANIMPAR_VALUE1);
					break;
					} // 
				case WCS_POSTPROCEVENT_TEXTURE_VALUE2:
					{
					if (UseValue(WCS_POSTPROCEVENT_ANIMPAR_VALUE2))
						AnimAffil = GetAnimPtr(WCS_POSTPROCEVENT_ANIMPAR_VALUE2);
					break;
					} // 
				case WCS_POSTPROCEVENT_TEXTURE_VALUE3:
					{
					if (UseValue(WCS_POSTPROCEVENT_ANIMPAR_VALUE3))
						AnimAffil = GetAnimPtr(WCS_POSTPROCEVENT_ANIMPAR_VALUE3);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if


return (0);

} // PostProcessEvent::GetAffiliates

/*===========================================================================*/

int PostProcessEvent::GetPopClassFlags(RasterAnimHostProperties *Prop)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

Prop->PopClassFlags = 0;
Prop->PopExistsFlags = 0;
Prop->PopEnabledFlags = 0;

if (GetAffiliates(Prop->ChildA, Prop->ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::GetPopClassFlags(Prop, AnimAffil, TexAffil, NULL));
	} // if

return (0);

} // PostProcessEvent::GetPopClassFlags

/*===========================================================================*/

int PostProcessEvent::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, NULL));
	} // if

return(0);

} // PostProcessEvent::AddSRAHBasePopMenus

/*===========================================================================*/

int PostProcessEvent::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
RootTexture *RootTex;
Texture *Tex;
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;
int Handled = 0;
long ItemNumber;
NotifyTag Changes[2];

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	if (! strcmp((char *)Action, "TX_CREATE") && TexAffil && ! (*TexAffil))
		{
		if ((ItemNumber = GetTexNumberFromAddr(TexAffil)) >= 0 && (RootTex = NewRootTexture(ItemNumber)))
			{
			// set texture type to fractal noise
			if (Tex = RootTex->AddNewTexture(NULL, WCS_TEXTURE_TYPE_FRACTALNOISE))
				{
				Tex->TexSize[2].SetValue(1.49598e11);
				} // if
			(*TexAffil)->EditRAHost();
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		Handled = 1;
		} // if
	else
		Handled = RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, NULL, this, NULL);
	} // if

return (Handled);

} // PostProcessEvent::HandleSRAHPopMenuSelection

/*===========================================================================*/

long PostProcessEvent::InitImageIDs(long &ImageID)
{
long NumImages = 0;
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (NumImages);

} // PostProcessEvent::InitImageIDs

/*===========================================================================*/

void PostProcessEvent::ResolveLoadLinkages(EffectsLib *Lib)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct))
		{
		GetTexRootPtr(Ct)->ResolveLoadLinkages(Lib);
		} // if
	} // for

} // PostProcessEvent::ResolveLoadLinkages

/*===========================================================================*/

int PostProcessEvent::AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->Enabled)
		{
		if (! GetTexRootPtr(Ct)->InitToRender(Opt, Buffers))
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // PostProcessEvent::AddRenderBuffers

/*===========================================================================*/

int PostProcessEvent::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->Enabled)
		{
		if (! GetTexRootPtr(Ct)->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // PostProcessEvent::InitToRender

/*===========================================================================*/

int PostProcessEvent::InitFrameToRender(EffectsLib *Lib, RenderData *Rend)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->Enabled)
		{
		if (! GetTexRootPtr(Ct)->InitFrameToRender(Lib, Rend))
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // PostProcessEvent::InitFrameToRender

/*===========================================================================*/

int PostProcessEvent::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

return(1); // do nothing in base implementation

} // PostProcessEvent::PrepForPostProc

/*===========================================================================*/

int PostProcessEvent::AdvanceRow(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

return(1); // do nothing in base implementation

} // PostProcessEvent::AdvanceRow

/*===========================================================================*/

int PostProcessEvent::CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

return(1); // do nothing in base implementation

} // PostProcessEvent::CleanupPostProc

/*===========================================================================*/

int PostProcessEvent::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double ModHSV[3], UnModHSV[3];
rPixelHeader *FragMap = NULL;

Success = 1;
CurFrag = NULL;

// if nothing to do
if (! (AffectRed || AffectGrn || AffectBlu))
	return (1);

if (FragBlock && InhibitRenderBeforeReflections())
	return (1);

Value[0] = Value[1] = Value[2] = 0.0;

// assign buffer ptrs - at least all the ones the renderer needs to know about
if (OptionalBitmaps)
	{
	Bitmap[0] = OptionalBitmaps[0];
	Bitmap[1] = OptionalBitmaps[1];
	Bitmap[2] = OptionalBitmaps[2];
	} // if
else
	{
	Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
	Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
	Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
	} // else

if (FragBlock)
	FragMap = FragBlock->GetFragMap();

if (! ((FragBlock && FragMap) || (Bitmap[0] && Bitmap[1] && Bitmap[2])))
	return (0);

ZBuf = (float *)Buffers->FindBuffer("ZBUF",								WCS_RASTER_BANDSET_FLOAT);
AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS",				WCS_RASTER_BANDSET_BYTE);
LatBuf = (float *)Buffers->FindBuffer("LATITUDE",						WCS_RASTER_BANDSET_FLOAT);
LonBuf = (float *)Buffers->FindBuffer("LONGITUDE",						WCS_RASTER_BANDSET_FLOAT);
ElevBuf = (float *)Buffers->FindBuffer("ELEVATION",						WCS_RASTER_BANDSET_FLOAT);
RelElBuf = (float *)Buffers->FindBuffer("RELATIVE ELEVATION",			WCS_RASTER_BANDSET_FLOAT);
ReflectionBuf = (float *)Buffers->FindBuffer("REFLECTION",				WCS_RASTER_BANDSET_FLOAT);
IllumBuf = (float *)Buffers->FindBuffer("ILLUMINATION",					WCS_RASTER_BANDSET_FLOAT);
SlopeBuf = (float *)Buffers->FindBuffer("SLOPE",						WCS_RASTER_BANDSET_FLOAT);
AspectBuf = (float *)Buffers->FindBuffer("ASPECT",						WCS_RASTER_BANDSET_FLOAT);
NormalBuf[0] = (float *)Buffers->FindBuffer("SURFACE NORMAL X",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[1] = (float *)Buffers->FindBuffer("SURFACE NORMAL Y",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[2] = (float *)Buffers->FindBuffer("SURFACE NORMAL Z",			WCS_RASTER_BANDSET_FLOAT);
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
ObjectBuf = (RasterAnimHost **)Buffers->FindBuffer("OBJECT",			WCS_RASTER_BANDSET_FLOAT);
ObjTypeBuf = (unsigned char *)Buffers->FindBuffer("OBJECT TYPE",		WCS_RASTER_BANDSET_BYTE);

// invoke prep on derived class, bail if it fails
if (!PrepForPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
	{
	CleanupPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	return(0);
	} // if

TexData.VDEM[0] = &VertDEM;
TexData.MetersPerDegLat = Rend->EarthLatScaleMeters;
TexData.MetersPerDegLon = Rend->RefLonScaleMeters;
TexData.TexRefLon = Rend->TexRefLon;
TexData.TexRefLat = Rend->TexRefLat;
TexData.TexRefElev = Rend->TexRefElev;

// support multi-pass operations
for (PassNum = 0; PassNum < InquirePasses(); PassNum++) // didn't want to wreck indenting, so no braces here
for (Row = PixZip = 0; Row < Height; Row ++)
	{
	for (Col = 0; Col < Width; Col ++, PixZip ++)
		{
		// set texture data
		TexData.Elev = ElevBuf ? ElevBuf[PixZip]: 0.0;
		TexData.RelElev = RelElBuf ? RelElBuf[PixZip]: 0.0f;
		TexData.Slope = SlopeBuf ? SlopeBuf[PixZip]: 0.0;
		TexData.Aspect = AspectBuf ? AspectBuf[PixZip]: 0.0;
		TexData.Illumination = IllumBuf ? IllumBuf[PixZip]: 0.0;
		TexData.Object = ObjectBuf ? ObjectBuf[PixZip]: NULL;
		TexData.ObjectType = ObjTypeBuf ? ObjTypeBuf[PixZip]: NULL;
		TexData.Latitude = LatBuf ? LatBuf[PixZip] + Rend->TexRefLat: 0.0;
		TexData.Longitude = LonBuf ? LonBuf[PixZip] + Rend->TexRefLon: 0.0;
		TexData.Reflectivity = ReflectionBuf ? ReflectionBuf[PixZip]: 0.0;
		TexData.Normal[0] = NormalBuf[0] ? NormalBuf[0][PixZip]: 0.0;
		TexData.Normal[1] = NormalBuf[1] ? NormalBuf[1][PixZip]: 0.0;
		TexData.Normal[2] = NormalBuf[2] ? NormalBuf[2][PixZip]: 0.0;
		TexData.ZDist = ZBuf ? ZBuf[PixZip]: 0.0;
		TexData.QDist = TexData.ZDist;

		TexData.PixelX[0] = (double)Col + Rend->Rend->SegmentOffsetX - Width * Rend->Cam->PanoPanels * .5;
		TexData.PixelX[1] = (double)(Col + 1) + Rend->Rend->SegmentOffsetX - Width * Rend->Cam->PanoPanels * .5;
		TexData.PixelY[0] = -((double)Row + Rend->Rend->SegmentOffsetY) + Height * Rend->Opt->RenderImageSegments * .5;
		TexData.PixelY[1] = -((double)(Row + 1) + Rend->Rend->SegmentOffsetY) + Height * Rend->Opt->RenderImageSegments * .5;
		TexData.PixelUnityX[0] = TexData.PixelX[0] / (Width * Rend->Cam->PanoPanels);
		TexData.PixelUnityX[1] = TexData.PixelX[1] / (Width * Rend->Cam->PanoPanels);
		TexData.PixelUnityY[0] = TexData.PixelY[0] / (Height * Rend->Opt->RenderImageSegments);
		TexData.PixelUnityY[1] = TexData.PixelY[1] / (Height * Rend->Opt->RenderImageSegments);

		//TexData.PixelX[0] = Col - Width * .5;
		//TexData.PixelX[1] = Col + 1 - Width * .5;
		//TexData.PixelY[0] = -Row + Height * .5;
		//TexData.PixelY[1] = -Row - 1 + Height * .5;
		//TexData.PixelUnityX[0] = TexData.PixelX[0] / Width;
		//TexData.PixelUnityX[1] = TexData.PixelX[1] / Width;
		//TexData.PixelUnityY[0] = TexData.PixelY[0] / Height;
		//TexData.PixelUnityY[1] = TexData.PixelY[1] / Height;
		VertDEM.ScrnXYZ[0] = Col + .5; 
		VertDEM.ScrnXYZ[1] = Row + .5; 
		//TexData.LowX = Col;
		//TexData.HighX = Col + 1;
		//TexData.LowY = Row;
		//TexData.HighY = Row + 1;
		//TexData.LowZ = TexData.ZDist;
		//TexData.HighZ = TexData.ZDist;
		//TexData.LatRange[0] = TexData.LatRange[1] = TexData.LatRange[2] = TexData.LatRange[3] = TexData.Latitude;
		//TexData.LonRange[0] = TexData.LonRange[1] = TexData.LonRange[2] = TexData.LonRange[3] = TexData.Longitude;
		if (Rend->Cam && Rend->Cam->TargPos)
			{
			TexData.CamAimVec[0] = -Rend->Cam->TargPos->XYZ[0];
			TexData.CamAimVec[1] = -Rend->Cam->TargPos->XYZ[1];
			TexData.CamAimVec[2] = -Rend->Cam->TargPos->XYZ[2];
			} // if
		//if (TexCoordType == WCS_POSTPROCEVENT_COORDTYPE_IMAGE)
		//	{
		//	TexData.LowX /= Width;
		//	TexData.LowX -= .5;
		//	TexData.HighX /= Width;
		//	TexData.HighX -= .5;
		//	TexData.LowY /= -Height;
		//	TexData.LowY += .5;
		//	TexData.HighY /= -Height;
		//	TexData.HighY += .5;
		//	} // if
		//else
		//	{
		//	TexData.LowX -= Width * .5;
		//	TexData.HighX -= Width * .5;
		//	TexData.LowY = -TexData.LowY + Height * .5;
		//	TexData.HighY = -TexData.HighY + Height * .5;
		//	} // else
		if (FragBlock)
			{
			// set fragment texture data
			if (CurFrag = FragMap[PixZip].FragList)
				{
				TexData.ZDist = CurFrag->ZBuf;
				TexData.QDist = TexData.ZDist;
				TexData.ObjectType = CurFrag->GetObjectType();
				} // if
			} // if
		while (! FragBlock || CurFrag)
			{
			// this sets up XYZ and lat lon coords for alternate texture coordinate spaces
			VertDEM.ScrnXYZ[2] = VertDEM.Q = TexData.ZDist > 0.0 ? TexData.ZDist: 1.0;
			Rend->Cam->UnProjectVertexDEM(Rend->DefCoords, &VertDEM, Rend->EarthLatScaleMeters, Rend->PlanetRad, 1);
			// get existing values
			if (CurFrag)
				{
				CurFrag->ExtractColor(Value);
				TexData.RGB[0] = TexColor[0] = OrigValue[0] = Value[0];
				TexData.RGB[1] = TexColor[1] = OrigValue[1] = Value[1];
				TexData.RGB[2] = TexColor[2] = OrigValue[2] = Value[2];
				} // if
			else
				{
				if (ExponentBuf)
					{
					Rend->Rend->GetPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
					TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0];
					TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1];
					TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2];
					} // if
				else
					{
					TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
					TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
					TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
					} // else
				} // if

			// evaluate texture intensity
			if ((Intensity = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].CurValue) > 0.0)
				{
				if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Enabled)
					{
					if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Eval(Value, &TexData)) > 0.0)
						{
						if (TexOpacity < 1.0)
							{
							// Value[0] has already been diminished by the texture's opacity
							Intensity *= (1.0 - TexOpacity + Value[0]);
							} // if
						else
							Intensity *= Value[0];
						} // if
					} // if
				} // if

			if (Intensity > 0.0)
				{
				// evaluate texture
				Value[0] = OrigValue[0];
				Value[1] = OrigValue[1];
				Value[2] = OrigValue[2];
				TexData.InputSuppliedInOutput = 1;

				// call derived processor if present
				if (!(EvalPostProcSample(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics)))
					{
					// if that did nothing, we'll run the texture code
					if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled)
						{
						if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Eval(Value, &TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								TexOpacity = 1.0 - TexOpacity;
								TexColor[0] = TexColor[0] * TexOpacity + Value[0];
								TexColor[1] = TexColor[1] * TexOpacity + Value[1];
								TexColor[2] = TexColor[2] * TexOpacity + Value[2];
								} // if
							else
								{
								TexColor[0] = Value[0];
								TexColor[1] = Value[1];
								TexColor[2] = Value[2];
								} // else
							} // if
						} // if
					} // if

				if (RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
					{
					RGBtoHSV(UnModHSV, OrigValue);
					RGBtoHSV(ModHSV, TexColor);
					if (AffectRed)
						UnModHSV[0] = ModHSV[0];
					if (AffectGrn)
						UnModHSV[1] = ModHSV[1];
					if (AffectBlu)
						UnModHSV[2] = ModHSV[2];
					HSVtoRGB(UnModHSV, TexColor);
					} // if

				// lerp output values
				InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
				if (CurFrag || (ExponentBuf && UpdateDiagnostics))
					{
					if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[0] = OrigValue[0] * InvIntensity + TexColor[0] * Intensity;
					if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[1] = OrigValue[1] * InvIntensity + TexColor[1] * Intensity;
					if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[2] = OrigValue[2] * InvIntensity + TexColor[2] * Intensity;
					} // if
				if (CurFrag)
					{
					CurFrag->PlotPixel(OrigValue);
					} // if
				else if (ExponentBuf && UpdateDiagnostics)
					{
					Rend->Rend->PlotPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
					} // if
				else
					{
					Intensity *= 255.0;
					if (TexColor[0] > 1.0)
						TexColor[0] = 1.0;
					if (TexColor[1] > 1.0)
						TexColor[1] = 1.0;
					if (TexColor[2] > 1.0)
						TexColor[2] = 1.0;
					if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						Bitmap[0][PixZip] = (unsigned char)(Bitmap[0][PixZip] * InvIntensity + TexColor[0] * Intensity);
					if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						Bitmap[1][PixZip] = (unsigned char)(Bitmap[1][PixZip] * InvIntensity + TexColor[1] * Intensity);
					if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						Bitmap[2][PixZip] = (unsigned char)(Bitmap[2][PixZip] * InvIntensity + TexColor[2] * Intensity);
					} // else
				} // if intensity

			if (CurFrag)
				{
				// set fragment texture data
				if (CurFrag = CurFrag->Next)
					{
					TexData.ZDist = CurFrag->ZBuf;
					TexData.QDist = TexData.ZDist;
					TexData.ObjectType = CurFrag->GetObjectType();
					} // if
				} // if
			else
				break;
			} // while
		} // for
	if (BWDE)
		{
		if (BWDE)
			{
			if (BWDE->Update(Row + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else if (Master)
		{
		Master->ProcUpdate(Row + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	AdvanceRow(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	} // for

// invoke cleanup on derived class
CleanupPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

return (Success);

} // PostProcessEvent::RenderPostProc

/*===========================================================================*/

int PostProcessEvent::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

return(0); // do nothing in base class implementation

} // PostProcessEvent::EvalPostProcSample

/*===========================================================================*/

int PostProcessEvent::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;

for (Ct = 0; Ct < GetNumTextures(); ++Ct)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (1);

} // PostProcessEvent::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int PostProcessEvent::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
PostProcessEvent *CurrentEvent = NULL;
CoordSys *CurrentCoordSys = NULL;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
char ReadBuf[WCS_POSTPROC_MAXNAMELENGTH];
char EventName[WCS_POSTPROC_MAXNAMELENGTH];

if (! ffile)
	return (0);

if (LoadToEffects = new EffectsLib())
	{
	if (LoadToImages = new ImageLib())
		{
		// set some global pointers so that things know what libraries to link to
		GlobalApp->LoadToEffectsLib = LoadToEffects;
		GlobalApp->LoadToImageLib = LoadToImages;

		while (BytesRead && Success)
			{
			// read block descriptor tag from file 
			if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
				WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
				{
				TotalRead += BytesRead;
				ReadBuf[8] = 0;
				// read block size from file 
				if (BytesRead = ReadBlock(ffile, (char *)&Size,
					WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
					{
					TotalRead += BytesRead;
					BytesRead = 0;
					if (! strnicmp(ReadBuf, "DEMBnds", 8))
						{
						//char CharSize;
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						// some bad files were written initially and this code cleans it up
						//fseek(ffile, 4, SEEK_CUR);
						//ReadBlock(ffile, (char *)&CharSize, WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_CHAR, ByteFlip);
						//ReadBlock(ffile, (char *)EventName, WCS_BLOCKTYPE_CHAR + CharSize, ByteFlip);
						//TotalRead += 5 + CharSize;
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "PostEvTp", 8))
						{
						BytesRead = ReadBlock(ffile, (char *)EventName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						} // if PostProcessEvent
					else if (! strnicmp(ReadBuf, "PostEvnt", 8))
						{
						if (CurrentEvent = NewEvent(NULL, GetEventTypeFromName(EventName)))
							{
							if ((BytesRead = CurrentEvent->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if PostProcessEvent
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					TotalRead += BytesRead;
					if (BytesRead != Size)
						{
						Success = 0;
						break;
						} // if error
					} // if size block read 
				else
					break;
				} // if tag block read 
			else
				break;
			} // while 
		} // if image lib
	else
		Success = 0;
	} // if effects lib
else
	Success = 0;

if (Success == 1 && CurrentEvent)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	if (CurrentEvent->GetType() == GetType())
		Copy(this, CurrentEvent);
	else
		{
		Success = -1;
		UserMessageOK("Load Post Process Event", "The loaded Event is not of the same type as the Event being loaded to. Change the Selected Event to the type that you wish to load and try again.");
		} // else
	delete CurrentEvent;
	} // if

if (LoadToEffects)
	delete LoadToEffects;
if (LoadToImages)
	delete LoadToImages;
GlobalApp->CopyFromEffectsLib = GlobalApp->AppEffects;
GlobalApp->CopyFromImageLib = GlobalApp->AppImages;
GlobalApp->LoadToEffectsLib = GlobalApp->AppEffects;
GlobalApp->LoadToImageLib = GlobalApp->AppImages;

return (Success);

} // PostProcessEvent::LoadObject

/*===========================================================================*/

int PostProcessEvent::SaveObject(FILE *ffile)
{
DEMBounds CurBounds;
char *EventName;
EffectList *CurEffect, *Coords = NULL;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
char StrBuf[12];

if (! ffile)
	return (0);

memset(StrBuf, 0, 9);

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	strcpy(StrBuf, "DEMBnds");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = CurBounds.Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if dem bounds saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if dem bounds

// Images
GlobalApp->AppImages->ClearRasterIDs();
if (InitImageIDs(ImageID))
	{
	strcpy(StrBuf, "Images");
	if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
		WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = GlobalApp->AppImages->Save(ffile, NULL, TRUE))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if Images saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // if images

if (BuildFileComponentsList(&Coords))
	{
	#ifdef WCS_BUILD_VNS
	CurEffect = Coords;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "CoordSys");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((CoordSys *)CurEffect->Me)->Save(ffile))
						{
						TotalWritten += BytesWritten;
						fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
						if (WriteBlock(ffile, (char *)&BytesWritten,
							WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
							{
							fseek(ffile, 0, SEEK_END);
							} // if wrote size of block 
						else
							goto WriteError;
						} // if CoordSys saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		CurEffect = CurEffect->Next;
		} // while
	#endif // WCS_BUILD_VNS

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	} // if

// PostProcessEventType
strcpy(StrBuf, "PostEvTp");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		EventName = PostProcEventNames[GetType()];

		if (BytesWritten = WriteBlock(ffile, (char *)EventName,
			(unsigned long)(strlen(EventName) + 1 + WCS_BLOCKTYPE_CHAR)))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if PostProcessEvent type saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

// PostProcessEvent
strcpy(StrBuf, "PostEvnt");
if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
	WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if PostProcessEvent saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

return (TotalWritten);

WriteError:

return (0);

} // PostProcessEvent::SaveObject

/*===========================================================================*/

int PostProcessEvent::ZSampleRelative(signed char XOff, signed char YOff, long Width, long Height, double &ZSample)
{
int ImgRow, SourceX;

ImgRow = Row + YOff;
SourceX = Col + XOff;
if ((SourceX < 0) || (SourceX >= Width)) return(0); // sample column out of image range
if ((ImgRow < 0) || (ImgRow >= Height)) return(0); // sample row out of image range
ZSample = ZBuf[(ImgRow * Width) + SourceX];
return(1);
} // PostProcessEvent::ZSampleRelative

/*===========================================================================*/

int PostProcessEvent::NormDiffSampleRelative(signed char XOff, signed char YOff, long Width, long Height, double &NormDiff, Point3d MySample)
{
Point3d OtherSample;
int ImgRow, SourceX;

if (!(NormalBuf[0] && NormalBuf[1] && NormalBuf[2])) return(0); // no normals available


ImgRow = Row + YOff;
SourceX = Col + XOff;
if ((SourceX < 0) || (SourceX >= Width)) return(0); // sample column out of image range
if ((ImgRow < 0) || (ImgRow >= Height)) return(0); // sample row out of image range
OtherSample[0] = NormalBuf[0][(ImgRow * Width) + SourceX];
OtherSample[1] = NormalBuf[1][(ImgRow * Width) + SourceX];
OtherSample[2] = NormalBuf[2][(ImgRow * Width) + SourceX];
NormDiff = PointDistanceNoSQRT(MySample, OtherSample);
return(1);
} // PostProcessEvent::NormDiffSampleRelative


/*===========================================================================*/
/*===========================================================================*/

PostProcGain::PostProcGain(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to gain
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_GAIN))
		{
		} // if
	} // if

} // PostProcGain::PostProcGain

/*===========================================================================*/

int PostProcGain::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_GAIN)
	{
	ParamNum = WCS_TEXTURE_GAIN_GAIN;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcGain::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcGamma::PostProcGamma(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to gamma
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_GAMMA))
		{
		} // if
	} // if

} // PostProcGamma::PostProcGamma

/*===========================================================================*/

int PostProcGamma::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_GAMMA)
	{
	ParamNum = WCS_TEXTURE_GAMMA_GAMMA;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcGamma::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcLevels::PostProcLevels(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to levels
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_LEVELS))
		{
		} // if
	} // if

} // PostProcLevels::PostProcLevels

/*===========================================================================*/

int PostProcLevels::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_LEVELS)
	{
	ParamNum = WCS_TEXTURE_LEVELS_MID;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcLevels::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcLighten::PostProcLighten(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to levels
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_LIGHTEN))
		{
		} // if
	} // if

} // PostProcLighten::PostProcLighten

/*===========================================================================*/

int PostProcLighten::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_LIGHTEN)
	{
	ParamNum = WCS_TEXTURE_LIGHTEN_MASK;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcLighten::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcDarken::PostProcDarken(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to levels
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_DARKEN))
		{
		} // if
	} // if

} // PostProcDarken::PostProcDarken

/*===========================================================================*/

int PostProcDarken::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_DARKEN)
	{
	ParamNum = WCS_TEXTURE_DARKEN_MASK;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcDarken::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcContrast::PostProcContrast(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to levels
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_CONTRAST))
		{
		} // if
	} // if

} // PostProcContrast::PostProcContrast

/*===========================================================================*/

int PostProcContrast::FetchADTPtr(AnimDoubleTime *&ADT, char *&ADTLabel)
{
int ParamNum;
RootTexture *Root;
Texture *Tex;

if ((Root = GetTexRootPtr(WCS_POSTPROCEVENT_TEXTURE_FILTER)) && (Tex = Root->Tex) && Tex->GetTexType() == WCS_TEXTURE_TYPE_CONTRAST)
	{
	ParamNum = WCS_TEXTURE_CONTRAST_CONTRAST;
	ADT = Tex->GetParamPtr(ParamNum);
	ADTLabel = Tex->GetParamLabel(ParamNum);
	return (1);
	} // if

return (0);

} // PostProcContrast::FetchADTPtr

/*===========================================================================*/
/*===========================================================================*/

PostProcExposure::PostProcExposure(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
double EffectDefault = -2.0;
double RangeDefaults[3] = {100.0, -100.0, .5};

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults);

} // PostProcExposure::PostProcExposure

/*===========================================================================*/

int PostProcExposure::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

Exposure = pow(2.0, AnimPar[WCS_TEXTEXPOSURE_ANIMPAR_EXPOSURE].CurValue);

return(1);

} // PostProcExposure::PrepForPostProc

/*===========================================================================*/

int PostProcExposure::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

TexColor[0] *= Exposure;
TexColor[1] *= Exposure;
TexColor[2] *= Exposure;

return(1);

} // PostProcExposure::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcTexture::PostProcTexture(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to fractal noise
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_FRACTALNOISE))
		{
		Tex->TexSize[2].SetValue(1.49598e11);
		} // if
	} // if


} // PostProcTexture::PostProcTexture

/*===========================================================================*/
/*===========================================================================*/

// code adapted from SunPosGUI::SetReverse
static void SunTimeString(class Light *lite, char *dest, char *option)
{
double FloatDays, SunTime;
double reflon = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
long SunMonth;
short Hour, Mins, Days;
char *months[12] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
char SunDate;

if (lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue > 23.0)
	{
	SunMonth = 5;
	SunDate = 20;
	} // if
else if (lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue < -23)
	{
	SunMonth = 11;
	SunDate = 19;
	} // else if
else
	{
	FloatDays = .5 + acos(-(lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LAT].CurValue / 23.0)) * PiUnder180 * 365.0 / 360.0; 

	if (lite->FallSeason)
		FloatDays = 365.0 - (int)FloatDays;

	Days = (short)FloatDays;
	if (Days < 12)
		{
		SunMonth = 11;
		SunDate = 19 + Days;
		} // else if
	else if (Days < 43)
		{
		SunMonth = 0;
		SunDate = Days - 12;
		} // else if
	else if (Days < 71)
		{
		SunMonth = 1;
		SunDate = Days - 43;
		} // else if
	else if (Days < 102)
		{
		SunMonth = 2;
		SunDate = Days - 71;
		} // else if
	else if (Days < 132)
		{
		SunMonth = 3;
		SunDate = Days - 102;
		} // else if
	else if (Days < 163)
		{
		SunMonth = 4;
		SunDate = Days - 132;
		} // else if
	else if (Days < 193)
		{
		SunMonth = 5;
		SunDate = Days - 163;
		} // else if
	else if (Days < 224)
		{
		SunMonth = 6;
		SunDate = Days - 193;
		} // else if
	else if (Days < 255)
		{
		SunMonth = 7;
		SunDate = Days - 224;
		} // else if
	else if (Days < 285)
		{
		SunMonth = 8;
		SunDate = Days - 255;
		} // else if
	else if (Days < 316)
		{
		SunMonth = 9;
		SunDate = Days - 285;
		} // else if
	else if (Days < 346)
		{
		SunMonth = 10;
		SunDate = Days - 316;
		} // else if
	else
		{
		SunMonth = 11;
		SunDate = Days - 346;
		} // else
	} // else
SunDate += 1;	// for display purposes

SunTime = 12.0 + (lite->AnimPar[WCS_EFFECTS_LIGHT_ANIMPAR_LON].CurValue - reflon) * 12.0 / 180.0;
while (SunTime < 0.0)
	SunTime += 24.0;
while (SunTime > 24.0)
	SunTime -= 24.0;
Hour = (short)SunTime;
Mins = (short)((SunTime - Hour) * 60 + .5);
if (Mins == 60)
	{
	Mins = 0;
	Hour++;
	} // if
// commented out - we're going 24 hour notation all the time
/***
if (Hour >= 12)
	{
	Hour -= 12;
	AMPM = 1;
	} // if
else
	AMPM = 0;
if (Hour == 0)
	Hour = 12;
***/

if (*option == 'D')	// date & time
	sprintf(dest, "%s %d  %1d:%02d", months[SunMonth], SunDate, Hour, Mins);
else if (*option == 'E')	// date only
	sprintf(dest, "%s %d", months[SunMonth], SunDate);
else if (*option == 'T')	// time only
	sprintf(dest, "%1d:%02d", Hour, Mins);

} // SunTimeString

/*===========================================================================*/
/*===========================================================================*/

PostProcText::PostProcText(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
double EffectDefault[10] = {.5, .5, 1.0, 1.0, 1.0, .25, .75, 2.0, 2.0, 3.0};
double RangeDefaults[10][3] = {10000.0, -10000.0, .01,   
								10000.0, -10000.0, .01,
								10000.0, 0.0, .01,
								10000.0, 0.0, .01,
								FLT_MAX, .01, 1.0,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								1000.0, 0.0, 1.0,
								1000.0, -1000.0, 1.0,
								100.0, 0.0, 1.0,
								};

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERX].SetDefaults(this, (char)1, EffectDefault[0]);	// Center X
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERX].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERX].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERY].SetDefaults(this, (char)2, EffectDefault[1]);	// Center Y
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERY].SetRangeDefaults(RangeDefaults[1]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERY].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_WIDTH].SetDefaults(this, (char)3, EffectDefault[2]);	// Width %
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_WIDTH].SetRangeDefaults(RangeDefaults[2]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_WIDTH].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_HEIGHT].SetDefaults(this, (char)4, EffectDefault[3]);	// Height %
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_HEIGHT].SetRangeDefaults(RangeDefaults[3]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_HEIGHT].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE].SetDefaults(this, (char)5, EffectDefault[4]);	// Z Distance %
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE].SetRangeDefaults(RangeDefaults[4]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT].SetDefaults(this, (char)6, EffectDefault[5]);	// Back Light %
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT].SetRangeDefaults(RangeDefaults[5]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY].SetDefaults(this, (char)7, EffectDefault[6]);	// Shadow Intensity %
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY].SetRangeDefaults(RangeDefaults[6]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY].SetMultiplier(100.0);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_KERNING].SetDefaults(this, (char)8, EffectDefault[7]);	// Kerning pixels
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_KERNING].SetRangeDefaults(RangeDefaults[7]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_LEADING].SetDefaults(this, (char)9, EffectDefault[8]);	// Leading pixels
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_LEADING].SetRangeDefaults(RangeDefaults[8]);
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_OUTLINE].SetDefaults(this, (char)10, EffectDefault[9]);	// Outline
AnimPar[WCS_TEXTOVERLAY_ANIMPAR_OUTLINE].SetRangeDefaults(RangeDefaults[9]);

Illuminate = 0;
ApplyVolumetrics = 0;
ReceiveShadows = 0;
strcpy(MesgText, "Type the overlay text here.");

} // PostProcText::PostProcText

/*===========================================================================*/

char *PPT_TextLabels[] = {"Camera Name", "Camera FOV", "Camera Heading Param", "Camera Compass Bearing", 

 "Camera Pitch Param", "Camera Tilt", "Camera Bank", "Camera Longitude", "Camera Latitude",

 "Camera Elevation",

 "Target Longitude", "Target Latitude", "Target Elevation", "Target Distance",
 
 "Frame Number", "Frame Number SMPTE",
 
 "Project Name", "Render Options Name", "Render Date & Time", "Render Date Only", "Render Time Only",

 "Sun Date & Time", "Sun Date Only", "Sun Time Only",

 "User Name", "User Email",

 "Justify Line Center", "Justify Line Left", "Justify Line Right", "No Justify",

 "Italic On/Off Toggle",

 "Plain Ampersand",

 NULL
 };

char *PPT_TextSymbols[] = {"&CN", "&CF%1", "&CH%1", "&CC%1",

 "&CP%1", "&CT%1", "&CB%1", "&CX%3", "&CY%3",

 "&CZ%0m",

 "&TX%3", "&TY%3", "&TZ%0m", "&TD%0m",

 "&FN4", "&FS",

 "&PN", "&RN", "&RD", "&RE", "&RT",

 "&SD", "&SE", "&ST",

 "&UN", "&UE",

 "&JC", "&JL", "&JR", "&JN",
 
 "&I",

 "&&",

 NULL
 };

// command strings:
// &C = camera, followed by N, F, H, P, B, X, Y, or Z (name, fov, heading, pitch, bank, longitude, latitude, elevation)
//   &CC -- camera compass heading, &CT -- camera Tilt
// &I = italic mode toggle {NeHe font switch} (defaults to off each scan line)
// &F = frame, followed by N or S (frame number, or SMPTE format {N option is followed by a digit ['1'..'9']})
// &J = justify, followed by C, L, N, or R (center, left, none, right)
// &PN = project name
// &RD = date and time of render; &RE = date of render; &RT = time of render
// &RN = render opts name
// &SD = sun date & time; &SE = sun date only; &ST = sun time only
// &T = target, followed by X, Y, or Z (longitude, latitude, elevation)
// &TD -- distance to target
// &UN = user name
// &UE = user email
//    note: numeric fields are followed by %n where n is '0' to '9' {# digits after decimal}
// CR/LF pair expected to separate lines of text
// use && to print '&', all other chars printed
// ===
// added units interpretation for Camera Elevation, Target Elevation, and Target Distance
// unit suffixes are as found in UsefulUnitTextSuffixes and must follow the digits field
// ie: &CZ%2ft will display the camera altitude in feet to two decimal places
int PostProcText::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double BackLightingPct, dval, PosX, PosY, OutWidth, OutHeight, FScaleX, FScaleY, TargDist, rScaleX, rScaleY;
FILE *font, *rout = NULL, *gout = NULL, *bout = NULL, *aout = NULL;
unsigned char *fontimage = NULL, *rimage = NULL, *gimage = NULL, *bimage = NULL, *aimage = NULL, *eimage = NULL;
class FontImage *FI;
char *ch;
int Success = 1, unitid;
long i = 0, fontcols = 1024, fontrows = 1024, width = 0, lines = 1, maxwidth = 0, outcols = 0, outrows = 0, outsize = 0, x, y;
long fontysize;
bool useLocale = false;
FIJustify justify = WCS_FONTIMAGE_JUSTIFY_CENTER;
Raster TextBlast;
//double ModHSV[3], UnModHSV[3];
VertexDEM Begin, End;
float ZFlt;
PolygonData Poly;
bool addpad = false, italics = false;
unsigned long unitchar;
unsigned char lastch;
char tmp[256], tmp2[256];
char units[8], n;
UBYTE kerning = 2;
char leading = 2;
UBYTE edging = 3;

InternalRasterFont IRF;

if (IRF.FontDataPointer)
	{
	fontrows = IRF.InternalFontHeight;
	fontcols = IRF.InternalFontWidth;
	fontimage = IRF.FontDataPointer;
	} // if
else
	{
	// this was how we originally got the font data in.  
	if (!(font = fopen("C:/CMSS-CMSSI-10.raw", "rb")))
		return 0;

	if (((fontcols % 16) != 0) || ((fontrows % 16) != 0))
		return 0;
	if (!(fontimage = (UBYTE *)AppMem_Alloc(fontcols * fontrows, 0UL)))
		return 0;
	fread(fontimage, 1, fontcols * fontrows, font);
	} // else

fontysize = fontrows / 16;


#ifdef WCS_POSTPROCESSEVENT_CREATE_ZLIBFONT
{
FILE *outfont;
unsigned char *compfont;
uLongf destLen;
// create compressed font resource data

destLen = fontcols * fontrows;
if (compfont = (unsigned char *)AppMem_Alloc(destLen, 0L))
	{
	// modified destLen to indicate output size
	if (compress2(compfont, &destLen, fontimage, fontcols * fontrows, Z_BEST_COMPRESSION) == Z_OK)
		{
		if (outfont = fopen("C:/CMSS-CMSSI-10_gz.raw", "wb"))
			{
			fwrite(compfont, 1, destLen, outfont);
			fclose(outfont);
			outfont = NULL;
			} // if
		} // if
	AppMem_Free(compfont, fontcols * fontrows);
	compfont = NULL;
	} // if


} // temp scope

#endif // WCS_POSTPROCESSEVENT_CREATE_ZLIBFONT


if (!(FI = new FontImage))
	return 0;

FI->SetFont(fontimage, (USHORT)fontcols, (USHORT)fontrows);

kerning = (unsigned char)AnimPar[WCS_TEXTOVERLAY_ANIMPAR_KERNING].CurValue;
leading = (signed char)AnimPar[WCS_TEXTOVERLAY_ANIMPAR_LEADING].CurValue;
edging = (unsigned char)AnimPar[WCS_TEXTOVERLAY_ANIMPAR_OUTLINE].CurValue;

// pass 1 - figure out raster size
ch = &MesgText[0];
while (*ch != 0)
	{
	lastch = (unsigned char)*ch;
	switch (*ch++)
		{
		case 13:	// expecting a CR/LF pair
			if (*ch != 10)
				break;
			ch++;
			if (width > maxwidth)
				maxwidth = width;
			lines++;
			width = 0;
			addpad = false;
			break;
		case '&':	// command string mode
			if (*ch == '&')
				{
				ch++;
				width += FI->CharWidth('&');
				break;
				} // if
			else if ((*ch != 'C') && (*ch != 'F') && (*ch != 'I') && (*ch != 'J') && (*ch != 'P') && (*ch != 'R') && (*ch != 'S') &&(*ch != 'T') && (*ch != 'U'))
				break;
			n = '2';	// default # decimal digits to print
			switch (*ch++)
				{
				case 'C':	// camera
					if ((*ch != 'B') && (*ch != 'F') && (*ch != 'H') && (*ch != 'N') && (*ch != 'P') && (*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'C') && (*ch != 'T'))
						break;
					switch (*ch++)
						{
						case 'B':	// bank
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'F':	// fov
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'H':	// heading
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'N':	// name
							strcpy(tmp, Rend->Cam->GetName());
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'P':	// pitch
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
							if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
								dval = -dval;
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'Y':	// latitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch !=' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'C':	// Compass bearing
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->CamHeading;
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'T':	// tilt (including targetting and pitch)
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->CamPitch;
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						default:
							break;
						} // switch
					break;
				case 'F':	// frame number options
					if ((*ch != 'N') && (*ch != 'S'))
						break;
					switch (*ch++)
						{
						case 'N':	// frame number
							if ((*ch < '1') || (*ch > '9'))
								break;
							n = *ch++;
							sprintf(tmp, "%0*u", n - '0', FrameNum);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'S':	// SMPTE type timecode
							unsigned long frate, h, m, s, f, t;
							frate = (unsigned long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + 0.5);
							if (frate != 0)
								{
								f = (unsigned long)FrameNum % frate;
								t = (unsigned long)FrameNum / frate;	// this frames time in seconds at this frame rate
								h = t / 3600;
								m = (t % 3600) / 60;
								s = t % 60;
								sprintf(tmp, "%02u:%02u:%02u:%02u", h, m, s, f);
								}
							else
								sprintf(tmp, "00:00:00:00");
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						default:
							break;
						}
					break;
				case 'I':	// italics mode toggle
					italics = !italics;
					break;
				case 'J':	// justify
					if ((*ch != 'C') && (*ch != 'L') && (*ch != 'R') && (*ch != 'N'))
						break;
					switch (*ch++)
						{
						case 'C':	// center
							justify = WCS_FONTIMAGE_JUSTIFY_CENTER;
							break;
						case 'L':	// left
							justify = WCS_FONTIMAGE_JUSTIFY_LEFT;
							break;
						case 'R':	// right
							justify = WCS_FONTIMAGE_JUSTIFY_RIGHT;
							break;
						case 'N':	// none
							justify = WCS_FONTIMAGE_JUSTIFY_NONE;
							break;
						default:
							break;
						}
					break;
				case 'P':	// project
					if (*ch != 'N')
						break;
					ch++;
					strcpy(tmp, GlobalApp->MainProj->projectname);
					width += FI->TextWidth(tmp, kerning);
					if (addpad)
						width += kerning + 1;
					addpad = true;
					break;
				case 'R':	// render opts
					time_t ltime;
					struct tm *now;

					if (GlobalApp->MainProj->Prefs.PublicQueryConfigOpt("UseLocale"))
						useLocale = true;

					if ((*ch != 'N') && (*ch != 'D') && (*ch != 'E') && (*ch != 'T'))
						break;
					switch (*ch++)
						{
						case 'N':	// render opts name
							strcpy(tmp, Rend->Opt->GetName());
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'D':	// render date & time
							if (useLocale)
								setlocale( LC_TIME, ".ACP" );
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp, sizeof(tmp), "%c", now);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'E':	// render date only
							if (useLocale)
								setlocale( LC_TIME, ".ACP" );
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp, sizeof(tmp), "%x", now);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'T':	// render time only
							if (useLocale)
								setlocale( LC_TIME, ".ACP" );
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp, sizeof(tmp), "%X", now);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						default:
							break;
						} // switch
					break;
				case 'S':	// sun
					{
					Light *lite = GlobalApp->AppEffects->Lights;
					bool found = false;

					if ((*ch != 'D') && (*ch != 'E') && (*ch != 'T'))
						break;
					// Our "sun" will be the first distant parallel light found
					while (lite && (! found))
						{
						if ((lite->Enabled) && (lite->Distant) && (lite->LightType == WCS_EFFECTS_LIGHTTYPE_PARALLEL))
							found = true;
						} // while
					if (found)
						{
						SunTimeString(lite, tmp, ch);
						width += FI->TextWidth(tmp, kerning);
						if (addpad)
							width += kerning + 1;
						addpad = true;
						} // if
					ch++;
					break;
					} // 'S'
				case 'T':	// target
					if ((*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'D'))
						break;
					switch (*ch++)
						{
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
							if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
								dval = -dval;
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'Y':	// latitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
							sprintf(tmp, "%.*f", n - '0', dval);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != ' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						case 'D':	// distance
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							TargDist = 0;
							if (Rend->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_TARGETED)
								{
								Begin.Lat = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
								Begin.Lon = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
								Begin.Elev = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
								End.Lat = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
								End.Lon = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
								End.Elev = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
								#ifdef WCS_BUILD_VNS
								Rend->DefCoords->DegToCart(&Begin);
								Rend->DefCoords->DegToCart(&End);
								#else // WCS_BUILD_VNS
								Begin.DegToCart(Rend->PlanetRad);
								End.DegToCart(Rend->PlanetRad);
								#endif // WCS_BUILD_VNS
								TargDist = PointDistance(Begin.XYZ, End.XYZ);
								} // if
							dval = TargDist;
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						default:
							break;
						}
					break;
				case 'U':	// user
					if ((*ch != 'N') && (*ch != 'E'))
						break;
					switch (*ch++)
						{
						case 'N':	// Name
							strcpy(tmp, GlobalApp->MainProj->UserName);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = TRUE;
							break;
						case 'E':	// EMail
							strcpy(tmp, GlobalApp->MainProj->UserEmail);
							width += FI->TextWidth(tmp, kerning);
							if (addpad)
								width += kerning + 1;
							addpad = true;
							break;
						default:
							break;
						}
					break;
				} // switch
			break;
		default:
			// it's just a normal printable char
			if (italics)
				lastch += 128;
			width += FI->CharWidth(lastch) + 1 + kerning;
		} // switch
	} // while

if (width > maxwidth)
	maxwidth = width;

// figure out the raster size needed
outcols = maxwidth + 2 + edging * 2;	// one pixel border each side
outrows = lines * fontysize + (lines - 1) * leading + 2 + edging * 2;	// one pixel border each side
outsize = outcols * outrows;

// calculate nominal font scaling factor
FScaleX = 16.0 / (fontcols / 16.0);
FScaleY = 16.0 / (fontrows / 16.0);

// pass 2 - create raster, print stuff
if (!(rimage = (UBYTE *)AppMem_Alloc(outsize, APPMEM_CLEAR)))
	return 0;
if (!(gimage = (UBYTE *)AppMem_Alloc(outsize, APPMEM_CLEAR)))
	return 0;
if (!(bimage = (UBYTE *)AppMem_Alloc(outsize, APPMEM_CLEAR)))
	return 0;
if (!(aimage = (UBYTE *)AppMem_Alloc(outsize, APPMEM_CLEAR)))
	return 0;
if (!(eimage = (UBYTE *)AppMem_Alloc(outsize, APPMEM_CLEAR)))
	return 0;

FI->SetOutput(outcols, outrows, rimage, gimage, bimage, aimage, eimage);

x = y = 0;	// first pixels in output raster to draw to
ch = &MesgText[0];
memset(tmp, 0, sizeof(tmp));
italics = false;
while (*ch != 0)
	{
	switch (*ch)
		{
		case 13:	// expecting a CR/LF pair
			ch++;
			if (*ch != 10)
				break;
			ch++;
			tmp[i] = 0;
			if (tmp[0])
				FI->PrintText(x, y, tmp, 210, 255, 21, justify, WCS_FONTIMAGE_DRAWMODE_NORMAL, leading, kerning, edging, this);
			memset(tmp, 0, sizeof(tmp));
			i = 0;
			x = 0;
			y += fontysize + leading;
			italics = false;
			break;
		case '&':	// command string mode
			ch++;
			if (*ch == '&')
				{
				ch++;
				break;
				} // if
			else if ((*ch != 'C') && (*ch != 'F') && (*ch != 'I') && (*ch != 'J') && (*ch != 'P') && (*ch != 'R') && (*ch != 'S') && (*ch != 'T') && (*ch != 'U'))
				break;
			n = '2';	// default # decimal digits to print
			switch (*ch++)
				{
				case 'C':
					if ((*ch != 'B') && (*ch != 'F') && (*ch != 'H') && (*ch != 'N') && (*ch != 'P') && (*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'C') && (*ch != 'T'))
						break;
					switch (*ch++)
						{
						case 'B':	// bank
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'F':	// fov
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'H':	// heading
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'N':	// name
							strcpy(tmp2, Rend->Cam->GetName());
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'P':	// pitch
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON].GetCurValue();
							if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
								dval = -dval;
							sprintf(tmp2, "%.*f", n - '0', dval);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'Y':	// latitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != ' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV].GetCurValue();
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'C':	// compass heading
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->CamHeading;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'T':	// tilt
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->CamPitch;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						} // switch camera subchar
					break;
				case 'F':	// frame number options
					if ((*ch != 'N') && (*ch != 'S'))
						break;
					switch (*ch++)
						{
						case 'N':	// frame number
							if ((*ch < '1') || (*ch > '9'))
								break;
							n = *ch++;
							sprintf(tmp2, "%0*u", n - '0', FrameNum);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'S':	// SMPTE type timecode
							unsigned long frate, h, m, s, f, t;
							frate = (unsigned long)(Rend->Opt->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue + 0.5);
							if (frate != 0)
								{
								f = (unsigned long)FrameNum % frate;
								t = (unsigned long)FrameNum / frate;	// this frames time in seconds at this frame rate
								h = t / 3600;
								m = (t % 3600) / 60;
								s = t % 60;
								sprintf(tmp2, "%02u:%02u:%02u:%02u", h, m, s, f);
								}
							else
								sprintf(tmp2, "00:00:00:00");
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						default:
							break;
						}
					break;
				case 'I':	// italics mode toggle
					italics = !italics;
					break;
				case 'J':	// justify
					if ((*ch != 'C') && (*ch != 'L') && (*ch != 'R') && (*ch != 'N'))
						break;
					switch (*ch++)
						{
						case 'C':	// center
							justify = WCS_FONTIMAGE_JUSTIFY_CENTER;
							break;
						case 'L':	// left
							justify = WCS_FONTIMAGE_JUSTIFY_LEFT;
							break;
						case 'R':	// right
							justify = WCS_FONTIMAGE_JUSTIFY_RIGHT;
							break;
						case 'N':	// none
							justify = WCS_FONTIMAGE_JUSTIFY_NONE;
							break;
						default:
							break;
						}
					break;
				case 'P':	// project
					if (*ch != 'N')
						break;
					ch++;
					strcpy(tmp2, GlobalApp->MainProj->projectname);
					if (italics)
						NeHeUpper(tmp2);
					strcat(tmp, tmp2);
					i = (long)strlen(tmp);
					break;
				case 'R':	// render opt name
					time_t ltime;
					struct tm *now;

					if ((*ch != 'N') && (*ch != 'D') && (*ch != 'E') && (*ch != 'T'))
						break;
					switch (*ch++)
						{
						case 'N':
							strcpy(tmp2, Rend->Opt->GetName());
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'D':
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp2, sizeof(tmp2), "%c", now);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'E':
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp2, sizeof(tmp2), "%x", now);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'T':
							time(&ltime);
							now = localtime(&ltime);
							strftime(tmp2, sizeof(tmp2), "%X", now);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						default:
							break;
						}
					break;
				case 'S':	// sun
					{
					Light *lite = GlobalApp->AppEffects->Lights;
					bool found = false;

					if ((*ch != 'D') && (*ch != 'E') && (*ch != 'T'))
						break;
					// Our "sun" will be the first distant parallel light found
					while (lite && (! found))
						{
						if ((lite->Enabled) && (lite->Distant) && (lite->LightType == WCS_EFFECTS_LIGHTTYPE_PARALLEL))
							found = true;
						} // while
					if (found)
						{
						SunTimeString(lite, tmp2, ch);
						strcat(tmp, tmp2);
						i = (long)strlen(tmp);
						} // if
					ch++;
					break;
					} // 'S'
				case 'T':	// target options
					if ((*ch != 'X') && (*ch != 'Y') && (*ch != 'Z') && (*ch != 'D'))
						break;
					switch (*ch++)
						{
						case 'X':	// longitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON].GetCurValue();
							if (GlobalApp->MainProj->Prefs.PosLonHemisphere == WCS_PROJPREFS_LONCONVENTION_POSEAST)
								dval = -dval;
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'Y':	// latitude
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								}
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT].GetCurValue();
							sprintf(tmp2, "%.*f", n - '0', dval);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'Z':	// elevation
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != ' ') && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							dval = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV].GetCurValue();
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'D':	// distance
							unitid = WCS_USEFUL_UNIT_METER;
							if (*ch == '%')
								{
								ch++;
								if ((*ch < '0') || (*ch > '9'))
									break;
								n = *ch++;
								// look for unit suffix
								unitchar = 0;
								while ((*ch != 0) && (*ch != 13) && (*ch != '&') && (unitchar < 7))
									{
									units[unitchar++] = *ch++;
									} // while
								units[unitchar] = 0;
								unitid = MatchUnitSuffix(units);
								if (unitid < 0)
									unitid = WCS_USEFUL_UNIT_METER;
								} // if
							dval = TargDist;
							if (unitid != WCS_USEFUL_UNIT_METER)
								dval = ConvertFromMeters(dval, unitid);
							sprintf(tmp2, "%.*f%s", n - '0', dval, GetUnitSuffix(unitid));
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						default:
							break;
						}
					break;
				case 'U':	// user
					if ((*ch != 'N') && (*ch != 'E'))
						break;
					switch (*ch++)
						{
						case 'N':	// Name
							strcpy(tmp2, GlobalApp->MainProj->UserName);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						case 'E':	// EMail
							strcpy(tmp2, GlobalApp->MainProj->UserEmail);
							if (italics)
								NeHeUpper(tmp2);
							strcat(tmp, tmp2);
							i = (long)strlen(tmp);
							break;
						default:
							break;
						}
					break;
				} // switch
			break;
		default:
			// it's just a normal printable char
			if (italics)
				{
				tmp[i++] = *ch + 128;
				ch++;
				}
			else
				tmp[i++] = *ch++;
			break;
		} // switch
	} // while
// see if there's something left to print
if (tmp[0])
	{
	FI->PrintText(x, y, tmp, 210, 255, 21, justify, WCS_FONTIMAGE_DRAWMODE_NORMAL, leading, kerning, edging, this);
	y += fontysize + leading;
	}
if (edging)
	FI->OutlineText(edging);

delete FI;

/*
// Debug file output
FILE *fout;

fout = fopen("D:/FTest_rgb.raw", "wb");
if (fout)
	{
	fwrite(rimage, 1, outsize, fout);
	fwrite(gimage, 1, outsize, fout);
	fwrite(bimage, 1, outsize, fout);
	fclose(fout);
	fout = fopen("D:/FTest_a.raw", "wb");
	if (fout)
		{
		fwrite(aimage, 1, outsize, fout);
		fclose(fout);
		}
	fout = fopen("D:/FTest_e.raw", "wb");
	if (fout)
		{
		fwrite(eimage, 1, outsize, fout);
		fclose(fout);
		}
	}
*/

/*
// debug test-composite in simple 2D
// stolen and adapted from PostProcessEvent::RenderPostProc
if (OptionalBitmaps)
	{
	Bitmap[0] = OptionalBitmaps[0];
	Bitmap[1] = OptionalBitmaps[1];
	Bitmap[2] = OptionalBitmaps[2];
	} // if
else
	{
	Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
	Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
	Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
	} // else

if (! FragBlock && ! (Bitmap[0] && Bitmap[1] && Bitmap[2]))
	return (0);
*/
// setup Raster object with useful data
TextBlast.Rows = outrows;
TextBlast.Cols = outcols;
TextBlast.Red = rimage;
TextBlast.Green = gimage;
TextBlast.Blue = bimage;

PosX = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERX].CurValue * Rend->SetupWidth;
PosY = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERY].CurValue * Rend->SetupHeight;
if (Rend->NumSegments != 1)
	{
	PosY -= Rend->Height * Rend->Segment;
	} // if
else if (Rend->Opt->TilingEnabled)
	{
	if (Rend->Rend->XTiles != 1)
		{
		PosX -= Rend->Rend->SegmentOffsetX;
		} // if
	if (Rend->Rend->YTiles != 1)
		{
		PosY -= Rend->Rend->SegmentOffsetY;
		} // if
	} // if
OutWidth = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_WIDTH].CurValue * outcols;
OutHeight = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_HEIGHT].CurValue * outrows;
ZFlt = (float)AnimPar[WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE].CurValue;
BackLightingPct = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT].CurValue;
Poly.ShadowFlags = ReceiveShadows ? (~0UL): 0;
Poly.ReceivedShadowIntensity = AnimPar[WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY].CurValue;
Poly.ShadowOffset = 0.0;
Poly.Object = GetRAHostRoot();

// Make sure this PPE scales with window size.  Use my QuadView size as base scale.
rScaleX = Rend->SetupWidth / 530.0;
rScaleY = Rend->SetupHeight / 397.0;

Success = PlotText(Rend, Buffers, FragBlock, &Poly, &TextBlast, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum,
	UpdateDiagnostics, PosX, PosY, OutWidth * FScaleX * rScaleX, OutHeight * FScaleY * rScaleY, BackLightingPct, Illuminate && UpdateDiagnostics, ApplyVolumetrics && UpdateDiagnostics, 
	ZFlt, aimage, eimage);

/*
// other variables:
// renderdata, rPixelBlockHeader, polygondata, raster(TextBlast), double PosX(PosX), double PosY(PosY), double Width(outrows), double Height(outcols)
// double backlighting%, int illuminateflag, int ApplyVolumetrics, float ZVal, unsigned char *Alpha(aimage)
// PolygonData: Shadows, ShadowIntensity, ShadowOffset, ObjectID(PostProcess *)

// scan through full rendered image, comping in text where available
// not the most intuitive order to do this, but I'm just recycling existing code
// since this is just for testing
int OffsetX, OffsetY;

// int-only for testing
OffsetX = (int)PosX;
OffsetY = (int)PosY;

for (Row = PixZip = 0; Row < Height; Row ++)
	{
	for (Col = 0; Col < Width; Col ++, PixZip ++)
		{
		// get existing color
		// (doesn't understand fragments, because I don't)
		TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
		TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
		TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
		// evaluate intensity
		if ((Intensity = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].CurValue) > 0.0)
			{
			// texturing intensity unsupported because I didn't steal the full texture setup code from PostProcessEvent::RenderPostProc
			
			if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Eval(Value, &TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						Intensity *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						Intensity *= Value[0];
					} // if
				} // if
			
			} // if

		if (Intensity > 0.0)
			{
			int TBX, TBY, TBZip;
			double AlphaFrac, ScaleFrac = 1.0 / 255.0;
			// combine Text raster data

			// calculate this screen-pixel's position within the Text bitmap
			TBX = Col - OffsetX;
			TBY = Row - OffsetY;

			if (TBX >= 0 && TBX < outcols && TBY >= 0 && TBY < outrows)
				{
				TBZip = TBX + TBY * outcols;
				if (aimage)
					{ // alpha-blend in
					AlphaFrac = aimage[TBZip] * ScaleFrac;
					TexColor[0] = ((rimage[TBZip] * ScaleFrac) * AlphaFrac) + (TexColor[0] * (1.0 - AlphaFrac));
					TexColor[1] = ((gimage[TBZip] * ScaleFrac) * AlphaFrac) + (TexColor[1] * (1.0 - AlphaFrac));
					TexColor[2] = ((bimage[TBZip] * ScaleFrac) * AlphaFrac) + (TexColor[2] * (1.0 - AlphaFrac));
					} // if
				else
					{ // no alpha, blast right in
					TexColor[0] = rimage[TBZip] * ScaleFrac;
					TexColor[1] = gimage[TBZip] * ScaleFrac;
					TexColor[2] = bimage[TBZip] * ScaleFrac;
					} // else
				} //

			if (RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
				{
				RGBtoHSV(UnModHSV, OrigValue);
				RGBtoHSV(ModHSV, TexColor);
				if (AffectRed)
					UnModHSV[0] = ModHSV[0];
				if (AffectGrn)
					UnModHSV[1] = ModHSV[1];
				if (AffectBlu)
					UnModHSV[2] = ModHSV[2];
				HSVtoRGB(UnModHSV, TexColor);
				} // if

			// lerp output values
			InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
			Intensity *= 255.0;
			if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
				Bitmap[0][PixZip] = (unsigned char)(Bitmap[0][PixZip] * InvIntensity + TexColor[0] * Intensity);
			if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
				Bitmap[1][PixZip] = (unsigned char)(Bitmap[1][PixZip] * InvIntensity + TexColor[1] * Intensity);
			if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
				Bitmap[2][PixZip] = (unsigned char)(Bitmap[2][PixZip] * InvIntensity + TexColor[2] * Intensity);
			} // if intensity

		} // for
	if (BWDE)
		{
		if (BWDE)
			{
			if (BWDE->Update(Row + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else if (Master)
		{
		Master->ProcUpdate(Row + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for
*/

AppMem_Free(eimage, outsize);
AppMem_Free(aimage, outsize);
AppMem_Free(bimage, outsize);
AppMem_Free(gimage, outsize);
AppMem_Free(rimage, outsize);

if (!IRF.FontDataPointer)
	{
	AppMem_Free(fontimage, fontcols * fontrows);
	fclose(font);
	} // if

return (Success);

} // PostProcText::RenderPostProc

/*===========================================================================*/

// Callback to look up what color should be used for a particular text pixel
// allowing for textured text!
void PostProcText::FetchPixelColor(long xpos, long ypos, unsigned char &RealR, unsigned char &RealG, unsigned char &RealB)
{
// <<<>>> Do something marvelous here, rewriting RealR, RealG and RealB.

// test code
/*
if (ypos % 2)
	{
	if (xpos % 2)
		{
		RealR = ~RealR;
		RealG = ~RealG;
		RealB = ~RealB;
		}
	}
else
	{
	if (!(xpos % 2))
		{
		RealR = ~RealR;
		RealG = ~RealG;
		RealB = ~RealB;
		}
	}
*/
return;

} // PostProcText::FetchPixelColor



/*===========================================================================*/
/*===========================================================================*/

// Base class for post processes that are not just point-sampled

PostProcNonPoint::PostProcNonPoint(RasterAnimHost *RAHost, int LinesReq)
: PostProcessEvent(RAHost)
{
int LineLoop;

SetLinesReq(LinesReq);
for (LineLoop = 0; LineLoop < NumLines; LineLoop++)
	{
	RedLines[LineLoop] = GreenLines[LineLoop] = BlueLines[LineLoop] = NULL;
	} // for

} // PostProcNonPoint::PostProcNonPoint

/*===========================================================================*/

void PostProcNonPoint::SetLinesReq(int NewLinesReq)
{
NumLines = min(NewLinesReq, WCS_POSTPROCEVENT_NONPOINT_MAXLINES);
MidLine = NumLines / 2;
} // PostProcNonPoint::SetLinesReq

/*===========================================================================*/

void PostProcNonPoint::TransferLineDouble(int DestLineNum, int SourceLineNum, long Width)
{
long TransferZip, XPixel;
unsigned long TransferRGB[3];

TransferZip = Width * SourceLineNum;

for (XPixel = 0; XPixel < Width; XPixel ++, TransferZip ++)
	{
	TransferRGB[0] = Bitmap[0][TransferZip];
	TransferRGB[1] = Bitmap[1][TransferZip];
	TransferRGB[2] = Bitmap[2][TransferZip];
	if (ExponentBuf && ExponentBuf[TransferZip])
		rPixelFragment::ExtractUnclippedExponentialColors(TransferRGB, ExponentBuf[TransferZip]);
	RedLines[DestLineNum][XPixel] = TransferRGB[0] * (1.0 / 255.0);
	GreenLines[DestLineNum][XPixel] = TransferRGB[1] * (1.0 / 255.0);
	BlueLines[DestLineNum][XPixel] = TransferRGB[2] * (1.0 / 255.0);
	} // for

} // PostProcNonPoint::TransferLineDouble

/*===========================================================================*/

int PostProcNonPoint::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
int Success = 1, LineLoop;

for (LineLoop = 0; LineLoop < NumLines; LineLoop++)
	{
	if (!(RedLines[LineLoop]   = (double *)AppMem_Alloc(Width * sizeof(double), APPMEM_CLEAR))) Success = 0;
	if (!(GreenLines[LineLoop] = (double *)AppMem_Alloc(Width * sizeof(double), APPMEM_CLEAR))) Success = 0;
	if (!(BlueLines[LineLoop]  = (double *)AppMem_Alloc(Width * sizeof(double), APPMEM_CLEAR))) Success = 0;
	} // for

NextLine = 0;

if (Success)
	{
	// fill from MidLine to last line (NumLines - 1)
	for (LineLoop = MidLine; LineLoop < NumLines; LineLoop++)
		{
		TransferLineDouble(LineLoop, NextLine, Width);
		NextLine++;
		} // for
	} // if

return(Success);
} // PostProcNonPoint::PrepForPostProc

/*===========================================================================*/

int PostProcNonPoint::AdvanceRow(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
int LineLoop;

// transfer any still-valid lines in the line buffer
for (LineLoop = 0; LineLoop < NumLines - 1; LineLoop++)
	{
	memcpy(RedLines[LineLoop], RedLines[LineLoop + 1], Width * sizeof(double));
	memcpy(GreenLines[LineLoop], GreenLines[LineLoop + 1], Width * sizeof(double));
	memcpy(BlueLines[LineLoop], BlueLines[LineLoop + 1], Width * sizeof(double));
	} // for

// fill last line from raster buffers
if (NextLine < Height)
	{
	TransferLineDouble(NumLines - 1, NextLine, Width);
	} // if
else
	{
	// clear incoming line in case we're dumb enough to read it
	memset(RedLines[NumLines - 1], 0, Width * sizeof(double));
	memset(GreenLines[NumLines - 1], 0, Width * sizeof(double));
	memset(BlueLines[NumLines - 1], 0, Width * sizeof(double));
	} // else
NextLine++;

return(1);
} // PostProcNonPoint::AdvanceRow

/*===========================================================================*/

int PostProcNonPoint::RGBSampleRelative(signed char XOff, signed char YOff, long Width, long Height,
 double &RedSample, double &GrnSample, double &BluSample)
{
int SourceRow, ImgRow, SourceX;

SourceRow = MidLine + YOff;
ImgRow = Row + YOff;
SourceX = Col + XOff;
if ((SourceX < 0) || (SourceX >= Width)) return(0); // sample column out of image range
if ((ImgRow < 0) || (ImgRow >= Height)) return(0); // sample row out of image range
if ((SourceRow < 0) || (SourceRow > NumLines - 1)) return(0);  // sample row out of row buffer range
RedSample = RedLines[SourceRow][SourceX];
GrnSample = GreenLines[SourceRow][SourceX];
BluSample = BlueLines[SourceRow][SourceX];
return(1);
} // PostProcNonPoint::RGBSampleRelative

/*===========================================================================*/

// same as above, but allows non-integer values for offset and returns
// barycentric weighted between-cell samples
int PostProcNonPoint::RGBSampleRelativeFrac(float XOff, float YOff, long Width, long Height,
 double &RedSample, double &GrnSample, double &BluSample)
{
char TopValid = 1, BotValid = 1, LeftValid = 1, RightValid = 1, ValidSamples;
double SampleR = 0.0, SampleG = 0.0, SampleB = 0.0, SourceRow, SourceX, ImgRow;
double XL, XR, YT, YB; // X left portion, right portion, Y top, bottom portions
double Weight; // to prevent multiple calculations each use
int XCL, XCR, YCT, YCB; // X & Y integer coords of left, right, top and bottom cells
int SampleRow;

if (fabs(YOff) > MidLine) return(0); // outside of buffer space in Y direction, sorry.

SourceRow = MidLine + YOff;
ImgRow = Row + YOff;
SourceX = Col + XOff;

XCL = (int)SourceX; // ANSI sez int cast is by truncation
XCR = (int)(SourceX + .5); // round up to higher number
YCT = (int)ImgRow; // ANSI sez int cast is by truncation
YCB = (int)(ImgRow + .5); // round up to higher number

// proportional distance between cells
XL = fabs(SourceX - (int)SourceX); // fractional distance beyond integer position
XR = 1.0 - XL; // the rest of the distance to next integer position
YT = fabs(SourceRow - (int)SourceRow);  // fractional distance beyond integer position
YB = 1.0 - YT; // the rest of the distance to next integer position

// determine which samples are valid (within original image region)
if (XCL < 0) LeftValid = 0;
if (XCR < 0) RightValid = 0;
if (XCR > Width - 1) RightValid = 0;
if (XCL > Width - 1) LeftValid = 0;

if (YCT < 0) TopValid = 0;
if (YCB < 0) BotValid = 0;
if (YCB > Height - 1) BotValid = 0;
if (YCT > Height - 1) TopValid = 0;

ValidSamples = (TopValid & LeftValid) + (TopValid & RightValid) + (BotValid & LeftValid) + (BotValid & RightValid);

if (ValidSamples)
	{
	if (ValidSamples == 4)
		{
		// UL
		Weight = XL * YT;
		SampleRow = MidLine + (YCT - Row);
		SampleR += RedLines[SampleRow][XCL] * Weight;
		SampleG += GreenLines[SampleRow][XCL] * Weight;
		SampleB += BlueLines[SampleRow][XCL] * Weight;

		// UR
		Weight = XR * YT;
		//SampleRow = MidLine + (YCT - Row); // sample row still valid from previous
		SampleR += RedLines[SampleRow][XCR] * Weight;
		SampleG += GreenLines[SampleRow][XCR] * Weight;
		SampleB += BlueLines[SampleRow][XCR] * Weight;

		// LL
		Weight = XL * YB;
		SampleRow = MidLine + (YCB - Row);
		SampleR += RedLines[SampleRow][XCL] * Weight;
		SampleG += GreenLines[SampleRow][XCL] * Weight;
		SampleB += BlueLines[SampleRow][XCL] * Weight;

		// LR
		Weight = XR * YB;
		//SampleRow = MidLine + (YCB - Row); // sample row still valid from previous
		SampleR += RedLines[SampleRow][XCR] * Weight;
		SampleG += GreenLines[SampleRow][XCR] * Weight;
		SampleB += BlueLines[SampleRow][XCR] * Weight;

		// no normalization of SampleR/G/B should be necessary
		// due to barycentric weights already multiplied into each sample
		RedSample = SampleR;
		GrnSample = SampleG;
		BluSample = SampleB;
		return(1);
		} // if
	else // must be two, as 1 or 3 seem geometrically impossible
		{
		if (LeftValid && RightValid && TopValid)
			{ // top pair
			// UL
			Weight = XL;
			SampleRow = MidLine + (YCT - Row);
			SampleR += RedLines[SampleRow][XCL] * Weight;
			SampleG += GreenLines[SampleRow][XCL] * Weight;
			SampleB += BlueLines[SampleRow][XCL] * Weight;

			// UR
			Weight = XR;
			//SampleRow = MidLine + (YCT - Row); // sample row still valid from previous
			SampleR += RedLines[SampleRow][XCR] * Weight;
			SampleG += GreenLines[SampleRow][XCR] * Weight;
			SampleB += BlueLines[SampleRow][XCR] * Weight;

			// no normalization of SampleR/G/B should be necessary
			// due to lerp weights already multiplied into each sample
			RedSample = SampleR;
			GrnSample = SampleG;
			BluSample = SampleB;
			return(1);
			} // if
		else if (LeftValid && RightValid && BotValid)
			{ // bottom pair
			// LL
			Weight = XL;
			SampleRow = MidLine + (YCB - Row);
			SampleR += RedLines[SampleRow][XCL] * Weight;
			SampleG += GreenLines[SampleRow][XCL] * Weight;
			SampleB += BlueLines[SampleRow][XCL] * Weight;

			// LR
			Weight = XR;
			//SampleRow = MidLine + (YCB - Row); // sample row still valid from previous
			SampleR += RedLines[SampleRow][XCR] * Weight;
			SampleG += GreenLines[SampleRow][XCR] * Weight;
			SampleB += BlueLines[SampleRow][XCR] * Weight;

			// no normalization of SampleR/G/B should be necessary
			// due to lerp weights already multiplied into each sample
			RedSample = SampleR;
			GrnSample = SampleG;
			BluSample = SampleB;
			return(1);
			} // else if
		else if (TopValid && BotValid && LeftValid)
			{ // left pair
			// UL
			Weight = XL * 0.5;
			SampleRow = MidLine + (YCT - Row);
			SampleR += RedLines[SampleRow][XCL] * Weight;
			SampleG += GreenLines[SampleRow][XCL] * Weight;
			SampleB += BlueLines[SampleRow][XCL] * Weight;

			// LL
			//Weight = XL; // weight still valid from previous
			SampleRow = MidLine + (YCB - Row);
			SampleR += RedLines[SampleRow][XCL] * Weight;
			SampleG += GreenLines[SampleRow][XCL] * Weight;
			SampleB += BlueLines[SampleRow][XCL] * Weight;

			// no normalization of SampleR/G/B should be necessary
			// due to lerp weights already multiplied into each sample
			RedSample = SampleR;
			GrnSample = SampleG;
			BluSample = SampleB;
			return(1);
			} // else if
		else if (TopValid && BotValid && RightValid)
			{ // right pair
			// UR
			Weight = XR * 0.5;
			SampleRow = MidLine + (YCT - Row);
			SampleR += RedLines[SampleRow][XCR] * Weight;
			SampleG += GreenLines[SampleRow][XCR] * Weight;
			SampleB += BlueLines[SampleRow][XCR] * Weight;

			// LR
			//Weight = XR; weight still valid from previous
			SampleRow = MidLine + (YCB - Row);
			SampleR += RedLines[SampleRow][XCR] * Weight;
			SampleG += GreenLines[SampleRow][XCR] * Weight;
			SampleB += BlueLines[SampleRow][XCR] * Weight;

			// no normalization of SampleR/G/B should be necessary
			// due to lerp weights already multiplied into each sample
			RedSample = SampleR;
			GrnSample = SampleG;
			BluSample = SampleB;
			return(1);
			} // else if
		} // else
	} //

// RedSample, GrnSample, BluSample untouched
return(0);
} // PostProcNonPoint::RGBSampleRelativeFrac

/*===========================================================================*/

int PostProcNonPoint::CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
int LineLoop;

for (LineLoop = 0; LineLoop < NumLines; LineLoop++)
	{
	if (RedLines[LineLoop])   AppMem_Free(RedLines[LineLoop],   Width * sizeof(double));
	RedLines[LineLoop]   = NULL;
	if (GreenLines[LineLoop]) AppMem_Free(GreenLines[LineLoop], Width * sizeof(double));
	GreenLines[LineLoop] = NULL;
	if (BlueLines[LineLoop])  AppMem_Free(BlueLines[LineLoop],  Width * sizeof(double));
	BlueLines[LineLoop]  = NULL;
	} // for
return(1);
} // PostProcNonPoint::CleanupPostProc


/*===========================================================================*/
/*===========================================================================*/

PostProcChromax::PostProcChromax(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{

strcpy(Name, PostProcEventNames[GetType()]);

} // PostProcChromax::PostProcChromax

/*===========================================================================*/

int PostProcChromax::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double ColorRGB[3], ColorConvertRGB[3], ColorConvertHSV[3];
double AdjustAmount, AdjustAdd, OrigValFrac, OrigSatFrac;
unsigned char AdjustByte;

// Fetch current values
ColorRGB[0] = TexColor[0];
ColorRGB[1] = TexColor[1];
ColorRGB[2] = TexColor[2];

// Convert to HSV
RGBtoHSV(ColorConvertHSV, ColorRGB);

// Determine adjustment factors from original Saturation and Value
OrigValFrac = ColorConvertHSV[2] / 100.0;
OrigSatFrac = ColorConvertHSV[1] / 100.0;

// Calculate Saturation gain
AdjustAmount = ((.5 - fabs(.5 - OrigSatFrac)) * 2); // AdjustAmount=0...1
if (AdjustAmount > 1.0) AdjustAmount = 1.0; // clamp
AdjustByte = (unsigned char)(254 * AdjustAmount); // 255 entries numbered 0...254
AdjustAdd = 35 * GlobalApp->MathTables->CurveTab.Lookup(AdjustByte);
ColorConvertHSV[1] = ColorConvertHSV[1] + AdjustAdd;

// Calculate Value gain
AdjustAmount = ((.5 - fabs(.5 - OrigValFrac)) * 2); // AdjustAmount=0...1
if (AdjustAmount > 1.0) AdjustAmount = 1.0; // clamp
AdjustByte = (unsigned char)(254 * AdjustAmount); // 255 entries numbered 0...254
AdjustAdd = 20 * GlobalApp->MathTables->CurveTab.Lookup(AdjustByte);
ColorConvertHSV[2] = ColorConvertHSV[2] + AdjustAdd;

// Safety limits
if (ColorConvertHSV[1] > 100) ColorConvertHSV[1] = 100;
if (ColorConvertHSV[2] > 100) ColorConvertHSV[2] = 100;

// Convert back to RGB
HSVtoRGB(ColorConvertHSV, ColorConvertRGB);
TexColor[0] = ColorConvertRGB[0];
TexColor[1] = ColorConvertRGB[1];
TexColor[2] = ColorConvertRGB[2];

return(1);
} // PostProcChromax::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcMedian::PostProcMedian(RasterAnimHost *RAHost)
: PostProcNonPoint(RAHost, 3) // 3x3 filter size
{

DoFive = 1;
strcpy(Name, PostProcEventNames[GetType()]);

} // PostProcMedian::PostProcMedian

/*===========================================================================*/

int PostProcMedian::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

if (DoFive)
	{
	SetLinesReq(5); // on base class
	} // if

// pass to superclass for more handling
return(PostProcNonPoint::PrepForPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics));
} // PostProcMedian::PrepForPostProc

/*===========================================================================*/

// qsort expects C-style functions
extern "C" {
int PostProcMedianQSCompare(const void *arg1, const void *arg2)
{
MedianSample *MS1, *MS2;
MS1 = (MedianSample *)arg1;
MS2 = (MedianSample *)arg2;

if (MS1->MSum < MS2->MSum) return(-1);
if (MS1->MSum > MS2->MSum) return(1);
return(0);
} // PostProcMedianQSCompare

} // extern C

/*===========================================================================*/

int PostProcMedian::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
struct MedianSample SampleSet[WCS_POSTPROCEVENT_MEDIAN_MAXDIM * WCS_POSTPROCEVENT_MEDIAN_MAXDIM];
int Samples = 0, MedSample, SampleVal;

// fetch up to 9 or 25 samples into SampleSet
if (DoFive)
	{ // 5x5 kernel
	// center 3x3 first, for simplicity
	Samples += RGBSampleRelative(-1, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);

	// outer ring
	// top row, left to right
	Samples += RGBSampleRelative(-2, -2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1, -2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0, -2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1, -2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 2, -2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);

	// bottom row, left to right
	Samples += RGBSampleRelative(-2,  2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1,  2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0,  2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1,  2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 2,  2, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);

	// left edge, top to bottom
	Samples += RGBSampleRelative(-2, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-2,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-2,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);

	// right edge, top to bottom
	Samples += RGBSampleRelative( 2, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 2,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 2,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	} // if
else
	{ // 3x3 kernel
	Samples += RGBSampleRelative(-1, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1, -1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1,  0, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative(-1,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 0,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	Samples += RGBSampleRelative( 1,  1, Width, Height, SampleSet[Samples].MRed, SampleSet[Samples].MGrn, SampleSet[Samples].MBlu);
	} // else

for (SampleVal = 0; SampleVal < Samples; SampleVal++)
	{
	SampleSet[SampleVal].MSum = SampleSet[SampleVal].MRed + SampleSet[SampleVal].MGrn + SampleSet[SampleVal].MBlu;
	} // for

if (Samples)
	{
	MedSample = ((Samples + 1) / 2) - 1; // (9:5, 8:4, 7:4, 6:3, 5:3, 4:2, 3:2, 2:1, 1:1) - 1 to get to 0-based index
	// Sort samples using Quicksort
	qsort((void *)SampleSet, (size_t)Samples, sizeof(MedianSample), PostProcMedianQSCompare);

	TexColor[0] = SampleSet[MedSample].MRed;
	TexColor[1] = SampleSet[MedSample].MGrn;
	TexColor[2] = SampleSet[MedSample].MBlu;
	} // if

return(1);
} // PostProcMedian::EvalPostProcSample

/*===========================================================================*/

ULONG PostProcMedian::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCMEDIAN_DOFIVE:
						{
						BytesRead = ReadBlock(ffile, (char *)&DoFive, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcMedian::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcMedian::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCMEDIAN_DOFIVE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DoFive)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcMedian::SaveSpecificData

/*===========================================================================*/

void PostProcMedian::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcMedian *CopyTo, *CopyFrom;

CopyTo = (PostProcMedian *)CopyToPP;
CopyFrom = (PostProcMedian *)CopyFromPP;

CopyTo->DoFive = CopyFrom->DoFive;

} // PostProcMedian::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

PostProcBoxFilter::PostProcBoxFilter(RasterAnimHost *RAHost)
: PostProcNonPoint(RAHost, 3) // 3x3 filter size
{

strcpy(Name, PostProcEventNames[GetType()]);

} // PostProcBoxFilter::PostProcBoxFilter

/*===========================================================================*/

int PostProcBoxFilter::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double NewR = 0.0, NewG = 0.0, NewB = 0.0, SampleR, SampleG, SampleB;
double Samples = 0.0;

if (RGBSampleRelative(-1, -1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 0, -1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 1, -1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative(-1,  0, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 0,  0, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 1,  0, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative(-1,  1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 0,  1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (RGBSampleRelative( 1,  1, Width, Height, SampleR, SampleG, SampleB)) {Samples++; NewR+=SampleR; NewG+=SampleG; NewB+=SampleB;}
if (Samples > 0)
	{
	NewR /= Samples;
	NewG /= Samples;
	NewB /= Samples;
	} // if

TexColor[0] = NewR;
TexColor[1] = NewG;
TexColor[2] = NewB;

return(1);
} // PostProcBoxFilter::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

// original integer matrices converted to float weights
// 3x3, 5x5, 9x9 from Image Processing Handbook (Russ)
// 7x7 from computer vision course at www.isbe.man.ac.uk
float DOF_Gauss_3x3[] = {
	0.031250f, 0.125000f, 0.031250f,
	0.125000f, 0.375000f, 0.125000f,
	0.031250f, 0.125000f, 0.031250f
	};

float DOF_Gauss_5x5[] = {
	0.008264f, 0.016529f, 0.024793f, 0.016529f, 0.008264f,
	0.016529f, 0.057851f, 0.090909f, 0.057851f, 0.016529f,
	0.024793f, 0.090909f, 0.140496f, 0.090909f, 0.024793f,
	0.016529f, 0.057851f, 0.090909f, 0.057851f, 0.016529f,
	0.008264f, 0.016529f, 0.024793f, 0.016529f, 0.008264f
	};

float DOF_Gauss_7x7[] = {
	0.000897f, 0.003587f, 0.006278f, 0.008969f, 0.006278f, 0.003587f, 0.000897f,
	0.003587f, 0.010762f, 0.023318f, 0.029596f, 0.023318f, 0.010762f, 0.003587f,
	0.006278f, 0.023318f, 0.049327f, 0.063677f, 0.049327f, 0.023318f, 0.006278f,
	0.008969f, 0.029596f, 0.063677f, 0.081614f, 0.063677f, 0.029596f, 0.008969f,
	0.006278f, 0.023318f, 0.049327f, 0.063677f, 0.049327f, 0.023318f, 0.006278f,
	0.003587f, 0.010762f, 0.023318f, 0.029596f, 0.023318f, 0.010762f, 0.003587f,
	0.000897f, 0.003587f, 0.006278f, 0.008969f, 0.006278f, 0.003587f, 0.000897f
	};


float DOF_Gauss_9x9[] = {
	0.000000f, 0.000000f, 0.003906f, 0.003906f, 0.003906f, 0.003906f, 0.003906f, 0.000000f, 0.000000f,
	0.000000f, 0.003906f, 0.007813f, 0.011719f, 0.011719f, 0.011719f, 0.007813f, 0.003906f, 0.000000f,
	0.003906f, 0.007813f, 0.011719f, 0.023438f, 0.027344f, 0.023438f, 0.011719f, 0.007813f, 0.003906f,
	0.003906f, 0.011719f, 0.023438f, 0.035156f, 0.042969f, 0.035156f, 0.023438f, 0.011719f, 0.003906f,
	0.003906f, 0.011719f, 0.027344f, 0.042969f, 0.046875f, 0.042969f, 0.027344f, 0.011719f, 0.003906f,
	0.003906f, 0.011719f, 0.023438f, 0.035156f, 0.042969f, 0.035156f, 0.023438f, 0.011719f, 0.003906f,
	0.003906f, 0.007813f, 0.011719f, 0.023438f, 0.027344f, 0.023438f, 0.011719f, 0.007813f, 0.003906f,
	0.000000f, 0.003906f, 0.007813f, 0.011719f, 0.011719f, 0.011719f, 0.007813f, 0.003906f, 0.000000f,
	0.000000f, 0.000000f, 0.003906f, 0.003906f, 0.003906f, 0.003906f, 0.003906f, 0.000000f, 0.000000f
	};

PostProcDOFFilter::PostProcDOFFilter(RasterAnimHost *RAHost)
: PostProcNonPoint(RAHost, 9) // 9x9 filter size
{
double EffectDefault[2] = {15.0, 3.0};
double RangeDefaults[2][3] = {FLT_MAX, 0.001, 5,   4.0, 1.0, 1};

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetDefaults(this, (char)1, EffectDefault[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetRangeDefaults(RangeDefaults[1]);

} // PostProcDOFFilter::PostProcDOFFilter

/*===========================================================================*/


int PostProcDOFFilter::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps,
	long FrameNum, int UpdateDiagnostics)
{
double NewR = 0.0, NewG = 0.0, NewB = 0.0, SampleR, SampleG, SampleB;
float Blend, tmp, ZSample;
float Focus = (float)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].GetCurValue();
float *weight;
char Radius = (char)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].GetCurValue();
char XPos, YPos;

ZSample = ZBuf[(Row * Width) + Col];
tmp = (float)fabs(ZSample - Focus);
// output = original * (1.0f - blend) + kernalized * blend;
if (tmp >= Focus)
	Blend = 1.0f;
else
	Blend = tmp / Focus;	// more in focus the closer we are to the focal point

if (Radius == 1)
	weight = DOF_Gauss_3x3;
else if (Radius == 2)
	weight = DOF_Gauss_5x5;
else if (Radius == 3)
	weight = DOF_Gauss_7x7;
else
	weight = DOF_Gauss_9x9;

for (YPos = -Radius; YPos <= Radius; YPos++)
	{
	for (XPos = -Radius; XPos <= Radius; XPos++, weight++)
		{
		if (RGBSampleRelative(XPos, YPos, Width, Height, SampleR, SampleG, SampleB))
			{
			NewR += SampleR * (*weight);
			NewG += SampleG * (*weight);
			NewB += SampleB * (*weight);
			} // if sampled
		} // XPos
	} // YPos

if (Blend > 0.0f)
	{
	TexColor[0] = TexColor[0] * (1.0f - Blend) + NewR * Blend;	// blend of 0.0 means no blurring, blend of 1.0 means full blurring
	TexColor[1] = TexColor[1] * (1.0f - Blend) + NewG * Blend;
	TexColor[2] = TexColor[2] * (1.0f - Blend) + NewB * Blend;
	}

return(1);

} // PostProcDOFFilter::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcDistort::PostProcDistort(RasterAnimHost *RAHost)
: PostProcNonPoint(RAHost, 5) // 5x5 filter size
{
double EffectDefault[2] = {15.0, 15.0};
double RangeDefaults[2][3] = {15.0, 0.0, .1,   15.0, 0.0, .1};
RootTexture *Root;
Texture *Tex;

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetDefaults(this, (char)1, EffectDefault[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetRangeDefaults(RangeDefaults[1]);


if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE1))
	{
	// set texture type to terrain param, luminosity input
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_FRACTALNOISE))
		{
		Tex->SetMiscDefaults();
		Tex->TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetCurValue(0, 5298.0);
		} // if
	} // if

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE2))
	{
	// set texture type to terrain param, luminosity input
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_FRACTALNOISE))
		{
		Tex->SetMiscDefaults();
		Tex->TexParam[WCS_TEXTURE_FRACTALNOISE_INPUTSEED].SetCurValue(0, 3141.0);
		} // if
	} // if


} // PostProcDistort::PostProcDistort

/*===========================================================================*/

int PostProcDistort::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double HalfMaxY;
int MaxY;

// calculate how many total rows we really need to buffer
HalfMaxY = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].GetCurValue() * .5;
HalfMaxY = (int)(HalfMaxY + .5); // number of rows on either side of main line rounded up to next whole
MaxY = (int)(HalfMaxY * 2.0); // numer of rows, excluding main line
MaxY++; // add in main line

SetLinesReq(MaxY); // on base class

// pass to superclass for more handling
return(PostProcNonPoint::PrepForPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics));
} // PostProcDistort::PrepForPostProc

/*===========================================================================*/

int PostProcDistort::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double DistortX, DistortY, TempValue[3], NewR = 0.0, NewG = 0.0, NewB = 0.0;

DistortX = (float)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].GetCurValue();
DistortY = (float)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].GetCurValue();

// evaluate X value-texture
if (DistortX != 0.0)
	{
	if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Enabled)
		{
		TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
		if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Eval(TempValue, &TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// TempValue[0] has already been diminished by the texture's opacity
				TempValue[0] -= TexOpacity * .5; 
				DistortX *= (1.0 - TexOpacity + TempValue[0]);
				} // if
			else
				{
				TempValue[0] -= .5; 
				DistortX *= TempValue[0];
				} // else
			} // if
		else
			DistortX *= .5;
		} // if
	else
		DistortX *= .5;
	} // if
// evaluate Y value-texture
if (DistortY > 0.0)
	{
	if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Enabled)
		{
		TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
		if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Eval(TempValue, &TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				// TempValue[0] has already been diminished by the texture's opacity
				TempValue[0] -= TexOpacity * .5; 
				DistortY *= (1.0 - TexOpacity + TempValue[0]);
				} // if
			else
				{
				TempValue[0] -= .5; 
				DistortY *= TempValue[0];
				} // else
			} // if
		else
			DistortY *= .5;
		} // if
	else
		DistortY *= .5;
	} // if

if (RGBSampleRelativeFrac((float)DistortX, (float)DistortY, Width, Height, NewR, NewG, NewB))
	{
	TexColor[0] = NewR;
	TexColor[1] = NewG;
	TexColor[2] = NewB;
	} // if
else
	{
	return(1);
	} // else

return(1);

} // PostProcDistort::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcEdgeInk::PostProcEdgeInk(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{

strcpy(Name, PostProcEventNames[GetType()]);

// param defaults
EdgeInkParams[0].Distance  = 1000.0;
EdgeInkParams[0].DistDiff  = 50.0;
EdgeInkParams[0].NormDiff  = 1.0;
EdgeInkParams[0].InkWeight = 1.0;
EdgeInkParams[0].InkColor  = 1.0;

EdgeInkParams[1].Distance  = 10000.0;
EdgeInkParams[1].DistDiff  = 1000.0;
EdgeInkParams[1].NormDiff  = 1.0;
EdgeInkParams[1].InkWeight = 1.0;
EdgeInkParams[1].InkColor  = 0.0;

EdgeInkParams[2].Distance  = 20000.0;
EdgeInkParams[2].DistDiff  = 1000000.0;
EdgeInkParams[2].NormDiff  = 1.0;
EdgeInkParams[2].InkWeight = 0.0;
EdgeInkParams[2].InkColor  = 0.0;

} // PostProcEdgeInk::PostProcEdgeInk

/*===========================================================================*/

int PostProcEdgeInk::AddRenderBuffers(RenderOpt *Opt, BufferNode *Buffers)
{

// this will warn user if attempt to allocate an unrecognized buffer and renderer will bail
if (! Buffers->AddBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT))
	return (0);
if (! Buffers->AddBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT))
	return (0);
if (! Buffers->AddBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT))
	return (0);

return (PostProcessEvent::AddRenderBuffers(Opt, Buffers));

} // PostProcEdgeInk::AddRenderBuffers

/*===========================================================================*/

void PostProcEdgeInk::LookupInkParams(double CellDist, double &DistDiff, double &NormDiff, double &InkWeight, double &InkColor)
{
double DistFactor;

if (CellDist < EdgeInkParams[0].Distance)
	{
	// NEAR
	InkWeight   = EdgeInkParams[0].InkWeight;
	InkColor    = EdgeInkParams[0].InkColor;
	DistDiff	= EdgeInkParams[0].DistDiff;
	NormDiff	= EdgeInkParams[0].NormDiff;
	return;
	} // if
else if (CellDist > EdgeInkParams[2].Distance)
	{
	// FAR
	InkWeight   = EdgeInkParams[2].InkWeight;
	InkColor    = EdgeInkParams[2].InkColor;
	DistDiff	= EdgeInkParams[2].DistDiff;
	NormDiff	= EdgeInkParams[2].NormDiff;
	return;
	} // else if
else if (CellDist > EdgeInkParams[0].Distance)
	{
	if (CellDist > EdgeInkParams[1].Distance)
		{
		// MID to FAR
		DistFactor  = (CellDist - EdgeInkParams[1].Distance) / (EdgeInkParams[2].Distance - EdgeInkParams[1].Distance);
		InkWeight   = flerp(DistFactor, EdgeInkParams[1].InkWeight, EdgeInkParams[2].InkWeight);
		InkColor    = flerp(DistFactor, EdgeInkParams[1].InkColor,  EdgeInkParams[2].InkColor);
		DistDiff    = flerp(DistFactor, EdgeInkParams[1].DistDiff,  EdgeInkParams[2].DistDiff);
		NormDiff    = flerp(DistFactor, EdgeInkParams[1].NormDiff,  EdgeInkParams[2].NormDiff);
		return;
		} // if
	else
		{
		// NEAR to MID
		DistFactor  = (CellDist - EdgeInkParams[0].Distance) / (EdgeInkParams[1].Distance - EdgeInkParams[0].Distance);
		InkWeight   = flerp(DistFactor, EdgeInkParams[0].InkWeight, EdgeInkParams[1].InkWeight);
		InkColor    = flerp(DistFactor, EdgeInkParams[0].InkColor,  EdgeInkParams[1].InkColor);
		DistDiff    = flerp(DistFactor, EdgeInkParams[0].DistDiff,  EdgeInkParams[1].DistDiff);
		NormDiff    = flerp(DistFactor, EdgeInkParams[0].NormDiff,  EdgeInkParams[1].NormDiff);
		return;
		} // else
	} // else if

} // LookupInkParams

/*===========================================================================*/

int PostProcEdgeInk::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double ColorRGB[3], ColorConvertRGB[3], ColorConvertHSV[3];
double CellDist, ZDif, MaxZDif, MaxNormDif, EdgeInkFactor, NormInkFactor, TotInkFactor, TempZ, AdjustAdd, NewSat;
double DistDiff, NormDiff, InkWeight, InkColor;
Point3d MySample;
int NormalZip;
char NormalsOk = 0;
// per-sample variables

CellDist = TexData.ZDist;

MaxNormDif = MaxZDif = 0.0;
NormInkFactor = EdgeInkFactor = 0.0;

if (NormalBuf[0] && NormalBuf[1] && NormalBuf[2]) NormalsOk = 1;

if (ZSampleRelative(-1, -1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative( 0, -1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative( 1, -1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative(-1,  0, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative( 1,  0, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative(-1,  1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative( 0,  1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if
if (ZSampleRelative( 1,  1, Width, Height, TempZ))
	{
	ZDif = TempZ - CellDist;
	if (ZDif > MaxZDif) MaxZDif = ZDif;
	} // if

if (NormalsOk)
	{
	NormalZip = (Row * Width) + Col;
	MySample[0] = NormalBuf[0][NormalZip];
	MySample[1] = NormalBuf[1][NormalZip];
	MySample[2] = NormalBuf[2][NormalZip];
	if (NormDiffSampleRelative(-1, -1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative( 0, -1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative( 1, -1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative(-1,  0, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative( 1,  0, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative(-1,  1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative( 0,  1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	if (NormDiffSampleRelative( 1,  1, Width, Height, NormDiff, MySample))
		{
		if (NormDiff > MaxNormDif) MaxNormDif = NormDiff;
		} // if
	} // if

NormDiff = 0.0; // clear it
LookupInkParams(CellDist, DistDiff, NormDiff, InkWeight, InkColor);

// edge-z inking
if (MaxZDif > DistDiff)
	{
	EdgeInkFactor = 1.0;
	} // if
else if (MaxZDif <= 0.)
	{
	EdgeInkFactor = 0.0;
	} // else if
else
	{
	EdgeInkFactor = MaxZDif / DistDiff;
	} // else


// normal-based inking
if (NormalsOk)
	{
	if (MaxNormDif > NormDiff)
		{
		NormInkFactor = 1.0;
		} // if
	else if (MaxNormDif <= 0.)
		{
		NormInkFactor = 0.0;
		} // else if
	else
		{
		NormInkFactor = MaxNormDif / NormDiff;
		} // else
	} // if


// Fetch current values
ColorRGB[0] = TexColor[0];
ColorRGB[1] = TexColor[1];
ColorRGB[2] = TexColor[2];

// Convert to HSV
RGBtoHSV(ColorConvertHSV, ColorRGB);

// Adjust highest inking amount by Ink Weight
TotInkFactor = max(EdgeInkFactor, NormInkFactor) * InkWeight;

// Adjust Ink Color
NewSat = (InkColor * ColorConvertHSV[1]);

// Ink it
AdjustAdd = 1.0 - TotInkFactor;
ColorConvertHSV[2] = flerp(AdjustAdd, 0.0, ColorConvertHSV[2]);
ColorConvertHSV[1] = flerp(AdjustAdd, NewSat, ColorConvertHSV[1]);
if (ColorConvertHSV[2] > 100) ColorConvertHSV[2] = 100;

// Convert back to RGB
HSVtoRGB(ColorConvertHSV, ColorConvertRGB);
TexColor[0] = ColorConvertRGB[0];
TexColor[1] = ColorConvertRGB[1];
TexColor[2] = ColorConvertRGB[2];

return(1);
} // PostProcEdgeInk::EvalPostProcSample

/*===========================================================================*/

ULONG PostProcEdgeInk::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long EdgeInkNum = 0;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCEDGEINK_DISTDIFF:
						{
						BytesRead = ReadBlock(ffile, (char *)&EdgeInkParams[EdgeInkNum].DistDiff, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEDGEINK_NORMDIFF:
						{
						BytesRead = ReadBlock(ffile, (char *)&EdgeInkParams[EdgeInkNum].NormDiff, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEDGEINK_INKWEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&EdgeInkParams[EdgeInkNum].InkWeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEDGEINK_INKCOLOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&EdgeInkParams[EdgeInkNum].InkColor, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCEDGEINK_DISTANCE:
						{
						BytesRead = ReadBlock(ffile, (char *)&EdgeInkParams[EdgeInkNum].Distance, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						EdgeInkNum ++;
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcEdgeInk::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcEdgeInk::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;

for (Ct = 0; Ct < 3; ++Ct)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEDGEINK_DISTDIFF, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&EdgeInkParams[Ct].DistDiff)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEDGEINK_NORMDIFF, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&EdgeInkParams[Ct].NormDiff)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEDGEINK_INKWEIGHT, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&EdgeInkParams[Ct].InkWeight)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEDGEINK_INKCOLOR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&EdgeInkParams[Ct].InkColor)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCEDGEINK_DISTANCE, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&EdgeInkParams[Ct].Distance)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcEdgeInk::SaveSpecificData

/*===========================================================================*/

void PostProcEdgeInk::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcEdgeInk *CopyTo, *CopyFrom;
long Ct;

CopyTo = (PostProcEdgeInk *)CopyToPP;
CopyFrom = (PostProcEdgeInk *)CopyFromPP;

for (Ct = 0; Ct < 3; ++Ct)
	{
	CopyTo->EdgeInkParams[Ct].DistDiff = CopyFrom->EdgeInkParams[Ct].DistDiff;
	CopyTo->EdgeInkParams[Ct].NormDiff = CopyFrom->EdgeInkParams[Ct].NormDiff;
	CopyTo->EdgeInkParams[Ct].InkWeight = CopyFrom->EdgeInkParams[Ct].InkWeight;
	CopyTo->EdgeInkParams[Ct].InkColor = CopyFrom->EdgeInkParams[Ct].InkColor;
	CopyTo->EdgeInkParams[Ct].Distance = CopyFrom->EdgeInkParams[Ct].Distance;
	} // for

} // PostProcEdgeInk::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

PostProcPosterize::PostProcPosterize(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
double RangeDefaults[3] = {256.0, 2.0, .5};

strcpy(Name, PostProcEventNames[GetType()]);
Levels = 4;

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetValue(4.0);

LevelWidth = 1.0 / 4.0;

} // PostProcPosterize::PostProcPosterize

/*===========================================================================*/

int PostProcPosterize::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

Levels = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].CurValue;
if (Levels < 2)
	Levels = 2;
LevelWidth = 1.0 / Levels;
LevelsMinusOne = Levels - 1;
InvLevelsMinusOne = 1.0 / LevelsMinusOne;

return (1);

} // PostProcPosterize::PrepForPostProc

/*===========================================================================*/

int PostProcPosterize::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double IntColor;
long Ct;

for (Ct = 0; Ct < 3; ++Ct)
	{
	IntColor = (long)(TexColor[Ct] * Levels);
	if (IntColor > LevelsMinusOne)
		IntColor = LevelsMinusOne;
	TexColor[Ct] = IntColor * LevelWidth;
	TexColor[Ct] += (TexColor[Ct] * InvLevelsMinusOne);
	} // for

return(1);

} // PostProcPosterize::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcGlow::PostProcGlow(RasterAnimHost *RAHost, int AsHalo)
: PostProcFullBuf(RAHost, 0) // uses byte-channel glow amount buffer
{
double EffectDefault[2] = {1.0, 5.0};
double RangeDefaults[2][3] = {1.0, 0.0, .1,   1000.0, 0.0, 1};
RootTexture *Root;
Texture *Tex;

DoHalo = AsHalo;

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetMultiplier(100.0);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetDefaults(this, (char)1, EffectDefault[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetRangeDefaults(RangeDefaults[1]);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE1))
	{
	// set texture type to terrain param, luminosity input
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_TERRAINPARAM))
		{
		Tex->Misc = WCS_TEXTURE_TERRAINPARAM_LUMINOSITY;
		Tex->SetMiscDefaults();
		Tex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetValue(80.0);
		Tex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetValue(100.0);
		} // if
	} // if

} // PostProcGlow::PostProcGlow

/*===========================================================================*/

int PostProcGlow::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
// Value/Texture 1 is driver
// Value/Texture 2 is radius
// ?? is color
Point3d GlowColor;
double GlowDriver, GlowRadius, TexOpacity;
double TempValue[3];
double GlowRadSq, InvGlowRadSq, GlowXSq, GlowYSq, GlowHaloDriver;
double HaloFade;
int GlowX, GlowY, GlowRow, GlowCol;
long TransferZip;

if (PassNum == 0)
	{
	// Determine Glow Radius at this central point
	TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
	if ((GlowRadius = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].CurValue) > 0.0)
		{
		if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Eval(TempValue, &TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					GlowRadius *= (1.0 - TexOpacity + TempValue[0]);
					} // if
				else
					GlowRadius *= TempValue[0];
				} // if
			} // if
		} // if

	if (GlowRadius > 0.0)
		{
		// Determine Glow Intensity (Driver) at this central point
		TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
		if ((GlowDriver = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].CurValue) > 0.0)
			{
			if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Eval(TempValue, &TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						GlowDriver *= (1.0 - TexOpacity + TempValue[0]);
						} // if
					else
						GlowDriver *= TempValue[0];
					} // if
				} // if
			} // if

		if (GlowDriver > 0.0)
			{
			double GlowDist;
			// Make our own central pixel glow
			//TexColor[0] = flerp(GlowDriver, TexColor[0], GlowColor[0]);
			//TexColor[1] = flerp(GlowDriver, TexColor[1], GlowColor[1]);
			//TexColor[2] = flerp(GlowDriver, TexColor[2], GlowColor[2]);

			// run glow kernal
			GlowRadSq = GlowRadius * GlowRadius;
			HaloFade = GlowRadius * .5;
			HaloFade *= HaloFade; // squared to work with GlowRadSq
			InvGlowRadSq = 1.0 / GlowRadSq;
			for (GlowY = (int)-GlowRadius; GlowY < (int)GlowRadius; GlowY++)
				{
				GlowRow = Row + GlowY;
				if (GlowRow > 0 && GlowRow < Height) // if current row within image?
					{
					TransferZip = Width * GlowRow;
					GlowYSq = GlowY * GlowY;
					for (GlowX = (int)-GlowRadius; GlowX < (int)GlowRadius; GlowX++)
						{
						GlowCol = Col + GlowX;
						if (GlowCol > 0 && GlowCol < Width) // is current column within image?
							{
							GlowXSq = GlowX * GlowX;
							GlowDist = GlowXSq + GlowYSq;
							if (GlowDist <= GlowRadSq) // is current pixel within glow radius?
								{
								// rewrite surrounding pixel with glow
								if (DoHalo)
									{
									if (GlowDist < GlowRadSq - HaloFade)
										{
										// inside halo
										// invert power function.
										GlowHaloDriver = (GlowDist / (GlowRadSq - HaloFade)); // if (GlowRadSq - 2 =< 0) we can't get here
										GlowHaloDriver *= GlowHaloDriver; // squared
										} // if
									else
										{
										// outside halo
										GlowHaloDriver = 1.0 - ((GlowDist - (GlowRadSq - HaloFade)) / HaloFade);
										GlowHaloDriver *= GlowHaloDriver; // squared
										} // else
									//GlowHaloDriver = GlowHaloDriver * GlowHaloDriver * GlowHaloDriver; // cubed
									} // if
								else
									{
									GlowHaloDriver = 1.0 - (GlowDist * InvGlowRadSq);
									} // else
								GlowHaloDriver = WCS_min(1.0, GlowHaloDriver * GlowDriver);
								// write the value ourselves since we've already range checked and Zipped it
								PPByteBuf[TransferZip + GlowCol] = max((unsigned char)(GlowHaloDriver * 255.0), PPByteBuf[TransferZip + GlowCol]);
								} // if
							} // if
						} // for X
					} // if
				} // for Y
			} // if
		} // if
	} // if Pass == 0
else
	{
	TransferZip = Col + Row * Width;
	// Determine glow color for this point
	//GlowColor[0] = GlowColor[1] = 1.0; GlowColor[2] = 0.0; // Full on yellow right now
	GlowColor[0] = Color.GetCompleteValue(0);
	GlowColor[1] = Color.GetCompleteValue(1);
	GlowColor[2] = Color.GetCompleteValue(2);

	//TexColor[0] = TexColor[1] = TexColor[2] = PPByteBuf[TransferZip];

	// Lookup Glow amount for this pixel
	GlowHaloDriver = (double)(PPByteBuf[TransferZip]) * (1.0 / 255.0);

	if (GlowHaloDriver > 0.0)
		{
		TexColor[0] = TexColor[0] + GlowHaloDriver * GlowColor[0];
		TexColor[1] = TexColor[1] + GlowHaloDriver * GlowColor[1];
		TexColor[2] = TexColor[2] + GlowHaloDriver * GlowColor[2];
		} // if
	
	} // else Pass == 1


return(1);

} // PostProcGlow::EvalPostProcSample

/*===========================================================================*/

ULONG PostProcGlow::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCGLOW_DOHALO:
						{
						BytesRead = ReadBlock(ffile, (char *)&DoHalo, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcGlow::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcGlow::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCGLOW_DOHALO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&DoHalo)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0UL);

} // PostProcGlow::SaveSpecificData

/*===========================================================================*/

void PostProcGlow::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcGlow *CopyTo, *CopyFrom;

CopyTo = (PostProcGlow *)CopyToPP;
CopyFrom = (PostProcGlow *)CopyFromPP;

CopyTo->DoHalo = CopyFrom->DoHalo;

} // PostProcGlow::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

PostProcStar::PostProcStar(RasterAnimHost *RAHost)
: PostProcFullBuf(RAHost, 0) // uses byte-channel star amount buffer
{
double EffectDefault[7] = {1.0, 5.0, 4.0, 0.0, .5, 0.0, 1.0};
double RangeDefaults[7][3] = {1.0, 0.0, .1,   1000.0, 0.0, 1,   1000.0, 2.0, 1,
   1.0, 0.0, .1,   1.0, 0.0, .1,   360.0, 0.0, 10.0,   100, 1.0, 1.0};
RootTexture *Root;
Texture *Tex;
GraphNode *GN;

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetMultiplier(100.0);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetDefaults(this, (char)1, EffectDefault[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetRangeDefaults(RangeDefaults[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].SetDefaults(this, (char)0, EffectDefault[2]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].SetRangeDefaults(RangeDefaults[2]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].SetDefaults(this, (char)0, EffectDefault[3]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].SetRangeDefaults(RangeDefaults[3]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].SetMultiplier(100.0);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].SetDefaults(this, (char)0, EffectDefault[4]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].SetRangeDefaults(RangeDefaults[4]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].SetMultiplier(100.0);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE6].SetDefaults(this, (char)0, EffectDefault[5]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE6].SetRangeDefaults(RangeDefaults[5]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE7].SetDefaults(this, (char)0, EffectDefault[6]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE7].SetRangeDefaults(RangeDefaults[6]);

// Build default power curve
// 0, 1
// 0.108033241m, 0.1785714286
// 0.7756232687m, 0.01
// 1, 0

PowerCurve.AddNode(0.0, 1.0, 0.0);
PowerCurve.AddNode(0.108033241, 0.2, 0.0);
PowerCurve.AddNode(0.4899713467, 0.0, 0.0);
if (GN = PowerCurve.AddNode(1.0, 0.0, 0.0))
	{
	GN->SetLinear(1);
	} // if

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_VALUE1))
	{
	// set texture type to terrain param, luminosity input
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_TERRAINPARAM))
		{
		Tex->Misc = WCS_TEXTURE_TERRAINPARAM_LUMINOSITY;
		Tex->SetMiscDefaults();
		Tex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW].SetValue(80.0);
		Tex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH].SetValue(100.0);
		} // if
	} // if

} // PostProcStar::PostProcStar

/*===========================================================================*/

int PostProcStar::SpecialEdit(void)
{

PowerCurve.EditRAHost();
return(1);

} // PostProcStar::SpecialEdit


/*===========================================================================*/

int PostProcStar::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
// Value/Texture 1 is driver
// Value/Texture 2 is radius
// ?? is color
Point3d GlowColor;
double GlowDriver, GlowRadius, TexOpacity;
double TempValue[3];
double GlowXf, GlowYf;
double GlowRadSq, GlowXSq, GlowYSq, GlowHaloDriver;
double StarRot = 0.0, ArmDeg, HalfArmDeg;
double StarSharp = 1.0, StarSqueeze = 0.0;
double SumWeightInv;
double SubStep;
double SubStart = 0.0; // 0 is used if AA=1
int GlowX, GlowY, GlowRow, GlowCol;
int StarPoints = 5;
int AA;
long TransferZip;

StarPoints = (int)WCS_max(2.0, AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].CurValue);
StarRot = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE6].CurValue;
StarSharp = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].CurValue;
StarSqueeze = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].CurValue;

// antialiasing
AA = (int)(WCS_max(AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE7].CurValue, 1.0)); // AA only goes down to 1
SubStep = 1.0 / (double)AA;
SumWeightInv = 1.0 / (double)(AA * AA);
if (AA > 1) SubStart = -0.5;

ArmDeg = 360.0 / (double)StarPoints; // Points should not be less than 2
HalfArmDeg = ArmDeg * .5;

if (PassNum == 0)
	{
	// Determine Glow Radius at this central point
	TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
	if ((GlowRadius = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].CurValue) > 0.0)
		{
		if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Enabled)
			{
			if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE2]->Eval(TempValue, &TexData)) > 0.0)
				{
				if (TexOpacity < 1.0)
					{
					// Value[0] has already been diminished by the texture's opacity
					GlowRadius *= (1.0 - TexOpacity + TempValue[0]);
					} // if
				else
					GlowRadius *= TempValue[0];
				} // if
			} // if
		} // if

	if (GlowRadius > 0.0)
		{
		// Determine Glow Intensity (Driver) at this central point
		TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
		if ((GlowDriver = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].CurValue) > 0.0)
			{
			if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Eval(TempValue, &TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						GlowDriver *= (1.0 - TexOpacity + TempValue[0]);
						} // if
					else
						GlowDriver *= TempValue[0];
					} // if
				} // if
			} // if

		if (GlowDriver > 0.0)
			{
			double GlowDist;
			// Make our own central pixel glow
			//TexColor[0] = flerp(GlowDriver, TexColor[0], GlowColor[0]);
			//TexColor[1] = flerp(GlowDriver, TexColor[1], GlowColor[1]);
			//TexColor[2] = flerp(GlowDriver, TexColor[2], GlowColor[2]);

			// run glow kernal
			// adjust size by driver amount
			GlowRadius *= GlowDriver;

			GlowRadSq = GlowRadius * GlowRadius;
			for (GlowY = (int)-GlowRadius; GlowY < (int)GlowRadius; GlowY++)
				{
				GlowRow = Row + GlowY;
				if (GlowRow > 0 && GlowRow < Height) // if current row within image?
					{
					TransferZip = Width * GlowRow;
					GlowYSq = GlowY * GlowY; // for main pixel
					for (GlowX = (int)-GlowRadius; GlowX < (int)GlowRadius; GlowX++)
						{
						GlowCol = Col + GlowX;
						if (GlowCol > 0 && GlowCol < Width) // is current column within image?
							{
							GlowXSq = GlowX * GlowX;  // for main pixel
							GlowDist = GlowXSq + GlowYSq;  // for main pixel
							if (GlowDist <= GlowRadSq) // is current pixel within glow radius?
								{
								double GlowHaloSum = 0.0; // accumulate subsample values
								// subsample x 100
								for (GlowYf = SubStart; GlowYf < .5; GlowYf += SubStep)
								//GlowYf = 0.0;
									{
									GlowYSq = ((double)GlowY + GlowYf) * ((double)GlowY + GlowYf); // for sub pixel
									for (GlowXf = SubStart; GlowXf < .5; GlowXf += SubStep)
									//GlowXf = 0.0;
										{
										double StarPhase, StarPhaseMod, StarPhaseAmt, SqueezePhase, SqueezedRange, TotalSqueeze;
										GlowXSq = ((double)GlowX + GlowXf) * ((double)GlowX + GlowXf); // for sub pixel
										GlowDist = GlowXSq + GlowYSq; // for sub pixel

										StarPhase = findangle3(-(GlowY + GlowYf), (GlowX + GlowXf)) * PiUnder180;
										StarPhase -= StarRot;
										StarPhase = fmod(StarPhase, 360.0);
										if (StarPhase < 0.0)
											{
											StarPhase += 360.0;
											} // if
										if (StarPhase > 360.0)
											{
											StarPhase -= 360.0;
											} // if

										if (StarSqueeze > 0.0)
											{
											SqueezePhase = fmod(StarPhase, 180.0); // symmetric along star's Y axis
											SqueezePhase = fabs(90.0 - SqueezePhase); // how far away from 90 degrees (X axis) are we
											SqueezePhase = 1.0 - (SqueezePhase / 90.0); // 0...1 range
											TotalSqueeze = StarSqueeze * SqueezePhase; // how much total squeeze we get here.
											SqueezedRange = GlowRadSq * (1.0 - TotalSqueeze); // Squeeze in our apparent distance
											} // if
										else
											{
											SqueezedRange = GlowRadSq;
											} // else

										StarPhaseMod = fmod(StarPhase, (double)ArmDeg);

										if (StarPhaseMod > HalfArmDeg)
											{
											StarPhaseMod = ArmDeg - StarPhaseMod;
											} // if
										//StarPhaseMod = WCS_max(StarPhaseMod, 0.0001); // prevent divide-by-zero, below
										//StarPhaseAmt = (1.0 - (StarPhaseMod / HalfArmDeg)); // linear
										//StarPhaseAmt = StarPhaseAmt * StarPhaseAmt * StarPhaseAmt * StarPhaseAmt * StarPhaseAmt * StarPhaseAmt;
										StarPhaseAmt = PowerCurve.GetValue(0, (StarPhaseMod / HalfArmDeg));
										if (StarPhaseAmt < 0)
											StarPhaseAmt = 0;
										if (StarPhaseAmt > .3)
											StarPhaseAmt = .3;
										//StarPhaseAmt = .5;

										if (StarPhaseAmt > 0.0)
											{
											GlowHaloDriver = min(1.0, (GlowDist / (SqueezedRange * StarPhaseAmt)));
											GlowHaloDriver = 1.0 - GlowHaloDriver;
											} // if
										else
											{
											GlowHaloDriver = 0.0;
											} // else
										//GlowHaloDriver *= GlowHaloDriver; // squared
										//GlowHaloDriver = GlowHaloDriver * GlowHaloDriver * GlowHaloDriver; // cubed
										GlowHaloDriver = min(1.0, GlowHaloDriver * GlowDriver);
										GlowHaloSum += (GlowHaloDriver * SumWeightInv);
										} // for
									} // for
								//GlowHaloSum *= (.01); // 1.0 / 100.0
								GlowHaloSum = WCS_min(1.0, GlowHaloSum); // range limit again
								// write the value ourselves since we've already range checked and Zipped it
								PPByteBuf[TransferZip + GlowCol] = max((unsigned char)(GlowHaloSum * 255.0), PPByteBuf[TransferZip + GlowCol]);
								} // if
							} // if
						} // for X
					} // if
				} // for Y
			} // if
		} // if
	} // if Pass == 0
else
	{
	TransferZip = Col + Row * Width;
	// Determine glow color for this point
	//GlowColor[0] = GlowColor[1] = 1.0; GlowColor[2] = 0.0; // Full on yellow right now
	GlowColor[0] = Color.GetCompleteValue(0);
	GlowColor[1] = Color.GetCompleteValue(1);
	GlowColor[2] = Color.GetCompleteValue(2);


	//TexColor[0] = TexColor[1] = TexColor[2] = PPByteBuf[TransferZip];

	// Lookup Glow amount for this pixel
	GlowHaloDriver = (double)(PPByteBuf[TransferZip]) * (1.0 / 255.0);

	if (GlowHaloDriver > 0.0)
		{
		TexColor[0] = TexColor[0] + GlowHaloDriver * GlowColor[0];
		TexColor[1] = TexColor[1] + GlowHaloDriver * GlowColor[1];
		TexColor[2] = TexColor[2] + GlowHaloDriver * GlowColor[2];
		} // if
	
	} // else Pass == 1

return(1);

} // PostProcStar::EvalPostProcSample

/*===========================================================================*/

ULONG PostProcStar::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCSTAR_POWERCURVE:
						{
						BytesRead = PowerCurve.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcStar::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcStar::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ItemTag = WCS_POSTPROCSTAR_POWERCURVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = PowerCurve.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} // if wrote size of block 
			else
				goto WriteError;
			} // if curve saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcStar::SaveSpecificData

/*===========================================================================*/

void PostProcStar::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcStar *CopyTo, *CopyFrom;

CopyTo = (PostProcStar *)CopyToPP;
CopyFrom = (PostProcStar *)CopyFromPP;

CopyTo->PowerCurve.Copy(&CopyTo->PowerCurve, &CopyFrom->PowerCurve);

} // PostProcStar::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

PostProcLine::PostProcLine(RasterAnimHost *RAHost)
: PostProcNonPoint(RAHost, 3) // 3x3 filter size
{
double EffectDefault[2] = {1.0, 1.0};
double RangeDefaults[2][3] = {1.0, 0.0, .1,   1.0, 0.0, .1};

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetDefaults(this, (char)0, EffectDefault[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].SetMultiplier(100.0);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetDefaults(this, (char)1, EffectDefault[1]);
AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].SetRangeDefaults(RangeDefaults[1]);

} // PostProcLine::PostProcLine

/*===========================================================================*/

int PostProcLine::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{ // Sobel edge algorithm
double RSample, GSample, BSample;
double RH, GH, BH, RV, GV, BV;
double Weight, Saturation;
double TempValue[3];

Saturation = (float)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1].GetCurValue();

RH = GH = BH = RV = GV = BV = 0.0;
// horizontal kernel
Weight = -1.0;	if (RGBSampleRelative(-1, -1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};
Weight = -2.0;	if (RGBSampleRelative( 0, -1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};
Weight = -1.0;	if (RGBSampleRelative( 1, -1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};
Weight =  1.0;	if (RGBSampleRelative(-1,  1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};
Weight =  2.0;	if (RGBSampleRelative( 0,  1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};
Weight =  1.0;	if (RGBSampleRelative( 1,  1, Width, Height, RSample, GSample, BSample)) {RH += RSample * Weight; GH += GSample * Weight; BH += BSample * Weight;};

// vertical kernel
Weight = -1.0;	if (RGBSampleRelative(-1, -1, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};
Weight = -2.0;	if (RGBSampleRelative(-1,  0, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};
Weight = -1.0;	if (RGBSampleRelative(-1,  1, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};
Weight =  1.0;	if (RGBSampleRelative( 1, -1, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};
Weight =  2.0;	if (RGBSampleRelative( 1,  0, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};
Weight =  1.0;	if (RGBSampleRelative( 1,  1, Width, Height, RSample, GSample, BSample)) {RV += RSample * Weight; GV += GSample * Weight; BV += BSample * Weight;};

RH = max(RH * .25, RV * .25);
GH = max(GH * .25, GV * .25);
BH = max(BH * .25, BV * .25);

// evaluate saturation value-texture
if (Saturation != 0.0)
	{
	if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Enabled)
		{
		TempValue[0] = TempValue[1] = TempValue[2] = 0.0;
		if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_VALUE1]->Eval(TempValue, &TexData)) > 0.0)
			{
			if (TexOpacity < 1.0)
				{
				TexOpacity = 1.0 - TexOpacity;
				Saturation = TempValue[0] + Saturation * TexOpacity;
				} // if
			} // if
		} // if
	} // if

if (Saturation == 0.0)
	{
	TexColor[0] = TexColor[1] = TexColor[2] = WCS_max(max(RH, GH), BH);
	} // if
else if (Saturation == 1.0)
	{
	TexColor[0] = RH;
	TexColor[1] = GH;
	TexColor[2] = BH;
	} // if
else
	{
	// reuse RV for monochrome version
	RV = max(max(RH, GH), BH);
	if (Saturation == 0.0) // fully monochrome
		{
		TexColor[0] = TexColor[1] = TexColor[2] = RV;
		} // if
	else
		{
		TexColor[0] = flerp(Saturation, RV, RH); // think these are in the right order
		TexColor[1] = flerp(Saturation, RV, GH);
		TexColor[2] = flerp(Saturation, RV, BH);
		} // else
	} // else monochrome used in some amount

return(1);

} // PostProcLine::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcNegative::PostProcNegative(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{

strcpy(Name, PostProcEventNames[GetType()]);

} // PostProcNegative::PostProcNegative

/*===========================================================================*/

int PostProcNegative::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

// simplest of all...
TexColor[0] = 1.0 - TexColor[0];
TexColor[1] = 1.0 - TexColor[1];
TexColor[2] = 1.0 - TexColor[2];
if (TexColor[0] < 0.0)
	TexColor[0] = 0.0;
if (TexColor[1] < 0.0)
	TexColor[1] = 0.0;
if (TexColor[2] < 0.0)
	TexColor[2] = 0.0;

return(1);

} // PostProcNegative::EvalPostProcSample

/*===========================================================================*/
/*===========================================================================*/

PostProcFullBuf::PostProcFullBuf(RasterAnimHost *RAHost, char UseFloatBuffer)
: PostProcessEvent(RAHost)
{

PPByteBuf = NULL;
PPFloatBuf = NULL;
UseFloat = UseFloatBuffer;

} // PostProcFullBuf::PostProcFullBuf

/*===========================================================================*/

int PostProcFullBuf::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics)
{
int Success = 0;

// ensure stuff is deallocated if necessary (safety check)
CleanupPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

if (UseFloat)
	{
	if (PPFloatBuf = (float *)AppMem_Alloc(Width * Height * sizeof(float), APPMEM_CLEAR))
		{
		Success = 1;
		} // if
	} // if
else
	{
	if (PPByteBuf = (unsigned char *)AppMem_Alloc(Width * Height * sizeof(char), APPMEM_CLEAR))
		{
		Success = 1;
		} // if
	} // else

return(Success);

} // PostProcFullBuf::PrepForPostProc

/*===========================================================================*/

int PostProcFullBuf::CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum, int UpdateDiagnostics)
{
int Success = 1;

if (PPByteBuf)
	{
	AppMem_Free(PPByteBuf, Width * Height * sizeof(char));
	PPByteBuf = NULL;
	} // if

if (PPFloatBuf)
	{
	AppMem_Free(PPFloatBuf, Width * Height * sizeof(float));
	PPFloatBuf = NULL;
	} // if

return(Success);

} // PostProcFullBuf::CleanupPostProc

/*===========================================================================*/

int PostProcFullBuf::ReadByteBufRelative(signed char XOff, signed char YOff, long Width, long Height, unsigned char &ByteSample)
{
int Success = 0;
unsigned long Zip;

if (RangeCheckRelativeZip(XOff, YOff, Width, Height, Zip))
	{
	ByteSample = PPByteBuf[Zip]; 
	Success = 1;
	} // if

return(Success);

} // PostProcFullBuf::ReadByteBufRelative 

/*===========================================================================*/

int PostProcFullBuf::WriteByteBufRelative(signed char XOff, signed char YOff, long Width, long Height, unsigned char ByteSample)
{
int Success = 0;
unsigned long Zip;

if (RangeCheckRelativeZip(XOff, YOff, Width, Height, Zip))
	{
	PPByteBuf[Zip] = ByteSample; 
	Success = 1;
	} // if

return(Success);

} // PostProcFullBuf::WriteByteBufRelative

/*===========================================================================*/

int PostProcFullBuf::ReadFloatBufRelative(signed char XOff, signed char YOff, long Width, long Height, float &FloatSample)
{
int Success = 0;
unsigned long Zip;

if (RangeCheckRelativeZip(XOff, YOff, Width, Height, Zip))
	{
	FloatSample = PPFloatBuf[Zip];
	Success = 1;
	} // if

return(Success);

} // PostProcFullBuf::ReadFloatBufRelative

/*===========================================================================*/

int PostProcFullBuf::WriteFloatBufRelative(signed char XOff, signed char YOff, long Width, long Height, float FloatSample)
{
int Success = 0;
unsigned long Zip;

if (RangeCheckRelativeZip(XOff, YOff, Width, Height, Zip))
	{
	PPFloatBuf[Zip] = FloatSample;
	Success = 1;
	} // if

return(Success);

} // PostProcFullBuf::WriteFloatBufRelative

/*===========================================================================*/
/*===========================================================================*/

int PostProcText::PlotText(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragBlock, PolygonData *Poly, Raster *SourceRast, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
	int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
	double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha, unsigned char *EdgeAlpha)
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtx, wty, SourceWt, EdgeWt, BodyWt, SumEdgeWt, SumBodyWt, MaxWt;
double ModHSV[3], UnModHSV[3];
rPixelFragment *PixFrag = NULL;
rPixelHeader *FragMap = NULL;
int FragBits, ReplaceValues;
long Px, Py, Pxp1, Pyp1, X, Y, DRows, DCols, DxStart, DyStart, PixZip, SourceZip, i, j;
unsigned short OldBits, NewBits, TotalBits;
PixelData Pix;
VertexDEM PixelVtx;

Success = 1;
CurFrag = NULL;

// if nothing to do
if (! (AffectRed || AffectGrn || AffectBlu))
	return (1);

Value[2] = Value[1] = Value[0] = 0.0;

// assign buffer ptrs - at least all the ones the renderer needs to know about
if (OptionalBitmaps)
	{
	Bitmap[0] = OptionalBitmaps[0];
	Bitmap[1] = OptionalBitmaps[1];
	Bitmap[2] = OptionalBitmaps[2];
	} // if
else
	{
	Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
	Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
	Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
	} // else

ZBuf = (float *)Buffers->FindBuffer("ZBUF",								WCS_RASTER_BANDSET_FLOAT);
AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS",				WCS_RASTER_BANDSET_BYTE);
LatBuf = (float *)Buffers->FindBuffer("LATITUDE",						WCS_RASTER_BANDSET_FLOAT);
LonBuf = (float *)Buffers->FindBuffer("LONGITUDE",						WCS_RASTER_BANDSET_FLOAT);
ElevBuf = (float *)Buffers->FindBuffer("ELEVATION",						WCS_RASTER_BANDSET_FLOAT);
RelElBuf = (float *)Buffers->FindBuffer("RELATIVE ELEVATION",			WCS_RASTER_BANDSET_FLOAT);
ReflectionBuf = (float *)Buffers->FindBuffer("REFLECTION",				WCS_RASTER_BANDSET_FLOAT);
IllumBuf = (float *)Buffers->FindBuffer("ILLUMINATION",					WCS_RASTER_BANDSET_FLOAT);
SlopeBuf = (float *)Buffers->FindBuffer("SLOPE",						WCS_RASTER_BANDSET_FLOAT);
AspectBuf = (float *)Buffers->FindBuffer("ASPECT",						WCS_RASTER_BANDSET_FLOAT);
NormalBuf[0] = (float *)Buffers->FindBuffer("SURFACE NORMAL X",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[1] = (float *)Buffers->FindBuffer("SURFACE NORMAL Y",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[2] = (float *)Buffers->FindBuffer("SURFACE NORMAL Z",			WCS_RASTER_BANDSET_FLOAT);
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
ObjectBuf = (RasterAnimHost **)Buffers->FindBuffer("OBJECT",			WCS_RASTER_BANDSET_FLOAT);
ObjTypeBuf = (unsigned char *)Buffers->FindBuffer("OBJECT TYPE",		WCS_RASTER_BANDSET_BYTE);

if (FragBlock)
	FragMap = FragBlock->GetFragMap();

if (! ((FragBlock && FragMap) || (Bitmap[0] && Bitmap[1] && Bitmap[2] && ZBuf && AABuf)))
	return (0);

// invoke prep on derived class, bail if it fails
if (!PrepForPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
	{
	CleanupPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	return(0);
	} // if

Pix.SetDefaults();
Pix.TexData = &RendData->TexData;
Pix.PixelType = WCS_PIXELTYPE_FOLIAGE;
Pix.CloudShading = BackLightingPct;
Pix.TranslucencyExp = 1.0;
Pix.ShadowFlags = Poly->ShadowFlags;
Pix.ReceivedShadowIntensity = Poly->ReceivedShadowIntensity;
Pix.ShadowOffset = Poly->ShadowOffset;
Pix.Object = Poly->Object;
TexData.VDEM[0] = &VertDEM;
TexData.MetersPerDegLat = RendData->EarthLatScaleMeters;
TexData.MetersPerDegLon = RendData->RefLonScaleMeters;
TexData.TexRefLon = RendData->TexRefLon;
TexData.TexRefLat = RendData->TexRefLat;
TexData.TexRefElev = RendData->TexRefElev;
Dox = CenterX - .5 * WidthInPixels;
Dex = CenterX + .5 * WidthInPixels;
Doy = CenterY - .5 * HeightInPixels;
Dey = CenterY + .5 * HeightInPixels;

dX = (double)SourceRast->Cols / WidthInPixels;
dY = (double)SourceRast->Rows / HeightInPixels;

MaxWt = dX * dY;

Sox = (WCS_floor(Dox) - Dox) * dX;
Soy = (WCS_floor(Doy) - Doy) * dY;

DxStart = quicklongfloor(Dox); 
DyStart = quicklongfloor(Doy); 
DCols = quickftol(WCS_ceil(Dex) - WCS_floor(Dox));
DRows = quickftol(WCS_ceil(Dey) - WCS_floor(Doy));

for (Y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0;
	j < DRows; j ++, Y ++, Coy += dY, Cey += dY)
	{
	if (Y < 0)
		continue;
	if (Y >= RendData->Height)
		break;
	PixZip = Y * RendData->Width + DxStart;
	for (X = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, X ++, Cox += dX, Cex += dX, PixZip ++)
		{
		if (X < 0)
			continue;
		if (X >= RendData->Width)
			break;
		if (FragBlock || (ZFlt < ZBuf[PixZip] || AABuf[PixZip] < 255))
			{
			Py = quickftol(Coy);
			SumEdgeWt = SumBodyWt = 0.0;
			for (Pyp1 = Py + 1; Py < Cey && Py < SourceRast->Rows; Py ++, Pyp1 ++)
				{
				if (Py < 0)
					continue;
				wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
				for (Px = quickftol(Cox), Pxp1 = quickftol(Cox + 1); Px < Cex && Px < SourceRast->Cols; Px ++, Pxp1 ++)
					{
					if (Px < 0)
						continue;
					wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
					SourceWt = wtx * wty;
					SourceZip = Py * SourceRast->Cols + Px;
					if (SourceRast->AltFloatMap)
						{
						BodyWt = SourceWt * SourceRast->AltFloatMap[SourceZip];
						EdgeWt = 0.0;
						} // if
					else
						{
						if (EdgeAlpha)
							EdgeWt = SourceWt * EdgeAlpha[SourceZip] * (1.0 / 255.0);
						else
							EdgeWt = 0.0;
						if (Alpha)
							BodyWt = SourceWt * Alpha[SourceZip] * (1.0 / 255.0);
						else
							BodyWt = 0.0;
						} // else

					SumEdgeWt += EdgeWt;
					SumBodyWt += BodyWt;
					} // for Px
				} // for Py

			// Plot pixel
			if (SumEdgeWt > 0.0 || SumBodyWt > 0.0)
				{
				// set texture data
				TexData.Elev = ElevBuf ? ElevBuf[PixZip]: 0.0;
				TexData.RelElev = RelElBuf ? RelElBuf[PixZip]: 0.0f;
				TexData.Slope = SlopeBuf ? SlopeBuf[PixZip]: 0.0;
				TexData.Aspect = AspectBuf ? AspectBuf[PixZip]: 0.0;
				TexData.Illumination = IllumBuf ? IllumBuf[PixZip]: 0.0;
				TexData.Object = ObjectBuf ? ObjectBuf[PixZip]: NULL;
				TexData.ObjectType = ObjTypeBuf ? ObjTypeBuf[PixZip]: NULL;
				TexData.Latitude = LatBuf ? LatBuf[PixZip] + RendData->TexRefLat: 0.0;
				TexData.Longitude = LonBuf ? LonBuf[PixZip] + RendData->TexRefLon: 0.0;
				TexData.Reflectivity = ReflectionBuf ? ReflectionBuf[PixZip]: 0.0;
				TexData.Normal[0] = NormalBuf[0] ? NormalBuf[0][PixZip]: 0.0;
				TexData.Normal[1] = NormalBuf[1] ? NormalBuf[1][PixZip]: 0.0;
				TexData.Normal[2] = NormalBuf[2] ? NormalBuf[2][PixZip]: 0.0;
				TexData.ZDist = ZBuf ? ZBuf[PixZip]: 0.0;
				TexData.QDist = TexData.ZDist;

				TexData.PixelX[0] = (double)X + RendData->Rend->SegmentOffsetX - Width * RendData->Cam->PanoPanels * .5;
				TexData.PixelX[1] = (double)(X + 1) + RendData->Rend->SegmentOffsetX - Width * RendData->Cam->PanoPanels * .5;
				TexData.PixelY[0] = -((double)Y + RendData->Rend->SegmentOffsetY) + Height * RendData->Opt->RenderImageSegments * .5;
				TexData.PixelY[1] = -((double)(Y + 1) + RendData->Rend->SegmentOffsetY) + Height * RendData->Opt->RenderImageSegments * .5;
				TexData.PixelUnityX[0] = TexData.PixelX[0] / (Width * RendData->Cam->PanoPanels);
				TexData.PixelUnityX[1] = TexData.PixelX[1] / (Width * RendData->Cam->PanoPanels);
				TexData.PixelUnityY[0] = TexData.PixelY[0] / (Height * RendData->Opt->RenderImageSegments);
				TexData.PixelUnityY[1] = TexData.PixelY[1] / (Height * RendData->Opt->RenderImageSegments);
				//TexData.PixelX[0] = X - Width * .5;
				//TexData.PixelX[1] = X + 1 - Width * .5;
				//TexData.PixelY[0] = -Y + Height * .5;
				//TexData.PixelY[1] = -Y - 1 + Height * .5;
				//TexData.PixelUnityX[0] = TexData.PixelX[0] / Width;
				//TexData.PixelUnityX[1] = TexData.PixelX[1] / Width;
				//TexData.PixelUnityY[0] = TexData.PixelY[0] / Height;
				//TexData.PixelUnityY[1] = TexData.PixelY[1] / Height;
				VertDEM.ScrnXYZ[0] = X + .5; 
				VertDEM.ScrnXYZ[1] = Y + .5; 

				//TexData.LowX = X;
				//TexData.HighX = X + 1;
				//TexData.LowY = Y;
				//TexData.HighY = Y + 1;
				//TexData.LowZ = TexData.ZDist;
				//TexData.HighZ = TexData.ZDist;
				//TexData.LatRange[0] = TexData.LatRange[1] = TexData.LatRange[2] = TexData.LatRange[3] = TexData.Latitude;
				//TexData.LonRange[0] = TexData.LonRange[1] = TexData.LonRange[2] = TexData.LonRange[3] = TexData.Longitude;
				if (RendData->Cam && RendData->Cam->TargPos)
					{
					TexData.CamAimVec[0] = -RendData->Cam->TargPos->XYZ[0];
					TexData.CamAimVec[1] = -RendData->Cam->TargPos->XYZ[1];
					TexData.CamAimVec[2] = -RendData->Cam->TargPos->XYZ[2];
					} // if
				//if (TexCoordType == WCS_POSTPROCEVENT_COORDTYPE_IMAGE)
				//	{
				//	TexData.LowX /= Width;
				//	TexData.LowX -= .5;
				//	TexData.HighX /= Width;
				//	TexData.HighX -= .5;
				//	TexData.LowY /= -Height;
				//	TexData.LowY += .5;
				//	TexData.HighY /= -Height;
				//	TexData.HighY += .5;
				//	} // if
				//else
				//	{
				//	TexData.LowX -= Width * .5;
				//	TexData.HighX -= Width * .5;
				//	TexData.LowY = -TexData.LowY + Height * .5;
				//	TexData.HighY = -TexData.HighY + Height * .5;
				//	} // else
				if (FragBlock)
					{
					// set fragment texture data
					if (CurFrag = FragMap[PixZip].FragList)
						{
						TexData.ZDist = CurFrag->ZBuf;
						TexData.QDist = TexData.ZDist;
						} // if
					} // if

				// this sets up XYZ and lat lon coords for alternate texture coordinate spaces
				VertDEM.ScrnXYZ[2] = VertDEM.Q = TexData.ZDist > 0.0 ? TexData.ZDist: 1.0;
				RendData->Cam->UnProjectVertexDEM(RendData->DefCoords, &VertDEM, RendData->EarthLatScaleMeters, RendData->PlanetRad, 1);

				// get existing values
				if (CurFrag)
					{
					CurFrag->ExtractColor(Value);
					TexData.RGB[0] = TexColor[0] = OrigValue[0] = Value[0];
					TexData.RGB[1] = TexColor[1] = OrigValue[1] = Value[1];
					TexData.RGB[2] = TexColor[2] = OrigValue[2] = Value[2];
					} // if
				else
					{
					if (ExponentBuf)
						{
						RendData->Rend->GetPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
						TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0];
						TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1];
						TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2];
						} // if
					else
						{
						TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
						TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
						TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
						} // else
					} // if

				// evaluate texture intensity
				if ((Intensity = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].CurValue) > 0.0)
					{
					if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Enabled)
						{
						if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Eval(Value, &TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								// Value[0] has already been diminished by the texture's opacity
								Intensity *= (1.0 - TexOpacity + Value[0]);
								} // if
							else
								Intensity *= Value[0];
							} // if
						} // if
					} // if

				if (Intensity > 0.0)
					{
					if (SumBodyWt > 0.0)
						{
						Pix.RGB[0] = Color.GetCompleteValue(0);
						Pix.RGB[1] = Color.GetCompleteValue(1);
						Pix.RGB[2] = Color.GetCompleteValue(2);
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled)
							{
							if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Eval(Value, &TexData)) > 0.0)
								{
								if (TexOpacity < 1.0)
									{
									TexOpacity = 1.0 - TexOpacity;
									Pix.RGB[0] = Pix.RGB[0] * TexOpacity + Value[0];
									Pix.RGB[1] = Pix.RGB[1] * TexOpacity + Value[1];
									Pix.RGB[2] = Pix.RGB[2] * TexOpacity + Value[2];
									} // if
								else
									{
									Pix.RGB[0] = Value[0];
									Pix.RGB[1] = Value[1];
									Pix.RGB[2] = Value[2];
									} // else
								} // if
							} // if
						if (SumEdgeWt > 0.0)
							{
							InvIntensity = SumBodyWt / (SumEdgeWt + SumBodyWt);
							Pix.RGB[0] *= InvIntensity;
							Pix.RGB[1] *= InvIntensity;
							Pix.RGB[2] *= InvIntensity;
							} // if
						} // if
					else
						{
						Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
						} // else

					SumBodyWt += SumEdgeWt;
					SumBodyWt /= MaxWt;
					if (SumBodyWt > 1.0)
						SumBodyWt = 1.0;

					Intensity *= SumBodyWt;
					NewBits = (unsigned short)(Intensity * 255.9);
					if (FragBits = NewBits)
						{
						if (FragBlock)
							{
							if (FragBits > 255)
								FragBits = 255;
							if (! (PixFrag = FragMap[PixZip].PlotPixel(FragBlock, ZFlt, (unsigned char)FragBits, ~0UL, ~0UL, RendData->Rend->FragmentDepth + 1, 
								(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC)))
								{
								continue;
								} // if
							} // if
						else
							{
							OldBits = AABuf[PixZip];
							if (ZFlt >= ZBuf[PixZip])					// new pixel farther than old pixel
								{
								if (NewBits + OldBits > 255)
									NewBits = 255 - OldBits;
								if (! NewBits)
									continue;
								} // if
							else
								{
								if (NewBits + OldBits > 255)
									OldBits = 255 - NewBits;
								} // else
							TotalBits = NewBits + OldBits;
							Intensity = (double)NewBits / (double)TotalBits;
							} // if ! FragBlock

						if (IlluminateText || (ApplyVolumetrics && RendData->Rend->VolumetricAtmospheresExist))
							{
							PixelVtx.ScrnXYZ[0] = X + .5;
							PixelVtx.ScrnXYZ[1] = Y + .5;
							PixelVtx.ScrnXYZ[2] = PixelVtx.Q = ZFlt;
							RendData->Cam->UnProjectVertexDEM(RendData->DefCoords, &PixelVtx, RendData->EarthLatScaleMeters, RendData->PlanetRad, 1);
							Pix.XYZ[0] = PixelVtx.XYZ[0];
							Pix.XYZ[1] = PixelVtx.XYZ[1];
							Pix.XYZ[2] = PixelVtx.XYZ[2];
							Pix.Q = PixelVtx.Q;
							Pix.Elev = PixelVtx.Elev;
							// view vector is used for specularity and for that it is best to have it point toward the camera
							Pix.ViewVec[0] = RendData->Cam->CamPos->XYZ[0] - Pix.XYZ[0];
							Pix.ViewVec[1] = RendData->Cam->CamPos->XYZ[1] - Pix.XYZ[1];
							Pix.ViewVec[2] = RendData->Cam->CamPos->XYZ[2] - Pix.XYZ[2];
							UnitVector(Pix.ViewVec); // Illumination code requires this to be unitized in advance to save time later
							if (IlluminateText)
								{
								// make surface normal point toward camera
								Pix.Normal[0] = Pix.ViewVec[0]; // ViewVec should already be unitized, so Normal will be too
								Pix.Normal[1] = Pix.ViewVec[1];
								Pix.Normal[2] = Pix.ViewVec[2];
								// needs to be normalized since it will be put in Normal XYZ buffers
								// UnitVector(Pix.Normal); // no longer necessary, since ViewVec is pre-normalized
								// modify normal based on vertical and horizontal position of pixel in foliage image
								if (RendData->Rend->LightTexturesExist)
									RendData->TransferTextureData(&PixelVtx);
								RendData->Rend->IlluminatePixel(&Pix);
								} // if
							} // if

						ReplaceValues = 0;

						if (! FragBlock)
							{
							if (UpdateDiagnostics)					// new tree farther than old tree
								{
								AABuf[PixZip] = (unsigned char)TotalBits;
								} // else ZFlt
							if (ZFlt < ZBuf[PixZip])
								{
								ReplaceValues = 1;
								} // if
							} // if
						else
							{
							ReplaceValues = (ZFlt <= FragMap[PixZip].GetFirstZ());
							} // else

						// plot data
						if (PixFrag)
							{
							PixFrag->PlotPixel(FragBlock, Pix.RGB, Pix.Reflectivity, Pix.Normal);
							RendData->Rend->ScreenPixelPlotFragments(&FragMap[PixZip], X + RendData->Rend->DrawOffsetX, Y + RendData->Rend->DrawOffsetY);
							} // if
						else
							{
							if (RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
								{
								RGBtoHSV(UnModHSV, OrigValue);
								RGBtoHSV(ModHSV, Pix.RGB);
								if (AffectRed)
									UnModHSV[0] = ModHSV[0];
								if (AffectGrn)
									UnModHSV[1] = ModHSV[1];
								if (AffectBlu)
									UnModHSV[2] = ModHSV[2];
								HSVtoRGB(UnModHSV, Pix.RGB);
								} // if

							// lerp output values
							InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
							if (ExponentBuf && UpdateDiagnostics)
								{
								if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[0] = (OrigValue[0] * InvIntensity + Pix.RGB[0] * Intensity);
								if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[1] = (OrigValue[1] * InvIntensity + Pix.RGB[1] * Intensity);
								if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[2] = (OrigValue[2] * InvIntensity + Pix.RGB[2] * Intensity);
								RendData->Rend->PlotPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
								} // if
							else
								{
								Intensity *= 255.0;
								if (Pix.RGB[0] > 1.0)
									Pix.RGB[0] = 1.0;
								if (Pix.RGB[1] > 1.0)
									Pix.RGB[1] = 1.0;
								if (Pix.RGB[2] > 1.0)
									Pix.RGB[2] = 1.0;
								if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[0][PixZip] = (unsigned char)(Bitmap[0][PixZip] * InvIntensity + Pix.RGB[0] * Intensity);
								if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[1][PixZip] = (unsigned char)(Bitmap[1][PixZip] * InvIntensity + Pix.RGB[1] * Intensity);
								if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[2][PixZip] = (unsigned char)(Bitmap[2][PixZip] * InvIntensity + Pix.RGB[2] * Intensity);
								} // else
							} // else

						if (UpdateDiagnostics && ReplaceValues)
							{
							if (! FragBlock && ZBuf)
								ZBuf[PixZip] = ZFlt;
							if (LatBuf)
								LatBuf[PixZip] = (float)(PixelVtx.Lat - RendData->Rend->TexRefLat);
							if (LonBuf)
								LonBuf[PixZip] = (float)(PixelVtx.Lon - RendData->Rend->TexRefLon);
							if (IllumBuf)
								IllumBuf[PixZip] = (float)Pix.Illum;
							if (RelElBuf)
								RelElBuf[PixZip] = 0.0f;
							if (SlopeBuf)
								SlopeBuf[PixZip] = 0.0f;
							if (AspectBuf)
								AspectBuf[PixZip] = 0.0f;
							if (FragBlock || (! ReflectionBuf || ! ReflectionBuf[PixZip] || TotalBits >= 255))
								{
								if (NormalBuf[0])
									NormalBuf[0][PixZip] = (float)Pix.Normal[0];
								if (NormalBuf[1])
									NormalBuf[1][PixZip] = (float)Pix.Normal[1];
								if (NormalBuf[2])
									NormalBuf[2][PixZip] = (float)Pix.Normal[2];
								} // if
							if (ObjectBuf)
								ObjectBuf[PixZip] = Poly->Object;
							if (ObjTypeBuf)
								ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC;
							if (ElevBuf)
								ElevBuf[PixZip] = (float)PixelVtx.Elev;
							} // if

						} // if NewBits
					} // if intensity
				} // if PixWt
			} // if
		} // for x
	if (BWDE)
		{
		if (BWDE)
			{
			if (BWDE->Update(Row + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else if (Master)
		{
		Master->ProcUpdate(Row + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for y

// invoke cleanup on derived class
CleanupPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

return (Success);

} // PostProcText::PlotText

/*===========================================================================*/

ULONG PostProcText::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCTEXT_MESGTEXT:
						{
						BytesRead = ReadBlock(ffile, (char *)MesgText, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCTEXT_ILLUMINATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Illuminate, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCTEXT_APPLYVOLUMETRICS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyVolumetrics, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCTEXT_RECEIVESHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcText::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcText::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_MESGTEXT, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MesgText) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)MesgText)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_ILLUMINATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Illuminate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_APPLYVOLUMETRICS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyVolumetrics)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_RECEIVESHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcText::SaveSpecificData

/*===========================================================================*/

void PostProcText::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcText *CopyTo, *CopyFrom;

CopyTo = (PostProcText *)CopyToPP;
CopyFrom = (PostProcText *)CopyFromPP;

CopyTo->Illuminate = CopyFrom->Illuminate;
CopyTo->ApplyVolumetrics = CopyFrom->ApplyVolumetrics;
CopyTo->ReceiveShadows = CopyFrom->ReceiveShadows;
strcpy(CopyTo->MesgText, CopyFrom->MesgText);

} // PostProcText::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

int PostProcImage::PlotImage(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragBlock, PolygonData *Poly, Raster *SourceRast, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
	int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
	double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha, unsigned char *EdgeAlpha)
/*** F2 MOD
int PostProcImage::PlotImage(RenderData *RendData, BufferNode *Buffers, rPixelBlockHeader *FragBlock, PolygonData *Poly, Raster *SourceRast, 
	float Width, float Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, long FrameNum,
	int UpdateDiagnostics, double CenterX, double CenterY, double WidthInPixels, double HeightInPixels,
	double BackLightingPct, int IlluminateText, int ApplyVolumetrics, float ZFlt, unsigned char *Alpha, unsigned char *EdgeAlpha)
***/
{
double Dox, Doy, Dex, Dey, SumEdgeWt, SumBodyWt;
double ModHSV[3], UnModHSV[3];
long X, Y, DRows, DCols, DxStart, DyStart, PixZip, i, j;
int FragBits, ReplaceValues;
unsigned short OldBits, NewBits, TotalBits;
rPixelFragment *PixFrag = NULL;
PixelData Pix;
VertexDEM PixelVtx;
rPixelHeader *FragMap = NULL;

Success = 1;
CurFrag = NULL;

// if nothing to do
if (! (AffectRed || AffectGrn || AffectBlu))
	return (1);

Value[2] = Value[1] = Value[0] = 0.0;

// assign buffer ptrs - at least all the ones the renderer needs to know about
if (OptionalBitmaps)
	{
	Bitmap[0] = OptionalBitmaps[0];
	Bitmap[1] = OptionalBitmaps[1];
	Bitmap[2] = OptionalBitmaps[2];
	} // if
else
	{
	Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
	Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
	Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
	} // else

ZBuf = (float *)Buffers->FindBuffer("ZBUF",								WCS_RASTER_BANDSET_FLOAT);
AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS",				WCS_RASTER_BANDSET_BYTE);
LatBuf = (float *)Buffers->FindBuffer("LATITUDE",						WCS_RASTER_BANDSET_FLOAT);
LonBuf = (float *)Buffers->FindBuffer("LONGITUDE",						WCS_RASTER_BANDSET_FLOAT);
ElevBuf = (float *)Buffers->FindBuffer("ELEVATION",						WCS_RASTER_BANDSET_FLOAT);
RelElBuf = (float *)Buffers->FindBuffer("RELATIVE ELEVATION",			WCS_RASTER_BANDSET_FLOAT);
ReflectionBuf = (float *)Buffers->FindBuffer("REFLECTION",				WCS_RASTER_BANDSET_FLOAT);
IllumBuf = (float *)Buffers->FindBuffer("ILLUMINATION",					WCS_RASTER_BANDSET_FLOAT);
SlopeBuf = (float *)Buffers->FindBuffer("SLOPE",						WCS_RASTER_BANDSET_FLOAT);
AspectBuf = (float *)Buffers->FindBuffer("ASPECT",						WCS_RASTER_BANDSET_FLOAT);
NormalBuf[0] = (float *)Buffers->FindBuffer("SURFACE NORMAL X",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[1] = (float *)Buffers->FindBuffer("SURFACE NORMAL Y",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[2] = (float *)Buffers->FindBuffer("SURFACE NORMAL Z",			WCS_RASTER_BANDSET_FLOAT);
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
ObjectBuf = (RasterAnimHost **)Buffers->FindBuffer("OBJECT",			WCS_RASTER_BANDSET_FLOAT);
ObjTypeBuf = (unsigned char *)Buffers->FindBuffer("OBJECT TYPE",		WCS_RASTER_BANDSET_BYTE);

if (FragBlock)
	FragMap = FragBlock->GetFragMap();

if (! ((FragBlock && FragMap) || (Bitmap[0] && Bitmap[1] && Bitmap[2] && ZBuf && AABuf)))
	return (0);

// invoke prep on derived class, bail if it fails
if (!PrepForPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
	{
	CleanupPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	return(0);
	} // if
/*** F2 MOD
if (!PrepForPostProc(RendData, Buffers, FragBlock, (long)(Width + 0.5f), (long)(Height + 0.5f), BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
	{
	CleanupPostProc(RendData, Buffers, FragBlock, (long)(Width + 0.5f), (long)(Height + 0.5f), BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	return(0);
	} // if
***/

Pix.SetDefaults();
Pix.TexData = &RendData->TexData;
Pix.PixelType = WCS_PIXELTYPE_FOLIAGE;
Pix.CloudShading = BackLightingPct;
Pix.TranslucencyExp = 1.0;
Pix.ShadowFlags = Poly->ShadowFlags;
Pix.ReceivedShadowIntensity = Poly->ReceivedShadowIntensity;
Pix.ShadowOffset = Poly->ShadowOffset;
Pix.Object = Poly->Object;
TexData.VDEM[0] = &VertDEM;
TexData.MetersPerDegLat = RendData->EarthLatScaleMeters;
TexData.MetersPerDegLon = RendData->RefLonScaleMeters;
TexData.TexRefLon = RendData->TexRefLon;
TexData.TexRefLat = RendData->TexRefLat;
TexData.TexRefElev = RendData->TexRefElev;
Dox = CenterX - .5 * WidthInPixels;
Dex = CenterX + .5 * WidthInPixels;
Doy = CenterY - .5 * HeightInPixels;
Dey = CenterY + .5 * HeightInPixels;

DxStart = quicklongfloor(Dox); 
DyStart = quicklongfloor(Doy); 
DCols = quickftol(WCS_ceil(Dex) - WCS_floor(Dox));
DRows = quickftol(WCS_ceil(Dey) - WCS_floor(Doy));

for (Y = DyStart, j = 0;
	j < DRows; j ++, Y ++)
	{
	if (Y < 0)
		continue;
	if (Y >= RendData->Height)
		break;
	PixZip = Y * RendData->Width + DxStart;
	for (X = DxStart, i = 0; i < DCols;
		i ++, X ++, PixZip ++)
		{
		if (X < 0)
			continue;
		if (X >= RendData->Width)
			break;
		if (FragBlock || (ZFlt < ZBuf[PixZip] || AABuf[PixZip] < 255))
			{
			SumEdgeWt = 0.0;
			SumBodyWt = 1.0;
			// Plot pixel

			if (SumEdgeWt > 0.0 || SumBodyWt > 0.0)
				{
				// set texture data
				TexData.Elev = ElevBuf ? ElevBuf[PixZip]: 0.0;
				TexData.RelElev = RelElBuf ? RelElBuf[PixZip]: 0.0f;
				TexData.Slope = SlopeBuf ? SlopeBuf[PixZip]: 0.0;
				TexData.Aspect = AspectBuf ? AspectBuf[PixZip]: 0.0;
				TexData.Illumination = IllumBuf ? IllumBuf[PixZip]: 0.0;
				TexData.Object = ObjectBuf ? ObjectBuf[PixZip]: NULL;
				TexData.ObjectType = ObjTypeBuf ? ObjTypeBuf[PixZip]: NULL;
				TexData.Latitude = LatBuf ? LatBuf[PixZip] + RendData->TexRefLat: 0.0;
				TexData.Longitude = LonBuf ? LonBuf[PixZip] + RendData->TexRefLon: 0.0;
				TexData.Reflectivity = ReflectionBuf ? ReflectionBuf[PixZip]: 0.0;
				TexData.Normal[0] = NormalBuf[0] ? NormalBuf[0][PixZip]: 0.0;
				TexData.Normal[1] = NormalBuf[1] ? NormalBuf[1][PixZip]: 0.0;
				TexData.Normal[2] = NormalBuf[2] ? NormalBuf[2][PixZip]: 0.0;
				TexData.ZDist = ZBuf ? ZBuf[PixZip]: 0.0;
				TexData.QDist = TexData.ZDist;

				TexData.PixelX[0] = (double)X + RendData->Rend->SegmentOffsetX - Width * RendData->Cam->PanoPanels * .5;
				TexData.PixelX[1] = (double)(X + 1) + RendData->Rend->SegmentOffsetX - Width * RendData->Cam->PanoPanels * .5;
				TexData.PixelY[0] = -((double)Y + RendData->Rend->SegmentOffsetY) + Height * RendData->Opt->RenderImageSegments * .5;
				TexData.PixelY[1] = -((double)(Y + 1) + RendData->Rend->SegmentOffsetY) + Height * RendData->Opt->RenderImageSegments * .5;
				TexData.PixelUnityX[0] = TexData.PixelX[0] / (Width * RendData->Cam->PanoPanels);
				TexData.PixelUnityX[1] = TexData.PixelX[1] / (Width * RendData->Cam->PanoPanels);
				TexData.PixelUnityY[0] = TexData.PixelY[0] / (Height * RendData->Opt->RenderImageSegments);
				TexData.PixelUnityY[1] = TexData.PixelY[1] / (Height * RendData->Opt->RenderImageSegments);
				//TexData.PixelX[0] = X - Width * .5;
				//TexData.PixelX[1] = X + 1 - Width * .5;
				//TexData.PixelY[0] = -Y + Height * .5;
				//TexData.PixelY[1] = -Y - 1 + Height * .5;
				//TexData.PixelUnityX[0] = TexData.PixelX[0] / Width;
				//TexData.PixelUnityX[1] = TexData.PixelX[1] / Width;
				//TexData.PixelUnityY[0] = TexData.PixelY[0] / Height;
				//TexData.PixelUnityY[1] = TexData.PixelY[1] / Height;
				VertDEM.ScrnXYZ[0] = Col + .5; 
				VertDEM.ScrnXYZ[1] = Row + .5; 
				//TexData.LowX = X;
				//TexData.HighX = X + 1;
				//TexData.LowY = Y;
				//TexData.HighY = Y + 1;
				//TexData.LowZ = TexData.ZDist;
				//TexData.HighZ = TexData.ZDist;
				//TexData.LatRange[0] = TexData.LatRange[1] = TexData.LatRange[2] = TexData.LatRange[3] = TexData.Latitude;
				//TexData.LonRange[0] = TexData.LonRange[1] = TexData.LonRange[2] = TexData.LonRange[3] = TexData.Longitude;
				if (RendData->Cam && RendData->Cam->TargPos)
					{
					TexData.CamAimVec[0] = -RendData->Cam->TargPos->XYZ[0];
					TexData.CamAimVec[1] = -RendData->Cam->TargPos->XYZ[1];
					TexData.CamAimVec[2] = -RendData->Cam->TargPos->XYZ[2];
					} // if
				//if (TexCoordType == WCS_POSTPROCEVENT_COORDTYPE_IMAGE)
				//	{
				//	TexData.LowX /= Width;
				//	TexData.LowX -= .5;
				//	TexData.HighX /= Width;
				//	TexData.HighX -= .5;
				//	TexData.LowY /= -Height;
				//	TexData.LowY += .5;
				//	TexData.HighY /= -Height;
				//	TexData.HighY += .5;
				//	} // if
				//else
				//	{
				//	TexData.LowX -= Width * .5;
				//	TexData.HighX -= Width * .5;
				//	TexData.LowY = -TexData.LowY + Height * .5;
				//	TexData.HighY = -TexData.HighY + Height * .5;
				//	} // else
				if (FragBlock)
					{
					// set fragment texture data
					if (CurFrag = FragMap[PixZip].FragList)
						{
						TexData.ZDist = CurFrag->ZBuf;
						TexData.QDist = TexData.ZDist;
						} // if
					} // if

				// this sets up XYZ and lat lon coords for alternate texture coordinate spaces
				VertDEM.ScrnXYZ[2] = VertDEM.Q = TexData.ZDist > 0.0 ? TexData.ZDist: 1.0;
				RendData->Cam->UnProjectVertexDEM(RendData->DefCoords, &VertDEM, RendData->EarthLatScaleMeters, RendData->PlanetRad, 1);

				// get existing values
				if (CurFrag)
					{
					CurFrag->ExtractColor(Value);
					TexData.RGB[0] = TexColor[0] = OrigValue[0] = Value[0];
					TexData.RGB[1] = TexColor[1] = OrigValue[1] = Value[1];
					TexData.RGB[2] = TexColor[2] = OrigValue[2] = Value[2];
					} // if
				else
					{
					if (ExponentBuf)
						{
						RendData->Rend->GetPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
						TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0];
						TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1];
						TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2];
						} // if
					else
						{
						TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
						TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
						TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
						} // else
					} // if

				// evaluate texture intensity
				if ((Intensity = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].CurValue) > 0.0)
					{
					if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Enabled)
						{
						if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Eval(Value, &TexData)) > 0.0)
							{
							if (TexOpacity < 1.0)
								{
								// Value[0] has already been diminished by the texture's opacity
								Intensity *= (1.0 - TexOpacity + Value[0]);
								} // if
							else
								Intensity *= Value[0];
							} // if
						} // if
					} // if

				if (Intensity > 0.0)
					{
					if (SumBodyWt > 0.0)
						{
						Pix.RGB[0] = TexData.RGB[0];
						Pix.RGB[1] = TexData.RGB[1];
						Pix.RGB[2] = TexData.RGB[2];
						TexData.TLowX = -.5 + (double)i / (double)DCols;
						TexData.THighX = -.5 + (double)(i + 1) / DCols;
						TexData.TLowY = -.5 + (1.0 - ((double)j / (double)DRows));
						TexData.THighY = -.5 + (1.0 - ((double)(j + 1) / (double)DRows));
						TexData.TLowZ = TexData.ZDist;
						TexData.THighZ = TexData.ZDist;
						if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled)
							{
							if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Eval(Value, &TexData)) > 0.0)
								{
								if (TexOpacity < 1.0)
									{
									TexOpacity = 1.0 - TexOpacity;
									Pix.RGB[0] = Pix.RGB[0] * TexOpacity + Value[0];
									Pix.RGB[1] = Pix.RGB[1] * TexOpacity + Value[1];
									Pix.RGB[2] = Pix.RGB[2] * TexOpacity + Value[2];
									} // if
								else
									{
									Pix.RGB[0] = Value[0];
									Pix.RGB[1] = Value[1];
									Pix.RGB[2] = Value[2];
									} // else
								} // if
							} // if
						if (SumEdgeWt > 0.0)
							{
							InvIntensity = SumBodyWt / (SumEdgeWt + SumBodyWt);
							Pix.RGB[0] *= InvIntensity;
							Pix.RGB[1] *= InvIntensity;
							Pix.RGB[2] *= InvIntensity;
							} // if
						} // if
					else
						{
						Pix.RGB[0] = Pix.RGB[1] = Pix.RGB[2] = 0.0;
						} // else

					SumBodyWt += SumEdgeWt;
					//SumBodyWt /= MaxWt;
					if (SumBodyWt > 1.0)
						SumBodyWt = 1.0;


					Intensity *= SumBodyWt;
					NewBits = (unsigned short)(Intensity * 255.9);
					if (FragBits = NewBits)
						{
						if (FragBlock)
							{
							if (FragBits > 255)
								FragBits = 255;
							if (! (PixFrag = FragMap[PixZip].PlotPixel(FragBlock, ZFlt, (unsigned char)FragBits, ~0UL, ~0UL, RendData->Rend->FragmentDepth + 1, 
								(unsigned char)WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC)))
								{
								continue;
								} // if
							} // if
						else
							{
							OldBits = AABuf[PixZip];
							if (ZFlt >= ZBuf[PixZip])					// new pixel farther than old pixel
								{
								if (NewBits + OldBits > 255)
									NewBits = 255 - OldBits;
								if (! NewBits)
									continue;
								} // if
							else
								{
								if (NewBits + OldBits > 255)
									OldBits = 255 - NewBits;
								} // else
							TotalBits = NewBits + OldBits;
							Intensity = (double)NewBits / (double)TotalBits;
							} // if ! FragBlock

						if (IlluminateText || (ApplyVolumetrics && RendData->Rend->VolumetricAtmospheresExist))
							{
							PixelVtx.ScrnXYZ[0] = X + .5;
							PixelVtx.ScrnXYZ[1] = Y + .5;
							PixelVtx.ScrnXYZ[2] = PixelVtx.Q = ZFlt;
							RendData->Cam->UnProjectVertexDEM(RendData->DefCoords, &PixelVtx, RendData->EarthLatScaleMeters, RendData->PlanetRad, 1);
							Pix.XYZ[0] = PixelVtx.XYZ[0];
							Pix.XYZ[1] = PixelVtx.XYZ[1];
							Pix.XYZ[2] = PixelVtx.XYZ[2];
							Pix.Q = PixelVtx.Q;
							Pix.Elev = PixelVtx.Elev;
							// view vector is used for specularity and for that it is best to have it point toward the camera
							Pix.ViewVec[0] = RendData->Cam->CamPos->XYZ[0] - Pix.XYZ[0];
							Pix.ViewVec[1] = RendData->Cam->CamPos->XYZ[1] - Pix.XYZ[1];
							Pix.ViewVec[2] = RendData->Cam->CamPos->XYZ[2] - Pix.XYZ[2];
							UnitVector(Pix.ViewVec); // Illumination code requires this to be unitized in advance to save time later
							if (IlluminateText)
								{
								// make surface normal point toward camera
								Pix.Normal[0] = Pix.ViewVec[0];
								Pix.Normal[1] = Pix.ViewVec[1];
								Pix.Normal[2] = Pix.ViewVec[2];
								// needs to be normalized since it will be put in Normal XYZ buffers
								//UnitVector(Pix.Normal); // no longer necessary, since ViewVec is pre-normalized
								// modify normal based on vertical and horizontal position of pixel in foliage image
								if (RendData->Rend->LightTexturesExist)
									RendData->TransferTextureData(&PixelVtx);
								RendData->Rend->IlluminatePixel(&Pix);
								} // if
							} // if

						ReplaceValues = 0;

						if (! FragBlock)
							{
							if (UpdateDiagnostics)					// new tree farther than old tree
								{
								AABuf[PixZip] = (unsigned char)TotalBits;
								} // else ZFlt
							if (ZFlt < ZBuf[PixZip])
								{
								ReplaceValues = 1;
								} // if
							} // if
						else
							{
							ReplaceValues = (ZFlt <= FragMap[PixZip].GetFirstZ());
							} // else

						// plot data
						if (PixFrag)
							{
							PixFrag->PlotPixel(FragBlock, Pix.RGB, Pix.Reflectivity, Pix.Normal);
							RendData->Rend->ScreenPixelPlotFragments(&FragMap[PixZip], X + RendData->Rend->DrawOffsetX, Y + RendData->Rend->DrawOffsetY);
							} // if
						else
							{
							if (RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
								{
								RGBtoHSV(UnModHSV, OrigValue);
								RGBtoHSV(ModHSV, Pix.RGB);
								if (AffectRed)
									UnModHSV[0] = ModHSV[0];
								if (AffectGrn)
									UnModHSV[1] = ModHSV[1];
								if (AffectBlu)
									UnModHSV[2] = ModHSV[2];
								HSVtoRGB(UnModHSV, Pix.RGB);
								} // if

							// lerp output values
							InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
							if (ExponentBuf && UpdateDiagnostics)
								{
								if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[0] = (OrigValue[0] * InvIntensity + Pix.RGB[0] * Intensity);
								if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[1] = (OrigValue[1] * InvIntensity + Pix.RGB[1] * Intensity);
								if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									OrigValue[2] = (OrigValue[2] * InvIntensity + Pix.RGB[2] * Intensity);
								RendData->Rend->PlotPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
								} // if
							else
								{
								Intensity *= 255.0;
								if (Pix.RGB[0] > 1.0)
									Pix.RGB[0] = 1.0;
								if (Pix.RGB[1] > 1.0)
									Pix.RGB[1] = 1.0;
								if (Pix.RGB[2] > 1.0)
									Pix.RGB[2] = 1.0;
								if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[0][PixZip] = (unsigned char)(Bitmap[0][PixZip] * InvIntensity + Pix.RGB[0] * Intensity);
								if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[1][PixZip] = (unsigned char)(Bitmap[1][PixZip] * InvIntensity + Pix.RGB[1] * Intensity);
								if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
									Bitmap[2][PixZip] = (unsigned char)(Bitmap[2][PixZip] * InvIntensity + Pix.RGB[2] * Intensity);
								} // else
							} // else

						if (UpdateDiagnostics && ReplaceValues)
							{
							if (! FragBlock && ZBuf)
								ZBuf[PixZip] = ZFlt;
							if (LatBuf)
								LatBuf[PixZip] = (float)(PixelVtx.Lat - RendData->Rend->TexRefLat);
							if (LonBuf)
								LonBuf[PixZip] = (float)(PixelVtx.Lon - RendData->Rend->TexRefLon);
							if (IllumBuf)
								IllumBuf[PixZip] = (float)Pix.Illum;
							if (RelElBuf)
								RelElBuf[PixZip] = 0.0f;
							if (SlopeBuf)
								SlopeBuf[PixZip] = 0.0f;
							if (AspectBuf)
								AspectBuf[PixZip] = 0.0f;
							if (FragBlock || (! ReflectionBuf || ! ReflectionBuf[PixZip] || TotalBits >= 255))
								{
								if (NormalBuf[0])
									NormalBuf[0][PixZip] = (float)Pix.Normal[0];
								if (NormalBuf[1])
									NormalBuf[1][PixZip] = (float)Pix.Normal[1];
								if (NormalBuf[2])
									NormalBuf[2][PixZip] = (float)Pix.Normal[2];
								} // if
							if (ObjectBuf)
								ObjectBuf[PixZip] = Poly->Object;
							if (ObjTypeBuf)
								ObjTypeBuf[PixZip] = WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC;
							if (ElevBuf)
								ElevBuf[PixZip] = (float)PixelVtx.Elev;
							} // if

						} // if NewBits
					} // if intensity
				} // if PixWt
			} // if
		} // for x
	if (BWDE)
		{
		if (BWDE)
			{
			if (BWDE->Update(Row + 1))
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else if (Master)
		{
		Master->ProcUpdate(Row + 1);
		if (! Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for y

// invoke cleanup on derived class
CleanupPostProc(RendData, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
/*** F2 MOD
CleanupPostProc(RendData, Buffers, FragBlock, (long)(Width + 0.5f), (long)(Height + 0.5f), BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
***/

return (Success);

} // PostProcImage::PlotImage

/*===========================================================================*/

ULONG PostProcImage::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCTEXT_ILLUMINATE:
						{
						BytesRead = ReadBlock(ffile, (char *)&Illuminate, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCTEXT_APPLYVOLUMETRICS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ApplyVolumetrics, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCTEXT_RECEIVESHADOWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ReceiveShadows, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 
return (TotalRead);

} // PostProcImage::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcImage::SaveSpecificData(FILE *ffile)
{

ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_ILLUMINATE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Illuminate)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_APPLYVOLUMETRICS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ApplyVolumetrics)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCTEXT_RECEIVESHADOWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ReceiveShadows)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcImage::SaveSpecificData

/*===========================================================================*/

void PostProcImage::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcImage *CopyTo, *CopyFrom;

CopyTo = (PostProcImage *)CopyToPP;
CopyFrom = (PostProcImage *)CopyFromPP;

CopyTo->Illuminate = CopyFrom->Illuminate;
CopyTo->ApplyVolumetrics = CopyFrom->ApplyVolumetrics;
CopyTo->ReceiveShadows = CopyFrom->ReceiveShadows;

} // PostProcImage::CopySpecificData

/*===========================================================================*/

PostProcImage::PostProcImage(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{
RootTexture *Root;
Texture *Tex;
double EffectDefault[WCS_POSTPROCEVENT_NUMANIMPAR] = {.5, .5, 1.0, 1.0, 1.0, .25, .75, 1.0, 1.0, 1.0, 1.0};
double RangeDefaults[WCS_POSTPROCEVENT_NUMANIMPAR][3] = {10000.0, -10000.0, .01,
								10000.0, -10000.0, .01,
								10000.0, 0.0, .01,
								10000.0, 0.0, .01,
								FLT_MAX, .01, 1.0,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								1.0, 0.0, .01,
								};

strcpy(Name, PostProcEventNames[GetType()]);

AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX].SetDefaults(this, (char)1, EffectDefault[0]);	// Center X
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX].SetRangeDefaults(RangeDefaults[0]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX].SetMultiplier(100.0);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY].SetDefaults(this, (char)2, EffectDefault[1]);	// Center Y
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY].SetRangeDefaults(RangeDefaults[1]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY].SetMultiplier(100.0);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_WIDTH].SetDefaults(this, (char)3, EffectDefault[2]);	// Width %
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_WIDTH].SetRangeDefaults(RangeDefaults[2]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_WIDTH].SetMultiplier(100.0);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT].SetDefaults(this, (char)4, EffectDefault[3]);	// Height %
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT].SetRangeDefaults(RangeDefaults[3]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT].SetMultiplier(100.0);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE].SetDefaults(this, (char)5, EffectDefault[4]);	// Z Distance %
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE].SetRangeDefaults(RangeDefaults[4]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT].SetDefaults(this, (char)6, EffectDefault[5]);	// Back Light %
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT].SetRangeDefaults(RangeDefaults[5]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT].SetMultiplier(100.0);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY].SetDefaults(this, (char)7, EffectDefault[6]);	// Shadow Intensity %
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY].SetRangeDefaults(RangeDefaults[6]);
AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY].SetMultiplier(100.0);

if (Root = NewRootTexture(WCS_POSTPROCEVENT_TEXTURE_FILTER))
	{
	// set texture type to planar image
	if (Tex = Root->AddNewTexture(NULL, WCS_TEXTURE_TYPE_PLANARIMAGE))
		{
		// Don't set image yet
		Tex->SetMiscDefaults();
		Tex->SetCoordSpace(WCS_TEXTURE_COORDSPACE_IMAGE_UNITYSCALE_NOZ, TRUE);
		Tex->SelfOpacity = 1; // to allow for Alpha-driven transparency
		} // if
	} // if


Illuminate = 0;
ApplyVolumetrics = 0;
ReceiveShadows = 0;

} // PostProcImage::PostProcImage

/*===========================================================================*/

int PostProcImage::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
double BackLightingPct, PosX, PosY, FScaleX, FScaleY, RenderAspectWtoH, OverlayAspectWtoH;
PolygonData Poly;
Raster *LocalRast;
float ZFlt;
Success = 1;

if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled
 && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img
 && (LocalRast = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img->GetRaster()))
	{
	FScaleX = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].CurValue;
	FScaleY = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].CurValue;

	// if one of these widths or heights is ever zero, somebody should be shot :)
	RenderAspectWtoH = (double)Width * Rend->Cam->PanoPanels / (Height * Rend->Opt->RenderImageSegments);
	OverlayAspectWtoH = (double)LocalRast->Cols / LocalRast->Rows;
	if (RenderAspectWtoH > OverlayAspectWtoH)
		FScaleX *= (OverlayAspectWtoH / RenderAspectWtoH);
	else
		FScaleY *= (RenderAspectWtoH / OverlayAspectWtoH);

	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexSize[0].CurValue = FScaleX;
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexSize[1].CurValue = FScaleY;

	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexCenter[0].CurValue = (double)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].CurValue - .5;
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexCenter[1].CurValue = (double)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].CurValue - .5;

	PosX = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX].CurValue * Width * Rend->Cam->PanoPanels;
	PosY = Height * Rend->Opt->RenderImageSegments * (1.0 - AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY].CurValue);
	ZFlt = (float)AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE].CurValue;
	BackLightingPct = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT].CurValue;
	Poly.ShadowFlags = ReceiveShadows ? (~0UL): 0;
	Poly.ReceivedShadowIntensity = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY].CurValue;
	Poly.ShadowOffset = 0.0;
	Poly.Object = GetRAHostRoot();


	Success = PlotImage(Rend, Buffers, FragBlock, &Poly, LocalRast, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum,
		UpdateDiagnostics, PosX, PosY, Width * FScaleX, Height * FScaleY, BackLightingPct, Illuminate && UpdateDiagnostics, ApplyVolumetrics && UpdateDiagnostics, 
		ZFlt, NULL, NULL);
	} // if

return (Success);

/*** F2 MOD
Success = 1;
double BackLightingPct, PosX, PosY, OutWidth, OutHeight;
float OverlayAspect, RenderAspect, XDim, YDim, ZFlt;
PolygonData Poly;
Raster *LocalRast;

if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled
 && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img
 && (LocalRast = TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img->GetRaster()))
	{
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexSize[0].CurValue = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4].CurValue;
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexSize[1].CurValue = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE5].CurValue;

	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexCenter[0].CurValue = (double)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2].CurValue - .5;
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->TexCenter[1].CurValue = (double)AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3].CurValue - .5;

	PosX = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX].CurValue * Width;
	PosY = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY].CurValue * Height;
	OutWidth = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_WIDTH].CurValue * LocalRast->Cols;
	OutHeight = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT].CurValue * LocalRast->Rows;
	ZFlt = (float)AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE].CurValue;
	BackLightingPct = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT].CurValue;
	Poly.ReceiveShadows = ReceiveShadows;
	Poly.ReceivedShadowIntensity = AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY].CurValue;
	Poly.ShadowOffset = 0.0;
	Poly.Object = GetRAHostRoot();

	// if one of these widths or heights is ever zero, somebody should be shot :)
	RenderAspect = (float)Width / Height;
	OverlayAspect = (float)(OutWidth / OutHeight);

	if (OverlayAspect >= RenderAspect)
		{
		// render width is the limiting factor
		XDim = (float)Width;
		YDim = (float)(OutHeight * (Width / OutWidth));
		}
	else
		{
		// render height is the limiting factor
		YDim = (float)Height;
		XDim = (float)(OutWidth * (Height / OutHeight));
		}

	Success = PlotImage(Rend, Buffers, FragBlock, &Poly, LocalRast, XDim, YDim, BWDE, Master, OptionalBitmaps, FrameNum,
		UpdateDiagnostics, PosX, PosY, Width, Height, BackLightingPct, Illuminate && UpdateDiagnostics, ApplyVolumetrics && UpdateDiagnostics, 
		ZFlt, NULL, NULL);
	} // if

return (Success);
***/

} // PostProcImage::RenderPostProc

/*===========================================================================*/

int PostProcImage::SetRaster(Raster *NewRast)
{
if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled
 && TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex)
	{
	TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->SetRaster(NewRast);
	return(1);
	} // if

return(0);
} // PostProcImage::SetRaster

/*===========================================================================*/
/*===========================================================================*/

PostProcComposite::PostProcComposite(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{

strcpy(Name, PostProcEventNames[GetType()]);
Img = NULL;
MyRend = NULL; 
MyBuffers = NULL; 
MyFragBlock = NULL; 
MyWidth = 0; 
MyHeight = 0; 
MyNumPanels = 1;
MyNumSegments = 1;
MyPanel = 0;
MySegment = 0;
MyBWDE = NULL; 
MyMaster = NULL; 
MyOptionalBitmaps = NULL; 
MyFrameNum = 0; 
MyUpdateDiagnostics = 0;


} // PostProcComposite::PostProcComposite

/*===========================================================================*/

PostProcComposite::~PostProcComposite(void)
{

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	// LINT thinks this test is redundant but it ain't - it can be NULLed by RemoveAttribute()
	if (Img)
		delete Img;
	Img = NULL;
	} // if
	
} // PostProcComposite::~PostProcComposite

/*===========================================================================*/

int PostProcComposite::SetRaster(Raster *NewRast)
{
NotifyTag Changes[2];

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	} // if
if (! Img)
	{
	Img = new RasterShell;
	} // else
if (Img && NewRast)
	{
	NewRast->AddAttribute(Img->GetType(), Img, this);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	return (1);
	} // if
else
	{
	delete Img;		// delete it here since it wasn't added to a raster
	Img = NULL;
	return (0);
	} // else

} // PostProcComposite::SetRaster

/*===========================================================================*/

char *PostProcComposite::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Composite Post Process image! Remove anyway?");

} // PostProcComposite::OKRemoveRaster

/*===========================================================================*/

void PostProcComposite::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img == Shell)
	Img = NULL;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // PostProcComposite::RemoveRaster

/*===========================================================================*/

RasterAnimHost *PostProcComposite::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Found = 0;

if (! Current)
	Found = 1;
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());

	// set Current to NULL or PPE will never return anything because it doesn't recognize Raster
if (Img && Img->GetRaster() && Current == Img->GetRaster())
	Current = NULL;

return (PostProcessEvent::GetRAHostChild(Current, ChildTypeFilter));

} // PostProcComposite::GetRAHostChild

/*===========================================================================*/

int PostProcComposite::GetDeletable(RasterAnimHost *Test)
{

if (Img && Test == Img->GetRaster())
	return (1);

return (PostProcessEvent::GetDeletable(Test));

} // PostProcComposite::GetDeletable

/*===========================================================================*/

int PostProcComposite::RemoveRAHost(RasterAnimHost *RemoveMe)
{

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	return (1);
	} // if

return (PostProcessEvent::RemoveRAHost(RemoveMe));

} // PostProcComposite::RemoveRAHost

/*===========================================================================*/

int PostProcComposite::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

Success = 1;
CurFrag = NULL;

// if nothing to do
if (! (AffectRed || AffectGrn || AffectBlu))
	return (1);

Value[0] = Value[1] = Value[2] = 0.0;

// assign buffer ptrs - at least all the ones the renderer needs to know about
if (OptionalBitmaps)
	{
	Bitmap[0] = OptionalBitmaps[0];
	Bitmap[1] = OptionalBitmaps[1];
	Bitmap[2] = OptionalBitmaps[2];
	} // if
else
	{
	Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",					WCS_RASTER_BANDSET_BYTE);
	Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN",				WCS_RASTER_BANDSET_BYTE);
	Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
	} // else

if (FragBlock)
	MyFragMap = FragBlock->GetFragMap();
else
	MyFragMap = NULL;

if (! ((FragBlock && MyFragMap) || (Bitmap[0] && Bitmap[1] && Bitmap[2])))
	return (0);

ZBuf = (float *)Buffers->FindBuffer("ZBUF",								WCS_RASTER_BANDSET_FLOAT);
AABuf = (unsigned char *)Buffers->FindBuffer("ANTIALIAS",				WCS_RASTER_BANDSET_BYTE);
LatBuf = (float *)Buffers->FindBuffer("LATITUDE",						WCS_RASTER_BANDSET_FLOAT);
LonBuf = (float *)Buffers->FindBuffer("LONGITUDE",						WCS_RASTER_BANDSET_FLOAT);
ElevBuf = (float *)Buffers->FindBuffer("ELEVATION",						WCS_RASTER_BANDSET_FLOAT);
RelElBuf = (float *)Buffers->FindBuffer("RELATIVE ELEVATION",			WCS_RASTER_BANDSET_FLOAT);
ReflectionBuf = (float *)Buffers->FindBuffer("REFLECTION",				WCS_RASTER_BANDSET_FLOAT);
IllumBuf = (float *)Buffers->FindBuffer("ILLUMINATION",					WCS_RASTER_BANDSET_FLOAT);
SlopeBuf = (float *)Buffers->FindBuffer("SLOPE",						WCS_RASTER_BANDSET_FLOAT);
AspectBuf = (float *)Buffers->FindBuffer("ASPECT",						WCS_RASTER_BANDSET_FLOAT);
NormalBuf[0] = (float *)Buffers->FindBuffer("SURFACE NORMAL X",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[1] = (float *)Buffers->FindBuffer("SURFACE NORMAL Y",			WCS_RASTER_BANDSET_FLOAT);
NormalBuf[2] = (float *)Buffers->FindBuffer("SURFACE NORMAL Z",			WCS_RASTER_BANDSET_FLOAT);
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
ObjectBuf = (RasterAnimHost **)Buffers->FindBuffer("OBJECT",			WCS_RASTER_BANDSET_FLOAT);
ObjTypeBuf = (unsigned char *)Buffers->FindBuffer("OBJECT TYPE",		WCS_RASTER_BANDSET_BYTE);

TexData.VDEM[0] = &VertDEM;
TexData.MetersPerDegLat = Rend->EarthLatScaleMeters;
TexData.MetersPerDegLon = Rend->RefLonScaleMeters;
TexData.TexRefLon = Rend->TexRefLon;
TexData.TexRefLat = Rend->TexRefLat;
TexData.TexRefElev = Rend->TexRefElev;

// invoke prep on derived class, bail if it fails
// all of the fun stuff is done during Prep
if (! PrepForPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics))
	{
	CleanupPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);
	return(0);
	} // if

// invoke cleanup on derived class
CleanupPostProc(Rend, Buffers, FragBlock, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

return (Success);

} // PostProcComposite::RenderPostProc

/*===========================================================================*/

int PostProcComposite::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
MyRend = Rend; 
MyBuffers = Buffers; 
MyFragBlock = FragBlock; 
MyWidth = Width; 
MyHeight = Height; 
MyBWDE = BWDE; 
MyMaster = Master; 
MyOptionalBitmaps = OptionalBitmaps; 
MyFrameNum = FrameNum; 
MyUpdateDiagnostics = UpdateDiagnostics;
MyPanel = Rend->PanoPanel;
MySegment = Rend->Segment;
MyNumPanels = Rend->NumPanels;
MyNumSegments = Rend->NumSegments;

// load image
if (Img && Img->GetRaster())
	{
	// call some special method to load the image that turns around and calls EvalOneRLASample() on (this) 
	// currently this is just the basic image loader
	Img->GetRaster()->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
	if (! Img->GetRaster()->LoadToComposite(GlobalApp->MainProj->Interactive->GetActiveTime(), 
		GlobalApp->MainProj->Interactive->GetFrameRate(), this, Width, Height))
		{
		Img->GetRaster()->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
		return (0);
		} // if
	Img->GetRaster()->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
	} // if

return(1);

} // PostProcComposite::PrepForPostProc

/*===========================================================================*/

int PostProcComposite::EvalPostProcSample(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragBlock, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{

// this method should do nothing but I fear that it will be called anyway for every pixel in the image

return(1);

} // PostProcComposite::EvalPostProcSample

/*===========================================================================*/

// this is where the pixel is processed
int PostProcComposite::EvalOneRLASample(RLASampleData *Samp)
{
double ModHSV[3], CompReflect, UnModHSV[3], ScaleWidth, CompNormal[3], ScaleHeight;
float DummyZ, DummyReflect, DummyNormal[3];
unsigned long TopCovg, BotCovg;
int ReplaceValues;
unsigned char Alpha;

CurFrag = NULL;
CompReflect = 0.0;
CompNormal[0] = CompNormal[1] = CompNormal[2] = 0.0;

// bounds check the sample's X and Y
// convert the sample X Y to the current render X Y taking into account multi-segmentation and multi-panel panoramas
if (Img && Img->GetRaster() && Samp->Alpha)
	{
	ScaleWidth = (double)(MyWidth * MyNumPanels) / Samp->ImageWidth;
	ScaleHeight = (double)(MyHeight * MyNumSegments) / Samp->ImageHeight;
	Col = quicklongfloor(.5 + Samp->X * ScaleWidth - MyPanel * MyWidth);
	Row = quicklongfloor(.5 + Samp->Y * ScaleHeight - MySegment * MyHeight);
	if (Col >= 0 && Col < MyWidth && Row >= 0 && Row < MyHeight)
		{
		PixZip = Row * MyWidth + Col;

		// evaluate intensity texture
		// set texture data
		TexData.Elev = ElevBuf ? ElevBuf[PixZip]: 0.0;
		TexData.RelElev = RelElBuf ? RelElBuf[PixZip]: 0.0f;
		TexData.Slope = SlopeBuf ? SlopeBuf[PixZip]: 0.0;
		TexData.Aspect = AspectBuf ? AspectBuf[PixZip]: 0.0;
		TexData.Illumination = IllumBuf ? IllumBuf[PixZip]: 0.0;
		TexData.Object = ObjectBuf ? ObjectBuf[PixZip]: NULL;
		TexData.ObjectType = ObjTypeBuf ? ObjTypeBuf[PixZip]: NULL;
		TexData.Latitude = LatBuf ? LatBuf[PixZip] + MyRend->TexRefLat: 0.0;
		TexData.Longitude = LonBuf ? LonBuf[PixZip] + MyRend->TexRefLon: 0.0;
		TexData.Reflectivity = ReflectionBuf ? ReflectionBuf[PixZip]: 0.0;
		TexData.Normal[0] = NormalBuf[0] ? NormalBuf[0][PixZip]: 0.0;
		TexData.Normal[1] = NormalBuf[1] ? NormalBuf[1][PixZip]: 0.0;
		TexData.Normal[2] = NormalBuf[2] ? NormalBuf[2][PixZip]: 0.0;
		TexData.ZDist = ZBuf ? ZBuf[PixZip]: 0.0;
		TexData.QDist = TexData.ZDist;

		TexData.PixelX[0] = Col - MyWidth * .5;
		TexData.PixelX[1] = Col + 1 - MyWidth * .5;
		TexData.PixelY[0] = -Row + MyHeight * .5;
		TexData.PixelY[1] = -Row - 1 + MyHeight * .5;
		TexData.PixelUnityX[0] = TexData.PixelX[0] / MyWidth;
		TexData.PixelUnityX[1] = TexData.PixelX[1] / MyWidth;
		TexData.PixelUnityY[0] = TexData.PixelY[0] / MyHeight;
		TexData.PixelUnityY[1] = TexData.PixelY[1] / MyHeight;
		VertDEM.ScrnXYZ[0] = Col + .5; 
		VertDEM.ScrnXYZ[1] = Row + .5; 
		if (MyRend->Cam && MyRend->Cam->TargPos)
			{
			TexData.CamAimVec[0] = -MyRend->Cam->TargPos->XYZ[0];
			TexData.CamAimVec[1] = -MyRend->Cam->TargPos->XYZ[1];
			TexData.CamAimVec[2] = -MyRend->Cam->TargPos->XYZ[2];
			} // if

		// this sets up XYZ and lat lon coords for alternate texture coordinate spaces
		VertDEM.ScrnXYZ[2] = VertDEM.Q = TexData.ZDist > 0.0 ? TexData.ZDist: 1.0;
		MyRend->Cam->UnProjectVertexDEM(MyRend->DefCoords, &VertDEM, MyRend->EarthLatScaleMeters, MyRend->PlanetRad, 1);
		// get existing values
		if (MyFragMap)
			{
			MyFragMap[PixZip].CollapsePixel(OrigValue, DummyZ, Alpha, DummyReflect, DummyNormal);
			TexData.RGB[0] = TexColor[0] = OrigValue[0];
			TexData.RGB[1] = TexColor[1] = OrigValue[1];
			TexData.RGB[2] = TexColor[2] = OrigValue[2];
			} // if
		else
			{
			if (ExponentBuf)
				{
				MyRend->Rend->GetPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
				TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0];
				TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1];
				TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2];
				} // if
			else
				{
				TexData.RGB[0] = Value[0] = TexColor[0] = OrigValue[0] = Bitmap[0][PixZip] * (1.0 / 255.0);
				TexData.RGB[1] = Value[1] = TexColor[1] = OrigValue[1] = Bitmap[1][PixZip] * (1.0 / 255.0);
				TexData.RGB[2] = Value[2] = TexColor[2] = OrigValue[2] = Bitmap[2][PixZip] * (1.0 / 255.0);
				} // else
			} // if

		// evaluate texture intensity
		if ((Intensity = AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY].CurValue) > 0.0)
			{
			if (TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY] && TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Enabled)
				{
				if ((TexOpacity = TexRoot[WCS_POSTPROCEVENT_TEXTURE_INTENSITY]->Eval(Value, &TexData)) > 0.0)
					{
					if (TexOpacity < 1.0)
						{
						// Value[0] has already been diminished by the texture's opacity
						Intensity *= (1.0 - TexOpacity + Value[0]);
						} // if
					else
						Intensity *= Value[0];
					} // if
				} // if
			} // if

		if (Intensity > 0.0)
			{
			// translate fragment coverage if the channel exists
			if (Samp->Channels & BMM_CHAN_MASK)
				{
				TranslateRPFMaskToPixelFrags(Samp->mask, TopCovg, BotCovg);
				} // if
			else
				TopCovg = BotCovg = ~0UL;
			if (MyFragMap)
				{
				Alpha = Samp->Alpha;
				// make the fragment depth arbitrarily large to ensure the fragment gets plotted.
				// all composite fragments have RenderOccluded flag bit turned on.
				CurFrag = MyFragMap[PixZip].PlotPixel(MyFragBlock, Samp->Z, (unsigned char)(Alpha * Intensity), TopCovg, BotCovg, MyRend->Rend->FragmentDepth + 1000, 
					(unsigned char)(WCS_PIXELFRAG_FLAGBIT_OBJECTTYPE_POSTPROC | WCS_PIXELFRAG_FLAGBIT_RENDEROCCLUDED));
				} // if
			else
				{
				Alpha = (unsigned char)((CountBits(TopCovg) + CountBits(BotCovg)) * Samp->Alpha * (1.0 / 64.0));
				} // else
			Intensity = Intensity * Alpha * (1.0 / 255.0);
			if (! MyFragMap || CurFrag)
				{
				// put color in TexColor
				TexColor[0] = Samp->color[0] * (1.0 / 255.0);
				TexColor[1] = Samp->color[1] * (1.0 / 255.0);
				TexColor[2] = Samp->color[2] * (1.0 / 255.0);

				if (RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
					{
					RGBtoHSV(UnModHSV, OrigValue);
					RGBtoHSV(ModHSV, TexColor);
					if (AffectRed)
						UnModHSV[0] = ModHSV[0];
					if (AffectGrn)
						UnModHSV[1] = ModHSV[1];
					if (AffectBlu)
						UnModHSV[2] = ModHSV[2];
					HSVtoRGB(UnModHSV, TexColor);
					} // if

				// lerp output values
				InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
				ReplaceValues = 0;
				if (MyFragMap && CurFrag)
					{
					if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[0] = TexColor[0];
					if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[1] = TexColor[1];
					if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[2] = TexColor[2];
					if (Samp->Z < MyFragMap[PixZip].GetFirstZ())
						ReplaceValues = 1;
					} // if
				else if (ExponentBuf && MyUpdateDiagnostics)
					{
					if (Samp->Z < ZBuf[PixZip])
						ReplaceValues = 1;
					else
						{
						Intensity *= (1.0 - (AABuf[PixZip] / 255.0));
						if (Intensity < 0.0)
							Intensity = 0.0;
						InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
						} // else
					if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[0] = OrigValue[0] * InvIntensity + TexColor[0] * Intensity;
					if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[1] = OrigValue[1] * InvIntensity + TexColor[1] * Intensity;
					if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
						OrigValue[2] = OrigValue[2] * InvIntensity + TexColor[2] * Intensity;
					} // if
				if (MyFragMap)
					{
					if (CurFrag)
						CurFrag->PlotPixel(MyFragBlock, OrigValue, CompReflect, CompNormal);
					} // if
				else if (ExponentBuf && MyUpdateDiagnostics)
					{
					MyRend->Rend->PlotPixelWithExponent(Bitmap, ExponentBuf, PixZip, OrigValue);
					} // if
				else
					{
					if (Samp->Z >= ZBuf[PixZip])
						{
						Intensity *= (1.0 - (AABuf[PixZip] / 255.0));
						if (Intensity < 0.0)
							Intensity = 0.0;
						InvIntensity = Intensity >= 1.0 ? 0.0: Intensity <= 0.0 ? 1.0: 1.0 - Intensity;
						} // else
					if (Intensity > 0.0)
						{
						Intensity *= 255.0;
						if (TexColor[0] > 1.0)
							TexColor[0] = 1.0;
						if (TexColor[1] > 1.0)
							TexColor[1] = 1.0;
						if (TexColor[2] > 1.0)
							TexColor[2] = 1.0;
						if (AffectRed || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
							Bitmap[0][PixZip] = (unsigned char)(Bitmap[0][PixZip] * InvIntensity + TexColor[0] * Intensity);
						if (AffectGrn || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
							Bitmap[1][PixZip] = (unsigned char)(Bitmap[1][PixZip] * InvIntensity + TexColor[1] * Intensity);
						if (AffectBlu || RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
							Bitmap[2][PixZip] = (unsigned char)(Bitmap[2][PixZip] * InvIntensity + TexColor[2] * Intensity);
						} // if
					} // else
				if (ReplaceValues)
					{
					ZBuf[PixZip] = Samp->Z;
					AABuf[PixZip] = (AABuf[PixZip] + Alpha) > 255 ? 255: AABuf[PixZip] + Alpha;
					if (ObjectBuf)
						ObjectBuf[PixZip] = this;
					} // if
				} // if
			} // if intensity
		} // if
	} // if

return(1);

} // PostProcComposite::EvalOneRLASample

/*===========================================================================*/

void PostProcComposite::TranslateRPFMaskToPixelFrags(unsigned short InputMask, unsigned long &TopCovg, unsigned long &BotCovg)
{

TopCovg = BotCovg = 0;

// input mask is by row left to right bottom to top, 4 bits per row, 4 rows
if (InputMask & (1 << 15))
	{
	BotCovg |= (3 << 6) | (3 << 14);
	} // if
if (InputMask & (1 << 14))
	{
	BotCovg |= (3 << 4) | (3 << 12);
	} // if
if (InputMask & (1 << 13))
	{
	BotCovg |= (3 << 2) | (3 << 10);
	} // if
if (InputMask & (1 << 12))
	{
	BotCovg |= (3 << 0) | (3 << 8);
	} // if

if (InputMask & (1 << 11))
	{
	BotCovg |= (3 << 22) | (3u << 30);
	} // if
if (InputMask & (1 << 10))
	{
	BotCovg |= (3 << 20) | (3 << 28);
	} // if
if (InputMask & (1 << 9))
	{
	BotCovg |= (3 << 18) | (3 << 26);
	} // if
if (InputMask & (1 << 8))
	{
	BotCovg |= (3 << 16) | (3 << 24);
	} // if

if (InputMask & (1 << 7))
	{
	TopCovg |= (3 << 6) | (3 << 14);
	} // if
if (InputMask & (1 << 6))
	{
	TopCovg |= (3 << 4) | (3 << 12);
	} // if
if (InputMask & (1 << 5))
	{
	TopCovg |= (3 << 2) | (3 << 10);
	} // if
if (InputMask & (1 << 4))
	{
	TopCovg |= (3 << 0) | (3 << 8);
	} // if

if (InputMask & (1 << 3))
	{
	TopCovg |= (3 << 22) | (3u << 30);
	} // if
if (InputMask & (1 << 2))
	{
	TopCovg |= (3 << 20) | (3 << 28);
	} // if
if (InputMask & (1 << 1))
	{
	TopCovg |= (3 << 18) | (3 << 26);
	} // if
if (InputMask & (1 << 0))
	{
	TopCovg |= (3 << 16) | (3 << 24);
	} // if

} // PostProcComposite::TranslateRPFMaskToPixelFrags

/*===========================================================================*/

ULONG PostProcComposite::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCCOMPOSITE_IMAGEID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						if (ImageID > 0 && (Img = new RasterShell))
							{
							GlobalApp->LoadToImageLib->MatchRasterSetShell(ImageID, Img, this);
							} // if
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 
return (TotalRead);

} // PostProcComposite::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcComposite::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long ImageID;

if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCCOMPOSITE_IMAGEID, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&ImageID)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcComposite::SaveSpecificData

/*===========================================================================*/

void PostProcComposite::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcComposite *CopyTo, *CopyFrom;
Raster *NewRast;

CopyTo = (PostProcComposite *)CopyToPP;
CopyFrom = (PostProcComposite *)CopyFromPP;

if (CopyTo->Img)
	{
	if (CopyTo->Img->GetRaster())
		{
		CopyTo->Img->GetRaster()->RemoveAttribute(CopyTo->Img);
		} // if
	if (CopyTo->Img)
		delete CopyTo->Img;
	} // if
CopyTo->Img = NULL;
if (CopyFrom->Img)
	{
	if (CopyTo->Img = new RasterShell())
		{
		if (CopyFrom->Img->GetRaster())
			{
			if (NewRast = GlobalApp->CopyToImageLib->MatchNameMakeRaster(CopyFrom->Img->GetRaster()))
				{
				NewRast->AddAttribute(CopyTo->Img->GetType(), CopyTo->Img, CopyTo);
				} // if
			} // if
		} // if
	} // if

} // PostProcComposite::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

PostProcHeading::PostProcHeading(RasterAnimHost *RAHost)
: PostProcessEvent(RAHost)
{

strcpy(Name, PostProcEventNames[GetType()]);

abuf = bbuf = gbuf = rbuf = NULL;
lines[2] = lines[1] = lines[0] = NULL;
sel_letters = 1;
sel_numbers = 2;
sel_numstyle = 0;
sel_ticks = 3;
scale1 = 1;
scale5 = 1;
letters = 1;
numbers = 1;
ticks = 1;

} // PostProcHeading::PostProcHeading

/*===========================================================================*/

int PostProcHeading::CleanupPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap,
									  long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
									  long FrameNum, int UpdateDiagnostics)
{

if (abuf)
	AppMem_Free(abuf, rsize);

if (bbuf)
	AppMem_Free(bbuf, rsize);

if (gbuf)
	AppMem_Free(gbuf, rsize);

if (rbuf)
	AppMem_Free(rbuf, rsize);

if (lines[2])
	AppMem_Free(lines[2], Width);

if (lines[1])
	AppMem_Free(lines[1], Width);

if (lines[0])
	AppMem_Free(lines[0], Width);

return (1);

} // PostProcHeading::CleanupPostProc

/*===========================================================================*/

void PostProcHeading::ComputeTick(const double angle, unsigned char intensity, bool& plot_l, bool& plot_r)
{
double halftick = 1.0 / 6.0, tick = 1.0 / 3.0;	// tick sizes as fraction of a degree (keep in sync!)
double tmp, tmp2, tmp3, tmp4;
bool rval = false;
unsigned char rl, rr, gl, gr, bl, br;

tmp = fmod(pixel_left + halftick, angle);
if (tmp <= tick)
	{
	plot_l = true;
	bl = gl = rl = intensity;
	} // if
tmp2 = fmod(pixel_right + halftick, angle);
if (tmp2 <= tick)
	{
	plot_r = true;
	br = gr = rr = intensity;
	} // if
if (!plot_l && !plot_r && (tmp >= tmp2))	// see if the entire tick fell within this pixel
	{
	plot_l = true;	// flag that it needs plotting
	tmp3 = tick / heading_delta;
	tmp4 = 1.0 - tmp3;
	rl = Bitmap[0][zippy];
	gl = Bitmap[1][zippy];
	bl = Bitmap[2][zippy];
	r = (float)(rl * tmp4 + intensity * tmp3);
	g = (float)(gl * tmp4 + intensity * tmp3);
	b = (float)(bl * tmp4 + intensity * tmp3);
	} // if
else if (plot_l && plot_r)
	{
	b = g = r = (float)intensity;
	} // else if
else if (plot_l && !plot_r)
	{
	tmp3 = (tick - tmp) / tick;	// compute % coverage
	tmp4 = 1.0 - tmp3;
	// fetch the colors from the image for the right side
	rr = Bitmap[0][zippy];
	gr = Bitmap[1][zippy];
	br = Bitmap[2][zippy];
	r = (float)(rl * tmp4 + rr * tmp3);
	g = (float)(gl * tmp4 + gr * tmp3);
	b = (float)(bl * tmp4 + br * tmp3);
	} // else if
else if (!plot_l && plot_r)
	{
	tmp3 = tmp2 / tick;	// compute % coverage
	tmp4 = 1.0 - tmp3;
	// fetch the colors from the image for the left side
	rl = Bitmap[0][zippy];
	gl = Bitmap[1][zippy];
	bl = Bitmap[2][zippy];
	r = (float)(rl * tmp4 + rr * tmp3);
	g = (float)(gl * tmp4 + gr * tmp3);
	b = (float)(bl * tmp4 + br * tmp3);
	} // else if

} // PostProcHeading::ComputeTick

/*===========================================================================*/

// See if the camera settings make sense for this Post Process
int PostProcHeading::Doable(RenderData *Rend)
{
int success = 1;
char msg[128];

if ((Rend->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD) || (Rend->Cam->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC))
	{
	sprintf(&msg[0], "heading Post Process doesn't support Overhead or Planimetric Cameras.");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	success = 0;
	} // if
else if ((Rend->Cam->CamBank != 0.0) || (fabs(Rend->Cam->CamPitch) > 1.0))	// allow some pitch so targetted cameras have a chance of working
	{
	sprintf(&msg[0], "heading Post Process only supports Cameras with Bank and Pitch set to zero.");
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	success = 0;
	} // else if

return(success);

} // PostProcHeading::Doable

/*===========================================================================*/

// F2NOTE: Currently assumes bank & pitch of 0.  Also need to figure out what camera's can & can't be used (ie: no planimetric).
int PostProcHeading::RenderPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap, 
	long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps, 
	long FrameNum, int UpdateDiagnostics)
{
int success;

success = Doable(Rend);

if (success)
	success = PrepForPostProc(Rend, Buffers, FragMap, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

//zippy = Width * (Height - 52);
//if (Height > 52)	// 16 for text, 6 for separation
//if (Height > 30)	// 21 for ticks, 4 for 5 degree bands, 1 for separator, 4 for 1 degree bands

if (success && letters)
	success = PlotCardinals(Rend, FragMap, Width, Height, BWDE, Master, FrameNum, UpdateDiagnostics);

if (success && numbers)
	success = PlotDegrees(Rend, FragMap, Width, Height, BWDE, Master, FrameNum, UpdateDiagnostics);

if (success && (scale5 || scale1))
	success = PlotScales(Rend, FragMap, Width, Height, BWDE, Master, FrameNum, UpdateDiagnostics);

if (success && ticks)
	success = PlotTicks(Rend, FragMap, Width, Height, BWDE, Master, FrameNum, UpdateDiagnostics);

CleanupPostProc(Rend, Buffers, FragMap, Width, Height, BWDE, Master, OptionalBitmaps, FrameNum, UpdateDiagnostics);

return(success);

} // PostProcHeading::RenderPostProc

/*===========================================================================*/

int PostProcHeading::PlotCardinals(RenderData *Rend, rPixelBlockHeader *FragMap,
								   long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics)
{
double heading, heading_begin, heading_end;
RasterResampler *resampler;
unsigned char *blitImage;
int success = 1;
long first_y, modulus, resized_x, resized_y, x, y;
bool plot;
char directionals[][4] = {"N","NNE","NE","ENE","E","ESE","SE","SSE","S","SSW","SW","WSW","W","WNW","NW","NNW","N"};
//char dmsg[128];

// proportionate to 3 22x22 chars on a 400 pixel high image
resized_x = (long)(Height * 0.165);
resized_y = (long)(Height * 0.055);
if (!(blitImage = (unsigned char *)AppMem_Alloc(resized_x * resized_y, 0)))
	return 0;

if (!(resampler = new UByteRasterResampler()))
	{
	AppMem_Free(blitImage, resized_x * resized_y);
	return 0;
	} // if

// F2NOTE: Heading is incorrect when using a targeted camera.
heading_begin = Rend->Cam->CamHeading - Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5;
if (Rend->Cam->PanoCam)
	heading_begin += Rend->PanoPanel * Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
// Normalize headings so that 0 to 360 maps onto them
while ((heading_begin + Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue) < 0)
	heading_begin += 360.0;
while (heading_begin >= 360.0)
	heading_begin -= 360.0;
heading_end = heading_begin + Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;

//sprintf(dmsg, "Heading = %f, fov = %f\n", Rend->Cam->CamHeading, Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue);
//OutputDebugString(dmsg);
//sprintf(dmsg, "  <%f ... %f>\n", heading_begin, heading_end);
//OutputDebugString(dmsg);

// compute the skip for the directional array
if (sel_letters == 0)
	modulus = 4;
else if (sel_letters == 1)
	modulus = 2;
else
	modulus = 1;

// Headings as cardinals/ordinals
// F2NOTE: Put this before digits & add flag so user can mix & match digits & letters (ie: N 30 60 W)
for (long direction = 0; direction < 17; direction += modulus)	// do 17 so we catch both 0 & 360 and avoid fmod
	{
	plot = false;
	heading = direction * 22.5;	// step every 22.5 degrees
	// see if the given heading exists on this image
	// F2NOTE: Probably need to come up with an algorithm to figure out heading size in degrees as rendered
	if ((heading >= (heading_begin - 8.0)) && (heading <= (heading_end + 8.0)))	// WNW can take up to around 16 degrees of view in my test
		plot = true;
	if ((heading_begin < 0.0) && (heading_end >= 0.0) && ((heading - 360.0) >= (heading_begin - 8.0)))
		plot = true;
	if (plot)
		{
		FILE *tmpfile;
		long start_x, finish_x, cur_x, zippy2;
		char name_in[MAX_PATH], name_out[MAX_PATH], name_tmp[L_tmpnam], path[MAX_PATH];

		// clear out the buffers
		memset(rbuf, 0, rsize);
		memset(gbuf, 0, rsize);
		memset(bbuf, 0, rsize);
		memset(abuf, 0, rsize);

		strcpy(headstring, directionals[direction]);
		fi.PrintText(1, 1, headstring, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 0, 4, 1);
		//sprintf(dmsg, "  --> %s (%f)\n", headstring, heading);
		//OutputDebugString(dmsg);
		//fi.DebugDump();
		GetTempPath(MAX_PATH, path);
		tmpnam(name_tmp);
		strcpy(name_in, path);
		strcat(name_in, name_tmp);
		tmpnam(name_tmp);
		strcpy(name_out, path);
		strcat(name_out, name_tmp);
		fi.Dump(name_in, FI_ALPHA);
		//resampler->Resample(tmpname_in, "C:/test_65x22.raw", rheight, rwidth, 22, 65);
		if (resampler->Resample(name_in, name_out, rheight, rwidth, resized_y, resized_x))
			{
			if (tmpfile = fopen(name_out, "rb"))
				{
				fread(blitImage, resized_x, resized_y, tmpfile);
				fclose(tmpfile);
				remove(name_out);
				fi.DumpKill(name_in);

				// compute pixel we should be centered over
				if (heading > heading_end)
					heading -= 360.0;
				start_x = (long)(((heading - heading_begin) / (heading_end - heading_begin)) * Width);
				// adjust to area to start copying to
				start_x -= resized_x / 2;	// half our raster
				finish_x = start_x + resized_x;
				zippy2 = 0;
				first_y = (long)(Height * .875);	// F2NOTE: Need to adjust % by elements used
				for (y = 0; y < resized_y; y++)
					{
					zippy = Width * (first_y + y) + start_x;
					cur_x = start_x;
					for (x = 0; x < resized_x; x++, zippy++, cur_x++, zippy2++)
						{
						// see if it's a plottable pixel
						if ((cur_x >= 0) && (cur_x < Width))
							{
							if (blitImage[zippy2])
								{
								float i_font, i_back;	// intensity of components
								unsigned char value = blitImage[zippy2];
								
								i_font = blitImage[zippy2] / 255.0f;
								i_back = 1.0f - i_font;
								Bitmap[0][zippy] = (unsigned char)(value * i_font + Bitmap[0][zippy] * i_back);
								Bitmap[1][zippy] = (unsigned char)(value * i_font + Bitmap[1][zippy] * i_back);
								Bitmap[2][zippy] = (unsigned char)(value * i_font + Bitmap[2][zippy] * i_back);
								} // if
							} // if
						} // for x
					} // for y
				} // if
			} // if

		} // if
	} // for

delete resampler;

AppMem_Free(blitImage, resized_x * resized_y);

return(success);

} // PostProcHeading::PlotCardinals

/*===========================================================================*/

int PostProcHeading::PlotDegrees(RenderData *Rend, rPixelBlockHeader *FragMap,
								  long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics)
{
double heading_begin, heading_end;
RasterResampler *resampler;
unsigned char *blitImage;
int success = 1;
long first_y, ihead, modulus, resized_x, resized_y, x, y;
char formatStrings[][5] = {"%03d", "%d", "%02d", "%d"};

// proportionate to 3 22x22 chars on a 400 pixel high image
resized_x = quickftol(Height * 0.165);
resized_y = quickftol(Height * 0.055);
if (!(blitImage = (unsigned char *)AppMem_Alloc(resized_x * resized_y, 0)))
	return 0;

if (!(resampler = new UByteRasterResampler()))
	{
	AppMem_Free(blitImage, resized_x * resized_y);
	return 0;
	} // if

// F2NOTE: Heading is incorrect when using a targeted camera.
heading_begin = Rend->Cam->CamHeading - Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5;
// Need to make sure we can handle negative headings somehow.  Also start heading of 0 will cause some calculations to be negative.
while (heading_begin <= 0.0)
	heading_begin += 360.0;
if (Rend->Cam->PanoCam)
	heading_begin += Rend->PanoPanel * Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
heading_end = heading_begin + Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;

// set up the correct modulo
if (sel_numbers == 0)
	modulus = 90;
else if (sel_numbers == 1)
	modulus = 45;
else
	modulus = 30;

// Headings as degrees
ihead = quicklongfloor(heading_begin);
while (ihead <= heading_end)
	{
	bool plot = false;

	if (ihead % modulus == 0)
		{
		plot = true;
		if (letters)
			{
			// Letters & numbers coincide on 90 degrees.  Letters take priority.
			if ((ihead % 90) == 0)
				plot = false;
			// 45 degree lettering can coincide too, and it takes priority
			else if ((sel_letters >= 1) && ((ihead % 45) == 0))
				plot = false;
			} // if
		} // if
	if (plot)
		{
		FILE *tmpfile;
		int printval;
		long start_x, finish_x, cur_x, zippy2;
		char name_in[MAX_PATH], name_out[MAX_PATH], name_tmp[L_tmpnam], path[MAX_PATH];

		// clear out the buffers
		memset(rbuf, 0, rsize);
		memset(gbuf, 0, rsize);
		memset(bbuf, 0, rsize);
		memset(abuf, 0, rsize);

		if (ihead >= 360)
			printval = ihead - 360;
		else
			printval = ihead;
		if (sel_numstyle > 1)
			printval = printval / 10;
		sprintf(headstring, formatStrings[sel_numstyle], printval);
		fi.PrintText(1, 1, headstring, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 0, 4, 1);
		//fi.DebugDump();
		GetTempPath(MAX_PATH, path);
		tmpnam(name_tmp);
		strcpy(name_in, path);
		strcat(name_in, name_tmp);
		tmpnam(name_tmp);
		strcpy(name_out, path);
		strcat(name_out, name_tmp);
		fi.Dump(name_in, FI_ALPHA);
		if (resampler->Resample(name_in, name_out, rheight, rwidth, resized_y, resized_x))
			{
			if (tmpfile = fopen(name_out, "rb"))
				{
				fread(blitImage, resized_x, resized_y, tmpfile);
				fclose(tmpfile);
				remove(name_out);
				fi.DumpKill(name_in);

				// compute pixel we should be centered over
				start_x = (long)(((ihead - heading_begin)/ (heading_end - heading_begin)) * Width);
				// adjust to area to start copying to
				start_x -= resized_x / 2;	// half our raster
				finish_x = start_x + resized_x;
				zippy2 = 0;
				first_y = quickftol(Height * .875);	// F2NOTE: Need to adjust % by elements used
				for (y = 0; y < resized_y; y++)
					{
					zippy = Width * (first_y + y) + start_x;
					cur_x = start_x;
					for (x = 0; x < resized_x; x++, zippy++, cur_x++, zippy2++)
						{
						// see if it's a plottable pixel
						if ((cur_x >= 0) && (cur_x < Width))
							{
							if (blitImage[zippy2])
								{
								float i_font, i_back;	// intensity of components
								unsigned char value = blitImage[zippy2];
								
								i_font = blitImage[zippy2] / 255.0f;
								i_back = 1.0f - i_font;
								Bitmap[0][zippy] = (unsigned char)(value * i_font + Bitmap[0][zippy] * i_back);
								Bitmap[1][zippy] = (unsigned char)(value * i_font + Bitmap[1][zippy] * i_back);
								Bitmap[2][zippy] = (unsigned char)(value * i_font + Bitmap[2][zippy] * i_back);
								} // if
							} // if
						} // for x
					} // for y
				} // if
			} // if
		} // if
	++ihead;
	} // while

delete resampler;

AppMem_Free(blitImage, resized_x * resized_y);

return(success);

} // PostProcHeading::PlotDegrees

/*===========================================================================*/

// Render 5 degree and 1 degree scales.  Each scale is 1% of the image height.  If both scales are rendered, a 0.25% gray separator is drawn.
int PostProcHeading::PlotScales(RenderData *Rend, rPixelBlockHeader *FragMap,
								long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics)
{
double heading_start, pixel_delta, pixel_l, pixel_r, sector, tmp;
float aa_bitmap, aa_lines, first_y;
int success = 1;
long suHeight, suWidth, x, y;
unsigned char value_pixel, value_l, value_r;

suHeight = Rend->SetupHeight;
suWidth = Rend->SetupWidth;

// F2NOTE: Heading is incorrect when using a targeted camera.
heading_start = Rend->Cam->CamHeading - Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5;
// Need to make sure we can handle negative headings somehow.  Also start heading of 0 will cause some calculations to be negative.
while (heading_start <= 0.0)
	heading_start += 360.0;
if (Rend->Cam->PanoCam)
	heading_start += Rend->PanoPanel * Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;

// compute 5 degree scale & store in lines[0]
if (scale5)
	{
	pixel_l = heading_start;
	sector = pixel_l / 5.0;
	value_l = (unsigned char)((int)sector % 2 * 255);
	value_pixel = value_l;
	pixel_delta = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / Width;
	pixel_r = heading_start + pixel_delta;
	for (x = 0; x < Width; x++)
		{
		sector = pixel_r / 5.0;
		value_r = (unsigned char)((int)sector % 2 * 255);
		if (value_l == value_r)
			{
			value_pixel = value_r;
			lines[0][x] = value_pixel;
			} // if
		else
			{
			tmp = (WCS_floor(pixel_r) - pixel_l) / pixel_delta;
			value_pixel = (unsigned char)(value_l * tmp + value_r * (1.0 - tmp));
			lines[0][x] = value_pixel;
			value_l = value_r;	// this right is next pixels left
			} // else
		pixel_l = pixel_r;
		pixel_r += pixel_delta;
		} // for
	} // if scale5

// gray separator in lines[1] if applicable
if (scale5 & scale1)
	{
	memset(lines[1], 127, Width);
	} // if

// compute 1 degree scale & store in lines[2]
if (scale1)
	{
	pixel_l = heading_start;
	value_l = (unsigned char)((int)pixel_l % 2 * 255);
	value_pixel = value_l;
	pixel_delta = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / Width;
	pixel_r = heading_start + pixel_delta;
	for (x = 0; x < Width; x++)
		{
		value_r = (unsigned char)((int)pixel_r % 2 * 255);
		if (value_l == value_r)
			{
			value_pixel = value_r;
			lines[2][x] = value_pixel;
			} // if
		else
			{
			tmp = (WCS_floor(pixel_r) - pixel_l) / pixel_delta;
			value_pixel = (unsigned char)(value_l * tmp + value_r * (1.0 - tmp));
			lines[2][x] = value_pixel;
			value_l = value_r;	// this right is next pixels left
			} // else
		pixel_l = pixel_r;
		pixel_r += pixel_delta;
		} // for
	} // if scale1

if (scale5 && scale1)
	{
	long separator_y = quickftol(Height * 0.9875);

	// all three line buffers will be copied
	first_y = Height * 0.9775f;
	y = quickftol((double)first_y);
	zippy = Width * y;
	aa_bitmap = first_y - (float)y;
	aa_lines = 1.0f - aa_bitmap;
	// anti-alias first copied line
	for (x = 0; x < Width; x++, zippy++)
		{
		UBYTE val = lines[0][x];

		Bitmap[0][zippy] = (unsigned char)(Bitmap[0][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[1][zippy] = (unsigned char)(Bitmap[1][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[2][zippy] = (unsigned char)(Bitmap[2][zippy] * aa_bitmap + val * aa_lines);
		} // for x
	// 5 degree scale
	for (y = quickftol(first_y + 1.0); y < separator_y; y++)
		{
		for (x = 0; x < Width; x++, zippy++)
			{
			Bitmap[0][zippy] = Bitmap[1][zippy] = Bitmap[2][zippy] = lines[0][x];
			} // for x;
		} // for y
	// F2_NOTE: Separator is of constant size to keep code simpler.  Also it and last scale always fill their rows (ie: no anti-aliasing)
	// for the same reason.
	// 1 line gray separator
	for (x = 0; x < Width; x++, zippy++)
		{
		Bitmap[0][zippy] = Bitmap[1][zippy] = Bitmap[2][zippy] = 127;
		} // for x;
	// 1 degree scale
	for (y = separator_y + 1; y < Height; y++)
		{
		for (x = 0; x < Width; x++, zippy++)
			{
			Bitmap[0][zippy] = Bitmap[1][zippy] = Bitmap[2][zippy] = lines[2][x];
			} // for x;
		} // for y
	} // if scale5 && scale1
else if (scale5 && !scale1)
	{
	// only lines[0] will be copied
	first_y = Height * 0.99f;
	y = quickftol((double)first_y);
	zippy = Width * y;
	aa_bitmap = first_y - (float)y;
	aa_lines = 1.0f - aa_bitmap;
	// anti-alias first copied line
	for (x = 0; x < Width; x++, zippy++)
		{
		UBYTE val = lines[0][x];

		Bitmap[0][zippy] = (unsigned char)(Bitmap[0][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[1][zippy] = (unsigned char)(Bitmap[1][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[2][zippy] = (unsigned char)(Bitmap[2][zippy] * aa_bitmap + val * aa_lines);
		} // for x
	// just jam the rest
	for (y = (long)(first_y + 1.0f); y < Height; y++)
		{
		for (x = 0; x < Width; x++, zippy++)
			{
			Bitmap[0][zippy] = Bitmap[1][zippy] = Bitmap[2][zippy] = lines[0][x];
			} // for x;
		} // for y
	} // else if scale5 && !scale1
else if (!scale5 && scale1)
	{
	// only lines[2] will be copied
	first_y = Height * 0.99f;
	y = (long)first_y;
	zippy = Width * y;
	aa_bitmap = first_y - (float)y;
	aa_lines = 1.0f - aa_bitmap;
	// anti-alias first copied line
	for (x = 0; x < Width; x++, zippy++)
		{
		UBYTE val = lines[2][x];

		Bitmap[0][zippy] = (unsigned char)(Bitmap[0][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[1][zippy] = (unsigned char)(Bitmap[1][zippy] * aa_bitmap + val * aa_lines);
		Bitmap[2][zippy] = (unsigned char)(Bitmap[2][zippy] * aa_bitmap + val * aa_lines);
		} // for x
	// just jam the rest
	for (y = (long)(first_y + 1.0f); y < Height; y++)
		{
		for (x = 0; x < Width; x++, zippy++)
			{
			Bitmap[0][zippy] = Bitmap[1][zippy] = Bitmap[2][zippy] = lines[2][x];
			} // for x;
		} // for y
	} // else if !scale5 && scale1

return(success);

} // PostProcHeading::PlotScales

/*===========================================================================*/

int PostProcHeading::PlotTicks(RenderData *Rend, rPixelBlockHeader *FragMap,
								long Width, long Height, BusyWin *BWDE, RenderInterface *Master, long FrameNum, int UpdateDiagnostics)
{
double heading_start, heading_delta2;
float aa_bitmap, aa_lines, first_y, last_y, small_y;
int success = 1;
long suHeight, suWidth, x, y, y0, y1, y2;
bool aa_me, plot_l, plot_r;	// aa_me refers to the vertical direction - they're always done in the horizontal direction.

// F2NOTE: Heading is incorrect when using a targeted camera.
heading_start = Rend->Cam->CamHeading - Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue * 0.5;
// Need to make sure we can handle negative headings somehow.  Also start heading of 0 will cause some calculations to be negative.
while (heading_start <= 0.0)
	heading_start += 360.0;
if (Rend->Cam->PanoCam)
	heading_start += Rend->PanoPanel * Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
heading_delta = Rend->Cam->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue / Width;	// width - 1???
heading_delta2 = heading_delta * 0.5;

// figure out where exactly the elements start/finish at
suHeight = Rend->SetupHeight;
suWidth = Rend->SetupWidth;
if (scale5 && scale1)
	{
	first_y = suHeight * 0.925f;	// 5.25% + 1% + 0.25% + 1%
	small_y = suHeight * 0.9425f;	// 3.5% + 1% + 0.25% + 1%
	last_y = suHeight * 0.9775f;
	} // if
else if (scale5 || scale1)
	{
	first_y = suHeight * 0.9375f;	// 5.25% + 1%
	small_y = suHeight * 0.955f;	// 3.5% + 1%
	last_y = suHeight * 0.99f;
	} // else if
else
	{
	first_y = suHeight * 0.9475f;	// 5.25%
	small_y = suHeight * 0.965f;	// 3.5%
	last_y = (float)suHeight;
	} // else

y1 = quickftol((double)small_y);
y2 = quickftol((double)last_y);
y0 = y = quickftol((double)first_y);
zippy = Width * y;

//char dmsg[1024];

while (y < y2)
	{
	//sprintf(dmsg, "Line %d\n", y);
	//OutputDebugString(dmsg);

	pixel_left = heading_start - heading_delta2;
	pixel_right = heading_start + heading_delta2;
	if (y == y0)
		{
		aa_me = true;
		aa_bitmap = first_y - (float)y;
		aa_lines = 1.0f - aa_bitmap;
		} // if
	else if (y == y1)
		{
		aa_me = false;	// will be toggled on as needed
		aa_bitmap = small_y - (float)y;
		aa_lines = 1.0f - aa_bitmap;
		} // else if
	else
		aa_me = false;
	for (x = 0; x < Width; x++, zippy++)
		{
		plot_l = plot_r = false;
		// 45 degrees
		if (sel_ticks == 0)
			{
			// see if it's a 90 degree mark first
			ComputeTick(90.0, 255, plot_l, plot_r);
			// if it wasn't, check to see if it's a 45 degree mark
			if (!(plot_l || plot_r))
				{
				ComputeTick(45.0, 231, plot_l, plot_r);
				} // if
			} // if 45 degrees
		// 30 degrees
		if ((sel_ticks == 1) || (sel_ticks == 2) || (sel_ticks == 3))
			{
			ComputeTick(30.0, 255, plot_l, plot_r);
			} // if 30 degrees
		// 10 degrees
		if ((sel_ticks == 2) || (sel_ticks == 3))
			{
			// don't retick
			if (!(plot_l || plot_r))
				{
				ComputeTick(10.0, 231, plot_l, plot_r);
				} // if
			} // if 10 degrees
		// 5 degrees
		if ((sel_ticks == 3) && (y >= y1))	// 5 degree marks are reduced in height
			{
			// don't retick
			if (!(plot_l || plot_r))
				{
				ComputeTick(5.0, 231, plot_l, plot_r);
				//if (plot_l || plot_r)
				//	{
				//	aa_me = true;
				//	} // if
				} // if
			} // if 5 degrees
		if (plot_l || plot_r)
			{
			if (aa_me)
				{
				//sprintf(dmsg, "AA @ %d ->", x);
				//OutputDebugString(dmsg);
				//sprintf(dmsg, "%f, %f, %f (%f)\n", r, g, b, aa_lines);
				//OutputDebugString(dmsg);

				Bitmap[0][zippy] = (unsigned char)(Bitmap[0][zippy] * aa_bitmap + r * aa_lines);
				Bitmap[1][zippy] = (unsigned char)(Bitmap[1][zippy] * aa_bitmap + g * aa_lines);
				Bitmap[2][zippy] = (unsigned char)(Bitmap[2][zippy] * aa_bitmap + b * aa_lines);

				if (y == y1)
					aa_me = false;	// reset flag so 5 degrees can plot both with & without aa
				} // if
			else
				{
				//sprintf(dmsg, "@ %d ->", x);
				//OutputDebugString(dmsg);
				//sprintf(dmsg, "%f, %f, %f\n", r, g, b);
				//OutputDebugString(dmsg);

				Bitmap[0][zippy] = (unsigned char)(r + 0.499f);
				Bitmap[1][zippy] = (unsigned char)(g + 0.499f);
				Bitmap[2][zippy] = (unsigned char)(b + 0.499f);
				} // else
			} // plot
		pixel_left = pixel_right;
		pixel_right += heading_delta;
		} // for x
	++y;
	} // while

return(success);

} // PostProcHeading::PlotTicks

/*===========================================================================*/

int PostProcHeading::PrepForPostProc(RenderData *Rend, BufferNode *Buffers, rPixelBlockHeader *FragMap,
									  long Width, long Height, BusyWin *BWDE, RenderInterface *Master, unsigned char **OptionalBitmaps,
									  long FrameNum, int UpdateDiagnostics)
{
int success = 0;

fi.SetFont(irf.FontDataPointer, (USHORT)irf.InternalFontWidth, (USHORT)irf.InternalFontHeight);

rwidth = (irf.InternalFontWidth / 16) * 3 + 2;	// one pixel AA border
rheight = (irf.InternalFontHeight / 16) + 2;	// one pixel AA border
rsize = rwidth * rheight;

rbuf = (UBYTE *)AppMem_Alloc(rsize, 0);
gbuf = (UBYTE *)AppMem_Alloc(rsize, 0);
bbuf = (UBYTE *)AppMem_Alloc(rsize, 0);
abuf = (UBYTE *)AppMem_Alloc(rsize, 0);

if (rbuf && gbuf && bbuf && abuf)
	{
	success = 1;

	fi.SetOutput(rwidth, rheight, rbuf, gbuf, bbuf, abuf);

	if (OptionalBitmaps)
		{
		Bitmap[0] = OptionalBitmaps[0];
		Bitmap[1] = OptionalBitmaps[1];
		Bitmap[2] = OptionalBitmaps[2];
		} // if
	else
		{
		Bitmap[0] = (unsigned char *)Buffers->FindBuffer("RED",	WCS_RASTER_BANDSET_BYTE);
		Bitmap[1] = (unsigned char *)Buffers->FindBuffer("GREEN", WCS_RASTER_BANDSET_BYTE);
		Bitmap[2] = (unsigned char *)Buffers->FindBuffer("BLUE", WCS_RASTER_BANDSET_BYTE);
		} // else

	} // if

if (success)
	{
	lines[0] = (UBYTE *)AppMem_Alloc(Width, 0);
	lines[1] = (UBYTE *)AppMem_Alloc(Width, 0);
	lines[2] = (UBYTE *)AppMem_Alloc(Width, 0);

	if (!lines[0] || !lines[1] || !lines[2])
		success = 0;
	} // /if

return(success);

} // PostProcHeading::PrepForPostProc

/*===========================================================================*/

ULONG PostProcHeading::LoadSpecificData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_POSTPROCHEADING_LETTERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&letters, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_NUMBERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&numbers, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SCALE1:
						{
						BytesRead = ReadBlock(ffile, (char *)&scale1, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SCALE5:
						{
						BytesRead = ReadBlock(ffile, (char *)&scale5, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_TICKS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ticks, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SELLETTERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&sel_letters, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SELNUMBERS:
						{
						BytesRead = ReadBlock(ffile, (char *)&sel_numbers, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SELNUMSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&sel_numstyle, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_POSTPROCHEADING_SELTICKS:
						{
						BytesRead = ReadBlock(ffile, (char *)&sel_ticks, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // PostProcHeading::LoadSpecificData

/*===========================================================================*/

unsigned long PostProcHeading::SaveSpecificData(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_LETTERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&letters)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_NUMBERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&numbers)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SCALE1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&scale1)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SCALE5, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&scale5)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_TICKS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ticks)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SELLETTERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&sel_letters)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SELNUMBERS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&sel_numbers)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SELNUMSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&sel_numstyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_POSTPROCHEADING_SELTICKS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&sel_ticks)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // PostProcHeading::SaveSpecificData

/*===========================================================================*/

void PostProcHeading::CopySpecificData(PostProcessEvent *CopyToPP, PostProcessEvent *CopyFromPP)
{
PostProcHeading *CopyTo, *CopyFrom;

CopyTo = (PostProcHeading *)CopyToPP;
CopyFrom = (PostProcHeading *)CopyFromPP;

CopyTo->letters = CopyFrom->letters;
CopyTo->numbers = CopyFrom->numbers;
CopyTo->scale1 = CopyFrom->scale1;
CopyTo->scale5 = CopyFrom->scale5;
CopyTo->ticks = CopyFrom->ticks;

CopyTo->sel_letters = CopyFrom->sel_letters;
CopyTo->sel_numbers = CopyFrom->sel_numbers;
CopyTo->sel_numstyle = CopyFrom->sel_numstyle;
CopyTo->sel_ticks = CopyFrom->sel_ticks;

} // PostProcHeading::CopySpecificData

/*===========================================================================*/
/*===========================================================================*/

// ADD_NEW_POSTPROC_EVENT	add the class constructor, destructor and any other methods needed - they could be in
//							another file if they get too large
