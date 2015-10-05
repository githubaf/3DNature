// Toolbar.h
// Header file for icon toolbar
// Created from scratch (and from RenderStatus.h) on 12/13/95 by CXH
// Copyright 1995

#ifndef WCS_TOOLBAR_H
#define WCS_TOOLBAR_H

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class Project;
class RasterAnimHost;

#include "Application.h"
#include "Fenetre.h"
#include "Notify.h"
#include "WCSWidgets.h"
#include "Security.h"
#include "GraphData.h"

#define WCS_TOOLBAR_ICONSPACE	2	// Between each icon
#define WCS_TOOLBAR_HMARGIN		4
#define WCS_TOOLBAR_VMARGIN		1
#define WCS_TOOLBAR_FRAMESLIDERHEIGHTSM	9
#define WCS_TOOLBAR_FRAMESLIDERHEIGHTLG	18

// stuff for adding and removing open windows from menu
#define WCS_WINDOWLIST_MINID	41000
#define WCS_WINDOWLIST_MAXNUMWINS	25
enum
	{
	WCS_MAINMENUNUMBER_FILE,
	WCS_MAINMENUNUMBER_VIEW,
	WCS_MAINMENUNUMBER_DATA,
	WCS_MAINMENUNUMBER_WINDOW,
	WCS_MAINMENUNUMBER_HELP
	}; // menu number

// Notify class
#define WCS_TOOLBARCLASS_MODULES		250
//#define WCS_TOOLBARCLASS_MENUACTIONS	251 // now retired (did it ever do much?)


// Subclass indicates what you want to do with the window (Modules class)
enum
	{
	WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_OPEN_MOD,
	WCS_TOOLBAR_REQUEST_CLOSE_MOD
	}; // class

// Items that can be opened or closed (Modules Class, Open or Close SubClass)
enum
	{
	WCS_TOOLBAR_ITEM_RSG = 1,
	WCS_TOOLBAR_ITEM_PEG,
	WCS_TOOLBAR_ITEM_MEG,
	WCS_TOOLBAR_ITEM_CEG,
	WCS_TOOLBAR_ITEM_EEG,
	WCS_TOOLBAR_ITEM_CLG,
	WCS_TOOLBAR_ITEM_WEG,
	WCS_TOOLBAR_ITEM_CWG,
	WCS_TOOLBAR_ITEM_DBM,
	WCS_TOOLBAR_ITEM_DOM, // RETIRED
	WCS_TOOLBAR_ITEM_PAM,
	WCS_TOOLBAR_ITEM_KSG,
	WCS_TOOLBAR_ITEM_SPG,
	WCS_TOOLBAR_ITEM_DBG,
	WCS_TOOLBAR_ITEM_LWG,
	WCS_TOOLBAR_ITEM_MVG,
	WCS_TOOLBAR_ITEM_FEG,
	WCS_TOOLBAR_ITEM_DLG,
	WCS_TOOLBAR_ITEM_PPG,
	WCS_TOOLBAR_ITEM_ISG,
	WCS_TOOLBAR_ITEM_IDG,
	WCS_TOOLBAR_ITEM_PNG,
	WCS_TOOLBAR_ITEM_DED, // retired
	WCS_TOOLBAR_ITEM_DEB,
	WCS_TOOLBAR_ITEM_DIG,
	WCS_TOOLBAR_ITEM_TLG,
	WCS_TOOLBAR_ITEM_CVG, // entire (Cam)ViewGUI
	WCS_TOOLBAR_ITEM_ELG,
	WCS_TOOLBAR_ITEM_DXG, // retired
	WCS_TOOLBAR_ITEM_VER,
	WCS_TOOLBAR_ITEM_DRG, // DEM Random GUI - retired
	WCS_TOOLBAR_ITEM_MLG,
	WCS_TOOLBAR_ITEM_RDG,
	WCS_TOOLBAR_ITEM_APG, // retired
	WCS_TOOLBAR_ITEM_CRG, // retired
	WCS_TOOLBAR_ITEM_SIM,
	WCS_TOOLBAR_ITEM_LEG,
	WCS_TOOLBAR_ITEM_EFG,
	WCS_TOOLBAR_ITEM_ECG,
	WCS_TOOLBAR_ITEM_IEG,
	WCS_TOOLBAR_ITEM_GPG,
	WCS_TOOLBAR_ITEM_RTG,
	WCS_TOOLBAR_ITEM_TAG,
	WCS_TOOLBAR_ITEM_SHG,
	WCS_TOOLBAR_ITEM_NPG,
	WCS_TOOLBAR_ITEM_CPG,
	WCS_TOOLBAR_ITEM_TPG,
	WCS_TOOLBAR_ITEM_KPG,
	WCS_TOOLBAR_ITEM_WPG,
	WCS_TOOLBAR_ITEM_LPG,
	WCS_TOOLBAR_ITEM_XPG,	// texture editor - not used
	WCS_TOOLBAR_ITEM_FLG,
	WCS_TOOLBAR_ITEM_SEG,
	WCS_TOOLBAR_ITEM_KDG,
	WCS_TOOLBAR_ITEM_IMD,   // Old OneStopImport -- retired
	WCS_TOOLBAR_ITEM_STL,
	WCS_TOOLBAR_ITEM_VEG,
	WCS_TOOLBAR_ITEM_DEG,
	WCS_TOOLBAR_ITEM_VPG,
	WCS_TOOLBAR_ITEM_DFG, // defaultGUI -- retired
	WCS_TOOLBAR_ITEM_OEG,
	WCS_TOOLBAR_ITEM_MAG,
	WCS_TOOLBAR_ITEM_DBO,
	WCS_TOOLBAR_ITEM_ILG,
	WCS_TOOLBAR_ITEM_GRG,
	WCS_TOOLBAR_ITEM_TVG, // TrackView -- retired
	WCS_TOOLBAR_ITEM_DKG,
	WCS_TOOLBAR_ITEM_TXG,
	WCS_TOOLBAR_ITEM_PUW, // Project Update Wizard - retired (now reinstated)
	WCS_TOOLBAR_ITEM_SAG,
	WCS_TOOLBAR_ITEM_DDL,
	WCS_TOOLBAR_ITEM_AEG,
	WCS_TOOLBAR_ITEM_CMG,
	WCS_TOOLBAR_ITEM_ENG,
	WCS_TOOLBAR_ITEM_GNG,
	WCS_TOOLBAR_ITEM_POG,
	WCS_TOOLBAR_ITEM_RJG,
	WCS_TOOLBAR_ITEM_ROG,
	WCS_TOOLBAR_ITEM_SNG,
	WCS_TOOLBAR_ITEM_STG,
	WCS_TOOLBAR_ITEM_MSG,
	WCS_TOOLBAR_ITEM_LVE,
	WCS_TOOLBAR_ITEM_BRD,
	WCS_TOOLBAR_ITEM_IVG,
	WCS_TOOLBAR_ITEM_RCG,
	WCS_TOOLBAR_ITEM_EFL,
	WCS_TOOLBAR_ITEM_IWG,
	WCS_TOOLBAR_ITEM_NUM,
	WCS_TOOLBAR_ITEM_GRD,
	WCS_TOOLBAR_ITEM_TGN,
	WCS_TOOLBAR_ITEM_SQU,
	WCS_TOOLBAR_ITEM_THM,
	WCS_TOOLBAR_ITEM_COS,
	WCS_TOOLBAR_ITEM_TBG,
	WCS_TOOLBAR_ITEM_DEM,
	WCS_TOOLBAR_ITEM_FCG,
	WCS_TOOLBAR_ITEM_TPM,
	WCS_TOOLBAR_ITEM_PTH,
	WCS_TOOLBAR_ITEM_DSS,
	WCS_TOOLBAR_ITEM_PPR,
	WCS_TOOLBAR_ITEM_DPG,
	WCS_TOOLBAR_ITEM_MRG,
	WCS_TOOLBAR_ITEM_SCN,
	WCS_TOOLBAR_ITEM_VPX,
	WCS_TOOLBAR_ITEM_CSC,
	WCS_TOOLBAR_ITEM_DRL,
	WCS_TOOLBAR_ITEM_EXP,
	WCS_TOOLBAR_ITEM_EXG,
	WCS_TOOLBAR_ITEM_AUT,
	WCS_TOOLBAR_ITEM_FWZ,
	WCS_TOOLBAR_ITEM_DWZ, // Diner Wizard retired
	WCS_TOOLBAR_ITEM_LBL,
	WCS_TOOLBAR_ITEM_MWZ,
	WCS_TOOLBAR_ITEM_GWZ,
	WCS_TOOLBAR_ITEM_VIW, // a single ViewGUI View, specified by the Page code
	WCS_TOOLBAR_ITEM_LAST_DONT_USE_ME // to keep comma insanity at bay
// <<<>>> ADD_NEW_EFFECTS add an ID for toolbar operations at the end of the list - 
// DO NOT EVER remove items from this list or add others any place but at the end! 
// These ID's are stored in the project file.
	//WCS_TOOLBAR_ITEM_,
	}; // class


class Toolbar : public SetCritter, public NotifyEx, public WCSModule, public GUIFenetre
	{
	friend class WCSApp;

	private:
		double LastFrameRedrawTime;
		void *MainMenu;		// cast as necessary
		char CurrentProgressText[80], CurrentStatusText[80];
		char BusyWinNest, CurrentlyPlaying;
		AnimDoubleTime FrameADT;

		int ButtonCount, FrameSliderHeight;
		int XAxisID, YAxisID, ZAxisID, HAxisID, PAxisID, BAxisID, NextKeyID, StatusLogID;
		NativeControl TopTB, BotTB, FrameScrub, FrameSlider, FrameBorder,
		 ProgressText, ProgressBar, ObjectText, StatusText, PlayButton;
		bool Disabled_TimeLine, Disabled_KeyAdd, Disabled_PrevKey, Disabled_NextKey;

		#ifdef _WIN32
		int BottomOfModulesLineY, BottomOfAnimLineY;
		HFONT CommonFont;
		#endif // _WIN32

		HWND CreateTopToolbar(HWND hwndParent, HINSTANCE g_hinst);
		HWND CreateBottomToolbar(HWND hwndParent, HINSTANCE g_hinst);
		HIMAGELIST toolBarIL;
		bool AddToolbarImageIcon(NativeControl Bar, char *HelpCapt, WORD IconID, unsigned long ImageNum, bool Tog, bool StartHidden = false);
		NativeControl AddToolbarPlayIcon(NativeControl Bar, char *HelpCapt, WORD Icon, int Y);
		bool AddToolbarTextButton(NativeControl Bar, char *String, char *HelpCapt, char Tog = NULL);
		bool AddToolbarDivider(NativeControl Bar, unsigned long Width = 10);
		NativeControl AddToolbarStatus(NativeControl Bar, char MaxLetters, WORD Id, bool MaximizeRemainingWidth, bool AutoPadToolbar, bool StartHidden);
		NativeControl AddToolbarProgress(NativeControl Bar, char MaxLetters, WORD Id, bool MaximizeRemainingWidth, bool AutoPadToolbar, bool StartHidden);
		NativeControl AddToolbarFrameScrub(NativeControl Bar);
		NativeControl AddToolbarFrameSlider(NativeControl Bar, int Y);

		void UpdateStatus(NativeControl, WORD Id, char *StatText);
		void UpdateProgress(NativeControl, WORD Id, float Percent);
		void UpdateProgress(NativeControl, WORD Id, int Percent);
		void ConfigureMatrix(void);

		long HandleModule(unsigned char Item, unsigned char OpenState, unsigned char Page);
		void UpdateBusy(char MoreBusy);


		GUIFenetre *WindowsInMenuList[WCS_WINDOWLIST_MAXNUMWINS];

#ifdef _WIN32
	HACCEL KeyAbbrev;
	HWND FetchWinHandle(void);
#endif // _WIN32

	public:
		Toolbar(WCSApp *AppSource);
		~Toolbar();

		// Don't need this one...
		NativeGUIWin Open(Project *Proj);

		NativeGUIWin Construct(void);

		long HandleEvent(void);
		void HandleNotifyEvent(void);

		long HandleRepaint(NativeGUIWin NW, NativeDrawContext NDC);
		long HandleInquireScrollColor(NativeControl Widget);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleMenuSelect(int MenuID);
		long HandlePopupMenuSelect(int MenuID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleBackgroundCrunch(int Siblings); // return 1 when complete
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);

		unsigned long NotifyFromID(unsigned long WinID);
		unsigned long MajorWindowNotifyFromID(unsigned long WinID);
		void OpenWindows(Project *Proj);
		void OpenAWindow(unsigned long WinID);
		void RequestWindowClose(unsigned long WinID);

		virtual void SetParam(int Notify, ...);
		virtual void GetParam(void *Value, ...);

		void CloseDangerousWindows(void);

		void ConfigureAnimWidgets(void);
		long BuildGroupList(void);
		void EditParam(void);
		void OpenTimeLine(void);
		void KeyGroupMode(void);
		void KeyFrameUpdate(int UpdateAll);
		void KeyFrameAdd(int SilentNow = 0); // SilentNow indicates don't ank for time, use now moment
		void KeyFrameRemove(void); // no longer takes an arg, ignored it in V5 anyway
		void PlayMode(void);
		void KeyFrameScale(void);
		void FrameNumber(void);
		void NextKeyFrame(short Direction);
		void FrameScroll(void);

		char QueryCurrentlyPlaying(void) {return(CurrentlyPlaying);};
		char GetAvailableGraphNumber(void);
		char GetAvailableDragnDropListNumber(void);
		long GetMaxFrame(void);
		double GetRealMaxTime(void);
		void UpdateRecentProjects(void);
		void AddWindowToMenuList(GUIFenetre *AddMe);
		void RemoveWindowFromMenuList(GUIFenetre *RemoveMe);
		void RealAddWindowToMenuList(GUIFenetre *AddMe);
		void RealRemoveWindowFromMenuList(GUIFenetre *RemoveMe);
		void SetCurrentProgressText(char *NewText);
		void SetCurrentProgressAmount(float Val);
		void SetCurrentProgressAmount(int Val);
		void ShowProgress(void);
		void ClearProgress(void);
		void HideProgress(void);
		void SetCurrentStatusText(const char *NewText);
		void ClearStatus(void);
		void SetCurrentNoteText(char *NewText);
		void ClearNote(void);
		void SetCurrentObjectText(char *NewText);
		void ClearObject(void);
		void SetCurrentObject(RasterAnimHost *NewActive, int CheckSAG);
		unsigned long GetWCSInstanceAsNumber(void)	{return ((unsigned long)LocalWinSys()->Instance());};
		void EnforceTaskModeWindowCompliance(long NewMode);
		void CloseAllWindowMenuWindows(void);
		void NumericValue(RasterAnimHost *RAH = NULL);
		void IncBusy(void) {if(!BusyWinNest++) UpdateBusy(1);}; // These are more clever than you might first think
		void DecBusy(void) {if(BusyWinNest--) UpdateBusy(0);};  // Note: Post-test increment/decrement
		char LookBusy(void) {return(BusyWinNest);};
		void RemoveCurrentUser(void);

		void SetToolbarButtonHidden(WORD IconID, bool NewHiddenState);
		void SetToolbarButtonPressed(WORD IconID, bool NewState);
		bool GetToolbarButtonPressed(WORD IconID);
		void SetToolbarButtonDisabled(WORD IconID, bool NewDisabledState);
		void ConfigureToolbarAxis(bool RotationMode);

	}; // class Toolbar

#endif // WCS_TOOLBAR_H
