// Fenetre.h
// Abstract window class
// Created from scratch on 5/16/95 by Chris "Xenon" Hanson
// Copyright 1995

class AppEvent;
class WCSApp;

class Fenetre;
class GUIFenetre;
class DrawingFenetre;
class PaletteMeister;

#ifndef WCS_FENETRE_H
#define WCS_FENETRE_H

#include "GUI.h"
#include "AppModule.h"
#include "Project.h"
#include "Palette.h"

#ifdef AMIGA

typedef APTR NativeGUIWin; // MUI Window
typedef struct Window * NativeDrwWin; // Intuition Window

#endif // AMIGA

#ifdef WIN32

typedef HWND NativeGUIWin;
typedef HWND NativeDrwWin;

#endif // WIN32

// Types of mouse pointers a Fenetre can ask for with GoPointer/InstallPointer
enum
	{
	WCS_FENETRE_POINTER_NORMAL,
	WCS_FENETRE_POINTER_WAIT
	// WCS_FENETRE_POINTER_COPY
	// WCS_FENETRE_POINTER_SWAP
	// WCS_FENETRE_POINTER
	}; // MousePointers

// Status as indicated by PointerFlag bits
enum
	{
	WCS_FENETRE_PFLAG_INSTALLED,
	WCS_FENETRE_PFLAG_MODAL
	}; // PointerFlag bits

class Fenetre
	{
	friend class PaletteMeister;
	friend class GUIContext;
	private:
		PaletteMeister *ColorMan;
		PenThing *CurRange;
		unsigned char PointerFlag;
		#ifdef WIN32
		HCURSOR PrevPointer;
		#endif // WIN32
		void GainFocus(void);
		void LoseFocus(void);
		void InstallPointer(unsigned char PointerType);
		virtual void InstallColorHook(PenThing NewColor) = 0;
		virtual void *GetNativeWin(void) = 0;
		#ifdef WIN32
		virtual HDC GetNativeDC(void) = 0;
		virtual void ReleaseNativeDC(void) = 0;
		#endif // WIN32
	public:
		GUIContext *WindowSystem;
		Fenetre *GCFenChain;
		unsigned short OriX, OriY, SizeX, SizeY;
		unsigned long int FenID, PalIdx;
		char *FenTitle;
		WCSModule *Owner;
	
		Fenetre(GUIContext *GUICon, unsigned long int WinID, WCSModule *Module, char *Title);
//		~Fenetre();
		long HandleEvent(AppEvent *Activity, WCSApp *AppScope); // Hand off to Owner
		virtual void SetTitle(char *NewTitle) = 0;
		void GoPointer(unsigned char PointerType, unsigned char Modal = 0);
		void EndPointer(void);
		/* inline */ void SelectDefPen(unsigned char PenDesc);
		/* inline */ void SelectRangePen(unsigned char PenDesc);
		/* inline */ void SelectColorRange(unsigned char Range);

	}; // Fenetre

class GUIFenetre : public Fenetre
	{
	private:
		NativeGUIWin NativeWin; // NOTE: This is really actually a pointer!
		void InstallColorHook(PenThing NewColor);
		void *GetNativeWin(void);
		#ifdef WIN32
//		HDC GetNativeDC(void) {return(GetDC(NativeWin));};
//		void ReleaseNativeDC(HDC CutLoose) {ReleaseDC(NativeWin, CutLoose);};
		#endif // WIN32
	public:
		GUIFenetre();
		int IsThisYou(NativeGUIWin Him) {return(Him == NativeWin);};
//		~GUIFenetre();
//		Open();
//		void Close();
//		void SetTitle(char *NewTitle);
	}; // GUIFenetre

class DrawingFenetre : public Fenetre
	{
	private:
		NativeDrwWin NativeWin; // NOTE: This is really actually a pointer!

		#ifdef WIN32
		HDC hdc, NewDC;  // Device Context, sorta like Amiga's rastport, only temporary
		int Paint;
		unsigned char PixelCol; // Because SetPixel uses it
//		HDC GetNativeDC(void) {return(GetDC(NativeWin));};
//		void ReleaseNativeDC(HDC CutLoose) {ReleaseDC(NativeWin, CutLoose);};
		HDC GetNativeDC(void);
		void ReleaseNativeDC(void);
		#endif // WIN32
		void InstallColorHook(PenThing NewColor);
		void *GetNativeWin(void);
		#ifdef AMIGA
		struct RastPort *TempRP;
		void ModifyEventTypes(unsigned long int NewMask);
		void StripIntuiMessages(void);
		#endif
		
		inline void OffsetCoords(unsigned short int &X, unsigned short int &Y);
	public:
		int IsThisYou(NativeDrwWin Him) {return(Him == NativeWin);};
		DrawingFenetre(GUIContext *GUICon, unsigned long int WinID, WCSModule *Module, char *Title);
		~DrawingFenetre();
		NativeDrwWin Open(Project *Proj);
		void Close();
		void Clear();
		void GetDrawingAreaSize(unsigned short int &Width, unsigned short int &Height);
		void SetTitle(char *NewTitle);
		void CueRedraw(void);
		#ifdef AMIGA
		void SetupForDrawing(void);
		void CleanupFromDrawing(void);
		#endif // AMIGA
		#ifdef WIN32
		void SetupForDrawing(HWND Win = NULL, PAINTSTRUCT *PS = NULL);
		void CleanupFromDrawing(HWND Win = NULL, PAINTSTRUCT *PS = NULL);
		#endif // WIN32
		void DrawPixel(unsigned short int X, unsigned short int Y);
		void DrawLine(unsigned short int XS, unsigned short int YS,
		 unsigned short int XE, unsigned short int YE);
		void MoveTo(unsigned short int X, unsigned short int Y);
		void DrawTo(unsigned short int X, unsigned short int Y);
		void SetFillCol(unsigned short FillColor);
		void SetLinePat(unsigned short int LinePat);
		void UnFilledBox(unsigned short int ULX, unsigned short int ULY,
		 unsigned short int LRX, unsigned short int LRY);
		void FilledBox(unsigned short int ULX, unsigned short int ULY,
		 unsigned short int LRX, unsigned short int LRY);
	}; // DrawingFenetre

#endif // WCS_FENETRE_H
