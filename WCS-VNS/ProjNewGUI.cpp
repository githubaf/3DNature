// ProjNewGUI.cpp
// Code for Wave editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "ProjNewGUI.h"
#include "Project.h"
#include "Application.h"
#include "Toolbar.h"
#include "ImportWizGUI.h"
#include "Requester.h"
#include "Conservatory.h"
#include "ViewGUI.h"
#include "resource.h"

NativeGUIWin ProjNewGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ProjNewGUI::Open

/*===========================================================================*/

NativeGUIWin ProjNewGUI::Construct(void)
{

if(!NativeWin)
	{
	#ifdef WCS_BUILD_VNS
	NativeWin = CreateWinFromTemplate(IDD_PROJECT_NEW_VNS, LocalWinSys()->RootWin);
	#else // WCS_BUILD_VNS
	NativeWin = CreateWinFromTemplate(IDD_PROJECT_NEW, LocalWinSys()->RootWin);
	#endif // WCS_BUILD_VNS

	if(NativeWin)
		{
		ConfigureWidgets();
		} // if

	} // if
 
return(NativeWin);
} // ProjNewGUI::Construct

/*===========================================================================*/

ProjNewGUI::ProjNewGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, ImageLib *ImageSource)
: GUIFenetre('PRON', this, "New Project"), ProjNew(ProjSource, DBSource, EffectsSource, ImageSource)
{

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
ActiveItem = 0;

if (! ProjNew.ProjHost->ProjectLoaded)
	ProjNew.CloneType = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // ProjNewGUI::ProjNewGUI

/*===========================================================================*/

ProjNewGUI::~ProjNewGUI()
{

GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // ProjNewGUI::~ProjNewGUI()

/*===========================================================================*/

long ProjNewGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_PNG, 0);

return(0);

} // ProjNewGUI::HandleCloseWin

/*===========================================================================*/

long ProjNewGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(073, 73);
switch(ButtonID)
	{
	case ID_SAVE:
		{
		if(AppScope->GUIWins->CVG) AppScope->GUIWins->CVG->CloseViewsForIO();
		if (CreateNewProject())
			{
			// can't have two delayed notifications in queue at once so only way to open Import Wizard and close
			// New project is to do it ourselves.
			if (UserMessageYN("Import Data", "Your new Project has been created and saved.\nWould you like to import some data now?"))
				{
				Close();
				if(!AppScope->GUIWins->IWG)
					{
					AppScope->GUIWins->IWG = new ImportWizGUI(AppScope->AppDB, AppScope->MainProj, AppScope->AppEffects, AppScope->StatusLog, true);
 					if(AppScope->GUIWins->IWG->ConstructError)
 						{
 						delete AppScope->GUIWins->IWG;
 						AppScope->GUIWins->IWG = NULL;
 						} // if
					} // if
				if(AppScope->GUIWins->IWG)
					{
					AppScope->GUIWins->IWG->Open(AppScope->MainProj);
					}
				} // if
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_PNG, 0);
			} // if saved
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_PNG, 0);
		break;
		} // 
	case IDC_ADDTEMPLATE:
		{
		AddTemplate();
		break;
		} // IDC_ADDTEMPLATE
	case IDC_REMOVETEMPLATE:
		{
		RemoveTemplate();
		break;
		} // IDC_REMOVETEMPLATE
	case IDC_MOVETEMPUP:
		{
		ChangeTemplateListPosition(1);
		break;
		} // IDC_MOVETEMPLATEUP
	case IDC_MOVETEMPDOWN:
		{
		ChangeTemplateListPosition(0);
		break;
		} // IDC_MOVETEMPLATEDOWN
	case IDC_ENABLETEMPLATE:
		{
		EnableTemplate(1);
		break;
		} // IDC_ENABLETEMPLATE
	case IDC_DISABLETEMPLATE:
		{
		EnableTemplate(0);
		break;
		} // IDC_DISABLETEMPLATE
	default: break;
	} // ButtonID

return(0);

} // ProjNewGUI::HandleButtonClick

/*===========================================================================*/

long ProjNewGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(052, 52);
switch (CtrlID)
	{
	case IDC_TEMPLATELIST:
		{
		SetActiveTemplate();
		break;
		}
	} // switch CtrlID

return (0);

} // ProjNewGUI::HandleListSel

/*===========================================================================*/

long ProjNewGUI::HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData)
{

switch(CtrlID)
	{
	case IDC_TEMPLATELIST:
		{
		RemoveTemplate();
		break;
		} // IDC_TEMPLATELIST
	} // switch

return(0);

} // ProjNewGUI::HandleListDelItem

/*===========================================================================*/

long ProjNewGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_TEMPLATELIST:
		{
		EnableTemplate(-1);
		break;
		}
	} // switch CtrlID

return (0);

} // ProjNewGUI::HandleListDoubleClick

/*===========================================================================*/

long ProjNewGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_USETEMPLATES:
	case IDC_CLONE:
		{
		DisableWidgets();
		break;
		} // 
	} // switch CtrlID

return(0);

} // ProjNewGUI::HandleSCChange

/*===========================================================================*/

long ProjNewGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_RADIOCLONECURRENT:
	case IDC_RADIOCLONEFROMDISK:
		{
		DisableWidgets();
		break;
		} // 
	} // switch CtrlID

return(0);

} // ProjNewGUI::HandleSRChange

/*===========================================================================*/

void ProjNewGUI::ConfigureWidgets(void)
{

ConfigureDD(NativeWin, IDC_PRJ_PROJ, (char *)ProjNew.ProjName.GetPath(), 255, (char *)ProjNew.ProjName.GetName(), 31, IDC_LABEL_PROJ);
ConfigureDD(NativeWin, IDC_PRJ_CLONE, (char *)ProjNew.CloneName.GetPath(), 255, (char *)ProjNew.CloneName.GetName(), 31, IDC_PRJ_CLONE_LABEL);

ConfigureSC(NativeWin, IDC_SUBDIRPROJ, &ProjNew.PlaceInSubDir, SCFlag_Char, NULL, 0);
ConfigureSC(NativeWin, IDC_CLONE, &ProjNew.Clone, SCFlag_Char, NULL, 0);

ConfigureSR(NativeWin, IDC_RADIOCLONECURRENT, IDC_RADIOCLONECURRENT, &ProjNew.CloneType, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOCLONECURRENT, IDC_RADIOCLONEFROMDISK, &ProjNew.CloneType, SRFlag_Char, 1, NULL, NULL);

ConfigureTB(NativeWin, IDC_ADDTEMPLATE, IDI_ADDSOMETHING, NULL);
ConfigureTB(NativeWin, IDC_REMOVETEMPLATE, IDI_DELETE, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_ENABLETEMPLATE, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_DISABLETEMPLATE, IDI_DISABLE, NULL);

ConfigureSC(NativeWin, IDC_USETEMPLATES, &ProjNew.ProjHost->Prefs.NewProjUseTemplates, SCFlag_Short, NULL, 0);

DisableWidgets();
BuildTemplateList();

} // ProjNewGUI::ConfigureWidgets()

/*===========================================================================*/

void ProjNewGUI::DisableWidgets(void)
{

WidgetSetDisabled(IDC_RADIOCLONECURRENT, ! (ProjNew.Clone && ProjNew.ProjHost->ProjectLoaded));
WidgetSetDisabled(IDC_RADIOCLONEFROMDISK, ! ProjNew.Clone);
WidgetSetDisabled(IDC_PRJ_CLONE_LABEL, ! (ProjNew.Clone && ProjNew.CloneType));
WidgetSetDisabled(IDC_PRJ_CLONE, ! (ProjNew.Clone && ProjNew.CloneType));

WidgetSetDisabled(IDC_USETEMPLATES, ProjNew.Clone);
WidgetSetDisabled(IDC_ADDTEMPLATE, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_REMOVETEMPLATE, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_MOVETEMPUP, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_MOVETEMPDOWN, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_ENABLETEMPLATE, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_DISABLETEMPLATE, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);
WidgetSetDisabled(IDC_TEMPLATELIST, ProjNew.Clone || ! ProjNew.ProjHost->Prefs.NewProjUseTemplates);

} // ProjNewGUI::DisableWidgets

/*===========================================================================*/

int ProjNewGUI::CreateNewProject(void)
{

return (ProjNew.Create());

} // ProjNewGUI::CreateNewProject()

/*===========================================================================*/

void ProjNewGUI::BuildTemplateList(void)
{
char TemplateName[512];
long Ct, LastPos = -1;

WidgetLBClear(IDC_TEMPLATELIST);

for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.GetPath()[0] && ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.GetName()[0])
		{
		BuildTemplateListEntry(TemplateName, &ProjNew.ProjHost->Prefs.DefaultTemplates[Ct]);
		LastPos = WidgetLBInsert(IDC_TEMPLATELIST, -1, TemplateName);
		} // if
	} // for

ActiveItem = min(ActiveItem, LastPos);
WidgetLBSetCurSel(IDC_TEMPLATELIST, ActiveItem);

} // ProjNewGUI::BuildTemplateList

/*===========================================================================*/

void ProjNewGUI::BuildTemplateListEntry(char *ListName, Template *Me)
{
char TempName[512];

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->PAF.GetPathAndName(TempName));

} // ProjNewGUI::BuildTemplateListEntry()

/*===========================================================================*/

void ProjNewGUI::AddTemplate(void)
{
long Ct;

for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (! (ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.GetPath()[0] && ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.GetName()[0]))
		{
		ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.SetPath("WCSProjects:");
		if (ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].PAF.SelectFile())
			{
			ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].Enabled = 1;
			ActiveItem = Ct;
			BuildTemplateList();
			} // if
		return;
		} // if
	} // for

UserMessageOK("Add Template", "You have reached the maximum number of Templates. Remove one or more and try adding again.");

} // ProjNewGUI::AddTemplate

/*===========================================================================*/

void ProjNewGUI::RemoveTemplate(void)
{
long Current, Ct;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
if (Current >= 0 && Current < WCS_MAX_TEMPLATES)
	{
	for (Ct = Current; Ct < WCS_MAX_TEMPLATES - 1; Ct ++)
		{
		ProjNew.ProjHost->Prefs.DefaultTemplates[Ct].Copy(&ProjNew.ProjHost->Prefs.DefaultTemplates[Ct + 1]);
		} // for
	ProjNew.ProjHost->Prefs.DefaultTemplates[WCS_MAX_TEMPLATES - 1].Clear();
	BuildTemplateList();
	} // if

} // ProjNewGUI::RemoveTemplate

/*===========================================================================*/

void ProjNewGUI::EnableTemplate(int NewState)
{
long Current;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
if (Current >= 0 && Current < WCS_MAX_TEMPLATES)
	{
	if (NewState >= 0)
		ProjNew.ProjHost->Prefs.DefaultTemplates[Current].Enabled = NewState;
	else
		ProjNew.ProjHost->Prefs.DefaultTemplates[Current].Enabled = 1 - ProjNew.ProjHost->Prefs.DefaultTemplates[Current].Enabled;
	BuildTemplateList();
	} // if

} // ProjNewGUI::EnableTemplate

/*===========================================================================*/

void ProjNewGUI::ChangeTemplateListPosition(int MoveUp)
{
long Current, NumItems;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
NumItems = WidgetLBGetCount(IDC_TEMPLATELIST);
if (MoveUp && Current > 0)
	{
	swmem(&ProjNew.ProjHost->Prefs.DefaultTemplates[Current], &ProjNew.ProjHost->Prefs.DefaultTemplates[Current - 1], sizeof (Template));
	ActiveItem = Current - 1;
	} // if up
else if (! MoveUp && Current < NumItems - 1)
	{
	swmem(&ProjNew.ProjHost->Prefs.DefaultTemplates[Current], &ProjNew.ProjHost->Prefs.DefaultTemplates[Current + 1], sizeof (Template));
	ActiveItem = Current + 1;
	} // else if down
BuildTemplateList();

} // ProjNewGUI::ChangeTemplateListPosition

/*===========================================================================*/

void ProjNewGUI::SetActiveTemplate(void)
{

ActiveItem = WidgetLBGetCurSel(IDC_TEMPLATELIST);

} // ProjNewGUI::SetActiveTemplate()

/*===========================================================================*/
