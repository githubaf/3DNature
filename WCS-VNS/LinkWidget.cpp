// LinkWidget.cpp
// Implementation of the Linkage widget
// Built from GridWidget.cpp on 4/24/2008 by Chris 'Xenon' Hanson
// Copyright 2008

#include "stdafx.h"
#include "Types.h"
#include "AppMem.h"
#include "WCSWidgets.h"
#include "WidgetSupport.h"
#include "Application.h"
#include "Joe.h"
#include "WCSVersion.h"
#include "resource.h"
#include "Useful.h"
#include "Palette.h"
#include "Fenetre.h"
#include "Notify.h"
#include "Database.h"
#include "EffectsLib.h"
#include "Lists.h"

#ifdef _WIN32
#include <mmsystem.h>
#include "VisualStylesXP.h"
#endif // _WIN32

//extern WCSApp *GlobalApp;

long WINAPI LinkWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);

long WINAPI LinkWidgetWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
LinkWidgetInstance *LWI = (LinkWidgetInstance *)GetWindowLong(hwnd, 0);
return(LWI->LinkWidgetInternalWndProc(hwnd, message, wParam, lParam)); // forward to object
} // LinkWidgetWndProc

/*===========================================================================*/

LinkWidgetInstance::~LinkWidgetInstance()
{

if (ActionPopupMenu) DestroyMenu(ActionPopupMenu);
ActionPopupMenu = NULL;

} // LinkWidgetInstance::~LinkWidgetInstance

/*===========================================================================*/

long LinkWidgetInstance::LinkWidgetInternalWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam)
{
WORD Notify, MyID;
NativeControl Parent;
LPCREATESTRUCT Create;
HINSTANCE MyInst;
HDC TextDC;
SIZE Ruler;
HFONT TextStyle;
LinkWidgetInstance *LWI, *NewConfig;
LONG Style;
POINT MouseTrans;

Parent  = GetParent(hwnd);
MyID    = (WORD)GetWindowLong(hwnd, GWL_ID);
LWI		= this;
Style   = GetWindowLong(hwnd, GWL_STYLE);

switch (message)
	{
	case WM_DESTROY:
		{
		delete LWI; // caution! We're INSIDE IT!
		SetWindowLong(hwnd, 0, 0);
		return(0);
		} // DESTROY
	case WM_PAINT:
		{
/*
		RECT Square;
		HDC Canvas;
		PAINTSTRUCT PS;
		
		Canvas = BeginPaint(hwnd, &PS);
		GetClientRect(hwnd, &Square);

		// <<<>>> need win2k theming
		HTHEME hTheme = g_xpStyle.OpenThemeData(hwnd, L"EDIT");
		g_xpStyle.DrawThemeBackground(hTheme, Canvas, EP_EDITTEXT, ETS_READONLY, &Square, &Square);
		g_xpStyle.CloseThemeData(hTheme);
		EndPaint(hwnd, &PS);
		Canvas = NULL;
		InvalidateRect(ActionButton, NULL, FALSE);
		return(1);
*/
		break;
		} // PAINT
/*
	case WM_ERASEBKGND:
		{
		RECT ControlRECT;
		GetClientRect(hwnd, &ControlRECT);
		g_xpStyle.DrawThemeParentBackground(hwnd, (HDC)wParam, &ControlRECT);
		return(1);
		} // ERASEBG
*/
	case WM_CTLCOLOREDIT:
	case WM_CTLCOLORBTN:
	case WM_CTLCOLORSTATIC:
		{
		return(CallWindowProc((WNDPROC)GetWindowLong(Parent, GWL_WNDPROC), Parent,
		 message, wParam, (LPARAM)lParam));
		} // WM_CTLCOLOR...
	case WM_SETFONT:
		{
		TextStyle = (HFONT)wParam;
		TextDC = GetDC(hwnd);
		if (TextStyle) SelectObject(TextDC, TextStyle);
		// Tell the children what font to use
		SendMessage(TextField, WM_SETFONT, (WPARAM)TextStyle, 0);
		SendMessage(ActionButton, WM_SETFONT, (WPARAM)TextStyle, 0);
		break;
		} // SETFONT
	case WM_CREATE:
		{ // "this" is NULL until after WM_CREATE, so must use LWI-> referencing
		if (LWI = new LinkWidgetInstance())
			{
			SetWindowLong(hwnd, 0, (long)LWI);
			} // if
		else
			{
			return(-1); // bah, humbug
			} // else

		MyInst = (HINSTANCE)GetClassLong(hwnd, GCL_HMODULE);
		Create = (LPCREATESTRUCT)lParam;
		Ruler.cx = 0;

		// Create TextField
		LWI->TextField = CreateWindowEx(WS_EX_CLIENTEDGE, "EDIT", "", WS_CHILD | ES_CENTER | ES_READONLY | ES_MULTILINE | WS_VISIBLE,
		 2, 2, Create->cx - (16 + 2), Create->cy - 2, hwnd, NULL, MyInst, NULL);

		if (LWI->ActionButton = CreateWindowEx(0, APP_CLASSPREFIX ".ToolButton", "Link Operations",
		 WS_CHILD | WS_VISIBLE | WCSW_TB_STYLE_ALLOWRIGHT | WCSW_TB_STYLE_XPLOOK, Create->cx - 16, 1, 16, 18, hwnd, NULL, MyInst, NULL))
			{
			SetWindowLong(LWI->ActionButton, GWL_ID, 63142);
			ConfigureTB(LWI->ActionButton, NULL, IDI_LINK, NULL);
			} // if
		
		if (LWI->TextField && LWI->ActionButton)
			{
			SetWindowLong(hwnd, 0, (long)LWI);
			return(0);
			} // if
		else
			{
			if (LWI->ActionButton) DestroyWindow(LWI->ActionButton);
			if (LWI->TextField) DestroyWindow(LWI->TextField);
			AppMem_Free(LWI, sizeof(LinkWidgetInstance));
			return(-1);
			} // else
		} // CREATE
	case WM_COMMAND:
		{
		Notify = HIWORD(wParam);
		switch (Notify)
			{
			case BN_CLICKED:
				{
				if (LOWORD(wParam) == 63142)
					{
					GetCursorPos(&MouseTrans);
					DisplayPopup(MouseTrans.x, MouseTrans.y);
					} // if
				return(0);
				} // CLICKED
			} // switch
		break;
		} // WM_COMMAND
	case WM_SETFOCUS:
		{
		SetFocus(ActionButton);
		return(0);
		} // WM_SETFOCUS
	case WM_WCSW_LW_SYNC:
		{
		Sync();
		return(0);
		} // SYNC
	case WM_WCSW_LW_SETUP:
		{
		NewConfig = (LinkWidgetInstance *)lParam;
		// Copy field-by-field so as not to obliterate HWNDs
		if (NewConfig)
			{
			ComponentHost = NewConfig->ComponentHost;
			DBHost = NewConfig->DBHost;
			EffectsHost = NewConfig->EffectsHost;
			NewQueryFlags = NewConfig->NewQueryFlags;
			} // if
		Sync();
		return(0);
		} // SETUP
	case WM_ENABLE:
		{
		// resync subwidgets: "The WM_ENABLE message is sent when an application changes the enabled state of a window. It is sent to the window whose enabled state is changing."
		if (TextField) EnableWindow(TextField, wParam);
		if (ActionButton) EnableWindow(ActionButton, wParam);
		InvalidateParentRect(hwnd);
		break;
		} // WM_ENABLE
	} // switch

return(DefWindowProc(hwnd, message, wParam, lParam));

} // LinkWidgetInstance::LinkWidgetInternalWndProc

/*===========================================================================*/

void LinkWidgetInstance::Sync(void)
{
char TextStr[256], FullString[512], ComponentStr[256], *RawComponentStr, *HardLinkText;
bool HardLink = false, SoftLink = false, IgnoreSoftLink = false, IsWave = false;
JoeList *CurJoe;
#ifdef WCS_BUILD_VNS
//long ListPos, Ct, NumEntries;
//SearchQuery *TestObj;
#endif // WCS_BUILD_VNS

// GetRAHostTypeString returns type name with () around it, just peel them off
ComponentStr[0] = NULL;
if (RawComponentStr = ComponentHost->GetRAHostTypeString())
	{
	strcpy(ComponentStr, &RawComponentStr[1]); // skip first (
	ComponentStr[strlen(ComponentStr) - 1] = NULL;
	} // if

if ((ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_WAVE) || (ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_CLOUD))
	{
	IgnoreSoftLink = true;
	HardLinkText = "attached";
	if (ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_WAVE)
		{
		IsWave = true;
		} // if
	} // if
else
	{
	HardLinkText = "hard-linked";
	} // else

NumJoes = 0;
if ((ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_SCENARIO) &&
	((NumJoes = ((RenderScenario *)ComponentHost)->CountVectors()) > 0))
	{
	HardLink = true;
	if (NumJoes > 1)
		sprintf(TextStr, "%d vectors are %s to this %s.\r\n", NumJoes, HardLinkText, ComponentStr);
	else
		sprintf(TextStr, "One vector is %s to this %s.\r\n", HardLinkText, ComponentStr);
	} // if
else if (CurJoe = ComponentHost->Joes)
	{
	NumJoes = 1;
	HardLink = true;
	while (CurJoe->Next)
		{
		NumJoes ++;
		CurJoe = CurJoe->Next;
		} // while
	if (NumJoes > 1)
		sprintf(TextStr, "%d vectors are %s to this %s.\r\n", NumJoes, HardLinkText, ComponentStr);
	else
		sprintf(TextStr, "One vector is %s to this %s.\r\n", HardLinkText, ComponentStr);
	} // if
else
	{
	HardLink = false;
	// if we ignore soft-linking, avoid suggesting linking with the button, since we can't
	// if we have a softlink that isn't producing, just summarize, but don't suggest linking via the button
	if (ComponentHost->Search || IgnoreSoftLink)
		{
		sprintf(TextStr, "No vectors are %s to this %s.\r\n", HardLinkText, ComponentStr);
		} // if
	else
		{
		sprintf(TextStr, "No vectors are linked to this %s.\r\nUse the button on the right to create links.", ComponentStr);
		} // else
	} // else

if (IsWave)
	{ // no softlinking, but we can summarize Lake/Stream usage in its place
	SoftLink = false;
	LakeEffect *CurLake;
	StreamEffect *CurStream;
	EffectList *CurWave;
	GradientCritter *CurGrad;
	long NumLakes = 0;

	if (CurLake = (LakeEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_LAKE))
		{
		while (CurLake)
			{
			CurGrad = CurLake->WaterMat.Grad;
			while (CurGrad)
				{
				if (CurGrad->GetThing())
					{
					if (CurWave = ((MaterialEffect *)CurGrad->GetThing())->Waves)
						{
						while (CurWave)
							{
							if (CurWave->Me == (GeneralEffect *)ComponentHost)
								{
								NumLakes ++;
								break;
								} // if
							CurWave = CurWave->Next;
							} // while
						} // if
					} // if
				CurGrad = CurGrad->Next;
				} // while
			CurLake = (LakeEffect *)CurLake->Next;
			} // while
		} // if
	if (CurStream = (StreamEffect *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_STREAM))
		{
		while (CurStream)
			{
			CurGrad = CurStream->WaterMat.Grad;
			while (CurGrad)
				{
				if (CurGrad->GetThing())
					{
					if (CurWave = ((MaterialEffect *)CurGrad->GetThing())->Waves)
						{
						while (CurWave)
							{
							if (CurWave->Me == (GeneralEffect *)ComponentHost)
								{
								NumLakes ++;
								break;
								} // if
							CurWave = CurWave->Next;
							} // while
						} // if
					} // if
				CurGrad = CurGrad->Next;
				} // while
			CurStream = (StreamEffect *)CurStream->Next;
			} // while
		} // if
	if (NumLakes > 1)
		sprintf(FullString, "%s%d Lakes and Streams are using this Wave Model.", TextStr, NumLakes);
	else if (NumLakes == 1)
		sprintf(FullString, "%sOne Lake or Stream is using this Wave Model.", TextStr);
	else
		sprintf(FullString, "%sNo Lakes or Streams are using this Wave Model.", TextStr);
	} // if
else if (ComponentHost->Search && !IgnoreSoftLink)
	{
	SoftLink = true;
	int Count = ComponentHost->Search->CountSuccessfulVectorCandidates(DBHost);
	if (Count > 1)
		sprintf(FullString, "%s%d vectors are dynamic-linked using query %s.", TextStr, Count, ComponentHost->Search->Name);
	else if (Count == 1)
		sprintf(FullString, "%sOne vector is dynamic-linked using query %s.", TextStr, ComponentHost->Search->Name);
	else
		sprintf(FullString, "%sNo vectors are dynamic-linked using query %s.", TextStr, ComponentHost->Search->Name);
	NumJoes += Count;
	} // if
else
	{
	SoftLink = false;
	strcpy(FullString, TextStr);
	} // else

// configure link icon color
if (HardLink && SoftLink)
	{
	ConfigureTB(ActionButton, NULL, IDI_LINKBOTH, NULL);
	} // if
else if (HardLink)
	{
	ConfigureTB(ActionButton, NULL, IDI_LINKGREEN, NULL);
	} // else if
else if (SoftLink)
	{
	ConfigureTB(ActionButton, NULL, IDI_LINKBLUE, NULL);
	} // else if
else
	{
	ConfigureTB(ActionButton, NULL, IDI_LINK, NULL);
	} // else
	
SetWindowText(TextField, FullString);
InvalidateRect(TextField, NULL, TRUE);

} // LinkWidgetInstance::Sync

/*===========================================================================*/

void LinkWidgetInstance::HardLink(void)
{

#ifdef WCS_BUILD_VNS
if (ComponentHost->Search)
	ComponentHost->HardLinkVectors(DBHost);
#endif // WCS_BUILD_VNS

} // LinkWidgetInstance::HardLink

/*===========================================================================*/

void LinkWidgetInstance::HardLinkSelectedVectors(void)
{

ComponentHost->HardLinkSelectedVectors(DBHost);

} // LinkWidgetInstance::HardLinkSelectedVectors

/*===========================================================================*/

void LinkWidgetInstance::FreeQuery(void)
{

#ifdef WCS_BUILD_VNS
if (ComponentHost->Search)
	ComponentHost->RemoveRAHost(ComponentHost->Search);
#endif // WCS_BUILD_VNS

} // LinkWidgetInstance::FreeQuery

/*===========================================================================*/

void LinkWidgetInstance::EditQuery(void)
{

#ifdef WCS_BUILD_VNS
if (ComponentHost->Search)
	ComponentHost->Search->EditRAHost();
#endif // WCS_BUILD_VNS

} // LinkWidgetInstance::EditQuery

/*===========================================================================*/

void LinkWidgetInstance::NewQuery(void)
{
#ifdef WCS_BUILD_VNS
SearchQuery *NewObj = NULL;

if (NewObj = (SearchQuery *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_SEARCHQUERY, ComponentHost->Name, NULL))
	{
	ComponentHost->SetQuery(NewObj);
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_CONTROLPT)) NewObj->Filters->PassControlPt = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_VECTOR)) NewObj->Filters->PassVector = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_DEM)) NewObj->Filters->PassDEM = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_ENABLED)) NewObj->Filters->PassEnabled = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_DISABLED)) NewObj->Filters->PassDisabled = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_LINE)) NewObj->Filters->PassLine = 0;
	if (!(NewQueryFlags & WM_WCSW_LW_NEWQUERY_FLAG_POINT)) NewObj->Filters->PassPoint = 0;
	NewObj->EditRAHost();
	} // if
#endif // WCS_BUILD_VNS

} // LinkWidgetInstance::NewQuery

/*===========================================================================*/

void LinkWidgetInstance::EnableVectors(bool NewState)
{

ComponentHost->EnableVectors(DBHost, NewState);

} // LinkWidgetInstance::EnableVectors

/*===========================================================================*/

void LinkWidgetInstance::SelectVectors(void)
{

ComponentHost->SelectVectors(DBHost);

} // LinkWidgetInstance::SelectVectors

/*===========================================================================*/

void LinkWidgetInstance::DisplayPopup(int ScreenX, int ScreenY)
{
bool IgnoreSoftLink = false;

if ((ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_WAVE) || (ComponentHost->GetRAHostTypeNumber() == WCS_EFFECTSSUBCLASS_CLOUD))
	{
	IgnoreSoftLink = true;
	} // if

if (!ActionPopupMenu)
	{
	ActionPopupMenu = CreatePopupMenu();
	} // if

if (ActionPopupMenu)
	{
	UINT DBOpDisableFlags = (NumJoes ? MF_ENABLED : MF_GRAYED);
	long LWPopResult;
#ifdef WCS_BUILD_VNS
	int Count;
	SearchQuery *MyEffect;
	if (!IgnoreSoftLink)
		{
		UINT QueryDisableFlags = (ComponentHost->Search ? MF_ENABLED : MF_GRAYED);
		HMENU SelectSub = NULL;
		if (SelectSub = CreatePopupMenu())
			{
			Count = 0;
			AppendMenu(SelectSub, MF_STRING, ID_LINKAGE_QUERYBASE + Count++, "New Search Query...");
			for (MyEffect = (SearchQuery *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY); MyEffect; MyEffect = (SearchQuery *)MyEffect->Next)
				{
				AppendMenu(SelectSub, MF_STRING | (MyEffect == ComponentHost->Search? MF_CHECKED : NULL), ID_LINKAGE_QUERYBASE + Count++, MyEffect->GetName());
				} // for
			AppendMenu(ActionPopupMenu, MF_STRING | MF_POPUP, (UINT)SelectSub, "Select Query");
			} // if
		AppendMenu(ActionPopupMenu, MF_STRING | QueryDisableFlags, ID_LINKAGE_EDIT, "Edit Query");
		AppendMenu(ActionPopupMenu, MF_STRING | QueryDisableFlags, ID_LINKAGE_FREE, "Free Query");
		AppendMenu(ActionPopupMenu, MF_STRING | DBOpDisableFlags, ID_LINKAGE_HARD, "Hard Link to Query Vectors");
		} // if
#endif // WCS_BUILD_VNS
	AppendMenu(ActionPopupMenu, MF_STRING | MF_ENABLED, ID_LINKAGE_HARDDBVECS, "Hard Link to selected Vectors");
	AppendMenu(ActionPopupMenu, MF_STRING | DBOpDisableFlags, ID_LINKAGE_SELECTDB, "Select linked vectors in Database");
	AppendMenu(ActionPopupMenu, MF_STRING | DBOpDisableFlags, ID_LINKAGE_ENABLEDB, "Enable linked vectors");
	AppendMenu(ActionPopupMenu, MF_STRING | DBOpDisableFlags, ID_LINKAGE_DISABLEDB, "Disable linked vectors");

	if (LWPopResult = TrackPopupMenu(ActionPopupMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_RIGHTBUTTON, ScreenX, ScreenY, 0, ActionButton, NULL))
		{
		switch (LWPopResult)
			{
			case ID_LINKAGE_HARDDBVECS:
				{
				HardLinkSelectedVectors();
				break;
				} // 
			case ID_LINKAGE_SELECTDB:
				{
				SelectVectors();
				break;
				} // 
			case ID_LINKAGE_ENABLEDB:
				{
				EnableVectors(true);
				break;
				} // 
			case ID_LINKAGE_DISABLEDB:
				{
				EnableVectors(false);
				break;
				} // 
#ifdef WCS_BUILD_VNS
			case ID_LINKAGE_QUERYBASE:
				{ // new query
				NewQuery();
				break;
				} // ID_LINKAGE_QUERYBASE
			case ID_LINKAGE_EDIT:
				{
				EditQuery();
				break;
				} // ID_LINKAGE_EDIT
			case ID_LINKAGE_FREE:
				{
				FreeQuery();
				break;
				} // ID_LINKAGE_FREE
			case ID_LINKAGE_HARD:
				{
				HardLink();
				break;
				} // ID_LINKAGE_HARD
#endif // WCS_BUILD_VNS
			default:
				{ // existing query
#ifdef WCS_BUILD_VNS
				int QueryNum = LWPopResult - ID_LINKAGE_QUERYBASE;
				Count = 1;
				for (MyEffect = (SearchQuery *)EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_SEARCHQUERY); MyEffect; MyEffect = (SearchQuery *)MyEffect->Next)
					{
					if (QueryNum == Count)
						{
						ComponentHost->SetQuery(MyEffect);
						break;
						} // if
					Count++;
					} // for
#endif // WCS_BUILD_VNS
				break;
				} // default: ID_LINKAGE_QUERYnnn
			} // LWPopResult
		} // if

	DestroyMenu(ActionPopupMenu);
	ActionPopupMenu = NULL;
	} // if

} // LinkWidgetInstance::DisplayPopup
