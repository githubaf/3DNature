// Object3DEditGUI.cpp
// Code for Object3D editor
// Built from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Object3DEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Database.h"
#include "Interactive.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "MaterialEditGUI.h"
#include "resource.h"
#include "Lists.h"
#ifdef WCS_BUILD_VNS
#include "DBFilterEvent.h"
#endif // WCS_BUILD_VNS

char *Object3DEditGUI::TabNames[WCS_OBJECT3DGUI_NUMTABS] = {"General", "Materials", "Size && Position", "Randomize", "Align", "Shadows", "Misc"};

long Object3DEditGUI::ActivePage;
long Object3DEditGUI::ActiveParam;
// advanced
long Object3DEditGUI::DisplayAdvanced;

static double lastScaleX, lastScaleY, lastScaleZ;

NativeGUIWin Object3DEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // Object3DEditGUI::Open

/*===========================================================================*/

NativeGUIWin Object3DEditGUI::Construct(void)
{
int TabIndex;

if (!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(WCS_FENETRE_GENERIC_EDIT_TEMPLATE, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_OBJECT3D_GENERAL_VNS, 0, 0);
	CreateSubWinFromTemplate(IDD_OBJECT3D_MATERIAL, 0, 1);
	CreateSubWinFromTemplate(IDD_OBJECT3D_SIZEPOS, 0, 2);
	CreateSubWinFromTemplate(IDD_OBJECT3D_RANDOMIZE, 0, 3);
	CreateSubWinFromTemplate(IDD_OBJECT3D_ALIGN, 0, 4);
	CreateSubWinFromTemplate(IDD_OBJECT3D_LIGHTING, 0, 5);
	CreateSubWinFromTemplate(IDD_OBJECT3D_MISC, 0, 6);

	if (NativeWin)
		{
		for (TabIndex = 0; TabIndex < WCS_OBJECT3DGUI_NUMTABS; TabIndex ++)
			{
			WidgetTCInsertItem(IDC_TAB1, TabIndex, TabNames[TabIndex]);
			} // for
		WidgetTCSetCurSel(IDC_TAB1, ActivePage);
		for(TabIndex = WCS_USEFUL_UNIT_MILLIMETER; TabIndex <= WCS_USEFUL_UNIT_MILE_US_STATUTE; TabIndex ++)
			{
			WidgetCBAddEnd(IDC_UNITSDROP, GetUnitName(TabIndex));
			} // for
		WidgetTCInsertItem(IDC_TAB2, 0, "Scale (%)");
		WidgetTCInsertItem(IDC_TAB2, 1, "Rotate (deg)");
		WidgetTCInsertItem(IDC_TAB2, 2, "Move (m)");

		WidgetTCInsertItem(IDC_TAB3, 0, "Scale");
		WidgetTCInsertItem(IDC_TAB3, 1, "Rotate");
		WidgetTCInsertItem(IDC_TAB3, 2, "Object Pos.");
		WidgetTCInsertItem(IDC_TAB3, 3, "Vertex Pos.");

		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
		ShowPanel(0, ActivePage);
		ConfigureWidgets();
		lastScaleX = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
		lastScaleY = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
		lastScaleZ = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
		} // if
	} // if
 
return (NativeWin);

} // Object3DEditGUI::Construct

/*===========================================================================*/

Object3DEditGUI::Object3DEditGUI(EffectsLib *EffectsSource, Database *DBSource, Object3DEffect *ActiveSource)
: GUIFenetre('3DOE', this, "3D Object Editor"), CommonComponentEditor((GeneralEffect **)(&Active), (Fenetre *)this)
{
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED), 
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_OBJECT3D, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
			/* query drop */	MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
			/* advanced */		MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GLOBALADVANCED, 0),
								0};
char NameStr[256];

ConstructError = 0;
EffectsHost = EffectsSource;
DBHost = DBSource;
Active = ActiveSource;
ActiveMaterial = NULL;

if (EffectsSource && ActiveSource)
	{
	sprintf(NameStr, "3D Object Editor - %s", Active->GetName());
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

} // Object3DEditGUI::Object3DEditGUI

/*===========================================================================*/

Object3DEditGUI::~Object3DEditGUI()
{
// ensure we don't destruct the shared thumbnail. That would be bad.
TempRast.Thumb = NULL;

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // Object3DEditGUI::~Object3DEditGUI()

/*===========================================================================*/

long Object3DEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_OEG, 0);

return(0);

} // Object3DEditGUI::HandleCloseWin

/*===========================================================================*/

// advanced
long Object3DEditGUI::HandleShowAdvanced(NativeGUIWin NW, bool NewState)
{

DisplayAdvanced = NewState;
Active->SetDisplayAdvanced(EffectsHost, (UBYTE)DisplayAdvanced);
DisplayAdvancedFeatures();
return(1);

} // Object3DEditGUI::HandleShowAdvanced

/*===========================================================================*/

long Object3DEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
// IDC_LOAD is overridden and handled by us
if (ButtonID != IDC_LOAD) HandleCommonEvent(ButtonID, EffectsHost, Active, DBHost);
NotifyTag Changes[2];

Changes[1] = 0;

switch (ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_OEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} // 
	case IDC_LOAD:
		{
		NewObject();
		break;
		} //
	case IDC_HARDLINK:
		{
		Active->HardLinkVectors(DBHost);
		break;
		} // IDC_HARDLINK
	case IDC_APPLYSCALE:
		{
		Active->ApplyUnitScaling();
		break;
		} // IDC_APPLYSCALE
	case IDC_EDITMATERIAL:
		{
		EditMaterial();
		break;
		} // IDC_EDITMATERIAL
	case IDC_REPLACEMATERIAL:
		{
		ReplaceMaterial();
		break;
		} // IDC_REPLACEMATERIAL
	case IDC_FREEVECTOR:
		{
		Active->SetAlignVec(NULL);
		break;
		} // IDC_FREEVECTOR
	case IDC_CONSTRAIN:
		Active->lockScales = WidgetGetCheck(IDC_CONSTRAIN);
		break;
	default:
		break;
	} // ButtonID

return(0);

} // Object3DEditGUI::HandleButtonClick

/*===========================================================================*/

long Object3DEditGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(089, 89);
switch (CtrlID)
	{
	case IDC_MATERIALLIST:
		{
		SelectMaterial();
		break;
		} // group list
	default:
		break;
	} // switch CtrlID

return (0);

} // Object3DEditGUI::HandleListSel

/*===========================================================================*/

long Object3DEditGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_MATERIALLIST:
		{
		EditMaterial();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // Object3DEditGUI::HandleListDoubleClick

/*===========================================================================*/

long Object3DEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_UNITSDROP:
		{
		NewUnits();
		break;
		}
	case IDC_VECTORDROP:
		{
		NewAlignVec();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // Object3DEditGUI::HandleCBChange

/*===========================================================================*/

long Object3DEditGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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

} // Object3DEditGUI::HandleStringLoseFocus

/*===========================================================================*/

long Object3DEditGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{

if (ScrollCode)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			Active->SetAlignBias((double)ScrollPos / 100.0);
			break;
			}
		} // switch
	return(0);
	} // if
else
	{
	return(5); // default scroll amount
	} // else

} // Object3DEditGUI::HandleScroll

/*===========================================================================*/

long Object3DEditGUI::HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID)
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
			case 6:
				{
				ShowPanel(0, 6);
				break;
				} // 5
			default:
				{
				ShowPanel(0, 0);
				NewPageID = 0;
				break;
				} // 0
			} // switch
		ActivePage = NewPageID;
		break;
		} // TAB1, the main window tab control
	case IDC_TAB2:
		{
		switch (NewPageID)
			{
			case 1: // rotate
				{
				ActiveParam = 1;
				WidgetShow(IDC_CONSTRAIN, FALSE);
				SetWidgetsVisible();
				break;
				} // 1
			case 2: // move
				{
				ActiveParam = 2;
				WidgetShow(IDC_CONSTRAIN, FALSE);
				SetWidgetsVisible();
				break;
				} // 2
			default: // scale
				{
				ActiveParam = 0;
				WidgetShow(IDC_CONSTRAIN, TRUE);
				SetWidgetsVisible();
				break;
				} // 0
			} // switch
		break;
		} // TAB2, Scale/Rotate/Move subtab on Size & Position tab
	case IDC_TAB3:
		{
		NotifyTag Changes[2];
		Changes[1] = 0;
		switch (NewPageID)
			{
			case 1: // rotate
				{
				Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJROTATE;
				break;
				} // 1
			case 2: // ObjPos
				{
				Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJPOSITION;
				break;
				} // 2
			case 3: // VertPos
				{
				Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_VERTPOSITION;
				break;
				} // 3
			default: // scale
				{
				Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE;
				break;
				} // 0
			} // switch
		ConfigureDetail();
		DisableWidgets();
		break;
		} // TAB3, Randomize Scale/Rotate/ObjPos/VertPos subtab on Randomize tab
	default:
		break;
	} // switch


return(0);

} // Object3DEditGUI::HandlePageChange

/*===========================================================================*/

long Object3DEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
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
	case IDC_CHECKCASTSHADOWS:
	case IDC_CHECKRECEIVESHADOWSTER:
	case IDC_CHECKRECEIVESHADOWSFOL:
	case IDC_CHECKRECEIVESHADOWS3D:
	case IDC_CHECKRECEIVESHADOWSCSM:
	case IDC_CHECKRECEIVESHADOWSVOL:
	case IDC_CHECKUSEMAPFILE:
	case IDC_CHECKREGENMAPFILE:
	case IDC_CHECKSHADOWSONLY:
	case IDC_CHECKALIGN:
	case IDC_CHECKVERTICALALIGN:
	case IDC_CHECKREVERSE:
	case IDC_CHECKGEOGINSTANCE:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKOBJSCALE:
	case IDC_CHECKISOMETRIC:
		{
		if (Active->RandomizeObj[0])
			{
			Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE;
			ConfigureDetail();
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKOBJROTATION:
		{
		if (Active->RandomizeObj[1])
			{
			Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJROTATE;
			ConfigureDetail();
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKOBJPOSITION:
		{
		if (Active->RandomizeObj[2])
			{
			Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJPOSITION;
			ConfigureDetail();
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_CHECKVERTPOSITION:
		{
		if (Active->RandomizeVert)
			{
			Active->ShowDetail = WCS_EFFECTS_OBJECT3D_SHOWDETAIL_VERTPOSITION;
			ConfigureDetail();
			} // if
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // Object3DEditGUI::HandleSCChange

/*===========================================================================*/

long Object3DEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
NotifyTag Changes[2];

Changes[1] = 0;

switch (CtrlID)
	{
	case IDC_RADIOPREVIEWOFF:
	case IDC_RADIOPREVIEWCUBE:
	case IDC_RADIOPREVIEWDETAIL:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), WCS_EFFECTS_OBJECT3D_NUMANIMPAR, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	case IDC_RADIOFRAGPERPIXEL:
	case IDC_RADIOFRAGPERPASS:
	case IDC_RADIOFRAGPEROBJECT:
	case IDC_RADIOAAVERYHIGH:
	case IDC_RADIOAAHIGH:
	case IDC_RADIOAAMED:
	case IDC_RADIOAALOW:
	case IDC_RADIOABSOLUTE:
	case IDC_RADIORELATIVEGRND:
	case IDC_RADIORELATIVEJOE:
	case IDC_RADIOSHADOWMAPVERYLARGE:
	case IDC_RADIOSHADOWMAPLARGE:
	case IDC_RADIOSHADOWMAPMED:
	case IDC_RADIOSHADOWMAPSMALL:
	case IDC_RADIOALIGNX:
	case IDC_RADIOALIGNY:
	case IDC_RADIOALIGNZ:
	case IDC_RADIOALIGNX2:
	case IDC_RADIOALIGNY2:
	case IDC_RADIOALIGNZ2:
	case IDC_RADIOALIGNVEC:
	case IDC_RADIOALIGNTERRAIN:
	case IDC_RADIOALIGNPLACEVEC:
	case IDC_RADIOALIGNSPECIALVEC:
		{
		Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
		GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // Object3DEditGUI::HandleSRChange

/*===========================================================================*/

long Object3DEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
double curScale, changeScale;
NotifyTag Changes[2];

switch (CtrlID)
	{
	case IDC_ALIGNMENTBIAS:
		{
		WidgetSetScrollPos(IDC_SCROLLBAR1, (short)(Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue * 100.0));
		break;
		} // IDC_ALIGNMENTBIAS
	case IDC_XSCALE:
		curScale = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
		if (Active->lockScales)
			{
			changeScale = curScale / lastScaleX;
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].SetCurValue( 
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue);
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].SetCurValue(
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue);
			lastScaleY = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
			lastScaleZ = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
			//ConfigureDimensions();
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		lastScaleX = curScale;
		break;
	case IDC_YSCALE:
		curScale = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
		if (Active->lockScales)
			{
			changeScale = curScale / lastScaleY;
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].SetCurValue(
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue);
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].SetCurValue(
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue);
			lastScaleX = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
			lastScaleZ = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
			//ConfigureDimensions();
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		lastScaleY = curScale;
		break;
	case IDC_ZSCALE:
		curScale = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue;
		if (Active->lockScales)
			{
			changeScale = curScale / lastScaleZ;
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].SetCurValue(
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue);
			Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].SetCurValue(
				changeScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue);
			lastScaleX = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue;
			lastScaleY = Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue;
			//ConfigureDimensions();
			Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
			} // if
		lastScaleZ = curScale;
		break;
	default:
		break;
	} // ID

return(0);

} // Object3DEditGUI::HandleFIChange

/*===========================================================================*/

void Object3DEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Interested[11];
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

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[3] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_VALUECHANGED);
Interested[4] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[5] = MAKE_ID(WCS_EFFECTSSUBCLASS_MATERIAL, WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[6] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED); 
Interested[7] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureColors(NULL);
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

Interested[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[1] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[2] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[3] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[4] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	FillVectorComboBox();
	Done = 1;
	} // if

#ifdef WCS_BUILD_VNS
// query drop
Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_SEARCHQUERY, 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	WidgetLWSync(IDC_VECLINKAGE);
	Done = 1;
	} // if query changed
#endif // WCS_BUILD_VNS

if (! Done)
	ConfigureWidgets();

} // Object3DEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void Object3DEditGUI::ConfigureWidgets(void)
{
char TextStr[256];

// query drop
WidgetLWConfig(IDC_VECLINKAGE, Active, DBHost, EffectsHost, WM_WCSW_LW_NEWQUERY_FLAG_VECTOR | WM_WCSW_LW_NEWQUERY_FLAG_ENABLED | WM_WCSW_LW_NEWQUERY_FLAG_LINE | WM_WCSW_LW_NEWQUERY_FLAG_POINT);

sprintf(TextStr, "3D Object Editor - %s", Active->GetName());
if (Active->GetRAHostRoot()->TemplateItem)
	strcat(TextStr, " (Templated)");
SetTitle(TextStr);
WidgetSetModified(IDC_NAME, FALSE);
WidgetSetText(IDC_NAME, Active->Name);

ConfigureSC(NativeWin, IDC_CHECKENABLED, &Active->Enabled, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRENDEROCCLUDED, &Active->RenderOccluded, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKSHADOWSONLY, &Active->ShadowsOnly, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKCASTSHADOWS, &Active->CastShadows, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSTER, &Active->ReceiveShadowsTerrain, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSFOL, &Active->ReceiveShadowsFoliage, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWS3D, &Active->ReceiveShadows3DObject, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSCSM, &Active->ReceiveShadowsCloudSM, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKRECEIVESHADOWSVOL, &Active->ReceiveShadowsVolumetric, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKUSEMAPFILE, &Active->UseMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREGENMAPFILE, &Active->RegenMapFile, SCFlag_Short, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKALIGN, &Active->AlignHeading, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTICALALIGN, &Active->AlignVertical, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKREVERSE, &Active->ReverseHeading, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKGEOGINSTANCE, &Active->GeographicInstance, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOBJSCALE, &Active->RandomizeObj[0], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOBJROTATION, &Active->RandomizeObj[1], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKOBJPOSITION, &Active->RandomizeObj[2], SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKVERTPOSITION, &Active->RandomizeVert, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CHECKISOMETRIC, &Active->Isometric, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOPREVIEWOFF, IDC_RADIOPREVIEWOFF, &Active->DrawEnabled, SRFlag_Char, WCS_EFFECTS_OBJECT3D_DRAW_NONE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPREVIEWOFF, IDC_RADIOPREVIEWCUBE, &Active->DrawEnabled, SRFlag_Char, WCS_EFFECTS_OBJECT3D_DRAW_CUBE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOPREVIEWOFF, IDC_RADIOPREVIEWDETAIL, &Active->DrawEnabled, SRFlag_Char, WCS_EFFECTS_OBJECT3D_DRAW_DETAIL, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOAAVERYHIGH, IDC_RADIOAAVERYHIGH, &Active->AAPasses, SRFlag_Char, 17, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAAVERYHIGH, IDC_RADIOAAHIGH, &Active->AAPasses, SRFlag_Char, 9, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAAVERYHIGH, IDC_RADIOAAMED, &Active->AAPasses, SRFlag_Char, 5, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOAAVERYHIGH, IDC_RADIOAALOW, &Active->AAPasses, SRFlag_Char, 1, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOFRAGPERPIXEL, IDC_RADIOFRAGPERPIXEL, &Active->FragmentOptimize, SRFlag_Char, WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPIXEL, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFRAGPERPIXEL, IDC_RADIOFRAGPERPASS, &Active->FragmentOptimize, SRFlag_Char, WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPASS, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOFRAGPERPIXEL, IDC_RADIOFRAGPEROBJECT, &Active->FragmentOptimize, SRFlag_Char, WCS_EFFECTS_OBJECT3D_OPTIMIZE_PEROBJECT, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPLARGE, &Active->ShadowMapWidth, SRFlag_Short, 1024, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPVERYLARGE, &Active->ShadowMapWidth, SRFlag_Short, 2048, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPSMALL, &Active->ShadowMapWidth, SRFlag_Short, 256, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOSHADOWMAPLARGE, IDC_RADIOSHADOWMAPMED, &Active->ShadowMapWidth, SRFlag_Short, 512, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIORELATIVEJOE, IDC_RADIORELATIVEJOE, &Active->Absolute, SRFlag_Short, 2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORELATIVEJOE, IDC_RADIOABSOLUTE, &Active->Absolute, SRFlag_Short, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIORELATIVEJOE, IDC_RADIORELATIVEGRND, &Active->Absolute, SRFlag_Short, 0, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNX, &Active->HeadingAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_X, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNY, &Active->HeadingAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_Y, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNX, IDC_RADIOALIGNZ, &Active->HeadingAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_Z, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOALIGNX2, IDC_RADIOALIGNX2, &Active->VerticalAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_X, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNX2, IDC_RADIOALIGNY2, &Active->VerticalAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_Y, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNX2, IDC_RADIOALIGNZ2, &Active->VerticalAxis, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGN_Z, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOALIGNVEC, IDC_RADIOALIGNVEC, &Active->AlignVertVec, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNVEC, IDC_RADIOALIGNTERRAIN, &Active->AlignVertVec, SRFlag_Char, WCS_EFFECTS_OBJECT3D_ALIGNVERT_TERRAIN, NULL, NULL);

ConfigureSR(NativeWin, IDC_RADIOALIGNPLACEVEC, IDC_RADIOALIGNPLACEVEC, &Active->AlignSpecialVec, SRFlag_Char, FALSE, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOALIGNPLACEVEC, IDC_RADIOALIGNSPECIALVEC, &Active->AlignSpecialVec, SRFlag_Char, TRUE, NULL, NULL);

ConfigureTB(NativeWin, IDC_CONSTRAIN, IDI_CONSTRASP, NULL);
WidgetSetCheck(IDC_CONSTRAIN, Active->lockScales);

WidgetSmartRAHConfig(IDC_LAT, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LAT], Active);
WidgetSmartRAHConfig(IDC_LON, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_LON], Active);
WidgetSmartRAHConfig(IDC_ELEV, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ELEV], Active);
WidgetSmartRAHConfig(IDC_XROT, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONX], Active);
WidgetSmartRAHConfig(IDC_YROT, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONY], Active);
WidgetSmartRAHConfig(IDC_ZROT, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ROTATIONZ], Active);
WidgetSmartRAHConfig(IDC_XSCALE, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX], Active);
WidgetSmartRAHConfig(IDC_YSCALE, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY], Active);
WidgetSmartRAHConfig(IDC_ZSCALE, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ], Active);
WidgetSmartRAHConfig(IDC_XTRANS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONX], Active);
WidgetSmartRAHConfig(IDC_YTRANS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONY], Active);
WidgetSmartRAHConfig(IDC_ZTRANS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_TRANSLATIONZ], Active);
WidgetSmartRAHConfig(IDC_SHADOWINTENS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWINTENS], Active);
WidgetSmartRAHConfig(IDC_SHADOWOFFSET, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SHADOWOFFSET], Active);
WidgetSmartRAHConfig(IDC_ALIGNMENTBIAS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS], Active);

WidgetSetScrollPos(IDC_SCROLLBAR1, (short)(Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_ALIGNVERTBIAS].CurValue * 100.0));

WidgetCBSetCurSel(IDC_UNITSDROP, Active->Units - WCS_USEFUL_UNIT_MILLIMETER);

// This editor has one remaining icon button specific to it
ConfigureTB(NativeWin, IDC_LOAD, IDI_FILEOPEN, NULL);

sprintf(TextStr, "Vertices: %d,  Polygons: %d,  Materials: %d", Active->NumVertices, Active->NumPolys, Active->NumMaterials);
WidgetSetText(IDC_NUMVERTS, TextStr);

TempRast.Thumb = NULL;
if (Active && Active->BrowseInfo)
	{
	TempRast.Thumb = Active->BrowseInfo->Thumb;
	ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, &TempRast);
	} // if
else
	{
	ConfigureTB(NativeWin, IDC_TNAIL1, NULL, NULL, NULL);
	} // else

BuildMaterialList();
BuildMaterialReplaceList();
SelectMaterial();
ConfigureDimensions();
ConfigureDetail();
FillVectorComboBox();
SetWidgetsVisible();
DisableWidgets();
// advanced
DisplayAdvancedFeatures();

} // Object3DEditGUI::ConfigureWidgets()

/*===========================================================================*/

void Object3DEditGUI::ConfigureDimensions(void)
{
double UnitScale;
char Str[24];

UnitScale = ConvertToMeters(1.0, Active->Units);

sprintf(Str, "%1.4f", Active->ObjectBounds[0] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZEHIX, Str);
sprintf(Str, "%1.4f", Active->ObjectBounds[1] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGX].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZELOX, Str);

sprintf(Str, "%1.4f", Active->ObjectBounds[2] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZEHIY, Str);
sprintf(Str, "%1.4f", Active->ObjectBounds[3] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGY].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZELOY, Str);

sprintf(Str, "%1.4f", Active->ObjectBounds[4] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZEHIZ, Str);
sprintf(Str, "%1.4f", Active->ObjectBounds[5] * UnitScale * Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_SCALINGZ].CurValue);
TrimZeros(Str);
WidgetSetText(IDC_SIZELOZ, Str);

} // Object3DEditGUI::ConfigureDimensions()

/*===========================================================================*/

void Object3DEditGUI::ConfigureDetail(void)
{

WidgetTCSetCurSel(IDC_TAB3, Active->ShowDetail);

switch (Active->ShowDetail)
	{
	case WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE:
		{
		WidgetSmartRAHConfig(IDC_XPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXPLUS], Active);
		WidgetSmartRAHConfig(IDC_YPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYPLUS], Active);
		WidgetSmartRAHConfig(IDC_ZPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZPLUS], Active);
		WidgetSmartRAHConfig(IDC_XMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEXMINUS], Active);
		WidgetSmartRAHConfig(IDC_YMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEYMINUS], Active);
		WidgetSmartRAHConfig(IDC_ZMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJSCALEZMINUS], Active);

		WidgetSmartRAHConfig(IDC_TEX1, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALE), Active);
		WidgetSmartRAHConfig(IDC_TEX2, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEX), Active);
		WidgetSmartRAHConfig(IDC_TEX3, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEY), Active);
		WidgetSmartRAHConfig(IDC_TEX4, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJSCALEZ), Active);

		WidgetSetText(IDC_RANGETXT, "Scale Range (%)");
		WidgetShow(IDC_RANGETXT2, FALSE);
		break;
		} // WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE
	case WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJROTATE:
		{
		WidgetSmartRAHConfig(IDC_XPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXPLUS], Active);
		WidgetSmartRAHConfig(IDC_YPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYPLUS], Active);
		WidgetSmartRAHConfig(IDC_ZPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZPLUS], Active);
		WidgetSmartRAHConfig(IDC_XMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEXMINUS], Active);
		WidgetSmartRAHConfig(IDC_YMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEYMINUS], Active);
		WidgetSmartRAHConfig(IDC_ZMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJROTATEZMINUS], Active);

		WidgetSmartRAHConfig(IDC_TEX1, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATE), Active);
		WidgetSmartRAHConfig(IDC_TEX2, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEX), Active);
		WidgetSmartRAHConfig(IDC_TEX3, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEY), Active);
		WidgetSmartRAHConfig(IDC_TEX4, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJROTATEZ), Active);

		WidgetSetText(IDC_RANGETXT, "Rotation Range (deg)");
		WidgetShow(IDC_RANGETXT2, FALSE);
		break;
		} // WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJROTATE
	case WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJPOSITION:
		{
		WidgetSmartRAHConfig(IDC_XPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXPLUS], Active);
		WidgetSmartRAHConfig(IDC_YPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYPLUS], Active);
		WidgetSmartRAHConfig(IDC_ZPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZPLUS], Active);
		WidgetSmartRAHConfig(IDC_XMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONXMINUS], Active);
		WidgetSmartRAHConfig(IDC_YMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONYMINUS], Active);
		WidgetSmartRAHConfig(IDC_ZMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMOBJPOSITIONZMINUS], Active);

		WidgetSmartRAHConfig(IDC_TEX1, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITION), Active);
		WidgetSmartRAHConfig(IDC_TEX2, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONX), Active);
		WidgetSmartRAHConfig(IDC_TEX3, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONY), Active);
		WidgetSmartRAHConfig(IDC_TEX4, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMOBJPOSITIONZ), Active);

		WidgetSetText(IDC_RANGETXT, "Object Position Range");
		WidgetShow(IDC_RANGETXT2, FALSE);
		break;
		} // WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJPOSITION
	case WCS_EFFECTS_OBJECT3D_SHOWDETAIL_VERTPOSITION:
		{
		WidgetSmartRAHConfig(IDC_XPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXPLUS], Active);
		WidgetSmartRAHConfig(IDC_YPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYPLUS], Active);
		WidgetSmartRAHConfig(IDC_ZPLUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZPLUS], Active);
		WidgetSmartRAHConfig(IDC_XMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONXMINUS], Active);
		WidgetSmartRAHConfig(IDC_YMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONYMINUS], Active);
		WidgetSmartRAHConfig(IDC_ZMINUS, &Active->AnimPar[WCS_EFFECTS_OBJECT3D_ANIMPAR_RANDOMVERTPOSITIONZMINUS], Active);

		WidgetSmartRAHConfig(IDC_TEX1, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITION), Active);
		WidgetSmartRAHConfig(IDC_TEX2, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONX), Active);
		WidgetSmartRAHConfig(IDC_TEX3, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONY), Active);
		WidgetSmartRAHConfig(IDC_TEX4, (RasterAnimHost **)Active->GetTexRootPtrAddr(WCS_EFFECTS_OBJECT3D_TEXTURE_RANDOMVERTPOSITIONZ), Active);

		WidgetSetText(IDC_RANGETXT, "Vertex Position Range");
		WidgetShow(IDC_RANGETXT2, TRUE);
		break;
		} // WCS_EFFECTS_OBJECT3D_SHOWDETAIL_VERTPOSITION
	default:
		break;
	} // switch

} // Object3DEditGUI::ConfigureDetail

/*===========================================================================*/

void Object3DEditGUI::ConfigureColors(MaterialEffect *Mat)
{
long Current;
char MaterialName[WCS_EFFECT_MAXNAMELENGTH + 10];


if (! Mat)
	{
	MaterialName[0] = 0;
	if (Active->NameTable)
		{
		if ((Current = WidgetLBGetCurSel(IDC_MATERIALLIST)) != LB_ERR)
			{
			if (WidgetLBGetText(IDC_MATERIALLIST, Current, MaterialName) > 0)
				{
				Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, MaterialName);
				} // if
			} // if
		} // if
	} // if
if (Mat)
	{
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, &Mat->DiffuseColor, Mat);
	} // if
else
	{
	WidgetSmartRAHConfig(IDC_DIFFUSECOLOR, (RasterAnimHost *)NULL, NULL);
	} // else

} // Object3DEditGUI::ConfigureColors()

/*===========================================================================*/

void Object3DEditGUI::FillVectorComboBox(void)
{
Joe *CurJoe;
long Found = -1, Pos;

DBHost->ResetGeoClip();
WidgetCBClear(IDC_VECTORDROP);

for (CurJoe = DBHost->GetFirst(); CurJoe; CurJoe = DBHost->GetNext(CurJoe))
	{
	if (! CurJoe->TestFlags(WCS_JOEFLAG_ISDEM) && ! CurJoe->TestFlags(WCS_JOEFLAG_HASKIDS))
		{
		Pos = WidgetCBInsert(IDC_VECTORDROP, -1, CurJoe->GetBestName());
		WidgetCBSetItemData(IDC_VECTORDROP, Pos, CurJoe);
		if (CurJoe == Active->AlignVec)
			Found = Pos;
		} // if
	} // for

WidgetCBSetCurSel(IDC_VECTORDROP, Found);

} // Object3DEditGUI::FillVectorComboBox

/*===========================================================================*/

void Object3DEditGUI::SyncWidgets(void)
{

WidgetSCSync(IDC_CHECKENABLED, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKCASTSHADOWS, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSTER, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSFOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWS3D, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSCSM, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKRECEIVESHADOWSVOL, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKUSEMAPFILE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREGENMAPFILE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKSHADOWSONLY, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKALIGN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVERTICALALIGN, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKREVERSE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKGEOGINSTANCE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOBJSCALE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOBJROTATION, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKOBJPOSITION, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKVERTPOSITION, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_CHECKISOMETRIC, WP_SCSYNC_NONOTIFY);

WidgetSRSync(IDC_RADIOPREVIEWOFF, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPREVIEWCUBE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOPREVIEWDETAIL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAAVERYHIGH, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAAHIGH, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAAMED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOAALOW, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFRAGPERPIXEL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFRAGPERPASS, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOFRAGPEROBJECT, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOABSOLUTE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEGRND, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIORELATIVEJOE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPVERYLARGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPLARGE, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPMED, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOSHADOWMAPSMALL, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNX, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNY, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNZ, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNX2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNY2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNZ2, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNVEC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNTERRAIN, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNPLACEVEC, WP_SRSYNC_NONOTIFY);
WidgetSRSync(IDC_RADIOALIGNSPECIALVEC, WP_SRSYNC_NONOTIFY);

WidgetSNSync(IDC_LAT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_LON, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ELEV, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_XROT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_YROT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZROT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_XSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_YSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZSCALE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_XTRANS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_YTRANS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZTRANS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SHADOWINTENS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_SHADOWOFFSET, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ALIGNMENTBIAS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_XPLUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_YPLUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZPLUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_XMINUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_YMINUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_ZMINUS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEX1, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEX2, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEX3, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_TEX4, WP_FISYNC_NONOTIFY);

WidgetCBSetCurSel(IDC_UNITSDROP, Active->Units - WCS_USEFUL_UNIT_MILLIMETER);
ConfigureDimensions();

} // Object3DEditGUI::SyncWidgets()

/*===========================================================================*/

void Object3DEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_LAT, ! (Active->GeographicInstance));
WidgetSetDisabled(IDC_LON, ! (Active->GeographicInstance));
WidgetSetDisabled(IDC_CHECKREGENMAPFILE, ! (Active->UseMapFile && Active->CastShadows));
WidgetSetDisabled(IDC_SHADOWINTENS, ! (Active->ReceiveShadowsTerrain || Active->ReceiveShadowsFoliage || 
	Active->ReceiveShadows3DObject || Active->ReceiveShadowsCloudSM || Active->ReceiveShadowsVolumetric));
WidgetSetDisabled(IDC_CHECKUSEMAPFILE, ! Active->CastShadows);
WidgetSetDisabled(IDC_SHADOWOFFSET, ! (Active->ReceiveShadowsTerrain || Active->ReceiveShadowsFoliage || 
	Active->ReceiveShadows3DObject || Active->ReceiveShadowsCloudSM));
WidgetSetDisabled(IDC_RADIOSHADOWMAPVERYLARGE, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPLARGE, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPMED, ! Active->CastShadows);
WidgetSetDisabled(IDC_RADIOSHADOWMAPSMALL, ! Active->CastShadows);
WidgetSetDisabled(IDC_YSCALE, ! Active->CalcNormals);
WidgetSetDisabled(IDC_ZSCALE, ! Active->CalcNormals);
WidgetSetDisabled(IDC_CHECKISOMETRIC, ! Active->CalcNormals || ! Active->RandomizeObj[0]);
if (Active->ShowDetail == WCS_EFFECTS_OBJECT3D_SHOWDETAIL_OBJSCALE)
	{
	WidgetSetDisabled(IDC_YPLUS, Active->Isometric);
	WidgetSetDisabled(IDC_ZPLUS, Active->Isometric);
	WidgetSetDisabled(IDC_YMINUS, Active->Isometric);
	WidgetSetDisabled(IDC_ZMINUS, Active->Isometric);
	WidgetSetDisabled(IDC_TEX3, Active->Isometric);
	WidgetSetDisabled(IDC_TEX4, Active->Isometric);
	} // if
else
	{
	WidgetSetDisabled(IDC_YPLUS, FALSE);
	WidgetSetDisabled(IDC_ZPLUS, FALSE);
	WidgetSetDisabled(IDC_YMINUS, FALSE);
	WidgetSetDisabled(IDC_ZMINUS, FALSE);
	WidgetSetDisabled(IDC_TEX3, FALSE);
	WidgetSetDisabled(IDC_TEX4, FALSE);
	} // else
WidgetSetDisabled(IDC_RADIOALIGNX, Active->VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X);
WidgetSetDisabled(IDC_RADIOALIGNY, Active->VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y);
WidgetSetDisabled(IDC_RADIOALIGNZ, Active->VerticalAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z);
WidgetSetDisabled(IDC_RADIOALIGNX2, Active->HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_X);
WidgetSetDisabled(IDC_RADIOALIGNY2, Active->HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Y);
WidgetSetDisabled(IDC_RADIOALIGNZ2, Active->HeadingAxis == WCS_EFFECTS_OBJECT3D_ALIGN_Z);
WidgetSetDisabled(IDC_CHECKREVERSE, ! Active->AlignHeading);
WidgetSetDisabled(IDC_RADIOALIGNVEC, ! Active->AlignVertical);
WidgetSetDisabled(IDC_RADIOALIGNTERRAIN, ! Active->AlignVertical);
WidgetSetDisabled(IDC_SCROLLBAR1, ! Active->AlignVertical);
WidgetSetDisabled(IDC_ALIGNMENTBIAS, ! Active->AlignVertical);

WidgetSetDisabled(IDC_VECTORDROP, ! Active->AlignSpecialVec);

if (Active->AlignVertVec == WCS_EFFECTS_OBJECT3D_ALIGNVERT_VECTOR)
	WidgetSetText(IDC_ALIGNTEXT, "Vector Normal");
else
	WidgetSetText(IDC_ALIGNTEXT, "Terrain Normal");

} // Object3DEditGUI::DisableWidgets()

/*===========================================================================*/

void Object3DEditGUI::SetWidgetsVisible(void)
{

WidgetShow(IDC_XSCALE, ActiveParam == 0);
WidgetShow(IDC_YSCALE, ActiveParam == 0);
WidgetShow(IDC_ZSCALE, ActiveParam == 0);
WidgetShow(IDC_XROT, ActiveParam == 1);
WidgetShow(IDC_YROT, ActiveParam == 1);
WidgetShow(IDC_ZROT, ActiveParam == 1);
WidgetShow(IDC_XTRANS, ActiveParam == 2);
WidgetShow(IDC_YTRANS, ActiveParam == 2);
WidgetShow(IDC_ZTRANS, ActiveParam == 2);

WidgetTCSetCurSel(IDC_TAB2, ActiveParam);

} // Object3DEditGUI::SetWidgetsVisible()

/*===========================================================================*/

void Object3DEditGUI::BuildMaterialList(void)
{
long Ct, PrevSelected;

PrevSelected = WidgetLBGetCurSel(IDC_MATERIALLIST);

WidgetLBClear(IDC_MATERIALLIST);

if (Active->NameTable)
	{
	for (Ct = 0; Ct < Active->NumMaterials; Ct ++)
		{
		if (Active->NameTable[Ct].Name[0])
			{
			EffectsHost->SetMakeMaterial(&Active->NameTable[Ct], NULL);
			WidgetLBInsert(IDC_MATERIALLIST, -1, Active->NameTable[Ct].Name);
			} // if
		else
			{
			WidgetLBInsert(IDC_MATERIALLIST, -1, "Unnamed Material");
			} // else
		} // for
	WidgetLBSetCurSel(IDC_MATERIALLIST, (PrevSelected >= 0 && PrevSelected < Active->NumMaterials) ? PrevSelected: 0);
	} // if

} // Object3DEditGUI::BuildMaterialList

/*===========================================================================*/

void Object3DEditGUI::BuildMaterialReplaceList(void)
{
MaterialEffect *Mat;
long PrevSelected, Entries;

PrevSelected = WidgetCBGetCurSel(IDC_MATERIALDROP);

WidgetCBClear(IDC_MATERIALDROP);

if (Mat = (MaterialEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_MATERIAL))
	{
	for (Entries = 0; Mat; Mat = (MaterialEffect *)Mat->Next, Entries ++)
		{
		WidgetCBInsert(IDC_MATERIALDROP, -1, Mat->Name);
		} // for
	WidgetCBSetCurSel(IDC_MATERIALDROP, (PrevSelected >= 0 && PrevSelected < Entries) ? PrevSelected: 0);
	} // if

} // Object3DEditGUI::BuildMaterialList

/*===========================================================================*/

// advanced
void Object3DEditGUI::DisplayAdvancedFeatures(void)
{

bool CompositeDisplayAdvanced = QueryDisplayAdvancedUIVisibleState();

if (CompositeDisplayAdvanced)
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, false);
	WidgetShow(IDC_HIDDENCONTROLMSG2, false);
	WidgetShow(IDC_HIDDENCONTROLMSG3, false);
	WidgetShow(IDC_HIDDENCONTROLMSG4, false);
	WidgetShow(IDC_RENDER_BOX, true);
	WidgetShow(IDC_FRAGMENT_BOX, true);
	WidgetShow(IDC_RADIOAAVERYHIGH, true);
	WidgetShow(IDC_RADIOAAHIGH, true);
	WidgetShow(IDC_RADIOAAMED, true);
	WidgetShow(IDC_RADIOAALOW, true);
	WidgetShow(IDC_RADIOFRAGPERPIXEL, true);
	WidgetShow(IDC_RADIOFRAGPERPASS, true);
	WidgetShow(IDC_RADIOFRAGPEROBJECT, true);
	WidgetShow(IDC_CHECKRENDEROCCLUDED, true);
	WidgetShow(IDC_HEADING_BOX, true);
	WidgetShow(IDC_VERTICAL_BOX, true);
	WidgetShow(IDC_VECTOR_BOX, true);
	WidgetShow(IDC_CHECKALIGN, true);
	WidgetShow(IDC_CHECKREVERSE, true);
	WidgetShow(IDC_RADIOALIGNX, true);
	WidgetShow(IDC_RADIOALIGNY, true);
	WidgetShow(IDC_RADIOALIGNZ, true);
	WidgetShow(IDC_CHECKVERTICALALIGN, true);
	WidgetShow(IDC_VERTICAL_TXT, true);
	WidgetShow(IDC_ALIGNTEXT, true);
	WidgetShow(IDC_RADIOALIGNVEC, true);
	WidgetShow(IDC_RADIOALIGNTERRAIN, true);
	WidgetShow(IDC_SCROLLBAR1, true);
	WidgetShow(IDC_ALIGNMENTBIAS, true);
	WidgetShow(IDC_RADIOALIGNX2, true);
	WidgetShow(IDC_RADIOALIGNY2, true);
	WidgetShow(IDC_RADIOALIGNZ2, true);
	WidgetShow(IDC_RADIOALIGNPLACEVEC, true);
	WidgetShow(IDC_RADIOALIGNSPECIALVEC, true);
	WidgetShow(IDC_VECTORDROP, true);
	WidgetShow(IDC_FREEVECTOR, true);
	WidgetShow(IDC_RANDOMIZE_BOX, true);
	WidgetShow(IDC_DETAILBOX, true);
	WidgetShow(IDC_CHECKOBJSCALE, true);
	WidgetShow(IDC_CHECKISOMETRIC, true);
	WidgetShow(IDC_CHECKOBJROTATION, true);
	WidgetShow(IDC_CHECKOBJPOSITION, true);
	WidgetShow(IDC_CHECKVERTPOSITION, true);
	WidgetShow(IDC_DEFORMATION_TXT, true);
	WidgetShow(IDC_RANGETXT, true);
	WidgetShow(IDC_RANGETXT2, true);
	WidgetShow(IDC_TAB3, true);
	WidgetShow(IDC_XPLUS, true);
	WidgetShow(IDC_XMINUS, true);
	WidgetShow(IDC_YPLUS, true);
	WidgetShow(IDC_YMINUS, true);
	WidgetShow(IDC_ZPLUS, true);
	WidgetShow(IDC_ZMINUS, true);
	WidgetShow(IDC_TEX1, true);
	WidgetShow(IDC_TEX2, true);
	WidgetShow(IDC_TEX3, true);
	WidgetShow(IDC_TEX4, true);
	WidgetShow(IDC_OVERALL_TXT, true);
	WidgetShow(IDC_LOW_TXT, true);
	WidgetShow(IDC_HIGH_TXT, true);
	} // if
else
	{
	WidgetShow(IDC_HIDDENCONTROLMSG1, true);
	WidgetShow(IDC_HIDDENCONTROLMSG2, true);
	WidgetShow(IDC_HIDDENCONTROLMSG3, true);
	WidgetShow(IDC_HIDDENCONTROLMSG4, true);
	WidgetShow(IDC_RENDER_BOX, false);
	WidgetShow(IDC_FRAGMENT_BOX, false);
	WidgetShow(IDC_RADIOAAVERYHIGH, false);
	WidgetShow(IDC_RADIOAAHIGH, false);
	WidgetShow(IDC_RADIOAAMED, false);
	WidgetShow(IDC_RADIOAALOW, false);
	WidgetShow(IDC_RADIOFRAGPERPIXEL, false);
	WidgetShow(IDC_RADIOFRAGPERPASS, false);
	WidgetShow(IDC_RADIOFRAGPEROBJECT, false);
	WidgetShow(IDC_CHECKRENDEROCCLUDED, false);
	WidgetShow(IDC_HEADING_BOX, false);
	WidgetShow(IDC_VERTICAL_BOX, false);
	WidgetShow(IDC_VECTOR_BOX, false);
	WidgetShow(IDC_CHECKALIGN, false);
	WidgetShow(IDC_CHECKREVERSE, false);
	WidgetShow(IDC_RADIOALIGNX, false);
	WidgetShow(IDC_RADIOALIGNY, false);
	WidgetShow(IDC_RADIOALIGNZ, false);
	WidgetShow(IDC_CHECKVERTICALALIGN, false);
	WidgetShow(IDC_VERTICAL_TXT, false);
	WidgetShow(IDC_ALIGNTEXT, false);
	WidgetShow(IDC_RADIOALIGNVEC, false);
	WidgetShow(IDC_RADIOALIGNTERRAIN, false);
	WidgetShow(IDC_SCROLLBAR1, false);
	WidgetShow(IDC_ALIGNMENTBIAS, false);
	WidgetShow(IDC_RADIOALIGNX2, false);
	WidgetShow(IDC_RADIOALIGNY2, false);
	WidgetShow(IDC_RADIOALIGNZ2, false);
	WidgetShow(IDC_RADIOALIGNPLACEVEC, false);
	WidgetShow(IDC_RADIOALIGNSPECIALVEC, false);
	WidgetShow(IDC_VECTORDROP, false);
	WidgetShow(IDC_FREEVECTOR, false);
	WidgetShow(IDC_RANDOMIZE_BOX, false);
	WidgetShow(IDC_DETAILBOX, false);
	WidgetShow(IDC_CHECKOBJSCALE, false);
	WidgetShow(IDC_CHECKISOMETRIC, false);
	WidgetShow(IDC_CHECKOBJROTATION, false);
	WidgetShow(IDC_CHECKOBJPOSITION, false);
	WidgetShow(IDC_CHECKVERTPOSITION, false);
	WidgetShow(IDC_DEFORMATION_TXT, false);
	WidgetShow(IDC_RANGETXT, false);
	WidgetShow(IDC_RANGETXT2, false);
	WidgetShow(IDC_TAB3, false);
	WidgetShow(IDC_XPLUS, false);
	WidgetShow(IDC_XMINUS, false);
	WidgetShow(IDC_YPLUS, false);
	WidgetShow(IDC_YMINUS, false);
	WidgetShow(IDC_ZPLUS, false);
	WidgetShow(IDC_ZMINUS, false);
	WidgetShow(IDC_TEX1, false);
	WidgetShow(IDC_TEX2, false);
	WidgetShow(IDC_TEX3, false);
	WidgetShow(IDC_TEX4, false);
	WidgetShow(IDC_OVERALL_TXT, false);
	WidgetShow(IDC_LOW_TXT, false);
	WidgetShow(IDC_HIGH_TXT, false);
	} // else

SetDisplayAdvancedUIVisibleStateFlag(DisplayAdvanced ? true: false);

} // Object3DEditGUI::DisplayAdvancedFeatures

/*===========================================================================*/

void Object3DEditGUI::Cancel(void)
{
NotifyTag Changes[2];

Active->Copy(Active, &Backup);

Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // Object3DEditGUI::Cancel

/*===========================================================================*/

void Object3DEditGUI::Name(void)
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

} // Object3DEditGUI::Name()

/*===========================================================================*/

void Object3DEditGUI::EditMaterial(void)
{
MaterialEffect *Mat;
long Current;
char MaterialName[WCS_EFFECT_MAXNAMELENGTH + 10];

MaterialName[0] = 0;
if ((Current = WidgetLBGetCurSel(IDC_MATERIALLIST)) != LB_ERR)
	{
	if (WidgetLBGetText(IDC_MATERIALLIST, Current, MaterialName) > 0)
		{
		if (Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, MaterialName))
			{
			//MaterialEditGUI::ActivePage = 1;
			Mat->Edit();
			} // if
		} // if
	} // if

} // Object3DEditGUI::EditMaterial

/*===========================================================================*/

void Object3DEditGUI::EditMaterialColor(void)
{
MaterialEffect *Mat;
long Current;
char MaterialName[WCS_EFFECT_MAXNAMELENGTH + 10];

MaterialName[0] = 0;
if (Active->NameTable)
	{
	if ((Current = WidgetLBGetCurSel(IDC_MATERIALLIST)) != LB_ERR)
		{
		if (WidgetLBGetText(IDC_MATERIALLIST, Current, MaterialName) > 0)
			{
			if (Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, MaterialName))
				{
				Mat->DiffuseColor.EditRAHost();
				} // if
			} // if
		} // if
	} // if

} // Object3DEditGUI::EditMaterialColor

/*===========================================================================*/

void Object3DEditGUI::ReplaceMaterial(void)
{
long Current, Replace;
NotifyTag Changes[2];
char MaterialName[WCS_EFFECT_MAXNAMELENGTH + 10];

MaterialName[0] = 0;
if (Active->NameTable)
	{
	if ((Replace = WidgetLBGetCurSel(IDC_MATERIALLIST)) != LB_ERR)
		{
		if ((Current = WidgetCBGetCurSel(IDC_MATERIALDROP)) != CB_ERR)
			{
			if (WidgetCBGetText(IDC_MATERIALDROP, Current, MaterialName) > 0)
				{
				Active->NameTable[Replace].SetName(MaterialName);
				Active->NameTable[Replace].Mat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, MaterialName);
				WidgetLBReplace(IDC_MATERIALLIST, Replace, Active->NameTable[Replace].Name);
				WidgetLBSetCurSel(IDC_MATERIALLIST, Replace);
				SelectMaterial();
				Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
				Changes[1] = NULL;
				GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());
				} // if
			} // if
		} // if
	} // if

} // Object3DEditGUI::ReplaceMaterial

/*===========================================================================*/

void Object3DEditGUI::SelectMaterial(void)
{
MaterialEffect *MyMat;
long Current;
char MaterialName[WCS_EFFECT_MAXNAMELENGTH + 10];

MaterialName[0] = 0;
if (Active->NameTable)
	{
	if ((Current = WidgetLBGetCurSel(IDC_MATERIALLIST)) != LB_ERR)
		{
		if (WidgetLBGetText(IDC_MATERIALLIST, Current, MaterialName) > 0)
			{
			MyMat = (MaterialEffect *)EffectsHost->FindByName(WCS_EFFECTSSUBCLASS_MATERIAL, MaterialName);
			ConfigureColors(MyMat);
			} // if
		} // if
	} // if

} // Object3DEditGUI::SelectMaterial

/*===========================================================================*/

void Object3DEditGUI::NewUnits(void)
{
NotifyTag Changes[2];

Active->Units = (char)(WidgetCBGetCurSel(IDC_UNITSDROP) + WCS_USEFUL_UNIT_MILLIMETER);
Changes[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_VALUECHANGED);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, Active->GetRAHostRoot());

} // Object3DEditGUI::NewUnits

/*===========================================================================*/

void Object3DEditGUI::NewAlignVec(void)
{
Joe *NewJoe;
long Current;

if ((Current = WidgetCBGetCurSel(IDC_VECTORDROP)) != CB_ERR)
	{
	if ((NewJoe = (Joe *)WidgetCBGetItemData(IDC_VECTORDROP, Current)) != (Joe *)CB_ERR && NewJoe)
		{
		if (! Active->SetAlignVec(NewJoe))
			FillVectorComboBox();
		} // if
	} // if

} // Object3DEditGUI::NewAlignVec

/*===========================================================================*/

void Object3DEditGUI::NewObject(void)
{

Active->OpenInputFileRequest();

} // Object3DEditGUI::NewObject

/*===========================================================================*/

bool Object3DEditGUI::QueryLocalDisplayAdvancedUIVisibleState(void)
{

return(DisplayAdvanced || Active->AAPasses != 1 || Active->FragmentOptimize != WCS_EFFECTS_OBJECT3D_OPTIMIZE_PERPASS ||
	Active->RenderOccluded || Active->AlignHeading || Active->AlignVertical || 
	Active->RandomizeObj[0] || Active->RandomizeObj[1] || Active->RandomizeObj[2] || Active->RandomizeVert ? true : false);

} // Object3DEditGUI::QueryLocalDisplayAdvancedUIVisibleState

