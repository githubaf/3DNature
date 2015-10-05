// MergerWiz.cpp
// Code file for MergerWiz
// Created from MergerWiz.cpp on 01/18/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "MergerWiz.h"
#include "Requester.h"
#include "Project.h"
#include "Raster.h"
#include "AppMem.h"
#include "DBFilterEvent.h"
#include "resource.h"

static char *MergerWizPageText[WCS_MERGERWIZ_NUMPAGES] = 
	{
	// WCS_MERGERWIZ_WIZPAGE_BOUNDS
	"The DEM that is created by the DEM Merger will completely fill the bounds that are set.  Bounds can be set in the following ways:\r\r\n\r\r\n\
   · Bounds by SQ - The created DEM will use the maximum of the extents found by all the DEMs selected with the Search Queries.  Coordinate \
conversion is performed as needed.\r\r\n\
   · Bounds set manually - Use this if you know the exact bounds you want to use.\r\r\n\
   · Bounds set in View - Use this if you want to set your bounds visually.",
	// WCS_MERGERWIZ_WIZPAGE_BOUNDS1
	"Set the bounds of your merge area (or the entire area for a multi-res merge).",
	// WCS_MERGERWIZ_WIZPAGE_BOUNDS2
	"Set the bounds of the high res merge area.",
	// WCS_MERGERWIZ_WIZPAGE_CANCEL
	"<CANCEL>",
	// WCS_MERGERWIZ_WIZPAGE_COMPLETE
	"<COMPLETE>",
	// WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR
	"<COMPLETEERROR>",
	// WCS_MERGERWIZ_WIZPAGE_CONFIRM
	"<CONFIRM>",
	// WCS_MERGERWIZ_WIZPAGE_COORDSYS
	"Merges use only a single output Coordinate System.  The default Coordinate System came from the Planet Options setting.  There is no way for \
the Wizard to know what's appropriate for your use.  The DEM(s) that the Merger creates will be based on the Coordinate System you choose.  Choose \
a Coordinate System below.",
	// WCS_MERGERWIZ_WIZPAGE_DEMSLOADED
	"The DEM Merger needs to create its DEMs from DEMs that are already loaded into the project.  Let the Wizard know if all the DEMs are already \
in the project.\r\r\n\r\r\nIf you still need to load DEMs, just launch the Import Wizard and load your data until you're done.  You can leave this \
window open while you import your data.\r\r\n\r\r\nOnce all your DEMs are loaded, press the YES & NEXT buttons.",
	// WCS_MERGERWIZ_WIZPAGE_MERGENAMES
	"On this page, you can can give your merger a descriptive name and choose name(s) for the DEM(s).\r\r\n\r\r\n\
The DEM created by the DEM Merger is named 'Merged'.  When a Multi-Res merger is being done, the second DEM is named 'HiResMerge'.  You can \
either use the default names, or set your own names.",
	// WCS_MERGERWIZ_WIZPAGE_MERGERES
	"The resolution of the DEM that is created by the DEM Merger needs to be set.  You can set this two different ways:\r\r\n\r\r\n\
   · Merge resolution from Search Queries - This will create a cell size equal to the smallest cell size found by the merger's Search Queries.\r\r\n\
   · Merge resolution manually set - Use this if you know the resolution that you want to use.",
	// WCS_MERGERWIZ_WIZPAGE_MERGETYPE
	"You can perform two types of merges.  The first is called a Normal merger.  The second is called a Multi-Res merger.\r\r\n\r\r\n\
A Normal merger will combine DEMs of possibly varying cell sizes and coordinate systems into a single DEM with a single coordinate system.  Reasons \
to use a merge of this type include:\r\r\n\
   · Creating a DEM in a common coordinate system from DEMs in multiple coordinate systems.\r\r\n\
   · Filling NULL data with elevations from another data set.\r\r\n\r\r\n\
A Multi-Res merger will create two DEMs from one or more source DEMs.  There will be an outer DEM with a hole, and an inner DEM of a higher \
resolution to fill that hole.  Reasons to create a merge of this type include:\r\r\n\
   · Creating a higher resolution section of a DEM to edit in the DEM Painter.\r\r\n\
   · Creating a lower resolution background area that renders faster.\r\r\n\r\r\n\
Set the type of merger below.",
	// WCS_MERGERWIZ_WIZPAGE_PAGEERROR
	"<PAGEERROR>",
	// WCS_MERGERWIZ_WIZPAGE_QUERYEDIT
	"Search Queries are used to determine which DEMs are merged. Use as many Queries as you need to select all the DEMs which are to be merged.\r\r\n\r\r\n\
If you want to create a high resolution DEM Insert, the first Query is used to select the DEM or DEMs for the High Res Insert. \
Remaining Queries select outlying or background DEMs.",
	// IDD_MERGERWIZ_QUERYMAKE
	"Merging is accomplished by having Search Queries select your source DEMs.",	// add more
	// WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS
	"Search Queries are used to select the DEMs for processing.  You have three options for using Search Queries.\r\r\n\r\r\n\
	· Use Search Queries that are already in your project\r\r\n\
	· Create new Search Queries now\r\r\n\
	· Select the DEMs in the database that you want to use, and a Search Query will be created for you\r\r\n\r\r\n\
You may create Search Queries yourself now without closing this wizard.  The 'Next' button will be disabled unless you choose the 'Yes' or 'Create' \
option.",
	// WCS_MERGERWIZ_WIZPAGE_SELECTDEMS
	"You will create a Search Query by selecting DEM(s) in the database.\r\r\n\r\r\n\
Press the 'Create Search Query' Button.  This will open the Database Editor if it's not already open.  Select all the DEMs that you want the, \
query to select, then press the 'Create Search Query' Button again to create the query.  Repeat until you have created all the queries you'll need \
(as little as one for a single merger, at least two for a multi-res merge).  The 'Next' button will be disabled until you create a Search Query.  You \
can close the Database Editor anytime after you create the query.\r\r\n\r\r\n\
NOTE: For multi-res merges, the first query needs to be the one that selects the data for the insert (higher res) DEM.",
	// WCS_MERGERWIZ_WIZPAGE_WELCOME
	"Welcome to the Visual Nature Studio Digital Elevation Model Merger Wizard, or \"Merger Wizard\" for short.\r\r\n\r\r\nThe Merger Wizard may \
be used as a fast and easy way to set up DEM Mergers. You will be asked questions about what you're trying to accomplish, and we'll guide you \
through the needed parts.\r\r\n\r\r\nProceed from panel to panel pressing the \"Next-->\" button whenever you are finished answering the questions \
on each panel. At any time you can backtrack with the \"<--Back\" button without losing any settings you have made along the way. If at some point \
the \"Next-->\" button does not respond then there is some question unanswered on the current panel. It must be answered before you can continue. \
When all the questions have been answered on all the panels, you can choose to go through one more time to review all your answers before you \
finalize the operation.\r\r\n\r\r\nIt is important to note that if you cancel the Wizard or close the window before you execute the \"Finish\" \
command on the last panel, no changes will be made to your VNS project. Once you hit the \"Finish\" button there will be changes to your project \
even if the Wizard operation can not be completed successfully for some reason. With this in mind it is a good idea to save the project now, \
before you begin the Wizard operation.  If you save your project, you'll be taken automatically to the next wizard page.",
	// WCS_MERGERWIZ_ADDPAGE - add the text for the page in the same order it is in the list of page defines in MergerWiz.h
	};

unsigned short MergerWizPageResourceID[WCS_MERGERWIZ_NUMPAGES] = 
	{
	IDD_MERGERWIZ_BOUNDS,
	IDD_MERGERWIZ_BOUNDS1,
	IDD_MERGERWIZ_BOUNDS2,
	IDD_MERGERWIZ_CANCEL,
	IDD_MERGERWIZ_COMPLETE,
	IDD_MERGERWIZ_COMPLETEERROR,
	IDD_MERGERWIZ_CONFIRM,
	IDD_MERGERWIZ_COORDSYS,
	IDD_MERGERWIZ_DEMSLOADED,
	IDD_MERGERWIZ_MERGENAMES,
	IDD_MERGERWIZ_MERGERES,
	IDD_MERGERWIZ_MERGETYPE,
	IDD_MERGERWIZ_PAGEERROR,
	IDD_MERGERWIZ_QUERYEDIT,
	IDD_MERGERWIZ_QUERYMAKE,
	IDD_MERGERWIZ_QUERYSTATUS,
	IDD_MERGERWIZ_SELECTDEMS,
	IDD_MERGERWIZ_WELCOME,
	// WCS_MERGERWIZ_ADDPAGE - add the panel resource ID in the same order it is in the list of page defines in MergerWiz.h
	};

/*===========================================================================*/

MergerWiz::MergerWiz()
{
unsigned short pageCt;

cancelOrder = 0;
finalOrderCancelled = 0;
highres = 0;
manualBounds = 0;
reviewOrder = 0;

for (pageCt = 0; pageCt < WCS_MERGERWIZ_NUMPAGES; pageCt ++)
	{
	ConfigurePage(&Wizzes[pageCt], pageCt);
	} // for

//Wizzes[WCS_MERGERWIZ_WIZPAGE_WELCOME].AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_CONFIRM]);
Wizzes[WCS_MERGERWIZ_WIZPAGE_WELCOME].AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_MERGETYPE]);

} // MergerWiz::MergerWiz

/*===========================================================================*/

MergerWiz::~MergerWiz()
{

} // MergerWiz::~MergerWiz

/*===========================================================================*/

WizardlyPage *MergerWiz::ProcessPage(WizardlyPage *ProcessMe, Project *CurProj)
{
WizardlyPage *wp;

if (cancelOrder && !finalOrderCancelled)
	{
	ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_CANCEL]);
	wp = ProcessMe->Next;
	} // if
else switch (ProcessMe->WizPageID)
	{
	case WCS_MERGERWIZ_WIZPAGE_BOUNDS:
		if (manualBounds)
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS1]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_MERGERES]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_BOUNDS1:
		if (highres)
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS2]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_MERGERES]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_BOUNDS2:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_MERGERES]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_CANCEL:
		{
		if (finalOrderCancelled || !ProcessMe->Prev || !ProcessMe->Prev->Prev)
			{
			EndOrderCancel();
			wp = NULL;
			break; // necessary to preserve logic flow after single-return optimization
			} // if cancelled
		ProcessMe = ProcessMe->Prev;
		ProcessMe->Revert();
		wp = ProcessMe;
		break;
		} // WCS_MERGERWIZ_WIZPAGE_CANCEL
	case WCS_MERGERWIZ_WIZPAGE_COMPLETE:
		{
		wp = NULL;
		break;
		} // WCS_MERGERWIZ_WIZPAGE_COMPLETE
	case WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR:
		{
		if (reviewOrder)
			{
			reviewOrder = 0;
			cancelOrder = finalOrderCancelled = 0;
			// return to second page (first page with selections to make)
			while (ProcessMe->Prev && ProcessMe->Prev->Prev)
				{
				ProcessMe = ProcessMe->Prev;
				ProcessMe->Revert();
				} // while
			wp = ProcessMe;
			} // if
		else
			{
			EndOrderCancel();
			wp = NULL;
			} // else
		break;
		} // WCS_MergerWiz_WIZPAGE_COMPLETEERROR
	case WCS_MERGERWIZ_WIZPAGE_CONFIRM:
		{
		if (reviewOrder)
			{
			reviewOrder = 0;
			// return to second page (first page with selections to make)
			while (ProcessMe->Prev && ProcessMe->Prev->Prev)
				{
				ProcessMe = ProcessMe->Prev;
				ProcessMe->Revert();
				} // while
			wp = ProcessMe;
			} // if
		else
			{
			if (! EndOrderComplete(CurProj))
				{
				cancelOrder = finalOrderCancelled = 1;
				ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR]);
				wp = ProcessMe->Next;
				} // if
			else
				wp = &Wizzes[WCS_MERGERWIZ_WIZPAGE_COMPLETE];
			} // else
		break;
		} // WCS_MERGERWIZ_WIZPAGE_CONFIRM
	case WCS_MERGERWIZ_WIZPAGE_COORDSYS:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_DEMSLOADED:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_COORDSYS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_MERGENAMES:
		wp = NULL;
		break;
	case WCS_MERGERWIZ_WIZPAGE_MERGERES:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_MERGENAMES]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_MERGETYPE:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_DEMSLOADED]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_PAGEERROR:
		{
		wp = NULL;
		break;
		} // WCS_MERGERWIZ_WIZPAGE_PAGEERROR
	case WCS_MERGERWIZ_WIZPAGE_SELECTDEMS:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_QUERYEDIT:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_QUERYMAKE:
		ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS:
		if (queryStatus == 1)
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_QUERYEDIT]);
		else if (queryStatus == 0)
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_BOUNDS]);
		else
			ProcessMe->AddNext(&Wizzes[WCS_MERGERWIZ_WIZPAGE_SELECTDEMS]);
		wp = ProcessMe->Next;
		break;
	case WCS_MERGERWIZ_WIZPAGE_WELCOME:
		{
		wp = ProcessMe->Next;
		break;
		} // WCS_MergerWiz_WIZPAGE_WELCOME
	default:
		{
		cancelOrder = finalOrderCancelled = 1;
		wp = &Wizzes[WCS_MERGERWIZ_WIZPAGE_PAGEERROR];
		break;
		} // default
	// WCS_MERGERWIZARD_ADDPAGE - add a case to handle the page when the page is being processed. If no case exists an error page will be displayed
	} // else switch

return(wp);

} // MergerWiz::ProcessPage

/*===========================================================================*/

int MergerWiz::ConfigurePage(WizardlyPage *ConfigMe, unsigned short ConfigPage)
{
int rVal = 0;

if (ConfigPage < WCS_MERGERWIZ_NUMPAGES)
	{
	ConfigMe->WizPageID = ConfigPage;
	ConfigMe->Text = MergerWizPageText[ConfigPage];
	ConfigMe->WizPageResourceID = MergerWizPageResourceID[ConfigPage];
	rVal = 1;
	} // if

return(rVal);

} // MergerWiz::ConfigurePage

/*===========================================================================*/

void MergerWiz::EndOrderCancel(void)
{

// clean up any resources used

} // MergerWiz::EndOrderCancel

/*===========================================================================*/

int MergerWiz::EndOrderComplete(Project *CurProj)
{
int Success = 0;

return (Success);

} // MergerWiz::EndOrderComplete

/*===========================================================================*/
