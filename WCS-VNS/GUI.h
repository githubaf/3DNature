// GUI.h
// The GUIContext object for encapsulating initialization and
// cleanup of windowing GUI systems.
// created from scratch on 5/21/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_GUI_H
#define WCS_GUI_H

class GUIContext;
class PaletteMeister;
class MessageLog;
class WidgetLib;

#include "Types.h"

// access Vista's dwmapi.dll/DwmGetWindowAttribute dynamically if available
#if (NTDDI_VERSION >= NTDDI_VISTA)
typedef WINGDIAPI HRESULT (APIENTRY * DWMGETWINDOWATTRIBUTE)(HWND, DWORD, PVOID, DWORD);
#endif // NTDDI_VERSION >= NTDDI_VISTA


class GUIContext
	{
	friend void WCSW_SetWidgetDrawColor(void *DrawCon, int NewColor);
	friend class Fenetre;
	friend class GUIFenetre;
	friend class DrawingFenetre;
	friend class FileReq;
	friend class PaletteMeister;
	friend class MessageLog;
	private:
		Fenetre *FenList;
		#ifdef _WIN32
		// Handy to have nearby for opening windows
		HINSTANCE WInstance;
		#endif // _WIN32
		int DisplayWidth, DisplayHeight;
		int VirtualDisplayWidth, VirtualDisplayHeight;
		unsigned char ModalLevel;
		long JoyCenterX[4], JoyCenterY[4], JoyCenterZ[4];
		long JoyMaxX[4], JoyMaxY[4], JoyMaxZ[4];
		long JoyMinX[4], JoyMinY[4], JoyMinZ[4];
		int BorderThemesEnabled;
		char RootTitleBackup[256];
		
		PaletteMeister *ColorControl; // Must be dynamically created after GUIContext is done
		NativeFont SystemFont;
		unsigned char ColorDepth; // 4, 8 or 9-32
		unsigned long ErrorStatus;
		bool IsVista; // cache this for speed

		// runtime bound access to Vista's dwmapi.dll/DwmGetWindowAttribute
		#if (NTDDI_VERSION >= NTDDI_VISTA)
		HMODULE DWMHM;
		DWMGETWINDOWATTRIBUTE DynamicDwmGetWindowAttribute;
		#endif // NTDDI_VERSION >= NTDDI_VISTA

		
		void RegisterFen(Fenetre *Moi);
		void DeRegFen(Fenetre *Moi);

	//protected:
	public:
		WidgetLib *WL; // To initialize custom Widgets
		#ifdef _WIN32
		// Takes the place of a Screen
		HWND RootWin;
		#endif // _WIN32

	public:
		GUIContext(NativeLoadHandle Instance, int StartupCode);
		~GUIContext();
		unsigned long InquireInitError(void) {return(ErrorStatus);}; // Ask if everything is OK
		unsigned long InquireSignalMask(void);
		void RationalizeWinCoords(short &X, short &Y, short &W, short &H);
		unsigned char InquireDepth(void) {return(ColorDepth);};
		int InquireDisplayWidth(void) {return(DisplayWidth);};
		int InquireDisplayHeight(void) {return(DisplayHeight);};
		int InquireRootWidth(void);
		int InquireRootHeight(void);
		int InquireMinimized(void);
		int InquireBorderThemesEnabled(void) {return(BorderThemesEnabled);}
		DWORD InquireWindowStyle(void);
		bool InquireRemoteSession(void) const; // detects and reports on Terminal Services/Remote Desktop/Citrix desktop sharing
		bool InquireIsVista(void) const;
		#ifdef _WIN32
		bool InquireTrueWindowRect(HWND Window, RECT *DestRECT) const; // this gets the Vista-correct size, or normal size if not on Vista
		#endif // _WIN32

		// These two are usually only called from the Fenetre class
		// GoPointer and EndPointer and from UserMessage*
		void GoModal(void);
		void EndModal(void);
		void UpdateDocking(int ResizeOnly = 0);
		void *GetRoot(void) {return(RootWin);};
		NativeFont GetSystemFont(void) {return(SystemFont);};
		void SetSystemFont(NativeFont SF) {SystemFont = SF;};
		void UpdateRootTitle(char *NewTitle, char *NewUser);
		void DisplayRenderTitle(char *NewTitle);
		void ClearRenderTitle(void);
		#ifdef _WIN32
		HINSTANCE Instance(void) {return(WInstance);};
		int CheckEvent(MSG *msg);
		int CheckNoWait(MSG *msg);
		#endif // _WIN32
		void DoBeep(void);
		Fenetre *FindOpenFen(unsigned long Culprit);
		
		// See qualifier table below. Oh, and please don't try to
		// combine multiple qualifiers together in one query.
		char CheckQualifier(unsigned long KeyCode);

		// For reading the status of one or more joystick-like devices
		int InitInputController(int Controller);
		float CheckInputControllerX(int Controller);
		float CheckInputControllerY(int Controller);
		float CheckInputControllerZ(int Controller);
		unsigned long CheckInputControllerButtons(int Controller, unsigned long ButtonMask);
	
	}; // class GUIContext


// Key codes for CheckQualifier
#define WCS_GUI_KEYCODE_LSHIFT		0x00000001
#define WCS_GUI_KEYCODE_RSHIFT		0x00000002
#define WCS_GUI_KEYCODE_SHIFT			0x00000004
#define WCS_GUI_KEYCODE_LALT			0x00000008
#define WCS_GUI_KEYCODE_RALT			0x00000010
#define WCS_GUI_KEYCODE_ALT			0x00000020
#define WCS_GUI_KEYCODE_CONTROL		0x00000040
#define WCS_GUI_KEYCODE_CAPSLOCK		0x00000400
#define WCS_GUI_KEYCODE_LWINDOWS		0x00000800
#define WCS_GUI_KEYCODE_RWINDOWS		0x00001000
#define WCS_GUI_KEYCODE_WINDOWS		0x00002000
#define WCS_GUI_KEYCODE_WINMENU		0x00004000
#define WCS_GUI_KEYCODE_LMOUSE		0x00008000
#define WCS_GUI_KEYCODE_MMOUSE		0x00010000
#define WCS_GUI_KEYCODE_RMOUSE		0x00020000
#define WCS_GUI_KEYCODE_Z			0x00040000


#endif // WCS_GUI_H

