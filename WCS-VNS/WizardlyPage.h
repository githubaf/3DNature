// WizardlyPage.h
// Header file for WizardlyPage
// Created from scratch on 2/3/04 by Gary R. Huber
// Copyright 2004 Questar Productions. All rights reserved.

#ifndef WCS_WIZARDLYPAGE_H
#define WCS_WIZARDLYPAGE_H


class WizardlyPage
	{
	public:
		unsigned short WizPageID, WizPageResourceID;
		char *Text;
		WizardlyPage *Prev, *Next, *RevertChain, *NextRevert;

		WizardlyPage();
		~WizardlyPage()	{};
		void AddNext(WizardlyPage *InsertNext);
		void Revert(void);

	}; // class WizardlyPage

#endif // WCS_WIZARDLYPAGE_H

