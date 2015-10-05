// EcotypeEditGUI.h
// Header file for Ecotype Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_ECOTYPEEDITGUI_H
#define WCS_ECOTYPEEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "Ecotype.h"

class ImageLib;
class EffectsLib;
class Ecotype;

#define WCS_ECOTYPEGUI_NUMTABS	4

// Very important that we list the base classes in this order.
class EcotypeEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		ImageLib *ImageHost;
		EffectsLib *EffectsHost;
		Ecotype *Active, Backup;
		FoliageGroup *ActiveGroup;
		Foliage *ActiveFol;
		static char *TabNames[WCS_ECOTYPEGUI_NUMTABS];
		static long ActivePage;
		char OldType;

	public:

		int ConstructError;

		EcotypeEditGUI(ImageLib *ImageSource, EffectsLib *EffectsSource, Ecotype *ActiveSource);
		~EcotypeEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Ecotype *GetActive(void) {return (Active);};

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	private:

		FoliageGroup *ActiveFoliageValid(void);
		void ConfigureGroup(void);
		void ConfigureFoliage(void);
		void SyncWidgets(void);
		FoliageGroup *BuildGroupList(void);
		void BuildGroupListEntry(char *ListName, FoliageGroup *Me);
		Foliage *BuildFoliageList(void);
		void BuildFoliageListEntry(char *ListName, Foliage *Me);
		void DisableWidgets(void);
		void SelectPanel(long PanelID);
		void Cancel(void);
		void SelectNewImage(long Current);
		void SelectNewObject(long Current);
		void SetActiveGroup(void);
		void SetActiveFoliage(void);
		void EditFoliageObject(void);
		void SetDensityUnits(void);
		void SetBAUnits(unsigned short WidID);
		void GroupName(void);
		void EnableTexture(int TexNumber, unsigned short WidID);
		void EditTexture(int TexNumber);
		void RemoveTexture(int TexNumber);
		void RemoveGroup(void);
		void RemoveFoliage(void);
		void AddGroup(void);
		void AddFoliage(void);
		void ChangeGroupListPosition(short MoveUp);
		void ChangeFoliageListPosition(short MoveUp);
		void OpenPreview(void);
		void ApplyBackLightToGroup(void);
		void CopyMaterial(int IsGroup);
		void PasteMaterial(int IsGroup);

	}; // class EcotypeEditGUI

#endif // WCS_ECOTYPEEDITGUI_H
