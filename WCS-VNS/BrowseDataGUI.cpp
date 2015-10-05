// BrowseDataGUI.cpp
// Code for ComponentBrowseData
// Built from scratch on 7/11/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "BrowseDataGUI.h"
#include "WCSWidgets.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "EffectsLib.h"
#include "RasterAnimHost.h"
#include "Raster.h"
#include "Notify.h"
#include "AppMem.h"
#include "Interactive.h"
#include "Conservatory.h"
#include "CoordSysEditGUI.h"
#include "FSSupport.h"
#include "resource.h"

// lets you modify a component author name
//#define WCS_ALLOW_AUTHOR_MODIFICATION

/*===========================================================================*/
/*===========================================================================*/

NativeGUIWin BrowseDataGUI::Open(Project *Proj)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Proj))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // BrowseDataGUI::Open

/*===========================================================================*/

NativeGUIWin BrowseDataGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_COMPONENT_INFO, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureDD(NativeWin, IDC_ACTIVEDIR, ActiveDir, 255, NULL, 0, IDC_LABEL_DEFDIR);
		ConfigureWidgets();
		SetUserData();
		} // if
	} // if
 
return (NativeWin);

} // BrowseDataGUI::Construct

/*===========================================================================*/

BrowseDataGUI::BrowseDataGUI(EffectsLib *EffectsSource, RasterAnimHost *ActiveSource)
: GUIFenetre('BSIN', this, "Component Signature") // Yes, I know...
{
static NotifyTag ThumbnailEvent[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0xff, 0xff),
								0};
char NameStr[256];
RasterAnimHostProperties Prop;

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
NumCategories = 0;
CategoryNames = NULL;
strcpy(ActiveDir, "WCSContent:");
AuthorChunk = NULL;
DefaultExt[0] = 0;
ActiveObjType = -1;
Listening = 0;
GoneModal = 0;

if (EffectsSource && ActiveSource)
	{
	Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_FILEINFO;
	Active->GetRAHostProperties(&Prop);
	sprintf(NameStr, "Component Signature - %s %s", Prop.Name, Prop.Type);
	SetTitle(NameStr);
	strcpy(DefaultName, Prop.Name);
	// determine the appropriate extension from the RAHost typenumber
	if (Prop.Ext)
		strcpy(DefaultExt, Prop.Ext);
	if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_PROJECT)
		strcpy(ActiveDir, Prop.Path);
	else
		strcat(ActiveDir, Prop.Path);
	ActiveObjType = Prop.TypeNumber;
	if (AuthorChunk = new BrowseData())
		{
		if (Active->BrowseInfo)
			{
			AuthorChunk->Copy(AuthorChunk, Active->BrowseInfo);
			} // if
		} // if
	else
		ConstructError = 1;
	if (MyRast = new Raster())
		{
		MyRast->PAF.SetPath(GlobalApp->MainProj->imagepath[0] ? GlobalApp->MainProj->imagepath: (char*)"WCSFrames:");
		strcpy(BackupPath, MyRast->PAF.GetPath());
		strcpy(BackupImage, MyRast->PAF.GetName());
		} // if
	else
		ConstructError = 1;
	GlobalApp->AppEx->RegisterClient(this, ThumbnailEvent);
	} // if
else
	ConstructError = 1;

if (ActiveObjType == WCS_EFFECTSSUBCLASS_COORDSYS && GlobalApp->GUIWins->COS && (GoneModal = GlobalApp->GUIWins->COS->GoneModal))
	{
	GoModal();
	} // if

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // BrowseDataGUI::BrowseDataGUI

/*===========================================================================*/

BrowseDataGUI::~BrowseDataGUI()
{

GlobalApp->AppEx->RemoveClient(this);

ClearAll();
if (AuthorChunk)
	delete AuthorChunk;
AuthorChunk = NULL;

if (MyRast)
	{
	MyRast->Thumb = NULL;
	delete MyRast;
	} // if
MyRast = NULL;

GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (GoneModal)
	EndModal();

} // BrowseDataGUI::~BrowseDataGUI()

/*===========================================================================*/

void BrowseDataGUI::ClearAll(void)
{
long Ct;

if (CategoryNames)
	{
	for (Ct = 0; Ct < NumCategories; Ct ++)
		{
		if (CategoryNames[Ct])
			AppMem_Free(CategoryNames[Ct], strlen(CategoryNames[Ct]) + 1);
		} // for
	AppMem_Free(CategoryNames, NumCategories * sizeof (char *));
	CategoryNames = NULL;
	} // if

NumCategories = 0;

} // BrowseDataGUI::ClearAll

/*===========================================================================*/

long BrowseDataGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_BRD, 0);

return(0);

} // BrowseDataGUI::HandleCloseWin

/*===========================================================================*/

long BrowseDataGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
switch(ButtonID)
	{
	case ID_KEEP:
		{
		if (SaveData(0))
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_BRD, 0);
		break;
		} // 
	case IDC_SAVE:
		{
		if (SaveData(1))
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_BRD, 0);
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_BRD, 0);
		break;
		} // 
	case IDC_NEWCATEGORY:
		{
		NewCategory();
		break;
		} // 
	case IDC_TNAIL1:
		{
		Listening = 1;
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // BrowseDataGUI::HandleButtonClick

/*===========================================================================*/

long BrowseDataGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL1:
		{
		ClearThumbnail();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // BrowseDataGUI::HandleButtonDoubleClick

/*===========================================================================*/

long BrowseDataGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_ACTIVEDIR:
		{
		ConfigureWidgets();
		break;
		} // IDC_ACTIVEDIR
	case IDC_SELECT_IMAGE:
		{
		NewImage();
		break;
		} // IDC_SELECT_IMAGE
	default:
		break;
	} // ID

return(0);

} // BrowseDataGUI::HandleDDChange

/*===========================================================================*/

void BrowseDataGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (Listening)
		CopyThumbnail((Thumbnail *)Activity->ChangeNotify->NotifyData);
	} // if

} // BrowseDataGUI::HandleNotifyEvent()

/*===========================================================================*/

void BrowseDataGUI::ConfigureWidgets(void)
{
char NameStr[256];
WIN32_FIND_DATA FileData;
HANDLE Hand;
long Ct = 0, NumItems;
BrowseData *BrowseList = NULL;

ClearAll();

ConfigureDD(NativeWin, IDC_SELECT_IMAGE, (char *)MyRast->PAF.GetPath(), 255, (char *)MyRast->PAF.GetName(), 63, IDC_LABEL_DEFDIR);

strcpy(NameStr, GlobalApp->MainProj->MungPath(ActiveDir));

if (NameStr[strlen(NameStr) - 1] == '\\')
	strcat(NameStr, "*.");
else
	strcat(NameStr, "\\*.");
strcat(NameStr, DefaultExt);

NumItems = 0;
NumCategories = 0;

if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
	{
	NumItems ++;
	while (FindNextFile(Hand, &FileData))
		{
		NumItems ++;
		} // while
	FindClose(Hand);
	} // if

if (! NumItems || (BrowseList = new BrowseData[NumItems]))
	{
	if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
		{
		// read browse data
		strmfp(NameStr, ActiveDir, FileData.cFileName);
		BrowseList[Ct].LoadBrowseInfo(NameStr);
		Ct ++;
		while (Ct < NumItems && FindNextFile(Hand, &FileData))
			{
			strmfp(NameStr, ActiveDir, FileData.cFileName);
			BrowseList[Ct].LoadBrowseInfo(NameStr);
			Ct ++;
			} // while
		FindClose(Hand);
		} // if
	// count categories
	if (NumCategories = CountCategories(BrowseList, NumItems))
		{
		if (CategoryNames = (char **)AppMem_Alloc(NumCategories * sizeof (char *), APPMEM_CLEAR))
			{
			FillCategories(BrowseList, NumItems);
			} // if
		} // if
	BuildCategoryList();
	if (BrowseList)
		delete [] BrowseList;
	} // if

} // BrowseDataGUI::ConfigureWidgets()

/*===========================================================================*/

void BrowseDataGUI::ConfigureImage(void)
{
Raster *TempRast;

MyRast->Thumb = AuthorChunk->Thumb;
TempRast = AuthorChunk->Thumb ? MyRast: NULL;
ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, TempRast);

} // BrowseDataGUI::ConfigureImage

/*===========================================================================*/

long BrowseDataGUI::CountCategories(BrowseData *BrowseList, long NumItems)
{
long Ct, Test, Found;

NumCategories = 1;

for (Ct = 0; Ct < NumItems; Ct ++)
	{
	if (BrowseList[Ct].Category && stricmp(BrowseList[Ct].Category, "General"))
		{
		Found = 0;
		for (Test = 0; Test < Ct; Test ++)
			{
			if (BrowseList[Test].Category && BrowseList[Ct].Category && 
				! stricmp(BrowseList[Test].Category, BrowseList[Ct].Category))
				{
				Found = 1;
				break;
				} // if found match
			} // for
		if (! Found)
			NumCategories ++;
		} // if
	} // for
if (AuthorChunk->Category && stricmp(AuthorChunk->Category, "General"))
	{
	Found = 0;
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if (BrowseList[Ct].Category && ! stricmp(BrowseList[Ct].Category, AuthorChunk->Category))
			{
			Found = 1;
			break;
			} // if
		} // for
	if (! Found)
		NumCategories ++;
	} // if

NumCategories += 1;		// allow room for user to add one

return (NumCategories);

} // BrowseDataGUI::CountCategories

/*===========================================================================*/

void BrowseDataGUI::FillCategories(BrowseData *BrowseList, long NumItems)
{
long Ct, Test, Found, Filled = 0;

ActiveCategory = -1;

if (CategoryNames[Filled] = (char *)AppMem_Alloc(strlen("General") + 1, 0))
	{
	strcpy(CategoryNames[Filled], "General");
	if (AuthorChunk->Category && ! stricmp(CategoryNames[Filled], AuthorChunk->Category))
		ActiveCategory = Filled;
	Filled ++;
	} // if
for (Ct = 0; Ct < NumItems && Filled < NumCategories; Ct ++)
	{
	if (BrowseList[Ct].Category && stricmp(BrowseList[Ct].Category, "General"))
		{
		Found = 0;
		for (Test = 0; Test < Ct; Test ++)
			{
			if (BrowseList[Test].Category && BrowseList[Ct].Category && 
				! stricmp(BrowseList[Test].Category, BrowseList[Ct].Category))
				{
				Found = 1;
				break;
				} // if found match
			} // for
		if (! Found)
			{
			if (CategoryNames[Filled] = (char *)AppMem_Alloc(strlen(BrowseList[Ct].Category) + 1, 0))
				{
				strcpy(CategoryNames[Filled], BrowseList[Ct].Category);
				if (ActiveCategory < 0 && AuthorChunk->Category && ! stricmp(CategoryNames[Filled], AuthorChunk->Category))
					ActiveCategory = Filled;
				Filled ++;
				} // if
			} // if
		} // if
	} // for
if (ActiveCategory < 0 && Filled < NumCategories && AuthorChunk->Category)
	{
	if (CategoryNames[Filled] = (char *)AppMem_Alloc(strlen(AuthorChunk->Category) + 1, 0))
		{
		strcpy(CategoryNames[Filled], AuthorChunk->Category);
		ActiveCategory = Filled;
		} // if
	} // if
if (ActiveCategory < 0)
	ActiveCategory = 0;

} // BrowseDataGUI::FillCategories

/*===========================================================================*/

void BrowseDataGUI::BuildCategoryList(void)
{
long Ct;

WidgetLBClear(IDC_CATEGORYLIST);

for (Ct = 0; Ct < NumCategories; Ct ++)
	{
	if (CategoryNames[Ct])
		WidgetLBInsert(IDC_CATEGORYLIST, -1, CategoryNames[Ct]);
	} // for
WidgetLBSetCurSel(IDC_CATEGORYLIST, ActiveCategory);

} // BrowseDataGUI::BuildCategoryList

/*===========================================================================*/

int BrowseDataGUI::SaveData(int SaveIt)
{
char filename[256], NameStash[WCS_EFFECT_MAXNAMELENGTH];
RasterAnimHostProperties Prop;
int NameSubstitute = 1, SaveResult;

CaptureUserData();

if (AuthorChunk->Comment && AuthorChunk->Author)
	{
	if (Active->BrowseInfo || (Active->BrowseInfo = new BrowseData()))
		{
		Active->BrowseInfo->Copy(Active->BrowseInfo, AuthorChunk);
		if (SaveIt)
			{
			WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH - 1, DefaultName);
			if (DefaultName[0])
				{
				Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME;
				Prop.FlagsMask = WCS_RAHOST_FLAGBIT_EDITNAME;
				Active->GetRAHostProperties(&Prop);
				if (Prop.Flags & WCS_RAHOST_FLAGBIT_EDITNAME)
					{
					if (strcmp(DefaultName, Prop.Name))
						{
						strcpy(NameStash, Prop.Name);
						NameSubstitute = UserMessageYN(Prop.Name, "Do you wish to change the name of this component in memory to the \"Save As\" name?");
						Prop.PropMask = WCS_RAHOST_MASKBIT_NAME;
						Prop.FlagsMask = 0;
						Prop.Name = DefaultName;
						Active->SetRAHostProperties(&Prop);
						} // if
					} // if
				Prop.PropMask = 0;
				Prop.FlagsMask = 0;
				Prop.Name = NULL;
				strmfp(filename, ActiveDir, DefaultName);
				if ((strlen(filename) < strlen(DefaultExt) + 1) 
					|| (filename[strlen(filename) - strlen(DefaultExt) - 1] != '.')
					|| (stricmp(&filename[strlen(filename) - strlen(DefaultExt)], DefaultExt)))
					{
					strcat(filename, ".");
					strcat(filename, DefaultExt);
					} // if
				Prop.Path = filename;
				SaveResult = Active->SaveFilePrep(&Prop);
				if (! NameSubstitute)
					{
					Prop.PropMask = WCS_RAHOST_MASKBIT_NAME;
					Prop.FlagsMask = 0;
					Prop.Name = NameStash;
					Active->SetRAHostProperties(&Prop);
					} // if
				return (SaveResult > 0);
				} // if
			} // if
		} // if
	} // if
else
	{
	UserMessageOK("Save Component", "Some fields have not been completed. Please enter at least your name and a comment.");
	return (0);
	} // else

return (1);

} // BrowseDataGUI::SaveData

/*===========================================================================*/

void BrowseDataGUI::SetUserData(void)
{
char *TextPtr;
time_t NowTime;
unsigned long Length;
char DateStr[40];

WidgetSetText(IDC_NAME, DefaultName);

if (AuthorChunk->Author)	// if an author string already exists
	{
	#ifdef WCS_ALLOW_AUTHOR_MODIFICATION
	SendMessage(GetWidgetFromID(IDC_AUTHOR), EM_SETREADONLY, 0, 0);
	#else // WCS_ALLOW_AUTHOR_MODIFICATION
	SendMessage(GetWidgetFromID(IDC_AUTHOR), EM_SETREADONLY, 1, 0);
	#endif // WCS_ALLOW_AUTHOR_MODIFICATION
	SendMessage(GetWidgetFromID(IDC_AUTHOR2), EM_SETREADONLY, 0, 0);
	WidgetSetText(IDC_AUTHOR, AuthorChunk->Author);
	WidgetSetText(IDC_AUTHOR2, AuthorChunk->Author2 ? AuthorChunk->Author2: 
		! GlobalApp->MainProj->UserName[0] ? "Your name":
		stricmp(AuthorChunk->Author, GlobalApp->MainProj->UserName) ? GlobalApp->MainProj->UserName: "");
	WidgetSetModified(IDC_AUTHOR, 0);
	WidgetSetModified(IDC_AUTHOR2, AuthorChunk->Author2 ? 0: 
		! GlobalApp->MainProj->UserName[0] ? 0: 
		stricmp(AuthorChunk->Author, GlobalApp->MainProj->UserName) ? 1: 0);
	} // if
else
	{
	SendMessage(GetWidgetFromID(IDC_AUTHOR), EM_SETREADONLY, 0, 0);
	SendMessage(GetWidgetFromID(IDC_AUTHOR2), EM_SETREADONLY, 1, 0);
	WidgetSetText(IDC_AUTHOR, GlobalApp->MainProj->UserName[0] ? GlobalApp->MainProj->UserName: "Your name");
	WidgetSetText(IDC_AUTHOR2, "");
	WidgetSetModified(IDC_AUTHOR, GlobalApp->MainProj->UserName[0] ? 1: 0);
	WidgetSetModified(IDC_AUTHOR2, 0);
	} // else

time(&NowTime);
strcpy(DateStr, (TextPtr = ctime(&NowTime)) ? TextPtr: "");
DateStr[24] = NULL;
if ((Length = (unsigned long)strlen(DateStr)) > 0)
	{
	AuthorChunk->FreeDate();
	if (TextPtr = AuthorChunk->NewDate(Length + 1))
		strcpy(TextPtr, DateStr);
	} // if

WidgetSetText(IDC_COMMENT, AuthorChunk->Comment ? AuthorChunk->Comment: 
	"Enter a comment here");
WidgetSetText(IDC_ADDRESS, AuthorChunk->Address ? AuthorChunk->Address: 
	GlobalApp->MainProj->UserEmail[0] ? GlobalApp->MainProj->UserEmail:"Your Email address");

WidgetSetText(IDC_DATE, DateStr);
WidgetSetModified(IDC_COMMENT, 0);
WidgetSetModified(IDC_ADDRESS, AuthorChunk->Address ? 0: 
	GlobalApp->MainProj->UserEmail[0] ? 1: 0);

ConfigureImage();

} // BrowseDataGUI::SetUserData

/*===========================================================================*/

void BrowseDataGUI::NewImage(void)
{

if (ImageNameChanged())
	{
	if (AuthorChunk->GetThumb() || AuthorChunk->SetThumb(new Thumbnail()))
		{
		if (MyRast->LoadnPrepImage(FALSE, FALSE))
			{
			if (AuthorChunk->Thumb != MyRast->Thumb)
				{
				AuthorChunk->Thumb->Copy(AuthorChunk->Thumb, MyRast->Thumb);
				delete MyRast->Thumb;
				MyRast->Thumb = NULL;
				} // if
			} // if
		} // if
	ConfigureImage();
	strcpy(GlobalApp->MainProj->imagepath, MyRast->PAF.GetPath());
	strcpy(BackupPath, MyRast->PAF.GetPath());
	strcpy(BackupImage, MyRast->PAF.GetName());
	} // if

} // BrowseDataGUI::NewImage

/*===========================================================================*/

void BrowseDataGUI::CopyThumbnail(Thumbnail *NewThumb)
{

if (Listening && NewThumb)
	{
	if (AuthorChunk->GetThumb() || AuthorChunk->SetThumb(new Thumbnail()))
		{
		AuthorChunk->Thumb->Copy(AuthorChunk->Thumb, NewThumb);
		} // if
	ConfigureImage();
	Listening = 0;
	} // if

} // BrowseDataGUI::CopyThumbnail

/*===========================================================================*/

void BrowseDataGUI::ClearThumbnail(void)
{

if (AuthorChunk->GetThumb())
	{
	AuthorChunk->FreeThumb();
	} // if
ConfigureImage();
Listening = 0;

} // BrowseDataGUI::ClearThumbnail

/*===========================================================================*/

int BrowseDataGUI::ImageNameChanged(void)
{

return (strcmp(BackupPath, MyRast->PAF.GetPath()) || strcmp(BackupImage, MyRast->PAF.GetName()));

} // BrowseDataGUI::ImageNameChanged

/*===========================================================================*/

void BrowseDataGUI::NewCategory(void)
{
char NewName[256];

NewName[0] = 0;

if (GetInputString("Enter name for new category.", "", NewName) && NewName[0])
	{
	if (CategoryNames[NumCategories - 1])
		AppMem_Free(CategoryNames[NumCategories - 1], strlen(CategoryNames[NumCategories - 1]) + 1);
	if (CategoryNames[NumCategories - 1] = (char *)AppMem_Alloc(strlen(NewName) + 1, 0))
		{
		strcpy(CategoryNames[NumCategories - 1], NewName);
		ActiveCategory = NumCategories - 1;
		} // if
	} // if
BuildCategoryList();

} // BrowseDataGUI::NewCategory

/*===========================================================================*/

void BrowseDataGUI::CaptureUserData(void)
{
unsigned long Length;
long Current;
char *TextPtr;

if (WidgetGetModified(IDC_COMMENT))
	{
	if ((Length = SendMessage(GetWidgetFromID(IDC_COMMENT), WM_GETTEXTLENGTH, 0, 0)) > 0)
		{
		if (TextPtr = AuthorChunk->NewComment(Length + 1))
			WidgetGetText(IDC_COMMENT, Length + 1, TextPtr);
		} // if
	} // if
if (WidgetGetModified(IDC_AUTHOR))
	{
	if ((Length = SendMessage(GetWidgetFromID(IDC_AUTHOR), WM_GETTEXTLENGTH, 0, 0)) > 0)
		{
		if (TextPtr = AuthorChunk->NewAuthor(Length + 1))
			{
			WidgetGetText(IDC_AUTHOR, Length + 1, TextPtr);
			strncpy(GlobalApp->MainProj->UserName, TextPtr, 127);
			GlobalApp->MainProj->UserName[127] = 0;
			} // if
		} // if
	} // if
if (WidgetGetModified(IDC_AUTHOR2))
	{
	if ((Length = SendMessage(GetWidgetFromID(IDC_AUTHOR2), WM_GETTEXTLENGTH, 0, 0)) > 0)
		{
		if (TextPtr = AuthorChunk->NewAuthor2(Length + 1))
			{
			WidgetGetText(IDC_AUTHOR2, Length + 1, TextPtr);
			strncpy(GlobalApp->MainProj->UserName, TextPtr, 127);
			GlobalApp->MainProj->UserName[127] = 0;
			} // if
		} // if
	} // if
if (WidgetGetModified(IDC_ADDRESS))
	{
	if ((Length = SendMessage(GetWidgetFromID(IDC_ADDRESS), WM_GETTEXTLENGTH, 0, 0)) > 0)
		{
		if (TextPtr = AuthorChunk->NewAddress(Length + 1))
			{
			WidgetGetText(IDC_ADDRESS, Length + 1, TextPtr);
			strncpy(GlobalApp->MainProj->UserEmail, TextPtr, 127);
			GlobalApp->MainProj->UserEmail[127] = 0;
			} // if
		} // if
	} // if
Current = WidgetLBGetCurSel(IDC_CATEGORYLIST);
if (Current < 0 || Current >= NumCategories)
	Current = 0;
if (TextPtr = AuthorChunk->NewCategory((long)strlen(CategoryNames[Current]) + 1))
	strcpy(TextPtr, CategoryNames[Current]);

} // BrowseDataGUI::CaptureUserData
