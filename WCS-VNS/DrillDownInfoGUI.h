// DrillDownInfoGUI.h
// Header file for DrillDownInfo Editor
// Created from scratch on 4/14/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#ifndef WCS_DRILLDOWNINFOGUI_H
#define WCS_DRILLDOWNINFOGUI_H

#include "Application.h"
#include "Fenetre.h"

class EffectsLib;
class Database;
class JoeList;
class Joe;
class JoeAttribute;
class GeneralEffect;

#define WCS_DRILLDOWNGUI_NUMTABS	2

// Very important that we list the base classes in this order.
class DrillDownInfoGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		JoeList *Joes;
		static char *TabNames[WCS_DRILLDOWNGUI_NUMTABS];
		static long ActivePage;


	public:

		int ConstructError;

		DrillDownInfoGUI(EffectsLib *EffectsSource, Database *DBSource, JoeList *JoeSource);
		~DrillDownInfoGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
		void SetNewJoeList(JoeList *JoeSource);
		void DisposeJoeList(void);
		void ConfigureWidgets(void);
		void AddVectorEntry(Joe *CurVec);
		long AddEffectToList(JoeAttribute *Me);
		void BuildEffectListEntry(char *ListName, GeneralEffect *Me, char *EffectType);
		void AddOtherEntries(void);
		void BuildOtherListEntry(char *ListName, GeneralEffect *Me);
		void EditItem(int CtrlID);

	}; // class DrillDownInfoGUI

#endif // WCS_DRILLDOWNINFOGUI_H
