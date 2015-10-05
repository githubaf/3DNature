// CameraEditGUI.cpp
// Code for Camera editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "CameraEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Raster.h"
#include "resource.h"

char *CameraEditGUI::TabNames[WCS_CAMERAGUI_NUMTABS] = {"General", "Lens", "Position && Orientation", "Target", "Stereo", "Misc"};

long CameraEditGUI::ActivePage;
// advanced
long CameraEditGUI::DisplayAdvanced;

NativeGUIWin CameraEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // CameraEditGUI::Open

/*===========================================================================*/

NativeGUIWin CameraEditGUI::Construct(void)
{
int TabIndex, Pos;
Raster *MyRast;
GeneralEffect *MyEffect;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	#ifdef WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_CAMERA_GENERAL_VNS, 0, 0);
	#else // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_CAMERA_GENERAL, 0, 0);
	#endif // WCS_BUILD_VNS
	CreateSubWinFromTemplate(IDD_CAMERA_LENS, 0, 1);
	CreateSubWinFromTemplate(IDD_CAMERA_POSITION, 0, 2);
	CreateSubWinFromTemplate(IDD_CAMERA_TARGET, 0, 3);
	CreateSubWinFromTemplate(IDD_CAMERA_STEREO, 0, 4);
	CreateSubWinFromTemplate(IDD_CAMERA_MISC, 0, 5);

	if(NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_CAMERAGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
		for (MyRast = GlobalApp->AppImages->GetFirstRast(); MyRast; MyRast = GlobalApp->AppImages->GetNextRast(MyRast))
			{
			Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
			WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
			} // for
		WidgetCBInsert(IDC_TARGETDROP, -1, "New 3D Object...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_TARGETDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_TARGETDROP, TabIndex, MyEffect);
			} // for
		#ifdef WCS_BUILD_VNS
		WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
		for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_COORDSDROP, TabIndex, MyEffect);
			} // for
		#endif // WCS_BUILD_VNS
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		} // if
	} // if
 
return (NativeWin);

} // CameraEditGUI::Construct

/*===========================================================================*/

CameraEditGUI::CameraEditGUI(EffectsLib *EffectsSource, ImageLib *ImageSource, Camera *ActiveSource)
: GUIFenetre('CAMP', this, "Camera Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_CAMERA, 0xff, 0xff, 0xff), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_RENDERJOB, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED), 
								#ifdef WCS_BUILD_VNS
								MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff),
								#endif // WCS_BUILD_VNS
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
ImageHost = ImageSource;
Active = ActiveSource;
SyncLensFOV = 0;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "Camera Editor - %s", Active->GetName());
	if (Active->GetRAHostRoot()->TemplateItem)
		strcat(NameStr, " (Templated)");
	SetTitle(NameStr);
	// advanced
	DisplayAdvanced = Active->GetDisplayAdvanced(EffectsHost);
	Active->Copy(&Backup, Active);
	GlobalApp->AppEx->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);
	} // if
else
	ConstructError = 1;

} // CameraEditGUI::CameraEditGUI

/*===========================================================================*/

CameraEditGUI::~CameraEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // CameraEditGUI::~CameraEditGUI()

/*===========================================================================*/

long CameraEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_CPG, 0);

return(0);

} // CameraEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long CameraEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{
DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);
} // CameraEditGUI::HandleShowAdvanced

/*===========================================================================*/

long CameraEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
HandleCommonEvent(ButtonID, EffectsHost, Active);

switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CPG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_BTNBANKKEYS:
		{
		Active->CreateBankKeys();
		break;
		} // IDC_BTNBANKKEYS
	case IDC_FREETARGET:
		{
		FreeTarget();
		break;
		} // IDC_FREETARGET
	case IDC_SETSEPARATION:
		{
		SetSeparation();
		break;
		} // IDC_SETSEPARATION
	case IDC_EDITCOORDS:
		{
		if (Active->Coords)
			Active->Coords->EditRAHost();
		break;
		} // IDC_EDITCOORDS
	default:
		break;
	} // ButtonID

return(0);

} // CameraEditGUI::HandleButtonClick

/*===========================================================================*/

long CameraEditGUI::HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

switch (ButtonID)
	{
	case IDC_TNAIL:
		{
		EditImage();
		break;
		} // 
	default:
		break;
	} // switch

return(0);

} // CameraEditGUI::HandleButtonDoubleClick

/*===========================================================================*/

long CameraEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_IMAGEDROP:
		{
		SelectNewImage();
		break;
		}
	case IDC_TARGETDROP:
		{
		SelectNewTarget();
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

if (Active->Floating)
	Active->SetFloating(0);

return (0);

} // CameraEditGUI::HandleCBChange

/*===========================================================================*/

long CameraEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // CameraEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long CameraEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
				} // 5
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		break;
		}
	default:
		break;
	} // switch

ActivePage = NewPageID;

return(0);

} // CameraEditGUI::HandlePageChange

/*===========================================================================*/

long CameraEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];
int FloatStash;

Changes[1] = 0;

switch (CtrlID)
	{
	//case IDC_CHECKENABLED:
	//	{
	//	Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED);
	//	GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
	//	break;
	//	} // 
	case IDC_CHECKSTEREO:
		{
		Active->Orthographic = 0;
		Active->PanoCam = 0;
		if (Active->Floating)
			Active->SetFloating(0);		// this sends the valuechanged message
		else
			{
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // else
		break;
		} // stereo
	case IDC_CHECKVRCAM:
		{
		Active->Orthographic = 0;
		Active->StereoCam = 0;
		// fall through
		} // panoramic
		//lint -fallthrough
	case IDC_CHECKORTHOGRAPHIC:
		{
		Active->LensDistortion = 0;
		// fall through
		} // orthographic
		//lint -fallthrough
	case IDC_CHECKBEYONDHORIZ:
	case IDC_CHECKFIELDS:
	case IDC_CHECKFLIPFIELDS:
	case IDC_CHECKBGENABLE:
	case IDC_CHECKDEPTHOFFIELD:
	case IDC_MOTIONBLUR:
	case IDC_BLURENABLE:
	case IDC_CHECKZBBLUR:
	case IDC_CHECKVELODIST:
	case IDC_CHECKLENSDISTORTION:
	case IDC_CHECKCONVERGENCE:
	case IDC_CHECKPROJECTED:
		{
		if (Active->Floating)
			Active->SetFloating(0);		// this sends the valuechanged message
		else
			{
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // else
		break;
		} // 
	case IDC_CHECKFLOATING:
		{
		FloatStash = Active->Floating;
		Active->SetFloating(Active->Floating);		// this sends the valuechanged message
		if (Active->Floating != FloatStash)
			WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
		break;
		} // 
	case IDC_CHECKSYNCLENSFOV:
		{
		if (SyncLensFOV)
			{
			UserMessageOK("Sync Lens & FOV", "This is a transient effect which allows linked adjustment of Lens Length, Film Size and Field of View while this window is open. No permanent link is formed between these parameters.");
			SyncLens();
			} // if
		if (Active->Floating)
			Active->SetFloating(0);		// this sends the valuechanged message
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // CameraEditGUI::HandleSCChange

/*===========================================================================*/

long CameraEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOPLANIMETRIC:
		{
		Active->Orthographic = 0;
		Active->LensDistortion = 0;
		Active->StereoCam = 0;
		// fall through
		} // overhead
		//lint -fallthrough
	case IDC_RADIOOVERHEAD:
		{
		Active->PanoCam = 0;
		// fall through
		} // overhead
		//lint -fallthrough
	case IDC_RADIOTARGETED:
	case IDC_RADIONONTARGETED:
	case IDC_RADIOALIGNED:
	case IDC_RADIOOPTREF:
	case IDC_RADIOOPT3DOBJ:
	case IDC_RADIOCENTERORIGIN:
	case IDC_RADIOCENTERCENTER:
	case IDC_RADIOPREVIEWLEFT:
	case IDC_RADIOPREVIEWCENTER:
	case IDC_RADIOPREVIEWRIGHT:
	case IDC_RADIORENDERLEFT:
	case IDC_RADIORENDERBOTH:
	case IDC_RADIORENDERRIGHT:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

if (Active->Floating)
	Active->SetFloating(0);

return(0);

} // CameraEditGUI::HandleSRChange

/*===========================================================================*/

long CameraEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_MAXBLUR:
	case IDC_AAPASSES:
	case IDC_BLURFAC:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		Changes[1] = NULL;
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_VIEWARC:
	case IDC_FILMSIZE:
		{
		if (SyncLensFOV)
			SyncLens();
		break;
		} // 
	case IDC_FOCALLENGTH:
		{
		if (SyncLensFOV)
			SyncFOV();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

if (Active->Floating)
	Active->SetFloating(0);

return(0);

} // CameraEditGUI::HandleFIChange

/*===========================================================================*/

void CameraEditGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
long Pos, CurPos, Done = 0;
Raster *MyRast, *MatchRast;
GeneralEffect *MyEffect, *MatchEffect;

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
	// advanced
	DisplayAdvancedFeatures();
	Done = 1;
	} // if

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = ImageHost->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchRast = (Active->Img) ? Active->Img->GetRaster(): NULL;
	WidgetCBClear(IDC_IMAGEDROP);
	WidgetCBInsert(IDC_IMAGEDROP, -1, "New Image Object...");
	for (MyRast = ImageHost->GetFirstRast(); MyRast; MyRast = ImageHost->GetNextRast(MyRast))
		{
		Pos = WidgetCBInsert(IDC_IMAGEDROP, -1, MyRast->GetUserName());
		WidgetCBSetItemData(IDC_IMAGEDROP, Pos, MyRast);
		if (MyRast == MatchRast)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_IMAGEDROP, CurPos);
	Done = 1;
	} // if image name changed
else
	{
	Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_RASTER, 0xff, 0xff, 0xff);
	Interested[1] = NULL;
	if (Changed = GlobalApp->AppImages->MatchNotifyClass(Interested, Changes, 0))
		{
		MatchRast = (Active->Img) ? Active->Img->GetRaster(): NULL;
		ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MatchRast);
		Done = 1;
		} // else
	} // else

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[2] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = Active->TargetObj;
	WidgetCBClear(IDC_TARGETDROP);
	WidgetCBInsert(IDC_TARGETDROP, -1, "New 3D Object...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_OBJECT3D); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_TARGETDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_TARGETDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_TARGETDROP, CurPos);
	Done = 1;
	} // if image name changed

#ifdef WCS_BUILD_VNS
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	CurPos = -1;
	MatchEffect = Active->Coords;
	WidgetCBClear(IDC_COORDSDROP);
	WidgetCBInsert(IDC_COORDSDROP, -1, "New Coordinate System...");
	for (MyEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_COORDSDROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_COORDSDROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, CurPos);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		Done = 1;
		} // if
	} // if Coordinate System name changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // CameraEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void CameraEditGUI::ConfigureWidgets(void)
{
Raster *MyRast, *TestRast;
RenderJob *CurJob;
Object3DEffect *TestObj;
#ifdef WCS_BUILD_VNS
CoordSys *TestCS;
#endif // WCS_BUILD_VNS
long ListPos, Ct, NumJobs = 0, NumEntries;
char TextStr[256];

sprintf(TextStr, "Camera Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

if (CurJob = (RenderJob *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_RENDERJOB))
	{
	while (CurJob)
		{
		if (CurJob->Cam == Active)
			NumJobs ++;
		CurJob = (RenderJob *)CurJob->Next;
		} // while
	} // if
if (NumJobs > 1)
	sprintf(TextStr, "There are %d Render Jobs using this Camera.", NumJobs);
else if (NumJobs == 1)
	strcpy(TextStr, "There is one Render Job using this Camera.");
else
	strcpy(TextStr, "There are no Render Jobs using this Camera.");
WidgetSetText(IDC_JOBSEXIST, TextStr);

ConfigureSC(NativeWin, IDC_CHECKBEYONDHORIZ, &Active->RenderBeyondHorizon, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFIELDS, &Active->FieldRender, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLIPFIELDS, &Active->FieldRenderPriority, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKBGENABLE, &Active->BackgroundImageEnabled, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKORTHOGRAPHIC, &Active->Orthographic, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVRCAM, &Active->PanoCam, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKDEPTHOFFIELD, &Active->DepthOfField, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_MOTIONBLUR, &Active->MotionBlur, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_BLURENABLE, &Active->BoxFilter, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKZBBLUR, &Active->ZBufBoxFilter, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVELODIST, &Active->VelocitySmoothing, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSYNCLENSFOV, &SyncLensFOV, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKLENSDISTORTION, &Active->LensDistortion, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFLOATING, &Active->Floating, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKFOLLOW, &Active->InterElevFollow, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCONVERGENCE, &Active->StereoConvergence, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSTEREO, &Active->StereoCam, SCFlag_Char, NULL, 0);
#ifdef WCS_BUILD_VNS
ConfigureSC(NativeWin, IDC_CHECKPROJECTED, &Active->Projected, SCFlag_Char, NULL, 0);
#endif // WCS_BUILD_VNS

ConfigureSR(NativeWin, IDC_RADIOTARGETED, IDC_RADIOTARGETED, &Active->CameraType, SRFlag_Char, WCS_EFFECTS_CAMERATYPE_TARGETED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTARGETED, IDC_RADIONONTARGETED, &Active->CameraType, SRFlag_Char, WCS_EFFECTS_CAMERATYPE_UNTARGETED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTARGETED, IDC_RADIOALIGNED, &Active->CameraType, SRFlag_Char, WCS_EFFECTS_CAMERATYPE_ALIGNED, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTARGETED, IDC_RADIOOVERHEAD, &Active->CameraType, SRFlag_Char, WCS_EFFECTS_CAMERATYPE_OVERHEAD, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOTARGETED, IDC_RADIOPLANIMETRIC, &Active->CameraType, SRFlag_Char, WCS_EFFECTS_CAMERATYPE_PLANIMETRIC, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOCENTERORIGIN, IDC_RADIOCENTERORIGIN, &Active->CenterOnOrigin, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCENTERORIGIN, IDC_RADIOCENTERCENTER, &Active->CenterOnOrigin, SRFlag_Char, 0, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOOPTREF, IDC_RADIOOPTREF, &Active->AAOptimizeReflection, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOOPTREF, IDC_RADIOOPT3DOBJ, &Active->AAOptimizeReflection, SRFlag_Char, 0, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOPREVIEWLEFT, IDC_RADIOPREVIEWLEFT, &Active->StereoPreviewChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_LEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPREVIEWLEFT, IDC_RADIOPREVIEWCENTER, &Active->StereoPreviewChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_CENTER, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPREVIEWLEFT, IDC_RADIOPREVIEWRIGHT, &Active->StereoPreviewChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_RIGHT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIORENDERLEFT, IDC_RADIORENDERLEFT, &Active->StereoRenderChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_LEFT, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORENDERLEFT, IDC_RADIORENDERBOTH, &Active->StereoRenderChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_CENTER, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORENDERLEFT, IDC_RADIORENDERRIGHT, &Active->StereoRenderChannel, SRFlag_Char, WCS_CAMERA_STEREOCHANNEL_RIGHT, NULL, NULL);

ConfigureFI(NativeWin, IDC_MAXBLUR,
 &Active->MaxDOFBlurRadius,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Long,
	  NULL,
	   0);

ConfigureFI(NativeWin, IDC_AAPASSES,
 &Active->AAPasses,
  1.0,
   1.0,
	17.0,
	 FIOFlag_Long,
	  NULL,
	   0);
ConfigureFI(NativeWin, IDC_BLURFAC,
 &Active->BoxFilterSize,
  1.0,
   1.0,
	100.0,
	 FIOFlag_Long,
	  NULL,
	   0);
WidgetSNConfig(IDC_EASEIN, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEIN]);
WidgetSNConfig(IDC_EASEOUT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_EASEOUT]);
WidgetSmartRAHConfig(IDC_CAMLAT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLAT], Active);
WidgetSmartRAHConfig(IDC_CAMLON, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMLON], Active);
WidgetSmartRAHConfig(IDC_CAMALT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CAMELEV], Active);
WidgetSmartRAHConfig(IDC_FOCLAT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLAT], Active);
WidgetSmartRAHConfig(IDC_FOCLON, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGLON], Active);
WidgetSmartRAHConfig(IDC_FOCALT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_TARGELEV], Active);
WidgetSmartRAHConfig(IDC_HEADING, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HEADING], Active);
WidgetSmartRAHConfig(IDC_PITCH, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_PITCH], Active);
WidgetSmartRAHConfig(IDC_CAMBANK, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_BANK], Active);
WidgetSmartRAHConfig(IDC_VIEWARC, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV], Active);
WidgetSmartRAHConfig(IDC_CENTERX, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX], Active);
WidgetSmartRAHConfig(IDC_CENTERY, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY], Active);
WidgetSmartRAHConfig(IDC_ZMINIMUM, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN], Active);
WidgetSmartRAHConfig(IDC_ZMAXIMUM, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX], Active);
WidgetSmartRAHConfig(IDC_MAGICZ, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ], Active);
WidgetSmartRAHConfig(IDC_FOCALLENGTH, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH], Active);
WidgetSmartRAHConfig(IDC_FILMSIZE, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE], Active);
WidgetSmartRAHConfig(IDC_FOCALDIST, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALDIST], Active);
WidgetSmartRAHConfig(IDC_FSTOP, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FSTOP], Active);
WidgetSmartRAHConfig(IDC_MOTIONBLURPCT, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MOBLURPERCENT], Active);
WidgetSmartRAHConfig(IDC_BLURMAXOFF, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZBUFBOXFILTOFFSET], Active);
WidgetSmartRAHConfig(IDC_SCALE, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_VIEWWIDTH], Active);
WidgetSmartRAHConfig(IDC_STEREOSEP, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION], Active);
WidgetSmartRAHConfig(IDC_STEREOCONVERGENCE, &Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE], Active);


#ifdef WCS_BUILD_DEMO
WidgetSetDisabled(IDC_CENTERX, TRUE);
WidgetSetDisabled(IDC_CENTERY, TRUE);
#endif // WCS_BUILD_DEMO

if (Active->Img && (MyRast = Active->Img->GetRaster()))
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
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, MyRast);
	} // if
else
	{
	WidgetCBSetCurSel(IDC_IMAGEDROP, -1);
	ConfigureTB(NativeWin, IDC_TNAIL, NULL, NULL, NULL);
	} // else

if (Active->TargetObj)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_TARGETDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestObj = (Object3DEffect *)WidgetCBGetItemData(IDC_TARGETDROP, Ct)) != (Object3DEffect *)LB_ERR && TestObj == Active->TargetObj)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_TARGETDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_TARGETDROP, -1);

#ifdef WCS_BUILD_VNS
if (Active->Coords)
	{
	ListPos = -1;
	NumEntries = WidgetCBGetCount(IDC_COORDSDROP);
	for (Ct = 0; Ct < NumEntries; Ct ++)
		{
		if ((TestCS = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Ct)) != (CoordSys *)LB_ERR && TestCS == Active->Coords)
			{
			ListPos = Ct;
			break;
			} // for
		} // for
	WidgetCBSetCurSel(IDC_COORDSDROP, ListPos);
	} // if
else
	WidgetCBSetCurSel(IDC_COORDSDROP, -1);
#endif // WCS_BUILD_VNS

DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // CameraEditGUI::ConfigureWidgets()

/*===========================================================================*/

void CameraEditGUI::SyncWidgets(void)
{

WidgetFISync(IDC_MAXBLUR, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_AAPASSES, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_BLURFAC, WP_FISYNC_NONOTIFY);

WidgetSNSync(IDC_CAMLAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CAMLON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CAMALT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOCLAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOCLON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOCALT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_HEADING, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_PITCH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CAMBANK, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_VIEWARC, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CENTERX, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_CENTERY, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZMINIMUM, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZMAXIMUM, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MAGICZ, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOCALLENGTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FILMSIZE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FOCALDIST, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FSTOP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_MOTIONBLURPCT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_BLURMAXOFF, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASEIN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_EASEOUT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_STEREOSEP, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_STEREOCONVERGENCE, WP_FISYNC_NONOTIFY);

//WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKBEYONDHORIZ, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFIELDS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLIPFIELDS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKBGENABLE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKORTHOGRAPHIC, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVRCAM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKDEPTHOFFIELD, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_MOTIONBLUR, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_BLURENABLE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKZBBLUR, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVELODIST, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKLENSDISTORTION, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFLOATING, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKFOLLOW, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCONVERGENCE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSTEREO, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOTARGETED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIONONTARGETED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOOVERHEAD, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOOPTREF, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOOPT3DOBJ, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCENTERORIGIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOCENTERCENTER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPREVIEWLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPREVIEWCENTER, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPREVIEWRIGHT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORENDERLEFT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORENDERBOTH, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORENDERRIGHT, WP_SRSYNC_NONOTIFY);

} // CameraEditGUI::SyncWidgets

/*===========================================================================*/

void CameraEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_FOCLAT, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED);
WidgetSetDisabled(IDC_FOCLON, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED);
WidgetSetDisabled(IDC_FOCALT, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED);
WidgetSetDisabled(IDC_CHECKBGENABLE, Active->PanoCam);
WidgetSetDisabled(IDC_IMAGEDROP, Active->PanoCam || ! Active->BackgroundImageEnabled);
WidgetSetDisabled(IDC_TARGETDROP, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED);
WidgetSetDisabled(IDC_CHECKVRCAM, Active->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Active->StereoCam || Active->Orthographic);
WidgetSetDisabled(IDC_CHECKORTHOGRAPHIC, Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Active->PanoCam || Active->StereoCam);
WidgetSetDisabled(IDC_HEADING, Active->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
WidgetSetDisabled(IDC_PITCH, Active->CameraType == WCS_EFFECTS_CAMERATYPE_OVERHEAD || Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
WidgetSetDisabled(IDC_CAMBANK, Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
WidgetSetDisabled(IDC_CHECKFLIPFIELDS, ! Active->FieldRender);
WidgetSetDisabled(IDC_MOTIONBLUR, Active->AAPasses <= 1);
WidgetSetDisabled(IDC_MOTIONBLURPCT, Active->AAPasses <= 1 || ! Active->MotionBlur);
WidgetSetDisabled(IDC_BLURFAC, ! Active->BoxFilter);
WidgetSetDisabled(IDC_CHECKZBBLUR, ! Active->BoxFilter || Active->BoxFilterSize < 1);
WidgetSetDisabled(IDC_BLURMAXOFF, ! Active->BoxFilter || Active->BoxFilterSize < 1 || ! Active->ZBufBoxFilter);
WidgetSetDisabled(IDC_CHECKSYNCLENSFOV, ! Active->DepthOfField || Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || (Active->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Active->Orthographic));
WidgetSetDisabled(IDC_FOCALLENGTH, ! Active->DepthOfField);
WidgetSetDisabled(IDC_FILMSIZE, ! Active->DepthOfField);
WidgetSetDisabled(IDC_FOCALDIST, ! Active->DepthOfField);
WidgetSetDisabled(IDC_FSTOP, ! Active->DepthOfField);
WidgetSetDisabled(IDC_EASEIN, ! Active->VelocitySmoothing);
WidgetSetDisabled(IDC_EASEOUT, ! Active->VelocitySmoothing);
WidgetSetDisabled(IDC_CHECKLENSDISTORTION, Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Active->Orthographic || Active->PanoCam);
WidgetSetDisabled(IDC_RADIOCENTERORIGIN, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED || ! Active->TargetObj);
WidgetSetDisabled(IDC_RADIOCENTERCENTER, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED || ! Active->TargetObj);
WidgetSetDisabled(IDC_STEREOSEP, ! Active->StereoCam);
WidgetSetDisabled(IDC_STEREOCONVERGENCE, ! (Active->StereoCam && Active->StereoConvergence));
WidgetSetDisabled(IDC_SETSEPARATION, ! (Active->StereoCam && Active->StereoConvergence));
WidgetSetDisabled(IDC_STEREOTXT, ! (Active->StereoCam && Active->StereoConvergence));
WidgetSetDisabled(IDC_CHECKCONVERGENCE, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIOPREVIEWLEFT, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIOPREVIEWCENTER, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIOPREVIEWRIGHT, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIORENDERLEFT, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIORENDERBOTH, ! Active->StereoCam);
WidgetSetDisabled(IDC_RADIORENDERRIGHT, ! Active->StereoCam);
WidgetSetDisabled(IDC_CHECKSTEREO, Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC || Active->Orthographic || Active->PanoCam);
#ifdef WCS_BUILD_VNS
WidgetSetDisabled(IDC_CHECKPROJECTED, Active->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC);
WidgetSetDisabled(IDC_COORDSDROP, ! (Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Active->Projected));
#endif // WCS_BUILD_VNS

WidgetShow(IDC_SCALE, (Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) || (Active->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Active->Orthographic));
WidgetShow(IDC_VIEWARC, ! ((Active->CameraType == WCS_EFFECTS_CAMERATYPE_PLANIMETRIC) || (Active->CameraType != WCS_EFFECTS_CAMERATYPE_PLANIMETRIC && Active->Orthographic)));
WidgetShow(IDC_TARGETED_TXT, Active->CameraType != WCS_EFFECTS_CAMERATYPE_TARGETED);

} // CameraEditGUI::DisableWidgets

/*===========================================================================*/

// advanced
void CameraEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_HIDDENCONTROLMSG5, false);
	WidgetShow(IDC_HIDDENCONTROLMSG6, false);
	WidgetShow(IDC_HIDDENCONTROLMSG7, false);
	WidgetShow(IDC_HIDDENCONTROLMSG8, false);
	WidgetShow(IDC_HIDDENCONTROLMSG9, false);
	WidgetShow(IDC_CHECKVRCAM, true);
	WidgetShow(IDC_CHECKORTHOGRAPHIC, true);
	WidgetShow(IDC_CHECKSTEREO, true);
	WidgetShow(IDC_CHECKLENSDISTORTION, true);
	WidgetShow(IDC_CHECKPROJECTED, true);
	WidgetShow(IDC_COORDSDROP, true);
	WidgetShow(IDC_EDITCOORDS, true);
	WidgetShow(IDC_CHECKDEPTHOFFIELD, true);
	WidgetShow(IDC_CHECKSYNCLENSFOV, true);
	WidgetShow(IDC_FOCALLENGTH, true);
	WidgetShow(IDC_FOCALDIST, true);
	WidgetShow(IDC_FSTOP, true);
	WidgetShow(IDC_FILMSIZE, true);
	WidgetShow(IDC_RADIUS_TXT, true);
	WidgetShow(IDC_MAXBLUR, true);
	WidgetShow(IDC_AAPASSES, true);
	WidgetShow(IDC_MOTIONBLUR, true);
	WidgetShow(IDC_MOTIONBLURPCT, true);
	WidgetShow(IDC_BLURENABLE, true);
	WidgetShow(IDC_BLURFAC, true);
	WidgetShow(IDC_CHECKZBBLUR, true);
	WidgetShow(IDC_BLURMAXOFF, true);
	WidgetShow(IDC_FOGFULL, true);
	WidgetShow(IDC_TARGETOBJ_TXT, true);
	WidgetShow(IDC_TARGETDROP, true);
	WidgetShow(IDC_FREETARGET, true);
	WidgetShow(IDC_AIM_TXT, true);
	WidgetShow(IDC_RADIOCENTERCENTER, true);
	WidgetShow(IDC_RADIOCENTERORIGIN, true);
	WidgetShow(IDC_CENTERSHIFT_TXT, true);
	WidgetShow(IDC_CENTERX, true);
	WidgetShow(IDC_CENTERY, true);
	WidgetShow(IDC_CHECKVELODIST, true);
	WidgetShow(IDC_EASEIN, true);
	WidgetShow(IDC_EASEOUT, true);
	WidgetShow(IDC_STEREOSEP, true);
	WidgetShow(IDC_CHECKCONVERGENCE, true);
	WidgetShow(IDC_STEREOTXT, true);
	WidgetShow(IDC_STEREOCONVERGENCE, true);
	WidgetShow(IDC_SETSEPARATION, true);
	WidgetShow(IDC_PREVIEW_TXT, true);
	WidgetShow(IDC_RADIOPREVIEWLEFT, true);
	WidgetShow(IDC_RADIOPREVIEWCENTER, true);
	WidgetShow(IDC_RADIOPREVIEWRIGHT, true);
	WidgetShow(IDC_RENDER_TXT, true);
	WidgetShow(IDC_RADIORENDERLEFT, true);
	WidgetShow(IDC_RADIORENDERBOTH, true);
	WidgetShow(IDC_RADIORENDERRIGHT, true);
	WidgetShow(IDC_CHECKFIELDS, true);
	WidgetShow(IDC_CHECKFLIPFIELDS, true);
	WidgetShow(IDC_OPTIMIZE_TXT, true);
	WidgetShow(IDC_RADIOOPTREF, true);
	WidgetShow(IDC_RADIOOPT3DOBJ, true);
	WidgetShow(IDC_MAGICZ, true);
	WidgetShow(IDC_ZMINIMUM, true);
	WidgetShow(IDC_ZMAXIMUM, true);
	WidgetShow(IDC_CHECKBEYONDHORIZ, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_HIDDENCONTROLMSG5, true);
	WidgetShow(IDC_HIDDENCONTROLMSG6, true);
	WidgetShow(IDC_HIDDENCONTROLMSG7, true);
	WidgetShow(IDC_HIDDENCONTROLMSG8, true);
	WidgetShow(IDC_HIDDENCONTROLMSG9, true);
	WidgetShow(IDC_CHECKVRCAM, false);
	WidgetShow(IDC_CHECKORTHOGRAPHIC, false);
	WidgetShow(IDC_CHECKSTEREO, false);
	WidgetShow(IDC_CHECKLENSDISTORTION, false);
	WidgetShow(IDC_CHECKPROJECTED, false);
	WidgetShow(IDC_COORDSDROP, false);
	WidgetShow(IDC_EDITCOORDS, false);
	WidgetShow(IDC_CHECKDEPTHOFFIELD, false);
	WidgetShow(IDC_CHECKSYNCLENSFOV, false);
	WidgetShow(IDC_FOCALLENGTH, false);
	WidgetShow(IDC_FOCALDIST, false);
	WidgetShow(IDC_FSTOP, false);
	WidgetShow(IDC_FILMSIZE, false);
	WidgetShow(IDC_RADIUS_TXT, false);
	WidgetShow(IDC_MAXBLUR, false);
	WidgetShow(IDC_AAPASSES, false);
	WidgetShow(IDC_MOTIONBLUR, false);
	WidgetShow(IDC_MOTIONBLURPCT, false);
	WidgetShow(IDC_BLURENABLE, false);
	WidgetShow(IDC_BLURFAC, false);
	WidgetShow(IDC_CHECKZBBLUR, false);
	WidgetShow(IDC_BLURMAXOFF, false);
	WidgetShow(IDC_FOGFULL, false);
	WidgetShow(IDC_TARGETOBJ_TXT, false);
	WidgetShow(IDC_TARGETDROP, false);
	WidgetShow(IDC_FREETARGET, false);
	WidgetShow(IDC_AIM_TXT, false);
	WidgetShow(IDC_RADIOCENTERCENTER, false);
	WidgetShow(IDC_RADIOCENTERORIGIN, false);
	WidgetShow(IDC_CENTERSHIFT_TXT, false);
	WidgetShow(IDC_CENTERX, false);
	WidgetShow(IDC_CENTERY, false);
	WidgetShow(IDC_CHECKVELODIST, false);
	WidgetShow(IDC_EASEIN, false);
	WidgetShow(IDC_EASEOUT, false);
	WidgetShow(IDC_STEREOSEP, false);
	WidgetShow(IDC_CHECKCONVERGENCE, false);
	WidgetShow(IDC_STEREOTXT, false);
	WidgetShow(IDC_STEREOCONVERGENCE, false);
	WidgetShow(IDC_SETSEPARATION, false);
	WidgetShow(IDC_PREVIEW_TXT, false);
	WidgetShow(IDC_RADIOPREVIEWLEFT, false);
	WidgetShow(IDC_RADIOPREVIEWCENTER, false);
	WidgetShow(IDC_RADIOPREVIEWRIGHT, false);
	WidgetShow(IDC_RENDER_TXT, false);
	WidgetShow(IDC_RADIORENDERLEFT, false);
	WidgetShow(IDC_RADIORENDERBOTH, false);
	WidgetShow(IDC_RADIORENDERRIGHT, false);
	WidgetShow(IDC_CHECKFIELDS, false);
	WidgetShow(IDC_CHECKFLIPFIELDS, false);
	WidgetShow(IDC_OPTIMIZE_TXT, false);
	WidgetShow(IDC_RADIOOPTREF, false);
	WidgetShow(IDC_RADIOOPT3DOBJ, false);
	WidgetShow(IDC_MAGICZ, false);
	WidgetShow(IDC_ZMINIMUM, false);
	WidgetShow(IDC_ZMAXIMUM, false);
	WidgetShow(IDC_CHECKBEYONDHORIZ, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // CameraEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void CameraEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // CameraEditGUI::Cancel

/*===========================================================================*/

void CameraEditGUI::EditImage(void)
{

if (Active->Img && Active->Img->GetRaster())
	{
	GlobalApp->AppImages->SetActive(Active->Img->GetRaster());
	GlobalApp->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
		WCS_TOOLBAR_ITEM_ILG, 0);
	} // if

} // CameraEditGUI::DoEditImage()

/*===========================================================================*/

void CameraEditGUI::Name(void)
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

} // CameraEditGUI::Name()

/*===========================================================================*/

void CameraEditGUI::SelectNewImage(void)
{
long Current;
Raster *NewRast, *MadeRast = NULL;

Current = WidgetCBGetCurSel(IDC_IMAGEDROP);
if (((NewRast = (Raster *)WidgetCBGetItemData(IDC_IMAGEDROP, Current, 0)) != (Raster *)LB_ERR && NewRast)
	|| (MadeRast = NewRast = GlobalApp->AppImages->AddRequestRaster()))
	{
	Active->SetRaster(NewRast);
	if (MadeRast)
		{
		GlobalApp->AppImages->SetActive(MadeRast);
		} // if
	} // if

} // CameraEditGUI::SelectNewImage

/*===========================================================================*/

void CameraEditGUI::SelectNewTarget(void)
{
Object3DEffect *OldObj, *NewObj, *MadeObj = NULL;
long Current;
NotifyTag Changes[2];

Current = WidgetCBGetCurSel(IDC_TARGETDROP);
if (((NewObj = (Object3DEffect *)WidgetCBGetItemData(IDC_TARGETDROP, Current, 0)) != (Object3DEffect *)LB_ERR && NewObj)
	|| (MadeObj = NewObj = (Object3DEffect *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_OBJECT3D, NULL, NULL)))
	{
	OldObj = Active->TargetObj;
	if (MadeObj)
		{
		if (! MadeObj->OpenInputFileRequest())
			{
			Changes[0] = MAKE_ID(MadeObj->GetNotifyClass(), MadeObj->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
			Changes[1] = NULL;
			EffectsHost->RemoveEffect(MadeObj);
			MadeObj = NULL;
			NewObj = OldObj;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			} // if
		} // if
	Active->SetTarget(NewObj);
	} // if

} // CameraEditGUI::SelectNewTarget

/*===========================================================================*/

void CameraEditGUI::FreeTarget(void)
{

Active->RemoveRAHost(Active->TargetObj);

} // CameraEditGUI::FreeTarget

/*===========================================================================*/

void CameraEditGUI::SyncLens(void)
{
double HFOV, NewLensLen, FilmSize;

FilmSize = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].CurValue;
HFOV = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
NewLensLen = (FilmSize / 2.0) / tan((HFOV / 2.0) * PiOver180);

Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH].SetCurValue(NewLensLen);

} // CameraEditGUI::SyncLens

/*===========================================================================*/

void CameraEditGUI::SyncFOV(void)
{
double NewHFOV, LensLen, FilmSize;

FilmSize = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].CurValue;
LensLen = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FOCALLENGTH].CurValue;
NewHFOV = 2.0 * atan((FilmSize / 2.0) / LensLen) / PiOver180;

Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].SetCurValue(NewHFOV);

} // CameraEditGUI::SyncFOV

/*===========================================================================*/

void CameraEditGUI::SetSeparation(void)
{
double HFOV, NewLensLen, FilmSize, NewSeparation;

FilmSize = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_FILMSIZE].CurValue;
HFOV = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_HFOV].CurValue;
NewLensLen = (FilmSize / 2.0) / tan((HFOV / 2.0) * PiOver180);

NewSeparation = Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOCONVERGENCE].CurValue / NewLensLen;

Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_STEREOSEPARATION].SetCurValue(NewSeparation);

} // CameraEditGUI::SetSeparation

/*===========================================================================*/

void CameraEditGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *NewObj;
long Current;

Current = WidgetCBGetCurSel(IDC_COORDSDROP);
if (((NewObj = (CoordSys *)WidgetCBGetItemData(IDC_COORDSDROP, Current, 0)) != (CoordSys *)LB_ERR && NewObj)
	|| (NewObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Active->SetCoords(NewObj);
	} // if
#endif // WCS_BUILD_VNS

} // CameraEditGUI::SelectNewCoords

/*===========================================================================*/

bool CameraEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->TargetObj || Active->DepthOfField || Active->BoxFilter || Active->AAPasses > 1 ||
	Active->FieldRender || Active->VelocitySmoothing || Active->Orthographic || Active->PanoCam || Active->LensDistortion || 
	Active->StereoCam || Active->Projected || Active->RenderBeyondHorizon || 
	Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERX].CurValue != .5 ||
	Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_CENTERY].CurValue != .5 ||
	Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_MAGICZ].CurValue != 0.0 ||
	Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMIN].CurValue > 0.0 ||
	Active->AnimPar[WCS_EFFECTS_CAMERA_ANIMPAR_ZMAX].CurValue < ConvertToMeters(10.0, WCS_USEFUL_UNIT_AU) ? true : false);

} // CameraEditGUI::QueryLocalDisplayAdvancedUIVisibleState

