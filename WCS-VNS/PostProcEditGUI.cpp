// PostProcEditGUI.cpp
// Code for PostProc editor
// Built from scratch on 2/18/02 by Gary R. Huber
// Copyright 2002 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "PostProcEditGUI.h"
#include "PostProcessEvent.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Conservatory.h"
#include "ViewGUI.h"
#include "RenderControlGUI.h"
#include "Raster.h"
#include "resource.h"

// These live in PostProcessEvent.cpp near PostProcText::RenderPostProc()
extern char *PPT_TextLabels[];
extern char *PPT_TextSymbols[];

char *PostProcEditGUI::TabNames[WCS_POSTPROCGUI_NUMTABS] = {"General", "Detail 1", "Detail 2"};

long PostProcEditGUI::ActivePage;

NativeGUIWin PostProcEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // PostProcEditGUI::Open

/*===========================================================================*/

NativeGUIWin PostProcEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_POSTPROC_GENERAL, 0, 0);
	CreateSubWinFromTemplate(IDD_POSTPROC_NODETAIL, 0, 1);
	CreateSubWinFromTemplate(IDD_POSTPROC_TEXTOVERLAY, 0, 2);
	CreateSubWinFromTemplate(IDD_POSTPROC_TEXTOVERLAY2, 0, 3);
	CreateSubWinFromTemplate(IDD_POSTPROC_STAR, 0, 4);
	CreateSubWinFromTemplate(IDD_POSTPROC_GLOW, 0, 5);
	CreateSubWinFromTemplate(IDD_POSTPROC_HALO, 0, 6);
	CreateSubWinFromTemplate(IDD_POSTPROC_COMPOSITE, 0, 7);
	CreateSubWinFromTemplate(IDD_POSTPROC_EDGEINK, 0, 8);
	CreateSubWinFromTemplate(IDD_POSTPROC_HEADING, 0, 9);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_POSTPROCGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for (TabIndex = 0; TabIndex < WCS_POSTPROCEVENT_NUMTYPES; TabIndex ++)
			{
			TabIndex = WidgetCBInsert(IDC_EVENTTYPEDROP, -1, PostProcessEvent::PostProcEventNames[TabIndex]);
			} // for
		HandleDetailPageChange(ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // PostProcEditGUI::Construct

/*===========================================================================*/

PostProcEditGUI::PostProcEditGUI(EffectsLib *EffectsSource, PostProcess *ActiveSource)
: GUIFenetre('PPRC', this, "Post Process Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_POSTPROC, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
PreviewEnabled = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
ActiveEvent = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Post Process Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	ActiveEvent = Active->Events;
	} // if
else
	ConstructError = 1;

} // PostProcEditGUI::PostProcEditGUI

/*===========================================================================*/

PostProcEditGUI::~PostProcEditGUI()
{

if (PreviewEnabled)
	{
	PreviewEnabled = 0;
	UpdatePreviews();
	} // if

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // PostProcEditGUI::~PostProcEditGUI()

/*===========================================================================*/

long PostProcEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_PPR, 0);

return(0);

} // PostProcEditGUI::HandleCloseWin

/*===========================================================================*/

long PostProcEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case IDC_EDITPROFILE:
		{
		if (ActiveEvent)
			ActiveEvent->SpecialEdit();
		break;
		} // 
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_PPR, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOADCOMPONENT:
		{
		if (ActiveEvent)
			ActiveEvent->OpenGallery(EffectsHost);
		break;
		} //
	case IDC_SAVECOMPONENT:
		{
		if (ActiveEvent)
			ActiveEvent->OpenBrowseData(EffectsHost);
		break;
		} //
	case IDC_ADDPROC:
		{
		AddEvent();
		break;
		} // IDC_ADDPROC
	case IDC_REMOVEPROC:
		{
		RemoveEvent();
		break;
		} // IDC_REMOVEPROC
	case IDC_MOVEPROCUP:
		{
		ChangeEventListPosition(1);
		break;
		} // IDC_MOVEPROCUP
	case IDC_MOVEPROCDOWN:
		{
		ChangeEventListPosition(0);
		break;
		} // IDC_MOVEPROCDOWN
	case IDC_EDITIMAGE:
		{
		EditImage();
		break;
		} // IDC_EDITIMAGE
	default:
		break;
	} // ButtonID

return(0);

} // PostProcEditGUI::HandleButtonClick

/*===========================================================================*/

long PostProcEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL:
		{
		OpenPreview();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // PostProcEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long PostProcEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	case IDC_EVENTNAME:
		{
		EventName();
		break;
		} // 
	case IDC_OVERLAYTEXT:
		{
		OverlayText();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // PostProcEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long PostProcEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(017, 17);
switch (CtrlID)
	{
	case IDC_PROCLIST:
		{
		SetActiveEvent();
		break;
		} // IDC_PROCLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // PostProcEditGUI::HandleListSel

/*===========================================================================*/

long PostProcEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_PROCLIST:
		{
		RemoveEvent();
		break;
		} // IDC_PROCLIST
	default:
		break;
	} // switch

return(0);

} // PostProcEditGUI::HandleListDelItem

/*===========================================================================*/

long PostProcEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_PROCLIST:
		{
		if (ActiveEvent)
			{
			ActiveEvent->Enabled = 1 - ActiveEvent->Enabled;
			Changes[0] = MAKE_ID(ActiveEvent->GetNotifyClass(), ActiveEvent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEvent->GetRAHostRoot());
			} // if
		break;
		} // IDC_PROCLIST
	case IDC_INSERTLIST:
		{
		HWND Wid;
		int LabelInsert;
		if((LabelInsert = WidgetLBGetCurSel(IDC_INSERTLIST)) != -1)
			{
			if(Wid = GetWidgetFromID(IDC_OVERLAYTEXT))
				{
				SendMessage(Wid, EM_REPLACESEL, 1, (LPARAM)PPT_TextSymbols[LabelInsert]);
				OverlayText();
				} // if
			} // if
		break;
		} // INSERTLIST
	default:
		break;
	} // switch CtrlID

return (0);

} // PostProcEditGUI::HandleListDoubleClick

/*===========================================================================*/

long PostProcEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
PostProcHeading *ppg;
long current;

switch (CtrlID)
	{
	case IDC_EVENTTYPEDROP:
		{
		SelectNewEventType();
		break;
		}
	case IDC_IMAGEDROP:
		{
		SelectNewImage();
		break;
		}
	case IDC_IMAGEDROP2:
		{
		SelectNewCompositeImage();
		break;
		}
	case IDC_PPHEADING_DROP_LETTERS:
		current = WidgetCBGetCurSel(IDC_PPHEADING_DROP_LETTERS);
		ppg = (PostProcHeading *)ActiveEvent;
		ppg->sel_letters = current;
		break;
	case IDC_PPHEADING_DROP_NUMBERS:
		current = WidgetCBGetCurSel(IDC_PPHEADING_DROP_NUMBERS);
		ppg = (PostProcHeading *)ActiveEvent;
		ppg->sel_numbers = current;
		break;
	case IDC_PPHEADING_DROP_NUMSTYLE:
		current = WidgetCBGetCurSel(IDC_PPHEADING_DROP_NUMSTYLE);
		ppg = (PostProcHeading *)ActiveEvent;
		ppg->sel_numstyle = current;
		break;
	case IDC_PPHEADING_DROP_TICKS:
		current = WidgetCBGetCurSel(IDC_PPHEADING_DROP_TICKS);
		ppg = (PostProcHeading *)ActiveEvent;
		ppg->sel_ticks = current;
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // PostProcEditGUI::HandleCBChange

/*===========================================================================*/

long PostProcEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		return(HandleDetailPageChange(NewPageID));
		} // 
	default:
		break;
	} // switch

return (0);

} // PostProcEditGUI::HandlePageChange

/*===========================================================================*/

long PostProcEditGUI::HandleDetailPageChange(long NewPageID)
{

switch (NewPageID)
	{
	case 0:
		{
		ShowPanel(0, 0);
		break;
		} // 1
	case 1:
		{
		if (ActiveEvent)
			{
			switch (ActiveEvent->GetType())
				{
				case WCS_POSTPROCEVENT_TYPE_IMAGE:
				case WCS_POSTPROCEVENT_TYPE_TEXT:
					{
					ShowPanel(0, 2);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_STAR:
					{
					ShowPanel(0, 4);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_GLOW:
					{
					ShowPanel(0, 5);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_HALO:
					{
					ShowPanel(0, 6);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_COMPOSITE:
					{
					ShowPanel(0, 7);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_EDGEINK:
					{
					ShowPanel(0, 8);
					break;
					} // 
				case WCS_POSTPROCEVENT_TYPE_HEADING:
					{
					ShowPanel(0, 9);
					break;
					} // 
				default:
					{
					ShowPanel(0, 1);
					break;
					} //
				} // switch
			} // if
		else
			{
			ShowPanel(0, 1);
			} // else
		break;
		} // 1
	case 2:
		{
		if (ActiveEvent)
			{
			switch (ActiveEvent->GetType())
				{
				case WCS_POSTPROCEVENT_TYPE_TEXT:
					{
					ShowPanel(0, 3);
					break;
					} // 
				default:
					{
					ShowPanel(0, 1);
					break;
					} //
				} // switch
			} // if
		else
			{
			ShowPanel(0, 1);
			} // else
		break;
		} // 2
	default:
		{
		ShowPanel(0, 0);
		NewPageID = 0;
		break;
		} // 0
	} // switch

ActivePage = NewPageID;

return(0);

} // PostProcEditGUI::HandleDetailPageChange

/*===========================================================================*/

long PostProcEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKPREVIEWENABLED:
		{
		UpdatePreviews();
		break;
		} // 
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKEVENTENABLED:
		{
		if (ActiveEvent)
			{
			Changes[0] = MAKE_ID(ActiveEvent->GetNotifyClass(), ActiveEvent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEvent->GetRAHostRoot());
			} // if
		break;
		} // IDC_CHECKEVENTENABLED
	case IDC_CHECKAFFECTRED:
	case IDC_CHECKAFFECTGRN:
	case IDC_CHECKAFFECTBLU:
	case IDC_CHECKBEFOREREFL:
	case IDC_CHECKILLUMINATE:
	case IDC_CHECKRECEIVESHADOWS:
	case IDC_CHECKATMOSPHERE:
		{
		if (ActiveEvent)
			{
			Changes[0] = MAKE_ID(ActiveEvent->GetNotifyClass(), ActiveEvent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveEvent->GetRAHostRoot());
			} // if
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PostProcEditGUI::HandleSCChange

/*===========================================================================*/

long PostProcEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	//case IDC_RADIOIMAGE:
	//case IDC_RADIOPROCEDURAL:
	case IDC_RADIORGB:
	case IDC_RADIOHSV:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PostProcEditGUI::HandleSRChange

/*===========================================================================*/

long PostProcEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

//switch (CtrlID)
	{
	// any of the floatints do the same thing
	//default:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		//break;
		} // 
	} // switch CtrlID

return(0);

} // PostProcEditGUI::HandleFIChange


/*===========================================================================*/

void PostProcEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

if (PreviewEnabled)
	{
	UpdatePreviews();
	} // if

} // PostProcEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void PostProcEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Post Process Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKBEFOREREFL, &Active->BeforeReflection, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKPREVIEWENABLED, &PreviewEnabled, SCFlag_Char, NULL, 0);

ConfigureTB(NativeWin, IDC_LOADCOMPONENT, IDI_GALLERY, NULL);
ConfigureTB(NativeWin, IDC_SAVECOMPONENT, IDI_FILESAVE, NULL);
ConfigureTB(NativeWin, IDC_ADDPROC, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEPROC, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEPROCUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEPROCDOWN, IDI_ARROWDOWN, NULL);

ConfigureEvent();
DisableWidgets();

} // PostProcEditGUI::ConfigureWidgets()

/*===========================================================================*/

void PostProcEditGUI::ConfigureEvent(void)
{
AnimDoubleTime *ActivePar;
char *ParLabel, Value1Displayed = 0;

if (ActiveEvent = BuildEventList())
	{
	WidgetSetModified(IDC_EVENTNAME, FALSE);
	WidgetSetText(IDC_EVENTNAME, ActiveEvent->Name);
	WidgetSetDisabled(IDC_INTENSITY, 0);
	WidgetSetDisabled(IDC_EVENTNAME, 0);
	WidgetSetDisabled(IDC_CHECKEVENTENABLED, 0);
	WidgetSetDisabled(IDC_CHECKAFFECTRED, 0);
	WidgetSetDisabled(IDC_CHECKAFFECTGRN, 0);
	WidgetSetDisabled(IDC_CHECKAFFECTBLU, 0);
	WidgetSetDisabled(IDC_EVENTTYPEDROP, 0);
	WidgetSetDisabled(IDC_LOADCOMPONENT, 0);
	WidgetSetDisabled(IDC_SAVECOMPONENT, 0);
	//WidgetSetDisabled(IDC_RADIOIMAGE, 0);
	//WidgetSetDisabled(IDC_RADIOPROCEDURAL, 0);
	WidgetSetDisabled(IDC_COORDSTEXT, 0);
	WidgetSetDisabled(IDC_RADIORGB, 0);
	WidgetSetDisabled(IDC_RADIOHSV, 0);
	ConfigureSC(NativeWin, IDC_CHECKEVENTENABLED, &ActiveEvent->Enabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKAFFECTRED, &ActiveEvent->AffectRed, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKAFFECTGRN, &ActiveEvent->AffectGrn, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKAFFECTBLU, &ActiveEvent->AffectBlu, SCFlag_Char, NULL, 0);

	//ConfigureSR(NativeWin, IDC_RADIOIMAGE, IDC_RADIOIMAGE, &ActiveEvent->TexCoordType, SRFlag_Char, WCS_POSTPROCEVENT_COORDTYPE_IMAGE, NULL, NULL);
	//ConfigureSR(NativeWin, IDC_RADIOIMAGE, IDC_RADIOPROCEDURAL, &ActiveEvent->TexCoordType, SRFlag_Char, WCS_POSTPROCEVENT_COORDTYPE_PROCEDURAL, NULL, NULL);

	ConfigureSR(NativeWin, IDC_RADIORGB, IDC_RADIORGB, &ActiveEvent->RGBorHSV, SRFlag_Char, WCS_POSTPROCEVENT_RGBORHSV_RGB, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIORGB, IDC_RADIOHSV, &ActiveEvent->RGBorHSV, SRFlag_Char, WCS_POSTPROCEVENT_RGBORHSV_HSV, NULL, NULL);

	if (ActiveEvent->RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
		{
		WidgetSetText(IDC_CHECKAFFECTRED, "Hue");
		WidgetSetText(IDC_CHECKAFFECTGRN, "Satur.");
		WidgetSetText(IDC_CHECKAFFECTBLU, "Value");
		} // if
	else
		{
		WidgetSetText(IDC_CHECKAFFECTRED, "Red");
		WidgetSetText(IDC_CHECKAFFECTGRN, "Green");
		WidgetSetText(IDC_CHECKAFFECTBLU, "Blue");
		} // else

	WidgetSmartRAHConfig(IDC_INTENSITY, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_INTENSITY], ActiveEvent);

	WidgetCBSetCurSel(IDC_EVENTTYPEDROP, ActiveEvent->GetType());

	if (ActiveEvent->UseColor(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_LIGHTCOLOR, 1);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, &ActiveEvent->Color, ActiveEvent);
		WidgetSetText(IDC_LIGHTCOLOR, ParLabel);
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetShow(IDC_FILTER, 0);
		} // if
	else if (ActiveEvent->UseFilter(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_FILTER, 1);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)ActiveEvent->GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_FILTER), ActiveEvent);
		WidgetSetText(IDC_FILTER, ParLabel);
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		} // if
	else if (ActiveEvent->UseValue1(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_VALUE, 1);
		WidgetSmartRAHConfig(IDC_VALUE, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1], ActiveEvent);
		WidgetSetText(IDC_VALUE, ParLabel);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
		WidgetShow(IDC_FILTER, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		Value1Displayed = 1;
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
		WidgetShow(IDC_FILTER, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		} // else

	if (Value1Displayed && ActiveEvent->UseValue2(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_VALUE2, 1);
		WidgetSmartRAHConfig(IDC_VALUE2, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2], ActiveEvent);
		WidgetSetText(IDC_VALUE2, ParLabel);
		} // if
	else if (ActiveEvent->FetchADTPtr(ActivePar, ParLabel) && ActivePar && ParLabel)
		{
		WidgetShow(IDC_VALUE2, 1);
		WidgetSmartRAHConfig(IDC_VALUE2, ActivePar, ActivePar->RAParent);
		WidgetSetText(IDC_VALUE2, ParLabel);
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_VALUE2, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE2, 0);
		} // else
	ConfigureSpecificEvent(ActiveEvent);
	} // if
else
	{
	WidgetSetText(IDC_EVENTNAME, "");
	WidgetSmartRAHConfig(IDC_INTENSITY, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_VALUE2, (RasterAnimHost *)NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKEVENTENABLED, NULL, 0, NULL, 0);
	WidgetCBSetCurSel(IDC_EVENTTYPEDROP, -1);
	WidgetSetDisabled(IDC_INTENSITY, 1);
	WidgetShow(IDC_FILTER, 0);
	WidgetShow(IDC_VALUE, 0);
	WidgetShow(IDC_VALUE2, 0);
	WidgetSetDisabled(IDC_EVENTNAME, 1);
	WidgetSetDisabled(IDC_CHECKEVENTENABLED, 1);
	WidgetSetDisabled(IDC_CHECKAFFECTRED, 1);
	WidgetSetDisabled(IDC_CHECKAFFECTGRN, 1);
	WidgetSetDisabled(IDC_CHECKAFFECTBLU, 1);
	//WidgetSetDisabled(IDC_RADIOIMAGE, 1);
	//WidgetSetDisabled(IDC_RADIOPROCEDURAL, 1);
	WidgetSetDisabled(IDC_COORDSTEXT, 1);
	WidgetSetDisabled(IDC_EVENTTYPEDROP, 1);
	WidgetSetDisabled(IDC_LOADCOMPONENT, 1);
	WidgetSetDisabled(IDC_SAVECOMPONENT, 1);
	WidgetSetDisabled(IDC_RADIORGB, 1);
	WidgetSetDisabled(IDC_RADIOHSV, 1);
	ConfigureSpecificEvent(NULL);
	} // else

} // PostProcEditGUI::ConfigureEvent

/*===========================================================================*/

void PostProcEditGUI::ConfigureSpecificEvent(PostProcessEvent *ConfigEvent)
{
Raster *MyRast, *TestRast;
PostProcImage *PPI;
PostProcEdgeInk *PPEI;
int Pos;
long ListPos, Ct, NumEntries;
char TextStr[256];

if (ConfigEvent)
	{
	WidgetShow(IDC_IMAGEDROP, (ConfigEvent->GetType() == WCS_POSTPROCEVENT_TYPE_IMAGE));
	WidgetSetDisabled(IDC_IMAGEDROP2, !(ConfigEvent->GetType() == WCS_POSTPROCEVENT_TYPE_COMPOSITE));
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
	switch (ConfigEvent->GetType())
		{
		case WCS_POSTPROCEVENT_TYPE_COMPOSITE:
			{
			WidgetCBClear(IDC_IMAGEDROP2);
			WidgetCBInsert(IDC_IMAGEDROP2, -1, "New Image Object...");
			for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
				{
				Pos = WidgetCBInsert(IDC_IMAGEDROP2, -1, MyRast->GetUserName());
				WidgetCBSetItemData(IDC_IMAGEDROP2, Pos, MyRast);
				} // for

			if (((PostProcComposite *)ConfigEvent)->Img && (MyRast = ((PostProcComposite *)ConfigEvent)->Img->GetRaster()))
				{
				ListPos = -1;
				NumEntries = WidgetCBGetCount(IDC_IMAGEDROP2);
				for (Ct = 0; Ct < NumEntries; Ct ++)
					{
					if ((TestRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP2, Ct)) != (Raster *)LB_ERR && TestRast == MyRast)
						{
						ListPos = Ct;
						break;
						} // for
					} // for
				WidgetCBSetCurSel(IDC_IMAGEDROP2, ListPos);
				ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MyRast);
				sprintf(TextStr, "%d", MyRast->GetWidth());
				WidgetSetText(IDC_IMAGEWIDTH, TextStr);
				sprintf(TextStr, "%d", MyRast->GetHeight());
				WidgetSetText(IDC_IMAGEHEIGHT, TextStr);
				sprintf(TextStr, "%d", (MyRast->GetIsColor() ? 3: 1));
				if (MyRast->GetAlphaStatus())
					strcat(TextStr, " + Alpha");
				WidgetSetText(IDC_IMAGECOLORS, TextStr);
				} // if
			else
				{
				WidgetSetText(IDC_IMAGEWIDTH, "");
				WidgetSetText(IDC_IMAGEHEIGHT, "");
				WidgetSetText(IDC_IMAGECOLORS, "");
				WidgetCBSetCurSel(IDC_IMAGEDROP2, -1);
				} // else
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_IMAGE:
			{
			WidgetSetText(IDC_DROPLABEL, "Image");
			WidgetShow(IDC_DROPLABEL, 1);
			PPI = (PostProcImage *)ConfigEvent;
			WidgetSmartRAHConfig(IDC_CENTERX, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERX], ActiveEvent);
			WidgetSmartRAHConfig(IDC_CENTERY, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_CENTERY], ActiveEvent);
			WidgetSmartRAHConfig(IDC_WIDTH, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_WIDTH], ActiveEvent);
			WidgetSmartRAHConfig(IDC_HEIGHT, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_HEIGHT], ActiveEvent);
			WidgetSmartRAHConfig(IDC_ZDEPTH, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_ZDISTANCE], ActiveEvent);
			WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_BACKLIGHT], ActiveEvent);
			WidgetSmartRAHConfig(IDC_SHADOWINTENS, &ActiveEvent->AnimPar[WCS_IMAGEOVERLAY_ANIMPAR_SHADOWINTENSITY], ActiveEvent);
			WidgetShow(IDC_KERNING, 0);
			WidgetShow(IDC_LEADING, 0);
			WidgetShow(IDC_OUTLINE, 0);

			ConfigureSC(NativeWin, IDC_CHECKILLUMINATE, &((PostProcText *)ActiveEvent)->Illuminate, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, &((PostProcText *)ActiveEvent)->ApplyVolumetrics, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS, &((PostProcText *)ActiveEvent)->ReceiveShadows, SCFlag_Char, NULL, 0);

			WidgetCBClear(IDC_IMAGEDROP);
			WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
			for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
				{
				Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
				WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
				} // for

			if(ConfigEvent->TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER] && ConfigEvent->TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Enabled
			 && ConfigEvent->TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex && ConfigEvent->TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img
			 && (MyRast = ConfigEvent->TexRoot[WCS_POSTPROCEVENT_TEXTURE_FILTER]->Tex->Img->GetRaster()))
				{
				ListPos = -1;
				NumEntries = WidgetCBGetCount(IDC_IMAGEDROP);
				for (Ct = 0; Ct < NumEntries; Ct ++)
					{
					if ((TestRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Ct)) != (Raster *)LB_ERR && TestRast == MyRast)
						{
						ListPos = Ct;
						break;
						} // for
					} // for
				WidgetCBSetCurSel(IDC_IMAGEDROP, ListPos);
				} // if
			else
				{
				WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
				} // else
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_TEXT:
			{
			int LabelInsert;
			char TempLabelFormat[100];

			WidgetSetText(IDC_DROPLABEL, "Font");
			WidgetShow(IDC_DROPLABEL, 0);
			WidgetShow(IDC_KERNING, 1);
			WidgetShow(IDC_LEADING, 1);
			WidgetShow(IDC_OUTLINE, 1);
			WidgetSmartRAHConfig(IDC_CENTERX, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERX], ActiveEvent);
			WidgetSmartRAHConfig(IDC_CENTERY, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_CENTERY], ActiveEvent);
			WidgetSmartRAHConfig(IDC_WIDTH, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_WIDTH], ActiveEvent);
			WidgetSmartRAHConfig(IDC_HEIGHT, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_HEIGHT], ActiveEvent);
			WidgetSmartRAHConfig(IDC_ZDEPTH, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_ZDISTANCE], ActiveEvent);
			WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_BACKLIGHT], ActiveEvent);
			WidgetSmartRAHConfig(IDC_SHADOWINTENS, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_SHADOWINTENSITY], ActiveEvent);
			WidgetSmartRAHConfig(IDC_KERNING, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_KERNING], ActiveEvent);
			WidgetSmartRAHConfig(IDC_LEADING, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_LEADING], ActiveEvent);
			WidgetSmartRAHConfig(IDC_OUTLINE, &ActiveEvent->AnimPar[WCS_TEXTOVERLAY_ANIMPAR_OUTLINE], ActiveEvent);

			ConfigureSC(NativeWin, IDC_CHECKILLUMINATE, &((PostProcText *)ActiveEvent)->Illuminate, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, &((PostProcText *)ActiveEvent)->ApplyVolumetrics, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS, &((PostProcText *)ActiveEvent)->ReceiveShadows, SCFlag_Char, NULL, 0);

			WidgetSetModified(IDC_OVERLAYTEXT, FALSE);
			WidgetSetText(IDC_OVERLAYTEXT, ((PostProcText *)ActiveEvent)->MesgText);

			WidgetLBClear(IDC_INSERTLIST);
			for(LabelInsert = 0; PPT_TextLabels[LabelInsert]; LabelInsert++)
				{
				if(PPT_TextSymbols[LabelInsert]) // safegaurd
					{
					sprintf(TempLabelFormat, "%s %s", PPT_TextLabels[LabelInsert], PPT_TextSymbols[LabelInsert]);
					WidgetLBInsert(IDC_INSERTLIST, -1, TempLabelFormat);
					} // if
				} // for

			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_STAR:
			{
			WidgetSmartRAHConfig(IDC_STARCOLOR, &ActiveEvent->Color, ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARAMOUNT, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1], ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARRADIUS, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2], ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARNUMPOINTS, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE3], ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARSQUEEZE, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE4], ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARROTATION, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE6], ActiveEvent);
			WidgetSmartRAHConfig(IDC_STARSHARPNESS, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE7], ActiveEvent);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_GLOW:
			{
			WidgetSmartRAHConfig(IDC_GLOWCOLOR, &ActiveEvent->Color, ActiveEvent);
			WidgetSmartRAHConfig(IDC_GLOWAMOUNT, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1], ActiveEvent);
			WidgetSmartRAHConfig(IDC_GLOWRADIUS, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2], ActiveEvent);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_HALO:
			{
			WidgetSmartRAHConfig(IDC_HALOCOLOR, &ActiveEvent->Color, ActiveEvent);
			WidgetSmartRAHConfig(IDC_HALOAMOUNT, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1], ActiveEvent);
			WidgetSmartRAHConfig(IDC_HALORADIUS, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2], ActiveEvent);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_EDGEINK:
			{
			PPEI = (PostProcEdgeInk *)ConfigEvent;
			WidgetFIConfig(IDC_DISTNEAR, &PPEI->EdgeInkParams[0].Distance, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_DISTDIFFNEAR, &PPEI->EdgeInkParams[0].DistDiff, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_NORMDIFFNEAR, &PPEI->EdgeInkParams[0].NormDiff, 1.0, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKWEIGHTNEAR, &PPEI->EdgeInkParams[0].InkWeight, 0.1, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKCOLORNEAR, &PPEI->EdgeInkParams[0].InkColor, 0.1, 0.0, 1.0, FIOFlag_Double);

			WidgetFIConfig(IDC_DISTMID, &PPEI->EdgeInkParams[1].Distance, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_DISTDIFFMID, &PPEI->EdgeInkParams[1].DistDiff, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_NORMDIFFMID, &PPEI->EdgeInkParams[1].NormDiff, 1.0, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKWEIGHTMID, &PPEI->EdgeInkParams[1].InkWeight, 0.1, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKCOLORMID, &PPEI->EdgeInkParams[1].InkColor, 0.1, 0.0, 1.0, FIOFlag_Double);

			WidgetFIConfig(IDC_DISTFAR, &PPEI->EdgeInkParams[2].Distance, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_DISTDIFFFAR, &PPEI->EdgeInkParams[2].DistDiff, 1.0, 0.0, 1000000.0, FIOFlag_Double);
			WidgetFIConfig(IDC_NORMDIFFFAR, &PPEI->EdgeInkParams[2].NormDiff, 1.0, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKWEIGHTFAR, &PPEI->EdgeInkParams[2].InkWeight, 0.1, 0.0, 1.0, FIOFlag_Double);
			WidgetFIConfig(IDC_INKCOLORFAR, &PPEI->EdgeInkParams[2].InkColor, 0.1, 0.0, 1.0, FIOFlag_Double);
			break;
			} // WCS_POSTPROCEVENT_TYPE_EDGEINK
		case WCS_POSTPROCEVENT_TYPE_HEADING:
			{
			WidgetCBClear(IDC_PPHEADING_DROP_LETTERS);
			WidgetCBInsert(IDC_PPHEADING_DROP_LETTERS, -1, "Cardinals");
			WidgetCBInsert(IDC_PPHEADING_DROP_LETTERS, -1, "& Ordinals");		// default
			WidgetCBInsert(IDC_PPHEADING_DROP_LETTERS, -1, "& Interordinals");
			WidgetCBSetCurSel(IDC_PPHEADING_DROP_LETTERS, ((PostProcHeading *)ActiveEvent)->sel_letters);

			WidgetCBClear(IDC_PPHEADING_DROP_NUMBERS);
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMBERS, -1, "90 Degrees");
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMBERS, -1, "45 Degrees");
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMBERS, -1, "30 Degrees");	// default
			WidgetCBSetCurSel(IDC_PPHEADING_DROP_NUMBERS, ((PostProcHeading *)ActiveEvent)->sel_numbers);

			WidgetCBClear(IDC_PPHEADING_DROP_NUMSTYLE);
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMSTYLE, -1, "090");	// default
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMSTYLE, -1, "90");
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMSTYLE, -1, "09");
			WidgetCBInsert(IDC_PPHEADING_DROP_NUMSTYLE, -1, "9");
			WidgetCBSetCurSel(IDC_PPHEADING_DROP_NUMSTYLE, ((PostProcHeading *)ActiveEvent)->sel_numstyle);

			WidgetCBClear(IDC_PPHEADING_DROP_TICKS);
			WidgetCBInsert(IDC_PPHEADING_DROP_TICKS, -1, "45 Degrees");
			WidgetCBInsert(IDC_PPHEADING_DROP_TICKS, -1, "30 Degrees");
			WidgetCBInsert(IDC_PPHEADING_DROP_TICKS, -1, "10 Degrees");
			WidgetCBInsert(IDC_PPHEADING_DROP_TICKS, -1, " 5 Degrees");	// default
			WidgetCBSetCurSel(IDC_PPHEADING_DROP_TICKS, ((PostProcHeading *)ActiveEvent)->sel_ticks);

			ConfigureSC(NativeWin, IDC_PPHEADING_1DEGREE, &((PostProcHeading *)ActiveEvent)->scale1, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_PPHEADING_5DEGREE, &((PostProcHeading *)ActiveEvent)->scale5, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_PPHEADING_LETTERS, &((PostProcHeading *)ActiveEvent)->letters, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_PPHEADING_NUMBERS, &((PostProcHeading *)ActiveEvent)->numbers, SCFlag_Char, NULL, 0);
			ConfigureSC(NativeWin, IDC_PPHEADING_TICKS, &((PostProcHeading *)ActiveEvent)->ticks, SCFlag_Char, NULL, 0);
			break;
			} // WCS_POSTPROCEVENT_TYPE_HEADING
		default:
			break;
		} // switch
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_CENTERX, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_CENTERY, (RasterAnimHost **)NULL, NULL);
	WidgetSmartRAHConfig(IDC_WIDTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_HEIGHT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ZDEPTH, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_ORIENTATIONSHADING, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_SHADOWINTENS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_KERNING, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_LEADING, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_OUTLINE, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARCOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARAMOUNT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARRADIUS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARNUMPOINTS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARSQUEEZE, (RasterAnimHost *)NULL, NULL);
	//WidgetSmartRAHConfig(IDC_STARSHARPNESS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_STARROTATION, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_GLOWCOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_GLOWAMOUNT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_GLOWRADIUS, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_HALOCOLOR, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_HALOAMOUNT, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_HALORADIUS, (RasterAnimHost *)NULL, NULL);
	ConfigureSC(NativeWin, IDC_CHECKILLUMINATE, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS, NULL, 0, NULL, 0);
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
	} // else

} // PostProcEditGUI::ConfigureSpecificEvent

/*===========================================================================*/

PostProcessEvent *PostProcEditGUI::ActiveEventValid(void)
{
PostProcessEvent *CurEvent;

if (ActiveEvent)
	{
	// check to see if active foliage group is valid
	CurEvent = Active->Events;
	while (CurEvent)
		{
		if (CurEvent == ActiveEvent)
			{
			break;
			} // if found
		CurEvent = CurEvent->Next;
		} // while
	if (! CurEvent)
		{
		ActiveEvent = NULL;
		} // if
	} // if
if (! ActiveEvent)
	{
	ActiveEvent = Active->Events;
	} // if

return (ActiveEvent);

} // PostProcEditGUI::ActiveEventValid

/*===========================================================================*/

PostProcessEvent *PostProcEditGUI::BuildEventList(void)
{
long Found = 0, Pos;
PostProcessEvent *CurEvent = Active->Events;
char ListName[WCS_POSTPROC_MAXNAMELENGTH + 4];

WidgetLBClear(IDC_PROCLIST);

while (CurEvent)
	{
	BuildEventListEntry(ListName, CurEvent);
	Pos = WidgetLBInsert(IDC_PROCLIST, -1, ListName);
	WidgetLBSetItemData(IDC_PROCLIST, Pos, CurEvent);
	if (CurEvent == ActiveEvent)
		{
		Found = 1;
		WidgetLBSetCurSel(IDC_PROCLIST, Pos);
		} // if
	CurEvent = CurEvent->Next;
	} // while

if (! Found)
	{
	if (ActiveEvent = Active->Events)
		{
		WidgetLBSetCurSel(IDC_PROCLIST, 0);
		}
	} // if

return (ActiveEvent);

} // PostProcEditGUI::BuildEventList

/*===========================================================================*/

void PostProcEditGUI::BuildEventListEntry(char *ListName, PostProcessEvent *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->Name);

} // PostProcEditGUI::BuildEventListEntry()

/*===========================================================================*/

void PostProcEditGUI::SyncWidgets(void)
{
AnimDoubleTime *ActivePar;
char *ParLabel, Value1Displayed = 0;

if (ActiveEvent)
	{
	WidgetSNSync(IDC_INTENSITY, WP_FISYNC_NONOTIFY);
	if (ActiveEvent->UseColor(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_LIGHTCOLOR, 1);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, &ActiveEvent->Color, ActiveEvent);
		WidgetSetText(IDC_LIGHTCOLOR, ParLabel);
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetShow(IDC_FILTER, 0);
		} // if
	else if (ActiveEvent->UseFilter(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_FILTER, 1);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)ActiveEvent->GetTexRootPtrAddr(WCS_POSTPROCEVENT_TEXTURE_FILTER), ActiveEvent);
		WidgetSetText(IDC_FILTER, ParLabel);
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		} // if
	else if (ActiveEvent->UseValue1(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_VALUE, 1);
		WidgetSmartRAHConfig(IDC_VALUE, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE1], ActiveEvent);
		WidgetSetText(IDC_VALUE, ParLabel);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
		WidgetShow(IDC_FILTER, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		Value1Displayed = 1;
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE, 0);
		WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
		WidgetShow(IDC_FILTER, 0);
		WidgetSmartRAHConfig(IDC_LIGHTCOLOR, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_LIGHTCOLOR, 0);
		} // else

	if (Value1Displayed && ActiveEvent->UseValue2(ParLabel) && ParLabel)
		{
		WidgetShow(IDC_VALUE2, 1);
		WidgetSmartRAHConfig(IDC_VALUE2, &ActiveEvent->AnimPar[WCS_POSTPROCEVENT_ANIMPAR_VALUE2], ActiveEvent);
		WidgetSetText(IDC_VALUE2, ParLabel);
		} // if
	else if (ActiveEvent->FetchADTPtr(ActivePar, ParLabel) && ActivePar && ParLabel)
		{
		WidgetShow(IDC_VALUE2, 1);
		WidgetSmartRAHConfig(IDC_VALUE2, ActivePar, ActivePar->RAParent);
		WidgetSetText(IDC_VALUE2, ParLabel);
		} // if
	else
		{
		WidgetSmartRAHConfig(IDC_VALUE2, (RasterAnimHost *)NULL, NULL);
		WidgetShow(IDC_VALUE2, 0);
		} // else
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_VALUE, (RasterAnimHost *)NULL, NULL);
	WidgetShow(IDC_VALUE, 0);
	WidgetSmartRAHConfig(IDC_VALUE2, (RasterAnimHost *)NULL, NULL);
	WidgetShow(IDC_VALUE2, 0);
	WidgetSmartRAHConfig(IDC_FILTER, (RasterAnimHost **)NULL, NULL);
	WidgetShow(IDC_FILTER, 0);
	} // else

if (ActiveEvent && ActiveEvent->RGBorHSV == WCS_POSTPROCEVENT_RGBORHSV_HSV)
	{
	WidgetSetText(IDC_CHECKAFFECTRED, "Hue");
	WidgetSetText(IDC_CHECKAFFECTGRN, "Satur.");
	WidgetSetText(IDC_CHECKAFFECTBLU, "Value");
	} // if
else if (ActiveEvent)
	{
	WidgetSetText(IDC_CHECKAFFECTRED, "Red");
	WidgetSetText(IDC_CHECKAFFECTGRN, "Green");
	WidgetSetText(IDC_CHECKAFFECTBLU, "Blue");
	} // else

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKEVENTENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKAFFECTRED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKAFFECTGRN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKAFFECTBLU, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKBEFOREREFL, WP_SCSYNC_NONOTIFY);

//WidgetSRSync(IDC_RADIOIMAGE, WP_SRSYNC_NONOTIFY);
//WidgetSRSync(IDC_RADIOPROCEDURAL, WP_SRSYNC_NONOTIFY);

SyncSpecificWidgets();

} // PostProcEditGUI::SyncWidgets

/*===========================================================================*/

void PostProcEditGUI::SyncSpecificWidgets(void)
{
PostProcImage *PPI;

if (ActiveEvent)
	{
	switch (ActiveEvent->GetType())
		{
		case WCS_POSTPROCEVENT_TYPE_IMAGE:
			{
			PPI = (PostProcImage *)ActiveEvent;
			WidgetSNSync(IDC_CENTERX, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_CENTERY, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_WIDTH, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_HEIGHT, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_ZDEPTH, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_ORIENTATIONSHADING, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_SHADOWINTENS, WP_FISYNC_NONOTIFY);

			WidgetSCSync(IDC_CHECKILLUMINATE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKATMOSPHERE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKRECEIVESHADOWS, WP_SCSYNC_NONOTIFY);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_TEXT:
			{
			WidgetSNSync(IDC_CENTERX, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_CENTERY, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_WIDTH, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_HEIGHT, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_ZDEPTH, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_ORIENTATIONSHADING, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_SHADOWINTENS, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_KERNING, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_LEADING, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_OUTLINE, WP_FISYNC_NONOTIFY);

			WidgetSCSync(IDC_CHECKILLUMINATE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKATMOSPHERE, WP_SCSYNC_NONOTIFY);
			WidgetSCSync(IDC_CHECKRECEIVESHADOWS, WP_SCSYNC_NONOTIFY);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_STAR:
			{
			WidgetSNSync(IDC_STARCOLOR, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_STARAMOUNT, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_STARRADIUS, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_STARNUMPOINTS, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_STARSQUEEZE, WP_FISYNC_NONOTIFY);
			//WidgetSNSync(IDC_STARSHARPNESS, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_STARROTATION, WP_FISYNC_NONOTIFY);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_GLOW:
			{
			WidgetSNSync(IDC_GLOWCOLOR, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_GLOWAMOUNT, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_GLOWRADIUS, WP_FISYNC_NONOTIFY);
			break;
			} // 
		case WCS_POSTPROCEVENT_TYPE_HALO:
			{
			WidgetSNSync(IDC_HALOCOLOR, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_HALOAMOUNT, WP_FISYNC_NONOTIFY);
			WidgetSNSync(IDC_HALORADIUS, WP_FISYNC_NONOTIFY);
			break;
			} // 
		default:
			break;
		} // switch
	} // if

} // PostProcEditGUI::SyncSpecificWidgets

/*===========================================================================*/

void PostProcEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_CHECKBEFOREREFL, ! Active->CheckBeforeReflectionsLegal());

} // PostProcEditGUI::DisableWidgets

/*===========================================================================*/

void PostProcEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // PostProcEditGUI::Cancel

/*===========================================================================*/

void PostProcEditGUI::Name(void)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // PostProcEditGUI::Name()

/*===========================================================================*/

void PostProcEditGUI::EventName(void)
{
char NewName[WCS_POSTPROC_MAXNAMELENGTH];

if (WidgetGetModified(IDC_EVENTNAME))
	{
	WidgetGetText(IDC_EVENTNAME, WCS_POSTPROC_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_EVENTNAME, FALSE);
	ActiveEvent->SetName(NewName);
	} // if 

} // PostProcEditGUI::EventName()

/*===========================================================================*/

void PostProcEditGUI::OverlayText(void)
{
char NewText[WCS_TEXTOVERLAY_MAXLEN];
NotifyTag Changes[2];

if (WidgetGetModified(IDC_OVERLAYTEXT))
	{
	WidgetGetText(IDC_OVERLAYTEXT, WCS_TEXTOVERLAY_MAXLEN, NewText);
	WidgetSetModified(IDC_OVERLAYTEXT, FALSE);
	if (ActiveEvent && ActiveEvent->GetType() == WCS_POSTPROCEVENT_TYPE_TEXT)
		{
		strcpy(((PostProcText *)ActiveEvent)->MesgText, NewText);
		Changes[0] = MAKE_ID(ActiveEvent->GetNotifyClass(), ActiveEvent->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, ActiveEvent->GetRAHostRoot());
		} // if
	} // if 

} // PostProcEditGUI::OverlayText()

/*===========================================================================*/

void PostProcEditGUI::AddEvent(void)
{
PostProcessEvent *NewEvent;

if (NewEvent = Active->AddEvent(NULL))
	{
	ActiveEvent = NewEvent;
	ConfigureEvent();
	} // if

} // PostProcEditGUI::AddEvent

/*===========================================================================*/

void PostProcEditGUI::RemoveEvent(void)
{
PostProcessEvent *EventToRemove;
int RemoveAll = 0;

if (ActiveEventValid())
	{
	EventToRemove = ActiveEvent;
	ActiveEvent = ActiveEvent->Next;
	Active->FindnRemoveRAHostChild(EventToRemove, RemoveAll);
	} // if

} // PostProcEditGUI::RemoveEvent

/*===========================================================================*/

void PostProcEditGUI::ChangeEventListPosition(short MoveUp)
{
long SendNotify = 0;
RasterAnimHost *MoveMe;
PostProcessEvent *Current, *PrevEvent = NULL, *PrevPrevEvent = NULL, *StashEvent;
NotifyTag Changes[2];

// don't send notification until all changes are done
if (ActiveEventValid())
	{
	MoveMe = ActiveEvent;
	if (MoveUp)
		{
		Current = Active->Events;
		while (Current != MoveMe)
			{
			PrevPrevEvent = PrevEvent;
			PrevEvent = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (PrevEvent)
				{
				StashEvent = Current->Next;
				if (PrevPrevEvent)
					{
					PrevPrevEvent->Next = Current;
					Current->Next = PrevEvent;
					} // if
				else
					{
					Active->Events = Current;
					Active->Events->Next = PrevEvent;
					} // else
				PrevEvent->Next = StashEvent;
				SendNotify = 1;
				} // else if
			} // if
		} // if
	else
		{
		Current = Active->Events;
		while (Current != MoveMe)
			{
			PrevPrevEvent = PrevEvent;
			PrevEvent = Current;
			Current = Current->Next;
			} // while
		if (Current)
			{
			if (Current->Next)
				{
				StashEvent = Current->Next->Next;
				if (PrevEvent)
					{
					PrevEvent->Next = Current->Next;
					PrevEvent->Next->Next = Current;
					} // if
				else
					{
					Active->Events = Current->Next;
					Active->Events->Next = Current;
					} // else
				Current->Next = StashEvent;
				SendNotify = 1;
				} // if move down
			} // if
		} // else
	} // if

// need to send a very general message that will cause SAG to completely rebuild
// Just updating the object will cause crash in SAG with NULL pointer
if (SendNotify)
	{
	Changes[0] = MAKE_ID(0xff, Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // PostProcEditGUI::ChangeEventListPosition

/*===========================================================================*/

void PostProcEditGUI::SetActiveEvent(void)
{
long Current;
PostProcessEvent *NewEvent;

Current = WidgetLBGetCurSel(IDC_PROCLIST);
if ((NewEvent = (PostProcessEvent *)WidgetLBGetItemData(IDC_PROCLIST, Current, 0)) != (PostProcessEvent *)LB_ERR && NewEvent)
	{
	if (NewEvent != ActiveEvent)
		{
		ActiveEvent = NewEvent;
		ConfigureEvent();
		} // if
	} // if
else
	ConfigureEvent();

} // PostProcEditGUI::SetActiveEvent

/*===========================================================================*/

void PostProcEditGUI::SelectNewEventType(void)
{
PostProcessEvent *NewEvent;
long Current;

if (ActiveEvent)
	{
	Current = WidgetCBGetCurSel(IDC_EVENTTYPEDROP);
	if (NewEvent = Active->ChangeEventType(ActiveEvent, (unsigned char)Current))
		{
		ActiveEvent = NewEvent;
		ConfigureEvent();
		if (NewEvent->GetType() == WCS_POSTPROCEVENT_TYPE_COMPOSITE && ! Active->BeforeReflection)
			{
			if (UserMessageYN("Composite Warning", "Compositing operations yield best results if performed before reflections. Enable \"Before Reflections\" now?"))
				{
				Active->BeforeReflection = 1;
				WidgetSCSync(IDC_CHECKBEFOREREFL, WP_SCSYNC_NONOTIFY);
				} // if
			} // if
		} // if
	} // if

} // PostProcEditGUI::SelectNewEventType

/*===========================================================================*/

void PostProcEditGUI::SelectNewImage(void)
{
long Current;
Raster *NewRast, *MadeRast = NULL;
PostProcImage *PPI;

if (ActiveEvent && ActiveEvent->GetType() == WCS_POSTPROCEVENT_TYPE_IMAGE)
	{
	Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
	if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)LB_ERR && NewRast)
		|| (MadeRast = NewRast = GlobalApp->AppImages->AddRequestRaster()))
		{
		PPI = (PostProcImage *)ActiveEvent;
		PPI->SetRaster(NewRast);
		if (MadeRast)
			{
			GlobalApp->AppImages->SetActive(MadeRast);
			} // if
		} // if
	} // if

} // PostProcEditGUI::SelectNewImage

/*===========================================================================*/

void PostProcEditGUI::SelectNewCompositeImage(void)
{
long Current;
Raster *NewRast, *MadeRast = NULL;
PostProcComposite *PPC;

if (ActiveEvent && ActiveEvent->GetType() == WCS_POSTPROCEVENT_TYPE_COMPOSITE)
	{
	Current = WidgetCBGetCurSel(IDC_IMAGEDROP2);
	if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP2, Current, 0)) != (Raster *)LB_ERR && NewRast)
		|| (MadeRast = NewRast = GlobalApp->AppImages->AddRequestRaster()))
		{
		PPC = (PostProcComposite *)ActiveEvent;
		PPC->SetRaster(NewRast);
		if (MadeRast)
			{
			GlobalApp->AppImages->SetActive(MadeRast);
			} // if
		} // if
	} // if

} // PostProcEditGUI::SelectNewCompositeImage

/*===========================================================================*/

void PostProcEditGUI::EditImage(void)
{

if (ActiveEvent && ActiveEvent->GetType() == WCS_POSTPROCEVENT_TYPE_COMPOSITE)
	{
	if (((PostProcComposite *)ActiveEvent)->Img && ((PostProcComposite *)ActiveEvent)->Img->GetRaster())
		{
		GlobalApp->AppImages->SetActive(((PostProcComposite *)ActiveEvent)->Img->GetRaster());
		GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
			WCS_TOOLBAR_ITEM_ILG, 0);
		} // if
	} // if

} // PostProcEditGUI::DoEditImage()

/*===========================================================================*/

void PostProcEditGUI::OpenPreview(void)
{

if (ActiveEvent && ActiveEvent->GetType() == WCS_POSTPROCEVENT_TYPE_COMPOSITE)
	{
	if (((PostProcComposite *)ActiveEvent)->Img && ((PostProcComposite *)ActiveEvent)->Img->GetRaster())
		{
		((PostProcComposite *)ActiveEvent)->Img->GetRaster()->OpenPreview(FALSE);
		} // if
	} // if

} // PostProcEditGUI::OpenPreview()

/*===========================================================================*/

void PostProcEditGUI::UpdatePreviews(void)
{
long ViewNum;
Renderer *TempRend;

// find any open renderers that have preview windows
// look in Views
if (GlobalApp->GUIWins->CVG)
	{
	ViewNum = -1;
	while (TempRend = GlobalApp->GUIWins->CVG->FindNextLiveRenderer(ViewNum))
		{
		TempRend->ApplyPostProc(Active);
		} // while
	} // if


// RenderController version is commented out because if exponent buffer is present it should not be 
// overwritten and it WILL be overwritten if it exists and post process is evaluated
//
// look for RenderController
//if (GlobalApp->GUIWins->RCG)
//	{
//	if (TempRend = GlobalApp->GUIWins->RCG->GetRenderHandle())
//		{
//		GlobalApp->GUIWins->RCG->SetRunning(1);
//		TempRend->ApplyPostProc(Active);
//		GlobalApp->GUIWins->RCG->SetRunning(0);
//		} // if renderer
//	} // if

} // PostProcEditGUI::UpdatePreviews
