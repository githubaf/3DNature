// EffectCelestial.cpp
// For managing Celestial Effects
// Built from scratch CelestialEffect.cpp on 06/12/97 by Gary R. Huber
// Copyright 1997 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Application.h"
#include "AppMem.h"
#include "Conservatory.h"
#include "CelestialEditGUI.h"
#include "Project.h"
#include "Types.h"
#include "Joe.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "Log.h"
#include "Raster.h"
#include "GraphData.h"
#include "Toolbar.h"
#include "requester.h"
#include "Database.h"
#include "Interactive.h"
#include "ViewGUI.h"
#include "Security.h"
#include "Lists.h"

CelestialEffect::CelestialEffect()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CELESTIAL;
SetDefaults();

} // CelestialEffect::CelestialEffect

/*===========================================================================*/

CelestialEffect::CelestialEffect(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_CELESTIAL;
SetDefaults();

} // CelestialEffect::CelestialEffect

/*===========================================================================*/

CelestialEffect::CelestialEffect(RasterAnimHost *RAHost, EffectsLib *Library, CelestialEffect *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_CELESTIAL;
Prev = Library->LastCelestial;
if (Library->LastCelestial)
	{
	Library->LastCelestial->Next = this;
	Library->LastCelestial = this;
	} // if
else
	{
	Library->Celestial = Library->LastCelestial = this;
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
	strcpy(NameBase, "Celestial");
	} // else
if (Library)
	SetUniqueName(Library, NameBase);

} // CelestialEffect::CelestialEffect

/*===========================================================================*/

CelestialEffect::~CelestialEffect()
{
EffectList *NextLight;

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->LPG && GlobalApp->GUIWins->LPG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->LPG;
		GlobalApp->GUIWins->LPG = NULL;
		} // if
	} // if

if (Img)
	{
	if (Img->GetRaster())
		{
		Img->GetRaster()->RemoveAttribute(Img);
		} // if
	// Img may be NULLed out by RemoveAttribute()
	if (Img)
		delete Img;
	Img = NULL;
	} // if

while (Lights)
	{
	NextLight = Lights;
	Lights = Lights->Next;
	delete NextLight;
	} // while

} // CelestialEffect::~CelestialEffect

/*===========================================================================*/

void CelestialEffect::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_CELESTIAL_NUMANIMPAR] = {EffectsLib::CelestialPresetDistance[10], 0.0, 0.0, 0.0,
															EffectsLib::CelestialPresetRadius[10], 1.0};
double RangeDefaults[WCS_EFFECTS_CELESTIAL_NUMANIMPAR][3] = {FLT_MAX, 0.0, 100000.0,
															1.0, 0.0, .01,
															90.0, -90.0, 1.0,
															FLT_MAX, -FLT_MAX, 1.0,
															FLT_MAX, 0.001, 1.0, 
															100.0, 0.0, .01};
long Ct;

for (Ct = 0; Ct < WCS_EFFECTS_CELESTIAL_NUMANIMPAR; Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
Color.SetDefaults(this, WCS_EFFECTS_CELESTIAL_NUMANIMPAR);
Img = NULL;
ShowPhase = 0;
Lights = NULL;

AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY].SetMultiplier(100.0);
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].SetMultiplier(100.0);

} // CelestialEffect::SetDefaults

/*===========================================================================*/

void CelestialEffect::Copy(CelestialEffect *CopyTo, CelestialEffect *CopyFrom)
{
long Result = -1;
Raster *NewRast;
EffectList *NextLight, **ToLight;
NotifyTag Changes[2];

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

while (CopyTo->Lights)
	{
	NextLight = CopyTo->Lights;
	CopyTo->Lights = CopyTo->Lights->Next;
	delete NextLight;
	} // if
NextLight = CopyFrom->Lights;
ToLight = &CopyTo->Lights;
while (NextLight)
	{
	if (NextLight->Me)
		{
		if (*ToLight = new EffectList())
			{
			if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
				{
				(*ToLight)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextLight->Me);
				} // if no need to make another copy, its all in the family
			else
				{
				if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(NextLight->Me->EffectType, NextLight->Me->Name))
					{
					Result = UserMessageCustom("Copy Celestial Object", "How do you wish to resolve Light name collisions?\n\nLink to existing Lights, replace existing Lights, or create new Lights?",
						"Link", "Create", "Overwrite", 1);
					} // if
				if (Result <= 0)
					{
					(*ToLight)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextLight->Me->EffectType, NULL, NextLight->Me);
					} // if create new
				else if (Result == 1)
					{
					(*ToLight)->Me = GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(NextLight->Me);
					} // if link to existing
				else if ((*ToLight)->Me = GlobalApp->CopyToEffectsLib->FindByName(NextLight->Me->EffectType, NextLight->Me->Name))
					{
					((Light *)(*ToLight)->Me)->Copy((Light *)(*ToLight)->Me, (Light *)NextLight->Me);
					Changes[0] = MAKE_ID((*ToLight)->Me->GetNotifyClass(), (*ToLight)->Me->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, (*ToLight)->Me);
					} // else if found and overwrite
				else
					{
					(*ToLight)->Me = GlobalApp->CopyToEffectsLib->AddEffect(NextLight->Me->EffectType, NULL, NextLight->Me);
					} // else
				} // else better copy or overwrite it since its important to get just the right ecosystem
			if ((*ToLight)->Me)
				ToLight = &(*ToLight)->Next;
			else
				{
				delete *ToLight;
				*ToLight = NULL;
				} // if
			} // if
		} // if
	NextLight = NextLight->Next;
	} // while

CopyTo->Color.Copy(&CopyTo->Color, &CopyFrom->Color);
CopyTo->ShowPhase = CopyFrom->ShowPhase;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // CelestialEffect::Copy

/*===========================================================================*/

char *CelestialEffect::OKRemoveRaster(void)
{

if (RAParent)
	return (RAParent->OKRemoveRaster());

return ("Image Object is used as a Celestial Object! Remove anyway?");

} // CelestialEffect::OKRemoveRaster

/*===========================================================================*/

void CelestialEffect::RemoveRaster(RasterShell *Shell)
{
NotifyTag Changes[2];

if (Img == Shell)
	Img = NULL;

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // CelestialEffect::RemoveRaster

/*===========================================================================*/

int CelestialEffect::RemoveRAHost(RasterAnimHost *RemoveMe)
{
EffectList *CurLight = Lights, *PrevLight = NULL;
NotifyTag Changes[2];

if (Img && Img->GetRaster() == (Raster *)RemoveMe)
	{
	Img->GetRaster()->RemoveAttribute(Img);
	return (1);
	} // if

while (CurLight)
	{
	if (CurLight->Me == (GeneralEffect *)RemoveMe)
		{
		if (PrevLight)
			PrevLight->Next = CurLight->Next;
		else
			Lights = CurLight->Next;

		delete CurLight;

		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

		return (1);
		} // if
	PrevLight = CurLight;
	CurLight = CurLight->Next;
	} // while

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // CelestialEffect::RemoveRAHost

/*===========================================================================*/

int CelestialEffect::SetRaster(Raster *NewRast)
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

} // CelestialEffect::SetRaster

/*===========================================================================*/

ULONG CelestialEffect::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
unsigned long ImageID;
char LightName[256];
EffectList **CurLight = &Lights;

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
					case WCS_EFFECTS_CELESTIAL_DISTANCE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_TRANSPARENCY:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_TRANSPARENCY].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_LATITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_LONGITUDE:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_RADIUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_SIZEFACTOR:
						{
						BytesRead = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_SIZEFACTOR].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_COLOR:
						{
						BytesRead = Color.Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_SHOWPHASE:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowPhase, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_CELESTIAL_SHOWHALO:
						{
						BytesRead = ReadBlock(ffile, (char *)&ShowHalo, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
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
					case WCS_EFFECTS_CELESTIAL_LIGHTNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)LightName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (LightName[0])
							{
							if (*CurLight = new EffectList())
								{
								if ((*CurLight)->Me = GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_LIGHT, LightName))
									CurLight = &(*CurLight)->Next;
								else
									{
									delete *CurLight;
									*CurLight = NULL;
									}
								} // if
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

} // CelestialEffect::Load

/*===========================================================================*/

unsigned long int CelestialEffect::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long ImageID;
unsigned long int AnimItemTag[WCS_EFFECTS_CELESTIAL_NUMANIMPAR] = {WCS_EFFECTS_CELESTIAL_DISTANCE,
																	WCS_EFFECTS_CELESTIAL_TRANSPARENCY,
																	WCS_EFFECTS_CELESTIAL_LATITUDE,
																	WCS_EFFECTS_CELESTIAL_LONGITUDE,
																	WCS_EFFECTS_CELESTIAL_RADIUS,
																	WCS_EFFECTS_CELESTIAL_SIZEFACTOR};
EffectList *CurLight;

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CELESTIAL_SHOWPHASE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShowPhase)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CELESTIAL_SHOWHALO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&ShowHalo)) == NULL)
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

ItemTag = WCS_EFFECTS_CELESTIAL_COLOR + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
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

CurLight = Lights;
while (CurLight)
	{
	if (CurLight->Me)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_CELESTIAL_LIGHTNAME, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(CurLight->Me->GetName()) + 1),
			WCS_BLOCKTYPE_CHAR, (char *)CurLight->Me->GetName())) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // if
	CurLight = CurLight->Next;
	} // while

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // CelestialEffect::Save

/*===========================================================================*/

void CelestialEffect::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->LPG)
	{
	delete GlobalApp->GUIWins->LPG;
	}
GlobalApp->GUIWins->LPG = new CelestialEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->LPG)
	{
	GlobalApp->GUIWins->LPG->Open(GlobalApp->MainProj);
	}

} // CelestialEffect::Edit

/*===========================================================================*/

char *CelestialEffectCritterNames[WCS_EFFECTS_CELESTIAL_NUMANIMPAR] = {"Distance (m)", "Transparency (%)",
										"Latitude (Deg)", "Longitude (Deg)",
										"Radius (m)", "Size Factor (%)"};

char *CelestialEffect::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (CelestialEffectCritterNames[Ct]);
	} // for
if (Test == &Color)
	return ("Color");

return ("");

} // CelestialEffect::GetCritterName

/*===========================================================================*/

int CelestialEffect::GetRAHostAnimated(void)
{

if (GeneralEffect::GetRAHostAnimated())
	return (1);
if (Color.GetRAHostAnimated())
	return (1);

return (0);

} // CelestialEffect::GetRAHostAnimated

/*===========================================================================*/

int CelestialEffect::SetToTime(double Time)
{
long Found = 0;

if (GeneralEffect::SetToTime(Time))
	Found = 1;
if (Color.SetToTime(Time))
	Found = 1;

return (Found);

} // CelestialEffect::SetToTime

/*===========================================================================*/

long CelestialEffect::GetKeyFrameRange(double &FirstKey, double &LastKey)
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
if (Color.GetKeyFrameRange(MinDist, MaxDist))
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

} // CelestialEffect::GetKeyFrameRange

/*===========================================================================*/

EffectList *CelestialEffect::AddLight(GeneralEffect *AddMe)
{
EffectList **CurLight = &Lights;
NotifyTag Changes[2];

if (AddMe)
	{
	while (*CurLight)
		{
		if ((*CurLight)->Me == AddMe)
			return (NULL);
		CurLight = &(*CurLight)->Next;
		} // while
	if (*CurLight = new EffectList())
		{
		(*CurLight)->Me = AddMe;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		} // if
	return (*CurLight);
	} // if

return (NULL);

} // CelestialEffect::AddLight

/*===========================================================================*/

long CelestialEffect::InitImageIDs(long &ImageID)
{
long NumImages = 0;
EffectList *CurLight = Lights;

if (Img && Img->GetRaster())
	{
	Img->GetRaster()->SetID(ImageID++);
	NumImages ++;
	} // if
while (CurLight)
	{
	if (CurLight->Me)
		NumImages += CurLight->Me->InitImageIDs(ImageID);
	CurLight = CurLight->Next;
	} // while
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // CelestialEffect::InitImageIDs

/*===========================================================================*/

int CelestialEffect::BuildFileComponentsList(EffectList **LightList)
{
EffectList **ListPtr, *CurLight = Lights;

while (CurLight)
	{
	if (CurLight->Me)
		{
		ListPtr = LightList;
		while (*ListPtr)
			{
			if ((*ListPtr)->Me == CurLight->Me)
				break;
			ListPtr = &(*ListPtr)->Next;
			} // if
		if (! (*ListPtr))
			{
			if (*ListPtr = new EffectList())
				(*ListPtr)->Me = CurLight->Me;
			else
				return (0);
			} // if
		} // if
	CurLight = CurLight->Next;
	} // while

return (1);

} // CelestialEffect::BuildFileComponentsList

/*===========================================================================*/

char CelestialEffect::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
if (DropType == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME
	|| DropType == WCS_RAHOST_OBJTYPE_RASTER
	|| DropType == WCS_EFFECTSSUBCLASS_LIGHT)
	return (1);

return (0);

} // CelestialEffect::GetRAHostDropOK

/*===========================================================================*/

int CelestialEffect::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (CelestialEffect *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (CelestialEffect *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_ANIMCOLORTIME)
	{
	Success = Color.ProcessRAHostDragDrop(DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_RAHOST_OBJTYPE_RASTER)
	{
	Success = SetRaster((Raster *)DropSource->DropSource);
	} // else if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_LIGHT)
	{
	Success = -1;
	sprintf(QueryStr, "Add %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		if (AddLight((GeneralEffect *)DropSource->DropSource))
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

} // CelestialEffect::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long CelestialEffect::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_CELESTIAL | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // CelestialEffect::GetRAFlags

/*===========================================================================*/

void CelestialEffect::GetInterFlags(RasterAnimHostProperties *Prop, RasterAnimHost *FlagMe)
{

if (! FlagMe)
	{
	Prop->InterFlags = 0;
	return;
	} // if

Prop->InterFlags = (WCS_RAHOST_INTERBIT_CLICKTOPOS | WCS_RAHOST_INTERBIT_MOVEX | 
					WCS_RAHOST_INTERBIT_MOVEY | WCS_RAHOST_INTERBIT_MOVEZ | 
					WCS_RAHOST_INTERBIT_MOVEELEV | WCS_RAHOST_INTERBIT_SCALEY);

} // CelestialEffect::GetInterFlags

/*===========================================================================*/

int CelestialEffect::GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil,
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
			return (1);
			} // if
		} // for
	if (ChildA == &Color)
		{
		AnimAffil = (AnimCritter *)ChildA;
		return (1);
		} // if
	} // if
else if (ChildB)
	{
	for (Ct = 0; Ct < GetNumTextures(); Ct ++)
		{
		if (ChildB == (RasterAnimHost **)GetTexRootPtrAddr(Ct))
			{
			TexAffil = (RootTexture **)ChildB;
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

} // CelestialEffect::GetAffiliates

/*===========================================================================*/

int CelestialEffect::ScaleMoveRotate(RasterAnimHost *MoveMe, DiagnosticData *Data, unsigned char Operation)
{
double NewVal, NewLat, NewLon;

if (! MoveMe)
	{
	return (0);
	} // if

// Camera group
if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOS)
	{
	if (GlobalApp->GUIWins->CVG->CollideCoord(Data->ViewSource, NewLat, NewLon, Data->PixelX, Data->PixelY, 
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].CurValue))
		{
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetCurValue(NewLat);
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetCurValue(NewLon);
		} // if
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETPOSNOQUERY)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_MOVEXYZ ||
	Operation == WCS_RAHOST_INTERACTIVEOP_MOVELATLONELEV)
	{
	Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE] = 1;
	Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION] = 1;
	Data->Value[WCS_DIAGNOSTIC_LATITUDE] = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].CurValue;
	Data->Value[WCS_DIAGNOSTIC_LONGITUDE] = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].CurValue;
	Data->Value[WCS_DIAGNOSTIC_ELEVATION] = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].CurValue;
	GlobalApp->GUIWins->CVG->ScaleMotion(Data);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LATITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_LONGITUDE]);
	if (Data->ValueValid[WCS_DIAGNOSTIC_ELEVATION])
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_DISTANCE].SetCurValue(Data->Value[WCS_DIAGNOSTIC_ELEVATION]);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SCALE)
	{
	// allow size factor
	NewVal = -Data->MoveY / 100.0;
	NewVal += 1.0;
	if (NewVal < .01)
		NewVal = .01;
	// let animcritter do clamping
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].SetCurValue(AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].CurValue * NewVal);
	return (1);
	} // if 
else if (Operation == WCS_RAHOST_INTERACTIVEOP_SETSIZE)
	{
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALX])
		{
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	if (Data->ValueValid[WCS_DIAGNOSTIC_NORMALY])
		{
		AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_RADIUS].SetCurValue(Data->Value[WCS_DIAGNOSTIC_NORMALX]);
		} // if
	return (1);
	} // if 

return (0);	// return 0 if nothing changed

} // CelestialEffect::ScaleMoveRotate

/*===========================================================================*/

RasterAnimHost *CelestialEffect::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;
EffectList *CurLight;

if (! Current)
	return (&Color);
if (Current == &Color)
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
if (Img && Img->GetRaster() == Current)
	Found = 1;
CurLight = Lights;
while (CurLight)
	{
	if (Found)
		return (CurLight->Me);
	if (Current == CurLight->Me)
		Found = 1;
	CurLight = CurLight->Next;
	} // while

return (NULL);

} // CelestialEffect::GetRAHostChild

/*===========================================================================*/

RasterAnimHost *CelestialEffect::GetNextGroupSibling(RasterAnimHost *FindMyBrother)
{

if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE))
	return (GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE));
if (FindMyBrother == GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE))
	return (GetAnimPtr(WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE));

return (NULL);

} // CelestialEffect::GetNextGroupSibling

/*===========================================================================*/

int CelestialEffect::GetDeletable(RasterAnimHost *Test)
{

return (0);

} // CelestialEffect::GetDeletable

/*===========================================================================*/

void CelestialEffect::SetFloating(char NewFloating)
{
DEMBounds CurBounds;
NotifyTag Changes[2];

if (GlobalApp->AppDB->FillDEMBounds(&CurBounds))
	{
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetValue((CurBounds.North + CurBounds.South) * 0.5);
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetValue((CurBounds.West + CurBounds.East) * 0.5);
	} // if
else
	{
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y));
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetValue(GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X));
	} // else
Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

} // CelestialEffect::SetFloating

/*===========================================================================*/

void CelestialEffect::ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds)
{
double ShiftWE, ShiftNS, TempVal;
GraphNode *CurNode;

ShiftWE = ((CurBounds->West + CurBounds->East) - (OldBounds->West - OldBounds->East)) * 0.5;
ShiftNS = ((CurBounds->North + CurBounds->South) - (OldBounds->North + OldBounds->South)) * 0.5;

AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].SetValue(
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].CurValue + ShiftNS);
if (CurNode = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].GetFirstNode(0))
	{
	TempVal = CurNode->GetValue() + ShiftNS;
	if (TempVal > 90.0)
		TempVal = 90.0;
	if (TempVal < -90.0)
		TempVal = -90.0;
	CurNode->SetValue(TempVal);
	while (CurNode = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LATITUDE].GetNextNode(0, CurNode))
		{
		TempVal = CurNode->GetValue() + ShiftNS;
		if (TempVal > 90.0)
			TempVal = 90.0;
		if (TempVal < -90.0)
			TempVal = -90.0;
		CurNode->SetValue(TempVal);
		} // while
	} // if
AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].SetValue(
	AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].CurValue + ShiftWE);
if (CurNode = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].GetFirstNode(0))
	{
	CurNode->SetValue(CurNode->GetValue() + ShiftWE);
	while (CurNode = AnimPar[WCS_EFFECTS_CELESTIAL_ANIMPAR_LONGITUDE].GetNextNode(0, CurNode))
		{
		CurNode->SetValue(CurNode->GetValue() + ShiftWE);
		} // while
	} // if

} // CelestialEffect::ScaleToDEMBounds

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int CelestialEffect::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
char ReadBuf[WCS_EFFECT_MAXNAMELENGTH];
ULONG Size, BytesRead = 1, TotalRead = 0;
int Success = -1, OldBoundsLoaded = 0;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
CelestialEffect *CurrentCelest = NULL;
Light *CurrentLight = NULL;
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
					else if (! strnicmp(ReadBuf, "Light", 8))
						{
						if (CurrentLight = new Light(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentLight->Load(ffile, Size, ByteFlip);
							}
						} // if light
					else if (! strnicmp(ReadBuf, "Celest", 8))
						{
						if (CurrentCelest = new CelestialEffect(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentCelest->Load(ffile, Size, ByteFlip)) == Size)
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

if (Success == 1 && CurrentCelest)
	{
	if (EffectsLib::LoadQueries && OldBoundsLoaded && GlobalApp->AppDB->FillDEMBounds(&CurBounds))
		{
		if (LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT) && UserMessageYN("Load Celestial Object", "Do you wish the loaded Celestial Object's attached Light\n positions to be scaled to current DEM bounds?"))
			{
			for (CurrentLight = (Light *)LoadToEffects->GetListPtr(WCS_EFFECTSSUBCLASS_LIGHT); CurrentLight; CurrentLight = (Light *)CurrentLight->Next)
				{
				CurrentLight->ScaleToDEMBounds(&OldBounds, &CurBounds);
				} // for
			} // if
		if (UserMessageYN("Load Celestial Object", "Do you wish the loaded Celestial Object's position\n to be scaled to current DEM bounds?"))
			{
			CurrentCelest->ScaleToDEMBounds(&OldBounds, &CurBounds);
			} // if
		} // if
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentCelest);
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

} // CelestialEffect::LoadObject

/*===========================================================================*/

int CelestialEffect::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
char StrBuf[12];
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, ImageID = 1;
EffectList *CurEffect, *LightList = NULL;
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

if (BuildFileComponentsList(&LightList))
	{
	CurEffect = LightList;
	while (CurEffect)
		{
		if (CurEffect->Me)
			{
			strcpy(StrBuf, "Light");
			if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
				WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((Light *)CurEffect->Me)->Save(ffile))
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
						} // if Light saved 
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

	while (LightList)
		{
		CurEffect = LightList;
		LightList = LightList->Next;
		delete CurEffect;
		} // while
	} // if

// CelestialEffect
strcpy(StrBuf, "Celest");
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
			} // if CelestialEffect saved 
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

} // CelestialEffect::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::CelestialEffect_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
CelestialEffect *Current;

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
						if (Current = new CelestialEffect(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (CelestialEffect *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::CelestialEffect_Load()

/*===========================================================================*/

ULONG EffectsLib::CelestialEffect_Save(FILE *ffile)
{
CelestialEffect *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = Celestial;
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
					} // if Celestial effect saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (CelestialEffect *)Current->Next;
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

} // EffectsLib::CelestialEffect_Save()

/*===========================================================================*/
