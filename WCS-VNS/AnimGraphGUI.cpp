// AnimGraphGUI.cpp
// Code for AnimGraph editor
// Built from AnimGraphGUI.cpp on 5/9/98 by Gary R Huber
// Copyright 1998 by Questar Productions. All rights reserved.

#include "stdafx.h"
#include "AnimGraphGUI.h"
#include "WCSWidgets.h"
#include "Toolbar.h"
#include "Conservatory.h"
#include "Interactive.h"
#include "Useful.h"
#include "GraphData.h"
#include "Raster.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "EffectsLib.h"
#include "resource.h"

#undef DeletePalette

extern WCSApp *GlobalApp;

char *BSText[] = {
//WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE - edge feathering profile
"This graph varies the Component's effect within its controlling vector.\
 The left edge is at the vector. The horizontal axis shows distance inward from\
 the Vector. The vertical axis shows effect amount.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION - tfx cross section
"This graph shows the change of elevation over distance from the center of a Terraffector.\
 Distance from the centerline vector is on the horizontal axis.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE - wave envelope
"This graph shows the strength of an amplitude variable over distance.\
 Distance from the outer spreading edge of the envelope is on the horizontal axis.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE - don't know if this is used - GH
"This graph shows the change of a variable over distance. Distance is on the horizontal axis.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF - cloud vertical profiles
"This graph shows the strength of a variable over the vertical range.\
 Distance from the base elevation is on the horizontal axis. A distance of 0 is the bottom\
 and 1 is the top.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE - foliage forestry conversion graphs
"This graph shows the change of a dependent variable (vertical axis) versus an independent variable (horizontal axis). The axes are labelled.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE - custom curve texture
"This graph determines the remapping from one value to another. Input values are on the horizontal axis and output is on the vertical. Both axes are in percentage no matter how they are labelled.",
//WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME - timeline
//WCS_RAHOST_OBJTYPE_ANIMCOLORTIME - color timeline
"This graph shows the change of a variable over time. Time is on the horizontal axis."
};

NativeGUIWin AnimGraphGUI::Open(Project *Proj)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Proj))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // AnimGraphGUI::Open

/*===========================================================================*/

NativeGUIWin AnimGraphGUI::Construct(void)
{
int TabIndex;
GeneralEffect *MyEffect;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_GR_MAINWIN, LocalWinSys()->RootWin);
	CreateSubWinFromTemplate(IDD_GR_TOPCONTROLS, 0, 0, false);
	CreateSubWinFromTemplate(IDD_GR_BOTCONTROLS, 1, 0, false);
	CreateSubWinFromTemplate(IDD_GR_SIDECONTROLS, 2, 0, false);

	TopControls = SubPanels[0][0];
	BotControls = SubPanels[1][0];
	SideControls = SubPanels[2][0];

	#ifdef _WIN32
	WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 100);
	WidgetSetScrollRange(IDC_SCROLLBAR2, 1, 100);

	WidgetSetScrollRange(IDC_SCROLLBAR4, 0, 100);
	WidgetSetScrollRange(IDC_SCROLLBAR5, 0, 99);


	#endif // _WIN32

	if(NativeWin && TopControls && BotControls && SideControls)
		{
		#ifdef _WIN32
		GraphWidget = GetDlgItem(NativeWin, IDC_GRAPH);

		ShowPanel(0, 0);
		ShowPanel(1, 0);
		ShowPanel(2, 0);

		WidgetCBInsert(IDC_ECODROP, -1, "New Ecosystem...");
		for (MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); MyEffect; MyEffect = MyEffect->Next)
			{
			TabIndex = WidgetCBInsert(IDC_ECODROP, -1, MyEffect->GetName());
			WidgetCBSetItemData(IDC_ECODROP, TabIndex, MyEffect);
			} // for

		ConfigureWidgets();	// paths

		#endif // _WIN32

		} // if

	} // if

return(NativeWin);
} // AnimGraphGUI::Construct

/*===========================================================================*/

AnimGraphGUI::AnimGraphGUI(AnimCritter *AnimSource, unsigned char IDSource, NotifyEx *NotifyExSource)
: GUIFenetre('ANGR', this, "Animation Graph")
{
double MaxDist, MinDist, ValRange;
static NotifyTag AllEvents[] = {0,
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED),
								MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllProjPrefsEvents[] = {MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff),
								0};

ConstructError = 0;
TopControls = BotControls = SideControls = NULL;
IgnoreNotify = 0;

HostCritter = AnimSource;
MyGraphID = IDSource;
NotifyExHost = NotifyExSource;
RestoreBackup = UndoBackup = NULL;

if (AnimSource)
	{
	memset(&WidgetLocal, 0, sizeof WidgetLocal);
	WidgetLocal.GrWin = this;
	WidgetLocal.Crit = AnimSource;
	WidgetLocal.ValuePrototype = &ValueADT;	// setting these to NULL will mean no value or distance labels in graph
	WidgetLocal.DistancePrototype = &DistanceADT;
	WidgetLocal.ActiveNode = AnimSource->GetFirstSelectedNode(0);
	WidgetLocal.drawgrid = 1;
	WidgetLocal.NumGraphs = (char)AnimSource->GetNumGraphs();
	WidgetLocal.HPan = 0;
	WidgetLocal.HVisible = 100;
	WidgetLocal.VPan = 0;
	WidgetLocal.VVisible = 100;
	WidgetLocal.DistGridLg = 5;
	WidgetLocal.SnapToInt = 0;
	WidgetLocal.DisplayByFrame = HostCritter->TimeDude() && (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES);
	WidgetLocal.MaxDistRange = (HostCritter->TimeDude() ? GlobalApp->MCP->GetMaxFrame() / GlobalApp->MainProj->Interactive->GetFrameRate(): (HostCritter->GetMinMaxDist(MinDist, MaxDist) ? (MaxDist > 1.0 ? MaxDist: 1.0): 1.0));
	if (WidgetLocal.MaxDistRange < 1.0)
		WidgetLocal.MaxDistRange = 1.0;
	WidgetLocal.LowDrawDist = 0.0;
	WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;
	HostCritter->GetMinMaxVal(WidgetLocal.MinLowDrawVal, WidgetLocal.MaxHighDrawVal);
	if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
		{
		ValRange = HostCritter->GetIncrement();
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // if
	else
		{
		ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .1;
		WidgetLocal.MaxHighDrawVal += ValRange;
		WidgetLocal.MinLowDrawVal -= ValRange;
		} // else

	Distance = 0.0;
	Value = 0.0;
	WidgetLocal.FrameRate = FrameRate = GlobalApp->MainProj->Interactive->GetFrameRate();

	AllEvents[0] = MAKE_ID(HostCritter->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
	NotifyEvents[0] = MAKE_ID(HostCritter->GetNotifyClass(), HostCritter->GetNotifySubclass(), HostCritter->GetNotifyItem(), 0xff);
	NotifyEvents[1] = NULL;
	// these two notifications are processed separately in HandleNotifyEvent
	NotifyExHost->RegisterClient(this, NotifyEvents);
	NotifyExHost->RegisterClient(this, AllEvents);
	GlobalApp->MainProj->RegisterClient(this, AllProjPrefsEvents);

	if (! CreateBackups())
		ConstructError = 1;
	} // if
else
	ConstructError = 1;

} // AnimGraphGUI::AnimGraphGUI

/*===========================================================================*/

AnimGraphGUI::~AnimGraphGUI()
{

// need to remove client twice since it is registered twice
if (NotifyExHost)
	NotifyExHost->RemoveClient(this);
if (NotifyExHost)
	NotifyExHost->RemoveClient(this);
GlobalApp->MainProj->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

switch (HostCritter->GetType())
	{
	case WCS_ANIMCRITTER_TYPE_DOUBLETIME:
		{
		delete (AnimDoubleTime *)RestoreBackup;
		delete (AnimDoubleTime *)UndoBackup;
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLETIME
	case WCS_ANIMCRITTER_TYPE_DOUBLEDIST:
		{
		delete (AnimDoubleDistance *)RestoreBackup;
		delete (AnimDoubleDistance *)UndoBackup;
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLEDIST
	case WCS_ANIMCRITTER_TYPE_DOUBLECURVE:
		{
		delete (AnimDoubleCurve *)RestoreBackup;
		delete (AnimDoubleCurve *)UndoBackup;
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLECURVE
	case WCS_ANIMCRITTER_TYPE_COLORTIME:
		{
		delete (AnimColorTime *)RestoreBackup;
		delete (AnimColorTime *)UndoBackup;
		break;
		} // WCS_ANIMCRITTER_TYPE_COLORTIME
	default:
		break;
	} // switch

} // AnimGraphGUI::~AnimGraphGUI()

/*===========================================================================*/

static RECT GrRePos, GrOri;

long AnimGraphGUI::HandleEvent(void)
{
LPMINMAXINFO LPMMI;

if(Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
	{
	switch (Activity->GUIMessage.message)
		{
		case WM_GETMINMAXINFO:
			{
			LPMMI = (LPMINMAXINFO)Activity->GUIMessage.lParam;
			LPMMI->ptMaxTrackSize.x = AppScope->WinSys->InquireDisplayWidth();
			LPMMI->ptMaxTrackSize.y = AppScope->WinSys->InquireDisplayHeight();
			LPMMI->ptMinTrackSize.x = StockX;
			LPMMI->ptMinTrackSize.y = StockY;
			return(0);
			} // MINMAXINFO
		default:
			break;
		} // switch
	} // else if

return(0);

} // AnimGraphGUI::HandleEvent

/*===========================================================================*/


long AnimGraphGUI::HandleReSized(int ReSizeType, long NewWidth, long NewHeight)
{
short NewOffX, NewBotOffX, NewBotY, NewWidWide, SideWidWide;
POINT ConvertCoord;

GetWindowRect(SideControls, &GrRePos);
SideWidWide = (short)(GrRePos.right - GrRePos.left);

GetWindowRect(TopControls, &GrRePos);
ConvertCoord.x = GrRePos.left;
ConvertCoord.y = GrRePos.top;
ScreenToClient(NativeWin, &ConvertCoord);
NewOffX = ((short)NewWidth - ((int)(GrRePos.right - GrRePos.left)))/2;
SetWindowPos(TopControls, NULL, NewOffX, ConvertCoord.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

GetWindowRect(BotControls, &GrRePos);
NewBotOffX = ((short)NewWidth - ((int)(GrRePos.right - GrRePos.left)))/2;

GetWindowRect(GraphWidget, &GrRePos);
ConvertCoord.x = GrRePos.left;
ConvertCoord.y = GrRePos.top;
ScreenToClient(NativeWin, &ConvertCoord);
WidX = ConvertCoord.x;
WidY = ConvertCoord.y;

GetClientRect(BotControls, &GrOri);
NewOffX = (short)GrOri.bottom;

GetClientRect(NativeWin, &GrOri);
NewWidWide = (short)NewWidth - SideWidWide - 10;
GetClientRect(GraphWidget, &GrRePos);
NewOffX = (short)(GrOri.bottom - NewOffX - WidY);

SetWindowPos(SideControls, NULL, (NewWidth - SideWidWide) - 2, WidY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

NewBotY = (short)(WidY + NewOffX + 1);
SetWindowPos(BotControls, NULL, NewBotOffX, NewBotY, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

SetWindowPos(GraphWidget, NULL, 0, 0, NewWidWide, (NewBotY - WidY) - 5, SWP_NOZORDER | SWP_NOMOVE);
InvalidateRect(GraphWidget, NULL, NULL);

return(0);
} // AnimGraphGUI::HandleReSized

/*===========================================================================*/

long AnimGraphGUI::HandleCloseWin(NativeGUIWin NW)
{

Keep();
AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_GRG, MyGraphID);

return(0);

} // AnimGraphGUI::HandleCloseWin

/*===========================================================================*/

unsigned long AnimGraphGUI::QueryHelpTopic(void)
{
RasterAnimHostProperties Prop;

Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
HostCritter->GetRAHostProperties(&Prop);

switch (Prop.TypeNumber)
	{
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE:
		{
		if (HostCritter->RAParent)
			{
			HostCritter->RAParent->GetRAHostProperties(&Prop);
			if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
				return ('ANCC');
			} // if
		return ('ANEF');
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION:
		{
		return ('ANCS');
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE:
		{
		return ('ANEN');
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE:
		{
		return ('ANDI');
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF:
		{
		if (HostCritter->RAParent)
			{
			HostCritter->RAParent->GetRAHostProperties(&Prop);
			if (Prop.TypeNumber == WCS_EFFECTSSUBCLASS_CLOUD)
				{
				if (HostCritter == &((CloudEffect *)HostCritter->RAParent)->CovgProf)
					return ('ANCP');
				if (HostCritter == &((CloudEffect *)HostCritter->RAParent)->DensityProf)
					return ('ANDP');
				if (HostCritter == &((CloudEffect *)HostCritter->RAParent)->ShadeProf)
					return ('ANSP');
				} // if
			} // if
		break; // shouldn't ever hit this, but it's here so lint won't complain about a fallthrough
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE:
		{
		return ('ANCV');
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE
	case WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME:
	case WCS_RAHOST_OBJTYPE_ANIMCOLORTIME:
		{
		return (FenID);
		} // WCS_RAHOST_OBJTYPE_ANIMDOUBLETIME
	default:
		break;
	} // switch

return (FenID);

} // AnimGraphGUI::QueryHelpTopic

/*===========================================================================*/

long AnimGraphGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{

if(ScrollCode == 1)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			WidgetLocal.HPan = (short)ScrollPos;
			RefreshGraph();
			break;
			}
		case IDC_SCROLLBAR2:
			{
			WidgetLocal.HVisible = (short)ScrollPos;
			RefreshGraph();
			break;
			}
		default:
			break;
		} // switch
	return(0);
	} // HSCROLL
else if (ScrollCode == 2)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR4:
			{
			WidgetLocal.VPan = 100 - (short)ScrollPos;
			RefreshGraph();
			break;
			}
		case IDC_SCROLLBAR5:
			{
			WidgetLocal.VVisible = 100 - (short)ScrollPos;
			RefreshGraph();
			break;
			}
		default:
			break;
		} // switch
	return(0);
	} // VSCROLL
else
	return(5);

} // AnimGraphGUI::HandleScroll


/*===========================================================================*/

long AnimGraphGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(063, 63);
switch (ButtonID)
	{
	case ID_KEEP:
		{
		Keep();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_GRG, MyGraphID);
		break;
		} // 
	case IDCANCEL:
		{
		Cancel();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_GRG, MyGraphID);
		break;
		} // 
	case IDC_NEXTNODE:
		{
		NextNode();
		break;
		} // 
	case IDC_PREVNODE:
		{
		PrevNode();
		break;
		} // 
	case IDC_ADDNODE:
		{
		AddNode();
		break;
		} // 
	case IDC_DELETENODE:
		{
		RemoveNode();
		break;
		} // 
	case IDC_SELECTALL:
		{
		SelectAllNodes();
		break;
		} // 
	case IDC_SELECTCLEAR:
		{
		ClearSelectNodes();
		break;
		} // 
	case IDC_SELECTTOGGLE:
		{
		ToggleSelectNodes();
		break;
		} // 
	case IDC_UNDO:
		{
		Undo();
		break;
		} // 
	case IDC_RESET:
		{
		Restore();
		break;
		} // 
	case IDC_FREEECO:
		{
		FreeEcosystem();
		NewEcosystem();
		ConfigureWidgets();
		break;
		} // IDC_FREEECO
	case IDC_EDITECO:
		{
		if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data && WidgetLocal.ActiveNode->Data->Eco)
			WidgetLocal.ActiveNode->Data->Eco->EditRAHost();
		break;
		} // IDC_EDITECO
	default:
		break;
	} // ButtonID

if (ButtonID != ID_KEEP && ButtonID != IDCANCEL)
	RasterAnimHost::SetActiveRAHost(HostCritter);

return(0);

} // AnimGraphGUI::HandleButtonClick

/*===========================================================================*/

long AnimGraphGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_ECODROP:
		{
		SelectNewEco();
		NewEcosystem();
		ConfigureWidgets();
		break;
		} // IDC_ECODROP
	default:
		break;
	} // switch CtrlID

RasterAnimHost::SetActiveRAHost(HostCritter);

return (0);

} // AnimGraphGUI::HandleCBChange

/*===========================================================================*/

long AnimGraphGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CHECKGRID:
		{
		InvalidateRect(GraphWidget, NULL, NULL);
		break;
		} // IDC_CHECKGRID
	case IDC_CHECKLIN:
		{
		NewLinear();
		//RefreshGraph();
		break;
		}
	default:
		break;
	} // switch

RasterAnimHost::SetActiveRAHost(HostCritter);

return (0);

} // AnimGraphGUI::HandleSCChange

/*===========================================================================*/

long AnimGraphGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_VALUE:
		{
		Value = ValueADT.CurValue;
		NewValue();
		break;
		}
	case IDC_DISTANCE:
		{
		Distance = DistanceADT.CurValue;
		NewDistance();
		break;
		}
	case IDC_MAXDISPLAYVAL:
		{
		WidgetLocal.MaxHighDrawVal = MaxValueADT.CurValue;
		RefreshGraph();
		break;
		}
	case IDC_TENSION:
		{
		NewTCB(0);
		//RefreshGraph();
		break;
		}
	case IDC_CONTINUITY:
		{
		NewTCB(1);
		//RefreshGraph();
		break;
		}
	case IDC_BIAS:
		{
		NewTCB(2);
		//RefreshGraph();
		break;
		}
	case IDC_PRIORITY:
		{
		NewPriority();
		break;
		}
	case IDC_SLOPEROUGH:
		{
		NewPriority();
		break;
		}
	case IDC_SLOPEECOMIX:
		{
		NewPriority();
		break;
		}
	case IDC_MINDISPLAYVAL:
		{
		WidgetLocal.MinLowDrawVal = MinValueADT.CurValue;
		RefreshGraph();
		break;
		}
	default:
		break;
	} // switch

RasterAnimHost::SetActiveRAHost(HostCritter);

return(0);

} // AnimGraphGUI::HandleFIChange

/*===========================================================================*/

void AnimGraphGUI::HandleNotifyEvent(void)
{
NotifyTag Changed, *Changes, Interested[7];
int Pos, CurPos, Configure = 0;
GeneralEffect *MyEffect, *MatchEffect;

if (! NativeWin || IgnoreNotify)
	return;

Changes = Activity->ChangeNotify->ChangeList;

if (Changed = NotifyExHost->MatchNotifyClass(NotifyEvents, Changes, 0))
	{
	if ((Changed & 0x000000ff) == WCS_NOTIFYCOMP_ANIM_ABOUTTOCHANGE)
		UpdateUndo();
	else
		{
		if ((Changed & 0x000000ff) == WCS_NOTIFYCOMP_ANIM_NODEREMOVED || (Changed & 0x000000ff) == WCS_NOTIFYCOMP_OBJECT_CHANGED)
			WidgetLocal.ActiveNode = HostCritter->GetFirstSelectedNode(0);
		Configure = 1;
		} // else
	} // if

Interested[0] = MAKE_ID(HostCritter->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	WidgetLocal.ActiveNode = HostCritter->GetFirstSelectedNode(0);
	Configure = 1;
	} // if

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Interested[2] = MAKE_ID(WCS_EFFECTSSUBCLASS_ECOSYSTEM, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
Interested[3] = NULL;
if (Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	CurPos = -1;
	MatchEffect = (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data) ? (GeneralEffect *)WidgetLocal.ActiveNode->Data->Eco: NULL;
	WidgetCBClear(IDC_ECODROP);
	WidgetCBInsert(IDC_ECODROP, -1, "New Ecosystem...");
	for (MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_ECOSYSTEM); MyEffect; MyEffect = MyEffect->Next)
		{
		Pos = WidgetCBInsert(IDC_ECODROP, -1, MyEffect->GetName());
		WidgetCBSetItemData(IDC_ECODROP, Pos, MyEffect);
		if (MyEffect == MatchEffect)
			CurPos = Pos;
		} // for
	WidgetCBSetCurSel(IDC_ECODROP, CurPos);
	} // if eco name changed

Interested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_UNITS, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->MainProj->MatchNotifyClass(Interested, Changes, 0))
	{
	Configure = 1;
	} // if

if (Configure)
	ConfigureWidgets();

} // AnimGraphGUI::HandleNotifyEvent()

/*===========================================================================*/

void AnimGraphGUI::ConfigureWidgets(void)
{
double TempMinLowDrawVal, TempMaxHighDrawVal, MaxDist, MinDist, ValRange;
EcosystemEffect *TestObj;
RasterAnimHost *Root;
RasterAnimHostProperties Prop;
long SelectedNodes, ListPos, Ct, NumEntries;
char Title[256];

WidgetSetScrollPos(IDC_SCROLLBAR1, WidgetLocal.HPan);
WidgetSetScrollPos(IDC_SCROLLBAR2, WidgetLocal.HVisible);
WidgetSetScrollPos(IDC_SCROLLBAR4, 100 - WidgetLocal.VPan);
WidgetSetScrollPos(IDC_SCROLLBAR5, 100 - WidgetLocal.VVisible);
ConfigureSC(NativeWin, IDC_CHECKGRID, &WidgetLocal.drawgrid, SCFlag_Short, NULL, NULL);
ConfigureSC(NativeWin, IDC_CHECKSNAP, &WidgetLocal.SnapToInt, SCFlag_Short, NULL, NULL);

if (HostCritter)
	{
	Title[0] = 0;
	if (HostCritter->GetRAHostName())
		{
		strcpy(Title, HostCritter->GetRAHostName());
		if (Root = HostCritter->GetRAHostRoot())
			{
			strcat(Title, " ");
			strcat(Title, Root->GetRAHostTypeString());
			} // if
		} // if
	if (HostCritter->RAParent)
		{
		strcat(Title, " ");
		strcat(Title, HostCritter->RAParent->GetCritterName(HostCritter));
		} // if
	if (Title[0])
		SetTitle(Title);
	
	ValueADT.SetMetricType(HostCritter->GetMetricType());
	ValueADT.SetMultiplier(HostCritter->GetMultiplier());
	ValueADT.SetIncrement(HostCritter->GetIncrement());

	DistanceADT.SetMetricType(HostCritter->TimeDude() ? WCS_ANIMDOUBLE_METRIC_TIME: 
		HostCritter->GetType() == WCS_ANIMCRITTER_TYPE_DOUBLEDIST ||
		HostCritter->GetType() == WCS_ANIMCRITTER_TYPE_COLORDIST ?
		WCS_ANIMDOUBLE_METRIC_DISTANCE: 
		HostCritter->GetType() == WCS_ANIMCRITTER_TYPE_DOUBLECURVE ? 
		((AnimDoubleCurve *)HostCritter)->GetHorizontalMetric(): WCS_ANIMDOUBLE_METRIC_DIMENSIONLESS);
	DistanceADT.SetMultiplier(1.0);
	DistanceADT.SetIncrement(HostCritter->TimeDude() ? 1.0 / GlobalApp->MainProj->Interactive->GetFrameRate(): 1.0);

	WidgetLocal.MaxDistRange = (HostCritter->TimeDude() ? GlobalApp->MCP->GetMaxFrame() / GlobalApp->MainProj->Interactive->GetFrameRate(): (HostCritter->GetMinMaxDist(MinDist, MaxDist) ? (MaxDist > 1.0 ? MaxDist: 1.0): 1.0));
	if (WidgetLocal.MaxDistRange < 1.0)
		WidgetLocal.MaxDistRange = 1.0;
	WidgetLocal.LowDrawDist = 0.0;
	WidgetLocal.HighDrawDist = WidgetLocal.MaxDistRange;

	HostCritter->GetMinMaxVal(TempMinLowDrawVal, TempMaxHighDrawVal);

	if (TempMinLowDrawVal < WidgetLocal.MinLowDrawVal || TempMaxHighDrawVal > WidgetLocal.MaxHighDrawVal)
		{
		WidgetLocal.MinLowDrawVal = TempMinLowDrawVal;
		WidgetLocal.MaxHighDrawVal = TempMaxHighDrawVal;
		if (WidgetLocal.MaxHighDrawVal == WidgetLocal.MinLowDrawVal)
			{
			ValRange = HostCritter->GetIncrement();
			WidgetLocal.MaxHighDrawVal += ValRange;
			WidgetLocal.MinLowDrawVal -= ValRange;
			} // if
		else
			{
			ValRange = (WidgetLocal.MaxHighDrawVal - WidgetLocal.MinLowDrawVal) * .1;
			WidgetLocal.MaxHighDrawVal += ValRange;
			WidgetLocal.MinLowDrawVal -= ValRange;
			} // else
		} // if

	MaxValueADT.SetValue(WidgetLocal.MaxHighDrawVal);
	MaxValueADT.SetMetricType(HostCritter->GetMetricType());
	MaxValueADT.SetMultiplier(HostCritter->GetMultiplier());
	MaxValueADT.SetIncrement(HostCritter->GetIncrement());

	MinValueADT.SetValue(WidgetLocal.MinLowDrawVal);
	MinValueADT.SetMetricType(HostCritter->GetMetricType());
	MinValueADT.SetMultiplier(HostCritter->GetMultiplier());
	MinValueADT.SetIncrement(HostCritter->GetIncrement());

	// figure distance increment
	WidgetSNConfig(IDC_MAXDISPLAYVAL, &MaxValueADT);
	WidgetSNConfig(IDC_MINDISPLAYVAL, &MinValueADT);

	//ConfigureFI(NativeWin, IDC_MAXDISPLAYVAL,
	// &WidgetLocal.MaxHighDrawVal,
	//  HostCritter->GetIncrement(),
	//   HostCritter->GetMinVal(),
	//	HostCritter->GetMaxVal(),
	//	 FIOFlag_Double, NULL, 0);

	//ConfigureFI(NativeWin, IDC_MINDISPLAYVAL,
	// &WidgetLocal.MinLowDrawVal,
	//  HostCritter->GetIncrement(),
	//   HostCritter->GetMinVal(),
	//	HostCritter->GetMaxVal(),
	//	 FIOFlag_Double, NULL, 0);

	if (WidgetLocal.ActiveNode)
		{
		if (HostCritter->TimeDude())
			{
			Distance = WidgetLocal.ActiveNode->GetDistance();
			if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
				{
				//Distance *= FrameRate;
				WidgetSetText(IDC_DISTFRAMETEXT, "Frame");
				} // IF
			else
				WidgetSetText(IDC_DISTFRAMETEXT, "Time (secs)");
			} // if
		else
			{
			Distance = WidgetLocal.ActiveNode->GetDistance();
			WidgetSetText(IDC_DISTFRAMETEXT, "Distance");
			} // else
		Value = WidgetLocal.ActiveNode->GetValue();

		DistanceADT.SetValue(Distance);
		ValueADT.SetValue(Value);

		WidgetSNConfig(IDC_DISTANCE, &DistanceADT);
		WidgetSNConfig(IDC_VALUE, &ValueADT);

		//ConfigureFI(NativeWin, IDC_DISTANCE,
		// &Distance,
		//  1.0,
		//   0.0,
		//	FLT_MAX,
		//	 FIOFlag_Double, NULL, 0);

		//ConfigureFI(NativeWin, IDC_VALUE,
		// &Value,
		//  HostCritter->GetIncrement(),
		//   HostCritter->GetMinVal(),
		//	HostCritter->GetMaxVal(),
		//	 FIOFlag_Double, NULL, 0);

		ConfigureFI(NativeWin, IDC_TENSION,
		 &WidgetLocal.ActiveNode->TCB[0],
		  .1,
		   -5.0,
			5.0,
			 FIOFlag_Double, NULL, 0);

		ConfigureFI(NativeWin, IDC_CONTINUITY,
		 &WidgetLocal.ActiveNode->TCB[1],
		  .1,
		   -5.0,
			5.0,
			 FIOFlag_Double, NULL, 0);

		ConfigureFI(NativeWin, IDC_BIAS,
		 &WidgetLocal.ActiveNode->TCB[2],
		  .1,
		   -5.0,
			5.0,
			 FIOFlag_Double, NULL, 0);

		ConfigureSC(NativeWin, IDC_CHECKLIN, &WidgetLocal.ActiveNode->Linear, SCFlag_Char, NULL, 0);

		WidgetSetDisabled(IDC_PREVNODE, ! WidgetLocal.ActiveNode->Prev);
		WidgetSetDisabled(IDC_NEXTNODE, ! WidgetLocal.ActiveNode->Next);

		if (! (SelectedNodes = HostCritter->GetNumSelectedNodes()))
			{
			WidgetLocal.ActiveNode->SetFlag(WCS_GRAPHNODE_FLAGS_SELECTED);
			SelectedNodes = 1;
			} // if
		sprintf(Title, "%1d", SelectedNodes);
		WidgetSetText(IDC_NUMNODES, Title);

		if (WidgetLocal.ActiveNode->Data)
			{
			WidgetSNConfig(IDC_SLOPEROUGH, &WidgetLocal.ActiveNode->Data->Roughness);
			WidgetSNConfig(IDC_SLOPEECOMIX, &WidgetLocal.ActiveNode->Data->EcoMixing);

			ConfigureFI(NativeWin, IDC_PRIORITY,
			 &WidgetLocal.ActiveNode->Data->Priority,
			  1.0,
			   -99.0,
				99.0,
				 FIOFlag_Short, NULL, 0);

			if (WidgetLocal.ActiveNode->Data->Eco)
				{
				ListPos = -1;
				NumEntries = WidgetCBGetCount(IDC_ECODROP);
				for (Ct = 0; Ct < NumEntries; Ct ++)
					{
					if ((TestObj = (EcosystemEffect *)WidgetCBGetItemData(IDC_ECODROP, Ct)) != (EcosystemEffect *)LB_ERR && TestObj == WidgetLocal.ActiveNode->Data->Eco)
						{
						ListPos = Ct;
						break;
						} // for
					} // for
				WidgetCBSetCurSel(IDC_ECODROP, ListPos);
				} // if
			else
				WidgetCBSetCurSel(IDC_ECODROP, -1);
			WidgetSetDisabled(IDC_ECODROP, FALSE);
			WidgetSetDisabled(IDC_FREEECO, FALSE);
			WidgetSetDisabled(IDC_EDITECO, FALSE);
			WidgetShow(IDC_SLOPEROUGH, 1);
			WidgetShow(IDC_SLOPEECOMIX, 1);
			WidgetShow(IDC_PRIORITY, 1);
			WidgetShow(IDC_ECODROP, 1);
			WidgetShow(IDC_FREEECO, 1);
			WidgetShow(IDC_EDITECO, 1);
			WidgetShow(IDC_ECOSYSTEXT, 1);
			WidgetShow(IDC_BSTEXT, 0);
			} // if
		else
			{
			WidgetSNConfig(IDC_SLOPEROUGH, NULL);
			WidgetSNConfig(IDC_SLOPEECOMIX, NULL);
			ConfigureFI(NativeWin, IDC_PRIORITY, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
			WidgetSetDisabled(IDC_ECODROP, TRUE);
			WidgetSetDisabled(IDC_FREEECO, TRUE);
			WidgetSetDisabled(IDC_EDITECO, TRUE);
			WidgetShow(IDC_SLOPEROUGH, 0);
			WidgetShow(IDC_SLOPEECOMIX, 0);
			WidgetShow(IDC_PRIORITY, 0);
			WidgetShow(IDC_ECODROP, 0);
			WidgetShow(IDC_FREEECO, 0);
			WidgetShow(IDC_EDITECO, 0);
			WidgetShow(IDC_ECOSYSTEXT, 0);
			WidgetShow(IDC_BSTEXT, 1);
			Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
			HostCritter->GetRAHostProperties(&Prop);
			if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE && HostCritter->RAParent)
				{
				HostCritter->RAParent->GetRAHostProperties(&Prop);
				if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
					WidgetSetText(IDC_BSTEXT, BSText[6]);
				else
					WidgetSetText(IDC_BSTEXT, BSText[0]);
				} // if
			else
				{
				WidgetSetText(IDC_BSTEXT, Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE ? BSText[0]:
					Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION ? BSText[1]: 
					Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE ? BSText[2]: 
					Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE ? BSText[3]: 
					Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF ? BSText[4]: 
					Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE ? BSText[5]: 
					BSText[7]);
				} // else
			} // else
		} // if
	else
		{
		Value = HostCritter->GetCurValue(0);
		ValueADT.SetValue(Value);
		WidgetSNConfig(IDC_VALUE, &ValueADT);

		//ConfigureFI(NativeWin, IDC_VALUE,
		// &Value,
		//  HostCritter->GetIncrement(),
		//   HostCritter->GetMinVal(),
		//	HostCritter->GetMaxVal(),
		//	 FIOFlag_Double, NULL, 0);
		ConfigureFI(NativeWin, IDC_DISTANCE, NULL, 0.0, 0.0, (double)FLT_MAX, 0, NULL, 0);
		ConfigureFI(NativeWin, IDC_TENSION, NULL, .1, -5.0, 5.0, 0, NULL, 0);
		ConfigureFI(NativeWin, IDC_CONTINUITY, NULL, .1, -5.0, 5.0, 0, NULL, 0);
		ConfigureFI(NativeWin, IDC_BIAS, NULL, .1, -5.0, 5.0, 0, NULL, 0);
		ConfigureSC(NativeWin, IDC_CHECKLIN, NULL, 0, NULL, 0);
		WidgetSetDisabled(IDC_PREVNODE, TRUE);
		WidgetSetDisabled(IDC_NEXTNODE, TRUE);
		sprintf(Title, "%1d", 0);
		WidgetSetText(IDC_NUMNODES, Title);
		if (HostCritter->TimeDude())
			{
			if (GlobalApp->MainProj->Prefs.TimeDisplayUnits == WCS_PROJPREFS_TIMEUNITS_FRAMES)
				WidgetSetText(IDC_DISTFRAMETEXT, "Frame");
			else
				WidgetSetText(IDC_DISTFRAMETEXT, "Time (secs)");
			} // if
		else
			{
			// if there were a way to change units then the units would be used to figure the distance & increment
			WidgetSetText(IDC_DISTFRAMETEXT, "Distance");
			} // else
		WidgetSNConfig(IDC_SLOPEROUGH, NULL);
		WidgetSNConfig(IDC_SLOPEECOMIX, NULL);
		ConfigureFI(NativeWin, IDC_PRIORITY, NULL, 0.0, 0.0, 0.0, 0, NULL, 0);
		WidgetSetDisabled(IDC_ECODROP, TRUE);
		WidgetSetDisabled(IDC_FREEECO, TRUE);
		WidgetSetDisabled(IDC_EDITECO, TRUE);
		WidgetShow(IDC_SLOPEROUGH, 0);
		WidgetShow(IDC_SLOPEECOMIX, 0);
		WidgetShow(IDC_PRIORITY, 0);
		WidgetShow(IDC_ECODROP, 0);
		WidgetShow(IDC_FREEECO, 0);
		WidgetShow(IDC_EDITECO, 0);
		WidgetShow(IDC_ECOSYSTEXT, 0);
		WidgetShow(IDC_BSTEXT, 1);
		Prop.PropMask = WCS_RAHOST_MASKBIT_TYPENUMBER;
		HostCritter->GetRAHostProperties(&Prop);
		if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE && HostCritter->RAParent)
			{
			HostCritter->RAParent->GetRAHostProperties(&Prop);
			if (Prop.TypeNumber == WCS_RAHOST_OBJTYPE_TEXTURE)
				WidgetSetText(IDC_BSTEXT, BSText[6]);
			else
				WidgetSetText(IDC_BSTEXT, BSText[0]);
			} // if
		else
			{
			WidgetSetText(IDC_BSTEXT, Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEPROFILE ? BSText[0]:
				Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLESECTION ? BSText[1]: 
				Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEENVELOPE ? BSText[2]: 
				Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLEDISTANCE ? BSText[3]: 
				Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLECLOUDPROF ? BSText[4]: 
				Prop.TypeNumber == WCS_RAHOST_OBJTYPE_ANIMDOUBLECURVE ? BSText[5]: 
				BSText[7]);
			} // else
		} // else
	WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
	ConfigureGR(NativeWin, IDC_GRAPH, &WidgetLocal);
	} // if
} // AnimGraphGUI::ConfigureWidgets()

/*===========================================================================*/

void AnimGraphGUI::Keep(void)
{

IgnoreNotify = 1;

} // AnimGraphGUI::Keep

/*===========================================================================*/

void AnimGraphGUI::Cancel(void)
{

IgnoreNotify = 1;
HostCritter->Copy(HostCritter, RestoreBackup);
NotifyExHost->GenerateNotify(NotifyEvents);

} // AnimGraphGUI::Cancel

/*===========================================================================*/

void AnimGraphGUI::UpdateUndo(void)
{

HostCritter->Copy(UndoBackup, HostCritter);

} // AnimGraphGUI::Undo

/*===========================================================================*/

void AnimGraphGUI::Undo(void)
{

HostCritter->Copy(HostCritter, UndoBackup);
WidgetLocal.ActiveNode = HostCritter->GetFirstSelectedNode(0);
NotifyExHost->GenerateNotify(NotifyEvents);

} // AnimGraphGUI::Undo

/*===========================================================================*/

void AnimGraphGUI::Restore(void)
{

HostCritter->Copy(HostCritter, RestoreBackup);
WidgetLocal.ActiveNode = HostCritter->GetFirstSelectedNode(0);
NotifyExHost->GenerateNotify(NotifyEvents);

} // AnimGraphGUI::Restore

/*===========================================================================*/

void AnimGraphGUI::NewActiveNode(GraphNode *NewActive)
{

WidgetLocal.ActiveNode = NewActive;
ConfigureWidgets();
if (HostCritter->TimeDude())
	GlobalApp->MainProj->Interactive->SetParam(1, WCS_INTERCLASS_TIME, WCS_TIME_SUBCLASS_TIME, 0, 0,
		NewActive->GetDistance(), 0);

} // AnimGraphGUI::NewActiveNode

/*===========================================================================*/

int AnimGraphGUI::CreateBackups(void)
{

switch (HostCritter->GetType())
	{
	case WCS_ANIMCRITTER_TYPE_DOUBLETIME:
		{
		if (RestoreBackup = (AnimCritter *)new AnimDoubleTime)
			{
			((AnimDoubleTime *)HostCritter)->Copy((AnimDoubleTime *)RestoreBackup, (AnimDoubleTime *)HostCritter);
			if (UndoBackup = (AnimCritter *)new AnimDoubleTime)
				{
				((AnimDoubleTime *)HostCritter)->Copy((AnimDoubleTime *)UndoBackup, (AnimDoubleTime *)HostCritter);
				return (1);
				} // if
			} // if
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLETIME
	case WCS_ANIMCRITTER_TYPE_DOUBLEDIST:
		{
		if (RestoreBackup = (AnimCritter *)new AnimDoubleDistance)
			{
			((AnimDoubleDistance *)HostCritter)->Copy((AnimDoubleDistance *)RestoreBackup, (AnimDoubleDistance *)HostCritter);
			if (UndoBackup = (AnimCritter *)new AnimDoubleDistance)
				{
				((AnimDoubleDistance *)HostCritter)->Copy((AnimDoubleDistance *)UndoBackup, (AnimDoubleDistance *)HostCritter);
				return (1);
				} // if
			} // if
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLEDIST
	case WCS_ANIMCRITTER_TYPE_DOUBLECURVE:
		{
		if (RestoreBackup = (AnimCritter *)new AnimDoubleCurve)
			{
			((AnimDoubleCurve *)HostCritter)->Copy((AnimDoubleCurve *)RestoreBackup, (AnimDoubleCurve *)HostCritter);
			if (UndoBackup = (AnimCritter *)new AnimDoubleCurve)
				{
				((AnimDoubleCurve *)HostCritter)->Copy((AnimDoubleCurve *)UndoBackup, (AnimDoubleCurve *)HostCritter);
				return (1);
				} // if
			} // if
		break;
		} // WCS_ANIMCRITTER_TYPE_DOUBLECURVE
	case WCS_ANIMCRITTER_TYPE_COLORTIME:
		{
		if (RestoreBackup = (AnimCritter *)new AnimColorTime)
			{
			((AnimColorTime *)HostCritter)->Copy((AnimColorTime *)RestoreBackup, (AnimColorTime *)HostCritter);
			if (UndoBackup = (AnimCritter *)new AnimColorTime)
				{
				((AnimColorTime *)HostCritter)->Copy((AnimColorTime *)UndoBackup, (AnimColorTime *)HostCritter);
				return (1);
				} // if
			} // if
		break;
		} // WCS_ANIMCRITTER_TYPE_COLORTIME
	default:
		break;
	} // switch

return (0);

} // AnimGraphGUI::CreateBackups

/*===========================================================================*/

void AnimGraphGUI::NextNode(void)
{

if (WidgetLocal.ActiveNode)
	{
	WidgetLocal.ActiveNode = WidgetLocal.ActiveNode->Next;
	HostCritter->ClearNodeSelectedAll();
	HostCritter->SetNodeSelected(WidgetLocal.ActiveNode, 1);
	} // if
else
	WidgetLocal.ActiveNode = HostCritter->GetFirstNode(0);

ConfigureWidgets();

} // AnimGraphGUI::NextNode

/*===========================================================================*/

void AnimGraphGUI::PrevNode(void)
{

if (WidgetLocal.ActiveNode)
	{
	WidgetLocal.ActiveNode = WidgetLocal.ActiveNode->Prev;
	HostCritter->ClearNodeSelectedAll();
	HostCritter->SetNodeSelected(WidgetLocal.ActiveNode, 1);
	} // if
else
	WidgetLocal.ActiveNode = HostCritter->GetFirstNode(0);

ConfigureWidgets();

} // AnimGraphGUI::PrevNode

/*===========================================================================*/

void AnimGraphGUI::AddNode(void)
{

WidgetLocal.inputflags = WCSW_GRW_NODE_NEW;

} // AnimGraphGUI::AddNode

/*===========================================================================*/

void AnimGraphGUI::RemoveNode(void)
{

if (WidgetLocal.ActiveNode)
	{
	HostCritter->RemoteRemoveSelectedNode();
	} // if

} // AnimGraphGUI::RemoveNode

/*===========================================================================*/

void AnimGraphGUI::SelectAllNodes(void)
{

HostCritter->SetNodeSelectedAll();

} // AnimGraphGUI::SelectAllNodes

/*===========================================================================*/

void AnimGraphGUI::ClearSelectNodes(void)
{

HostCritter->ClearNodeSelectedAll();
if (WidgetLocal.ActiveNode)
	HostCritter->SetNodeSelected(WidgetLocal.ActiveNode, 1);

} // AnimGraphGUI::ClearSelectNodes

/*===========================================================================*/

void AnimGraphGUI::ToggleSelectNodes(void)
{

HostCritter->ToggleNodeSelectedAll();
WidgetLocal.ActiveNode = HostCritter->GetFirstSelectedNode(0);

} // AnimGraphGUI::ToggleSelectNodes

/*===========================================================================*/

void AnimGraphGUI::NewValue(void)
{

if (WidgetLocal.ActiveNode)
	{
	HostCritter->RemoteAlterSelectedNodeValue(Value, WidgetLocal.ActiveNode->GetValue());
	} // if
else
	{
	HostCritter->SetCurValue(0, Value);
	} // else

} // AnimGraphGUI::NewValue

/*===========================================================================*/

void AnimGraphGUI::NewDistance(void)
{
//double Temp;

if (WidgetLocal.ActiveNode)
	{
	if (WidgetLocal.SnapToInt)
		{
		if (WidgetLocal.DisplayByFrame)
			{
			Distance = quickdblfloor(Distance * FrameRate + .5) / FrameRate;
			} // if
		WidgetFISync(IDC_DISTANCE, WP_FISYNC_NONOTIFY);
		} // if
	//if (WidgetLocal.DisplayByFrame)
	//	{
	//	Temp = Distance / FrameRate;
	//	} // if
	//else
	//	{
	//	Temp = Distance;
	//	} // else
	HostCritter->RemoteAlterSelectedNodePosition(Distance, WidgetLocal.ActiveNode->GetDistance());
	Distance = WidgetLocal.ActiveNode->GetDistance();
	DistanceADT.SetValue(Distance);
	WidgetFISync(IDC_DISTANCE, WP_FISYNC_NONOTIFY);
	} // if

} // AnimGraphGUI::NewDistance

/*===========================================================================*/

void AnimGraphGUI::NewLinear(void)
{

if (WidgetLocal.ActiveNode)
	{
	HostCritter->RemoteAlterSelectedNodeLinear(WidgetLocal.ActiveNode->GetLinear());
	} // if

} // AnimGraphGUI::NewLinear

/*===========================================================================*/

void AnimGraphGUI::NewTCB(char Channel)
{

if (WidgetLocal.ActiveNode)
	{
	HostCritter->RemoteAlterSelectedNodeTCB(Channel, WidgetLocal.ActiveNode->GetTCB(Channel));
	} // if

} // AnimGraphGUI::NewTCB

/*===========================================================================*/

void AnimGraphGUI::NewPriority(void)
{

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	HostCritter->RemoteAlterSelectedNodePriority(WidgetLocal.ActiveNode->Data->GetPriority());
	} // if

} // AnimGraphGUI::NewPriority

/*===========================================================================*/

void AnimGraphGUI::NewRoughness(void)
{

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	HostCritter->RemoteAlterSelectedNodeRoughness(WidgetLocal.ActiveNode->Data->GetRoughness());
	} // if

} // AnimGraphGUI::NewRoughness

/*===========================================================================*/

void AnimGraphGUI::NewEcoMixing(void)
{

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	HostCritter->RemoteAlterSelectedNodeEcoMixing(WidgetLocal.ActiveNode->Data->GetEcoMixing());
	} // if

} // AnimGraphGUI::NewEcoMixing

/*===========================================================================*/

void AnimGraphGUI::NewEcosystem(void)
{

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	HostCritter->RemoteAlterSelectedNodeEcosystem(WidgetLocal.ActiveNode->Data->GetEcosystem());
	} // if

} // AnimGraphGUI::NewEcosystem

/*===========================================================================*/

void AnimGraphGUI::RefreshGraph(void)
{

WidgetLocal.drawflags = WCSW_GRW_MADF_DRAWOBJECT;
ConfigureGR(NativeWin, IDC_GRAPH, &WidgetLocal);

} // AnimGraphGUI::RefreshGraph

/*===========================================================================*/

unsigned char AnimGraphGUI::GetNotifyClass(void)
{

return (HostCritter->GetNotifyClass());

} // AnimGraphGUI::GetNotifyClass

/*===========================================================================*/

unsigned char AnimGraphGUI::GetNotifySubclass(void)
{

return (HostCritter->GetNotifySubclass());

} // AnimGraphGUI::GetNotifySublass

/*===========================================================================*/

void AnimGraphGUI::SelectNewEco(void)
{
EcosystemEffect *NewEco;
long Current;

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	Current = WidgetCBGetCurSel(IDC_ECODROP);
	if (((NewEco = (EcosystemEffect *)WidgetCBGetItemData(IDC_ECODROP, Current, 0)) != (EcosystemEffect *)LB_ERR && NewEco)
		|| (NewEco = (EcosystemEffect *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_ECOSYSTEM, NULL, NULL)))
		{
		WidgetLocal.ActiveNode->Data->SetEco(NewEco);
		} // if
	} // if

} // AnimGraphGUI::SelectNewEco

/*===========================================================================*/

void AnimGraphGUI::FreeEcosystem(void)
{

if (WidgetLocal.ActiveNode && WidgetLocal.ActiveNode->Data)
	{
	WidgetLocal.ActiveNode->Data->SetEco(NULL);
	} // if

} // AnimGraphGUI::FreeEcosystem

/*===========================================================================*/
