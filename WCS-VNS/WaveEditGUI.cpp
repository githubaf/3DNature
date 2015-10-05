// WaveEditGUI.cpp
// Code for Wave editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include <stdafx.h>
#include "WaveEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Raster.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "AppMem.h"
#include "ViewGUI.h"
#include "Conservatory.h"
#include "resource.h"
#include "Lists.h"


char *WaveEditGUI::TabNames[WCS_WAVEGUI_NUMTABS] = {"General && Vectors", "Position && Envelope", "Sources", "Source Envelope"};
long WaveEditGUI::ActivePage;
// advanced
long WaveEditGUI::DisplayAdvanced;

NativeGUIWin WaveEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // WaveEditGUI::Open

/*===========================================================================*/

NativeGUIWin WaveEditGUI::Construct(void)
{
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_WAVE_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_WAVE_BASIC, 0, 1);
	CreateSubWinFromTemplate(IDD_WAVE_SOURCELIST, 0, 2);
	CreateSubWinFromTemplate(IDD_WAVE_SOURCEPOS, 1, 0);
	CreateSubWinFromTemplate(IDD_WAVE_SOURCEENVELOPE, 1, 1);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_WAVEGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		SelectPanel(ActivePage);
		ConfigureWidgets();
		UpdateThumbnail();
		} // if
	} // if
 
return (NativeWin);

} // WaveEditGUI::Construct

/*===========================================================================*/

WaveEditGUI::WaveEditGUI(EffectsLib *EffectsSource, Database *DBSource, WaveEffect *ActiveSource)
: GUIFenetre('WVED', this, "Wave Model Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_WAVE, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_LAKE, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_STREAM, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0xff),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
ActiveWave = NULL;
ReceivingDiagnostics = ReceivingWave = BackgroundInstalled = ReceivingWaveSource = 0;
WaveRast = NULL;
WaveSampleData = NULL;
NumEnvelopes = 0;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Wave Model Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	if (! (WaveRast = new Raster()))
		ConstructError = 1;
	if (! (WaveSampleData = new TextureSampleData()))
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // WaveEditGUI::WaveEditGUI

/*===========================================================================*/

WaveEditGUI::~WaveEditGUI()
{

GlobalApp->RemoveBGHandler(this);

if (WaveRast)
	delete WaveRast;
if (WaveSampleData)
	delete WaveSampleData;
GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // WaveEditGUI::~WaveEditGUI()

/*===========================================================================*/

long WaveEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_WEG, 0);

return(0);

} // WaveEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long WaveEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{

DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);

} // WaveEditGUI::HandleShowAdvanced

/*===========================================================================*/

long WaveEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_WEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_EDITPROFILE:
		{
		Active->ADProf.OpenTimeline();
		break;
		} // IDC_EDITPROFILE
	case IDC_EDITENVELOPE:
		{
		if (ActiveWaveValid())
			ActiveWave->Envelope.OpenTimeline();
		break;
		} // IDC_EDITENVELOPE
	case IDC_SETCENTERPOS:
		{
		SetPosInView(NULL);
		break;
		} //
	case IDC_ADDSOURCEINVIEW:
		{
		AddWaveInView(NULL);
		break;
		} //
	case IDC_SETSOURCEINVIEW:
		{
		SetSourcePosInView(NULL);
		break;
		} //
	case IDC_ADDSOURCE:
		{
		AddWave();
		break;
		} // IDC_ADDSOURCE
	case IDC_REMOVESOURCE:
		{
		RemoveWave();
		break;
		} // IDC_REMOVESOURCE
	case IDC_ZOOMIN3:
		{
		ZoomWaveScale(1);
		break;
		} // IDC_ZOOMIN
	case IDC_ZOOMOUT3:
		{
		ZoomWaveScale(-1);
		break;
		} // IDC_ZOOMOUT
	default: break;
	} // ButtonID
return(0);

} // WaveEditGUI::HandleButtonClick

/*===========================================================================*/

long WaveEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(021, 21);
switch (CtrlID)
	{
	case IDC_SOURCELIST:
		{
		SetActiveWave();
		break;
		} // IDC_SOURCELIST
	} // switch CtrlID

return (0);

} // WaveEditGUI::HandleListSel

/*===========================================================================*/

long WaveEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		RemoveWave();
		break;
		} // IDC_SOURCELIST
	} // switch

return(0);

} // WaveEditGUI::HandleListDelItem

/*===========================================================================*/

long WaveEditGUI::HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		CopyWave();
		break;
		} // IDC_SOURCELIST
	} // switch

return(0);

} // WaveEditGUI::HandleListCopyItem

/*===========================================================================*/

long WaveEditGUI::HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_SOURCELIST:
		{
		PasteWave();
		break;
		} // IDC_SOURCELIST
	} // switch

return(0);

} // WaveEditGUI::HandleListPasteItem

/*===========================================================================*/

long WaveEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_NAME:
		{
		Name();
		break;
		} // 
	} // switch CtrlID

return (0);

} // WaveEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long WaveEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				SelectPanel(1);
				break;
				} // 0
			case 2:
				{
				SelectPanel(2);
				break;
				} // 0
			case 3:
				{
				SelectPanel(3);
				break;
				} // 0
			default:
				{
				SelectPanel(0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

ActivePage = NewPageID;

return(0);

} // WaveEditGUI::HandlePageChange

/*===========================================================================*/

long WaveEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_CHECKENABLED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_SOURCEENABLED:
		{
		if (ActiveWaveValid())
			{
			Changes[0] = MAKE_ID(ActiveWave->GetNotifyClass(), ActiveWave->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveWave->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_CHECKGRADFILL:
	case IDC_CHECKRANDOMSTART:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKREPEATBEFORE:
	case IDC_CHECKREPEATAFTER:
		{
		if (ActiveWaveValid())
			{
			Changes[0] = MAKE_ID(ActiveWave->GetNotifyClass(), ActiveWave->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveWave->GetRAHostRoot());
			} // if
		break;
		} // 
	case IDC_CHECKENVENABLED:
		{
		if (ActiveWaveValid())
			{
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), ActiveWave->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, ActiveWave->GetRAHostRoot());
			} // if
		break;
		} // 
	} // switch CtrlID

return(0);

} // WaveEditGUI::HandleSCChange

/*===========================================================================*/

long WaveEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIORANDOMTYPE:
	case IDC_RADIOCLUSTEREDTYPE:
	case IDC_RADIOAREATYPE:
	case IDC_RADIOPOINTTYPE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // WaveEditGUI::HandleSRChange

/*===========================================================================*/

long WaveEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_SOURCESPERVERTEX:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_SOURCEAMP:
		{
		BuildWaveList();
		break;
		} // 
	} // switch CtrlID

return(0);

} // WaveEditGUI::HandleFIChange

/*===========================================================================*/

void WaveEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long UpdateThumbs = 0, Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, WCS_DIAGNOSTIC_ITEM_MOUSEDOWN, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	if (ReceivingDiagnostics)
		SetPosInView((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	else if (ReceivingWave)
		AddWaveInView((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	else if (ReceivingWaveSource)
		SetSourcePosInView((DiagnosticData *)Activity->ChangeNotify->NotifyData);
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[5] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	DisableWidgets();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	UpdateThumbs = 1;
	Done = 1;
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	DisableWidgets();
	UpdateThumbs = 1;
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

if (! Done)
	{
	ConfigureWidgets();
	UpdateThumbnail();
	} // if
else if (UpdateThumbs)
	UpdateThumbnail();

} // WaveEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void WaveEditGUI::ConfigureWidgets(void)
{
double Width;
WaveSource *CurSource;
long Ct;
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

NumEnvelopes = 0;
if (CurSource = Active->WaveSources)
	{
	while (CurSource)
		{
		if (CurSource->EnvelopeEnabled && CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue != 0.0)
			NumEnvelopes ++;
		CurSource = CurSource->Next;
		} // while
	} // if
if (NumEnvelopes > 1)
	sprintf(TextStr, "There are %d Enabled Motion Envelopes.", NumEnvelopes);
else if (NumEnvelopes == 1)
	strcpy(TextStr, "There is one Enabled Motion Envelope.");
else
	strcpy(TextStr, "There are no Enabled Motion Envelopes.");
WidgetSetText(IDC_ENVSEXIST, TextStr);

sprintf(TextStr, "Wave Model Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGRADFILL, &Active->UseGradient, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRANDOMSTART, &Active->RandomizeEnvStart, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIORANDOMTYPE, IDC_RADIORANDOMTYPE, &Active->RandomizeType, SRFlag_Char, WCS_WAVE_RANDOMIZETYPE_POISSON, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORANDOMTYPE, IDC_RADIOCLUSTEREDTYPE, &Active->RandomizeType, SRFlag_Char, WCS_WAVE_RANDOMIZETYPE_GAUSS, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOAREATYPE, IDC_RADIOAREATYPE, &Active->WaveEffectType, SRFlag_Char, WCS_WAVE_WAVETYPE_AREA, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAREATYPE, IDC_RADIOPOINTTYPE, &Active->WaveEffectType, SRFlag_Char, WCS_WAVE_WAVETYPE_POINT, NULL, NULL);

//WidgetSNConfig(IDC_LAT, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE]);
//WidgetSNConfig(IDC_LON, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE]);
//WidgetSNConfig(IDC_AMP, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE]);
WidgetSNConfig(IDC_STARTRANGE, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_ENVSTARTRANGE]);
//WidgetSNConfig(IDC_VERTEXDELAY, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY]);
WidgetSmartRAHConfig(IDC_LAT, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LATITUDE], Active);
WidgetSmartRAHConfig(IDC_LON, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_LONGITUDE], Active);
WidgetSmartRAHConfig(IDC_AMP, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_AMPLITUDE], Active);
WidgetSmartRAHConfig(IDC_VERTEXDELAY, &Active->AnimPar[WCS_EFFECTS_WAVE_ANIMPAR_PERVERTEXENVDELAY], Active);

ConfigureFI(NativeWin, IDC_SOURCESPERVERTEX,
 &Active->WaveSourcesPerVertex,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   NULL);

ConfigureTB(NativeWin, IDC_ADDSOURCE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVESOURCE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_ZOOMIN3, IDI_ZMINARROW, NULL);
ConfigureTB(NativeWin, IDC_ZOOMOUT3, IDI_ZMOUTARROW, NULL);

WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

Width = 2000.0;
for (Ct = 1; Ct < Active->WavePreviewSize; Ct ++)
	Width /= 2.0;
sprintf(TextStr, "%1.1f m", Width);
WidgetSetText(IDC_PREVIEWSIZETXT3, TextStr);

BuildWaveList();
ConfigureWave();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // WaveEditGUI::ConfigureWidgets()

/*===========================================================================*/

void WaveEditGUI::BuildWaveList(void)
{
WaveSource *Current = Active->WaveSources;
RasterAnimHost *CurrentRAHost = NULL;
WaveSource **SelectedItems = NULL;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

NumListItems = WidgetLBGetCount(IDC_SOURCELIST);

ActiveWaveValid();

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_SOURCELIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (WaveSource **)AppMem_Alloc(NumSelected * sizeof (WaveSource *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_SOURCELIST, TempCt))
				{
				SelectedItems[SelCt ++] = (WaveSource *)WidgetLBGetItemData(IDC_SOURCELIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_SOURCELIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == (WaveSource *)CurrentRAHost)
			{
			BuildWaveListEntry(ListName, Current);
			WidgetLBReplace(IDC_SOURCELIST, Ct, ListName);
			WidgetLBSetItemData(IDC_SOURCELIST, Ct, Current);
			if (Current == ActiveWave)
				WidgetLBSetSelState(IDC_SOURCELIST, 1, Ct);
			else if (SelectedItems)
				{
				for (SelCt = 0; SelCt < NumSelected; SelCt ++)
					{
					if (SelectedItems[SelCt] == Current)
						{
						WidgetLBSetSelState(IDC_SOURCELIST, 1, Ct);
						break;
						} // if
					} // for
				} // if
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current == (WaveSource *)WidgetLBGetItemData(IDC_SOURCELIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildWaveListEntry(ListName, Current);
				WidgetLBReplace(IDC_SOURCELIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_SOURCELIST, TempCt, Current);
				if (Current == ActiveWave)
					WidgetLBSetSelState(IDC_SOURCELIST, 1, TempCt);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_SOURCELIST, 1, TempCt);
							break;
							} // if
						} // for
					} // if
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_SOURCELIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildWaveListEntry(ListName, Current);
				Place = WidgetLBInsert(IDC_SOURCELIST, Ct, ListName);
				WidgetLBSetItemData(IDC_SOURCELIST, Place, Current);
				if (Current == ActiveWave)
					WidgetLBSetSelState(IDC_SOURCELIST, 1, Place);
				else if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current)
							{
							WidgetLBSetSelState(IDC_SOURCELIST, 1, Place);
							break;
							} // if
						} // for
					} // if
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_SOURCELIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (WaveSource *));

} // WaveEditGUI::BuildWaveList

/*===========================================================================*/

void WaveEditGUI::BuildWaveListEntry(char *ListName, WaveSource *Me)
{
char AmpStr[64];

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
sprintf(AmpStr, "%f", Me->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE].CurValue);
TrimZeros(AmpStr);
strcat(ListName, AmpStr);

} // WaveEditGUI::BuildWaveListEntry()

/*===========================================================================*/

WaveSource *WaveEditGUI::ActiveWaveValid(void)
{
WaveSource *CurWave;

if (ActiveWave)
	{
	CurWave = Active->WaveSources;
	while (CurWave)
		{
		if (CurWave == ActiveWave)
			{
			return (ActiveWave);
			} // if
		CurWave = CurWave->Next;
		} // while
	} // if

return (ActiveWave = Active->WaveSources);

} // WaveEditGUI::ActiveWaveValid

/*===========================================================================*/

void WaveEditGUI::ConfigureWave(void)
{

if (ActiveWaveValid())
	{
	ConfigureSC(NativeWin, IDC_SOURCEENABLED, &ActiveWave->Enabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKENVENABLED, &ActiveWave->EnvelopeEnabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKREPEATBEFORE, &ActiveWave->RepeatEnvelopeBefore, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKREPEATAFTER, &ActiveWave->RepeatEnvelopeAfter, SCFlag_Char, NULL, 0);

	WidgetSNConfig(IDC_OFFSETX, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETX]);
	WidgetSNConfig(IDC_OFFSETY, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_OFFSETY]);
	WidgetSNConfig(IDC_WAVELEN, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_WAVELENGTH]);
	WidgetSNConfig(IDC_VEL, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_VELOCITY]);
	WidgetSNConfig(IDC_STARTTIME, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPESTARTTIME]);
	WidgetSNConfig(IDC_ENVVELOCITY, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY]);
	WidgetSmartRAHConfig(IDC_SOURCEAMP, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_AMPLITUDE], ActiveWave);
	WidgetSmartRAHConfig(IDC_PHASE, &ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_PHASE], ActiveWave);
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_SOURCEENABLED, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKENVENABLED, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKREPEATBEFORE, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKREPEATAFTER, NULL, 0, NULL, 0);
	WidgetSNConfig(IDC_OFFSETX, NULL);
	WidgetSNConfig(IDC_OFFSETY, NULL);
	WidgetSNConfig(IDC_WAVELEN, NULL);
	WidgetSNConfig(IDC_VEL, NULL);
	WidgetSNConfig(IDC_STARTTIME, NULL);
	WidgetSNConfig(IDC_ENVVELOCITY, NULL);
	WidgetSmartRAHConfig(IDC_SOURCEAMP, (RasterAnimHost *)NULL, NULL);
	WidgetSmartRAHConfig(IDC_PHASE, (RasterAnimHost *)NULL, NULL);
	} // else

} // WaveEditGUI::ConfigureWave

/*===========================================================================*/

void WaveEditGUI::SyncWidgets(void)
{
double Width;
WaveSource *CurSource;
int Ct;
char TextStr[64];

WidgetFISync(IDC_SOURCESPERVERTEX, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_AMP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_STARTRANGE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VERTEXDELAY, WP_FISYNC_NONOTIFY);

WidgetSRSync(IDC_RADIORANDOMTYPE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCLUSTEREDTYPE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAREATYPE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPOINTTYPE, WP_SRSYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGRADFILL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRANDOMSTART, WP_SCSYNC_NONOTIFY);

if (ActiveWaveValid())
	{
	WidgetSNSync(IDC_OFFSETX, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_OFFSETY, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_SOURCEAMP, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_WAVELEN, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_PHASE, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_VEL, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_STARTTIME, WP_FISYNC_NONOTIFY);
	WidgetSNSync(IDC_ENVVELOCITY, WP_FISYNC_NONOTIFY);

	WidgetSCSync(IDC_SOURCEENABLED, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKENVENABLED, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKREPEATBEFORE, WP_SCSYNC_NONOTIFY);
	WidgetSCSync(IDC_CHECKREPEATAFTER, WP_SCSYNC_NONOTIFY);
	} // if

WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

Width = 2000.0;
for (Ct = 1; Ct < Active->WavePreviewSize; Ct ++)
	Width /= 2.0;
sprintf(TextStr, "%1.1f m", Width);
WidgetSetText(IDC_PREVIEWSIZETXT3, TextStr);

NumEnvelopes = 0;
if (CurSource = Active->WaveSources)
	{
	while (CurSource)
		{
		if (CurSource->EnvelopeEnabled && CurSource->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue != 0.0)
			NumEnvelopes ++;
		CurSource = CurSource->Next;
		} // while
	} // if
if (NumEnvelopes > 1)
	sprintf(TextStr, "There are %d Enabled Motion Envelopes.", NumEnvelopes);
else if (NumEnvelopes == 1)
	strcpy(TextStr, "There is one Enabled Motion Envelope.");
else
	strcpy(TextStr, "There are no Enabled Motion Envelopes.");
WidgetSetText(IDC_ENVSEXIST, TextStr);

} // WaveEditGUI::SyncWidgets

/*===========================================================================*/

void WaveEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_ZOOMOUT3, Active->WavePreviewSize <= 1);

WidgetSetDisabled(IDC_SOURCESPERVERTEX, ! Active->Joes || Active->WaveEffectType != WCS_WAVE_WAVETYPE_POINT);
WidgetSetDisabled(IDC_VERTEXDELAY, ! Active->Joes || Active->WaveEffectType != WCS_WAVE_WAVETYPE_POINT);
WidgetSetDisabled(IDC_RADIOAREATYPE, ! Active->Joes);
WidgetSetDisabled(IDC_RADIOPOINTTYPE, ! Active->Joes);
WidgetSetDisabled(IDC_CHECKRANDOMSTART, NumEnvelopes <= 0);
WidgetSetDisabled(IDC_RADIORANDOMTYPE, ! Active->RandomizeEnvStart || NumEnvelopes <= 0);
WidgetSetDisabled(IDC_RADIOCLUSTEREDTYPE, ! Active->RandomizeEnvStart || NumEnvelopes <= 0);
WidgetSetDisabled(IDC_STARTRANGE, ! Active->RandomizeEnvStart || NumEnvelopes <= 0);

if (ActiveWaveValid())
	{
	WidgetSetDisabled(IDC_CHECKREPEATBEFORE, ! ActiveWave->EnvelopeEnabled);
	WidgetSetDisabled(IDC_CHECKREPEATAFTER, ! ActiveWave->EnvelopeEnabled);
	WidgetSetDisabled(IDC_STARTTIME, ! ActiveWave->EnvelopeEnabled || ActiveWave->AnimPar[WCS_EFFECTS_WAVESOURCE_ANIMPAR_ENVELOPEVELOCITY].CurValue == 0.0);
	WidgetSetDisabled(IDC_ENVVELOCITY, ! ActiveWave->EnvelopeEnabled);
	} // if

// need vectors
WidgetSetDisabled(IDC_CHECKGRADFILL, ! Active->Joes);
WidgetSetDisabled(IDC_EDITPROFILE, ! Active->Joes);

} // WaveEditGUI::DisableWidgets

/*===========================================================================*/

void WaveEditGUI::SelectPanel(long PanelID)
{

switch(PanelID)
	{
	case 1:
		{
		ShowPanel(1, -1);
		ShowPanel(0, 1);
		break;
		} // 1
	case 2:
		{
		ShowPanel(0, 2);
		ShowPanel(1, 0);
		break;
		} // 1
	case 3:
		{
		ShowPanel(0, 2);
		ShowPanel(1, 1);
		break;
		} // 1
	default:
		{
		ShowPanel(1, -1);
		ShowPanel(0, 0);
		break;
		} // None
	} // PanelID

} // WaveEditGUI::SelectPanel

/*===========================================================================*/

// advanced
void WaveEditGUI::DisplayAdvancedFeatures(void)
{
bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_HIDDENCONTROLMSG5, false);
	WidgetShow(IDC_WAVETYPE_TXT, true);
	WidgetShow(IDC_RADIOAREATYPE, true);
	WidgetShow(IDC_RADIOPOINTTYPE, true);
	WidgetShow(IDC_VERTEXDELAY, true);
	WidgetShow(IDC_SOURCESPERVERTEX, true);
	WidgetShow(IDC_CHECKENVENABLED, true);
	WidgetShow(IDC_CHECKREPEATBEFORE, true);
	WidgetShow(IDC_CHECKREPEATAFTER, true);
	WidgetShow(IDC_STARTTIME, true);
	WidgetShow(IDC_ENVVELOCITY, true);
	WidgetShow(IDC_EDITENVELOPE, true);
	WidgetShow(IDC_ENVSEXIST, true);
	WidgetShow(IDC_CHECKRANDOMSTART, true);
	WidgetShow(IDC_RADIORANDOMTYPE, true);
	WidgetShow(IDC_RADIOCLUSTEREDTYPE, true);
	WidgetShow(IDC_STARTRANGE, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_HIDDENCONTROLMSG5, true);
	WidgetShow(IDC_WAVETYPE_TXT, false);
	WidgetShow(IDC_RADIOAREATYPE, false);
	WidgetShow(IDC_RADIOPOINTTYPE, false);
	WidgetShow(IDC_VERTEXDELAY, false);
	WidgetShow(IDC_SOURCESPERVERTEX, false);
	WidgetShow(IDC_CHECKENVENABLED, false);
	WidgetShow(IDC_CHECKREPEATBEFORE, false);
	WidgetShow(IDC_CHECKREPEATAFTER, false);
	WidgetShow(IDC_STARTTIME, false);
	WidgetShow(IDC_ENVVELOCITY, false);
	WidgetShow(IDC_EDITENVELOPE, false);
	WidgetShow(IDC_ENVSEXIST, false);
	WidgetShow(IDC_CHECKRANDOMSTART, false);
	WidgetShow(IDC_RADIORANDOMTYPE, false);
	WidgetShow(IDC_RADIOCLUSTEREDTYPE, false);
	WidgetShow(IDC_STARTRANGE, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // WaveEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void WaveEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // WaveEditGUI::Cancel

/*===========================================================================*/

void WaveEditGUI::Name(void)
{
NotifyTag Changes[2];
char NewName[WCS_EFFECT_MAXNAMELENGTH];

if (WidgetGetModified(IDC_NAME))
	{
	WidgetGetText(IDC_NAME, WCS_EFFECT_MAXNAMELENGTH, NewName);
	WidgetSetModified(IDC_NAME, FALSE);
	Active->SetUniqueName(EffectsHost, NewName);
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if 

} // WaveEditGUI::Name()

/*===========================================================================*/

void WaveEditGUI::ZoomWaveScale(char InOrOut)
{
NotifyTag Changes[2];

Active->WavePreviewSize += InOrOut;
if (Active->WavePreviewSize < 1)
	Active->WavePreviewSize = 1;
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // WaveEditGUI::ZoomWaveScale

/*===========================================================================*/

void WaveEditGUI::AddWave(void)
{

ActiveWave = Active->AddWave(NULL);
BuildWaveList();
ConfigureWave();
DisableWidgets();

} // WaveEditGUI::AddWave

/*===========================================================================*/

void WaveEditGUI::RemoveWave(void)
{
RasterAnimHost **RemoveItems;
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;

if ((NumListEntries = WidgetLBGetCount(IDC_SOURCELIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_SOURCELIST, Ct))
			{
			NumSelected ++;
			} // if
		} // for
	if (NumSelected)
		{
		if (RemoveItems = (RasterAnimHost **)AppMem_Alloc(NumSelected * sizeof (RasterAnimHost *), APPMEM_CLEAR))
			{
			for (Ct = 0, Found = 0; Ct < NumListEntries && Found < NumSelected; Ct ++)
				{
				if (WidgetLBGetSelState(IDC_SOURCELIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_SOURCELIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll);
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Wave Source", "There are no Wave Sources selected to remove.");
		} // else
	} // if

} // WaveEditGUI::RemoveWave

/*===========================================================================*/

void WaveEditGUI::SetActiveWave(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_SOURCELIST);
Current = (long)WidgetLBGetItemData(IDC_SOURCELIST, Current);
if (Current != LB_ERR)
	ActiveWave = (WaveSource *)Current;
ConfigureWave();
DisableWidgets();

} // WaveEditGUI::SetActiveWave()

/*===========================================================================*/

long WaveEditGUI::HandleBackgroundCrunch(int Siblings)
{
int Handled = 0;

if (WaveSampleData->Running)
	{
	if (Active->EvalOneSampleLine(WaveSampleData) || ! (WaveSampleData->y % 10))
		{
		ConfigureTB(NativeWin, IDC_TNAIL3, NULL, NULL, WaveRast);
		} // if
	Handled = 1;
	} // if

if (! Handled)
	{
	// we're all done
	BackgroundInstalled = 0;
	return (1);		// 1 uninstalls background process
	} // if

// we've got more to do, come again when you have more time
return (0);

} // WaveEditGUI::HandleBackgroundCrunch

/*===========================================================================*/

void WaveEditGUI::UpdateThumbnail(void)
{

WaveSampleData->PreviewDirection = 2;
WaveSampleData->AndChildren = 1;
if (Active->EvalSampleInit(WaveRast, WaveSampleData))
	{
	if (! BackgroundInstalled)
		{
		if (GlobalApp->AddBGHandler(this))
			BackgroundInstalled = 1;
		} // if
	} // if initialized

} // WaveEditGUI::UpdateThumbnail

/*===========================================================================*/

void WaveEditGUI::SetPosInView(DiagnosticData *Data)
{

if (ReceivingDiagnostics == 0)
	{
	if (UserMessageOKCAN("Set Wave Model Position", "The next point clicked in any View will\n become this Wave Model's new position."))
		{
		ReceivingDiagnostics = 1;
		ReceivingWave = 0;
		ReceivingWaveSource = 0;
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
			LatEvent = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			Active->SetPosition(LatEvent, LonEvent);
			ReceivingDiagnostics = 0;
			} // if
		} // if
	else
		ReceivingDiagnostics = 0;
	} // else if
else
	ReceivingDiagnostics = 0;

WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // WaveEditGUI::SetPosInView

/*===========================================================================*/

void WaveEditGUI::SetSourcePosInView(DiagnosticData *Data)
{

if (ActiveWaveValid())
	{
	if (ReceivingWaveSource == 0)
		{
		if (UserMessageOKCAN("Set Wave Source Position", "The next point clicked in any View will\n become the active Wave Source's new position."))
			{
			ReceivingWaveSource = 1;
			ReceivingWave = 0;
			ReceivingDiagnostics = 0;
			GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
			} // if
		} // if
	else if (ReceivingWaveSource == 1)
		{
		if (Data)
			{
			if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
				{
				LatEvent = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
				LonEvent = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
				Active->SetSourcePosition(ActiveWave, LatEvent, LonEvent);
				ReceivingWaveSource = 0;
				} // if
			} // if
		else
			ReceivingWaveSource = 0;
		} // else if
	else
		ReceivingWaveSource = 0;
	} // if
else
	{
	UserMessageOK("Set Wave Source Position", "There is no active Wave Source selected in the list");
	ReceivingWaveSource = 0;
	} // else

WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // WaveEditGUI::SetSourcePosInView

/*===========================================================================*/

void WaveEditGUI::AddWaveInView(DiagnosticData *Data)
{

if (ReceivingWave == 0)
	{
	if (UserMessageOKCAN("Add Wave Source", "The next point clicked in any View will\n create a new Wave Source for this Wave Model."))
		{
		ReceivingWave = 1;
		ReceivingWaveSource = 0;
		ReceivingDiagnostics =0;
		GlobalApp->GUIWins->CVG->SetViewManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		GlobalApp->GUIWins->CVG->SetObjectManipulationMode(WCS_VIEWGUI_MANIP_NONE);
		} // if
	} // if
else if (ReceivingWave == 1)
	{
	if (Data)
		{
		if (Data->ValueValid[WCS_DIAGNOSTIC_LATITUDE] && Data->ValueValid[WCS_DIAGNOSTIC_LONGITUDE])
			{
			LatEvent = Data->Value[WCS_DIAGNOSTIC_LATITUDE];
			LonEvent = Data->Value[WCS_DIAGNOSTIC_LONGITUDE];
			Active->AddWave(LatEvent, LonEvent);
			ReceivingWave = 0;
			} // if
		} // if
	else
		ReceivingWave = 0;
	} // else if
else
	ReceivingWave = 0;

WidgetSetCheck(IDC_ADDSOURCEINVIEW, ReceivingWave);
WidgetSetCheck(IDC_SETCENTERPOS, ReceivingDiagnostics);
WidgetSetCheck(IDC_SETSOURCEINVIEW, ReceivingWaveSource);

} // WaveEditGUI::AddWaveInView

/*===========================================================================*/

void WaveEditGUI::CopyWave(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *CopyHost;
char CopyMsg[256];

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveWaveValid() && (CopyHost = ActiveWave))
		{
		CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
		CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
		CopyHost->GetRAHostProperties(CopyProp);
		if (CopyProp->Flags & WCS_RAHOST_FLAGBIT_DRAGGABLE)
			{
			RasterAnimHost::SetCopyOfRAHost(CopyHost);
			sprintf(CopyMsg, "%s %s copied to clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
			} // if
		else
			{
			UserMessageOK("Copy", "Selected item cannot be copied.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // WaveEditGUI::CopyWave

/*===========================================================================*/

void WaveEditGUI::PasteWave(void)
{
RasterAnimHostProperties *CopyProp;
RasterAnimHost *PasteHost, *CopyHost, *TempHost;
char CopyMsg[256], CopyIllegal = 0;

if (CopyProp = new RasterAnimHostProperties())
	{
	if (ActiveWaveValid() && (PasteHost = ActiveWave))
		{
		if (CopyHost = RasterAnimHost::GetCopyOfRAHost())
			{
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_NAME | WCS_RAHOST_MASKBIT_TYPE | WCS_RAHOST_MASKBIT_TYPENUMBER;
			CopyProp->FlagsMask = WCS_RAHOST_FLAGBIT_DRAGGABLE;
			CopyHost->GetRAHostProperties(CopyProp);
			CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPOK;
			PasteHost->GetRAHostProperties(CopyProp);
			if (CopyProp->DropOK)
				{
				CopyProp->PropMask = WCS_RAHOST_MASKBIT_DROPSOURCE;
				CopyProp->DropSource = CopyHost;
				// Result = 1 if drop complete, 0 if failed and -1 if inconclusive, 
				//  eg. still in progress through a DragnDropListGUI
				// determine if copying is legal - can't copy from a texture to a child of itself or a parent of itself
				TempHost = PasteHost;
				while (TempHost)
					{
					if (CopyHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->RAParent;
					} // while
				TempHost = CopyHost;
				while (TempHost)
					{
					if (PasteHost == TempHost)
						{
						CopyIllegal = 1;
						break;
						} // if
					TempHost = TempHost->RAParent;
					} // while
				if (! CopyIllegal)
					{
					PasteHost->SetRAHostProperties(CopyProp);
					sprintf(CopyMsg, "%s %s pasted from clipboard", CopyProp->Name ? CopyProp->Name: "", CopyProp->Type ? CopyProp->Type: "");
					GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, CopyMsg);
					} // if
				else
					{
					UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Can't copy from parent to child or child to its parent or copy texture to itself.");
					} // else
				} // if
			else
				{
				UserMessageOK(CopyProp->Name ? CopyProp->Name: "Paste", "Item in copy buffer cannot be pasted on selected target.");
				} // else
			} // if
		else
			{
			UserMessageOK("Paste", "There is nothing in the clipboard to paste.");
			} // else
		} // if
	delete CopyProp;
	} // if

} // WaveEditGUI::PasteWave

/*===========================================================================*/

bool WaveEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{
WaveSource *CurWave;
bool EnvelopeEnabled = false;

for (CurWave = Active->WaveSources; CurWave; CurWave = CurWave->Next)
	{
	if (CurWave->EnvelopeEnabled)
		EnvelopeEnabled = true;
	} // for

return(DisplayAdvanced || Active->WaveEffectType != WCS_WAVE_WAVETYPE_AREA || EnvelopeEnabled ? true : false);

} // WaveEditGUI::QueryLocalDisplayAdvancedUIVisibleState
