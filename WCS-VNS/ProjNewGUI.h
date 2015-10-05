// ProjNewGUI.h
// Header file for ...Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_PROJNEW_H
#define WCS_PROJNEW_H

//class Project;
//class Database;
//class EffectsLib;
//class ImageLib;

#include "Application.h"
#include "Fenetre.h"
#include "Project.h"

// Very important that we list the base classes in this order.
class ProjNewGUI : public WCSModule, public GUIFenetre
	{
	private:
		NewProject ProjNew;

		
	public:

		int ConstructError;
		long ActiveItem;

		ProjNewGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, ImageLib *ImageSource);
		~ProjNewGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);
		void BuildTemplateList(void);
		void BuildTemplateListEntry(char *ListName, Template *Me);
		void DisableWidgets(void);
		int CreateNewProject(void);
		void AddTemplate(void);
		void RemoveTemplate(void);
		void ChangeTemplateListPosition(int MoveUp);
		void EnableTemplate(int NewState);
		void SetActiveTemplate(void);

	}; // class ProjNewGUI

#endif // WCS_PROJNEW_H
