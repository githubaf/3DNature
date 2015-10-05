// GalleryGUI.cpp
// Code for ComponentGallery
// Built from scratch on 7/11/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "GalleryGUI.h"
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
#include "Conservatory.h"
#include "SceneViewGUI.h"
#include "CoordSysEditGUI.h"
#include "FSSupport.h"
#include "resource.h"

/*===========================================================================*/

// This junk is shared between the following two functions.
// Very hackish.
// Less so now that they're in File Scope - FPW2
static BrowseData BCurrentItem;
static Raster Blaster;
static WIN32_FIND_DATA FileData;
static char INameStr[256], IdxOutPath[256], IdxOutName[100];

HANDLE IHand = NULL;

int DoIcon = 1, DoName = 1, DoAuth = 1, DoAuth2 = 1, DoComment = 1,
 DoDate = 1, DoAddr = 1, DoCat = 1;

void GalleryGUI::WriteHTMLComponentIndex(FILE *HTML, char *OutputPath, char *ComponentPath, char *ComponentWild)
{

IHand = NULL;

if(HTML)
	{
	strmfp(INameStr, GlobalApp->MainProj->MungPath(ComponentPath), ComponentWild);

	do
		{
		if(!IHand) // first iteration
			{
			IHand = FindFirstFile(INameStr, &FileData);
			} // if
		else // subsequent iterations
			{
			if(!(FindNextFile(IHand, &FileData))) break;
			} // else

		if((IHand != NULL) && (IHand != INVALID_HANDLE_VALUE))
			{
			if(FileData.cFileName[0] != '.')
				{
				if(!(FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
					{
					strmfp(INameStr, GlobalApp->MainProj->MungPath(ComponentPath), FileData.cFileName);
					if (BCurrentItem.LoadBrowseInfo(INameStr))
						{
						if(DoIcon || DoName)
							{
							sprintf(INameStr, "%s.jpg", FileData.cFileName);
							} // if
						if(DoIcon && BCurrentItem.Thumb && BCurrentItem.Thumb->TNail[0])
							{
							BCurrentItem.Thumb->ClearPadArea();
							Blaster.Rows = Blaster.Cols = 100;
							Blaster.ByteBands = 3;
							Blaster.PAF.SetPathAndName(GlobalApp->MainProj->MungPath(OutputPath), INameStr);
							Blaster.ByteMap[0] = Blaster.Red   = BCurrentItem.Thumb->TNail[0];
							Blaster.ByteMap[1] = Blaster.Green = BCurrentItem.Thumb->TNail[1];
							Blaster.ByteMap[2] = Blaster.Blue  = BCurrentItem.Thumb->TNail[2];
							Blaster.SaveImage(1);
							} // if
						fprintf(HTML, "<tr>\n");
						if(DoIcon)
							{
							if(DoIcon) fprintf(HTML, "<td width=\"100\" align=center valign=top><img src=\"%s\" width=100 height=100 border=0></td>", INameStr);
							} // if

						if(DoComment || DoName)
							{
							fprintf(HTML, "<td valign=top>");
							if(DoName) fprintf(HTML, "<b>%s</b>\n", FileData.cFileName);
							if(DoComment && DoName) fprintf(HTML, "<br>");
							if(DoComment) fprintf(HTML, "%s\n", BCurrentItem.Comment ? BCurrentItem.Comment : "");
							fprintf(HTML, "</td>");
							} // if
						if(DoAuth || DoAuth2 || DoDate || DoAddr)
							{
							fprintf(HTML, "<td valign=top>\n");
							/*fprintf(HTML, "<td>%s%s%s</td>\n", DoAuth ? (BCurrentItem.Author ? BCurrentItem.Author : "") : "",
							 (DoAuth && DoAuth2) ? "<br>" : "",
							 DoAuth2 ? (BCurrentItem.Author2 ? BCurrentItem.Author2 : "") : ""); */
							if(DoAuth) fprintf(HTML, "%s<br>\n", (BCurrentItem.Author ? BCurrentItem.Author : ""));
							if(DoAuth2) fprintf(HTML, "%s<br>\n", (BCurrentItem.Author2 ? BCurrentItem.Author2 : ""));
							if(DoDate) fprintf(HTML, "%s<br>\n", BCurrentItem.Date ? BCurrentItem.Date : "");
							if(DoAddr) fprintf(HTML, "%s<br>\n", BCurrentItem.Address ? BCurrentItem.Address : "");
							fprintf(HTML, "</td>\n");
							} // if

						if(DoCat) fprintf(HTML, "<td valign=top>%s</td>\n", BCurrentItem.Category ? BCurrentItem.Category : "");
						fprintf(HTML, "</tr>\n");

						if(DoIcon)
							{
							Blaster.ByteMap[0] = Blaster.Red   = NULL;
							Blaster.ByteMap[1] = Blaster.Green = NULL;
							Blaster.ByteMap[2] = Blaster.Blue  = NULL;
							Blaster.Rows = Blaster.Cols = 0;
							Blaster.ByteBands = 0;
							BCurrentItem.FreeAll();
							} // if
						} // if
					} // if
				} // if
			} // if
		} while ((IHand != NULL) && (IHand != INVALID_HANDLE_VALUE));
	if(IHand)
		{
		FindClose(IHand);
		} // if
	} // if

} // GalleryGUI::WriteHTMLComponentIndex

void GalleryGUI::WriteHTMLComponentIndex(char *OutputPath)
{
FILE *HTML = NULL;
FileReq FR;

if(!OutputPath)
	{
	FR.SetDefFile("index.html");
	if(FR.Request(WCS_REQUESTER_FILE_SAVE))
		{
		strcpy(INameStr, FR.GetFirstName());
		BreakFileName(INameStr, IdxOutPath, 250, IdxOutName, 100);
		OutputPath = IdxOutPath;
		} // if
	} // if

if(!OutputPath) return;

strmfp(INameStr, GlobalApp->MainProj->MungPath(OutputPath), "index.html");

if(HTML = fopen(INameStr, "w"))
	{
	fprintf(HTML, "<!doctype html public \"-//IETF//DTD HTML 3.0//EN\">\n");
	fprintf(HTML, "<html>\n");
	fprintf(HTML, "<head>\n");
	fprintf(HTML, "<title>Component Index</title>\n");
	fprintf(HTML, "</head>\n");
	fprintf(HTML, "<body>\n");
	fprintf(HTML, "<table border=1 cellspacing=0>\n");

	fprintf(HTML, "<tr>\n");
	if(DoIcon) fprintf(HTML, "<td width=\"100\" valign=top>Component</td>\n");
	if(DoComment || DoName) fprintf(HTML, "<td valign=top>%s%s%s</td>\n", (DoName ? "Name" : ""), (DoName && DoComment) ? "<br>" : "", (DoComment ? "Comment" : ""));
	if(DoAuth || DoAuth2 || DoDate || DoAddr)
		{
		fprintf(HTML, "<td valign=top>\n");
		//fprintf(HTML, "<td>%s%s%s</td>\n", DoAuth ? "Author" : "", (DoAuth && DoAuth2) ? "<br>" : "", DoAuth2 ? "Author2" : "");
		if(DoAuth) fprintf(HTML, "%s<br>\n", "Author");
		if(DoAuth2)fprintf(HTML, "%s<br>\n", "Author2");
		if(DoDate) fprintf(HTML, "%s<br>\n", "Date");
		if(DoAddr) fprintf(HTML, "%s<br>\n", "Address");
		fprintf(HTML, "</td>\n");
		} // if
	if(DoCat) fprintf(HTML, "<td valign=top>%s</td>\n", "Category");
	fprintf(HTML, "</tr>\n");

	// Projects
	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Projects");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Demos");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSProjects:Demos", "*.proj");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Others");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSProjects:", "*.proj");

	// 3D Objects
	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "3DObject");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Objects");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:3DObject", "*.w3d");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Materials");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:3DObject", "*.mat");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Walls");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:3DObject", "*.fnc");

	// LandCover
	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "LandCover");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Color Maps");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.cmp");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Ecosystems");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.eco");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Environments");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.env");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Foliage Effects");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.fol");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Ground Effects");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.gnd");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Snow Effects");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.sno");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Ecotypes");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.etp");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Foliage Groups");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.fgp");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Material Strata");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.mst");
	//fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Thematic Maps");
	//WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:LandCover", "*.thm");

	// Light
	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Light");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Lights");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Light", "*.lgt");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Shadows");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Light", "*.shd");

	// Render
	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Render");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Cameras");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Render", "*.cam");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Render Jobs");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Render", "*.rnj");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Render Options");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Render", "*.rno");

	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Sky");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Atmospheres");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.atm");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Atmosphere Constituants");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.atc");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Celestial Objects");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.cel");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Cloud Models");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.cld");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Skies");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.sky");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Starfields");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Sky", "*.stf");

	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Terrain");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Area Terraffectors");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.ata");
	//fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Control Points");
	//WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.");
	//fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Coordinate Systems");
	//WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.cos");
	//fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "DEMs");
	//WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Planet Options");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.plo");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Terraffectors");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.ter");
	//fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Terrain Generators");
	//WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.tgn");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Terrain Gridders");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.tgr");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Terrain Parameters");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Terrain", "*.tep");

	fprintf(HTML, "<tr><td colspan=7><h2>%s</h2></td></tr>\n", "Water");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Lakes");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Water", "*.lak");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Streams");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Water", "*.str");
	fprintf(HTML, "<tr><td ALIGN=center><h4>%s</h4></td></tr>\n", "Wave Models");
	WriteHTMLComponentIndex(HTML, OutputPath, "WCSContent:Water", "*.wve");

	fprintf(HTML, "</table>\n");
	fprintf(HTML, "</body></html>\n");
	fclose(HTML);

	} // if


} // GalleryGUI::WriteHTMLComponentIndex

/*===========================================================================*/
/*===========================================================================*/

NativeGUIWin GalleryGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // GalleryGUI::Open

/*===========================================================================*/

NativeGUIWin GalleryGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_COMPONENT_GALLERY, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_COMPONENT_TNAILS, 0, 0, true);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < 1; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, "General");
			} // for
		WidgetTCSetCurSel(IDC_TAB1, 0);
		ShowPanel(0, 0);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // GalleryGUI::Construct

/*===========================================================================*/

GalleryGUI::GalleryGUI(EffectsLib *EffectsSource, RasterAnimHost *ActiveSource)
: GUIFenetre('LOUV', this, "Component Gallery") // Yes, I know...
{
RasterAnimHostProperties Prop;
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
NumItems = NumValidItems = 0;
NameList = NULL;
NumCategories = ActiveRange = ActiveCategory = NumRanges = ActiveItem = 0;
NumCategoryItems = NULL;
InCategory = NULL;
CategoryNames = NULL;
strcpy(ActiveDir, "WCSContent:");
strcpy(ContentDir, "WCSContent:");
RasterList = NULL;
BrowseList = NULL;
DefaultExt[0] = 0;
ActiveObjType = -1;
NumObjTypes = 0;
FirstItemInRange = 0;
LastItemInRange = 0;
memset(&DisplayedItem[0], 0, WCS_GALLERY_NUMNAILS * sizeof (long));
GoneModal = 0;

if (EffectsSource)
	{
	if (Active)
		{
		NumObjTypes = 1;
		Prop.PropMask = WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_FILEINFO;
		Active->GetRAHostProperties(&Prop);
		sprintf(NameStr, "Component Gallery - %s %s", Prop.Name, Prop.Type);
		SetTitle(NameStr);
		// determine the appropriate extension from the RAHost typenumber
		if (Prop.Ext && Prop.Path)
			{
			strcpy(DefaultExt, Prop.Ext);
			strcat(ActiveDir, Prop.Path);
			} // if
		else
			ConstructError = 1;
		ActiveObjType = Prop.TypeNumber;
		} // if
	if (! (RasterList = new Raster[WCS_GALLERY_NUMNAILS]))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

if (ActiveObjType == WCS_EFFECTSSUBCLASS_COORDSYS && GlobalApp->GUIWins->COS && (GoneModal = GlobalApp->GUIWins->COS->GoneModal))
	{
	GoModal();
	} // if
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // GalleryGUI::GalleryGUI

/*===========================================================================*/

GalleryGUI::~GalleryGUI()
{
long Ct;

ClearAll();

if (RasterList)
	{
	for (Ct = 0; Ct < WCS_GALLERY_NUMNAILS; Ct ++)
		RasterList[Ct].Thumb = NULL;
	delete [] RasterList;
	} // if
RasterList = NULL;
GlobalApp->MCP->RemoveWindowFromMenuList(this);
if (GoneModal)
	EndModal();

} // GalleryGUI::~GalleryGUI()

/*===========================================================================*/

void GalleryGUI::ClearAll(void)
{
long Ct;

if (NameList)
	{
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if (NameList[Ct])
			AppMem_Free(NameList[Ct], strlen(NameList[Ct]) + 1);
		} // for
	AppMem_Free(NameList, NumItems * sizeof (char *));
	NameList = NULL;
	} // if
if (InCategory)
	AppMem_Free(InCategory, NumItems * sizeof (long));
InCategory = NULL;

if (BrowseList)
	delete [] BrowseList;
BrowseList = NULL;

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

if (NumCategoryItems)
	AppMem_Free(NumCategoryItems, NumCategories * sizeof (long));
NumCategoryItems =NULL;

NumItems = NumValidItems = 0;
NumCategories = 0;
NumRanges = 0;
FirstItemInRange = 0;
LastItemInRange = 0;
ActiveItem = 0;
ActiveCategory = 0;
memset(&DisplayedItem[0], 0, WCS_GALLERY_NUMNAILS * sizeof (long));

} // GalleryGUI::ClearAll

/*===========================================================================*/

long GalleryGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_LVE, 0);

return(0);

} // GalleryGUI::HandleCloseWin

/*===========================================================================*/

long GalleryGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
NotifyTag Changes[2];

int ThumbNailNum = ButtonID - IDC_TNAIL1; // We've rearranged the IDs to be consecutive to simplify code

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_LVE, 0);
		break;
		} // 
/*
	case IDC_TNAIL1:
		{
		if(GlobalApp->MainProj->Prefs.QueryConfigOpt("component_index"))
			{
			WriteHTMLComponentIndex(NULL);
			} // if
		else
			{
			SetActiveItem(0);
			if (RasterList[0].Thumb)
				{
				Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0, 0);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, RasterList[0].Thumb);
				} // if
			} // else
		break;
		} // 
*/
	case IDC_TNAIL1:
	case IDC_TNAIL2:
	case IDC_TNAIL3:
	case IDC_TNAIL4:
	case IDC_TNAIL5:
	case IDC_TNAIL6:
	case IDC_TNAIL7:
	case IDC_TNAIL8:
	case IDC_TNAIL9:
	case IDC_TNAIL10:
	case IDC_TNAIL11:
	case IDC_TNAIL12:
	case IDC_TNAIL13:
	case IDC_TNAIL14:
		{
		SetActiveItem(ThumbNailNum);
		if (RasterList[ThumbNailNum].Thumb)
			{
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL, 0, 0);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, RasterList[ThumbNailNum].Thumb);
			} // if
		break;
		} // 
	default: break;
	} // ButtonID
return(0);

} // GalleryGUI::HandleButtonClick

/*===========================================================================*/

long GalleryGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

int ThumbNailNum = ButtonID - IDC_TNAIL1; // We've rearranged the IDs to be consecutive to simplify code

switch (ButtonID)
	{
	case IDC_TNAIL1:
	case IDC_TNAIL2:
	case IDC_TNAIL3:
	case IDC_TNAIL4:
	case IDC_TNAIL5:
	case IDC_TNAIL6:
	case IDC_TNAIL7:
	case IDC_TNAIL8:
	case IDC_TNAIL9:
	case IDC_TNAIL10:
	case IDC_TNAIL11:
	case IDC_TNAIL12:
	case IDC_TNAIL13:
	case IDC_TNAIL14:
		{
		CaptureActiveItem(ThumbNailNum);
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // GalleryGUI::HandleButtonDoubleClick

/*===========================================================================*/

long GalleryGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		NewCategory(NewPageID);
		break;
		} // object types
	default:
		break;
	} // switch

return(0);

} // GalleryGUI::HandlePageChange

/*===========================================================================*/

long GalleryGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_ACTIVEDIR:
		{
		ConfigureWidgets();
		break;
		} // IDC_ACTIVEDIR
	default:
		break;
	} // ID

return(0);

} // GalleryGUI::HandleDDChange

/*===========================================================================*/

void GalleryGUI::HandleNotifyEvent(void)
{

if (! NativeWin)
	return;

} // GalleryGUI::HandleNotifyEvent()

/*===========================================================================*/

long GalleryGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
long rVal;

if (ScrollCode)
	{
	ConfigureImages(ScrollPos);
	rVal = 0;
	} // if
else
	rVal = 1;

return (rVal);

} // GalleryGUI::HandleScroll

/*===========================================================================*/

void GalleryGUI::ConfigureWidgets(void)
{
WIN32_FIND_DATA FileData;
HANDLE Hand;
char NameStr[512], TempStr[256];

ClearAll();

if (ActiveDir[0] && DefaultExt[0])
	{
	strmfp(NameStr, GlobalApp->MainProj->MungPath(ActiveDir), "*.");
	strcat(NameStr, DefaultExt);

	NumItems = NumValidItems = 0;

	if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
		{
		NumItems ++;
		while (FindNextFile(Hand, &FileData))
			{
			NumItems ++;
			} // while
		FindClose(Hand);
		} // if

	if (NumItems)
		{
		if (NameList = (char **)AppMem_Alloc(NumItems * sizeof (char *), APPMEM_CLEAR))
			{
			if (BrowseList = new BrowseData[NumItems])
				{
				if (InCategory = (long *)AppMem_Alloc(NumItems * sizeof (long), APPMEM_CLEAR))
					{
					if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
						{
						strcpy(TempStr, FileData.cFileName);
						StripExtension(TempStr);
						if (NameList[NumValidItems] = (char *)AppMem_Alloc(strlen(TempStr) + 1, APPMEM_CLEAR))
							{
							strcpy(NameList[NumValidItems], TempStr);
							// read browse data
							strmfp(NameStr, ActiveDir, FileData.cFileName);
							if (BrowseList[NumValidItems].LoadBrowseInfo(NameStr))
								{
								//StripExtension(NameList[NumValidItems]);
								NumValidItems ++;
								} // if
							else
								{
								AppMem_Free(NameList[NumValidItems], strlen(TempStr) + 1);
								NameList[NumValidItems] = NULL;
								} // else
							while (NumValidItems < NumItems && FindNextFile(Hand, &FileData))
								{
								strcpy(TempStr, FileData.cFileName);
								StripExtension(TempStr);
								if (NameList[NumValidItems] = (char *)AppMem_Alloc(strlen(TempStr) + 1, APPMEM_CLEAR))
									{
									strcpy(NameList[NumValidItems], TempStr);
									strmfp(NameStr, ActiveDir, FileData.cFileName);
									if (BrowseList[NumValidItems].LoadBrowseInfo(NameStr))
										{
										//StripExtension(NameList[NumValidItems]);
										NumValidItems ++;
										} // if
									else
										{
										AppMem_Free(NameList[NumValidItems], strlen(TempStr) + 1);
										NameList[NumValidItems] = NULL;
										} // else
									} // if
								else
									{
									break;
									} // else
								} // while
							FindClose(Hand);
							// count categories
							if (NumCategories = CountCategories(BrowseList, NumValidItems))
								{
								if ((CategoryNames = (char **)AppMem_Alloc(NumCategories * sizeof (char *), APPMEM_CLEAR))
									&& (NumCategoryItems = (long *)AppMem_Alloc(NumCategories * sizeof (long), APPMEM_CLEAR)))
									{
									FillCategories(BrowseList, NumValidItems);
									} // if
								else
									ClearAll();
								} // if
							} // if
						} // if
					} // if
				else
					{
					ClearAll();
					} // else
				} // if
			else
				{
				ClearAll();
				} // else
			} // if
		else
			NumItems = NumValidItems = 0;
		} // if

	} // if

// safe even if no categories
BuildCategoryList();

if (NumCategoryItems)
	NumRanges = (NumCategoryItems[ActiveCategory]) / WCS_GALLERY_NUMNAILS + ((NumCategoryItems[ActiveCategory]) % WCS_GALLERY_NUMNAILS ? 1: 0);

WidgetSetScrollRange(IDC_SCROLLBAR5, 0, NumRanges - 1);
WidgetSetScrollPos(IDC_SCROLLBAR5, 0);
ActiveRange = ActiveItem = 0;
ConfigureImages(0);

SetActiveItem(0);

ConfigureDD(NativeWin, IDC_ACTIVEDIR, ActiveDir, 255, NULL, 0, IDC_LABEL_DEFDIR);

// <<<>>> Hack: Force a repaint of entire window to make up for problem where deleting all tabs in a
// Tab widget causes our STATIC text to disappear.
// Remove this if we can determine and fix the underlying problem.
CueRedraw();

} // GalleryGUI::ConfigureWidgets()

/*===========================================================================*/

long GalleryGUI::CountCategories(BrowseData *LocalBrowseList, long LocalNumValidItems)
{
long Ct, Test, Found;

NumCategories = 1;

for (Ct = 0; Ct < LocalNumValidItems; Ct ++)
	{
	if (LocalBrowseList[Ct].Category && stricmp(LocalBrowseList[Ct].Category, "General"))
		{
		Found = 0;
		for (Test = 0; Test < Ct; Test ++)
			{
			if (LocalBrowseList[Test].Category && LocalBrowseList[Ct].Category && 
				! stricmp(LocalBrowseList[Test].Category, LocalBrowseList[Ct].Category))
				{
				Found = 1;
				break;
				} // if found match
			} // for
		if (! Found)
			NumCategories ++;
		} // if
	} // for

return (NumCategories);

} // GalleryGUI::CountCategories

/*===========================================================================*/

void GalleryGUI::FillCategories(BrowseData *LocalBrowseList, long LocalNumValidItems)
{
long Ct, Test, Found, Filled = 0;

ActiveCategory = -1;
FirstItemInRange = 0;
LastItemInRange = 0;

if (CategoryNames[Filled] = (char *)AppMem_Alloc(strlen("General") + 1, 0))
	{
	strcpy(CategoryNames[Filled], "General");
	Filled ++;
	} // if
for (Ct = 0; Ct < LocalNumValidItems; Ct ++)
	{
	if (LocalBrowseList[Ct].Category)
		{
		if (! stricmp(LocalBrowseList[Ct].Category, "General"))
			{
			InCategory[Ct] = 0;
			if (ActiveCategory < 0)
				ActiveCategory = 0;
			NumCategoryItems[0] ++;
			} // if general category
		else
			{
			Found = 0;
			for (Test = 0; Test < Ct; Test ++)
				{
				if (LocalBrowseList[Test].Category && LocalBrowseList[Ct].Category && 
					! stricmp(LocalBrowseList[Test].Category, LocalBrowseList[Ct].Category))
					{
					Found = 1;
					InCategory[Ct] = InCategory[Test];
					NumCategoryItems[InCategory[Test]] ++;
					break;
					} // if found match
				} // for
			if (! Found)
				{
				if (CategoryNames[Filled] = (char *)AppMem_Alloc(strlen(LocalBrowseList[Ct].Category) + 1, 0))
					{
					strcpy(CategoryNames[Filled], LocalBrowseList[Ct].Category);
					InCategory[Ct] = Filled;
					//if (ActiveCategory < 0)
					//	ActiveCategory = Filled;
					NumCategoryItems[Filled] ++;
					Filled ++;
					} // if
				} // if
			} // else not general category
		} // if
	} // for

if (ActiveCategory < 0)
	ActiveCategory = 0;

} // GalleryGUI::FillCategories

/*===========================================================================*/

void GalleryGUI::BuildCategoryList(void)
{
long Ct;

WidgetTCClear(IDC_TAB1); // <<<>>> this line seems to be causing the STATIC text beneath the Toolbutton widgets to erase

for (Ct = 0; Ct < NumCategories; Ct ++)
	{
	if (CategoryNames[Ct])
		WidgetTCInsertItem(IDC_TAB1, Ct, CategoryNames[Ct]);
	} // for
if (NumCategories)
	WidgetTCSetCurSel(IDC_TAB1, ActiveCategory);

} // GalleryGUI::BuildCategoryList

/*===========================================================================*/

void GalleryGUI::ConfigureImages(long ScrollPos)
{
char *Title, ComposedDescription[4096], *ChosenDescription;
long Ct, BaseCt, Found = 0;
Raster *MyRast;

BaseCt = ScrollPos * WCS_GALLERY_NUMNAILS;
FirstItemInRange = 0;

for (Ct = 0; Ct < NumValidItems && Found < BaseCt; Ct ++)
	{
	if (InCategory[Ct] == ActiveCategory)
		Found ++;
	} // for
if (Ct < NumValidItems)
	FirstItemInRange = Ct;
Found = 0;
for (Ct = FirstItemInRange; Found < WCS_GALLERY_NUMNAILS; Ct ++)
	{
	if (Ct >= NumValidItems || InCategory[Ct] == ActiveCategory)
		{
		int TitleNum = Found + IDC_TITLE1;
		int ThumbNailNum = Found + IDC_TNAIL1; // We've rearranged the IDs to be consecutive to simplify code

		if (Ct < NumValidItems && NameList[Ct])
			{
			Title = NameList[Ct];
			RasterList[Found].Thumb = BrowseList[Ct].Thumb;
			MyRast = BrowseList[Ct].Thumb ? &RasterList[Found]: NULL;
			DisplayedItem[Found] = Ct;
			if(BrowseList[Ct].Author2 && strlen(BrowseList[Ct].Author2))
				{
				sprintf(ComposedDescription, "%s\r\nModified by %s (%s) on %s\r\nOriginally created by %s\r\n%s%s\r\n(Double click to load)", Title,
				 BrowseList[Ct].Author2 ? BrowseList[Ct].Author2: "",
				 BrowseList[Ct].Address ? BrowseList[Ct].Address: "",
				 BrowseList[Ct].Date ? BrowseList[Ct].Date: "",
				 BrowseList[Ct].Author ? BrowseList[Ct].Author: "",
				 BrowseList[Ct].Comment ? "\r\n" : "", // add extra line break if comment present
				 BrowseList[Ct].Comment ? BrowseList[Ct].Comment: "");
				} // if
			else
				{
				sprintf(ComposedDescription, "%s\r\nCreated by %s (%s) on %s\r\n%s%s\r\n(Double click to load)", Title,
				 BrowseList[Ct].Author ? BrowseList[Ct].Author: "",
				 BrowseList[Ct].Address ? BrowseList[Ct].Address: "",
				 BrowseList[Ct].Date ? BrowseList[Ct].Date: "",
				 BrowseList[Ct].Comment ? "\r\n" : "", // add extra line break if comment present
				 BrowseList[Ct].Comment ? BrowseList[Ct].Comment: "");
				} // else
			ChosenDescription = ComposedDescription;
			
			WidgetSetText(TitleNum, Title);
			ConfigureTB(NativeWin, ThumbNailNum, NULL, NULL, MyRast);
			WidgetSetText(ThumbNailNum, ChosenDescription);
			WidgetShow(TitleNum, true);
			WidgetShow(ThumbNailNum, true);
			} // if
		else
			{
			DisplayedItem[Found] = NumValidItems;
			ConfigureTB(NativeWin, ThumbNailNum, NULL, NULL, NULL);
			WidgetShow(TitleNum, false);
			WidgetShow(ThumbNailNum, false);
			} // else
			
		Found ++;
		} // if
	} // for

ActiveRange = ScrollPos;

} // GalleryGUI::ConfigureImages

/*===========================================================================*/

void GalleryGUI::SetActiveItem(long NewActive)
{

if ((NewActive = DisplayedItem[NewActive]) < NumValidItems && NameList[NewActive])
	{
	// capture any data the user may have entered
	ActiveItem = NewActive;
	} // if
else
	{
	ActiveItem = 0;
	} // else

} // GalleryGUI::SetActiveItem

/*===========================================================================*/

void GalleryGUI::CaptureActiveItem(long NewActive)
{
long ItemStash = NewActive, Result;
//NotifyTag Changes[2];
RasterAnimHostProperties Prop;
char filename[256];

if ((NewActive = DisplayedItem[NewActive]) < NumValidItems && NameList[NewActive])
	{
	if (! Active)
		{
		if (ActiveObjType >= WCS_EFFECTSSUBCLASS_LAKE && ActiveObjType < WCS_MAXIMPLEMENTED_EFFECTS)
			{
			Active = EffectsHost->AddEffect(ActiveObjType, NULL, NULL);
			} // if
		else if (ActiveObjType == WCS_RAHOST_OBJTYPE_PROJECT)
			{
			Active = GlobalApp->MainProj;
			} // if
		} // if
	if (Active)
		{
		ActiveItem = NewActive;

		strmfp(filename, ActiveDir, NameList[ActiveItem]);
		if (ActiveObjType == WCS_RAHOST_OBJTYPE_PROJECT)
			{
			if (strlen(filename) >= 5 && ! stricmp(&filename[strlen(filename) - 5], ".proj"))
				StripExtension(filename);
			} // if
		strcat(filename, ".");
		strcat(filename, DefaultExt);
		Prop.Path = filename;
		Prop.PropMask = WCS_RAHOST_MASKBIT_LOADFILE;
		if ((Result = Active->LoadFilePrep(&Prop)) > 0)
			{
			// this notification was not early enough to prevent crashing when widgets were repainted. It now takes place before the
			// requester telling of component loading success in LoadFilePrep().
			//Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
			//Changes[1] = NULL;
			//GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_LVE, 0);
			} // if
		} // if
	} // if
else
	{
	SetActiveItem(ItemStash);
	} // else

} // GalleryGUI::CaptureActiveItem

/*===========================================================================*/

// obsolete
/*
void GalleryGUI::NewObjectType(long NewPage)
{

if (NewPage < NumObjTypes)
	{
	if(ActiveObjType != ObjectType[NewPage])
		{
		ActiveObjType = ObjectType[NewPage];
		if (ActiveObjType >= WCS_EFFECTSSUBCLASS_LAKE && ActiveObjType < WCS_MAXIMPLEMENTED_EFFECTS)
			{
			strcpy(DefaultExt, EffectsLib::DefaultExtensions[ActiveObjType]);
			strmfp(ActiveDir, ContentDir, EffectsLib::DefaultPaths[ActiveObjType]);
			ConfigureWidgets();
			} // if
		else if (ActiveObjType == WCS_RAHOST_OBJTYPE_PROJECT)
			{
			strcpy(DefaultExt, "proj");
			strcpy(ActiveDir, GlobalApp->MainProj->projectpath[0] ? GlobalApp->MainProj->projectpath: "WCSProjects");
			ConfigureWidgets();
			} // if
		} // if
	} // if

} // GalleryGUI::NewObjectType
*/

/*===========================================================================*/

void GalleryGUI::NewCategory(long NewPage)
{

if (NewPage < NumCategories && NewPage != ActiveCategory)
	{
	ActiveCategory = NewPage;
	NumRanges = (NumCategoryItems[ActiveCategory]) / WCS_GALLERY_NUMNAILS + ((NumCategoryItems[ActiveCategory]) % WCS_GALLERY_NUMNAILS ? 1: 0);

	WidgetSetScrollRange(IDC_SCROLLBAR5, 0, NumRanges - 1);
	WidgetSetScrollPos(IDC_SCROLLBAR5, 0);
	ActiveRange = ActiveItem = 0;
	ConfigureImages(0);

	SetActiveItem(0);
	} // if

} // GalleryGUI::NewCategory

/*===========================================================================*/
