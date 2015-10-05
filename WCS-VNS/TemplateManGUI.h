// TemplateManGUI.h
// Header file for ...Editor
// Created from ProjNewGUI.h on 6/9/01 by GRH
// Copyright 2001 by Questar Productions. All rights reserved.

#ifndef WCS_TEMPLATEMAN_H
#define WCS_TEMPLATEMAN_H

//class Project;
//class Database;
//class EffectsLib;
//class ImageLib;
class Pier1;

#include "Application.h"
#include "Fenetre.h"
#include "Project.h"

// Very important that we list the base classes in this order.
class TemplateManGUI : public WCSModule, public GUIFenetre
	{
	private:
		Template Active[WCS_MAX_TEMPLATES];
		
	public:

		int ConstructError;
		long ActiveItem, NumImportItems;
		char *ImportEnabledList;
		Project *ProjHost;
		Pier1 *ActiveImport;

		TemplateManGUI(Project *ProjSource);
		~TemplateManGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);
		void ConfigureTemplate(void);
		void ConfigureImport(void);
		void BuildTemplateList(void);
		void BuildTemplateListEntry(char *ListName, Template *Me);
		void DisableWidgets(void);
		int Apply(void);
		void ChangeTemplateListPosition(int MoveUp);
		void EnableTemplate(int NewState);
		void SetActiveTemplate(void);
		void ReplaceTemplate(void);
		void BuildImportList(void);
		void BuildImportListEntry(char *ListName, Pier1 *Me);
		void EnableImport(int NewState);
		void SetActiveImport(void);
		void BuildImportEnabledList(void);
		void FreeImportEnabledList(void);
		void UndoChanges(void);

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	}; // class TemplateManGUI

#endif // WCS_TEMPLATEMAN_H
