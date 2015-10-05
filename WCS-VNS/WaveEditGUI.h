// WaveEditGUI.h
// Header file for Wave Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_WAVEEDITGUI_H
#define WCS_WAVEEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "CommonComponentEditor.h"

class EffectsLib;
class Database;
class WaveEffect;
class WaveSource;
class DiagnosticData;
class TextureSampleData;
class Raster;

#define WCS_WAVEGUI_NUMTABS	4

// Very important that we list the base classes in this order.
class WaveEditGUI : public WCSModule, public GUIFenetre, public CommonComponentEditor
	{
	private:
		EffectsLib *EffectsHost;
		Database *DBHost;
		WaveEffect *Active, Backup;
		WaveSource *ActiveWave;
		Raster *WaveRast;
		TextureSampleData *WaveSampleData;
		static char *TabNames[WCS_WAVEGUI_NUMTABS];
		unsigned char BackgroundInstalled, ReceivingDiagnostics, ReceivingWave, ReceivingWaveSource;
		double LatEvent, LonEvent;
		long NumEnvelopes;
		// advanced
		static long ActivePage, DisplayAdvanced;

	public:

		int ConstructError;

		WaveEditGUI(EffectsLib *EffectsSource, Database *DBSource, WaveEffect *ActiveSource);
		~WaveEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleBackgroundCrunch(int Siblings);
		// advanced
		long HandleShowAdvanced(NativeGUIWin NW, bool NewState);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		WaveEffect *GetActive(void) {return (Active);};
		// advanced
		bool RespondToCanShowAdvancedFeatures(void) {return(true);};
		bool QueryLocalDisplayAdvancedUIVisibleState(void);

	private:

		void BuildWaveList(void);
		void BuildWaveListEntry(char *ListName, WaveSource *Me);
		WaveSource *ActiveWaveValid(void);
		void ConfigureWave(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void SelectPanel(long PanelID);
		// advanced
		void DisplayAdvancedFeatures(void);
		void Cancel(void);
		void Name(void);
		void ZoomWaveScale(char InOrOut);
		void SetPosInView(DiagnosticData *Data);
		void SetSourcePosInView(DiagnosticData *Data);
		void AddWave(void);
		void AddWaveInView(DiagnosticData *Data);
		void RemoveWave(void);
		void SetActiveWave(void);
		void UpdateThumbnail(void);
		void CopyWave(void);
		void PasteWave(void);

	}; // class WaveEditGUI

#endif // WCS_WAVEEDITGUI_H
