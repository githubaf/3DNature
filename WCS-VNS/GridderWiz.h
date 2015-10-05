// GridderWiz.h
// Header file for GridderWiz
// Created from MergerWiz.h on 03/20/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#ifndef WCS_GRIDDERWIZ_H
#define WCS_GRIDDERWIZ_H

enum GWiz_Pages
	{
	WCS_GRIDDERWIZ_WIZPAGE_BOUNDS,
	WCS_GRIDDERWIZ_WIZPAGE_CANCEL,
	WCS_GRIDDERWIZ_WIZPAGE_COMPLETE,
	WCS_GRIDDERWIZ_WIZPAGE_COMPLETEERROR,
	WCS_GRIDDERWIZ_WIZPAGE_COORDSYS,
	WCS_GRIDDERWIZ_WIZPAGE_DATALOADED,
	WCS_GRIDDERWIZ_WIZPAGE_DEMNAME,
	WCS_GRIDDERWIZ_WIZPAGE_PAGEERROR,
	WCS_GRIDDERWIZ_WIZPAGE_SELECTDATA,
	WCS_GRIDDERWIZ_WIZPAGE_WELCOME,
	// WCS_GRIDDERWIZARD_ADDPAGE - add the basic page define in any order you want.
	WCS_GRIDDERWIZ_NUMPAGES
	};

#include "WizardlyPage.h"
#include "PathAndFile.h"

class GridderWiz
	{
	public:
		WizardlyPage Wizzes[WCS_GRIDDERWIZ_NUMPAGES];
		char cancelOrder, finalOrderCancelled, reviewOrder;

		GridderWiz();
		~GridderWiz();
		WizardlyPage *ProcessPage(WizardlyPage *ProcessMe, Project *CurProj);
		int ConfigurePage(WizardlyPage *ConfigMe, unsigned short ConfigPage);
		void EndOrderCancel(void);
		int EndOrderComplete(Project *CurProj);

	}; // class GridderWiz

#endif // WCS_GRIDDERWIZ_H
