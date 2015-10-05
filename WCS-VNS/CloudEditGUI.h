// CloudEditGUI.h
// Header file for Cloud Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_CLOUDEDITGUI_H
#define WCS_CLOUDEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"
// material GUI
#include "PortableMaterialGUI.h"

class EffectsLib;
class Database;
class CloudEffect;
class Raster;
class DiagnosticData;
// material GUI
class PortableMaterialGUI;
class CloudEditGUI;
class CloudEditGUIPortableMaterialGUINotifyFunctor;

#define WCS_CLOUDGUI_NUMTABS	6

// material GUI
class CloudEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		CloudEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // CloudEditGUIPortableMaterialGUINotifyFunctor

// Very important that we list the base classes in this order.
class CloudEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		CloudEffect *Active, Backup;
		GradientCritter *ActiveGrad;
		WaveSource *ActiveWave;
		Raster *Rast, *WaveRast;
		TextureSampleData *SampleData, *WaveSampleData;
		// material GUI
		PortableMaterialGUI *MatGUI;
		CloudEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;

		static char *TabNames[WCS_CLOUDGUI_NUMTABS];
		unsigned char BackgroundInstalled, EvolveFast, ReceivingDiagnostics, ReceivingWave, ReceivingWaveSource;
		double LatEvent[2], LonEvent[2];
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		CloudEditGUI(EffectsLib *EffectsSource, Database *DBSource, CloudEffect *ActiveSource);
		~CloudEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleBackgroundCrunch(int Siblings);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SelectPanel(short PanelID);
		CloudEffect *GetActive(void) {return (Active);};
		// material GUI
		void ConfigureColors(void); // so we can call from CloudEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode) {ActiveGrad = NewNode;}; // so we can call from CloudEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowMaterialPopDrop(false);}; // hide popdrop when necessary
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void ConfigureLayerThickness(void);
		void BuildWaveList(void);
		void BuildWaveListEntry(char *ListName, WaveSource *Me);
		WaveSource *ActiveWaveValid(void);
		void ConfigureWave(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void ZoomScale(char InOrOut);
		void ZoomWaveScale(char InOrOut);
		void CloudType(void);
		void AddWave(void);
		void SetSourcePosInView(DiagnosticData *Data);
		void AddWaveInView(DiagnosticData *Data);
		void RemoveWave(void);
		void SetActiveWave(void);
		void UpdateThumbnail(void);
		void DoEvolution(void);
		void SetBounds(DiagnosticData *Data);
		void CopyWave(void);
		void PasteWave(void);

	}; // class CloudEditGUI

#endif // WCS_CLOUDEDITGUI_H
