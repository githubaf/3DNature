// EffectStarField.cpp
// For managing StarField Effects
// Built from scratch StarFieldEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "Application.h"
#include "StarfieldEditGUI.h"
#include "Project.h"
#include "Useful.h"
#include "Toolbar.h"
#include "Raster.h"
#include "EffectsLib.h"
#include "Requester.h"
#include "EffectsIO.h"
#include "Joe.h"
#include "Conservatory.h"
#include "DragnDropListGUI.h"
#include "Database.h"
#include "Security.h"

StarFieldEffect::StarFieldEffect()
: GeneralEffect(NULL), ColorGrad(this, 0)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_STARFIELD;
SetDefaults();

} // StarFieldEffect::StarFieldEffect

/*===========================================================================*/

StarFieldEffect::StarFieldEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost), ColorGrad(this, 0)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_STARFIELD;
SetDefaults();

} // StarFieldEffect::StarFieldEffect

/*===========================================================================*/

StarFieldEffect::StarFieldEffect(RasterAnimHost *RAHost, EffectsLib *Library, StarFieldEffect *Proto)
: GeneralEffect(RAHost), ColorGrad(this, 0)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_STARFIELD;
Prev = Library->LastStarField;
if (Library->LastStarField)
	{
	Library->LastStarField->Next = this;
	Library->LastStarField = this;
	} // if
else
	{
	Library->StarField = Library->LastStarField = this;
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
	strcpy(NameBase, "Starfield");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // StarFieldEffect::StarFieldEffect

/*===========================================================================*/

StarFieldEffect::~StarFieldEffect()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->STG && GlobalApp->GUIWins->STG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->STG;
		GlobalApp->GUIWins->STG = NULL;
		} // if
	} // if

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	// Img may be NULLed by RemoveAttribute()
	if (Img)
		delete Img;
	Img = NULL;
	} // if
	
} // StarFieldEffect::~StarFieldEffect

/*===========================================================================*/

void StarFieldEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_STARFIELD_NUMANIMPAR] = {1.0, .6, .7, 1.0, 0.0, .5};
double RangeDefaults[WCS_EFFECTS_STARFIELD_NUMANIMPAR][3] = {10.0, 0.0, .01,
																10.0, 0.0, .01,
																1.0, 0.0, .01,
																1.0, 0.0, .01,
																FLT_MAX, -FLT_MAX, 1.0,
																1.0, 0.0, .01};
long Ct;
GradientCritter *CurGrad;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
if (CurGrad = ColorGrad.AddNode(0.0))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(255.0 / 255.0, 229.0 / 255.0, 224.0 / 255.0);
	} // if
if (CurGrad = ColorGrad.AddNode(0.25))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(234.0 / 255.0, 244.0 / 255.0, 255.0 / 255.0);
	} // if
if (CurGrad = ColorGrad.AddNode(0.5))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(255.0 / 255.0, 255.0 / 255.0, 255.0 / 255.0);
	} // if
if (CurGrad = ColorGrad.AddNode(.75))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(255.0 / 255.0, 255.0 / 255.0, 216.0 / 255.0);
	} // if
if (CurGrad = ColorGrad.AddNode(1.0))
	{
	((ColorTextureThing *)CurGrad->GetThing())->Color.SetValue3(255.0 / 255.0, 229.0 / 255.0, 224.0 / 255.0);
	} // if
Img = NULL;
RandomField = 1;
RandomSeed = (long)(xrand48() * 1000);

AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP].SetMultiplier(100.0);

} // StarFieldEffect::SetDefaults

/*===========================================================================*/

void StarFieldEffect::Copy(StarFieldEffect *CopyTo, StarFieldEffect *CopyFrom)
{
Raster *NewRast;

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

ColorGrad.Copy(&CopyTo->ColorGrad, &CopyFrom->ColorGrad);
CopyTo->RandomField = CopyFrom->RandomField;
CopyTo->RandomSeed = CopyFrom->RandomSeed;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // StarFieldEffect::Copy

/*===========================================================================*/

char *StarFieldEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Starfield image! Remove anyway?");

} // StarFieldEffect::OKRemoveRaster

/*===========================================================================*/

void StarFieldEffect::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img && Img == Shell)
	{
	Img = NULL;
	} // if

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // StarFieldEffect::RemoveRaster

/*===========================================================================*/

int StarFieldEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	return (1);
	} // if

return (0);

} // StarFieldEffect::RemoveRAHost

/*===========================================================================*/

int StarFieldEffect::SetRaster(Raster *NewRast)
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

} // StarFieldEffect::SetRaster

/*===========================================================================*/

ULONG StarFieldEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_EFFECTS_STARFIELD_DENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_DENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_SIZEFACTOR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_SIZEFACTOR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_INTENSITY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_INTENSITYRANGE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_INTENSITYRANGE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_LONGITUDEROT:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_TWINKLEAMP:
						{
						BytesRead = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_TWINKLEAMP].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_COLOR:
						{
						BytesRead = ColorGrad.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_RANDOMFIELD:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomField, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_STARFIELD_RANDOMSEED:
						{
						BytesRead = ReadBlock(ffile, (char *)&RandomSeed, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_IMAGEID:
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

} // StarFieldEffect::Load

/*===========================================================================*/

unsigned long int StarFieldEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long ImageID;
unsigned long int AnimItemTag[WCS_EFFECTS_CELESTIAL_NUMANIMPAR] = {WCS_EFFECTS_STARFIELD_DENSITY,
																	WCS_EFFECTS_STARFIELD_SIZEFACTOR,
																	WCS_EFFECTS_STARFIELD_INTENSITY,
																	WCS_EFFECTS_STARFIELD_INTENSITYRANGE,
																	WCS_EFFECTS_STARFIELD_LONGITUDEROT,
																	WCS_EFFECTS_STARFIELD_TWINKLEAMP};

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

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_STARFIELD_RANDOMFIELD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RandomField)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_STARFIELD_RANDOMSEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&RandomSeed)) == NULL)
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
					} /* if wrote size of block */
				else
					goto WriteError;
				} /* if anim param saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;
	} // for

ItemTag = WCS_EFFECTS_STARFIELD_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = ColorGrad.Save(ffile))
			{
			TotalWritten += BytesWritten;
			fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
			if (WriteBlock(ffile, (char *)&BytesWritten,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				fseek(ffile, 0, SEEK_END);
				} /* if wrote size of block */
			else
				goto WriteError;
			} /* if anim color saved */
		else
			goto WriteError;
		} /* if size written */
	else
		goto WriteError;
	} /* if tag written */
else
	goto WriteError;

if (Img && (ImageID = Img->GetRasterID()) > 0)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_IMAGEID, WCS_BLOCKSIZE_CHAR,
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

} // StarFieldEffect::Save

/*===========================================================================*/

void StarFieldEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->STG)
	{
	delete GlobalApp->GUIWins->STG;
	}
GlobalApp->GUIWins->STG = new StarfieldEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->STG)
	{
	GlobalApp->GUIWins->STG->Open(GlobalApp->MainProj);
	}

} // StarFieldEffect::Edit

/*===========================================================================*/

char *StarFieldEffectCritterNames[WCS_EFFECTS_STARFIELD_NUMANIMPAR] = {"Density (%)",
										"Size Factor (%)", "Intensity (%)",
										"Intensity Range (%)", "Longitude Rotation (Deg)",
										"Twinkle (%)"};

char *StarFieldEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (StarFieldEffectCritterNames[Ct]);
	} // for
if (Test == &ColorGrad)
	return ("Colors");

return ("");

} // StarFieldEffect::GetCritterName

/*===========================================================================*/

int StarFieldEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (ColorGrad.GetRAHostAnimated())
	return (1);

return (0);

} // StarFieldEffect::GetRAHostAnimated

/*===========================================================================*/

int StarFieldEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (ColorGrad.SetToTime(Time))
	Found = 1;

return (Found);

} // StarFieldEffect::SetToTime

/*===========================================================================*/

long StarFieldEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (ColorGrad.GetKeyFrameRange(MinDist, MaxDist))
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

} // StarFieldEffect::GetKeyFrameRange

/*===========================================================================*/

long StarFieldEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	NumImages ++;
	} // if
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // StarFieldEffect::InitImageIDs

/*===========================================================================*/

char StarFieldEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT
	|| DropType == WCS_RAHOST_OBJTYPE_COLORTEXTURE
	|| DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER)
	return (1);

return (0);

} // StarFieldEffect::GetRAHostDropOK

/*===========================================================================*/

int StarFieldEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (StarFieldEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (StarFieldEffect *)DropSource->DropSource);
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
	Success = ColorGrad.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = SetRaster((Raster *)DropSource->DropSource);
	} // else if
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // StarFieldEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long StarFieldEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_STAR | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // StarFieldEffect::GetRAFlags

/*===========================================================================*/

RasterAnimHost *StarFieldEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	return (&ColorGrad);
if (Current == &ColorGrad)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
if (Found && Img && Img->GetRaster())
	return (Img->GetRaster());

return (NULL);

} // StarFieldEffect::GetRAHostChild

/*===========================================================================*/

void StarFieldEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ShiftWE;
GraphNode *CurNode;

ShiftWE = ((CurBounds->West + CurBounds->East) - (OldBounds->West - OldBounds->East)) * 0.5;

AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].SetValue(
	AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_STARFIELD_ANIMPAR_LONGITUDEROT].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if

} // StarFieldEffect::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int StarFieldEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
StarFieldEffect *CurrentStar = NULL;
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
						} // if Images
					else if (! strnicmp(ReadBuf, "StarFld", 8))
						{
						if (CurrentStar = new StarFieldEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentStar->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if CelestialEffect
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

if (Success == 1 && CurrentStar)
	{
	if (OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (UserMessageYN("Load Starfield", "Do you wish the loaded Starfield's longitude\n to be scaled to current DEM bounds?"))
			{
			CurrentStar->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentStar);
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

} // StarFieldEffect::LoadObject

/*===========================================================================*/

int StarFieldEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
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

// StarFieldEffect
strcpy(StrBuf, "StarFld");
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
			} // if StarFieldEffect saved 
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

} // StarFieldEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::StarFieldEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
StarFieldEffect *Current;

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
						if (Current = new StarFieldEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (StarFieldEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::StarFieldEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::StarFieldEffect_Save(FILE *ffile)
{
StarFieldEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = StarField;
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
					} // if StarField effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (StarFieldEffect *)Current->Next;
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

} // EffectsLib::StarFieldEffect_Save()

/*===========================================================================*/
