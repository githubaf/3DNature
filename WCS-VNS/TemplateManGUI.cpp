// TemplateManGUI.cpp
// Code for Template Manager editor
// Built from ColorEditGUI.cpp on 2/27/96 by Chris "Xenon" Hanson & Gary Huber
// Copyright 1996 Questar Productions

#include "stdafx.h"
#include "TemplateManGUI.h"
#include "Project.h"
#include "Application.h"
#include "Toolbar.h"
#include "Requester.h"
#include "ImportThing.h"
#include "AppMem.h"
#include "resource.h"

bool TemplateManGUI::InquireWindowCapabilities(FenetreWindowCapabilities AskAbout)
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

} // TemplateManGUI::InquireWindowCapabilities

/*===========================================================================*/

NativeGUIWin TemplateManGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // TemplateManGUI::Open

/*===========================================================================*/

NativeGUIWin TemplateManGUI::Construct(void)
{

if (!NativeWin)
	{
	#ifdef WCS_BUILD_VNS
	NativeWin = CreateWinFromTemplate(IDD_TEMPLATE_MANAGER, LocalWinSys()->RootWin);

	if (NativeWin)
		{
		ConfigureWidgets();
		WidgetLBSetHorizExt(IDC_TEMPLATELIST, 1000);
		WidgetLBSetHorizExt(IDC_IMPORTLIST, 1000);
		} // if
	#else // WCS_BUILD_VNS
	return (NULL);
	#endif // WCS_BUILD_VNS
	} // if
 
return(NativeWin);

} // TemplateManGUI::Construct

/*===========================================================================*/

TemplateManGUI::TemplateManGUI(Project *ProjSource)
: GUIFenetre('TPLM', this, "Template Manager")
{
long Ct;

_OwnerdrawMode = WCS_FENETRE_OWNERDRAW_MODE_BASIC;
ConstructError = 0;
ActiveItem = 0;
ProjHost = ProjSource;
ActiveImport = ProjHost->Imports;
ImportEnabledList = NULL;
NumImportItems = 0;

if (ProjHost && ProjHost->ProjectLoaded)
	{
	for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
		Active[Ct].Copy(&ProjHost->ProjectTemplates[Ct]);
	} // if
else
	ConstructError = 1;

SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);

} // TemplateManGUI::TemplateManGUI

/*===========================================================================*/

TemplateManGUI::~TemplateManGUI()
{

FreeImportEnabledList();
GlobalApp->MCP->RemoveWindowFromMenuList(this);

} // TemplateManGUI::~TemplateManGUI()

/*===========================================================================*/

long TemplateManGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_TPM, 0);

return(0);

} // TemplateManGUI::HandleCloseWin

/*===========================================================================*/

long TemplateManGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{

SECURITY_INLINE_CHECK(073, 73);
switch (ButtonID)
	{
	case ID_SAVE:
		{
		if (Apply())
			{
			AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
				WCS_TOOLBAR_ITEM_TPM, 0);
			} // if
		break;
		} // 
	case IDCANCEL:
		{
		AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
			WCS_TOOLBAR_ITEM_TPM, 0);
		break;
		} // 
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
	case IDC_ENABLEIMPORT:
		{
		EnableImport(1);
		break;
		} // IDC_ENABLEIMPORT
	case IDC_DISABLEIMPORT:
		{
		EnableImport(0);
		break;
		} // IDC_DISABLEIMPORT
	case IDC_LOAD:
		{
		ReplaceTemplate();
		break;
		} // IDC_DISABLETEMPLATE
	case IDC_WINUNDO:
		{
		UndoChanges();
		break;
		} // IDC_DISABLETEMPLATE
	default:
		break;
	} // ButtonID

return(0);

} // TemplateManGUI::HandleButtonClick

/*===========================================================================*/

long TemplateManGUI::HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

SECURITY_INLINE_CHECK(032, 32);
switch (CtrlID)
	{
	case IDC_TEMPLATELIST:
		{
		SetActiveTemplate();
		break;
		}
	case IDC_IMPORTLIST:
		{
		SetActiveImport();
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // TemplateManGUI::HandleListSel

/*===========================================================================*/

long TemplateManGUI::HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_TEMPLATELIST:
		{
		EnableTemplate(-1);
		break;
		}
	case IDC_IMPORTLIST:
		{
		EnableImport(-1);
		break;
		}
	default:
		break;
	} // switch CtrlID

return (0);

} // TemplateManGUI::HandleListDoubleClick

/*===========================================================================*/

long TemplateManGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_EMBED:
	case IDC_REMOVETEMPLATE:
	case IDC_EMBEDIMPORT:
	case IDC_REMOVEIMPORT:
		{
		DisableWidgets();
		break;
		} // 
	default:
		break;
	} // switch CtrlID

return(0);

} // TemplateManGUI::HandleSCChange

/*===========================================================================*/

void TemplateManGUI::ConfigureWidgets(void)
{

ConfigureTB(NativeWin, IDC_MOVETEMPUP, IDI_ARROWUP, NULL);
ConfigureTB(NativeWin, IDC_MOVETEMPDOWN, IDI_ARROWDOWN, NULL);
ConfigureTB(NativeWin, IDC_ENABLETEMPLATE, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_DISABLETEMPLATE, IDI_DISABLE, NULL);
ConfigureTB(NativeWin, IDC_ENABLEIMPORT, IDI_ENABLE, NULL);
ConfigureTB(NativeWin, IDC_DISABLEIMPORT, IDI_DISABLE, NULL);
ConfigureTB(NativeWin, IDC_LOAD, IDI_FILEOPEN, NULL);

BuildTemplateList();
ConfigureTemplate();
BuildImportList();
ConfigureImport();
BuildImportEnabledList();
DisableWidgets();

} // TemplateManGUI::ConfigureWidgets()

/*===========================================================================*/

void TemplateManGUI::ConfigureTemplate(void)
{

if ((ActiveItem = WidgetLBGetCurSel(IDC_TEMPLATELIST)) != LB_ERR)
	{
	ConfigureSC(NativeWin, IDC_EMBED, &Active[ActiveItem].Embed, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_REMOVETEMPLATE, &Active[ActiveItem].Remove, SCFlag_Char, NULL, 0);
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_EMBED, NULL, NULL, NULL, 0);
	ConfigureSC(NativeWin, IDC_REMOVETEMPLATE, NULL, NULL, NULL, 0);
	} // else

} // TemplateManGUI::ConfigureTemplate

/*===========================================================================*/

void TemplateManGUI::ConfigureImport(void)
{
Pier1 *TempImport;
long Current;

ActiveImport = NULL;

if ((Current = WidgetLBGetCurSel(IDC_IMPORTLIST)) != LB_ERR)
	{
	if ((TempImport = (Pier1 *)WidgetLBGetItemData(IDC_IMPORTLIST, Current)) != (Pier1 *)LB_ERR)
		{
		ActiveImport = TempImport;
		} // for
	} // if
if (ActiveImport)
	{
	ConfigureSC(NativeWin, IDC_EMBEDIMPORT, &ActiveImport->Embed, SCFlag_Char, NULL, 0);
	ConfigureSC(NativeWin, IDC_REMOVEIMPORT, &ActiveImport->Remove, SCFlag_Char, NULL, 0);
	} // if
else
	{
	ConfigureSC(NativeWin, IDC_EMBEDIMPORT, NULL, NULL, NULL, 0);
	ConfigureSC(NativeWin, IDC_REMOVEIMPORT, NULL, NULL, NULL, 0);
	} // else

} // TemplateManGUI::ConfigureImport

/*===========================================================================*/

void TemplateManGUI::DisableWidgets(void)
{

if (ActiveItem != LB_ERR)
	{
	WidgetSetDisabled(IDC_EMBED, Active[ActiveItem].Remove);
	WidgetSetDisabled(IDC_REMOVETEMPLATE, Active[ActiveItem].Embed);
	} // if
if (ActiveImport)
	{
	WidgetSetDisabled(IDC_EMBEDIMPORT, ActiveImport->Remove);
	WidgetSetDisabled(IDC_REMOVEIMPORT, ActiveImport->Embed);
	} // if

} // TemplateManGUI::DisableWidgets

/*===========================================================================*/

int TemplateManGUI::Apply(void)
{
#ifndef WCS_BUILD_DEMO
Pier1 *TempPier;
long Ct, MoreChanges, Changes = 0, FileChanges = 0, EnableChanges = 0, RemoveChanges = 0, EmbedChanges = 0;

// test for changes
for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (strcmp(ProjHost->ProjectTemplates[Ct].PAF.GetPath(), Active[Ct].PAF.GetPath()))
		Changes = FileChanges = 1;
	if (strcmp(ProjHost->ProjectTemplates[Ct].PAF.GetName(), Active[Ct].PAF.GetName()))
		Changes = FileChanges = 1;
	if (Active[Ct].Enabled != ProjHost->ProjectTemplates[Ct].Enabled)
		Changes = EnableChanges = 1;
	if (Active[Ct].Remove)
		Changes = RemoveChanges = 1;
	if (Active[Ct].Embed)
		Changes = EmbedChanges = 1;
	} // for

TempPier = ProjHost->Imports;
while (TempPier)
	{
	//if (Active[Ct].Enabled != ProjHost->ProjectTemplates[Ct].Enabled)
	//	Changes = EnableChanges = 1;
	if (TempPier->Remove)
		Changes = RemoveChanges = 1;
	if (TempPier->Embed)
		Changes = EmbedChanges = 1;
	TempPier = TempPier->Next;
	} // while

if (Changes)
	{
	if (UserMessageOKCAN("Template Manager", "Component linkages in a Project are very sensitive to the order in which they are loaded from the Project and Template files. Improper changes to Templates may cause incomplete component linkage. If the Project has been modified since the last time it was saved, it should be saved prior to executing Template changes so that you can revert to it if the changes are unsatisfactory. Do you wish to proceed with the changes now?"))
		{
		for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
			ProjHost->ProjectTemplates[Ct].Copy(&Active[Ct]);

		if (RemoveChanges)
			{
			MoreChanges = 1;
			while (MoreChanges)
				{
				MoreChanges = 0;
				for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
					{
					if (ProjHost->ProjectTemplates[Ct].Remove)
						{
						MoreChanges = 1;
						break;
						} // if
					} // for
				if (MoreChanges)
					{
					for ( ; Ct < WCS_MAX_TEMPLATES - 1; Ct ++)
						{
						ProjHost->ProjectTemplates[Ct].Copy(&ProjHost->ProjectTemplates[Ct + 1]);
						} // for
					ProjHost->ProjectTemplates[WCS_MAX_TEMPLATES - 1].Clear();
					} // if
				} // while
			MoreChanges = 1;
			while (MoreChanges)
				{
				MoreChanges = 0;
				TempPier = ProjHost->Imports;
				while (TempPier)
					{
					if (TempPier->Remove)
						{
						MoreChanges = 1;
						ProjHost->RemovePier(TempPier);
						break;
						} // if
					TempPier = TempPier->Next;
					} // while
				} // while
			} // else if

		// save changes to the resume file and then resume to make them take effect
		ProjHost->SavePrefs(GlobalApp->GetProgDir());
		ProjHost->LoadResumeState(GlobalApp->GetProgDir());

		if (EmbedChanges)
			{
			// template components were embedded, now remove template from project
			MoreChanges = 1;
			while (MoreChanges)
				{
				MoreChanges = 0;
				for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
					{
					if (ProjHost->ProjectTemplates[Ct].Embed)
						{
						MoreChanges = 1;
						break;
						} // if
					} // for
				if (MoreChanges)
					{
					for ( ; Ct < WCS_MAX_TEMPLATES - 1; Ct ++)
						{
						ProjHost->ProjectTemplates[Ct].Copy(&ProjHost->ProjectTemplates[Ct + 1]);
						} // for
					ProjHost->ProjectTemplates[WCS_MAX_TEMPLATES - 1].Clear();
					} // if
				} // while
			MoreChanges = 1;
			while (MoreChanges)
				{
				MoreChanges = 0;
				TempPier = ProjHost->Imports;
				while (TempPier)
					{
					if (TempPier->Embed)
						{
						MoreChanges = 1;
						ProjHost->RemovePier(TempPier);
						break;
						} // if
					TempPier = TempPier->Next;
					} // while
				} // while
			ProjHost->SavePrefs(GlobalApp->GetProgDir());
			} // if
		// since the window might stay open, show the latest changes
		ConfigureWidgets();
		UserMessageOK("Template Manager", "Changes have been applied to the Resume file and to the Project in memory. When you are satisfied that the changes are correct please resave the Project (CTRL+S). To undo the changes reload the project (CTRL+O) before resaving.");
		} // if changes approved
	else
		return (0);
	} // if changes
else
	{
	UserMessageOK("Template Manager", "There are no changes to apply.");
	} // else no changes
#else
UserMessageDemo("Templates may not be changed.");
#endif // WCS_BUILD_DEMO

return (1);

} // TemplateManGUI::Apply()

/*===========================================================================*/

void TemplateManGUI::BuildTemplateList(void)
{
long Ct, LastPos = LB_ERR;
char TemplateName[512];

WidgetLBClear(IDC_TEMPLATELIST);

for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	{
	if (Active[Ct].PAF.GetPath()[0] && Active[Ct].PAF.GetName()[0])
		{
		BuildTemplateListEntry(TemplateName, &Active[Ct]);
		LastPos = WidgetLBInsert(IDC_TEMPLATELIST, -1, TemplateName);
		} // if
	} // for

ActiveItem = min(ActiveItem, LastPos);
WidgetLBSetCurSel(IDC_TEMPLATELIST, ActiveItem);

} // TemplateManGUI::BuildTemplateList

/*===========================================================================*/

void TemplateManGUI::BuildTemplateListEntry(char *ListName, Template *Me)
{
char TempName[512];

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strcat(ListName, Me->PAF.GetPathAndName(TempName));

} // TemplateManGUI::BuildTemplateListEntry()

/*===========================================================================*/

void TemplateManGUI::EnableTemplate(int NewState)
{
long Current;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
if (Current >= 0 && Current < WCS_MAX_TEMPLATES)
	{
	if (NewState >= 0)
		Active[Current].Enabled = NewState;
	else
		Active[Current].Enabled = 1 - Active[Current].Enabled;
	BuildTemplateList();
	} // if

} // TemplateManGUI::EnableTemplate

/*===========================================================================*/

void TemplateManGUI::ChangeTemplateListPosition(int MoveUp)
{
long Current, NumItems;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
NumItems = WidgetLBGetCount(IDC_TEMPLATELIST);
if (MoveUp && Current > 0)
	{
	swmem(&Active[Current], &Active[Current - 1], sizeof (Template));
	ActiveItem = Current - 1;
	} // if up
else if (! MoveUp && Current < NumItems - 1)
	{
	swmem(&Active[Current], &Active[Current + 1], sizeof (Template));
	ActiveItem = Current + 1;
	} // else if down
BuildTemplateList();
ConfigureTemplate();

} // TemplateManGUI::ChangeTemplateListPosition

/*===========================================================================*/

void TemplateManGUI::SetActiveTemplate(void)
{

ActiveItem = WidgetLBGetCurSel(IDC_TEMPLATELIST);
ConfigureTemplate();

} // TemplateManGUI::SetActiveTemplate()

/*===========================================================================*/

void TemplateManGUI::ReplaceTemplate(void)
{
long Current;

Current = WidgetLBGetCurSel(IDC_TEMPLATELIST);
if (Current >= 0 && Current < WCS_MAX_TEMPLATES)
	{
	if (Active[Current].PAF.SelectFile())
		{
		Active[Current].Enabled = 1;
		Active[Current].Embed = 0;
		Active[Current].Remove = 0;
		BuildTemplateList();
		ConfigureTemplate();
		} // if
	} // if

} // TemplateManGUI::ReplaceTemplate

/*===========================================================================*/

void TemplateManGUI::BuildImportList(void)
{
Pier1 *TempImport;
long Found = -1, LastPos;
char ImportName[512];

WidgetLBClear(IDC_IMPORTLIST);

TempImport = ProjHost->Imports;
while (TempImport)
	{
	if (! TempImport->TemplateItem)
		{
		BuildImportListEntry(ImportName, TempImport);
		LastPos = WidgetLBInsert(IDC_IMPORTLIST, -1, ImportName);
		WidgetLBSetItemData(IDC_IMPORTLIST, LastPos, TempImport);
		if (TempImport == ActiveImport)
			Found = LastPos;
		} // if
	TempImport = TempImport->Next;
	} // while

WidgetLBSetCurSel(IDC_IMPORTLIST, Found);

} // TemplateManGUI::BuildImportList

/*===========================================================================*/

void TemplateManGUI::BuildImportListEntry(char *ListName, Pier1 *Me)
{
char TempName[512];

if (Me->Enabled)
	strcpy(ListName, "* ");
else
	strcpy(ListName, "  ");
strmfp(TempName, Me->InDir, Me->InFile);
strcat(ListName, TempName);

} // TemplateManGUI::BuildImportListEntry()

/*===========================================================================*/

void TemplateManGUI::EnableImport(int NewState)
{
Pier1 *TempImport;
long Current;

if ((Current = WidgetLBGetCurSel(IDC_IMPORTLIST)) != LB_ERR)
	{
	if ((TempImport = (Pier1 *)WidgetLBGetItemData(IDC_IMPORTLIST, Current)) != (Pier1 *)LB_ERR)
		{
		if (NewState >= 0)
			TempImport->Enabled = NewState;
		else
			TempImport->Enabled = 1 - TempImport->Enabled;
		BuildImportList();
		} // for
	} // if

} // TemplateManGUI::EnableImport

/*===========================================================================*/

void TemplateManGUI::SetActiveImport(void)
{

ConfigureImport();

} // TemplateManGUI::SetActiveImport()

/*===========================================================================*/

void TemplateManGUI::BuildImportEnabledList(void)
{
Pier1 *TempPier;
long Ct;

FreeImportEnabledList();

TempPier = ProjHost->Imports;
while (TempPier)
	{
	NumImportItems ++;
	TempPier = TempPier->Next;
	} // while

if (NumImportItems > 0 && (ImportEnabledList = (char *)AppMem_Alloc(NumImportItems, APPMEM_CLEAR)))
	{
	TempPier = ProjHost->Imports;
	Ct = 0;
	while (TempPier)
		{
		ImportEnabledList[Ct] = TempPier->Enabled;
		Ct ++;
		TempPier = TempPier->Next;
		} // while
	} // if

} // TemplateManGUI::BuildImportEnabledList

/*===========================================================================*/

void TemplateManGUI::FreeImportEnabledList(void)
{

if (ImportEnabledList)
	{
	AppMem_Free(ImportEnabledList, NumImportItems);
	ImportEnabledList = NULL;
	NumImportItems = 0;
	} // if

} // TemplateManGUI::FreeImportEnabledList

/*===========================================================================*/

void TemplateManGUI::UndoChanges(void)
{
Pier1 *TempPier;
long Ct;

for (Ct = 0; Ct < WCS_MAX_TEMPLATES; Ct ++)
	Active[Ct].Copy(&ProjHost->ProjectTemplates[Ct]);

TempPier = ProjHost->Imports;
Ct = 0;
while (TempPier)
	{
	TempPier->Embed = TempPier->Remove = 0;
	if (ImportEnabledList)
		{
		TempPier->Enabled = ImportEnabledList[Ct];
		} // if
	Ct ++;
	TempPier = TempPier->Next;
	} // while

BuildTemplateList();
ConfigureTemplate();
BuildImportList();
ConfigureImport();

} // TemplateManGUI::UndoChanges
