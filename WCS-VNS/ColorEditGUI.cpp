// ColorEditGUI.cpp
// Code for Color editor
// Built from MotEditGUI.cpp on 2/26/96 by Chris "Xenon" Hanson
// Copyright 1996

#include "stdafx.h"
#include "ColorEditGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "Requester.h"
#include "Application.h"
#include "Toolbar.h"
#include "Useful.h"
#include "Interactive.h"
#include "Project.h"
#include "Raster.h"
#include "AppMem.h"
#include "resource.h"
#include "FSSupport.h"

bool ColorEditGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
{

//lint -save -e788 (not all enumerated values are in switch)
switch (AskAbout)
	{
	case WCS_FENETRE_WINCAP_CANUNDO:
		return(true);
	default:
		return(false);
	} // AskAbout
//lint -restore

} // ColorEditGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin ColorEditGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ColorEditGUI::Open

/*===========================================================================*/

NativeGUIWin ColorEditGUI::Construct(void)
{
char *SampleType[] = {"RGB", "Red", "Green", "Blue", "Hue", "Saturation", "Value"};
int TabIndex;

if(!NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_COLOR_EDIT_TEST, LocalWinSys()->RootWin);

	if(NativeWin)
		{
		WidgetSetScrollRange(IDC_SCROLLBAR1, 0, 255);
		WidgetSetScrollRange(IDC_SCROLLBAR2, 0, 255);
		WidgetSetScrollRange(IDC_SCROLLBAR3, 0, 255);
		WidgetSetScrollRange(IDC_SCROLLBAR4, 0, 360);
		WidgetSetScrollRange(IDC_SCROLLBAR5, 0, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR6, 0, 100);
		WidgetSetScrollRange(IDC_SCROLLBAR7, 0, 100);

		for (TabIndex = 0; TabIndex < 7; TabIndex ++)
			{
			WidgetCBInsert(IDC_SAMPLETYPEDROP, TabIndex, SampleType[TabIndex]);
			} // for
		FillSwatchDrop();
		MatchSetColorSwatch();
		SECURITY_INLINE_CHECK(048, 48);
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // ColorEditGUI::Construct

/*===========================================================================*/

ColorEditGUI::ColorEditGUI(AnimColorTime *ActiveSource)
: GUIFenetre('EDCO', this, "Color Editor") // Yes, I know...
{
static NotifyTag AllEvents[] = {0, 
								MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff),
								MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED),
								0};
static NotifyTag AllIntercommonEvents[] = {MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff),
								0};

ConstructError = 0;
Active = ActiveSource;
Configured = 0;
ResponseEnabled = 0;
RespondMode = WCS_COLOREDIT_RESPOND_ONCE;
RespondToWhat = WCS_COLOREDIT_RESPOND_RGB;
NumColorSamples = 0;
Sampling = 0;
SwatchTable = NULL;
SwatchItems = 0;

// for some reason, defining these in the variable declaration gave varying results, sometimes correct, sometimes not
AllEvents[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, 0xff);

HSV[0] = HSV[1] = HSV[2] = 0.0;
LastHSV[0] = LastHSV[1] = LastHSV[2] = -1.0;
GlobalApp->AppEx->RegisterClient(this, AllEvents);
GlobalApp->MainProj->Interactive->RegisterClient(this, AllIntercommonEvents);

Active->RGBtoHSV(HSV);
Active->Copy(&Backup, Active);
SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK);

memset(&HueWidData, 0, sizeof (struct RasterWidgetData));
HueWidData.RDWin = this;
HueWidData.MainRast = new Raster();
HueWidData.OverRast = new Raster();
HueWidData.Visible = 1.0;
memset(&ValueWidData, 0, sizeof (struct RasterWidgetData));
ValueWidData.RDWin = this;
ValueWidData.MainRast = new Raster();
ValueWidData.OverRast = new Raster();
ValueWidData.Visible = 1.0;
memset(&SwatchData, 0, sizeof (struct RasterWidgetData));
SwatchData.RDWin = this;
SwatchData.Visible = 1.0;

if (! HueWidData.MainRast || ! ValueWidData.MainRast || ! HueWidData.OverRast || ! ValueWidData.OverRast)
	ConstructError = 1;
else
	{
	ValueWidData.MainRast->Rows = ValueWidData.MainRast->Cols = 188;
	ValueWidData.MainRast->ByteBands = 3;
	if (ValueWidData.MainRast->AllocByteBand(0))
		{
		ValueWidData.MainRast->ClearByteBand(0);
		if (ValueWidData.MainRast->AllocByteBand(1))
			{
			ValueWidData.MainRast->ClearByteBand(1);
			if (ValueWidData.MainRast->AllocByteBand(2))
				{
				ValueWidData.MainRast->ClearByteBand(2);
				} // if
			else
				ConstructError = 1;
			} // if
		else
			ConstructError = 1;
		} // if
	else
		ConstructError = 1;
	ValueWidData.OverRast->Rows = ValueWidData.OverRast->Cols = 188;
	ValueWidData.OverRast->ByteBands = 3;
	if (ValueWidData.OverRast->AllocByteBand(0))
		{
		ValueWidData.OverRast->ClearByteBand(0);
		if (ValueWidData.OverRast->AllocByteBand(1))
			{
			ValueWidData.OverRast->ClearByteBand(1);
			if (ValueWidData.OverRast->AllocByteBand(2))
				{
				ValueWidData.OverRast->ClearByteBand(2);
				} // if
			else
				ConstructError = 1;
			} // if
		else
			ConstructError = 1;
		} // if
	else
		ConstructError = 1;
	HueWidData.MainRast->Rows = 188;
	HueWidData.MainRast->Cols = 5;
	HueWidData.MainRast->ByteBands = 3;
	if (HueWidData.MainRast->AllocByteBand(0))
		{
		HueWidData.MainRast->ClearByteBand(0);
		if (HueWidData.MainRast->AllocByteBand(1))
			{
			HueWidData.MainRast->ClearByteBand(1);
			if (HueWidData.MainRast->AllocByteBand(2))
				{
				HueWidData.MainRast->ClearByteBand(2);
				} // if
			else
				ConstructError = 1;
			} // if
		else
			ConstructError = 1;
		} // if
	else
		ConstructError = 1;
	HueWidData.OverRast->Rows = 188;
	HueWidData.OverRast->Cols = 5;
	HueWidData.OverRast->ByteBands = 3;
	if (HueWidData.OverRast->AllocByteBand(0))
		{
		HueWidData.OverRast->ClearByteBand(0);
		if (HueWidData.OverRast->AllocByteBand(1))
			{
			HueWidData.OverRast->ClearByteBand(1);
			if (HueWidData.OverRast->AllocByteBand(2))
				{
				HueWidData.OverRast->ClearByteBand(2);
				} // if
			else
				ConstructError = 1;
			} // if
		else
			ConstructError = 1;
		} // if
	else
		ConstructError = 1;
	if (! ConstructError)
		ComputeColorPickerB();
	} // else

} // ColorEditGUI::ColorEditGUI

/*===========================================================================*/

ColorEditGUI::~ColorEditGUI()
{

GlobalApp->AppEx->RemoveClient(this);
GlobalApp->MainProj->Interactive->RemoveClient(this);
GlobalApp->MCP->RemoveWindowFromMenuList(this);

if (HueWidData.MainRast)
	delete HueWidData.MainRast;
if (ValueWidData.MainRast)
	delete ValueWidData.MainRast;
if (HueWidData.OverRast)
	delete HueWidData.OverRast;
if (ValueWidData.OverRast)
	delete ValueWidData.OverRast;
if (SwatchData.MainRast)
	delete SwatchData.MainRast;
if (SwatchTable)
	AppMem_Free(SwatchTable, SwatchItems * 256);
SwatchTable = NULL;

} // ColorEditGUI::~ColorEditGUI()

/*===========================================================================*/

long ColorEditGUI::HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID)
{
short SetRGB = 0;

if(ScrollCode)
	{
	switch (CtrlID)
		{
		case IDC_SCROLLBAR1:
			{
			Active->SetValue(0, (double)ScrollPos / 255.0);
			Active->RGBtoHSV(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR2:
			{
			Active->SetValue(1, (double)ScrollPos / 255.0);
			Active->RGBtoHSV(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR3:
			{
			Active->SetValue(2, (double)ScrollPos / 255.0);
			Active->RGBtoHSV(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR4:
			{
			HSV[0]= (double)ScrollPos;
			Active->HSVtoRGB(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR5:
			{
			HSV[1]= (double)ScrollPos;
			Active->HSVtoRGB(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR6:
			{
			HSV[2]= (double)ScrollPos;
			Active->HSVtoRGB(HSV);
			SetRGB = 1;
			break;
			}
		case IDC_SCROLLBAR7:
			{
			Active->Intensity.SetValue((double)ScrollPos / 100.0);
			Active->Intensity.ValueChanged();
			break;
			}
		} // switch
	if (SetRGB)
		{
		Active->ValueChanged();
		} // if
	return(0);
	} // if
else
	{
	return(5); // default scroll amount
	} // else

} // ColorEditGUI::HandleScroll

/*===========================================================================*/

long ColorEditGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_CEG, 0);

return(0);

} // ColorEditGUI::HandleCloseWin

/*===========================================================================*/

long ColorEditGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(043, 43);
switch(ButtonID)
	{
	case ID_KEEP:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CEG, 0);
		break;
		} // 
	case IDCANCEL:
		{
		Cancel();
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_CEG, 0);
		break;
		} // 
	case IDC_WINUNDO:
		{
		Cancel();
		break;
		} //
	default:
		break;
	} // ButtonID

return(0);

} // ColorEditGUI::HandleButtonClick

/*===========================================================================*/

long ColorEditGUI::HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift)
{

switch (CtrlID)
	{
	case IDC_VALUEPICKER:
		{
		Sampling = 1;
		SetRGBFromSatVal(X / 187.0, Y / 187.0);
		break;
		} // IDC_VALUEPICKER
	case IDC_HUEPICKER:
		{
		Sampling = 1;
		SetRGBFromHue(X / 19.0, Y / 187.0);
		break;
		} // IDC_HUEPICKER
	case IDC_COLORSWATCH:
		{
		Sampling = 1;
		SetRGBFromSwatch(X / 219.0, Y / 50.0);
		break;
		} // IDC_HUEPICKER
	default:
		break;
	} // switch CtrlID

return (0);

} // ColorEditGUI::HandleLeftButtonDown

/*===========================================================================*/

long ColorEditGUI::HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift)
{

Sampling = 0;

return (0);

} // ColorEditGUI::HandleLeftButtonUp

/*===========================================================================*/

long ColorEditGUI::HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right)
{

if(!Left) Sampling = 0; // in case we missed the mouseup

switch (CtrlID)
	{
	case IDC_VALUEPICKER:
		{
		if (Sampling)
			SetRGBFromSatVal(X / 187.0, Y / 187.0);
		break;
		} // IDC_VALUEPICKER
	case IDC_HUEPICKER:
		{
		if (Sampling)
			SetRGBFromHue(X / 19.0, Y / 187.0);
		break;
		} // IDC_HUEPICKER
	case IDC_COLORSWATCH:
		{
		if (Sampling)
			SetRGBFromSwatch(X / 219.0, Y / 50.0);
		break;
		} // IDC_COLORSWATCH
	default:
		break;
	} // switch CtrlID

return (0);

} // ColorEditGUI::HandleMouseMove

/*===========================================================================*/

long ColorEditGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

DisableWidgets();
NumColorSamples = 0;

return(0);

} // ColorEditGUI::HandleSCChange

/*===========================================================================*/

long ColorEditGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

NumColorSamples = 0;

return(0);

} // ColorEditGUI::HandleSRChange

/*===========================================================================*/

long ColorEditGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_SAMPLETYPEDROP:
		{
		SelectSampleType();
		break;
		} // IDC_SAMPLETYPEDROP
	case IDC_SWATCHDROP:
		{
		SelectColorSwatch();
		break;
		} // IDC_SWATCHDROP
	default:
		break;
	} // switch CtrlID

return (0);

} // ColorEditGUI::HandleCBChange

/*===========================================================================*/

long ColorEditGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch(CtrlID)
	{
	case IDC_COLH:
	case IDC_COLS:
	case IDC_COLV:
		{
		HSVTwiddle();
		break;
		} // HSV
	case IDC_COLRED:
		{
		Active->SetCurValue(0, RGB[0] / 255.0);
		RGBTwiddle();
		break;
		} // red
	case IDC_COLGRN:
		{
		Active->SetCurValue(1, RGB[1] / 255.0);
		RGBTwiddle();
		break;
		} // red
	case IDC_COLBLU:
		{
		Active->SetCurValue(2, RGB[2] / 255.0);
		RGBTwiddle();
		break;
		} // red
	default:
		break;
	} // ID

return(0);

} // ColorEditGUI::HandleFIChange

/*===========================================================================*/

void ColorEditGUI::HandleNotifyEvent(void)
{
NotifyTag *Changes, Changed, Interested[10];

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

if (ResponseEnabled)
	{
	Interested[0] = MAKE_ID(WCS_NOTIFYCLASS_DIAGNOSTICDATA, WCS_SUBCLASS_DIAGNOSTIC_DATA, 0xff, 0xff);
	Interested[1] = NULL;
	if ((Changed = GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0)) && Activity->ChangeNotify->NotifyData)
		{
		if (((DiagnosticData *)Activity->ChangeNotify->NotifyData)->ValueValid[WCS_DIAGNOSTIC_RGB])
			RespondColorNotify((DiagnosticData *)Activity->ChangeNotify->NotifyData);
		} // if
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), Active->GetNotifySubclass(), 0xff, 0xff);
Interested[1] = MAKE_ID(Active->Intensity.GetNotifyClass(), Active->Intensity.GetNotifySubclass(), 0xff, 0xff);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	ConfigureWidgets();
	} // if

Interested[0] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
Interested[1] = MAKE_ID(WCS_INTERCLASS_TIME, 0xff, 0xff, 0xff);
Interested[2] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	Active->RGBtoHSV(HSV);
	ConfigureWidgets();
	} // if

Interested[1] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[2] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMDOUBLETIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[3] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
Interested[4] = MAKE_ID(Active->GetNotifyClass(), WCS_SUBCLASS_ANIMCOLORTIME, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
Interested[5] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_COUNTCHANGED);
Interested[6] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED); 
Interested[7] = MAKE_ID(Active->GetNotifyClass(), 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED); 
Interested[8] = MAKE_ID(WCS_EFFECTSSUBCLASS_THEMATICMAP, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_ENABLEDCHANGED); 
Interested[9] = NULL;

if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	{
	SyncWidgets();
	} // if

} // ColorEditGUI::HandleNotifyEvent()

/*===========================================================================*/

void ColorEditGUI::ConfigureWidgets(void)
{
unsigned char Red, Grn, Blu;

// color notify response
ConfigureSC(NativeWin, IDC_CHECKRESPONSEENABLED, &ResponseEnabled, SCFlag_Char, NULL, 0);

if(ResponseEnabled)
	{
	ConfigureSR(NativeWin, IDC_RADIOMRESPONDONCE, IDC_RADIOMRESPONDONCE, &RespondMode, SRFlag_Char, WCS_COLOREDIT_RESPOND_ONCE, NULL, NULL);
	ConfigureSR(NativeWin, IDC_RADIOMRESPONDONCE, IDC_RADIORESPONDMANY, &RespondMode, SRFlag_Char, WCS_COLOREDIT_RESPOND_MANY, NULL, NULL);
	} // if

WidgetCBSetCurSel(IDC_SAMPLETYPEDROP, RespondToWhat);

// Color
RGB[0] = Active->CurValue[0] * 255.0;
RGB[1] = Active->CurValue[1] * 255.0;
RGB[2] = Active->CurValue[2] * 255.0;

ConfigureFI(NativeWin, IDC_COLRED,
 &RGB[0],
  1.0,
   0.0,
    255.0,
     FIOFlag_Double,
      NULL,
       NULL);

ConfigureFI(NativeWin, IDC_COLGRN,
 &RGB[1],
  1.0,
   0.0,
    255.0,
     FIOFlag_Double,
      NULL,
       NULL);

ConfigureFI(NativeWin, IDC_COLBLU,
 &RGB[2],
  1.0,
   0.0,
    255.0,
     FIOFlag_Double,
      NULL,
       NULL);

ConfigureFI(NativeWin, IDC_COLH,
 &HSV[0],
  5.0,
   0.0,
    360.0,
     FIOFlag_Double);

ConfigureFI(NativeWin, IDC_COLS,
 &HSV[1],
  5.0,
   0.0,
    100.0,
     FIOFlag_Double);

ConfigureFI(NativeWin, IDC_COLV,
 &HSV[2],
  5.0,
   0.0,
    100.0,
     FIOFlag_Double);

Red = (unsigned char)(Active->GetCurValue(0) * 255);
Grn = (unsigned char)(Active->GetCurValue(1) * 255);
Blu = (unsigned char)(Active->GetCurValue(2) * 255);
if (! Configured)
	{
	SetColorPot(0, Red, Grn, Blu, 0);
	ConfigureCB(NativeWin, ID_COLORPOT1, 100, CBFlag_CustomColor, 0);	// 100 percentage filled, ..., color pot
	} // if
SetColorPot(1, Red, Grn, Blu, 1);
ConfigureCB(NativeWin, ID_COLORPOT2, 100, CBFlag_CustomColor, 1);	// 100 percentage filled, ..., color pot

WidgetSetScrollPos(IDC_SCROLLBAR1, (short)(Active->GetCurValue(0) * 255));
WidgetSetScrollPos(IDC_SCROLLBAR2, (short)(Active->GetCurValue(1) * 255));
WidgetSetScrollPos(IDC_SCROLLBAR3, (short)(Active->GetCurValue(2) * 255));
WidgetSetScrollPos(IDC_SCROLLBAR4, (short)HSV[0]);
WidgetSetScrollPos(IDC_SCROLLBAR5, (short)HSV[1]);
WidgetSetScrollPos(IDC_SCROLLBAR6, (short)HSV[2]);

// Intensity

WidgetSNConfig(IDC_INTENSITY, &Active->Intensity);

Red = (unsigned char)(Active->GetClampedCompleteValue(0) * 255);
Grn = (unsigned char)(Active->GetClampedCompleteValue(1) * 255);
Blu = (unsigned char)(Active->GetClampedCompleteValue(2) * 255);
if (! Configured)
	{
	SetColorPot(2, Red, Grn, Blu, 0);
	ConfigureCB(NativeWin, ID_COLORPOT3, 100, CBFlag_CustomColor, 2);	// 100 percentage filled, ..., color pot
	} // if
SetColorPot(3, Red, Grn, Blu, 1);
ConfigureCB(NativeWin, ID_COLORPOT4, 100, CBFlag_CustomColor, 3);	// 100 percentage filled, ..., color pot

WidgetSetScrollPos(IDC_SCROLLBAR7, (short)(Active->GetIntensity() * 100));

if (SatValChanged)
	ComputeOverlayA();
if (HueChanged)
	{
	ComputeColorPickerA();
	ComputeOverlayB();
	ConfigureRD(NativeWin, IDC_HUEPICKER, &HueWidData);
	} // if
ConfigureRD(NativeWin, IDC_VALUEPICKER, &ValueWidData);
LastHSV[0] = HSV[0];
LastHSV[1] = HSV[1];
LastHSV[2] = HSV[2];

WidgetSmartRAHConfig(IDC_IMAGEKEYFRAME, Active, Active->RAParent);
WidgetSmartRAHConfig(IDC_IMAGEKEYFRAME2, &Active->Intensity, Active);

DisableWidgets();
Configured = 1;

} // ColorEditGUI::ConfigureWidgets()

/*===========================================================================*/

void ColorEditGUI::SyncWidgets(void)
{

WidgetSNSync(IDC_IMAGEKEYFRAME, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_IMAGEKEYFRAME2, WP_FISYNC_NONOTIFY);

} // ColorEditGUI::SyncWidgets

/*===========================================================================*/

void ColorEditGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOMRESPONDONCE, ! ResponseEnabled);
WidgetSetDisabled(IDC_RADIORESPONDMANY, ! ResponseEnabled);
WidgetSetDisabled(IDC_SAMPLETYPEDROP, ! ResponseEnabled);

} // ColorEditGUI::DisableWidgets

/*===========================================================================*/

void ColorEditGUI::HSVTwiddle(void)
{

Active->HSVtoRGB(HSV);
Active->ValueChanged();
RasterAnimHost::SetActiveRAHost(Active);
// <<<>>> if Record Mode, add node

} //ColorEditGUI::HSVTwiddle()

/*===========================================================================*/

void ColorEditGUI::RGBTwiddle(void)
{

Active->RGBtoHSV(HSV);
Active->ValueChanged();
// <<<>>> if Record Mode, add node

} //ColorEditGUI::RGBTwiddle()

/*===========================================================================*/

void ColorEditGUI::Cancel(void)
{

Active->Copy(Active, &Backup);
Active->ObjectChanged();

} // ColorEditGUI::Cancel()

/*===========================================================================*/

void ColorEditGUI::SelectSampleType(void)
{

RespondToWhat = (char)WidgetCBGetCurSel(IDC_SAMPLETYPEDROP);

NumColorSamples = 0;

} // ColorEditGUI::SelectSampleType

/*===========================================================================*/

void ColorEditGUI::RespondColorNotify(DiagnosticData *Data)
{
AnimColorTime Temp;
double TempHSV[3];

if (Data->DataRGB[0] || Data->DataRGB[1] || Data->DataRGB[2])
	{
	if (RespondMode == WCS_COLOREDIT_RESPOND_ONCE)
		{
		switch (RespondToWhat)
			{
			case WCS_COLOREDIT_RESPOND_RGB:
				{
				Active->SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_RED:
				{
				Active->SetValue(0, Data->DataRGB[0] / 255.0);
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_GRN:
				{
				Active->SetValue(1, Data->DataRGB[1] / 255.0);
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_BLU:
				{
				Active->SetValue(2, Data->DataRGB[2] / 255.0);
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_HUE:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[0] = TempHSV[0];
				Active->HSVtoRGB(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_SAT:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[1] = TempHSV[1];
				Active->HSVtoRGB(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_VAL:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[2] = TempHSV[2];
				Active->HSVtoRGB(HSV);
				break;
				}
			default:
				break;
			} // switch
		} // if
	else
		{
		Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
		switch (RespondToWhat)
			{
			case WCS_COLOREDIT_RESPOND_RGB:
				{
				Active->SetValue3((Active->GetCurValue(0) * NumColorSamples + Temp.GetCurValue(0)) / (NumColorSamples + 1),
					(Active->GetCurValue(1) * NumColorSamples + Temp.GetCurValue(1)) / (NumColorSamples + 1),
					(Active->GetCurValue(2) * NumColorSamples + Temp.GetCurValue(2)) / (NumColorSamples + 1));
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_RED:
				{
				Active->SetValue(0, (Active->GetCurValue(0) * NumColorSamples + Temp.GetCurValue(0)) / (NumColorSamples + 1));
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_GRN:
				{
				Active->SetValue(1, (Active->GetCurValue(1) * NumColorSamples + Temp.GetCurValue(1)) / (NumColorSamples + 1));
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_BLU:
				{
				Active->SetValue(2, (Active->GetCurValue(2) * NumColorSamples + Temp.GetCurValue(2)) / (NumColorSamples + 1));
				Active->RGBtoHSV(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_HUE:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[0] = (HSV[0] * NumColorSamples + TempHSV[0]) / (NumColorSamples + 1);
				Active->HSVtoRGB(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_SAT:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[1] = (HSV[1] * NumColorSamples + TempHSV[1]) / (NumColorSamples + 1);
				Active->HSVtoRGB(HSV);
				break;
				}
			case WCS_COLOREDIT_RESPOND_VAL:
				{
				Temp.SetValue3(Data->DataRGB[0] / 255.0, Data->DataRGB[1] / 255.0, Data->DataRGB[2] / 255.0);
				Temp.RGBtoHSV(TempHSV);
				HSV[2] = (HSV[2] * NumColorSamples + TempHSV[2]) / (NumColorSamples + 1);
				Active->HSVtoRGB(HSV);
				break;
				}
			default:
				break;
			} // switch
		NumColorSamples ++;
		} // if

	Active->ValueChanged();
	} // if

} // ColorEditGUI::RespondColorNotify

/*===========================================================================*/

void ColorEditGUI::ComputeColorPickerA(void)
{
double TempRGB[3], TempHSV[3], SatInc, ValInc;
unsigned char *Red, *Green, *Blue;
long Rows, Cols, X, Y, Zip;

Rows = ValueWidData.MainRast->Rows;
Cols = ValueWidData.MainRast->Cols;
Red = ValueWidData.MainRast->ByteMap[0];
Green = ValueWidData.MainRast->ByteMap[1];
Blue = ValueWidData.MainRast->ByteMap[2];

TempHSV[0] = HSV[0];
TempHSV[1] = 0.0;
TempHSV[2] = 100.0;
SatInc = 100.0 / (Cols - 1);
ValInc = 100.0 / (Rows - 1);
for (Y = Zip = 0; Y < Rows; Y ++, TempHSV[2] -= ValInc)
	{
	if (TempHSV[2] < 0.0)
		TempHSV[2] = 0.0;
	TempHSV[1] = 0.0;
	for (X = 0; X < Cols; X ++, Zip ++, TempHSV[1] += SatInc)
		{
		HSVtoRGB(TempHSV, TempRGB);
		Red[Zip] = (unsigned char)(TempRGB[0] * 255);
		Green[Zip] = (unsigned char)(TempRGB[1] * 255);
		Blue[Zip] = (unsigned char)(TempRGB[2] * 255);
		} // for
	} // for 

} // ColorEditGUI::ComputeColorPickerA

/*===========================================================================*/

void ColorEditGUI::ComputeColorPickerB(void)
{
double TempRGB[3], TempHSV[3], HueInc;
unsigned char *Red, *Green, *Blue;
long Rows, Cols, X, Y, Zip;

Rows = HueWidData.MainRast->Rows;
Cols = HueWidData.MainRast->Cols;
Red = HueWidData.MainRast->ByteMap[0];
Green = HueWidData.MainRast->ByteMap[1];
Blue = HueWidData.MainRast->ByteMap[2];

TempHSV[0] = 0.0;
TempHSV[1] = 100.0;
#ifdef WCS_COLORMODEL_HSL
TempHSV[2] = 50.0;
#else // WCS_COLORMODEL_HSL
TempHSV[2] = 100.0;
#endif // WCS_COLORMODEL_HSL
HueInc = 360.0 / (Rows - 1);
for (Y = Zip = 0; Y < Rows; Y ++, TempHSV[0] += HueInc)
	{
	HSVtoRGB(TempHSV, TempRGB);
	for (X = 0; X < Cols; X ++, Zip ++)
		{
		Red[Zip] = (unsigned char)(TempRGB[0] * 255);
		Green[Zip] = (unsigned char)(TempRGB[1] * 255);
		Blue[Zip] = (unsigned char)(TempRGB[2] * 255);
		} // for
	} // for 

} // ColorEditGUI::ComputeColorPickerB

/*===========================================================================*/

void ColorEditGUI::ComputeOverlayA(void)
{
double SatInc, ValInc;
unsigned char *Red, *Green, *Blue;
long Rows, Cols, X, Y, i, j, FillX, FillY, Zip;

Rows = ValueWidData.OverRast->Rows;
Cols = ValueWidData.OverRast->Cols;
Red = ValueWidData.OverRast->ByteMap[0];
Green = ValueWidData.OverRast->ByteMap[1];
Blue = ValueWidData.OverRast->ByteMap[2];

ValueWidData.OverRast->ClearByteBand(0);
ValueWidData.OverRast->ClearByteBand(1);
ValueWidData.OverRast->ClearByteBand(2);

SatInc = 100.0 / (Cols - 1);
ValInc = 100.0 / (Rows - 1);

Y = (long)((100.0 - HSV[2]) / ValInc);
if (Y < 0)
	Y = 0;
X = (long)(HSV[1] / SatInc);

for (i = -3; i <= 3; i ++)
	{
	FillY = Y + i;
	if (FillY > 0 && FillY < Rows)
		{
		for (j = -3; j <= 3; j ++)
			{
			FillX = X + j;
			if (FillX > 0 && FillX < Cols)
				{
				Zip = FillY * Cols + FillX;
				Red[Zip] = (unsigned char)(255);
				Green[Zip] = (unsigned char)(255);
				Blue[Zip] = (unsigned char)(255);
				} // if
			} // for
		} // if
	} // for
for (i = -1; i <= 1; i ++)
	{
	FillY = Y + i;
	if (FillY > 0 && FillY < Rows)
		{
		for (j = -1; j <= 1; j ++)
			{
			FillX = X + j;
			if (FillX > 0 && FillX < Cols)
				{
				Zip = FillY * Cols + FillX;
				Red[Zip] = (unsigned char)(1);
				Green[Zip] = (unsigned char)(0);
				Blue[Zip] = (unsigned char)(0);
				} // if
			} // for
		} // if
	} // for

} // ColorEditGUI::ComputeOverlayA

/*===========================================================================*/

void ColorEditGUI::ComputeOverlayB(void)
{
double HueInc;
unsigned char *Red, *Green, *Blue;
long Rows, Cols, X, Y, Zip;

Rows = HueWidData.OverRast->Rows;
Cols = HueWidData.OverRast->Cols;
Red = HueWidData.OverRast->ByteMap[0];
Green = HueWidData.OverRast->ByteMap[1];
Blue = HueWidData.OverRast->ByteMap[2];

HueWidData.OverRast->ClearByteBand(0);
HueWidData.OverRast->ClearByteBand(1);
HueWidData.OverRast->ClearByteBand(2);

HueInc = 360.0 / (Rows - 1);

Y = (long)(HSV[0] / HueInc);

if (Y > 0)
	{
	Zip = (Y - 1) * Cols;
	for (X = 0; X < Cols; X ++, Zip ++)
		{
		Red[Zip] = (unsigned char)(1);
		Green[Zip] = (unsigned char)(0);
		Blue[Zip] = (unsigned char)(0);
		} // for
	} // if
Zip = Y * Cols;
for (X = 0; X < Cols; X ++, Zip ++)
	{
	Red[Zip] = (unsigned char)(1);
	Green[Zip] = (unsigned char)(0);
	Blue[Zip] = (unsigned char)(0);
	} // for

} // ColorEditGUI::ComputeOverlayB

/*===========================================================================*/

void ColorEditGUI::SetRGBFromHue(double X, double Y)
{

HSV[0] = Y * 360.0;
if (HSV[0] < 0.0)
	HSV[0] = 0.0;
if (HSV[0] > 360.0)
	HSV[0] = 360.0;
Active->HSVtoRGB(HSV);
Active->ValueChanged();

} // ColorEditGUI::SetRGBFromHue

/*===========================================================================*/

void ColorEditGUI::SetRGBFromSatVal(double X, double Y)
{

HSV[1] = X * 100.0;
if (HSV[1] < 0.0)
	HSV[1] = 0.0;
if (HSV[2] > 100.0)
	HSV[2] = 100.0;
HSV[2] = (1.0 - Y) * 100.0;
if (HSV[2] < 0.0)
	HSV[2] = 0.0;
if (HSV[2] > 100.0)
	HSV[2] = 100.0;
Active->HSVtoRGB(HSV);
Active->ValueChanged();

} // ColorEditGUI::SetRGBFromSatVal

/*===========================================================================*/

void ColorEditGUI::SetRGBFromSwatch(double X, double Y)
{
long SampleX, SampleY, Zip;

if (SwatchData.MainRast && SwatchData.MainRast->BandValid(SwatchData.MainRast->ByteMap[0]) && SwatchData.MainRast->BandValid(SwatchData.MainRast->ByteMap[1]) && SwatchData.MainRast->BandValid(SwatchData.MainRast->ByteMap[2]))
	{
	SampleX = (long)(X * SwatchData.MainRast->Cols);
	SampleY = (long)(Y * SwatchData.MainRast->Rows);
	Zip = SampleY * SwatchData.MainRast->Cols + SampleX;
	if (SampleX >= 0 && SampleY >= 0 && SampleX < SwatchData.MainRast->Cols && SampleY < SwatchData.MainRast->Rows)
		{
		RGB[0] = SwatchData.MainRast->ByteMap[0][Zip] / 255.0;
		RGB[1] = SwatchData.MainRast->ByteMap[1][Zip] / 255.0;
		RGB[2] = SwatchData.MainRast->ByteMap[2][Zip] / 255.0;
		Active->SetValue3(RGB[0], RGB[1], RGB[2]);
		Active->RGBtoHSV(HSV);
		Active->ValueChanged();
		} // if
	} // if

} // ColorEditGUI::SetRGBFromSwatch

/*===========================================================================*/

void ColorEditGUI::FillSwatchDrop(void)
{
long Ct;
WIN32_FIND_DATA FileData;
HANDLE Hand;
char NameStr[256];

WidgetCBClear(IDC_SWATCHDROP);
if (SwatchTable)
	AppMem_Free(SwatchTable, SwatchItems * 256);
SwatchTable = NULL;

strcpy(NameStr, "WCSContent:Image/Color Swatches/*.*");
strcpy(NameStr, GlobalApp->MainProj->MungPath(NameStr));

SwatchItems = 0;

if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
	{
	if (FileData.cFileName[0] != '.')
		SwatchItems ++;
	while (FindNextFile(Hand, &FileData))
		{
		if (FileData.cFileName[0] != '.')
			SwatchItems ++;
		} // while
	FindClose(Hand);
	} // if

if (SwatchItems > 0)
	{
	// build table of names
	if (SwatchTable = (char *)AppMem_Alloc(SwatchItems * 256, APPMEM_CLEAR))
		{
		Ct = 0;
		if ((Hand = FindFirstFile(NameStr, &FileData)) != INVALID_HANDLE_VALUE)
			{
			if (FileData.cFileName[0] != '.')
				{
				strcpy(&SwatchTable[Ct * 256], FileData.cFileName);
				Ct ++;
				} // if
			while (FindNextFile(Hand, &FileData))
				{
				if (FileData.cFileName[0] != '.')
					{
					strcpy(&SwatchTable[Ct * 256], FileData.cFileName);
					Ct ++;
					} // if
				} // while
			FindClose(Hand);
			} // if
		for (Ct = 0; Ct < SwatchItems; Ct ++)
			{
			strcpy(NameStr, &SwatchTable[Ct * 256]);
			StripExtension(NameStr);
			WidgetCBInsert(IDC_SWATCHDROP, Ct, NameStr);
			} // for
		} // if table
	} // if at least one entry

WidgetCBSetCurSel(IDC_SWATCHDROP, -1);

} // ColorEditGUI::FillSwatchDrop

/*===========================================================================*/

void ColorEditGUI::MatchSetColorSwatch()
{
long Ct;

for (Ct = 0; Ct < SwatchItems; Ct ++)
	{
	if (! stricmp(&SwatchTable[Ct * 256], GlobalApp->MainProj->Prefs.LastColorSwatch))
		{
		WidgetCBSetCurSel(IDC_SWATCHDROP, Ct);
		SelectColorSwatch();
		return;
		} // if
	} // for

ConfigureRD(NativeWin, IDC_COLORSWATCH, NULL);

} // ColorEditGUI::MatchSetColorSwatch

/*===========================================================================*/

void ColorEditGUI::StoreLastColorSwatch(char *Name)
{

strcpy(GlobalApp->MainProj->Prefs.LastColorSwatch, Name);

} // ColorEditGUI::StoreLastColorSwatch

/*===========================================================================*/

void ColorEditGUI::SelectColorSwatch()
{
long Current;
char FileName[256];

if ((Current = WidgetCBGetCurSel(IDC_SWATCHDROP)) != CB_ERR && Current < SwatchItems)
	{
	strmfp(FileName, "WCSContent:Image/Color Swatches", &SwatchTable[Current * 256]);
	} // if
else
	return;

if (SwatchData.MainRast || (SwatchData.MainRast = new Raster()))
	{
	if (SwatchData.MainRast->LoadnProcessImage(0, FileName))
		{
		ConfigureRD(NativeWin, IDC_COLORSWATCH, &SwatchData);
		StoreLastColorSwatch(&SwatchTable[Current * 256]);
		} // if
	else
		{
		ConfigureRD(NativeWin, IDC_COLORSWATCH, NULL);
		} // else
	} // if
else
	{
	ConfigureRD(NativeWin, IDC_COLORSWATCH, NULL);
	} // else

} // ColorEditGUI::SelectColorSwatch
