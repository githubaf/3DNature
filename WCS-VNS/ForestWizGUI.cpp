// ForestWizGUI.cpp
// Code for ForestWizGUI
// Built from scratch on 2/4/04 by Gary R. Huber
// Copyright 2004 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ForestWizGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "Raster.h"
#include "Conservatory.h"
#include "ImageLibGUI.h"
#include "resource.h"

extern unsigned short ForestWizPageResourceID[];

NativeGUIWin ForestWizGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ForestWizGUI::Open

/*===========================================================================*/

NativeGUIWin ForestWizGUI::Construct(void)
{
unsigned short PanelCt = 0;

#ifdef WCS_FORESTRY_WIZARD
if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMWIZ, LocalWinSys()->RootWin);

	// these must be in the same order as the defines for the page numbers
	for (PanelCt = 0; PanelCt < WCS_FORESTWIZ_NUMPAGES; PanelCt ++)
		{
		CreateSubWinFromTemplate(ForestWizPageResourceID[PanelCt], 0, PanelCt, false);
		} // for

	if(NativeWin)
		{
		ConfigureWidgets();
		DisplayPage();
		} // if
	} // if
#endif // WCS_FORESTRY_WIZARD
 
return (NativeWin);

} // ForestWizGUI::Construct

/*===========================================================================*/

ForestWizGUI::ForestWizGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Database *DBSource, Project *ProjectSource)
: GUIFenetre('FOWZ', this, "Forestry Wizard")
{
double RangeDefaults[3] = {10000.0, 0.0, 1.0};
static NotifyTag AllEvents[] = {MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								0};

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
EffectsHost = EffectsSource;
DBHost = DBSource;
ImageHost = ImageSource;
ProjectHost = ProjectSource;
ActivePage = &Wizzer.Wizzes[WCS_FORESTWIZ_WIZPAGE_WELCOME];
CMapMatchRGB[0] = CMapMatchRGB[1] = CMapMatchRGB[2] = 0;
CMapMatchName[0] = 0;
CurrentEcoName[0] = 0;
CurrentColor = 0;
CurrentEcoData = 0;
CurrentEcoFolFile = 0;
CurrentClassData = 0;
CurrentClassFolFile = 0;
CurrentClassGroup = 0;
CurrentSpeciesData = 0;

CMapHtADT.SetMetricType(WCS_ANIMDOUBLE_METRIC_HEIGHT);
CMapHtADT.SetRangeDefaults(RangeDefaults);

if (ProjectHost->ProjectLoaded)
	{
	#ifdef WCS_FORESTRY_WIZARD
	if ((GlobalApp->ForestryAuthorized = GlobalApp->Sentinal->CheckAuthFieldForestry() ? 1: 0) && EffectsHost && DBHost && ImageHost && ProjectHost)
		{
		GlobalApp->AppEx->RegisterClient(this, AllEvents);
		ConstructError = 0;
		#ifdef WCS_BUILD_DEMO
		UserMessageOK("Forestry Wizard", "The Forestry Wizard is a feature of the VNS Forestry Edition and is not part of the basic VNS program. It is enabled in this Demo version so you can see what it looks like and try it out. Be sure to specify that you want the Forestry Edition when you purchase VNS.");
		#endif // WCS_BUILD_DEMO
		} // if
	else
	#endif // WCS_FORESTRY_WIZARD
		ConstructError = 1;
	} // if
else
	{
	UserMessageOK("Forestry Wizard", "There is no Project in memory. You must load or create a Project before you can use the Forestry Wizard.");
	ConstructError = 1;
	} // else

} // ForestWizGUI::ForestWizGUI

/*===========================================================================*/

ForestWizGUI::~ForestWizGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ForestWizGUI::~ForestWizGUI()

/*===========================================================================*/

long ForestWizGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_FWZ, 0);

return(0);

} // ForestWizGUI::HandleCloseWin

/*===========================================================================*/

long ForestWizGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch(ButtonID)
	{
	case ID_KEEP:
	case IDCANCEL:
		{
		DoCancel();
		break;
		} // 
	case IDC_NEXT:
		{
		DoNext();
		break;
		} // 
	case IDC_PREV:
		{
		DoPrev();
		break;
		} // 
	case IDC_WIZ_REFRESHLIST:
		{
		BuildCMapImageList();
		break;
		} // 
	case IDC_WIZ_SAVEPROJECT:
		{
		if (ProjectHost->ProjectLoaded)
			{
			ProjectHost->Save(NULL, NULL, DBHost, EffectsHost, ImageHost, NULL, 0xffffffff);
			#ifndef WCS_BUILD_DEMO
			ProjectHost->SavePrefs(AppScope->GetProgDir());
			#endif // WCS_BUILD_DEMO
			} // else
		else
			UserMessageOK("Save Project", "There is no Project to save. You must load or create a Project first.");
		break;
		} // 
	case IDC_WIZ_REFRESHATTRLIST:
	case IDC_WIZ_REFRESHATTRLIST2:
	case IDC_WIZ_REFRESHATTRLIST3:
	case IDC_WIZ_REFRESHATTRLIST4:
	case IDC_WIZ_REFRESHATTRLIST5:
	case IDC_WIZ_REFRESHATTRLIST6:
	case IDC_WIZ_REFRESHATTRLIST7:
	case IDC_WIZ_REFRESHATTRLIST8:
	case IDC_WIZ_REFRESHATTRLIST9:
	case IDC_WIZ_REFRESHATTRLIST10:
	case IDC_WIZ_REFRESHATTRLIST11:
	case IDC_WIZ_REFRESHATTRLIST12:
	case IDC_WIZ_REFRESHATTRLIST13:
	case IDC_WIZ_REFRESHATTRLIST14:
	case IDC_WIZ_REFRESHATTRLIST15:
	case IDC_WIZ_REFRESHATTRLIST16:
	case IDC_WIZ_REFRESHATTRLIST17:
	case IDC_WIZ_REFRESHATTRLIST18:
	case IDC_WIZ_REFRESHATTRLIST19:
	case IDC_WIZ_REFRESHATTRLIST20:
	case IDC_WIZ_REFRESHATTRLIST21:
	case IDC_WIZ_REFRESHATTRLIST22:
	case IDC_WIZ_REFRESHATTRLIST23:
	case IDC_WIZ_REFRESHATTRLIST24:
	case IDC_WIZ_REFRESHATTRLIST25:
	case IDC_WIZ_REFRESHATTRLIST26:
		{
		FillDBAttributeCombos();
		break;
		} // 
	case IDC_WIZ_SHOWIMAGE:
		{
		ShowCMapImage();
		break;
		} // 
	case IDC_WIZ_NEXTCOLOR:
	case IDC_WIZ_NEXTCOLOR2:
		{
		ProcessMatchColor(TRUE, TRUE);
		break;
		} // 
	case IDC_WIZ_NEXTCOLOR3:
		{
		ProcessMatchColor(FALSE, TRUE);
		break;
		} // 
	case IDC_WIZ_PREVCOLOR:
	case IDC_WIZ_PREVCOLOR2:
	case IDC_WIZ_PREVCOLOR3:
		{
		ProcessMatchColor(FALSE, FALSE);
		GoBackAMatchColor();
		break;
		} // 
	case IDC_WIZ_ECONEXTUNIT:
		{
		ProcessEcoUnit();
		break;
		} // 
	case IDC_WIZ_ECONEXTUNIT2:
		{
		ProcessClass();
		break;
		} // 
	case IDC_WIZ_ECOPREVUNIT:
		{
		GoBackAnEcoUnit();
		break;
		} // 
	case IDC_WIZ_ECOPREVUNIT2:
		{
		GoBackAClass();
		break;
		} // 
	case IDC_WIZ_NEXTSPECIES:
		{
		ProcessSpecies();
		break;
		} // 
	case IDC_WIZ_PREVSPECIES:
		{
		GoBackASpecies();
		break;
		} // 
	case IDC_WIZ_NEXTSPECIES2:
		{
		ProcessDBH();
		break;
		} // 
	case IDC_WIZ_PREVSPECIES2:
		{
		GoBackADBH();
		break;
		} // 
	case IDC_WIZ_NEXTSPECIES3:
		{
		ProcessSPDens();
		break;
		} // 
	case IDC_WIZ_PREVSPECIES3:
		{
		GoBackASPDens();
		break;
		} // 
	case IDC_WIZ_ECONEXTIMAGE:
		{
		ProcessEcoImageName(TRUE);
		break;
		} // 
	case IDC_WIZ_ECONEXTIMAGE2:
		{
		ProcessClassImageName(TRUE);
		break;
		} // 
	case IDC_WIZ_ECOPREVIMAGE:
		{
		GoBackAnEcoImageName();
		break;
		} // 
	case IDC_WIZ_ECOPREVIMAGE2:
		{
		GoBackAClassImageName();
		break;
		} // 
	case IDC_WIZ_GRABIMAGES:
		{
		GrabClassImages();
		break;
		} // 
	case IDC_WIZ_GRABIMAGES2:
		{
		GrabEcoImages();
		break;
		} // 
	case IDC_WIZ_OPENIMPORTWIZ:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_IWG, 0);
		break;
		} // 
	case IDC_WIZ_CANCELORDER:
		{
		Wizzer.FinalOrderCancelled = 1;
		DoNext();
		break;
		} // 
	case IDC_WIZ_REVIEWORDER:
		{
		Wizzer.ReviewOrder = 1;
		DoNext();
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // ForestWizGUI::HandleButtonClick

/*===========================================================================*/

long ForestWizGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_IMAGEOBJLIST:
		{
		SelectCMapImage();
		break;
		} // IDC_WIZ_IMAGEOBJLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleListSel

/*===========================================================================*/

long ForestWizGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_CMAPNAME:
	case IDC_WIZ_CMAPNAME2:
	case IDC_WIZ_CMAPNAME3:
		{
		ChangeCMapName(CtrlID);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleStringLoseFocus

/*===========================================================================*/

long ForestWizGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_ECOGROUNDTEXDROP:
		{
		SelectNewEcoGroundTex();
		break;
		}
	case IDC_WIZ_AREAUNITSDROP:
		{
		SetEcoDensityUnits();
		break;
		}
	case IDC_WIZ_ECOGROUNDTEXDROP2:
		{
		SelectNewClassGroundTex();
		break;
		}
	case IDC_WIZ_MULTISIZEATTRDROP:
		{
		SetSpeciesSize();
		break;
		}
	case IDC_WIZ_MULTIDBHATTRDROP:
		{
		SetSpeciesDBH();
		break;
		}
	case IDC_WIZ_ONEDBHATTRDROP:
	case IDC_WIZ_FIRSTDBHATTRDROP:
	case IDC_WIZ_SECONDDBHATTRDROP:
	case IDC_WIZ_ONESPATTRDROP:
	case IDC_WIZ_ONESIZEATTRDROP:
	case IDC_WIZ_ONEDENSATTRDROP:
	case IDC_WIZ_FIRSTSPATTRDROP:
	case IDC_WIZ_SECONDSPATTRDROP:
	case IDC_WIZ_FIRSTSIZEATTRDROP:
	case IDC_WIZ_SECONDSIZEATTRDROP:
	case IDC_WIZ_FIRSTDENSATTRDROP:
	case IDC_WIZ_SECONDDENSATTRDROP:
	case IDC_WIZ_MULTISPDENSATTRDROP:
	case IDC_WIZ_MINHTATTRDROP:
	case IDC_WIZ_MINDBHATTRDROP:
	case IDC_WIZ_MINAGEATTRDROP:
	case IDC_WIZ_MINHTATTRDROP2:
	case IDC_WIZ_MINDBHATTRDROP2:
	case IDC_WIZ_MINAGEATTRDROP2:
	case IDC_WIZ_MINHTATTRDROP3:
	case IDC_WIZ_MINDBHATTRDROP3:
	case IDC_WIZ_MINAGEATTRDROP3:
		{
		SelectDBAttribute(CtrlID);
		break;
		}
	case IDC_WIZ_HTUNITSDROP:
	case IDC_WIZ_HTUNITSDROP2:
	case IDC_WIZ_HTUNITSDROP3:
		{
		SelectFolHeightUnits(CtrlID);
		break;
		}
	case IDC_WIZ_DIAUNITSDROP:
		{
		SelectFolDBHUnits(CtrlID);
		break;
		}
	case IDC_WIZ_DBHUNITSDROP:
		{
		SelectFolDBHThemeUnits(CtrlID);
		break;
		}
	case IDC_WIZ_BAUNITSDROP:
		{
		SelectFolBasalAreaUnits(CtrlID);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleCBChange

/*===========================================================================*/

long ForestWizGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_MATCHRED:
	case IDC_WIZ_MATCHGREEN:
	case IDC_WIZ_MATCHBLUE:
	case IDC_WIZ_MATCHRED2:
	case IDC_WIZ_MATCHGREEN2:
	case IDC_WIZ_MATCHBLUE2:
	case IDC_WIZ_MATCHRED3:
	case IDC_WIZ_MATCHGREEN3:
	case IDC_WIZ_MATCHBLUE3:
		{
		DisableCMapButtons();
		ConfigureCMapColors();
		break;
		} // 
	case IDC_WIZ_ECOFOLHT:
		{
		DisplayEcoData.FolHt[CurrentEcoFolFile] = CMapHtADT.CurValue;
		break;
		}
	case IDC_WIZ_ECOFOLHT2:
		{
		DisplayClassData.Groups[CurrentClassGroup].FolHt[CurrentClassFolFile] = CMapHtADT.CurValue;
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleFIChange

/*===========================================================================*/

long ForestWizGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_CHECKECOHASFOLIAGE:
		{
		UpdateEcoFoliage();
		break;
		} // 
	case IDC_WIZ_CHECKECOHASFOLIAGE2:
		{
		UpdateClassFoliage();
		break;
		} // 
	case IDC_WIZ_CHECKMINHTTHEME:
	case IDC_WIZ_CHECKMINHTTHEME2:
	case IDC_WIZ_CHECKMINHTTHEME3:
	case IDC_WIZ_CHECKMINHTTHEME4:
	case IDC_WIZ_CHECKMINHTTHEME5:
	case IDC_WIZ_CHECKMINHTTHEME6:
	case IDC_WIZ_CHECKMINHTTHEME7:
	case IDC_WIZ_CHECKMINHTTHEME8:
	case IDC_WIZ_CHECKMINHTTHEME9:
		{
		DisableMinHtWidgets();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleSCChange

/*===========================================================================*/

long ForestWizGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_WIZ_ECOIMAGEFILE:
		{
		DisableEcoButtons();
		// add the image now
		ProcessEcoImageName(FALSE);
		break;
		} // 
	case IDC_WIZ_ECOIMAGEFILE2:
		{
		DisableClassButtons();
		// add the image now
		ProcessClassImageName(FALSE);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // ForestWizGUI::HandleDDChange

/*===========================================================================*/

void ForestWizGUI::DisplayPage(void)
{

if (ActivePage)
	{
	WidgetSetText(IDC_IMWIZTEXT, ActivePage->Text);
	WidgetSetText(IDC_NEXT, 
		(ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_COMPLETE ||
		ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR ||
		ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES ||
		ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_PAGEERROR) ? "Close": ActivePage->Next ? "Next -->": "Finish");
	WidgetSetDisabled(IDC_NEXT, ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_CANCEL);
	WidgetSetDisabled(IDC_PREV, ActivePage->Prev && ActivePage->Prev->WizPageID != WCS_FORESTWIZ_WIZPAGE_WELCOME && ActivePage->WizPageID != WCS_FORESTWIZ_WIZPAGE_COMPLETE ? 0: 1);
	WidgetSetDisabled(IDCANCEL, ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_COMPLETE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR);
	SelectPanel(ActivePage->WizPageID);
	Wizzer.CMapColorResponseEnabled = (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG || 
		ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR);

	if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR)
		SetFirstUnidentifiedCMapColor(0);
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC)
		{
		CurrentColor = 0;
		SyncMatchColors();
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO)
		{
		Wizzer.SetupForestWizEcoData();
		CurrentEcoData = 0;
		ConfigureEcoData(FALSE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO)
		{
		CurrentClassData = 0;
		ConfigureClassData(FALSE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD)
		{
		CurrentSpeciesData = 0;
		ConfigureSpeciesData(FALSE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD)
		{
		CurrentSpeciesData = 0;
		ConfigureDBHData(FALSE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD)
		{
		CurrentSpeciesData = 0;
		ConfigureSPDensData(FALSE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SIZEMULT || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_DBHMULT)
		{
		WidgetFISync(IDC_WIZ_SIZEMULT, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_SIZEMULT2, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SIZEAGEEQUIV || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV)
		{
		WidgetFISync(IDC_WIZ_AGEEQUIV, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_DBHEQUIV, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD)
		FillDBAttributeCombos();
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_AVGHEIGHTTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_AVGDBHTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_AVGAGETYPE)
		{
		DisableMinHtWidgets();
		FillDBAttributeCombos();
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME2, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME3, WP_SCSYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_AVGHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_AVGHEIGHT2, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_AVGHEIGHT3, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXHEIGHTTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXDBHTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXAGETYPE)
		{
		DisableMinHtWidgets();
		FillDBAttributeCombos();
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME4, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME5, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME6, WP_SCSYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT2, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT3, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXMINHEIGHTTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXMINDBHTYPE || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MAXMINAGETYPE)
		{
		DisableMinHtWidgets();
		FillDBAttributeCombos();
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME7, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME8, WP_SCSYNC_NONOTIFY);
		WidgetSCSync(IDC_WIZ_CHECKMINHTTHEME9, WP_SCSYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT4, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT5, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_MAXHEIGHT6, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_ONEORTWODBHFIELD || ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MULTIDBHFIELD)
		{
		WidgetFISync(IDC_WIZ_RADIONODBH, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_RADIOONEDBH, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_RADIOTWODBH, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_RADIONODBH2, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_RADIOONEDBH2, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_WIZ_RADIOTWODBH2, WP_FISYNC_NONOTIFY);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV)
		{
		SetDBHUnitLabels();
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_NUMSIZEFIELD
		|| ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD)
		{
		if (Wizzer.DensityMethod == WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE && 
			(Wizzer.NumDensFields == 1 || (Wizzer.NumDensFields > 1 && ! Wizzer.DensAttr[1])))
			{
			if (Wizzer.NumSizeFields > 1)
				{
				Wizzer.NumSizeFields = 1;
				WidgetSRSync(IDC_WIZ_RADIOTWOSIZE, WP_SRSYNC_NONOTIFY);
				WidgetSRSync(IDC_WIZ_RADIOONESIZE, WP_SRSYNC_NONOTIFY);
				WidgetSRSync(IDC_WIZ_RADIOTWOSIZE2, WP_SRSYNC_NONOTIFY);
				WidgetSRSync(IDC_WIZ_RADIOONESIZE2, WP_SRSYNC_NONOTIFY);
				} // if
			WidgetSetDisabled(IDC_WIZ_RADIOTWOSIZE, TRUE);
			WidgetSetDisabled(IDC_WIZ_RADIOTWOSIZE2, TRUE);
			} // if
		else
			{
			WidgetSetDisabled(IDC_WIZ_RADIOTWOSIZE, FALSE);
			WidgetSetDisabled(IDC_WIZ_RADIOTWOSIZE2, FALSE);
			} // else
		} // else if
	// WCS_FORESTRYWIZARD_ADDPAGE - if there are specific items that need to be reconfigured or disabled do it here
	} // if

} // ForestWizGUI::DisplayPage

/*===========================================================================*/

void ForestWizGUI::SelectPanel(unsigned short PanelID)
{

ShowPanel(0, PanelID);

} // ForestWizGUI::SelectPanel

/*===========================================================================*/

void ForestWizGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Changed, Interested[10];

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

if (Wizzer.CMapColorResponseEnabled)
	{
	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
	Interested[1] = NULL;
	if ((Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0)) && Activity->ChangeNotify->NotifyData)
		{
		if (((DiagnosticData *)Activity->ChangeNotify->NotifyData)->ValueValid[WCS_DIAGNOSTIC_RGB])
			RespondCmapColorNotify((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	} // if

} // ForestWizGUI::HandleNotifyEvent

/*===========================================================================*/

void ForestWizGUI::ConfigureWidgets(void)
{

FillEcoGroundTexCombo();
FillEcoUnitsCombo();
SetDBHUnitLabels();

//WCS_FORESTWIZ_WIZPAGE_WELCOME,
//WCS_FORESTWIZ_WIZPAGE_CANCEL,
//WCS_FORESTWIZ_WIZPAGE_CONFIRM,
//WCS_FORESTWIZ_WIZPAGE_COMPLETE,
//WCS_FORESTWIZ_WIZPAGE_COMPLETEERROR,
ConfigureSR(NativeWin, IDC_WIZ_RADIOREVIEWORDER, IDC_WIZ_RADIOREVIEWORDER, &Wizzer.ReviewOrder, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOREVIEWORDER, IDC_WIZ_RADIOCANCELORDER, &Wizzer.ReviewOrder, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_CMAPORVEC
ConfigureSR(NativeWin, IDC_WIZ_RADIOCMAP, IDC_WIZ_RADIOCMAP, &Wizzer.CMapOrVec, SRFlag_Char, WCS_FORESTWIZ_CMAPORVEC_CMAP, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOCMAP, IDC_WIZ_RADIOVEC, &Wizzer.CMapOrVec, SRFlag_Char, WCS_FORESTWIZ_CMAPORVEC_POLYGONS, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOCMAP, IDC_WIZ_RADIOPOINT, &Wizzer.CMapOrVec, SRFlag_Char, WCS_FORESTWIZ_CMAPORVEC_POINTDATA, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_IMGLOADED
ConfigureSR(NativeWin, IDC_WIZ_RADIOIMAGELOADED, IDC_WIZ_RADIOIMAGELOADED, &Wizzer.ImageLoaded, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOIMAGELOADED, IDC_WIZ_RADIOIMAGENOTLOADED, &Wizzer.ImageLoaded, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SELIMG,
BuildCMapImageList();
//WCS_FORESTWIZ_WIZPAGE_FINDIMG
ConfigureDD(NativeWin, IDC_WIZ_CMAPIMAGEFILE, Wizzer.CMapImage.Path, WCS_PATHANDFILE_PATH_LEN_MINUSONE, Wizzer.CMapImage.Name, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
//WCS_FORESTWIZ_WIZPAGE_CLICKORNUM
ConfigureSR(NativeWin, IDC_WIZ_RADIOCLICKIMG, IDC_WIZ_RADIOCLICKIMG, &Wizzer.ClickOrNumeric, SRFlag_Char, WCS_FORESTWIZ_CLICKORNUMERIC_CLICK, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOCLICKIMG, IDC_WIZ_RADIONUMERIC, &Wizzer.ClickOrNumeric, SRFlag_Char, WCS_FORESTWIZ_CLICKORNUMERIC_NUMERIC, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_CMAPCLICKIMG
WidgetSetText(IDC_WIZ_CMAPNAME, CMapMatchName);
WidgetSetModified(IDC_WIZ_CMAPNAME, FALSE);
ConfigureFI(NativeWin, IDC_WIZ_MATCHRED, &CMapMatchRGB[0], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHGREEN, &CMapMatchRGB[1], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHBLUE, &CMapMatchRGB[2], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_CMAPNUMERIC
WidgetSetText(IDC_WIZ_CMAPNAME2, CMapMatchName);
WidgetSetModified(IDC_WIZ_CMAPNAME2, FALSE);
ConfigureFI(NativeWin, IDC_WIZ_MATCHRED2, &CMapMatchRGB[0], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHGREEN2, &CMapMatchRGB[1], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHBLUE2, &CMapMatchRGB[2], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_ANALYZEIMG
ConfigureSC(NativeWin, IDC_WIZ_CHECKANALYZEIMG, &Wizzer.AnalyzeImage, SCFlag_Char, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_UNIDENTCOLOR
ConfigureSR(NativeWin, IDC_WIZ_RADIOCOLORRANGES, IDC_WIZ_RADIOCOLORRANGES, &Wizzer.CMapColorRange, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOCOLORRANGES, IDC_WIZ_RADIONOCOLORRANGES, &Wizzer.CMapColorRange, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_NORANGES,
//WCS_FORESTWIZ_WIZPAGE_UNMATCHEDCOLOR,
WidgetSetText(IDC_WIZ_CMAPNAME3, CMapMatchName);
WidgetSetModified(IDC_WIZ_CMAPNAME3, FALSE);
ConfigureFI(NativeWin, IDC_WIZ_MATCHRED3, &CMapMatchRGB[0], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHGREEN3, &CMapMatchRGB[1], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MATCHBLUE3, &CMapMatchRGB[2], 1.0, 0.0, 255.0, FIOFlag_Char | FIOFlag_Unsigned, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_NOCMAPMATCHES,
//WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO,
ConfigureEcoData(TRUE);
//WCS_FORESTWIZ_WIZPAGE_UNITFOLIAGE1,
//WCS_FORESTWIZ_WIZPAGE_VECLOADED,
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECLOADED, IDC_WIZ_RADIOVECLOADED, &Wizzer.VectorsLoaded, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECLOADED, IDC_WIZ_RADIOVECNOTLOADED, &Wizzer.VectorsLoaded, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_LOADVEC,
//WCS_FORESTWIZ_WIZPAGE_SPSCENARIO,
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECONESP, IDC_WIZ_RADIOVECONESP, &Wizzer.SpeciesScenario, SRFlag_Char, WCS_FORESTWIZ_SPSCENARIO_ONESPECIESFIELD, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECONESP, IDC_WIZ_RADIOVECTWOSP, &Wizzer.SpeciesScenario, SRFlag_Char, WCS_FORESTWIZ_SPSCENARIO_TWOSPECIESFIELD, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECONESP, IDC_WIZ_RADIOVECMULTI, &Wizzer.SpeciesScenario, SRFlag_Char, WCS_FORESTWIZ_SPSCENARIO_MULTISPECIESFIELD, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOVECONESP, IDC_WIZ_RADIOVECFARSITE, &Wizzer.SpeciesScenario, SRFlag_Char, WCS_FORESTWIZ_SPSCENARIO_FARSITE, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG,
ConfigureSR(NativeWin, IDC_WIZ_RADIOVEGEXISTS, IDC_WIZ_RADIOVEGEXISTS, &Wizzer.VegetationExists, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOVEGEXISTS, IDC_WIZ_RADIONOVEGEXISTS, &Wizzer.VegetationExists, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_HTDENSATTRIBS,
ConfigureSR(NativeWin, IDC_WIZ_RADIOHDATTRIBEXIST, IDC_WIZ_RADIOHDATTRIBEXIST, &Wizzer.HDAttribsExist, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOHDATTRIBEXIST, IDC_WIZ_RADIONOHDATTRIBEXIST, &Wizzer.HDAttribsExist, SRFlag_Char, 0, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SELONESPFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELONESIZEFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELONEDENSFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELFIRSTSPFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELSECONDSPFIELD,
//WCS_FORESTWIZ_WIZPAGE_NUMSIZEFIELD,
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE, IDC_WIZ_RADIONOSIZE, &Wizzer.NumSizeFields, SRFlag_Long, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE, IDC_WIZ_RADIOONESIZE, &Wizzer.NumSizeFields, SRFlag_Long, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE, IDC_WIZ_RADIOTWOSIZE, &Wizzer.NumSizeFields, SRFlag_Long, 2, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_NUMDENSFIELD,
ConfigureSR(NativeWin, IDC_WIZ_RADIONODENS, IDC_WIZ_RADIONODENS, &Wizzer.NumDensFields, SRFlag_Long, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODENS, IDC_WIZ_RADIOONEDENS, &Wizzer.NumDensFields, SRFlag_Long, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODENS, IDC_WIZ_RADIOTWODENS, &Wizzer.NumDensFields, SRFlag_Long, 2, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SELFIRSTSIZEFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELSECONDSIZEFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELFIRSTDENSFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELSECONDDENSFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELMULTIDENSFIELD,
//WCS_FORESTWIZ_WIZPAGE_MULTISIZEFIELD,
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE2, IDC_WIZ_RADIONOSIZE2, &Wizzer.NumSizeFields, SRFlag_Long, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE2, IDC_WIZ_RADIOONESIZE2, &Wizzer.NumSizeFields, SRFlag_Long, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONOSIZE2, IDC_WIZ_RADIOTWOSIZE2, &Wizzer.NumSizeFields, SRFlag_Long, 2, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD,
//WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO,
ConfigureClassData(TRUE);
//WCS_FORESTWIZ_WIZPAGE_SIZEHTDBHAGE,
ConfigureSR(NativeWin, IDC_WIZ_RADIOHEIGHT, IDC_WIZ_RADIOHEIGHT, &Wizzer.SizeMeasurement, SRFlag_Char, WCS_FORESTWIZ_SIZEMEASUREMENT_HEIGHT, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOHEIGHT, IDC_WIZ_RADIODIAMETER, &Wizzer.SizeMeasurement, SRFlag_Char, WCS_FORESTWIZ_SIZEMEASUREMENT_DIAMETER, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOHEIGHT, IDC_WIZ_RADIOAGE, &Wizzer.SizeMeasurement, SRFlag_Char, WCS_FORESTWIZ_SIZEMEASUREMENT_AGE, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SIZEUNITS,
FillFolHeightCombos();
//WCS_FORESTWIZ_WIZPAGE_SIZEAGEEQUIV,
ConfigureFI(NativeWin, IDC_WIZ_AGEEQUIV, &Wizzer.AgeEquiv[0], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AGEEQUIV2, &Wizzer.AgeEquiv[1], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AGEEQUIV3, &Wizzer.AgeEquiv[2], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AGEEQUIV4, &Wizzer.AgeEquiv[3], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_SIZEDBHEQUIV,
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIV, &Wizzer.DBHEquiv[0], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIV2, &Wizzer.DBHEquiv[1], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIV3, &Wizzer.DBHEquiv[2], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIV4, &Wizzer.DBHEquiv[3], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIVUNITS, &Wizzer.DBHEquivUnits[0], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIVUNITS2, &Wizzer.DBHEquivUnits[1], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIVUNITS3, &Wizzer.DBHEquivUnits[2], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_DBHEQUIVUNITS4, &Wizzer.DBHEquivUnits[3], 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_SIZEMULT,
ConfigureFI(NativeWin, IDC_WIZ_SIZEMULT, &Wizzer.SizeThemeMult, 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_SIZEMULT2, &Wizzer.SizeThemeMult, 1.0, 0.0, 1000000.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_HEIGHTTYPE,
ConfigureSR(NativeWin, IDC_WIZ_RADIOAVGHEIGHT, IDC_WIZ_RADIOAVGHEIGHT, &Wizzer.HeightType, SRFlag_Char, WCS_FORESTWIZ_HEIGHTTYPE_AVERAGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOAVGHEIGHT, IDC_WIZ_RADIOMAXHEIGHT, &Wizzer.HeightType, SRFlag_Char, WCS_FORESTWIZ_HEIGHTTYPE_MAXIMUM, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOAVGHEIGHT, IDC_WIZ_RADIOMAXMINHEIGHT, &Wizzer.HeightType, SRFlag_Char, WCS_FORESTWIZ_HEIGHTTYPE_MAXMIN, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_AVGHEIGHTTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AVGHEIGHT, &Wizzer.AvgHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_AVGDBHTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME2, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AVGHEIGHT2, &Wizzer.AvgHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_AVGAGETYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME3, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_AVGHEIGHT3, &Wizzer.AvgHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXHEIGHTTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME4, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT, &Wizzer.MinHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXDBHTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME5, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT2, &Wizzer.MinHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXAGETYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME6, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT3, &Wizzer.MinHeightRange, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXMINHEIGHTTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME7, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT4, &Wizzer.MinHeightAbs, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXMINDBHTYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME8, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT5, &Wizzer.MinHeightAbs, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_MAXMINAGETYPE,
ConfigureSC(NativeWin, IDC_WIZ_CHECKMINHTTHEME9, &Wizzer.MinSizeThemePresent, SCFlag_Char, NULL, 0);
ConfigureFI(NativeWin, IDC_WIZ_MAXHEIGHT6, &Wizzer.MinHeightAbs, 1.0, 0.0, 100.0, FIOFlag_Double, NULL, 0);
//WCS_FORESTWIZ_WIZPAGE_DENSITYMETHOD,
ConfigureSR(NativeWin, IDC_WIZ_RADIOSTEMS, IDC_WIZ_RADIOSTEMS, &Wizzer.DensityMethod, SRFlag_Char, WCS_FORESTWIZ_DENSITYMETHOD_STEMS, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOSTEMS, IDC_WIZ_RADIOBASALAREA, &Wizzer.DensityMethod, SRFlag_Char, WCS_FORESTWIZ_DENSITYMETHOD_BASALAREA, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOSTEMS, IDC_WIZ_RADIOCLOSURE, &Wizzer.DensityMethod, SRFlag_Char, WCS_FORESTWIZ_DENSITYMETHOD_CLOSURE, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_CLOSUREPRECISION,
ConfigureSR(NativeWin, IDC_WIZ_RADIOCLOSUREPERCENT, IDC_WIZ_RADIOCLOSUREPERCENT, &Wizzer.ClosurePrecision, SRFlag_Char, WCS_FORESTWIZ_PRECISION_PERCENTAGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOCLOSUREPERCENT, IDC_WIZ_RADIOCLOSUREDECIMAL, &Wizzer.ClosurePrecision, SRFlag_Char, WCS_FORESTWIZ_PRECISION_DECIMAL, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_MULTIDBHFIELD,
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH, IDC_WIZ_RADIONODBH, &Wizzer.NumDBHFields, SRFlag_Long, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH, IDC_WIZ_RADIOONEDBH, &Wizzer.NumDBHFields, SRFlag_Long, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH, IDC_WIZ_RADIOTWODBH, &Wizzer.NumDBHFields, SRFlag_Long, 2, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_ONEORTWODBHFIELD,
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH2, IDC_WIZ_RADIONODBH2, &Wizzer.NumDBHFields, SRFlag_Long, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH2, IDC_WIZ_RADIOONEDBH2, &Wizzer.NumDBHFields, SRFlag_Long, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIONODBH2, IDC_WIZ_RADIOTWODBH2, &Wizzer.NumDBHFields, SRFlag_Long, 2, NULL, NULL);
//WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELMULTISPFIELD,
//WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD,
//WCS_FORESTWIZ_WIZPAGE_SPDENSPRECISION,
ConfigureSR(NativeWin, IDC_WIZ_RADIOSPDENSPERCENT, IDC_WIZ_RADIOSPDENSPERCENT, &Wizzer.SPDensPrecision, SRFlag_Char, WCS_FORESTWIZ_PRECISION_PERCENTAGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_WIZ_RADIOSPDENSPERCENT, IDC_WIZ_RADIOSPDENSDECIMAL, &Wizzer.SPDensPrecision, SRFlag_Char, WCS_FORESTWIZ_PRECISION_DECIMAL, NULL, NULL);

// WCS_FORESTRYWIZARD_ADDPAGE - configure the variables on the page

DisableCMapButtons();
ConfigureCMapColors();

} // ForestWizGUI::ConfigureWidgets

/*===========================================================================*/

void ForestWizGUI::DoNext(void)
{

if (ActivePage)
	{
	// one of the pages for color map match specification. Need to capture last entry
	if (Wizzer.CMapColorResponseEnabled)
		ProcessMatchColor(TRUE, FALSE);
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO)	// eco info
		{
		ProcessEcoUnit();
		ConfigureEcoData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO)	// poly info
		{
		ProcessClass();
		ConfigureClassData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD)	// poly multi species size
		{
		ProcessSpecies();
		ConfigureSpeciesData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD)	// poly multi species DBH
		{
		ProcessDBH();
		ConfigureDBHData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD)	// poly multi species dens
		{
		ProcessSPDens();
		ConfigureSPDensData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SPSCENARIO)	// Vector scenario
		FillDBAttributeCombos();
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_ISTHEREVEG)	// Vegetation
		FillDBAttributeCombos();
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTIDENSFIELD)	// multi dens attribs
		SelectMultiDensAttribs(IDC_WIZ_MULTIDENSLIST);
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISPFIELD)	// multi species attribs
		SelectMultiDensAttribs(IDC_WIZ_MULTISPLIST);
	if (ActivePage = Wizzer.ProcessPage(ActivePage, ProjectHost))
		DisplayPage();
	else
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_FWZ, 0);
	} // if

} // ForestWizGUI::DoNext

/*===========================================================================*/

void ForestWizGUI::DoPrev(void)
{

if (ActivePage->Prev)
	{
	if (Wizzer.CMapColorResponseEnabled)	// color map match specification
		ProcessMatchColor(TRUE, FALSE);
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_UNITBASICINFO)	// eco info
		{
		ProcessEcoUnit();
		ConfigureEcoData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_POLYBASICINFO)	// poly info
		{
		ProcessClass();
		ConfigureClassData(TRUE);	// configure to NULL
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISIZEFIELD)	// poly multi species size
		{
		ProcessSpecies();
		ConfigureSpeciesData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTIDBHFIELD)	// poly multi species diameter
		{
		ProcessDBH();
		ConfigureDBHData(TRUE);
		} // else if
	else if (ActivePage->WizPageID == WCS_FORESTWIZ_WIZPAGE_SELMULTISPDENSFIELD)	// poly multi species diameter
		{
		ProcessSPDens();
		ConfigureSPDensData(TRUE);
		} // else if
	Wizzer.CancelOrder = Wizzer.FinalOrderCancelled = 0;
	ActivePage = ActivePage->Prev;
	ActivePage->Revert();
	DisplayPage();
	} // if

} // ForestWizGUI::DoPrev

/*===========================================================================*/

void ForestWizGUI::DoCancel(void)
{

/***
if (Wizzer.CancelOrder)
	Wizzer.FinalOrderCancelled = 1;
Wizzer.CancelOrder = 1;
if (ActivePage = Wizzer.ProcessPage(ActivePage, ProjectHost))
	DisplayPage();
else
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
		WCS_TOOLBAR_ITEM_FWZ, 0);
***/

if (UserMessageYN("Forestry Wizard", "Do you really wish to cancel?"))
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD, WCS_TOOLBAR_ITEM_FWZ, 0);

} // ForestWizGUI::DoCancel

/*===========================================================================*/

void ForestWizGUI::BuildCMapImageList(void)
{
Raster *CurRast;
long Place;
char ListName[WCS_IMAGE_MAXNAMELENGTH + 2];

WidgetLBClear(IDC_WIZ_IMAGEOBJLIST);

CurRast = ImageHost->List;
while (CurRast)
	{
	if (CurRast->MatchAttribute(WCS_RASTERSHELL_TYPE_GEOREF))	// georeferenced
		{
		BuildImageListEntry(ListName, CurRast);
		Place = WidgetLBInsert(IDC_WIZ_IMAGEOBJLIST, -1, ListName);
		WidgetLBSetItemData(IDC_WIZ_IMAGEOBJLIST, Place, CurRast);
		if (Wizzer.CMapRast == CurRast)
			WidgetLBSetCurSel(IDC_WIZ_IMAGEOBJLIST, Place);
		} // if
	CurRast = CurRast->Next;
	} // while

} // ForestWizGUI::BuildCMapImageList

/*===========================================================================*/

void ForestWizGUI::BuildImageListEntry(char *ListName, Raster *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->GetUserName());

} // ForestWizGUI::BuildListEntry()

/*===========================================================================*/

void ForestWizGUI::SelectCMapImage(void)
{
Raster *NewActive;
long Current;

if ((Current = WidgetLBGetCurSel(IDC_WIZ_IMAGEOBJLIST)) != LB_ERR)
	{
	if ((NewActive = (Raster *)WidgetLBGetItemData(IDC_WIZ_IMAGEOBJLIST, Current)) != Wizzer.CMapRast && NewActive != (Raster *)LB_ERR)
		{
		Wizzer.CMapRast = NewActive;
		} // if
	} // if

} // ForestWizGUI::SelectCMapImage

/*===========================================================================*/

void ForestWizGUI::ShowCMapImage(void)
{
int ShowAnyway = 1;

if (Wizzer.CMapRast)
	{
	if (Wizzer.CMapRast->Rows > 3000 || Wizzer.CMapRast->Cols > 3000)
		ShowAnyway = UserMessageYN((char *)Wizzer.CMapRast->GetName(), "This image is larger than 3000 pixels in at least one dimension. It may be slow to display and will need to be extensively downsampled to fit on screen. It may be difficult to pick colors from due to downsampling. Do you wish to display the image now anyway?");
	if (ShowAnyway)
		Wizzer.CMapRast->OpenPreview(FALSE);
	} // if

} // ForestWizGUI::ShowCMapImage

/*===========================================================================*/

void ForestWizGUI::SyncMatchColors(void)
{

if (CurrentColor < WCS_FORESTWIZ_MAXNUMUNITS)
	{
	Wizzer.CMapFetchColor(CurrentColor, CMapMatchName, CMapMatchRGB);
	SyncCMapName();
	SyncCMapColors();
	DisableCMapButtons();
	} // if

} // ForestWizGUI::SyncMatchColors

/*===========================================================================*/

void ForestWizGUI::ProcessMatchColor(int ClearColors, int Advance)
{

if (CMapMatchRGB[0] || CMapMatchRGB[1] || CMapMatchRGB[2])
	{
	if (CMapMatchName[0])
		{
		if (Wizzer.CMapAddColor(CurrentColor, CMapMatchName, CMapMatchRGB))
			{
			// test for validity since if the name was a duplicate the color will be applied to the original leaving the current slot empty.
			if (Wizzer.ValidateCMapUnit(CurrentColor) && Advance)
				CurrentColor ++;
			if (CurrentColor < WCS_FORESTWIZ_MAXNUMUNITS)
				{
				Wizzer.CMapFetchColor(CurrentColor, CMapMatchName, CMapMatchRGB);
				if (ClearColors && ! CMapMatchName[0])
					CMapMatchRGB[0] = CMapMatchRGB[1] = CMapMatchRGB[2] = 0;
				SyncCMapName();
				SyncCMapColors();
				} // if
			else
				{
				CurrentColor --;
				UserMessageOK("Forestry Wizard", "The capacity of this Wizard is only 1000 cover units. Additional matching units can be added in the Color Map Editor.");
				} // else
			} // if
		else
			{
			UserMessageOK("Forestry Wizard", "That color has already been used. The colors will be cleared. Please try again.");
			CMapMatchRGB[0] = CMapMatchRGB[1] = CMapMatchRGB[2] = 0;
			SyncCMapColors();
			} // else
		} // if
	else if (! ClearColors && Advance)
		{	// can only be reached from IDC_WIZ_NEXTCOLOR3
		CurrentColor ++;
		if (CurrentColor < WCS_FORESTWIZ_MAXNUMUNITS)
			{
			Wizzer.CMapFetchColor(CurrentColor, CMapMatchName, CMapMatchRGB);
			SyncCMapName();
			SyncCMapColors();
			} // if
		else
			{
			CurrentColor --;
			UserMessageOK("Forestry Wizard", "The capacity of this Wizard is only 1000 cover units. Additional matching units can be added in the Color Map Editor.");
			} // else
		} // else
	} // if

DisableCMapButtons();

} // ForestWizGUI::ProcessMatchColor

/*===========================================================================*/

void ForestWizGUI::GoBackAMatchColor(void)
{

if (CurrentColor > 0)
	{
	CurrentColor --;
	Wizzer.CMapFetchColor(CurrentColor, CMapMatchName, CMapMatchRGB);
	SyncCMapName();
	SyncCMapColors();
	DisableCMapButtons();
	} // if

} // ForestWizGUI::GoBackAMatchColor

/*===========================================================================*/

void ForestWizGUI::ChangeCMapName(unsigned short CtrlID)
{

if (WidgetGetModified(CtrlID))
	{
	WidgetGetText(CtrlID, WCS_FORESTWIZ_MAXUNITNAMELEN, CMapMatchName);
	WidgetSetModified(IDC_NAME, FALSE);
	DisableCMapButtons();
	} // if

} // ForestWizGUI::ChangeCMapName

/*===========================================================================*/

void ForestWizGUI::RespondCmapColorNotify(DiagnosticData *Data)
{

if (Data->DataRGB[0] || Data->DataRGB[1] || Data->DataRGB[2])
	{
	CMapMatchRGB[0] = Data->DataRGB[0];
	CMapMatchRGB[1] = Data->DataRGB[1];
	CMapMatchRGB[2] = Data->DataRGB[2];
	SyncCMapColors();
	DisableCMapButtons();
	} // if

} // ForestWizGUI::RespondCmapColorNotify

/*===========================================================================*/

void ForestWizGUI::SetFirstUnidentifiedCMapColor(long StartCt)
{
long CurColor, Found = 0;
char UnmStr[128];

for (CurColor = StartCt; CurColor < WCS_FORESTWIZ_MAXNUMUNITS; CurColor ++)
	{
	if (! Wizzer.ValidateCMapUnit(CurColor))
		{
		Wizzer.CMapFetchColor(CurColor, CMapMatchName, CMapMatchRGB);
		if (! CMapMatchName[0] && (CMapMatchRGB[0] || CMapMatchRGB[1] || CMapMatchRGB[2]))
			{
			sprintf(UnmStr, "There are %d unmatched colors - Give them names!", Wizzer.CountUnmatchedColors());
			WidgetSetText(IDC_WIZ_UNMATCHEDCOLORTXT, UnmStr);
			Found = 1;
			break;
			} // if
		} // if
	} // for

if (! Found)
	{
	CMapMatchName[0] = CMapMatchRGB[0] = CMapMatchRGB[1] = CMapMatchRGB[2] = 0;
	WidgetSetText(IDC_WIZ_UNMATCHEDCOLORTXT, "There are no unmatched colors.");
	} // if
else
	CurrentColor = CurColor;

SyncCMapName();
SyncCMapColors();
DisableCMapButtons();

} // ForestWizGUI::SetFirstUnidentifiedCMapColor

/*===========================================================================*/

void ForestWizGUI::SyncCMapColors(void)
{

WidgetFISync(IDC_WIZ_MATCHRED, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHRED2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHRED3, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHGREEN, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHGREEN2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHGREEN3, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHBLUE, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHBLUE2, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_WIZ_MATCHBLUE3, WP_FISYNC_NONOTIFY);
ConfigureCMapColors();

} // ForestWizGUI::SyncCMapColors

/*===========================================================================*/

void ForestWizGUI::SyncCMapName(void)
{

WidgetSetText(IDC_WIZ_CMAPNAME, CMapMatchName);
WidgetSetText(IDC_WIZ_CMAPNAME2, CMapMatchName);
WidgetSetText(IDC_WIZ_CMAPNAME3, CMapMatchName);
WidgetSetModified(IDC_WIZ_CMAPNAME, FALSE);
WidgetSetModified(IDC_WIZ_CMAPNAME2, FALSE);
WidgetSetModified(IDC_WIZ_CMAPNAME3, FALSE);

} // ForestWizGUI::SyncCMapName

/*===========================================================================*/

void ForestWizGUI::DisableCMapButtons(void)
{

WidgetSetDisabled(IDC_WIZ_NEXTCOLOR, ! (CMapMatchName[0] && (CMapMatchRGB[0] || CMapMatchRGB[1] || CMapMatchRGB[2])));
WidgetSetDisabled(IDC_WIZ_NEXTCOLOR2, ! (CMapMatchName[0] && (CMapMatchRGB[0] || CMapMatchRGB[1] || CMapMatchRGB[2])));
WidgetSetDisabled(IDC_WIZ_NEXTCOLOR3, ! ((CMapMatchRGB[0] || CMapMatchRGB[1] || CMapMatchRGB[2])));
WidgetSetDisabled(IDC_WIZ_PREVCOLOR, CurrentColor == 0);
WidgetSetDisabled(IDC_WIZ_PREVCOLOR2, CurrentColor == 0);
WidgetSetDisabled(IDC_WIZ_PREVCOLOR3, CurrentColor == 0);

} // ForestWizGUI::DisableCMapButtons

/*===========================================================================*/

void ForestWizGUI::ConfigureCMapColors(void)
{

SetColorPot(0, CMapMatchRGB[0], CMapMatchRGB[1], CMapMatchRGB[2], 1);
ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);
SetColorPot(1, CMapMatchRGB[0], CMapMatchRGB[1], CMapMatchRGB[2], 1);
ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);
SetColorPot(2, CMapMatchRGB[0], CMapMatchRGB[1], CMapMatchRGB[2], 1);
ConfigureCB(NativeWin, ID_COLORPOT3, 100, CBFlag_CustomColor, 2);

} // ForestWizGUI::ConfigureCMapColors

/*===========================================================================*/

void ForestWizGUI::FillEcoGroundTexCombo(void)
{
long Ct;
char *GroundTexNames[WCS_FORESTWIZECODATA_NUMGROUNDTEX] = 
	{
	"Random Color",
	"Plain Gray",
	"Plain Tan",
	"Plain Green",
	"Plain Red",
	"Plain Yellow",
	"Plain Blue",
	"Water",
	"Earth",
	"Rock",
	"Boulders",
	"Gravel",
	"Dry Grass",
	"Lawn Grass",
	"Deciduous Forest",
	"Conifer Forest"
	};

WidgetCBClear(IDC_WIZ_ECOGROUNDTEXDROP);
WidgetCBClear(IDC_WIZ_ECOGROUNDTEXDROP2);

for (Ct = 0; Ct < WCS_FORESTWIZECODATA_NUMGROUNDTEX; Ct ++)
	{
	WidgetCBAddEnd(IDC_WIZ_ECOGROUNDTEXDROP, GroundTexNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_ECOGROUNDTEXDROP2, GroundTexNames[Ct]);
	} // for

} // ForestWizGUI::FillEcoGroundTexCombo

/*===========================================================================*/

void ForestWizGUI::FillEcoUnitsCombo(void)
{
char *Units[] = {"Hectare", "Acre", "Square Meter", "Square Foot"};
long Ct;

for (Ct = 0; Ct < 4; Ct ++)
	{
	WidgetCBInsert(IDC_WIZ_AREAUNITSDROP, -1, Units[Ct]);
	} // for
WidgetCBSetCurSel(IDC_WIZ_AREAUNITSDROP, Wizzer.EcoAreaUnits);

} // ForestWizGUI::FillEcoUnitsCombo

/*===========================================================================*/

void ForestWizGUI::ConfigureEcoData(int ConfigNULL)
{
char EcoTxt[64];

if (CurrentEcoData >= Wizzer.NumEcoUnits)
	CurrentEcoData = 0;
CurrentEcoFolFile = 0;

sprintf(EcoTxt, "Unit %d of %d", CurrentEcoData + 1, Wizzer.NumEcoUnits);
WidgetSetText(IDC_ECOTXT, EcoTxt);

if (! ConfigNULL && Wizzer.FetchEcoData(CurrentEcoData, &DisplayEcoData))
	{
	while (DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0] && DisplayEcoData.FolNames[CurrentEcoFolFile + 1].GetName()[0])
		CurrentEcoFolFile ++;
	Wizzer.FetchEcoName(CurrentEcoData, CurrentEcoName);
	WidgetSetText(IDC_WIZ_ECONAME, CurrentEcoName);
	WidgetCBSetCurSel(IDC_WIZ_ECOGROUNDTEXDROP, DisplayEcoData.GroundTexture);
	ConfigureSC(NativeWin, IDC_WIZ_CHECKECOHASFOLIAGE, &DisplayEcoData.HasFoliage, SCFlag_Char, NULL, 0);
	UpdateEcoFoliage();
	} // if
else
	{
	WidgetSetText(IDC_WIZ_ECONAME, "");
	WidgetCBSetCurSel(IDC_WIZ_ECOGROUNDTEXDROP, -1);
	ConfigureSC(NativeWin, IDC_WIZ_CHECKECOHASFOLIAGE, NULL, 0, NULL, 0);
	WidgetSetText(IDC_WIZ_ECONUMIMAGES, "");
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT, NULL);
	} // else

DisableEcoButtons();

} // ForestWizGUI::ConfigureEcoData

/*===========================================================================*/

void ForestWizGUI::UpdateEcoFoliage(void)
{
char NumFolStr[32];

if (DisplayEcoData.HasFoliage)
	{
	sprintf(NumFolStr, "%d", DisplayEcoData.CountFoliageNames());
	WidgetSetText(IDC_WIZ_ECONUMIMAGES, NumFolStr);
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT, &DisplayEcoData.FolCount[CurrentEcoFolFile], 1.0, 0.0, 10000.0, FIOFlag_Long, NULL, 0);
	CMapHtADT.SetValue(DisplayEcoData.FolHt[CurrentEcoFolFile]);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT, &CMapHtADT);
	} // if
else
	{
	WidgetSetText(IDC_WIZ_ECONUMIMAGES, "");
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT, NULL);
	} // else

DisableEcoButtons();

} // ForestWizGUI::UpdateEcoFoliage

/*===========================================================================*/

void ForestWizGUI::DisableEcoButtons(void)
{

WidgetSetDisabled(IDC_WIZ_ECONEXTUNIT, CurrentEcoData >= Wizzer.NumEcoUnits - 1);
WidgetSetDisabled(IDC_WIZ_ECOPREVUNIT, CurrentEcoData == 0);
WidgetSetDisabled(IDC_WIZ_ECONEXTIMAGE, ! (DisplayEcoData.HasFoliage && DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0]));
WidgetSetDisabled(IDC_WIZ_ECOPREVIMAGE, ! (DisplayEcoData.HasFoliage && CurrentEcoFolFile > 0));
WidgetSetDisabled(IDC_WIZ_ECOIMAGEFILE, ! (DisplayEcoData.HasFoliage));
WidgetSetDisabled(IDC_WIZ_ECOFOLWT, ! (DisplayEcoData.HasFoliage && DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0]));
WidgetSetDisabled(IDC_WIZ_ECOFOLHT, ! (DisplayEcoData.HasFoliage && DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0]));

WidgetShow(IDC_WIZ_ECONEXTIMAGE, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOPREVIMAGE, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOIMAGEFILE, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOFOLWT, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOFOLHT, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECONUMIMAGES, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_CHECKECOHASFOLIAGE, Wizzer.VegetationExists);
WidgetShow(IDC_FILETXT2, Wizzer.VegetationExists);
WidgetShow(IDC_IMAGESTXT2, Wizzer.VegetationExists);
WidgetShow(IDC_AREALUNITSTXT2, Wizzer.VegetationExists);
WidgetShow(IDC_ANDORTXT2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_GRABIMAGES2, Wizzer.VegetationExists);

} // ForestWizGUI::DisableEcoButtons

/*===========================================================================*/

void ForestWizGUI::ProcessEcoUnit(void)
{

if (CurrentEcoData < Wizzer.NumEcoUnits)
	{
	ProcessEcoImageName(TRUE);
	Wizzer.SetEcoData(CurrentEcoData, &DisplayEcoData);
	if (CurrentEcoData < Wizzer.NumEcoUnits - 1)
		{
		CurrentEcoData ++;
		ConfigureEcoData(FALSE);
		} // if
	} // if

} // ForestWizGUI::ProcessEcoUnit

/*===========================================================================*/

void ForestWizGUI::GoBackAnEcoUnit(void)
{

if (CurrentEcoData > 0)
	{
	Wizzer.SetEcoData(CurrentEcoData, &DisplayEcoData);
	CurrentEcoData --;
	ConfigureEcoData(FALSE);
	} // if

} // ForestWizGUI::GoBackAnEcoUnit

/*===========================================================================*/

void ForestWizGUI::ProcessEcoImageName(int AdvanceImage)
{
char NumFolStr[32];

if (CurrentEcoFolFile < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE - 1)
	{
	if (DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0])
		{
		strcpy(ProjectHost->imagepath, DisplayEcoData.FolNames[CurrentEcoFolFile].GetPath());
		if (AdvanceImage)
			{
			CurrentEcoFolFile ++;
			if (! DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0])
				DisplayEcoData.FolNames[CurrentEcoFolFile].SetPath(ProjectHost->imagepath);
			ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
			} // if
		sprintf(NumFolStr, "%d", DisplayEcoData.CountFoliageNames());
		WidgetSetText(IDC_WIZ_ECONUMIMAGES, NumFolStr);
		} // if
	} // if
else
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);

UpdateEcoFoliage();

} // ForestWizGUI::ProcessEcoImageName

/*===========================================================================*/

void ForestWizGUI::GoBackAnEcoImageName(void)
{

if (CurrentEcoFolFile > 0)
	{
	CurrentEcoFolFile --;
	UpdateEcoFoliage();
	} // if

} // ForestWizGUI::GoBackAnEcoImageName

/*===========================================================================*/

void ForestWizGUI::SelectNewEcoGroundTex(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_WIZ_ECOGROUNDTEXDROP);
DisplayEcoData.GroundTexture = (char)Current;

} // ForestWizGUI::SelectNewEcoGroundTex

/*===========================================================================*/

void ForestWizGUI::SetEcoDensityUnits(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_WIZ_AREAUNITSDROP);
Wizzer.EcoAreaUnits = (char)Current;

} // ForestWizGUI::SetEcoDensityUnits

/*===========================================================================*/

void ForestWizGUI::GrabEcoImages(void)
{
Raster *CurRast = NULL;
long ImgTotal, ImgCt, Found;

if (GlobalApp->GUIWins->ILG)
	{
	// process whatever is entered already
	ProcessEcoImageName(TRUE);
	// go ahead to a blank space
	while (DisplayEcoData.FolNames[CurrentEcoFolFile].GetName()[0])
		ProcessEcoImageName(TRUE);
	// find the total images already so we don't exceed limit
	ImgTotal = DisplayEcoData.CountFoliageNames();
	// for each image that is not already listed add an item
	for (CurRast = GlobalApp->GUIWins->ILG->GetNextSelected(CurRast); CurRast && ImgTotal < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE - 1; CurRast = GlobalApp->GUIWins->ILG->GetNextSelected(CurRast))
		{
		Found = 0;
		ImgTotal = DisplayEcoData.CountFoliageNames();
		for (ImgCt = 0; ImgCt < ImgTotal; ImgCt ++)
			{
			if (! stricmp(DisplayEcoData.FolNames[ImgCt].GetName(), CurRast->GetName()))
				{
				Found = 1;
				break;
				} // if
			} // for
		if (! Found)
			{
			DisplayEcoData.HasFoliage = 1;
			WidgetSCSync(IDC_WIZ_CHECKECOHASFOLIAGE, WP_SCSYNC_NONOTIFY);
			DisplayEcoData.FolNames[CurrentEcoFolFile].Copy(&DisplayEcoData.FolNames[CurrentEcoFolFile], &CurRast->PAF);
			ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayEcoData.FolNames[CurrentEcoFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
			ProcessEcoImageName(TRUE);
			ImgTotal ++;
			} // if
		} // for
	// go back one so it can be seen that something was done
	GoBackAnEcoImageName();
	} // if
else
	{
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // else

} // ForestWizGUI::GrabEcoImages

/*===========================================================================*/

void ForestWizGUI::ConfigureClassData(int ConfigNULL)
{
char ClassTxt[64];

if (CurrentClassData >= Wizzer.NumClassUnits)
	CurrentClassData = 0;
CurrentClassFolFile = 0;
CurrentClassGroup = 0;

sprintf(ClassTxt, "Class %d of %d", CurrentClassData + 1, Wizzer.NumClassUnits);
WidgetSetText(IDC_CLASSTXT, ClassTxt);

if (! ConfigNULL && Wizzer.FetchClassData(CurrentClassData, &DisplayClassData))
	{
	while (DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0] && DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile + 1].GetName()[0])
		CurrentClassFolFile ++;
	WidgetSetText(IDC_WIZ_ECONAME2, DisplayClassData.PolyName);
	WidgetCBSetCurSel(IDC_WIZ_ECOGROUNDTEXDROP2, DisplayClassData.GroundTexture);
	ConfigureSC(NativeWin, IDC_WIZ_CHECKECOHASFOLIAGE2, &DisplayClassData.HasFoliage, SCFlag_Char, NULL, 0);
	UpdateClassFoliage();
	} // if
else
	{
	WidgetSetText(IDC_WIZ_ECONAME2, "");
	WidgetCBSetCurSel(IDC_WIZ_ECOGROUNDTEXDROP2, -1);
	ConfigureSC(NativeWin, IDC_WIZ_CHECKECOHASFOLIAGE2, NULL, 0, NULL, 0);
	WidgetSetText(IDC_WIZ_ECONUMIMAGES2, "");
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT2, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT2, NULL);
	} // else

DisableClassButtons();

} // ForestWizGUI::ConfigureClassData

/*===========================================================================*/

void ForestWizGUI::UpdateClassFoliage(void)
{
char NumFolStr[32];

if (DisplayClassData.HasFoliage)
	{
	sprintf(NumFolStr, "%d", DisplayClassData.Groups[CurrentClassGroup].CountFoliageNames());
	WidgetSetText(IDC_WIZ_ECONUMIMAGES2, NumFolStr);
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT2, &DisplayClassData.Groups[CurrentClassGroup].FolCount[CurrentClassFolFile], 1.0, 0.0, 10000.0, FIOFlag_Long, NULL, 0);
	CMapHtADT.SetValue(DisplayClassData.Groups[CurrentClassGroup].FolHt[CurrentClassFolFile]);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT2, &CMapHtADT);
	} // if
else
	{
	WidgetSetText(IDC_WIZ_ECONUMIMAGES2, "");
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
	ConfigureFI(NativeWin, IDC_WIZ_ECOFOLWT2, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
	WidgetSNConfig(IDC_WIZ_ECOFOLHT2, NULL);
	} // else

DisableClassButtons();

} // ForestWizGUI::UpdateClassFoliage

/*===========================================================================*/

void ForestWizGUI::DisableClassButtons(void)
{
int DisplayFoliage;

DisplayFoliage = (DisplayClassData.HasFoliage || Wizzer.CMapOrVec == WCS_FORESTWIZ_CMAPORVEC_POINTDATA);
WidgetSetDisabled(IDC_WIZ_ECONEXTUNIT2, CurrentClassData >= Wizzer.NumClassUnits - 1);
WidgetSetDisabled(IDC_WIZ_ECOPREVUNIT2, CurrentClassData == 0);
WidgetSetDisabled(IDC_WIZ_ECONEXTIMAGE2, ! (DisplayFoliage && DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0]));
WidgetSetDisabled(IDC_WIZ_ECOPREVIMAGE2, ! (DisplayFoliage && CurrentClassFolFile > 0));
WidgetSetDisabled(IDC_WIZ_ECOIMAGEFILE2, ! (DisplayFoliage));
WidgetSetDisabled(IDC_WIZ_ECOFOLWT2, ! (DisplayFoliage && DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0]));
WidgetSetDisabled(IDC_WIZ_ECOFOLHT2, ! (DisplayFoliage && DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0]));

WidgetShow(IDC_WIZ_ECONEXTIMAGE2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOPREVIMAGE2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOIMAGEFILE2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOFOLWT2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOFOLHT2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECONUMIMAGES2, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_CHECKECOHASFOLIAGE2, Wizzer.VegetationExists && Wizzer.CMapOrVec != WCS_FORESTWIZ_CMAPORVEC_POINTDATA);
WidgetShow(IDC_FILETXT, Wizzer.VegetationExists);
WidgetShow(IDC_IMAGESTXT, Wizzer.VegetationExists);
WidgetShow(IDC_AREALUNITSTXT, Wizzer.VegetationExists && Wizzer.CMapOrVec != WCS_FORESTWIZ_CMAPORVEC_POINTDATA);
WidgetShow(IDC_ANDORTXT, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_GRABIMAGES, Wizzer.VegetationExists);
WidgetShow(IDC_WIZ_ECOGROUNDTEXDROP2, Wizzer.CMapOrVec != WCS_FORESTWIZ_CMAPORVEC_POINTDATA);
WidgetShow(IDC_GROUNDTEXTXT, Wizzer.CMapOrVec != WCS_FORESTWIZ_CMAPORVEC_POINTDATA);

} // ForestWizGUI::DisableClassButtons

/*===========================================================================*/

void ForestWizGUI::ProcessClass(void)
{

if (CurrentClassData < Wizzer.NumClassUnits)
	{
	ProcessClassImageName(TRUE);
	Wizzer.SetClassData(CurrentClassData, &DisplayClassData);
	if (CurrentClassData < Wizzer.NumClassUnits - 1)
		{
		CurrentClassData ++;
		ConfigureClassData(FALSE);
		} // if
	} // if

} // ForestWizGUI::ProcessClass

/*===========================================================================*/

void ForestWizGUI::GoBackAClass(void)
{

if (CurrentClassData > 0)
	{
	Wizzer.SetClassData(CurrentClassData, &DisplayClassData);
	CurrentClassData --;
	ConfigureClassData(FALSE);
	} // if

} // ForestWizGUI::GoBackAClass

/*===========================================================================*/

void ForestWizGUI::ProcessClassImageName(int AdvanceImage)
{
char NumFolStr[32];

if (CurrentClassFolFile < WCS_FORESTWIZECODATA_MAXNUMFOLIAGE - 1)
	{
	if (DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0])
		{
		strcpy(ProjectHost->imagepath, DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetPath());
		if (AdvanceImage)
			{
			CurrentClassFolFile ++;
			if (! DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0])
				DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].SetPath(ProjectHost->imagepath);
			ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
			} // if
		sprintf(NumFolStr, "%d", DisplayClassData.Groups[CurrentClassGroup].CountFoliageNames());
		WidgetSetText(IDC_WIZ_ECONUMIMAGES2, NumFolStr);
		} // if
	} // if
else
	ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, NULL, WCS_PATHANDFILE_PATH_LEN_MINUSONE, NULL, WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);

UpdateClassFoliage();

} // ForestWizGUI::ProcessClassImageName

/*===========================================================================*/

void ForestWizGUI::GoBackAClassImageName(void)
{

if (CurrentClassFolFile > 0)
	{
	CurrentClassFolFile --;
	UpdateClassFoliage();
	} // if

} // ForestWizGUI::GoBackAClassImageName

/*===========================================================================*/

void ForestWizGUI::SelectNewClassGroundTex(void)
{
long Current;

Current = WidgetCBGetCurSel(IDC_WIZ_ECOGROUNDTEXDROP2);
DisplayClassData.GroundTexture = (char)Current;

} // ForestWizGUI::SelectNewClassGroundTex

/*===========================================================================*/

void ForestWizGUI::GrabClassImages(void)
{
Raster *CurRast = NULL;
long ImgTotal, ImgCt, Found;


if (GlobalApp->GUIWins->ILG)
	{
	// process whatever is entered already
	ProcessClassImageName(TRUE);
	// go ahead to a blank space
	while (DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName()[0])
		ProcessClassImageName(TRUE);
	// find the total images already so we don't exceed limit
	ImgTotal = DisplayClassData.Groups[CurrentClassGroup].CountFoliageNames();
	// for each image that is not already listed add an item
	for (CurRast = GlobalApp->GUIWins->ILG->GetNextSelected(CurRast); CurRast && ImgTotal < WCS_FORESTWIZCLASSDATA_MAXNUMFOLIAGE - 1; CurRast = GlobalApp->GUIWins->ILG->GetNextSelected(CurRast))
		{
		Found = 0;
		ImgTotal = DisplayClassData.Groups[CurrentClassGroup].CountFoliageNames();
		for (ImgCt = 0; ImgCt < ImgTotal; ImgCt ++)
			{
			if (! stricmp(DisplayClassData.Groups[CurrentClassGroup].FolNames[ImgCt].GetName(), CurRast->GetName()))
				{
				Found = 1;
				break;
				} // if
			} // for
		if (! Found)
			{
			DisplayClassData.HasFoliage = 1;
			WidgetSCSync(IDC_WIZ_CHECKECOHASFOLIAGE2, WP_SCSYNC_NONOTIFY);
			DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].Copy(&DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile], &CurRast->PAF);
			ConfigureDD(NativeWin, IDC_WIZ_ECOIMAGEFILE2, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetPath(), WCS_PATHANDFILE_PATH_LEN_MINUSONE, (char *)DisplayClassData.Groups[CurrentClassGroup].FolNames[CurrentClassFolFile].GetName(), WCS_PATHANDFILE_NAME_LEN_MINUSONE, IDC_LABEL_CMAP);
			ProcessClassImageName(TRUE);
			ImgTotal ++;
			} // if
		} // for
	// go back one so it can be seen that something was done
	GoBackAClassImageName();
	} // if
else
	{
	AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // else

} // ForestWizGUI::GrabClassImages

/*===========================================================================*/

int ForestWizGUI::FetchSpeciesData(long SpeciesCt, ForestWizClassData *ClassData)
{

if (Wizzer.SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	return (Wizzer.FetchSpDensData(SpeciesCt, ClassData));

return (Wizzer.FetchClassData(SpeciesCt, ClassData));

} // ForestWizGUI::FetchSpeciesData

/*===========================================================================*/

void ForestWizGUI::ConfigureSpeciesData(int ConfigNULL)
{
long NumAttrs, FoundPos;

if (Wizzer.SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	{
	if (CurrentSpeciesData >= Wizzer.NumSpDensFields)
		CurrentSpeciesData = 0;
	} // if
else
	{
	if (CurrentSpeciesData >= Wizzer.NumClassUnits)
		CurrentSpeciesData = 0;
	} // else

if (! ConfigNULL && FetchSpeciesData(CurrentSpeciesData, &DisplayClassData))
	{
	WidgetSetText(IDC_WIZ_SPNAME, DisplayClassData.PolyName);
	NumAttrs = WidgetCBGetCount(IDC_WIZ_MULTISIZEATTRDROP);
	for (FoundPos = 0; FoundPos < NumAttrs; FoundPos ++)
		{
		if (Wizzer.SizeAttr[CurrentSpeciesData] == (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTISIZEATTRDROP, FoundPos))
			{
			WidgetCBSetCurSel(IDC_WIZ_MULTISIZEATTRDROP, FoundPos);
			break;
			} // for
		} // for
	if (FoundPos >= NumAttrs)
		WidgetCBSetCurSel(IDC_WIZ_MULTISIZEATTRDROP, -1);
	} // if
else
	{
	WidgetSetText(IDC_WIZ_SPNAME, "");
	WidgetCBSetCurSel(IDC_WIZ_MULTISIZEATTRDROP, -1);
	} // else

DisableSpeciesButtons();

} // ForestWizGUI::ConfigureSpeciesData

/*===========================================================================*/

void ForestWizGUI::ConfigureDBHData(int ConfigNULL)
{
long NumAttrs, FoundPos;

if (CurrentSpeciesData >= Wizzer.NumClassUnits)
	CurrentSpeciesData = 0;

if (! ConfigNULL && Wizzer.FetchClassData(CurrentSpeciesData, &DisplayClassData))
	{
	WidgetSetText(IDC_WIZ_SPNAME2, DisplayClassData.PolyName);
	NumAttrs = WidgetCBGetCount(IDC_WIZ_MULTIDBHATTRDROP);
	for (FoundPos = 0; FoundPos < NumAttrs; FoundPos ++)
		{
		if (Wizzer.DBHAttr[CurrentSpeciesData] == (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTIDBHATTRDROP, FoundPos))
			{
			WidgetCBSetCurSel(IDC_WIZ_MULTIDBHATTRDROP, FoundPos);
			break;
			} // for
		} // for
	if (FoundPos >= NumAttrs)
		WidgetCBSetCurSel(IDC_WIZ_MULTIDBHATTRDROP, -1);
	} // if
else
	{
	WidgetSetText(IDC_WIZ_SPNAME2, "");
	WidgetCBSetCurSel(IDC_WIZ_MULTIDBHATTRDROP, -1);
	} // else

DisableDBHButtons();

} // ForestWizGUI::ConfigureDBHData

/*===========================================================================*/

void ForestWizGUI::ConfigureSPDensData(int ConfigNULL)
{
long NumAttrs, FoundPos;

if (CurrentSpeciesData >= Wizzer.NumSpDensFields)
	CurrentSpeciesData = 0;

if (! ConfigNULL && Wizzer.FetchSpDensData(CurrentSpeciesData, &DisplayClassData))
	{
	WidgetSetText(IDC_WIZ_SPNAME3, DisplayClassData.PolyName);
	NumAttrs = WidgetCBGetCount(IDC_WIZ_MULTISPDENSATTRDROP);
	for (FoundPos = 0; FoundPos < NumAttrs; FoundPos ++)
		{
		if (Wizzer.SpDensAttr[CurrentSpeciesData] == (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTISPDENSATTRDROP, FoundPos))
			{
			WidgetCBSetCurSel(IDC_WIZ_MULTISPDENSATTRDROP, FoundPos);
			break;
			} // for
		} // for
	if (FoundPos >= NumAttrs)
		WidgetCBSetCurSel(IDC_WIZ_MULTISPDENSATTRDROP, -1);
	} // if
else
	{
	WidgetSetText(IDC_WIZ_SPNAME3, "");
	WidgetCBSetCurSel(IDC_WIZ_MULTISPDENSATTRDROP, -1);
	} // else

DisableSPDensButtons();

} // ForestWizGUI::ConfigureSPDensData

/*===========================================================================*/

void ForestWizGUI::ProcessSpecies(void)
{
long ClassLimit;

if (Wizzer.SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	ClassLimit = Wizzer.NumSpDensFields;
else
	ClassLimit = Wizzer.NumClassUnits;

if (CurrentSpeciesData < ClassLimit)
	{
	SetSpeciesSize();
	if (CurrentSpeciesData < ClassLimit - 1)
		{
		CurrentSpeciesData ++;
		ConfigureSpeciesData(FALSE);
		} // if
	} // if

} // ForestWizGUI::ProcessSpecies

/*===========================================================================*/

void ForestWizGUI::ProcessDBH(void)
{

if (CurrentSpeciesData < Wizzer.NumClassUnits)
	{
	SetSpeciesDBH();
	if (CurrentSpeciesData < Wizzer.NumClassUnits - 1)
		{
		CurrentSpeciesData ++;
		ConfigureDBHData(FALSE);
		} // if
	} // if

} // ForestWizGUI::ProcessDBH

/*===========================================================================*/

void ForestWizGUI::ProcessSPDens(void)
{

if (CurrentSpeciesData < Wizzer.NumSpDensFields)
	{
	SetSpeciesSPDens();
	if (CurrentSpeciesData < Wizzer.NumSpDensFields - 1)
		{
		CurrentSpeciesData ++;
		ConfigureSPDensData(FALSE);
		} // if
	} // if

} // ForestWizGUI::ProcessSPDens

/*===========================================================================*/

void ForestWizGUI::GoBackASpecies(void)
{

if (CurrentSpeciesData > 0)
	{
	SetSpeciesSize();
	CurrentSpeciesData --;
	ConfigureSpeciesData(FALSE);
	} // if

} // ForestWizGUI::GoBackASpecies

/*===========================================================================*/

void ForestWizGUI::GoBackADBH(void)
{

if (CurrentSpeciesData > 0)
	{
	SetSpeciesDBH();
	CurrentSpeciesData --;
	ConfigureDBHData(FALSE);
	} // if

} // ForestWizGUI::GoBackADBH

/*===========================================================================*/

void ForestWizGUI::GoBackASPDens(void)
{

if (CurrentSpeciesData > 0)
	{
	SetSpeciesSPDens();
	CurrentSpeciesData --;
	ConfigureSPDensData(FALSE);
	} // if

} // ForestWizGUI::GoBackASPDens

/*===========================================================================*/

void ForestWizGUI::SetSpeciesSize(void)
{
LayerEntry *Selected;
long Current;

if ((Current = WidgetCBGetCurSel(IDC_WIZ_MULTISIZEATTRDROP)) != CB_ERR)
	{
	if ((Selected = (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTISIZEATTRDROP, Current)) && Selected != (LayerEntry *)CB_ERR)
		{
		Wizzer.SizeAttr[CurrentSpeciesData] = Selected;
		} // if
	} // if

} // ForestWizGUI::SetSpeciesSize

/*===========================================================================*/

void ForestWizGUI::SetSpeciesDBH(void)
{
LayerEntry *Selected;
long Current;

if ((Current = WidgetCBGetCurSel(IDC_WIZ_MULTIDBHATTRDROP)) != CB_ERR)
	{
	if ((Selected = (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTIDBHATTRDROP, Current)) && Selected != (LayerEntry *)CB_ERR)
		{
		Wizzer.DBHAttr[CurrentSpeciesData] = Selected;
		} // if
	} // if

} // ForestWizGUI::SetSpeciesDBH

/*===========================================================================*/

void ForestWizGUI::SetSpeciesSPDens(void)
{
LayerEntry *Selected;
long Current;

if ((Current = WidgetCBGetCurSel(IDC_WIZ_MULTISPDENSATTRDROP)) != CB_ERR)
	{
	if ((Selected = (LayerEntry *)WidgetCBGetItemData(IDC_WIZ_MULTISPDENSATTRDROP, Current)) && Selected != (LayerEntry *)CB_ERR)
		{
		Wizzer.SpDensAttr[CurrentSpeciesData] = Selected;
		} // if
	} // if

} // ForestWizGUI::SetSpeciesSPDens

/*===========================================================================*/

void ForestWizGUI::DisableSpeciesButtons(void)
{
long ClassLimit;

if (Wizzer.SpeciesScenario == WCS_FORESTWIZ_SPSCENARIO_FARSITE)
	ClassLimit = Wizzer.NumSpDensFields;
else
	ClassLimit = Wizzer.NumClassUnits;

WidgetSetDisabled(IDC_WIZ_NEXTSPECIES, CurrentSpeciesData >= ClassLimit - 1);
WidgetSetDisabled(IDC_WIZ_PREVSPECIES, CurrentSpeciesData == 0);

} // ForestWizGUI::DisableSpeciesButtons

/*===========================================================================*/

void ForestWizGUI::DisableDBHButtons(void)
{

WidgetSetDisabled(IDC_WIZ_NEXTSPECIES2, CurrentSpeciesData >= Wizzer.NumClassUnits - 1);
WidgetSetDisabled(IDC_WIZ_PREVSPECIES2, CurrentSpeciesData == 0);

} // ForestWizGUI::DisableDBHButtons

/*===========================================================================*/

void ForestWizGUI::DisableSPDensButtons(void)
{

WidgetSetDisabled(IDC_WIZ_NEXTSPECIES3, CurrentSpeciesData >= Wizzer.NumSpDensFields - 1);
WidgetSetDisabled(IDC_WIZ_PREVSPECIES3, CurrentSpeciesData == 0);

} // ForestWizGUI::DisableSPDensButtons

/*===========================================================================*/

void ForestWizGUI::FillDBAttributeCombos(void)
{
LayerEntry *Entry;
const char *LayerName;
long ItemCt, CurItem;
char EntryName[256];

WidgetCBClear(IDC_WIZ_ONESPATTRDROP);
WidgetCBClear(IDC_WIZ_ONESIZEATTRDROP);
WidgetCBClear(IDC_WIZ_ONEDENSATTRDROP);

WidgetCBClear(IDC_WIZ_FIRSTSPATTRDROP);
WidgetCBClear(IDC_WIZ_SECONDSPATTRDROP);
WidgetCBClear(IDC_WIZ_FIRSTSIZEATTRDROP);
WidgetCBClear(IDC_WIZ_SECONDSIZEATTRDROP);
WidgetCBClear(IDC_WIZ_FIRSTDENSATTRDROP);
WidgetCBClear(IDC_WIZ_SECONDDENSATTRDROP);

WidgetCBClear(IDC_WIZ_MULTISIZEATTRDROP);
WidgetLBClear(IDC_WIZ_MULTIDENSLIST);
WidgetLBClear(IDC_WIZ_MULTISPLIST);

WidgetCBClear(IDC_WIZ_ONEDBHATTRDROP);
WidgetCBClear(IDC_WIZ_FIRSTDBHATTRDROP);
WidgetCBClear(IDC_WIZ_SECONDDBHATTRDROP);
WidgetCBClear(IDC_WIZ_MULTIDBHATTRDROP);
WidgetCBClear(IDC_WIZ_MULTISPDENSATTRDROP);

WidgetCBClear(IDC_WIZ_MINHTATTRDROP);
WidgetCBClear(IDC_WIZ_MINAGEATTRDROP);
WidgetCBClear(IDC_WIZ_MINDBHATTRDROP);
WidgetCBClear(IDC_WIZ_MINHTATTRDROP2);
WidgetCBClear(IDC_WIZ_MINAGEATTRDROP2);
WidgetCBClear(IDC_WIZ_MINDBHATTRDROP2);
WidgetCBClear(IDC_WIZ_MINHTATTRDROP3);
WidgetCBClear(IDC_WIZ_MINAGEATTRDROP3);
WidgetCBClear(IDC_WIZ_MINDBHATTRDROP3);

strcpy(EntryName, "* ");
Entry = DBHost->DBLayers.FirstEntry();
while (Entry)
	{
	if (! Entry->TestFlags(WCS_LAYER_LINKATTRIBUTE))
		{
		LayerName = Entry->GetName();
		if (LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_DBL || LayerName[0] == WCS_LAYER_ATTRIB_MARKER_SYMBOL_TXT)
			{
			strcpy(&EntryName[2], &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_ONESPATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_ONESPATTRDROP, ItemCt, Entry);
			if (Wizzer.SpeciesAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_ONESPATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_ONESIZEATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_ONESIZEATTRDROP, ItemCt, Entry);
			if (Wizzer.SizeAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_ONESIZEATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_ONEDENSATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_ONEDENSATTRDROP, ItemCt, Entry);
			if (Wizzer.DensAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_ONEDENSATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_FIRSTSPATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_FIRSTSPATTRDROP, ItemCt, Entry);
			if (Wizzer.SpeciesAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_FIRSTSPATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_SECONDSPATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_SECONDSPATTRDROP, ItemCt, Entry);
			if (Wizzer.SpeciesAttr[1] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_SECONDSPATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_FIRSTSIZEATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_FIRSTSIZEATTRDROP, ItemCt, Entry);
			if (Wizzer.SizeAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_FIRSTSIZEATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_SECONDSIZEATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_SECONDSIZEATTRDROP, ItemCt, Entry);
			if (Wizzer.SizeAttr[1] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_SECONDSIZEATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_FIRSTDENSATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_FIRSTDENSATTRDROP, ItemCt, Entry);
			if (Wizzer.DensAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_FIRSTDENSATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_SECONDDENSATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_SECONDDENSATTRDROP, ItemCt, Entry);
			if (Wizzer.DensAttr[1] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_SECONDDENSATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_MULTISIZEATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_MULTISIZEATTRDROP, ItemCt, Entry);

			ItemCt = WidgetLBInsert(IDC_WIZ_MULTIDENSLIST, -1, EntryName);
			WidgetLBSetItemData(IDC_WIZ_MULTIDENSLIST, ItemCt, Entry);
			for (CurItem = 0; CurItem < Wizzer.NumSpeciesFields; CurItem ++)
				{
				if (Wizzer.SpeciesAttr[CurItem] == Entry)
					{
					WidgetLBSetSelState(IDC_WIZ_MULTIDENSLIST, 1, ItemCt);
					break;
					} // if
				} // for

			ItemCt = WidgetLBInsert(IDC_WIZ_MULTISPLIST, -1, EntryName);
			WidgetLBSetItemData(IDC_WIZ_MULTISPLIST, ItemCt, Entry);
			for (CurItem = 0; CurItem < Wizzer.NumSpeciesFields; CurItem ++)
				{
				if (Wizzer.SpeciesAttr[CurItem] == Entry)
					{
					WidgetLBSetSelState(IDC_WIZ_MULTISPLIST, 1, ItemCt);
					break;
					} // if
				} // for

			ItemCt = WidgetCBInsert(IDC_WIZ_ONEDBHATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_ONEDBHATTRDROP, ItemCt, Entry);
			if (Wizzer.DBHAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_ONEDBHATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_FIRSTDBHATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_FIRSTDBHATTRDROP, ItemCt, Entry);
			if (Wizzer.DBHAttr[0] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_FIRSTDBHATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_SECONDDBHATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_SECONDDBHATTRDROP, ItemCt, Entry);
			if (Wizzer.DBHAttr[1] == Entry)
				WidgetCBSetCurSel(IDC_WIZ_SECONDDBHATTRDROP, ItemCt);

			ItemCt = WidgetCBInsert(IDC_WIZ_MULTIDBHATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_MULTIDBHATTRDROP, ItemCt, Entry);

			ItemCt = WidgetCBInsert(IDC_WIZ_MULTISPDENSATTRDROP, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_MULTISPDENSATTRDROP, ItemCt, Entry);

			ItemCt = WidgetCBInsert(IDC_WIZ_MINHTATTRDROP, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINDBHATTRDROP, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINAGEATTRDROP, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINHTATTRDROP2, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINDBHATTRDROP2, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINAGEATTRDROP2, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINHTATTRDROP3, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINDBHATTRDROP3, -1, &LayerName[1]);
			ItemCt = WidgetCBInsert(IDC_WIZ_MINAGEATTRDROP3, -1, &LayerName[1]);
			WidgetCBSetItemData(IDC_WIZ_MINHTATTRDROP, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINDBHATTRDROP, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINAGEATTRDROP, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINHTATTRDROP2, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINDBHATTRDROP2, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINAGEATTRDROP2, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINHTATTRDROP3, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINDBHATTRDROP3, ItemCt, Entry);
			WidgetCBSetItemData(IDC_WIZ_MINAGEATTRDROP3, ItemCt, Entry);
			if (Wizzer.SizeAttr[2] == Entry)
				{
				WidgetCBSetCurSel(IDC_WIZ_MINHTATTRDROP, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINDBHATTRDROP, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINAGEATTRDROP, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINHTATTRDROP2, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINDBHATTRDROP2, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINAGEATTRDROP2, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINHTATTRDROP3, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINDBHATTRDROP3, ItemCt);
				WidgetCBSetCurSel(IDC_WIZ_MINAGEATTRDROP3, ItemCt);
				} // if
				
			} // if an attribute layer
		} // if
	Entry = DBHost->DBLayers.NextEntry(Entry);
	} // while

} // ForestWizGUI::FillDBAttributeCombos

/*===========================================================================*/

void ForestWizGUI::SelectDBAttribute(unsigned short WidID)
{
long Current;
LayerEntry *Selected;

if ((Current = WidgetCBGetCurSel(WidID)) != CB_ERR)
	{
	if ((Selected = (LayerEntry *)WidgetCBGetItemData(WidID, Current)) && Selected != (LayerEntry *)CB_ERR)
		{
		switch (WidID)
			{
			case IDC_WIZ_ONESPATTRDROP:
				{
				Wizzer.SpeciesAttr[0] = Selected;
				break;
				} // IDC_WIZ_ONESPATTRDROP
			case IDC_WIZ_ONESIZEATTRDROP:
				{
				Wizzer.SizeAttr[0] = Selected;
				break;
				} // IDC_WIZ_ONESIZEATTRDROP
			case IDC_WIZ_ONEDENSATTRDROP:
				{
				Wizzer.DensAttr[0] = Selected;
				break;
				} // IDC_WIZ_ONEDENSATTRDROP
			case IDC_WIZ_FIRSTSPATTRDROP:
				{
				Wizzer.SpeciesAttr[0] = Selected;
				break;
				} // IDC_WIZ_FIRSTSPATTRDROP
			case IDC_WIZ_SECONDSPATTRDROP:
				{
				Wizzer.SpeciesAttr[1] = Selected;
				break;
				} // IDC_WIZ_SECONDSPATTRDROP
			case IDC_WIZ_FIRSTSIZEATTRDROP:
				{
				Wizzer.SizeAttr[0] = Selected;
				break;
				} // IDC_WIZ_FIRSTSIZEATTRDROP
			case IDC_WIZ_SECONDSIZEATTRDROP:
				{
				Wizzer.SizeAttr[1] = Selected;
				break;
				} // IDC_WIZ_SECONDSIZEATTRDROP
			case IDC_WIZ_FIRSTDENSATTRDROP:
				{
				Wizzer.DensAttr[0] = Selected;
				break;
				} // IDC_WIZ_FIRSTDENSATTRDROP
			case IDC_WIZ_SECONDDENSATTRDROP:
				{
				Wizzer.DensAttr[1] = Selected;
				break;
				} // IDC_WIZ_SECONDDENSATTRDROP
			case IDC_WIZ_MULTISIZEATTRDROP:
				{
				Wizzer.SizeAttr[CurrentSpeciesData] = Selected;
				break;
				} // IDC_WIZ_MULTISIZEATTRDROP
			case IDC_WIZ_ONEDBHATTRDROP:
				{
				Wizzer.DBHAttr[0] = Selected;
				break;
				} // IDC_WIZ_ONEDBHATTRDROP
			case IDC_WIZ_FIRSTDBHATTRDROP:
				{
				Wizzer.DBHAttr[0] = Selected;
				break;
				} // IDC_WIZ_FIRSTDBHATTRDROP
			case IDC_WIZ_SECONDDBHATTRDROP:
				{
				Wizzer.DBHAttr[1] = Selected;
				break;
				} // IDC_WIZ_SECONDDBHATTRDROP
			case IDC_WIZ_MULTIDBHATTRDROP:
				{
				Wizzer.DBHAttr[CurrentSpeciesData] = Selected;
				break;
				} // IDC_WIZ_MULTIDBHATTRDROP
			case IDC_WIZ_MULTISPDENSATTRDROP:
				{
				Wizzer.SpDensAttr[CurrentSpeciesData] = Selected;
				break;
				} // IDC_WIZ_MULTISPDENSATTRDROP
			case IDC_WIZ_MINHTATTRDROP:
			case IDC_WIZ_MINAGEATTRDROP:
			case IDC_WIZ_MINDBHATTRDROP:
			case IDC_WIZ_MINHTATTRDROP2:
			case IDC_WIZ_MINAGEATTRDROP2:
			case IDC_WIZ_MINDBHATTRDROP2:
			case IDC_WIZ_MINHTATTRDROP3:
			case IDC_WIZ_MINAGEATTRDROP3:
			case IDC_WIZ_MINDBHATTRDROP3:
				{
				Wizzer.SizeAttr[2] = Selected;
				break;
				} // IDC_WIZ_MINHTATTRDROP
			default:
				break;
			} // switch
		} // if
	} // if

} // ForestWizGUI::SelectDBAttribute

/*===========================================================================*/

void ForestWizGUI::SelectMultiDensAttribs(unsigned short WidID)
{
LayerEntry *Selected;
long Current, NumItems, CurItem;

NumItems = WidgetLBGetCount(WidID);
for (Current = CurItem = 0; Current < NumItems && CurItem < WCS_FORESTWIZ_MAXSPECIES; Current ++)
	{
	if (WidgetLBGetSelState(WidID, Current))
		{
		if ((Selected = (LayerEntry *)WidgetLBGetItemData(WidID, Current)) && Selected != (LayerEntry *)LB_ERR)
			{
			Wizzer.SpeciesAttr[CurItem ++] = Selected;
			} // if
		} // if selected
	} // for
Wizzer.NumSpeciesFields = CurItem;

for ( ; CurItem < WCS_FORESTWIZ_MAXSPECIES; CurItem ++)
	Wizzer.SpeciesAttr[CurItem ++] = NULL;

} // ForestWizGUI::SelectMultiDensAttribs

/*===========================================================================*/

void ForestWizGUI::FillFolHeightCombos(void)
{
long Ct;
char *HeightUnitNames[4] = 
	{
	"Meters",
	"Centimeters",
	"Feet",
	"Inches"
	};
char *BasalAreaUnitNames[4] = 
	{
	"Sq. Meters",
	"Sq. Centimeters",
	"Sq. Feet",
	"Sq. Inches"
	};

WidgetCBClear(IDC_WIZ_HTUNITSDROP);
WidgetCBClear(IDC_WIZ_HTUNITSDROP2);
WidgetCBClear(IDC_WIZ_HTUNITSDROP3);
WidgetCBClear(IDC_WIZ_DIAUNITSDROP);
WidgetCBClear(IDC_WIZ_DBHUNITSDROP);
WidgetCBClear(IDC_WIZ_BAUNITSDROP);

for (Ct = 0; Ct < 4; Ct ++)
	{
	WidgetCBAddEnd(IDC_WIZ_HTUNITSDROP, HeightUnitNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_HTUNITSDROP2, HeightUnitNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_HTUNITSDROP3, HeightUnitNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_DIAUNITSDROP, HeightUnitNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_DBHUNITSDROP, HeightUnitNames[Ct]);
	WidgetCBAddEnd(IDC_WIZ_BAUNITSDROP, BasalAreaUnitNames[Ct]);
	if (Ct == Wizzer.SizeUnits)
		{
		WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP, Ct);
		WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP2, Ct);
		WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP3, Ct);
		} // if
	if (Ct == Wizzer.DBHUnits)
		{
		WidgetCBSetCurSel(IDC_WIZ_DIAUNITSDROP, Ct);
		} // if
	if (Ct == Wizzer.DBHThemeUnits)
		{
		WidgetCBSetCurSel(IDC_WIZ_DBHUNITSDROP, Ct);
		} // if
	if (Ct == Wizzer.BasalAreaUnits)
		{
		WidgetCBSetCurSel(IDC_WIZ_BAUNITSDROP, Ct);
		} // if
	} // for


} // ForestWizGUI::FillFolHeightCombos

/*===========================================================================*/

void ForestWizGUI::SelectFolHeightUnits(unsigned short WidID)
{
long Current;

if ((Current = WidgetCBGetCurSel(WidID)) != CB_ERR)
	{
	Wizzer.SizeUnits = (char)Current;
	WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP, Current);
	WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP2, Current);
	WidgetCBSetCurSel(IDC_WIZ_HTUNITSDROP3, Current);
	} // if

} // ForestWizGUI::SelectFolHeightUnits

/*===========================================================================*/

void ForestWizGUI::SelectFolDBHUnits(unsigned short WidID)
{
long Current;

if ((Current = WidgetCBGetCurSel(WidID)) != CB_ERR)
	{
	Wizzer.DBHUnits = (char)Current;
	SetDBHUnitLabels();
	} // if

} // ForestWizGUI::SelectFolDBHUnits

/*===========================================================================*/

void ForestWizGUI::SelectFolDBHThemeUnits(unsigned short WidID)
{
long Current;

if ((Current = WidgetCBGetCurSel(WidID)) != CB_ERR)
	{
	Wizzer.DBHThemeUnits = (char)Current;
	} // if

} // ForestWizGUI::SelectFolDBHThemeUnits

/*===========================================================================*/

void ForestWizGUI::SelectFolBasalAreaUnits(unsigned short WidID)
{
long Current;

if ((Current = WidgetCBGetCurSel(WidID)) != CB_ERR)
	{
	Wizzer.BasalAreaUnits = (char)Current;
	} // if

} // ForestWizGUI::SelectFolBasalAreaUnits

/*===========================================================================*/

void ForestWizGUI::SetDBHUnitLabels(void)
{

if (Wizzer.DBHUnits == WCS_FORESTWIZ_SIZEUNITS_INCHES)
	{
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS, "inches of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS2, "inches of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS3, "inches of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS4, "inches of diameter");
	} // if
else if (Wizzer.DBHUnits == WCS_FORESTWIZ_SIZEUNITS_FEET)
	{
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS, "feet of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS2, "feet of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS3, "feet of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS4, "feet of diameter");
	} // if
else if (Wizzer.DBHUnits == WCS_FORESTWIZ_SIZEUNITS_CENTIMETERS)
	{
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS, "centimeters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS2, "centimeters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS3, "centimeters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS4, "centimeters of diameter");
	} // if
else //(Wizzer.DBHUnits == WCS_FORESTWIZ_SIZEUNITS_METERS)
	{
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS, "meters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS2, "meters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS3, "meters of diameter");
	WidgetSetText(IDC_WIZ_DBHEQUIVUNITS4, "meters of diameter");
	} // else

} // ForestWizGUI::SetDBHUnitLabels

/*===========================================================================*/

void ForestWizGUI::DisableMinHtWidgets(void)
{

WidgetSetDisabled(IDC_WIZ_MINHTATTRDROP, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINDBHATTRDROP, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINAGEATTRDROP, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINHTATTRDROP2, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINDBHATTRDROP2, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINAGEATTRDROP2, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINHTATTRDROP3, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINDBHATTRDROP3, ! Wizzer.MinSizeThemePresent);
WidgetSetDisabled(IDC_WIZ_MINAGEATTRDROP3, ! Wizzer.MinSizeThemePresent);

} // ForestWizGUI::DisableMinHtWidgets
