// Application.h
// Root-level app object
// Created from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

class AppEvent;
class GUIContext;
class Database;
class Project;
class MessageLog;
class WCSApp;
class BusyWin;
class Renderer;
class EngineController;
class Fenetre;
class GUIFenetre;
class WCSModule;
class Toolbar;
class NotifyEvent;
class Conservatory;
class NotifyEx;
class EffectsLib;
class ScriptParser;
class ScriptActor;
class Security;
class AppHelp;
class ImageLib;
class MathAccellerators;
class PopupNotifier;
class EDSSControl;
#include "ImageFormatConfig.h"
#ifdef WCS_BUILD_ECW_SUPPORT
class ECWDLLAPI;
#endif // WCS_BUILD_ECW_SUPPORT

#ifndef WCS_APPLICATION_H
#define WCS_APPLICATION_H

#include "Types.h"
#include "AppModule.h"

#ifdef _WIN32
typedef MSG NativeMessage;
#endif // _WIN32

#define WCS_APPLICATION_BACKGROUND_HANDLERS_MAX		10

#ifdef _WIN32
// *********************************************************************
//                      The Dread Pirate WndProc!
// *********************************************************************
long WINAPI WndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);

// *********************************************************************
//                      His little brother DlgProc.
// *********************************************************************
long WINAPI DlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);

#endif // _WIN32

class AppEvent
	{
	friend class WCSApp;
	friend class WCSModule;
	public:
		AppEvent() {EventClear();};
		void EventClear(void)
			{
			Type = NULL; ChangeNotify = NULL; Origin = NULL;
			GenericData = NULL;
			};
		unsigned long Type;
		NotifyEvent *ChangeNotify;
		NativeMessage GUIMessage;
		Fenetre *Origin;
		void *GenericData;
	}; // AppEvent

class WCSApp
	{
	friend class Fenetre;
	friend class GUIFenetre;
	friend class BusyWin;
	friend class EngineController;
	friend class Renderer;
	friend class NotifyEx;
	#ifdef _WIN32
	friend long WINAPI WndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
	friend long WINAPI DlgProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
	#endif // _WIN32

	private:
		unsigned long ErrorStatus;
		char ProgDir[255];
		const char *StartupParam;

		// These implement the delayed-gratification notify mechanism
		NotifyEx *Delayed;
		// Only set if unset, return null if already set.
		inline NotifyEx *SetDelayNotify(NotifyEx *SetDelay) {return(Delayed ? NULL : (Delayed = SetDelay));};
		
		#ifdef _WIN32
		// Yet another Windows hack. This one allows us to specifiy a Fenetre
		// to forward events to during window creation. Otherwise the Fenetre
		// pointer would not be set up until after the window creation stage,
		// and some of the window-creation messages (such as WM_INITDIALOG or
		// WM_MEASUREITEM) will fall through to the default window handler --
		// perhaps not what you'd want.
		Fenetre *SDFK;
		void SetupDlgFenetreKludge(Fenetre *Target) {SDFK = Target;};
		#endif // _WIN32

		unsigned char Terminate;
		unsigned short ReturnCode;

		// Background processing support
		unsigned int NumBackGroundHandlers;
		WCSModule *BackGroundHandlers[WCS_APPLICATION_BACKGROUND_HANDLERS_MAX];

		// As the name implies, this is part of the Application
		// object, and gets called if all the generalized init code in
		// the App object constructor succeeds.
		//
		// This method is implemented outside of Application.cpp, usually
		// in the main or startup module of your application.
		//
		// Here, you perform application-specific initialization and
		// allocation of object pointers found in the WCSApp object.
		//
		// It doesn't hurt to have lots of extraneous pointers in the
		// Application object, as long as you don't use them (or init
		// them in this method) and don't link with their code, they're
		// just unused pointers and you can ignore them.
		//
		// In this way, we can share one (compiled) Application object
		// with many different projects or applications without having
		// to rebuild it every time we rebuild a different app.
		unsigned long AppSpecificInit(void);

		// This cleans up from the above
		void AppSpecificCleanup(void);
		
	
	public:
		
		GUIContext *WinSys;
		Database *AppDB;
		Project *MainProj;
		MessageLog *StatusLog;
		Renderer *Engine;
		EffectsLib *AppEffects, *LoadToEffectsLib, *CopyFromEffectsLib, *CopyToEffectsLib;
		ImageLib *AppImages, *LoadToImageLib, *CopyFromImageLib, *CopyToImageLib;
		ScriptParser *SuperScript;
		ScriptActor *Bogart;
		Security *Sentinal;
		AppHelp *HelpSys;
		MathAccellerators *MathTables;
		NotifyEx *AppEx;
		PopupNotifier *Toaster; // pops up little things from the bottom
		EDSSControl *StartEDSS;
		char TemplateLoadInProgress, ComponentSaveInProgress, EDSSEnabled, SXAuthorized, ForestryAuthorized;
		char DatabaseDisposalInProgress, ImageDisposalInProgress, EffectsDisposalInProgress;

		Toolbar *MCP;
		Conservatory *GUIWins;
		NativeLoadHandle IntInst;

#ifdef WCS_BUILD_ECW_SUPPORT
		ECWDLLAPI *ECWSupport;
#endif // WCS_BUILD_ECW_SUPPORT

		unsigned long InquireInitError(void) {return(ErrorStatus);}; // Ask if everything is OK
		void LoopAndWait(void);
		inline void SetTerminate(unsigned char TermCode = 1) {Terminate = TermCode;};
		unsigned char GetTerminate(void) {return(Terminate);};
		inline void SetResult(unsigned short Res) {ReturnCode = Res;};
		inline unsigned short ResultCode(void) {return(ReturnCode);};
		void SetProcPri(signed char NewLevel);
		unsigned char GetProgDir(char *Dest, unsigned char Len);
		inline char *GetProgDir(void) {return (ProgDir);};
		inline const char *GetStartupArgs(void) {return (StartupParam);};
		int CheckToolsDir(char *ProgDir);
		void UpdateProjectByTime(void);
		WCSApp(NativeLoadHandle Instance, const char *FirstParam, int StartupCode);
		#ifdef _WIN32
		void ProcessOSEvent(MSG *msg);
		#endif // _WIN32

		// Background Handlers
		int AddBGHandler(WCSModule *NewHandler);
		void RemoveBGHandler(WCSModule *RemHandler);
		void RemoveBGHandlers(WCSModule *RemHandler);
		int IsBGHandler(WCSModule *CheckHandler);

		// These bits are used for deriving generic configuration information
		char InquireTempBuf[500], PrivGLVendorString[100],  PrivGLRenderString[100], PrivGLVersionString[100];

		char *InquireOSDescString(char *StrBuf, int StrBufLen);
		char *InquireCPUDescString(char *StrBuf, int StrBufLen);
		char *InquireSystemNameString(char *StrBuf, int StrBufLen);

		char *InquireGLVendorString(void) {return(PrivGLVendorString);};
		char *InquireGLRenderString(void) {return(PrivGLRenderString);};
		char *InquireGLVersionString(void) {return(PrivGLVersionString);};
		char *InquireDisplayResString(char *StrBuf, int StrBufLen);

		void SetGLVendorString(const char *StrBuf) {strncpy(PrivGLVendorString, StrBuf, 99); PrivGLVendorString[99]=NULL;};
		void SetGLRenderString(const char *StrBuf) {strncpy(PrivGLRenderString, StrBuf, 99); PrivGLRenderString[99]=NULL;};
		void SetGLVersionString(const char *StrBuf) {strncpy(PrivGLVersionString, StrBuf, 99); PrivGLVersionString[99]=NULL;};

		int InquireNumCPUS(void);
		unsigned long GetSystemProcessID(void);
		void SetCopyToLibs(EffectsLib *NewEffects, ImageLib *NewImages)	{CopyToEffectsLib = NewEffects; CopyToImageLib = NewImages;};

		~WCSApp();
	
	}; // WCSApp


// Errors that can cause program failure on startup.
enum
	{
	WCS_ERR_GUI_ALLOCATE = 1,
	WCS_ERR_GUI_NOPALETTE,
	WCS_ERR_GUI_NOWIDGETS,
	WCS_ERR_GUI_WINDOWSETUP,
	WCS_ERR_GUI_NOIDCMP,
	WCS_ERR_GUI_MUIREV,
	WCS_ERR_GUI_GADTOOLSREV,
	WCS_ERR_GUI_ASLREV,
	WCS_ERR_GUI_GFXREV,
	WCS_ERR_GUI_INTUITIONREV,
	WCS_ERR_GUI_EXIT,
	WCS_ERR_LOG_ALLOCATE,
	WCS_ERR_DB_ALLOCATE,
	WCS_ERR_PARAM_ALLOCATE,
	WCS_ERR_PROJ_ALLOCATE,
	WCS_ERR_TOOLBAR_ALLOCATE,
	WCS_ERR_ENGINE_ALLOCATE,
	WCS_ERR_IMAGE_ALLOCATE,
	WCS_ERR_GUIWINS_ALLOCATE,
	WCS_ERR_EFFECTS_ALLOCATE,
	WCS_ERR_IMAGES_ALLOCATE,
	WCS_ERR_SCRIPT_ALLOCATE,
	WCS_ERR_MATH_ALLOCATE,
	WCS_ERR_INTERCOMMON_ALLOCATE,
	WCS_ERR_NOTIFY_ALLOCATE
	}; // ErrorCodes

// Types of AppEvents
enum
	{
	WCS_APP_EVENTTYPE_INTUI,
	WCS_APP_EVENTTYPE_MUI,
	WCS_APP_EVENTTYPE_MSWIN,
	WCS_APP_EVENTTYPE_NOTIFYCHANGE,
	WCS_APP_EVENTTYPE_SCRIPT,
	WCS_APP_EVENTTYPE_MAPVIEW,
	WCS_APP_EVENTTYPE_CAMVIEW
	}; // AppEvent Types

#endif // WCS_APPLICATION_H
