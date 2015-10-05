// Fenetre.h
// Abstract window class
// Created from scratch on 5/16/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_FENETRE_H
#define WCS_FENETRE_H

// This is here to satify LINT
#ifdef _WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#endif // _WIN32

// This takes the place of the Windows GDI's RGB() macro
#define WINGDI_RGB(r,g,b)          ((COLORREF)(((BYTE)(r)|((WORD)((BYTE)(g))<<8))|(((DWORD)(BYTE)(b))<<16)))

#define WCS_FENETRE_GENERIC_EDIT_TEMPLATE	IDD_GENERIC_EDIT1

#ifdef WCS_FENETRE_NO_SETPIXELV
#define VSetPixel(a,b,c,d)	(void)SetPixel(a,b,c,d)
#else // !WCS_FENETRE_NO_SETPIXELV
#define VSetPixel(a,b,c,d)	(void)SetPixelV(a,b,c,d)
#endif // !WCS_FENETRE_NO_SETPIXELV

#include "Types.h"

class AppEvent;
class WCSApp;

class Fenetre;
class GUIFenetre;
class DrawingFenetre;
class PaletteMeister;
class Project;
class WCSModule;
class VectorClipper;
class CamViewGUI;
class AnimDoubleTime;
class AnimGradient;
class SetCritter;
class RasterAnimHost;
class GridWidgetInstance;
class TextColumnMarkerWidgetInstance;
class GeneralEffect;
class Database;
class Raster; // for WidgetTBConfigThumb
class EffectsLib; // for WidgetLWConfig

struct PolyPoint
	{
	long x, y;
	}; // PolyPoint

#ifdef _WIN32
#include <gl/gl.h>
#include <gl/glu.h>
#endif // _WIN32

#ifdef _MSC_VER // Are we using MSVC++?
// Disable annoying "warning C4355: 'this' : used in base member initializer list"
#pragma warning(disable : 4355)
#endif // _MSC_VER

#include "GUI.h"
#include "Palette.h"

#ifdef _WIN32

// It's a sin... But it works.
typedef WINGDIAPI BOOL (APIENTRY * DGLMAKECURRENT)(NativeDrawContext hdc, HGLRC hglrc);
typedef WINGDIAPI BOOL (APIENTRY * DGLDELETECONTEXT)(HGLRC hglrc);
typedef WINGDIAPI HGLRC (APIENTRY * DGLCREATECONTEXT)(NativeDrawContext hdc);

typedef WINGDIAPI void (APIENTRY * DGLLOOKAT)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef WINGDIAPI void (APIENTRY * DGLPERSPECTIVE)(GLdouble, GLdouble, GLdouble, GLdouble);
typedef WINGDIAPI void (APIENTRY * DGLBEGIN)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLCLEAR)(GLbitfield);
typedef WINGDIAPI void (APIENTRY * DGLCLEARCOLOR)(GLclampf, GLclampf, GLclampf, GLclampf);
typedef WINGDIAPI void (APIENTRY * DGLCLEARDEPTH)(GLclampd);
typedef WINGDIAPI void (APIENTRY * DGLCOLOR3FV)(const GLfloat *);
typedef WINGDIAPI void (APIENTRY * DGLCOLOR3F)(GLfloat, GLfloat, GLfloat);
typedef WINGDIAPI void (APIENTRY * DGLCOLORMATERIAL)(GLenum, GLenum);
typedef WINGDIAPI void (APIENTRY * DGLDEPTHFUNC)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLDRAWBUFFER)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLENABLE)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLDISABLE)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLEND)(void);
typedef WINGDIAPI void (APIENTRY * DGLFLUSH)(void);
typedef WINGDIAPI void (APIENTRY * DGLFOGF)(GLenum, GLfloat);
typedef WINGDIAPI void (APIENTRY * DGLFOGFV)(GLenum, const GLfloat *);
typedef WINGDIAPI void (APIENTRY * DGLFOGI)(GLenum, GLint);
typedef WINGDIAPI void (APIENTRY * DGLLIGHTFV)(GLenum, GLenum, const GLfloat *);
typedef WINGDIAPI void (APIENTRY * DGLLIGHTMODELI)(GLenum, GLint);
typedef WINGDIAPI void (APIENTRY * DGLLOADIDENTITY)(void);
typedef WINGDIAPI void (APIENTRY * DGLMATERIALFV)(GLenum, GLenum, const GLfloat *);
typedef WINGDIAPI void (APIENTRY * DGLMATRIXMODE)(GLenum);
typedef WINGDIAPI void (APIENTRY * DGLPUSHMATRIX)(void);
typedef WINGDIAPI void (APIENTRY * DGLPOPMATRIX)(void);
typedef WINGDIAPI void (APIENTRY * DGLFRUSTUM)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
typedef WINGDIAPI void (APIENTRY * DGLVIEWPORT)(GLint, GLint, GLsizei, GLsizei);
typedef WINGDIAPI void (APIENTRY * DGLNORMAL3FV)(const GLfloat *);
typedef WINGDIAPI void (APIENTRY * DGLNORMAL3D)(GLdouble, GLdouble, GLdouble);
typedef WINGDIAPI void (APIENTRY * DGLROTATED)(GLdouble, GLdouble, GLdouble, GLdouble);
typedef WINGDIAPI void (APIENTRY * DGLVERTEX3DV)(const GLdouble *);
typedef WINGDIAPI void (APIENTRY * DGLVERTEX3D)(GLdouble, GLdouble, GLdouble);

#endif // _WIN32

// Types of mouse pointers a Fenetre can ask for with GoPointer/InstallPointer
enum
	{
	WCS_FENETRE_POINTER_NORMAL = 0,
	WCS_FENETRE_POINTER_WAIT,
	WCS_FENETRE_POINTER_COPY,
	WCS_FENETRE_POINTER_SWAP
	// WCS_FENETRE_POINTER
	}; // MousePointers

#define WCS_FENTRACK_FLAGS_ENABLE		1
#define WCS_FENTRACK_FLAGS_DISABLE		2
#define WCS_FENTRACK_FLAGS_SET			0
#define WCS_FENTRACK_FLAGS_OPEN			0x00000001
#define WCS_FENTRACK_FLAGS_UNDOCKED		0x00000002

#define WCS_FENETRE_MAX_SUBPANELS		10
#define WCS_FENETRE_MAX_SUBPANES		72

#define WCS_FENETRE_WINMAN_ONTOP		(1 << 0)
#define WCS_FENETRE_WINMAN_GUIFENSIZE	(1 << 1) // was NOMOVE (deprecated)
#define WCS_FENETRE_WINMAN_NOSIZE		(1 << 2)
#define WCS_FENETRE_WINMAN_SHOWADV		(1 << 3)
//#define  (1 << 3) // was SNAPMOVE, never used
#define WCS_FENETRE_WINMAN_NODOCK		(1 << 4)
#define WCS_FENETRE_WINMAN_ISDOCKED		(1 << 5)
#define WCS_FENETRE_WINMAN_ISHIDDEN		(1 << 6)
#define WCS_FENETRE_WINMAN_ISMOVING		(1 << 7)
#define WCS_FENETRE_WINMAN_NOPOPUP		(1 << 8)
#define WCS_FENETRE_WINMAN_ISVIEW		(1 << 9)
//#define (1 << 9) // was TOOLDOCK
#define WCS_FENETRE_WINMAN_ISDIAG		(1 << 10)
#define WCS_FENETRE_WINMAN_SMTITLE		(1 << 11)
#define WCS_FENETRE_WINMAN_NOWINLIST	(1 << 12)
#define WCS_FENETRE_WINMAN_ISRENDPREV	(1 << 13)
#define WCS_FENETRE_WINMAN_ISSUBDIALOG	(1 << 14)

#define WCS_FENETRE_MAX_CUSTOM_LONG		4
#define WCS_FENETRE_MAX_CUSTOM_VOID		4

void ReleaseRastBlast(void);
void RastBlast(NativeDrawContext BlastDest, unsigned short X, unsigned short Y,
 unsigned short W, unsigned char *R, unsigned char *G, unsigned char *B);

enum ENUM_FENETRE_OWNERDRAW_MODE
	{
	WCS_FENETRE_OWNERDRAW_MODE_NONE, // doesn't use Ownerdraw
	WCS_FENETRE_OWNERDRAW_MODE_BASIC, // used by almost everybody
	WCS_FENETRE_OWNERDRAW_MODE_JOE // only used by DBEditGUI
	}; // enum ENUM_FENETRE_OWNERDRAW_MODE

enum FenetreWindowCapabilities
	{
	WCS_FENETRE_WINCAP_IS_A_COMPONENT_EDITOR = 1,
	WCS_FENETRE_WINCAP_CANNEXT,
	WCS_FENETRE_WINCAP_CANPREV,
	WCS_FENETRE_WINCAP_CANUNDO,
	WCS_FENETRE_WINCAP_CANLOAD,
	WCS_FENETRE_WINCAP_CANSAVE,
	WCS_FENETRE_WINCAP_CANZOOM,
	WCS_FENETRE_WINCAP_CANPAN,
	WCS_FENETRE_WINCAP_CANROT,
	WCS_FENETRE_WINCAP_CANDELETE,
	WCS_FENETRE_WINCAP_CANEMBED,
	WCS_FENETRE_WINCAP_CANSHOWADV,
	WCS_FENETRE_WINCAP_MAX // to carry the missing comma
	}; // FenetreWindowCapabilities


#define	WCS_FENETRE_WINSTATE_DOCK			(1 << 0)
#define	WCS_FENETRE_WINSTATE_ZOOM			(1 << 1)
#define	WCS_FENETRE_WINSTATE_PAN			(1 << 2)
#define	WCS_FENETRE_WINSTATE_ROTATE			(1 << 3)
#define	WCS_FENETRE_WINSTATE_EMBED			(1 << 4)
#define	WCS_FENETRE_WINSTATE_ZOOMBOX		(1 << 5)
#define	WCS_FENETRE_WINSTATE_RENDER			(1 << 6)
#define	WCS_FENETRE_WINSTATE_CONSTRAIN		(1 << 7)
#define	WCS_FENETRE_WINSTATE_SHOWADV		(1 << 8)


class Fenetre
	{
	friend class PaletteMeister;
	friend class GUIContext;
	friend class WCSApp;
	friend BOOL CALLBACK ShuffleChildren(HWND hwnd, LPARAM lParam);
	private:
		Fenetre *GCFenChain;
		ColPalThingy Swatches[WCS_PALETTE_COLOR_SIZE];

		unsigned char RangeBase, RangeSizeCheck;

		unsigned long WinManagerFlags;
		unsigned char ForceMoveSize;
		#ifdef _WIN32
		HCURSOR PrevPointer;
		#endif // _WIN32
		void GainFocus(void);
		void LoseFocus(void);
		virtual void UpdateModal(void) = 0;
		virtual void InstallColorHook(PenThing NewColor) = 0;
		virtual void *GetNativeWin(void) const = 0;
		static void SetupLocalWinSys(GUIContext *InitWinSys);

	protected:
		NativeAnyWin WinCaptionButtonBar;
		char VPID[4];
		void InstallPointer(unsigned char PointerType);
		GUIContext *LocalWinSys(void);
		unsigned char CurrentPointer, PointerStore;
		unsigned char ModalInhibit;
		short NominalWidth, NominalHeight;
		virtual void HandleBackupContent(AppEvent *Activity) = 0;
		#ifdef _WIN32
		virtual NativeDrawContext GetNativeDC(void) = 0;
		virtual NativeDrawContext GetBackupDC(void) = 0;
		virtual void ReleaseNativeDC(void) = 0;
		virtual char DoYouEraseBG(void) = 0;
		virtual int DoPopup(short X, short Y, char LeftButton) = 0;
		int InternalDoPopup(char FenType, NativeGUIWin NGW, short X, short Y, char LeftButton);
		void InternalDockUpdate(int ResizeOnly);
		char DockFitOk(int Pane, int ResizeOnly);
		NativeDrawContext NewDC;
		HMENU PopupMenu;
		ENUM_FENETRE_OWNERDRAW_MODE _OwnerdrawMode;
		#endif // _WIN32
		short PopupX, PopupY;
		
		// relating to Windows caption-bar button support
		void PositionWinCaptionBar(int NewHostWinX, int NewHostWinY, HDWP DeferPos = NULL);
		bool CreateWinCaptionBar(void);
		bool CustomCloseButton;

#if (NTDDI_VERSION >= NTDDI_VISTA)
		// for Vista window size-correction variables
		long SizeCorrectX, SizeCorrectY;
#endif // NTDDI_VERSION >= NTDDI_VISTA

	public:
		unsigned short OriX, OriY, SizeX, SizeY;
		unsigned long FenID;
		char RealFenType;
		char FenTitle[260];
		unsigned long CustomDataLong[WCS_FENETRE_MAX_CUSTOM_LONG];
		void *CustomDataVoid[WCS_FENETRE_MAX_CUSTOM_VOID];
		WCSModule *Owner;
	
		Fenetre(unsigned long WinID, WCSModule *Module, char *Title);
		~Fenetre();
		long DispatchEvent(AppEvent *Activity, WCSApp *AppScope); // Hand off to Owner
		virtual void SetTitle(const char *NewTitle) = 0;
		virtual int GetTitle(char *FetchTitle, int FetchLen) = 0;
		virtual void Close(void) = 0;
		void SetDockState(bool NewState);
		bool GetDockState(void) const;
		virtual void DockUpdate(int ResizeOnly);
		void GoPointer(unsigned char PointerType);
		void EndPointer(void);
		void GoModal(void);
		void EndModal(void);
		const char *GetVPID(void) const {return(VPID);};
		unsigned long GetWinManFlags(void) {return(WinManagerFlags);};
		unsigned long TestWinManFlags(unsigned long TestFlags) const {return(WinManagerFlags & TestFlags);};
		void SetWinManFlags(unsigned long SetFlags) {unsigned long PreFlags = WinManagerFlags; WinManagerFlags |= SetFlags; if(WinManagerFlags != PreFlags) ResyncWinMan(SetFlags);};
		void ClearWinManFlags(unsigned long ClearFlags) {unsigned long PreFlags = WinManagerFlags; WinManagerFlags &= ~ClearFlags; if(WinManagerFlags != PreFlags) ResyncWinMan(ClearFlags);};
		void ResyncWinMan(unsigned long ChangedFlags);
		void MoveFen(short X, short Y);
		void SizeFen(unsigned short Width, unsigned short Height);
		void MoveAndSizeFen(short X, short Y, unsigned short Width, unsigned short Height);
		inline void SelectDefPen(unsigned char PenDesc);
		void SetColorPot(unsigned char PotNum, unsigned char PotRed,
		 unsigned char PotGreen, unsigned char PotBlue, int Immediate = 1);
		inline unsigned char FetchColPotRed(unsigned char PotNum) const {return(UNSHIFT32(Swatches[PotNum].peRed));};
		inline unsigned char FetchColPotGrn(unsigned char PotNum) const {return(UNSHIFT32(Swatches[PotNum].peGreen));};
		inline unsigned char FetchColPotBlu(unsigned char PotNum) const {return(UNSHIFT32(Swatches[PotNum].peBlue));};
		void JumpTop(void);
		void JumpBottom(void);
		void JumpVPane(char *VP);
		void JumpVPane(int Pane);
		virtual void Hide(void);
		virtual void HideSubordinateWindows(void){return;}; // do-nothing in base class
		virtual void Show(void);
		void SetFocusToMe(void);
		virtual int SaveContentsToFile(const char *OutName) = 0;
		virtual int SaveContentsToFile(FILE *Out) = 0;
		int AreYouActive(void) const {return(GetActiveWindow() == GetNativeWin());};
		void SetModalInhibit(unsigned char NewInhibit)	{ModalInhibit = NewInhibit; UpdateModal();};
		void CueRedraw(void);

		// New runtime-bound Help topic query
		virtual unsigned long QueryHelpTopic(void) {return(FenID);};
		
		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);
		virtual char *InquireWindowCapabilityCaption(FenetreWindowCapabilities AskAbout) {return(NULL);};
		void UpdateWinCaptionBarState(int Updating);
		
		// determine if Advanced Features should be displayed in the UI
		bool QueryDisplayAdvancedUIVisibleState(void); // call this to check global (and local via callback below) UI state and return one composite answer
		virtual bool QueryLocalDisplayAdvancedUIVisibleState(void) {return(false);};
		
		// this flags our window (via WinMan flags) as showing advanced features, so the caption bar can indicate it
		// also resyncs the caption bar button
		void SetDisplayAdvancedUIVisibleStateFlag(bool NewState);
		
		#ifdef _WIN32
		ENUM_FENETRE_OWNERDRAW_MODE GetOwnerdrawMode(void) const {return(_OwnerdrawMode);};
		#endif // _WIN32
		virtual bool GetFrozen(void) const {return(false);}; // override as needed

	}; // Fenetre

// Because of Construct and Destruct members, you must derive
// a class from this to instantiate it. Each program GUI window will
// be a special instantiation of GUIFenetre with its own creation code
// in the Construct method, and cleanup code in Destruct.

class GUIFenetre : public Fenetre
	{
	protected:
		NativeGUIWin NativeWin; // NOTE: This is really actually a pointer!
		NativeGUIWin SubPanels[WCS_FENETRE_MAX_SUBPANELS][WCS_FENETRE_MAX_SUBPANES];
		#ifdef _WIN32
		virtual NativeDrawContext GetNativeDC(void);
		virtual void ReleaseNativeDC(void);
		virtual NativeDrawContext GetBackupDC(void) {return(NULL);};
		virtual char DoYouEraseBG(void) {return(1);};
		// Yet another Windows hack. This one allows us to specifiy a Fenetre
		// to forward events to during window creation. Otherwise the Fenetre
		// pointer would not be set up until after the window creation stage,
		// and some of the window-creation messages (such as WM_INITDIALOG or
		// WM_MEASUREITEM) will fall through to the default window handler --

		// perhaps not what you'd want.
		NativeGUIWin FenCreateDialog(HINSTANCE a, LPCTSTR b, NativeGUIWin c, DLGPROC d, WCSApp *Ap = NULL, Fenetre *Dest = NULL);
		#endif // _WIN32
		virtual void HandleBackupContent(AppEvent *Activity) {return;};
		virtual int DoPopup(short X, short Y, char LeftButton);
		int StockX, StockY;

	private:
		void InstallColorHook(PenThing NewColor);
		void *GetNativeWin(void) const;
		void UpdateModal(void);
	public:
		NativeGUIWin CreateWinFromTemplate(unsigned long TemplateID, NativeGUIWin Parent = NULL, bool TabAppearance = false);
		NativeGUIWin CreateSubWinFromTemplate(unsigned long TemplateID, int Panel, int Pane, bool TabAppearance = true); // see note below
		// If you don't create your SubWin's in numeric order, widgets may not appear.  You've been warned :)
		GUIFenetre(unsigned long WinID, WCSModule *Module, char *Title);
		int IsThisYou(NativeGUIWin Him) {return(Him == NativeWin);};
		void InstallPointer(unsigned char PointerType);
		~GUIFenetre();
		NativeGUIWin Open(Project *Proj = NULL);
		void Close();
		virtual void Hide(void);
		virtual void HideSubordinateWindows(void){return;}; // do-nothing in base class
		virtual void Show(void);
		void SetTitle(const char *NewTitle);
		int GetTitle(char *FetchTitle, int FetchLen);
		void DockUpdate(int ResizeOnly);
		virtual NativeGUIWin Construct(void) = 0;
		void Destruct(void);
		void ShowPanel(int Panel, int Pane);
		void SetPanel(int Panel, int Pane, NativeGUIWin NewPanel) {SubPanels[Panel][Pane] = NewPanel;};
		NativeGUIWin GetPanel(int Panel, int Pane) const {return(SubPanels[Panel][Pane]);};
		void ShowPanelAsPopDrop(WIDGETID WidgetID, int Panel, int Pane, NativeGUIWin Host = NULL);
		bool IsPaneDisplayed(int Panel, int Pane);
		int SaveContentsToFile(const char *OutName) {return(0);}; // Doesn't work: not useful on GUIFen
		int SaveContentsToFile(FILE *Out) {return(0);}; // Doesn't work: not useful on GUIFen

		// Obscure
		GUIFenetre *GetThySelf(void) {return(this);};

		NativeControl GetWidgetFromID(WIDGETID WidgetID);

		// Widget-interaction methods, found in FenetreWidget.cpp
		void WidgetSetDisabled(WIDGETID WidgetID, char Disabled, NativeGUIWin DestWin = NULL);
		void WidgetSetText(WIDGETID WidgetID, const char *Text, NativeGUIWin DestWin = NULL);
		 long WidgetGetText(WIDGETID WidgetID, int BufSize, char *Buffer, NativeGUIWin DestWin = NULL);
		void WidgetSetCheck(WIDGETID WidgetID, unsigned long State, NativeGUIWin DestWin = NULL);
		 char WidgetGetCheck(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetSetStyle(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin = NULL);
		void WidgetSetStyleOn(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin = NULL);
		void WidgetSetStyleOff(WIDGETID WidgetID, unsigned long Style, NativeGUIWin DestWin = NULL);
		 unsigned long WidgetGetStyle(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetRepaint(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetShow(WIDGETID WidgetID, int Reveal, NativeGUIWin DestWin = NULL);
		int WidgetGetSize(WIDGETID WidgetID, int &Width, int &Height, NativeGUIWin DestWin = NULL);

		// Edit Fields
		void WidgetEMSetSelected(WIDGETID WidgetID, long StartSel, long EndSel, NativeGUIWin DestWin = NULL);
		void WidgetEMSetReadOnly(WIDGETID WidgetID, int ReadOnly, NativeGUIWin DestWin = NULL);
		void WidgetEMScroll(WIDGETID WidgetID, long HScroll, long VScroll, NativeGUIWin DestWin = NULL);

		// Scrollbars
		long WidgetSetScrollRange(WIDGETID WidgetID, long Min, long Max, NativeGUIWin DestWin = NULL);
		long WidgetSetScrollPos(WIDGETID WidgetID, long Pos, NativeGUIWin DestWin = NULL);
		long WidgetSetScrollPosQuiet(WIDGETID WidgetID, long Pos, NativeGUIWin DestWin = NULL);
		 long WidgetGetScrollPos(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		
		// SmartWidgets
		#define WidgetFISync	WidgetSNSync
		void WidgetSNSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin = NULL);
		long WidgetSNGetEditText(WIDGETID WidgetID, int BufSize, char *Buffer, NativeGUIWin DestWin = NULL);
		void WidgetFIConfig(WIDGETID WidgetID, void *MV, double IDA, double Min, double Max, unsigned long Flags, SetCritter *Crit = NULL, unsigned long SetGetID = NULL);
		void WidgetSNConfig(WIDGETID WidgetID, AnimDoubleTime *AD, unsigned long Flags = NULL);
		void WidgetSmartRAHConfig(WIDGETID WidgetID, RasterAnimHost *ConfigRAH, RasterAnimHost *RAHParent, unsigned long Flags = NULL);
		void WidgetSmartRAHConfig(WIDGETID WidgetID, RasterAnimHost **ConfigRAH, RasterAnimHost *RAHParent, unsigned long Flags = NULL);
		void WidgetAGSync(WIDGETID WidgetID);
		void WidgetAGConfig(WIDGETID WidgetID, AnimGradient *AG);
		void WidgetSCSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin = NULL);
		void WidgetSCConfig(WIDGETID WidgetID, void *MV, unsigned long Flags, NativeGUIWin DestWin = NULL);
		void WidgetSCConfig(WIDGETID WidgetID, unsigned long Flags, SetCritter *Crit, unsigned long SetGetID, NativeGUIWin DestWin = NULL);
		void WidgetDDSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin = NULL);
		// can ONLY be called while processing HandleFIChange()
		double WidgetSNGetPrevValue(void);

		// SmartRadio
		void WidgetSRSync(WIDGETID WidgetID, unsigned long Flags, NativeGUIWin DestWin = NULL);
		void WidgetSRConfig(WIDGETID GroupWidgetID, WIDGETID WidgetID, void *MV, unsigned long Flags, signed long SetVal, NativeGUIWin DestWin = NULL);
		void WidgetSRConfigCritter(WIDGETID GroupWidgetID, WIDGETID WidgetID, unsigned long Flags, signed long SetVal, SetCritter *Crit, unsigned long SetGetID, NativeGUIWin DestWin = NULL);

		// GridWidget
		void WidgetGWConfig(WIDGETID WidgetID, GridWidgetInstance *Instance);
		void WidgetGWSetStyle(WIDGETID WidgetID, unsigned long GWStyle);
		
		// LinkWidget
		void WidgetLWConfig(WIDGETID WidgetID, GeneralEffect *Component, Database *DatabaseHost, EffectsLib *EffectsHost, unsigned long int NewQueryFlags = ~0);
		void WidgetLWSync(WIDGETID WidgetID);

		// TextColumnMarker
		void WidgetTCConfig(WIDGETID WidgetID, TextColumnMarkerWidgetInstance *Instance);
		void WidgetTCSetText(WIDGETID WidgetID, const char *NewText);
		void WidgetTCGetData(WIDGETID WidgetID, std::vector<unsigned long> *DestinationContainer);
		
		// ComboBoxes
		void WidgetCBSetUIMode(WIDGETID WidgetID, char Mode, NativeGUIWin DestWin = NULL);
		void WidgetCBClear(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetCBAddEnd(WIDGETID WidgetID, const char *String, NativeGUIWin DestWin = NULL);
		long WidgetCBInsert(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin = NULL);
		void WidgetCBDelete(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin = NULL);
		long WidgetCBReplace(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin = NULL);
		long WidgetCBStuffStrings(WIDGETID WidgetID, NativeGUIWin DestWin, ...);
		void WidgetCBSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		long WidgetCBGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetCBSetItemData(WIDGETID WidgetID, long Current, void *ItemData, NativeGUIWin DestWin = NULL);
		void *WidgetCBGetItemData(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		long WidgetCBGetCount(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetCBGetText(WIDGETID WidgetID, long Entry, char *String, NativeGUIWin DestWin = NULL);

		// ListBoxes
		void WidgetLBClear(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetLBInsert(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin = NULL);
		void WidgetLBDelete(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin = NULL);
		long WidgetLBReplace(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin = NULL);
		long WidgetLBStuffStrings(WIDGETID WidgetID, NativeGUIWin DestWin, ...);
		long WidgetLBGetText(WIDGETID WidgetID, long Entry, char *Buffer, NativeGUIWin DestWin = NULL);
		 long WidgetLBGetCount(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		 long WidgetLBGetSelCount(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetLBSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		void WidgetLBClearSel(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		 long WidgetLBGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetLBSetSelState(WIDGETID WidgetID, long State, long Current, NativeGUIWin DestWin = NULL);
		 char WidgetLBGetSelState(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		void WidgetLBSetItemData(WIDGETID WidgetID, long Current, void *ItemData, NativeGUIWin DestWin = NULL);
		 void *WidgetLBGetItemData(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		void WidgetLBSetHorizExt(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		long WidgetLBGetSelItems(WIDGETID WidgetID, long ArraySize, LONG *SelItems, NativeGUIWin DestWin = NULL);

		// String Widgets
		void WidgetSetModified(WIDGETID WidgetID, char Modified, NativeGUIWin DestWin = NULL);
		 char WidgetGetModified(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);

		// Tree Widgets
		void WidgetTVDeleteAll(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetTVGetVisibleCount(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void *WidgetTVGetFirstVisible(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void *WidgetTVGetNextVisible(WIDGETID WidgetID, void *CurVis, NativeGUIWin DestWin = NULL);
		void WidgetTVSetBoldState(WIDGETID WidgetID, void *Item, char BoldState, NativeGUIWin DestWin = NULL);
		void WidgetTVSetItemText(WIDGETID WidgetID, void *Item, char *ItemText, NativeGUIWin DestWin = NULL);
		 long WidgetTVGetItemText(WIDGETID WidgetID, void *Item, char *TextBuffer, NativeGUIWin DestWin = NULL);
		void WidgetTVSetItemData(WIDGETID WidgetID, void *Item, void *ItemData, NativeGUIWin DestWin = NULL);
		 void *WidgetTVGetItemData(WIDGETID WidgetID, void *Item, NativeGUIWin DestWin = NULL);
		void WidgetTVSetItemSelected(WIDGETID WidgetID, void *Item, char SelState, NativeGUIWin DestWin = NULL);
		 char WidgetTVGetItemSelected(WIDGETID WidgetID, void *Item, NativeGUIWin DestWin = NULL);
		void *WidgetTVInsert(WIDGETID WidgetID, char *ItemText, void *ItemData, void *ItemParent, char Children, NativeGUIWin DestWin = NULL);
		void *WidgetTVInsertExpanded(WIDGETID WidgetID, char *ItemText, void *ItemData, void *ItemParent, char Children, char Expanded, NativeGUIWin DestWin = NULL);
		void *WidgetTVGetRoot(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void *WidgetTVGetChild(WIDGETID WidgetID, void *CurItem, NativeGUIWin DestWin = NULL);
		void *WidgetTVGetNextSibling(WIDGETID WidgetID, void *CurItem, NativeGUIWin DestWin = NULL);
		bool WidgetTVEnsureVisible(WIDGETID WidgetID, void *CurVis, NativeGUIWin DestWin = NULL);

		void WidgetTVSet(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		void WidgetTVGet(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);

		// Tab controls
		long WidgetTCInsertItem(WIDGETID WidgetID, long Entry, const char *String, NativeGUIWin DestWin = NULL);
		long WidgetTCDeleteItem(WIDGETID WidgetID, long Entry, NativeGUIWin DestWin = NULL);
		long WidgetTCSetCurSel(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		long WidgetTCClear(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetTCGetItemCount(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetTCGetCurSel(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		long WidgetTCGetItemText(WIDGETID WidgetID, long Entry, char *String, long MaxTextLen, NativeGUIWin DestWin = NULL);

		// Up-down controls
		long WidgetUDSetPos(WIDGETID WidgetID, long Current, NativeGUIWin DestWin = NULL);
		
		// ColumnedListView
		void WidgetColumnedListViewSetActiveColumn(WIDGETID WidgetID, signed long ActiveCol, NativeGUIWin DestWin = NULL);
		signed long WidgetColumnedListViewGetActiveColumn(WIDGETID WidgetID, NativeGUIWin DestWin = NULL);
		
		// Toolbutton
		void WidgetTBConfig(WIDGETID WidgetID, int Normal, int Select, NativeGUIWin DestWin = NULL);
		void WidgetTBConfigThumb(WIDGETID WidgetID, Raster *Thumb, NativeGUIWin DestWin = NULL);

	}; // GUIFenetre


class DrawingFenetre : public Fenetre
	{
	friend class PaletteMeister;
	friend class CamViewGUI;
	protected:
		NativeDrwWin NativeWin; // NOTE: This is really actually a pointer!
		int OSCBWidth, OSCBHeight, ClientSizeX, ClientSizeY;
		NativeDrawContext OffScreen; // DC for ContentBackup Bitmap
		NativeBitmap ContentBackup;

		virtual void HandleBackupContent(AppEvent *Activity);
		virtual int DoPopup(short X, short Y, char LeftButton);
		#ifdef _WIN32
		virtual NativeDrawContext GetNativeDC(void);
		virtual NativeDrawContext GetBackupDC(void) {return(OffScreen);};
		virtual void ReleaseNativeDC(void);
		virtual char DoYouEraseBG(void) {return(0);};
		#endif // _WIN32

	private:
		#ifdef _WIN32
		NativeDrawContext hdc;  // Device Context, sorta like Amiga's rastport, only temporary
		int Paint;
		COLORREF PixelCol; // this is the color of the last-selected Palette Pen, used for BGDrawPixel and DrawPixel
		
		// These are used for truecolor blitting
		//NativeBitmap CacheTransfer;
		unsigned char *CTBitArray;
		unsigned long CTBASize;
		HGDIOBJ PutBack;
		BITMAPINFO BMI;
		//void *BitArray;

		#endif // _WIN32
		void InstallColorHook(PenThing NewColor);
		void *GetNativeWin(void) const;
		void UpdateModal(void);

		unsigned short StoredPat;

		inline void OffsetCoords(unsigned short &X, unsigned short &Y);

	public:
		int IsThisYou(NativeDrwWin Him) {return(Him == NativeWin);};
		DrawingFenetre(unsigned long WinID, WCSModule *Module, char *Title);
		~DrawingFenetre();
		virtual NativeDrwWin Open(Project *Proj);
		void Close();
		virtual void Hide(void);
		virtual void Show(void);
		void Clear();
		void ClearFG(unsigned short X = 0, unsigned short Y = 0, short Width = -1, short Height = -1);
		void ClearBackup();
		void GetDrawingAreaSize(unsigned short &Width, unsigned short &Height);
		void SetDrawingAreaSize(unsigned short Width, unsigned short Height);
		void SetTitle(const char *NewTitle);
		int GetTitle(char *FetchTitle, int FetchLen);
		void DockUpdate(int ResizeOnly);
		virtual void SetupForDrawing(void);
		virtual void CleanupFromDrawing(void);
		int SaveContentsToFile(const char *OutName);
		int SaveContentsToFile(FILE *Out);

		// These draw to the background buffer, and you must
		// use SyncBackground() to transfer to the foreground
		void BGPlot24(int X, int Y, unsigned char R, unsigned char G, unsigned char B);
		void BGDrawLine24(int X, int Y, unsigned short W, unsigned char *R, unsigned char *G, unsigned char *B);
		void BGDrawPixel(int X, int Y);
		void BGDrawLine(int XS, int YS, int XE, int YE);
		void SyncBackground(int X, int Y, int W, int H, int XOff = 0, int YOff = 0);

		// These draw to the foreground immediately...
		void Plot24(int X, int Y, unsigned char R, unsigned char G, unsigned char B);
		void DrawPixel(int X, int Y);
		void DrawLine(int XS, int YS, int XE, int YE);
		void MoveTo(int X, int Y);
		void DrawTo(int X, int Y);
#ifdef UNUSED_CODE
		void SetFillCol(unsigned short FillColor);
		void UnFilledBox(unsigned short ULX, unsigned short ULY,
		 unsigned short LRX, unsigned short LRY);
		void FilledBox(unsigned short ULX, unsigned short ULY,
		 unsigned short LRX, unsigned short LRY);
		void UnFilledCircle(int CX, int CY, unsigned short R);
		void FilledCircle(unsigned short CX, unsigned short CY, unsigned short R);
		void DrawEllipse12(VectorClipper *VC, int CX, int CY, unsigned short RX, unsigned short RY);
		void DrawTempEllipse12(VectorClipper *VC, int CX, int CY,
		 unsigned short RX, unsigned short RY, short Erase, unsigned char DefPenDesc = WCS_PALETTE_DEF_YELLOW);
#endif // UNUSED_CODE

		// Filled Polygon drawing code
		// Note polygon fill color will be whatever the drawing pen was
		// when you called SetupPolyDraw
		int SetupPolyDraw(unsigned long MaxPoints, unsigned long CacheSize = 0);
		int DrawNGon(unsigned long Points, PolyPoint *PointList);
		// Note that CleanupPolyDraw will do a SyncBackground...
		void CleanupPolyDraw(void);

		// This draws clipped lines
		void LineClip(VectorClipper *VC, double XS, double YS, double XE, double YE);

		// These draw or erase a 'temporary' line
		void TempLineClip(VectorClipper *VC, double XS, double YS, double XE, double YE, short Erase, unsigned char DefPenDesc = WCS_PALETTE_DEF_YELLOW);
		void DrawTempLine(unsigned short XS, unsigned short YS,
		 unsigned short XE, unsigned short YE, unsigned char DefPenDesc = WCS_PALETTE_DEF_YELLOW);
		void EraseTempLine(unsigned short XS, unsigned short YS,
		 unsigned short XE, unsigned short YE);

		void CaptureInput(void);
		static void ReleaseInput(void);

	}; // DrawingFenetre

#define WCS_FENETRE_MAX_SHARED_GLRCS	32

class GLDrawingFenetre : public DrawingFenetre
	{
	friend class PaletteMeister;
	friend class CamViewGUI;
	friend class Fenetre;
	friend class DrawingFenetre;
	private:
		float *CBZ;
		#ifdef _WIN32
		// GL support
		static HGLRC CurrentGDLF_GLRCs[WCS_FENETRE_MAX_SHARED_GLRCS];
		void InsertGLRC(HGLRC Ins);
		void RemoveGLRC(HGLRC Rem);
		void ShareGLRC(HGLRC Share);
		NativeGLWin FGLSubWin;
		NativeDrawContext FGLHDC;
		// GL support
		int PixelFormat;
		HMODULE GLUHM, OGLHM;
		HGLRC FGLRC;


		// Pointers for runtime mapping of OGL/GLU library-dll functions
		// Ugly, aren't they?
		DGLMAKECURRENT dglMakeCurrent; 
		DGLDELETECONTEXT dglDeleteContext;
		DGLCREATECONTEXT dglCreateContext;

		DGLLOOKAT dglLookAt;
		DGLPERSPECTIVE dglPerspective;
		DGLBEGIN dglBegin;
		DGLCLEAR dglClear;
		DGLCLEARCOLOR dglClearColor;
		DGLCLEARDEPTH dglClearDepth;
		DGLCOLOR3FV dglColor3fv;
		DGLCOLOR3F dglColor3f;
		DGLCOLORMATERIAL dglColorMaterial;
		DGLDEPTHFUNC dglDepthFunc;
		DGLDRAWBUFFER dglDrawBuffer;
		DGLENABLE dglEnable;
		DGLDISABLE dglDisable;
		DGLEND dglEnd;
		DGLFLUSH dglFlush;
		DGLFOGF dglFogf;
		DGLFOGFV dglFogfv;
		DGLFOGI dglFogi;
		DGLLIGHTFV dglLightfv;
		DGLLIGHTMODELI dglLightModeli;
		DGLLOADIDENTITY dglLoadIdentity;
		DGLMATERIALFV dglMaterialfv;
		DGLMATRIXMODE dglMatrixMode;
		DGLPUSHMATRIX dglPushMatrix;
		DGLPOPMATRIX dglPopMatrix;
		DGLFRUSTUM dglFrustum;
		DGLVIEWPORT dglViewport;
		DGLNORMAL3FV dglNormal3fv;
		DGLNORMAL3D dglNormal3d;
		DGLROTATED dglRotated;
		DGLVERTEX3DV dglVertex3dv;
		DGLVERTEX3D dglVertex3d;
		#endif // _WIN32

		char GLON, GLSetup;

	protected:
		virtual void HandleBackupContent(AppEvent *Activity);
		void GoGL(void);
		void ReGL(void); // reallocate GL rendering Context
		void GoGDI(void);

	public:
		GLDrawingFenetre(unsigned long WinID, WCSModule *Module, char *Title);
		~GLDrawingFenetre();
		NativeDrwWin Open(Project *Proj = NULL, GLDrawingFenetre *ShareWith = NULL);
		void SetupForDrawing(void);
		void CleanupFromDrawing(void);
		static int LastOneOut(void); // should we free shared displaylists?

		// Fenetre GL (fgl) support code
		// All of the functions from wgl, glu or gl have been renamed to
		// fgl. We divert GL calls to inline member functions in Fenetre.h,
		// so that we can easily map them to various implementations of GL
		// (Microsoft OpenGL, MesaGL, CyberGL, IrisGL?) that may have
		// inconsistant naming conventions.
		//
		// Our own GL-related functions
		int fglInit(void);
		void fglHide(void);
		void fglShow(void);
		void fglCleanup(void);
		float *GetCBZ(void) {return(CBZ);};
		#ifdef _WIN32
		inline void fglSwapBuffers(void) {SwapBuffers(FGLHDC);};
		#endif // _WIN32
		// Standard GL functions
		// We're only implementing mapping for the subset of GL calls that we
		// currently use. Add more as needed.
		//
		// used to be glu*
		inline void fglLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz);
		inline void fglPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar);
		// plain gl*
		inline void fglBegin(GLenum mode);
		inline void fglClear(GLbitfield mask);
		inline void fglClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
		inline void fglClearDepth(GLclampd depth);
		inline void fglColor3fv(const GLfloat *v);
		inline void fglColor3f(GLfloat r, GLfloat g, GLfloat b);
		inline void fglColorMaterial(GLenum face, GLenum mode);
		inline void fglDepthFunc(GLenum func);
		inline void fglDrawBuffer(GLenum mode);
		inline void fglEnable(GLenum cap);
		inline void fglDisable(GLenum cap);
		inline void fglEnd(void);
		inline void fglFlush(void);
		inline void fglFogf(GLenum pname, GLfloat param);
		inline void fglFogfv(GLenum pname, const GLfloat *params);
		inline void fglFogi(GLenum pname, GLint param);
		inline void fglLightfv(GLenum light, GLenum pname, const GLfloat *params);
		inline void fglLightModeli(GLenum pname, GLint param);
		inline void fglLoadIdentity(void);
		inline void fglMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
		inline void fglMatrixMode(GLenum mode);
		inline void fglPushMatrix(void);
		inline void fglPopMatrix(void);
		inline void fglFrustum(GLdouble leftval, GLdouble rightval, GLdouble bottomval,
		 GLdouble topval, GLdouble nearval, GLdouble farval);
		inline void fglViewport(GLint x, GLint y, GLsizei width, GLsizei height);
		inline void fglNormal3fv(const GLfloat *v);
		inline void fglNormal3d(GLdouble x, GLdouble y, GLdouble z);
		inline void fglRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z);
		inline void fglVertex3dv(const GLdouble *v);
		inline void fglVertex3d(GLdouble x, GLdouble y, GLdouble z);

		void MakeCurrentGL(void);
		double GLGetPixelZ(int X, int Y);
		void GLGetPixelRGBA(int X, int Y, unsigned char &R, unsigned char &G, unsigned char &B, unsigned char &A);

		void RestoreRGBZ(void);
		void FreezeRGBZ(void);

		#ifdef _WIN32
		inline BOOL fglMakeCurrent(NativeDrawContext, HGLRC); 
		inline HGLRC fglCreateContext(NativeDrawContext);
		inline BOOL fglDeleteContext(HGLRC);
		#endif // _WIN32

	}; // GLDrawingFenetre


// Here is code for various inline Fenetre methods that must have the
// definition in scope as well as the declaration.

inline void Fenetre::SelectDefPen(unsigned char PenDesc)
{
LocalWinSys()->ColorControl->PickDefColor(PenDesc, this);
} // Fenetre::SelectDefPen



// Here's the code for various inline DrawingFenetre methods that
// must have the definition in scope as well as the declaration.

#ifdef _WIN32
inline void DrawingFenetre::OffsetCoords(unsigned short &X, unsigned short &Y)
{
// Do nothing, window system offsets for us...
} // DrawingFenetre::OffsetCoords

inline void DrawingFenetre::BGPlot24(int X, int Y, unsigned char R, unsigned char G, unsigned char B)
{
VSetPixel(OffScreen, X, Y, WINGDI_RGB(R, G, B));
} // DrawingFenetre::BGPlot24

inline void DrawingFenetre::BGDrawPixel(int X, int Y)
{
VSetPixel(OffScreen, X, Y, PixelCol);
} // DrawingFenetre::BGDrawPixel

inline void DrawingFenetre::BGDrawLine(int XS, int YS, int XE, int YE)
{
(void)MoveToEx(OffScreen, XS, YS, NULL);
(void)LineTo(OffScreen, XE, YE);
} // DrawingFenetre::BGDrawLine

inline void DrawingFenetre::Plot24(int X, int Y, unsigned char R, unsigned char G, unsigned char B)
{
VSetPixel(hdc, X, Y, WINGDI_RGB(R, G, B));
VSetPixel(OffScreen, X, Y, WINGDI_RGB(R, G, B));
} // DrawingFenetre::Plot24

inline void DrawingFenetre::DrawPixel(int X, int Y)
{
VSetPixel(hdc, X, Y, PixelCol);
VSetPixel(OffScreen, X, Y, PixelCol);
} // DrawingFenetre::DrawPixel

inline void DrawingFenetre::DrawLine(int XS, int YS, int XE, int YE)
{
if((XS == XE) && (YS == YE))
	{
	DrawPixel(XS, YS);
	} // if
else
	{
	(void)MoveToEx(hdc, XS, YS, NULL);
	(void)LineTo(hdc, XE, YE);
	(void)MoveToEx(OffScreen, XS, YS, NULL);
	(void)LineTo(OffScreen, XE, YE);
	} // else
} // DrawingFenetre::DrawLine

inline void DrawingFenetre::MoveTo(int X, int Y)
{
(void)MoveToEx(hdc, X, Y, NULL);
(void)MoveToEx(OffScreen, X, Y, NULL);
} // DrawingFenetre::MoveTo

inline void DrawingFenetre::DrawTo(int X, int Y)
{
(void)LineTo(hdc, X, Y);
(void)LineTo(OffScreen, X, Y);
} // DrawingFenetre::DrawTo

#endif // _WIN32


#ifdef _WIN32
// Here are the inline GL mappings for Microsoft Windows OpenGL

#ifdef  GL_NON_DYNAMIC_BIND
// used to be glu*
inline void GLDrawingFenetre::fglLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz)
 {gluLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);};
inline void GLDrawingFenetre::fglPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
 {gluPerspective(fovy, aspect, zNear, zFar);};

// plain gl*
inline void GLDrawingFenetre::fglBegin(GLenum mode) {glBegin(mode);};
inline void GLDrawingFenetre::fglClear(GLbitfield mask) {glClear(mask);};
inline void GLDrawingFenetre::fglClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
 {glClearColor(red, green, blue, alpha);};
inline void GLDrawingFenetre::fglClearDepth(GLclampd depth) {glClearDepth(depth);};
inline void GLDrawingFenetre::fglColor3fv(const GLfloat *v) {glColor3fv(v);};
inline void GLDrawingFenetre::fglColor3f(GLfloat r, GLfloat g, GLfloat b) {glColor3f(r, g, b);};
inline void GLDrawingFenetre::fglColorMaterial(GLenum face, GLenum mode) {glColorMaterial(face, mode);};
inline void GLDrawingFenetre::fglDepthFunc(GLenum func) {glDepthFunc(func);};
inline void GLDrawingFenetre::fglDrawBuffer(GLenum mode) {glDrawBuffer(mode);};
inline void GLDrawingFenetre::fglEnable(GLenum cap) {glEnable(cap);};
inline void GLDrawingFenetre::fglDisable(GLenum cap) {glDisable(cap);};
inline void GLDrawingFenetre::fglEnd(void) {glEnd();};
inline void GLDrawingFenetre::fglFlush(void) {glFlush();};
inline void GLDrawingFenetre::fglFogf(GLenum pname, GLfloat param) {glFogf(pname, param);};
inline void GLDrawingFenetre::fglFogfv(GLenum pname, const GLfloat *params) {glFogfv(pname, params);};
inline void GLDrawingFenetre::fglFogi(GLenum pname, GLint param) {glFogi(pname, param);};
inline void GLDrawingFenetre::fglLightfv(GLenum light, GLenum pname, const GLfloat *params) {glLightfv(light, pname, params);};
inline void GLDrawingFenetre::fglLightModeli(GLenum pname, GLint param) {glLightModeli(pname, param);};
inline void GLDrawingFenetre::fglLoadIdentity(void) {glLoadIdentity();};
inline void GLDrawingFenetre::fglMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
 {glMaterialfv(face, pname, params);};
inline void GLDrawingFenetre::fglMatrixMode(GLenum mode) {glMatrixMode(mode);};
inline void GLDrawingFenetre::fglPushMatrix(void) {glPushMatrix();};
inline void GLDrawingFenetre::fglPopMatrix(void) {glPopMatrix();};
inline void GLDrawingFenetre::fglFrustum(GLdouble leftval, GLdouble rightval, GLdouble bottomval,
 GLdouble topval, GLdouble nearval, GLdouble farval) {glFrustum(leftval, rightval, bottomval, topval, nearval, farval);};
inline void GLDrawingFenetre::fglViewport(GLint x, GLint y, GLsizei width, GLsizei height) {glViewport(x, y, width, height);};
inline void GLDrawingFenetre::fglNormal3fv(const GLfloat *v) {glNormal3fv(v);};
inline void GLDrawingFenetre::fglNormal3d(GLdouble x, GLdouble y, GLdouble z) {glNormal3d(x, y, z);};
inline void GLDrawingFenetre::fglRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {glRotated(angle, x, y, z);};
inline void GLDrawingFenetre::fglVertex3dv(const GLdouble *v) {glVertex3dv(v);};
inline void GLDrawingFenetre::fglVertex3d(GLdouble x, GLdouble y, GLdouble z) {glVertex3d(x, y, z);};

inline HGLRC GLDrawingFenetre::fglCreateContext(NativeDrawContext hdc) {return(wglCreateContext(hdc));};
inline BOOL  GLDrawingFenetre::fglDeleteContext(HGLRC hglrc) {return(wglDeleteContext(hglrc));};
inline BOOL  GLDrawingFenetre::fglMakeCurrent(NativeDrawContext hdc, HGLRC hglrc) {return(wglMakeCurrent(hdc, hglrc));};

#else // !GL_NON_DYNAMIC_BIND

// used to be glu*
inline void GLDrawingFenetre::fglLookAt(GLdouble eyex, GLdouble eyey, GLdouble eyez, GLdouble centerx, GLdouble centery, GLdouble centerz, GLdouble upx, GLdouble upy, GLdouble upz)
 {dglLookAt(eyex, eyey, eyez, centerx, centery, centerz, upx, upy, upz);};
inline void GLDrawingFenetre::fglPerspective(GLdouble fovy, GLdouble aspect, GLdouble zNear, GLdouble zFar)
 {dglPerspective(fovy, aspect, zNear, zFar);};

// plain gl*
inline void GLDrawingFenetre::fglBegin(GLenum mode) {dglBegin(mode);};
inline void GLDrawingFenetre::fglClear(GLbitfield mask) {dglClear(mask);};
inline void GLDrawingFenetre::fglClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
 {dglClearColor(red, green, blue, alpha);};
inline void GLDrawingFenetre::fglClearDepth(GLclampd depth) {dglClearDepth(depth);};
inline void GLDrawingFenetre::fglColor3fv(const GLfloat *v) {dglColor3fv(v);};
inline void GLDrawingFenetre::fglColor3f(GLfloat r, GLfloat g, GLfloat b) {dglColor3f(r, g, b);};
inline void GLDrawingFenetre::fglColorMaterial(GLenum face, GLenum mode) {dglColorMaterial(face, mode);};
inline void GLDrawingFenetre::fglDepthFunc(GLenum func) {dglDepthFunc(func);};
inline void GLDrawingFenetre::fglDrawBuffer(GLenum mode) {dglDrawBuffer(mode);};
inline void GLDrawingFenetre::fglEnable(GLenum cap) {dglEnable(cap);};
inline void GLDrawingFenetre::fglDisable(GLenum cap) {dglDisable(cap);};
inline void GLDrawingFenetre::fglEnd(void) {dglEnd();};
inline void GLDrawingFenetre::fglFlush(void) {dglFlush();};
inline void GLDrawingFenetre::fglFogf(GLenum pname, GLfloat param) {dglFogf(pname, param);};
inline void GLDrawingFenetre::fglFogfv(GLenum pname, const GLfloat *params) {dglFogfv(pname, params);};
inline void GLDrawingFenetre::fglFogi(GLenum pname, GLint param) {dglFogi(pname, param);};
inline void GLDrawingFenetre::fglLightfv(GLenum light, GLenum pname, const GLfloat *params) {dglLightfv(light, pname, params);};
inline void GLDrawingFenetre::fglLightModeli(GLenum pname, GLint param) {dglLightModeli(pname, param);};
inline void GLDrawingFenetre::fglLoadIdentity(void) {dglLoadIdentity();};
inline void GLDrawingFenetre::fglMaterialfv(GLenum face, GLenum pname, const GLfloat *params)
 {dglMaterialfv(face, pname, params);};
inline void GLDrawingFenetre::fglMatrixMode(GLenum mode) {dglMatrixMode(mode);};
inline void GLDrawingFenetre::fglPushMatrix(void) {dglPushMatrix();};
inline void GLDrawingFenetre::fglPopMatrix(void) {dglPopMatrix();};
inline void GLDrawingFenetre::fglFrustum(GLdouble leftval, GLdouble rightval, GLdouble bottomval, GLdouble topval, GLdouble nearval, GLdouble farval) {dglFrustum(leftval, rightval, bottomval, topval, nearval, farval);};
inline void GLDrawingFenetre::fglViewport(GLint x, GLint y, GLsizei width, GLsizei height) {dglViewport(x, y, width, height);};

inline void GLDrawingFenetre::fglNormal3fv(const GLfloat *v) {dglNormal3fv(v);};
inline void GLDrawingFenetre::fglNormal3d(GLdouble x, GLdouble y, GLdouble z) {dglNormal3d(x, y, z);};
inline void GLDrawingFenetre::fglRotated(GLdouble angle, GLdouble x, GLdouble y, GLdouble z) {dglRotated(angle, x, y, z);};
inline void GLDrawingFenetre::fglVertex3dv(const GLdouble *v) {dglVertex3dv(v);};
inline void GLDrawingFenetre::fglVertex3d(GLdouble x, GLdouble y, GLdouble z) {dglVertex3d(x, y, z);};

inline HGLRC GLDrawingFenetre::fglCreateContext(NativeDrawContext refhdc) {return(dglCreateContext(refhdc));};
inline BOOL  GLDrawingFenetre::fglDeleteContext(HGLRC hglrc) {return(dglDeleteContext(hglrc));};
inline BOOL  GLDrawingFenetre::fglMakeCurrent(NativeDrawContext refhdc, HGLRC hglrc) {return(dglMakeCurrent(refhdc, hglrc));};

#endif // !GL_NON_DYNAMIC_BIND

#endif // _WIN32

#endif // WCS_FENETRE_H
