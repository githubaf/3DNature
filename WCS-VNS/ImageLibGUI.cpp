// ImageLibGUI.cpp
// Code for Effects Library Editor
// Built from scratch on 5/1/97 by Gary Huber
// Copyright 1997 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ImageLibGUI.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "EffectsLib.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "Raster.h"
#include "ImageInputFormat.h"
#include "ViewGUI.h"
#include "ProjectDispatch.h"
#include "AppMem.h"
#include "resource.h"

char *TabNames[] = {"Image", "Browse", "Sequence && Dissolve", "Color", "Geo Reference", "Image Mgr"};

NativeGUIWin ImageLibGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ImageLibGUI::Open

/*===========================================================================*/

NativeGUIWin ImageLibGUI::Construct(void)
{
char *BandLabels[] = {"Band 1", "Band 2", "Band 3"};//, "Band 4", "Band 5", "Band 6", "Band 7"};
int ItemCt;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMAGE_LIB, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_IMAGE, 0, 0);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_BROWSE, 0, 1);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_SEQDIS, 0, 2);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_COLORCONTROL, 0, 3);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_GEOREF_VNS, 0, 4);
	// this string is so long it bombs the Mac resource compiler, so we leave it blank until here and set it on the fly.
	WidgetSetText(IDC_STATIC_REGCOORD, "Registration Coordinates below are referenced to the Coordinate System selected above or in degrees if there is no selection. You can set the units and the way the values are displayed in the Preferences Editor (CTRL+=).");
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_GEOREF, 0, 4);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_IMAGE_MANAGE, 0, 5);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_DISSOLVETREE, 1, 0);
	CreateSubWinFromTemplate(IDD_IMAGE_LIB_SEQUENCETREE, 1, 1);

	SeqDisPanel =		SubPanels[0][0];

	#ifndef WCS_IMAGELIB_TILEABLEIMAGE
	WidgetShow(IDC_CREATETILEABLE, 0);
	#endif // WCS_IMAGELIB_TILEABLEIMAGE

	if(NativeWin && SeqDisPanel)
		{
		WidgetCBInsert(IDC_STARTBEHAVIOR, -1, "No Image");
		WidgetCBInsert(IDC_STARTBEHAVIOR, -1, "Hold Image");
		WidgetCBInsert(IDC_STARTBEHAVIOR, -1, "Loop");
		WidgetCBInsert(IDC_ENDBEHAVIOR, -1, "No Image");
		WidgetCBInsert(IDC_ENDBEHAVIOR, -1, "Hold Image");
		WidgetCBInsert(IDC_ENDBEHAVIOR, -1, "Loop");

		for (ItemCt = 0; ItemCt < 3/*7*/; ItemCt ++)
			{
			WidgetCBInsert(IDC_BANDRED, -1, BandLabels[ItemCt]);
			WidgetCBInsert(IDC_BANDGREEN, -1, BandLabels[ItemCt]);
			WidgetCBInsert(IDC_BANDBLUE, -1, BandLabels[ItemCt]);
			WidgetCBInsert(IDC_BANDIN, -1, BandLabels[ItemCt]);
			} // for

		WidgetCBInsert(IDC_BANDOUT, -1, "Red");
		WidgetCBInsert(IDC_BANDOUT, -1, "Green");
		WidgetCBInsert(IDC_BANDOUT, -1, "Blue");

		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 255);
		WidgetSetScrollRange(IDC_SCROLLBAR2, 0, 255);
		WidgetSetScrollRange(IDC_SCROLLBAR3, 0, 255);

		BuildList(HostLib, IDC_PARLIST);
		#ifndef WCS_IMAGE_MANAGEMENT
		WidgetShow(IDC_MANAGERTXT, 0);
		WidgetShow(IDC_ADDIMAGEMAN, 0);
		WidgetShow(IDC_REMOVEIMAGEMAN, 0);
		WidgetShow(IDC_CHECKTILING, 0);
		#endif // WCS_IMAGE_MANAGEMENT

		ConfigureWidgets();

		} // if

	} // if
 
return(NativeWin);
} // ImageLibGUI::Construct

/*===========================================================================*/

ImageLibGUI::ImageLibGUI(ImageLib *ImageSource, EffectsLib *EffectsSource, Project *ProjSource)
: GUIFenetre('IMGL', this, "Image Object Library") // Yes, I know...
{
static NotifyTag AllOldImageEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_IMAGES, 0xff, 0xff, 0xff), 
								0};
static NotifyTag AllImageEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_FREEZE, 0xff, 0xff, 0xff), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED), 
									MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED), 
									MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
									#ifdef WCS_BUILD_VNS
									MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
									#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
HostEffectsLib = EffectsSource;
HostLib = ImageSource;
HostProj = ProjSource;
BrowseEffectsLib = new EffectsLib;
BrowseLib = new ImageLib;
ListItems = 0;
SeqDisPanel = NULL;
CurPanel = -1;
CurSeqDisPanel = 0;
ActiveSequence = NULL;
ActiveDissolve = NULL;
ActiveShell = NULL;
FormulaOutBand = FormulaInBand = 0;
TVNumItems = 0;
SequenceAdjust = 0;
ImageRast = SequenceRast = DissolveRast = DissolveRast2 = ColorControlRast = BrowseRast = TileRast = NULL;
//Pane = NULL;
Frozen = 0;
StashPath[0] = StashName[0] = 0;
ReceivingDiagnostics = ReceivingDiagnosticsShift = 0;
LatEvent[0] = LonEvent[0] = LatEvent[1] = LonEvent[1] = 0.0;

if (ImageSource && EffectsSource && ProjSource)
	{
	ConstructError = 0;
	Active = HostLib->GetActive();
	HostLib->RegisterClient(this, AllOldImageEvents);
	GlobalApp->AppEx->RegisterClient(this, AllImageEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

SECURITY_INLINE_CHECK(047, 47);

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // ImageLibGUI::ImageLibGUI

/*===========================================================================*/

ImageLibGUI::~ImageLibGUI()
{

HostLib->RemoveClient(this);
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
delete BrowseLib;
delete BrowseEffectsLib;
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ImageLibGUI::~ImageLibGUI()

/*===========================================================================*/

long ImageLibGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{
SECURITY_INLINE_CHECK(062, 62);
switch (CtrlID)
	{
	case IDC_TAB1:
		{
		SelectPanel(-1, 0);
		break;
		}
	default:
		break;
	} // switch

return(0);

} // ImageLibGUI::HandlePageChange

/*===========================================================================*/

long ImageLibGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
RasterAttribute *MyAttr;
ColorControlShell *MyShell;
long rVal;

if(ScrollCode)
	{
	if (Active)
		{
		if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
			{
			if (MyShell = (ColorControlShell *)MyAttr->GetShell())
				{
				switch (CtrlID)
					{
					case IDC_SCROLLBAR1:
						{
						MyShell->RGB[MyShell->DisplayRGBSet][0] = (short)ScrollPos;
						WidgetFISync(IDC_COLRED, WP_FISYNC_NONOTIFY);
						break;
						}
					case IDC_SCROLLBAR2:
						{
						MyShell->RGB[MyShell->DisplayRGBSet][1] = (short)ScrollPos;
						WidgetFISync(IDC_COLGRN, WP_FISYNC_NONOTIFY);
						break;
						}
					case IDC_SCROLLBAR3:
						{
						MyShell->RGB[MyShell->DisplayRGBSet][2] = (short)ScrollPos;
						WidgetFISync(IDC_COLBLU, WP_FISYNC_NONOTIFY);
						break;
						}
					default:
						break;
					} // switch
				RevealThumbnail();
				ConfigureSequenceColors(MyShell);
				} // if
			} // if
		} // if
	rVal = 0;
	} // if
else
	rVal = 5;

return (rVal);

} // ImageLibGUI::HandleScroll

/*===========================================================================*/

long ImageLibGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_PARLIST:
		{
		RemoveImage(FALSE, FALSE);
		break;
		} // IDC_PARLIST
	case IDC_SEQDISLIST:
		{
		RemovePartialSequenceDissolve();
		break;
		} // IDC_SEQDISLIST
	default:
		break;
	} // switch

return(0);

} // ImageLibGUI::HandleListDelItem

/*===========================================================================*/

long ImageLibGUI::HandleCloseWin(NativeGUIWin NW)
{

//if (Activity->Origin && Activity->Origin->FenID == 'FOLP')
//	{
//	ClosePreview();
//	} // if
//else
//	{
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
//	} // else

return(1);

} // ImageLibGUI::HandleCloseWin

/*===========================================================================*/

long ImageLibGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAILIMAGE:
		{
		if (ImageRast)
			ImageRast->OpenPreview(FALSE);
		break;
		} // 
	case IDC_TNAILSEQUENCE:
		{
		if (SequenceRast)
			SequenceRast->OpenPreview(FALSE);
		break;
		} // 
	case IDC_TNAILDISSOLVE1:
		{
		if (DissolveRast)
			DissolveRast->OpenPreview(FALSE);
		break;
		} // 
	case IDC_TNAILDISSOLVE2:
		{
		if (DissolveRast2)
			DissolveRast2->OpenPreview(FALSE);
		break;
		} // 
	case IDC_TNAILCOLORCONTROL:
		{
		if (ColorControlRast)
			ColorControlRast->OpenPreview(FALSE);
		break;
		} // 
	case IDC_TNAILBROWSE:
		{
		if (BrowseRast)
			BrowseRast->OpenPreview(FALSE);
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // ImageLibGUI::HandleButtonDoubleClick

/*===========================================================================*/

long ImageLibGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
RasterAttribute *MyAttr;

SECURITY_INLINE_CHECK(049, 49);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_ILG, 0);
		break;
		} // 
	case IDC_ADDIMAGE:
		{
		SECURITY_INLINE_CHECK(034, 34);
		#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
		char Qualifier;
		Qualifier = LocalWinSys()->CheckQualifier(WCS_GUI_KEYCODE_SHIFT) ? 1 : 0;
		AddImage(Qualifier);
		#else // WCS_BUILD_REMOTEFILE_SUPPORT
		AddImage();
		#endif // WCS_BUILD_REMOTEFILE_SUPPORT
		DisableButtons();
		break;
		} // 
	case IDC_REMOVEIMAGE:
		{
		RemoveImage(FALSE, FALSE);
		DisableButtons();
		break;
		} // 
	case IDC_REMOVEALLIMAGES:
		{
		RemoveImage(TRUE, FALSE);
		DisableButtons();
		break;
		} // 
	case IDC_UPDATE:
		{
		UpdateImage(TRUE);
#ifdef WCS_IMAGE_MANAGEMENT
		if(Active->QueryImageManagementEnabled())
			{
			if (UserMessageOKCAN("Update Thumbnail", "To update Image Managed thumbnail, entire image must be previewed."))
				{
				Active->OpenPreview(FALSE); // will preview image and update thumbnail when complete
				break;
				} // if
			else
				{
				break;
				} // else
			} // if
#endif // WCS_IMAGE_MANAGEMENT
		break;
		} // 
	case IDC_VIEWIMAGE:
		{
		if (Active)
			Active->OpenPreview(TRUE);
		break;
		} // 
	case IDC_ADDDISSOLVE:
		{
		SECURITY_INLINE_CHECK(011, 11);
		AddDissolve();
		DisableButtons();
		break;
		} // 
	case IDC_ADDCOLORCONTROL:
		{
		SECURITY_INLINE_CHECK(012, 12);
		AddColorControl();
		DisableButtons();
		break;
		} // 
	case IDC_ADDSEQUENCE:
		{
		SECURITY_INLINE_CHECK(010, 10);
		AddSequence();
		DisableButtons();
		break;
		} // 
	case IDC_ADDGEOREF:
		{
		SECURITY_INLINE_CHECK(023, 23);
		AddGeoRef();
		DisableButtons();
		break;
		} // 
	case IDC_ADDIMAGEMAN:
		{
		#ifdef WCS_IMAGE_MANAGEMENT
		SECURITY_INLINE_CHECK(024, 24);
		AddImageManager();
		DisableButtons();
		#endif // WCS_IMAGE_MANAGEMENT
		break;
		} // 
	case IDC_REMOVEDISSOLVE:
		{
		RemoveAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE);
		DisableButtons();
		break;
		} // 
	case IDC_REMOVECOLORCONTROL:
		{
		RemoveAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL);
		DisableButtons();
		break;
		} // 
	case IDC_REMOVESEQUENCE:
		{
		RemoveAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE);
		DisableButtons();
		break;
		} // 
	case IDC_REMOVEGEOREF:
		{
		RemoveAttribute(WCS_RASTERSHELL_TYPE_GEOREF);
		DisableButtons();
		break;
		} // 
	case IDC_REMOVEIMAGEMAN:
		{
		#ifdef WCS_IMAGE_MANAGEMENT
		RemoveAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER);
		DisableButtons();
		#endif // WCS_IMAGE_MANAGEMENT
		break;
		} // 
	case IDC_OPENEFFECTS:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_EFG, 0);
		break;
		} // 
	case IDC_APPLYTOALL:
		{
		ApplyMemoryToAll();
		break;
		} // 
	case IDC_APPLYTOALL2:
		{
		ApplyLoadFastToAll();
		break;
		} // 
	case IDC_DISSOLVE_ADD:
		{
		AddSequenceDissolve();
		break;
		} // 
	case IDC_DISSOLVE_DELETE:
		{
		RemovePartialSequenceDissolve();
		break;
		} // 
	case ID_COLORPOT1:
		{
		DoTransparencySelect(0);
		break;
		} // 
	case ID_COLORPOT2:
		{
		DoTransparencySelect(1);
		break;
		} // 
	case ID_COLORPOT3:
		{
		DoTransparencySelect(2);
		break;
		} // 
	case IDC_ADDFILE:
		{
		BrowseFile();
		break;
		} // 
	case IDC_SELECTALL:
		{
		SelectAll();
		break;
		} // 
	case IDC_COPYTOPROJ:
		{
		CopyToProj();
		break;
		} // 
	case IDC_SHIFTBOUNDS:
		{
		ShiftBounds(NULL);
		break;
		} // 
	case IDC_SETBOUNDS:
		{
		SetBounds(NULL);
		break;
		} // 
	case IDC_SNAPBOUNDS:
		{
		if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			((GeoRefShell *)MyAttr->GetShell())->GeoReg.SnapToDEMBounds((CoordSys *)((GeoRefShell *)MyAttr->GetShell())->Host);
			} // if
		break;
		} // 
	case IDC_EDITCOORDS:
		{
		if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
			{
			if (MyAttr->GetShell()->GetHost())
				MyAttr->GetShell()->GetHost()->EditRAHost();
			} // if
		break;
		} // IDC_EDITCOORDS
	case IDC_CREATETILEABLE:
		{
		#ifdef WCS_IMAGELIB_TILEABLEIMAGE
		CreateTileableImage();
		#endif // WCS_IMAGELIB_TILEABLEIMAGE
		break;
		} // IDC_CREATETILEABLE
	case IDC_REMOVEUNUSEDIMAGES:
		{
		RemoveImage(TRUE, TRUE);
		DisableButtons();
		break;
		} // IDC_REMOVEUNUSEDIMAGES
	case IDC_SORTIMAGES:
		{
		HostLib->SortImagesByAlphabet();
		break;
		} // IDC_SORTIMAGES
	case IDC_MOVETEMPUP:
		{
		AdjustSequenceDissolvePosition(1);
		break;
		} // IDC_MOVETEMPUP
	case IDC_MOVETEMPDOWN:
		{
		AdjustSequenceDissolvePosition(-1);
		break;
		} // IDC_MOVETEMPUP
	default:
		break;
	} // ButtonID

return(0);

} // ImageLibGUI::HandleButtonClick

/*===========================================================================*/

long ImageLibGUI::HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // ImageLibGUI::HandleStringEdit

/*===========================================================================*/

long ImageLibGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_STARTBEHAVIOR:
		{
		StartBehavior();
		break;
		}
	case IDC_ENDBEHAVIOR:
		{
		EndBehavior();
		break;
		}
	case IDC_TARGETDROP:
		{
		SelectDissolveTarget();
		break;
		}
	case IDC_BANDRED:
	case IDC_BANDGREEN:
	case IDC_BANDBLUE:
		{
		DoBandAssignment((short)CtrlID);
		break;
		}
	case IDC_BANDOUT:
		{
		DoFormulaOutBand();
		break;
		}
	case IDC_BANDIN:
		{
		DoFormulaInBand();
		break;
		}
	case IDC_COORDSDROP:
		{
		#ifdef WCS_BUILD_VNS
		SelectNewCoords();
		#endif // WCS_BUILD_VNS
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // ImageLibGUI::HandleCBChange

/*===========================================================================*/

long ImageLibGUI::HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long int TreeItem, void *TreeItemData)
{

// TreeItem is itemNew.hItem
// TreeItemData is itemNew.lParam

switch (CtrlID)
	{
	case IDC_SEQDISLIST:
		{
		SelectSequenceOrDissolve((RasterShell *)TreeItemData);
		break;
		} // Sequence List
	} // switch

return(0);

} // ImageLibGUI::HandleTreeChange

/*===========================================================================*/

long ImageLibGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(087, 87);
switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		SelectImage();
		break;
		} // if list
	case IDC_BROWSELIST:
		{
		SelectBrowseImage();
		break;
		} // if list
	default:
		break;
	} // switch CtrlID

return (0);

} // ImageLibGUI::HandleListSel

/*===========================================================================*/

long ImageLibGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PARLIST:
		{
		if (Active)
			Active->OpenPreview(FALSE);
		break;
		}
	case IDC_EFFECTLIST:
		{
		EditApplication();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // ImageLibGUI::HandleListDoubleClick

/*===========================================================================*/

long ImageLibGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
short Rebuild = 0;
RasterAttribute *MyAttr;

switch (CtrlID)
	{
	case IDC_BANDFACTOR:
		{
		ResyncColorFormula(NULL);
		break;
		} // Start
	case IDC_COLRED:
	case IDC_COLGRN:
	case IDC_COLBLU:
		{
		ResyncTransparency(NULL);
		RevealThumbnail();
		break;
		} // Start
	case IDC_ENDTIME:
	case IDC_ENDSPACE:
	case IDC_FIRST_IMAGE:
	case IDC_LAST_IMAGE:
	case IDC_SPEED:
	case IDC_NUM_LOOPS:
		{
		NewSequenceEvent();
		Rebuild = 1;
		break;
		} // 
	case IDC_STARTSPACE:
		{
		NewSequenceStartSpace();
		Rebuild = 1;
		break;
		} // 
	case IDC_STARTTIME:
		{
		NewSequenceStartFrame();
		Rebuild = 1;
		break;
		} // 
	case IDC_STARTFRAME:
		{
		NewDissolveStartFrame();
		Rebuild = 1;
		break;
		} // Start
	case IDC_ENDFRAME:
		{
		NewDissolveEndFrame();
		Rebuild = 1;
		break;
		} // Start
	case IDC_DURATION:
		{
		NewDissolveDuration();
		Rebuild = 1;
		break;
		} // Start
	case IDC_USERTILEX:
	case IDC_USERTILEY:
	case IDC_PIXELRESX:
	case IDC_PIXELRESY:
		{
		if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER)))
			{
			Active->ClearAllTilesFromCache();
			ConfigureImageManager((ImageManagerShell *)MyAttr->GetShell());
			} // if
		break;
		} // Start
	default:
		break;
	} // switch CtrlID

if (Rebuild)
	{
	UpdateTVText();
	} // Start

return(0);

} // ImageLibGUI::HandleFIChange

/*===========================================================================*/

long ImageLibGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
RasterAttribute *MyAttr;

Changes[1] = NULL;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKALPHAENABLED:
		{
		UpdateImage(FALSE);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKCOLORCONTROL:
	case IDC_CHECKUSECOLOR:
	case IDC_CHECKSHOWTRANSPAR:
		{
		RevealThumbnail();
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_CHECKTILING:
	case IDC_CHECKSEQUENCE:
	case IDC_CHECKDISSOLVE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // Start
	case IDC_CHECKVIRTRESENABLED:
		{
		if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER)))
			ConfigureImageManager((ImageManagerShell *)MyAttr->GetShell());
		break;
		} // Start
	default:
		break;
	} // switch CtrlID

return(0);

} // ImageLibGUI::HandleSCChange

/*===========================================================================*/

long ImageLibGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
RasterAttribute *MyAttr;

switch (CtrlID)
	{
	case IDC_RADIOBANDASSIGNMENT:
	case IDC_RADIOFORMULA:
		{
		DisableColorControlWidgets(NULL);
		break;
		} // 
	case IDC_RADIOMAXTRANSPAR:
	case IDC_RADIOMINTRANSPAR:
	case IDC_RADIOREPLACE:
		{
		ResyncTransparency(NULL);
		break;
		} // 
	case IDC_RADIOAUTO:
	case IDC_RADIOMANUAL:
	case IDC_RADIOPIXELRES:
	case IDC_RADIOSCALEFACTOR:
	case IDC_RADIOGROUNDUNITS:
		{
		if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER)))
			ConfigureImageManager((ImageManagerShell *)MyAttr->GetShell());
		break;
		} // Start
	default:
		break;
	} // switch CtrlID

return(0);

} // ImageLibGUI::HandleSRChange

/*===========================================================================*/

long ImageLibGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
short Rebuild = 0;

switch (CtrlID)
	{
	case IDC_IMAGE_FILE:
		{
		UpdateImage(TRUE);
		break;
		} // 
	case IDC_SEQ_FILE:
		{
		Rebuild = 1;
		break;
		} // 
	default:
		break;
	} // switch CtrlID
if (Rebuild)
	{
	UpdateTVText();
	} // Start

return(0);

} // ImageLibGUI::HandleDDChange

/*===========================================================================*/

void ImageLibGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[9];
Raster *CurrentRast, *MatchRast;
RasterAttribute *MyAttr;
long CurPos, Place;

if (! NativeWin)
	return;
SECURITY_INLINE_CHECK(062, 62);

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	Freeze();
	return;
	} // if freeze
Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	UnFreeze();		// does not rebuild all
	BuildList(HostLib, IDC_PARLIST);
	ConfigureWidgets();
	return;
	} // if thaw
if (Frozen)
	return;

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildList(HostLib, IDC_PARLIST);
	ConfigureWidgets();
	} // if images added changed

SECURITY_INLINE_CHECK(054, 54);

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	ResetListNames();
	if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)) && MyAttr->GetShell())
		{
		WidgetCBClear(IDC_TARGETDROP);
		CurPos = -1;
		MatchRast = ((DissolveShell *)MyAttr->GetShell())->GetTarget();
		for (CurrentRast = HostLib->List; CurrentRast; CurrentRast = CurrentRast->Next)
			{
			Place = WidgetCBInsert(IDC_TARGETDROP, -1, CurrentRast->GetUserName());
			WidgetCBSetItemData(IDC_TARGETDROP, Place, CurrentRast);
			if (CurrentRast == MatchRast)
				CurPos = Place;
			} // for
		WidgetCBSetCurSel(IDC_TARGETDROP, CurPos);
		} // if
	} // if name changed

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
Interested[1] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	ResetListEnabled();
	WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
	} // if enabled status changed

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Interested[1] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	} // if image changed

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[1] = NULL;
if (HostLib->MatchNotifyClass(Interested, Changes, 0))
	{
	BuildAttributeList();
	if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)))
		{
		ConfigureGeoRef((GeoRefShell *)MyAttr->GetShell());
		} // if
	} // if attribute changed

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_IMAGES, WCS_IMAGESSUBCLASS_GENERIC, WCS_IMAGESGENERIC_ACTIVECHANGED, 0xff);
if (HostLib->MatchNotifyClass(Interested, Changes, 1))
	{
	Active = HostLib->GetActive();
	if (! Active->GetPreppedStatus())
		{
		GlobalApp->MCP->GoModal();
		Active->LoadnPrepImage(FALSE, TRUE);
		GlobalApp->MCP->EndModal();
		} // if
	ListSelectOneItem(IDC_PARLIST, Active);
//	WidgetLBSetCurSel(IDC_PARLIST, FindInList(Active));
	ConfigureWidgets();
	} // if enabled status changed

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
Interested[1] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		{
		if (((Changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			SetBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	if (ReceivingDiagnosticsShift)
		{
		if (((Changed & 0xff00) >> 8) == WCS_DIAGNOSTIC_ITEM_MOUSEDOWN)
			ShiftBounds((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	} // if

if(Active)
	{
	Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
	Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
	Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
	Interested[3] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
	Interested[4] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
	Interested[5] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
	Interested[6] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
			{
			ConfigureGeoRef((GeoRefShell *)MyAttr->GetShell());
			} // if
		} // if
	Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Interested[1] = NULL;
	if (HostLib->MatchNotifyClass(Interested, Changes, 0))
		{
		ConfigureWidgets();
		} // if value changed
	} // if

} // ImageLibGUI::HandleNotifyEvent()

/*===========================================================================*/

void ImageLibGUI::ConfigureWidgets(void)
{
char TextStr[24], ActiveTabText[24];
long TabExists, NumItems, Current, ResetActiveTab = 0, TabsChanged = 0;
RasterAttribute *MyAttr, *MyAttr1;

ConfigureTB(NativeWin, IDC_ADDIMAGE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEIMAGE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_DISSOLVE_ADD, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_DISSOLVE_DELETE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDCOLORCONTROL, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVECOLORCONTROL, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDSEQUENCE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESEQUENCE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDDISSOLVE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEDISSOLVE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDGEOREF, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEGEOREF, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ADDIMAGEMAN, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEIMAGEMAN, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPDOWN, IDI_ARROWDOWN, NULL);

if ((NumItems = WidgetTCGetItemCount(IDC_TAB1)) > 0)
	{
	Current = WidgetTCGetCurSel(IDC_TAB1);
	Current = WidgetTCGetItemText(IDC_TAB1, Current, ActiveTabText, 24);
	ResetActiveTab = 1;
	} // if

if (CheckTabExists(TabNames[0]) < 0)
	WidgetTCInsertItem(IDC_TAB1, 0, TabNames[0]);

if (CheckTabExists(TabNames[1]) < 0)
	WidgetTCInsertItem(IDC_TAB1, 1, TabNames[1]);

if (Active)
	{
	WidgetSetModified(IDC_NAME, FALSE);
	WidgetSetText(IDC_NAME, Active->Name);
	ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKALPHAENABLED, &Active->AlphaEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOLORCONTROL, &Active->ColorControlEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKSEQUENCE, &Active->SequenceEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKDISSOLVE, &Active->DissolveEnabled, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKTILING, &Active->ImageManagerEnabled, SCFlag_Char, NULL, NULL);
	WidgetSetDisabled(IDC_CHECKALPHAENABLED, ! Active->AlphaAvailable);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVSHORT, &Active->Longevity, SCFlag_Char, WCS_RASTER_LONGEVITY_SHORT, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVMED, &Active->Longevity, SCFlag_Char, WCS_RASTER_LONGEVITY_MEDIUM, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVLONG, &Active->Longevity, SCFlag_Char, WCS_RASTER_LONGEVITY_LONG, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLOADFAST, IDC_RADIOLOADFAST, &Active->LoadFast, SCFlag_Char, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLOADFAST, IDC_RADIOSAVEDISKSPACE, &Active->LoadFast, SCFlag_Char, 0, NULL, NULL);
	sprintf(TextStr, "%1d", Active->Rows);
	WidgetSetText(IDC_IMAGEHEIGHT, TextStr);
	sprintf(TextStr, "%1d", Active->Cols);
	WidgetSetText(IDC_IMAGEWIDTH, TextStr);
	sprintf(TextStr, "%1d", Active->ByteBands);
	WidgetSetText(IDC_NUMCOLORS, TextStr);
	ConfigureDD(NativeWin, IDC_IMAGE_FILE, Active->PAF.Path, 255, Active->PAF.Name, 31, IDC_LABEL_CMAP);
	WidgetSetDisabled(IDC_IMAGE_FILE, FALSE);
	BuildAttributeList();

	// sequences and dissolves
	TabExists = CheckTabExists(TabNames[2]);
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE);
	if ((MyAttr1 = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE)) || MyAttr)
		{
		if (TabExists < 0)
			{
			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[2]);
			TabsChanged = 1;
			} // if need to add tab
		if (MyAttr)
			{
			ConfigureDissolve((DissolveShell *)MyAttr->GetShell());
			ActiveShell = MyAttr->GetShell();
			WidgetSetDisabled(IDC_CHECKDISSOLVE, FALSE);
			} // if
		else
			WidgetSetDisabled(IDC_CHECKDISSOLVE, TRUE);
		if (MyAttr1)
			{
			ConfigureSequence((SequenceShell *)MyAttr1->GetShell());
			ActiveShell = MyAttr1->GetShell();
			WidgetSetDisabled(IDC_CHECKSEQUENCE, FALSE);
			} // if
		else
			WidgetSetDisabled(IDC_CHECKSEQUENCE, TRUE);
		if (Active->SequenceEnabled || Active->DissolveEnabled)
			{
			Active->ImageManagerEnabled = 0;
			WidgetSetDisabled(IDC_CHECKTILING, TRUE);
			WidgetSCSync(IDC_CHECKTILING, WP_SCSYNC_NONOTIFY);
			} // if
		BuildSequenceList(Active, NULL);
		} // if
	else if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureSequence(NULL);
		ConfigureDissolve(NULL);
		ActiveShell = NULL;
		BuildSequenceList(NULL, NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		Active->DissolveEnabled = 0;
		Active->SequenceEnabled = 0;
		WidgetSetDisabled(IDC_CHECKDISSOLVE, TRUE);
		WidgetSetDisabled(IDC_CHECKSEQUENCE, TRUE);
		} // else if need to remove tab
	else
		{
		ConfigureSequence(NULL);
		ConfigureDissolve(NULL);
		ActiveShell = NULL;
		BuildSequenceList(NULL, NULL);
		Active->DissolveEnabled = 0;
		Active->SequenceEnabled = 0;
		WidgetSetDisabled(IDC_CHECKDISSOLVE, TRUE);
		WidgetSetDisabled(IDC_CHECKSEQUENCE, TRUE);
		} // else

	// color control
	TabExists = CheckTabExists(TabNames[3]);
	if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
		{
		if (TabExists < 0)
			{
			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[3]);
			TabsChanged = 1;
			} // if need to add tab
		ConfigureColorControl((ColorControlShell *)MyAttr->GetShell());
		WidgetSetDisabled(IDC_CHECKCOLORCONTROL, FALSE);
		if (Active->ColorControlEnabled)
			{
			Active->ImageManagerEnabled = 0;
			WidgetSetDisabled(IDC_CHECKTILING, TRUE);
			WidgetSCSync(IDC_CHECKTILING, WP_SCSYNC_NONOTIFY);
			} // if
		} // if
	else if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureColorControl(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		Active->ColorControlEnabled = 0;
		WidgetSetDisabled(IDC_CHECKCOLORCONTROL, TRUE);
		} // else if need to remove tab
	else
		{
		ConfigureColorControl(NULL);
		Active->ColorControlEnabled = 0;
		WidgetSetDisabled(IDC_CHECKCOLORCONTROL, TRUE);
		} // else

	// georeference
	TabExists = CheckTabExists(TabNames[4]);
	if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
		{
		if (TabExists < 0)
			{
			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[4]);
			TabsChanged = 1;
			} // if need to add tab
		ConfigureGeoRef((GeoRefShell *)MyAttr->GetShell());
		} // if
	else if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureGeoRef(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		} // else if need to remove tab
	else
		{
		ConfigureGeoRef(NULL);
		} // else

	#ifdef WCS_IMAGE_MANAGEMENT
	// image manager
	TabExists = CheckTabExists(TabNames[5]);
	if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
		{
		if (TabExists < 0)
			{
			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[5]);
			TabsChanged = 1;
			} // if need to add tab
		ConfigureImageManager((ImageManagerShell *)MyAttr->GetShell());
		WidgetSetDisabled(IDC_CHECKTILING, (Active->SequenceEnabled || Active->DissolveEnabled ||Active->ColorControlEnabled));
		if (Active->ImageManagerEnabled)
			{
			Active->SequenceEnabled = 0;
			Active->DissolveEnabled = 0;
			Active->ColorControlEnabled = 0;
			WidgetSetDisabled(IDC_CHECKSEQUENCE, TRUE);
			WidgetSetDisabled(IDC_CHECKDISSOLVE, TRUE);
			WidgetSetDisabled(IDC_CHECKCOLORCONTROL, TRUE);
			WidgetSCSync(IDC_CHECKSEQUENCE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKDISSOLVE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKCOLORCONTROL, WP_SCSYNC_NONOTIFY);
			} // if
		} // if
	else if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureImageManager(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		Active->ImageManagerEnabled = 0;
		WidgetSetDisabled(IDC_CHECKTILING, TRUE);
		} // else if need to remove tab
	else
	#endif // WCS_IMAGE_MANAGEMENT
		{
		ConfigureImageManager(NULL);
		Active->ImageManagerEnabled = 0;
		WidgetSetDisabled(IDC_CHECKTILING, TRUE);
		} // else

	strcpy(StashPath, Active->PAF.GetPath());	// store this in case user uses disk dir to get new file and gets wrong type so need to restore
	strcpy(StashName, Active->PAF.GetName());

	} // if Active
else
	{
	WidgetSetModified(IDC_NAME, FALSE);
	WidgetSetText(IDC_NAME, "");
	ConfigureSC(NativeWin, IDC_CHECKENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKALPHAENABLED, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKCOLORCONTROL, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKSEQUENCE, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKDISSOLVE, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKTILING, NULL, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVSHORT, NULL, 0, WCS_RASTER_LONGEVITY_SHORT, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVMED, NULL, 0, WCS_RASTER_LONGEVITY_MEDIUM, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLONGEVSHORT, IDC_RADIOLONGEVLONG, NULL, 0, WCS_RASTER_LONGEVITY_LONG, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLOADFAST, IDC_RADIOLOADFAST, NULL, 0, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOLOADFAST, IDC_RADIOSAVEDISKSPACE, NULL, 0, 0, NULL, NULL);
	sprintf(TextStr, "%1d", 0);
	WidgetSetText(IDC_IMAGEHEIGHT, TextStr);
	WidgetSetText(IDC_IMAGEWIDTH, TextStr);
	WidgetSetText(IDC_NUMCOLORS, TextStr);
	ConfigureDD(NativeWin, IDC_IMAGE_FILE, NULL, 0, NULL, 0, IDC_LABEL_CMAP);
	WidgetSetDisabled(IDC_IMAGE_FILE, TRUE);
	BuildAttributeList();
	TabExists = CheckTabExists(TabNames[2]);
	if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureSequence(NULL);
		ConfigureDissolve(NULL);
		ActiveShell = NULL;
		BuildSequenceList(NULL, NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		} // else if need to remove tab
	TabExists = CheckTabExists(TabNames[3]);
	if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureColorControl(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		} // else if need to remove tab
	TabExists = CheckTabExists(TabNames[4]);
	if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureGeoRef(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		} // else if need to remove tab
	TabExists = CheckTabExists(TabNames[5]);
	if (TabExists >= 0)
		{
		TabsChanged = 1;
		ConfigureImageManager(NULL);
		WidgetTCDeleteItem(IDC_TAB1, TabExists);
		} // else if need to remove tab

	StashPath[0] = StashName[0] = 0;
	} // else
DisableButtons();

if (ResetActiveTab)
	{
	SetTabByTitle(ActiveTabText);
	SelectPanel(-1, (short)TabsChanged);
	} // if
else
	RevealThumbnail();

} // ImageLibGUI::ConfigureWidgets()

/*===========================================================================*/

void ImageLibGUI::DisableButtons(void)
{
RasterAttribute *MyAttr;

if (Active)
	{
	WidgetSetDisabled(IDC_REMOVEIMAGE, FALSE);
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE);
	if (MyAttr)
		{
		WidgetSetDisabled(IDC_ADDDISSOLVE, TRUE);
		WidgetSetDisabled(IDC_REMOVEDISSOLVE, FALSE);
		} // if
	else
		{
		WidgetSetDisabled(IDC_ADDDISSOLVE, FALSE);
		WidgetSetDisabled(IDC_REMOVEDISSOLVE, TRUE);
		} // if
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE);
	if (MyAttr)
		{
		WidgetSetDisabled(IDC_ADDSEQUENCE, TRUE);
		WidgetSetDisabled(IDC_REMOVESEQUENCE, FALSE);
		} // if
	else
		{
		WidgetSetDisabled(IDC_ADDSEQUENCE, FALSE);
		WidgetSetDisabled(IDC_REMOVESEQUENCE, TRUE);
		} // if
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL);
	if (MyAttr)
		{
		WidgetSetDisabled(IDC_ADDCOLORCONTROL, TRUE);
		WidgetSetDisabled(IDC_REMOVECOLORCONTROL, FALSE);
		} // if
	else
		{
		WidgetSetDisabled(IDC_ADDCOLORCONTROL, FALSE);
		WidgetSetDisabled(IDC_REMOVECOLORCONTROL, TRUE);
		} // if
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF);
	if (MyAttr)
		{
		WidgetSetDisabled(IDC_ADDGEOREF, TRUE);
		WidgetSetDisabled(IDC_REMOVEGEOREF, FALSE);
		} // if
	else
		{
		WidgetSetDisabled(IDC_ADDGEOREF, FALSE);
		WidgetSetDisabled(IDC_REMOVEGEOREF, TRUE);
		} // if
	MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER);
	if (MyAttr)
		{
		WidgetSetDisabled(IDC_ADDIMAGEMAN, TRUE);
		WidgetSetDisabled(IDC_REMOVEIMAGEMAN, FALSE);
		} // if
	else
		{
		WidgetSetDisabled(IDC_ADDIMAGEMAN, FALSE);
		WidgetSetDisabled(IDC_REMOVEIMAGEMAN, TRUE);
		} // if
	} // if
else
	{
	WidgetSetDisabled(IDC_REMOVEIMAGE, TRUE);
	WidgetSetDisabled(IDC_ADDDISSOLVE, TRUE);
	WidgetSetDisabled(IDC_REMOVEDISSOLVE, TRUE);
	WidgetSetDisabled(IDC_ADDSEQUENCE, TRUE);
	WidgetSetDisabled(IDC_REMOVESEQUENCE, TRUE);
	WidgetSetDisabled(IDC_ADDCOLORCONTROL, TRUE);
	WidgetSetDisabled(IDC_REMOVECOLORCONTROL, TRUE);
	WidgetSetDisabled(IDC_ADDGEOREF, TRUE);
	WidgetSetDisabled(IDC_REMOVEGEOREF, TRUE);
	WidgetSetDisabled(IDC_ADDIMAGEMAN, TRUE);
	WidgetSetDisabled(IDC_REMOVEIMAGEMAN, TRUE);
	} // else

} // ImageLibGUI::DisableButtons

/*===========================================================================*/

void ImageLibGUI::ListSelectOneItem(WIDGETID ListView, Raster *Active)
{
long Ct, NumItems, Found = 0;

NumItems = WidgetLBGetCount(ListView);

for (Ct = 0; Ct < NumItems; Ct ++)
	{
	if (! Found && ((Raster *)WidgetLBGetItemData(ListView, Ct) == Active))
		{
		WidgetLBSetSelState(ListView, 1, Ct);
		Found = 1;
		} // if
	else
		WidgetLBSetSelState(ListView, 0, Ct);
	} // for

} // ImageLibGUI::ListSelectOneItem

/*===========================================================================*/

void ImageLibGUI::ListSelectOneItemByNumber(WIDGETID ListView, long NumSelection)
{
long Ct, NumItems;

NumItems = WidgetLBGetCount(ListView);

for (Ct = 0; Ct < NumItems; Ct ++)
	{
	if (Ct == NumSelection)
		{
		WidgetLBSetSelState(ListView, 1, Ct);
		} // if
	else
		WidgetLBSetSelState(ListView, 0, Ct);
	} // for

} // ImageLibGUI::ListSelectOneItemByNumber

/*===========================================================================*/

void ImageLibGUI::BuildList(ImageLib *SourceLib, WIDGETID ListView)
{
long Place, ActiveFound = -1;
char ListName[WCS_IMAGE_MAXNAMELENGTH + 2];
Raster *Me;

WidgetLBClear(ListView);
ListItems = 0;

Me = SourceLib->List;
while (Me)
	{
	BuildListEntry(ListName, Me);
	Place = WidgetLBInsert(ListView, -1, ListName);
	WidgetLBSetItemData(ListView, Place, Me);
	if (Active == Me)
		ActiveFound = Place;
	Me = Me->Next;
	ListItems ++;
	} // while
if (ListView == IDC_PARLIST)
	{
	if (ListItems > 0)
		{
		if (ActiveFound >= 0)
			ListSelectOneItemByNumber(ListView, ActiveFound);
//			WidgetLBSetCurSel(ListView, ActiveFound);
		else
			{
			Active = (Raster *)WidgetLBGetItemData(ListView, 0);
			ListSelectOneItemByNumber(ListView, 0);
//			WidgetLBSetCurSel(ListView, 0);
			} // if
		ConfigureWidgets();
		} // if
	else
		Active = NULL;
	} // if
else if (ListView == IDC_BROWSELIST)
	{
	if ((Me = (Raster *)WidgetLBGetItemData(ListView, 0)) != (Raster *)LB_ERR && Me)
		{
		BrowseRast = Me;
		WidgetLBSetSelState(ListView, TRUE, 0);
		RevealThumbnail();
		} // if
	else
		BrowseRast = NULL;
	} // else if

} // ImageLibGUI::BuildList()

/*===========================================================================*/

long ImageLibGUI::FindInList(Raster *FindMe)
{
long Ct, NumItems;
Raster *Test;

NumItems = WidgetLBGetCount(IDC_PARLIST);

for (Ct = 0; Ct < NumItems; Ct ++)
	{
	if ((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Ct)) == FindMe)
		return (Ct);
	} // for

return (-1);

} // ImageLibGUI::FindInList()

/*===========================================================================*/

void ImageLibGUI::BuildListEntry(char *ListName, Raster *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->GetUserName());

} // ImageLibGUI::BuildListEntry()

/*===========================================================================*/

void ImageLibGUI::BuildAttributeList(void)
{
long Place;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 40];
RasterAttribute *Me;
RasterShell *MyShell;
RasterAnimHost *Root;

WidgetLBClear(IDC_EFFECTLIST);

if (Active)
	{
	Me = Active->Attr;
	while (Me)
		{
		if ((MyShell = Me->GetShell()) && MyShell->GetType() != WCS_RASTERSHELL_TYPE_SEQUENCE &&
			MyShell->GetType() != WCS_RASTERSHELL_TYPE_DISSOLVE &&
			MyShell->GetType() != WCS_RASTERSHELL_TYPE_IMAGEMANAGER &&
			MyShell->GetType() != WCS_RASTERSHELL_TYPE_COLORCONTROL)
			{
			// test to see if this attribute belongs to a valid Effect, otherwise it might be a backup copy
			if (MyShell->Host && (Root = MyShell->Host->GetRAHostRoot()) && HostEffectsLib->IsEffectValid((GeneralEffect *)Root, 0))
				{
				if (Me->GetEnabled())
					strcpy(ListName, "* ");
				else
					strcpy(ListName, "  ");
				if (Me->GetName())
					strcat(ListName, Me->GetName());
				else
					strcat(ListName, "Unnamed");
				strcat(ListName, " ");
				if (Me->GetTypeString())
					strcat(ListName, Me->GetTypeString());
				else
					strcat(ListName, "(Unknown type)");
				Place = WidgetLBInsert(IDC_EFFECTLIST, -1, ListName);
				WidgetLBSetItemData(IDC_EFFECTLIST, Place, MyShell);
				} // if
			} // if
		Me = Me->Next;
		} // while
	} // if

} // ImageLibGUI::BuildAttributeList()

/*===========================================================================*/

char BuildName[256];
long ImageLibGUI::BuildSequenceList(Raster *Rast, void *Parent)
{
double NextStart;
SequenceShell *CurrentSeq = NULL;
DissolveShell *CurrentDis = NULL;
RasterShell *Current = NULL;
RasterAttribute *MyAttr;
void *TempHandle, *TempParent;
char NumLoopsTxt[24], StartFrameTxt[24], EndFrameTxt[24];

// delete all items;
if (Parent == NULL)
	{
	TVNumItems = 0;
	WidgetTVDeleteAll(IDC_SEQDISLIST);
	} // if

if (Rast)
	{
	if (MyAttr = Rast->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
		CurrentSeq = (SequenceShell *)MyAttr->GetShell();
	if (MyAttr = Rast->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
		CurrentDis = (DissolveShell *)MyAttr->GetShell();
	while (CurrentSeq || CurrentDis)
		{
		NextStart = 10000000.0;
		if (CurrentSeq)
			{
			Current = (RasterShell *)CurrentSeq;
			NextStart = CurrentSeq->StartFrame - CurrentSeq->StartSpace;
			} // if
		if (CurrentDis && CurrentDis->StartFrame < NextStart)
			{
			Current = (RasterShell *)CurrentDis;
			NextStart = CurrentDis->StartFrame;
			} // if
		if (Current && Current->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
			{
			sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame - CurrentSeq->StartSpace);
			TrimZeros(StartFrameTxt);
			sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame + CurrentSeq->EndSpace);
			TrimZeros(EndFrameTxt);
			sprintf(BuildName, "Seq: %s - %s %s", StartFrameTxt, EndFrameTxt, CurrentSeq->GetSeqName());
			TempParent = WidgetTVInsert(IDC_SEQDISLIST, BuildName, Current, Parent, 1);
			if (TVNumItems < 100)
				TVItemHandles[TVNumItems++] = (long)TempParent;
			
			// pre-sequence space
			sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame - CurrentSeq->StartSpace);
			TrimZeros(StartFrameTxt);
			sprintf(EndFrameTxt, "%f", CurrentSeq->StartFrame);
			TrimZeros(EndFrameTxt);
			if (CurrentSeq->StartBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE)
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Loop");
			else if (CurrentSeq->StartBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD)
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Hold");
			else
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Gap");
			TempHandle = WidgetTVInsert(IDC_SEQDISLIST, BuildName, Current, TempParent, 0);
			if (TVNumItems < 100)
				TVItemHandles[TVNumItems++] = (long)TempHandle;

			// sequence
			sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame);
			TrimZeros(StartFrameTxt);
			sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame);
			TrimZeros(EndFrameTxt);
			sprintf(NumLoopsTxt, "%f", CurrentSeq->NumLoops);
			TrimZeros(NumLoopsTxt);
			sprintf(BuildName, "%s - %s x %s %s", StartFrameTxt, EndFrameTxt, NumLoopsTxt, CurrentSeq->GetSeqName());
			TempHandle = WidgetTVInsert(IDC_SEQDISLIST, BuildName, Current, TempParent, 0);
			if (TVNumItems < 100)
				TVItemHandles[TVNumItems++] = (long)TempHandle;

			// post-sequence space
			sprintf(StartFrameTxt, "%f", CurrentSeq->EndFrame);
			TrimZeros(StartFrameTxt);
			sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame + CurrentSeq->EndSpace);
			TrimZeros(EndFrameTxt);
			if (CurrentSeq->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE)
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Loop");
			else if (CurrentSeq->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD)
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Hold");
			else
				sprintf(BuildName, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Gap");
			TempHandle = WidgetTVInsert(IDC_SEQDISLIST, BuildName, Current, TempParent, 0);
			if (TVNumItems < 100)
				TVItemHandles[TVNumItems++] = (long)TempHandle;

			CurrentSeq = (SequenceShell *)CurrentSeq->NextSeq;
			} // if
		else if (Current && Current->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
			{
			char Children;
			sprintf(BuildName, "Dis: %d - %d %s", CurrentDis->StartFrame, CurrentDis->EndFrame, CurrentDis->GetDisName());
			Children = (CurrentDis->GetTarget() && 
				(CurrentDis->GetTarget()->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) || 
				CurrentDis->GetTarget()->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))) ? 1: 0;
			TempParent = WidgetTVInsert(IDC_SEQDISLIST, BuildName, Current, Parent, Children);
			if (TVNumItems < 100)
				TVItemHandles[TVNumItems++] = (long)TempParent;
			BuildSequenceList(CurrentDis->GetTarget(), TempParent);
			CurrentDis = (DissolveShell *)CurrentDis->NextDis;
			} // if
		else
			break;
		} // while
	} // if

return (0);

} // ImageLibGUI::BuildSequenceList()

/*===========================================================================*/

void ImageLibGUI::UpdateTVText(void)
{
long Ct, SequencePart = 0;
RasterShell *Prev = NULL, *MyShell;
DissolveShell *CurrentDis;
SequenceShell *CurrentSeq;
char NumLoopsTxt[24], StartFrameTxt[24], EndFrameTxt[24], LocalText[255];

for (Ct = 0; Ct < TVNumItems; Ct ++)
	{
	if (MyShell = (RasterShell *)WidgetTVGetItemData(IDC_SEQDISLIST, (void *)TVItemHandles[Ct]))
		{
		if (MyShell == Prev)
			SequencePart ++;
		else
			SequencePart = 0;
		Prev = MyShell;
		if (MyShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
			{
			CurrentSeq = (SequenceShell *)MyShell;
			switch (SequencePart)
				{
				case 0:
					{
					sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame - CurrentSeq->StartSpace);
					TrimZeros(StartFrameTxt);
					sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame + CurrentSeq->EndSpace);
					TrimZeros(EndFrameTxt);
					sprintf(LocalText, "Seq: %s - %s %s", StartFrameTxt, EndFrameTxt, CurrentSeq->GetSeqName());
					break;
					} // parent
				case 1:
					{
					sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame - CurrentSeq->StartSpace);
					TrimZeros(StartFrameTxt);
					sprintf(EndFrameTxt, "%f", CurrentSeq->StartFrame);
					TrimZeros(EndFrameTxt);
					if (CurrentSeq->StartBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE)
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Loop");
					else if (CurrentSeq->StartBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD)
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Hold");
					else
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Pre-sequence Gap");
					break;
					} // pre-sequence
				case 2:
					{
					sprintf(StartFrameTxt, "%f", CurrentSeq->StartFrame);
					TrimZeros(StartFrameTxt);
					sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame);
					TrimZeros(EndFrameTxt);
					sprintf(NumLoopsTxt, "%f", CurrentSeq->NumLoops);
					TrimZeros(NumLoopsTxt);
					sprintf(LocalText, "%s - %s x %s %s", StartFrameTxt, EndFrameTxt, NumLoopsTxt, CurrentSeq->GetSeqName());
					break;
					} // body
				case 3:
					{
					sprintf(StartFrameTxt, "%f", CurrentSeq->EndFrame);
					TrimZeros(StartFrameTxt);
					sprintf(EndFrameTxt, "%f", CurrentSeq->EndFrame + CurrentSeq->EndSpace);
					TrimZeros(EndFrameTxt);
					if (CurrentSeq->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_EXTRAPOLATE)
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Loop");
					else if (CurrentSeq->EndBehavior == WCS_RASTERSHELL_SEQUENCE_BEHAVIOR_HOLD)
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Hold");
					else
						sprintf(LocalText, "%s - %s %s", StartFrameTxt, EndFrameTxt, "Post-sequence Gap");
					break;
					} // post-sequence
				} // switch
			} // if
		else if (MyShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
			{
			CurrentDis = (DissolveShell *)MyShell;
			sprintf(LocalText, "Dis: %d - %d %s", CurrentDis->StartFrame, CurrentDis->EndFrame, CurrentDis->GetDisName());
			} // else if
		} // if
	WidgetTVSetItemText(IDC_SEQDISLIST, (void *)TVItemHandles[Ct], LocalText);
	} // for

} // ImageLibGUI::UpdateTVText

/*===========================================================================*/

void ImageLibGUI::ConfigureSequence(SequenceShell *MyShell)
{

if (MyShell)
	{
	ActiveSequence = MyShell;
	ConfigureFI(NativeWin, IDC_STARTTIME, &MyShell->StartFrame, 1.0, MyShell->PrevSeq ? ((SequenceShell *)MyShell->PrevSeq)->EndFrame + ((SequenceShell *)MyShell->PrevSeq)->EndSpace: 0.0, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDTIME, &MyShell->EndFrame, 1.0, MyShell->StartFrame, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_STARTSPACE, &MyShell->StartSpace, 1.0, 0.0, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDSPACE, &MyShell->EndSpace, 1.0, 0.0, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIRST_IMAGE, &MyShell->StartImage, 1.0, 0.0, (double)MyShell->EndImage, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_LAST_IMAGE, &MyShell->EndImage, 1.0, (double)MyShell->StartImage, 1000000.0, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_SPEED, &MyShell->Speed, 1.0, 1.0, 1000000.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_NUM_LOOPS, &MyShell->NumLoops, 1.0, .001, 1000000.0, FIOFlag_Double, NULL, NULL);
	ConfigureDD(NativeWin, IDC_SEQ_FILE, MyShell->PAF.Path, 255, MyShell->PAF.Name, 31, IDC_LABEL_CMAP);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTENDFRAME, &SequenceAdjust, SCFlag_Long, WCS_IMAGELIB_ADJUSTENDFRAME, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTENDIMAGE, &SequenceAdjust, SCFlag_Long, WCS_IMAGELIB_ADJUSTENDIMAGE, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTSPEED, &SequenceAdjust, SCFlag_Long, WCS_IMAGELIB_ADJUSTSPEED, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTNUMLOOPS, &SequenceAdjust, SCFlag_Long, WCS_IMAGELIB_ADJUSTNUMLOOPS, NULL, NULL);
	WidgetCBSetCurSel(IDC_STARTBEHAVIOR, MyShell->StartBehavior);
	WidgetCBSetCurSel(IDC_ENDBEHAVIOR, MyShell->EndBehavior);
	} // if
else
	{
	ActiveSequence = NULL;
	ConfigureFI(NativeWin, IDC_STARTTIME, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDTIME, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_STARTSPACE, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDSPACE, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIRST_IMAGE, NULL, 1.0, 0.0, 1000000.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_LAST_IMAGE, NULL, 1.0, 0.0, 1000000.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_SPEED, NULL, 1.0, 0.0, 1000000.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_NUM_LOOPS, NULL, 1.0, 0.0, 1000000.0, 0, NULL, NULL);
	ConfigureDD(NativeWin, IDC_SEQ_FILE, NULL, 255, NULL, 31, IDC_LABEL_CMAP);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTENDFRAME, NULL, 0, WCS_IMAGELIB_ADJUSTENDFRAME, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTENDIMAGE, NULL, 0, WCS_IMAGELIB_ADJUSTENDIMAGE, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTSPEED, NULL, 0, WCS_IMAGELIB_ADJUSTSPEED, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOADJUSTENDFRAME, IDC_RADIOADJUSTNUMLOOPS, NULL, 0, WCS_IMAGELIB_ADJUSTNUMLOOPS, NULL, NULL);
	WidgetCBSetCurSel(IDC_STARTBEHAVIOR, 0);
	WidgetCBSetCurSel(IDC_ENDBEHAVIOR, 0);
	} // else

} // ImageLibGUI::ConfigureSequence()

/*===========================================================================*/

void ImageLibGUI::ConfigureDissolve(DissolveShell *MyShell)
{
long NumItems, Current, Place;
Raster *CurrentRast;

if (MyShell)
	{
	ActiveDissolve = MyShell;
	ConfigureFI(NativeWin, IDC_STARTFRAME, &MyShell->StartFrame, 1.0, MyShell->PrevDis ? ((DissolveShell *)MyShell->PrevDis)->EndFrame: 0.0, 32767.0, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDFRAME, &MyShell->EndFrame, 1.0, (double)MyShell->StartFrame, 32767.0, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_DURATION, &MyShell->Duration, 1.0, 0.0, 32767.0, FIOFlag_Long, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKEASEIN, &MyShell->EaseIn, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKEASEOUT, &MyShell->EaseOut, SCFlag_Char, NULL, NULL);
	WidgetCBClear(IDC_TARGETDROP);
	for (CurrentRast = HostLib->List; CurrentRast; CurrentRast = CurrentRast->Next)
		{
		Place = WidgetCBInsert(IDC_TARGETDROP, -1, CurrentRast->GetUserName());
		WidgetCBSetItemData(IDC_TARGETDROP, Place, CurrentRast);
		} // for
	if (MyShell->GetTarget())
		{
		NumItems = WidgetCBGetCount(IDC_TARGETDROP);
		for (Current = 0; Current < NumItems; Current ++)
			{
			if (MyShell->GetTarget() == (Raster *)WidgetCBGetItemData(IDC_TARGETDROP, Current))
				{
				WidgetCBSetCurSel(IDC_TARGETDROP, Current);
				break;
				} // if
			} // for
		} // if
	} // if
else
	{
	ActiveDissolve = NULL;
	ConfigureFI(NativeWin, IDC_STARTFRAME, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDFRAME, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_DURATION, NULL, 1.0, 0.0, 32767.0, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKEASEIN, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKEASEOUT, NULL, 0, NULL, NULL);
	WidgetCBClear(IDC_TARGETDROP);
	} // else

} // ImageLibGUI::ConfigureDissolve()

/*===========================================================================*/

void ImageLibGUI::ConfigureColorControl(ColorControlShell *MyShell)
{

if (MyShell)
	{
	ConfigureSR(NativeWin, IDC_RADIOBANDASSIGNMENT, IDC_RADIOBANDASSIGNMENT, &MyShell->UseBandAssignment, SCFlag_Char, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOBANDASSIGNMENT, IDC_RADIOFORMULA, &MyShell->UseBandAssignment, SCFlag_Char, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOMAXTRANSPAR, &MyShell->DisplayRGBSet, SCFlag_Char, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOMINTRANSPAR, &MyShell->DisplayRGBSet, SCFlag_Char, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOREPLACE, &MyShell->DisplayRGBSet, SCFlag_Char, 2, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKUSECOLOR, &MyShell->UseAsColor, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKAUTOGRAYRANGE, &MyShell->GrayAutoRange, SCFlag_Char, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKSHOWTRANSPAR, &MyShell->ShowTransparency, SCFlag_Char, NULL, NULL);
	WidgetCBSetCurSel(IDC_BANDRED, MyShell->UseBandAs[0]);
	WidgetCBSetCurSel(IDC_BANDGREEN, MyShell->UseBandAs[1]);
	WidgetCBSetCurSel(IDC_BANDBLUE, MyShell->UseBandAs[2]);
	WidgetCBSetCurSel(IDC_BANDOUT, FormulaOutBand);
	WidgetCBSetCurSel(IDC_BANDIN, FormulaInBand);
	ResyncTransparency(MyShell);
	ResyncColorFormula(MyShell);
	DisableColorControlWidgets(MyShell);
	} // if
else
	{
	ConfigureSR(NativeWin, IDC_RADIOBANDASSIGNMENT, IDC_RADIOBANDASSIGNMENT, NULL, 0, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOBANDASSIGNMENT, IDC_RADIOFORMULA, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOMAXTRANSPAR, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOMINTRANSPAR, NULL, 0, 1, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMAXTRANSPAR, IDC_RADIOREPLACE, NULL, 0, 2, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKUSECOLOR, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKAUTOGRAYRANGE, NULL, 0, NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKSHOWTRANSPAR, NULL, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_COLRED, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_COLGRN, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_COLBLU, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_BANDFACTOR, NULL, 1.0, (double)-FLT_MAX, (double)FLT_MAX, 0, NULL, NULL);
	} // else

} // ImageLibGUI::ConfigureColorControl

/*===========================================================================*/

void ImageLibGUI::ConfigureGeoRef(GeoRefShell *MyShell)
{
#ifdef WCS_BUILD_VNS
long CurPos, Pos;
GeneralEffect *MatchEffect, *MyEffect;
#endif // WCS_BUILD_VNS

if (MyShell)
	{
	#ifdef WCS_BUILD_VNS
	WidgetSetDisabled(IDC_COORDSDROP, FALSE);
	CurPos = -1;
	MatchEffect = (GeneralEffect *)MyShell->Host;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = HostEffectsLib->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	if (MyShell->Host)
		{
		if (((CoordSys *)MyShell->Host)->Method.GCTPMethod)
			{
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
			} // if projected
		else
			{
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_LATITUDE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
			MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetMetricType(WCS_ANIMDOUBLE_METRIC_LONGITUDE);
			} // else geographic
		} // if
	#endif // WCS_BUILD_VNS
	WidgetSmartRAHConfig(IDC_NORTH, &MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH], &MyShell->GeoReg);
	WidgetSmartRAHConfig(IDC_WEST, &MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST], &MyShell->GeoReg);
	WidgetSmartRAHConfig(IDC_EAST, &MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST], &MyShell->GeoReg);
	WidgetSmartRAHConfig(IDC_SOUTH, &MyShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH], &MyShell->GeoReg);

	ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLEDGES, &MyShell->BoundsType, SRFlag_Char, WCS_GEOREFSHELL_BOUNDSTYPE_EDGES, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLCENTERS, &MyShell->BoundsType, SRFlag_Char, WCS_GEOREFSHELL_BOUNDSTYPE_CENTERS, NULL, NULL);

	WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
	WidgetSetCheck(IDC_SHIFTBOUNDS, ReceivingDiagnosticsShift);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_NORTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_WEST, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_EAST, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SOUTH, (RasterAnimHost *)NULL, NULL);

	ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLEDGES, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOCELLEDGES, IDC_RADIOCELLCENTERS, NULL, 0, 0, NULL, NULL);

	#ifdef WCS_BUILD_VNS
	WidgetCBClear(IDC_COORDSDROP);
	WidgetSetDisabled(IDC_COORDSDROP, TRUE);
	#endif // WCS_BUILD_VNS
	} // else

} // ImageLibGUI::ConfigureGeoRef

/*===========================================================================*/

void ImageLibGUI::ConfigureImageManager(ImageManagerShell *MyShell)
{
#ifdef WCS_BUILD_VNS
char TextStr[256];

ConfigureFI(NativeWin, IDC_MAXMEM,
 &HostLib->MaxTileMemory,
  10.0,
   1.0,
	1000000.0,
	 FIOFlag_Long,
	  NULL,
	   0);
if (Active && MyShell)
	{
	ConfigureSC(NativeWin, IDC_CHECKVIRTRESENABLED, &MyShell->VirtResEnabled, SCFlag_Char, NULL, NULL);

	ConfigureSR(NativeWin, IDC_RADIOAUTO, IDC_RADIOAUTO, &MyShell->TilingControlType, SRFlag_Char, WCS_IMAGEMGRSHELL_TILINGTYPE_AUTO, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOAUTO, IDC_RADIOMANUAL, &MyShell->TilingControlType, SRFlag_Char, WCS_IMAGEMGRSHELL_TILINGTYPE_MANUAL, NULL, NULL);

	ConfigureSR(NativeWin, IDC_RADIOPIXELRES, IDC_RADIOPIXELRES, &MyShell->VirtResType, SRFlag_Char, WCS_IMAGEMGRSHELL_VIRTRESTYPE_PIXELS, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOPIXELRES, IDC_RADIOSCALEFACTOR, &MyShell->VirtResType, SRFlag_Char, WCS_IMAGEMGRSHELL_VIRTRESTYPE_SCALINGFACTOR, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOPIXELRES, IDC_RADIOGROUNDUNITS, &MyShell->VirtResType, SRFlag_Char, WCS_IMAGEMGRSHELL_VIRTRESTYPE_GROUNDUNITS, NULL, NULL);

	ConfigureFI(NativeWin, IDC_USERTILEX,
	 &MyShell->ManualTileSizeX,
	  1.0,
	   1.0,
		1000000.0,
		 FIOFlag_Long,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_USERTILEY,
	 &MyShell->ManualTileSizeY,
	  1.0,
	   1.0,
		1000000.0,
		 FIOFlag_Long,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_PIXELRESX,
	 &MyShell->VirtResX,
	  1.0,
	   1.0,
		1000000.0,
		 FIOFlag_Long,
		  NULL,
		   0);
	ConfigureFI(NativeWin, IDC_PIXELRESY,
	 &MyShell->VirtResY,
	  1.0,
	   1.0,
		1000000.0,
		 FIOFlag_Long,
		  NULL,
		   0);

	sprintf(TextStr, "%1d", Active->Rows);
	WidgetSetText(IDC_IMAGEHEIGHT2, TextStr);
	sprintf(TextStr, "%1d", Active->Cols);
	WidgetSetText(IDC_IMAGEWIDTH2, TextStr);
	sprintf(TextStr, "%1d", Active->ByteBands);
	WidgetSetText(IDC_NUMCOLORS2, TextStr);
	sprintf(TextStr, "%1d", Active->Rows * Active->Cols * Active->ByteBands);
	WidgetSetText(IDC_MEMREQTXT, TextStr);
	ConfigureImageManagerCacheEst();
	sprintf(TextStr, "%1d", Active->NativeTileWidth);
	WidgetSetText(IDC_INTERNALTILEX, TextStr);
	sprintf(TextStr, "%1d", Active->NativeTileHeight);
	WidgetSetText(IDC_INTERNALTILEY, TextStr);

	if (Active->ImageCapabilityFlags & WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE)
		{
		WidgetSetDisabled(IDC_RADIOAUTO, FALSE);
		WidgetSetDisabled(IDC_RADIOMANUAL, FALSE);
		WidgetSetDisabled(IDC_USERTILEX, MyShell->TilingControlType != WCS_IMAGEMGRSHELL_TILINGTYPE_MANUAL);
		WidgetSetDisabled(IDC_USERTILEY, MyShell->TilingControlType != WCS_IMAGEMGRSHELL_TILINGTYPE_MANUAL);
		sprintf(TextStr, "%1d", MyShell->AutoTileSizeX);
		WidgetSetText(IDC_AUTOTILEX, TextStr);
		sprintf(TextStr, "%1d", MyShell->AutoTileSizeY);
		WidgetSetText(IDC_AUTOTILEY, TextStr);
		} // if
	else
		{
		WidgetSetDisabled(IDC_RADIOAUTO, TRUE);
		WidgetSetDisabled(IDC_RADIOMANUAL, TRUE);
		WidgetSetDisabled(IDC_USERTILEX, TRUE);
		WidgetSetDisabled(IDC_USERTILEY, TRUE);
		WidgetSetText(IDC_AUTOTILEX, "");
		WidgetSetText(IDC_AUTOTILEY, "");
		} // else
#ifdef WCS_IMAGE_MANAGEMENT_VIRTRES
	if (Active->ImageCapabilityFlags & WCS_BITMAPS_IMAGECAPABILITY_SUPPORTVIRTUALRES)
		{
		WidgetSetDisabled(IDC_RADIOPIXELRES, ! MyShell->VirtResEnabled);
		WidgetSetDisabled(IDC_RADIOSCALEFACTOR, ! MyShell->VirtResEnabled);
		WidgetSetDisabled(IDC_RADIOGROUNDUNITS, ! MyShell->VirtResEnabled);
		WidgetSetDisabled(IDC_CHECKVIRTRESENABLED, FALSE);
		WidgetSetDisabled(IDC_PIXELRESX, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_PIXELS);
		WidgetSetDisabled(IDC_PIXELRESY, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_PIXELS);
		WidgetSetDisabled(IDC_SCALEFACTORX, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_SCALINGFACTOR);
		WidgetSetDisabled(IDC_SCALEFACTORY, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_SCALINGFACTOR);
		WidgetSetDisabled(IDC_GROUNDUNITSX, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_GROUNDUNITS);
		WidgetSetDisabled(IDC_GROUNDUNITSY, ! MyShell->VirtResEnabled || MyShell->VirtResType != WCS_IMAGEMGRSHELL_VIRTRESTYPE_GROUNDUNITS);
		} // if
	else
		{
		WidgetSetDisabled(IDC_RADIOPIXELRES, TRUE);
		WidgetSetDisabled(IDC_RADIOSCALEFACTOR, TRUE);
		WidgetSetDisabled(IDC_RADIOGROUNDUNITS, TRUE);
		WidgetSetDisabled(IDC_CHECKVIRTRESENABLED, TRUE);
		WidgetSetDisabled(IDC_PIXELRESX, TRUE);
		WidgetSetDisabled(IDC_PIXELRESY, TRUE);
		WidgetSetDisabled(IDC_SCALEFACTORX, TRUE);
		WidgetSetDisabled(IDC_SCALEFACTORY, TRUE);
		WidgetSetDisabled(IDC_GROUNDUNITSX, TRUE);
		WidgetSetDisabled(IDC_GROUNDUNITSY, TRUE);
		} // else
	WidgetShow(IDC_VIRTRESSPT, Active->ImageCapabilityFlags & WCS_BITMAPS_IMAGECAPABILITY_SUPPORTVIRTUALRES);
	#endif // WCS_IMAGE_MANAGEMENT_VIRTRES

	WidgetShow(IDC_IMGMANSPT, Active->ImageCapabilityFlags & WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE);
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_CHECKVIRTRESENABLED, NULL, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOAUTO, IDC_RADIOAUTO, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMANUAL, IDC_RADIOAUTO, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOPIXELRES, IDC_RADIOPIXELRES, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOSCALEFACTOR, IDC_RADIOPIXELRES, NULL, 0, 0, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOGROUNDUNITS, IDC_RADIOPIXELRES, NULL, 0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_USERTILEX, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_USERTILEY, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_PIXELRESX, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_PIXELRESY, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_SCALEFACTORX, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_SCALEFACTORY, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_GROUNDUNITSX, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	ConfigureFI(NativeWin, IDC_GROUNDUNITSY, NULL, 1.0, 0.0, 255.0, 0, NULL, NULL);
	WidgetShow(IDC_IMGMANSPT, FALSE);
	WidgetShow(IDC_VIRTRESSPT, FALSE);
	WidgetSetText(IDC_IMAGEHEIGHT2, "");
	WidgetSetText(IDC_IMAGEWIDTH2, "");
	WidgetSetText(IDC_NUMCOLORS2, "");
	WidgetSetText(IDC_INTERNALTILEX, "");
	WidgetSetText(IDC_INTERNALTILEY, "");
	WidgetSetText(IDC_AUTOTILEX, "");
	WidgetSetText(IDC_AUTOTILEY, "");
	WidgetSetText(IDC_MEMREQTXT, "");
	} // else
#endif // WCS_BUILD_VNS

} // ImageLibGUI::ConfigureImageManager

/*===========================================================================*/

Raster *ImageLibGUI::GetNextSelected(long &LastOBN)
{
long NumItems;
Raster *ThisGuy;

if ((NumItems = WidgetLBGetCount(IDC_PARLIST)) != LB_ERR)
	{
	for (LastOBN = LastOBN >= 0 ? LastOBN + 1: 0; LastOBN < NumItems; LastOBN ++)
		{
		if (WidgetLBGetSelState(IDC_PARLIST, LastOBN))
			{
			ThisGuy = (Raster *)WidgetLBGetItemData(IDC_PARLIST, LastOBN);
			return (ThisGuy);
			} // if
		} // for
	} // if

return (NULL);

} // ImageLibGUI::GetNextSelected

/*===========================================================================*/

void ImageLibGUI::ShiftBounds(DiagnosticData *Data)
{
RasterAttribute *MyAttr;

if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	if (ReceivingDiagnosticsShift == 0)
		{
		if (UserMessageOKCAN("Shift Geo Reference Bounds", "The next two points clicked in any View\n will be used to shift the georeferencing of this image\nthe distance between the points and towards the second point."))
			{
			ReceivingDiagnosticsShift = 1;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingDiagnosticsShift == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				ReceivingDiagnosticsShift = 2;
				} // if
			} // if
		else
			ReceivingDiagnosticsShift = 0;
		} // else if
	else if (ReceivingDiagnosticsShift == 2)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];

				// Add the X Delta and YDelta to each of the respective X and Y coords of the
				// georeferencing
				((GeoRefShell *)MyAttr->GetShell())->ShiftBounds(LatEvent, LonEvent);
				ReceivingDiagnosticsShift = 0;
				} // if
			} // if
		else
			ReceivingDiagnosticsShift = 0;
		} // else if
	else
		ReceivingDiagnosticsShift = 0;

	WidgetSetCheck(IDC_SHIFTBOUNDS, ReceivingDiagnosticsShift);
	} // if

} // ImageLibGUI::ShiftBounds

/*===========================================================================*/

void ImageLibGUI::SetBounds(DiagnosticData *Data)
{
RasterAttribute *MyAttr;

if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	if (ReceivingDiagnostics == 0)
		{
		if (UserMessageOKCAN("Set Geo Reference Bounds", "The next two points clicked in any View\n will become this Image Objects's new bounds.\n\nPoints may be selected in any order."))
			{
			ReceivingDiagnostics = 1;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingDiagnostics == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[0] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[0] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				ReceivingDiagnostics = 2;
				} // if
			} // if
		else
			ReceivingDiagnostics = 0;
		} // else if
	else if (ReceivingDiagnostics == 2)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent[1] = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent[1] = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				((GeoRefShell *)MyAttr->GetShell())->SetBounds(LatEvent, LonEvent);
				ReceivingDiagnostics = 0;
				} // if
			} // if
		else
			ReceivingDiagnostics = 0;
		} // else if
	else
		ReceivingDiagnostics = 0;

	WidgetSetCheck(IDC_SETBOUNDS, ReceivingDiagnostics);
	} // if

} // ImageLibGUI::SetBounds

/*===========================================================================*/

void ImageLibGUI::ConfigureSequenceColors(ColorControlShell *MyShell)
{

SetColorPot(0, (unsigned char)min(255, MyShell->RGB[0][0]),
 (unsigned char)min(255, MyShell->RGB[0][1]),
  (unsigned char)min(255, MyShell->RGB[0][2]), 1);
ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);
SetColorPot(1, (unsigned char)min(255, MyShell->RGB[1][0]),
 (unsigned char)min(255, MyShell->RGB[1][1]),
  (unsigned char)min(255, MyShell->RGB[1][2]), 1);
ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);
SetColorPot(2, (unsigned char)min(255, MyShell->RGB[2][0]),
 (unsigned char)min(255, MyShell->RGB[2][1]),
  (unsigned char)min(255, MyShell->RGB[2][2]), 1);
ConfigureCB(NativeWin, ID_COLORPOT3, 100, CBFlag_CustomColor, 2);


} // ImageLibGUI::ConfigureSequenceColors

/*===========================================================================*/

void ImageLibGUI::AddSequenceDissolve(void)
{

if (ActiveShell)
	{
	if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
		AddDissolve();
	else if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
		AddSequence();
	} // if

} // ImageLibGUI::AddSequenceDissolve

/*===========================================================================*/

void ImageLibGUI::AddSequence(void)
{
long NumItems, Ct;
RasterAttribute *MyAttr;
SequenceShell *MyShell = NULL, *OldShell = NULL;
char *MyName;

if (Active)
	{
	if (Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER) && Active->ImageManagerEnabled)
		{
		UserMessageOK("Add Sequence", "Sequence and Image Manager attributes are incompatible. If you wish to use a Sequence effect on this image you will need to remove or disable the Image Manager attribute.");
		return;
		} // if
	if (! (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE)))
		{
		if (MyAttr = Active->AddAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
			{
			MyShell = (SequenceShell *)MyAttr->GetShell();
			Active->SequenceEnabled = 1;
			WidgetSetDisabled(IDC_CHECKSEQUENCE, FALSE);
			WidgetSCSync(IDC_CHECKSEQUENCE, WP_SCSYNC_NONOTIFY);

			WidgetSetDisabled(IDC_CHECKTILING, TRUE);
			} // if
		} // if
	else if (MyShell = (SequenceShell *)MyAttr->GetShell())
		{
		while (MyShell->NextSeq)
			{
			MyShell = (SequenceShell *)MyShell->NextSeq;
			} // while
		OldShell = MyShell;
		MyShell->NextSeq = (RasterShell *)new SequenceShell;
		MyShell = (SequenceShell *)MyShell->NextSeq;
		} // else if
	if (MyShell)
		{
		MyShell->PrevSeq = (RasterShell *)OldShell;
		MyShell->SetSeqPath((char *)Active->GetPath());
		MyShell->SetSeqName((char *)Active->GetName());
		MyShell->SetRaster(Active);
		for (Ct = (long)(strlen(MyName = (char *)MyShell->GetSeqName()) - 1); Ct >= 0; Ct --)
			{
			if (isdigit((unsigned char)MyName[Ct]))
				break;
			} // for
		for (; Ct >= 0; Ct --)
			{
			if (isdigit((unsigned char)MyName[Ct]))
				MyName[Ct] = '#';
			else
				break;
			} // for
		if (MyShell->PrevSeq)
			{
			MyShell->StartFrame = ((SequenceShell *)MyShell->PrevSeq)->EndFrame + ((SequenceShell *)MyShell->PrevSeq)->EndSpace;
			MyShell->EndFrame = MyShell->StartFrame + MyShell->Duration;
			} // if

		NumItems = WidgetTCGetItemCount(IDC_TAB1);
		if (CheckTabExists(TabNames[2]) < 0)
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[2]);
		ConfigureSequence(MyShell);
		ActiveShell = (RasterShell *)MyShell;
		SelectPanel(IDD_IMAGE_LIB_SEQDIS, 0);
		BuildSequenceList(Active, NULL);
		} // if
	} // if

} // ImageLibGUI::AddSequence()

/*===========================================================================*/

void ImageLibGUI::AddDissolve(void)
{
long NumItems;
RasterAttribute *MyAttr;
DissolveShell *MyShell = NULL, *OldShell = NULL;

if (Active)
	{
	if (Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER) && Active->ImageManagerEnabled)
		{
		UserMessageOK("Add Dissolve", "Disolve and Image Manager attributes are incompatible. If you wish to use a Dissolve effect on this image you will need to remove or disable the Image Manager attribute.");
		return;
		} // if
	if (! (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE)))
		{
		if (MyAttr = Active->AddAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
			{
			MyShell = (DissolveShell *)MyAttr->GetShell();
			Active->DissolveEnabled = 1;
			WidgetSetDisabled(IDC_CHECKDISSOLVE, FALSE);
			WidgetSCSync(IDC_CHECKDISSOLVE, WP_SCSYNC_NONOTIFY);

			WidgetSetDisabled(IDC_CHECKTILING, TRUE);
			} // if
		} // if
	else if (MyShell = (DissolveShell *)MyAttr->GetShell())
		{
		while (MyShell->NextDis)
			{
			MyShell = (DissolveShell *)MyShell->NextDis;
			} // while
		OldShell = MyShell;
		MyShell->NextDis = (RasterShell *)new DissolveShell;
		MyShell = (DissolveShell *)MyShell->NextDis;
		} // else if
	if (MyShell)
		{
		MyShell->PrevDis = (RasterShell *)OldShell;
		MyShell->SetRaster(Active);
		if (MyShell->PrevDis)
			{
			MyShell->StartFrame = ((DissolveShell *)MyShell->PrevDis)->EndFrame;
			MyShell->EndFrame = MyShell->StartFrame + MyShell->Duration;
			} // if

		NumItems = WidgetTCGetItemCount(IDC_TAB1);
		if (CheckTabExists(TabNames[2]) < 0)
			WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[2]);
		ConfigureDissolve(MyShell);
		ActiveShell = (RasterShell *)MyShell;
		SelectPanel(IDD_IMAGE_LIB_SEQDIS, 0);
		BuildSequenceList(Active, NULL);
		} // if
	} // if

} // ImageLibGUI::AddDissolve()

/*===========================================================================*/

void ImageLibGUI::AddColorControl(void)
{
long NumItems;
RasterAttribute *MyAttr;

if (Active)
	{
	if (Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER) && Active->ImageManagerEnabled)
		{
		UserMessageOK("Add Color Control", "Color Control and Image Manager attributes are incompatible. If you wish to use a Color Control effect on this image you will need to remove or disable the Image Manager attribute.");
		return;
		} // if
	if (! Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
		{
		if (MyAttr = Active->AddAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
			{
			Active->ColorControlEnabled = 1;
			WidgetSetDisabled(IDC_CHECKCOLORCONTROL, FALSE);
			WidgetSCSync(IDC_CHECKCOLORCONTROL, WP_SCSYNC_NONOTIFY);

			WidgetSetDisabled(IDC_CHECKTILING, TRUE);

			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			if (CheckTabExists(TabNames[3]) < 0)
				WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[3]);
			SelectPanel(IDD_IMAGE_LIB_COLORCONTROL, 0);
			ConfigureColorControl((ColorControlShell *)MyAttr->Shell);
			} // if
		} // if
	else
		UserMessageOK("Add Color Control", "This image already has a Color Control attribute.");
	} // if

} // ImageLibGUI::AddColorControl()

/*===========================================================================*/

void ImageLibGUI::AddGeoRef(void)
{
long NumItems;
RasterAttribute *MyAttr;

if (Active)
	{
	if (! Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
		{
		if (MyAttr = Active->AddAttribute(WCS_RASTERSHELL_TYPE_GEOREF))
			{
			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			if (CheckTabExists(TabNames[4]) < 0)
				WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[4]);
			SelectPanel(IDD_IMAGE_LIB_GEOREF, 0);
			ConfigureGeoRef((GeoRefShell *)MyAttr->Shell);
			} // if
		} // if
	else
		UserMessageOK("Add Geo Reference", "This image already has a Geo Reference attribute.");
	} // if

} // ImageLibGUI::AddGeoRef()

/*===========================================================================*/

void ImageLibGUI::AddImageManager(void)
{
#ifdef WCS_IMAGE_MANAGEMENT
long NumItems;
RasterAttribute *MyAttr;

if (Active)
	{
	if ((Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL) && Active->ColorControlEnabled) ||
		(Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE) && Active->SequenceEnabled) ||
		(Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE) && Active->DissolveEnabled))
		{
		UserMessageOK("Add Image Manager", "Color Control, Dissolve and Sequence attributes and Image Manager attributes are incompatible. If you wish to use an Image Manager on this image you will need to remove or disable Color Control, Dissolve and Sequence attributes.");
		return;
		} // if
	if (! Active->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
		{
		if (MyAttr = Active->AddAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
			{
			Active->ImageManagerEnabled = 1;
			WidgetSetDisabled(IDC_CHECKTILING, FALSE);
			WidgetSCSync(IDC_CHECKTILING, WP_SCSYNC_NONOTIFY);

			WidgetSetDisabled(IDC_CHECKCOLORCONTROL, TRUE);
			WidgetSetDisabled(IDC_CHECKDISSOLVE, TRUE);
			WidgetSetDisabled(IDC_CHECKSEQUENCE, TRUE);

			NumItems = WidgetTCGetItemCount(IDC_TAB1);
			if (CheckTabExists(TabNames[5]) < 0)
				WidgetTCInsertItem(IDC_TAB1, NumItems, TabNames[5]);
			SelectPanel(IDD_IMAGE_LIB_IMAGE_MANAGE, 0);
			ConfigureImageManager((ImageManagerShell *)MyAttr->Shell);
			} // if
		} // if
	else
		UserMessageOK("Add Image Manager", "This image already has an Image Manager attribute.");
	} // if
#endif // WCS_IMAGE_MANAGEMENT

} // ImageLibGUI::AddImageManager()

/*===========================================================================*/

void ImageLibGUI::SelectPanel(short PanelID, short TabRemoved)
{
long Current;
char TabText[24];
short ShowMe = 0, ShowMe2 = 0;

if (PanelID < 0)
	{
	Current = WidgetTCGetCurSel(IDC_TAB1);
	WidgetTCGetItemText(IDC_TAB1, Current, TabText, 24);
	if (! strcmp(TabText, TabNames[2]))
		{
		ShowMe = 2;
		if (ActiveShell)
			{
			if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
				ShowMe2 = 0;
			else if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
				ShowMe2 = 1;
			} // if
		} // Dissolve
	else if (! strcmp(TabText, TabNames[3]))
		{
		ShowMe = 3;
		} // Color Control
	else if (! strcmp(TabText, TabNames[4]))
		{
		ShowMe = 4;
		} // Geo Reference
	#ifdef WCS_IMAGE_MANAGEMENT
	else if (! strcmp(TabText, TabNames[5]))
		{
		ShowMe = 5;
		} // Image Manager
	#endif // WCS_IMAGE_MANAGEMENT
	else if (! strcmp(TabText, TabNames[1]))
		{
		ShowMe = 1;
		} // Browse
	else
		{
		ShowMe = 0;
		} // Image
	} // if
else
	{
	switch(PanelID)
		{
		case IDD_IMAGE_LIB_SEQDIS:
			{
			ShowMe = 2;
			if (ActiveShell)
				{
				if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
					ShowMe2 = 0;
				else if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
					ShowMe2 = 1;
				} // if
			SetTabByTitle(TabNames[2]);
			break;
			} // IDD_IMAGE_LIB_DISSOLVETREE
		case IDD_IMAGE_LIB_DISSOLVETREE:
			{
			ShowMe = 2;
			ShowMe2 = 0;
			break;
			} // IDD_IMAGE_LIB_DISSOLVETREE
		case IDD_IMAGE_LIB_SEQUENCETREE:
			{
			ShowMe = 2;
			ShowMe2 = 1;
			break;
			} // IDD_IMAGE_LIB_SEQUENCE
		case IDD_IMAGE_LIB_COLORCONTROL:
			{
			ShowMe = 3;
			SetTabByTitle(TabNames[3]);
			break;
			} // IDD_IMAGE_LIB_COLORCONTROL
		case IDD_IMAGE_LIB_GEOREF:
			{
			ShowMe = 4;
			SetTabByTitle(TabNames[4]);
			break;
			} // IDD_IMAGE_LIB_COLORCONTROL
		case IDD_IMAGE_LIB_IMAGE_MANAGE:
			{
			ShowMe = 5;
			SetTabByTitle(TabNames[5]);
			break;
			} // IDD_IMAGE_LIB_COLORCONTROL
		case IDD_IMAGE_LIB_BROWSE:
			{
			ShowMe = 1;
			SetTabByTitle(TabNames[1]);
			break;
			} // IDD_IMAGE_LIB_BROWSE
		case IDD_IMAGE_LIB_IMAGE:
			{
			ShowMe = 0;
			SetTabByTitle(TabNames[0]);
			break;
			} // IDD_IMAGE_LIB_IMAGE
		default:
			break;
		} // PanelID
	} // else

if (TabRemoved || ShowMe != CurPanel)
	{
	ShowPanel(1, -1);
	ShowPanel(0, ShowMe);
	CurPanel = ShowMe;
	InvalidateRect(NativeWin, NULL, TRUE);
	} // if

if (ShowMe == 2)
	{
	ShowPanel(1, ShowMe2);
	CurSeqDisPanel = ShowMe2;
	} // if

RevealThumbnail();

} // ImageLibGUI::SelectPanel()

/*===========================================================================*/

long ImageLibGUI::SetTabByTitle(char *MatchText)
{
long NumItems, Current, Found = 0;
char TabText[24];

NumItems = WidgetTCGetItemCount(IDC_TAB1);

for (Current = 0; Current < NumItems && ! Found; Current ++)
	{
	WidgetTCGetItemText(IDC_TAB1, Current, TabText, 24);
	if (! strcmp(TabText, MatchText))
		{
		WidgetTCSetCurSel(IDC_TAB1, Current);	// this does not cause notification
		Found = 1;
		} // if
	} // for
if (NumItems > 0 && ! Found)
	WidgetTCSetCurSel(IDC_TAB1, 0);	// this does not cause notification
return (Current);

} // ImageLibGUI::SetTabByTitle

/*===========================================================================*/

int ImageLibGUI::CheckTabExists(char *MatchText)
{
long NumItems, Current, Found = -1;
char TabText[24];

NumItems = WidgetTCGetItemCount(IDC_TAB1);

for (Current = 0; Current < NumItems && Found < 0; Current ++)
	{
	WidgetTCGetItemText(IDC_TAB1, Current, TabText, 24);
	if (! strcmp(TabText, MatchText))
		{
		Found = Current;
		} // if
	} // for
return (Found);

} // ImageLibGUI::CheckTabExists

/*===========================================================================*/

void ImageLibGUI::AddImage(char UseSimpleRequester)
{
Raster *NewRast;

if (NewRast = HostLib->AddRequestRaster(UseSimpleRequester))
	HostLib->SetActive(NewRast);

} // ImageLibGUI::AddImage

/*===========================================================================*/

void ImageLibGUI::SelectImage(void)
{
long Current;
Raster *NewActive;

// is there more than one item selected? then this is a multi-selection and the active item does not change
if ((Current = WidgetLBGetSelCount(IDC_PARLIST)) > 1)
	return;

if ((Current = WidgetLBGetCurSel(IDC_PARLIST)) != LB_ERR)
	{
	if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Current)) != Active && NewActive != (Raster *)LB_ERR)
		{
		HostLib->SetActive(NewActive);
		} // if
	} // if

} // ImageLibGUI::SelectImage

/*===========================================================================*/

void ImageLibGUI::SelectBrowseImage(void)
{
long Current;
Raster *NewBrowseActive;

if ((Current = WidgetLBGetCurSel(IDC_BROWSELIST)) != LB_ERR)
	{
	if ((NewBrowseActive = (Raster *)WidgetLBGetItemData(IDC_BROWSELIST, Current)) != BrowseRast && NewBrowseActive != (Raster *)LB_ERR)
		{
		BrowseRast = NewBrowseActive;
		RevealThumbnail();
		} // if
	} // if
else
	BrowseRast = NULL;

} // ImageLibGUI::SelectBrowseImage

/*===========================================================================*/

void ImageLibGUI::SelectSequenceOrDissolve(RasterShell *NewShell)
{

if (NewShell)
	{
	if (Active->MatchRasterShell(NewShell, TRUE) || HostLib->MatchRasterShell(NewShell, TRUE))
		{
		if (NewShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
			{
			// configure the sequence data
			ConfigureSequence((SequenceShell *)NewShell);
			// make the selected sequence the active shell
			ActiveShell = NewShell;
			SelectPanel(IDD_IMAGE_LIB_SEQDIS, 0);
			RevealThumbnail();
			} // if
		else if (NewShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
			{
			// configure the dissolve data
			ConfigureDissolve((DissolveShell *)NewShell);
			// make the selected dissolve the active shell
			ActiveShell = NewShell;
			SelectPanel(IDD_IMAGE_LIB_SEQDIS, 0);
			RevealThumbnail();
			} // else if
		} // if shell still exists
	else
		{
		ConfigureSequence(NULL);
		ConfigureDissolve(NULL);
		ActiveShell = NULL;
		RevealThumbnail();
		} // else
	} // if

} // ImageLibGUI::SelectSequenceOrDissolve

/*===========================================================================*/

void ImageLibGUI::StartBehavior(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_STARTBEHAVIOR);
if (Active && ActiveSequence && Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
	{
	ActiveSequence->StartBehavior = (char)Current;
	UpdateTVText();
	} // if

} // ImageLibGUI::StartBehavior

/*===========================================================================*/

void ImageLibGUI::EndBehavior(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_ENDBEHAVIOR);
if (Active && ActiveSequence && Active->MatchAttribute(WCS_RASTERSHELL_TYPE_SEQUENCE))
	{
	ActiveSequence->EndBehavior = (char)Current;
	UpdateTVText();
	} // if

} // ImageLibGUI::EndBehavior

/*===========================================================================*/

void ImageLibGUI::SelectDissolveTarget(void)
{
long Current;
Raster *NewRast, *OldRast, *MatchRast;

Current = WidgetCBGetCurSel(IDC_TARGETDROP);
if (Active && ActiveDissolve && Active->MatchAttribute(WCS_RASTERSHELL_TYPE_DISSOLVE))
	{
	if (NewRast = (Raster *)WidgetCBGetItemData(IDC_TARGETDROP, Current))
		{
		if (NewRast == (Raster *)CB_ERR)
			NewRast = NULL;
		OldRast = ActiveDissolve->GetTarget();
		ActiveDissolve->SetTarget(NewRast);

		if (MatchRast = HostLib->CheckDissolveRecursion())
			{
			UserMessageOK(MatchRast->GetUserName(), "Illegal Target! Recursive dissolve created in this Image.");
			ActiveDissolve->SetTarget(OldRast);
			} // if
		ConfigureDissolve(ActiveDissolve);
		UpdateTVText();
		RevealThumbnail();
		} // if
	} // if

} // ImageLibGUI::SelectDissolveTarget

/*===========================================================================*/

void ImageLibGUI::RevealThumbnail(void)
{

if (Active)
	{
	if (CurPanel == 0)
		{
		ConfigureTB(NativeWin, IDC_TNAILIMAGE, NULL, NULL, Active);
		ImageRast = Active;
		} // if
	else if (CurPanel == 2 && CurSeqDisPanel == 0)
		{
		if (ActiveDissolve)
			{
			if (ActiveDissolve->PrevDis)
				{
				ConfigureTB(NativeWin, IDC_TNAILDISSOLVE1, NULL, NULL, ((DissolveShell *)ActiveDissolve->PrevDis)->GetTarget());
				DissolveRast = ((DissolveShell *)ActiveDissolve->PrevDis)->GetTarget();
				} // if
			else
				{
				ConfigureTB(NativeWin, IDC_TNAILDISSOLVE1, NULL, NULL, ActiveDissolve->GetRaster());
				DissolveRast = ActiveDissolve->GetRaster();
				} // else
			ConfigureTB(NativeWin, IDC_TNAILDISSOLVE2, NULL, NULL, ActiveDissolve->GetTarget());
			DissolveRast2 = ActiveDissolve->GetTarget();
			} // if
		else
			{
			ConfigureTB(NativeWin, IDC_TNAILDISSOLVE1, NULL, NULL, NULL);
			ConfigureTB(NativeWin, IDC_TNAILDISSOLVE2, NULL, NULL, NULL);
			DissolveRast = NULL;
			DissolveRast2 = NULL;
			} // else
		} // if
	else if (CurPanel == 2 && CurSeqDisPanel == 1)
		{
		if (ActiveSequence)
			{
			ConfigureTB(NativeWin, IDC_TNAILSEQUENCE, NULL, NULL, ActiveSequence->GetRaster());
			SequenceRast = ActiveSequence->GetRaster();
			} // if
		else
			{
			ConfigureTB(NativeWin, IDC_TNAILSEQUENCE, NULL, NULL, NULL);
			SequenceRast = NULL;
			} // else
		} // if
	else if (CurPanel == 3)
		{
		ConfigureTB(NativeWin, IDC_TNAILCOLORCONTROL, NULL, NULL, Active);
		ColorControlRast = Active;
		} // if
	else if (CurPanel == 1)
		{
		ConfigureTB(NativeWin, IDC_TNAILBROWSE, NULL, NULL, BrowseRast);
		} // if
	} // if
else
	{
	ConfigureTB(NativeWin, IDC_TNAILIMAGE, NULL, NULL, NULL);
	ImageRast = NULL;
	} // else

} // ImageLibGUI::RevealThumbnail

/*===========================================================================*/

void ImageLibGUI::RemoveImage(int RemoveAll, int RemoveUnusedOnly)
{
NotifyTag Changes[2];
long ActiveItem, Selected, ListItems, Ct, ItemCt;
Raster *NewActive, **ItemList;
char MesgStr[128];

if (RemoveAll && RemoveUnusedOnly)
	{
	if (UserMessageOKCAN("Image Library", "Remove ALL Unused Images from the Library?"))
		{
		HostLib->RemoveAll(HostEffectsLib, TRUE);
		} // if
	} // if
else if (RemoveAll)
	{
	if (UserMessageOKCAN("Image Library", "Remove ALL Images from the Library?"))
		{
		HostLib->RemoveAll(HostEffectsLib, FALSE);
		} // if
	} // if
else if (Active)
	{
	Selected = WidgetLBGetSelCount(IDC_PARLIST);
	if (Selected > 1)
		{
		sprintf(MesgStr, "Remove %d selected Images from the Library?", Selected);
		if (UserMessageOKCAN("Remove Image Objects", MesgStr))
			{
			// remove all selected items
			// identify the active item
			if ((ActiveItem = FindInList(Active)) >= 0)
				{
				// make list of selected items
				if (ItemList = (Raster **)AppMem_Alloc(Selected * sizeof (Raster *), APPMEM_CLEAR))
					{
					Active = NULL;
					ListItems = WidgetLBGetCount(IDC_PARLIST);
					for (Ct = ListItems - 1, ItemCt = 0; Ct >= 0 && ItemCt < Selected; Ct --)
						{
						if (WidgetLBGetSelState(IDC_PARLIST, Ct))
							{
							if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Ct)) && (NewActive != (Raster *)LB_ERR))
								{
								ItemList[ItemCt] = NewActive;
								ItemCt ++;
								} // if
							} // if
						} // for
					// remove each selected item
					for (ItemCt = 0; ItemCt < Selected; ItemCt ++)
						{
						if (ItemList[ItemCt])
							HostLib->RemoveRaster(ItemList[ItemCt], HostEffectsLib);
						} // for
					// get the next or previous item
					if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, ActiveItem)) == (Raster *)LB_ERR)
						{
						ListItems -= Selected;
						if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, ListItems - 1)) == (Raster *)LB_ERR)
							NewActive = NULL;
						} // if
					Active = NewActive;
					Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
					Changes[1] = NULL;
					GlobalApp->AppEx->GenerateNotify(Changes, NULL);
					AppMem_Free(ItemList, Selected * sizeof (Raster *));
					} // if
				} // if
			} // if
		} // if
	else
		{
		if (UserMessageOKCAN(Active->GetUserName(), "Remove this Image from the Library?"))
			{
			// identify the active item
			if ((ActiveItem = FindInList(Active)) >= 0)
				{
				// get the next or previous item
				if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, ActiveItem + 1)) == (Raster *)LB_ERR)
					{
					if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_PARLIST, ActiveItem - 1)) == (Raster *)LB_ERR)
						NewActive = NULL;
					} // if
				} // if

			if (HostLib->RemoveRaster(Active, HostEffectsLib))
				Active = NewActive;
			Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		} // if
	} // if

} // ImageLibGUI::RemoveImage

/*===========================================================================*/

void ImageLibGUI::UpdateImage(int UpdateCoordSys)
{
char FileName[256];
FILE *ffile;
NotifyTag Changes[2];

if (Active)
	{
	if (Active->PAF.GetPathAndName(FileName))
		{
		StripExtension(FileName);
		strcat(FileName, ".wfl");
		if (ffile = PROJ_fopen(FileName, "rb"))
			{
			fclose(ffile);
			PROJ_remove(FileName);
			} // if
		} // if
	GlobalApp->MCP->GoModal();
	if (! Active->LoadnPrepImage(FALSE, UpdateCoordSys, Active->QueryImageManagementEnabled()))
		{
		// image file could not be loaded so restore old path
		Active->PAF.SetPath(StashPath);
		Active->PAF.SetName(StashName);
		} // if
	GlobalApp->MCP->EndModal();
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // ImageLibGUI::UpdateImage

/*===========================================================================*/

void ImageLibGUI::DoBandAssignment(short ButtonID)
{
long Current;
RasterAttribute *MyAttr;
ColorControlShell *MyShell;

if (Active)
	{
	if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
		{
		if (MyShell = (ColorControlShell *)MyAttr->GetShell())
			{
			Current = WidgetCBGetCurSel(ButtonID);
			if (ButtonID == IDC_BANDRED)
				MyShell->UseBandAs[0] = (char)Current;
			else if (ButtonID == IDC_BANDGREEN)
				MyShell->UseBandAs[1] = (char)Current;
			else if (ButtonID == IDC_BANDBLUE)
				MyShell->UseBandAs[2] = (char)Current;
			} // if
		} // if
	} // if

} // ImageLibGUI::DoBandAssignment

/*===========================================================================*/

void ImageLibGUI::DoFormulaOutBand(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_BANDOUT);
FormulaOutBand = Current;
ResyncColorFormula(NULL);

} // ImageLibGUI::DoFormulaOutBand

/*===========================================================================*/

void ImageLibGUI::DoFormulaInBand(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_BANDIN);
FormulaInBand = Current;
ResyncColorFormula(NULL);

} // ImageLibGUI::DoFormulaInBand

/*===========================================================================*/

void ImageLibGUI::ResyncColorFormula(ColorControlShell *MyShell)
{
char FormulaText[256], ValueText[256], Band, FoundBand = 0;
RasterAttribute *MyAttr;

if (Active)
	{
	if (! MyShell)
		{
		if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
			{
			MyShell = (ColorControlShell *)MyAttr->GetShell();
			} // if
		} // if
	if (MyShell)
		{
		ConfigureFI(NativeWin, IDC_BANDFACTOR, &MyShell->BandFactor[FormulaOutBand][FormulaInBand], 0.1, (double)-FLT_MAX, (double)FLT_MAX, FIOFlag_Double, NULL, NULL);
		if (FormulaOutBand == 0)
			strcpy(FormulaText, "Red = ");
		else if (FormulaOutBand == 1)
			strcpy(FormulaText, "Green = ");
		else
			strcpy(FormulaText, "Blue = ");
		for (Band = 0; Band < WCS_RASTER_MAX_BANDS; Band ++)
			{
			if (MyShell->BandFactor[FormulaOutBand][Band] != 0.0)
				{
				sprintf(ValueText, "B%d x %1f", Band + 1, MyShell->BandFactor[FormulaOutBand][Band]);
				TrimZeros(ValueText);
				if (FoundBand)
					strcat(FormulaText, " + ");
				strcat(FormulaText, ValueText);
				FoundBand = 1;
				} // if
			} // for
		if (! FoundBand)
			strcat(FormulaText, "0");

		WidgetSetText(IDC_FORMULA, FormulaText);
		} // if
	else
		WidgetSetText(IDC_FORMULA, "");
	} // if
else
	WidgetSetText(IDC_FORMULA, "");

} // ImageLibGUI::ResyncColorFormula

/*===========================================================================*/

void ImageLibGUI::ResyncTransparency(ColorControlShell *MyShell)
{
RasterAttribute *MyAttr;

if (Active)
	{
	if (! MyShell)
		{
		if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
			{
			MyShell = (ColorControlShell *)MyAttr->GetShell();
			} // if
		} // if
	if (MyShell)
		{
		ConfigureFI(NativeWin, IDC_COLRED, &MyShell->RGB[MyShell->DisplayRGBSet][0], 1.0, 0.0, 255.0, FIOFlag_Short, NULL, NULL);
		ConfigureFI(NativeWin, IDC_COLGRN, &MyShell->RGB[MyShell->DisplayRGBSet][1], 1.0, 0.0, 255.0, FIOFlag_Short, NULL, NULL);
		ConfigureFI(NativeWin, IDC_COLBLU, &MyShell->RGB[MyShell->DisplayRGBSet][2], 1.0, 0.0, 255.0, FIOFlag_Short, NULL, NULL);
		WidgetSetScrollPos(IDC_SCROLLBAR1, MyShell->RGB[MyShell->DisplayRGBSet][0]);
		WidgetSetScrollPos(IDC_SCROLLBAR2, MyShell->RGB[MyShell->DisplayRGBSet][1]);
		WidgetSetScrollPos(IDC_SCROLLBAR3, MyShell->RGB[MyShell->DisplayRGBSet][2]);
		} // if
	ConfigureSequenceColors(MyShell);
	} // if

} // ImageLibGUI::ResyncTransparency

/*===========================================================================*/

void ImageLibGUI::DoTransparencySelect(char Item)
{
RasterAttribute *MyAttr;
ColorControlShell *MyShell;

if (Active)
	{
	if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
		{
		if (MyShell = (ColorControlShell *)MyAttr->GetShell())
			{
			MyShell->DisplayRGBSet = Item;
			WidgetSRSync(IDC_RADIOMAXTRANSPAR, WP_SRSYNC_NONOTIFY);
			WidgetSRSync(IDC_RADIOMINTRANSPAR, WP_SRSYNC_NONOTIFY);
			WidgetSRSync(IDC_RADIOREPLACE, WP_SRSYNC_NONOTIFY);
			ResyncTransparency(MyShell);
			} // if
		} // if
	} // if

} // ImageLibGUI::DoTransparencySelect

/*===========================================================================*/

void ImageLibGUI::DisableColorControlWidgets(ColorControlShell *MyShell)
{
RasterAttribute *MyAttr;

if (Active)
	{
	if (! MyShell)
		{
		if (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_COLORCONTROL))
			{
			MyShell = (ColorControlShell *)MyAttr->GetShell();
			} // if
		} // if
	if (MyShell)
		{
		if (MyShell->UseBandAssignment)
			{
			WidgetSetDisabled(IDC_BANDOUT, TRUE);
			WidgetSetDisabled(IDC_BANDIN, TRUE);
			WidgetSetDisabled(IDC_BANDFACTOR, TRUE);
			WidgetSetDisabled(IDC_FORMULA, TRUE);
			WidgetSetDisabled(IDC_BANDRED, FALSE);
			WidgetSetDisabled(IDC_BANDGREEN, FALSE);
			WidgetSetDisabled(IDC_BANDBLUE, FALSE);
			} // if
		else
			{
			WidgetSetDisabled(IDC_BANDRED, TRUE);
			WidgetSetDisabled(IDC_BANDGREEN, TRUE);
			WidgetSetDisabled(IDC_BANDBLUE, TRUE);
			WidgetSetDisabled(IDC_BANDOUT, FALSE);
			WidgetSetDisabled(IDC_BANDIN, FALSE);
			WidgetSetDisabled(IDC_BANDFACTOR, FALSE);
			WidgetSetDisabled(IDC_FORMULA, FALSE);
			} // else
		} // if
	} // if

} // ImageLibGUI::DisableColorControlWidgets

/*===========================================================================*/

void ImageLibGUI::RemoveAttribute(char RemoveAttrType)
{
RasterAttribute *MyAttr;

if (Active)
	{
	if (MyAttr = Active->MatchAttribute(RemoveAttrType))
		{
		if (UserMessageOKCAN(MyAttr->GetName(), "OK to remove entire Image Attribute?"))
			{
			Active->RemoveAttribute(RemoveAttrType, TRUE);
			ConfigureWidgets();
			} // if
		} // if
	} // if

} // ImageLibGUI::RemoveAttribute

/*===========================================================================*/

void ImageLibGUI::RemovePartialSequenceDissolve(void)
{

if (ActiveShell)
	{
	if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
		RemovePartialDissolve();
	else if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
		RemovePartialSequence();
	} // if

} // ImageLibGUI::RemovePartialSequenceDissolve

/*===========================================================================*/

void ImageLibGUI::RemovePartialSequence(void)
{

if (Active && ActiveSequence)
	{
	if (UserMessageOKCAN((char *)ActiveSequence->PAF.GetName(), "OK to remove current Sequence?"))
		{
		if (Active->RemovePartialSequence(ActiveSequence))
			ConfigureWidgets();
		} // if
	} // if

} // ImageLibGUI::RemovePartialSequence

/*===========================================================================*/

void ImageLibGUI::RemovePartialDissolve(void)
{

if (Active && ActiveDissolve)
	{
	if (UserMessageOKCAN(ActiveDissolve->GetTarget() ? ActiveDissolve->GetTarget()->GetUserName(): (char*)"No Target", (char*)"OK to remove current Dissolve?"))
		{
		if (Active->RemovePartialDissolve(ActiveDissolve))
			ConfigureWidgets();
		} // if
	} // if

} // ImageLibGUI::RemovePartialDissolve

/*===========================================================================*/

void ImageLibGUI::ChangeEnabled(void)
{
NotifyTag Changes[2];

if (Active)
	{
	Active->Enabled = (char)(! Active->Enabled);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // ImageLibGUI::ChangeEnabled


void ImageLibGUI::NewSequenceStartFrame(void)
{
double MinStart;

MinStart = ActiveSequence->PrevSeq ? ((SequenceShell *)ActiveSequence->PrevSeq)->EndFrame + 
	((SequenceShell *)ActiveSequence->PrevSeq)->EndSpace: 0;
ActiveSequence->StartSpace = ActiveSequence->StartFrame - MinStart;
NewSequenceEvent();

} // ImageLibGUI::NewSequenceStartFrame

/*===========================================================================*/

void ImageLibGUI::NewSequenceStartSpace(void)
{
double MinStart;

MinStart = ActiveSequence->PrevSeq ? ((SequenceShell *)ActiveSequence->PrevSeq)->EndFrame + 
	((SequenceShell *)ActiveSequence->PrevSeq)->EndSpace: 0;
ActiveSequence->StartFrame = ActiveSequence->StartSpace + MinStart;
NewSequenceEvent();

} // ImageLibGUI::NewSequenceStartSpace

/*===========================================================================*/

void ImageLibGUI::NewSequenceEvent(void)
{
//double MinStart;

if (ActiveSequence)
	{
//	MinStart = ActiveSequence->PrevSeq ? ((SequenceShell *)ActiveSequence->PrevSeq)->EndFrame + 
//		((SequenceShell *)ActiveSequence->PrevSeq)->EndSpace: 0;
//	ActiveSequence->StartSpace = ActiveSequence->StartFrame - MinStart;
	switch (SequenceAdjust)
		{
		case WCS_IMAGELIB_ADJUSTENDFRAME:
			{
			if (ActiveSequence->Speed == 0.0)
				ActiveSequence->EndFrame = ActiveSequence->StartFrame;
			else
				ActiveSequence->EndFrame = ActiveSequence->StartFrame - 1 + ((ActiveSequence->EndImage - ActiveSequence->StartImage + 1) * ActiveSequence->NumLoops) / (ActiveSequence->Speed / 100.0);
			ActiveSequence->Duration = ActiveSequence->EndFrame - ActiveSequence->StartFrame;
			break;
			} // WCS_IMAGELIB_ADJUSTENDFRAME
		case WCS_IMAGELIB_ADJUSTENDIMAGE:
			{
			if (ActiveSequence->NumLoops == 0.0)
				ActiveSequence->EndImage = ActiveSequence->StartImage;
			else
				ActiveSequence->EndImage = ActiveSequence->StartImage - 1 + (long)(((ActiveSequence->EndFrame - ActiveSequence->StartFrame + 1) * (ActiveSequence->Speed / 100.0)) / ActiveSequence->NumLoops);
			break;
			} // WCS_IMAGELIB_ADJUSTENDIMAGE
		case WCS_IMAGELIB_ADJUSTSPEED:
			{
			if (ActiveSequence->EndFrame == ActiveSequence->StartFrame)
				ActiveSequence->Speed = 0.0;
			else
				ActiveSequence->Speed = (((ActiveSequence->EndImage - ActiveSequence->StartImage + 1) * ActiveSequence->NumLoops) / (ActiveSequence->EndFrame - ActiveSequence->StartFrame + 1)) * 100.0;
			break;
			} // WCS_IMAGELIB_ADJUSTSPEED
		case WCS_IMAGELIB_ADJUSTNUMLOOPS:
			{
			if (ActiveSequence->EndImage == ActiveSequence->StartImage)
				ActiveSequence->NumLoops = 0.0;
			else
				ActiveSequence->NumLoops = ((ActiveSequence->EndFrame - ActiveSequence->StartFrame + 1) * (ActiveSequence->Speed / 100.0)) / (ActiveSequence->EndImage - ActiveSequence->StartImage + 1);
			break;
			} // WCS_IMAGELIB_ADJUSTNUMLOOPS
		} // switch
	ActiveSequence->AdjustSubsequent();
	ConfigureFI(NativeWin, IDC_STARTTIME, &ActiveSequence->StartFrame, 1.0, ActiveSequence->PrevSeq ? ((SequenceShell *)ActiveSequence->PrevSeq)->EndFrame + ((SequenceShell *)ActiveSequence->PrevSeq)->EndSpace: 0.0, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_ENDTIME, &ActiveSequence->EndFrame, 1.0, (double)ActiveSequence->StartFrame, 32767.0, FIOFlag_Double, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIRST_IMAGE, &ActiveSequence->StartImage, 1.0, 0.0, (double)ActiveSequence->EndImage, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_LAST_IMAGE, &ActiveSequence->EndImage, 1.0, (double)ActiveSequence->StartImage, 1000000.0, FIOFlag_Long, NULL, NULL);
	WidgetFISync(IDC_STARTSPACE, WP_FISYNC_NONOTIFY);
	WidgetFISync(IDC_SPEED, WP_FISYNC_NONOTIFY);
	WidgetFISync(IDC_NUM_LOOPS, WP_FISYNC_NONOTIFY);
	} // if

} // ImageLibGUI::NewSequenceEvent

/*===========================================================================*/

void ImageLibGUI::NewDissolveStartFrame(void)
{

if (ActiveDissolve)
	{
	ActiveDissolve->EndFrame = ActiveDissolve->StartFrame + ActiveDissolve->Duration;
	ActiveDissolve->AdjustSubsequent();
	ConfigureFI(NativeWin, IDC_ENDFRAME, &ActiveDissolve->EndFrame, 1.0, (double)ActiveDissolve->StartFrame, 32767.0, FIOFlag_Long, NULL, NULL);
	} // if

} // ImageLibGUI::NewDissolveStartFrame

/*===========================================================================*/

void ImageLibGUI::NewDissolveEndFrame(void)
{

if (ActiveDissolve)
	{
	ActiveDissolve->Duration = ActiveDissolve->EndFrame - ActiveDissolve->StartFrame;
	ActiveDissolve->AdjustSubsequent();
	WidgetFISync(IDC_DURATION, WP_FISYNC_NONOTIFY);
	} // if

} // ImageLibGUI::NewDissolveEndFrame

/*===========================================================================*/

void ImageLibGUI::NewDissolveDuration(void)
{

if (ActiveDissolve)
	{
	ActiveDissolve->EndFrame = ActiveDissolve->StartFrame + ActiveDissolve->Duration;
	ActiveDissolve->AdjustSubsequent();
	WidgetFISync(IDC_ENDFRAME, WP_FISYNC_NONOTIFY);
	} // if

} // ImageLibGUI::NewDissolveDuration

/*===========================================================================*/

void ImageLibGUI::Name(void)
{
char NewName[WCS_IMAGE_MAXNAMELENGTH];
NotifyTag Changes[2];

if (Active && WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_IMAGE_MAXNAMELENGTH, NewName);
	Active->SetUserName(NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 


} // ImageLibGUI::Name()

/*===========================================================================*/

void ImageLibGUI::ResetListNames(void)
{
char TestName[WCS_IMAGE_MAXNAMELENGTH + 10];
long Selected, Found = 0;
Raster *Test;

Selected = WidgetLBGetCurSel(IDC_PARLIST);

if((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (Raster *)LB_ERR && Test)
	{
	WidgetLBGetText(IDC_PARLIST, Selected, TestName);
	if (strcmp(Test->GetUserName(), &TestName[2]))
		{
		BuildListEntry(TestName, Test);
		WidgetLBReplace(IDC_PARLIST, Selected, TestName);
		WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
		ListSelectOneItemByNumber(IDC_PARLIST, Selected);
//		WidgetLBSetCurSel(IDC_PARLIST, Selected);
		Found = 1;
		} // if its the active list item
	} // if it could bethe active list item
if (! Found)
	{
	if ((Found = WidgetLBGetCount(IDC_PARLIST)) != LB_ERR)
		{
		for (Selected = 0; Selected < Found; Selected ++)
			{
			if((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (Raster *)LB_ERR && Test)
				{
				WidgetLBGetText(IDC_PARLIST, Selected, TestName);
				if (strcmp(Test->GetUserName(), &TestName[2]))
					{
					BuildListEntry(TestName, Test);
					WidgetLBReplace(IDC_PARLIST, Selected, TestName);
					WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
					} // if
				} // if 
			} // for
		} // if some items in the list
	} // if its not the active list item

} // ImageLibGUI::ResetListNames

/*===========================================================================*/

void ImageLibGUI::ResetListEnabled(void)
{
char TestName[WCS_IMAGE_MAXNAMELENGTH + 10], EnabledState = 1;
long Selected, Count = 0;
Raster *Test;

if (Active)
	{
	EnabledState = Active->Enabled;
	} // if
/*
Selected = WidgetLBGetCurSel(IDC_PARLIST);

if((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (Raster *)LB_ERR && Test)
	{
	WidgetLBGetText(IDC_PARLIST, Selected, TestName);
	if (Test->GetEnabled() && TestName[0] == ' ' || ! Test->GetEnabled() && TestName[0] == '*')
		{
		BuildListEntry(TestName, Test);
		WidgetLBReplace(IDC_PARLIST, Selected, TestName);
		WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
		ListSelectOneItemByNumber(IDC_PARLIST, Selected);
//		WidgetLBSetCurSel(IDC_PARLIST, Selected);
		Found = 1;
		} // if its the active list item
	} // if it could be the active list item
*/
//if (! Found)
//	{
	if ((Count = WidgetLBGetCount(IDC_PARLIST)) != LB_ERR)
		{
		for (Selected = 0; Selected < Count; Selected ++)
			{
			if (WidgetLBGetSelState(IDC_PARLIST, Selected))
				{
				if((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, Selected)) != (Raster *)LB_ERR && Test)
					{
					Test->Enabled = EnabledState;
					BuildListEntry(TestName, Test);
					WidgetLBReplace(IDC_PARLIST, Selected, TestName);
					WidgetLBSetItemData(IDC_PARLIST, Selected, Test);
					WidgetLBSetSelState(IDC_PARLIST, TRUE, Selected);
					} // if
				} // if
			} // for
		} // if some items in the list
//	} // if

} // ImageLibGUI::ResetListEnabled

/*===========================================================================*/

Raster *ImageLibGUI::GetNextSelected(Raster *CurSelection)
{
long NumItems, ListPos = -1;
Raster *Test;

NumItems = WidgetLBGetCount(IDC_PARLIST);

// find the selected item in the list then look for next one selected
if (CurSelection)
	ListPos = FindInList(CurSelection);
CurSelection = NULL;

ListPos ++;

for ( ; ListPos < NumItems; ListPos ++)
	{
	if (WidgetLBGetSelState(IDC_PARLIST, ListPos))
		{
		if ((Test = (Raster *)WidgetLBGetItemData(IDC_PARLIST, ListPos)) && Test != (Raster *)LB_ERR)
			{
			CurSelection = Test;
			break;
			} // if
		} // if
	} // for

return (CurSelection);

} // ImageLibGUI::GetNextSelected

/*===========================================================================*/

void ImageLibGUI::AdjustSequenceDissolvePosition(int Direction)
{
long NewPos = -Direction; // not sure why this needs to be negated, but it seems so in testing
Raster *AffectedRast;

if (Active)
	{
	if (ActiveShell)
		{
		if (AffectedRast = ActiveShell->GetRaster())
			{
			if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_SEQUENCE)
				{
				if (AffectedRast->AdjustSequenceOrder((SequenceShell *)ActiveShell, NewPos))
					{
					ConfigureSequence((SequenceShell *)ActiveShell);
					BuildSequenceList(Active, NULL);
					} // if
				} // if
			else if (ActiveShell->GetType() == WCS_RASTERSHELL_TYPE_DISSOLVE)
				{
				if (AffectedRast->AdjustDissolveOrder((DissolveShell *)ActiveShell, NewPos))
					{
					ConfigureDissolve((DissolveShell *)ActiveShell);
					BuildSequenceList(Active, NULL);
					} // if
				} // else if
			} // if
		} // if
	} // if

} // ImageLibGUI::AdjustSequenceDissolvePosition

/*===========================================================================*/

void ImageLibGUI::EditApplication(void)
{
long Current;
RasterShell *MyShell;

Current = WidgetLBGetCurSel(IDC_EFFECTLIST);
if ((MyShell = (RasterShell *)WidgetLBGetItemData(IDC_EFFECTLIST, Current)) != (RasterShell *)LB_ERR && MyShell)
	{
	MyShell->EditHost();
	} // if

} // ImageLibGUI::EditApplication

/*===========================================================================*/

void ImageLibGUI::ApplyMemoryToAll(void)
{

if (Active)
	{
	HostLib->SetAllMemoryLongevity(Active->GetLongevity());
	} // if

} // ImageLibGUI::ApplyMemoryToAll

/*===========================================================================*/

void ImageLibGUI::ApplyLoadFastToAll(void)
{

if (Active)
	{
	HostLib->SetAllLoadFast(Active->GetLoadFast());
	} // if

} // ImageLibGUI::ApplyLoadFastToAll

/*===========================================================================*/

void ImageLibGUI::BrowseFile(void)
{

if (BrowseLib)
	{
	HostProj->Load(NULL, NULL, NULL, NULL, BrowseLib,
		HostProj->IODetail(WCS_PROJECT_IODETAILFLAG_DESTROY,
		WCS_PROJECT_IODETAILTAG_CHUNKID, "Images",
		WCS_PROJECT_IODETAILTAG_FLAGS, WCS_IMAGES_LOAD_CLEAR,
		WCS_PROJECT_IODETAILTAG_DONE),
		0x0110ffff);
	BuildList(BrowseLib, IDC_BROWSELIST);
	} // if

} // ImageLibGUI::BrowseFile()

/*===========================================================================*/

void ImageLibGUI::SelectAll(void)
{
long Ct, NumItems;
Raster *Test;

if (BrowseLib)
	{
	NumItems = WidgetLBGetCount(IDC_BROWSELIST);
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if((Test = (Raster *)WidgetLBGetItemData(IDC_BROWSELIST, Ct)) != (Raster *)LB_ERR && Test)
			{
			WidgetLBSetSelState(IDC_BROWSELIST, TRUE, Ct);
			} // if
		} // for
	} // if

} // ImageLibGUI::SelectAll

/*===========================================================================*/

void ImageLibGUI::CopyToProj(void)
{
long Ct, Selected, NumItems, ImagesAdded = 0;
Raster *Test;
NotifyTag Changes[2];

if (BrowseLib)
	{
	NumItems = WidgetLBGetCount(IDC_BROWSELIST);
	for (Ct = 0; Ct < NumItems; Ct ++)
		{
		if ((Selected = WidgetLBGetSelState(IDC_BROWSELIST, Ct)) != LB_ERR && Selected)
			{
			if ((Test = (Raster *)WidgetLBGetItemData(IDC_BROWSELIST, Ct)) != (Raster *)LB_ERR && Test)
				{
				if (HostLib->MatchNameMakeRaster(Test))
					ImagesAdded ++;
				} // if its really an image
			} // if an item selected
		} // for
	if (ImagesAdded)
		{
		Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, NULL);
		} // if
	} // if

} // ImageLibGUI::CopyToProj

/*===========================================================================*/

void ImageLibGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
RasterAttribute *MyAttr;
long Current;

if (Active && (MyAttr = Active->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF)) && MyAttr->GetShell())
	{
	Current = WidgetCBGetCurSel(IDC_COORDSDROP);
	if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
		|| (NewObj = (CoordSys *)HostEffectsLib->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
		{
		NewObj->AddRaster(Active);
		} // if
	} // if
#endif // WCS_BUILD_VNS

} // ImageLibGUI::SelectNewCoords

/*===========================================================================*/

unsigned long int ImageLibGUI::EstimateIMMaxCacheBytes(void)
{
unsigned long int CalcSize = 0, MaxSize = 0, ManagedImages = 0, TotalManagedSize = 0;
Raster *ScanRast;
RasterAttribute *MyAttr;
ImageManagerShell *IMS;

for(ScanRast = GlobalApp->AppImages->List; ScanRast; ScanRast = ScanRast->Next)
	{
	if (MyAttr = ScanRast->MatchAttribute(WCS_RASTERSHELL_TYPE_IMAGEMANAGER))
		{
		if(IMS = (ImageManagerShell *)MyAttr->Shell)
			{
			if(ScanRast->ImageManagerEnabled)
				{
				unsigned long int MyTotalSize, MyCacheSize, MyXTiles, MyYTiles, MyXTilesMem, MyYTilesMem;
				ManagedImages++;
				MyXTiles = ScanRast->Cols / max(IMS->GetPreferredTileSizeX(), 1); //lint !e666 !e573 // prevent divide by zero
				MyYTiles = ScanRast->Rows / max(IMS->GetPreferredTileSizeY(), 1); //lint !e666 !e573 // prevent divide by zero
				MyXTilesMem = 2 * MyXTiles * (IMS->GetPreferredTileSizeX() * IMS->GetPreferredTileSizeY() * ScanRast->ByteBands);
				MyYTilesMem = 2 * MyYTiles * (IMS->GetPreferredTileSizeX() * IMS->GetPreferredTileSizeY() * ScanRast->ByteBands);
				MyTotalSize = ScanRast->Rows * ScanRast->Cols * ScanRast->ByteBands;
				TotalManagedSize += MyTotalSize;
				MyCacheSize = max(MyXTilesMem, MyYTilesMem);
				MyCacheSize = min(MyCacheSize, MyTotalSize); // keep my cache size from exceeding my total memory size, silly
				if(MyCacheSize > MaxSize)
					{
					MaxSize = MyCacheSize;
					} // if
				} // if
			} // if
		} // if
	} // if

// MaxSize, plus an additional MaxSize if more than one image, plus 10% of maxsize for each additional image beyond 1
CalcSize = MaxSize + (ManagedImages > 1 ? MaxSize : 0);
if(ManagedImages > 2)
	{
	CalcSize += ((MaxSize / 10) * ManagedImages - 2);
	} // if

return(CalcSize);
} // ImageLibGUI::EstimateIMMaxCacheBytes

/*===========================================================================*/

void ImageLibGUI::ConfigureImageManagerCacheEst(void)
{
char TextStr[256];
unsigned long TempCacheEst;
TempCacheEst = EstimateIMMaxCacheBytes();
sprintf(TextStr, "%1d", max(1, TempCacheEst / 1024000));
WidgetSetText(IDC_SUGGESTEDSIZE, TextStr);
} // ImageLibGUI::ConfigureImageManagerCacheEst

/*===========================================================================*/

void ImageLibGUI::CreateTileableImage(void)
{
#ifdef WCS_IMAGELIB_TILEABLEIMAGE
double Overlap;
int TileResult;
char OverlapStr[128];

if (Active)
	{
	// get overlap and which dimensions to tile
	if (TileResult = UserMessageCustomQuad("Create Tileable Image", "Which directions do you want to be tileable?", "Both", "Cancel", "Horizontal", "Vertical", 0))
		{
		strcpy(OverlapStr, "10");
		if (GetInputString("What percent of the image width and/or height are you willing to lose to create the overlap?", WCS_REQUESTER_POSINTEGERS_ONLY, OverlapStr))
			Overlap = atof(OverlapStr) / 100.0;

		Active->CreateTileableImage(TileResult == 1 || TileResult == 2, TileResult == 1 || TileResult == 3, Overlap);
		} // if
	} // if

#endif // WCS_IMAGELIB_TILEABLEIMAGE
} // ImageLibGUI::CreateTileableImage
