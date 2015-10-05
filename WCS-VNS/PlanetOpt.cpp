// PlanetOpt.cpp
// For managing WCS PlanetOpts
// Built from scratch on 03/24/99 by Gary R. Huber
// Copyright 1999 Questar Productions

#include "stdafx.h"
#include "EffectsLib.h"
#include "Joe.h"
#include "Application.h"
#include "Conservatory.h"
#include "EffectsIO.h"
#include "Useful.h"
#include "GraphData.h"
#include "requester.h"
#include "PlanetOptEditGUI.h"
#include "Database.h"
#include "Raster.h"
#include "Security.h"
#include "Lists.h"

/*===========================================================================*/

PlanetOpt::PlanetOpt()
: GeneralEffect(NULL)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_PLANETOPT;
SetDefaults();

} // PlanetOpt::PlanetOpt

/*===========================================================================*/

PlanetOpt::PlanetOpt(RasterAnimHost *RAHost)
: GeneralEffect(RAHost)
{

EffectType = WCS_JOE_ATTRIB_INTERNAL_PLANETOPT;
SetDefaults();

} // PlanetOpt::PlanetOpt

/*===========================================================================*/

PlanetOpt::PlanetOpt(RasterAnimHost *RAHost, EffectsLib *Library, PlanetOpt *Proto)
: GeneralEffect(RAHost)
{
char NameBase[WCS_EFFECT_MAXNAMELENGTH];

EffectType = WCS_JOE_ATTRIB_INTERNAL_PLANETOPT;
Prev = Library->LastPlanetOpt;
if (Library->LastPlanetOpt)
	{
	Library->LastPlanetOpt->Next = this;
	Library->LastPlanetOpt = this;
	} // if
else
	{
	Library->PlanetOpts = Library->LastPlanetOpt = this;
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
	strcpy(NameBase, "Planet Options");
	} // else
if (Library)
	{
	SetUniqueName(Library, NameBase);
	Coords = (CoordSys *)Library->GetDefaultEffect(WCS_EFFECTSSUBCLASS_COORDSYS, 0, NULL);
	} // if

} // PlanetOpt::PlanetOpt

/*===========================================================================*/

PlanetOpt::~PlanetOpt()
{

if (GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->POG && GlobalApp->GUIWins->POG->GetActive() == this)
		{
		delete GlobalApp->GUIWins->POG;
		GlobalApp->GUIWins->POG = NULL;
		} // if
	} // if

} // PlanetOpt::~PlanetOpt

/*===========================================================================*/

void PlanetOpt::SetDefaults(void)
{
double EffectDefault[WCS_EFFECTS_PLANETOPT_NUMANIMPAR] = {EffectsLib::CelestialPresetRadius[3], 0.0, 1.0, 0.0};
double RangeDefaults[WCS_EFFECTS_PLANETOPT_NUMANIMPAR][3] = {
												FLT_MAX, .001, 1.0,		// radius
												FLT_MAX, -FLT_MAX, 1.0,	// rotation
												100.0, -100.0, .1,		// vertical exaggeration
												FLT_MAX, -FLT_MAX, 10.0	// datum elev
												};
long Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	AnimPar[Ct].SetDefaults(this, (char)Ct, EffectDefault[Ct]);
	AnimPar[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	} // for
EcoExageration = 1;
Coords = NULL;

AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].SetMultiplier(100.0);

} // PlanetOpt::SetDefaults

/*===========================================================================*/

void PlanetOpt::Copy(PlanetOpt *CopyTo, PlanetOpt *CopyFrom)
{
long Result = -1;
#ifdef WCS_BUILD_VNS
NotifyTag Changes[2];
#endif // WCS_BUILD_VNS

CopyTo->Coords = NULL;
#ifdef WCS_BUILD_VNS
if (CopyFrom->Coords)
	{
	if (Result == 1 || GlobalApp->CopyFromEffectsLib == GlobalApp->CopyToEffectsLib)
		{
		CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
		} // if no need to make another copy, its all in the family
	else
		{
		if (Result < 0 && GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			Result = UserMessageCustom("Copy Planet Options", "How do you wish to resolve Coordinate System name collisions?\n\nLink to existing Coordinate Systems, replace existing Systems, or create new Systems?",
				"Link", "Create", "Overwrite", 1);
			} // if
		if (Result <= 0)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // if create new
		else if (Result == 1)
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->MatchNameMakeEffect(CopyFrom->Coords);
			} // if link to existing
		else if (CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->FindByName(CopyFrom->Coords->EffectType, CopyFrom->Coords->Name))
			{
			CopyTo->Coords->Copy(CopyTo->Coords, CopyFrom->Coords);
			Changes[0] = MAKE_ID(CopyTo->Coords->GetNotifyClass(), CopyTo->Coords->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, CopyTo->Coords);
			} // else if found and overwrite
		else
			{
			CopyTo->Coords = (CoordSys *)GlobalApp->CopyToEffectsLib->AddEffect(CopyFrom->Coords->EffectType, NULL, CopyFrom->Coords);
			} // else
		} // else better copy or overwrite it since its important to get just the right object
	} // if
if (GlobalApp->CopyToEffectsLib->GetDefaultEffect(CopyTo->EffectType, 0, NULL) == CopyTo)
	GlobalApp->CopyToEffectsLib->UpdateDefaultCoords(CopyTo->Coords, TRUE);
#endif // WCS_BUILD_VNS

CopyTo->EcoExageration = CopyFrom->EcoExageration;
GeneralEffect::Copy((GeneralEffect *)CopyTo, (GeneralEffect *)CopyFrom);

} // PlanetOpt::Copy

/*===========================================================================*/

ULONG PlanetOpt::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, RadiusRead = 0;
union MultiVal MV;
char CoordsName[256];

Coords = NULL;
// update default PO's
#ifdef WCS_BUILD_VNS
if (GlobalApp->AppEffects->GetDefaultEffect(EffectType, 0, NULL) == this)
	GlobalApp->AppEffects->UpdateDefaultCoords(Coords, FALSE);
#endif // WCS_BUILD_VNS

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
					case WCS_EFFECTS_PLANETOPT_ECOEXAGERATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&EcoExageration, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PLANETOPT_RADIUS:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].Load(ffile, Size, ByteFlip);
						RadiusRead = 1;
						break;
						}
					case WCS_EFFECTS_PLANETOPT_ROTATION:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_ROTATION].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PLANETOPT_VERTICALEXAG:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PLANETOPT_DATUM:
						{
						BytesRead = AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_PLANETOPT_COORDSYSNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)CoordsName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						#ifdef WCS_BUILD_VNS
						if (CoordsName[0])
							{
							Coords = (CoordSys *)GlobalApp->LoadToEffectsLib->FindByName(WCS_EFFECTSSUBCLASS_COORDSYS, CoordsName);
							} // if
						#endif // WCS_BUILD_VNS
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

#ifdef WCS_BUILD_VNS
if (RadiusRead && fabs(AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].CurValue - EffectsLib::CelestialPresetRadius[3]) > 1.0)
	{
	if (Coords = (CoordSys *)GlobalApp->LoadToEffectsLib->GetDefaultEffect(WCS_EFFECTSSUBCLASS_COORDSYS, 1, NULL))
		{
		Coords->Datum.Ellipse.GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR)->Copy(
			Coords->Datum.Ellipse.GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR), 
			GetAnimPtr(WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS));
		Coords->Datum.Ellipse.GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR)->Copy(
			Coords->Datum.Ellipse.GetAnimPtr(WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR), 
			GetAnimPtr(WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS));
		} // if
	} // if
#endif // WCS_BUILD_VNS

return (TotalRead);

} // PlanetOpt::Load

/*===========================================================================*/

unsigned long int PlanetOpt::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct, ParamCt;
#ifdef WCS_BUILD_VNS
unsigned long int AnimItemTag[WCS_EFFECTS_PLANETOPT_NUMANIMPAR - 1] = {WCS_EFFECTS_PLANETOPT_ROTATION,
																 WCS_EFFECTS_PLANETOPT_VERTICALEXAG,
																 WCS_EFFECTS_PLANETOPT_DATUM};
#else
unsigned long int AnimItemTag[WCS_EFFECTS_PLANETOPT_NUMANIMPAR] = {WCS_EFFECTS_PLANETOPT_RADIUS,
																 WCS_EFFECTS_PLANETOPT_ROTATION,
																 WCS_EFFECTS_PLANETOPT_VERTICALEXAG,
																 WCS_EFFECTS_PLANETOPT_DATUM};
#endif // WCS_BUILD_VNS

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
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PLANETOPT_ECOEXAGERATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EcoExageration)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = ParamCt = 0; ParamCt < GetNumAnimParams(); ParamCt ++)
	{
	#ifdef WCS_BUILD_VNS
	if (ParamCt == WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS)
		continue;
	#endif // WCS_BUILD_VNS
	ItemTag = AnimItemTag[Ct ++] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = AnimPar[ParamCt].Save(ffile))
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

#ifdef WCS_BUILD_VNS
if (Coords)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_PLANETOPT_COORDSYSNAME, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Coords->GetName()) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)Coords->GetName())) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
#endif // WCS_BUILD_VNS

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

} // PlanetOpt::Save

/*===========================================================================*/

void PlanetOpt::Edit(void)
{

DONGLE_INLINE_CHECK()
if(GlobalApp->GUIWins->POG)
	{
	delete GlobalApp->GUIWins->POG;
	}
GlobalApp->GUIWins->POG = new PlanetOptEditGUI(GlobalApp->AppEffects, this);
if(GlobalApp->GUIWins->POG)
	{
	GlobalApp->GUIWins->POG->Open(GlobalApp->MainProj);
	}

} // PlanetOpt::Edit

/*===========================================================================*/

short PlanetOpt::AnimateShadows(void)
{

if (GetAnimPtr(WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS)->GetNumNodes(0) > 1)
	return (1);

return (0);

} // PlanetOpt::AnimateShadows

/*===========================================================================*/

int PlanetOpt::SetCoords(CoordSys *NewCoords)
{
NotifyTag Changes[2];

Coords = NewCoords;

#ifdef WCS_BUILD_VNS
if ((GlobalApp->AppEffects->GetDefaultEffect(EffectType, 0, NULL) == this) || (NewCoords == NULL))
	GlobalApp->AppEffects->UpdateDefaultCoords(Coords, TRUE);
#endif // WCS_BUILD_VNS

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // PlanetOpt::SetCoords

/*===========================================================================*/

int PlanetOpt::SetExportCoords(CoordSys *NewCoords, Database *ExportDB, Project *ExportProj)
{
NotifyTag Changes[2];

Coords = NewCoords;

#ifdef WCS_BUILD_VNS
if ((GlobalApp->CopyToEffectsLib->GetDefaultEffect(EffectType, 0, NULL) == this) || (NewCoords == NULL))
	GlobalApp->CopyToEffectsLib->UpdateExportDefaultCoords(Coords, TRUE, ExportDB, ExportProj);
#endif // WCS_BUILD_VNS

Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

return (1);

} // PlanetOpt::SetExportCoords

/*===========================================================================*/

char *PlanetOptCritterNames[WCS_EFFECTS_PLANETOPT_NUMANIMPAR] = {"Planet Radius (m)", "Rotation (deg)",
	"Vertical Scale (%)", "Scaling Datum (m)"};

char *PlanetOpt::GetCritterName(RasterAnimHost *Test)
{
char Ct;

for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	if (Test == GetAnimPtr(Ct))
		return (PlanetOptCritterNames[Ct]);
	} // for
return ("");

} // PlanetOpt::GetCritterName

/*===========================================================================*/

int PlanetOpt::RemoveRAHost(RasterAnimHost *RemoveMe)
{
NotifyTag Changes[2];

if (Coords == (CoordSys *)RemoveMe)
	{
	Coords = NULL;

	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());

	return (1);
	} // if

return (GeneralEffect::RemoveRAHost(RemoveMe));

} // PlanetOpt::RemoveRAHost

/*===========================================================================*/

RasterAnimHost *PlanetOpt::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
char Ct, Found = 0;

if (! Current)
	Found = 1;
for (Ct = 0; Ct < GetNumAnimParams(); Ct ++)
	{
	#ifdef WCS_BUILD_VNS
	if (Ct == WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS)
		continue;
	#endif // WCS_BUILD_VNS
	if (Found)
		return (GetAnimPtr(Ct));
	if (Current == GetAnimPtr(Ct))
		Found = 1;
	} // for
#ifdef WCS_BUILD_VNS
if (Found && Coords)
	return (Coords);
#endif // WCS_BUILD_VNS

return (NULL);

} // PlanetOpt::GetRAHostChild

/*===========================================================================*/

int PlanetOpt::GetDeletable(RasterAnimHost *Test)
{

if (Test == Coords)
	return (1);

return (0);

} // PlanetOpt::GetDeletable

/*===========================================================================*/

long PlanetOpt::InitImageIDs(long &ImageID)
{
long NumImages = 0;

if (Coords)
	NumImages += Coords->InitImageIDs(ImageID);
NumImages += GeneralEffect::InitImageIDs(ImageID);

return (NumImages);

} // PlanetOpt::InitImageIDs

/*===========================================================================*/

int PlanetOpt::BuildFileComponentsList(EffectList **CoordSystems)
{
EffectList **ListPtr;

if (Coords)
	{
	ListPtr = CoordSystems;
	while (*ListPtr)
		{
		if ((*ListPtr)->Me == Coords)
			break;
		ListPtr = &(*ListPtr)->Next;
		} // if
	if (! (*ListPtr))
		{
		if (*ListPtr = new EffectList())
			(*ListPtr)->Me = Coords;
		else
			return (0);
		} // if
	} // if

return (1);

} // PlanetOpt::BuildFileComponentsList

/*===========================================================================*/

char PlanetOpt::GetRAHostDropOK(long DropType)
{

if (GeneralEffect::GetRAHostDropOK(DropType))
	return (1);
#ifdef WCS_BUILD_VNS
if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS)
	return (1);
#endif // WCS_BUILD_VNS

return (0);

} // PlanetOpt::GetRAHostDropOK

/*===========================================================================*/

int PlanetOpt::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
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
	if (this != (PlanetOpt *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (PlanetOpt *)DropSource->DropSource);
			strcpy(NameStr, Name);
			SetUniqueName(GlobalApp->AppEffects, NameStr);
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
#ifdef WCS_BUILD_VNS
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_COORDSYS)
	{
	Success = SetCoords((CoordSys *)DropSource->DropSource);
	} // else if
#endif // WCS_BUILD_VNS
else
	{
	Success = GeneralEffect::ProcessRAHostDragDrop(DropSource);
	} // else if

return (Success);

} // PlanetOpt::ProcessRAHostDragDrop

/*===========================================================================*/

unsigned long PlanetOpt::GetRAFlags(unsigned long Mask)
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

Mask &= (WCS_RAHOST_ICONTYPE_PLANET | WCS_RAHOST_FLAGBIT_CHILDREN | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (Mask);

} // PlanetOpt::GetRAFlags

/*===========================================================================*/

// return 0 if load failed due to file corruption or -1 if the correct object type was not found
int PlanetOpt::LoadObject(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
DEMBounds OldBounds, CurBounds;
PlanetOpt *CurrentPlanet = NULL;
EffectsLib *LoadToEffects = NULL;
ImageLib *LoadToImages = NULL;
#ifdef WCS_BUILD_VNS
CoordSys *CurrentCoords = NULL;
#endif // WCS_BUILD_VNS
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
					#ifdef WCS_BUILD_VNS
					else if (! strnicmp(ReadBuf, "CoordSys", 8))
						{
						if (CurrentCoords = new CoordSys(NULL, LoadToEffects, NULL))
							{
							BytesRead = CurrentCoords->Load(ffile, Size, ByteFlip);
							}
						} // if Camera
					#endif // WCS_BUILD_VNS
					else if (! strnicmp(ReadBuf, "PlnetOpt", 8))
						{
						if (CurrentPlanet = new PlanetOpt(NULL, LoadToEffects, NULL))
							{
							if ((BytesRead = CurrentPlanet->Load(ffile, Size, ByteFlip)) == Size)
								Success = 1;	// we got our man
							}
						} // if PlnetOpt
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

if (Success == 1 && CurrentPlanet)
	{
	GlobalApp->CopyFromEffectsLib = LoadToEffects;
	GlobalApp->CopyFromImageLib = LoadToImages;
	Copy(this, CurrentPlanet);
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

} // PlanetOpt::LoadObject

/*===========================================================================*/

int PlanetOpt::SaveObject(FILE *ffile, const char *SuppliedFileName)
{
DEMBounds CurBounds;
#ifdef WCS_BUILD_VNS
EffectList *CurEffect, *CoordsList = NULL;
#endif // WCS_BUILD_VNS
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

#ifdef WCS_BUILD_VNS
if (BuildFileComponentsList(&CoordsList))
	{
	CurEffect = CoordsList;
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

	while (CoordsList)
		{
		CurEffect = CoordsList;
		CoordsList = CoordsList->Next;
		delete CurEffect;
		} // while
	} // if
#endif // WCS_BUILD_VNS

// PlanetOpt
strcpy(StrBuf, "PlnetOpt");
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
			} // if PlanetOpt saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // PlanetOpt::SaveObject

/*===========================================================================*/
/*===========================================================================*/

ULONG EffectsLib::PlanetOpt_Load(FILE *ffile, ULONG ReadSize, short ByteFlip)
{
PlanetOpt *Current;
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
					case WCS_EFFECTSBASE_DATA:
						{
						if (Current = new PlanetOpt(NULL, this, NULL))
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip);
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
							if (Current = (PlanetOpt *)FindDuplicateByName(Current->EffectType, Current))
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

} // EffectsLib::PlanetOpt_Load()

/*===========================================================================*/

ULONG EffectsLib::PlanetOpt_Save(FILE *ffile)
{
PlanetOpt *Current;
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

Current = PlanetOpts;
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
					} // if PlanetOpt saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	Current = (PlanetOpt *)Current->Next;
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

} // EffectsLib::PlanetOpt_Save()

/*===========================================================================*/
