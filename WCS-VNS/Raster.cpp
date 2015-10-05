// Raster.cpp
// Storage and manipultion of bitmapped objects for World Construction Set v4
// Built from GeoRaster.cpp 4/13/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "AppMem.h"
#include "Application.h"
#include "Joe.h"
#include "Points.h"
#include "Raster.h"
#include "Useful.h"
#include "Project.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "Log.h"
#include "Requester.h"
#include "ImageInputFormat.h"
#include "Toolbar.h"
#include "Conservatory.h"
#include "ImageViewGUI.h"
#include "RenderControlGUI.h"
#include "Render.h"
#include "Interactive.h"
#include "ImageOutputEvent.h"
#include "PixelManager.h"
#include "PostProcessEvent.h"
#include "RLA.h"
#include "FontImage.h" // for creating text on Standin image

#if defined (WCS_BUILD_W6) || defined (WCS_BUILD_V2)
extern const char *FileExtension[];
#endif // WCS_BUILD_W6 or WCS_BUILD_V2

unsigned long RasterShellIDCounter;

// this struct and static array are used by the static function GetStandardBufferType() 

struct StdBufType
	{
	char *Name;
	char Type;
	}; // struct StdBufType

struct StdBufType StandardBufTypes[] = 
	{
	{"ZBUF", WCS_RASTER_BANDSET_FLOAT},
	{"RED", WCS_RASTER_BANDSET_BYTE},
	{"GREEN", WCS_RASTER_BANDSET_BYTE},
	{"BLUE", WCS_RASTER_BANDSET_BYTE},
	{"ANTIALIAS", WCS_RASTER_BANDSET_BYTE},
	{"FOLIAGE ZBUF", WCS_RASTER_BANDSET_FLOAT},
	{"FOLIAGE RED", WCS_RASTER_BANDSET_BYTE},
	{"FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE},
	{"FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE},
	{"FOLIAGE ANTIALIAS", WCS_RASTER_BANDSET_BYTE},
	{"LATITUDE", WCS_RASTER_BANDSET_FLOAT},
	{"LONGITUDE", WCS_RASTER_BANDSET_FLOAT},
	{"ELEVATION", WCS_RASTER_BANDSET_FLOAT},
	{"ILLUMINATION", WCS_RASTER_BANDSET_FLOAT},
	{"SLOPE", WCS_RASTER_BANDSET_FLOAT},
	{"ASPECT", WCS_RASTER_BANDSET_FLOAT},
	{"OBJECT", WCS_RASTER_BANDSET_FLOAT},
	{"REFLECTION", WCS_RASTER_BANDSET_FLOAT},
	{"SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT},
	{"SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT},
	{"SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT RED", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT GREEN", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT BLUE", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT FOLIAGE RED", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT FOLIAGE GREEN", WCS_RASTER_BANDSET_FLOAT},
	{"FLOAT FOLIAGE BLUE", WCS_RASTER_BANDSET_FLOAT},
	{"RGB EXPONENT", WCS_RASTER_BANDSET_SHORT},
	{"", -1}
	}; // StandardBufTypes


// ImageLib

ImageLib::ImageLib()
{

List = NULL;
ActiveRaster = NULL;
MaxTileMemory = 100;
StandInRast = NULL;
HighestID = 0;

} // ImageLib::ImageLib

/*===========================================================================*/

ImageLib::~ImageLib()
{
delete StandInRast; StandInRast = NULL; // safe even if StandInRast is already NULL
DeleteAll();

} // ImageLib::~ImageLib

/*===========================================================================*/

void ImageLib::SetActive(Raster *NewActive)
{

ActiveRaster = NewActive;
SetParam(1, WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ACTIVECHANGED, 0, 0);

} // ImageLib::SetActive

/*===========================================================================*/

Raster *ImageLib::FindByName(char *FindPath, char *FindName)
{
Raster *Current;

Current = List;
while (Current)
	{
	if (Current->MatchPathAndName(FindPath, FindName))
		{
		return (Current);
		} // if
	Current = Current->Next;
	} // while

return (NULL);

} // ImageLib::FindByName

/*===========================================================================*/

Raster *ImageLib::FindByUserName(char *FindName)
{
Raster *Current;

Current = List;
while (Current)
	{
	if (! strcmp(Current->GetUserName(), FindName))
		{
		return (Current);
		} // if
	Current = Current->Next;
	} // while

return (NULL);

} // ImageLib::FindByName

/*===========================================================================*/

Raster *ImageLib::FindByID(unsigned long FindID)
{
Raster *Current;

Current = List;
while (Current)
	{
	if (Current->GetID() == FindID)
		{
		return (Current);
		} // if
	Current = Current->Next;
	} // while

return (NULL);

} // ImageLib::FindByID

/*===========================================================================*/

RasterAttribute *ImageLib::MatchRasterSetShell(unsigned long MatchID, RasterShell *ShellSource, RasterAnimHost *HostSource)
{
Raster *Found;

if (ShellSource && HostSource)
	{
	if (Found = FindByID(MatchID))
		{
		return (Found->AddAttribute(ShellSource->GetType(), ShellSource, HostSource));
		} // if
	} // if

return (NULL);

} // ImageLib::MatchRasterSetShell

/*===========================================================================*/

RasterAttribute *ImageLib::MatchRasterAttributeSetHost(unsigned long MatchID, char AttributeType, RasterAnimHost *HostSource)
{
Raster *Found;
RasterAttribute *MyAttr;

if (HostSource)
	{
	if (Found = FindByID(MatchID))
		{
		if ((MyAttr = Found->MatchAttribute(AttributeType)) && MyAttr->GetShell())
			{
			MyAttr->GetShell()->SetHostNotify(HostSource);
			return (MyAttr);
			} // if
		} // if
	} // if

return (NULL);

} // ImageLib::MatchRasterAttributeSetHost

/*===========================================================================*/

Raster *ImageLib::MatchAttribute(char MatchType)
{
Raster *CurRast = List;

while (CurRast)
	{
	if (CurRast->MatchAttribute(MatchType))
		return (CurRast);
	CurRast = CurRast->Next;
	} // while

return (NULL);

} // ImageLib::MatchAttribute

/*===========================================================================*/

void ImageLib::DeleteAll(void)
{
Raster *NextRast;
NotifyTag Changes[2];

while (List)
	{
	NextRast = List->Next;
	delete List;
	List = NextRast;
	} // while

if (this == GlobalApp->AppImages)
	{
	Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, NULL);
	} // if
	
} // ImageLib::DeleteAll

/*===========================================================================*/

void ImageLib::RemoveAll(EffectsLib *CheckLib, int RemoveUnusedOnly)
{
Raster *TempRast = List, *NextRast;

while (TempRast)
	{
	NextRast = TempRast->Next;
	if (! RemoveUnusedOnly || ! TempRast->CheckImageInUse())
		{
		if (! RemoveRaster(TempRast, CheckLib))		// sends notification
			{
			if (! UserMessageOKCAN("Remove Image Objects", "Continue removing remaining Image Objects?"))
				return;
			} // if remove failed
		} // if
	TempRast = NextRast;
	} // while

} // ImageLib::RemoveAll

/*===========================================================================*/

void ImageLib::FreeBands(char FreeLongevity)
{
Raster *Current;

Current = List;
while (Current)
	{
	Current->FreeAllBands(FreeLongevity);
	Current = Current->Next;
	} // while

} // ImageLib::FreeBands

/*===========================================================================*/

Raster *ImageLib::AddRaster(void)
{
Raster *MyRast;

if (MyRast = List)
	{
	while (MyRast->Next)
		{
		MyRast = MyRast->Next;
		} // while
	return (MyRast->Next = new Raster);
	} // if
else
	{
	return (List = new Raster);
	} // else

} // ImageLib::AddRaster

/*===========================================================================*/

Raster *ImageLib::AddRequestRaster(char UseSimpleRequester, Raster ***ManyRasters, long *NumMany)
{
char NewPath[1024], NewName[256];
Raster *NewRast = NULL;
NotifyTag Changes[2];
FileReq FR;
long k;
char filename[1024];
int NumFiles = 0;

strcpy(NewPath, GlobalApp->MainProj->imagepath);
NewName[0] = 0;

#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
char URL[1024];
URL[0] = 0;

if (UseSimpleRequester)
	{
	if (GetInputString("Enter image URL:", "", URL) && URL[0])
		{
		NumFiles = 1;
		} // if
	else
		{
		return(NULL);
		} // else
	} // if


if (!URL[0])
#endif // WCS_BUILD_REMOTEFILE_SUPPORT
	{
	FR.SetDefPath(NewPath);
	if (FR.Request(WCS_REQUESTER_FILE_MULTI))
		{
		NumFiles = (int)FR.FilesSelected();
		} // 
	else
		{
		return(NULL);
		} // else
	} // if

if (NumFiles != 0)
	{
	if (ManyRasters && NumFiles > 1)
		{
		if (! (*ManyRasters = (Raster **)AppMem_Alloc(NumFiles * sizeof (Raster *), APPMEM_CLEAR)))
			{
			ManyRasters = NULL;
			} // if
		else
			*NumMany = NumFiles;
		} // if
	else
		ManyRasters = NULL;
	for (k = 0; k < NumFiles; k ++)
		{
#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
		if (URL[0])
			{
			strcpy(filename, URL);
			} // if
		else
#endif // WCS_BUILD_REMOTEFILE_SUPPORT
			{
			if (k == 0)
				{
				strcpy(filename, FR.GetFirstName());
				} // if
			else
				{
				strcpy(filename, FR.GetNextName());
				} // else
			} // else
		BreakFileName((char *)filename, NewPath, 1024, NewName, 256);
		strcpy(GlobalApp->MainProj->imagepath, NewPath);
		// fails if item exists already or loading the file fails for any reason
	#ifdef WCS_IMAGE_MANAGEMENT
		if ((NewRast = AddRaster(NewPath, NewName, 2, FALSE, TRUE, TRUE)) && NewRast != (Raster *)WCS_IMAGE_ERR)
	#else // !WCS_IMAGE_MANAGEMENT
		if ((NewRast = AddRaster(NewPath, NewName, 2, FALSE, TRUE, FALSE)) && NewRast != (Raster *)WCS_IMAGE_ERR)
	#endif // !WCS_IMAGE_MANAGEMENT
			{
			Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			if (ManyRasters)
				(*ManyRasters)[k] = NewRast;
			} // if
		else if (NewRast == (Raster *)WCS_IMAGE_ERR)
			{
			if (UserMessageYN(NewName, "This image already exists in the Library!\nDo you wish to add another copy?"))
				{
	#ifdef WCS_IMAGE_MANAGEMENT
				if ((NewRast = AddRaster(NewPath, NewName, FALSE, FALSE, TRUE, TRUE)) && NewRast != (Raster *)WCS_IMAGE_ERR)
	#else // !WCS_IMAGE_MANAGEMENT
				if ((NewRast = AddRaster(NewPath, NewName, FALSE, FALSE, TRUE, FALSE)) && NewRast != (Raster *)WCS_IMAGE_ERR)
	#endif // !WCS_IMAGE_MANAGEMENT
					{
					Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					if (ManyRasters)
						(*ManyRasters)[k] = NewRast;
					} // if
				} // if
			else if (ManyRasters)
				{
				if (NewRast = FindByName(NewPath, NewName))
					(*ManyRasters)[k] = NewRast;
				} // else if
			else
				NewRast = NULL;
			} // else
		else
			{
			UserMessageOK(NewName, "Image could not be loaded. Possible reasons include invalid file path, \
	file permission denied, improper file format, invalid or incomplete file, out of memory, etc.");
			} // else
		} // for
	} // if

return (NewRast);

} // ImageLib::AddRequestRaster

/*===========================================================================*/

Raster *ImageLib::AddRaster(char *AddPath, char *AddName, short NonRedundant, short ConfirmFile, short LoadnPrep, char AllowImageManagement)
{
Raster *MyRast, *NewRast = NULL;
short Retry = 1;
char filename[256];
PathAndFile TestPAF;

if (TestPAF.SetPath(AddPath) && TestPAF.SetName(AddName))
	{
	if (ConfirmFile)
		{
		while (Retry)
			{
			Retry = 0;
			if (! TestPAF.GetValidPathAndName(filename))
				{
				if (NewRast = new Raster())
					{
					NewRast->PAF.SetPath((char *)TestPAF.GetPath());
					NewRast->PAF.SetName((char *)TestPAF.GetName());
					if (NewRast->AttemptValidatePathAndName())
						{
						TestPAF.SetPath((char *)NewRast->PAF.GetPath());
						TestPAF.SetName((char *)NewRast->PAF.GetName());
						delete NewRast;
						NewRast = NULL;
						} // if
					else
						{
						delete NewRast;
						NewRast = NULL;
						return (NULL);
						} // else
					} // if
				else
					return (NULL);
				} // if
			} // while
		} // if

	if (NonRedundant)
		{
		MyRast = List;
		while (MyRast)
			{
			if (MyRast->PAF.MatchPathAndName((char *)TestPAF.GetPath(), (char *)TestPAF.GetName()))
				{
				return (NonRedundant > 1 ? (Raster *)WCS_IMAGE_ERR: MyRast);
				} // if match found
			MyRast = MyRast->Next;
			} // while
		} // if

	if (NewRast = new Raster)
		{
		NewRast->PAF.Copy(&NewRast->PAF, &TestPAF);
		// try loading the image
		if (! NewRast->LoadnPrepImage(! LoadnPrep, LoadnPrep, AllowImageManagement))
			{
			delete NewRast;
			NewRast = NULL;
			} // if bad news
		} // if
	if (NewRast)
		{
		if (MyRast = List)
			{
			while (MyRast->Next)
				{
				MyRast = MyRast->Next;
				} // while
			MyRast->Next = NewRast;
			} // if
		else
			{
			List = NewRast;
			} // else
		} // if
	} // if

return (NewRast);

} // ImageLib::AddRaster

/*===========================================================================*/

long ImageLib::AddRasterReturnID(Raster *AddRast)
{
Raster *MyRast;

if (MyRast = List)
	{
	while (MyRast->Next)
		{
		MyRast = MyRast->Next;
		} // while
	MyRast->Next = AddRast;
	} // if
else
	{
	List = AddRast;
	} // else

HighestID ++;
AddRast->ID = HighestID;
return (HighestID);

} // ImageLib::AddRasterReturnID

/*===========================================================================*/

int ImageLib::RemoveRaster(Raster *RemoveMe, EffectsLib *CheckLib)
{
Raster *MyRast, *TestRast, *PrevRast = NULL;
NotifyTag Changes[2];

if (MyRast = List)
	{
	while (MyRast && MyRast != RemoveMe)
		{
		PrevRast = MyRast;
		MyRast = MyRast->Next;
		} // while
	if (MyRast)
		{
		if (MyRast->CheckOKRemove())
			{
			TestRast = List;
			while (TestRast)
				{
				if (TestRast != MyRast)
					{
					if (! TestRast->CheckOKRemove(MyRast))
						return (0);
					} // if
				TestRast = TestRast->Next;
				} // while
			TestRast = List;
			while (TestRast)
				{
				if (TestRast != MyRast)
					{
					if (! TestRast->RemoveRaster(MyRast))
						return (0);
					} // if
				TestRast = TestRast->Next;
				} // while
			Changes[0] = MAKE_ID(RemoveMe->GetNotifyClass(), RemoveMe->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			if (CheckLib->RemoveRAHost(MyRast, 1))	// added 9/16/02 - this finds all instances of attachment in the effects library
				{
				if (PrevRast)
					PrevRast->Next = MyRast->Next;
				else
					List = MyRast->Next;
				delete MyRast;
				} // if
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			return (1);
			} // if
		} // if
	} // if

return (0);

} // ImageLib::RemoveRaster

/*===========================================================================*/

int ImageLib::RemoveRAHost(RasterAnimHost *RemoveMe, EffectsLib *CheckLib)
{

return (RemoveRaster((Raster *)RemoveMe, CheckLib));

} // ImageLib::RemoveRAHost

/*===========================================================================*/

void ImageLib::InitRasterIDs(void)
{
Raster *Current;
unsigned long RasterIDCounter = 0;

Current = List;
while (Current)
	{
	Current->SetID(++RasterIDCounter);
	Current = Current->Next;
	} // while

HighestID = RasterIDCounter;

} // ImageLib::InitRasterIDs

/*===========================================================================*/

void ImageLib::ClearRasterIDs(void)
{
Raster *Current;

Current = List;
while (Current)
	{
	Current->SetID(0);
	Current = Current->Next;
	} // while
HighestID = 0;

} // ImageLib::ClearRasterIDs

/*===========================================================================*/

void ImageLib::SortImagesByAlphabet(void)
{
long NumImages, ImgCt, FirstFound, LowFound, FirstCheck;
Raster **ImagePointers, *CurRast;
NotifyTag Changes[2];

// count images
if ((NumImages = GetImageCount()) > 1)
	{
	// alloc an array
	if (ImagePointers = (Raster **)AppMem_Alloc(NumImages * sizeof (Raster *), APPMEM_CLEAR))
		{
		// copy pointers to array
		for (CurRast = List, ImgCt = 0; ImgCt < NumImages && CurRast; ImgCt ++, CurRast = CurRast->Next)
			{
			ImagePointers[ImgCt] = CurRast;
			} // while

		// sort array
		for (FirstFound = 0; FirstFound < NumImages - 1; FirstFound ++)
			{
			LowFound = FirstFound;
			for (FirstCheck = FirstFound + 1; FirstCheck < NumImages; FirstCheck ++)
				{
				if (stricmp(ImagePointers[LowFound]->Name, ImagePointers[FirstCheck]->Name) > 0)
					{
					LowFound = FirstCheck;
					} // if
				} // for
			if (LowFound > FirstFound)
				swmem(&ImagePointers[LowFound], &ImagePointers[FirstFound], sizeof (Raster *));
			} // for

		// allocate Next pointers
		List = ImagePointers[0];
		for (ImgCt = 0; ImgCt < NumImages - 1; ImgCt ++)
			{
			ImagePointers[ImgCt]->Next = ImagePointers[ImgCt + 1];
			} // for
		ImagePointers[NumImages - 1]->Next = NULL;

		AppMem_Free(ImagePointers, NumImages * sizeof (Raster *));

		// send notify
		Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	} // if

} // ImageLib::SortImagesByAlphabet

/*===========================================================================*/

void ImageLib::MungPaths(Project *ProjHost)
{
Raster *Current;

Current = List;
while (Current)
	{
	Current->PAF.MungPath(ProjHost);
	Current = Current->Next;
	} // while

} // ImageLib::MungPaths

/*===========================================================================*/

void ImageLib::UnMungPaths(Project *ProjHost)
{
Raster *Current;

Current = List;
while (Current)
	{
	Current->PAF.UnMungPath(ProjHost);
	Current = Current->Next;
	} // while

} // ImageLib::UnMungPaths

/*===========================================================================*/

void ImageLib::LinkDissolvesToRasters(void)
{
Raster *Current;
RasterAttribute *MyAttr;
DissolveShell *MyShell;

Current = List;
while (Current)
	{
	if (MyAttr = Current->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
		{
		for (MyShell = (DissolveShell *)MyAttr->GetShell(); MyShell; MyShell = (DissolveShell *)MyShell->NextDis)
			{
			MyShell->SetTarget(FindByID(MyShell->GetTargetID()));
			} // for
		} // if
	Current = Current->Next;
	} // while

} // ImageLib::LinkDissolvesToRasters

/*===========================================================================*/

Raster *ImageLib::CheckDissolveRecursion(void)
{
Raster *Current, *Found = NULL;
RasterAttribute *MyAttr;

Current = List;
while (Current)
	{
	if (MyAttr = Current->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
		{
		if (Found = MyAttr->TraceDissolveRasterMatch(Current, 0))
			break;
		} // if
	Current = Current->Next;
	} // while

return (Found);

} // ImageLib::CheckDissolveRecursion

/*===========================================================================*/

Raster *ImageLib::GetNextRast(Raster *Me)
{

if (Me)
	return (Me->Next);
return (NULL);

} // ImageLib::GetNextRast

/*===========================================================================*/

void ImageLib::ApplicationSetTime(double Time, long Frame, double FrameFraction)
{
Raster *Current;

for (Current = List; Current; Current = Current->Next)
	{
	if ((Current->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) && Current->SequenceEnabled) || (Current->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE) && Current->DissolveEnabled))
		{
		Current->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
		} // if
	Current->SetToTime(Time);
	} // for

} // ImageLib::ApplicationSetTime

/*===========================================================================*/

long ImageLib::GetImageCount(void)
{
Raster *Current;
long ImageCt;

for (Current = List, ImageCt = 0; Current; Current = Current->Next, ImageCt ++);	//lint !e722

return (ImageCt);

} // ImageLib::GetImageCount

/*===========================================================================*/

int ImageLib::InitFoliageRasters(double Time, double FrameRate, int EcosystemsEnabled, int FoliageEffectsEnabled,
	int StreamEffectsEnabled, int LakeEffectsEnabled, Renderer *Rend)
{
int Success = 1;
long ImageCt;
Raster *Current;
RasterAttribute *MyAttr;
FoliageShell *MyShell;
RasterAnimHost *Host;
BusyWin *BWDE = NULL;
RasterAnimHostProperties Prop;

if (Rend->IsCamView)
	{
	BWDE = new BusyWin("Loading Foliage", GetImageCount(), 'BWDE', 0);
	} // if
else
	{
	Rend->Master->ProcInit(GetImageCount(), "Loading Foliage");
	} // else
for (Current = List, ImageCt = 0; Success && Current; Current = Current->Next, ImageCt ++)
	{
	if (Current->GetEnabled())
		{
		MyAttr = NULL;
		while (Success && (MyAttr = Current->MatchNextAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE, MyAttr)))
			{
			if ((MyShell = (FoliageShell *)MyAttr->GetShell()) && (Host = MyShell->GetHost()) && MyShell->GetHostEnabled())
				{
				if (Host = Host->GetRAHostRoot())
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
					Host->GetRAHostProperties(&Prop);

					if ((Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM && EcosystemsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE && FoliageEffectsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE && LakeEffectsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM && StreamEffectsEnabled))
						{
						if (! Current->LoadToRender(Time, FrameRate, WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED))
							{
							if (! UserMessageOKCAN(Current->GetUserName(), "Unable to load this Image Object!\nContinue without it?"))
								Success = 0;
							else
								Current->SetEnabled(0);		// so it doesn't try again on each frame
							} // if failed
						break;	// we found at least one foliage enabled so let's get out
						} // if
					} // if
				} // if foliage enabled
			} // else foliage attribute exists
		} // if raster enabled
	if (Rend->IsCamView)	// gotta have this distinction
		{
		if (BWDE)
			{
			if (BWDE->Update(ImageCt + 1))	// don't need this for abort checking
				{
				Success = 0;
				break;
				} // if
			} // if
		} // if
	else
		{
		Rend->Master->ProcUpdate(ImageCt + 1);	// calls Master->CheckAbort() - do M->CheckAbort() any time - it returns the Run variable
		if (! Rend->Master->IsRunning())
			{
			Success = 0;
			break;
			} // if
		} // else
	} // for

if (BWDE)
	delete BWDE;

return (Success);

} // ImageLib::InitFoliageRasters

/*===========================================================================*/

int ImageLib::InitFoliageRastersOneAtATime(double Time, double FrameRate, int EcosystemsEnabled, int FoliageEffectsEnabled,
	int StreamEffectsEnabled, int LakeEffectsEnabled, int LabelsEnabled, Raster *&Current, AnimColorTime *&ReplaceRGB, int &Shade3D)
{
int Success = 1, Found = 0;
RasterAttribute *MyAttr;
FoliageShell *MyShell;
RasterAnimHost *Host, *BigHost;
RasterAnimHostProperties Prop;

ReplaceRGB = NULL;

Current = (Current ? Current->Next: List);
while (Success && Current)
	{
	if (Current->GetEnabled())
		{
		MyAttr = NULL;
		while (Success && (MyAttr = Current->MatchNextAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE, MyAttr)))
			{
			if ((MyShell = (FoliageShell *)MyAttr->GetShell()) && (Host = MyShell->GetHost()) && MyShell->GetHostEnabled())
				{
				if (BigHost = Host->GetRAHostRoot())
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
					BigHost->GetRAHostProperties(&Prop);

					if ((Prop.TypeNumber == WCS_EFFECTSSUBCLASS_ECOSYSTEM && EcosystemsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_FOLIAGE && FoliageEffectsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LAKE && LakeEffectsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LABEL && LabelsEnabled)
						|| (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_STREAM && StreamEffectsEnabled))
						{
						if (! Current->LoadToRender(Time, FrameRate, WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED))
							{
							if (! UserMessageOKCAN(Current->GetUserName(), "Unable to load this Image Object!\nContinue without it?"))
								Success = 0;
							} // if failed
						else	// get replacement color
							{
							if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_LABEL)
								{
								Shade3D = ((Label *)Host)->GetShade3D();
								} // if
							else
								{
								if (! Current->GetIsColor())
									ReplaceRGB = ((Foliage *)Host)->GetReplacementColor();
								Shade3D = ((Foliage *)Host)->GetShade3D();
								} // else
							Found = 1;
							} // else
						break;	// we found at least one foliage enabled so let's get out
						} // if
					} // if
				} // if foliage enabled
			} // while
		} // if raster enabled
	if (Found)
		break;
	Current = Current->Next;
	} // while

return (Success);

} // ImageLib::InitFoliageRastersOneAtATime

/*===========================================================================*/

void ImageLib::ClearImageRasters(char FreeLongevity)
{
Raster *Current;
char FreeLong;

for (Current = List; Current; Current = Current->Next)
	{
	if ((Current->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) && Current->SequenceEnabled) || (Current->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE) && Current->DissolveEnabled))
		{
		FreeLong = WCS_RASTER_LONGEVITY_FORCEFREE;
		} // if
	else
		{
		FreeLong = FreeLongevity;
		} // else
	Current->FreeAllBands(FreeLong);
	} // for

} // ImageLib::ClearImageRasters

/*===========================================================================*/

void ImageLib::SetAllMemoryLongevity(char NewLongevity)
{
Raster *Current;

for (Current = List; Current; Current = Current->Next)
	{
	Current->SetLongevity(NewLongevity);
	if (NewLongevity <= WCS_RASTER_LONGEVITY_MEDIUM)
		Current->FreeAllBands(WCS_RASTER_LONGEVITY_MEDIUM);
	} // for

} // ImageLib::SetAllMemoryLongevity

/*===========================================================================*/

void ImageLib::SetAllLoadFast(char NewLoadFast)
{
Raster *Current;

for (Current = List; Current; Current = Current->Next)
	{
	Current->SetLoadFast(NewLoadFast);
	} // for

} // ImageLib::SetAllMemoryLongevity

/*===========================================================================*/

Raster *ImageLib::MatchShell(RasterShell *FindMe, char MatchType)
{
Raster *Test;

for (Test = List; Test; Test = Test->Next)
	{
	if (Test->MatchRasterShell(FindMe))
		{
		if (FindMe->GetType() == MatchType)
			return (Test);
		break;
		} // if
	} // for

return (NULL);

} // ImageLib::MatchShell

/*===========================================================================*/

Raster *ImageLib::MatchRasterShell(RasterShell *FindMe, int SearchDeep)
{
Raster *Test;

for (Test = List; Test; Test = Test->Next)
	{
	if (Test->MatchRasterShell(FindMe, SearchDeep))
		{
		return (Test);
		} // if
	} // for

return (NULL);

} // ImageLib::MatchRasterShell

/*===========================================================================*/

int ImageLib::MatchRaster(Raster *FindMe)
{
Raster *Test;

for (Test = List; Test; Test = Test->Next)
	{
	if (Test == FindMe)
		{
		return (1);
		} // if
	} // for

return (0);

} // ImageLib::MatchRaster

/*===========================================================================*/

Raster *ImageLib::MatchNameMakeRaster(Raster *MatchMe)
{
Raster *Test;
RasterAttribute *MyAttr;
DissolveShell *MyShell;

if (MatchMe)
	{
	for (Test = List; Test; Test = Test->Next)
		{
		if (! strcmp(MatchMe->GetUserName(), Test->GetUserName()))
			{
			return (Test);
			} // if
		} // for

	if (Test = AddRaster())
		{
		if ((MyAttr = MatchMe->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)) && (MyShell = (DissolveShell *)MyAttr->GetShell()))
			{
			do
				{
				MyShell->SetTarget(MatchNameMakeRaster(MyShell->GetTarget()));
				} while (MyShell = (DissolveShell *)MyShell->NextDis);
			} // if dissolve shell
		Test->Copy(Test, MatchMe, 0);	// copy only internal attributes
		return (Test);
		} // if
	} // if

return (NULL);

} // ImageLib::MatchNameMakeRaster

/*===========================================================================*/

ULONG ImageLib::Load(FILE *ffile, ULONG ReadSize, struct ChunkIODetail *Detail)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, ByteOrder;
char Version, Revision;
union MultiVal MV;
short ByteFlip;
static char StructBuf[24];  // Big enuf to hold anything we fread...
Raster *Current;

if (! ffile)
	{
	return (0);
	} // if need to open file

// header data is 6 bytes
if ((fread((char *)StructBuf, 6, 1, ffile)) != 1)
	return (0);

Version = StructBuf[0];
Revision = StructBuf[1];
memcpy(&ByteOrder, &StructBuf[2], 4);

// Endian flop if necessary

if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

TotalRead = 6;
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
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
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
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_IMAGES_MAXTILEMEM:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxTileMemory, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_IMAGES_RASTER:
						{
						if (Current = AddRaster())
							{
							BytesRead = Current->Load(ffile, Size, ByteFlip, WCS_RASTER_FILENORMAL, 1);	// 1 = validate path
							Current->TemplateItem = GlobalApp->TemplateLoadInProgress;
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

LinkDissolvesToRasters();

return (TotalRead);

} // ImageLib::Load

/*===========================================================================*/

ULONG ImageLib::Save(FILE *ffile, struct ChunkIODetail *Detail, int IDsInitialized)
{
char Version = WCS_IMAGES_VERSION, Revision = WCS_IMAGES_REVISION, str[32];
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;
Raster *Current;

RasterShellIDCounter = 1;
if (! IDsInitialized)
	InitRasterIDs();

if (ffile)
	{
	// no tags or sizes for first three items: version, revision & byte order
	if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteBlock(ffile, WCS_IMAGES_MAXTILEMEM, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, (char *)&MaxTileMemory)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	Current = List;
	while (Current)
		{
		if (Current->ID > 0 && (! Current->TemplateItem || GlobalApp->ComponentSaveInProgress))
			{
			ItemTag = WCS_IMAGES_RASTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = Current->Save(ffile, WCS_RASTER_FILENORMAL))
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
						} // if raster saved 
					else
						goto WriteError;
					} // if size written 
				else
					goto WriteError;
				} // if tag written 
			else
				goto WriteError;
			} // if
		Current = Current->Next;
		} // while

	ItemTag = WCS_PARAM_DONE;
	if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if This 

return (TotalWritten);

WriteError:

sprintf(str, "Images, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, str);

return (0L);

} // ImageLib::Save

/*===========================================================================*/

void ImageLib::SetParam(int Notify, ...)
{
va_list VarA;
ULONG NotifyChanges[WCS_NOTIFY_DELAY_MAX_CHANGE + 1];
unsigned int Change = 0, ParClass, SubClass, Item, Component;

va_start(VarA, Notify);

ParClass = va_arg(VarA, int);

while (ParClass && Change < WCS_NOTIFY_DELAY_MAX_CHANGE)
	{
	if (ParClass > 255)
		{
		NotifyChanges[Change] = ParClass;
		SubClass = (ParClass & 0xff0000) >> 16;
		Item = (ParClass & 0xff00) >> 8;
		Component = (ParClass & 0xff);
		ParClass >>= 24;
		} // if event passed as longword ID
	else
		{
		SubClass = va_arg(VarA, int);
		Item = va_arg(VarA, int);
		Component = va_arg(VarA, int);
		NotifyChanges[Change] = (ParClass << 24) + (SubClass << 16) + (Item << 8) + Component;
		} // else event passed as decomposed list of class, subclass, item and component

	ParClass = va_arg(VarA, int);
	Change ++;

	} // while 

va_end(VarA);

NotifyChanges[Change] = 0;


if (Notify)
	{
	GenerateNotify(NotifyChanges);
	} // if notify clients of changes

} // ImageLib::SetParam() 

/*===========================================================================*/

void ImageLib::GetParam(void *Value, ...)
{

} // ImageLib::GetParam() 

/*===========================================================================*/

// this creates a thumbnail-sized dummy raster useable as a stand-in
// anywhere where it would be prohibitive to create a real thumbnail
Raster *ImageLib::CreateStandInRast(void)
{
Raster *TestRast = NULL;
int Rows = 100, Cols = 100, Yloop, Xloop;
unsigned char OtherColor = 0, RedGun = 0;

if (TestRast = new Raster)
	{
	TestRast->Cols = Cols;
	TestRast->Rows = Rows;
	TestRast->ByteBands = 3;
	TestRast->ByteBandSize = Rows * Cols;
	TestRast->AlphaAvailable = 0;
	TestRast->AlphaEnabled = 0;

	TestRast->ByteMap[0] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Standin Thumbnail Bitmaps");
	TestRast->ByteMap[1] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Standin Thumbnail Bitmaps");
	TestRast->ByteMap[2] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Standin Thumbnail Bitmaps");

	if (TestRast->ByteMap[0] && TestRast->ByteMap[1] && TestRast->ByteMap[2]) // everything ok?
		{
		FontImage FI;
		InternalRasterFont IRF(1);
		int XCenter, YPos, YHeight;
		char StandinString[30];

		// draw red/white checkerboard backdrop
		for (Yloop = 0; Yloop < Rows; Yloop++)
			{
			for (Xloop = 0; Xloop < Cols; Xloop++)
				{
				// make 10x10 pixel checkerboard pattern
				if (((Xloop / 10) & 0x01) ^ ((Yloop / 10) & 0x01))
					{
					// make it grey
					OtherColor = 128;
					RedGun = 128;
					} // if
				else
					{
					// make G & B 0 to make it red
					OtherColor = 0;
					RedGun = 200;
					} // else
				TestRast->ByteMap[0][Yloop * Cols + Xloop] = RedGun;
				TestRast->ByteMap[1][Yloop * Cols + Xloop] = TestRast->ByteMap[2][Yloop * Cols + Xloop] = OtherColor;
				} // for
			} // for

		// not sure why this looks better than 50
		XCenter = 0;
		YHeight = IRF.InternalFontHeight / 16 + 2;

		FI.SetFont(IRF.FontDataPointer, (USHORT)IRF.InternalFontWidth, (USHORT)IRF.InternalFontHeight);
		FI.SetOutput(Cols, Rows, TestRast->ByteMap[0], TestRast->ByteMap[1], TestRast->ByteMap[2], NULL);
		YPos = 35;

		sprintf(StandinString, "STAND-IN");
		// fake bold by offset impressions
		FI.PrintText(XCenter - 1, YPos - 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter + 1, YPos + 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter + 1, YPos - 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter - 1, YPos + 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter, YPos, StandinString, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		YPos += YHeight;

		sprintf(StandinString, "IMAGE");
		// fake bold by offset impressions
		FI.PrintText(XCenter - 1, YPos - 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter + 1, YPos + 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter + 1, YPos - 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter - 1, YPos + 1, StandinString, 0, 0, 0, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		FI.PrintText(XCenter, YPos, StandinString, 255, 255, 255, WCS_FONTIMAGE_JUSTIFY_CENTER, WCS_FONTIMAGE_DRAWMODE_NORMAL, 1, 1, 0, NULL);
		} // if
	TestRast->SetInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED | WCS_RASTER_INITFLAGS_DOWNSAMPLED);
	} // if

return(TestRast);

} // ImageLib::CreateStandInRast

/*===========================================================================*/
/*===========================================================================*/

// RasterAttribute

RasterAttribute::RasterAttribute()
{

Next = NULL;
Shell = NULL;

} // RasterAttribute::RasterAttribute

/*===========================================================================*/

RasterAttribute::RasterAttribute(RasterShell *AddShell, char AddType, Raster *RastSource, RasterAnimHost *HostSource)
{

Next = NULL;

if (AddShell)
	{
	Shell = AddShell;
	} // if shell provided
else
	{
	Shell = NULL;
	switch (AddType)
		{
		case WCS_RASTERSHELL_TYPE_IMAGEMANAGER:
			{
			Shell = (RasterShell *)(new ImageManagerShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGEMANAGER
		case WCS_RASTERSHELL_TYPE_GEOREF:
			{
			Shell = (RasterShell *)(new GeoRefShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_GEOREF
		case WCS_RASTERSHELL_TYPE_IMAGE:
			{
			Shell = (RasterShell *)(new ImageShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGE
		case WCS_RASTERSHELL_TYPE_FOLIAGE:
			{
			Shell = (RasterShell *)(new FoliageShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_FOLIAGE
		case WCS_RASTERSHELL_TYPE_SEQUENCE:
			{
			Shell = (RasterShell *)(new SequenceShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_SEQUENCE
		case WCS_RASTERSHELL_TYPE_DISSOLVE:
			{
			Shell = (RasterShell *)(new DissolveShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_DISSOLVE
		case WCS_RASTERSHELL_TYPE_COLORCONTROL:
			{
			Shell = (RasterShell *)(new ColorControlShell(RastSource));
			break;
			} // WCS_RASTERSHELL_TYPE_COLORCONTROL
		} // switch
	} // else not provided
if (Shell)
	{
	Shell->SetHost(HostSource);
	Shell->SetRaster(RastSource);
	} // if

} // RasterAttribute::RasterAttribute

/*===========================================================================*/

RasterAttribute::~RasterAttribute()
{
RasterShell *CurShell;
RasterAnimHost *RemoveHost;

if (Shell)
	{
	if (Shell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
		{
		while (Shell)
			{
			CurShell = Shell;
			Shell = ((SequenceShell *)Shell)->NextSeq;
			if (RemoveHost = CurShell->Host)
				{
				CurShell->Host = NULL;
				RemoveHost->RemoveRaster(CurShell);
				} // if
			delete CurShell;
			} // while
		} // if
	else if (Shell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
		{
		while (Shell)
			{
			CurShell = Shell;
			Shell = ((DissolveShell *)Shell)->NextDis;
			if (RemoveHost = CurShell->Host)
				{
				CurShell->Host = NULL;
				RemoveHost->RemoveRaster(CurShell);
				} // if
			delete CurShell;
			} // while
		} // if
	else
		{
		CurShell = Shell;
		Shell = NULL;
		if (RemoveHost = CurShell->Host)
			{
			CurShell->Host = NULL;
			RemoveHost->RemoveRaster(CurShell);
			} // if
		delete CurShell;
		} // else
	} // if

} // RasterAttribute::~RasterAttribute

/*===========================================================================*/

void RasterAttribute::Copy(RasterAttribute *CopyTo, RasterAttribute *CopyFrom)
{

if (CopyFrom->Shell)
	{
	switch(CopyFrom->Shell->GetType())
		{
		case WCS_RASTERSHELL_TYPE_IMAGEMANAGER:
			{
			CopyTo->Shell = (RasterShell *)(new ImageManagerShell);
			((ImageManagerShell *)CopyTo->Shell)->Copy((ImageManagerShell *)CopyTo->Shell, (ImageManagerShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGEMANAGER
		case WCS_RASTERSHELL_TYPE_GEOREF:
			{
			CopyTo->Shell = (RasterShell *)(new GeoRefShell);
			((GeoRefShell *)CopyTo->Shell)->Copy((GeoRefShell *)CopyTo->Shell, (GeoRefShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_GEOREF
		case WCS_RASTERSHELL_TYPE_IMAGE:
			{
			CopyTo->Shell = (RasterShell *)(new ImageShell);
			((ImageShell *)CopyTo->Shell)->Copy((ImageShell *)CopyTo->Shell, (ImageShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGE
		case WCS_RASTERSHELL_TYPE_FOLIAGE:
			{
			CopyTo->Shell = (RasterShell *)(new FoliageShell);
			((FoliageShell *)CopyTo->Shell)->Copy((FoliageShell *)CopyTo->Shell, (FoliageShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_FOLIAGE
		case WCS_RASTERSHELL_TYPE_SEQUENCE:
			{
			CopyTo->Shell = (RasterShell *)(new SequenceShell);
			((SequenceShell *)CopyTo->Shell)->Copy((SequenceShell *)CopyTo->Shell, (SequenceShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_SEQUENCE
		case WCS_RASTERSHELL_TYPE_DISSOLVE:
			{
			CopyTo->Shell = (RasterShell *)(new DissolveShell);
			((DissolveShell *)CopyTo->Shell)->Copy((DissolveShell *)CopyTo->Shell, (DissolveShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_DISSOLVE
		case WCS_RASTERSHELL_TYPE_COLORCONTROL:
			{
			CopyTo->Shell = (RasterShell *)(new ColorControlShell);
			((ColorControlShell *)CopyTo->Shell)->Copy((ColorControlShell *)CopyTo->Shell, (ColorControlShell *)CopyFrom->Shell);
			break;
			} // WCS_RASTERSHELL_TYPE_COLORCONTROL
		} // switch
	} // if
CopyTo->Next = NULL;

} // RasterAttribute::Copy

/*===========================================================================*/

int RasterAttribute::CheckOKRemove(void)
{

if (Shell)
	return (Shell->CheckOKRemove());
return (1);

} // RasterAttribute::CheckOKRemove

/*===========================================================================*/

int RasterAttribute::CheckOKRemove(Raster *CheckMe)
{

if (Shell)
	return (Shell->CheckOKRemove(CheckMe));
return (1);

} // RasterAttribute::CheckOKRemove

/*===========================================================================*/

int RasterAttribute::RemoveRaster(Raster *RemoveMe)
{

if (Shell)
	return (Shell->RemoveRaster(RemoveMe, 0));
return (1);

} // RasterAttribute::RemoveRaster

/*===========================================================================*/

char *RasterAttribute::GetName(void)
{

if (Shell)
	return (Shell->GetHostName());
return ("Unknown Attribute");

} // RasterAttribute::GetName

/*===========================================================================*/

char *RasterAttribute::GetTypeString(void)
{

if (Shell)
	return (Shell->GetHostTypeString());
return ("Unknown Attribute");

} // RasterAttribute::GetTypeString

/*===========================================================================*/

int RasterAttribute::GetEnabled(void)
{

if (Shell)
	return (Shell->GetHostEnabled());
return (0);

} // RasterAttribute::GetEnabled

/*===========================================================================*/

void RasterAttribute::Edit(void)
{

if (Shell)
	Shell->EditHost();

} // RasterAttribute::Edit

/*===========================================================================*/

Raster *RasterAttribute::TraceDissolveRasterMatch(Raster *MatchRast, short Iteration)
{
DissolveShell *Current;
Raster *MyRast;
RasterAttribute *MyAttr;

if (Iteration >= 10)
	return (MatchRast);
if (Shell)
	{
	for (Current = (DissolveShell *)Shell; Current; Current = (DissolveShell *)Current->NextDis)
		{
		if ((MyRast = Current->GetTarget()))
			{
			if (MyRast == MatchRast)
				return (MatchRast);
			if (MyAttr = MyRast->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
				{
				if (MyAttr->TraceDissolveRasterMatch(MatchRast, Iteration + 1))
					return (MatchRast);
				} // if
			} // if
		} // for
	} // if

return (NULL);

} // RasterAttribute::TraceDissolveRasterMatch

/*===========================================================================*/

RasterShell *RasterAttribute::RemovePartialSequence(SequenceShell *RemoveMe)
{
SequenceShell *MyShell, *PrevShell = NULL;

if (MyShell = (SequenceShell *)Shell)
	{
	while (MyShell && MyShell != RemoveMe)
		{
		PrevShell = MyShell;
		MyShell = (SequenceShell *)MyShell->NextSeq;
		} // while
	if (MyShell)
		{
		if (PrevShell)
			{
			((SequenceShell *)PrevShell)->NextSeq = ((SequenceShell *)MyShell)->NextSeq;
			if (((SequenceShell *)PrevShell)->NextSeq)
				((SequenceShell *)((SequenceShell *)PrevShell)->NextSeq)->PrevSeq = PrevShell;
			} // if
		else
			{
			Shell = ((SequenceShell *)MyShell)->NextSeq;
			if (Shell)
				((SequenceShell *)Shell)->PrevSeq = NULL;
			} // else
		delete MyShell;
		} // if
	} // if

return (Shell);

} // RasterAttribute::RemovePartialSequence

/*===========================================================================*/

RasterShell *RasterAttribute::RemovePartialDissolve(DissolveShell *RemoveMe)
{
DissolveShell *MyShell, *PrevShell = NULL;

if (MyShell = (DissolveShell *)Shell)
	{
	while (MyShell && MyShell != RemoveMe)
		{
		PrevShell = MyShell;
		MyShell = (DissolveShell *)MyShell->NextDis;
		} // while
	if (MyShell)
		{
		if (PrevShell)
			{
			((DissolveShell *)PrevShell)->NextDis = ((DissolveShell *)MyShell)->NextDis;
			if (((DissolveShell *)PrevShell)->NextDis)
				((DissolveShell *)((DissolveShell *)PrevShell)->NextDis)->PrevDis = PrevShell;
			} // if
		else
			{
			Shell = ((DissolveShell *)MyShell)->NextDis;
			if (Shell)
				((DissolveShell *)Shell)->PrevDis = NULL;
			} // else
		delete MyShell;
		} // if
	} // if

return (Shell);

} // RasterAttribute::RemovePartialDissolve

/*===========================================================================*/

RasterShell *RasterAttribute::AdjustSequenceOrder(SequenceShell *AdjustMe, long Adjustment)
{
SequenceShell *MyShell, *PrevShell = NULL, *PrevPrevShell = NULL, *TempShell;

if (MyShell = (SequenceShell *)Shell)
	{
	while (MyShell && MyShell != AdjustMe)
		{
		if (PrevShell)
			PrevPrevShell = PrevShell;
		PrevShell = MyShell;
		MyShell = (SequenceShell *)MyShell->NextSeq;
		} // while
	if (MyShell)
		{
		if (Adjustment > 0)
			{
			if (MyShell->NextSeq)
				{
				((SequenceShell *)MyShell->NextSeq)->StartFrame = MyShell->StartFrame - MyShell->StartSpace + ((SequenceShell *)MyShell->NextSeq)->StartSpace;
				((SequenceShell *)MyShell->NextSeq)->EndFrame = ((SequenceShell *)MyShell->NextSeq)->StartFrame + ((SequenceShell *)MyShell->NextSeq)->Duration;
				if (PrevShell)
					{
					TempShell = (SequenceShell *)((SequenceShell *)MyShell->NextSeq)->NextSeq;
					PrevShell->NextSeq = MyShell->NextSeq;
					((SequenceShell *)PrevShell->NextSeq)->NextSeq = (RasterShell *)MyShell;
					MyShell->NextSeq = (RasterShell *)TempShell;
					} // if
				else
					{
					TempShell = (SequenceShell *)((SequenceShell *)MyShell->NextSeq)->NextSeq;
					Shell = MyShell->NextSeq;
					((SequenceShell *)Shell)->NextSeq = (RasterShell *)MyShell;
					MyShell->NextSeq = (RasterShell *)TempShell;
					} // else
				return (Shell);
				} // if
			} // if
		else if (Adjustment < 0)
			{
			if (PrevShell)
				{
				MyShell->StartFrame = PrevShell->StartFrame - PrevShell->StartSpace + MyShell->StartSpace;
				MyShell->EndFrame = MyShell->StartFrame + MyShell->Duration;
				if (PrevPrevShell)
					{
					TempShell = (SequenceShell *)MyShell->NextSeq;
					PrevPrevShell->NextSeq = (RasterShell *)MyShell;
					MyShell->NextSeq = (RasterShell *)PrevShell;
					PrevShell->NextSeq = (RasterShell *)TempShell; 
					} // if
				else
					{
					TempShell = (SequenceShell *)MyShell->NextSeq;
					Shell = (RasterShell *)MyShell;
					MyShell->NextSeq = (RasterShell *)PrevShell;
					PrevShell->NextSeq = (RasterShell *)TempShell; 
					} // else
				return (Shell);
				} // if
			} // else if
		} // if
	} // if

return (NULL);

} // RasterAttribute::AdjustSequenceOrder

/*===========================================================================*/

RasterShell *RasterAttribute::AdjustDissolveOrder(DissolveShell *AdjustMe, long Adjustment)
{
DissolveShell *MyShell, *PrevShell = NULL, *PrevPrevShell = NULL, *TempShell;

if (MyShell = (DissolveShell *)Shell)
	{
	while (MyShell && MyShell != AdjustMe)
		{
		if (PrevShell)
			PrevPrevShell = PrevShell;
		PrevShell = MyShell;
		MyShell = (DissolveShell *)MyShell->NextDis;
		} // while
	if (MyShell)
		{
		if (Adjustment > 0)
			{
			if (MyShell->NextDis)
				{
				swmem(&MyShell->StartFrame, &((DissolveShell *)MyShell->NextDis)->StartFrame, sizeof (double));
				swmem(&MyShell->EndFrame, &((DissolveShell *)MyShell->NextDis)->EndFrame, sizeof (double));
				if (PrevShell)
					{
					TempShell = (DissolveShell *)((DissolveShell *)MyShell->NextDis)->NextDis;
					PrevShell->NextDis = MyShell->NextDis;
					((DissolveShell *)PrevShell->NextDis)->NextDis = (RasterShell *)MyShell;
					MyShell->NextDis = (RasterShell *)TempShell;
					} // if
				else
					{
					TempShell = (DissolveShell *)((DissolveShell *)MyShell->NextDis)->NextDis;
					Shell = MyShell->NextDis;
					((DissolveShell *)Shell)->NextDis = (RasterShell *)MyShell;
					MyShell->NextDis = (RasterShell *)TempShell;
					} // else
				return (Shell);
				} // if
			} // if
		else if (Adjustment < 0)
			{
			if (PrevShell)
				{
				swmem(&MyShell->StartFrame, &PrevShell->StartFrame, sizeof (double));
				swmem(&MyShell->EndFrame, &PrevShell->EndFrame, sizeof (double));
				if (PrevPrevShell)
					{
					TempShell = (DissolveShell *)MyShell->NextDis;
					PrevPrevShell->NextDis = (RasterShell *)MyShell;
					MyShell->NextDis = (RasterShell *)PrevShell;
					PrevShell->NextDis = (RasterShell *)TempShell; 
					} // if
				else
					{
					TempShell = (DissolveShell *)MyShell->NextDis;
					Shell = (RasterShell *)MyShell;
					MyShell->NextDis = (RasterShell *)PrevShell;
					PrevShell->NextDis = (RasterShell *)TempShell; 
					} // else
				return (Shell);
				} // if
			} // else if
		} // if
	} // if

return (NULL);

} // RasterAttribute::AdjustDissolveOrder

/*===========================================================================*/

ULONG RasterAttribute::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
      case WCS_RASTER_ATTRIB_SHELL_IMAGEMANAGER:
       {
       if (Shell = (RasterShell *)(new ImageManagerShell(RastSource)))
        {
        BytesRead = ((ImageManagerShell *)Shell)->Load(ffile, Size, ByteFlip);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_GEOREF:
       {
       if (Shell = (RasterShell *)(new GeoRefShell(RastSource)))
        {
        BytesRead = ((GeoRefShell *)Shell)->Load(ffile, Size, ByteFlip);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_IMAGE:
       {
       if (Shell = (RasterShell *)(new ImageShell(RastSource)))
        {
        BytesRead = ((ImageShell *)Shell)->Load(ffile, Size, ByteFlip);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_FOLIAGE:
       {
       if (Shell = (RasterShell *)(new FoliageShell(RastSource)))
        {
        BytesRead = ((FoliageShell *)Shell)->Load(ffile, Size, ByteFlip);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_SEQUENCE:
       {
       if (Shell = (RasterShell *)(new SequenceShell(RastSource)))
        {
        BytesRead = ((SequenceShell *)Shell)->Load(ffile, Size, ByteFlip, RastSource);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_DISSOLVE:
       {
       if (Shell = (RasterShell *)(new DissolveShell(RastSource)))
        {
        BytesRead = ((DissolveShell *)Shell)->Load(ffile, Size, ByteFlip, RastSource);
        } // if
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_ATTRIB_SHELL_COLORCONTROL:
       {
       if (Shell = (RasterShell *)(new ColorControlShell(RastSource)))
        {
        BytesRead = ((ColorControlShell *)Shell)->Load(ffile, Size, ByteFlip);
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

} // RasterAttribute::Load

/*===========================================================================*/

ULONG RasterAttribute::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (Shell)
	{
	switch (Shell->GetType())
		{
		case WCS_RASTERSHELL_TYPE_IMAGEMANAGER:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_IMAGEMANAGER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ImageManagerShell *)Shell)->Save(ffile))
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
						} // if ImageManagerShell saved 
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGEMANAGER
		case WCS_RASTERSHELL_TYPE_GEOREF:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_GEOREF + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((GeoRefShell *)Shell)->Save(ffile))
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
						} // if geo ref shell saved 
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_GEOREF
		case WCS_RASTERSHELL_TYPE_IMAGE:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_IMAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ImageShell *)Shell)->Save(ffile, TRUE))
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
						} /* if image shell saved */
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_IMAGE
		case WCS_RASTERSHELL_TYPE_FOLIAGE:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_FOLIAGE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((FoliageShell *)Shell)->Save(ffile))
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
						} /* if foliage shell saved */
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_FOLIAGE
		case WCS_RASTERSHELL_TYPE_SEQUENCE:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_SEQUENCE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((SequenceShell *)Shell)->Save(ffile))
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
						} /* if sequence shell saved */
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_SEQUENCE
		case WCS_RASTERSHELL_TYPE_DISSOLVE:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_DISSOLVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((DissolveShell *)Shell)->Save(ffile))
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
						} /* if dissolve shell saved */
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_DISSOLVE
		case WCS_RASTERSHELL_TYPE_COLORCONTROL:
			{
			ItemTag = WCS_RASTER_ATTRIB_SHELL_COLORCONTROL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				ItemTag = 0;
				if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					TotalWritten += BytesWritten;

					if (BytesWritten = ((ColorControlShell *)Shell)->Save(ffile))
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
						} /* if color control shell saved */
					else
						goto WriteError;
					} /* if size written */
				else
					goto WriteError;
				} /* if tag written */
			else
				goto WriteError;
			break;
			} // WCS_RASTERSHELL_TYPE_COLORCONTROL
		} // switch
	} // if

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // RasterAttribute::Save

/*===========================================================================*/
/*===========================================================================*/
// ImageCacheControl

ImageCacheControl::ImageCacheControl()
{

QueryOnlyFlags = NULL;
ImageManagementEnable = LoadingSubTile = 0;
NativeTileWidth = NativeTileHeight = 0;
LoadXOri = LoadYOri = LoadWidth = LoadHeight = 0;
} // ImageCacheControl::ImageCacheControl

/*===========================================================================*/
/*===========================================================================*/
// Raster

Raster::Raster()
: RasterAnimHost(NULL)
{
char Ct;

ID = 0;
Rows = Cols = ByteBandSize = FloatBandSize = ShortBandSize = AltByteBandSize = AltFloatBandSize = Row = Col = RasterCell = AltData = 0;
ConstructError = ByteBands = FloatBands = FormatLastLoaded = SaveFormat = 0;
Enabled = 1;
DissolveEnabled = SequenceEnabled = ColorControlEnabled = ImageManagerEnabled = 0;
Longevity = WCS_RASTER_LONGEVITY_LONG;
LoadFast = 0;
AlphaAvailable = 0;
AlphaEnabled = 0;
Name[0] = 0;
InitFlags = 0;
Thumb = NULL;
ThumbDisplayListNum = ULONG_MAX; // no displaylist
LoaderGeoRefShell = NULL;

ImageCapabilityFlags = 0;
NativeTileWidth = NativeTileHeight = 0;
CacheNext = CachePrev = NULL;

TNRowsReq = TNColsReq = TNRealRows = TNRealCols = 0;

for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS; Ct ++)
	{
	ByteMap[Ct] = NULL;
	FloatMap[Ct] = NULL;
	ShortMap[Ct] = NULL;
	} // for
for (Ct = 0; Ct < 3; Ct ++)
	{
	AverageBand[Ct] = .5;
	} // for
RowFloatArray[0] = RowFloatArray[1] = NULL;
AltByteMap = NULL;
AltFloatMap = NULL;
RowZip = NULL;
rPixelBlock = NULL;
PolyListBlock = NULL;
Attr = NULL;
Next = Smaller = NULL;
Red = Green = Blue = AABuf = NULL;
AverageCoverage = 1.0;
Buffers = NULL;
ExponentBuf = NULL;
NormalBuf[0] = NormalBuf[1] = NormalBuf[2] = ZBuf = ReflectionBuf = NULL;

LoadCompositer = NULL;

} // Raster::Raster

/*===========================================================================*/

Raster::~Raster()
{
RasterAttribute *NextAttr;
BufferNode *NextBuf;

ClearAllTilesFromCache();

while (Buffers)
	{
	NextBuf = Buffers;
	Buffers = Buffers->Next;
	delete NextBuf;
	} // if

FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);

DiscardLoaderGeoRefShell();

if (ThumbDisplayListNum != ULONG_MAX)
	{
	glDeleteLists(ThumbDisplayListNum, 1);
	ThumbDisplayListNum = ULONG_MAX;
	} // if

if (Thumb)
	{
	Thumbnail *DelThumb = Thumb;
	Thumb = NULL;
	delete DelThumb;
	} // if
while (Attr)
	{
	NextAttr = Attr;
	Attr = Attr->Next;
	delete NextAttr;
	} //
delete Smaller;		// hope this don' blow de stack!

if (GlobalApp && GlobalApp->GUIWins)
	{
	if (GlobalApp->GUIWins->IVG && GlobalApp->GUIWins->IVG->GetActive() == this
		&& ! GlobalApp->GUIWins->IVG->Closing)
		{
		delete GlobalApp->GUIWins->IVG;
		GlobalApp->GUIWins->IVG = NULL;
		} // if
	} // if

} // Raster::~Raster


/*===========================================================================*/


unsigned long Raster::QueryMaxCacheSize(void)
{
return(GlobalApp->AppImages->MaxTileMemory * 1024 * 1024);  // value is stored in Mb
} // Raster::QueryMaxCacheSize


/*===========================================================================*/

Raster *Raster::TileCache; // static, one copy is shared by all Rasters but only scoped to Raster object
// cache memory consumption control (static, one copy is shared by all Rasters but only scoped to Raster object)
unsigned long Raster::CacheCurrentMemorySize;

void Raster::ClearAllTilesFromCache(void)
{
Raster *CacheEmpty, *CacheClearNext, *TempTileCache;

// empty all our tiles from image management cache
if (TileCache)
	{
	TempTileCache = TileCache; // prevent recursion when we delete tile Rasters, invoking their delete and therefore ClearAllTilesFromCache
	TileCache = NULL;
	for (CacheEmpty = TempTileCache; CacheEmpty; CacheEmpty = CacheClearNext)
		{
		CacheClearNext = CacheEmpty->CacheNext;
		if (CacheEmpty->Next == this) // is it one of ours?
			{
			// decrement current cache size
			CacheCurrentMemorySize -= ((CacheEmpty->Rows * CacheEmpty->Cols) * CacheEmpty->ByteBands);
			if (TempTileCache == CacheEmpty)
				{
				TempTileCache = CacheEmpty->CacheNext;
				} // if
			if (CacheEmpty->CachePrev)
				{
				CacheEmpty->CachePrev->CacheNext = CacheEmpty->CacheNext;
				} // if
			if (CacheEmpty->CacheNext)
				{
				CacheEmpty->CacheNext->CachePrev = CacheEmpty->CachePrev;
				} // if
			// is the tile we're deleting in fact the current head of the list?
			if (TempTileCache == CacheEmpty)
				{
				// move the next tile (if any) to head of list
				TempTileCache = CacheEmpty->CacheNext;
				} // if
			delete CacheEmpty;
			} // if
		} // for
	TileCache = TempTileCache;
	} // if
} // Raster::ClearAllTilesFromCache


/*===========================================================================*/

void Raster::DiscardLoaderGeoRefShell(void)
{
if (LoaderGeoRefShell)
	{
	if (LoaderGeoRefShell->Host)
		{
		CoordSys *DeleteCS;
		DeleteCS = (CoordSys *)LoaderGeoRefShell->Host;
		delete DeleteCS;
		LoaderGeoRefShell->Host = NULL;
		} // 
	delete LoaderGeoRefShell;
	LoaderGeoRefShell = NULL;
	} // if
} // Raster::DiscardLoaderGeoRefShell

/*===========================================================================*/

void Raster::Copy(Raster *CopyTo, Raster *CopyFrom, int CopyExternalAttributes)
{
RasterAttribute *FromAttr, **ToAttrPtr;

CopyTo->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
CopyTo->ID = CopyFrom->ID;
CopyTo->Rows = CopyFrom->Rows;
CopyTo->Cols = CopyFrom->Cols;
CopyTo->Row = 0;
CopyTo->Col = 0;
CopyTo->RasterCell = 0;
CopyTo->AltData = CopyFrom->AltData;
CopyTo->ConstructError = 0;
CopyTo->ByteBands = CopyFrom->ByteBands;
CopyTo->FloatBands = CopyFrom->FloatBands;
CopyTo->ByteBandSize = CopyFrom->ByteBandSize;
CopyTo->FloatBandSize = CopyFrom->FloatBandSize;
CopyTo->AltByteBandSize = CopyFrom->AltByteBandSize;
CopyTo->AltFloatBandSize = CopyFrom->AltFloatBandSize;
CopyTo->FormatLastLoaded = CopyFrom->FormatLastLoaded;
CopyTo->SaveFormat = CopyFrom->SaveFormat;
CopyTo->Longevity = CopyFrom->Longevity;
CopyTo->LoadFast = CopyFrom->LoadFast;
CopyTo->AlphaAvailable = CopyFrom->AlphaAvailable;
CopyTo->AlphaEnabled = CopyFrom->AlphaEnabled;
CopyTo->DissolveEnabled = CopyFrom->DissolveEnabled;
CopyTo->SequenceEnabled = CopyFrom->SequenceEnabled;
CopyTo->ColorControlEnabled = CopyFrom->ColorControlEnabled;
CopyTo->ImageManagerEnabled = CopyFrom->ImageManagerEnabled;
CopyTo->Enabled = CopyFrom->Enabled;
CopyTo->ImageCapabilityFlags = CopyFrom->ImageCapabilityFlags;
CopyTo->NativeTileWidth = CopyFrom->NativeTileWidth;
CopyTo->NativeTileHeight = CopyFrom->NativeTileHeight;
CopyTo->Mask = 0;
if (CopyFrom->Thumb)
	{
	if (CopyTo->Thumb || (CopyTo->Thumb = new Thumbnail()))
		CopyTo->Thumb->Copy(CopyTo->Thumb, CopyFrom->Thumb);
	} // if
else if (CopyTo->Thumb)
	{
	Thumbnail *DelThumb = CopyTo->Thumb;
	CopyTo->Thumb = NULL;
	delete DelThumb;
	} // else if
CopyTo->RowZip = NULL;
while (CopyTo->Attr)
	{
	FromAttr = CopyTo->Attr;
	CopyTo->Attr = CopyTo->Attr->Next;
	delete FromAttr;
	} // while
FromAttr = CopyFrom->Attr;
ToAttrPtr = &CopyTo->Attr;
while (FromAttr)
	{
	if (CopyExternalAttributes || FromAttr->IsInternal())
		{
		if (*ToAttrPtr = new RasterAttribute)
			{
			// note that this sets the Shell's Raster pointer to the source shell's raster pointer
			// I'm not sure that is ever a good thing.
			(*ToAttrPtr)->Copy(*ToAttrPtr, FromAttr);
			// So we will set the Rast member of the shell to our CopyTo raster and hope this doesn't cause problems.
			// Actually, it might prevent problems.
			(*ToAttrPtr)->GetShell()->Rast = CopyTo;
			// if this attribute is a georef shell then we don't want to prejudice the copy with a possibly bogus host pointer
			if (! CopyExternalAttributes && FromAttr->GetShell() && FromAttr->GetShell()->GetType() == WCS_RASTERSHELL_TYPE_GEOREF)
				{
				(*ToAttrPtr)->GetShell()->Host = NULL;
				} // if
			ToAttrPtr = &(*ToAttrPtr)->Next;
			} // if
		} // if
	FromAttr = FromAttr->Next;
	} // while
PAF.Copy(&CopyTo->PAF, &CopyFrom->PAF);
strcpy(CopyTo->Name, CopyFrom->Name);
CopyTo->Next = CopyTo->Smaller = NULL;

} // Raster::Copy

/*===========================================================================*/

int Raster::CheckImageInUse(void)
{
RasterAttribute *MyAttr;

if (MyAttr = Attr)
	{
	while (MyAttr)
		{
		if (MyAttr->Shell && MyAttr->Shell->Host)
			return (1);
		MyAttr = MyAttr->Next;
		} // while
	} // if

return (0);

} // Raster::CheckImageInUse

/*===========================================================================*/

int Raster::CheckOKRemove(void)
{
RasterAttribute *MyAttr;
int RemoveIt = 1;

if (MyAttr = Attr)
	{
	while (MyAttr && (RemoveIt = MyAttr->CheckOKRemove()))
		{
		MyAttr = MyAttr->Next;
		} // while
	} // if

return (RemoveIt);

} // Raster::CheckOKRemove

/*===========================================================================*/

int Raster::CheckOKRemove(Raster *CheckMe)
{
RasterAttribute *MyAttr;
int RemoveIt = 1;

if (MyAttr = Attr)
	{
	while (MyAttr && (RemoveIt = MyAttr->CheckOKRemove(CheckMe)))
		{
		MyAttr = MyAttr->Next;
		} // while
	} // if

return (RemoveIt);

} // Raster::CheckOKRemove

/*===========================================================================*/

int Raster::RemoveRaster(Raster *RemoveMe)
{
RasterAttribute *MyAttr;
int RemoveIt = 1;

if (MyAttr = Attr)
	{
	while (MyAttr && (RemoveIt = MyAttr->RemoveRaster(RemoveMe)))
		{
		MyAttr = MyAttr->Next;
		} // while
	} // if

return (RemoveIt);

} // Raster::RemoveRaster

/*===========================================================================*/

int Raster::SetToTime(double Time)
{
long Found = 0;
RasterAttribute *MyAttr;

if ((MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	if (((GeoRefShell *)MyAttr->GetShell())->GeoReg.SetToTime(Time))
		Found = 1;
	} // if

return (Found);

} // Raster::SetToTime

/*===========================================================================*/

int Raster::GetRAHostAnimated(void)
{
RasterAttribute *MyAttr;

if ((MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	if (((GeoRefShell *)MyAttr->GetShell())->GeoReg.GetRAHostAnimated())
		{
		return (1);
		} // if
	} // if

return (0);

} // Raster::GetRAHostAnimated

/*===========================================================================*/

long Raster::GetKeyFrameRange(double &FirstKey, double &LastKey)
{
double TestFirst = FLT_MAX, TestLast = 0.0, MaxDist = 0.0, MinDist = 0.0;
long Found = 0;
RasterAttribute *MyAttr;

if ((MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	if (((GeoRefShell *)MyAttr->GetShell())->GeoReg.GetKeyFrameRange(MinDist, MaxDist))
		{
		if (MinDist < TestFirst)
			TestFirst = MinDist;
		if (MaxDist > TestLast)
			TestLast = MaxDist;
		Found = 1;
		} // if
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

} // Raster::GetKeyFrameRange

/*===========================================================================*/

UBYTE **Raster::CreateImageRGBThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_BYTE, ByteMap[WCS_RASTER_IMAGE_BAND_RED], ByteMap[WCS_RASTER_IMAGE_BAND_GREEN], ByteMap[WCS_RASTER_IMAGE_BAND_BLUE]));

} // Raster::CreateImageRGBThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateRenderRGBThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_BYTE, ByteMap[WCS_RASTER_RENDER_BAND_RED], ByteMap[WCS_RASTER_RENDER_BAND_GREEN], ByteMap[WCS_RASTER_RENDER_BAND_BLUE]));

} // Raster::CreateRenderRGBThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateRenderMergedRGBThumbNail(void)
{

return (CreateMergedThumbNail(
	ByteMap[WCS_RASTER_RENDER_BAND_RED], ByteMap[WCS_RASTER_RENDER_BAND_GREEN], ByteMap[WCS_RASTER_RENDER_BAND_BLUE],
	FloatMap[WCS_RASTER_RENDER_BAND_ZBUF], 
	ByteMap[WCS_RASTER_RENDER_BAND_FOLIAGERED], ByteMap[WCS_RASTER_RENDER_BAND_FOLIAGEGREEN], ByteMap[WCS_RASTER_RENDER_BAND_FOLIAGEBLUE],
	FloatMap[WCS_RASTER_RENDER_BAND_FOLIAGEZBUF]));

} // Raster::CreateRenderMergedRGBThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateFoliageRGBThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_BYTE, ByteMap[WCS_RASTER_FOLIAGE_BAND_RED], ByteMap[WCS_RASTER_FOLIAGE_BAND_GREEN], ByteMap[WCS_RASTER_FOLIAGE_BAND_BLUE]));

} // Raster::CreateFoliageRGBThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateFoliageZOffThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_BYTE, AltByteMap, NULL, NULL));

} // Raster::CreateFoliageZOffThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateFoliageCovgThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_FLOAT, AltFloatMap, NULL, NULL));

} // Raster::CreateFoliageCovgThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateEffectThumbNail(void)
{

return (CreateThumbNail(WCS_RASTER_BANDSET_BYTE, ByteMap[WCS_RASTER_GEOREF_BAND_RED], ByteMap[WCS_RASTER_GEOREF_BAND_GREEN], ByteMap[WCS_RASTER_GEOREF_BAND_BLUE]));

} // Raster::CreateEffectThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateThumbNail(char BandType, void *Band1, void *Band2, void *Band3, unsigned short *OptExponBuf)
{
double Scale, SampleIntX, SampleIntY, SampX, SampY;
float MaxFlt, MinFlt, FltScale;
long CtX, CtY, zip;

if (Thumb || (Thumb = new Thumbnail()))
	{
	Thumb->TNailPadX = Thumb->TNailPadY = WCS_RASTER_TNAIL_SIZE / 2;

	if (Thumb->TNailsValid() || Thumb->AllocTNails())
		{
		if (Cols > 0 && Rows > 0)
			{
			if (Cols > Rows)
				{
				Thumb->TNailPadX = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Cols);
				SampleIntX = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				Thumb->TNailPadY = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Rows * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0) <= 0)
					Thumb->TNailPadY = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntY = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0);
				} // if
			else
				{
				Thumb->TNailPadY = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Rows);
				SampleIntY = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				Thumb->TNailPadX = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Cols * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0) <= 0)
					Thumb->TNailPadX = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntX = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0);
				} // else
			if (BandType == WCS_RASTER_BANDSET_FLOAT)
				{
				FindMaxMinFloatValues((float *)Band1, MaxFlt, MinFlt);
				if (MaxFlt > MinFlt)
					{
					FltScale = (float)255.0 / (MaxFlt - MinFlt);
					} // if
				else
					{
					FltScale = 0.0f;
					} // else
				} // if float
			for (CtY = Thumb->TNailPadY, SampY = 0.0; CtY < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY; CtY ++, SampY += SampleIntY)
				{
				for (CtX = Thumb->TNailPadX, SampX = 0.0, zip = CtY * WCS_RASTER_TNAIL_SIZE + Thumb->TNailPadX; CtX < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX; zip ++, CtX ++, SampX += SampleIntX)
					{
					if (BandType == WCS_RASTER_BANDSET_BYTE)
						{
						SampleBytePoint(SampX, SampY, (UBYTE *)Band1, (UBYTE *)Band2, (UBYTE *)Band3, Thumb->TNail[0][zip], Thumb->TNail[1][zip], Thumb->TNail[2][zip], OptExponBuf);
						} // if
					else if (BandType == WCS_RASTER_BANDSET_FLOAT)
						{
						SampleFloatPoint(SampX, SampY, MinFlt, FltScale, (float *)Band1, Thumb->TNail[0][zip], Thumb->TNail[1][zip], Thumb->TNail[2][zip]);
						} // else
					} // for CtX
				} // for CtY
			return (Thumb->TNail);
			} // if
		} // if bands allocated
	else
		{
		delete Thumb;
		Thumb = NULL;
		} // else
	} // if

return (NULL);

} // Raster::CreateThumbNail

/*===========================================================================*/

Thumbnail *Raster::CreateThumbnail(void)
{
double Scale, SampleIntX, SampleIntY, SampX, SampY;
long CtX, CtY, zip;
Thumbnail *NewNail;

if (NewNail = new Thumbnail())
	{
	NewNail->TNailPadX = NewNail->TNailPadY = WCS_RASTER_TNAIL_SIZE / 2;

	if (NewNail->AllocTNails())	// allocates and clears
		{
		if (Cols > 0 && Rows > 0)
			{
			if (Cols > Rows)
				{
				NewNail->TNailPadX = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Cols);
				SampleIntX = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				NewNail->TNailPadY = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Rows * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadY * 2.0) - 2.0) <= 0)
					NewNail->TNailPadY = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntY = 1.0 / ((WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadY * 2.0) - 2.0);
				} // if
			else
				{
				NewNail->TNailPadY = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Rows);
				SampleIntY = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				NewNail->TNailPadX = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Cols * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadX * 2.0) - 2.0) <= 0)
					NewNail->TNailPadX = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntX = 1.0 / ((WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadX * 2.0) - 2.0);
				} // else
			for (CtY = NewNail->TNailPadY, SampY = 0.0; CtY < WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadY; CtY ++, SampY += SampleIntY)
				{
				for (CtX = NewNail->TNailPadX, SampX = 0.0, zip = CtY * WCS_RASTER_TNAIL_SIZE + NewNail->TNailPadX; CtX < WCS_RASTER_TNAIL_SIZE - NewNail->TNailPadX; zip ++, CtX ++, SampX += SampleIntX)
					{
					SampleBytePoint(SampX, SampY, ByteMap[0], ByteMap[1], ByteMap[2], NewNail->TNail[0][zip], NewNail->TNail[1][zip], NewNail->TNail[2][zip]);
					} // for CtX
				} // for CtY
			return (NewNail);
			} // if
		} // if bands allocated
	delete NewNail;
	} // if

return (NULL);

} // Raster::CreateThumbnail

/*===========================================================================*/

UBYTE **Raster::CreateMergedThumbNail(UBYTE *Band1, UBYTE *Band2, UBYTE *Band3, float *Depth,
	UBYTE *AltBand1, UBYTE *AltBand2, UBYTE *AltBand3, float *AltDepth)
{
double Scale, SampleIntX, SampleIntY, SampX, SampY;
float Val1, Val2;
long CtX, CtY, zip;

if (Thumb || (Thumb = new Thumbnail()))
	{
	Thumb->TNailPadX = Thumb->TNailPadY = WCS_RASTER_TNAIL_SIZE / 2;

	if (Thumb->TNailsValid() || Thumb->AllocTNails())
		{
		if (Cols > 0 && Rows > 0)
			{
			if (Cols > Rows)
				{
				Thumb->TNailPadX = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Cols);
				SampleIntX = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				Thumb->TNailPadY = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Rows * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0) <= 0)
					Thumb->TNailPadY = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntY = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0);
				} // if
			else
				{
				Thumb->TNailPadY = 0;
				Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Rows);
				SampleIntY = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
				Thumb->TNailPadX = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Cols * Scale) * 0.5);
				if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0) <= 0)
					Thumb->TNailPadX = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
				SampleIntX = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0);
				} // else
			for (CtY = Thumb->TNailPadY, SampY = 0.0; CtY < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY; CtY ++, SampY += SampleIntY)
				{
				for (CtX = Thumb->TNailPadX, SampX = 0.0, zip = CtY * WCS_RASTER_TNAIL_SIZE + Thumb->TNailPadX; CtX < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX; zip ++, CtX ++, SampX += SampleIntX)
					{
					SampleFloatPoint(SampX, SampY, Depth, Val1);
					SampleFloatPoint(SampX, SampY, AltDepth, Val2);
					if (Val1 < Val2)
						{
						SampleBytePoint(SampX, SampY, (UBYTE *)Band1, (UBYTE *)Band2, (UBYTE *)Band3, Thumb->TNail[0][zip], Thumb->TNail[1][zip], Thumb->TNail[2][zip]);
						} // if
					else
						{
						SampleBytePoint(SampX, SampY, (UBYTE *)AltBand1, (UBYTE *)AltBand2, (UBYTE *)AltBand3, Thumb->TNail[0][zip], Thumb->TNail[1][zip], Thumb->TNail[2][zip]);
						} // if
					} // for CtX
				} // for CtY
			return (Thumb->TNail);
			} // if
		} // if bands allocated
	else
		{
		delete Thumb;
		Thumb = NULL;
		} // else
	} // if thumbnail allocated


return (NULL);

} // Raster::CreateMergedThumbNail

/*===========================================================================*/

UBYTE **Raster::CreateFragThumbnail(rPixelHeader *HeaderArray)
{
double Scale, SampleIntX, SampleIntY, SampX, SampY;
long CtX, CtY, zip, iX, iY, PixCt;

if (HeaderArray)
	{
	if (Thumb || (Thumb = new Thumbnail()))
		{
		Thumb->TNailPadX = Thumb->TNailPadY = WCS_RASTER_TNAIL_SIZE / 2;

		if (Thumb->TNailsValid() || Thumb->AllocTNails())
			{
			if (Cols > 0 && Rows > 0)
				{
				if (Cols > Rows)
					{
					Thumb->TNailPadX = 0;
					Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Cols);
					SampleIntX = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
					Thumb->TNailPadY = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Rows * Scale) * 0.5);
					if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0) <= 0)
						Thumb->TNailPadY = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
					SampleIntY = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY * 2.0) - 2.0);
					} // if
				else
					{
					Thumb->TNailPadY = 0;
					Scale = (double)(WCS_RASTER_TNAIL_SIZE / (double)Rows);
					SampleIntY = 1.0 / (WCS_RASTER_TNAIL_SIZE - 2.0);
					Thumb->TNailPadX = (UBYTE)((double)(WCS_RASTER_TNAIL_SIZE - Cols * Scale) * 0.5);
					if (((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0) <= 0)
						Thumb->TNailPadX = (WCS_RASTER_TNAIL_SIZE / 2) - 2;
					SampleIntX = 1.0 / ((WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX * 2.0) - 2.0);
					} // else
				for (CtY = Thumb->TNailPadY, SampY = 0.0; CtY < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadY; CtY ++, SampY += SampleIntY)
					{
					for (CtX = Thumb->TNailPadX, SampX = 0.0, zip = CtY * WCS_RASTER_TNAIL_SIZE + Thumb->TNailPadX; CtX < WCS_RASTER_TNAIL_SIZE - Thumb->TNailPadX; zip ++, CtX ++, SampX += SampleIntX)
						{
						if ((iX = quickftol(.5 + SampX * (Cols - 1))) >= 0 && iX < Cols && (iY = quickftol(.5 + SampY * (Rows - 1))) >= 0 && iY < Rows)
							{
							PixCt = iX + iY * Cols;
							HeaderArray[PixCt].CollapsePixel(Thumb->TNail[0][zip], Thumb->TNail[1][zip], Thumb->TNail[2][zip]);
							} // if
						else
							Thumb->TNail[0][zip] = Thumb->TNail[1][zip] = Thumb->TNail[2][zip] = 0;
						} // for CtX
					} // for CtY
				return (Thumb->TNail);
				} // if
			} // if bands allocated
		else
			{
			delete Thumb;
			Thumb = NULL;
			} // else
		} // if
	} // if

return (NULL);

} // Raster::CreateFragThumbnail

/*===========================================================================*/

float Raster::FindMaxMinFloatValues(float *Band, float &MaxFlt, float &MinFlt)
{
long Ct, MaxCt, Found = 0;

if (Band)
	{
	MaxFlt = (float)-FLT_MAX;
	MinFlt = (float)FLT_MAX;
	MaxCt = Rows * Cols;
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		if (Band[Ct] != FLT_MAX)
			{
			Found = 1;
			if (Band[Ct] > MaxFlt)
				MaxFlt = Band[Ct];
			if (Band[Ct] < MinFlt)
				MinFlt = Band[Ct];
			} // if
		} // for
	} // if

if (Found)
	return (MaxFlt - MinFlt);
MaxFlt = MinFlt = 0.0f;
return (0.0f);

} // Raster::FindMaxMinFloatValues

/*===========================================================================*/

// called from ImageViewGUI::SampleImage, and Raster::CreateThumbNail and CreateMergedThumbNail
long Raster::SampleBytePoint(double dX, double dY, UBYTE *Band1, UBYTE *Band2, UBYTE *Band3, UBYTE &RedVal, UBYTE &GreenVal, UBYTE &BlueVal, unsigned short *OptExponBuf)
{
long iX, iY, zip;
unsigned char TempRGB[3];

// this function rounds the sample numbers
if ((iX = quickftol(.5 + dX * (Cols - 1))) >= 0 && iX < Cols && (iY = quickftol(.5 + dY * (Rows - 1))) >= 0 && iY < Rows)
	{
	zip = iX + iY * Cols;
	#ifdef WCS_IMAGE_MANAGEMENT
	if (!QueryImageManagementEnabled())
		{
	#endif // WCS_IMAGE_MANAGEMENT
		if (Band1)
			RedVal = Band1[zip];
		else
			RedVal = 0;
		if (Band2)
			GreenVal = Band2[zip];
		else
			GreenVal = RedVal;
		if (Band3)
			BlueVal = Band3[zip];
		else
			BlueVal = RedVal;
		if (OptExponBuf && OptExponBuf[zip])
			{
			TempRGB[0] = RedVal;
			TempRGB[1] = GreenVal;
			TempRGB[2] = BlueVal;
			rPixelFragment::ExtractClippedExponentialColors(TempRGB, OptExponBuf[zip]);
			RedVal = TempRGB[0];
			GreenVal = TempRGB[1];
			BlueVal = TempRGB[2];
			} // if
	#ifdef WCS_IMAGE_MANAGEMENT
		} // if
	else
		{
		double CellOutput[3];
		int Abort;
		// don't care if it's RGB or monochrome, SampleManagedByteDouble3() handles that for us
		SampleManagedByteDouble3(CellOutput, (unsigned)iX, (unsigned)iY, Abort);
		// we need to add in DBL_EPSILON before converting back to unsigned char to prevent rounding down
		// caused by imperfect precision in doubles for some values, IE:
		// ubyte:132 becomes double:131.99999999... becomes 131
		// correction: we'll just add .5 (so we're rounding) because we don't know that DBL_EPSILON will always be enough
		RedVal = (unsigned char)((CellOutput[0] * 255.0) + 0.5);
		GreenVal = (unsigned char)((CellOutput[1] * 255.0) + 0.5);
		BlueVal = (unsigned char)((CellOutput[2] * 255.0) + 0.5);
		} // else
	#endif // WCS_IMAGE_MANAGEMENT
	return (zip);
	} // if
RedVal = GreenVal = BlueVal = 0;
return (-1);

} // Raster::SampleBytePoint

/*===========================================================================*/

// apparently only called from CreateThumbNail
long Raster::SampleFloatPoint(double dX, double dY, float Base,  float Scale, float *Band1, UBYTE &RedVal, UBYTE &GreenVal, UBYTE &BlueVal)
{
long iX, iY, zip;

// this function rounds the sample numbers
if ((iX = quickftol(.5 + dX * (Cols - 1))) >= 0 && iX < Cols && (iY = quickftol(.5 + dY * (Rows - 1))) >= 0 && iY < Rows)
	{
	zip = iX + iY * Cols;
	if (Band1 && Band1[zip] != FLT_MAX)
		RedVal = GreenVal = BlueVal = (UBYTE)((Band1[zip] - Base) * Scale);
	else
		RedVal = GreenVal = BlueVal = 0;
	return (zip);
	} // if
RedVal = GreenVal = BlueVal = 0;
return (-1);

} // Raster::SampleFloatPoint

/*===========================================================================*/

// apparently only used when making thumbnails from CreateMergedThumbNail
long Raster::SampleFloatPoint(double dX, double dY, float *Band1, float &Value)
{
long iX, iY, zip;

// this function rounds the sample numbers
if ((iX = quickftol(.5 + dX * (Cols - 1))) >= 0 && iX < Cols && (iY = quickftol(.5 + dY * (Rows - 1))) >= 0 && iY < Rows)
	{
	zip = iX + iY * Cols;
	Value = Band1[zip];
	return (zip);
	} // if
Value  = 0.0f;
return (-1);

} // Raster::SampleFloatPoint

/*===========================================================================*/

void Raster::FreeAllBands(char FreeLongevity)
{
char Ct;

if (Longevity <= FreeLongevity)
	{
	for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS; Ct ++)
		{
		FreeByteBand(Ct);
		FreeFloatBand(Ct);
		FreeShortBand(Ct);
		} // for
	FreeRowFloatArray(0);
	FreeRowFloatArray(1);
	FreeAltByteBand();
	FreeAltFloatBand();
	FreePixelFragmentMap();
	FreePolyListMap();
	ByteBandSize = FloatBandSize = AltByteBandSize = AltFloatBandSize = 0;
	if (RowZip)
		delete [] RowZip;
	RowZip = NULL;
	if (Smaller)
		delete Smaller;
	Smaller = NULL;
	ClearInitFlags();
	Red = Green = Blue = NULL;
	#ifdef WCS_IMAGE_MANAGEMENT
	ClearAllTilesFromCache();
	#endif // WCS_IMAGE_MANAGEMENT
	} // if

} // Raster::FreeAllBands

/*===========================================================================*/

void Raster::NullMapPtrs(void)
{
char Ct;

for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS; Ct ++)
	{
	ByteMap[Ct] = NULL;
	FloatMap[Ct] = NULL;
	ShortMap[Ct] = NULL;
	} // for

} // Raster::NullMapPtrs

/*===========================================================================*/

void *Raster::AllocBuffer(BufferNode *BufNode)
{
char Ct;

if (BufNode->Type == WCS_RASTER_BANDSET_BYTE)
	{
	for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS && ByteMap[Ct]; Ct ++);	//lint !e722
	if (Ct < WCS_RASTER_MAX_BANDS)
		{
		if (BufNode->Buffer = AllocByteBand(Ct))
			{
			BufNode->Index = Ct;
			return (BufNode->Buffer);
			} // if
		} // if
	} // if
else if (BufNode->Type == WCS_RASTER_BANDSET_FLOAT)
	{
	for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS && FloatMap[Ct]; Ct ++);	//lint !e722
	if (Ct < WCS_RASTER_MAX_BANDS)
		{
		if (BufNode->Buffer = AllocFloatBand(Ct))
			{
			BufNode->Index = Ct;
			return (BufNode->Buffer);
			} // if
		} // if
	} // if
else if (BufNode->Type == WCS_RASTER_BANDSET_SHORT)
	{
	for (Ct = 0; Ct < WCS_RASTER_MAX_BANDS && ShortMap[Ct]; Ct ++);	//lint !e722
	if (Ct < WCS_RASTER_MAX_BANDS)
		{
		if (BufNode->Buffer = AllocShortBand(Ct))
			{
			BufNode->Index = Ct;
			return (BufNode->Buffer);
			} // if
		} // if
	} // if

return (NULL);

} // Raster::AllocBuffer

/*===========================================================================*/

UBYTE *Raster::AllocByteBand(char Band)
{

ByteBandSize = Rows * Cols;

return (ByteMap[Band] = (UBYTE *)AppMem_Alloc(ByteBandSize, 0));

} // Raster::AllocByteBand

/*===========================================================================*/

UBYTE *Raster::ClearByteBand(char Band)
{

if (ByteMap[Band])
	memset(ByteMap[Band], 0, ByteBandSize);
return (ByteMap[Band]);

} // Raster::ClearByteBand

/*===========================================================================*/

void Raster::FreeByteBand(char Band)
{

if (ByteMap[Band])
	AppMem_Free(ByteMap[Band], ByteBandSize);
ByteMap[Band] = NULL;

} // Raster::FreeByteBand

/*===========================================================================*/

UBYTE *Raster::AllocAltByteBand(long NewRows, long NewCols)
{

AltByteBandSize = NewRows * NewCols;

return (AltByteMap = (UBYTE *)AppMem_Alloc(AltByteBandSize, 0));

} // Raster::AllocAltByteBand

/*===========================================================================*/

UBYTE *Raster::ClearAltByteBand(void)
{

if (AltByteMap)
	memset(AltByteMap, 0, AltByteBandSize);
return (AltByteMap);

} // Raster::ClearAltByteBand

/*===========================================================================*/

void Raster::FreeAltByteBand(void)
{

if (AltByteMap)
	AppMem_Free(AltByteMap, AltByteBandSize);
AltByteMap = NULL;

} // Raster::FreeAltByteBand

/*===========================================================================*/

float *Raster::AllocFloatBand(char Band)
{

FloatBandSize = Rows * Cols * sizeof (float);

return (FloatMap[Band] = (float *)AppMem_Alloc(FloatBandSize, 0));

} // Raster::AllocFloatBand

/*===========================================================================*/

float *Raster::ClearFloatBand(char Band)
{

if (FloatMap[Band])
	memset(FloatMap[Band], 0, FloatBandSize);
return (FloatMap[Band]);

} // Raster::ClearFloatBand

/*===========================================================================*/

float *Raster::ClearFloatBandValue(char Band, float Value)
{
long MaxZip, Ct;

MaxZip = Rows * Cols;

if (FloatMap[Band])
	{
	for (Ct = 0; Ct < MaxZip; Ct ++)
		FloatMap[Band][Ct] = Value;
	} // for

return (FloatMap[Band]);

} // Raster::ClearFloatBandValue

/*===========================================================================*/

void Raster::FreeFloatBand(char Band)
{

if (FloatMap[Band])
	AppMem_Free(FloatMap[Band], FloatBandSize);
FloatMap[Band] = NULL;

} // Raster::FreeFloatBand

/*===========================================================================*/

float *Raster::AllocAltFloatBand(long NewRows, long NewCols)
{

AltFloatBandSize = NewRows * NewCols * sizeof (float);

return (AltFloatMap = (float *)AppMem_Alloc(AltFloatBandSize, 0));

} // Raster::AllocAltFloatBand

/*===========================================================================*/

float *Raster::ClearAltFloatBand(void)
{

if (AltFloatMap)
	memset(AltFloatMap, 0, AltFloatBandSize);
return (AltFloatMap);

} // Raster::ClearAltFloatBand

/*===========================================================================*/

void Raster::FreeAltFloatBand(void)
{

if (AltFloatMap)
	AppMem_Free(AltFloatMap, AltFloatBandSize);
AltFloatMap = NULL;

} // Raster::FreeAltFloatBand

/*===========================================================================*/

float *Raster::AllocRowFloatArray(char Band)
{

return (RowFloatArray[Band] = (float *)AppMem_Alloc(Rows * sizeof (float), 0));

} // Raster::AllocRowFloatArray

/*===========================================================================*/

void Raster::FreeRowFloatArray(char Band)
{

if (RowFloatArray[Band])
	AppMem_Free(RowFloatArray[Band], Rows * sizeof (float));
RowFloatArray[Band] = NULL;

} // Raster::FreeRowFloatArray

/*===========================================================================*/

unsigned short *Raster::AllocShortBand(char Band)
{

ShortBandSize = Rows * Cols * sizeof (unsigned short);

return (ShortMap[Band] = (unsigned short *)AppMem_Alloc(ShortBandSize, 0));

} // Raster::AllocShortBand

/*===========================================================================*/

unsigned short *Raster::ClearShortBand(char Band)
{

if (ShortMap[Band])
	memset(ShortMap[Band], 0, ShortBandSize);
return (ShortMap[Band]);

} // Raster::ClearFloatBand

/*===========================================================================*/

void Raster::FreeShortBand(char Band)
{

if (ShortMap[Band])
	AppMem_Free(ShortMap[Band], ShortBandSize);
ShortMap[Band] = NULL;

} // Raster::FreeShortBand

/*===========================================================================*/

// this assumes the image is already loaded so the correct size is known
// also the rgb bands are not created here - done when image is loaded
int Raster::AllocFoliageBands(void)
{
int NewFloatBand = 0;

if (Rows > 0 && Cols > 0)
	{
	if (AllocAltByteBand(Rows, Cols))
		{
		ClearAltByteBand();
		// AltFloatBand may have already been created from an alpha channel loaded from file
		if (AltFloatMap || (NewFloatBand = (int)AllocAltFloatBand(Rows, Cols)))
			{
			if (NewFloatBand)
				ClearAltFloatBand();
			if (AllocRowFloatArray(WCS_RASTER_FOLIAGE_ARRAY_MIDPT))
				{
				if (AllocRowFloatArray(WCS_RASTER_FOLIAGE_ARRAY_SPAN))
					{
					return (1);
					} // if
				} // if
			} // if
		} // if
	} // if

return (0);

} // Raster::AllocFoliageBands

/*===========================================================================*/

int Raster::FoliageBandsValid(void)
{

return (AltByteMap && AltFloatMap && RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN]);

} // Raster::FoliageBandsValid

/*===========================================================================*/

int Raster::AllocRender3DBands(void)
{
BufferNode *CurBuf;
	
// we need RGB, AA, Z, Refl, Normal

if (CurBuf = Buffers = new BufferNode("ZBUF", WCS_RASTER_BANDSET_FLOAT))
	{
	// we know we aren't creating duplicate nodes since these are the first to be created
	// so we'll just call AddBufferNode on the last node created to save a bit of string comparing
	if (! (CurBuf = CurBuf->AddBufferNode("RED", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("REFLECTION", WCS_RASTER_BANDSET_FLOAT)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL X", WCS_RASTER_BANDSET_FLOAT)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL Y", WCS_RASTER_BANDSET_FLOAT)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("SURFACE NORMAL Z", WCS_RASTER_BANDSET_FLOAT)))
		return (0);
	if (! (CurBuf = CurBuf->AddBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT)))
		return (0);

	CurBuf = Buffers;
	while (CurBuf)
		{
		if (CurBuf->Name[0])
			{
			if (! AllocBuffer(CurBuf))
				return (0);
			} // if
		CurBuf = CurBuf->Next;
		} // while

	CurBuf = Buffers;
	while (CurBuf)
		{
		if (CurBuf->Buffer)
			{
			if (CurBuf->Type == WCS_RASTER_BANDSET_BYTE)
				ClearByteBand(CurBuf->Index);
			else if (CurBuf->Type == WCS_RASTER_BANDSET_SHORT)
				ClearShortBand(CurBuf->Index);
			else if (CurBuf->Type == WCS_RASTER_BANDSET_FLOAT)
				{
				if (! stricmp(CurBuf->Name, "ZBUF"))
					ClearFloatBandValue(CurBuf->Index, FLT_MAX);
				else
					ClearFloatBand(CurBuf->Index);
				} // else if
			} // if
		CurBuf = CurBuf->Next;
		} // while

	} // if

Red = (unsigned char *)			Buffers->FindBuffer("RED",				WCS_RASTER_BANDSET_BYTE);
Green = (unsigned char *)		Buffers->FindBuffer("GREEN",			WCS_RASTER_BANDSET_BYTE);
Blue = (unsigned char *)		Buffers->FindBuffer("BLUE",				WCS_RASTER_BANDSET_BYTE);
AABuf = (unsigned char *)		Buffers->FindBuffer("ANTIALIAS",		WCS_RASTER_BANDSET_BYTE);
ExponentBuf = (unsigned short *)Buffers->FindBuffer("RGB EXPONENT",		WCS_RASTER_BANDSET_SHORT);
NormalBuf[0] = (float *)		Buffers->FindBuffer("SURFACE NORMAL X",	WCS_RASTER_BANDSET_FLOAT);
NormalBuf[1] = (float *)		Buffers->FindBuffer("SURFACE NORMAL Y",	WCS_RASTER_BANDSET_FLOAT);
NormalBuf[2] = (float *)		Buffers->FindBuffer("SURFACE NORMAL Z",	WCS_RASTER_BANDSET_FLOAT);
ZBuf = (float *)				Buffers->FindBuffer("ZBUF",				WCS_RASTER_BANDSET_FLOAT);
ReflectionBuf = (float *)		Buffers->FindBuffer("REFLECTION",		WCS_RASTER_BANDSET_FLOAT);

if (Red && Green && Blue && AABuf && ExponentBuf && NormalBuf[0] && NormalBuf[1] && NormalBuf[2] && ZBuf && ReflectionBuf)
	return (1);

/*
if (Rows > 0 && Cols > 0)
	{
	if (Red = AllocByteBand(WCS_RASTER_RENDER_BAND_RED))
		{
		ClearByteBand(WCS_RASTER_RENDER_BAND_RED);
		if (Green = AllocByteBand(WCS_RASTER_RENDER_BAND_GREEN))
			{
			ClearByteBand(WCS_RASTER_RENDER_BAND_GREEN);
			if (Blue = AllocByteBand(WCS_RASTER_RENDER_BAND_BLUE))
				{
				ClearByteBand(WCS_RASTER_RENDER_BAND_BLUE);
				if (AllocFloatBand(WCS_RASTER_RENDER_BAND_ZBUF))
					{
					ClearFloatBandValue(WCS_RASTER_RENDER_BAND_ZBUF, FLT_MAX);
					if (AllocAltFloatBand(Rows, Cols))
						{
						ClearAltFloatBand();
						if (RowZip = new long[Rows])
							{
							for (Row = 0, Cell = 0; Row < Rows; Row ++, Cell += Cols)
								RowZip[Row] = Cell;
							return (1);
							} // if
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if
*/
return (0);

} // Raster::AllocRender3DBands

/*===========================================================================*/

void Raster::ClearRender3DBands(void)
{

ClearByteBand(WCS_RASTER_RENDER_BAND_RED);
ClearByteBand(WCS_RASTER_RENDER_BAND_GREEN);
ClearByteBand(WCS_RASTER_RENDER_BAND_BLUE);
ClearAltFloatBand();
ClearFloatBandValue(WCS_RASTER_RENDER_BAND_ZBUF, FLT_MAX);
ClearPixelFragMap();

} // Raster::ClearRender3DBands

/*===========================================================================*/

int Raster::AllocRGBBands(void)
{

if (Rows > 0 && Cols > 0)
	{
	if (Red = AllocByteBand(WCS_RASTER_RENDER_BAND_RED))
		{
		ClearByteBand(WCS_RASTER_RENDER_BAND_RED);
		if (Green = AllocByteBand(WCS_RASTER_RENDER_BAND_GREEN))
			{
			ClearByteBand(WCS_RASTER_RENDER_BAND_GREEN);
			if (Blue = AllocByteBand(WCS_RASTER_RENDER_BAND_BLUE))
				{
				ClearByteBand(WCS_RASTER_RENDER_BAND_BLUE);
				if (RowZip = new long[Rows])
					{
					for (Row = 0, RasterCell = 0; Row < Rows; Row ++, RasterCell += Cols)
						RowZip[Row] = RasterCell;
					return (1);
					} // if
				} // if
			} // if
		} // if
	} // if

return (0);

} // Raster::AllocRGBBands

/*===========================================================================*/

int Raster::AllocShadow3DBands(void)
{

if (Rows > 0 && Cols > 0)
	{
	if (Red = AllocByteBand(WCS_RASTER_SHADOWMAP_BAND_NEAROPACITY))
		{
		ClearByteBand(WCS_RASTER_SHADOWMAP_BAND_NEAROPACITY);
		if (Green = AllocByteBand(WCS_RASTER_SHADOWMAP_BAND_FAROPACITY))
			{
			ClearByteBand(WCS_RASTER_SHADOWMAP_BAND_FAROPACITY);
			if (AllocFloatBand(WCS_RASTER_SHADOWMAP_BAND_ZBUF))
				{
				ClearFloatBand(WCS_RASTER_SHADOWMAP_BAND_ZBUF);
				if (AllocFloatBand(WCS_RASTER_SHADOWMAP_BAND_DEPTH))
					{
					ClearFloatBand(WCS_RASTER_SHADOWMAP_BAND_DEPTH);
					if (RowZip = new long[Rows])
						{
						for (Row = 0, RasterCell = 0; Row < Rows; Row ++, RasterCell += Cols)
							RowZip[Row] = RasterCell;
						return (1);
						} // if
					} // if
				} // if
			} // if
		} // if
	} // if

return (0);

} // Raster::AllocShadow3DBands

/*===========================================================================*/

long *Raster::AllocRowZip(void)
{

if (RowZip = new long[Rows])
	{
	for (Row = 0, RasterCell = 0; Row < Rows; Row ++, RasterCell += Cols)
		RowZip[Row] = RasterCell;
	} // if

return (RowZip);

} // Raster::AllocRowZip

/*===========================================================================*/

Thumbnail *Raster::AllocThumbnail(void)
{

if (! Thumb)
	Thumb = new Thumbnail();
if (Thumb)
	Thumb->AllocTNails();

return (Thumb);

} // Raster::AllocThumbnail

/*===========================================================================*/

void Raster::ClearThumbnail(void)
{

if (Thumb)
	Thumb->ClearTNails();

} // Raster::ClearThumbnail

/*===========================================================================*/

VectorPolygonList **Raster::AllocPolyListMap(void)
{

if (Rows > 0 && Cols > 0)
	{
	PolyListBlock = new VectorPolygonList *[Rows * Cols];
	ClearPolyListMap();
	} // if
return (PolyListBlock);

} // Raster::AllocPolyListMap

/*===========================================================================*/

void Raster::FreePolyListMap(void)
{

if (PolyListBlock)
	delete [] PolyListBlock;
PolyListBlock = NULL;

} // Raster::FreePolyListMap

/*===========================================================================*/

VectorPolygonList **Raster::ClearPolyListMap(void)
{

if (PolyListBlock)
	memset(PolyListBlock, 0, Rows * Cols * sizeof (VectorPolygonList *));

return (PolyListBlock);

} // Raster::ClearPolyListMap

/*===========================================================================*/

RasterAttribute *Raster::AddAttribute(char AddType, RasterShell *AddShell, RasterAnimHost *HostSource)
{
RasterAttribute *MyAttr, *NewAttr = NULL;
NotifyTag Changes[2];

if (MyAttr = Attr)
	{
	while (MyAttr->Next)
		{
		MyAttr = MyAttr->Next;
		} // while
	NewAttr = MyAttr->Next = new RasterAttribute(AddShell, AddType, this, HostSource);
	} // if
else
	{
	NewAttr = Attr = new RasterAttribute(AddShell, AddType, this, HostSource);
	} // else

if (NewAttr)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (NewAttr);

} // Raster::AddAttribute

/*===========================================================================*/

RasterAttribute *Raster::AddAttribute(void)
{
RasterAttribute *MyAttr, *NewAttr = NULL;
NotifyTag Changes[2];

if (MyAttr = Attr)
	{
	while (MyAttr->Next)
		{
		MyAttr = MyAttr->Next;
		} // while
	NewAttr = MyAttr->Next = new RasterAttribute;
	} // if
else
	{
	NewAttr = Attr = new RasterAttribute;
	} // else

if (NewAttr)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (NewAttr);

} // Raster::AddAttribute

/*===========================================================================*/

RasterShell *Raster::MatchRasterShell(unsigned long MatchID)
{
RasterAttribute *MyAttr;

if (MyAttr = Attr)
	{
	while (MyAttr)
		{
		if (MyAttr->Shell && MyAttr->Shell->GetID() == MatchID)
			{
			return (MyAttr->Shell);
			} // if
		MyAttr = MyAttr->Next;
		} // while
	} // if
return (NULL);

} // Raster::MatchRasterShell

/*===========================================================================*/

RasterAttribute *Raster::MatchRasterShell(RasterShell *MatchShell, int SearchDeep)
{
RasterAttribute *MyAttr;
RasterShell *MyShell, *TestShell;

if (MyAttr = Attr)
	{
	while (MyAttr)
		{
		if ((MyShell = MyAttr->GetShell()) == MatchShell)
			{
			return (MyAttr);
			} // if
		else if (SearchDeep)
			{
			if (MyShell && MyShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
				{
				if (TestShell = ((SequenceShell *)MyShell)->NextSeq)
					{
					do
						{
						if (TestShell == MatchShell)
							return (MyAttr);
						} while (TestShell = ((SequenceShell *)TestShell)->NextSeq);
					} // if
				} // else if
			else if (MyShell && MyShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
				{
				if (TestShell = ((DissolveShell *)MyShell)->NextDis)
					{
					do
						{
						if (TestShell == MatchShell)
							return (MyAttr);
						} while (TestShell = ((DissolveShell *)TestShell)->NextDis);
					} // if
				} // else if
			} // else if
		MyAttr = MyAttr->Next;
		} // while
	} // if
return (NULL);

} // Raster::MatchRasterShell

/*===========================================================================*/

RasterAttribute *Raster::MatchAttribute(char MatchType)
{
RasterAttribute *MyAttr;

if (MyAttr = Attr)
	{
	while (MyAttr)
		{
		if (MyAttr->Shell && MyAttr->Shell->GetType() == MatchType)
			{
			return (MyAttr);
			} // if
		MyAttr = MyAttr->Next;
		} // while
	} // if
return (NULL);

} // Raster::MatchAttribute

/*===========================================================================*/

RasterAttribute *Raster::MatchNextAttribute(char MatchType, RasterAttribute *StartHere)
{
RasterAttribute *MyAttr;

if (MyAttr = StartHere)
	{
	MyAttr = StartHere->Next;
	while (MyAttr)
		{
		if (MyAttr->Shell && MyAttr->Shell->GetType() == MatchType)
			{
			return (MyAttr);
			} // if
		MyAttr = MyAttr->Next;
		} // while
	return (NULL);
	} // if
else
	return (MatchAttribute(MatchType));

} // Raster::MatchAttribute

/*===========================================================================*/

int Raster::MatchPathAndName(char *FindPath, char *FindName)
{

return ((! stricmp(FindPath, PAF.GetPath())) && (! stricmp(FindName, PAF.GetName())));

} // Raster::MatchPathAndName

/*===========================================================================*/

int Raster::RemoveAttribute(char RemoveShellType, int RemoveAll)
{
RasterAttribute *MyAttr, *PrevAttr = NULL;
int RemoveMore = 1, RemovedItems = 0;
NotifyTag Changes[2];

while (RemoveMore)
	{
	RemoveMore = 0;
	if (MyAttr = Attr)
		{
		while (MyAttr && MyAttr->GetShell() && MyAttr->GetShell()->GetType() != RemoveShellType)
			{
			PrevAttr = MyAttr;
			MyAttr = MyAttr->Next;
			} // while
		if (MyAttr)
			{
			if (PrevAttr)
				PrevAttr->Next = MyAttr->Next;
			else
				Attr = MyAttr->Next;
			delete MyAttr;
			RemovedItems ++;
			if (RemoveAll)
				RemoveMore = 1;
			} // if
		} // if
	} // while

if (RemovedItems)
	{
	if (! MatchAttribute(RemoveShellType))
		{
		switch (RemoveShellType)
			{
			case WCS_RASTERSHELL_TYPE_SEQUENCE:
				{
				SequenceEnabled = 0;
				break;
				} // 
			case WCS_RASTERSHELL_TYPE_DISSOLVE:
				{
				DissolveEnabled = 0;
				break;
				} // 
			case WCS_RASTERSHELL_TYPE_COLORCONTROL:
				{
				ColorControlEnabled = 0;
				break;
				} // 
			case WCS_RASTERSHELL_TYPE_IMAGEMANAGER:
				{
				ImageManagerEnabled = 0;
				break;
				} // 
			} // switch
		} // if
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if
return (RemovedItems);

} // Raster::RemoveAttribute

/*===========================================================================*/

int Raster::RemoveAttribute(RasterShell *RemoveShell)
{
RasterAttribute *MyAttr, *PrevAttr = NULL;
NotifyTag Changes[2];

if (MyAttr = Attr)
	{
	while (MyAttr && MyAttr->GetShell() != RemoveShell)
		{
		PrevAttr = MyAttr;
		MyAttr = MyAttr->Next;
		} // while
	if (MyAttr)
		{
		if (PrevAttr)
			PrevAttr->Next = MyAttr->Next;
		else
			Attr = MyAttr->Next;
		delete MyAttr;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		return (1);
		} // if
	} // if

return (0);

} // Raster::RemoveAttribute

/*===========================================================================*/

int Raster::RemoveAttribute(RasterAttribute *RemoveAttr)
{
RasterAttribute *MyAttr, *PrevAttr = NULL;
NotifyTag Changes[2];

if (MyAttr = Attr)
	{
	while (MyAttr && MyAttr != RemoveAttr)
		{
		PrevAttr = MyAttr;
		MyAttr = MyAttr->Next;
		} // while
	if (MyAttr)
		{
		if (PrevAttr)
			PrevAttr->Next = MyAttr->Next;
		else
			Attr = MyAttr->Next;
		delete MyAttr;
		Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
		return (1);
		} // if
	} // if

return (0);

} // Raster::RemoveAttribute

/*===========================================================================*/

char *Raster::SetDefaultName(void)
{
char NameBase[WCS_IMAGE_MAXNAMELENGTH];

strncpy(NameBase, PAF.GetName(), WCS_IMAGE_MAXNAMELENGTH);
NameBase[WCS_IMAGE_MAXNAMELENGTH - 1] = 0;
SetUniqueName(GlobalApp->LoadToImageLib, NameBase);
return (Name);

} // Raster::SetDefaultName

/*===========================================================================*/

char *Raster::SetUserName(char *NewName)
{
char NameBase[WCS_IMAGE_MAXNAMELENGTH];

strncpy(NameBase, NewName, WCS_IMAGE_MAXNAMELENGTH);
NameBase[WCS_IMAGE_MAXNAMELENGTH - 1] = 0;
SetUniqueName(GlobalApp->LoadToImageLib, NameBase);
return (Name);

} // Raster::SetUserName

/*===========================================================================*/

void Raster::Edit(void)
{

GlobalApp->AppImages->SetActive(this);
GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_ITEM_ILG, 0);

} // Raster::Edit

/*===========================================================================*/

unsigned long Raster::GetRAFlags(unsigned long FMask)
{
unsigned long Flags = 0;

if (FMask & WCS_RAHOST_FLAGBIT_DELETABLE)
	{
	if (! RAParent || RAParent->GetDeletable(this))
		Flags |= WCS_RAHOST_FLAGBIT_DELETABLE;
	} // if
if (FMask & WCS_RAHOST_FLAGBIT_ENABLED)
	{
	if (GetRAEnabled())
		Flags |= WCS_RAHOST_FLAGBIT_ENABLED;
	} // if
if (FMask & WCS_RAHOST_FLAGBIT_EXPANDED)
	{
	GetExpansionFlags(FMask, Flags);
	} // if
if (FMask & WCS_RAHOST_FLAGBIT_ANIMATED)
	{
	if (GetRAHostAnimated())
		Flags |= WCS_RAHOST_FLAGBIT_ANIMATED;
	} // if
if (FMask & WCS_RAHOST_FLAGBIT_EDITNAME)
	{
	if (! RAParent)
		Flags |= WCS_RAHOST_FLAGBIT_EDITNAME;
	} // if
if (FMask & WCS_RAHOST_FLAGBIT_CHILDREN)
	{
	if (MatchAttribute(WCS_RASTERSHELL_TYPE_RASTER) || MatchAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE) || MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
		Flags |= WCS_RAHOST_FLAGBIT_CHILDREN;
	} // if

FMask &= (WCS_RAHOST_ICONTYPE_RASTER | WCS_RAHOST_FLAGBIT_DRAGGABLE |
	WCS_RAHOST_FLAGBIT_DRAGTARGET | Flags);

return (FMask);

} // Raster::GetRAFlags

/*===========================================================================*/

void Raster::GetRAHostProperties(RasterAnimHostProperties *Prop)
{

if (Prop->PropMask & WCS_RAHOST_MASKBIT_FLAGS)
	{
	Prop->Flags = GetRAFlags(Prop->FlagsMask);
	} // if
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	Prop->Name = GetUserName();
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

} // Raster::GetRAHostProperties

/*===========================================================================*/

int Raster::SetRAHostProperties(RasterAnimHostProperties *Prop)
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
if (Prop->PropMask & WCS_RAHOST_MASKBIT_NAME)
	{
	SetUserName(Prop->Name);	// this creates a unique name
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

return (Success);

} // Raster::SetRAHostProperties

/*===========================================================================*/

char Raster::GetRAHostDropOK(long DropType)
{

if (DropType == GetRAHostTypeNumber())
	return (1);
if (DropType == WCS_EFFECTSSUBCLASS_COORDSYS && MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
	return (1);

return (0);

} // Raster::GetRAHostDropOK

/*===========================================================================*/

RasterAnimHost *Raster::GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter)
{
RasterAttribute *MyAttr;
char Found = 0;

if (! Current)
	Found = 1;
if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE))
	{
	if (MyAttr->GetShell() && MyAttr->GetShell()->Host)
		{
		if (Found)
			return (MyAttr->GetShell()->Host);
		if (Current == MyAttr->GetShell()->Host)
			Found = 1;
		} // if
	while (MyAttr = MatchNextAttribute(WCS_RASTERSHELL_TYPE_FOLIAGE, MyAttr))
		{
		if (MyAttr->GetShell() && MyAttr->GetShell()->Host)
			{
			if (Found)
				return (MyAttr->GetShell()->Host);
			if (Current == MyAttr->GetShell()->Host)
				Found = 1;
			} // if
		} // while
	} // if
if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_RASTER))
	{
	if (MyAttr->GetShell() && MyAttr->GetShell()->Host)
		{
		if (Found)
			return (MyAttr->GetShell()->Host);
		if (Current == MyAttr->GetShell()->Host)
			Found = 1;
		} // if
	while (MyAttr = MatchNextAttribute(WCS_RASTERSHELL_TYPE_RASTER, MyAttr))
		{
		if (MyAttr->GetShell() && MyAttr->GetShell()->Host)
			{
			if (Found)
				return (MyAttr->GetShell()->Host);
			if (Current == MyAttr->GetShell()->Host)
				Found = 1;
			} // if
		} // while
	} // for
if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
	{
	if (Found)
		return (&((GeoRefShell *)MyAttr->GetShell())->GeoReg);
	if (Current == &((GeoRefShell *)MyAttr->GetShell())->GeoReg)
		Found = 1;
	if (MyAttr->GetShell())
		{
		if (MyAttr->GetShell()->Host)
			{
			if (Found)
				return (MyAttr->GetShell()->Host);
			if (Current == MyAttr->GetShell()->Host)
				Found = 1;
			} // if
		} // if
	} // if

return (NULL);

} // Raster::GetRAHostChild

/*===========================================================================*/

int Raster::RemoveRAHost(RasterAnimHost *RemoveMe)
{
RasterAttribute *MyAttr = Attr;
NotifyTag Changes[2];

while (MyAttr)
	{
	if (MyAttr->GetShell() && MyAttr->GetShell()->Host && MyAttr->GetShell()->Host == RemoveMe)
		{
		if (MyAttr->GetShell()->GetType() == WCS_RASTERSHELL_TYPE_GEOREF)
			{
			MyAttr->GetShell()->Host->RemoveRaster(MyAttr->GetShell());
			MyAttr->GetShell()->SetHost(NULL);
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			return (1);
			} // if
		return (RemoveAttribute(MyAttr));
		} // if
	MyAttr = MyAttr->Next;
	} // while

return (0);

} // Raster::RemoveRAHost

/*===========================================================================*/

int Raster::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource)
{
char QueryStr[256], NameStr[128];
int Success = 0;
RasterAnimHostProperties Prop;
NotifyTag Changes[2];

Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE;
GetRAHostProperties(&Prop);
sprintf(NameStr, "%s %s", Prop.Name, Prop.Type);
if (DropSource->TypeNumber == GetRAHostTypeNumber())
	{
	Success = -1;
	if (this != (Raster *)DropSource->DropSource)
		{
		sprintf(QueryStr, "Copy %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
		if (UserMessageOKCAN(NameStr, QueryStr))
			{
			Copy(this, (Raster *)DropSource->DropSource, 0);
			SetUserName(Name);	// this creates a unique name
			Success = 1;
			Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
			} // if
		} // if
	} // if
else if (DropSource->TypeNumber == WCS_EFFECTSSUBCLASS_COORDSYS)
	{
	Success = -1;
	sprintf(QueryStr, "Attach %s %s to %s?", DropSource->Name, DropSource->Type, NameStr);
	if (UserMessageOKCAN(NameStr, QueryStr))
		{
		// calls SetHostNotify() on GeoRefShell if one is found on this
		Success = (int)((CoordSys *)DropSource->DropSource)->AddRaster(this);
		} // if
	} // else if

return (Success);

} // Raster::ProcessRAHostDragDrop

/*===========================================================================*/

char *Raster::SetUniqueName(ImageLib *Library, char *NameBase)
{
char NewName[WCS_IMAGE_MAXNAMELENGTH], FoundMatch = 1, Ct;

if (NameBase)
	{
	if (Library->FindByUserName(NameBase))
		{
		StripExtension(NameBase);
		if (strlen(NameBase) > WCS_IMAGE_MAXNAMELENGTH - 3)
			NameBase[WCS_IMAGE_MAXNAMELENGTH - 3] = 0;
		for (Ct = 0; Ct < 100 && FoundMatch; Ct ++)
			{
			FoundMatch = 0;
			if (Ct)
				sprintf(NewName, "%s.%d", NameBase, Ct);
			else
				sprintf(NewName, "%s", NameBase);
			if (Library->FindByUserName(NewName))
				FoundMatch = 1;
			} // for
		strcpy(Name, NewName);
		} // if
	else
		strcpy(Name, NameBase);
	} // if

return (Name);

} // Raster::SetUniqueName

/*===========================================================================*/

int Raster::TestAllInitFlags(unsigned long TestMe)
{

if (TestMe & WCS_RASTER_INITFLAGS_IMAGELOADED)
	{
	if (! TestInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED))
		return (0);
	} // if
if (TestMe & WCS_RASTER_INITFLAGS_FOLIAGELOADED)
	{
	if (! TestInitFlags(WCS_RASTER_INITFLAGS_FOLIAGELOADED))
		return (0);
	} // if
if (TestMe & WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED)
	{
	if (! TestInitFlags(WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED))
		return (0);
	} // if
if (TestMe & WCS_RASTER_INITFLAGS_DOWNSAMPLED)
	{
	if (! TestInitFlags(WCS_RASTER_INITFLAGS_DOWNSAMPLED))
		return (0);
	} // if

return (1);

} // Raster::TestAllInitFlags

/*===========================================================================*/

int Raster::LoadToComposite(double RenderTime, double FrameRate, PostProcComposite *Compositer, long OutputCols, long OutputRows)
{
RLASampleData Samp;
long X, Y, PixZip, Success = 1;
char TZR[512], TZRLastTemp[512], NoZ = 0;
RasterAttribute *MySequence;
Raster TempZRaster;

// If LoadCompositer is NULLed when we return from LoadToRender, it means
// that the loader was fragment-aware and processed the compositing via callback
// as it loaded.
// If it is left intact, it means that the loader was unaware and regular flat image/Z
// compositing now needs to be performed with the image we loaded.
LoadCompositer = Compositer; // So LoadToRender and its descendents can find and utilize it


// I doubt this loader will do what you want it to but here is the call for loading a normal image for what it is worth
if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED))
	{
	return (0);
	} // if

if (LoadCompositer)
	{ // last loader was not fragment-aware, must search for a corresponding Z file
	TZR[0] = NULL;

	if ((MySequence = MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE)) && SequenceEnabled)
		{
		double TempOffset = 0.0;
		//((SequenceShell *)MySequence->GetShell())->GetOpenFileNames(FirstSequenceName, SecondSequenceName, SequenceOffset, RenderTime * FrameRate);
		((SequenceShell *)MySequence->GetShell())->GetOpenFileNames(TZR, TZRLastTemp, TempOffset, RenderTime * FrameRate);
		} // if
	else
		{ // not a sequence, odd but possible
		PAF.GetPathAndName(TZR);
		} // else
	if (TZR[0])
		{
		// truncate any extension, add .zb
		StripExtension(TZR);
		AddExtension(TZR, "zb");
		if (!TempZRaster.LoadIFFZBUF(TZR, 1))
			{
			sprintf(TZRLastTemp, "Unable to locate corresponding ZB file: %s", TZR);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, TZRLastTemp);
			NoZ = 1;
			} // if
		} // if
	else
		{
		sprintf(TZRLastTemp, "Unable to locate corresponding ZB file.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, TZRLastTemp);
		NoZ = 1;
		} // else
	} // if

Samp.ImageWidth = Cols;
Samp.ImageHeight = Rows;

// check and see if composite-via-callback was not performed above
// if LoadCompositer==NULL then we took care of it during load, above
if (LoadCompositer && Red && Green && Blue)
	{
	// temporary test loop
	for (Y = PixZip = 0; Y < Rows; Y ++)
		{
		for (X = 0; X < Cols; X ++, PixZip ++)
			{
			Samp.X = X;
			Samp.Y = Y;
			Samp.Z = 0.0f;
			if (!NoZ)
				{
				unsigned long Zip;
				Zip = (Y * TempZRaster.Cols) + X;
				if (Zip < (unsigned long int)(TempZRaster.Cols * TempZRaster.Rows))
					{
					Samp.Z = TempZRaster.FloatMap[0][Zip];
					} // if
				} // if
			Samp.Channels = (BMM_CHAN_Z);
			Samp.Alpha = 255;
			if (AlphaAvailable && AlphaEnabled && AltFloatMap)
				{
				Samp.Alpha = (unsigned char)(AltFloatMap[PixZip] * 255.0);
				} // if
			Samp.color[0] = Red[PixZip];
			Samp.color[1] = Green[PixZip];
			Samp.color[2] = Blue[PixZip];
			if (! Compositer->EvalOneRLASample(&Samp))
				{
				Success = 0;
				break;
				} // if
			} // for
		} // for
	} // for

// close image file ?

// Set this back to NULL so no one tries using it as a callback later
LoadCompositer = NULL;

return (Success);

} // Raster::LoadToComposite

/*===========================================================================*/

int Raster::LoadToRender(double RenderTime, double FrameRate, unsigned long LocalInitFlags)
{
double DissolveOffset = 0.0, SequenceOffset = 0.0;
RasterAttribute *MyDissolve, *MySequence;
Raster *Host = NULL, *Guest = NULL, *Temp = NULL;
unsigned long IDStash;
int Success = 1, SaveFast = 0;
char FirstSequenceName[512], SecondSequenceName[512];

// check flags to see if it is already loaded

// re-init image ID's which may have been mangled
IDStash = ID;

if (LocalInitFlags & WCS_RASTER_INITFLAGS_FORCERELOAD)
	FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
else if (TestAllInitFlags(LocalInitFlags))
	return (1);

if (! TestInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED))
	{
	if (! GetPreppedStatus())
		{
		if (! LoadnPrepImage(FALSE, FALSE))	// we may need the image size and number of colors
			return (0);
		} // if

	// don't need to free bitmaps - that will be done if a new image is loaded

	SaveFast = GetLoadFast();
	// check if dissolves exist
	if ((MyDissolve = MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)) && DissolveEnabled && !LoadCompositer)
		{
		SaveFast = 0;
		// where are we in the dissolve chain
		((DissolveShell *)MyDissolve->GetShell())->GetEndMembers(Host, Guest, DissolveOffset, RenderTime * FrameRate);
		if (Guest)
			{
			if (! Guest->LoadToRender(RenderTime, FrameRate, LocalInitFlags & ~(WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED | WCS_RASTER_INITFLAGS_DOWNSAMPLED)))
				Success = 0;
			} // if
		if (Host != this)
			{
			if (! Host->LoadToRender(RenderTime, FrameRate, LocalInitFlags & ~(WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED | WCS_RASTER_INITFLAGS_DOWNSAMPLED)))
				Success = 0;
			} // if
		} // if
	if (Success && (! Host || Host == this))
		{
		if ((MySequence = MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE)) && SequenceEnabled)
			{
			SaveFast = 0;
			((SequenceShell *)MySequence->GetShell())->GetOpenFileNames(FirstSequenceName, SecondSequenceName, SequenceOffset, RenderTime * FrameRate);
			if (LoadCompositer)
				{ // disable tweening when compositing
				SequenceOffset = 0.0;
				} // if
			if (FirstSequenceName[0])
				{
				if (! LoadnProcessImage(0, FirstSequenceName))
					Success = 0;
				if (Success && SecondSequenceName[0] && SequenceOffset > 0.0 && SequenceOffset < 1.0)
					{
					if (Temp = new Raster)
						{
						Copy(Temp, this, FALSE);
						if (Temp->LoadnProcessImage(0, SecondSequenceName))
							{
							// dissolve between sequence frames using SequenceOffset as the fraction
							if (! CopyBitmaps(Temp, SequenceOffset, LocalInitFlags & WCS_RASTER_INITFLAGS_FOLIAGELOADED))
								Success = 0;
							} // if
						else
							Success = 0;
						delete Temp;
						Temp = NULL;
						} // if
					else
						Success = 0;
					} // if
				} // if
			else
				{
				// no image so just allocate blank buffers
				Success = 0;
				} // else
			} // if sequence
		else
			{
			if (SaveFast)
				{
				if (! LoadFoliage() && ! LoadnProcessImage(0))
					Success = 0;
				} // if
			else if (! LoadnProcessImage(0))
				Success = 0;
			} // else
		} // if

	if (Success && Host && Guest)
		{
		// dissolve between using DissolveOffset as the fraction, host as the target
		if (! Host->CopyBitmaps(Guest, DissolveOffset, LocalInitFlags & WCS_RASTER_INITFLAGS_FOLIAGELOADED))
			Success = 0;
		} // if
	if (Success && Host && Host != this)
		{
		// copy from host to this
		FreeAllBands(WCS_RASTER_LONGEVITY_LONG);
		if (Success = (int)AllocByteBand(0))
			ClearByteBand(0);
		if (Success && ByteBands > 1 && GetIsColor())
			{
			if (Success = (int)AllocByteBand(1))
				ClearByteBand(1);
			} // if
		if (Success && ByteBands > 2 && GetIsColor())
			{
			if (Success = (int)AllocByteBand(2))
				ClearByteBand(2);
			} // if
		Red = ByteMap[0];
		Green = ByteMap[1];
		Blue = ByteMap[2];
		if (Success)
			{
			if (! CopyBitmaps(Host, 1.0, LocalInitFlags & WCS_RASTER_INITFLAGS_FOLIAGELOADED))
				Success = 0;
			} // if
		} // if
	if (! Success)	// if all else fails then just allocate some blank buffers - we don't like to fail completely!
		{
		FreeAllBands(WCS_RASTER_LONGEVITY_LONG);
		if (Success = (int)AllocByteBand(0))
			ClearByteBand(0);
		if (Success && ByteBands > 1 && GetIsColor())
			{
			if (Success = (int)AllocByteBand(1))
				ClearByteBand(1);
			} // if
		if (Success && ByteBands > 2 && GetIsColor())
			{
			if (Success = (int)AllocByteBand(2))
				ClearByteBand(2);
			} // if
		Red = ByteMap[0];
		Green = ByteMap[1];
		Blue = ByteMap[2];
		} // if
	if (Success)
		SetInitFlags(WCS_RASTER_INITFLAGS_IMAGELOADED);
	} // if

if (Success && (LocalInitFlags & WCS_RASTER_INITFLAGS_FOLIAGELOADED) && ! TestInitFlags(WCS_RASTER_INITFLAGS_FOLIAGELOADED))
	{
	// create foliage stuff
	Success = CreateFoliageBands();
	} // if foliage
if (Success && (LocalInitFlags & WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED) && ! TestInitFlags(WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED))
	{
	// <<<>>> downsample image
	if ((Success = CreateSmallerFoliage()) && SaveFast)
		{
		SaveFoliage();
		} // if
	} // if foliage
if (Success && (LocalInitFlags & WCS_RASTER_INITFLAGS_DOWNSAMPLED) && ! TestInitFlags(WCS_RASTER_INITFLAGS_DOWNSAMPLED))
	{
	// <<<>>> downsample image
	Success = CreateSmallerImage();
	} // if foliage

// re-init image ID which may have been mangled by LoadFast
ID = IDStash;

return (Success);

} // Raster::LoadToRender

/*===========================================================================*/

int Raster::LoadnPrepImage(int NameOnly, int UpdateCoordSys, char AllowImageManagement)
{
char filename[256];
short /* Width, Height, Planes, */ Retry = 1, LastEnabled;
/* UBYTE *TempAlpha = NULL; */
CoordSys *MatchSys;
RasterAttribute *GeoAttr;
NotifyTag Changes[2];
#ifdef WCS_IMAGE_MANAGEMENT
ImageCacheControl ICC;
int AlphaAvailStash = 0, AlphaEnableStash = 0;
#endif // WCS_IMAGE_MANAGEMENT

// discard old displaylist here
// whether we succeed or not, the old data is invalid

LastEnabled = Enabled;

if (ThumbDisplayListNum != ULONG_MAX)
	{
	glDeleteLists(ThumbDisplayListNum, 1);
	ThumbDisplayListNum = ULONG_MAX;
	} // if

while (Retry)
	{
	Retry = 0;
#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
	/*if*/ (PAF.GetValidPathAndName(filename));
#else // !WCS_BUILD_REMOTEFILE_SUPPORT
	if (PAF.GetValidPathAndName(filename))
#endif // !WCS_BUILD_REMOTEFILE_SUPPORT
		{
		FreeAllBands(WCS_RASTER_LONGEVITY_LONG);
		if (NameOnly)
			{
			if (! Name[0])
				SetDefaultName();
			return (1);
			} // if
		//if (LoadRasterImage(filename, 0, ByteMap, 0, 0, 0, &Width, &Height, &Planes, &TempAlpha))
		// we only want to load a thumbnail-sized image here, if the loader is
		// smart enough to do that
		TNRowsReq = TNColsReq = 100; // Note: ECW doesn't like loading images smaller than 128x128, so it won't
		TNRealRows = TNRealCols = 0;
#ifdef WCS_IMAGE_MANAGEMENT
		ICC.QueryOnlyFlags = NULL;
		if (LoadRasterImage(filename, this, 0, AllowImageManagement == 1 ? &ICC : NULL)) // disallow IM autoswitch-on if directed
#else // !WCS_IMAGE_MANAGEMENT
		if (LoadRasterImage(filename, this, 0, NULL))
#endif // !WCS_IMAGE_MANAGEMENT
			{
			//Cols = Width;
			//Rows = Height;
			//if (Planes == 8)
			//	ByteBands = 1;
			//else if (Planes == 24)
			//	ByteBands = 3;
			//ByteBandSize = Rows * Cols;
			//if (TempAlpha)
			//	AlphaAvailable = 1;
			//else
			//	AlphaAvailable = 0;
			if (! Name[0])
				SetDefaultName();
			#ifdef WCS_IMAGE_MANAGEMENT
			if (ICC.ImageManagementEnable)
				{
				// force image management on
				if (! MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
					{ // don't have attribute
					RasterAttribute *MyAttr;
					if (MyAttr = AddAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
						{
						ImageManagerEnabled = 1;
						// may need to force ImgObjLib to redraw if open
						} // if
					} // if
				else
					{ // already have such an attribute
					// switch on Image Management if necessary
					ImageManagerEnabled = 1;
					// may need to force ImgObjLib to redraw if open
					} // else
				if (ImageManagerEnabled && (ColorControlEnabled || SequenceEnabled || DissolveEnabled))
					{
					ColorControlEnabled = 0;
					DissolveEnabled = 0;
					SequenceEnabled = 0;
					UserMessageOK("Add Image Manager", "It has been determined that an Image Manager attribute is advisable for this image to reduce memory consumption. Color Control, Dissolve and Sequence attributes and Image Manager attributes are incompatible. If you wish to use a Color Control, Dissolve or Sequence on this image you will need to remove or disable the Image Manager attribute.");
					} // if

				if (ICC.QueryOnlyFlags & WCS_BITMAPS_IMAGECAPABILITY_EFFICIENTTHUMBNAIL)
					{ // we should have recieved a valid mini-image thumbnail, so proceed as usual
					// nothing to do
					} // if
				else
					{
					// create stand-in thumbnail
					Raster *StandInNail = NULL;
					
					// stash real settings temporarily
					AlphaAvailStash = AlphaAvailable;
					AlphaEnableStash = AlphaEnabled;
					TNRealCols = Cols;
					TNRealRows = Rows;

					// abuse settings to make stand-in
					Cols = 100;
					Rows = 100;
					ByteBands = 3;
					ByteBandSize = Rows * Cols;
					AlphaAvailable = 0;
					AlphaEnabled = 0;
					
					ByteMap[WCS_RASTER_IMAGE_BAND_RED] = NULL;
					ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] = NULL;
					ByteMap[WCS_RASTER_IMAGE_BAND_BLUE] = NULL;

					if (StandInNail = GlobalApp->LoadToImageLib->CreateStandInRast())
						{
						// steal image settings from stand-in
						Cols = StandInNail->Cols;
						Rows = StandInNail->Rows;
						ByteBands = StandInNail->ByteBands;
						ByteBandSize = StandInNail->ByteBandSize;
						AlphaAvailable = StandInNail->AlphaAvailable;
						AlphaEnabled = StandInNail->AlphaEnabled;
					
						// steal the byte bands from the standin
						ByteMap[WCS_RASTER_IMAGE_BAND_RED] = StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_RED];
						ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] = StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN];
						ByteMap[WCS_RASTER_IMAGE_BAND_BLUE] = StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE];

						// prevent double-free when stand-in is disposed, as we now own the byte bands
						StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_RED] = NULL;
						StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] = NULL;
						StandInNail->ByteMap[WCS_RASTER_IMAGE_BAND_BLUE] = NULL;

						delete StandInNail; StandInNail = NULL; // clean up
						} // if
					else
						{
						for (int tempband = 0; tempband < 3; tempband++ )
							{
							if (ByteMap[tempband] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "Thumbnail Bitmaps"))
								{
								// Clear
								// clear to grey for now so we can see what's up
								memset(ByteMap[tempband], 127, Cols * Rows);
								} // if
							else
								{
								break;
								} // else
							} // for
						} // else

					} // else
				} // if
			// copy over queried flags
			ImageCapabilityFlags = ICC.QueryOnlyFlags;

			#endif // WCS_IMAGE_MANAGEMENT
			// temporarily backup and disable Image Management while creating thumbnail
			// from already-loaded thumbnail raster data
			unsigned char IMBackup = ImageManagerEnabled;
			ImageManagerEnabled = 0;
			CreateImageRGBThumbNail();
			ImageManagerEnabled = IMBackup;
			FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
			if (LoaderGeoRefShell)
				{
				if (UpdateCoordSys)
					{
					MatchSys = NULL;
					LoaderGeoRefShell->Rast = this;
					if (LoaderGeoRefShell->Host)
						{
						MatchSys = GlobalApp->LoadToEffectsLib->CompareMakeCoordSys((CoordSys *)LoaderGeoRefShell->Host, 1);
						delete (CoordSys *)LoaderGeoRefShell->Host;
						LoaderGeoRefShell->Host = NULL;
						} // if
					if ((GeoAttr = MatchAttribute(LoaderGeoRefShell->GetType())) && GeoAttr->GetShell())
						{
						if (GeoAttr->GetShell()->Host)
							GeoAttr->GetShell()->Host->RemoveRaster(GeoAttr->GetShell());
						((GeoRefShell *)GeoAttr->GetShell())->Copy((GeoRefShell *)GeoAttr->GetShell(), LoaderGeoRefShell);
						if (MatchSys)
							{
							MatchSys->RemoveRaster(GeoAttr->GetShell());
							MatchSys->AddRaster(this);
							} // if
						delete LoaderGeoRefShell;
						} // if already a geo ref
					else
						{
						AddAttribute(LoaderGeoRefShell->GetType(), LoaderGeoRefShell, MatchSys);
						MatchSys->AddRaster(this);
						} // else
					} // if
				else
					{
					if (LoaderGeoRefShell->Host)
						delete (CoordSys *)LoaderGeoRefShell->Host;
					LoaderGeoRefShell->Host = NULL;
					delete LoaderGeoRefShell;
					} // else
				LoaderGeoRefShell = NULL;
				} // if
			// get real image dimensions, if all we loaded was a thumbnail
			if (TNRealCols != 0)
				{
				Cols = TNRealCols;
				Rows = TNRealRows;
				} // if
			//if (TempAlpha)
			//	{
			//	AppMem_Free(TempAlpha, Rows * Cols);
			//	TempAlpha = NULL;
			//	} // if
			return (1);
			} // if
		else if (UserMessageOKCAN((char *)PAF.GetName(), "Image could not be loaded. Possible reasons include invalid file path, \
file permission denied, improper file format, invalid or incomplete file, out of memory, etc.\nSelect a replacement file?"))
			{
			if (PAF.SelectFile())
				Retry = 1;
			else
				Enabled = 0;
			} // if
		else
			Enabled = 0;
		} // if
#ifndef WCS_BUILD_REMOTEFILE_SUPPORT
	else
		{
		if (UserMessageOKCAN((char *)PAF.GetName(), "Invalid file path or name! File cannot be opened.\nSelect a replacement file?"))
			{
			if (PAF.SelectFile())
				Retry = 1;
			else
				Enabled = 0;
			} // if
		else
			Enabled = 0;
		} // else
#endif // !WCS_BUILD_REMOTEFILE_SUPPORT
	} // while

if (Enabled != LastEnabled)
	{
	Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
	} // if

return (0);

} // Raster::LoadnPrepImage

/*===========================================================================*/

int Raster::LoadnProcessImage(int ReplaceTransparent, char *SuppliedName)
{
char filename[256];
short /* Width, Height, Planes, */ MaxVal;
ColorControlShell *ColorShell = NULL;
RasterAttribute *MyAttr;
char Band, ComplexFormula;
double TempDbl;
int Success = 1;
/* UBYTE *TempAlpha = NULL; */

#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
if (SuppliedName || (PAF.GetValidPathAndName(filename) || 1))
#else // !WCS_BUILD_REMOTEFILE_SUPPORT
if (SuppliedName || PAF.GetValidPathAndName(filename))
#endif // !WCS_BUILD_REMOTEFILE_SUPPORT
	{
	if (SuppliedName)
		strcpy(filename, SuppliedName);
	FreeAllBands(WCS_RASTER_LONGEVITY_LONG);
	//if (LoadRasterImage(filename, 0, ByteMap, 0, 0, 0, &Width, &Height, &Planes, &TempAlpha))
	TNRowsReq = TNColsReq = 0; // ensure we don't load a thumbnail
	if (LoadRasterImage(filename, this, 0))
		{
		//Cols = Width;
		//Rows = Height;
		//if (Planes == 8)
		//	ByteBands = 1;
		//else if (Planes == 24)
		//	ByteBands = 3;
		//ByteBandSize = Rows * Cols;
		//if (TempAlpha)
		//	{
		//	AlphaAvailable = 1;
		//	if (AlphaEnabled)
		//		{
		//		CopyAlphaToCoverage(TempAlpha);
		//		} // if
		//	AppMem_Free(TempAlpha, ByteBandSize);
		//	TempAlpha = NULL;
		//	} // if
		//else
		//	{
		//	AlphaAvailable = 0;
		//	AlphaEnabled = 0;
		//	} // else
		if ((MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL)) && ColorControlEnabled)
			{
			ColorShell = (ColorControlShell *)MyAttr->GetShell();
			} // if
		if (ColorShell)
			{
			// swap color pointers
			if (ColorShell->UseBandAssignment)
				{
				Red = ByteMap[ColorShell->UseBandAs[0]];
				Green = ByteMap[ColorShell->UseBandAs[1]];
				Blue = ByteMap[ColorShell->UseBandAs[2]];
				} // if
			else
				{
				ComplexFormula = 0;
				for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
					{
					if (ByteMap[Band])
						{
						if ((Band != 0 && ColorShell->BandFactor[0][Band] != 0.0) || (Band == 0 && ColorShell->BandFactor[0][Band] != 1.0))
							ComplexFormula = 1;
						if ((Band != 1 && ColorShell->BandFactor[1][Band] != 0.0) || (Band == 1 && ColorShell->BandFactor[1][Band] != 1.0))
							ComplexFormula = 1;
						if ((Band != 2 && ColorShell->BandFactor[2][Band] != 0.0) || (Band == 2 && ColorShell->BandFactor[2][Band] != 1.0))
							ComplexFormula = 1;
						} // if
					} // for
				if (ComplexFormula)
					{
					Red = (UBYTE *)AppMem_Alloc(ByteBandSize, APPMEM_CLEAR);
					Green = (UBYTE *)AppMem_Alloc(ByteBandSize, APPMEM_CLEAR);
					Blue = (UBYTE *)AppMem_Alloc(ByteBandSize, APPMEM_CLEAR);
					if (Red && Green && Blue)
						{
						for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
							{
							if (ByteMap[Band] && ColorShell->BandFactor[0][Band] != 0.0)
								{
								for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
									{
									if ((TempDbl = Red[RasterCell] + (ByteMap[Band][RasterCell] * ColorShell->BandFactor[0][Band])) < 256.0)
										Red[RasterCell] = (UBYTE)TempDbl;
									else
										Red[RasterCell] = 255;
									} // for
								} // if
							} // for
						for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
							{
							if (ByteMap[Band] && ColorShell->BandFactor[1][Band] != 0.0)
								{
								for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
									{
									if ((TempDbl = Green[RasterCell] + (ByteMap[Band][RasterCell] * ColorShell->BandFactor[1][Band])) < 256.0)
										Green[RasterCell] = (UBYTE)TempDbl;
									else
										Green[RasterCell] = 255;
									} // for
								} // if
							} // for
						for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
							{
							if (ByteMap[Band] && ColorShell->BandFactor[2][Band] != 0.0)
								{
								for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
									{
									if ((TempDbl = Blue[RasterCell] + (ByteMap[Band][RasterCell] * ColorShell->BandFactor[2][Band])) < 256.0)
										Blue[RasterCell] = (UBYTE)TempDbl;
									else
										Blue[RasterCell] = 255;
									} // for
								} // if
							} // for
						FreeByteBand(0);
						FreeByteBand(1);
						FreeByteBand(2);
						ByteMap[0] = Red;
						ByteMap[1] = Green;
						ByteMap[2] = Blue;
						} // if RGB
					else
						{
						Success = 0;
						goto ErrorCleanup;
						} // else allocation error
					} // if Complex formula
				else
					{
					Red = ByteMap[0];
					Green = ByteMap[1];
					Blue = ByteMap[2];
					} // else use default formula
				} // else use formula
			// replace transparent range with 0
			if (ColorShell->RGB[0][0] != 0 || ColorShell->RGB[1][0] != 0 || 
				ColorShell->RGB[0][1] != 0 || ColorShell->RGB[1][1] != 0 || 
				ColorShell->RGB[0][2] != 0 || ColorShell->RGB[1][2] != 0)
				{
				if (Red && Green && Blue)
					{
					for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
						{
						if (Red[RasterCell] >= ColorShell->RGB[1][0] && Red[RasterCell] <= ColorShell->RGB[0][0] &&
							Green[RasterCell] >= ColorShell->RGB[1][1] && Green[RasterCell] <= ColorShell->RGB[0][1] &&
							Blue[RasterCell] >= ColorShell->RGB[1][2] && Blue[RasterCell] <= ColorShell->RGB[0][2])
							Red[RasterCell] = Green[RasterCell] = Blue[RasterCell] = 0;
						} // for
					} // if all three colors
				else if (Red)
					{
					for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
						{
						if (Red[RasterCell] >= ColorShell->RGB[1][0] && Red[RasterCell] <= ColorShell->RGB[0][0])
							Red[RasterCell] = 0;
						} // for
					} // else if
				} // if non-default transparent range
			if (ReplaceTransparent && ColorShell->ShowTransparency &&
				(ColorShell->RGB[2][0] != 0 || ColorShell->RGB[2][1] != 0 || ColorShell->RGB[2][2] != 0))
				{
				if (Red && Green && Blue)
					{
					for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
						{
						if (Red[RasterCell] == 0 && Green[RasterCell] == 0 && Blue[RasterCell] == 0)
							{
							Red[RasterCell] = (UBYTE)ColorShell->RGB[2][0];
							Green[RasterCell] = (UBYTE)ColorShell->RGB[2][1];
							Blue[RasterCell] = (UBYTE)ColorShell->RGB[2][2];
							} // if
						} // for
					} // if all three colors
				else if (Red)
					{
					for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
						{
						if (Red[RasterCell] == 0)
							Red[RasterCell] = (UBYTE)ColorShell->RGB[2][0];
						} // for
					} // else if
				} // if replace and show transparency
			if (! ColorShell->UseAsColor && Red && Green && Blue)
				{
				for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
					{
					Red[RasterCell] = (Red[RasterCell] + Green[RasterCell] + Blue[RasterCell]) / 3;
					} // for
				FreeByteBand(1);
				FreeByteBand(2);
				Green = Blue = NULL;
				} // if use as gray
			if (ColorShell->GrayAutoRange && (! Green || ! Blue))
				{
				MaxVal = 0;
				for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
					{
					if (Red[RasterCell] > MaxVal)
						MaxVal = Red[RasterCell];
					} // for
				TempDbl = 255.9 / MaxVal;
				for (RasterCell = 0; RasterCell < ByteBandSize; RasterCell ++)
					{
					Red[RasterCell] = (UBYTE)(Red[RasterCell] * TempDbl);
					} // for
				} // if expand gray range
			} // if Color Shell
		else
			{
			Red = ByteMap[0];
			Green = ByteMap[1];
			Blue = ByteMap[2];
			} // else no Color Shell
		} // if image loaded
	} // if
else
	{
	UserMessageOK((char *)PAF.GetName(), "Invalid file name or path!\nFile cannot be opened.");\
	Success = 0;
	} // else

return (Success);

ErrorCleanup:

if (Red)
	AppMem_Free(Red, ByteBandSize);
Red = NULL;
if (Green)
	AppMem_Free(Green, ByteBandSize);
Green = NULL;
if (Blue)
	AppMem_Free(Blue, ByteBandSize);
Blue = NULL;
return (0);

} // Raster::LoadnProcessImage

/*===========================================================================*/

int Raster::CopyBitmaps(Raster *Source, double SourceWt, int AsFoliage)
{
long Error = 0;
Raster *Temp = NULL;

if (SourceWt == 0.0)
	return (1);

if (AsFoliage && ! FoliageBandsValid())
	{
	if ((SourceWt < 1.0 && ! CreateFoliageBands()) || (SourceWt >= 1.0 && ! AllocFoliageBands()))
		return (0);
	} // if
if (AsFoliage && ! Source->FoliageBandsValid())
	{
	if (! Source->CreateFoliageBands())
		return (0);
	} // if
if ((BandValid(Green) != BandValid(Source->Green)) || (BandValid(Blue) != BandValid(Source->Blue)))
	{
	if (Temp = new Raster)
		{
		Temp->Copy(Temp, Source);
		if (AsFoliage && ! Temp->AllocFoliageBands())
			Error = 1;
		} // if
	else
		return (0);
	if (Source->Green && Source->Blue)
		{
		if (! ((Temp->Red = Temp->AllocByteBand(0)) && Temp->ColorToGray(Source) && Temp->CopyNonRGBBands(Source, 1.0)))
			{
			Error = 1;
			} // else
		} // if need to copy down to one band
	else if (Green && Blue)
		{
		if (! ((Temp->Red = Temp->AllocByteBand(0)) && (Temp->Green = Temp->AllocByteBand(1)) && (Temp->Blue = Temp->AllocByteBand(2)) && Temp->GrayToColor(Source) && Temp->CopyNonRGBBands(Source, 1.0)))
			{
			Error = 1;
			} // else
		} // else copy up to three bands
	else
		{
		Error = 1;
		} // else not recognized configuration
	if (Error)
		{
		delete Temp;
		return (0);
		} // if
	Source = Temp;
	} // if

if (AsFoliage)
	{
	if (! CopyByteMapMerge(Source, Cols, Rows, Source->Cols, Source->Rows, SourceWt))
		{
		if (Temp)
			delete Temp;
		return (0);
		} // if
	} // if
else
	{
	if (Source->Red && Red && (! CopyByteMap(Red, Source->Red, Cols, Rows, Source->Cols, Source->Rows, SourceWt)))
		return (0);
	if (Source->Green && Green && (! CopyByteMap(Green, Source->Green, Cols, Rows, Source->Cols, Source->Rows, SourceWt)))
		return (0);
	if (Source->Blue && Blue && (! CopyByteMap(Blue, Source->Blue, Cols, Rows, Source->Cols, Source->Rows, SourceWt)))
		return (0);
	}
if (! CopyNonRGBBands(Source, SourceWt))
	{
	if (Temp)
		delete Temp;
	return (0);
	} // if

if (Temp)
	delete Temp;
return (1);

} // Raster::CopyBitmaps

/*===========================================================================*/

int Raster::ColorToGray(Raster *Source)
{
long Ct;

if (Cols != Source->Cols || Rows != Source->Rows || ! Red || ! Source->Red || ! Source->Green || ! Source->Blue)
	return (0);

for (Ct = 0; Ct < ByteBandSize; Ct ++)
	{
	Red[Ct] = (UBYTE)(.5 + (Source->Red[Ct] + Source->Green[Ct] + Source->Blue[Ct]) * (1.0 / 3.0));
	} // for

return (1);

} // Raster::ColorToGray

/*===========================================================================*/

int Raster::GrayToColor(Raster *Source)
{
long Ct;

if (Cols != Source->Cols || Rows != Source->Rows || ! Red || ! Source->Red || ! Green || ! Blue)
	return (0);

for (Ct = 0; Ct < ByteBandSize; Ct ++)
	{
	Red[Ct] = Green[Ct] = Blue[Ct] = Source->Red[Ct];
	} // for

return (1);

} // Raster::GrayToColor

/*===========================================================================*/

int Raster::CopyAlphaToCoverage(UBYTE *Source)
{
long Ct, BandCt;

if (! Source || ! (Rows && Cols))
	return (0);

if (AltFloatMap && (AltFloatBandSize != (long)(ByteBandSize * sizeof (float))))
	{
	FreeAltFloatBand();
	} // if wrong size
if (! AltFloatMap)
	{
	AllocAltFloatBand(Rows, Cols);
	} // if
if (AltFloatMap)
	{
	for (Ct = 0; Ct < ByteBandSize; Ct ++)
		{
		if (! Source[Ct])
			{
			for (BandCt = 0; BandCt < WCS_RASTER_MAX_BANDS; BandCt ++)
				{
				if (ByteMap[BandCt])
					ByteMap[BandCt][Ct] = 0;
				if (FloatMap[BandCt])
					FloatMap[BandCt][Ct] = 0.0f;
				} // for
			} // if
		AltFloatMap[Ct] = (float)(Source[Ct] / 255.0);
		} // for

	return (1);
	} // if
else
	return (0);

} // Raster::CopyAlphaToCoverage

/*===========================================================================*/

int Raster::CopyCoverageToAlpha(void)
{
long Ct;

if (! (Rows && Cols && AltFloatMap))
	return (0);

if (ByteMap[WCS_RASTER_IMAGE_BAND_ALPHA] || AllocByteBand(WCS_RASTER_IMAGE_BAND_ALPHA))
	{
	AABuf = ByteMap[WCS_RASTER_IMAGE_BAND_ALPHA];
	for (Ct = 0; Ct < ByteBandSize; Ct ++)
		{
		AABuf[Ct] = (unsigned char)(AltFloatMap[Ct] * 255.99f);
		} // for
	AlphaEnabled = 1;
	return (1);
	} // if

return (0);

} // Raster::CopyCoverageToAlpha

/*===========================================================================*/

int Raster::CopyNonRGBBands(Raster *Source, double SourceWt)
{
char Band;

for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
	{
	if (Source->ByteMap[Band] && (ByteMap[Band] || ((ByteMap[Band] = AllocByteBand(Band)) && ClearByteBand(Band))) && ByteMap[Band] != Red && ByteMap[Band] != Green && ByteMap[Band] != Blue)
		{
		if (! CopyByteMap(ByteMap[Band], Source->ByteMap[Band], Cols, Rows, Source->Cols, Source->Rows, SourceWt))
			return (0);
		} // if this band
	if (Source->FloatMap[Band] && (FloatMap[Band] || ((FloatMap[Band] = AllocFloatBand(Band)) && ClearFloatBand(Band))))
		{
		if (! CopyFloatMap(FloatMap[Band], Source->FloatMap[Band], Cols, Rows, Source->Cols, Source->Rows, SourceWt))
			return (0);
		} // if this band
	} // for
if (Source->AltByteMap && AltByteMap)
	{
	CopyByteMap(AltByteMap, Source->AltByteMap, Cols, Rows, Source->Cols, Source->Rows, SourceWt);
	} // if
if (Source->AltFloatMap && AltFloatMap)
	{
	CopyFloatMap(AltFloatMap, Source->AltFloatMap, Cols, Rows, Source->Cols, Source->Rows, SourceWt);
	} // if
if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
	{
	CopyFloatMap(RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT], Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT], 1, Rows, 1, Source->Rows, SourceWt);
	} // if
if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
	{
	CopyFloatMap(RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN], Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN], 1, Rows, 1, Source->Rows, SourceWt);
	} // if

return (1);

} // Raster::CopyNonRGBBands

/*===========================================================================*/

int Raster::CopyByteMap(UBYTE *Dest, UBYTE *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt)
{
double DestWt, IntX, IntY, ScoX, ScoY, ScoXp1, ScoYp1, CovgX, CovgY, SpArea, SpColor, SpW;
long SpX, SpY, SpXp1, SpYp1, DpX, DpY, DpZip, SpZip;

// D stands for destination, S is for Source, p is for pixel, o is for origin, p1 is for +1, Int is for interval, Covg is for coverage

if (Sw <= 0 || Sh <= 0 || Dw <= 0 || Dh <= 0)
	return (0);

DestWt = 1.0 - SourceWt;
if (DestWt < 0.0)
	{
	DestWt = 0.0;
	SourceWt = 1.0;
	} // if

if (Sw == Dw && Sh == Dh)
	{
	SpZip = Sw * Sh;	// SpZip is an unused variable so we'll use it for the maximum counter limit
	if (SourceWt >= 1.0)
		{
		for (DpZip = 0; DpZip < SpZip; DpZip ++)
			{
			Dest[DpZip] = Source[DpZip];
			} // for
		} // if simple replace - might as weel elliminate the multiplication
	else
		{
		for (DpZip = 0; DpZip < SpZip; DpZip ++)
			{
			Dest[DpZip] = (UBYTE)(Source[DpZip] * SourceWt + Dest[DpZip] * DestWt);
			} // for
		} // else
	} // if same size
else
	{
	IntX = (double)Sw / (double)Dw;
	IntY = (double)Sh / (double)Dh;
	SpArea = IntX * IntY;

	for (DpY = 0, ScoY = 0.0, DpZip = 0, ScoYp1 = IntY; DpY < Dh; DpY ++, ScoY = ScoYp1, ScoYp1 += IntY)
		{
		if (ScoYp1 > Sh)
			ScoYp1 = Sh;
		for (DpX = 0, ScoX = 0.0, ScoXp1 = IntX; DpX < Dw; DpX ++, ScoX = ScoXp1, ScoXp1 += IntX, DpZip ++)
			{
			if (ScoXp1 > Sw)
				ScoXp1 = Sw;
			SpColor = 0.0;
			SpY = quicklongfloor(ScoY);
			for (SpYp1 = SpY + 1; (double)SpY < ScoYp1; SpY ++, SpYp1 ++)
				{
				CovgY = min((double)SpYp1, ScoYp1) - max((double)SpY, ScoY);
				SpX = quicklongfloor(ScoX);
				for (SpXp1 = SpX + 1, SpZip = SpY * Sw + SpX; (double)SpX < ScoXp1; SpX ++, SpXp1 ++, SpZip ++)
					{
					CovgX = min((double)SpXp1, ScoXp1) - max((double)SpX, ScoX);
					SpW = (CovgX * CovgY) / SpArea;
					SpColor += (SpW * Source[SpZip]);
					} // for SpX
				} // for SpY
			Dest[DpZip] = (UBYTE)(SpColor * SourceWt + Dest[DpZip] * DestWt);
			} // for DpX
		} // for DpY
	} // else not same size

return (1);

} // Raster::CopyByteMap

/*===========================================================================*/

int Raster::CopyByteMapMerge(Raster *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt)
{
double DestWt, IntX, IntY, ScoX, ScoY, ScoXp1, ScoYp1, CovgX, CovgY, SpArea, SpColor[3], SpCovg, SpZOff = 0.0, SpSpan = 0.0, SpMidPt = 0.0, SpW, SumSpW, SumSpYW;
long SpX, SpY, SpXp1, SpYp1, DpX, DpY, DpZip, SpZip, SourceFound, DestFound, SourceRowHasData;

// D stands for destination, S is for Source, p is for pixel, o is for origin, p1 is for +1, Int is for interval, Covg is for coverage

if (Sw <= 0 || Sh <= 0 || Dw <= 0 || Dh <= 0 || ! Red || ! Source->Red || ! AltFloatMap)
	return (0);

DestWt = 1.0 - SourceWt;
if (DestWt < 0.0)
	{
	DestWt = 0.0;
	SourceWt = 1.0;
	} // if

if (Sw == Dw && Sh == Dh)
	{
	SpZip = Sw * Sh;	// SpZip is an unused variable so we'll use it for the maximum counter limit
	if (SourceWt >= 1.0)
		{
		if (Green && Source->Green && Blue && Source->Blue)
			{
			for (DpZip = 0; DpZip < SpZip; DpZip ++)
				{
				if (Source->Red[DpZip] || Source->Green[DpZip] || Source->Blue[DpZip])
					{
					AltFloatMap[DpZip] = (float)Source->AltFloatMap[DpZip];
					if (AltByteMap && Source->AltByteMap)
						AltByteMap[DpZip] = Source->AltByteMap[DpZip];
					Red[DpZip] = Source->Red[DpZip];
					Green[DpZip] = Source->Green[DpZip];
					Blue[DpZip] = Source->Blue[DpZip];
					} // if
				else
					{
					AltFloatMap[DpZip] = 0.0f;
					if (AltByteMap)
						AltByteMap[DpZip] = 0;
					Red[DpZip] = 0;
					Green[DpZip] = 0;
					Blue[DpZip] = 0;
					} // else
				} // for
			} // if rgb
		else
			{
			for (DpZip = 0; DpZip < SpZip; DpZip ++)
				{
				if (Source->Red[DpZip])
					{
					AltFloatMap[DpZip] = Source->AltFloatMap[DpZip];
					if (AltByteMap && Source->AltByteMap)
						AltByteMap[DpZip] = Source->AltByteMap[DpZip];
					Red[DpZip] = Source->Red[DpZip];
					} // if
				else
					{
					AltFloatMap[DpZip] = 0.0f;
					if (AltByteMap)
						AltByteMap[DpZip] = 0;
					Red[DpZip] = 0;
					} // else
				} // for
			} // else gray
		for (DpZip = 0; DpZip < Rows; DpZip ++)
			{
			if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
				RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpZip] = Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpZip];
			if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
				RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpZip] = Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpZip];
			} // for
		} // if simple replace - might as well elliminate the multiplication
	else
		{
		if (Green && Source->Green && Blue && Source->Blue)
			{
			for (DpY = 0, DpZip = 0; DpY < Rows; DpY ++)
				{
				SourceFound = DestFound = 0;
				for (DpX = 0; DpX < Cols; DpX ++, DpZip ++)
					{
					if (Red[DpZip] || Green[DpZip] || Blue[DpZip])
						{
						if (Source->Red[DpZip] || Source->Green[DpZip] || Source->Blue[DpZip])
							{
							AltFloatMap[DpZip] =  (float)(SourceWt * Source->AltFloatMap[DpZip] + DestWt * AltFloatMap[DpZip]);
							if (AltByteMap && Source->AltByteMap)
								AltByteMap[DpZip] = (UBYTE)(SourceWt * Source->AltByteMap[DpZip] + DestWt * AltByteMap[DpZip]);
							Red[DpZip] = (UBYTE)(SourceWt * Source->Red[DpZip] + DestWt * Red[DpZip]);
							Green[DpZip] = (UBYTE)(SourceWt * Source->Green[DpZip] + DestWt * Green[DpZip]);
							Blue[DpZip] = (UBYTE)(SourceWt * Source->Blue[DpZip] + DestWt * Blue[DpZip]);
							SourceFound = 1;
							} // if
						else
							{
							AltFloatMap[DpZip] = (float)DestWt * AltFloatMap[DpZip];
							// destination colors don't change
							} // else
						DestFound = 1;
						} // if
					else if (Source->Red[DpZip] || Source->Green[DpZip] || Source->Blue[DpZip])
						{
						AltFloatMap[DpZip] =  (float)(SourceWt * Source->AltFloatMap[DpZip]);
						if (AltByteMap && Source->AltByteMap)
							AltByteMap[DpZip] =  Source->AltByteMap[DpZip];
						Red[DpZip] = (UBYTE)(Source->Red[DpZip]);
						Green[DpZip] = (UBYTE)(Source->Green[DpZip]);
						Blue[DpZip] = (UBYTE)(Source->Blue[DpZip]);
						SourceFound = 1;
						} // if
					else
						{
						AltFloatMap[DpZip] = 0.0f;
						// destination colors remain 0
						} // else
					} // for DpX
				if (SourceFound && DestFound)
					{
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
							(float)(SourceWt * Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] + 
							DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY]);
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
							(float)(Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] +
							DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY]);
					} // if
				else if (SourceFound)
					{
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
							Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY];
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
							Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY];
					} // else if
				} // for DpY
			} // if rgb
		else
			{
			for (DpY = 0, DpZip = 0; DpY < Rows; DpY ++)
				{
				SourceFound = DestFound = 0;
				for (DpX = 0; DpX < Cols; DpX ++, DpZip ++)
					{
					if (Red[DpZip])
						{
						if (Source->Red[DpZip])
							{
							AltFloatMap[DpZip] =  (float)(SourceWt * Source->AltFloatMap[DpZip] + DestWt * AltFloatMap[DpZip]);
							if (AltByteMap && Source->AltByteMap)
								AltByteMap[DpZip] = (UBYTE)(SourceWt * Source->AltByteMap[DpZip] + DestWt * AltByteMap[DpZip]);
							Red[DpZip] = (UBYTE)(SourceWt * Source->Red[DpZip] + DestWt * Red[DpZip]);
							SourceFound = 1;
							} // if
						else
							{
							AltFloatMap[DpZip] = (float)DestWt * AltFloatMap[DpZip];
							// destination colors don't change
							} // else
						DestFound = 1;
						} // if
					else if (Source->Red[DpZip])
						{
						AltFloatMap[DpZip] =  (float)(SourceWt * Source->AltFloatMap[DpZip]);
						if (AltByteMap && Source->AltByteMap)
							AltByteMap[DpZip] =  Source->AltByteMap[DpZip];
						Red[DpZip] = (UBYTE)(Source->Red[DpZip]);
						SourceFound = 1;
						} // if
					else
						{
						AltFloatMap[DpZip] = 0.0f;
						// destination colors remain 0
						} // else
					} // for DpX
				if (SourceFound && DestFound)
					{
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
							(float)(SourceWt * Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] + 
							DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY]);
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
							(float)(Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] +
							DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY]);
					} // if
				else if (SourceFound)
					{
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
							Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY];
					if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN] && Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
						RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
							Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY];
					} // else if
				} // for DpY
			} // else gray
		} // else SouceWt < 1
	} // if same size
else
	{
	IntX = (double)Sw / (double)Dw;
	IntY = (double)Sh / (double)Dh;
	SpArea = IntX * IntY;

	if (Green && Source->Green && Blue && Source->Blue)
		{
		for (DpY = 0, ScoY = 0.0, DpZip = 0, ScoYp1 = IntY; DpY < Dh; DpY ++, ScoY = ScoYp1, ScoYp1 += IntY)
			{
			if (ScoYp1 > Sh)
				ScoYp1 = Sh;
			SpSpan = SpMidPt = SumSpYW = 0.0;
			SourceFound = DestFound = 0;
			for (DpX = 0, ScoX = 0.0, ScoXp1 = IntX; DpX < Dw; DpX ++, ScoX = ScoXp1, ScoXp1 += IntX, DpZip ++)
				{
				if (ScoXp1 > Sw)
					ScoXp1 = Sw;
				SpColor[0] = SpColor[1] = SpColor[2] = 0.0;
				SpCovg = SpZOff = SumSpW = 0.0;
				SpY = quicklongfloor(ScoY);
				for (SpYp1 = SpY + 1; (double)SpY < ScoYp1 && SpY < Sh; SpY ++, SpYp1 ++)
					{
					SourceRowHasData = 0;
					CovgY = min((double)SpYp1, ScoYp1) - max((double)SpY, ScoY);
					SpX = quicklongfloor(ScoX);
					for (SpXp1 = SpX + 1, SpZip = SpY * Sw + SpX; (double)SpX < ScoXp1 && SpX < Sw; SpX ++, SpXp1 ++, SpZip ++)
						{
						if (Source->Red[SpZip] || Source->Green[SpZip] || Source->Blue[SpZip])
							{
							CovgX = min((double)SpXp1, ScoXp1) - max((double)SpX, ScoX);
							SpW = (CovgX * CovgY) / SpArea;
							SpColor[0] += (SpW * Source->Red[SpZip]);
							SpColor[1] += (SpW * Source->Green[SpZip]);
							SpColor[2] += (SpW * Source->Blue[SpZip]);
							SpCovg += (SpW * Source->AltFloatMap[SpZip]);
							if (Source->AltByteMap)
								SpZOff += (SpW * Source->AltByteMap[SpZip]);
							SumSpW += SpW;
							SourceRowHasData = 1;
							} // if
						} // for SpX
					if (DpX == 0 && SourceRowHasData)
						{
						CovgY /= IntY;
						SumSpYW += CovgY;
						if (Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
							SpSpan += Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][SpY] * CovgY;
						if (Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
							SpMidPt += Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][SpY] * CovgY;
						} // if
					} // for SpY
				if (SumSpW != 1.0 && SumSpW > 0.0)
					{
					SpColor[0] /= SumSpW;
					SpColor[1] /= SumSpW;
					SpColor[2] /= SumSpW;
					SpCovg /= SumSpW;
					SpZOff /= SumSpW;
					} // if
				if (Red[DpZip] || Green[DpZip] || Blue[DpZip])
					{
					if (SumSpW > 0.0)
						{
						AltFloatMap[DpZip] =  (float)(SourceWt * SpCovg + DestWt * AltFloatMap[DpZip]);
						if (AltByteMap)
							AltByteMap[DpZip] = (UBYTE)(SourceWt * SpZOff + DestWt * AltByteMap[DpZip]);
						Red[DpZip] = (UBYTE)(SourceWt * SpColor[0] + DestWt * Red[DpZip]);
						Green[DpZip] = (UBYTE)(SourceWt * SpColor[1] + DestWt * Green[DpZip]);
						Blue[DpZip] = (UBYTE)(SourceWt * SpColor[2] + DestWt * Blue[DpZip]);
						SourceFound = 1;
						} // if
					else
						{
						AltFloatMap[DpZip] = (float)DestWt * AltFloatMap[DpZip];
						// destination colors don't change
						} // else
					DestFound = 1;
					} // if
				else if (SumSpW > 0.0)
					{
					AltFloatMap[DpZip] =  (float)(SourceWt * SpCovg);
					if (AltByteMap)
						AltByteMap[DpZip] =  (UBYTE)SpZOff;
					Red[DpZip] = (UBYTE)(SpColor[0]);
					Green[DpZip] = (UBYTE)(SpColor[1]);
					Blue[DpZip] = (UBYTE)(SpColor[2]);
					SourceFound = 1;
					} // if
				else
					{
					AltFloatMap[DpZip] = 0.0f;
					// destination colors remain 0
					} // else
				} // for DpX
			if (SumSpYW != 1.0 && SumSpYW > 0.0)
				{
				SpSpan /= SumSpYW;
				SpMidPt /= SumSpYW;
				} // if
			if (SourceFound && DestFound)
				{
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
						(float)(SourceWt * SpMidPt + 
						DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY]);
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
						(float)(SourceWt * SpSpan +
						DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY]);
				} // if
			else if (SourceFound)
				{
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = (float)SpMidPt;
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = (float)SpSpan;
				} // else if
			} // for DpY
		} // if rgb
	else
		{
		for (DpY = 0, ScoY = 0.0, DpZip = 0, ScoYp1 = IntY; DpY < Dh; DpY ++, ScoY = ScoYp1, ScoYp1 += IntY)
			{
			if (ScoYp1 > Sh)
				ScoYp1 = Sh;
			SpSpan = SpMidPt = SumSpYW = 0.0;
			SourceFound = DestFound = 0;
			for (DpX = 0, ScoX = 0.0, ScoXp1 = IntX; DpX < Dw; DpX ++, ScoX = ScoXp1, ScoXp1 += IntX, DpZip ++)
				{
				if (ScoXp1 > Sw)
					ScoXp1 = Sw;
				SpColor[0] = 0.0;
				SpCovg = SpZOff = SumSpW = 0.0;
				SpY = quicklongfloor(ScoY);
				for (SpYp1 = SpY + 1; (double)SpY < ScoYp1; SpY ++, SpYp1 ++)
					{
					SourceRowHasData = 0;
					CovgY = min((double)SpYp1, ScoYp1) - max((double)SpY, ScoY);
					SpX = quicklongfloor(ScoX);
					for (SpXp1 = SpX + 1, SpZip = SpY * Sw + SpX; (double)SpX < ScoXp1; SpX ++, SpXp1 ++, SpZip ++)
						{
						if (Source->Red[SpZip])
							{
							CovgX = min((double)SpXp1, ScoXp1) - max((double)SpX, ScoX);
							SpW = (CovgX * CovgY) / SpArea;
							SpColor[0] += (SpW * Source->Red[SpZip]);
							SpCovg += (SpW * Source->AltFloatMap[SpZip]);
							if (Source->AltByteMap)
								SpZOff += (SpW * Source->AltByteMap[SpZip]);
							SumSpW += SpW;
							SourceRowHasData = 1;
							} // if
						} // for SpX
					if (DpX == 0 && SourceRowHasData)
						{
						CovgY /= IntY;
						SumSpYW += CovgY;
						if (Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
							SpSpan += Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][SpY] * CovgY;
						if (Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
							SpMidPt += Source->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][SpY] * CovgY;
						} // if
					} // for SpY
				if (SumSpW != 1.0 && SumSpW > 0.0)
					{
					SpColor[0] /= SumSpW;
					SpCovg /= SumSpW;
					SpZOff /= SumSpW;
					} // if
				if (Red[DpZip])
					{
					if (SumSpW > 0.0)
						{
						AltFloatMap[DpZip] =  (float)(SourceWt * SpCovg + DestWt * AltFloatMap[DpZip]);
						if (AltByteMap)
							AltByteMap[DpZip] = (UBYTE)(SourceWt * SpZOff + DestWt * AltByteMap[DpZip]);
						Red[DpZip] = (UBYTE)(SourceWt * SpColor[0] + DestWt * Red[DpZip]);
						SourceFound = 1;
						} // if
					else
						{
						AltFloatMap[DpZip] = (float)DestWt * AltFloatMap[DpZip];
						// destination colors don't change
						} // else
					DestFound = 1;
					} // if
				else if (SumSpW > 0.0)
					{
					AltFloatMap[DpZip] =  (float)(SourceWt * SpCovg);
					if (AltByteMap)
						AltByteMap[DpZip] =  (UBYTE)SpZOff;
					Red[DpZip] = (UBYTE)(SpColor[0]);
					SourceFound = 1;
					} // if
				else
					{
					AltFloatMap[DpZip] = 0.0f;
					// destination colors remain 0
					} // else
				} // for DpX
			if (SumSpYW != 1.0 && SumSpYW > 0.0)
				{
				SpSpan /= SumSpYW;
				SpMidPt /= SumSpYW;
				} // if
			if (SourceFound && DestFound)
				{
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = 
						(float)(SourceWt * SpMidPt + 
						DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY]);
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = 
						(float)(SourceWt * SpSpan +
						DestWt * RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY]);
				} // if
			else if (SourceFound)
				{
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][DpY] = (float)SpMidPt;
				if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
					RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][DpY] = (float)SpSpan;
				} // else if
			} // for DpY
		} // else gray
	} // else not same size

return (1);

} // Raster::CopyByteMapMerge

/*===========================================================================*/

int Raster::CopyRenderMapMerge(double SourceWt)
{
double DestWt;
float SourceZ, SourceReflect, SourceNormal[3];
unsigned long ThreeColorsA[3], ThreeColorsB[3];
long DpZip, SpZip;
unsigned short SourceExponent;
unsigned char SourceRed, SourceGreen, SourceBlue, SourceCoverage;

// D stands for destination, S is for Source, p is for pixel, o is for origin, p1 is for +1, Int is for interval, Covg is for coverage

if (Cols <= 0 || Rows <= 0 || ! Red || ! Green || ! Blue || ! AABuf || ! rPixelBlock || ! rPixelBlock->FragMap)
	return (0);

DestWt = 1.0 - SourceWt;
if (DestWt < 0.0)
	{
	DestWt = 0.0;
	SourceWt = 1.0;
	} // if

SpZip = Rows * Cols;	// SpZip is an unused variable so we'll use it for the maximum counter limit
if (SourceWt >= 1.0)
	{
	for (DpZip = 0; DpZip < SpZip; DpZip ++)
		{
		rPixelBlock->FragMap[DpZip].CollapsePixel(SourceRed, SourceGreen, SourceBlue, SourceZ, SourceCoverage,
			SourceReflect, SourceNormal, &SourceExponent, FALSE);
		if (SourceCoverage > 0)
			{
			AABuf[DpZip] = SourceCoverage;
			ZBuf[DpZip] = SourceZ;
			ReflectionBuf[DpZip] = SourceReflect;
			NormalBuf[0][DpZip] = SourceNormal[0];
			NormalBuf[1][DpZip] = SourceNormal[1];
			NormalBuf[2][DpZip] = SourceNormal[2];
			Red[DpZip] = SourceRed;
			Green[DpZip] = SourceGreen;
			Blue[DpZip] = SourceBlue;
			ExponentBuf[DpZip] = SourceExponent;
			} // if
		else
			{
			AABuf[DpZip] = 0;
			ZBuf[DpZip] = 0.0f;
			ReflectionBuf[DpZip] = 0.0f;
			NormalBuf[0][DpZip] = 0.0f;
			NormalBuf[1][DpZip] = 0.0f;
			NormalBuf[2][DpZip] = 0.0f;
			Red[DpZip] = 0;
			Green[DpZip] = 0;
			Blue[DpZip] = 0;
			ExponentBuf[DpZip] = 0;
			} // else
		} // for
	} // if simple replace - might as well elliminate the multiplication
else
	{
	for (DpZip = 0; DpZip < SpZip; DpZip ++)
		{
		rPixelBlock->FragMap[DpZip].CollapsePixel(SourceRed, SourceGreen, SourceBlue, SourceZ, SourceCoverage,
			SourceReflect, SourceNormal, &SourceExponent, FALSE);
		if (AABuf[DpZip] > 0)
			{
			if (SourceCoverage > 0)
				{
				AABuf[DpZip] =  (unsigned char)(SourceWt * SourceCoverage + DestWt * AABuf[DpZip]);
				ZBuf[DpZip] = (float)(SourceWt * SourceZ + DestWt * ZBuf[DpZip]);
				ReflectionBuf[DpZip] = (float)(SourceWt * SourceReflect + DestWt * ReflectionBuf[DpZip]);
				NormalBuf[0][DpZip] = (float)(SourceWt * SourceNormal[0] + DestWt * NormalBuf[0][DpZip]);
				NormalBuf[1][DpZip] = (float)(SourceWt * SourceNormal[1] + DestWt * NormalBuf[1][DpZip]);
				NormalBuf[2][DpZip] = (float)(SourceWt * SourceNormal[2] + DestWt * NormalBuf[2][DpZip]);
				if (ExponentBuf[DpZip] || SourceExponent)
					{
					ThreeColorsA[0] = Red[DpZip];
					ThreeColorsA[1] = Green[DpZip];
					ThreeColorsA[2] = Blue[DpZip];
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColorsA, ExponentBuf[DpZip]);
					ThreeColorsB[0] = SourceRed;
					ThreeColorsB[1] = SourceGreen;
					ThreeColorsB[2] = SourceBlue;
					rPixelFragment::ExtractUnclippedExponentialColors(ThreeColorsB, SourceExponent);
					ThreeColorsA[0] = (unsigned long)(SourceWt * ThreeColorsB[0] + DestWt * ThreeColorsA[0]);
					ThreeColorsA[1] = (unsigned long)(SourceWt * ThreeColorsB[1] + DestWt * ThreeColorsA[1]);
					ThreeColorsA[2] = (unsigned long)(SourceWt * ThreeColorsB[2] + DestWt * ThreeColorsA[2]);
					rPixelFragment::ExtractExponentialColors(ThreeColorsA, ExponentBuf[DpZip]);
					Red[DpZip] = (unsigned char)ThreeColorsA[0];
					Green[DpZip] = (unsigned char)ThreeColorsA[1];
					Blue[DpZip] = (unsigned char)ThreeColorsA[2];
					} // if
				else
					{
					} // else
				} // if
			else
				{
				AABuf[DpZip] = (unsigned char)(DestWt * AABuf[DpZip]);
				// destination colors don't change
				} // else
			} // if
		else if (SourceCoverage > 0)
			{
			AABuf[DpZip] =  (unsigned char)(SourceWt * SourceCoverage);
			ZBuf[DpZip] = SourceZ;
			ReflectionBuf[DpZip] = SourceReflect;
			NormalBuf[0][DpZip] = SourceNormal[0];
			NormalBuf[1][DpZip] = SourceNormal[1];
			NormalBuf[2][DpZip] = SourceNormal[2];
			Red[DpZip] = SourceRed;
			Green[DpZip] = SourceGreen;
			Blue[DpZip] = SourceBlue;
			ExponentBuf[DpZip] = SourceExponent;
			} // if
		else
			{
			AABuf[DpZip] = 0;
			// destination colors remain 0
			} // else
		} // for DpZip
	} // else SouceWt < 1

return (1);

} // Raster::CopyRenderMapMerge

/*===========================================================================*/

int Raster::CopyFloatMap(float *Dest, float *Source, long Dw, long Dh, long Sw, long Sh, double SourceWt)
{
double DestWt, IntX, IntY, ScoX, ScoY, ScoXp1, ScoYp1, CovgX, CovgY, SpArea, SpColor, SpW;
long SpX, SpY, SpXp1, SpYp1, DpX, DpY, DpZip, SpZip;

// D stands for destination, S is for Source, p is for pixel, o is for origin, p1 is for +1, Int is for interval, Covg is for coverage

if (Sw <= 0 || Sh <= 0 || Dw <= 0 || Dh <= 0)
	return (0);

DestWt = 1.0 - SourceWt;
if (DestWt < 0.0)
	{
	DestWt = 0.0;
	SourceWt = 1.0;
	} // if

if (Sw == Dw && Sh == Dh)
	{
	SpZip = Sw * Sh;	// SpZip is an unused variable so we'll use it for the maximum counter limit
	if (SourceWt >= 1.0)
		{
		for (DpZip = 0; DpZip < SpZip; DpZip ++)
			{
			Dest[DpZip] = Source[DpZip];
			} // for
		} // if simple replace - might as well elliminate the multiplication
	else
		{
		for (DpZip = 0; DpZip < SpZip; DpZip ++)
			{
			Dest[DpZip] = (float)(Source[DpZip] * SourceWt + Dest[DpZip] * DestWt);
			} // for
		} // else
	} // if same size
else
	{
	IntX = (double)Sw / (double)Dw;
	IntY = (double)Sh / (double)Dh;
	SpArea = IntX * IntY;

	for (DpY = 0, ScoY = 0.0, DpZip = 0, ScoYp1 = IntY; DpY < Dh; DpY ++, ScoY = ScoYp1, ScoYp1 += IntY)
		{
		if (ScoYp1 > Sh)
			ScoYp1 = Sh;
		for (DpX = 0, ScoX = 0.0, ScoXp1 = IntX; DpX < Dw; DpX ++, ScoX = ScoXp1, ScoXp1 += IntX, DpZip ++)
			{
			if (ScoXp1 > Sw)
				ScoXp1 = Sw;
			SpColor = 0.0;
			SpY = quicklongfloor(ScoY);
			for (SpYp1 = SpY + 1; (double)SpY < ScoYp1; SpY ++, SpYp1 ++)
				{
				CovgY = min((double)SpYp1, ScoYp1) - max((double)SpY, ScoY);
				SpX = quicklongfloor(ScoX);
				for (SpXp1 = SpX + 1, SpZip = SpY * Sw + SpX; (double)SpX < ScoXp1; SpX ++, SpXp1 ++, SpZip ++)
					{
					CovgX = min((double)SpXp1, ScoXp1) - max((double)SpX, ScoX);
					SpW = (CovgX * CovgY) / SpArea;
					SpColor += (SpW * Source[SpZip]);
					} // for SpX
				} // for SpY
			Dest[DpZip] = (float)(SpColor * SourceWt + Dest[DpZip] * DestWt);
			} // for DpX
		} // for DpY
	} // else not same size

return (1);

} // Raster::CopyFloatMap

/*===========================================================================*/

int Raster::CreateFoliageBands(void)
{
bool CoveragePreExisting = false;

if (AltFloatMap)
	CoveragePreExisting = true;

if (! AllocFoliageBands())
	return (0);

if (! CoveragePreExisting)
	CreateCoverage();
CreateMidPtSpan();
if (! CreateZOffset())
	return (0);

SetInitFlags(WCS_RASTER_INITFLAGS_FOLIAGELOADED);
return (1);

} // Raster::CreateFoliageBands

/*===========================================================================*/

void Raster::CreateCoverage(void)
{
long Ct, MaxCt;

MaxCt = Rows * Cols;

if (Red && Green && Blue)
	{
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		if (Red[Ct] || Green[Ct] || Blue[Ct])
			AltFloatMap[Ct] = 1.0f;
		} // for
	} // if rgb
else
	{
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		if (Red[Ct])
			AltFloatMap[Ct] = 1.0f;
		} // for
	} // else gray

} // Raster::CreateCoverage

/*===========================================================================*/

int Raster::CreateZOffset(void)
{
double HalfWidth;
USHORT *TreeZSht = NULL, *FiltZSht = NULL;
long x, y, Success = 1;
long Zip;

if (AltByteMap
	&& (TreeZSht = (USHORT *)AppMem_Alloc(Cols * Rows * sizeof (USHORT), APPMEM_CLEAR))
	&& (FiltZSht = (USHORT *)AppMem_Alloc(Cols * Rows * sizeof (USHORT), APPMEM_CLEAR)))
	{
	CopyImageToShort(TreeZSht, Red && Green && Blue, (USHORT)20000);
	HalfWidth = (double)ImageGradient(TreeZSht, (USHORT)20000);
	HalfWidth = (double)BoxFilter(TreeZSht, FiltZSht, (short)(min(10, max(3, Cols / 50)))); // clamp to max of 10. 10/20/06 CXH
	if (HalfWidth = (double)AddLuma(FiltZSht, Red && Green && Blue))
		{
		Zip = 0;
		for (y = 0; y < Rows; y ++)
			{
			for (x = 0; x < Cols; x ++, Zip ++)
				{
				AltByteMap[Zip] = (UBYTE)(255.999 * sin(HalfPi * (FiltZSht[Zip] / HalfWidth)));
				} // for x
			} // for y
		} // if
	} // if Z allocated
else
	{
	Success = 0;
	} // else

if (TreeZSht)
	AppMem_Free(TreeZSht, Cols * Rows * sizeof (USHORT));
if (FiltZSht)
	AppMem_Free(FiltZSht, Cols * Rows * sizeof (USHORT));

return (Success);

} // Raster::CreateZOffset

/*===========================================================================*/

void Raster::CreateMidPtSpan(void)
{
long x, y, Zip, FirstPt, LastPt, RunSumSamples, Samples;
float RunSum;
float *TempFloatArray;

if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT] && RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
	{
	if (Red && Green && Blue)
		{
		for (y = 0, Zip = 0; y < Rows; y ++)
			{
			LastPt = 0;
			FirstPt = -1;
			for (x = 0; x < Cols; x ++, Zip ++)
				{
				if (Red[Zip] || Green[Zip] || Blue[Zip])
					{
					if (FirstPt == -1)
						FirstPt = x;
					LastPt = x;
					} // if 
				} // for x=... 

			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y] = (float)FirstPt;
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] = (float)(LastPt + 1);
			} // for y=... 
		} // if rgb
	else
		{
		for (y = 0, Zip = 0; y < Rows; y ++)
			{
			LastPt = 0;
			FirstPt = -1;
			for (x = 0; x < Cols; x ++, Zip ++)
				{
				if (Red[Zip])
					{
					if (FirstPt == -1)
						FirstPt = x;
					LastPt = x;
					} // if 
				} // for x=... 

			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y] = (float)FirstPt;
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] = (float)(LastPt + 1);
			} // for y=... 
		} // else gray

	if (TempFloatArray = (float *)AppMem_Alloc(Rows * sizeof (float), 0))
		{
		// running average to elliminate high frequency chatter and abrupt changes
		RunSumSamples = Rows > 40 ? Rows / 20: 2;
		RunSum = 0.0f;
		Samples = 0;
		for (y = 0; y <= RunSumSamples; y ++)
			{
			RunSum += RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y];
			Samples ++;
			} // for
		for (y = 0; y < Rows; )
			{
			TempFloatArray[y] = RunSum / Samples;
			if (y - RunSumSamples >= 0)
				{
				RunSum -= RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y - RunSumSamples];
				Samples --;
				} // if
			y ++;
			if (y + RunSumSamples < Rows)
				{
				RunSum += RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y + RunSumSamples];
				Samples ++;
				} // if
			} // for
		for (y = 0; y < Rows; y ++)
			{
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y] = TempFloatArray[y];
			} // for

		RunSum = 0.0f;
		Samples = 0;
		for (y = 0; y <= RunSumSamples; y ++)
			{
			RunSum += RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y];
			Samples ++;
			} // for
		for (y = 0; y < Rows; )
			{
			TempFloatArray[y] = RunSum / Samples;
			if (y - RunSumSamples >= 0)
				{
				RunSum -= RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y - RunSumSamples];
				Samples --;
				} // if
			y ++;
			if (y + RunSumSamples < Rows)
				{
				RunSum += RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y + RunSumSamples];
				Samples ++;
				} // if
			} // for
		for (y = 0; y < Rows; y ++)
			{
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] = TempFloatArray[y];
			} // for


		for (y = 0; y < Rows; y ++)
			{
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] -= RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y];
			RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y] += (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] / (float)2.0);
			} // for

		AppMem_Free(TempFloatArray, Rows * sizeof (float));
		} // if
	} // if

} // Raster::CreateMidPtSpan

/*===========================================================================*/

void Raster::CopyImageToShort(USHORT *DBits, short ColorImage, USHORT FillVal)
{
long x, y, Zip;

for (y = 0, Zip = 0; y < Rows; y ++)
	{
	for (x = 0; x < Cols; x ++, Zip ++)
		{
		if (ColorImage)
			{
			if (Red[Zip] || Green[Zip] || Blue[Zip])
				{
				DBits[Zip] = FillVal;
				} // if
			} // if
		else
			{
			if (Red[Zip])
				{
				DBits[Zip] = FillVal;
				} // if
			} // else
		} // for
	} // for

} // Raster::CopyImageToShort

/*===========================================================================*/

USHORT Raster::AddLuma(USHORT *DBits, short ColorImage)
{
double Luma;
long x, y, Zip;
USHORT MaxAvg = 0;

for (y = 0, Zip = 0; y < Rows; y ++)
	{
	for (x = 0; x < Cols; x ++, Zip ++)
		{
		Luma = ColorImage ? (Red[Zip] + Green[Zip] + Blue[Zip]) / 3: Red[Zip]; //lint !e653
		if (Luma > 0.0)
			{
			Luma = DBits[Zip] + ((Luma - 127.5) / 127.5) * .3 * DBits[Zip];
			if (Luma < 0.0)
				Luma = 0.0;
			DBits[Zip] = (USHORT)Luma;
			if (DBits[Zip] > MaxAvg)
				MaxAvg = DBits[Zip];
			} // if
		} // for
	} // for

return (MaxAvg);

} // Raster::AddLuma

/*===========================================================================*/

USHORT Raster::BoxFilter(USHORT *SBits, USHORT *DBits, short FiltSize)
{
long x, y, Sy, LowX, HighX, LowY, HighY, Zip, SZip, Sum, Values, HFSize;
USHORT MaxAvg = 0;

HFSize = FiltSize / 2;

if (HFSize > 0)
	{
	for (y = 0, Zip = 0; y < Rows; y ++)
		{
		LowY = y - HFSize;
		HighY = y + HFSize;
		if (LowY < 0)
			LowY = 0;
		if (HighY >= Rows)
			HighY = Rows - 1;
		for (x = 0; x < Cols; x ++, Zip ++)
			{
			long deltaX;

			Sum = 0;
			Values = 0;
			LowX = x - HFSize;
			HighX = x + HFSize;
			if (LowX < 0)
				LowX = 0;
			if (HighX >= Cols)
				HighX = Cols - 1;
			deltaX = HighX - LowX;
			for (Sy = LowY; Sy <= HighY; Sy ++)
				{
				long SZip_limit = deltaX;

				//looky here
				SZip = Sy * Cols + LowX;
				SZip_limit += SZip;
				for (; SZip <= SZip_limit; ++SZip)
					{
					if (SBits[SZip])
						{
						Sum += SBits[SZip];
						++Values;
						} // if
					} // for
				} // for
			if (Sum > 0)
				{
				DBits[Zip] = (USHORT)(.5 + (double)Sum / Values);
				if (DBits[Zip] > MaxAvg)
					MaxAvg = DBits[Zip];
				} // if
			} // for
		} // for
	} // if

return (MaxAvg);

} // Raster::BoxFilter

/*===========================================================================*/

USHORT Raster::ImageGradient(USHORT *DBits, USHORT FillVal)
{
long xIn = 0, xOut = Cols - 1, yIn = 0, yOut = Rows - 1, Zip, x, y, MoreToFill = 1;
USHORT NewFill = FillVal, InlineMin, DiagMin, MaxSearch, MaxFill;

MaxSearch = 0;
MaxFill = 3;

while (MoreToFill)
	{
	MoreToFill = 0;
	for (y = yIn; y <= yOut; y ++)
		{
		Zip = y * Cols + xIn;
		for (x = xIn; x <= xOut; x ++, Zip ++)
			{
			if (DBits[Zip] == FillVal)
				{
				InlineMin = 2 + MinInline(DBits, Zip, x, y, MaxSearch);
				DiagMin = 3 + MinDiag(DBits, Zip, x, y, MaxSearch);
				NewFill = min(InlineMin, DiagMin);
				if (NewFill <= MaxFill)
					{
					DBits[Zip] = NewFill; 
					} // if
				MoreToFill = 1;
				} // if
			} // for
		} // for
	xIn ++;
	xOut --;
	yIn ++;
	yOut --;
	MaxSearch += 2;
	MaxFill += 2;
	} // for

return (NewFill);

} // Raster::ImageGradient

/*===========================================================================*/

USHORT Raster::MinInline(USHORT *DBits, long Zip, long x, long y, USHORT MaxSearch)
{
USHORT MinVal = DBits[Zip], NewMin;

if (x > 0)
	{
	NewMin = DBits[Zip - 1];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (x < Cols - 1)
	{
	NewMin = DBits[Zip + 1];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (y > 0)
	{
	NewMin = DBits[Zip - Cols];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);
if (y < Rows - 1)
	{
	NewMin = DBits[Zip + Cols];
	if (NewMin < MinVal && NewMin <= MaxSearch)
		MinVal = NewMin;
	} // if
else
	return (0);

return (MinVal);

} // Raster::MinInline

/*===========================================================================*/

USHORT Raster::MinDiag(USHORT *DBits, long Zip, long x, long y, USHORT MaxSearch)
{
USHORT MinVal = DBits[Zip], NewMin;

if (x > 0)
	{
	if (y > 0)
		{
		NewMin = DBits[Zip - 1 - Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = DBits[Zip - 1 + Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	} // if
if (x < Cols - 1)
	{
	if (y > 0)
		{
		NewMin = DBits[Zip + 1 - Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	if (y < Rows - 1)
		{
		NewMin = DBits[Zip + 1 + Cols];
		if (NewMin < MinVal && NewMin <= MaxSearch)
			MinVal = NewMin;
		} // if
	} // if

return (MinVal);

} // Raster::MinDiag

/*===========================================================================*/

int Raster::CreateSmallerFoliage(void)
{
Raster *Temp = this, *Parent;
long TempRows = Rows / 2, TempCols = Cols / 2, zip, BandSamples = 0;

while (TempCols >= 5 && TempRows >= 5)
	{
	if (Temp->Smaller = new Raster)
		{
		Parent = Temp;
		Temp = Temp->Smaller;
		Temp->Copy(Temp, Parent, 0);
		Temp->Rows = TempRows;
		Temp->Cols = TempCols;
		if (Red && Green && Blue)
			{
			if ((Temp->Red = Temp->AllocByteBand(0)) && (Temp->Green = Temp->AllocByteBand(1)) && (Temp->Blue = Temp->AllocByteBand(2)) && Temp->AllocFoliageBands())
				{
				Temp->ClearByteBand(0);
				Temp->ClearByteBand(1);
				Temp->ClearByteBand(2);
				Temp->DownsampleFoliage(Parent, 1, 0.0, 0.0);
				} // if bands allocated
			else
				return (0);
			} // if rgb
		else
			{
			if ((Temp->Red = Temp->AllocByteBand(0)) && Temp->AllocFoliageBands())
				{
				Temp->ClearByteBand(0);
				Temp->DownsampleFoliage(Parent, 0, 0.0, 0.0);
				} // if bands allocated
			else
				return (0);
			} // else gray
		} // if new raster
	else
		{
		return (0);
		} // else
	TempRows = Temp->Rows / 2;
	TempCols = Temp->Cols / 2;
	} // while need smaller

// use the smallest downsample to compute averages used in texture mapping
AverageCoverage = AverageBand[0] = AverageBand[1] = AverageBand[2] = 0.0;
for (Row = 0, zip = 0; Row < Temp->Rows; Row ++)
	{
	for (Col = 0; Col < Temp->Cols; Col ++, zip ++)
		{
		if (Temp->AltFloatMap[zip] > 0.0)
			{
			if (Temp->Red)
				AverageBand[0] += Temp->Red[zip];
			if (Temp->Green)
				AverageBand[1] += Temp->Green[zip];
			if (Temp->Blue)
				AverageBand[2] += Temp->Blue[zip];
			BandSamples ++;
			} // if
		AverageCoverage += Temp->AltFloatMap[zip];
		} // for
	} // for

if (Temp->ByteBandSize > 0)
	AverageCoverage /= Temp->ByteBandSize;
if (BandSamples > 0)
	{
	if (Temp->Red)
		AverageBand[0] /= (255.0 * BandSamples);
	if (Temp->Green)
		AverageBand[1] /= (255.0 * BandSamples);
	if (Temp->Blue)
		AverageBand[2] /= (255.0 * BandSamples);
	} // if

SetInitFlags(WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED | WCS_RASTER_INITFLAGS_DOWNSAMPLED);
return (1);

} // Raster::CreateSmallerFoliage

/*===========================================================================*/

int Raster::CreateSmallerImage(void)
{
Raster *Temp = this, *Parent;
long TempRows = Rows / 2, TempCols = Cols / 2, zip, BandSamples = 0;

while (TempCols >= 5 && TempRows >= 5)
	{
	if (Temp->Smaller = new Raster)
		{
		Parent = Temp;
		Temp = Temp->Smaller;
		Temp->Copy(Temp, Parent, 0);
		Temp->Rows = TempRows;
		Temp->Cols = TempCols;
		if (Parent->AltFloatMap)
			{
			if (Temp->AllocAltFloatBand(Temp->Rows, Temp->Cols))
				{
				Temp->ClearAltFloatBand();
				} // if
			else
				return (0);
			} // if
		if (Red && Green && Blue)
			{
			if ((Temp->Red = Temp->AllocByteBand(0)) && (Temp->Green = Temp->AllocByteBand(1)) && (Temp->Blue = Temp->AllocByteBand(2)))
				{
				Temp->ClearByteBand(0);
				Temp->ClearByteBand(1);
				Temp->ClearByteBand(2);
				Temp->DownsampleImage(Parent, 1, 0.0, 0.0);
				} // if bands allocated
			else
				return (0);
			} // if rgb
		else
			{
			if (Temp->Red = Temp->AllocByteBand(0))
				{
				Temp->ClearByteBand(0);
				Temp->DownsampleImage(Parent, 0, 0.0, 0.0);
				} // if bands allocated
			else
				return (0);
			} // else gray
		} // if new raster
	else
		{
		return (0);
		} // else
/* for saving a gray-scale downsampled image
	{
	char OutFileName[256];
	long *RowZip, row;

	if (RowZip = (long *)AppMem_Alloc(Temp->Rows * sizeof (long), 0))
		{
		for (row = 0; row < Temp->Rows; row ++)
			RowZip[row] = row * Temp->Cols;
		} // if
	sprintf(OutFileName, "WCSFrames:Image%d.iff", Temp->Rows);
	saveILBM(8, 0, (unsigned char **)&Temp->Red, RowZip, 0, 1, 0, (short)Temp->Cols, (short)Temp->Rows, OutFileName);
	AppMem_Free(RowZip, Temp->Rows * sizeof (long));
	} // temp scope
*/
	TempRows = Temp->Rows / 2;
	TempCols = Temp->Cols / 2;
	} // while need smaller

// use the smallest downsample to compute averages used in texture mapping
AverageCoverage = AverageBand[0] = AverageBand[1] = AverageBand[2] = 0.0;
for (Row = 0, zip = 0; Row < Temp->Rows; Row ++)
	{
	for (Col = 0; Col < Temp->Cols; Col ++, zip ++)
		{
		if (Temp->AltFloatMap)
			{
			if (Temp->AltFloatMap[zip] > 0.0)
				{
				if (Temp->Red)
					AverageBand[0] += Temp->Red[zip];
				if (Temp->Green)
					AverageBand[1] += Temp->Green[zip];
				if (Temp->Blue)
					AverageBand[2] += Temp->Blue[zip];
				BandSamples ++;
				} // if
			AverageCoverage += Temp->AltFloatMap[zip];
			} // if
		else
			{
			if (Temp->Red)
				AverageBand[0] += Temp->Red[zip];
			if (Temp->Green)
				AverageBand[1] += Temp->Green[zip];
			if (Temp->Blue)
				AverageBand[2] += Temp->Blue[zip];
			BandSamples ++;
			} // else
		} // for
	} // for

if (Temp->AltFloatMap)
	AverageCoverage /= Temp->ByteBandSize;
else
	AverageCoverage = 1.0;
if (BandSamples > 0)
	{
	if (Temp->Red)
		AverageBand[0] /= (255.0 * BandSamples);
	if (Temp->Green)
		AverageBand[1] /= (255.0 * BandSamples);
	if (Temp->Blue)
		AverageBand[2] /= (255.0 * BandSamples);
	} // if

SetInitFlags(WCS_RASTER_INITFLAGS_DOWNSAMPLED);
return (1);

} // Raster::CreateSmallerImage

/*===========================================================================*/

void Raster::ComputeAverageBands(void)
{
long Row, Col, zip, BandSamples = 0;

AverageCoverage = AverageBand[0] = AverageBand[1] = AverageBand[2] = 0.0;
for (Row = 0, zip = 0; Row < Rows; Row ++)
	{
	for (Col = 0; Col < Cols; Col ++, zip ++)
		{
		if (AltFloatMap)
			{
			if (AltFloatMap[zip] > 0.0)
				{
				if (Red)
					AverageBand[0] += Red[zip];
				if (Green)
					AverageBand[1] += Green[zip];
				if (Blue)
					AverageBand[2] += Blue[zip];
				BandSamples ++;
				} // if
			AverageCoverage += AltFloatMap[zip];
			} // if
		else
			{
			if (Red)
				AverageBand[0] += Red[zip];
			if (Green)
				AverageBand[1] += Green[zip];
			if (Blue)
				AverageBand[2] += Blue[zip];
			BandSamples ++;
			} // else
		} // for
	} // for

if (AltFloatMap)
	AverageCoverage /= ByteBandSize;
else
	AverageCoverage = 1.0;
if (BandSamples > 0)
	{
	if (Red)
		AverageBand[0] /= (255.0 * BandSamples);
	if (Green)
		AverageBand[1] /= (255.0 * BandSamples);
	if (Blue)
		AverageBand[2] /= (255.0 * BandSamples);
	} // if

} // Raster::ComputeAverageBands

/*===========================================================================*/

// This method needs to be functionally identical to the method used to paste images into a render raster.
// See Image_Paste() in Render.cpp

int Raster::DownsampleFoliage(Raster *Parent, short ColorImage, double Dx, double Dy)
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtx, wty, wt, PixWt, MaxWt, SpanWt;
long Px, Py, Pxp1, Pyp1, x, y, DRows, DCols, DxStart, DyStart, PixVal[4],
	zip, SourceZip, i, j, suby, StartPt[4];
float SpanSum, MidSum;

if (Rows <= 0 || Cols <= 0)
	return (0);

Dox = Dx;
Dex = Dx + Cols;
Doy = Dy;
Dey = Dy + Rows;

dX = (double)Parent->Cols / (double)Cols;
dY = (double)Parent->Rows / (double)Rows;

MaxWt = dX * dY;

Sox = ((int)Dox - Dox) * dX;
Soy = ((int)Doy - Doy) * dY;

DxStart = quickftol(Dox); 
DyStart = quickftol(Doy); 
DCols = quickftol(Dex - DxStart + 1.0);
DRows = quickftol(Dey - DyStart + 1.0);

for (y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0, PixWt = 0.0, PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0;
	j < DRows; j ++, y ++, Coy += dY, Cey += dY)
	{
	if (y < 0)
		continue;
	if (y >= Rows)
		break;
	zip = y * Cols + DxStart;
	SpanSum = 0.0f;
	MidSum = 0.0f;
	SpanWt = 0.0;
	StartPt[0] = -1;
	StartPt[1] = -1;
	StartPt[2] = -1;
	StartPt[3] = -1;
	for (x = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, x ++, Cox += dX, Cex += dX, PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0, PixWt = 0.0, zip ++)
		{
		if (x < 0)
			continue;
		if (x >= Cols)
			break;
		for (Py = quickftol(Coy), Pyp1 = quickftol(Coy + 1), suby = 0; Py < Cey && Py < Parent->Rows; Py ++, Pyp1 ++, suby ++)
			{
			if (Py < 0)
				continue;
			wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
			if (x == DxStart)
				{
				if (Parent->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
					SpanSum += (float)(Parent->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][Py] * wty);
				if (Parent->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
					MidSum += (float)(Parent->RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][Py] * wty);
				SpanWt += wty;
				} // if
			for (Px = (long)Cox, Pxp1 = (long)Cox + 1; Px < Cex && Px < Parent->Cols; Px ++, Pxp1 ++)
				{
				if (Px < 0)
					continue;
				wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
				wt = wtx * wty;
				SourceZip = Py * Parent->Cols + Px;
				if (Parent->AltFloatMap)
					wt *= Parent->AltFloatMap[SourceZip];
				if (ColorImage)
					{
					if (Parent->Red[SourceZip] || Parent->Green[SourceZip] || Parent->Blue[SourceZip])
						{
						PixWt += wt;
						PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)Parent->Red[SourceZip]);
						PixVal[1] = quickftol(PixVal[1] + wt * (unsigned int)Parent->Green[SourceZip]);
						PixVal[2] = quickftol(PixVal[2] + wt * (unsigned int)Parent->Blue[SourceZip]);
						if (Parent->AltByteMap)
							PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)Parent->AltByteMap[SourceZip]);
						if (StartPt[suby] < 0)
							{
							StartPt[suby] = Px;
							} // if 
						} // if 
					} // if 
				else
					{
					if (Parent->Red[SourceZip])
						{
						PixWt += wt;
						PixVal[0] = quickftol(PixVal[0] + wt * (unsigned int)Parent->Red[SourceZip]);
						if (Parent->AltByteMap)
							PixVal[3] = quickftol(PixVal[3] + wt * (unsigned int)Parent->AltByteMap[SourceZip]);
						if (StartPt[suby] < 0)
							{
							StartPt[suby] = Px;
							} // if 
						}
					}
				} // for
			} // for Py

		if (ColorImage)
			{
			if ((PixVal[0] || PixVal[1] || PixVal[2]) && PixWt > 0.0)
				{
				PixVal[0] = quickftol(PixVal[0] / PixWt);
				PixVal[1] = quickftol(PixVal[1] / PixWt);
				PixVal[2] = quickftol(PixVal[2] / PixWt);
				PixVal[3] = quickftol(PixVal[3] / PixWt);
				PixWt /= MaxWt;
				if (PixWt > 1.0)
					PixWt = 1.0;
				Red[zip] = (UBYTE)PixVal[0];
				Green[zip] = (UBYTE)PixVal[1];
				Blue[zip] = (UBYTE)PixVal[2];
				if (AltByteMap)
					AltByteMap[zip] = (UBYTE)PixVal[3];
				AltFloatMap[zip] = (float)PixWt;
				}
			else
				{
				AltFloatMap[zip] = 0.0f;
				} // else 
			} // if 3 colors
		else
			{
			if (PixVal[0] && PixWt > 0.0)
				{
				PixVal[0] = quickftol(PixVal[0] / PixWt);
				PixVal[3] = quickftol(PixVal[3] / PixWt);
				PixWt /= MaxWt;
				if (PixWt > 1.0)
					PixWt = 1.0;
				Red[zip] = (UBYTE)PixVal[0];
				if (AltByteMap)
					AltByteMap[zip] = (UBYTE)PixVal[3];
				AltFloatMap[zip] = (float)PixWt;
				} // if
			else
				{
				AltFloatMap[zip] = 0.0f;
				} // else 
			} // else 
		} // for
	
	SpanSum = (float)(SpanSum / SpanWt);
	MidSum = (float)(MidSum / SpanWt);
	if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN])
		RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_SPAN][y] = (float)((SpanSum * Cols) / Parent->Cols);
	if (RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT])
		RowFloatArray[WCS_RASTER_FOLIAGE_ARRAY_MIDPT][y] = (float)((MidSum * Cols) / Parent->Cols);
	} // for

return (1);

} // Raster::DownsampleFoliage

/*===========================================================================*/

int Raster::DownsampleImage(Raster *Parent, short ColorImage, double Dx, double Dy)
{
double Dox, Doy, Dex, Dey, dX, dY, Sox, Soy, Cox, Coy, Cex, Cey,
	wtx, wty, wt, PixWt, MaxWt, PixVal[4];
long Px, Py, Pxp1, Pyp1, x, y, DRows, DCols, DxStart, DyStart,
	zip, SourceZip, i, j;

if (Rows <= 0 || Cols <= 0)
	return (0);

Dox = Dx;
Dex = Dx + Cols;
Doy = Dy;
Dey = Dy + Rows;

dX = (double)Parent->Cols / (double)Cols;
dY = (double)Parent->Rows / (double)Rows;

MaxWt = dX * dY;

Sox = ((int)Dox - Dox) * dX;
Soy = ((int)Doy - Doy) * dY;

DxStart = quickftol(Dox); 
DyStart = quickftol(Doy); 
DCols = quickftol(Dex - DxStart + 1.0);
DRows = quickftol(Dey - DyStart + 1.0);

for (y = DyStart, Coy = Soy, Cey = Soy + dY, j = 0, PixWt = 0.0, PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0.0;
	j < DRows; j ++, y ++, Coy += dY, Cey += dY)
	{
	if (y < 0)
		continue;
	if (y >= Rows)
		break;
	zip = y * Cols + DxStart;
	for (x = DxStart, Cox = Sox, Cex = Sox + dX, i = 0; i < DCols;
		i ++, x ++, Cox += dX, Cex += dX, PixVal[0] = PixVal[1] = PixVal[2] = PixVal[3] = 0.0, PixWt = 0.0, zip ++)
		{
		if (x < 0)
			continue;
		if (x >= Cols)
			break;
		for (Py = quickftol(Coy), Pyp1 = quickftol(Coy + 1); Py < Cey && Py < Parent->Rows; Py ++, Pyp1 ++)
			{
			if (Py < 0)
				continue;
			// in the new vernacular
			wty = min((double)Pyp1, Cey) - max((double)Py, Coy);
			//wtys = Py > Coy ? 1.0: Pyp1 - Coy;	// this was causing the round pixel anomalies in enlarged images 
			//wtye = Pyp1 < Cey ? 1.0: Cey - Py;
			//wty = wtys * wtye;	// these should not have been multiplied
			for (Px = quickftol(Cox), Pxp1 = quickftol(Cox + 1); Px < Cex && Px < Parent->Cols; Px ++, Pxp1 ++)
				{
				if (Px < 0)
					continue;
				// in the new vernacular
				wtx = min((double)Pxp1, Cex) - max((double)Px, Cox);
				wt = wtx * wty;
				//wtxs = Px > Cox ? 1.0: Pxp1 - Cox;	// this was causing the round pixel anomalies in enlarged images 
				//wtxe = Pxp1 < Cex ? 1.0: Cex - Px;
				//wt = wty * wtxs * wtxe;	// these should not have been multiplied
				SourceZip = Py * Parent->Cols + Px;
				if (Parent->AltFloatMap)
					wt *= Parent->AltFloatMap[SourceZip];
				if (ColorImage)
					{
					if (Parent->Red[SourceZip] || Parent->Green[SourceZip] || Parent->Blue[SourceZip])
						{
						PixWt += wt;
						PixVal[0] += (wt * (unsigned int)Parent->Red[SourceZip]);
						PixVal[1] += (wt * (unsigned int)Parent->Green[SourceZip]);
						PixVal[2] += (wt * (unsigned int)Parent->Blue[SourceZip]);
						} // if 
					} // if 
				else
					{
					if (Parent->Red[SourceZip])
						{
						PixWt += wt;
						PixVal[0] += (wt * (unsigned int)Parent->Red[SourceZip]);
						}
					}
				} // for
			} // for

		if (ColorImage)
			{
			if (PixWt > 0.0)
				{
				PixVal[0] /= PixWt;
				PixVal[1] /= PixWt;
				PixVal[2] /= PixWt;
				PixWt /= MaxWt;
				if (PixWt > 1.0)
					PixWt = 1.0;
				Red[zip] = PixVal[0] > 254.9 ? 255: (UBYTE)PixVal[0];
				Green[zip] = PixVal[1] > 254.9 ? 255: (UBYTE)PixVal[1];
				Blue[zip] = PixVal[2] > 254.9 ? 255: (UBYTE)PixVal[2];
				if (AltFloatMap)
					AltFloatMap[zip] = (float)PixWt;
				}
			else
				{
				if (AltFloatMap)
					AltFloatMap[zip] = 0.0f;
				} // else 
			} // if 3 colors
		else
			{
			if (PixWt > 0.0)
				{
				PixVal[0] /= PixWt;
				PixVal[3] /= PixWt;
				PixWt /= MaxWt;
				if (PixWt > 1.0)
					PixWt = 1.0;
				Red[zip] = PixVal[0] > 254.9 ? 255: (UBYTE)PixVal[0];
				if (AltFloatMap)
					AltFloatMap[zip] = (float)PixWt;
				} // if
			else
				{
				if (AltFloatMap)
					AltFloatMap[zip] = 0.0f;
				} // else 
			} // else 
		} // for x
	} // for y

return (1);

} // Raster::DownsampleImage

/*===========================================================================*/

char Wt[5][5] = {1, 2, 2, 2, 1,  2, 3, 4, 3, 2,  2, 4, 5, 4, 2,  2, 3, 4, 3, 2,  1, 2, 2, 2, 1};

void Raster::BlurByteFloatMaps(char Band)
{
long X, Y, sX, sY, i, j, ByteSum, ByteSumWt, FloatSumWt, Zip, sZip;
float FloatSum;
Raster *Temp;

if (Temp = new Raster)
	{
	Temp->Rows = Rows;
	Temp->Cols = Cols;
	if (Temp->AllocByteBand(0))
		{
		Temp->ClearByteBand(0);
		if (Temp->AllocFloatBand(0))
			{
			Temp->ClearFloatBand(0);
			for (Y = 0, Zip = 0; Y < Rows; Y ++)
				{
				for (X = 0; X < Cols; X ++, Zip ++)
					{
					FloatSumWt = ByteSum = ByteSumWt = 0;
					FloatSum = 0.0f;
					for (sY = Y - 2, j = 0; j < 5; sY ++, j ++)
						{
						if (sY >= 0 && sY < Rows)
							{
							sZip = sY * Cols + X - 1;
							for (sX = X - 2, i = 0; i < 5; sX ++, i ++, sZip ++)
								{
								if (sX >= 0 && sX < Cols)
									{
									if (ByteMap[Band][sZip])
										{
										FloatSum += (FloatMap[Band][sZip] * Wt[j][i]);
										ByteSum += (ByteMap[Band][sZip] * Wt[j][i]);
										FloatSumWt += Wt[j][i];
										} // if
									ByteSumWt += Wt[j][i];
									} // if in bounds
								} // for sX
							} // if in bounds
						} // for sY
					if (ByteSumWt)
						Temp->ByteMap[0][Zip] = (UBYTE)(ByteSum / ByteSumWt);
					if (FloatSumWt)
						Temp->FloatMap[0][Zip] = FloatSum / FloatSumWt;
					} // for X
				} // for Y
			memcpy(ByteMap[Band], Temp->ByteMap[0], ByteBandSize);
			memcpy(FloatMap[Band], Temp->FloatMap[0], FloatBandSize);
			} // if float band
		} // if byte band
	delete Temp;
	} // if Temp Raster

} // Raster::BlurByteFloatMaps

/*===========================================================================*/

int Raster::GetPreppedStatus(void)
{
char filename[256];

return (PAF.GetValidPathAndName(filename) && 
	Rows > 0 && Cols > 0 && 
	(ByteBands > 0 || FloatBands > 0) && Thumb &&
	Thumb->TNail[0] && Thumb->TNail[1] && Thumb->TNail[2]);

} // Raster::GetPreppedStatus

/*===========================================================================*/

int Raster::RemovePartialSequence(SequenceShell *RemoveMe)
{
RasterAttribute *MyAttr;
RasterShell *MyShell;

if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
	{
	if (! (MyShell = MyAttr->RemovePartialSequence(RemoveMe)))
		{
		RemoveAttribute(MyAttr);
		} // if
	else
		{
		((SequenceShell *)MyShell)->AdjustSubsequent();
		} // else
	return (1);
	} // if

return (0);

} // Raster::RemovePartialSequence

/*===========================================================================*/

int Raster::RemovePartialDissolve(DissolveShell *RemoveMe)
{
RasterAttribute *MyAttr;
RasterShell *MyShell;

if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
	{
	if (! (MyShell = MyAttr->RemovePartialDissolve(RemoveMe)))
		{
		RemoveAttribute(MyAttr);
		} // if
	else
		{
		((DissolveShell *)MyShell)->AdjustSubsequent();
		} // else
	return (1);
	} // if

return (0);

} // Raster::RemovePartialDissolve

/*===========================================================================*/

int Raster::AdjustSequenceOrder(SequenceShell *AdjustMe, long Adjustment)
{
RasterAttribute *MyAttr;
RasterShell *MyShell;

if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
	{
	if (MyShell = MyAttr->AdjustSequenceOrder(AdjustMe, Adjustment))
		{
		((SequenceShell *)MyShell)->AdjustSubsequent();
		return (1);
		} // else
	} // if

return (0);

} // Raster::AdjustSequenceOrder

/*===========================================================================*/

int Raster::AdjustDissolveOrder(DissolveShell *AdjustMe, long Adjustment)
{
RasterAttribute *MyAttr;
RasterShell *MyShell;

if (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
	{
	if (MyShell = MyAttr->AdjustDissolveOrder(AdjustMe, Adjustment))
		{
		((DissolveShell *)MyShell)->AdjustSubsequent();
		return (1);
		} // else
	} // if

return (0);

} // Raster::AdjustDissolveOrder

/*===========================================================================*/

char Raster::GetIsColor(void)
{
RasterAttribute *MyAttr;
RasterShell *MyShell;

if (ByteBands > 1 && (MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL)))
	{
	if (MyShell = MyAttr->GetShell())
		{
		if (ColorControlEnabled && ! ((ColorControlShell *)MyShell)->UseAsColor)
			return (0);
		} // if
	} // if
else if (ByteBands == 1)
	return (0);

return (1);

} // Raster::GetIsColor

/*===========================================================================*/

int Raster::SampleByteCell3(unsigned char *Output, long SampleX, long SampleY, int &Abort)
{
unsigned long SampleCell;
SampleCell = SampleY * Cols + SampleX;

// Abort must be preinitialized to 0

// if not loaded, load to render
#ifdef WCS_IMAGE_MANAGEMENT
if (!QueryImageManagementEnabled())
#endif // WCS_IMAGE_MANAGEMENT
	{
	if (! LoadToRender(GlobalApp->MainProj->Interactive->GetActiveTime(), GlobalApp->MainProj->Interactive->GetFrameRate(), WCS_RASTER_INITFLAGS_IMAGELOADED))
		{
		Output[0] = Output[1] = Output[2] = 0;
		Abort = 1;
		return (0);
		} // if
	} // if

// WARNING! No test for valid sample range

#ifdef WCS_IMAGE_MANAGEMENT
if (!QueryImageManagementEnabled())
#endif // WCS_IMAGE_MANAGEMENT
	{
	if (ByteMap[WCS_RASTER_IMAGE_BAND_GREEN] && ByteMap[WCS_RASTER_IMAGE_BAND_BLUE])
		{
		Output[0] = ByteMap[WCS_RASTER_IMAGE_BAND_RED][SampleCell];
		Output[1] = ByteMap[WCS_RASTER_IMAGE_BAND_GREEN][SampleCell];
		Output[2] = ByteMap[WCS_RASTER_IMAGE_BAND_BLUE][SampleCell];
		} // if
	else
		{
		Output[0] = Output[1] = Output[2] = ByteMap[WCS_RASTER_IMAGE_BAND_RED][SampleCell];
		} // else
	} // if
#ifdef WCS_IMAGE_MANAGEMENT
else
	{
	double CellOutput[3];
	// don't care if it's RGB or monochrome, SampleManagedByteDouble3() handles that for us
	SampleManagedByteDouble3(CellOutput, (unsigned)SampleX, (unsigned)SampleY, Abort);
	// we need to add in DBL_EPSILON before converting back to unsigned char to prevent rounding down
	// caused by imperfect precision in doubles for some values, IE:
	// ubyte:132 becomes double:131.99999999... becomes 131
	// correction: we'll just add .5 (so we're rounding) because we don't know that DBL_EPSILON will always be enough
	Output[0] = (unsigned char)((CellOutput[0] * 255.0) + 0.5);
	Output[1] = (unsigned char)((CellOutput[1] * 255.0) + 0.5);
	Output[2] = (unsigned char)((CellOutput[2] * 255.0) + 0.5);
	} // else
#endif // WCS_IMAGE_MANAGEMENT

return (Output[0] || Output[1] || Output[2]);

} // Raster::SampleByteCell3

/*===========================================================================*/

TriStimulus *Raster::SortByteMaps(unsigned long &Elements)
{
TriStimulus *Stim = NULL;
long Ct, LowGreen, HighGreen, LowRed, HighRed, NumGreenToSort, NumBlueToSort;
if (LoadnProcessImage(TRUE))
	{
	if (Stim = new TriStimulus[ByteBandSize])
		{
		for (Ct = 0; Ct < ByteBandSize; Ct ++)
			{
			Stim[Ct].RGB[0] = Red[Ct];
			Stim[Ct].RGB[1] = Green[Ct];
			Stim[Ct].RGB[2] = Blue[Ct];
			} // for
		// sort the entire array by red component
		qsort(&Stim[0], (size_t)ByteBandSize, (size_t)(sizeof (TriStimulus)), RasterSortByRed);
		// sort ranges of equal red value by green component
		for (HighRed = 0; HighRed < ByteBandSize; )
			{
			for (LowRed = HighRed, HighRed ++; HighRed < ByteBandSize; HighRed ++)
				{
				if (Stim[HighRed].RGB[0] != Stim[LowRed].RGB[0])
					break;
				} // for
			if (NumGreenToSort = HighRed - LowRed - 1)
				{
				qsort(&Stim[LowRed], (size_t)NumGreenToSort, (size_t)(sizeof (TriStimulus)), RasterSortByGreen);
				// sort ranges of equal green value by blue component
				for (HighGreen = LowRed; HighGreen < HighRed; )
					{
					for (LowGreen = HighGreen, HighGreen ++; HighGreen < HighRed; HighGreen ++)
						{
						if (Stim[HighGreen].RGB[1] != Stim[LowGreen].RGB[1])
							break;
						} // for
					if (NumBlueToSort = HighGreen - LowGreen - 1)
						{
						qsort(&Stim[LowGreen], (size_t)NumBlueToSort, (size_t)(sizeof (TriStimulus)), RasterSortByBlue);
						} // if
					} // for
				} // if
			} // for
		} // if
	} // if
else
	UserMessageOK("Sort Image Pixels", "Error loading Image Object!");

Elements = ByteBandSize;
return (Stim);

} // Raster::SortByteMaps

/*===========================================================================*/

int RasterSortByRed(const void *elem1, const void *elem2)
{

return (
	((TriStimulus *)elem1)->RGB[0] > ((TriStimulus *)elem2)->RGB[0] ? 1:
	(((TriStimulus *)elem1)->RGB[0] < ((TriStimulus *)elem2)->RGB[0] ? -1: 0)
	);

} // RasterSortByRed

/*===========================================================================*/

int RasterSortByGreen(const void *elem1, const void *elem2)
{

return (
	((TriStimulus *)elem1)->RGB[1] > ((TriStimulus *)elem2)->RGB[1] ? 1:
	(((TriStimulus *)elem1)->RGB[1] < ((TriStimulus *)elem2)->RGB[1] ? -1: 0)
	);

} // RasterSortByGreen

/*===========================================================================*/

int RasterSortByBlue(const void *elem1, const void *elem2)
{

return (
	((TriStimulus *)elem1)->RGB[2] > ((TriStimulus *)elem2)->RGB[2] ? 1:
	(((TriStimulus *)elem1)->RGB[2] < ((TriStimulus *)elem2)->RGB[2] ? -1: 0)
	);

} // RasterSortByBlue

/*===========================================================================*/

// Note: this function is not meant for public release at this time. It does not handle resampling alpha channels.
void Raster::CreateTileableImage(int TileWidth, int TileHeight, double OverlapFraction)
{
double tWeight, WeightInc;
Raster *NewRast;
long TransferCols, TransferRows, tCol, tRow, tZip, fZip;
NotifyTag Changes[2];
char FileName[512], Extension[64];

if (! (TileWidth || TileHeight))
	return;
if (OverlapFraction <= 0.0 || OverlapFraction > .5)
	{
	UserMessageOK("Create Tileable Image", "Overlap fraction must be greater than 0 and less than half.");
	return;
	} // if bad news

// load image if not already loaded
if (! LoadnProcessImage(TRUE))
	{
	UserMessageOK("Create Tileable Image", "Error loading source image.");
	return;
	} // if bad news

if (NewRast = new Raster)
	{
	NewRast->PAF.Copy(&NewRast->PAF, &PAF);
	// modify the extents and allocate
	// new size is smaller by overlapPct
	NewRast->Cols = Cols;
	NewRast->Rows = Rows;
	NewRast->ByteBands = ByteBands;
	if (TileWidth)
		NewRast->Cols = (NewRast->Cols - (long)(NewRast->Cols * OverlapFraction));
	if (TileHeight)
		NewRast->Rows = (NewRast->Rows - (long)(NewRast->Rows * OverlapFraction));
	if (NewRast->Red = NewRast->AllocByteBand(WCS_RASTER_RENDER_BAND_RED))
		{
		if (! Green || (NewRast->Green = NewRast->AllocByteBand(WCS_RASTER_RENDER_BAND_GREEN)))
			{
			if (! Blue || (NewRast->Blue = NewRast->AllocByteBand(WCS_RASTER_RENDER_BAND_BLUE)))
				{
				// transfer data from right to left
				if (TransferCols = Cols - NewRast->Cols)
					{
					WeightInc = 1.0 / TransferCols;

					for (tRow = 0; tRow < Rows; tRow ++)
						{
						tZip = tRow * Cols;
						fZip = Cols - TransferCols + tRow * Cols;
						for (tCol = 0, tWeight = 1.0; tCol < TransferCols; tCol ++, tZip ++, fZip ++, tWeight -= WeightInc)
							{
							Red[tZip] = (unsigned char)(Red[fZip] * tWeight + Red[tZip] * (1.0 - tWeight));
							if (Green)
								Green[tZip] = (unsigned char)(Green[fZip] * tWeight + Green[tZip] * (1.0 - tWeight));
							if (Blue)
								Blue[tZip] = (unsigned char)(Blue[fZip] * tWeight + Blue[tZip] * (1.0 - tWeight));
							} // for
						} // for
					} // if

				// transfer data from bottom to top
				if (TransferRows = Rows - NewRast->Rows)
					{
					WeightInc = 1.0 / TransferRows;

					for (tRow = 0, tWeight = 1.0; tRow < TransferRows; tRow ++, tWeight -= WeightInc)
						{
						tZip = tRow * Cols;
						fZip = (Rows - TransferRows + tRow) * Cols;
						for (tCol = 0; tCol < Cols; tCol ++, tZip ++, fZip ++)
							{
							Red[tZip] = (unsigned char)(Red[fZip] * tWeight + Red[tZip] * (1.0 - tWeight));
							if (Green)
								Green[tZip] = (unsigned char)(Green[fZip] * tWeight + Green[tZip] * (1.0 - tWeight));
							if (Blue)
								Blue[tZip] = (unsigned char)(Blue[fZip] * tWeight + Blue[tZip] * (1.0 - tWeight));
							} // for
						} // for
					} // if

				// copy data into new raster
				for (tRow = 0; tRow < NewRast->Rows; tRow ++)
					{
					tZip = tRow * NewRast->Cols;
					fZip = tRow * Cols;
					for (tCol = 0; tCol < NewRast->Cols; tCol ++, tZip ++, fZip ++)
						{
						NewRast->Red[tZip] = Red[fZip];
						if (Green)
							NewRast->Green[tZip] = Green[fZip];
						if (Blue)
							NewRast->Blue[tZip] = Blue[fZip];
						} // for
					} // for


				// modify file name
				strcpy(FileName, NewRast->PAF.GetName());
				stcgfe(Extension, FileName);
				StripExtension(FileName);
				strcat(FileName, "_Tileable");
				if (TileWidth)
					strcat(FileName, "X");
				if (TileHeight)
					strcat(FileName, "Y");
				AddExtension(FileName, Extension);
				NewRast->PAF.SetName(FileName);

				// save new raster
				NewRast->SaveImage(TRUE);	// save with name in PAF
				NewRast->FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);	// free up the memory

				// modify current raster to use new file and sizes
				FreeAllBands(WCS_RASTER_LONGEVITY_FORCEFREE);
				PAF.Copy(&PAF, &NewRast->PAF);
				LoadnPrepImage(FALSE, FALSE);	// creates new thumbnail and sets new sizes

				// send notification that image has changed
				Changes[0] = MAKE_ID(GetNotifyClass(), GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, GetRAHostRoot());
				} // if blue
			} // if green
		} // if red
	delete NewRast;
	} // if

} // Raster::CreateTileableImage

/*===========================================================================*/

void Raster::OpenPreview(int LoadAsRendered)
{

if (GlobalApp->GUIWins->IVG)
	{
	delete GlobalApp->GUIWins->IVG;
	}
GlobalApp->GUIWins->IVG = new ImageViewGUI(GlobalApp->AppImages, this, 0, LoadAsRendered);
if (GlobalApp->GUIWins->IVG)
	{
	GlobalApp->GUIWins->IVG->Open(GlobalApp->MainProj);
	}

} // Raster::OpenPreview

/*===========================================================================*/

int Raster::AttemptValidatePathAndName(void)
{
#if defined (WCS_BUILD_W6) || defined (WCS_BUILD_V2)
unsigned short TryMe = 1;
char FindName[1024], OrigName[64], Msg[256];
#endif // WCS6 or VNS2
char PathStash[512], BasePath[512];


if (! PAF.GetValidPathAndName(PathStash))
	{
	sprintf(PathStash, "Searching for lost image: %s (originally in %s)", PAF.GetName(), PAF.GetPath());
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, PathStash);
	strcpy(PathStash, PAF.GetPath());

	// search imagepath
	strcpy(BasePath, GlobalApp->MainProj->imagepath);
	PAF.SetPath(BasePath);
	if (PAF.GetValidPathAndName(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			return (1);
		} // if

	// search WCSProjects:EcoModels
	strmfp(BasePath, GlobalApp->MainProj->pcprojectpath, "EcoModels");
	if (PAF.SearchDirectoryTree(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			return (1);
		} // if

#if defined (WCS_BUILD_W6) || defined (WCS_BUILD_V2)
	// Still here?  Look for it in another format
	strcpy(OrigName, PAF.Name);
	while (FileExtension[TryMe])
		{
		strcpy(FindName, OrigName);
		(void)StripExtension(FindName);
		strcat(FindName, FileExtension[TryMe]);
		PAF.SetName(FindName);

		// first search original location
		strcpy(BasePath, PathStash);
		PAF.SetPath(BasePath);
		if (PAF.GetValidPathAndName(BasePath))
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
			if (LoadnPrepImage(FALSE, FALSE))
				{
				sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
				return (1);
				}
			} // if

		// search imagepath
		strcpy(BasePath, GlobalApp->MainProj->imagepath);
		PAF.SetPath(BasePath);
		if (PAF.GetValidPathAndName(BasePath))
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
			if (LoadnPrepImage(FALSE, FALSE))
				{
				sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
				return (1);
				}
			} // if

		// search WCSProjects:EcoModels
		strmfp(BasePath, GlobalApp->MainProj->pcprojectpath, "EcoModels");
		if (PAF.SearchDirectoryTree(BasePath))
			{
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
			if (LoadnPrepImage(FALSE, FALSE))
				{
				sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
				GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
				return (1);
				}
			} // if

		TryMe++;
		} // while

	strcpy(FindName, OrigName);
	(void)StripExtension(FindName);
	strcat(FindName, ".jpg"); // we only do .iff8/.iff24->.iff, .iff->.jpg and .iff->.png conversions in WCSContent/Image, so don't search laboriously for others
	PAF.SetName(FindName);

	// search WCSContent:Image
	strmfp(BasePath, GlobalApp->MainProj->contentpath, "Image");
	if (PAF.SearchDirectoryTree(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			{
			sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
			return (1);
			}
		} // if

	strcpy(FindName, OrigName);
	(void)StripExtension(FindName);
	strcat(FindName, ".png"); // we only do .iff8/.iff24->.iff, .iff->.jpg and .iff->.png conversions in WCSContent/Image, so don't search laboriously for others
	PAF.SetName(FindName);

	// search WCSContent:Image
	strmfp(BasePath, GlobalApp->MainProj->contentpath, "Image");
	if (PAF.SearchDirectoryTree(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			{
			sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
			return (1);
			}
		} // if

	strcpy(FindName, OrigName);
	(void)StripExtension(FindName);
	strcat(FindName, ".iff"); // we only do .iff8/.iff24->.iff, .iff->.jpg and .iff->.png conversions in WCSContent/Image, so don't search laboriously for others
	PAF.SetName(FindName);

	// search WCSContent:Image
	strmfp(BasePath, GlobalApp->MainProj->contentpath, "Image");
	if (PAF.SearchDirectoryTree(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found in another format. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			{
			sprintf(Msg, "Replaced %s with %s", OrigName, FindName);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, Msg);
			return (1);
			}
		} // if


	PAF.SetName(OrigName);
#endif // WCS6 or VNS2

	// search WCSContent:Image for image in original format (for migrations from WCS4)
	strmfp(BasePath, GlobalApp->MainProj->contentpath, "Image");
	if (PAF.SearchDirectoryTree(BasePath))
		{
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found. Testing image...");
		if (LoadnPrepImage(FALSE, FALSE))
			return (1);
		} // if


	// ask user for image
	PAF.SetPath(GlobalApp->MainProj->imagepath);
	if (UserMessageOKCAN((char *)PAF.GetName(), "Invalid image file path or name! File cannot be found.\nSelect a replacement file?"))
		{
		if (PAF.SelectFile())
			{
			strcpy(GlobalApp->MainProj->imagepath, PAF.GetPath());
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, "Image found. Testing image...");
			if (LoadnPrepImage(FALSE, FALSE))
				return (1);
			} // if
		} // if
	} // if
else
	return (1);

GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Image not found. Rendering difficulties may be encountered. The image will be disabled in the Image object Library.");
Enabled = 0;
return (0);

} // Raster::AttemptValidatePathAndName

/*===========================================================================*/

int Raster::SaveImage(int SaveAsNamed)
{
ImageOutputEvent IOEvent;
int Success = 0;
char FileName[256], Ext[32], *Extension, *DefBuf;
BufferNode *CurBuf, *RootBuf;

strcpy(IOEvent.FileType, "IFF");
strcpy(IOEvent.OutBuffers[0], "RED");
strcpy(IOEvent.OutBuffers[1], "GREEN");
strcpy(IOEvent.OutBuffers[2], "BLUE");
if (AABuf && AlphaEnabled)
	strcpy(IOEvent.OutBuffers[3], "ANTIALIAS");
if (DefBuf = ImageSaverLibrary::GetNextCodec(IOEvent.FileType, NULL))
	strcpy(IOEvent.Codec, DefBuf);
IOEvent.PAF.SetPathAndName((char *)PAF.GetPath(), (char *)PAF.GetName());
IOEvent.AutoExtension = 1;
IOEvent.AutoDigits = 0;
strcpy(Ext, WCS_REQUESTER_WILDCARD);

if (SaveAsNamed || GetFileNamePtrn(1, "Save Image", (char *)IOEvent.PAF.GetPath(), (char *)IOEvent.PAF.GetName(), Ext, WCS_PATHANDFILE_NAME_LEN))
	{
	strcpy(FileName, (char *)IOEvent.PAF.GetName());
	if (strcmp(PAF.GetName(), IOEvent.PAF.GetName()))
		{
		PAF.SetName((char *)IOEvent.PAF.GetName());
		} // if
	if (strcmp(PAF.GetPath(), IOEvent.PAF.GetPath()))
		{
		PAF.SetPath((char *)IOEvent.PAF.GetPath());
		} // if
	if (Extension = FindFileExtension(FileName))
		{
		if (DefBuf = ImageSaverLibrary::GetFormatFromExtension(Extension))
			{
			strcpy(IOEvent.FileType, DefBuf);
			IOEvent.AutoExtension = 0;
			} // if
		else
			strcat((char *)PAF.GetName(), ".iff"); // <<<>>> Should this be changed to PNG or something more modern?
		} // if
	else
		strcat((char *)PAF.GetName(), ".iff"); // <<<>>> Should this be changed to PNG or something more modern?

	// create a set of Buffer Nodes
	if (CurBuf = RootBuf = new BufferNode("RED", WCS_RASTER_BANDSET_BYTE))
		{
		if (Red) CurBuf->Buffer = Red;
		else CurBuf->Buffer = ByteMap[0];
		if (CurBuf = CurBuf->AddBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE))
			{
			if (Green) CurBuf->Buffer = Green;
			else CurBuf->Buffer = ByteMap[0];
			if (CurBuf = CurBuf->AddBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE))
				{
				if (Blue) CurBuf->Buffer = Blue;
				else CurBuf->Buffer = ByteMap[0];
				if ((AABuf && AlphaEnabled) && (CurBuf = CurBuf->AddBufferNode("ANTIALIAS", WCS_RASTER_BANDSET_BYTE)))
					CurBuf->Buffer = AABuf;

				// this sets up some necessary format-specific allocations
				// <<<>>> first arg would be a renderer to pass along to image savers
				// so-inclined to use one.
				IOEvent.InitSequence(NULL, RootBuf, Cols, Rows);

				// prep to save
				for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
					{
					if (CurBuf->Buffer)
						{
						CurBuf->PrepToSave(NULL, 0, Cols, 0);
						} // if
					} // for

				// Save it
				// <<<>>> first arg would be a renderer to pass along to image savers
				// so-inclined to use one.
				Success = IOEvent.SaveImage(NULL, RootBuf, Cols, Rows, 0, NULL);

				// Cleanup all prep work
				for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
					{
					if (CurBuf->Buffer)
						{
						CurBuf->CleanupFromSave();
						} // if
					} // for
				} // if
			} // if

		while (RootBuf)
			{
			CurBuf = RootBuf->Next;
			delete RootBuf;
			RootBuf = CurBuf;
			} // if

		} // if
	} // if file name

return (Success);

} // Raster::SaveImage

/*===========================================================================*/

unsigned long Raster::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, unsigned long LoadFlags, int Validate)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
RasterAttribute *CurrentAttrib;

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
					case WCS_RASTER_ID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA:
						{
						BytesRead = LoadData(ffile, Size, ByteFlip);
						break;
						}
					case WCS_RASTER_NAME:
						{
						if (LoadFlags & WCS_RASTER_FILEFLAGS_BASIC)
							{
							BytesRead = PAF.Load(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_TNAIL:	// version 4 flavor
						{
						if (LoadFlags & WCS_RASTER_FILEFLAGS_TNAIL)
							{
							BytesRead = LoadTNail(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_THUMBNAIL:	// new, improved for v5
						{
						if (LoadFlags & WCS_RASTER_FILEFLAGS_TNAIL)
							{
							if (Thumb || (Thumb = new Thumbnail()))
								BytesRead = Thumb->Load(ffile, Size, ByteFlip);
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_ATTRIB:
						{
						if ((LoadFlags & WCS_RASTER_FILEFLAGS_ATTRIBUTE) && (CurrentAttrib = AddAttribute()))
							{
							BytesRead = CurrentAttrib->Load(ffile, Size, ByteFlip, this);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_BANDDATA:
						{
						if (LoadFlags & WCS_RASTER_FILEFLAGS_BANDDATA)
							{
							BytesRead = LoadBandData(ffile, Size, ByteFlip);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_SMALLER:
						{
						if ((LoadFlags & WCS_RASTER_FILEFLAGS_SMALLER) && (Smaller = new Raster))
							{
							BytesRead = Smaller->Load(ffile, Size, ByteFlip, LoadFlags, 0);	// 0 = no validate path);
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_AVGCOVG:
						{
						BytesRead = ReadBlock(ffile, (char *)&AverageCoverage, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_AVGRED:
						{
						BytesRead = ReadBlock(ffile, (char *)&AverageBand[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_AVGGREEN:
						{
						BytesRead = ReadBlock(ffile, (char *)&AverageBand[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_AVGBLUE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AverageBand[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
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

if (Validate)
	AttemptValidatePathAndName();

return (TotalRead);

} // Raster::Load

/*===========================================================================*/

unsigned long Raster::LoadData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_RASTER_DATA_USERNAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_ROWS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Rows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_COLS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Cols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_BYTEBANDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&ByteBands, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_FLOATBANDS:
						{
						BytesRead = ReadBlock(ffile, (char *)&FloatBands, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_FORMATLASTLOADED:
						{
						BytesRead = ReadBlock(ffile, (char *)&FormatLastLoaded, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_SAVEFORMAT:
						{
						BytesRead = ReadBlock(ffile, (char *)&SaveFormat, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_LONGEVITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&Longevity, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_ENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&Enabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_SEQUENCEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&SequenceEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_DISSOLVEENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&DissolveEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_COLORCONTROLENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ColorControlEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_IMAGEMANAGERENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageManagerEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_LOADFAST:
						{
						BytesRead = ReadBlock(ffile, (char *)&LoadFast, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_ALPHAAVAILABLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlphaAvailable, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_ALPHAENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&AlphaEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_IMAGECAPABILITY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ImageCapabilityFlags, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_NATIVETILEWIDTH:
						{
						BytesRead = ReadBlock(ffile, (char *)&NativeTileWidth, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_NATIVETILEHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NativeTileHeight, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_DATA_TNAILPADX:
						{
						if (Thumb || (Thumb = new Thumbnail()))
							BytesRead = ReadBlock(ffile, (char *)&Thumb->TNailPadX, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_RASTER_DATA_TNAILPADY:
						{
						if (Thumb || (Thumb = new Thumbnail()))
							BytesRead = ReadBlock(ffile, (char *)&Thumb->TNailPadY, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
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

} // Raster::LoadData

/*===========================================================================*/

unsigned long Raster::LoadTNail(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char Band = -1;
//unsigned char *TempBand;

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
      case WCS_RASTER_TNAIL_BANDNUM:
       {
       BytesRead = ReadBlock(ffile, (char *)&Band, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_TNAIL_BYTEDATA:
       {
       if (Band >= 0 && (Size == WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE) && (Thumb->TNail[Band] || Thumb->AllocTNail(Band)))
        {
        BytesRead = ReadLongBlock(ffile, (char *)Thumb->TNail[Band], Size);
		} // if memory
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

} // Raster::LoadTNail

/*===========================================================================*/

unsigned long Raster::LoadBandData(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char Band = -1;
unsigned long Ct, BandSize = Rows * Cols;

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
      case WCS_RASTER_BANDDATA_BANDNUM:
       {
       BytesRead = ReadBlock(ffile, (char *)&Band, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_BANDDATA_BYTEDATA:
       {
       if (Band >= 0 && (Size == BandSize) && AllocByteBand(Band))
        {
        BytesRead = ReadLongBlock(ffile, (char *)ByteMap[Band], Size);
		} // if memory
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_BANDDATA_ALTBYTEDATA:
       {
       if (Band >= 0 && ((Size == (ULONG)AltByteBandSize && AltByteMap) || ((Size == BandSize) && AllocAltByteBand(Rows, Cols))))
        {
        BytesRead = ReadLongBlock(ffile, (char *)AltByteMap, Size);
		} // if memory
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_BANDDATA_FLOATDATA:
       {
       if (Band >= 0 && (Size == BandSize * sizeof (float)) && AllocFloatBand(Band))
        {
        if (BytesRead = ReadLongBlock(ffile, (char *)FloatMap[Band], Size))
	     {
		 if (ByteFlip)
		  {
		  for (Ct = 0; Ct < BandSize; Ct ++)
		   {
           SimpleEndianFlip32F((void *)&FloatMap[Band][Ct], (float *)&FloatMap[Band][Ct]);
           } // for
		  } // if
		 } // if
		} // if memory
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_BANDDATA_ALTFLOATDATA:
       {
       if (Band >= 0 && (Size == BandSize * sizeof (float)) && AllocAltFloatBand(Rows, Cols))
        {
        if (BytesRead = ReadLongBlock(ffile, (char *)AltFloatMap, Size))
	     {
		 if (ByteFlip)
		  {
		  for (Ct = 0; Ct < BandSize; Ct ++)
		   {
           SimpleEndianFlip32F((void *)&AltFloatMap[Ct], (float *)&AltFloatMap[Ct]);
           } // for
		  } // if
		 } // if
		} // if memory
	   else if (! fseek(ffile, Size, SEEK_CUR))
        BytesRead = Size;
       break;
	   }
      case WCS_RASTER_BANDDATA_ROWFLOATDATA:
       {
       if (Band >= 0 && (Size == Rows * sizeof (float)) && AllocRowFloatArray(Band))
        {
        if (BytesRead = ReadLongBlock(ffile, (char *)RowFloatArray[Band], Size))
	     {
		 if (ByteFlip)
		  {
		  for (Ct = 0; Ct < (unsigned long)Rows; Ct ++)
		   {
           SimpleEndianFlip32F((void *)&RowFloatArray[Band][Ct], (float *)&RowFloatArray[Band][Ct]);
           } // for
		  } // if
		 } // if
		} // if memory
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

} // Raster::LoadBandData

/*===========================================================================*/

unsigned long Raster::Save(FILE *ffile, unsigned long SaveFlags)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char BandType, CurrentBand;
RasterAttribute *CurrentAttrib;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  ItemTag = WCS_RASTER_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
  if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
   {
   TotalWritten += BytesWritten;

   ItemTag = 0;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    if (BytesWritten = SaveData(ffile, SaveFlags))
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
     } /* if raster data saved */
    else
     goto WriteError;
    } /* if size written */
   else
    goto WriteError;
   } /* if tag written */
  else
   goto WriteError;

  if (SaveFlags & WCS_RASTER_FILEFLAGS_BASIC)
   {
   ItemTag = WCS_RASTER_NAME + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     if (BytesWritten = PAF.Save(ffile))
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
      } /* if illumination effect saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } // if save basic data

  if ((SaveFlags & WCS_RASTER_FILEFLAGS_TNAIL) && Thumb && Thumb->TNailsValid())
   {
   ItemTag = WCS_RASTER_THUMBNAIL + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     if (BytesWritten = Thumb->Save(ffile))
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
      } /* if thumbnail saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } // if TNail

  if (SaveFlags & WCS_RASTER_FILEFLAGS_ATTRIBUTE)
   {
   CurrentAttrib = Attr;
   while (CurrentAttrib)
    {
    if (CurrentAttrib->Shell)
     {
	 if (CurrentAttrib->Shell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE ||
		CurrentAttrib->Shell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE ||
		CurrentAttrib->Shell->GetType() == WCS_RASTERSHELL_TYPE_COLORCONTROL ||
		CurrentAttrib->Shell->GetType() == WCS_RASTERSHELL_TYPE_IMAGEMANAGER ||
		CurrentAttrib->Shell->GetType() == WCS_RASTERSHELL_TYPE_GEOREF)
      {
      ItemTag = WCS_RASTER_ATTRIB + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
      if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
       {
       TotalWritten += BytesWritten;

       ItemTag = 0;
       if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
        {
        TotalWritten += BytesWritten;

        if (BytesWritten = CurrentAttrib->Save(ffile))
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
         } /* if attribute saved */
        else
         goto WriteError;
        } /* if size written */
       else
        goto WriteError;
       } /* if tag written */
      else
       goto WriteError;
      } // if an interesting attribute - other attributes will be saved with effects
	 } // if shell exists
    CurrentAttrib = CurrentAttrib->Next;
    } // while
   } // if save attributes

  if (SaveFlags & WCS_RASTER_FILEFLAGS_BANDDATA)
   {
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_AVGCOVG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AverageCoverage)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_AVGRED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AverageBand[0])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_AVGGREEN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AverageBand[1])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_AVGBLUE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&AverageBand[2])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;

   for (BandType = 0; BandType < WCS_RASTER_BANDSET_MAXTYPES; BandType ++)
    {
	for (CurrentBand = 0; CurrentBand < WCS_RASTER_MAX_BANDS; CurrentBand ++)
     {
	 if ((BandType == WCS_RASTER_BANDSET_BYTE && ByteMap[CurrentBand]) || 
		(BandType == WCS_RASTER_BANDSET_FLOAT && FloatMap[CurrentBand]))
      {
      ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
      if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
       {
       TotalWritten += BytesWritten;

       ItemTag = 0;
       if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
        {
        TotalWritten += BytesWritten;

        if (BytesWritten = SaveBandData(ffile, CurrentBand, BandType))
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
         } /* if band data saved */
        else
         goto WriteError;
        } /* if size written */
       else
        goto WriteError;
       } /* if tag written */
      else
       goto WriteError;
      } // if
     } // for
    } // for

   for (CurrentBand = 0; CurrentBand < 2; CurrentBand ++)
    {
    if (RowFloatArray[CurrentBand])
     {
     ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
     if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
      {
      TotalWritten += BytesWritten;

      ItemTag = 0;
      if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
       {
       TotalWritten += BytesWritten;

       if (BytesWritten = SaveBandData(ffile, CurrentBand, WCS_RASTER_BANDSET_ROWFLOAT))
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
        } // if band data saved
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

   if (AltByteMap)
    {
    ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     ItemTag = 0;
     if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
      {
      TotalWritten += BytesWritten;

      if (BytesWritten = SaveBandData(ffile, 0, WCS_RASTER_BANDSET_ALTBYTE))
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
       } // if band data saved
      else
       goto WriteError;
      } // if size written
     else
      goto WriteError;
     } // if tag written
    else
     goto WriteError;
    } // if

   if (AltFloatMap)
    {
    ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     ItemTag = 0;
     if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
      {
      TotalWritten += BytesWritten;

      if (BytesWritten = SaveBandData(ffile, 0, WCS_RASTER_BANDSET_ALTFLOAT))
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
       } // if band data saved
      else
       goto WriteError;
      } // if size written
     else
      goto WriteError;
     } // if tag written
    else
     goto WriteError;
    } // if
   } // if Save Bands

  if ((SaveFlags & WCS_RASTER_FILEFLAGS_SMALLER) && Smaller)
   {
   ItemTag = WCS_RASTER_SMALLER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     if (BytesWritten = Smaller->Save(ffile, SaveFlags))
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
      } // if smaller image saved
     else
      goto WriteError;
     } // if size written
    else
     goto WriteError;
    } // if tag written
   else
    goto WriteError;
   } // if Smaller

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag, WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	  goto WriteError;
  TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // Raster::Save

/*===========================================================================*/

ULONG Raster::SaveData(FILE *ffile, unsigned long SaveFlags)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_ROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Rows)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_COLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Cols)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_BYTEBANDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ByteBands)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_FLOATBANDS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FloatBands)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if (SaveFlags & WCS_RASTER_FILEFLAGS_BASIC) 
   {
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_USERNAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_FORMATLASTLOADED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&FormatLastLoaded)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_SAVEFORMAT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SaveFormat)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_LONGEVITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Longevity)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_ENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Enabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_SEQUENCEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SequenceEnabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_DISSOLVEENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DissolveEnabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_COLORCONTROLENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ColorControlEnabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_IMAGEMANAGERENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ImageManagerEnabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_LOADFAST, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LoadFast)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_ALPHAAVAILABLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlphaAvailable)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_ALPHAENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&AlphaEnabled)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_IMAGECAPABILITY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ImageCapabilityFlags)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_NATIVETILEWIDTH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NativeTileWidth)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_NATIVETILEHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NativeTileHeight)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if Save Basic

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // Raster::SaveData

/*===========================================================================*/

ULONG Raster::SaveBandData(FILE *ffile, char Band, char BandType)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_BANDDATA_BANDNUM, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Band)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if (BandType == WCS_RASTER_BANDSET_BYTE)
   {
   if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_RASTER_BANDDATA_BYTEDATA, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, ByteBandSize,
	WCS_BLOCKTYPE_CHAR, (char *)ByteMap[Band])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if byte band
  else if (BandType == WCS_RASTER_BANDSET_FLOAT)
   {
   if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_RASTER_BANDDATA_FLOATDATA, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, FloatBandSize,
	WCS_BLOCKTYPE_CHAR, (char *)FloatMap[Band])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if byte band
  else if (BandType == WCS_RASTER_BANDSET_ROWFLOAT)
   {
   if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_RASTER_BANDDATA_ROWFLOATDATA, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, Rows * sizeof (float),
	WCS_BLOCKTYPE_CHAR, (char *)RowFloatArray[Band])) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if byte band
  else if (BandType == WCS_RASTER_BANDSET_ALTBYTE)
   {
   if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_RASTER_BANDDATA_ALTBYTEDATA, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, AltByteBandSize,
	WCS_BLOCKTYPE_CHAR, (char *)AltByteMap)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if byte band
  else if (BandType == WCS_RASTER_BANDSET_ALTFLOAT)
   {
   if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_RASTER_BANDDATA_ALTFLOATDATA, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, AltFloatBandSize,
	WCS_BLOCKTYPE_CHAR, (char *)AltFloatMap)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if byte band

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // Raster::SaveBandData

/*===========================================================================*/

int Raster::SaveFoliage(void)
{
char FileName[256], FileType[12], StrBuf[12];
char Version = WCS_FOLIAGE_VERSION;
char Revision = WCS_FOLIAGE_REVISION;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;
FILE *ffile = NULL;

if (PAF.GetPathAndName(FileName))
	{
	StripExtension(FileName);
	strcat(FileName, ".wfl");
	if (ffile = PROJ_fopen(FileName, "wb"))
		{
		strcpy(FileType, "WCS File");

		// no tags or sizes for first four items: file descriptor, version, revision & byte order
		if ((BytesWritten = WriteBlock(ffile, (char *)FileType,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		memset(StrBuf, 0, 9);
		strcpy(StrBuf, "Foliage");

		if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = FoliageRaster_Save(ffile))
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
					} /* if Paths saved */
				else
					goto WriteError;
				} /* if size written */
			else
				goto WriteError;
			} /* if tag written */
		else
			goto WriteError;

		fclose(ffile);
		} // if file
	else
		{
		sprintf(FileName, "Foliage %s", PAF.GetName());
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, FileName);
		return (0);
		} // else
	} // if name
else
	return (0);

return (1);

WriteError:

fclose(ffile);
sprintf(FileName, "Foliage, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, FileName);

return (0);

} // Raster::SaveFoliage

/*===========================================================================*/

unsigned long Raster::FoliageRaster_Save(FILE *ffile)
{
char Version = WCS_IMAGES_VERSION, Revision = WCS_IMAGES_REVISION;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;

if (ffile)
	{
	// no tags or sizes for first three items: version, revision & byte order
	if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_IMAGES_RASTER + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = Save(ffile, WCS_RASTER_FILEFOLIAGE))
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
				} /* if raster saved */
			else
				goto WriteError;
			} /* if size written */
		else
			goto WriteError;
		} /* if tag written */
	else
		goto WriteError;

	ItemTag = WCS_PARAM_DONE;
	if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} /* if file */

return (TotalWritten);

WriteError:

return (0L);

} // Raster::FoliageRaster_Save

/*===========================================================================*/

int Raster::LoadFoliage(void)
{
char FileName[256], Title[12], ReadBuf[32];
char Version, Revision;
short ByteFlip;
FILE *ffile;
ULONG Size, BytesRead, TotalRead = 0, ByteOrder;
int Success = 0;

if (PAF.GetPathAndName(FileName))
	{
	StripExtension(FileName);
	strcat(FileName, ".wfl");
	if (ffile = PROJ_fopen(FileName, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, ffile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Version, &ReadBuf[8], 1);
		memcpy(&Revision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (Version < WCS_FOLIAGE_VERSION)
			goto ReadError;

		if (ByteOrder == 0xaabbccdd)
			ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			ByteFlip = 1;
		else
			goto ReadError;

		Title[11] = '\0';
		if (! strnicmp(Title, "WCS File", 8))
			{
			TotalRead = BytesRead = 14;

			while (BytesRead)
				{
				/* read block descriptor tag from file */
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
						if (! strnicmp(ReadBuf, "Foliage", 8))
							{
							if (BytesRead = FoliageRaster_Load(ffile, Size))
								Success = 1;
							} // else if images
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TotalRead += BytesRead;
						if (BytesRead != Size)
							{
							break;
							} // if error
						} // if size block read 
					else
						break;
					} // if tag block read 
				else
					break;
				} // while 

			} // if WCS File
		fclose(ffile);
		} // if file
	else
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, FileName);
		return (0);
		} // else
	} // if name

if (Success)
	SetInitFlags(WCS_RASTER_INITFLAGS_FOLIAGELOADED | WCS_RASTER_INITFLAGS_FOLIAGEDOWNSAMPLED | WCS_RASTER_INITFLAGS_DOWNSAMPLED);
return (Success);

ReadError:

fclose(ffile);
sprintf(FileName, "Foliage, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_READ_FAIL, FileName);
return (0);

} // Raster::LoadFoliage

/*===========================================================================*/

ULONG Raster::FoliageRaster_Load(FILE *ffile, ULONG ReadSize)
{
char Version, Revision;
static char StructBuf[24];  // Big enuf to hold anything we fread...
short ByteFlip;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, ByteOrder;
union MultiVal MV;
RasterAttribute *MyAttr;
ColorControlShell *ColorShell;
Raster *Temp;

if (! ffile)
	{
	return (0);
	} // if need to open file

// header data is 6 bytes
if ((fread((char *)StructBuf, 6, 1, ffile)) != 1)
	return (0);

Version = StructBuf[0];
Revision = StructBuf[1];
memcpy(&ByteOrder, &StructBuf[2], 4);

// Endian flop if necessary

if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

TotalRead = 6;
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
					case WCS_IMAGES_RASTER:
						{
						BytesRead = Load(ffile, Size, ByteFlip, WCS_RASTER_FILEFOLIAGE, 0);
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

// we don't want to save band assignments in the file - user should be able to change them at will

if (GetIsColor())
	{
	if (ByteMap[0] && ByteMap[1] && ByteMap[2])
		{
		if ((MyAttr = MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL)) && (ColorShell = (ColorControlShell *)MyAttr->GetShell()))
			{
			if (ColorShell->UseBandAssignment)
				{
				Red = ByteMap[ColorShell->UseBandAs[0]];
				Green = ByteMap[ColorShell->UseBandAs[1]];
				Blue = ByteMap[ColorShell->UseBandAs[2]];
				} // if
			else
				{
				Red = ByteMap[0];
				Green = ByteMap[1];
				Blue = ByteMap[2];
				} // else
			} // if color control
		else
			{
			Red = ByteMap[0];
			Green = ByteMap[1];
			Blue = ByteMap[2];
			} // else
		} // if rgb bands exist
	else
		{
		return (0);
		} // else if
	} // if rgb
else
	{
	if (ByteMap[0] && ! ByteMap[1] && ! ByteMap[2])
		{
		Red = ByteMap[0];
		} // if only red band exists
	else
		{
		return (0);
		} // else
	} // else if
if (Temp = Smaller)
	{
	while (Temp)
		{
		if (GetIsColor() && Temp->ByteMap[0] && Temp->ByteMap[1] && Temp->ByteMap[2])
			{
			Temp->Red = Temp->ByteMap[0];
			Temp->Green = Temp->ByteMap[1];
			Temp->Blue = Temp->ByteMap[2];
			} // if
		else if (! GetIsColor() && Temp->ByteMap[0])
			{
			Temp->Red = ByteMap[0];
			} // else if
		else
			{
			return (0);
			} // else
		Temp = Temp->Smaller;
		} // while
	} // if

return (TotalRead);

} // Raster::FoliageRaster_Load

/*===========================================================================*/

int Raster::LoadShadow3D(char *SuppliedName, ShadowMap3D *SM3D)
{
char FileName[256], Title[12], ReadBuf[32];
char Version, Revision;
short ByteFlip;
FILE *ffile;
ULONG Size, BytesRead, TotalRead = 0, ByteOrder;
int Success = 0;

if (SuppliedName)
	{
	strmfp(FileName, GlobalApp->MainProj->dirname, SuppliedName);
	strcat(FileName, ".wsm");
	if (ffile = PROJ_fopen(FileName, "rb"))
		{
		fread((char *)ReadBuf, 14, 1, ffile);

		memcpy(Title, &ReadBuf[0], 8);
		memcpy(&Version, &ReadBuf[8], 1);
		memcpy(&Revision, &ReadBuf[9], 1);
		memcpy(&ByteOrder, &ReadBuf[10], 4);

		if (Version != WCS_PROJECT_CURRENT_VERSION)
			goto ReadError;

		if (ByteOrder == 0xaabbccdd)
			ByteFlip = 0;
		else if (ByteOrder == 0xddccbbaa)
			ByteFlip = 1;
		else
			goto ReadError;

		Title[11] = '\0';
		if (! strnicmp(Title, "WCS File", 8))
			{
			TotalRead = BytesRead = 14;

			while (BytesRead)
				{
				/* read block descriptor tag from file */
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
						if (! strnicmp(ReadBuf, "Shadow3D", 8))
							{
							if (BytesRead = ShadowRaster3D_Load(ffile, Size, SM3D))
								Success = 1;
							} // else if images
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						TotalRead += BytesRead;
						if (BytesRead != Size)
							{
							break;
							} // if error
						} // if size block read 
					else
						break;
					} // if tag block read 
				else
					break;
				} // while 

			} // if WCS File
		fclose(ffile);
		} // if file
	else
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, FileName);
		return (0);
		} // else
	} // if name

return (Success);

ReadError:

fclose(ffile);
sprintf(FileName, "Shadow Map, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_READ_FAIL, FileName);
return (0);

} // Raster::LoadShadow3D

/*===========================================================================*/

ULONG Raster::ShadowRaster3D_Load(FILE *ffile, ULONG ReadSize, ShadowMap3D *SM3D)
{
char Version, Revision, Band;
static char StructBuf[24];  // Big enuf to hold anything we fread...
short ByteFlip;
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0, ByteOrder;
union MultiVal MV;

if (! ffile)
	{
	return (0);
	} // if need to open file

// header data is 6 bytes
if ((fread((char *)StructBuf, 6, 1, ffile)) != 1)
	return (0);

Version = StructBuf[0];
Revision = StructBuf[1];
memcpy(&ByteOrder, &StructBuf[2], 4);

if (Version != WCS_IMAGES_VERSION)
	return (0);

// Endian flop if necessary

if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

TotalRead = 6;
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
					case WCS_RASTER_DATA:
						{
						BytesRead = LoadShadowData3D(ffile, Size, ByteFlip, SM3D);
						break;
						}
					case WCS_RASTER_BANDDATA:
						{
						BytesRead = LoadBandData(ffile, Size, ByteFlip);
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

SM3D->AABuf = NULL;
SM3D->ZBuf = NULL;
for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
	{
	if (FloatMap[Band])
		{
		SM3D->ZBuf = FloatMap[Band];
		if (SM3D->AABuf)
			break;
		} // if
	if (ByteMap[Band])
		{
		SM3D->AABuf = ByteMap[Band];
		if (SM3D->ZBuf)
			break;
		} // if
	} // for

if (! SM3D->AABuf || ! SM3D->ZBuf)
	return (0);

return (TotalRead);

} // Raster::ShadowRaster3D_Load

/*===========================================================================*/

unsigned long Raster::LoadShadowData3D(FILE *ffile, unsigned long ReadSize, short ByteFlip, ShadowMap3D *SM3D)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
long x = 0, y = 0;

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
      case WCS_RASTER_DATA_ROWS:
       {
       BytesRead = ReadBlock(ffile, (char *)&Rows, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DATA_COLS:
       {
       BytesRead = ReadBlock(ffile, (char *)&Cols, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_TYPE:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->MyType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_VPX:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->VP.XYZ[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_VPY:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->VP.XYZ[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_VPZ:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->VP.XYZ[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_OBJX:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->RefPos.XYZ[0], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_OBJY:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->RefPos.XYZ[1], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_OBJZ:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->RefPos.XYZ[2], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_DISTOFFSET:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->DistanceOffset, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_CENTERX:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->CenterX, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_CENTERY:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->CenterY, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_HORSCALE:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->HorScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_ZOFFSET:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->ZOffset, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_SM3D_DATA_SOLARROTMATX:
       {
       BytesRead = ReadBlock(ffile, (char *)&SM3D->SolarRotMatx[y][x], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
	   x ++;
	   if (x >= 3)
		{
	    y ++;
		x = 0;
		} // if
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

} // Raster::LoadShadowData3D

/*===========================================================================*/

int Raster::SaveShadow3D(char *SuppliedName, ShadowMap3D *SM3D)
{
char FileName[256], FileType[12], StrBuf[12];
char Version = WCS_PROJECT_CURRENT_VERSION;
char Revision = WCS_PROJECT_CURRENT_REVISION;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;
FILE *ffile = NULL;

if (SuppliedName)
	{
	strmfp(FileName, GlobalApp->MainProj->dirname, SuppliedName);
	strcat(FileName, ".wsm");
	if (ffile = PROJ_fopen(FileName, "wb"))
		{
		strcpy(FileType, "WCS File");

		// no tags or sizes for first four items: file descriptor, version, revision & byte order
		if ((BytesWritten = WriteBlock(ffile, (char *)FileType,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
			WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;

		memset(StrBuf, 0, 9);
		strcpy(StrBuf, "Shadow3D");

		if (BytesWritten = WriteBlock(ffile, (char *)StrBuf,
			WCS_BLOCKSIZE_DOUBLE + WCS_BLOCKTYPE_CHAR))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = ShadowRaster3D_Save(ffile, SM3D))
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
					} /* if Paths saved */
				else
					goto WriteError;
				} /* if size written */
			else
				goto WriteError;
			} /* if tag written */
		else
			goto WriteError;

		fclose(ffile);
		} // if file
	else
		{
		sprintf(FileName, "Shadow Map %s", SuppliedName);
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, FileName);
		return (0);
		} // else
	} // if name
else
	return (0);

return (1);

WriteError:

fclose(ffile);
sprintf(FileName, "Shadow Map, Ver %d", Version);
GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, FileName);

return (0);

} // Raster::SaveShadow3D

/*===========================================================================*/

unsigned long Raster::ShadowRaster3D_Save(FILE *ffile, ShadowMap3D *SM3D)
{
char Version = WCS_IMAGES_VERSION, Revision = WCS_IMAGES_REVISION, Band;
ULONG ItemTag, TotalWritten = 0, ByteOrder = 0xaabbccdd;
long BytesWritten;

if (ffile)
	{
	// no tags or sizes for first three items: version, revision & byte order
	if ((BytesWritten = WriteBlock(ffile, (char *)&Version,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&Revision,
		WCS_BLOCKSIZE_CHAR + WCS_BLOCKTYPE_CHAR)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = WriteBlock(ffile, (char *)&ByteOrder,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	ItemTag = WCS_RASTER_DATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = SaveShadowData3D(ffile, SM3D))
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
				} // if raster saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;

	for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
		{
		if (ByteMap[Band] == SM3D->AABuf)
			break;
		} // for
	if (Band < WCS_RASTER_MAX_BANDS && ByteMap[Band])
		{
		ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SaveBandData(ffile, Band, WCS_RASTER_BANDSET_BYTE))
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
					} // if raster saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	else
		goto WriteError;

	for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
		{
		if (FloatMap[Band] == SM3D->ZBuf)
			break;
		} // for
	if (Band < WCS_RASTER_MAX_BANDS && FloatMap[Band])
		{
		ItemTag = WCS_RASTER_BANDDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SaveBandData(ffile, Band, WCS_RASTER_BANDSET_FLOAT))
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
					} // if raster saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		} // if
	else
		goto WriteError;

	ItemTag = WCS_PARAM_DONE;
	if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if file 

return (TotalWritten);

WriteError:

return (0L);

} // Raster::ShadowRaster3D_Save

/*===========================================================================*/

ULONG Raster::SaveShadowData3D(FILE *ffile, ShadowMap3D *SM3D)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, x, y;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_ROWS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Rows)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DATA_COLS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Cols)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_TYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SM3D->MyType)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_VPX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->VP.XYZ[0])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_VPY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->VP.XYZ[1])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_VPZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->VP.XYZ[2])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_OBJX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->RefPos.XYZ[0])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_OBJY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->RefPos.XYZ[1])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_OBJZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->RefPos.XYZ[2])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_DISTOFFSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->DistanceOffset)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_CENTERX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->CenterX)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_CENTERY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->CenterY)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_HORSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->HorScale)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_ZOFFSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->ZOffset)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  for (y = 0; y < 3; y ++)
   {
   for (x = 0; x < 3; x ++)
    {
    if ((BytesWritten = PrepWriteBlock(ffile, WCS_SM3D_DATA_SOLARROTMATX, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
		WCS_BLOCKTYPE_DOUBLE, (char *)&SM3D->SolarRotMatx[y][x])) == NULL)
	 goto WriteError;
    TotalWritten += BytesWritten;
    } // for
   } // for

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // Raster::SaveShadowData3D


/*===========================================================================*/
/*===========================================================================*/
// RasterShell

RasterShell::RasterShell()
{

ID = 0;
Rast = NULL;
Host = NULL;

} // RasterShell::RasterShell

/*===========================================================================*/

RasterShell::RasterShell(Raster *RastSource)
{

ID = 0;
Rast = RastSource;
Host = NULL;

} // RasterShell::RasterShell

/*===========================================================================*/

RasterShell::~RasterShell()
{

if (Host)
	Host->RemoveRaster(this);
Host = NULL;

} // RasterShell::~RasterShell

/*===========================================================================*/

void RasterShell::Copy(RasterShell *CopyTo, RasterShell *CopyFrom)
{

CopyTo->ID = CopyFrom->ID;
CopyTo->Rast = CopyFrom->Rast;
CopyTo->Host = CopyFrom->Host;

} // RasterShell::Copy

/*===========================================================================*/

int RasterShell::CheckOKRemove(void)
{
char *RemoveQuery;
int RemoveIt = 1;

if (Host)
	{
	if (! (RemoveQuery = Host->OKRemoveRaster()) || (RemoveIt = UserMessageOKCAN((char *)Rast->GetName(), RemoveQuery)))
		{
		Host->RemoveRaster(this);
		Host = NULL;
		} // if
	} // if

return (RemoveIt);

} // RasterShell::CheckOKRemove

/*===========================================================================*/

char *RasterShell::GetHostName(void)
{

if (Host)
	{
	return (Host->GetRAHostName());
	} // if
else if (GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
	return ("Sequence");
else if (GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
	return ("Dissolve");
else if (GetType() == WCS_RASTERSHELL_TYPE_COLORCONTROL)
	return ("Color Control");
else if (GetType() == WCS_RASTERSHELL_TYPE_IMAGEMANAGER)
	return ("Image Manager");
else if (GetType() == WCS_RASTERSHELL_TYPE_GEOREF)
	return ("Geo Reference");

return ("Unknown");

} // RasterShell::GetHostName

/*===========================================================================*/

char *RasterShell::GetHostTypeString(void)
{

if (Host)
	{
	return (Host->GetRAHostTypeString());
	} // if

return (NULL);

} // RasterShell::GetHostTypeString

/*===========================================================================*/

int RasterShell::GetHostEnabled(void)
{

if (Host)
	{
	return (Host->GetRAEnabled());
	} // if

return (0);

} // RasterShell::GetHostEnabled

/*===========================================================================*/

void RasterShell::EditHost(void)
{

if (Host)
	{
	Host->EditRAHost();
	} // if

} // RasterShell::EditHost

/*===========================================================================*/

void RasterShell::SetHostNotify(RasterAnimHost *HostSource)
{
NotifyTag Changes[2];

Host = HostSource;
if (Rast)
	{
	Changes[0] = MAKE_ID(Rast->GetNotifyClass(), Rast->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Rast->GetRAHostRoot());
	} // if

} // RasterShell::SetHostNotify

/*===========================================================================*/
/*===========================================================================*/
// GeoRefShell

GeoRefShell::GeoRefShell()
: GeoReg(NULL)
{

SetDefaults();

} // GeoRefShell::GeoRefShell

/*===========================================================================*/

GeoRefShell::GeoRefShell(Raster *RastSource)
: GeoReg(RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // GeoRefShell::GeoRefShell

/*===========================================================================*/

void GeoRefShell::SetDefaults(void)
{

BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS;

} // GeoRefShell::SetDefaults

/*===========================================================================*/

void GeoRefShell::Copy(GeoRefShell *CopyTo, GeoRefShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);
GeoReg.Copy(&CopyTo->GeoReg, &CopyFrom->GeoReg);
CopyTo->BoundsType = CopyFrom->BoundsType;

} // GeoRefShell::Copy

/*===========================================================================*/

void GeoRefShell::SetBounds(double LatRange[2], double LonRange[2])
{

GeoReg.SetBounds((CoordSys *)Host, LatRange, LonRange);
/*
NotifyTag Changes[2];
VertexDEM Vert;
double Coords[8];

// can't use the bounds if they are equal
if (LatRange[0] != LatRange[1] && LonRange[0] != LonRange[1])
	{
	if (LatRange[1] < LatRange[0])
		swmem(&LatRange[0], &LatRange[1], sizeof (double));
	if (LonRange[1] < LonRange[0])
		swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	if (fabs(LonRange[1] - LonRange[0]) > 180.0)
		{
		LonRange[1] -= 360.0;
		} // if
	// unproject all corners and figure out which ones to use
	Vert.Lat = Coords[1] = LatRange[0];
	Vert.Lon = Coords[0] = LonRange[0];
	if (Host)
		{
		((CoordSys *)Host)->DefDegToProj(&Vert);
		Coords[0] = Vert.xyz[0];
		Coords[1] = Vert.xyz[1];
		} // if 

	Vert.Lat = Coords[3] = LatRange[0];
	Vert.Lon = Coords[2] = LonRange[1];
	if (Host)
		{
		((CoordSys *)Host)->DefDegToProj(&Vert);
		Coords[2] = Vert.xyz[0];
		Coords[3] = Vert.xyz[1];
		} // if 

	Vert.Lat = Coords[5] = LatRange[1];
	Vert.Lon = Coords[4] = LonRange[0];
	if (Host)
		{
		((CoordSys *)Host)->DefDegToProj(&Vert);
		Coords[4] = Vert.xyz[0];
		Coords[5] = Vert.xyz[1];
		} // if 

	Vert.Lat = Coords[7] = LatRange[1];
	Vert.Lon = Coords[6] = LonRange[1];
	if (Host)
		{
		((CoordSys *)Host)->DefDegToProj(&Vert);
		Coords[6] = Vert.xyz[0];
		Coords[7] = Vert.xyz[1];
		} // if 

	LatRange[0] = min(Coords[1], Coords[3]);
	LatRange[0] = min(LatRange[0], Coords[5]);
	LatRange[0] = min(LatRange[0], Coords[7]);
	LatRange[1] = max(Coords[1], Coords[3]);
	LatRange[1] = max(LatRange[1], Coords[5]);
	LatRange[1] = max(LatRange[1], Coords[7]);
	if (! Host || ! ((CoordSys *)Host)->Method.GCTPMethod)
		{
		LonRange[0] = min(Coords[0], Coords[2]);
		LonRange[0] = min(LonRange[0], Coords[4]);
		LonRange[0] = min(LonRange[0], Coords[6]);
		LonRange[1] = max(Coords[0], Coords[2]);
		LonRange[1] = max(LonRange[1], Coords[4]);
		LonRange[1] = max(LonRange[1], Coords[6]);
		} // if
	else
		{
		LonRange[0] = max(Coords[0], Coords[2]);
		LonRange[0] = max(LonRange[0], Coords[4]);
		LonRange[0] = max(LonRange[0], Coords[6]);
		LonRange[1] = min(Coords[0], Coords[2]);
		LonRange[1] = min(LonRange[1], Coords[4]);
		LonRange[1] = min(LonRange[1], Coords[6]);
		} // else

	//if (LatRange[1] < LatRange[0])
	//	swmem(&LatRange[0], &LatRange[1], sizeof (double));
	//if (LonRange[1] < LonRange[0])
	//	swmem(&LonRange[0], &LonRange[1], sizeof (double));
	// if bounds appear to wrap more than halfway around earth then probably 
	// want to take the smaller arc
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(LatRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(LatRange[0]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(LonRange[1]);
	GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(LonRange[0]);
	Changes[0] = MAKE_ID(GeoReg.GetNotifyClass(), GeoReg.GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, GeoReg.GetRAHostRoot());
	} // if
*/
} // GeoRefShell::SetBounds

/*===========================================================================*/

void GeoRefShell::ShiftBounds(double LatRange[2], double LonRange[2])
{

GeoReg.ShiftBounds((CoordSys *)Host, LatRange, LonRange);
} // GeoRefShell::ShiftBounds


/*===========================================================================*/

int GeoRefShell::SampleLatLonElev(double X, double Y, double &Lat, double &Lon, double &Elev, int &ElevReject)
{
double ExtraWidth = 0.0, ExtraHeight = 0.0;
PlanetOpt *DefPlanetOpt;
VertexDEM Vert;

if (X >= 0.0 && X <= 1.0 && Y >= 0.0 && Y <= 1.0)
	{
	if (BoundsType == WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS && Rast)
		{
		ExtraWidth = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
		ExtraHeight = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
		ExtraWidth /= ((Rast->Cols - 1) * 2.0);
		ExtraHeight /= ((Rast->Rows - 1) * 2.0);
		} // if
	Lat = Vert.xyz[1] = ExtraHeight + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - 
		Y * (2 * ExtraHeight + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
	Lon = Vert.xyz[0] = ExtraWidth + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - 
		X * (2 * ExtraWidth + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);
	if (Host)
		{
		((CoordSys *)Host)->ProjToDefDeg(&Vert);
		Lat = Vert.Lat;
		Lon = Vert.Lon;
		} // if
	Elev = GlobalApp->MainProj->Interactive->ElevationPointNULLReject(Lat, Lon, ElevReject);
	if (DefPlanetOpt = (PlanetOpt *)GlobalApp->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_PLANETOPT, 1, NULL))
		{
		Elev = DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue
			+ (Elev - DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_DATUM].CurValue) 
			* DefPlanetOpt->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_VERTICALEXAG].CurValue;
		} // if
	return (1);
	} // if

return (0);

} // GeoRefShell::SampleLatLonElev

/*===========================================================================*/

int GeoRefShell::SampleXY(double Lat, double Lon, double &X, double &Y)
{
double ExtraWidth = 0.0, ExtraHeight = 0.0;
VertexDEM Vert;

Vert.Lat = Y = Lat;
Vert.Lon = X = Lon;

if (Host)
	{
	((CoordSys *)Host)->DefDegToProj(&Vert);
	Y = Vert.xyz[1];
	X = Vert.xyz[0];
	} // if

if (BoundsType == WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS && Rast)
	{
	ExtraWidth = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
	ExtraHeight = GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
	ExtraWidth /= ((Rast->Cols - 1) * 2.0);
	ExtraHeight /= ((Rast->Rows - 1) * 2.0);
	} // if
Y = (ExtraHeight + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - Y)
	/ (2 * ExtraHeight + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue);
X = (ExtraWidth + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - X)
	/ (2 * ExtraWidth + GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue - GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue);

if (Y >= 0.0 && Y <= 1.0 && X >= 0.0 && X <= 1.0)
	return (1);

return (0);

} // GeoRefShell::SampleXY

/*===========================================================================*/

unsigned long GeoRefShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_RASTER_SHELL_ID:
						{
						BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_GEOREFSHELL_BOUNDSTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BoundsType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_GEOREFSHELL_GEOREG:
						{
						BytesRead = GeoReg.Load(ffile, Size, ByteFlip);
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

} // GeoRefShell::Load

/*===========================================================================*/

unsigned long GeoRefShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

ID = RasterShellIDCounter++;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_GEOREFSHELL_BOUNDSTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BoundsType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_RASTER_GEOREFSHELL_GEOREG + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = GeoReg.Save(ffile))
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
			} // if registration saved 
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

} // GeoRefShell::Save

/*===========================================================================*/
/*===========================================================================*/
// ImageManagerShell

ImageManagerShell::ImageManagerShell()
{

SetDefaults();

} // ImageManagerShell::ImageManagerShell

/*===========================================================================*/

ImageManagerShell::ImageManagerShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // ImageManagerShell::ImageManagerShell

/*===========================================================================*/

void ImageManagerShell::SetDefaults(void)
{

TilingControlType = WCS_IMAGEMGRSHELL_TILINGTYPE_AUTO;
VirtResEnabled = 0;
VirtResType = WCS_IMAGEMGRSHELL_VIRTRESTYPE_PIXELS;
AutoTileSizeX = AutoTileSizeY = 256; 
ManualTileSizeX = ManualTileSizeY = 256;
VirtResX = VirtResY = 256;

} // ImageManagerShell::SetDefaults

/*===========================================================================*/

void ImageManagerShell::Copy(ImageManagerShell *CopyTo, ImageManagerShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);

} // ImageManagerShell::Copy

/*===========================================================================*/

unsigned long ImageManagerShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_RASTER_IMAGEMGRSHELL_TILINGTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&TilingControlType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_VIRTRESENABLED:
						{
						BytesRead = ReadBlock(ffile, (char *)&VirtResEnabled, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_VIRTRESTYPE:
						{
						BytesRead = ReadBlock(ffile, (char *)&VirtResType, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&AutoTileSizeX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEY:
						{
						BytesRead = ReadBlock(ffile, (char *)&AutoTileSizeY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ManualTileSizeX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ManualTileSizeY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_VIRTRESX:
						{
						BytesRead = ReadBlock(ffile, (char *)&VirtResX, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_RASTER_IMAGEMGRSHELL_VIRTRESY:
						{
						BytesRead = ReadBlock(ffile, (char *)&VirtResY, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
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

} // ImageManagerShell::Load

/*===========================================================================*/

unsigned long ImageManagerShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_TILINGTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TilingControlType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_VIRTRESENABLED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VirtResEnabled)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_VIRTRESTYPE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&VirtResType)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&AutoTileSizeX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_AUTOTILESIZEY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&AutoTileSizeY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ManualTileSizeX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_MANUALTILESIZEY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ManualTileSizeY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_VIRTRESX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&VirtResX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEMGRSHELL_VIRTRESY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&VirtResY)) == NULL)
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

} // ImageManagerShell::Save

/*===========================================================================*/
/*===========================================================================*/
// ImageShell

ImageShell::ImageShell()
{

SetDefaults();

} // ImageShell::ImageShell

/*===========================================================================*/

ImageShell::ImageShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // ImageShell::ImageShell

/*===========================================================================*/

void ImageShell::SetDefaults(void)
{

Color = 0;

} // ImageShell::SetDefaults

/*===========================================================================*/

void ImageShell::Copy(ImageShell *CopyTo, ImageShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);
CopyTo->Color = CopyFrom->Color;

} // ImageShell::Copy

/*===========================================================================*/

unsigned long ImageShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
      case WCS_RASTER_SHELL_ID:
       {
       BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_IMAGEESHELL_COLOR:
       {
       BytesRead = ReadBlock(ffile, (char *)&Color, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
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

} // ImageShell::Load

/*===========================================================================*/

unsigned long ImageShell::Save(FILE *ffile, short SaveID)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

  if (SaveID)
   ID = RasterShellIDCounter++;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_IMAGEESHELL_COLOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
	WCS_BLOCKTYPE_SHORTINT, (char *)&Color)) == NULL)
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

} // ImageShell::Save

/*===========================================================================*/
/*===========================================================================*/
// ColorControlShell

ColorControlShell::ColorControlShell()
{

SetDefaults();

} // ColorControlShell::ColorControlShell

/*===========================================================================*/

ColorControlShell::ColorControlShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // ColorControlShell::ImageShell

/*===========================================================================*/

void ColorControlShell::SetDefaults(void)
{

memset(RGB, 0, sizeof RGB);
RGB[2][0] = RGB[2][1] = RGB[2][2] = 255;
UseAsColor = 1;
GrayAutoRange = 1;
DisplayRGBSet = 0;
ShowTransparency = 0;
UseBandAssignment = 1;
UseBandAs[0] = 0;
UseBandAs[1] = 1;
UseBandAs[2] = 2;
memset(BandFactor, 0, sizeof BandFactor);
BandFactor[0][0] = 1.0;
BandFactor[1][1] = 1.0;
BandFactor[2][2] = 1.0;

} // ColorControlShell::SetDefaults

/*===========================================================================*/

void ColorControlShell::Copy(ColorControlShell *CopyTo, ColorControlShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);
CopyTo->UseAsColor = CopyFrom->UseAsColor;
CopyTo->GrayAutoRange = CopyFrom->GrayAutoRange;
CopyTo->DisplayRGBSet = CopyFrom->DisplayRGBSet;
CopyTo->ShowTransparency = CopyFrom->ShowTransparency;
CopyTo->UseBandAssignment = CopyFrom->UseBandAssignment;
CopyTo->UseBandAs[0] = CopyFrom->UseBandAs[0];
CopyTo->UseBandAs[1] = CopyFrom->UseBandAs[1];
CopyTo->UseBandAs[2] = CopyFrom->UseBandAs[2];
memcpy(CopyTo->RGB, CopyFrom->RGB, sizeof RGB);
memcpy(CopyTo->BandFactor, CopyFrom->BandFactor, sizeof BandFactor);

} // ImageShell::Copy

/*===========================================================================*/

unsigned long ColorControlShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char OutBand = 0, InBand = 0, OutRGB = 0, InRGB = 0;

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
      case WCS_RASTER_SHELL_ID:
       {
       BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_USEASCOLOR:
       {
       BytesRead = ReadBlock(ffile, (char *)&UseAsColor, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_GRAYAUTORANGE:
       {
       BytesRead = ReadBlock(ffile, (char *)&GrayAutoRange, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_DISPLAYRGBSET:
       {
       BytesRead = ReadBlock(ffile, (char *)&DisplayRGBSet, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_SHOWTRANSPAR:
       {
       BytesRead = ReadBlock(ffile, (char *)&ShowTransparency, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_USEBANDASSIGN:
       {
       BytesRead = ReadBlock(ffile, (char *)&UseBandAssignment, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_0:
       {
       BytesRead = ReadBlock(ffile, (char *)&UseBandAs[0], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_1:
       {
       BytesRead = ReadBlock(ffile, (char *)&UseBandAs[1], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_2:
       {
       BytesRead = ReadBlock(ffile, (char *)&UseBandAs[2], WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_OUTBAND:
       {
       BytesRead = ReadBlock(ffile, (char *)&OutBand, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_INBAND:
       {
       BytesRead = ReadBlock(ffile, (char *)&InBand, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR:
       {
       BytesRead = ReadBlock(ffile, (char *)&BandFactor[OutBand][InBand], WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_RGB_APP:
       {
       BytesRead = ReadBlock(ffile, (char *)&OutRGB, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_RGB_BAND:
       {
       BytesRead = ReadBlock(ffile, (char *)&InRGB, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_COLORCONTROLSHELL_RGB:
       {
       BytesRead = ReadBlock(ffile, (char *)&RGB[OutRGB][InRGB], WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
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

} // ColorControlShell::Load

/*===========================================================================*/

unsigned long ColorControlShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
char OutBand, InBand;

  ID = RasterShellIDCounter++;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_USEASCOLOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseAsColor)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_GRAYAUTORANGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&GrayAutoRange)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_DISPLAYRGBSET, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DisplayRGBSet)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_SHOWTRANSPAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&ShowTransparency)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_USEBANDASSIGN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseBandAssignment)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_0, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseBandAs[0])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_1, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseBandAs[1])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_USEBANDAS_2, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UseBandAs[2])) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  for (OutBand = 0; OutBand < 3; OutBand ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_OUTBAND, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&OutBand)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	for (InBand = 0; InBand < WCS_RASTER_MAX_BANDS; InBand ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR_INBAND, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&InBand)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_BANDFACTOR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
			WCS_BLOCKTYPE_DOUBLE, (char *)&BandFactor[OutBand][InBand])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // for
  for (OutBand = 0; OutBand < 3; OutBand ++)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_RGB_APP, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
		WCS_BLOCKTYPE_CHAR, (char *)&OutBand)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	for (InBand = 0; InBand < 3; InBand ++)
		{
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_RGB_BAND, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, (char *)&InBand)) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_COLORCONTROLSHELL_RGB, WCS_BLOCKSIZE_CHAR,
			WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_SHORT,
			WCS_BLOCKTYPE_SHORTINT, (char *)&RGB[OutBand][InBand])) == NULL)
			goto WriteError;
		TotalWritten += BytesWritten;
		} // for
	} // for

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // ColorControlShell::Save

/*===========================================================================*/
/*===========================================================================*/
// FoliageShellLink - replaces old BitmapImage structure

FoliageShellLink::FoliageShellLink(FoliageShell *SetShell)
{

Shell = SetShell;
Next = NULL;

} // FoliageShellLink::FoliageShellLink

/*===========================================================================*/
/*===========================================================================*/
// FoliageShell

FoliageShell::FoliageShell()
{

SetDefaults();

} // FoliageShell::FoliageShell

/*===========================================================================*/

FoliageShell::FoliageShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // FoliageShell::FoliageShell

/*===========================================================================*/

void FoliageShell::SetDefaults(void)
{

HeightPct = DensityPct = 1.0;
PosShade = Shade3D = 1;
Colors = 0;

} // FoliageShell::SetDefaults

/*===========================================================================*/

void FoliageShell::Copy(FoliageShell *CopyTo, FoliageShell *CopyFrom)
{

ImageShell::Copy((ImageShell *)CopyTo, (ImageShell *)CopyFrom);
CopyTo->DensityPct = CopyFrom->DensityPct;
CopyTo->HeightPct = CopyFrom->HeightPct;
CopyTo->PosShade = CopyFrom->PosShade;
CopyTo->Shade3D = CopyFrom->Shade3D;
CopyTo->Colors = CopyFrom->Colors;

} // FoliageShell::Copy

/*===========================================================================*/

unsigned long FoliageShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
      case WCS_RASTER_SHELL_ID:
       {
       BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_HEIGHTPCT:
       {
       BytesRead = ReadBlock(ffile, (char *)&HeightPct, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_DENSITYPCT:
       {
       BytesRead = ReadBlock(ffile, (char *)&DensityPct, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_POSSHADE:
       {
       BytesRead = ReadBlock(ffile, (char *)&PosShade, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_SHADE3D:
       {
       BytesRead = ReadBlock(ffile, (char *)&Shade3D, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_COLORS:
       {
       BytesRead = ReadBlock(ffile, (char *)&Colors, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_FOLIAGESHELL_IMAGEDATA:
       {
       BytesRead = ImageShell::Load(ffile, Size, ByteFlip);
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

} // FoliageShell::Load

/*===========================================================================*/

unsigned long FoliageShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

  ID = RasterShellIDCounter++;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_FOLIAGESHELL_HEIGHTPCT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&HeightPct)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_FOLIAGESHELL_DENSITYPCT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&DensityPct)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_FOLIAGESHELL_POSSHADE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&PosShade)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_FOLIAGESHELL_SHADE3D, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Shade3D)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_FOLIAGESHELL_COLORS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Colors)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  ItemTag = WCS_RASTER_FOLIAGESHELL_IMAGEDATA + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
  if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
   {
   TotalWritten += BytesWritten;

   ItemTag = 0;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    if (BytesWritten = ImageShell::Save(ffile, FALSE))
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
     } /* if imageshell saved */
    else
     goto WriteError;
    } /* if size written */
   else
    goto WriteError;
   } /* if tag written */
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

} // FoliageShell::Save

/*===========================================================================*/
/*===========================================================================*/
// SequenceShell

SequenceShell::SequenceShell()
{

SetDefaults();

} // SequenceShell::SequenceShell

/*===========================================================================*/

SequenceShell::SequenceShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // SequenceShell::SequenceShell

/*===========================================================================*/

void SequenceShell::SetDefaults(void)
{

StartFrame = 0.0;
EndFrame = 30.0;
Duration = 30.0;
StartSpace = 0.0;
EndSpace = 0.0;
StartImage = 0;
EndImage = 30;
NumLoops = 1.0;
Speed = 100.0;
StartBehavior = EndBehavior = WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD;
NextSeq = PrevSeq = NULL;

} // SequenceShell::SetDefaults

/*===========================================================================*/

void SequenceShell::Copy(SequenceShell *CopyTo, SequenceShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);
CopyTo->StartFrame = CopyFrom->StartFrame;
CopyTo->EndFrame = CopyFrom->EndFrame;
CopyTo->Duration = CopyFrom->Duration;
CopyTo->StartSpace = CopyFrom->StartSpace;
CopyTo->EndSpace = CopyFrom->EndSpace;
CopyTo->StartImage = CopyFrom->StartImage;
CopyTo->EndImage = CopyFrom->EndImage;
CopyTo->NumLoops = CopyFrom->NumLoops;
CopyTo->Speed = CopyFrom->Speed;
CopyTo->StartBehavior = CopyFrom->StartBehavior;
CopyTo->EndBehavior = CopyFrom->EndBehavior;
CopyTo->PAF.Copy(&CopyTo->PAF, &CopyFrom->PAF);
NextSeq = PrevSeq = NULL;

} // SequenceShell::Copy

/*===========================================================================*/

void SequenceShell::AdjustSubsequent(void)
{

if (NextSeq)
	{
	((SequenceShell *)NextSeq)->StartFrame = EndFrame + EndSpace + ((SequenceShell *)NextSeq)->StartSpace;
	((SequenceShell *)NextSeq)->EndFrame = ((SequenceShell *)NextSeq)->StartFrame + ((SequenceShell *)NextSeq)->Duration;
	((SequenceShell *)NextSeq)->AdjustSubsequent();
	} // if

} // SequenceShell::AdjustSubsequent

/*===========================================================================*/

void SequenceShell::GetOpenFileNames(char *FirstName, char *LastName, double &Offset, double SequenceFrame)
{
double OffsetFromStartFrame, OffsetFromStartImage, ImageRange, TestImage;
SequenceShell *Temp, *Prev = NULL;
FILE *fFile;
long FirstTestImage = -1, LastTestImage = -1;
char NumStr[10];

Offset = 0.0;

for (Temp = this; Temp && SequenceFrame >= Temp->EndFrame + Temp->EndSpace; Temp = (SequenceShell *)Temp->NextSeq)
	{
	Prev = Temp;
	} // for

if (! Temp && Prev)
	{
	Temp = Prev;
	if (WCS_floor(SequenceFrame) == Prev->EndFrame + Prev->EndSpace)
		{
		SequenceFrame = WCS_floor(SequenceFrame);	// last frame should be treated as a whole frame duration
		} // if
	} // if
if (Temp)
	{
	if (SequenceFrame >= Temp->StartFrame)
		{
		if (SequenceFrame > Temp->EndFrame && Temp->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_NOIMAGE)
			{
			} // if
		else if (SequenceFrame > Temp->EndFrame && Temp->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD)
			{
			FirstTestImage = Temp->EndImage;
			} // if
		else
			{
			OffsetFromStartFrame = SequenceFrame - Temp->StartFrame;
			OffsetFromStartImage = OffsetFromStartFrame * Temp->Speed / 100.0;
			ImageRange = Temp->EndImage - Temp->StartImage;
			if (ImageRange <= 0)
				{
				FirstTestImage = Temp->StartImage;
				} // if
			else
				{
				while (OffsetFromStartImage > ImageRange + 1)
					OffsetFromStartImage -= (ImageRange + 1);
				TestImage = Temp->StartImage + OffsetFromStartImage;
				FirstTestImage = quicklongfloor(TestImage);
				LastTestImage = quicklongceil(TestImage);
				} // while
			} // else
		} // if
	else
		{
		switch (Temp->StartBehavior)
			{
			case WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_NOIMAGE:
				{
				break;
				} // 
			case WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD:
				{
				FirstTestImage = Temp->StartImage;
				break;
				} // 
			case WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE:
				{
				OffsetFromStartFrame = SequenceFrame - Temp->EndFrame;
				OffsetFromStartImage = OffsetFromStartFrame * Temp->Speed / 100.0;
				ImageRange = Temp->StartImage - Temp->EndImage;
				if (ImageRange >= 0)
					{
					FirstTestImage = Temp->StartImage;
					} // if
				else
					{
					while (OffsetFromStartImage < ImageRange)
						OffsetFromStartImage -= (ImageRange - 1);
					TestImage = Temp->EndImage + OffsetFromStartImage;
					FirstTestImage = quicklongfloor(TestImage);
					LastTestImage = quicklongceil(TestImage);
					} // while
				break;
				} // 
			} // switch
		} // else start space
	if (FirstTestImage >= 0 && FirstTestImage >= Temp->StartImage)
		{
		while (FirstTestImage >= Temp->StartImage)
			{
			sprintf(NumStr, "%08d", FirstTestImage);
			InsertNameNum(PAF.GetPathAndName(FirstName), NumStr, FALSE);
			if (fFile = PROJ_fopen(FirstName, "rb"))
				{
				fclose(fFile);
				break;
				} // if
			FirstTestImage -= 1;
			} // while
		} // if
	else
		FirstTestImage = -1;

	if (LastTestImage != FirstTestImage && LastTestImage >= 0 && LastTestImage <= Temp->EndImage)
		{
		while (LastTestImage <= Temp->EndImage)
			{
			sprintf(NumStr, "%08d", LastTestImage);
			InsertNameNum(PAF.GetPathAndName(LastName), NumStr, FALSE);
			if (fFile = PROJ_fopen(LastName, "rb"))
				{
				fclose(fFile);
				break;
				} // if
			LastTestImage += 1;
			} // while
		} // if
	else
		LastTestImage = -1;

	if (FirstTestImage < Temp->StartImage || FirstTestImage < 0)
		{
		if (LastTestImage > Temp->EndImage || LastTestImage < 0)
			{
			FirstName[0] = 0;
			LastName[0] = 0;
			} // if
		else
			{
			strcpy(FirstName, LastName);
			LastName[0] = 0;
			} // else
		} // if
	else if (LastTestImage > Temp->EndImage || LastTestImage < 0)
		{
		LastName[0] = 0;
		} // else
	if (FirstName[0] && LastName[0] && LastTestImage != FirstTestImage)	// the last test here shouldn't be necessary - famous last words!
		Offset = (TestImage - FirstTestImage) / (LastTestImage - FirstTestImage);
	} // if
else
	{
	FirstName[0] = 0;
	LastName[0] = 0;
	} // else

} // SequenceShell::GetOpenFileNames

/*===========================================================================*/

unsigned long SequenceShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag, WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
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
		case WCS_RASTER_SHELL_ID:
			{
			BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_NAME:
			{
			BytesRead = PAF.Load(ffile, Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_STARTFRAME:
			{
			BytesRead = ReadBlock(ffile, (char *)&StartFrame, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_ENDFRAME:
			{
			BytesRead = ReadBlock(ffile, (char *)&EndFrame, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_DURATION:
			{
			BytesRead = ReadBlock(ffile, (char *)&Duration, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_STARTSPACE:
			{
			BytesRead = ReadBlock(ffile, (char *)&StartSpace, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_ENDSPACE:
			{
			BytesRead = ReadBlock(ffile, (char *)&EndSpace, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_STARTIMAGE:
			{
			BytesRead = ReadBlock(ffile, (char *)&StartImage, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_ENDIMAGE:
			{
			BytesRead = ReadBlock(ffile, (char *)&EndImage, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_NUMLOOPS:
			{
			BytesRead = ReadBlock(ffile, (char *)&NumLoops, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_SPEED:
			{
			BytesRead = ReadBlock(ffile, (char *)&Speed, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_STARTBEHAVIOR:
			{
			BytesRead = ReadBlock(ffile, (char *)&StartBehavior, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_ENDBEHAVIOR:
			{
			BytesRead = ReadBlock(ffile, (char *)&EndBehavior, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
			break;
			}
		case WCS_RASTER_SEQUENCESHELL_NEXTSEQUENCE:
			{
			if (NextSeq = (RasterShell *)(new SequenceShell(RastSource)))
				{
				BytesRead = ((SequenceShell *)NextSeq)->Load(ffile, Size, ByteFlip, RastSource);
				((SequenceShell *)NextSeq)->PrevSeq = (RasterShell *)this;
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
	} /* if size block read */
	else
	break;
	} /* if not done flag */
	} /* if tag block read */
	else
		break;
	} // while

return (TotalRead);

} // SequenceShell::Load

/*===========================================================================*/

unsigned long SequenceShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

  ID = RasterShellIDCounter++;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  ItemTag = WCS_RASTER_SEQUENCESHELL_NAME + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
  if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
   {
   TotalWritten += BytesWritten;

   ItemTag = 0;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    if (BytesWritten = PAF.Save(ffile))
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
     } /* if pathandfile saved */
    else
     goto WriteError;
    } /* if size written */
   else
    goto WriteError;
   } /* if tag written */
  else
   goto WriteError;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_STARTFRAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&StartFrame)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_ENDFRAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&EndFrame)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_DURATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Duration)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_STARTSPACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&StartSpace)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_ENDSPACE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&EndSpace)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_STARTIMAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&StartImage)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_ENDIMAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&EndImage)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_NUMLOOPS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NumLoops)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_SPEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&Speed)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_STARTBEHAVIOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&StartBehavior)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SEQUENCESHELL_ENDBEHAVIOR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EndBehavior)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if (NextSeq)
   {
   ItemTag = WCS_RASTER_SEQUENCESHELL_NEXTSEQUENCE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     if (BytesWritten = ((SequenceShell *)NextSeq)->Save(ffile))
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
      } /* if next sequence saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } // if

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // SequenceShell::Save

/*===========================================================================*/
/*===========================================================================*/
// DissolveShell

DissolveShell::DissolveShell()
{

SetDefaults();

} // DissolveShell::DissolveShell

/*===========================================================================*/

DissolveShell::DissolveShell(Raster *RastSource)
{

SetDefaults();
RasterShell::SetRaster(RastSource);

} // DissolveShell::DissolveShell

/*===========================================================================*/

void DissolveShell::SetDefaults(void)
{

StartFrame = 0;
EndFrame = 30;
Duration = 30;
EaseIn = EaseOut = 0;
Target = NULL;
TargetID = 0;
NextDis = PrevDis = NULL;		// these will be created as DissolveShells

} // DissolveShell::SetDefaults

/*===========================================================================*/

void DissolveShell::Copy(DissolveShell *CopyTo, DissolveShell *CopyFrom)
{

RasterShell::Copy((RasterShell *)CopyTo, (RasterShell *)CopyFrom);
CopyTo->StartFrame = CopyFrom->StartFrame;
CopyTo->EndFrame = CopyFrom->EndFrame;
CopyTo->Duration = CopyFrom->Duration;
CopyTo->EaseIn = CopyFrom->EaseIn;
CopyTo->EaseOut = CopyFrom->EaseOut;
CopyTo->Target = CopyFrom->Target;
CopyTo->TargetID = CopyFrom->TargetID;
NextDis = PrevDis = NULL;		// these will be created as DissolveShells

} // DissolveShell::Copy

/*===========================================================================*/

int DissolveShell::CheckOKRemove(Raster *CheckMe)
{

if (GetTarget() == CheckMe)
	{
	if (! UserMessageOKCAN(GetRaster()->GetUserName(), "Image is in use by this Dissolve Sequence!\nRemove it anyway?"))
		return (0);
	} // if
else if (NextDis)
	{
	return (((DissolveShell *)NextDis)->CheckOKRemove(CheckMe));
	} // else if

return (1);

} // DissolveShell::CheckOKRemove

/*===========================================================================*/

int DissolveShell::RemoveRaster(Raster *RemoveMe, int Iteration)
{
RasterShell *TempDis;

if (NextDis)
	{
	if (((DissolveShell *)NextDis)->RemoveRaster(RemoveMe, Iteration + 1))
		{
		TempDis = NextDis;
		NextDis = ((DissolveShell *)NextDis)->NextDis;
		delete TempDis;
		} // if
	} // for
if (RemoveMe == GetTarget())
	{
	SetTarget(NULL);
	return (1);
	} // if
if (Iteration)
	return (0);
return (1);

} // DissolveShell::RemoveRaster

/*===========================================================================*/

char *DissolveShell::GetDisName(void)
{

return ((char*)(Target ? Target->GetUserName(): ""));

} // DissolveShell::GetDisName

/*===========================================================================*/

void DissolveShell::AdjustSubsequent(void)
{

if (NextDis)
	{
	if (((DissolveShell *)NextDis)->StartFrame < EndFrame)
		{
		((DissolveShell *)NextDis)->StartFrame = EndFrame;
		((DissolveShell *)NextDis)->EndFrame = ((DissolveShell *)NextDis)->StartFrame + ((DissolveShell *)NextDis)->Duration;
		} // if
	((DissolveShell *)NextDis)->AdjustSubsequent();
	} // if

} // DissolveShell::AdjustSubsequent

/*===========================================================================*/

void DissolveShell::GetEndMembers(Raster *&FirstRast, Raster *&LastRast, double &Offset, double DissolveFrame)
{
DissolveShell *Temp;
AnimDoubleTime Curve;
GraphNode *Node;

FirstRast = GetRaster();
LastRast = GetTarget();
Offset = 0.0;

for (Temp = this; Temp && LastRast && DissolveFrame >= Temp->EndFrame; Temp = (DissolveShell *)Temp->NextDis)
	{
	FirstRast = LastRast;
	LastRast = Temp->NextDis ? ((DissolveShell *)Temp->NextDis)->GetTarget(): NULL;
	} // for
if (Temp && DissolveFrame <= Temp->StartFrame)
	{
	LastRast = NULL;
	} // if
else if (Temp && LastRast)
	{
	if (Curve.NewGraph(0))
		{
		if (Node = Curve.AnimDouble::AddNode((double)Temp->StartFrame))
			{
			Node->SetValue(0.0);
			if (Temp->EaseIn)
				Node->SetTension(1.0);
			if (Node = Curve.AnimDouble::AddNode((double)Temp->EndFrame))
				{
				Node->SetValue(1.0);
				if (Temp->EaseOut)
					Node->SetTension(1.0);
				Offset = Curve.GetValue(0, DissolveFrame);
				} // if
			} // if
		} // if
	} // else

} // DissolveShell::GetEndMembers

/*===========================================================================*/

unsigned long DissolveShell::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip, Raster *RastSource)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

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
      case WCS_RASTER_SHELL_ID:
       {
       BytesRead = ReadBlock(ffile, (char *)&ID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_RASTERID:
       {
       BytesRead = ReadBlock(ffile, (char *)&TargetID, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_STARTFRAME:
       {
       BytesRead = ReadBlock(ffile, (char *)&StartFrame, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_ENDFRAME:
       {
       BytesRead = ReadBlock(ffile, (char *)&EndFrame, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_DURATION:
       {
       BytesRead = ReadBlock(ffile, (char *)&Duration, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_EASEIN:
       {
       BytesRead = ReadBlock(ffile, (char *)&EaseIn, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_EASEOUT:
       {
       BytesRead = ReadBlock(ffile, (char *)&EaseOut, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
       break;
	   }
      case WCS_RASTER_DISSOLVESHELL_NEXTDISSOLVE:
       {
       if (NextDis = (RasterShell *)(new DissolveShell(RastSource)))
        {
        BytesRead = ((DissolveShell *)NextDis)->Load(ffile, Size, ByteFlip, RastSource);
		((DissolveShell *)NextDis)->PrevDis = (RasterShell *)this;
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

} // DissolveShell::Load

/*===========================================================================*/

unsigned long DissolveShell::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;
unsigned long LocalID;

  ID = RasterShellIDCounter++;

  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_SHELL_ID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&ID)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if (Target)
   {
   LocalID = Target->GetID();
   if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_RASTERID, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&LocalID)) == NULL)
    goto WriteError;
   TotalWritten += BytesWritten;
   } // if
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_STARTFRAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&StartFrame)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_ENDFRAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&EndFrame)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_DURATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&Duration)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_EASEIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EaseIn)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_RASTER_DISSOLVESHELL_EASEOUT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&EaseOut)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  if (NextDis)
   {
   ItemTag = WCS_RASTER_DISSOLVESHELL_NEXTDISSOLVE + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
   if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
    {
    TotalWritten += BytesWritten;

    ItemTag = 0;
    if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
     {
     TotalWritten += BytesWritten;

     if (BytesWritten = ((DissolveShell *)NextDis)->Save(ffile))
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
      } /* if next sequence saved */
     else
      goto WriteError;
     } /* if size written */
    else
     goto WriteError;
    } /* if tag written */
   else
    goto WriteError;
   } // if

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

 return (TotalWritten);

WriteError:

 return (0L);

} // DissolveShell::Save

/*===========================================================================*/
/*===========================================================================*/

Thumbnail::Thumbnail()
{

TNail[0] = TNail[1] = TNail[2] = NULL;
TNailPadX = TNailPadY = 0;

} // Thumbnail::Thumbnail

/*===========================================================================*/

Thumbnail::~Thumbnail()
{

if (TNail[0])
	AppMem_Free(TNail[0], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);
if (TNail[1])
	AppMem_Free(TNail[1], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);
if (TNail[2])
	AppMem_Free(TNail[2], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);

} // Thumbnail::~Thumbnail

/*===========================================================================*/

UBYTE **Thumbnail::AllocTNails(void)
{

if (TNail[0] || AllocTNail(0))
	{
	ClearTNail(0);
	if (TNail[1] || AllocTNail(1))
		{
		ClearTNail(1);
		if (TNail[2] || AllocTNail(2))
			{
			ClearTNail(2);
			return (TNail);
			} // if
		} // if
	} // if
return (NULL);

} // Thumbnail::AllocTNails

/*===========================================================================*/

UBYTE *Thumbnail::AllocTNail(char Band)
{

return (TNail[Band] = (UBYTE *)AppMem_Alloc(WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE, 0));

} // Thumbnail::AllocTNail

/*===========================================================================*/

void Thumbnail::ClearTNails(void)
{

ClearTNail(0);
ClearTNail(1);
ClearTNail(2);

} // Thumbnail::ClearTNail

/*===========================================================================*/

void Thumbnail::ClearTNail(char Band)
{

if (TNail[Band])
	memset(TNail[Band], 0, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);

} // Thumbnail::ClearTNail

/*===========================================================================*/

void Thumbnail::ClearPadArea(void)
{
long Ct, RowCt, PadSize;

if (TNail[0] && TNail[1] && TNail[2])
	{
	if (TNailPadY > 0)
		{
		PadSize = TNailPadY * WCS_RASTER_TNAIL_SIZE;
		memset(&TNail[0][0], 0, PadSize);
		memset(&TNail[1][0], 0, PadSize);
		memset(&TNail[2][0], 0, PadSize);
		RowCt = WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE - PadSize;
		memset(&TNail[0][RowCt], 0, PadSize);
		memset(&TNail[1][RowCt], 0, PadSize);
		memset(&TNail[2][RowCt], 0, PadSize);
		} // if
	if (TNailPadX > 0)
		{
		RowCt = 0;
		for (Ct = 0; Ct < WCS_RASTER_TNAIL_SIZE; Ct ++)
			{
			memset(&TNail[0][RowCt], 0, TNailPadX);
			memset(&TNail[1][RowCt], 0, TNailPadX);
			memset(&TNail[2][RowCt], 0, TNailPadX);
			RowCt += WCS_RASTER_TNAIL_SIZE;
			memset(&TNail[0][RowCt - TNailPadX], 0, TNailPadX);
			memset(&TNail[1][RowCt - TNailPadX], 0, TNailPadX);
			memset(&TNail[2][RowCt - TNailPadX], 0, TNailPadX);
			} // for
		} // if
	} // if

} // Thumbnail::ClearPadArea

/*===========================================================================*/

void Thumbnail::Copy(Thumbnail *CopyTo, Thumbnail *CopyFrom)
{

CopyTo->TNailPadX = CopyFrom->TNailPadX;
CopyTo->TNailPadY = CopyFrom->TNailPadY;
if (CopyFrom->TNailsValid() && (CopyTo->TNailsValid() || CopyTo->AllocTNails()))
	{
	memcpy(CopyTo->TNail[0], CopyFrom->TNail[0], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);
	memcpy(CopyTo->TNail[1], CopyFrom->TNail[1], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);
	memcpy(CopyTo->TNail[2], CopyFrom->TNail[2], WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE);
	} // if

} // Thumbnail::Copy

/*===========================================================================*/

int Thumbnail::TNailsValid(void)
{

return (TNail[0] && TNail[1] && TNail[2]);

} // Thumbnail::TNailsValid

/*===========================================================================*/

unsigned long Thumbnail::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
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
					case WCS_THUMBNAIL_TNAILPADX:
						{
						BytesRead = ReadBlock(ffile, (char *)&TNailPadX, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_THUMBNAIL_TNAILPADY:
						{
						BytesRead = ReadBlock(ffile, (char *)&TNailPadY, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_THUMBNAIL_BANDRED:
						{
						if ((Size == WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE) && AllocTNail(0))
							{
							BytesRead = ReadLongBlock(ffile, (char *)TNail[0], Size);
							} // if memory
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_THUMBNAIL_BANDGREEN:
						{
						if ((Size == WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE) && AllocTNail(1))
							{
							BytesRead = ReadLongBlock(ffile, (char *)TNail[1], Size);
							} // if memory
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					case WCS_THUMBNAIL_BANDBLUE:
						{
						if ((Size == WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE) && AllocTNail(2))
							{
							BytesRead = ReadLongBlock(ffile, (char *)TNail[2], Size);
							} // if memory
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

} // Thumbnail::Load

/*===========================================================================*/

unsigned long Thumbnail::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_THUMBNAIL_TNAILPADX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TNailPadX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_THUMBNAIL_TNAILPADY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&TNailPadY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if (TNailsValid())
	{
	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_THUMBNAIL_BANDRED, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE,
		WCS_BLOCKTYPE_CHAR, (char *)TNail[0])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_THUMBNAIL_BANDGREEN, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE,
		WCS_BLOCKTYPE_CHAR, (char *)TNail[1])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;

	if ((BytesWritten = PrepWriteLongBlock(ffile, WCS_THUMBNAIL_BANDBLUE, WCS_BLOCKSIZE_LONG,
		WCS_BLOCKTYPE_LONGINT, WCS_RASTER_TNAIL_SIZE * WCS_RASTER_TNAIL_SIZE,
		WCS_BLOCKTYPE_CHAR, (char *)TNail[2])) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

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

} // Thumbnail::Save

/*===========================================================================*/
/*===========================================================================*/

BufferNode::BufferNode()
{

Name[0] = 0;
Type = 0;
Index = 0;
Buffer = NULL;
Next = NULL;
fFile = NULL;
RowData = NULL;
RowDataSize = 0;

} // BufferNode::BufferNode

/*===========================================================================*/

BufferNode::BufferNode(char *NewName, char NewType)
{

strncpy(Name, NewName, WCS_MAX_BUFFERNODE_NAMELEN);
Name[WCS_MAX_BUFFERNODE_NAMELEN - 1] = 0;
Type = NewType;
Index = 0;
Buffer = NULL;
Next = NULL;
fFile = NULL;
RowData = NULL;
RowDataSize = 0;

} // BufferNode::BufferNode

/*===========================================================================*/

BufferNode::~BufferNode()
{

CleanupFromSave();

} // BufferNode::~BufferNode

/*===========================================================================*/

BufferNode *BufferNode::AddBufferNode(char *NewName, char NewType)
{
BufferNode *CurNode = this;
char TestType;

// check for valid type
if (NewType < 0 || NewType >= WCS_RASTER_BANDSET_MAXTYPES)
	{
	if ((NewType = GetStandardBufferType(NewName)) < 0)
		{
		UserMessageOK("Create Buffer", "Illegal attempt to create non-standard buffer of unidentified value size.");
		return (NULL);
		} // if
	} // if
else if ((TestType = GetStandardBufferType(NewName)) >= 0 && TestType != NewType)
	{
	UserMessageOK("Create Buffer", "Illegal attempt to create a standard buffer with non-standard value size.");
	return (NULL);
	} // else if

// if there is already a node of this name return it
while (CurNode->Next)
	{
	if (! strnicmp(NewName, CurNode->Name, WCS_MAX_BUFFERNODE_NAMELEN - 1))
		{
		if (CurNode->Type == NewType)
			return (CurNode);
		else
			{
			UserMessageOK("Create Buffer", "Illegal attempt to allocate two buffers of same name but different value size.");
			return (NULL);
			} // else
		} // if name match
	CurNode = CurNode->Next;
	} // while

// check last buffer in chain for match
if (! strnicmp(NewName, CurNode->Name, WCS_MAX_BUFFERNODE_NAMELEN - 1))
	{
	if (CurNode->Type == NewType)
		return (CurNode);
	else
		{
		UserMessageOK("Create Buffer", "Illegal attempt to allocate two buffers of same name but different value size.");
		return (NULL);
		} // else
	} // if name match

// create a new one
return (CurNode->Next = new BufferNode(NewName, NewType));

} // BufferNode::AddBufferNode

/*===========================================================================*/

void *BufferNode::FindBuffer(char *FindName, char FindType)
{
BufferNode *CurNode = this;

// if there is a node of this name and type return it
while (CurNode)
	{
	if (! strnicmp(FindName, CurNode->Name, WCS_MAX_BUFFERNODE_NAMELEN - 1) && CurNode->Type == FindType)
		return (CurNode->Buffer);
	CurNode = CurNode->Next;
	} // while

return (NULL);

} // BufferNode::FindBuffer

/*===========================================================================*/

BufferNode *BufferNode::FindBufferNode(char *FindName, char FindType)
{
BufferNode *CurNode = this;

// if there is a node of this name and type return it
while (CurNode)
	{
	if (! strnicmp(FindName, CurNode->Name, WCS_MAX_BUFFERNODE_NAMELEN - 1) && CurNode->Type == FindType)
		return (CurNode);
	CurNode = CurNode->Next;
	} // while

return (NULL);

} // BufferNode::FindBufferNode

/*===========================================================================*/

int BufferNode::PrepToSave(RenderOpt *Opt, long Frame, long Width, long BeforePost)
{

// Look for different types of Temp files.
// Temp files must be of full output image size, not necessarily the size of the rendered buffer.
// If there is no Temp file then it is assumed that the entire completed image buffer is in 
// memory and pointed to by void *Buffer.
if (Opt)
	{
	if (! (fFile = Opt->OpenTempFile("rb", "Export", Name, Frame, BeforePost)))
		{
		if (! (fFile = Opt->OpenTempFile("rb", "Pano", Name, Frame, BeforePost)))
			{
			if (! (fFile = Opt->OpenTempFile("rb", "Field", Name, Frame, BeforePost)))
				{
				if (! (fFile = Opt->OpenTempFile("rb", "AAPass", Name, Frame, BeforePost)))
					{
					if (! (fFile = Opt->OpenTempFile("rb", "Segment", Name, Frame, BeforePost)))
						{
						fFile = Opt->OpenTempFile("rb", "Tile", Name, Frame, BeforePost);
						} // if no segment file
					} // if no aapass file
				} // if no field file
			} // if no panorama file
		} // if no export file
	} // if
else
	fFile = NULL;

if (Type == WCS_RASTER_BANDSET_BYTE)
	RowDataSize = sizeof (unsigned char);
else if (Type == WCS_RASTER_BANDSET_FLOAT)
	RowDataSize = sizeof (float);
else if (Type == WCS_RASTER_BANDSET_SHORT)
	RowDataSize = sizeof (unsigned short);
else
	{
	UserMessageOK("Image Output", "Unable to prep buffer to save. Undefined variable size.");
	return (0);
	} // else
RowDataSize *= Width;

if (fFile)
	{
	if (! (RowData = (char *)AppMem_Alloc(RowDataSize, 0)))
		{
		fclose(fFile);
		fFile = NULL;
		UserMessageOK("Image Output", "Unable to prep buffer to save. Out of memory.");
		return (0);
		} // if
	} // if

return (1);

} // BufferNode::PrepToSave

/*===========================================================================*/

void BufferNode::CleanupFromSave(void)
{

if (RowData)
	AppMem_Free(RowData, RowDataSize);
RowData = NULL;
if (fFile)
	fclose(fFile);
fFile = NULL;

} // BufferNode::CleanupFromSave

/*===========================================================================*/

void BufferNode::RemoveTempFiles(RenderOpt *Opt, long Frame)
{
BufferNode *CurNode = this;

while (CurNode)
	{
	Opt->RemoveTempFile("Pano", CurNode->Name, Frame, 0);
	Opt->RemoveTempFile("Field", CurNode->Name, Frame, 0);
	Opt->RemoveTempFile("AAPass", CurNode->Name, Frame, 0);
	Opt->RemoveTempFile("Segment", CurNode->Name, Frame, 0);
	Opt->RemoveTempFile("Tile", CurNode->Name, Frame, 0);
	Opt->RemoveTempFile("Pano", CurNode->Name, Frame, 1);
	Opt->RemoveTempFile("Field", CurNode->Name, Frame, 1);
	Opt->RemoveTempFile("AAPass", CurNode->Name, Frame, 1);
	Opt->RemoveTempFile("Segment", CurNode->Name, Frame, 1);
	Opt->RemoveTempFile("Tile", CurNode->Name, Frame, 1);
	CurNode = CurNode->Next;
	} // while

} // BufferNode::RemoveTempFiles

/*===========================================================================*/

char *BufferNode::GetDataLine(long Line, char CheckType)
{
long Offset;

if (Type == CheckType)
	{
	Offset = Line * RowDataSize;
	if (fFile)
		{
		if (! fseek(fFile, Offset, SEEK_SET))
			{
			if (fread(RowData, RowDataSize, 1, fFile) == 1)
				{
				return (RowData);
				} // if read OK
			} // if seek OK
		UserMessageOK("Image Output", "Unable to re-load temporary file. Possible reasons include disk file corrupt.");
		return (NULL);
		} // if
	else
		{
		return (&((char *)Buffer)[Offset]);
		} // else
	} // if

UserMessageOK("Image Save", "Illegal attempt to access buffer with incorrect value size.");
return (NULL);

} // BufferNode::GetDataLine

/*===========================================================================*/

// this is a static method and can be called anytime
// function returns buffer type or -1 if buffer is not one of the standards
char BufferNode::GetStandardBufferType(char *BufName)
{
long Ct;

for (Ct = 0; ; Ct ++)
	{
	if (! StandardBufTypes[Ct].Name[0])
		break;
	if (! strnicmp(StandardBufTypes[Ct].Name, BufName, WCS_MAX_BUFFERNODE_NAMELEN - 1))
		{
		return (StandardBufTypes[Ct].Type);
		} // if
	} // for

// not a standard buffer known to the renderer, you're on your own!
return (-1);

} // BufferNode::GetStandardBufferType

/*===========================================================================*/
/*===========================================================================*/

RLASampleData::RLASampleData()
{

X = Y = 0;
Z = 0.0f;
Alpha = 0;

} // RLASampleData::RLASampleData
