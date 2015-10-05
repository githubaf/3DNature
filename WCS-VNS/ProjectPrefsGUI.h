// ProjectPrefsGUI.h
// Header file for Project Preferences Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_PROJECTPREFSGUI_H
#define WCS_PROJECTPREFSGUI_H

//class Project;
class InterCommon;
struct DirList;

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"


// Very important that we list the base classes in this order.
class ProjectPrefsGUI : public WCSModule, public GUIFenetre
	{
	private:
		Project *HostProj;
		InterCommon *InterStash;
		static char *TabNames[8];
		static long ActivePage;
		#ifdef _WIN32
		HCURSOR Curse;
		#endif // _WIN32
		short MoveInProgress;
		short SwapInProgress;
		int ActiveItem;
		struct DirList *DLCopy;
		char Dirname[256];
		AnimDoubleTime RefElevADT, RefLatADT, RefLonADT;

	public:

		int ConstructError;

		ProjectPrefsGUI(Project *Moi, InterCommon *ISource);
		~ProjectPrefsGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SyncFrameDrop(void);
		void SetFrameRate(void);
		void SetDistanceUnits(void);
		void SetHeightUnits(void);
		void SetAngleUnits(void);
		void SetTimeUnits(void);
		void SetPosLonHemi(void);
		void SetLatLonDisplay(void);
		void SetGeoProjDisplay(void);
		void SetViewport(short ViewportID);
		short BuildList(int Active);
		void DoKeep(void);
		void DoCancel(void);
		void DoDefaultDir(void);
		void DoAdd(void);
/*
		void DoSwap(short EndIt);
		void DoMove(short EndIt);
*/
		void HandleMove(int Direction);
		void DoRemove(void);
		void DoReadOnly(void);
		void DoLoad(void);
		void DoActiveChange(void);
		void DoAdvConfig(int DoWhat);
		static void SetActivePage(long NewPage) {ActivePage = NewPage;};

	}; // class ProjectPrefsGUI

#endif // WCS_PROJECTPREFSGUI_H
