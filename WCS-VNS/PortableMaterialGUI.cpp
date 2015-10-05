// PortableMaterialGUI.cpp
// Header file for material controls of numerous component editors
// Created from PortableFoliageGUI.cpp on 4/17/08 CXH
// Copyright 2008 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "Application.h"
#include "Fenetre.h"
#include "PortableMaterialGUI.h"
#include "WCSWidgets.h"
#include "Notify.h"
#include "GraphData.h"
#include "EffectsLib.h"
#include "Ecotype.h"
#include "Raster.h"
#include "resource.h"
#include "Useful.h"
#include "Requester.h"
#include "AppMem.h"
#include "Joe.h"
#include "Log.h"

extern WCSApp *GlobalApp;

// ranges of valid controls for multiple instances of editor
static int ControlRangeLow[] = {IDC_POPDROP_MATNAME1, IDC_POPDROP_MATNAME2};
static int ControlRangeHigh[] = {IDC_POPDROP_COLOR1, IDC_POPDROP_COLOR2};
static int MaterialTemplateByOrdinals[] = {IDD_POPDROP_MATERIAL1, IDD_POPDROP_MATERIAL2};
static int ColorTemplateByOrdinals[] = {IDD_POPDROP_COLOR1, IDD_POPDROP_COLOR2};
// n is base control ID, m is Ordinal
#define WCS_PORTABLEMATERIALGUI_OFFSET_ID(n, m) (n + ((IDC_POPDROP_MATNAME2 - IDC_POPDROP_MATNAME1) * m))

PortableMaterialGUI::PortableMaterialGUI(int InitOrdinal, GUIFenetre *FenetreSource, EffectsLib *EffectsSource,  
	GeneralEffect *ActiveSource, AnimGradient *GradSource, int AnimParDriverSubscriptSource, int TexSubscriptSource)
{
Setup(InitOrdinal, FenetreSource, EffectsSource, ActiveSource, GradSource, AnimParDriverSubscriptSource, TexSubscriptSource);

ConstructError = 0;
NotifyFunctor = NULL;
Panel = SecondPanel = 0;

DummyValue0 = 0.0;
DummyValue100 = 100.0;

} // PortableMaterialGUI::PortableMaterialGUI

/*===========================================================================*/

void PortableMaterialGUI::Setup(int InitOrdinal, GUIFenetre *FenetreSource, EffectsLib *EffectsSource,  
	GeneralEffect *ActiveSource, AnimGradient *GradSource, int AnimParDriverSubscriptSource, int TexSubscriptSource)
{
Ordinal = InitOrdinal;
FenetreHost = FenetreSource;
EffectsHost = EffectsSource;
Grad = GradSource;
Active = ActiveSource;
AnimParDriverSubscript = AnimParDriverSubscriptSource;
TexSubscript = TexSubscriptSource;

} // PortableMaterialGUI::Setup

/*===========================================================================*/

NativeGUIWin PortableMaterialGUI::Construct(int PanelSource, int SecondPanelSource)
{
char *BlendStyles[] = {"Sharp Edge", "Soft Edge", "Quarter Blend", "Half Blend", "Full Blend", "Fast Increase", "Slow Increase", "S-Curve"};
NativeGUIWin Shell;
unsigned long TemplateID, ShellTemplateID;

if (Grad && Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
	{
	TemplateID = MaterialTemplateByOrdinals[Ordinal];
	ShellTemplateID = IDD_POPDROP_SHELL;
	} // if
else
	{ // color
	TemplateID = ColorTemplateByOrdinals[Ordinal];
	ShellTemplateID = IDD_POPDROP_SHELL_SM;
	} // else


Panel = PanelSource;
SecondPanel = SecondPanelSource;

Shell = FenetreHost->CreateSubWinFromTemplate(ShellTemplateID, Panel, 0); // create the shell, to get the dropshadow
FenetreHost->SetPanel(SecondPanel, 0, FenetreHost->CreateWinFromTemplate(TemplateID, Shell, true)); // create the contents to get the proper dialog theme background

for (int TabIndex = 0; TabIndex < 8; TabIndex ++)
	FenetreHost->WidgetCBInsert(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), -1, BlendStyles[TabIndex]);

return(Shell);
} // PortableMaterialGUI::Construct

/*===========================================================================*/



PortableMaterialGUI::~PortableMaterialGUI()
{
	
} // PortableMaterialGUI::~PortableMaterialGUI

/*===========================================================================*/

long PortableMaterialGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

// filter by ordinal
if (!(ButtonID >= ControlRangeLow[Ordinal]	&& ButtonID <= ControlRangeHigh[Ordinal]))
	return(0);

switch(ButtonID)
	{
	case IDC_POPDROP_ADDMAT1:
	case IDC_POPDROP_ADDMAT2:
		{
		AddMatNode();
		return(1);
		} // IDC_POPDROP_ADDMAT1
	case IDC_POPDROP_REMOVEMAT1:
	case IDC_POPDROP_REMOVEMAT2:
		{
		RemoveMatNode();
		return(1);
		} // IDC_POPDROP_REMOVEMAT1
	default:
		break;
	} // ButtonID

return(0);

} // PortableMaterialGUI::HandleButtonClick

/*===========================================================================*/

long PortableMaterialGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// filter by ordinal
if (!(CtrlID >= ControlRangeLow[Ordinal]	&& CtrlID <= ControlRangeHigh[Ordinal]))
	return(0);

switch (CtrlID)
	{
	case IDC_POPDROP_BLENDSTYLE1:
	case IDC_POPDROP_BLENDSTYLE2:
		{
		BlendStyle();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // PortableMaterialGUI::HandleCBChange

/*===========================================================================*/


long PortableMaterialGUI::HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// filter by ordinal
if (!(CtrlID >= ControlRangeLow[Ordinal]	&& CtrlID <= ControlRangeHigh[Ordinal]))
	return(0);

switch (CtrlID)
	{
	case IDC_POPDROP_MATNAME1:
	case IDC_POPDROP_MATNAME2:
		{
		MatName(CtrlID);
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return (0);

} // PortableMaterialGUI::HandleStringLoseFocus

/*===========================================================================*/

long PortableMaterialGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

// filter by ordinal
if (!(CtrlID >= ControlRangeLow[Ordinal]	&& CtrlID <= ControlRangeHigh[Ordinal]))
	return(0);

switch (CtrlID)
	{
	case IDC_POPDROP_GRADIENTPOS1:
	case IDC_POPDROP_GRADIENTPOS2:
		{
		if (Grad) Grad->CertifyNodePosition(Grad->GetActiveNode());
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // PortableMaterialGUI::HandleFIChange

/*===========================================================================*/

void PortableMaterialGUI::ConfigureWidgets(void)
{

FenetreHost->WidgetTBConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_ADDMAT1, Ordinal), IDI_ADDSOMETHING, NULL);
FenetreHost->WidgetTBConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_REMOVEMAT1, Ordinal), IDI_DELETE, NULL);
FenetreHost->WidgetAGConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_ANIMGRADIENT1, Ordinal), Grad);

DisableWidgets(); // handle Grad==NULL disabling

if (!Grad) return;

if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
	{
	FenetreHost->WidgetSmartRAHConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATDRIVER1, Ordinal), Active->GetAnimPtr(AnimParDriverSubscript), Active);
	FenetreHost->WidgetShow(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATDRIVER1, Ordinal), true);
	FenetreHost->WidgetShow(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal), true);
	FenetreHost->WidgetShow(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal), true);

	// materials section

	// gradient range for material
	// these widgets won't do anything useful unless there is a terrain param texture for the material driver
	Texture *TempTex;
	if (Active->GetTexRootPtr(TexSubscript) && Active->GetTexRootPtr(TexSubscript)->Enabled
		&& (TempTex = Active->GetTexRootPtr(TexSubscript)->Tex)
		&& TempTex->Enabled && TempTex->GetTexType() == WCS_TEXTURE_TYPE_TERRAINPARAM)
		{
		FenetreHost->WidgetSNConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal), &TempTex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTLOW]);
		FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal), 0);

		FenetreHost->WidgetSNConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal), &TempTex->TexParam[WCS_TEXTURE_TERRAINPARAM_INPUTHIGH]);
		FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal), 0);
		} // if
	else
		{
		FenetreHost->WidgetFIConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal),
		 &DummyValue0,
		  0.0,
		   0.0,
			0.0,
			 FIOFlag_Double,
			  NULL,
			   NULL);
		FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal), 1);

		FenetreHost->WidgetFIConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal),
		 &DummyValue100,
		  0.0,
		   100.0,
			100.0,
			 FIOFlag_Double,
			  NULL,
			   NULL);
		FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal), 1);
		} // else

	} // if
else
	{ // nothing to do at this time
	} // else

} // PortableMaterialGUI::ConfigureWidgets

/*===========================================================================*/

void PortableMaterialGUI::SyncWidgets(void)
{
if (!Grad) return;

FenetreHost->WidgetAGSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_ANIMGRADIENT1, Ordinal));

// this code is only applicable to Material Gradients
if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
	{
	FenetreHost->WidgetSNSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATDRIVER1, Ordinal), WP_FISYNC_NONOTIFY);
	if (Active->GetTexRootPtr(TexSubscript) && Active->GetTexRootPtr(TexSubscript)->Enabled
		&& (Active->GetTexRootPtr(TexSubscript)->Tex)
		&& Active->GetTexRootPtr(TexSubscript)->Tex->GetTexType() == WCS_TEXTURE_TYPE_TERRAINPARAM)
		{
		FenetreHost->WidgetSNSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADLOW1, Ordinal), WP_FISYNC_NONOTIFY);
		FenetreHost->WidgetSNSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATGRADHIGH1, Ordinal), WP_FISYNC_NONOTIFY);
		} // if
	} // if

if (ActiveGrad = Grad->GetActiveNode()) 
	{
	FenetreHost->WidgetCBSetCurSel(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), ActiveGrad->BlendStyle);
	FenetreHost->WidgetSNSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_GRADIENTPOS1, Ordinal), WP_FISYNC_NONOTIFY);
	if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT)
		FenetreHost->WidgetSNSync(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_COLOR1, Ordinal), WP_FISYNC_NONOTIFY);
	} // if


} // PortableMaterialGUI::SyncWidgets

/*===========================================================================*/

void PortableMaterialGUI::DisableWidgets(void)
{
// gradient
// determine if it's part of an ecosystem, and if so, check if it's transparent
if (Active && Active->GetRAHostTypeNumber() == WCS_JOE_ATTRIB_INTERNAL_ECOSYSTEM) // from Joe.h
	{ // only EcosystemEffect has the "Transparent" setting.
	FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATDRIVER1, Ordinal), (char)(((EcosystemEffect *)Active)->Transparent));
	} // if

// material
if (Grad)
	{
	ActiveGrad = Grad->GetActiveNode(); // get it up to date
	} // if
else
	{
	ActiveGrad = NULL;
	} // else
FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), ! ActiveGrad);
FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), ! ActiveGrad);
FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_GRADIENTPOS1, Ordinal), ! ActiveGrad);
FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_REMOVEMAT1, Ordinal), ! ActiveGrad);
FenetreHost->WidgetSetDisabled(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_COLOR1, Ordinal), ! ActiveGrad);

} // PortableMaterialGUI::DisableWidgets

/*===========================================================================*/

void PortableMaterialGUI::ConfigureMaterial(void)
{
MaterialEffect *Mat;
bool Configured = false;

if (!Grad) return;

if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
	{
	FenetreHost->WidgetShow(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), true);
	} // if
else
	{
	FenetreHost->WidgetShow(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), false); // this widget may not even be in the dialog template we're using...
	} // else


if (ActiveGrad = Grad->GetActiveNode())
	{
	FenetreHost->WidgetSmartRAHConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_GRADIENTPOS1, Ordinal), &ActiveGrad->Position, Grad);
	FenetreHost->WidgetCBSetCurSel(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), ActiveGrad->BlendStyle);
	Configured = true;
	if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMMATERIALGRADIENT)
		{ // it's a material, try to configure extra material controls
		if (Mat = (MaterialEffect *)ActiveGrad->GetThing())
			{
			FenetreHost->WidgetSetText(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), Mat->Name);
			FenetreHost->WidgetSetModified(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), FALSE);
			// leave Configured = true
			} // if
		else
			{ // failed to fully configure
			Configured = false;
			} // else
		} // if
	else if (Grad->GetRAHostTypeNumber() == WCS_RAHOST_OBJTYPE_ANIMCOLORGRADIENT )
		{ // nothing to do at this time
		FenetreHost->WidgetSmartRAHConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_COLOR1, Ordinal), &((ColorTextureThing *)ActiveGrad->GetThing())->Color, (ColorTextureThing *)ActiveGrad->GetThing());
		} // else if
	} // if
if (!Configured)
	{
	// configure everything to NULL
	FenetreHost->WidgetCBSetCurSel(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), 0);
	FenetreHost->WidgetSmartRAHConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_GRADIENTPOS1, Ordinal), (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSmartRAHConfig(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_COLOR1, Ordinal), (RasterAnimHost *)NULL, NULL);
	FenetreHost->WidgetSetText(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_MATNAME1, Ordinal), "");
	} // else

} // PortableMaterialGUI::ConfigureMaterial

/*===========================================================================*/


void PortableMaterialGUI::BlendStyle(void)
{
long Current;
NotifyTag Changes[2];

if (!Grad) return;

if (ActiveGrad = Grad->GetActiveNode())
	{
	Current = FenetreHost->WidgetCBGetCurSel(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal));
	ActiveGrad->BlendStyle = (unsigned char)Current;
	FenetreHost->WidgetCBSetCurSel(WCS_PORTABLEMATERIALGUI_OFFSET_ID(IDC_POPDROP_BLENDSTYLE1, Ordinal), ActiveGrad->BlendStyle);
	Changes[0] = MAKE_ID(Grad->GetNotifyClass(), Grad->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_ATTRIBUTE_CHANGED);
	Changes[1] = NULL;
	GlobalApp->AppEx->GenerateNotify(Changes, Grad->GetRAHostRoot());
	} // if

} // PortableMaterialGUI::BlendStyle()

/*===========================================================================*/

void PortableMaterialGUI::AddMatNode(void)
{
char PositionStr[64];
double NewPos;
GradientCritter *NewNode;

if (!Grad) return;

ActiveGrad = Grad->GetActiveNode();
strcpy(PositionStr, ActiveGrad ? "50 %": "0 %");
if (! ActiveGrad || GetInputString("Enter gradient position (in percent) for new material.", WCS_REQUESTER_POSDIGITS_ONLY, PositionStr))
	{
	NewPos = atof(PositionStr) / 100.0;
	if (NewNode = Grad->AddNodeNotify(NewPos, 1))
		{
		Grad->SetActiveNode(NewNode);
		ActiveGrad = NewNode; // need to tell host about this
		if (NotifyFunctor) NotifyFunctor->HandleNewActiveGrad(ActiveGrad);
		} // if
	} // if

} // PortableMaterialGUI::AddMatNode

/*===========================================================================*/

void PortableMaterialGUI::RemoveMatNode(void)
{
if (!Grad) return;

ActiveGrad = Grad->GetActiveNode();
if (ActiveGrad)
	{
	Grad->RemoveNode(ActiveGrad);
	ActiveGrad = NULL;
	} // if
// need to inform host
if (NotifyFunctor) NotifyFunctor->HandleConfigureMaterial();

} // PortableMaterialGUI::RemoveMatNode

/*===========================================================================*/

void PortableMaterialGUI::MatName(short WidID)
{
char NewName[WCS_EFFECT_MAXNAMELENGTH];
NotifyTag Changes[2];
MaterialEffect *Mat;

if (!Grad) return;

if (ActiveGrad = Grad->GetActiveNode())
	{
	if (Mat = (MaterialEffect *)ActiveGrad->GetThing())
		{
		if (FenetreHost->WidgetGetModified(WidID))
			{
			FenetreHost->WidgetGetText(WidID, WCS_EFFECT_MAXNAMELENGTH, NewName);
			FenetreHost->WidgetSetModified(WidID, FALSE);
			strncpy(Mat->Name, NewName, WCS_EFFECT_MAXNAMELENGTH);
			Mat->Name[WCS_EFFECT_MAXNAMELENGTH - 1] = 0;
			Changes[0] = MAKE_ID(Mat->GetNotifyClass(), Mat->GetNotifySubclass(), 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, Mat->GetRAHostRoot());
			} // if 
		} // if
	} // if

} // PortableMaterialGUI::MatName()

/*===========================================================================*/

bool PortableMaterialGUI::QueryIsDisplayed(void)
{
if (FenetreHost)
	{
	return(FenetreHost->IsPaneDisplayed(GetPanel(), 0));
	} // if
return(false);
} // PortableMaterialGUI::QueryIsDisplayed
