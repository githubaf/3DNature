// SXExtension.cpp
// Code module for SceneExporter Extensions.
// Built from scratch on 11/10/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "SXExtension.h"
#include "EffectsIO.h"
#include "UsefulIO.h"
#include "Types.h"
#include "AppMem.h"
#include "Application.h"
#include "Project.h"
#include "SXQueryAction.h"
#include "Security.h"

extern WCSApp *GlobalApp;

SXExtension::SXExtension()
{

} // SXExtension::SXExtension

/*===========================================================================*/

SXExtension::~SXExtension()
{

} // SXExtension::~SXExtension

/*===========================================================================*/
/*===========================================================================*/

SXExtensionSTL::SXExtensionSTL()
{

MaxDimX = MaxDimY = 100.0;
MaxDimZ = 10.0;
ActualDimX = ActualDimY = 100.0;
ActualDimZ = 10.0;
BuildScale = 10.0;
VertExag = 100.0;	// in percent
MinThickness = 6.0;
BuildMode = WCS_EFFECTS_SXEXTENSION_BUILDMODE_TOFIT;
UnitOfMeasure = WCS_EFFECTS_SXEXTENSION_BUILDUNIT_MM;

} // SXExtensionSTL::SXExtensionSTL

/*===========================================================================*/

SXExtensionSTL::~SXExtensionSTL()
{


} // SXExtensionSTL::~SXExtensionSTL

/*===========================================================================*/

void SXExtensionSTL::Copy(SXExtension *SXCopyFrom)
{
SXExtensionSTL *CopyFrom;

// copy only if correct type
if (SXCopyFrom->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_STL)
	{
	CopyFrom = (SXExtensionSTL *)SXCopyFrom;

	MaxDimX = CopyFrom->MaxDimX;
	MaxDimY = CopyFrom->MaxDimY;
	MaxDimZ = CopyFrom->MaxDimZ;
	ActualDimX = CopyFrom->ActualDimX;
	ActualDimY = CopyFrom->ActualDimY;
	ActualDimZ = CopyFrom->ActualDimZ;
	BuildScale = CopyFrom->BuildScale;
	VertExag = CopyFrom->VertExag;
	MinThickness = CopyFrom->MinThickness;
	BuildMode = CopyFrom->BuildMode;
	UnitOfMeasure = CopyFrom->UnitOfMeasure;
	} // if

} // SXExtensionSTL::Copy

/*===========================================================================*/

unsigned long SXExtensionSTL::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
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
			//read block size from file
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
					case WCS_EFFECTS_SXEXTENSION_STL_BUILDMODE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BuildMode, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_UNITOFMEASURE:
						{
						BytesRead = ReadBlock(ffile, (char *)&UnitOfMeasure, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMX:
						{
						BytesRead = ReadBlock(ffile, (char *)&ActualDimX, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMY:
						{
						BytesRead = ReadBlock(ffile, (char *)&ActualDimY, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&ActualDimZ, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_MAXDIMX:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDimX, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_MAXDIMY:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDimY, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_MAXDIMZ:
						{
						BytesRead = ReadBlock(ffile, (char *)&MaxDimZ, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_BUILDSCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&BuildScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_VERTEXAG:
						{
						BytesRead = ReadBlock(ffile, (char *)&VertExag, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_STL_MINTHICKNESS:
						{
						BytesRead = ReadBlock(ffile, (char *)&MinThickness, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
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

} // SXExtensionSTL::Load

/*===========================================================================*/

unsigned long SXExtensionSTL::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_BUILDMODE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&BuildMode)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_UNITOFMEASURE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UnitOfMeasure)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ActualDimX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ActualDimY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_ACTUALDIMZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&ActualDimZ)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_MAXDIMX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&MaxDimX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_MAXDIMY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&MaxDimY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_MAXDIMZ, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&MaxDimZ)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_BUILDSCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&BuildScale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_VERTEXAG, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&VertExag)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_STL_MINTHICKNESS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&MinThickness)) == NULL)
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

} // SXExtensionSTL::Save

/*===========================================================================*/
/*===========================================================================*/

SXExtensionOF::SXExtensionOF()
{

CreateFoliage = TRUE;
IndividualFolLOD = TRUE;

} // SXExtensionOF::SXExtensionOF

/*===========================================================================*/

SXExtensionOF::~SXExtensionOF()
{


} // SXExtensionOF::~SXExtensionOF

/*===========================================================================*/

void SXExtensionOF::Copy(SXExtension *SXCopyFrom)
{
SXExtensionOF *CopyFrom;

// copy only if correct type
if (SXCopyFrom->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_OPENFLIGHT)
	{
	CopyFrom = (SXExtensionOF *)SXCopyFrom;

	CreateFoliage = CopyFrom->CreateFoliage;
	IndividualFolLOD = CopyFrom->IndividualFolLOD;
	} // if

} // SXExtensionOF::Copy

/*===========================================================================*/

unsigned long SXExtensionOF::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
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
			//read block size from file
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
					case WCS_EFFECTS_SXEXTENSION_OF_CREATEFOLIAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&CreateFoliage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_OF_INDIVIDUALFOLLOD:
						{
						BytesRead = ReadBlock(ffile, (char *)&IndividualFolLOD, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
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

} // SXExtensionOF::Load

/*===========================================================================*/

unsigned long SXExtensionOF::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_OF_CREATEFOLIAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&CreateFoliage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_OF_INDIVIDUALFOLLOD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&IndividualFolLOD)) == NULL)
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

} // SXExtensionOF::Save

/*===========================================================================*/
/*===========================================================================*/

SXExtensionActionable::SXExtensionActionable()
{

ActionList = ActiveAction = NULL;

} // SXExtensionActionable::SXExtensionActionable


/*===========================================================================*/

SXExtensionActionable::~SXExtensionActionable()
{

RemoveActions();

} // SXExtensionActionable::~SXExtensionActionable

/*===========================================================================*/

void SXExtensionActionable::CopyActions(SXExtension *SXCopyFrom)
{
SXExtensionActionable *CopyFrom;
SXQueryAction *SourceAction;

// no type checking needed, since SXExtensionActionable is a pure virtual class
// and type error checking is done in the derived class
CopyFrom = (SXExtensionActionable *)SXCopyFrom;

RemoveActions();

SourceAction = CopyFrom->GetActionList();
while (SourceAction)
	{
	AddQueryAction(SourceAction);
	SourceAction = SourceAction->Next;
	} // while

} // SXExtensionActionable::CopyActions


/*===========================================================================*/

void SXExtensionActionable::RemoveActions(void)
{
SXQueryAction *NextAction;

while (ActionList)
	{
	NextAction = ActionList->Next;
	delete ActionList;
	ActionList = NextAction;
	} // while

} // SXExtensionActionable::RemoveActions

/*===========================================================================*/

SXQueryAction *SXExtensionActionable::AddQueryAction(SXQueryAction *CopyFrom)
{
SXQueryAction *CurAction = ActionList;

if (GlobalApp->Sentinal->CheckAuthFieldSX2())
	{
	if (! CurAction)
		{
		if (ActionList = new SXQueryAction)
			{
			if (CopyFrom)
				ActionList->Copy(CopyFrom);
			else
				ActionList->SetActionType(GetDefaultActionType()) ;
			} // if
		return (ActionList);
		} // if
	while (CurAction->Next)
		{
		CurAction = CurAction->Next;
		} // while

	if (CurAction->Next = new SXQueryAction)
		{
		if (CopyFrom)
			{
			CurAction->Next->Copy(CopyFrom);
			} // if
		else
			CurAction->Next->SetActionType(GetDefaultActionType()) ;
		return (CurAction->Next);
		} // if
	} // if

return (NULL);

} // SXExtensionActionable::AddQueryAction

/*===========================================================================*/

SXQueryAction *SXExtensionActionable::RemoveQueryAction(SXQueryAction *RemoveMe)
{
SXQueryAction *CurAction = ActionList, *LastAction = NULL;

while (CurAction)
	{
	if (CurAction == RemoveMe)
		{
		if (CurAction == ActiveAction)
			ActiveAction = NULL;
		if (LastAction)
			LastAction->Next = CurAction->Next;
		else
			ActionList = CurAction->Next;
		delete CurAction;
		break;
		} // if
	LastAction = CurAction;
	CurAction = CurAction->Next;
	} // while

// returns next action if there is one
return (LastAction && LastAction->Next ? LastAction->Next: ActionList);

} // SXExtensionActionable::RemoveQueryAction

/*===========================================================================*/

SXQueryAction *SXExtensionActionable::VerifyActiveAction(SXQueryAction *VerifyMe)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	if (CurAction == VerifyMe)
		{
		return (CurAction);
		} // if
	CurAction = CurAction->Next;
	} // while

// returns first action if there is one
return (ActionList);

} // SXExtensionActionable::VerifyActiveAction

/*===========================================================================*/

void SXExtensionActionable::ResolveDBLoadLinkages(Database *HostDB, Joe **UniqueIDTable, unsigned long HighestDBID)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	CurAction->ResolveDBLoadLinkages(HostDB, UniqueIDTable, HighestDBID);
	CurAction = CurAction->Next;
	} // while

} // SXExtensionActionable::ResolveDBLoadLinkages

/*===========================================================================*/

int SXExtensionActionable::RemoveRAHost(RasterAnimHost *RemoveMe, RasterAnimHost *NotifyObj)
{
SXQueryAction *CurAction = ActionList;
int Removed = 0;

while (CurAction)
	{
	Removed += CurAction->RemoveRAHost(RemoveMe, NotifyObj);
	CurAction = CurAction->Next;
	} // while

return (Removed);

} // SXExtensionActionable::RemoveRAHost

/*===========================================================================*/

int SXExtensionActionable::InitToExport(const char *OutputFilePath)
{
#ifdef WCS_BUILD_SX2
SXQueryAction *CurAction = ActionList;

if (GlobalApp->Sentinal->CheckAuthFieldSX2())
	{
	while (CurAction)
		{
		if (! CurAction->InitToExport(OutputFilePath))
			return (0);
		CurAction = CurAction->Next;
		} // while
	} // if
#endif // WCS_BUILD_SX2

return (1);

} // SXExtensionActionable::InitToExport

/*===========================================================================*/

void SXExtensionActionable::CloseQueryFiles(void)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	CurAction->CloseActionFile();
	if (CurAction->Items && CurAction->Items->Me)
		{
		((SXQueryItem *)CurAction->Items->Me)->CloseObjectFile();
		break;
		} // if
	CurAction = CurAction->Next;
	} // while

} // SXExtensionActionable::CloseQueryFiles

/*===========================================================================*/

void SXExtensionActionable::CleanupFromExport(SceneExporter *MyExporter)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	CurAction->CleanupFromExport();
	CurAction = CurAction->Next;
	} // while

} // SXExtensionActionable::CleanupFromExport

/*===========================================================================*/

bool SXExtensionActionable::AcceptDragDrop(long DropID)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	if (CurAction == ActiveAction && CurAction->AcceptItemType(DropID))
		return (true);
	CurAction = CurAction->Next;
	} // while

return (false);

} // SXExtensionActionable::AcceptDragDrop

/*===========================================================================*/

int SXExtensionActionable::ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource, int GenNotify, RasterAnimHost *NotifyObj)
{
SXQueryAction *CurAction = ActionList;

while (CurAction)
	{
	if (CurAction == ActiveAction)
		return (CurAction->ProcessRAHostDragDrop(DropSource, GenNotify, NotifyObj));
	CurAction = CurAction->Next;
	} // while

return (false);

} // SXExtensionActionable::ProcessRAHostDragDrop


unsigned long SXExtensionActionable::SaveActions(FILE *ffile)
{
ULONG TotalWritten = 0;
#ifdef WCS_BUILD_SX2
long BytesWritten;
ULONG ItemTag;
SXQueryAction *SXQueryAct;
#endif // WCS_BUILD_SX2

#ifdef WCS_BUILD_SX2
if (GlobalApp->Sentinal->CheckAuthFieldSX2())
	{
	SXQueryAct = ActionList;
	while (SXQueryAct)
		{
		ItemTag = WCS_EFFECTS_SXEXTENSION_NVE_SXQUERYACTION + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			ItemTag = 0;
			if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
				WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
				{
				TotalWritten += BytesWritten;

				if (BytesWritten = SXQueryAct->Save(ffile))
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
					} // if file path saved 
				else
					goto WriteError;
				} // if size written 
			else
				goto WriteError;
			} // if tag written 
		else
			goto WriteError;
		SXQueryAct = SXQueryAct->Next;
		} // while
	} // if

return (TotalWritten);

WriteError:
#endif // WCS_BUILD_SX2

return (0L);

} // SXExtensionActionable::Save



/*===========================================================================*/
/*===========================================================================*/



SXExtensionNVE::SXExtensionNVE()
{

NavFollowTerrainHeight = 5.0;
NavFollowTerrainMaxHeight = 1000.0;
NavFriction = .05;
NavSpeed = 50.0;
NavAcceleration = 1.0;
NavInertia = 1.0;
NavStyle = WCS_SXEXTENSIONNVE_NAVSTYLE_SIMPLE;
NavConstrain = 1;
NavMaxHtConstrain = 0;
NavUseDefaultSpeed = 1;

LODMinFeatureSizePixels = 5;
LODMaxFoliageStems = 100000;
LODCompressTerrainTex = 0;
LODCompressFoliageTex = 0;
LODOptimizeMove = 0;

OverlayNumLines = 4;
OverlayShowMap = 1;
OverlayLogoText = WatermarkText = MetaName = MetaCopyright = MetaAuthor = MetaEmail =
	MetaUser1 = MetaUser2 = MetaUser3 = MetaUser4 = MetaUser5 = NULL;

OverlayLogoFileName.SetPath("WCSFrames:");

CreateString(&MetaName, GlobalApp->MainProj->projectname);
CreateString(&MetaAuthor, GlobalApp->MainProj->UserName);
CreateString(&MetaEmail, GlobalApp->MainProj->UserEmail);
// more sophisticated date function
time_t Now;
char *NowStr;
time(&Now);
NowStr = ctime(&Now);
// return from ctime is short-term modifiable
NowStr[24] = NULL; // remove EOL
CreateString(&MetaCopyright, NowStr);

} // SXExtensionNVE::SXExtensionNVE

/*===========================================================================*/

SXExtensionNVE::~SXExtensionNVE()
{

RemoveStrings();
RemoveActions();

} // SXExtensionNVE::~SXExtensionNVE

/*===========================================================================*/

void SXExtensionNVE::Copy(SXExtension *SXCopyFrom)
{
SXExtensionNVE *CopyFrom;

// copy only if correct type
if (SXCopyFrom->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_NVE)
	{
	CopyFrom = (SXExtensionNVE *)SXCopyFrom;
	CopyActions(SXCopyFrom); // copy stuff in SXExtensionActionable base class

	RemoveStrings();

	NavFollowTerrainHeight = CopyFrom->NavFollowTerrainHeight;
	NavFollowTerrainMaxHeight = CopyFrom->NavFollowTerrainMaxHeight;
	NavFriction = CopyFrom->NavFriction;
	NavSpeed = CopyFrom->NavSpeed;
	NavAcceleration = CopyFrom->NavAcceleration;
	NavInertia = CopyFrom->NavInertia;
	NavStyle = CopyFrom->NavStyle;
	NavConstrain = CopyFrom->NavConstrain;
	NavMaxHtConstrain = CopyFrom->NavMaxHtConstrain;
	NavUseDefaultSpeed = CopyFrom->NavUseDefaultSpeed;

	LODMinFeatureSizePixels = CopyFrom->LODMinFeatureSizePixels;
	LODMaxFoliageStems = CopyFrom->LODMaxFoliageStems;
	LODCompressTerrainTex = CopyFrom->LODCompressTerrainTex;
	LODCompressFoliageTex = CopyFrom->LODCompressFoliageTex;
	LODOptimizeMove = CopyFrom->LODOptimizeMove;

	OverlayNumLines = CopyFrom->OverlayNumLines;
	OverlayShowMap = CopyFrom->OverlayShowMap;
	OverlayLogoFileName.Copy(&OverlayLogoFileName, &CopyFrom->OverlayLogoFileName);
	if (CopyFrom->OverlayLogoText)
		CreateString(&OverlayLogoText, CopyFrom->OverlayLogoText);
	if (CopyFrom->WatermarkText)
		CreateString(&WatermarkText, CopyFrom->WatermarkText);
	if (CopyFrom->MetaName)
		CreateString(&MetaName, CopyFrom->MetaName);
	if (CopyFrom->MetaCopyright)
		CreateString(&MetaCopyright, CopyFrom->MetaCopyright);
	if (CopyFrom->MetaAuthor)
		CreateString(&MetaAuthor, CopyFrom->MetaAuthor);
	if (CopyFrom->MetaEmail)
		CreateString(&MetaEmail, CopyFrom->MetaEmail);
	if (CopyFrom->MetaUser1)
		CreateString(&MetaUser1, CopyFrom->MetaUser1);
	if (CopyFrom->MetaUser2)
		CreateString(&MetaUser2, CopyFrom->MetaUser2);
	if (CopyFrom->MetaUser3)
		CreateString(&MetaUser3, CopyFrom->MetaUser3);
	if (CopyFrom->MetaUser4)
		CreateString(&MetaUser4, CopyFrom->MetaUser4);
	if (CopyFrom->MetaUser5)
		CreateString(&MetaUser5, CopyFrom->MetaUser5);
	} // if

} // SXExtensionNVE::Copy

/*===========================================================================*/

void SXExtensionNVE::RemoveStrings(void)
{

if (OverlayLogoText)
	AppMem_Free(OverlayLogoText, strlen(OverlayLogoText) + 1);
if (WatermarkText)
	AppMem_Free(WatermarkText, strlen(WatermarkText) + 1);
if (MetaName)
	AppMem_Free(MetaName, strlen(MetaName) + 1);
if (MetaCopyright)
	AppMem_Free(MetaCopyright, strlen(MetaCopyright) + 1);
if (MetaAuthor)
	AppMem_Free(MetaAuthor, strlen(MetaAuthor) + 1);
if (MetaEmail)
	AppMem_Free(MetaEmail, strlen(MetaEmail) + 1);
if (MetaUser1)
	AppMem_Free(MetaUser1, strlen(MetaUser1) + 1);
if (MetaUser2)
	AppMem_Free(MetaUser2, strlen(MetaUser2) + 1);
if (MetaUser3)
	AppMem_Free(MetaUser3, strlen(MetaUser3) + 1);
if (MetaUser4)
	AppMem_Free(MetaUser4, strlen(MetaUser4) + 1);
if (MetaUser5)
	AppMem_Free(MetaUser5, strlen(MetaUser5) + 1);
OverlayLogoText = WatermarkText = MetaName = MetaCopyright = MetaAuthor = MetaEmail =
	MetaUser1 = MetaUser2 = MetaUser3 = MetaUser4 = MetaUser5 = NULL;

} // SXExtensionNVE::RemoveStrings

/*===========================================================================*/

char *SXExtensionNVE::CreateString(char **StringLoc, char *NewString)
{
long Len;

if (*StringLoc)
	AppMem_Free(*StringLoc, strlen(*StringLoc) + 1);
*StringLoc = NULL;
if (NewString && (Len = (long)strlen(NewString)))
	{
	if (*StringLoc = (char *)AppMem_Alloc(Len + 1, 0))
		{
		strcpy(*StringLoc, NewString);
		} // if
	} // if

return (*StringLoc);

} // SXExtensionNVE::CreateString

/*===========================================================================*/

bool SXExtensionNVE::ApproveActionAvailable(int TestType)
{

return (TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_HIGHLIGHTOBJECTSET // or Highlight Object, perhaps
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_LOADNEWSCENE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_RUNSHELLCOMMAND // NVX only
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_LAUNCHPLUGIN // NVX only
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_PLAYSOUNDINTERNAL
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGEINTERNAL
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_PLAYMEDIAFILEINTERNAL
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_VIEWTEXTFILE);

} // SXExtensionNVE::ApproveActionAvailable

/*===========================================================================*/

bool SXExtensionNVE::ApproveActionItemAvailable(int TestItem)
{

return (TestItem == WCS_RAHOST_OBJTYPE_VECTOR
	|| TestItem == WCS_EFFECTSSUBCLASS_OBJECT3D
	|| TestItem == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| TestItem == WCS_EFFECTSSUBCLASS_FENCE
	|| TestItem == WCS_EFFECTSSUBCLASS_LABEL);

} // SXExtensionNVE::ApproveActionItemAvailable

/*===========================================================================*/

char SXExtensionNVE::GetDefaultActionType(void)
{

return (WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYLABEL);

} // SXExtensionNVE::GetDefaultActionType

/*===========================================================================*/

unsigned long SXExtensionNVE::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char TempBuffer[2048];
#ifdef WCS_BUILD_SX2
SXQueryAction *SXQueryAct;
#endif // WCS_BUILD_SX2

RemoveActions();

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			//read block size from file
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
					case WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYLOGOTEXT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&OverlayLogoText, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_WATERMARKTEXT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&WatermarkText, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METANAME:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaName, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METACOPYRIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaCopyright, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAAUTHOR:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaAuthor, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAEMAIL:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaEmail, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAUSER1:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaUser1, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAUSER2:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaUser2, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAUSER3:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaUser3, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAUSER4:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaUser4, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_METAUSER5:
						{
						BytesRead = ReadBlock(ffile, (char *)&TempBuffer, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						if (TempBuffer[0])
							CreateString(&MetaUser5, TempBuffer);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVSTYLE:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavStyle, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVCONSTRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavConstrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVMAXHTCONSTRAIN:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavMaxHtConstrain, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVUSEDEFAULTSPEED:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavUseDefaultSpeed, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_LODCOMPRESSTERRAINTEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODCompressTerrainTex, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_LODCOMPRESSFOLIAGETEX:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODCompressFoliageTex, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_LODOPTIMIZEMOVE:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODOptimizeMove, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYSHOWMAP:
						{
						BytesRead = ReadBlock(ffile, (char *)&OverlayShowMap, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_LODMINFEATURESIZEPIXELS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODMinFeatureSizePixels, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_LODMAXFOLIAGESTEMS:
						{
						BytesRead = ReadBlock(ffile, (char *)&LODMaxFoliageStems, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYNUMLINES:
						{
						BytesRead = ReadBlock(ffile, (char *)&OverlayNumLines, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVFOLLOWTERRAINHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavFollowTerrainHeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVFOLLOWTERRAINMAXHEIGHT:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavFollowTerrainMaxHeight, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVFRICTION:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavFriction, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVSPEED:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavSpeed, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVACCELERATION:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavAcceleration, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_NAVINERTIA:
						{
						BytesRead = ReadBlock(ffile, (char *)&NavInertia, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYLOGOFILENAME:
						{
						BytesRead = OverlayLogoFileName.Load(ffile, Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_SX2
					case WCS_EFFECTS_SXEXTENSION_NVE_SXQUERYACTION:
						{
						if (GlobalApp->Sentinal->CheckAuthFieldSX2())
							{
							if (SXQueryAct = AddQueryAction(NULL))
								BytesRead = SXQueryAct->Load(ffile, Size, ByteFlip);
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_SX2
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

} // SXExtensionNVE::Load

/*===========================================================================*/

unsigned long SXExtensionNVE::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if (OverlayLogoText)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYLOGOTEXT, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(OverlayLogoText) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)OverlayLogoText)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (WatermarkText)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_WATERMARKTEXT, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(WatermarkText) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)WatermarkText)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaName)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METANAME, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaName) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaName)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaCopyright)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METACOPYRIGHT, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaCopyright) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaCopyright)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaAuthor)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAAUTHOR, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaAuthor) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaAuthor)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaEmail)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAEMAIL, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaEmail) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaEmail)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaUser1)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAUSER1, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaUser1) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaUser1)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaUser2)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAUSER2, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaUser2) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaUser2)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaUser3)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAUSER3, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaUser3) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaUser3)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaUser4)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAUSER4, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaUser4) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaUser4)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if
if (MetaUser5)
	{
	if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_METAUSER5, WCS_BLOCKSIZE_SHORT,
		WCS_BLOCKTYPE_SHORTINT, (unsigned long)(strlen(MetaUser5) + 1),
		WCS_BLOCKTYPE_CHAR, (char *)MetaUser5)) == NULL)
		goto WriteError;
	TotalWritten += BytesWritten;
	} // if

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVSTYLE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NavStyle)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVCONSTRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NavConstrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVMAXHTCONSTRAIN, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NavMaxHtConstrain)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVUSEDEFAULTSPEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&NavUseDefaultSpeed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_LODCOMPRESSTERRAINTEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LODCompressTerrainTex)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_LODCOMPRESSFOLIAGETEX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LODCompressFoliageTex)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_LODOPTIMIZEMOVE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&LODOptimizeMove)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYSHOWMAP, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&OverlayShowMap)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_LODMINFEATURESIZEPIXELS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&LODMinFeatureSizePixels)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_LODMAXFOLIAGESTEMS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&LODMaxFoliageStems)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYNUMLINES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&OverlayNumLines)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVFOLLOWTERRAINHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavFollowTerrainHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVFOLLOWTERRAINMAXHEIGHT, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavFollowTerrainMaxHeight)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVFRICTION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavFriction)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVSPEED, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavSpeed)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVACCELERATION, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavAcceleration)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_NVE_NAVINERTIA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&NavInertia)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_SXEXTENSION_NVE_OVERLAYLOGOFILENAME + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = OverlayLogoFileName.Save(ffile))
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
			} // if file path saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

TotalWritten += SaveActions(ffile);

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

} // SXExtensionNVE::Save

/*===========================================================================*/
/*===========================================================================*/

SXExtensionFBX::SXExtensionFBX()
{

SaveV5 = EmbedMedia = UsePassword = 0;
SaveBinary = 1;
Password[0] = 0;

} // SXExtensionFBX::SXExtensionFBX

/*===========================================================================*/

SXExtensionFBX::~SXExtensionFBX()
{

} // SXExtensionFBX::~SXExtensionFBX

/*===========================================================================*/

void SXExtensionFBX::Copy(SXExtension *SXCopyFrom)
{
SXExtensionFBX *CopyFrom;

// copy only if correct type
if (SXCopyFrom->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_FBX)
	{
	CopyFrom = (SXExtensionFBX *)SXCopyFrom;

	SaveV5 = CopyFrom->SaveV5;
	EmbedMedia = CopyFrom->EmbedMedia;
	UsePassword = CopyFrom->UsePassword;
	SaveBinary = CopyFrom->SaveBinary;
	strcpy(Password, CopyFrom->Password);
	} // if

} // SXExtensionFBX::Copy

/*===========================================================================*/

unsigned long SXExtensionFBX::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
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
			//read block size from file
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
					case WCS_EFFECTS_SXEXTENSION_FBX_SAVEV5:
						{
						BytesRead = ReadBlock(ffile, (char *)&SaveV5, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_FBX_SAVEBINARY:
						{
						BytesRead = ReadBlock(ffile, (char *)&SaveBinary, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_FBX_EMBEDMEDIA:
						{
						BytesRead = ReadBlock(ffile, (char *)&EmbedMedia, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_FBX_USEPASSWORD:
						{
						BytesRead = ReadBlock(ffile, (char *)&UsePassword, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_FBX_PASSWORD:
						{
						BytesRead = ReadBlock(ffile, (char *)Password, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
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

} // SXExtensionFBX::Load

/*===========================================================================*/

unsigned long SXExtensionFBX::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_FBX_SAVEV5, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SaveV5)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_FBX_SAVEBINARY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&SaveBinary)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_FBX_EMBEDMEDIA, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UsePassword)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_FBX_USEPASSWORD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&UsePassword)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_FBX_PASSWORD, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Password) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Password)) == NULL)
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

} // SXExtensionFBX::Save

/*===========================================================================*/

void SXExtensionFBX::SetPassword(char *NewPassword)
{

strncpy(Password, NewPassword, 31);
Password[31] = 0;

} // SXExtensionFBX::SetPassword


/*===========================================================================*/
/*===========================================================================*/

SXExtensionGE::SXExtensionGE()
{

Reverse3DNormals = DrawOrder = 0;
FoliageRescale = LabelRescale = 100.0;
overlayX = 0.0;
overlayY = 1.0;
Message[0] = 0;

} // SXExtensionGE::SXExtensionGE

/*===========================================================================*/

SXExtensionGE::~SXExtensionGE()
{

} // SXExtensionGE::~SXExtensionGE

/*===========================================================================*/

void SXExtensionGE::Copy(SXExtension *SXCopyFrom)
{
SXExtensionGE *CopyFrom;

// copy only if correct type
if (SXCopyFrom->GetType() == WCS_EFFECTS_SCENEEXPORTER_EXTENSIONTYPE_GE)
	{
	CopyFrom = (SXExtensionGE *)SXCopyFrom;
	CopyActions(SXCopyFrom); // copy stuff in SXExtensionActionable base class

	Reverse3DNormals = CopyFrom->Reverse3DNormals;
	DrawOrder = CopyFrom->DrawOrder;
	FoliageRescale = CopyFrom->FoliageRescale;
	LabelRescale = CopyFrom->LabelRescale;
	overlayX = CopyFrom->overlayX;
	overlayY = CopyFrom->overlayY;
	strcpy(Message, CopyFrom->Message);
	overlayFilename.Copy(&overlayFilename, &CopyFrom->overlayFilename);
	} // if

} // SXExtensionGE::Copy

/*===========================================================================*/

bool SXExtensionGE::ApproveActionAvailable(int TestType)
{

return (TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYWEBPAGE
	|| TestType == WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYIMAGEINTERNAL);

} // SXExtensionGE::ApproveActionAvailable

/*===========================================================================*/

bool SXExtensionGE::ApproveActionItemAvailable(int TestItem)
{

return (TestItem == WCS_EFFECTSSUBCLASS_FOLIAGE
	|| TestItem == WCS_EFFECTSSUBCLASS_OBJECT3D
	|| TestItem == WCS_EFFECTSSUBCLASS_FENCE
	|| TestItem == WCS_EFFECTSSUBCLASS_LABEL);

} // SXExtensionGE::ApproveActionItemAvailable

/*===========================================================================*/

char SXExtensionGE::GetDefaultActionType(void)
{

return (WCS_SXQUERYACTION_ACTIONTYPE_DISPLAYDATATABLE);

} // SXExtensionGE::GetDefaultActionType


/*===========================================================================*/


void SXExtensionGE::CleanupFromExport(SceneExporter *MyExporter)
{ // cleanup unused action files we couldn't cleanup elsewhere because they were in use
char FullOutPath[512];


SXExtensionActionable::CleanupFromExport(MyExporter);

strmfp(FullOutPath, MyExporter->OutPath.GetPath(), "SXObjectIndex.nqx");
PROJ_remove(FullOutPath);
strmfp(FullOutPath, MyExporter->OutPath.GetPath(), "SXObjectRecords.nqa");
PROJ_remove(FullOutPath);
strmfp(FullOutPath, MyExporter->OutPath.GetPath(), "SXActionIndex.nqx");
PROJ_remove(FullOutPath);
strmfp(FullOutPath, MyExporter->OutPath.GetPath(), "SXActionRecords.nqa");
PROJ_remove(FullOutPath);

} // SXExtensionGE::CleanupFromExport


/*===========================================================================*/

unsigned long SXExtensionGE::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
#ifdef WCS_BUILD_SX2
SXQueryAction *SXQueryAct;
#endif // WCS_BUILD_SX2

RemoveActions();

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			//read block size from file
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
					case WCS_EFFECTS_SXEXTENSION_GE_REVERSENORMALS:
						{
						BytesRead = ReadBlock(ffile, (char *)&Reverse3DNormals, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_DRAWORDER:
						{
						BytesRead = ReadBlock(ffile, (char *)&DrawOrder, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_FOLIAGERESCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&FoliageRescale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_LABELRESCALE:
						{
						BytesRead = ReadBlock(ffile, (char *)&LabelRescale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_OVERLAYX:
						{
						BytesRead = ReadBlock(ffile, (char *)&overlayX, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_OVERLAYY:
						{
						BytesRead = ReadBlock(ffile, (char *)&overlayY, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_MESSAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)Message, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_EFFECTS_SXEXTENSION_GE_OVERLAYFNAME:
						{
						BytesRead = overlayFilename.Load(ffile, Size, ByteFlip);
						break;
						}
					#ifdef WCS_BUILD_SX2
					case WCS_EFFECTS_SXEXTENSION_NVE_SXQUERYACTION:
						{
						if (GlobalApp->Sentinal->CheckAuthFieldSX2())
							{
							if (SXQueryAct = AddQueryAction(NULL))
								BytesRead = SXQueryAct->Load(ffile, Size, ByteFlip);
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // if
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						}
					#endif // WCS_BUILD_SX2
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

} // SXExtensionGE::Load

/*===========================================================================*/

unsigned long SXExtensionGE::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_REVERSENORMALS, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Reverse3DNormals)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_DRAWORDER, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DrawOrder)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_FOLIAGERESCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&FoliageRescale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_LABELRESCALE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&LabelRescale)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_OVERLAYX, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&overlayX)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_OVERLAYY, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_DOUBLE,
	WCS_BLOCKTYPE_DOUBLE, (char *)&overlayY)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_EFFECTS_SXEXTENSION_GE_MESSAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Message) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Message)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

ItemTag = WCS_EFFECTS_SXEXTENSION_GE_OVERLAYFNAME + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
	{
	TotalWritten += BytesWritten;

	ItemTag = 0;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		if (BytesWritten = overlayFilename.Save(ffile))
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
			} // if file path saved 
		else
			goto WriteError;
		} // if size written 
	else
		goto WriteError;
	} // if tag written 
else
	goto WriteError;

TotalWritten += SaveActions(ffile);

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

} // SXExtensionGE::Save

/*===========================================================================*/

void SXExtensionGE::SetMessage(char *NewMessage)
{

strncpy(Message, NewMessage, 80);
Message[80] = 0;

} // SXExtensionGE::SetMessage

/*===========================================================================*/
/*===========================================================================*/
