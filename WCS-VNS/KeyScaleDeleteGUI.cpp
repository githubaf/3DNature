// KeyScaleDeleteGUI.cpp
// Code for Key Scale and Delete GUI
// Built from KeyDeleteShortGUI.cpp on 1/13/00 by Gary R. Huber
// Copyright 2000 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "KeyScaleDeleteGUI.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Interactive.h"
#include "Notify.h"
#include "EffectsLib.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "RasterAnimHost.h"
#include "SceneViewGUI.h"
#include "Conservatory.h"
#include "resource.h"

extern WCSApp *GlobalApp;

NativeGUIWin KeyScaleDeleteGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // KeyScaleDeleteGUI::Open

/*===========================================================================*/

NativeGUIWin KeyScaleDeleteGUI::Construct(void)
{

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_KEY_SCALEDELETE, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);

} // KeyScaleDeleteGUI::Construct

/*===========================================================================*/

KeyScaleDeleteGUI::KeyScaleDeleteGUI(Project *ProjSource, EffectsLib *EffectsSource, RasterAnimHost *CritterSupplied, int ScaleOrRemove)
: GUIFenetre('KDSG', this, "Scale or Remove Key Frames") // Yes, I know...
{
double RangeDefaults[3] = {FLT_MAX, 0.0, 0.0};	// set increment to 0 to use project frame rate
RasterAnimHostProperties Prop;
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};
RasterAnimHost *Sib;

ConstructError = 0;
EffectsHost = EffectsSource;
ProjHost = ProjSource;
SiblingsExist = 0;
RootObj = NULL;
HostCritterNameStr[0] = 0;
RootCritterNameStr[0] = 0;
ObjClassNameStr[0] = 0;
RootObjKeyRange[0] = RootObjKeyRange[1] = CurObjKeyRange[0] = CurObjKeyRange[1] = 
	ProjectKeyRange[0] = ProjectKeyRange[1] = GroupKeyRange[0] = GroupKeyRange[1] = 0.0;
RootObjClass = 0;
HostCritterAnimated = RootObjAnimated = 0;
HostCritter = NULL;

if (GlobalApp && GlobalApp->MainProj && GlobalApp->MainProj->Interactive)
	{
	if ((FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate()) <= 0.0)
		FrameRate = 30.0;
	} // if
else
	FrameRate = 30.0;

//RangeDefaults[2] = 1.0 / FrameRate;	by setting this to 0 we assure that we use whatever the current project frame rate is

OldFrame[0].SetRangeDefaults(RangeDefaults);
OldFrame[1].SetRangeDefaults(RangeDefaults);
NewFrame[0].SetRangeDefaults(RangeDefaults);
NewFrame[1].SetRangeDefaults(RangeDefaults);

OldFrame[0].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
OldFrame[1].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
NewFrame[0].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);
NewFrame[1].SetMetricType(WCS_ANIMDOUBLE_METRIC_TIME);

if (! CritterSupplied)
	HostCritter = RasterAnimHost::GetActiveRAHost();
else
	{
	if ((unsigned long)CritterSupplied >= WCS_EFFECTSSUBCLASS_LAKE && (unsigned long)CritterSupplied < WCS_MAXIMPLEMENTED_EFFECTS)
		{
		RootObjClass = (long)CritterSupplied;
		strcpy(ObjClassNameStr, EffectsHost->GetEffectTypeName(RootObjClass));
		} // else if
	else if ((unsigned long)CritterSupplied >= WCS_MAXIMPLEMENTED_EFFECTS)
		HostCritter = CritterSupplied;
	} // else

if (HostCritter)
	{
	RootObj = HostCritter->GetRAHostRoot();
	if (HostCritter->GetRAHostName())
		{
		strcpy(HostCritterNameStr, HostCritter->GetRAHostName());
		} // if
	if (HostCritter->RAParent)
		{
		strcat(HostCritterNameStr, HostCritterNameStr[0] ? ", ": "");
		strcat(HostCritterNameStr, HostCritter->RAParent->GetCritterName(HostCritter));
		} // if
	if (HostCritterNameStr[0])
		SetTitle(HostCritterNameStr);

	SiblingsExist = (HostCritter->RAParent && (Sib = HostCritter->RAParent->GetNextGroupSibling(HostCritter))
		&& (Sib != HostCritter));

	Prop.PropMask = WCS_RAHOST_MASKBIT_FLAGS | WCS_RAHOST_MASKBIT_KEYRANGE | WCS_RAHOST_MASKBIT_TYPENUMBER | WCS_RAHOST_MASKBIT_NAME;
	Prop.FlagsMask = WCS_RAHOST_FLAGBIT_ANIMATED;
	RootObj->GetRAHostProperties(&Prop);
	RootObjKeyRange[0] = Prop.KeyNodeRange[0];
	RootObjKeyRange[1] = Prop.KeyNodeRange[1];
	RootObjClass = Prop.TypeNumber;
	strcpy(RootCritterNameStr, Prop.Name ? Prop.Name: "");
	strcpy(ObjClassNameStr, RootObjClass < WCS_MAXIMPLEMENTED_EFFECTS ? EffectsHost->GetEffectTypeName(RootObjClass): "");
	RootObjAnimated = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;

	HostCritter->GetRAHostProperties(&Prop);
	CurObjKeyRange[0] = Prop.KeyNodeRange[0];
	CurObjKeyRange[1] = Prop.KeyNodeRange[1];
	HostCritterAnimated = (Prop.Flags & WCS_RAHOST_FLAGBIT_ANIMATED) ? 1: 0;
	} // if
else if (! CritterSupplied)
	{
	if (GlobalApp->GUIWins->SAG)
		{
		GlobalApp->GUIWins->SAG->GetSelectedObjectAndCategory(RootObjClass);
		strcpy(ObjClassNameStr, RootObjClass >= WCS_EFFECTSSUBCLASS_LAKE && RootObjClass < WCS_MAXIMPLEMENTED_EFFECTS ? EffectsHost->GetEffectTypeName(RootObjClass): "");
		} // if
	} // else

FrameOp = WCS_KEYOPERATION_ONEKEY;
Operation = ScaleOrRemove;

Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;
Prop.ItemOperator = WCS_KEYOPERATION_ALLOBJ;
EffectsHost->GetRAHostProperties(&Prop);
ProjectKeyRange[0] = Prop.KeyNodeRange[0];
ProjectKeyRange[1] = Prop.KeyNodeRange[1];

Prop.TypeNumber = RootObjClass;
Prop.ItemOperator = WCS_KEYOPERATION_OBJCLASS;
EffectsHost->GetRAHostProperties(&Prop);
GroupKeyRange[0] = Prop.KeyNodeRange[0];
GroupKeyRange[1] = Prop.KeyNodeRange[1];

if (SiblingsExist && GlobalApp->MainProj->GetKeyGroupMode() && HostCritterAnimated)
	ItemOp = WCS_KEYOPERATION_CUROBJGROUP;
else if (RootObj && RootObj == HostCritter && RootObjAnimated)
	ItemOp = WCS_KEYOPERATION_ROOTOBJ;
else if (HostCritter && HostCritterAnimated)
	ItemOp = WCS_KEYOPERATION_CUROBJ;
else if (EffectsHost->GetListPtr(RootObjClass))
	ItemOp = WCS_KEYOPERATION_OBJCLASS;
else
	ItemOp = WCS_KEYOPERATION_ALLOBJ;

ProjHost->Interactive->RegisterClient(this, AllIntercommonEvents);
ProjHost->RegisterClient(this, AllProjPrefsEvents);

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // KeyScaleDeleteGUI::KeyScaleDeleteGUI

/*===========================================================================*/

KeyScaleDeleteGUI::~KeyScaleDeleteGUI()
{

ProjHost->Interactive->RemoveClient(this);
ProjHost->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // KeyScaleDeleteGUI::~KeyScaleDeleteGUI()

/*===========================================================================*/

long KeyScaleDeleteGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_DKG, 0);

return(0);

} // KeyScaleDeleteGUI::HandleCloseWin

/*===========================================================================*/

long KeyScaleDeleteGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(064, 64);
switch(ButtonID)
	{
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DKG, 0);
		break;
		} // 
	case IDC_OPERATE:
		{
		Operate();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_DKG, 0);
		break;
		} // 
	default:
		break;
	} // ButtonID

return(0);

} // KeyScaleDeleteGUI::HandleButtonClick

/*===========================================================================*/

long KeyScaleDeleteGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

ResetTimes();
DisableWidgets();

return(0);

} // KeyScaleDeleteGUI::HandleSRChange

/*===========================================================================*/

void KeyScaleDeleteGUI::HandleNotifyEvent(void)
{

DisableWidgets();

} // KeyScaleDeleteGUI::HandleNotifyEvent()

/*===========================================================================*/

void KeyScaleDeleteGUI::ConfigureWidgets(void)
{
char TempStr[256];

WidgetSNConfig(IDC_OLDLOWFRAME, &OldFrame[0]);
WidgetSNConfig(IDC_OLDHIGHFRAME, &OldFrame[1]);
WidgetSNConfig(IDC_NEWLOWFRAME, &NewFrame[0]);
WidgetSNConfig(IDC_NEWHIGHFRAME, &NewFrame[1]);

ConfigureSR(NativeWin, IDC_RADIODELETEKEYS, IDC_RADIODELETEKEYS, &Operation, SRFlag_Char, WCS_KEYOPERATION_DELETE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIODELETEKEYS, IDC_RADIOSCALEKEYS, &Operation, SRFlag_Char, WCS_KEYOPERATION_SCALE, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOCURRENTOBJ, IDC_RADIOCURRENTOBJ, &ItemOp, SRFlag_Char, WCS_KEYOPERATION_CUROBJ, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCURRENTOBJ, IDC_RADIOCURRENTGROUP, &ItemOp, SRFlag_Char, WCS_KEYOPERATION_CUROBJGROUP, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCURRENTOBJ, IDC_RADIOROOTOBJ, &ItemOp, SRFlag_Char, WCS_KEYOPERATION_ROOTOBJ, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCURRENTOBJ, IDC_RADIOOBJCLASS, &ItemOp, SRFlag_Char, WCS_KEYOPERATION_OBJCLASS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCURRENTOBJ, IDC_RADIOALLOBJECTS, &ItemOp, SRFlag_Char, WCS_KEYOPERATION_ALLOBJ, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOONEFRAME, IDC_RADIOONEFRAME, &FrameOp, SRFlag_Char, WCS_KEYOPERATION_ONEKEY, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOONEFRAME, IDC_RADIOFRAMERANGE, &FrameOp, SRFlag_Char, WCS_KEYOPERATION_KEYRANGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOONEFRAME, IDC_RADIOALLFRAMES, &FrameOp, SRFlag_Char, WCS_KEYOPERATION_ALLKEYS, NULL, NULL);

WidgetSetText(IDC_RADIOCURRENTOBJ, HostCritterNameStr);
strcpy(TempStr, SiblingsExist ? HostCritterNameStr: "");
strcat(TempStr, TempStr[0] ? " Group": "");
WidgetSetText(IDC_RADIOCURRENTGROUP, TempStr);
WidgetSetText(IDC_RADIOROOTOBJ, RootCritterNameStr);
WidgetSetText(IDC_RADIOOBJCLASS, ObjClassNameStr);

ResetTimes();
DisableWidgets();

} // KeyScaleDeleteGUI::ConfigureWidgets()

/*===========================================================================*/

void KeyScaleDeleteGUI::DisableWidgets(void)
{

WidgetSNSync(IDC_OLDLOWFRAME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_OLDHIGHFRAME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NEWLOWFRAME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_NEWHIGHFRAME, WP_FISYNC_NONOTIFY);

WidgetSetDisabled(IDC_RADIOCURRENTOBJ, ! HostCritter || HostCritter == RootObj || ! HostCritterAnimated);
WidgetSetDisabled(IDC_RADIOCURRENTGROUP, ! SiblingsExist);
WidgetSetDisabled(IDC_RADIOROOTOBJ, ! RootObj || ! RootObjAnimated);
WidgetSetDisabled(IDC_RADIOOBJCLASS, ! EffectsHost->GetListPtr(RootObjClass));

WidgetSetText(IDC_SCALEDELETETEXT, Operation == WCS_KEYOPERATION_DELETE ? "Remove": FrameOp == WCS_KEYOPERATION_ONEKEY ? "Move": "Scale");
WidgetSetText(IDC_SCALEDELETETEXT2, FrameOp == WCS_KEYOPERATION_ONEKEY ? "key frame at": FrameOp == WCS_KEYOPERATION_KEYRANGE ? "all key frames between": "all key frames");
WidgetSetText(IDC_SCALEDELETETEXT3, FrameOp == WCS_KEYOPERATION_ONEKEY ? "to": "to the range from");

WidgetShow(IDC_OLDLOWFRAME, FrameOp != WCS_KEYOPERATION_ALLKEYS);
WidgetShow(IDC_ANDTEXT, FrameOp == WCS_KEYOPERATION_KEYRANGE);
WidgetShow(IDC_OLDHIGHFRAME, FrameOp == WCS_KEYOPERATION_KEYRANGE);

WidgetShow(IDC_SCALEDELETETEXT3, Operation == WCS_KEYOPERATION_SCALE);
WidgetShow(IDC_NEWLOWFRAME, Operation == WCS_KEYOPERATION_SCALE);
WidgetShow(IDC_TOTEXT, Operation == WCS_KEYOPERATION_SCALE && FrameOp != WCS_KEYOPERATION_ONEKEY);
WidgetShow(IDC_NEWHIGHFRAME, Operation == WCS_KEYOPERATION_SCALE && FrameOp != WCS_KEYOPERATION_ONEKEY);

} // KeyScaleDeleteGUI::DisableWidgets

/*===========================================================================*/

void KeyScaleDeleteGUI::ResetTimes(void)
{
double CurTime;

if (FrameOp == WCS_KEYOPERATION_ONEKEY)
	{
	CurTime = GlobalApp->MainProj->Interactive->GetActiveTime();

	OldFrame[0].SetValue(CurTime); 
	OldFrame[1].SetValue(CurTime); 
	NewFrame[0].SetValue(CurTime); 
	NewFrame[1].SetValue(CurTime); 
	} // if
else if (ItemOp == WCS_KEYOPERATION_CUROBJ || ItemOp == WCS_KEYOPERATION_CUROBJGROUP)
	{
	OldFrame[0].SetValue(CurObjKeyRange[0]); 
	OldFrame[1].SetValue(CurObjKeyRange[1]); 
	NewFrame[0].SetValue(CurObjKeyRange[0]); 
	NewFrame[1].SetValue(CurObjKeyRange[1]); 
	} // if
else if (ItemOp == WCS_KEYOPERATION_ROOTOBJ)
	{
	OldFrame[0].SetValue(RootObjKeyRange[0]); 
	OldFrame[1].SetValue(RootObjKeyRange[1]); 
	NewFrame[0].SetValue(RootObjKeyRange[0]); 
	NewFrame[1].SetValue(RootObjKeyRange[1]); 
	} // if
else if (ItemOp == WCS_KEYOPERATION_OBJCLASS)
	{
	OldFrame[0].SetValue(GroupKeyRange[0]); 
	OldFrame[1].SetValue(GroupKeyRange[1]); 
	NewFrame[0].SetValue(GroupKeyRange[0]); 
	NewFrame[1].SetValue(GroupKeyRange[1]); 
	} // if
else
	{
	OldFrame[0].SetValue(ProjectKeyRange[0]); 
	OldFrame[1].SetValue(ProjectKeyRange[1]); 
	NewFrame[0].SetValue(ProjectKeyRange[0]); 
	NewFrame[1].SetValue(ProjectKeyRange[1]); 
	} // else

} // KeyScaleDeleteGUI::ResetTimes

/*===========================================================================*/

void KeyScaleDeleteGUI::Operate(void)
{
RasterAnimHostProperties Prop;

Prop.TypeNumber = RootObjClass;
Prop.ItemOperator = ItemOp;
Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;

// get the latest keyframe range - only needed for scaling all keys
if (FrameOp == WCS_KEYOPERATION_ALLKEYS && Operation == WCS_KEYOPERATION_SCALE)
	{
	if (ItemOp == WCS_KEYOPERATION_ALLOBJ || ItemOp == WCS_KEYOPERATION_OBJCLASS)
		EffectsHost->GetRAHostProperties(&Prop);
	else if (ItemOp == WCS_KEYOPERATION_ROOTOBJ && RootObj)
		RootObj->GetRAHostProperties(&Prop);
	else if ((ItemOp == WCS_KEYOPERATION_CUROBJ || ItemOp == WCS_KEYOPERATION_CUROBJGROUP) && HostCritter)
		HostCritter->GetRAHostProperties(&Prop);
	} // if
else
	{
	Prop.KeyNodeRange[0] = OldFrame[0].CurValue;
	Prop.KeyNodeRange[1] = OldFrame[1].CurValue;
	} // else

Prop.ItemOperator = ItemOp;
Prop.FrameOperator = FrameOp;
Prop.KeyframeOperation = Operation;
Prop.TypeNumber = RootObjClass;
Prop.NewKeyNodeRange[0] = NewFrame[0].CurValue;
Prop.NewKeyNodeRange[1] = NewFrame[1].CurValue;
Prop.PropMask = WCS_RAHOST_MASKBIT_KEYRANGE;

switch (ItemOp)
	{
	case WCS_KEYOPERATION_CUROBJ:
		{
		if (HostCritter)
			HostCritter->SetRAHostProperties(&Prop);
		break;
		} // 
	case WCS_KEYOPERATION_CUROBJGROUP:
		{
		if (HostCritter)
			HostCritter->SetRAHostProperties(&Prop);
		break;
		} // 
	case WCS_KEYOPERATION_ROOTOBJ:
		{
		if (RootObj)
			RootObj->SetRAHostProperties(&Prop);
		break;
		} // 
	case WCS_KEYOPERATION_OBJCLASS:
		{
		if (EffectsHost)
			EffectsHost->SetRAHostProperties(&Prop);	// EffectsLib is not an RAHost but it has this method
		break;
		} // 
	case WCS_KEYOPERATION_ALLOBJ:
		{
		if (EffectsHost)
			EffectsHost->SetRAHostProperties(&Prop);	// EffectsLib is not an RAHost but it has this method
		break;
		} // 
	default:
		break;
	} // switch

} // KeyScaleDeleteGUI::Operate
