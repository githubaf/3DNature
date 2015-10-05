// RenderOptEditGUI.cpp
// Code for RenderOpt editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RenderOptEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "ImageOutputEvent.h"
#include "AppMem.h"
#include "resource.h"
#include "Lists.h"

char *RenderOptEditGUI::TabNames[WCS_RENDEROPTGUI_NUMTABS] = {"Size && Range", "File Output", "Post", "Enabled 1", "Enabled 2", "Misc."};
long RenderOptEditGUI::ActivePage;

struct RenderPreset RPresets[] =
	{
	{"NTSC D1", 720, 486, .9, 30.0},
	{"NTSC D2", 752, 486, .859, 30.0},
	{"NTSC M-JPEG", 720, 480, .9, 30.0},
	{"NTSC Toaster/Flyer", 752, 480, .859, 30.0},
	{"PAL D1", 720, 576, 1.067, 25.0},
	{"PAL D2", 752, 576, 1.019, 25.0},
	{"HDTV 1080i", 1920, 1080, 1.0, 30.0},
	{"HDTV 720p", 1280, 720, 1.0, 30.0},
	// this one needs to be second to last
	{"1:1 (VGA/Mac/Print/Film)", 0, 0, 1.0, 24.0},
	// Custom must always be the last in the list,
	// And must have zeros for all fields.
	// Film 4K Flat 4096x3112, 1.666?
	// Film 4K Academy Flat 3656x2664, 1.666?
	// Film 4K CinemaScope 3656x3112, 2.0?
	// Film 2K Flat 2048x1556, 1.666?
	// Film 2K Academy Flat 1828x1332, 1.666?
	// Film 2K CinemaScope 1828x1556, 2.0?
	// IMAX ? x ?, ?
	{"Custom", 0, 0, 0, 30}
	}; // RPresets;

NativeGUIWin RenderOptEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // RenderOptEditGUI::Open

/*===========================================================================*/

NativeGUIWin RenderOptEditGUI::Construct(void)
{
int TabIndex;
char *SetName;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_RENDER_TILES
	CreateSubWinFromTemplate(IDD_RENDEROPT_GENERAL_VNS, 0, 0);
	#else // WCS_RENDER_TILES
	CreateSubWinFromTemplate(IDD_RENDEROPT_GENERAL, 0, 0);
	#endif // WCS_RENDER_TILES
	CreateSubWinFromTemplate(IDD_RENDEROPT_OUTPUT, 0, 1);
	CreateSubWinFromTemplate(IDD_RENDEROPT_POSTPROC, 0, 2);
	#ifdef WCS_LABEL
	CreateSubWinFromTemplate(IDD_RENDEROPT_ENABLED1_VNS, 0, 3);
	#else // WCS_LABEL
	CreateSubWinFromTemplate(IDD_RENDEROPT_ENABLED1, 0, 3);
	#endif // WCS_LABEL
	CreateSubWinFromTemplate(IDD_RENDEROPT_ENABLED2, 0, 4);
	CreateSubWinFromTemplate(IDD_RENDEROPT_MISC, 0, 5);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_RENDEROPTGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for(TabIndex = 0; ; TabIndex++)
			{
			WidgetCBAddEnd(IDC_RENDERPRESETS, RPresets[TabIndex].PSName);
			if(RPresets[TabIndex].Aspect == 0.0)
				{
				CustomRes = TabIndex;
				break;
				} // if
			} // for
		SetName = NULL;
		for(TabIndex = 0; ; TabIndex++)
			{
			if (! (SetName = ImageSaverLibrary::GetNextFileFormat(SetName)))
				break;
			WidgetCBAddEnd(IDC_FORMATDROP, SetName);
			} // for
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // RenderOptEditGUI::Construct

/*===========================================================================*/

RenderOptEditGUI::RenderOptEditGUI(EffectsLib *EffectsSource, RenderOpt *ActiveSource)
: GUIFenetre('RNDO', this, "Render Options Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_RENDEROPT, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_POSTPROC, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_POSTPROC, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
char NameStr[256];

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
EffectsHost = EffectsSource;
Active = ActiveSource;
CustomRes = 0;
ActiveEvent = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Render Options Editor- %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // RenderOptEditGUI::RenderOptEditGUI

/*===========================================================================*/

RenderOptEditGUI::~RenderOptEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // RenderOptEditGUI::~RenderOptEditGUI()

/*===========================================================================*/

long RenderOptEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_ROG, 0);

return(0);

} // RenderOptEditGUI::HandleCloseWin

/*===========================================================================*/

long RenderOptEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_ROG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_ADDEVENT:
		{
		AddNewOutputEvent();
		break;
		} //
	case IDC_REMOVEEVENT:
		{
		RemoveOutputEvent();
		break;
		} //
	case IDC_CONSTRAIN:
		{
		SetConstrain();
		break;
		} // 
	case IDC_SCALE:
		{
		ScaleSize();
		break;
		} // 
	case IDC_ADDPROC:
		{
		AddPostProc();
		break;
		} // IDC_ADDPROC
	case IDC_REMOVEPROC:
		{
		RemovePostProc();
		break;
		} // IDC_REMOVEPROC
	case IDC_MOVEPROCUP:
		{
		ChangePostProcListPosition(1);
		break;
		} // IDC_MOVEPROCUP
	case IDC_MOVEPROCDOWN:
		{
		ChangePostProcListPosition(0);
		break;
		} // IDC_MOVEPROCDOWN
	default: break;
	} // ButtonID
return(0);

} // RenderOptEditGUI::HandleButtonClick

/*===========================================================================*/

long RenderOptEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_EVENTLIST:
		{
		SetActiveOutputEvent();
		break;
		} // IDC_EVENTLIST
	case IDC_BUFFERLIST:
		{
		SelectOutputBuffer();
		break;
		} // IDC_BUFFERLIST
	case IDC_CODECLIST:
		{
		SelectOutputCodec();
		break;
		} // IDC_CODECLIST
	} // switch CtrlID

return (0);

} // RenderOptEditGUI::HandleListSel

/*===========================================================================*/

long RenderOptEditGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_PROCLIST:
		{
		RemovePostProc();
		break;
		} // IDC_PROCLIST
	} // switch

return(0);

} // RenderOptEditGUI::HandleListDelItem

/*===========================================================================*/

long RenderOptEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_PROCLIST:
		{
		EditPostProc();
		break;
		}
	} // switch CtrlID

return (0);

} // RenderOptEditGUI::HandleListDoubleClick

/*===========================================================================*/

long RenderOptEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RENDERPRESETS:
		{
		SelectNewResolution();
		break;
		} // IDC_RENDERPRESETS
	case IDC_FORMATDROP:
		{
		SelectNewOutputFormat();
		break;
		} // IDC_FORMATDROP
	} // switch CtrlID

return (0);

} // RenderOptEditGUI::HandleCBChange

/*===========================================================================*/

long RenderOptEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // RenderOptEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long RenderOptEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
{

switch (CtrlID)
	{
	case IDC_TAB1:
		{
		switch (NewPageID)
			{
			case 1:
				{
				ShowPanel(0, 1);
				break;
				} // 1
			case 2:
				{
				ShowPanel(0, 2);
				break;
				} // 2
			case 3:
				{
				ShowPanel(0, 3);
				break;
				} // 3
			case 4:
				{
				ShowPanel(0, 4);
				break;
				} // 4
			case 5:
				{
				ShowPanel(0, 5);
				break;
				} // 4
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	} // switch

ActivePage = NewPageID;

return(0);

} // RenderOptEditGUI::HandlePageChange

/*===========================================================================*/

long RenderOptEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKTILING:
		{
		if (Active->TilingEnabled)
			Active->ConcatenateTiles = 1;
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKBEYONDHORIZ:
	case IDC_VECTOBMAP:
	case IDC_VECHAZE:
	case IDC_VECLUMINOUS:
	case IDC_CHECKRENDEROVERHEAD:
	case IDC_CHECKFRAGENABLED:

	case IDC_CHECKREFLECTIONS:
	case IDC_CHECKTRANSPARWATER:
	case IDC_CHECKCLOUDSHADOW:
	case IDC_CHECKOBJECTSHADOW:
	case IDC_CHECKFOLSHADOWSONLY:
	case IDC_CHECKTERRAIN:
	case IDC_CHECKFOLIAGE:

	case IDC_CHECKLAKEEFFECTS:
	case IDC_CHECKSTREAMS:
	case IDC_CHECKECOSYSEFFECTS:
	case IDC_CHECKWAVES:
	case IDC_CHECK3DCLOUD:

	case IDC_CHECKCMENABLE:
	case IDC_CHECKDIAGNOSTICDATA:
	case IDC_CHECKRASTERTA:
	case IDC_CHECKTERRAFFECTORS:
	case IDC_CHECK3DOBJECTS:
	case IDC_CHECKFENCES:
	case IDC_CHECKLABELS:
	case IDC_CHECKMULTIPASSAA:
	case IDC_CHECKDEPTHOFFIELD:

	case IDC_CHECKLIGHTS:
	case IDC_CHECKSHADOWS:
	case IDC_CHECKCELEST:
	case IDC_CHECKSTARS:
	case IDC_CHECKSNOW:
	case IDC_CHECKSKY:

	case IDC_CHECKATMOSPHERE:
	case IDC_CHECKVOLUMETRICS:
	case IDC_CHECKFOLEFFECTS:
	case IDC_CHECKAUTOEXTENSION:
	case IDC_CHECKBEFOREPOST:
	case IDC_CHECKCONCAT:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKEVENTENABLED:
		{
		BuildEventList();
		break;
		} // 
	} // switch CtrlID

return(0);

} // RenderOptEditGUI::HandleSCChange

/*===========================================================================*/

long RenderOptEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOREFLTYPEB:
	case IDC_RADIOREFLTYPED:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // RenderOptEditGUI::HandleSRChange

/*===========================================================================*/

long RenderOptEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
double RangeDefaults[3];

switch (CtrlID)
	{
	case IDC_IMAGEWIDTH:
	case IDC_IMAGEHEIGHT:
		{
		if (Active->LockAspect)
			{
			if (CtrlID == IDC_IMAGEWIDTH)
				Active->OutputImageHeight = (long)(.5 + Active->OutputImageWidth  / Active->ImageAspectRatio);
			else
				Active->OutputImageWidth = (long)(.5 + Active->OutputImageHeight * Active->ImageAspectRatio);
			if (Active->OutputImageHeight < 1)
				Active->OutputImageHeight = 1;
			if (Active->OutputImageWidth < 1)
				Active->OutputImageWidth = 1;
			} // if
		} // 
		//lint -fallthrough
	case IDC_SEGMENT:
	case IDC_AUTODIGITS:
	case IDC_FRAMESTEP:
	case IDC_FRAGDEPTH:
	case IDC_TILESX:
	case IDC_TILESY:
		{
		#ifdef WCS_BUILD_DEMO
		if (Active->OutputImageWidth > 640)
			Active->OutputImageWidth = 640;
		if (Active->OutputImageHeight > 450)
			Active->OutputImageHeight = 450;
		#endif // WCS_BUILD_DEMO
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_FRAMERATE:
		{
		#ifdef WCS_BUILD_DEMO
		RangeDefaults[0] = 5.0;
		#else // WCS_BUILD_DEMO
		RangeDefaults[0] = FLT_MAX;
		#endif // WCS_BUILD_DEMO
		RangeDefaults[1] = 0.0;
		RangeDefaults[2] = Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue > 0.0 ? 1.0 / Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue: 1.0 / 30.0;
		Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetRangeDefaults(RangeDefaults);
		Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetRangeDefaults(RangeDefaults);
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	} // switch CtrlID

return(0);

} // RenderOptEditGUI::HandleFIChange

/*===========================================================================*/

long RenderOptEditGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

return(0);

} // RenderOptEditGUI::HandleDDChange

/*===========================================================================*/

void RenderOptEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[7];
long Done = 0;

if (! NativeWin)
	return;

Changes = Activity->ChangeNotify->ChangeList;

/* nothing to synchronize for now
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
*/
Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	DisableWidgets();
	Done = 1;
	} // if

if (! Done)
	ConfigureWidgets();

} // RenderOptEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void RenderOptEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

sprintf(TextStr, "Render Options Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_VECTOBMAP, &Active->VectorsEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_VECHAZE, &Active->HazeVectors, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_VECLUMINOUS, &Active->LuminousVectors, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRENDEROVERHEAD, &Active->RenderFromOverhead, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFRAGENABLED, &Active->FragmentRenderingEnabled, SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKLAKEEFFECTS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSTREAMS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREFLECTIONS, &Active->ReflectionsEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTRANSPARWATER, &Active->TransparentWaterEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKWAVES, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE], SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECK3DCLOUD, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCELEST, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSTARS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSKY, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKATMOSPHERE, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVOLUMETRICS, &Active->VolumetricsEnabled, SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKLIGHTS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSHADOWS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCLOUDSHADOW, &Active->CloudShadowsEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOBJECTSHADOW, &Active->ObjectShadowsEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFOLSHADOWSONLY, &Active->FoliageShadowsOnly, SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKTERRAIN, &Active->TerrainEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCMENABLE, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRASTERTA, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKTERRAFFECTORS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR], SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECKFOLIAGE, &Active->FoliageEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKECOSYSEFFECTS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFOLEFFECTS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSNOW, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW], SCFlag_Char, NULL, 0);

ConfigureSC(NativeWin, IDC_CHECK3DOBJECTS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFENCES, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE], SCFlag_Char, NULL, 0);
#ifdef WCS_LABEL
ConfigureSC(NativeWin, IDC_CHECKLABELS, &Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL], SCFlag_Char, NULL, 0);
#endif // WCS_LABEL

ConfigureSC(NativeWin, IDC_CHECKMULTIPASSAA, &Active->MultiPassAAEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDEPTHOFFIELD, &Active->DepthOfFieldEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDIAGNOSTICDATA, &Active->RenderDiagnosticData, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOREFLTYPEB, IDC_RADIOREFLTYPEB, &Active->ReflectionType, SRFlag_Char, WCS_REFLECTIONSTYLE_BEAMTRACE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOREFLTYPEB, IDC_RADIOREFLTYPED, &Active->ReflectionType, SRFlag_Char, WCS_REFLECTIONSTYLE_BTFRAGS, NULL, NULL);

#ifdef WCS_RENDER_TILES
ConfigureSC(NativeWin, IDC_CHECKTILING, &Active->TilingEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCONCAT, &Active->ConcatenateTiles, SCFlag_Char, NULL, 0);

ConfigureFI(NativeWin, IDC_TILESX,
 &Active->TilesX,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_TILESY,
 &Active->TilesY,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);
#endif // WCS_RENDER_TILES

if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
	{
	WidgetSetText(IDC_STARTTIME, "Start Frame ");
	WidgetSetText(IDC_ENDTIME, "End Frame ");
	} // if
else
	{
	WidgetSetText(IDC_STARTTIME, "Start Time ");
	WidgetSetText(IDC_ENDTIME, "End Time ");
	} // else

WidgetSNConfig(IDC_STARTTIME, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME]);
WidgetSNConfig(IDC_ENDTIME, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME]);
WidgetSNConfig(IDC_FRAMERATE, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE]);
WidgetSNConfig(IDC_PIXELASPECT, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT]);
WidgetSNConfig(IDC_SIDEOVERSCAN, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_SIDEOVERSCAN]);
WidgetSNConfig(IDC_BOTTOMOVERSCAN, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_BOTTOMOVERSCAN]);
WidgetSNConfig(IDC_ZOFFSET, &Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_VECTOROFFSET]);

ConfigureFI(NativeWin, IDC_FRAGDEPTH,
 &Active->FragmentDepth,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_SEGMENT,
 &Active->RenderImageSegments,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Short,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_IMAGEWIDTH,
 &Active->OutputImageWidth,
  1.0,
   1.0,
#ifdef WCS_BUILD_DEMO
	640.0,
#else // WCS_BUILD_DEMO
	100000.0,
#endif // WCS_BUILD_DEMO
	 FIOFlag_Long,
	  NULL,
	   0);

#ifdef WCS_BUILD_DEMO

#endif // WCS_BUILD_DEMO

ConfigureFI(NativeWin, IDC_IMAGEHEIGHT,
 &Active->OutputImageHeight,
  1.0,
   1.0,
#ifdef WCS_BUILD_DEMO
	450.0,
#else // WCS_BUILD_DEMO
	100000.0,
#endif // WCS_BUILD_DEMO
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_FRAMESTEP,
 &Active->FrameStep,
  1.0,
   1.0,
	100000.0,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureDD(NativeWin, IDC_TEMPPATH, (char *)Active->TempPath.GetPath(), 255, NULL, 0, IDC_LABEL_TEMP);

ConfigureTB(NativeWin, IDC_ADDPROC, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEPROC, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVEPROCUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVEPROCDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_ADDEVENT, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVEEVENT, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_CONSTRAIN, IDI_CONSTRASP, NULL);

WidgetSetCheck(IDC_CONSTRAIN, Active->LockAspect);

ConfigureSizeDrop();
BuildEventList();
ConfigureOutputEvent();
BuildPostProcList();
DisableWidgets();

} // RenderOptEditGUI::ConfigureWidgets()

/*===========================================================================*/

void RenderOptEditGUI::ConfigureSizeDrop(void)
{
int Preset;
int TestW, TestH;
double TestA;

// Identify closest preset
TestW = Active->OutputImageWidth;
TestH = Active->OutputImageHeight;
TestA = Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].CurValue;

for (Preset = 0; ; Preset++)
	{
	if(RPresets[Preset].Aspect == 0.0)
		{ // end of list, break and set to Custom
		break;
		} // if
	if((RPresets[Preset].Aspect == TestA) &&
	   (RPresets[Preset].Width  == TestW) &&
	   (RPresets[Preset].Height == TestH))
		{
		break;
		} // if
	if((RPresets[Preset].Aspect == TestA) &&
	   (RPresets[Preset].Width  == 0) &&
	   (RPresets[Preset].Height == 0))
		{
		break;
		} // if
	} // for

WidgetCBSetCurSel(IDC_RENDERPRESETS, Preset);


} // RenderOptEditGUI::ConfigureSizeDrop

/*===========================================================================*/

void RenderOptEditGUI::ConfigureOutputEvent(void)
{
long NumCBEntries, Ct;
char FormatStr[WCS_MAX_FILETYPELENGTH];

if (ActiveEvent)
	{
	ConfigureSC(NativeWin, IDC_CHECKEVENTENABLED, &ActiveEvent->Enabled, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKAUTOEXTENSION, &ActiveEvent->AutoExtension, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKBEFOREPOST, &ActiveEvent->BeforePost, SCFlag_Char, NULL, 0);

	ConfigureFI(NativeWin, IDC_AUTODIGITS,
	 &ActiveEvent->AutoDigits,
	  1.0,
	   0.0,
		10.0,
		 FIOFlag_Char,
		  NULL,
		   0);

	ConfigureDD(NativeWin, IDC_SAVEPATH, (char *)ActiveEvent->PAF.GetPath(), 255, (char *)ActiveEvent->PAF.GetName(), 63, NULL);
	WidgetSetDisabled(IDC_SAVEPATH, 0);

	NumCBEntries = WidgetCBGetCount(IDC_FORMATDROP);
	for (Ct = 0; Ct < NumCBEntries; Ct ++)
		{
		WidgetCBGetText(IDC_FORMATDROP, Ct, FormatStr);
		if (! stricmp(ActiveEvent->FileType, FormatStr))
			{
			WidgetCBSetCurSel(IDC_FORMATDROP, Ct);
			break;
			} // if
		} // for 

	BuildEventBufferList();
	BuildEventCodecList();
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_CHECKEVENTENABLED, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKAUTOEXTENSION, NULL, 0, NULL, 0);
	ConfigureSC(NativeWin, IDC_CHECKBEFOREPOST, NULL, 0, NULL, 0);

	ConfigureFI(NativeWin, IDC_AUTODIGITS,
	 NULL,
	  1.0,
	   0.0,
		10.0,
		 0,
		  NULL,
		   0);
	ConfigureDD(NativeWin, IDC_SAVEPATH, " ", 1, " ", 1, NULL);
	WidgetSetDisabled(IDC_SAVEPATH, 1);
	BuildEventBufferList();
	BuildEventCodecList();
	} // else

} // RenderOptEditGUI::ConfigureOutputEvents

/*===========================================================================*/

void RenderOptEditGUI::BuildEventBufferList(void)
{
char *SetName = NULL;
long Ct, Pos;

WidgetLBClear(IDC_BUFFERLIST);

if (ActiveEvent)
	{
	while (SetName = ImageSaverLibrary::GetNextBuffer(ActiveEvent->FileType, SetName))
		{
		Pos = WidgetLBInsert(IDC_BUFFERLIST, -1, SetName);
		for (Ct = 0; Ct < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
			{
			if (! stricmp(ActiveEvent->OutBuffers[Ct], SetName))
				{
				WidgetLBSetSelState(IDC_BUFFERLIST, 1, Pos);
				break;
				} // if
			} // for
		} // while
	} // if

} // RenderOptEditGUI::BuildEventBufferList

/*===========================================================================*/

void RenderOptEditGUI::BuildEventCodecList(void)
{
char *SetName = NULL;
long Pos;

WidgetLBClear(IDC_CODECLIST);

if (ActiveEvent)
	{
	while (SetName = ImageSaverLibrary::GetNextCodec(ActiveEvent->FileType, SetName))
		{
		Pos = WidgetLBInsert(IDC_CODECLIST, -1, SetName);
		if (! stricmp(ActiveEvent->Codec, SetName))
			{
			WidgetLBSetCurSel(IDC_CODECLIST, Pos);
			} // if
		} // while
	} // if

} // RenderOptEditGUI::BuildEventCodecList

/*===========================================================================*/

void RenderOptEditGUI::BuildEventList(void)
{
ImageOutputEvent *Current = Active->OutputEvents, *CurrentEvent;
long Ct = 0, TempCt, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];

NumListItems = WidgetLBGetCount(IDC_EVENTLIST);

ActiveEventValid();

while (Current || Ct < NumListItems)
	{
	CurrentEvent = Ct < NumListItems ? (ImageOutputEvent *)WidgetLBGetItemData(IDC_EVENTLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current == CurrentEvent)
			{
			BuildEventListEntry(ListName, Current);
			WidgetLBReplace(IDC_EVENTLIST, Ct, ListName);
			WidgetLBSetItemData(IDC_EVENTLIST, Ct, Current);
			if (Current == ActiveEvent)
				WidgetLBSetCurSel(IDC_EVENTLIST, Ct);
			Ct ++;
			} // if
		else
			{
			FoundIt = 0;
			for (TempCt = Ct + 1; TempCt < NumListItems; TempCt ++)
				{
				if (Current == (ImageOutputEvent *)WidgetLBGetItemData(IDC_EVENTLIST, TempCt))
					{
					FoundIt = 1;
					break;
					} // if
				} // for
			if (FoundIt)
				{
				BuildEventListEntry(ListName, Current);
				WidgetLBReplace(IDC_EVENTLIST, TempCt, ListName);
				WidgetLBSetItemData(IDC_EVENTLIST, TempCt, Current);
				if (Current == ActiveEvent)
					WidgetLBSetCurSel(IDC_EVENTLIST, TempCt);
				for (TempCt -- ; TempCt >= Ct; TempCt --)
					{
					WidgetLBDelete(IDC_EVENTLIST, TempCt);
					NumListItems --;
					} // for
				Ct ++;
				} // if
			else
				{
				BuildEventListEntry(ListName, Current);
				Place = WidgetLBInsert(IDC_EVENTLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_EVENTLIST, Place, Current);
				if (Current == ActiveEvent)
					WidgetLBSetCurSel(IDC_EVENTLIST, Place);
				NumListItems ++;
				Ct ++;
				} // else
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_EVENTLIST, Ct);
		NumListItems --;
		} // else
	} // while

} // RenderOptEditGUI::BuildEventList

/*===========================================================================*/

void RenderOptEditGUI::BuildEventListEntry(char *ListName, ImageOutputEvent *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->FileType);

} // RenderOptEditGUI::BuildEventListEntry()

/*===========================================================================*/

void RenderOptEditGUI::BuildPostProcList(void)
{
EffectList *Current = Active->Post;
long Ct = 0, TempCt, SelCt = 0, NumSelected = 0, Place, NumListItems, FoundIt;
char ListName[WCS_EFFECT_MAXNAMELENGTH + 4];
RasterAnimHost *CurrentRAHost = NULL;
GeneralEffect **SelectedItems = NULL;

NumListItems = WidgetLBGetCount(IDC_PROCLIST);

for (TempCt = 0; TempCt < NumListItems; TempCt ++)
	{
	if (WidgetLBGetSelState(IDC_PROCLIST, TempCt))
		{
		NumSelected ++;
		} // if
	} // for

if (NumSelected)
	{
	if (SelectedItems = (GeneralEffect **)AppMem_Alloc(NumSelected * sizeof (GeneralEffect *), 0))
		{
		for (TempCt = 0; TempCt < NumListItems; TempCt ++)
			{
			if (WidgetLBGetSelState(IDC_PROCLIST, TempCt))
				{
				SelectedItems[SelCt ++] = (GeneralEffect *)WidgetLBGetItemData(IDC_PROCLIST, TempCt);
				} // if
			} // for
		} // if
	} // if

while (Current || Ct < NumListItems)
	{
	CurrentRAHost = Ct < NumListItems ? (RasterAnimHost *)WidgetLBGetItemData(IDC_PROCLIST, Ct): NULL;
	
	if (Current)
		{
		if (Current->Me)
			{
			if (Current->Me == (GeneralEffect *)CurrentRAHost)
				{
				BuildPostProcListEntry(ListName, Current->Me);
				WidgetLBReplace(IDC_PROCLIST, Ct, ListName);
				WidgetLBSetItemData(IDC_PROCLIST, Ct, Current->Me);
				if (SelectedItems)
					{
					for (SelCt = 0; SelCt < NumSelected; SelCt ++)
						{
						if (SelectedItems[SelCt] == Current->Me)
							{
							WidgetLBSetSelState(IDC_PROCLIST, 1, Ct);
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
					if (Current->Me == (GeneralEffect *)WidgetLBGetItemData(IDC_PROCLIST, TempCt))
						{
						FoundIt = 1;
						break;
						} // if
					} // for
				if (FoundIt)
					{
					BuildPostProcListEntry(ListName, Current->Me);
					WidgetLBReplace(IDC_PROCLIST, TempCt, ListName);
					WidgetLBSetItemData(IDC_PROCLIST, TempCt, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_PROCLIST, 1, TempCt);
								break;
								} // if
							} // for
						} // if
					for (TempCt -- ; TempCt >= Ct; TempCt --)
						{
						WidgetLBDelete(IDC_PROCLIST, TempCt);
						NumListItems --;
						} // for
					Ct ++;
					} // if
				else
					{
					BuildPostProcListEntry(ListName, Current->Me);
					Place = WidgetLBInsert(IDC_PROCLIST, Ct, ListName);
					WidgetLBSetItemData(IDC_PROCLIST, Place, Current->Me);
					if (SelectedItems)
						{
						for (SelCt = 0; SelCt < NumSelected; SelCt ++)
							{
							if (SelectedItems[SelCt] == Current->Me)
								{
								WidgetLBSetSelState(IDC_PROCLIST, 1, Place);
								break;
								} // if
							} // for
						} // if
					NumListItems ++;
					Ct ++;
					} // else
				} // if
			} // if
		Current = Current->Next;
		} // if
	else
		{
		WidgetLBDelete(IDC_PROCLIST, Ct);
		NumListItems --;
		} // else
	} // while

if (SelectedItems)
	AppMem_Free(SelectedItems, NumSelected * sizeof (GeneralEffect *));

} // RenderOptEditGUI::BuildPostProcList

/*===========================================================================*/

void RenderOptEditGUI::BuildPostProcListEntry(char *ListName, GeneralEffect *Me)
{

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
if (Me->Joes)
	strcat(ListName, "+ ");
else
	strcat(ListName, "  ");
strcat(ListName, Me->Name);

} // RenderOptEditGUI::BuildPostProcListEntry()

/*===========================================================================*/

ImageOutputEvent *RenderOptEditGUI::ActiveEventValid(void)
{
ImageOutputEvent *CurEvent;

if (ActiveEvent)
	{
	CurEvent = Active->OutputEvents;
	while (CurEvent)
		{
		if (CurEvent == ActiveEvent)
			{
			return (ActiveEvent);
			} // if
		CurEvent = CurEvent->Next;
		} // while
	} // if

return (ActiveEvent = Active->OutputEvents);

} // RenderOptEditGUI::ActiveEventValid

/*===========================================================================*/

void RenderOptEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_IMAGEWIDTH, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_IMAGEHEIGHT, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_SEGMENT, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FRAMESTEP, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FRAGDEPTH, WP_FISYNC_NONOTIFY);

WidgetSNSync(IDC_STARTTIME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ENDTIME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FRAMERATE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PIXELASPECT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SIDEOVERSCAN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BOTTOMOVERSCAN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZOFFSET, WP_FISYNC_NONOTIFY);

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_VECTOBMAP, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_VECHAZE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_VECLUMINOUS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRENDEROVERHEAD, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFRAGENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLAKEEFFECTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSTREAMS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREFLECTIONS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTRANSPARWATER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKWAVES, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK3DCLOUD, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCELEST, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSTARS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSKY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKATMOSPHERE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVOLUMETRICS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLIGHTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCLOUDSHADOW, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOBJECTSHADOW, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFOLSHADOWSONLY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTERRAIN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCMENABLE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRASTERTA, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKTERRAFFECTORS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFOLIAGE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKECOSYSEFFECTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFOLEFFECTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSNOW, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECK3DOBJECTS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFENCES, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLABELS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKMULTIPASSAA, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDEPTHOFFIELD, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDIAGNOSTICDATA, WP_SCSYNC_NONOTIFY);
#ifdef WCS_RENDER_TILES
WidgetSCSync(IDC_CHECKTILING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCONCAT, WP_SCSYNC_NONOTIFY);
WidgetFISync(IDC_TILESX, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_TILESY, WP_FISYNC_NONOTIFY);
#endif // WCS_RENDER_TILES

WidgetSRSync(IDC_RADIOREFLTYPEB, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOREFLTYPED, WP_SRSYNC_NONOTIFY);

ConfigureSizeDrop();

} // RenderOptEditGUI::SyncWidgets

/*===========================================================================*/

void RenderOptEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_FRAMESTEP, fabs(Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].CurValue -
	Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].CurValue) < .0001);
WidgetSetDisabled(IDC_ZOFFSET, ! Active->VectorsEnabled);
WidgetSetDisabled(IDC_VECHAZE, ! Active->VectorsEnabled);
WidgetSetDisabled(IDC_VECLUMINOUS, ! Active->VectorsEnabled);

WidgetSetDisabled(IDC_CHECKWAVES, ! Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] && ! Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM]);
WidgetSetDisabled(IDC_CHECKTRANSPARWATER, ! (Active->FragmentRenderingEnabled && (Active->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] || Active->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM])));
WidgetSetDisabled(IDC_RADIOREFLTYPEB, ! (Active->FragmentRenderingEnabled && Active->ReflectionsEnabled));
WidgetSetDisabled(IDC_RADIOREFLTYPED, ! (Active->FragmentRenderingEnabled && Active->ReflectionsEnabled));

#ifdef WCS_RENDER_TILES
WidgetSetDisabled(IDC_SEGMENT, Active->TilingEnabled);
WidgetSetDisabled(IDC_CHECKCONCAT, ! Active->TilingEnabled);
WidgetSetDisabled(IDC_TILESX, ! Active->TilingEnabled);
WidgetSetDisabled(IDC_TILESY, ! Active->TilingEnabled);
#endif // WCS_RENDER_TILES

} // RenderOptEditGUI::DisableWidgets

/*===========================================================================*/

void RenderOptEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // RenderOptEditGUI::Cancel

/*===========================================================================*/

void RenderOptEditGUI::Name(void)
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

} // RenderOptEditGUI::Name()

/*===========================================================================*/

void RenderOptEditGUI::SelectNewResolution(void)
{
long Current;
double RangeDefaults[3];
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_RENDERPRESETS);
if (Current != CustomRes)
	{
	if (RPresets[Current].Width)
		Active->OutputImageWidth = RPresets[Current].Width;
	if (RPresets[Current].Height)
		Active->OutputImageHeight = RPresets[Current].Height;
	if (RPresets[Current].Aspect > 0.0)
		Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_PIXELASPECT].SetValue(RPresets[Current].Aspect);
	if (RPresets[Current].FrameRate > 0.0)
		Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].SetValue(RPresets[Current].FrameRate);

	#ifdef WCS_BUILD_DEMO
	RangeDefaults[0] = 5.0;
	#else // WCS_BUILD_DEMO
	RangeDefaults[0] = FLT_MAX;
	#endif // WCS_BUILD_DEMO
	RangeDefaults[1] = 0.0;
	RangeDefaults[2] = Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue > 0.0 ? 1.0 / Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_FRAMERATE].CurValue: 1.0 / 30.0;
	Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_STARTTIME].SetRangeDefaults(RangeDefaults);
	Active->AnimPar[WCS_EFFECTS_RENDEROPT_ANIMPAR_ENDTIME].SetRangeDefaults(RangeDefaults);

	SetConstrain();

	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // RenderOptEditGUI::SelectNewResolution

/*===========================================================================*/

void RenderOptEditGUI::AddNewOutputEvent(void)
{
ImageOutputEvent *NewEvent;

if (NewEvent = Active->AddOutputEvent())
	{
	ActiveEvent = NewEvent;
	BuildEventList();
	ConfigureOutputEvent();
	} // if

} // RenderOptEditGUI::AddNewOutputEvent

/*===========================================================================*/

void RenderOptEditGUI::RemoveOutputEvent(void)
{
ImageOutputEvent *RemoveEvent;
long Selected;

if ((Selected = WidgetLBGetCurSel(IDC_EVENTLIST)) != LB_ERR)
	{
	if ((RemoveEvent = (ImageOutputEvent *)WidgetLBGetItemData(IDC_EVENTLIST, Selected)) != (ImageOutputEvent *)LB_ERR && RemoveEvent)
		{
		Active->RemoveOutputEvent(RemoveEvent);
		} // if
	} // if

} // RenderOptEditGUI::RemoveOutputEvent

/*===========================================================================*/

void RenderOptEditGUI::SetActiveOutputEvent(void)
{
ImageOutputEvent *NewEvent;
long Selected;

if ((Selected = WidgetLBGetCurSel(IDC_EVENTLIST)) != LB_ERR)
	{
	if ((NewEvent = (ImageOutputEvent *)WidgetLBGetItemData(IDC_EVENTLIST, Selected)) != (ImageOutputEvent *)LB_ERR && NewEvent)
		{
		ActiveEvent = NewEvent;
		ConfigureOutputEvent();
		} // if
	} // if

} // RenderOptEditGUI::SetActiveOutputEvent

/*===========================================================================*/

void RenderOptEditGUI::SelectNewOutputFormat(void)
{
long Selected, BufCt;
char FormatStr[WCS_MAX_FILETYPELENGTH];
char *DefBuf;

if (ActiveEvent)
	{
	Selected = WidgetCBGetCurSel(IDC_FORMATDROP);

	WidgetCBGetText(IDC_FORMATDROP, Selected, FormatStr);

	if (strcmp(FormatStr, ActiveEvent->FileType))
		{
		if (ImageSaverLibrary::GetPlanOnly(FormatStr))
			{
			if (! UserMessageOKCAN(FormatStr, "This output format is suitable only for rendering with Planimetric\n type Cameras. Continue with format selection?"))
				{
				ConfigureOutputEvent();
				return;
				} // if
			} // if WCS DEM
		// set new file type
		strcpy(ActiveEvent->FileType, FormatStr);

		// clear old values
		for (BufCt = 0; BufCt < WCS_MAX_IMAGEOUTBUFFERS; BufCt ++)
			ActiveEvent->OutBuffers[BufCt][0] = 0;
		ActiveEvent->Codec[0] = 0;

		// set new values for buffers and codec
		BufCt = 0;
		DefBuf = NULL;
		while (DefBuf = ImageSaverLibrary::GetNextDefaultBuffer(FormatStr, DefBuf))
			strcpy(ActiveEvent->OutBuffers[BufCt ++], DefBuf);
		if (DefBuf = ImageSaverLibrary::GetNextCodec(FormatStr, NULL))
			strcpy(ActiveEvent->Codec, DefBuf);
			
		BuildEventList();
		ConfigureOutputEvent();
		} // if file type actually changed
	} // if

} // RenderOptEditGUI::SelectNewOutputFormat

/*===========================================================================*/

void RenderOptEditGUI::SelectOutputBuffer(void)
{
long NumLBEntries, Ct, BufCt = 0;
char BufferStr[WCS_MAX_BUFFERNODE_NAMELEN];

if (ActiveEvent)
	{
	NumLBEntries = WidgetLBGetCount(IDC_BUFFERLIST);
	for (Ct = 0; Ct < NumLBEntries && BufCt < WCS_MAX_IMAGEOUTBUFFERS; Ct ++)
		{
		WidgetLBGetText(IDC_BUFFERLIST, Ct, BufferStr);
		// if it is a required buffer make it selected
		if (! WidgetLBGetSelState(IDC_BUFFERLIST, Ct))
			{
			if (ImageSaverLibrary::GetIsBufferRequired(ActiveEvent->FileType, BufferStr))
				WidgetLBSetSelState(IDC_BUFFERLIST, 1, Ct);
			} // if
		// copy selected items to OutBuffers
		if (WidgetLBGetSelState(IDC_BUFFERLIST, Ct))
			{
			strcpy(ActiveEvent->OutBuffers[BufCt ++], BufferStr);
			} // if
		} // for 
	// chop the rest of the OutBuffers
	for (; BufCt < WCS_MAX_IMAGEOUTBUFFERS; BufCt ++)
		{
		ActiveEvent->OutBuffers[BufCt][0] = 0;
		} // for 
	} // if

} // RenderOptEditGUI::SelectOutputBuffer

/*===========================================================================*/

void RenderOptEditGUI::SelectOutputCodec(void)
{
long Selected;
char CodecStr[WCS_MAX_CODECLENGTH];

if (ActiveEvent)
	{
	if ((Selected = WidgetLBGetCurSel(IDC_CODECLIST)) != LB_ERR)
		{
		WidgetLBGetText(IDC_CODECLIST, Selected, CodecStr);
		strcpy(ActiveEvent->Codec, CodecStr);
		} // if
	} // if

} // RenderOptEditGUI::SelectOutputCodec

/*===========================================================================*/

void RenderOptEditGUI::ScaleSize(void)
{
char Str[64];
double Factor = 1.0, Test;
NotifyTag Changes[2];

Str[0] = 0;

if (GetInputString("Enter a scale factor, percent or letter (such as \"h\" for half).", "", Str) && Str[0])
	{
	if (Str[strlen(Str) - 1] == '%')
		{
		Factor = atof(Str) / 100.0;
		if (Factor <= 0.0)
			{
			if (Factor <= -1.0 || Factor == 0.0)
				Factor = 1.0;
			else
				Factor = 1.0 + Factor;
			} // if
		} // if
	else
		{
		switch (Str[0])
			{
			case 'h':
			case 'H':
				{
				Factor = .5;
				break;
				} // half
			case 'q':
			case 'Q':
				{
				Factor = .25;
				break;
				} // half
			case 'd':
			case 'D':
				{
				Factor = 2.0;
				break;
				} // half
			case 't':
			case 'T':
				{
				if (Str[1] == 'h' || Str[1] == 'H')
					Factor = .33334;
				else
					Factor = 3.0;
				break;
				} // half
			case 'f':
				{
				Factor = 4.0;
				break;
				} // half
			default:
				{
				Test = atof(Str);
				if (Test > -100.0 && Test <= -1.0)
					{
					Factor = (100.0 + Test) / 100.0;
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > -1.0 && Test < 0.0)
					{
					Factor = (1.0 + Test);
					} // if
				else if (Test > 0.0 && Test <= 10.0)
					{
					Factor = Test;
					} // if
				else if (Test > 10.0)
					{
					Factor = Test / 100.0;
					} // if
				break;
				} // half

			} // switch
		} // else

	Active->OutputImageWidth = (long)(.5 + Factor * Active->OutputImageWidth);
	Active->OutputImageHeight = (long)(.5 + Factor * Active->OutputImageHeight);

	if (Active->OutputImageWidth < 1)
		Active->OutputImageWidth = 1;
	if (Active->OutputImageHeight < 1)
		Active->OutputImageHeight = 1;
	#ifdef WCS_BUILD_DEMO
	if (Active->OutputImageWidth > 640)
		Active->OutputImageWidth = 640;
	if (Active->OutputImageHeight > 450)
		Active->OutputImageHeight = 450;
	#endif // WCS_BUILD_DEMO
	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	} // if

} // RenderOptEditGUI::ScaleSize

/*===========================================================================*/

void RenderOptEditGUI::SetConstrain(void)
{

Active->LockAspect = WidgetGetCheck(IDC_CONSTRAIN);

Active->ImageAspectRatio = (double)Active->OutputImageWidth / Active->OutputImageHeight;

} // RenderOptEditGUI::SetConstrain

/*===========================================================================*/

void RenderOptEditGUI::AddPostProc(void)
{

EffectsHost->AddAttributeByList(Active, WCS_EFFECTSSUBCLASS_POSTPROC);

} // RenderOptEditGUI::AddPostProc

/*===========================================================================*/

void RenderOptEditGUI::RemovePostProc(void)
{
long Ct, Found, NumListEntries, NumSelected = 0;
int RemoveAll = 0;
RasterAnimHost **RemoveItems;

if ((NumListEntries = WidgetLBGetCount(IDC_PROCLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_PROCLIST, Ct))
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
				if (WidgetLBGetSelState(IDC_PROCLIST, Ct))
					{
					RemoveItems[Found ++] = (RasterAnimHost *)WidgetLBGetItemData(IDC_PROCLIST, Ct);
					} // if
				} // for
			for (Ct = 0; Ct < NumSelected; Ct ++)
				{
				if (RemoveItems[Ct])
					{
					if (Active->FindnRemoveRAHostChild(RemoveItems[Ct], RemoveAll))
						{
						//EffectsHost->RemoveRAHost(RemoveItems[Ct], 0);
						} // if
					} // if
				} // for
			AppMem_Free(RemoveItems, NumSelected * sizeof (RasterAnimHost *));
			} // if
		} // if
	else
		{
		UserMessageOK("Remove Post Process", "There are no Post Processes selected to remove.");
		} // else
	} // if

} // RenderOptEditGUI::RemovePostProc

/*===========================================================================*/

void RenderOptEditGUI::EditPostProc(void)
{
long Ct, NumListEntries;
RasterAnimHost *EditMe;

if ((NumListEntries = WidgetLBGetCount(IDC_PROCLIST)) > 0)
	{
	for (Ct = 0; Ct < NumListEntries; Ct ++)
		{
		if (WidgetLBGetSelState(IDC_PROCLIST, Ct))
			{
			if (EditMe = (RasterAnimHost *)WidgetLBGetItemData(IDC_PROCLIST, Ct))
				EditMe->EditRAHost();
			} // if
		} // for
	} // if

} // RenderOptEditGUI::EditPostProc

/*===========================================================================*/

void RenderOptEditGUI::ChangePostProcListPosition(short MoveUp)
{
long Ct, NumListEntries, SendNotify = 0;
RasterAnimHost *MoveMe;
EffectList *Current, *PrevProc = NULL, *PrevPrevProc = NULL, *StashProc;
NotifyTag Changes[2];

// don't send notification until all changes are done
if ((NumListEntries = WidgetLBGetCount(IDC_PROCLIST)) > 0)
	{
	if (MoveUp)
		{
		for (Ct = 0; Ct < NumListEntries; Ct ++)
			{
			if (WidgetLBGetSelState(IDC_PROCLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_PROCLIST, Ct))
					{
					Current = Active->Post;
					while (Current->Me != MoveMe)
						{
						PrevPrevProc = PrevProc;
						PrevProc = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (PrevProc)
							{
							StashProc = Current->Next;
							if (PrevPrevProc)
								{
								PrevPrevProc->Next = Current;
								Current->Next = PrevProc;
								} // if
							else
								{
								Active->Post = Current;
								Active->Post->Next = PrevProc;
								} // else
							PrevProc->Next = StashProc;
							SendNotify = 1;
							} // else if
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
		} // if
	else
		{
		for (Ct = NumListEntries - 1; Ct >= 0; Ct --)
			{
			if (WidgetLBGetSelState(IDC_PROCLIST, Ct))
				{
				if (MoveMe = (GeneralEffect *)WidgetLBGetItemData(IDC_PROCLIST, Ct))
					{
					Current = Active->Post;
					while (Current->Me != MoveMe)
						{
						PrevPrevProc = PrevProc;
						PrevProc = Current;
						Current = Current->Next;
						} // while
					if (Current && Current->Me)
						{
						if (Current->Next)
							{
							StashProc = Current->Next->Next;
							if (PrevProc)
								{
								PrevProc->Next = Current->Next;
								PrevProc->Next->Next = Current;
								} // if
							else
								{
								Active->Post = Current->Next;
								Active->Post->Next = Current;
								} // else
							Current->Next = StashProc;
							SendNotify = 1;
							} // if move down
						else
							break;
						} // if
					else
						break;
					} // if
				} // if
			} // for
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

} // RenderOptEditGUI::ChangePostProcListPosition

/*===========================================================================*/
