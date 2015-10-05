// EffectsLibGUI.h
// Header file for Effects Library Editor
// Created from ColorEditGUI.h on 5/1/97 by CXH & GRH
// Copyright 1997 Questar Productions

#ifndef WCS_EFFECTSLIBGUI_H
#define WCS_EFFECTSLIBGUI_H

//class EffectsLib;
//class ImageLib;
//class Database;
//class Project;
class GeneralEffect;
class DigitizeGUI;
class Joe;

#include "Application.h"
#include "Fenetre.h"


// Very important that we list the base classes in this order.
class EffectsLibGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *HostLib, *BrowseLib;
		ImageLib *BrowseImageLib;
		Project *HostProj;
		Database *HostDB;
		int Frozen;

	public:

		int ConstructError;

		EffectsLibGUI(EffectsLib *LibSource, Project *ProjSource, Database *DBSource);
		~EffectsLibGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void BuildList(EffectsLib *SourceLib, WIDGETID ListView);
		void BuildListEntry(char *ListName, GeneralEffect *Me);
		void NewEffect(void);
		void ModifyEffect(void);
		void RemoveEffect(void);
		long GetEffectClassFromListEntry(long Selected, GeneralEffect *&Test, char *Category);
		void SortEffects(void);
		void RemoveAllEffects(void);
		void CloseDangerousWindows(long EffectClass);
		long FindInList(GeneralEffect *FindMe, WIDGETID ListID);
		void ResetListNames(void);
		void ResetListEnabled(void);
		void ResetListJoes(void);
		long ApplyToActive(Joe *NewVec);
		void CreateVecApplyToActive(void);
		void BrowseFile(void);
		void SelectAll(void);
		void CopyToProj(void);
		void Freeze(void)	{Frozen = 1;};
		void UnFreeze(void)	{Frozen = 0; ConfigureWidgets();};
		bool GetFrozen(void) const {return(Frozen == 1);};

	}; // class EffectsLibGUI

#endif // WCS_EFFECTSLIBGUI_H
