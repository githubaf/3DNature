// StartupWin.h
// Header file for Startup window
// built from VersionGUI.h on March 22 2002 by CXH
// Created from ProjectPrefsGUI.h on 6/15/96 by CXH
// Copyright 1996

#ifndef WCS_STARTUPWIN_H
#define WCS_STARTUPWIN_H

#include "Application.h"
#include "Fenetre.h"
#include "WCSWidgets.h"
#include "AppHelp.h"
#include "Raster.h"
#include "Effectslib.h"

class BrowseData;
class Raster;

#define WCS_VERSION_LOGO_IMAGE_W 214
#define WCS_VERSION_LOGO_IMAGE_H 171

#define WCS_VERSIONGUI_NUMNAILS	6
#define WCS_STARTUPGUINEWPAGE_NUMTABS	6
#define WCS_STARTUPGUINEWPAGE_NUMTYPES	5

#define WCS_STARTUPGUINEWPAGE_TYPE_TERRAIN	0
#define WCS_STARTUPGUINEWPAGE_TYPE_GROUND	1
#define WCS_STARTUPGUINEWPAGE_TYPE_ENVIRON	2
#define WCS_STARTUPGUINEWPAGE_TYPE_SKY		3
#define WCS_STARTUPGUINEWPAGE_TYPE_CLOUD	4
#define WCS_STARTUPGUINEWPAGE_TYPE_MAX		5

class CombinedComponentData
	{
	public:
		CombinedComponentData *Next;
		BrowseData BD;
		char ComponentName[WCS_EFFECT_MAXNAMELENGTH];

		CombinedComponentData() {Next = NULL;};
	}; // CombinedComponentData

// Very important that we list the base classes in this order.
class VersionGUI : public WCSModule, public GUIFenetre
	{
	private:
#               ifdef _WIN32
		TrueColorIcon IconImage;
//		char IIRed[WCS_VERSION_LOGO_IMAGE_W * WCS_VERSION_LOGO_IMAGE_H],
//		 IIGrn[WCS_VERSION_LOGO_IMAGE_W * WCS_VERSION_LOGO_IMAGE_H],
//		 IIBlu[WCS_VERSION_LOGO_IMAGE_W * WCS_VERSION_LOGO_IMAGE_H];
#               endif // _WIN32

		char BuildStr[256];
		char SerialStr[19];
		char VersionAppend[30];
#               ifdef _WIN32
		unsigned char *LogoRedBuf, *LogoGrnBuf, *LogoBluBuf;
		unsigned long LogoW, LogoH;
#               endif // _WIN32
		long LastTip;
		char UpdateTip, UpdateWarned, NameChanged;
		long ActivePage;
		static long ActiveSubPage;
		short ShowThisPage;
		time_t OpenMoment;
		unsigned char BackgroundInstalled;
		HelpFileIndex HFI;
		int CurTutorialNum;
		int JustRecentConfigured;
		int MadeTutColumns;

		char **NameList, DefaultExt[8], ActiveDir[256];
		long NumItems, NumValidItems, NumRanges, ActiveRange, ActiveItem, FirstItemInRange, LastItemInRange,
			ScrollPos, DisplayedItem[WCS_VERSIONGUI_NUMNAILS];
		BrowseData *BrowseList;
		Raster *RasterList, *CompRasterList;
		HFONT TipTextFont;


		BrowseData SelectedCompData[WCS_STARTUPGUINEWPAGE_NUMTYPES];
		char SelectedCompNames[WCS_STARTUPGUINEWPAGE_NUMTYPES][60];
		Raster SelectedCompRaster[WCS_STARTUPGUINEWPAGE_NUMTYPES];
		CombinedComponentData *CCD[WCS_STARTUPGUINEWPAGE_NUMTYPES];
		int NumValidComps[WCS_STARTUPGUINEWPAGE_NUMTYPES];
		int NumCompRanges[WCS_STARTUPGUINEWPAGE_NUMTYPES];
		int CompActiveRange[WCS_STARTUPGUINEWPAGE_NUMTYPES];

		static char *TabNames[WCS_STARTUPGUINEWPAGE_NUMTABS];

		int InstallBG(void);
		int UninstallBG(void);
		void HideSplash(void);
		void ShowSplash(void);
		void CustomizeSplash(int WithSerNum);
		
	public:

		VersionGUI();
		~VersionGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleSubPageChange(long NewPageID);
		long HandleBackgroundCrunch(int Siblings);
		int LoadLogo(void);
		void UnloadLogo(void);
		void ConfigureWidgets(void);
		void SetTip(char *Tip);
		void PickAndSetTip(int WhichTip);
		void TipInfo(int WhichTip);
		void DoOSVersion(void);
		void DoUpdateMessage(void);
		void CaptureUserData(void);
		void LoadPrefs(void);
		void ConfigureMultiUser(void);
		void ConfigureTutorials(void);
		void SelectNewUser(void);
		void DisableWidgets(void);
		void SelectPage(int NewPage);
		int ScanTutorials(void);
		int SelectTutorial(int TutorialNum);
		int LaunchTutorial(int TutorialNum);
		void ConfigureProjects(int JustRecent);
		void NewProjectRange(int MoveDir);
		void ClearProjects(void);
		void ConfigureProjectImages(void);
		void SetActiveProject(long NewActive);
		void CaptureActiveProject(long NewActive);
		void ShowPageWidgets(int ShowWidgets);
		char *FormatWithPath(char *Input);
		char *FormatWithoutPath(char *Input);
		void ConfigureNewProj(void);
		void ScanComponents(int MyType);
		void CreateNewProj(void);
		void ClearComponents(void);
		void RandomizeSelection(void);
		//void ConfigureAllComponentImages(void);
		void ConfigureComponentImages(int MyType);
		void ConfigureSummaryImages(void);
		void SetActiveComponentByIconNum(int IconNum);
		void SetActiveComponentByNum(int MyType, int CompNum);
		void SetActiveComponentByCCD(int MyType, CombinedComponentData *NewActiveComponent);
		void NewComponentRange(int MyType, int MoveDir);
		void CaptureComponent(long NewActive);

	}; // class VersionGUI

#endif // WCS_STARTUPWIN_H
