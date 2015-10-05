// GridderWiz.cpp
// Code file for GridderWiz
// Created from GridderWiz.cpp on 03/20/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "GridderWiz.h"
#include "Requester.h"
#include "Project.h"
#include "Raster.h"
#include "AppMem.h"
#include "DBFilterEvent.h"
#include "resource.h"

static char *GridderWizPageText[WCS_GRIDDERWIZ_NUMPAGES] = 
	{
	// WCS_GRIDDERWIZ_WIZPAGE_BOUNDS
	"The bounds set the limits of the DEM to be created by the Terrain Gridder.  The coordinates for the boundaries are given in units \
of your chosen Coordinate System.",
	// WCS_GRIDDERWIZ_WIZPAGE_CANCEL
	"< Cancel >",
	// WCS_GRIDDERWIZ_WIZPAGE_COMPLETE
	"A Terrain Gridder has been added to your project.  You will need to run the Terrain Gridder ('Grid & Save' on the 'Output & Filters' tab) \
in order to create your DEM.",
	// WCS_GRIDDERWIZ_WIZPAGE_COMPLETEERROR
	"< Complete Error >",
	// WCS_GRIDDERWIZ_WIZPAGE_COORDSYS
	"Choose the Coordinate System that you wish to use on the DEM to be created by the Terrain Gridder.  The combo box contains a list of the \
ones currently in use for your project.",
	// WCS_GRIDDERWIZ_WIZPAGE_DATALOADED
	"The Gridder needs to create a DEM from Control Points or Vectors that are already loaded into the project. Let the Wizard know if all \
the data needed for gridding is already in the project.\r\r\n\r\r\nIf you still need to load data for gridding, just launch the Import Wizard \
and load the data needed until you're done. You can leave this window open while you import your data.\r\r\n\r\r\nOnce all the necessary data \
is loaded, press the YES & NEXT buttons.",
	// WCS_GRIDDERWIZ_WIZPAGE_DEMNAME
	"You should enter a descriptive name for your DEM here. This will be the name of the DEM in the database once the Terrain Gridder \
generates the DEM.",
	// WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR
	"< Page Error >",
	// WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA
	"In the Database Editor, select all the data you wish to be gridded together into a single DEM. Make sure only the data you want to grid \
is selected. You can use the CONTROL & SHIFT keys to multi-select items. When you have done this, select the NEXT button.",
	// WCS_GRIDDERWIZ_WIZPAGE_WELCOME
	"Welcome to the Digital Elevation Model Gridder Wizard, or \"Gridder Wizard\" for short.\r\r\n\r\r\nThe Gridder Wizard may \
be used as a fast and easy way to set up data to be gridded. You will be asked questions about what you're trying to accomplish, and we'll guide you \
through the needed parts.\r\r\n\r\r\nProceed from panel to panel pressing the \"Next-->\" button whenever you are finished answering the questions \
on each panel. At any time you can backtrack with the \"<--Back\" button without losing any settings you have made along the way. If at some point \
the \"Next-->\" button does not respond then there is some question unanswered on the current panel. It must be answered before you can continue. \
\r\r\n\r\r\nIt is important to note that if you cancel the Wizard or close the window before you execute the \"Finish\" \
command on the last panel, no changes will be made to your VNS project. Once you hit the \"Finish\" button there will be changes to your project \
even if the Wizard operation can not be completed successfully for some reason. With this in mind it is a good idea to save the project now, \
before you begin the Wizard operation.  If you save your project, you'll be taken automatically to the next wizard page.",
	// WCS_GRIDDERWIZ_ADDPAGE - add the text for the page in the same order it is in the list of page defines in GridderWiz.h
	};

unsigned short GridderWizPageResourceID[WCS_GRIDDERWIZ_NUMPAGES] = 
	{
	IDD_GRIDDERWIZ_BOUNDS,
	IDD_GRIDDERWIZ_CANCEL,
	IDD_GRIDDERWIZ_COMPLETE,
	IDD_GRIDDERWIZ_COMPLETEERROR,
	IDD_GRIDDERWIZ_COORDSYS,
	IDD_GRIDDERWIZ_DATALOADED,
	IDD_GRIDDERWIZ_DEMNAME,
	IDD_GRIDDERWIZ_PAGEERROR,
	IDD_GRIDDERWIZ_SELECTDATA,
	IDD_GRIDDERWIZ_WELCOME,
	// WCS_GRIDDERWIZ_ADDPAGE - add the panel resource ID in the same order it is in the list of page defines in GridderWiz.h
	};

/*===========================================================================*/

GridderWiz::GridderWiz()
{
unsigned short pageCt;

cancelOrder = 0;
finalOrderCancelled = 0;
reviewOrder = 0;

for (pageCt = 0; pageCt < WCS_GRIDDERWIZ_NUMPAGES; pageCt ++)
	{
	ConfigurePage(&Wizzes[pageCt], pageCt);
	} // for

Wizzes[WCS_GRIDDERWIZ_WIZPAGE_WELCOME].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_DATALOADED]);
Wizzes[WCS_GRIDDERWIZ_WIZPAGE_DATALOADED].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA]);
Wizzes[WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_COORDSYS]);
Wizzes[WCS_GRIDDERWIZ_WIZPAGE_COORDSYS].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_BOUNDS]);
Wizzes[WCS_GRIDDERWIZ_WIZPAGE_BOUNDS].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_DEMNAME]);
Wizzes[WCS_GRIDDERWIZ_WIZPAGE_DEMNAME].AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_COMPLETE]);

} // GridderWiz::GridderWiz

/*===========================================================================*/

GridderWiz::~GridderWiz()
{

} // GridderWiz::~GridderWiz

/*===========================================================================*/

WizardlyPage *GridderWiz::ProcessPage(WizardlyPage *ProcessMe, Project *CurProj)
{
WizardlyPage *wp;

if (cancelOrder && !finalOrderCancelled)
	{
	ProcessMe->AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_CANCEL]);
	wp = ProcessMe->Next;
	} // if
else switch (ProcessMe->WizPageID)
	{
	case WCS_GRIDDERWIZ_WIZPAGE_BOUNDS:
		//ProcessMe->AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_DEMNAME]);
		wp = ProcessMe->Next;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_CANCEL:
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
	case WCS_GRIDDERWIZ_WIZPAGE_COMPLETE:
		wp = NULL;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_COMPLETEERROR:
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
	case WCS_GRIDDERWIZ_WIZPAGE_DATALOADED:
		//ProcessMe->AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA]);
		wp = ProcessMe->Next;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_DEMNAME:
		wp = ProcessMe->Next;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR:
		wp = NULL;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA:
		//ProcessMe->AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_COORDSYS]);
		wp = ProcessMe->Next;
		break;
	case WCS_GRIDDERWIZ_WIZPAGE_WELCOME:
		wp = ProcessMe->Next;
		break;
	// below need to be coded still
	case WCS_GRIDDERWIZ_WIZPAGE_COORDSYS:
		ProcessMe->AddNext(&Wizzes[WCS_GRIDDERWIZ_WIZPAGE_BOUNDS]);
		wp = ProcessMe->Next;
		break;
	default:
		{
		cancelOrder = finalOrderCancelled = 1;
		wp = &Wizzes[WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR];
		break;
		} // default
	// WCS_GRIDDERWIZARD_ADDPAGE - add a case to handle the page when the page is being processed. If no case exists an error page will be displayed
	} // else switch

return(wp);

} // GridderWiz::ProcessPage

/*===========================================================================*/

int GridderWiz::ConfigurePage(WizardlyPage *ConfigMe, unsigned short ConfigPage)
{
int rVal = 0;

if (ConfigPage < WCS_GRIDDERWIZ_NUMPAGES)
	{
	ConfigMe->WizPageID = ConfigPage;
	ConfigMe->Text = GridderWizPageText[ConfigPage];
	ConfigMe->WizPageResourceID = GridderWizPageResourceID[ConfigPage];
	rVal = 1;
	} // if

return(rVal);

} // GridderWiz::ConfigurePage

/*===========================================================================*/

void GridderWiz::EndOrderCancel(void)
{

// clean up any resources used

} // GridderWiz::EndOrderCancel

/*===========================================================================*/

int GridderWiz::EndOrderComplete(Project *CurProj)
{
int Success = 0;

return (Success);

} // GridderWiz::EndOrderComplete

/*===========================================================================*/
