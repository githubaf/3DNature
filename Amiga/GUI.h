// GUI.h
// The GUIContext object for encapsulating initialization and
// cleanup of windowing GUI systems.
// created from scratch on 5/21/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_GUI_H
#define WCS_GUI_H

class GUIContext;
class PaletteMeister;

#ifndef EXTERN
#define EXTERN extern
#endif // EXTERN

#ifdef AMIGA
extern "C" {
#include <intuition/intuition.h>
#include <intuition/intuitionbase.h>
#include <intuition/screens.h>
#include <graphics/gfxbase.h>
#include <graphics/display.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <hardware/blit.h>
#include <libraries/mui.h>
#include <libraries/asl.h>
// <<<>>> Should be obsolete after GST and proto/all.h
#include <proto/intuition.h>
#include <proto/graphics.h>
#include <proto/exec.h>
#include <proto/asl.h>
#include <proto/muimaster.h>
#include <clib/alib_protos.h>

// This should be picked up in <pragmas/muimaster_pragmas.h>, except that
// maybe this define isn't taking place since we're C++ now...
#ifndef __SASC_60
#pragma tagcall MUIMasterBase MUI_NewObject 1e 9802
#pragma tagcall MUIMasterBase MUI_Request 2a BA9821007
#pragma tagcall MUIMasterBase MUI_AllocAslRequestTags 30 8002
#pragma tagcall MUIMasterBase MUI_AslRequestTags 36 9802
#pragma tagcall MUIMasterBase MUI_MakeObject 78 8002
#endif // __SASC_60

} // extern C
#endif // AMIGA

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#ifdef AMIGA
EXTERN struct IntuitionBase *IntuitionBase;
EXTERN struct GfxBase *GfxBase;
EXTERN struct Library *AslBase;
EXTERN struct Library *GadToolsBase;
EXTERN struct Library *MUIMasterBase;

#endif // AMIGA


class GUIContext
	{
	friend class Fenetre;
	friend class GUIFenetre;
	friend class DrawingFenetre;
	friend class FileReq;
	friend class PaletteMeister;
	private:
		Fenetre *FenList;
		#ifdef AMIGA
		struct Screen *AppScreen;
		APTR MUIAppObj; // Application object returned by MUI
		struct Window *OldSysReqWin; // Pointer to window for old System Requesters
		struct Process *AppProc; // Pointer to our Process, for above
		struct MsgPort *CommonIDCMP;
		int OSVersion, OSRevision;
		#endif // AMIGA
		#ifdef WIN32
		HANDLE WInstance;
		#endif // WIN32
		int DisplayWidth, DisplayHeight;
		unsigned char ModalLevel;
		
		PaletteMeister *ColorControl; // Must be dynamically created after GUIContext is done
		unsigned char ColorDepth; // 4, 8 or 24
		unsigned long int ErrorStatus;
		// InputDispatcher *EventHandler;
		
		void RegisterFen(Fenetre *Moi);
		void DeRegFen(Fenetre *Moi);

		// These two are usually only called from the Fenetre class
		// GoPointer and EndPointer
		void GoModal(void);
		void EndModal(void);
		// This one is just called from the GoModal/EndModal methods
		void ReTagFens(unsigned char PointerType);

	
	public:
		#ifdef AMIGA
		GUIContext();
		#endif // AMIGA
		#ifdef WIN32
		GUIContext(HANDLE Instance, WNDPROC WndProc);
		#endif // WIN32
		~GUIContext();
		unsigned long int InquireInitError(void) {return(ErrorStatus);}; // Ask if everything is OK
		unsigned long int InquireSignalMask(void);
		void RationalizeWinCoords(unsigned short int &X, unsigned short int &Y,
		 unsigned short int &W, unsigned short int &H);
		unsigned char InquireDepth(void) {return(ColorDepth);};
		#ifdef AMIGA
		struct IntuiMessage *CheckEvent();
		void TossEvent(struct IntuiMessage *Wolves);
		struct WCSScreenMode *ModeList_New(void);
		struct WCSScreenMode *ModeList_Choose(struct WCSScreenMode *This,
		 struct WCSScreenData *ScrnData);
		void ModeList_Del(struct WCSScreenMode *ModeList);
		#endif // AMIGA
	
	}; // class GUIContext

// Errors, left here for lack of a better place right now...
enum
	{
	WCS_ERR_GUI_NOPALETTE,
	WCS_ERR_GUI_WINDOWSETUP,
	WCS_ERR_GUI_NOIDCMP,
	WCS_ERR_GUI_MUIREV,
	WCS_ERR_GUI_GADTOOLSREV,
	WCS_ERR_GUI_ASLREV,
	WCS_ERR_GUI_GFXREV,
	WCS_ERR_GUI_INTUITIONREV,
	WCS_ERR_GUI_EXIT
	}; // ErrorCodes


#endif // WCS_GUI_H

