// MergerWiz.h
// Header file for MergerWiz
// Created from ForestWiz.h on 01/18/07 by Frank Weed II
// Copyright 2007 3D Nature, LLC. All rights reserved.

#ifndef WCS_MERGERWIZ_H
#define WCS_MERGERWIZ_H

enum MWiz_Pages
	{
	WCS_MERGERWIZ_WIZPAGE_BOUNDS,
	WCS_MERGERWIZ_WIZPAGE_BOUNDS1,
	WCS_MERGERWIZ_WIZPAGE_BOUNDS2,
	WCS_MERGERWIZ_WIZPAGE_CANCEL,
	WCS_MERGERWIZ_WIZPAGE_COMPLETE,
	WCS_MERGERWIZ_WIZPAGE_COMPLETEERROR,
	WCS_MERGERWIZ_WIZPAGE_CONFIRM,
	WCS_MERGERWIZ_WIZPAGE_COORDSYS,
	WCS_MERGERWIZ_WIZPAGE_DEMSLOADED,
	WCS_MERGERWIZ_WIZPAGE_MERGENAMES,
	WCS_MERGERWIZ_WIZPAGE_MERGERES,
	WCS_MERGERWIZ_WIZPAGE_MERGETYPE,
	WCS_MERGERWIZ_WIZPAGE_PAGEERROR,
	WCS_MERGERWIZ_WIZPAGE_QUERYEDIT,
	WCS_MERGERWIZ_WIZPAGE_QUERYMAKE,
	WCS_MERGERWIZ_WIZPAGE_QUERYSTATUS,
	WCS_MERGERWIZ_WIZPAGE_SELECTDEMS,
	WCS_MERGERWIZ_WIZPAGE_WELCOME,
	// WCS_MERGERWIZARD_ADDPAGE - add the basic page define in any order you want.
	WCS_MERGERWIZ_NUMPAGES
	};

#include "WizardlyPage.h"
#include "PathAndFile.h"

class MergerWiz
	{
	public:
		WizardlyPage Wizzes[WCS_MERGERWIZ_NUMPAGES];
		char cancelOrder, finalOrderCancelled, highres, manualBounds, queryStatus, reviewOrder;

		MergerWiz();
		~MergerWiz();
		WizardlyPage *ProcessPage(WizardlyPage *ProcessMe, Project *CurProj);
		int ConfigurePage(WizardlyPage *ConfigMe, unsigned short ConfigPage);
		void EndOrderCancel(void);
		int EndOrderComplete(Project *CurProj);

	}; // class MergerWiz

#endif // WCS_MERGERWIZ_H
