// Wave.cpp
// For managing WCS waves
// Built from scratch on 04/15/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "Texture.h"
#include "Raster.h"
#include "Useful.h"
#include "Application.h"
#include "EffectsLib.h"
#include "EffectsIO.h"
#include "Joe.h"
#include "Conservatory.h"
#include "Project.h"
#include "Interactive.h"
#include "DragnDropListGUI.h"
#include "requester.h"
#include "Toolbar.h"
#include "WaveEditGUI.h"
#include "AppMem.h"
#include "Database.h"
#include "ViewGUI.h"
#include "Lists.h"
#include "FeatureConfig.h"


WaveSource::WaveSource(RasterAnimHost *RAHost)
: RasterAnimHost(RAHost)
{
double EffectDefault[WCS_EFFECTS_WAVESOURCE_NUMANIMPAR] = {0.0, 0.0, 1.0, 1.0, 0.0, .5, 0.0, 0.0};
double RangeDefaults[WCS_EFFECTS_WAVESOURCE_NUMANIMPAR][3] = {FLT_MAX, -FLT_MAX, 1.0, FLT_MAX, -FLT_MAX, 1.0, 
	FLT_MAX, 0.0, .1,
	FLT_MAX, 0.00001, 1.0,
	FLT_MAX, 0.0, .01,
	FLT_MAX, -FLT_MAX, 1.0, 
	FLT_MAX, 0.0, 0.0, 	// set increment to 0 to use project frame rate
	FLT_MAX, -FLT_MAX, 1.0};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct + WCS_EFFECTS_WAVE_NUMANIMPAR, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
Envelope.SetDefaults(this, (char)WCS_EFFECTS_WAVESOURCE_NUMANIMPAR + WCS_EFFECTS_WAVE_NUMANIMPAR);
EnvelopeEnabled = RepeatEnvelopeBefore = RepeatEnvelopeAfter = 0;
Enabled = 1;
Next = NULL;

AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].SetMetricType(WCS_ANIMDOUBLE_METRIC_VELOCITY);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].SetMetricType(WCS_ANIMDOUBLE_METRIC_VELOCITY);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].SetNoNodes(1);

} // WaveSource::WaveSource

/*===========================================================================*/

WaveSource::~WaveSource()
{
long Ct;
RootTexture *DelTex;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

} // WaveSource::~WaveSource

/*===========================================================================*/

void WaveSource::Copy(WaveSource *CopyTo, WaveSource *CopyFrom)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].Copy((AnimCritter *)&CopyTo->AnimPar[Ct], (AnimCritter *)&CopyFrom->AnimPar[Ct]);
	} // for
/*
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (CopyTo->TexRoot[Ct])
		{
		delete CopyTo->TexRoot[Ct];
		CopyTo->TexRoot[Ct] = NULL;
		} // if
	if (CopyFrom->TexRoot[Ct])
		{
		if (CopyTo->TexRoot[Ct] = new RootTexture(CopyTo, 0, 0, 0))
			{
			CopyTo->TexRoot[Ct]->Copy(CopyTo->TexRoot[Ct], CopyFrom->TexRoot[Ct]);
			}
		} // if
	} // for
*/
Envelope.Copy(&CopyTo->Envelope, &CopyFrom->Envelope);
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->EnvelopeEnabled = CopyFrom->EnvelopeEnabled;
CopyTo->RepeatEnvelopeBefore =CopyFrom->RepeatEnvelopeBefore;
CopyTo->RepeatEnvelopeAfter = CopyFrom->RepeatEnvelopeAfter;
RootTextureParent::Copy(CopyTo, CopyFrom);
RasterAnimHost::Copy(CopyTo, CopyFrom);

} // WaveSource::Copy

/*===========================================================================*/

unsigned long int WaveSource::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1;

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
					case WCS_EFFECTS_WAVESOURCE_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_ENVELOPEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&EnvelopeEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_REPEATENVBEFORE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RepeatEnvelopeBefore, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_REPEATENVAFTER:
						{
						BytesRead = ReadBlock(ffile, (char *)&RepeatEnvelopeAfter, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_ENVELOPE:
						{
						BytesRead = Envelope.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_OFFSETX:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_OFFSETY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_AMPLITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_WAVELENGTH:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_PHASE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_VELOCITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_ENVELOPESTARTTIME:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_ENVELOPEVELOCITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_TEXAMPLITUDE:
						{
						if (TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_WAVESOURCE_TEXPHASE:
						{
						if (TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
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

} // WaveSource::Load

/*===========================================================================*/

unsigned long int WaveSource::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_WAVESOURCE_NUMANIMPAR] = {WCS_EFFECTS_WAVESOURCE_OFFSETX,
																	WCS_EFFECTS_WAVESOURCE_OFFSETY,
																	WCS_EFFECTS_WAVESOURCE_AMPLITUDE,
																	WCS_EFFECTS_WAVESOURCE_WAVELENGTH,
																	WCS_EFFECTS_WAVESOURCE_PHASE,
																	WCS_EFFECTS_WAVESOURCE_VELOCITY,
																	WCS_EFFECTS_WAVESOURCE_ENVELOPESTARTTIME,
																	WCS_EFFECTS_WAVESOURCE_ENVELOPEVELOCITY};
unsigned long int TextureItemTag[WCS_EFFECTS_WAVESOURCE_NUMTEXTURES] = {WCS_EFFECTS_WAVESOURCE_TEXAMPLITUDE,
														WCS_EFFECTS_WAVESOURCE_TEXPHASE};

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVESOURCE_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVESOURCE_ENVELOPEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EnvelopeEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVESOURCE_REPEATENVBEFORE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RepeatEnvelopeBefore)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVESOURCE_REPEATENVAFTER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RepeatEnvelopeAfter)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_WAVESOURCE_ENVELOPE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = Envelope.Save(ffile))
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
			} // if color saved
		else
			goto WriteError;
		} // if size written
	else
		goto WriteError;
	} // if tag written
else
	goto WriteError;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
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

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
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
					} // if anim param saved
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

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // WaveSource::Save

/*===========================================================================*/

char *WaveSourceCritterNames[WCS_EFFECTS_WAVESOURCE_NUMANIMPAR] = {"Offset X (m)", "Offset Y (m)",
	"Amplitude (m)", "Wavelength (m)", "Phase (%)", "Velocity (m/sec)", "Envelope Start Time (sec)", "Envelope Velocity (m/sec)"};
char *WaveSourceTextureNames[WCS_EFFECTS_WAVESOURCE_NUMTEXTURES] = {"Amplitude (%)", "Phase (%)"};

char *WaveSource::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (WaveSourceCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (WaveSourceTextureNames[Ct]);
		} // if
	} // for
if (Test == &Envelope)
	return ("Amplitude Envelope (%)");

return ("");

} // WaveSource::GetCritterName

/*===========================================================================*/

char *WaveSource::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Wave Source Texture! Remove anyway?");

} // WaveSource::OKRemoveRaster

/*===========================================================================*/

int WaveSource::RemoveRAHost(RasterAnimHost *RemoveMe)
{
char Ct;
NotifyTag Changes[2];
int Removed = 0;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (RemoveMe == GetTexRootPtr(Ct))
		{
		delete GetTexRootPtr(Ct);
		SetTexRootPtr(Ct, NULL);
		Removed = 1;
		} // if
	} // for

if (Removed)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (Removed);

} // WaveSource::RemoveRAHost

/*===========================================================================*/

int WaveSource::SetToTime(double Time)
{
long Found = 0, Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->SetToTime(Time))
		{
		Found = 1;
		} // if
	} // for

return (Found);

} // WaveSource::SetToTime

/*===========================================================================*/

long WaveSource::InitImageIDs(long &ImageID)
{
long NumImages = 0;
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		NumImages += GetTexRootPtr(Ct)->InitImageIDs(ImageID);
		} // if
	} // for

return (NumImages);

} // WaveSource::InitImageIDs

/*===========================================================================*/

long WaveSource::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Ct, Found = 0;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
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
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
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

} // WaveSource::GetKeyFrameRange

/*===========================================================================*/

char WaveSource::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (GetNumAnimParams() > 0 && DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	return (1);
if (GetNumTextures() > 0 && (DropType == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropType == WCS_RAHOST_OBJTYPE_TEXTURE))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE)
	return (1);

return (0);

} // WaveSource::GetRAHostDropOK

/*===========================================================================*/

int WaveSource::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char WinNum, QueryStr[256], NameStr[128];
int Success = 0;
long Ct, NumListItems = 0;
RasterAnimHostProperties Prop;
RasterAnimHost *TargetList[30];
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (WaveSource *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (WaveSource *)DropSource->DropSource);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		TargetList[Ct] = GetAnimPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ROOTTEXTURE || DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
	{
	Success = -1;
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		TargetList[Ct] = GetTexRootPtr(Ct);
		} // for
	NumListItems = Ct;
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE)
	{
	Success = Envelope.ProcessRAHostDragDrop(DropSource);
	} // if

if (NumListItems > 0)
	{
	if ((WinNum = GlobalApp->MCP->GetAvailableDragnDropListNumber()) >= 0)
		{
		GlobalApp->GUIWins->DDL[WinNum] = new DragnDropListGUI(WinNum, DropSource, TargetList, NumListItems, this, NULL);
		if(GlobalApp->GUIWins->DDL[WinNum])
			{
			GlobalApp->GUIWins->DDL[WinNum]->Open(GlobalApp->MainProj);
			}
		} // if
	else
		UserMessageOK("Open Drag 'n Drop List", "All List windows are in use.\nTry dropping on a target deeper in the hierarchy\n and closer to your desired target.");
	} // if

return (Success);

} // WaveSource::ProcessRAHostDragDrop

/*===========================================================================*/

char *WaveSource::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? WaveSourceTextureNames[TexNumber]: (char*)"");

} // WaveSource::GetTextureName

/*===========================================================================*/

RootTexture *WaveSource::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // WaveSource::NewRootTexture

/*===========================================================================*/

int WaveSource::GetRAHostAnimated(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimated())
		return (1);
	} // for

return (0);

} // WaveSource::GetRAHostAnimated

/*===========================================================================*/

bool WaveSource::AnimateMaterials(void)
{
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (GetAnimPtr(Ct)->GetRAHostAnimated())
		return (true);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct) && GetTexRootPtr(Ct)->GetRAHostAnimatedInclVelocity())
		return (true);
	} // for
if (GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY)->CurValue != 0.0)
	return (true);
	
return (false);

} // WaveSource::AnimateMaterials

/*===========================================================================*/

unsigned long WaveSource::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_WAVESOURCE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // WaveSource::GetRAFlags

/*===========================================================================*/

void WaveSource::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = RAParent ? RAParent->GetCritterName(this): (char*)"";
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
if (Prop->PropMask & WCS_RAHOST_MASKBIT_POPCLASS)
	{
	GetPopClassFlags(Prop);
	} // get pop-up menu classes

} // WaveSource::GetRAHostProperties

/*===========================================================================*/

int WaveSource::SetRAHostProperties(RasterAnimHostProperties *Prop)
{
int Success = 0;
NotifyTag Changes[2];

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

return (Success);

} // WaveSource::SetRAHostProperties

/*===========================================================================*/

int WaveSource::BuildFileComponentsList(EffectList **Coords)
{
#ifdef WCS_BUILD_VNS
long Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (GetTexRootPtr(Ct))
		{
		if (! GetTexRootPtr(Ct)->BuildFileComponentsList(Coords))
			return (0);
		} // if
	} // for
#endif // WCS_BUILD_VNS

return (1);

} // WaveSource::BuildFileComponentsList

/*===========================================================================*/

int WaveSource::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE);
					break;
					} // 
				case WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_WAVESOURCE_TEXTURE_AMPLITUDE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE);
					break;
					} // 
				case WCS_EFFECTS_WAVESOURCE_TEXTURE_PHASE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // WaveSource::GetAffiliates

/*===========================================================================*/

int WaveSource::GetPopClassFlags(RasterAnimHostProperties *Prop)
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

} // WaveSource::GetPopClassFlags

/*===========================================================================*/

int WaveSource::AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long int MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::AddSRAHBasePopMenus(PMA, MenuClassFlags, ChildA, ChildB, AnimAffil, TexAffil, NULL));
	} // if

return(0);

} // WaveSource::AddSRAHBasePopMenus

/*===========================================================================*/

int WaveSource::HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB)
{
AnimCritter *AnimAffil = NULL;
RootTexture **TexAffil = NULL;

if (GetAffiliates(ChildA, ChildB, AnimAffil, TexAffil))
	{
	return (RasterAnimHost::HandleSRAHPopMenuSelection(Action, AnimAffil, TexAffil, NULL, this, NULL));
	} // if

return(0);

} // WaveSource::HandleSRAHPopMenuSelection

/*===========================================================================*/

RasterAnimHost *WaveSource::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&Envelope);
if (Current == &Envelope)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // WaveSource::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *WaveSource::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX))
	return (GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY))
	return (GetAnimPtr(WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX));

return (NULL);

} // WaveSource::GetNextGroupSibling

/*===========================================================================*/

int WaveSource::GetDeletable(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for

return (0);

} // WaveSource::GetDeletable

/*===========================================================================*/

int WaveSource::InitToRender(void)
{
char Ct;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for

return (1);

} // WaveSource::InitToRender

/*===========================================================================*/
/*===========================================================================*/

WaveEffect::WaveEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_WAVE;
SetDefaults();

} // WaveEffect::WaveEffect

/*===========================================================================*/

WaveEffect::WaveEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_WAVE;
SetDefaults();

} // WaveEffect::WaveEffect

/*===========================================================================*/

WaveEffect::WaveEffect(RasterAnimHost *RAHost, EffectsLib *Library, WaveEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_WAVE;
if (Library)
	{
	Prev = Library->LastWave;
	if (Library->LastWave)
		{
		Library->LastWave->Next = this;
		Library->LastWave = this;
		} // if
	else
		{
		Library->Wave = Library->LastWave = this;
		} // else
	} // if
Name[0] = NULL;
SetDefaults();
if (Proto)
	{
	Copy(this, Proto);
	Name[0] = NULL;
	strcpy(NameBase, Proto->Name);
	} // if
else
	{
	strcpy(NameBase, "Wave");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // WaveEffect::WaveEffect

/*===========================================================================*/

WaveEffect::~WaveEffect()
{
WaveSource *CurSource;
RootTexture *DelTex;
long Ct;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->WEG && GlobalApp->GUIWins->WEG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->WEG;
		GlobalApp->GUIWins->WEG = NULL;
		} // if
	} // if

// delete array of wave pointers if it exists - it was used for rendering
if (WaveSourceArray)
	{
	AppMem_Free(WaveSourceArray, NumSources * sizeof (WaveSource *));
	WaveSourceArray = NULL;
	} // if

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (DelTex = TexRoot[Ct])
		{
		TexRoot[Ct] = NULL;
		delete DelTex;
		} // if
	} // for

while (WaveSources)
	{
	CurSource = WaveSources;
	WaveSources = WaveSources->Next;
	delete CurSource;
	} // while

} // WaveEffect::~WaveEffect

/*===========================================================================*/

void WaveEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_WAVE_NUMANIMPAR] = {0.0, 0.0, 1.0, 1.0, 0.0};
double RangeDefaults[WCS_EFFECTS_WAVE_NUMANIMPAR][3] = {90.0, -90.0, .0001, FLT_MAX, -FLT_MAX, .0001, 
	FLT_MAX, 0.0, .01,
	FLT_MAX, 0.0, 0.0,		// set increment to 0 to use project frame rate
	FLT_MAX, 0.0, 0.0};		// set increment to 0 to use project frame rate
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	TexRoot[Ct] = NULL;
	} // for
WaveSourcesPerVertex = 1;
WaveEffectType = WCS_WAVE_WAVETYPE_AREA;
RandomizeType = WCS_WAVE_RANDOMIZETYPE_POISSON;
RandomizeEnvStart = 1;
WaveSources = NULL;
WaveSourceArray = NULL;
NumSources = 0;
WavePreviewSize = 4;
ADProf.SetDefaults(this, (char)GetNumAnimParams());

AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].SetNoNodes(1);
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].SetNoNodes(1);

} // WaveEffect::SetDefaults

/*===========================================================================*/

void WaveEffect::Copy(WaveEffect *CopyTo, WaveEffect *CopyFrom)
{
WaveSource *CurrentFrom = CopyFrom->WaveSources, **ToPtr, *NextSource;

// delete existing wave sources
while (CopyTo->WaveSources)
	{
	NextSource = CopyTo->WaveSources;
	CopyTo->WaveSources = CopyTo->WaveSources->Next;
	delete NextSource;
	} // while

ToPtr = &CopyTo->WaveSources;

while (CurrentFrom)
	{
	if (*ToPtr = new WaveSource(CopyTo))
		{
		(*ToPtr)->Copy(*ToPtr, CurrentFrom);
		} // if
	ToPtr = &(*ToPtr)->Next;
	CurrentFrom = CurrentFrom->Next;
	} // while

CopyTo->WaveEffectType = CopyFrom->WaveEffectType;
CopyTo->RandomizeType =CopyFrom->RandomizeType;
CopyTo->RandomizeEnvStart = CopyFrom->RandomizeEnvStart;
CopyTo->WaveSourcesPerVertex = CopyFrom->WaveSourcesPerVertex;
CopyTo->WavePreviewSize = CopyFrom->WavePreviewSize;
ADProf.Copy(&CopyTo->ADProf, &CopyFrom->ADProf);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // WaveEffect::Copy

/*===========================================================================*/

WaveSource *WaveEffect::AddWave(WaveSource *AddMe)
{
WaveSource **CurSource = &WaveSources;
NotifyTag Changes[2];

while (*CurSource)
	{
	CurSource = &(*CurSource)->Next;
	} // while
if (*CurSource = new WaveSource(this))
	{
	if (AddMe)
		(*CurSource)->Copy(*CurSource, AddMe);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurSource);

} // WaveEffect::AddWave

/*===========================================================================*/

WaveSource *WaveEffect::AddWave(double NewLat, double NewLon)
{
WaveSource **CurSource = &WaveSources;
NotifyTag Changes[2];
double LatScaleMeters, LonScaleMeters, RelX, RelY, TempLon;

while (*CurSource)
	{
	CurSource = &(*CurSource)->Next;
	} // while
if (*CurSource = new WaveSource(this))
	{
	LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
	LonScaleMeters = LonScale(GlobalApp->AppEffects->GetPlanetRadius(), AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue);

	TempLon = NewLon - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
	if (fabs(TempLon) > 180.0)
		{
		TempLon += 180;
		if (fabs(TempLon) >= 360.0)
			TempLon = fmod(TempLon, 360.0);	// retains the original sign
		if (TempLon < 0.0)
			TempLon += 360.0;
		TempLon -= 180.0;
		NewLon = TempLon + AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
		} // if
	// replaced by above
	//if (NewLon > AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue)
	//	{
	//	while (NewLon - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue > 180.0)
	//		NewLon -= 360.0;
	//	} // if
	//else if (NewLon < AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue)
	//	{
	//	while (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - NewLon  > 180.0)
	//		NewLon += 360.0;
	//	} // if
	RelY = (NewLat - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue) * LatScaleMeters;
	RelX = (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - NewLon) * LonScaleMeters;

	(*CurSource)->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetValue(RelY);
	(*CurSource)->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetValue(RelX);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (*CurSource);

} // WaveEffect::AddWave

/*===========================================================================*/

unsigned long int WaveEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TexRootNumber = -1;
WaveSource **LoadTo = &WaveSources;

while (ItemTag != WCS_PARAM_DONE)
	{
	/* read block descriptor tag from file */
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			/* read block size from file */
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
					} /* switch */

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
					case WCS_EFFECTS_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PRIORITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Priority, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_USEGRADIENT:
						{
						BytesRead = ReadBlock(ffile, (char *)&UseGradient, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_WAVEEFFECTTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&WaveEffectType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_RANDOMIZETYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_RANDOMIZEENVSTART:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomizeEnvStart, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_WAVESOURCESPERVERTEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&WaveSourcesPerVertex, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_WAVEPREVIEWSIZE:
						{
						BytesRead = ReadBlock(ffile, (char *)&WavePreviewSize, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_LATITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_LONGITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_AMPLITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_ENVSTARTRANGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_PERVERTEXENVDELAY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_WAVE_WAVESOURCE:
						{
						if (*LoadTo = new WaveSource(this))
							{
							BytesRead = (*LoadTo)->Load(ffile, Size, ByteFlip);
							LoadTo = &(*LoadTo)->Next;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_WAVE_TEXTUREROOTNUM:
						{
						BytesRead = ReadBlock(ffile, (char *)&TexRootNumber, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures())
							{
							TexRoot[TexRootNumber] = new RootTexture(this, 0, 0, 0);
							} // if
						break;
						}
					case WCS_EFFECTS_WAVE_TEXTUREROOT:
						{
						if (TexRootNumber >= 0 && TexRootNumber < GetNumTextures() && TexRoot[TexRootNumber])
							BytesRead = TexRoot[TexRootNumber]->Load(ffile, Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TexRootNumber = -1;
						break;
						}
					case WCS_EFFECTS_WAVE_TEXAMPLITUDE:
						{
						if (TexRoot[WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE] = new RootTexture(this, 0, 0, 0))
							{
							BytesRead = TexRoot[WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE]->Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_EFFECTS_WAVE_PROFILE:
						{
						BytesRead = ADProf.Load(ffile, Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} /* switch */

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} /* if size block read */
			else
				break;
			} /* if not done flag */
		} /* if tag block read */
	else
		break;
	} /* while */

return (TotalRead);

} // WaveEffect::Load

/*===========================================================================*/

unsigned long int WaveEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_WAVE_NUMANIMPAR] = {WCS_EFFECTS_WAVE_LATITUDE,
																	WCS_EFFECTS_WAVE_LONGITUDE,
																	WCS_EFFECTS_WAVE_AMPLITUDE,
																	WCS_EFFECTS_WAVE_ENVSTARTRANGE,
																	WCS_EFFECTS_WAVE_PERVERTEXENVDELAY};
unsigned long int TextureItemTag[WCS_EFFECTS_WAVE_NUMTEXTURES] = {WCS_EFFECTS_WAVE_TEXAMPLITUDE};
WaveSource *Current;

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Enabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PRIORITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Priority)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_USEGRADIENT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&UseGradient)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVE_WAVEEFFECTTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&WaveEffectType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVE_RANDOMIZETYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVE_RANDOMIZEENVSTART, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&RandomizeEnvStart)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVE_WAVESOURCESPERVERTEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&WaveSourcesPerVertex)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_WAVE_WAVEPREVIEWSIZE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&WavePreviewSize)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;


for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
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

Current = WaveSources;
while (Current)
	{
	ItemTag = WCS_EFFECTS_WAVE_WAVESOURCE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Current->Save(ffile))
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
				} // if wave source saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	Current = Current->Next;
	} // while

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
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
					} // if anim param saved
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

ItemTag = WCS_EFFECTS_WAVE_PROFILE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ADProf.Save(ffile))
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
			} // if profile gradient saved 
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

} // WaveEffect::Save

/*===========================================================================*/

char *WaveEffectCritterNames[WCS_EFFECTS_WAVE_NUMANIMPAR] = {"Latitude (deg)", "Longitude (deg)",
	"Amplitude (%)", "Envelope Start Range (sec)", "Per Vertex Envelope Delay (sec)"};
char *WaveEffectTextureNames[WCS_EFFECTS_WAVE_NUMTEXTURES] = {"Amplitude (%)"};

char *WaveEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (WaveEffectCritterNames[Ct]);
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (WaveEffectTextureNames[Ct]);
		} // if
	} // for
if (Test == &ADProf)
	return ("Edge Feathering Profile");

return ("");

} // WaveEffect::GetCritterName

/*===========================================================================*/

char *WaveEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Wave Texture! Remove anyway?");

} // WaveEffect::OKRemoveRaster

/*===========================================================================*/

int WaveEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
WaveSource *CurWave = WaveSources, *PrevWave = NULL;
NotifyTag Changes[2];

while (CurWave)
	{
	if (CurWave == (WaveSource *)RemoveMe)
		{
		if (PrevWave)
			PrevWave->Next = CurWave->Next;
		else
			WaveSources = CurWave->Next;

		delete CurWave;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevWave = CurWave;
	CurWave = CurWave->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // WaveEffect::RemoveRAHost

/*===========================================================================*/

void WaveEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->WEG)
	{
	delete GlobalApp->GUIWins->WEG;
	}
GlobalApp->GUIWins->WEG = new WaveEditGUI(GlobalApp->AppEffects, GlobalApp->AppDB, this);
if(GlobalApp->GUIWins->WEG)
	{
	GlobalApp->GUIWins->WEG->Open(GlobalApp->MainProj);
	}

} // WaveEffect::Edit

/*===========================================================================*/

int WaveEffect::GetRAHostAnimated(void)
{
WaveSource *Current = WaveSources;
int rVal = 1;

if (GeneralEffect::GetRAHostAnimated())
	goto ReturnIt;	// rVal = 1
while (Current)
	{
	if (Current->GetRAHostAnimated())
		goto ReturnIt;	// rVal = 1
	Current = Current->Next;
	} // while

rVal = 0;

ReturnIt:
return (rVal);

} // WaveEffect::GetRAHostAnimated

/*===========================================================================*/

bool WaveEffect::AnimateMaterials(void)
{
WaveSource *Current = WaveSources;
bool rVal = true;

if (GeneralEffect::AnimateMaterials())
	goto ReturnIt;	// rVal = 1
while (Current)
	{
	if (Current->AnimateMaterials())
		goto ReturnIt;	// rVal = 1
	Current = Current->Next;
	} // while

rVal = 0;

ReturnIt:
return (rVal);

} // WaveEffect::AnimateMaterials

/*===========================================================================*/

int WaveEffect::SetToTime(double Time)
{
WaveSource *Current = WaveSources;
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
while (Current)
	{
	if (Current->SetToTime(Time))
		Found = 1;
	Current = Current->Next;
	} // while

return (Found);

} // WaveEffect::SetToTime

/*===========================================================================*/

long WaveEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	NumImages += CurWave->InitImageIDs(ImageID);
	CurWave = CurWave->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // WaveEffect::InitImageIDs

/*===========================================================================*/

long WaveEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
WaveSource *CurWave = WaveSources;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
while (CurWave)
	{
	if (CurWave->GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
	CurWave = CurWave->Next;
	} // while

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

} // WaveEffect::GetKeyFrameRange

/*===========================================================================*/

char WaveEffect::GetRAHostDropOK(long DropType)
{
char rVal = 1;

if (GeneralEffect::GetRAHostDropOK(DropType))
	goto ReturnIt;	// rVal = 1;
if (DropType == WCS_RAHOST_OBJTYPE_WAVESOURCE)
	goto ReturnIt;	// rVal = 1;
if (DropType == WCS_RAHOST_OBJTYPE_VECTOR
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	goto ReturnIt;	// rVal = 1;

rVal = 0;

ReturnIt:
return (rVal);

} // WaveEffect::GetRAHostDropOK

/*===========================================================================*/

int WaveEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (WaveEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (WaveEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_WAVESOURCE)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddWave((WaveSource *)DropSource->DropSource))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE)
	{
	Success = ADProf.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_VECTOR)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (((Joe *)DropSource->DropSource)->AddEffect(this, -1))
			{
			Success = 1;
			} // if
		} // if
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // WaveEffect::ProcessRAHostDragDrop

/*===========================================================================*/

char *WaveEffect::GetTextureName(long TexNumber)
{

return (TexNumber < GetNumTextures() ? WaveEffectTextureNames[TexNumber]: (char*)"");

} // WaveEffect::GetTextureName

/*===========================================================================*/

RootTexture *WaveEffect::NewRootTexture(long TexNumber)
{
char ApplyToColor = 0;
char ApplyToDisplace = 0;
char ApplyToEcosys = 1;

if (TexNumber < GetNumTextures())
	return (TexRoot[TexNumber] ? TexRoot[TexNumber]:
		(TexRoot[TexNumber] = new RootTexture(this, ApplyToEcosys, ApplyToColor, ApplyToDisplace)));

return (NULL);

} // WaveEffect::NewRootTexture

/*===========================================================================*/

unsigned long WaveEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_WAVE | WCS_RAHOST_FLAGBIT_CHILDREN | 
	WCS_RAHOST_FLAGBIT_DRAGGABLE | WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // WaveEffect::GetRAFlags

/*===========================================================================*/

void WaveEffect::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | 
					WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ);

} // WaveEffect::GetInterFlags

/*===========================================================================*/

int WaveEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
	RootTexture **&TexAffil, ThematicMap **&ThemeAffil)
{
long Ct;

AnimAffil = NULL;
TexAffil = NULL;
ThemeAffil = NULL;

if (ChildA)
	{
	for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
		{
		if (ChildA == GetAnimPtr(Ct))
			{
			AnimAffil = (AnimCritter *)ChildA;
			switch (Ct)
				{
				case WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE:
					{
					TexAffil = GetTexRootPtrAddr(WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
			switch (Ct)
				{
				case WCS_EFFECTS_WAVE_TEXTURE_AMPLITUDE:
					{
					AnimAffil = GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE);
					break;
					} // 
				} // switch
			return (1);
			} // if
		} // for
	for (Ct = 0; Ct < GetNumThemes(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetThemeAddr(Ct))
			{
			ThemeAffil = (ThematicMap **)ChildB;
			return (1);
			} // if
		} // for
	} // else if

return (0);

} // WaveEffect::GetAffiliates

/*===========================================================================*/

double WaveEffect::GetMaxProfileDistance(void)
{
double FindMin, FindMax;

if (ADProf.GetMinMaxDist(FindMin, FindMax))
	return (FindMax);

return (0.0);

} // WaveEffect::GetMaxProfileDistance

/*===========================================================================*/

int WaveEffect::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{

if (! MoveMe)
	{
	return (0);
	} // if

// Camera group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS ||
	Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	return (1);
	} // if 

return (0);	// return 0 if nothing changed

} // WaveEffect::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *WaveEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
WaveSource *CurWave;
JoeList *CurJoe = Joes;

if (! Current)
	return (&ADProf);
if (Current == &ADProf)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Found && GetTexRootPtr(Ct))
		return (GetTexRootPtr(Ct));
	if (Current == GetTexRootPtr(Ct))
		Found = 1;
	} // for
CurWave = WaveSources;
while (CurWave)
	{
	if (Found)
		return (CurWave);
	if (Current == CurWave)
		Found = 1;
	CurWave = CurWave->Next;
	} // while
while (CurJoe)
	{
	if (Found && CurJoe->Me)
		return (CurJoe->Me);
	if (Current == CurJoe->Me)
		Found = 1;
	CurJoe = CurJoe->Next;
	} // while

return (NULL);

} // WaveEffect::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *WaveEffect::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE))
	return (GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE))
	return (GetAnimPtr(WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE));

return (NULL);

} // WaveEffect::GetNextGroupSibling

/*===========================================================================*/

int WaveEffect::GetDeletable(RasterAnimHost *Test)
{
char Ct;
WaveSource *CurWave;

for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (Test == GetTexRootPtr(Ct))
		{
		return (1);
		} // if
	} // for
CurWave = WaveSources;
while (CurWave)
	{
	if (Test == CurWave)
		return (1);
	CurWave = CurWave->Next;
	} // while

return (0);

} // WaveEffect::GetDeletable

/*===========================================================================*/

void WaveEffect::SetPosition(double NewLat, double NewLon)
{
NotifyTag Changes[2];

// this will set new wave center lat/lon.

if (NewLat < 90.0 && NewLat > -90.0)
	{
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetValue(NewLat);
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetValue(NewLon);
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

} // WaveEffect::SetPosition

/*===========================================================================*/

void WaveEffect::SetSourcePosition(WaveSource *SetSource, double NewLat, double NewLon)
{
WaveSource *CurSource = WaveSources;
NotifyTag Changes[2];
double LatScaleMeters, LonScaleMeters, RelX, RelY, TempLon;

// this will set new wave source x and y.

if (NewLat < 90.0 && NewLat > -90.0)
	{
	while (CurSource)
		{
		if (CurSource == SetSource)
			{
			LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
			LonScaleMeters = LonScale(GlobalApp->AppEffects->GetPlanetRadius(), AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue);

			TempLon = NewLon - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
			if (fabs(TempLon) > 180.0)
				{
				TempLon += 180;
				if (fabs(TempLon) >= 360.0)
					TempLon = fmod(TempLon, 360.0);	// retains the original sign
				if (TempLon < 0.0)
					TempLon += 360.0;
				TempLon -= 180.0;
				NewLon = TempLon + AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
				} // if
			// replaced by above
			//if (NewLon > AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue)
			//	{
			//	while (NewLon - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue > 180.0)
			//		NewLon -= 360.0;
			//	} // if
			//else if (NewLon < AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue)
			//	{
			//	while (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - NewLon  > 180.0)
			//		NewLon += 360.0;
			//	} // if
			RelY = (NewLat - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue) * LatScaleMeters;
			RelX = (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - NewLon) * LonScaleMeters;

			CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].SetValue(RelY);
			CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].SetValue(RelX);
			Changes[0] = MAKE_ID(CurSource->GetNotifyClass(), CurSource->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CurSource->GetRAHostRoot());
			break;
			} // if
		CurSource = CurSource->Next;
		} // while
	} // if

} // WaveEffect::SetSourcePosition

/*===========================================================================*/

void WaveEffect::SetFloating(char NewFloating)
{
DEMBounds CurBounds;
NotifyTag Changes[2];

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetValue((CurBounds.North + CurBounds.South) * 0.5);  // Optimized out division. Was / 2.0
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetValue((CurBounds.West + CurBounds.East) * 0.5);  // Optimized out division. Was / 2.0
	} // if
else
	{
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
	AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	} // else
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // WaveEffect::SetFloating

/*===========================================================================*/

int WaveEffect::EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{
long ReadyGo;

Samp->Running = 0;

if (! PlotRast->ThumbnailValid())
	ReadyGo = (long)PlotRast->AllocThumbnail();
else
	{
	ReadyGo = 1;
	PlotRast->ClearThumbnail();
	} // else
Samp->Thumb = PlotRast->Thumb;

if (ReadyGo)
	{
	Samp->PreviewSize = 2000.0;
	Samp->PreviewSize /= pow(2.0, (int)(WavePreviewSize - 1));
	if (! InitToRender(NULL, NULL))
		return (0);
	Samp->SampleInc = GetMaxWaveAmp();
	Samp->y = Samp->zip = 0;
	Samp->Running = 1;
	return (1);
	} // if

return (0);

} // WaveEffect::EvalSampleInit

/*===========================================================================*/

int WaveEffect::EvalOneSampleLine(TextureSampleData *Samp)
{
int X;
double Lat, Lon, WaveX, WaveY, CurTime, WaveHt, LatScaleMeters, LonScaleMeters, MidSample;

CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();
LatScaleMeters = LatScale(GlobalApp->AppEffects->GetPlanetRadius());
LonScaleMeters = cos(AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue * PiOver180) * LatScaleMeters;
MidSample = WCS_RASTER_TNAIL_SIZE / 2;

// evaluate wave amp relative to maximum range possible

WaveY = ((MidSample - Samp->y) / WCS_RASTER_TNAIL_SIZE) * Samp->PreviewSize;
Lat = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue + WaveY / LatScaleMeters;

for (X = 0; X < WCS_RASTER_TNAIL_SIZE; X ++, Samp->zip ++)
	{
	if (Samp->SampleInc <= 0.0)
		{
		Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = 128;
		} // if
	else
		{
		WaveX = ((MidSample - X) / WCS_RASTER_TNAIL_SIZE) * Samp->PreviewSize;
		if (LonScaleMeters > 0.0)
			Lon = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue + WaveX / LonScaleMeters;
		else
			Lon = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue;
		WaveHt = .5 + .5 * EvalSampleHeight(LatScaleMeters, CurTime, Lat, Lon) / Samp->SampleInc;
		Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = 
			Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = (unsigned char)(WaveHt * 255);
		} // else
	} // for

Samp->y ++;
if (Samp->y < WCS_RASTER_TNAIL_SIZE)
	return (0);

Samp->Running = 0;
return (1);

} // WaveEffect::EvalOneSampleLine

/*===========================================================================*/

double WaveEffect::EvalSampleHeight(double EarthLatScaleMeters, double CurTime, double Lat, double Lon)
{
short WaveNum, SourceCt;
long PtCt;
double WaveAmp = 0.0, Elev = 0.0, TempAmp, RelX, RelY, lonscale, DistX, DistY, DistZ, EnvTimeOffset = 0.0;
WaveSource *CurWave;
JoeList *CurJoe;
VectorPoint *PtLink;
JoeCoordSys *MyAttr;
CoordSys *MyCoords;
VertexDEM MyVert;


if (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue > 0.0 && WaveSourceArray)
	{
	// compute vertex offset from wave effect coords
	lonscale = cos(Lat * PiOver180) * EarthLatScaleMeters;
	RelY = (Lat - AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue) * EarthLatScaleMeters;
	RelX = (AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - Lon) * lonscale;

	if (CurJoe = Joes)
		{
		while (CurJoe)
			{
			if (CurJoe->Me && CurJoe->Me->TestFlags(WCS_JOEFLAG_ACTIVATED))
				{
				TempAmp = 0.0;
				if (WaveEffectType == WCS_WAVE_WAVETYPE_POINT)
					{
					// apply sources at vector vertices
					if (PtLink = CurJoe->Me->GetFirstRealPoint())
						{
						if (MyAttr = (JoeCoordSys *)CurJoe->Me->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)) 
							MyCoords = MyAttr->Coord;
						else
							MyCoords = NULL;
						for (PtCt = 0; PtLink; PtCt ++, PtLink = PtLink->Next)
							{
							if (PtLink->ProjToDefDeg(MyCoords, &MyVert))
								{
								RelY = (Lat - MyVert.Lat) * EarthLatScaleMeters;
								RelX = (MyVert.Lon - Lon) * lonscale;
								
								// seed the random number generator
								Rand.Seed64BitShift(PtCt * 1371 + WCS_SEEDOFFSET_WAVE, PtCt * 897 + WCS_SEEDOFFSET_WAVE);

								// determine a random envelope start time
								if (RandomizeEnvStart)
									{
									if (RandomizeType == WCS_WAVE_RANDOMIZETYPE_POISSON)
										{
										EnvTimeOffset = 2.0 * (Rand.GenPRN() - .5) * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // if
									else
										{
										EnvTimeOffset = Rand.GenGauss() * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE].CurValue;
										} // else
									} // if
								// add per vertex delay
								EnvTimeOffset += PtCt * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY].CurValue;

								// roll dice to see which wave sources get evaluated
								WaveNum = (short)(Rand.GenPRN() * NumSources);
								for (SourceCt = 0; SourceCt < WaveSourcesPerVertex; SourceCt ++, WaveNum ++)
									{
									if (WaveNum >= NumSources)
										WaveNum = 0;
									TempAmp += WaveSourceArray[WaveNum]->EvalSampleHeight(RelX, RelY, CurTime, EnvTimeOffset);
									} // for
								} // if
							} // for
						} // if
					} // if point style
				else
					{
					// find out if vertex coords are within joe outline
					if (CurJoe->Me->SimpleContained(Lat, Lon))
						{
						// evaluate each wave source
						CurWave = WaveSources;
						while (CurWave)
							{
							if (CurWave->Enabled)
								TempAmp += CurWave->EvalSampleHeight(RelX, RelY, CurTime, 0.0);
							CurWave = CurWave->Next;
							} // while
						// reduce amplitude if there is a gradient profile
						if (UseGradient)
							{
							// this undoubtedly isn't the most efficient way to get a distance but it
							// doesn't require georasters and it is more accurate than using georasters.
							DistX = fabs(CurJoe->Me->MinDistToPoint(Lat, Lon, Elev, EarthLatScaleMeters, 1, 0.0, 1.0, DistX, DistY, DistZ));
							TempAmp *= ADProf.GetValue(0, DistX);
							} // if
						} // if
					} // else line style
				WaveAmp += TempAmp;
				} // if
			CurJoe = CurJoe->Next;
			} // while
		} // if Joes
	else
		{
		// no Joes, treat same as if it is inbounds but no profile gradient
		// evaluate each wave source
		CurWave = WaveSources;
		while (CurWave)
			{
			if (CurWave->Enabled)
				WaveAmp += CurWave->EvalSampleHeight(RelX, RelY, CurTime, 0.0);
			CurWave = CurWave->Next;
			} // while
		} // else no Joes

	// scale wave amp by amplitude factor
	TempAmp = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue;
	// no analyze amplitude textures for sample
	return (WaveAmp * TempAmp);
	} // if amplitude > 0

return (0.0);

} // WaveEffect::EvalSampleHeight

/*===========================================================================*/

double WaveSource::EvalSampleHeight(double RelX, double RelY, double CurTime, double EnvTimeOffset)
{
double WaveAmp, DistX, DistY, Dist, Phase, Frequency, WaveTime, t0, EnvStartDist, EnvFirstDist, EnvLastDist, EnvDist, 
	EnvAmp, TempVal, EnvWidth;

if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue > 0.0)
	{
	// this could really be calculated once per time slice since it doesn't depend on any local variables
	Frequency = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY].CurValue / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue;

	// calculate vertex distance from wave source
	DistX = RelX - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX].CurValue;
	DistY = RelY - AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY].CurValue;
	Dist = sqrt(DistX * DistX + DistY * DistY);

	if (EnvelopeEnabled)
		{
		EnvDist = Dist;
		// this is for static envelopes used for making falloff from the source point
		if (AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue == 0.0)
			{
			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvLastDist) && EnvLastDist > 0.0)
				{
				EnvWidth = EnvLastDist - EnvFirstDist;
				if (Dist > EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = EnvFirstDist + fmod((Dist - EnvFirstDist), EnvWidth);
					} // if
				else if (Dist < EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvLastDist - fmod((EnvLastDist - Dist), EnvWidth);
					} // if
				} // if
			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// no analyze phase textures for sample
				Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // if
		else
			{
			// this is for moving envelopes like would occur if a rock were dropped in the water
			t0 = EnvTimeOffset + AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME].CurValue;
			WaveTime = CurTime - t0;
			EnvStartDist = WaveTime * AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue;

			if (Envelope.GetMinMaxDist(EnvFirstDist, EnvWidth) && EnvWidth > 0.0)
				{
				EnvFirstDist = EnvStartDist;
				EnvLastDist = EnvFirstDist - EnvWidth;
				if (Dist > EnvFirstDist)
					{
					if (RepeatEnvelopeBefore)
						EnvDist = EnvFirstDist - (EnvLastDist + fmod((Dist - EnvLastDist), EnvWidth));
					else
						EnvDist = 0.0;
					} // if
				else if (Dist < EnvLastDist)
					{
					if (RepeatEnvelopeAfter)
						EnvDist = fmod((EnvFirstDist - Dist), EnvWidth);
					else
						EnvDist = EnvWidth;
					} // if
				else
					EnvDist = EnvFirstDist - Dist;
				} // if
			if ((EnvAmp = Envelope.GetValue(0, EnvDist)) > 0.0)
				{
				TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
				// no analyze phase textures for sample
				Phase = ((WaveTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
					TempVal) * TwoPi;
				WaveAmp = sin(Phase) * EnvAmp;
				} // if
			else
				return (0.0);
			} // else
		} // if
	else
		{
		TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE].CurValue;
		// no analyze phase textures for sample
		Phase = ((CurTime * Frequency) - (Dist / AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH].CurValue) + 
			TempVal) * TwoPi;
		WaveAmp = sin(Phase);
		} // else

	// scale the result and return
	TempVal = AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	// no analyze amplitude textures for sample
	return (WaveAmp * TempVal);
	} // if amplitude > 0

return (0.0);

} // WaveSource::EvalSampleHeight

/*===========================================================================*/

int WaveEffect::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
short Ct;	// NumSources is a short
WaveSource *CurWave;

// delete array of wave pointers if it exists
if (WaveSourceArray)
	{
	AppMem_Free(WaveSourceArray, NumSources * sizeof (WaveSource *));
	WaveSourceArray = NULL;
	} // if

// create an array of wave source pointers
// count enabled wave sources
// NumSources and WaveSourceArray are members of WaveEffect
CurWave = WaveSources;
NumSources = 0;
while (CurWave)
	{
	if (CurWave->Enabled)
		{
		NumSources ++;
		} // if
	CurWave = CurWave->Next;
	} // while

// add a wave source if none exists
if (! NumSources)
	{
	return (1);
	} // if

// allocate wave source array
if (WaveSourceArray = (WaveSource **)AppMem_Alloc(NumSources * sizeof (WaveSource *), 0))
	{
	// assign each enabled source to an element of the array
	for (Ct = 0, CurWave = WaveSources; Ct < NumSources; CurWave = CurWave->Next)
		{
		if (CurWave->Enabled)
			{
			WaveSourceArray[Ct] = CurWave;
			Ct ++;
			} // if
		} // for
	} // if
else
	return (0);

// init textures
for (Ct = 0; Ct < GetNumTextures(); Ct ++)
	{
	if (TexRoot[Ct] && TexRoot[Ct]->Enabled)
		{
		if (! TexRoot[Ct]->InitAAChain())
			{
			return (0);
			} // if
		} // if
	} // for
CurWave = WaveSources;
while (CurWave)
	{
	if (CurWave->Enabled)
		{
		if (! CurWave->InitToRender())
			return (0);
		} // if
	CurWave = CurWave->Next;
	} // while

return (1);

} // WaveEffect::InitToRender

/*===========================================================================*/

double WaveEffect::GetMaxWaveAmp(void)
{
double MaxAmp = 0.0;
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	if (CurWave->Enabled)
		MaxAmp += CurWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue;
	CurWave = CurWave->Next;
	} // while

return (MaxAmp * AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE].CurValue);

} // WaveEffect::GetMaxWaveAmp

/*===========================================================================*/

void WaveEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ScaleWE, ScaleNS, TempVal;
GraphNode *CurNode;

if (OldBounds->West > OldBounds->East)
	ScaleWE = (CurBounds->West - CurBounds->East) / (OldBounds->West - OldBounds->East);
else
	ScaleWE = 1.0;
if (OldBounds->North > OldBounds->South)
	ScaleNS = (CurBounds->North - CurBounds->South) / (OldBounds->North - OldBounds->South);
else
	ScaleNS = 1.0;

AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].SetValue(
	(AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].CurValue - OldBounds->South) * ScaleNS + CurBounds->South);
if (CurNode = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].GetFirstNode(0))
	{
	TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
	if (TempVal > 89.0)
		TempVal = 89.0;
	if (TempVal < -89.0)
		TempVal = -89.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE].GetNextNode(0, CurNode))
		{
		TempVal = (CurNode->GetValue() - OldBounds->South) * ScaleNS + CurBounds->South;
		if (TempVal > 89.0)
			TempVal = 89.0;
		if (TempVal < -89.0)
			TempVal = -89.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].SetValue(
	(AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].CurValue - OldBounds->East) * ScaleWE + CurBounds->East);
if (CurNode = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].GetFirstNode(0))
	{
	CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
	while (CurNode = AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE].GetNextNode(0, CurNode))
		{
		CurNode->SetValue((CurNode->GetValue() - OldBounds->East) * ScaleWE + CurBounds->East);
		} // while
	} // if

} // WaveEffect::ScaleToDEMBounds

/*===========================================================================*/

int WaveEffect::BuildFileComponentsList(EffectList **Queries, EffectList **Themes, EffectList **Coords)
{
WaveSource *CurWave = WaveSources;

while (CurWave)
	{
	if (! CurWave->BuildFileComponentsList(Coords))
		return (0);
	CurWave = CurWave->Next;
	} // while

return (GeneralEffect::BuildFileComponentsList(Queries, Themes, Coords));

} // WaveEffect::BuildFileComponentsList

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int WaveEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
WaveEffect *CurrentWave = NULL;
SearchQuery *CurrentQuery = NULL;
ThematicMap *CurrentTheme = NULL;
CoordSys *CurrentCoordSys = NULL;
DEMBounds OldBounds, CurBounds;

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
						if ((BytesRead = OldBounds.Load(ffile, Size, ByteFlip)) == Size)
							OldBoundsLoaded = 1;
						} // if material
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if material
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoordSys = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoordSys->Load(ffile, Size, ByteFlip);
							}
						} // if CoordSys
					else if (! strnicmp(ReadBuf, "Search", 8))
						{
						if (CurrentQuery = new SearchQuery(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentQuery->Load(ffile, Size, ByteFlip);
							}
						} // if search query
					else if (! strnicmp(ReadBuf, "ThemeMap", 8))
						{
						if (CurrentTheme = new ThematicMap(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentTheme->Load(ffile, Size, ByteFlip);
							}
						} // if thematic map
					else if (! strnicmp(ReadBuf, "Wave", 8))
						{
						if (CurrentWave = new WaveEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentWave->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if 3d object
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

if (Success == 1 && CurrentWave)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Wave Model", "Do you wish the loaded Wave Model's position\n to be scaled to current DEM bounds?"))
			CurrentWave->ScaleToDEMBounds(&OldBounds, &CurBounds);
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentWave);
	strcpy(ReadBuf, Name);
	SetUniqueName(GlobalApp->AppEffects, ReadBuf);
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

} // WaveEffect::LoadObject

/*===========================================================================*/

int WaveEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *Queries = NULL, *Themes = NULL, *Coords = NULL;
DEMBounds CurBounds;

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

if (BuildFileComponentsList(&Queries, &Themes, &Coords)
	&& GeneralEffect::BuildFileComponentsList(&Queries, &Themes, &Coords))
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

	CurEffect = Queries;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Search");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SearchQuery *)CurEffect->Me)->Save(ffile))
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
						} // if SearchQuery saved 
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

	#ifdef WCS_THEMATIC_MAP
	CurEffect = Themes;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "ThemeMap");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ThematicMap *)CurEffect->Me)->Save(ffile))
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
						} // if ThemeMap saved 
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
	#endif // WCS_THEMATIC_MAP

	while (Coords)
		{
		CurEffect = Coords;
		Coords = Coords->Next;
		delete CurEffect;
		} // while
	while (Queries)
		{
		CurEffect = Queries;
		Queries = Queries->Next;
		delete CurEffect;
		} // while
	while (Themes)
		{
		CurEffect = Themes;
		Themes = Themes->Next;
		delete CurEffect;
		} // while
	} // if

// Wave
strcpy(StrBuf, "Wave");
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
			} // if Wave saved 
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

} // WaveEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::WaveEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
WaveEffect *Current;

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
					} /* switch */

				switch (ItemTag & 0xffff0000)
					{
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new WaveEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (WaveEffect *)FindDuplicateByName(Current->EffectType, Current))
								{
								RemoveRAHost(Current, 1);
								Current = NULL;
								} // if
							}
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

} // EffectsLib::WaveEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::WaveEffect_Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
WaveEffect *Current;

Current = Wave;
while (Current)
	{
	if (! Current->TemplateItem)
		{
		ItemTag = WCS_EFFECTSBASE_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = Current->Save(ffile))
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
					} // if Wave effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (WaveEffect *)Current->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // EffectsLib::WaveEffect_Save()

/*===========================================================================*/
