// Application.h
// Root-level app object
// Created from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

class AppEvent;
class WCSApp;

#define WCS_GUI

#ifndef WCS_APPLICATION_H
#define WCS_APPLICATION_H

#include "Database.h"
#include "AppModule.h"
#include "CustomModules.h"
#include "GUI.h"
#include "Palette.h"
#include "Fenetre.h"

#ifdef AMIGA
#include <proto/exec.h>
typedef struct IntuiMessage *NativeMessage;
#endif // AMIGA

#ifdef WIN32
#include <windows.h>
typedef MSG NativeMessage;
#endif // WIN32

class AppEvent
	{
	friend class WCSApp;
	friend class WCSModule;
	public:
		NativeMessage GUIMessage;
	}; // AppEvent

class WCSApp
	{
	friend class Fenetre;
	
	private:
		#ifdef AMIGA
		unsigned long int MasterSigMask;
		#endif // AMIGA
		unsigned char Terminate;
		
		//void DispatchToModule(AppEvent *Active);

		#ifdef AMIGA
		// *********************************************************************
	   //             Big funnel through which all IDCMP flows
		// *********************************************************************
		void ProcessIntuiMessage(struct IntuiMessage *GUIMessage);
		#endif // AMIGA
		
	
	public:
		
		#ifdef WCS_GUI
		GUIContext WinSys;
		#endif // WCS_GUI
		WCSMapModule MapView;
		Database AppDB;
		unsigned short int ReturnCode;

		void LoopAndWait(void);
		inline void SetTerminate(void) {Terminate = 1;};
		inline void SetResult(unsigned short int Res) {ReturnCode = Res;};
		inline unsigned short int ResultCode(void) {return(ReturnCode);};
		#ifdef AMIGA
		WCSApp();
		#endif // AMIGA
		#ifdef WIN32
		WCSApp(HANDLE Instance);
		#endif // WIN32
//		~WCSApp();
		
	
	}; // WCSApp

#ifdef WIN32
// Is this really gonna work?
// *********************************************************************
//                      The Dread Pirate WndProc!
// *********************************************************************
long FAR PASCAL WndProc(HWND hwnd, UINT message, UINT wParam, LONG lParam);
#endif // WIN32


#endif // WCS_APPLICATION_H
