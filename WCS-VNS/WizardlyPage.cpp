// WizardlyPage.cpp
// Code file for WizardlyPage
// Created from scratch on 2/3/04 by Gary R. Huber
// Copyright 2004 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "WizardlyPage.h"

WizardlyPage::WizardlyPage()
{

Prev = Next = RevertChain = NextRevert = NULL;
WizPageID = WizPageResourceID = 0;
Text = NULL;

} // WizardlyPage::WizardlyPage

/*===========================================================================*/

void WizardlyPage::AddNext(WizardlyPage *InsertNext)
{
WizardlyPage *Current = Next;

// can't add something that is already in the loop
while (Current)
	{
	if (Current == InsertNext)
		return;
	Current = Current->Next;
	} // while

if (Next)
	Next->Prev = InsertNext;
InsertNext->Next = Next;
InsertNext->Prev = this;
Next = InsertNext;

InsertNext->NextRevert = RevertChain;
RevertChain = InsertNext;

} // WizardlyPage::AddNext

/*===========================================================================*/

void WizardlyPage::Revert(void)
{
WizardlyPage *Current;

while (RevertChain)
	{
	for (Current = Next; Current; Current = Current->Next)
		{
		if (Current == RevertChain)
			{
			if (Current->Prev)
				Current->Prev->Next = Current->Next;
			if (Current->Next)
				Current->Next->Prev = Current->Prev;
			break;
			} // if
		} // for
	Current = RevertChain;
	RevertChain = RevertChain->NextRevert;
	Current->NextRevert = NULL;
	} // while

} // WizardlyPage::Revert

/*===========================================================================*/
