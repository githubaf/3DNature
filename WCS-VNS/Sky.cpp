// Sky.cpp
// For managing WCS Skies
// Built from scratch on 6/14/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "SkyEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "GraphData.h"
#include "requester.h"
#include "Raster.h"
#include "Database.h"
#include "Security.h"

/*===========================================================================*/

Sky::Sky()
: GeneralEffect(NULL), SkyGrad(this, 0), LightGrad(this, 0)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SKY;
SetDefaults();

} // Sky::Sky

/*===========================================================================*/

Sky::Sky(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), SkyGrad(this, 0), LightGrad(this, 0)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_SKY;
SetDefaults();

} // Sky::Sky

/*===========================================================================*/

Sky::Sky(RasterAnimHost *RAHost, EffectsLib *Library, Sky *Proto)
: GeneralEffect(RAHost), SkyGrad(this, 0), LightGrad(this, 0)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_SKY;
Prev = Library->LastSky;
if (Library->LastSky)
	{
	Library->LastSky->Next = this;
	Library->LastSky = this;
	} // if
else
	{
	Library->Skies = Library->LastSky = this;
	} // else
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
	strcpy(NameBase, "Sky");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // Sky::Sky

/*===========================================================================*/

Sky::~Sky()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->KPG && GlobalApp->GUIWins->KPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->KPG;
		GlobalApp->GUIWins->KPG = NULL;
		} // if
	} // if

if (DitherTable)
	delete [] DitherTable;
	
} // Sky::~Sky

/*===========================================================================*/

void Sky::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_SKY_NUMANIMPAR] = {1.0, .03};
double RangeDefaults[WCS_EFFECTS_SKY_NUMANIMPAR][3] = {
												1.0, 0.0, .01,	// intensity
												1.0, 0.0, .01		// dithering
												};
long Ct;
GradientCritter *CurGrad;

DitherTable = NULL;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
if (CurGrad = SkyGrad.AddNode(0.0))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(111.0 / 255.0, 172.0 / 255.0, 242.0 / 255.0);
	} // if
if (CurGrad = SkyGrad.AddNode(.5))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(240.0 / 255.0, 244.0 / 255.0, 247.0 / 255.0);
	CurGrad->BlendStyle = WCS_GRADIENTCRITTER_BLENDSTYLE_SLOWINCREASE;
	} // if
// for light gradient 1 is away from light, 0 is toward light
if (CurGrad = LightGrad.AddNode(0.0))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(90.0 / 255.0, 90.0 / 255.0, 90.0 / 255.0);
	} // if
if (CurGrad = LightGrad.AddNode(.5))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(0.0, 0.0, 0.0);
	CurGrad->BlendStyle = WCS_GRADIENTCRITTER_BLENDSTYLE_FASTINCREASE;
	} // if

AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].SetMultiplier(100.0);

} // Sky::SetDefaults

/*===========================================================================*/

void Sky::Copy(Sky *CopyTo, Sky *CopyFrom)
{

SkyGrad.Copy(&CopyTo->SkyGrad, &CopyFrom->SkyGrad);
LightGrad.Copy(&CopyTo->LightGrad, &CopyFrom->LightGrad);
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // Sky::Copy

/*===========================================================================*/

ULONG Sky::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_EFFECTS_SKY_INTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SKY_PIXELDITHER:
						{
						BytesRead = AnimPar[WCS_EFFECTS_SKY_ANIMPAR_PIXELDITHER].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SKY_SKYCOLOR:
						{
						BytesRead = SkyGrad.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SKY_LIGHTCOLOR:
						{
						BytesRead = LightGrad.Load(ffile, Size, ByteFlip);
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

} // Sky::Load

/*===========================================================================*/

unsigned long int Sky::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_EFFECTS_SKY_NUMANIMPAR] = {WCS_EFFECTS_SKY_INTENSITY,
																 WCS_EFFECTS_SKY_PIXELDITHER
																 };

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

ItemTag = WCS_EFFECTS_SKY_SKYCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = SkyGrad.Save(ffile))
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

ItemTag = WCS_EFFECTS_SKY_LIGHTCOLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = LightGrad.Save(ffile))
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

} // Sky::Save

/*===========================================================================*/

void Sky::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->KPG)
	{
	delete GlobalApp->GUIWins->KPG;
	}
GlobalApp->GUIWins->KPG = new SkyEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->KPG)
	{
	GlobalApp->GUIWins->KPG->Open(GlobalApp->MainProj);
	}

} // Sky::Edit

/*===========================================================================*/

char *SkyCritterNames[WCS_EFFECTS_SKY_NUMANIMPAR] = {"Sky Intensity (%)", "Pixel Dithering (%)"};

char *Sky::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (SkyCritterNames[Ct]);
	} // for
if (Test == &SkyGrad)
	return ("Sky Colors");
if (Test == &LightGrad)
	return ("Sky Light Colors");

return ("");

} // Sky::GetCritterName

/*===========================================================================*/

int Sky::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (SkyGrad.GetRAHostAnimated())
	return (1);
if (LightGrad.GetRAHostAnimated())
	return (1);

return (0);

} // Sky::GetRAHostAnimated

/*===========================================================================*/

int Sky::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (SkyGrad.SetToTime(Time))
	Found = 1;
if (LightGrad.SetToTime(Time))
	Found = 1;

return (Found);

} // Sky::SetToTime

/*===========================================================================*/

long Sky::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;

if (GeneralEffect::GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (SkyGrad.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if
if (LightGrad.GetKeyFrameRange(MinDist, MaxDist))
	{
	if (MinDist < TestFirst)
		TestFirst = MinDist;
	if (MaxDist > TestLast)
		TestLast = MaxDist;
	Found = 1;
	} // if

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

} // Sky::GetKeyFrameRange

/*===========================================================================*/

char Sky::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT
	|| DropType == WCS_RAHOST_OBJTYPE_COLORTEXTURE
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	return (1);

return (0);

} // Sky::GetRAHostDropOK

/*===========================================================================*/

int Sky::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
int QueryResult, Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];
char QueryStr[256], NameStr[128];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Sky *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Sky *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT ||
	DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_COLORTEXTURE ||
	DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = -1;
	sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, "which Color");
	if (QueryResult = UserMessageCustom(NameStr, QueryStr, "Sky", "Cancel", "Sky Light", 0))
		{
		if (QueryResult == 1)
			{
			Success = SkyGrad.ProcessRAHostDragDrop(DropSource);
			} // if
		else if (QueryResult == 2)
			{
			Success = LightGrad.ProcessRAHostDragDrop(DropSource);
			} // else if
		} // if
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // Sky::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long Sky::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_SKY | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // Sky::GetRAFlags

/*===========================================================================*/

RasterAnimHost *Sky::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&SkyGrad);
if (Current == &SkyGrad)
	return (&LightGrad);
if (Current == &LightGrad)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for

return (NULL);

} // Sky::GetRAHostChild

/*===========================================================================*/

int Sky::InitToRender(RenderOpt *Opt, BufferNode *Buffers)
{
int rVal = 1;

if (DitherTable)
	delete [] DitherTable;
DitherTable = NULL;

if (! SkyGrad.InitToRender())
	rVal = 0;
if (! LightGrad.InitToRender())
	rVal = 0;

return(rVal);

} // Sky::InitToRender

/*===========================================================================*/

void Sky::CleanupFromRender(void)
{

if (DitherTable)
	delete [] DitherTable;
DitherTable = NULL;

} // Sky::CleanupFromRender

/*===========================================================================*/

int Sky::EvalSampleInit(Raster *PlotRast, TextureSampleData *Samp)
{
int rVal = 0;
bool ReadyGo = false;

Samp->Running = 0;
Samp->XHInc = 1.0 / (WCS_RASTER_TNAIL_SIZE - 1);
Samp->YVInc = .5 / (WCS_RASTER_TNAIL_SIZE - 1);

if (! PlotRast->ThumbnailValid())
	{
	if (PlotRast->AllocThumbnail())
		ReadyGo = true;
	} // if
else
	{
	ReadyGo = true;
	PlotRast->ClearThumbnail();
	} // else
Samp->Thumb = PlotRast->Thumb;

if (ReadyGo)
	{
	Samp->y = Samp->zip = 0;
	Samp->StartY = 0.0;
	Samp->Running = 1;
	rVal = 1;
	} // if

return(rVal);

} // Sky::EvalSampleInit

/*===========================================================================*/

int Sky::EvalOneSampleLine(TextureSampleData *Samp)
{
double SumValue[3], SampleX, LightValue[3], SkyValue[3];
Light *FirstLight, *CurLight;
long x;

FirstLight = (Light *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT);

SkyGrad.GetBasicColor(SkyValue[0], SkyValue[1], SkyValue[2], .5 - .5 * cos(Samp->StartY * Pi));
for (x = 0, SampleX = 0.0; x < WCS_RASTER_TNAIL_SIZE; x ++, Samp->zip ++, SampleX += Samp->XHInc)
	{
	SumValue[0] = SkyValue[0];
	SumValue[1] = SkyValue[1];
	SumValue[2] = SkyValue[2];
	LightGrad.GetBasicColor(LightValue[0], LightValue[1], LightValue[2], .5 - .5 * cos(SampleX * Pi));
	for (CurLight = FirstLight; CurLight; CurLight = (Light *)CurLight->Next)
		{
		if (CurLight->Enabled && CurLight->Distant)
			{
			SumValue[0] += LightValue[0] * CurLight->Color.GetCompleteValue(0);
			SumValue[1] += LightValue[1] * CurLight->Color.GetCompleteValue(1);
			SumValue[2] += LightValue[2] * CurLight->Color.GetCompleteValue(2);
			} // if
		} // for
	SumValue[0] *= 255.99 * AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
	SumValue[1] *= 255.99 * AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
	SumValue[2] *= 255.99 * AnimPar[WCS_EFFECTS_SKY_ANIMPAR_INTENSITY].CurValue;
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_RED][Samp->zip] = (UBYTE)min(255, SumValue[0]);
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_GREEN][Samp->zip] = (UBYTE)min(255, SumValue[1]);
	Samp->Thumb->TNail[WCS_RASTER_IMAGE_BAND_BLUE][Samp->zip] = (UBYTE)min(255, SumValue[2]);
	} // for

Samp->y ++;
Samp->StartY += Samp->YVInc;

if (Samp->y < WCS_RASTER_TNAIL_SIZE)
	return (0);

Samp->Running = 0;
return (1);

} // Sky::EvalOneSampleLine

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int Sky::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
Sky *CurrentSky = NULL;
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];

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
						} // if DEMBnds
					else if (! strnicmp(ReadBuf, "Images", 8))
						{
						BytesRead = GlobalApp->LoadToImageLib->Load(ffile, Size, NULL);
						} // if Images
					else if (! strnicmp(ReadBuf, "Sky", 8))
						{
						if (CurrentSky = new Sky(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentSky->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if Sky
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

if (Success == 1 && CurrentSky)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentSky);
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

} // Sky::LoadObject

/*===========================================================================*/

int Sky::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
DEMBounds CurBounds;
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

// Sky
strcpy(StrBuf, "Sky");
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
			} // if Sky saved 
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

} // Sky::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::Sky_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
Sky *Current;

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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new Sky(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (Sky *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::Sky_Load()

/*===========================================================================*/

ULONG EffectsLib::Sky_Save(FILE *ffile)
{
Sky *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Skies;
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
					} // if Sky saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (Sky *)Current->Next;
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

} // EffectsLib::Sky_Save()

/*===========================================================================*/
