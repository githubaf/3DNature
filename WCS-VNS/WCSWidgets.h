// WCSWidgets.h
// Application-supplied Widget/MuiCustomClass/CustomControl header
// Built from scratch on 1/21/96 by Chris 'Xenon' Hanson
// Copyright 1996

#include "stdafx.h"

#include <vector>
#include <algorithm>

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCSWIDGETS_H
#define WCSWIDGETS_H
#include "Types.h"
#include "GUI.h"
#include "Fenetre.h"

class SetCritter;
class Param;
class WidgetLib;
class AppEvent;
class Raster;
class RasterAnimHost;
class TrueColorIcon;
class AnimGradient;
class AnimDouble;
class AnimColorTime;
class AnimDoubleTime;
class Joe;
class Database;
class DBEditGUI;
class GeneralEffect;

// Moved here from Defines.h on 2/20/06 -CXH
#ifdef _WIN32
#define WCS_SYMBOL_DEGREE		"\xb0"
#define WCS_SYMBOL_DEGREE_CHAR	0xb0
#endif // _WIN32

#define WCSWIDGETS_TEXTCOLUMN_MAX_COLMARKERS	30

struct PopMenuEvent
	{
	unsigned long TreeItem;
	RasterAnimHost *RAH;
	void *ActionText;
	int Derived;
	}; // PopMenuEvent

/* ************************** WidgetLib ************************** */

class WidgetLib
	{
	friend void WCSW_SetWidgetDrawColor(void *DrawCon, int NewColor);
	private:
		unsigned long ErrorStatus;
		#ifdef _WIN32
		HPEN WidgetPens[8];
		#endif // _WIN32
		HBITMAP XPGreyButtonDisabled, XPGreyButtonHot, XPGreyButtonNormal, XPGreyButtonPressed,
		 XPGreyButtonDisabledLg, XPGreyButtonHotLg, XPGreyButtonNormalLg, XPGreyButtonPressedLg;
		HBITMAP XPCloseButtonHot, XPCloseButtonNormal, XPCloseButtonPressed,
		 XPCloseButtonHotLg, XPCloseButtonNormalLg, XPCloseButtonPressedLg;
		void *_BoldListBoxFont, *_ItalicListBoxFont;
	public:
		HBRUSH BackgroundGrey;

		unsigned long InquireInitError(void) {return(ErrorStatus);}; // Ask if everything is OK
		WidgetLib(GUIContext *GUI);
		~WidgetLib();
		void DrawToolButtonImage(NativeDrawContext ButtonDC, NativeControl ButtonWnd, HICON Norm, HICON Hi, TrueColorIcon *Img, char State, bool NewStyle, bool Hot);
		void DrawCaptionButtonImage(NativeDrawContext ButtonDC, NativeControl ButtonWnd, char State, bool Hot, bool Enabled, bool LargeSize, bool AsClose);
		void DrawThumbNail(NativeControl hwnd, Raster *Ras, char State);
		int SampleTNailColor(NativeControl hwnd, Raster *Ras, int XC, int YC, unsigned char &ROut, unsigned char &GOut, unsigned char &BOut);
		void StoreBoldListBoxFont(void *FontHandle) {_BoldListBoxFont = FontHandle;};
		void StoreItalicListBoxFont(void *FontHandle) {_ItalicListBoxFont = FontHandle;};
		void *FetchBoldListBoxFont(void) const {return(_BoldListBoxFont);};
		void *FetchItalicListBoxFont(void) const {return(_ItalicListBoxFont);};
	}; // class WidgetLib

#ifdef _WIN32

// Mode == 1 means there is a Joe pointer in the itemData (DBEditGUI
// functionality). Mode == 0 means to do highlighting based upon the
// presence of a '3' or an '*' in the second character position of
// the string (MotEditGUI, ColorEditGUI, EcoEditGUI).

void HandleOwnerDrawGook(NativeControl hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, int Mode);
#endif // _WIN32

// Universal preferred-unit formatting, like the SmartNumerics do
char *FormatAsPreferredUnit(char *Text, unsigned char MetricType, double TheD, double Increment, AnimDouble *AD = NULL, RasterAnimHost *RAHParent = NULL);
char *FormatAsPreferredUnit(char *Text, AnimDouble *AD, double TheD, RasterAnimHost *RAHParent = NULL);
char *FormatAsPreferredUnit(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent = NULL);
void SNADmFormat(char *Text, AnimDouble *AD, double TheD, RasterAnimHost *RAHParent = NULL);
double SNADmUnformat(char *Text, AnimDouble *AD, RasterAnimHost *RAHParent = NULL);
double SNADmUnformatTime(char *Text, AnimDouble *AD);

/* *************************** Message ID Bases *********************** */

#define WM_WCS_MSG_BASE			0xC000

#define WM_REQ_NOTIFY			(WM_WCS_MSG_BASE +  2) // used by Requester.cpp's PopupNotifier class

#define WM_WCSW_SMARTCHANGE		(WM_WCS_MSG_BASE +  3)
#define WM_WCSW_SMARTPRECHANGE	(WM_WCS_MSG_BASE +  4)

#define WM_WCSW_LIST_BASE		(WM_WCS_MSG_BASE +  5)  // List has 1
#define WM_WCSW_TB_BASE			(WM_WCS_MSG_BASE +  6)  // TB has 1
#define WM_WCSW_FI_BASE			(WM_WCS_MSG_BASE +  7)  // FI has 4 (+2)
#define WM_WCSW_CB_BASE			(WM_WCS_MSG_BASE + 12)  // CB has 2
#define WM_WCSW_DD_BASE			(WM_WCS_MSG_BASE + 15) // DD has 2 (+1)
#define WM_WCSW_SC_BASE			(WM_WCS_MSG_BASE + 18) // SC has 2 (+1)
#define WM_WCSW_SR_BASE			(WM_WCS_MSG_BASE + 22) // SR has 2 (+1)
#define WM_WCSW_AG_BASE			(WM_WCS_MSG_BASE + 26) // AG has 1 (+1)
#define WM_WCSW_CLV_BASE		(WM_WCS_MSG_BASE + 28) // ColumnedListView has 1
#define WM_WCSW_GW_BASE			(WM_WCS_MSG_BASE + 30) // GridWidget has 8 for now
#define WM_WCSW_TC_BASE			(WM_WCS_MSG_BASE + 40) // TextColumnMarker has 4
#define WM_WCSW_LW_BASE			(WM_WCS_MSG_BASE + 45) // LinkWidget has 2

/* *************************** Superclassed Lists and ListTrees *********************** */


// Messages 
#define WM_WCSW_LIST_DELITEM	(WM_WCSW_LIST_BASE)
#define WM_WCSW_LIST_COPY		(WM_WCSW_LIST_BASE + 1)
#define WM_WCSW_LIST_PASTE		(WM_WCSW_LIST_BASE + 2)
// WM_WCSW_LIST_CONTEXTMENU actually only comes from TreeViews right now...
#define WM_WCSW_LIST_CONTEXTMENU		(WM_WCSW_LIST_BASE + 3)


/* *************************** Superclassed ListView (ColumnedListView) *********************** */
// Messages 
#define WM_WCSW_COLLISTVIEW_SELCOLUMN	(WM_WCSW_CLV_BASE)


/* *************************** ToolButton *********************** */


// Messages supported by ToolButton Widget
#define WM_WCSW_TB_SETUP	WM_WCSW_TB_BASE

// Style defines for ToolButton
#define WCSW_TB_STYLE_TOG		0x01
#define WCSW_TB_STYLE_TRUECOLOR	0x02
#define WCSW_TB_STYLE_NOCHANGE	0x04
#define WCSW_TB_STYLE_XPLOOK	0x08	// used to be corner tab
#define WCSW_TB_STYLE_ISTHUMB	0x10
#define WCSW_TB_STYLE_NOFOCUS	0x20
#define WCSW_TB_STYLE_ALLOWRIGHT	0x40
#define WCSW_TB_STYLE_CAPTIONBUTTON	0x80

struct ToolButtonConfig
	{
	HICON Normal, Hilite;
	#ifdef _WIN32
	NativeControl CaptionHelp;
	HFONT Font;
	#endif // _WIN32
	unsigned long TimeID;
	char State, Capture, Trigger, Hot;
	}; // TBC


void ConfigureTB(NativeControl Dest, int Target, int Normal, int Select, Raster *Thumb = NULL);

/* *************************** FloatInt *********************** */

// Messages supported by FloatInt Widget
#define WM_WCSW_FI_SETUP	WM_WCSW_FI_BASE
#define WM_WCSW_FI_SYNC		(WM_WCSW_FI_BASE + 0x01)
// Returned from Widget
#define WM_WCSW_FI_CHANGE	WM_WCSW_SMARTCHANGE
#define WM_WCSW_FI_PRECHANGE	WM_WCSW_SMARTPRECHANGE
#define WM_WCSW_FI_OPT1		(WM_WCSW_FI_BASE + 0x02)
#define WM_WCSW_FI_OPT2		(WM_WCSW_FI_BASE + 0x03)
#define WM_WCSW_FI_OPT3		(WM_WCSW_FI_BASE + 0x04)
#define WM_WCSW_FI_COLOR	(WM_WCSW_FI_BASE + 0x05)
#define WM_WCSW_FI_CHANGEARROW	(WM_WCSW_FI_BASE + 0x06) // same as WM_WCSW_FI_CHANGE, but changed via updown arrow

// WPARAM values for WPARAM of WM_WCSW_FI_SYNC
#define WP_FISYNC_NONOTIFY	0x01

// Kludgy hack
double GetSNPrevValue(void);

// Convenient setup function, fill out the below and sends it on.
void ConfigureFI(NativeGUIWin Dest, int Target, void *MV, double IDA, double Min, double Max, unsigned long Flags,
	SetCritter *Crit = NULL, unsigned long SetGetID = NULL);

struct FloatIntConfig
	{
	void *MasterVariable, *AuxVariable;
	double IncDecAmount, MaxAmount, MinAmount;
	unsigned long FIFlags;
	long SmallIDA;
	char *LabelText;
	unsigned long CritID;
	NativeControl Label, Edit, Less, Opt1, Opt2, Opt3, Color;
	}; // FloatIntConfig

// Data type flags for *MasterVariable
#define FIOFlag_Frac			(1 << 0)
#define FIOFlag_Float			(1 << 1)
#define FIOFlag_Double			(1 << 2)
#define FIOFlag_Char			(1 << 3)
#define FIOFlag_Short			(1 << 4)
#define FIOFlag_Long			(1 << 5)
#define FIOFlag_Unsigned		(1 << 6)
#define FIOFlag_NonZero			(1 << 7)

#define FIOFlag_Critter			(1 << 8)
#define FIOFlag_SmallIncDec     (1 << 9)
#define FIOFlag_KeysExist		(1 << 9) // don't think this is used anymore
#define FIOFlag_AnimDouble		(1 << 10)
#define FIOFlag_RAH				(1 << 11)
#define FIOFlag_IRAH			(1 << 12)

#define FIOFlag_ArrowNotify		(1 << 13)


#define FI_TypeMask (FIOFlag_Frac | FIOFlag_Float | FIOFlag_Double | FIOFlag_Char | FIOFlag_Short | FIOFlag_Long | FIOFlag_Unsigned)

#define FI_SmallIDAScale 100000000

// Style defines for FloatIntWidget
#define WCSW_FI_STYLE_WIDTH2	0x02
#define WCSW_FI_STYLE_WIDTH3	0x03
#define WCSW_FI_STYLE_WIDTH4	0x04
#define WCSW_FI_STYLE_WIDTH5	0x05
#define WCSW_FI_STYLE_WIDTH6	0x06
#define WCSW_FI_STYLE_WIDTH7	0x07
#define WCSW_FI_STYLE_WIDTH8	0x08
#define WCSW_FI_STYLE_WIDTH9	0x09
#define WCSW_FI_STYLE_WIDTH10	0x0a
#define WCSW_FI_STYLE_WIDTH11	0x0b
#define WCSW_FI_STYLE_WIDTH12	0x0c
#define WCSW_FI_STYLE_WIDTH13	0x0d
#define WCSW_FI_STYLE_WIDTH14	0x0e
#define WCSW_FI_STYLE_WIDTH15	0x0f


// These two flags affect widget creation, and cannot be changed
// dynamically on the fly.
#define WCSW_FI_STYLE_LABELRIGHT	0x10
#define WCSW_FI_STYLE_NOBUTTONS		0x20
#define WCSW_FI_STYLE_EXTRABUTTONS	0x40
#define WCSW_FI_STYLE_DELKEY		0x80

// WCSW_FI_STYLE_KEYSEXIST is no longer used, SRAH is now
//#define WCSW_FI_STYLE_KEYSEXIST		0x0100
#define WCSW_FI_STYLE_AUTOCONTROL	0x0100 // work in progress
#define WCSW_FI_STYLE_SRAH			0x0200
#define WCSW_FI_STYLE_SRAH_COLOR	0x0400
#define WCSW_FI_STYLE_SRAH_NOFIELD	0x0800

// SRAHs by default have space for three buttons,
// these flags allow you to alter that
#define WCSW_FI_STYLE_SRAH_ONEBUTTON	0x1000
#define WCSW_FI_STYLE_SRAH_TWOBUTTON	0x2000
#define WCSW_FI_STYLE_SRAH_NOBUTTON		0x4000

#define WCSW_FI_STYLE_SRAH_AUTOLABEL	0x8000


/* *************************** ColorBar *********************** */

// Convenient setup function, fill out the below and sends it on.
void ConfigureCB(NativeGUIWin Dest, int Target, int Percent, unsigned long Flags,
	unsigned char ColorPotNum, AnimColorTime *ACT = NULL);

struct ColorBarConfig
	{
	unsigned char PotNum;
	unsigned long Flags;
	AnimColorTime *ACT;
	int Percent;
	}; // ColorBarConfig

// Messages supported by ColorBar Widget
#define WM_WCSW_CB_SETUP	WM_WCSW_CB_BASE
#define WM_WCSW_CB_NEWVAL	(WM_WCSW_CB_BASE + 0x01)

// Flags for ColorBar
#define CBFlag_CustomColor			(1 << 0)

/* *************************** DiskDir *********************** */


// Messages supported by DiskDir Widget
#define WM_WCSW_DD_SETUP	WM_WCSW_DD_BASE
#define WM_WCSW_DD_SYNC		(WM_WCSW_DD_BASE + 0x01)
// Returned from Widget
#define WM_WCSW_DD_CHANGE	WM_WCSW_SMARTCHANGE
// WPARAM values for WPARAM of WM_WCSW_DD_SYNC
#define WP_DDSYNC_NONOTIFY	0x01

// Convenient setup function, fill out the below and sends it on.
void ConfigureDD(NativeGUIWin Dest, int Target, char *PBuf, int PBSize,
	char *FBuf, int FBSize, int LabelID = NULL);

struct DiskDirConfig
	{
	char *Path, *File;
	int PSize, FSize;
	int DDLabelID;
	NativeControl Text, Disk, Edit, Frame;
	#ifdef _WIN32
	HFONT Font;
	#endif // _WIN32
	long FWidth;
	}; // DiskDirConfig

// Style defines for DiskDir
#define WCSW_DD_STYLE_SAVETYPE	(1 << 0)
#define WCSW_DD_STYLE_NOMUNGE	(1 << 1)
#define WCSW_DD_STYLE_DIRONLY	(1 << 2)


/* **************************** SmartCheck Widget ****************** */

// Messages supported by SmartCheck Widget
#define WM_WCSW_SC_SETUP	WM_WCSW_SC_BASE
#define WM_WCSW_SC_SYNC		(WM_WCSW_SC_BASE +  0x01)
// Returned from Widget
#define WM_WCSW_SC_CHANGE	WM_WCSW_SMARTCHANGE
// WPARAM values for WPARAM of WM_WCSW_SC_SYNC
#define WP_SCSYNC_NONOTIFY	0x01


// Convenient setup function, fill out the below and sends it on.
void ConfigureSC(NativeGUIWin Dest, int Target, void *MV, unsigned long Flags,
	SetCritter *Crit = NULL, unsigned long SetGetID = NULL);

struct SmartCheckConfig
	{
	void *MasterVariable;
	unsigned long SCFlags;
	char *LabelText;
	SetCritter *Crit;
	unsigned long CritID;
	}; // SmartCheckConfig

// Data type flags for *MasterVariable
#define SCFlag_Char			(1 << 0)
#define SCFlag_Short		(1 << 1)
#define SCFlag_Long			(1 << 2)

#define SC_TypeMask (SCFlag_Char | SCFlag_Short | SCFlag_Long)


/* **************************** SmartRadio Widget ****************** */

// Messages supported by SmartRadio Widget
#define WM_WCSW_SR_SETUP	WM_WCSW_SR_BASE
#define WM_WCSW_SR_SYNC		(WM_WCSW_SR_BASE +  0x01)
// Returned from Widget
#define WM_WCSW_SR_CHANGE	WM_WCSW_SMARTCHANGE
// WPARAM values for WPARAM of WM_WCSW_SR_SYNC
#define WP_SRSYNC_NONOTIFY	0x01


// Convenient setup function, fill out the below and sends it on.
void ConfigureSR(NativeGUIWin Dest, int MasterTarget, int Target, void *MV, unsigned long Flags, signed long SetVal,
	SetCritter *Crit = NULL, unsigned long SetGetID = NULL);

struct SmartRadioConfig
	{
	void *MasterVariable;
	unsigned long SRFlags;
	signed long RadioSetVal;
	char *LabelText;
	SetCritter *Crit;
	unsigned long CritID;
	}; // SmartRadioConfig

// Data type flags for *MasterVariable
#define SRFlag_Char				(1 << 0)
#define SRFlag_Short			(1 << 1)
#define SRFlag_Long				(1 << 2)
#define SRFlag_Unsigned			(1 << 3)

#define SR_TypeMask (SRFlag_Char | SRFlag_Short | SRFlag_Long | SRFlag_Unsigned)


// Messages supported by AnimGradient Widget
#define WM_WCSW_AG_SETUP	WM_WCSW_AG_BASE
#define WM_WCSW_AG_SYNC		(WM_WCSW_AG_BASE + 0x01)
// Returned from Widget
/*
#define WM_WCSW_FI_CHANGE	WM_WCSW_SMARTCHANGE
#define WM_WCSW_FI_PRECHANGE	WM_WCSW_SMARTPRECHANGE
#define WM_WCSW_FI_OPT1		WM_WCSW_FI_BASE + 0x02
#define WM_WCSW_FI_OPT2		WM_WCSW_FI_BASE + 0x03

// WPARAM values for WPARAM of WM_WCSW_FI_SYNC
#define WP_FISYNC_NONOTIFY	0x01
*/

// Convenient setup function
void ConfigureAG(NativeGUIWin Dest, int Target, AnimGradient *AG);


/* ********** The one, the only, the TimeLine Widget! ************* */

// longword Offsets into TimeLine class
enum
	{
	WCSW_TL_OFF_TLDATA =		0
//	WCSW_TL_OFF_ =	4,
//	WCSW_TL_OFF_ =	8,
//	WCSW_TL_OFF_ =			12
	};

class GraphSupport
	{
	public:
		virtual short GetActiveKey(void) = 0;
		virtual void SetActiveKey(long KnotNumber) = 0;
		virtual int Precision(void) = 0;
		virtual void AddKey(long Frame) = 0;
		virtual void RespondSubChannel(void) = 0;
		virtual void SetValue(double Value) = 0;
		virtual double GetDistance(long KnotNumber) = 0;
		virtual union KeyFrame **AreThereKnots(void) = 0;
		virtual void NewActiveKey(long KnotNumber) = 0;

	}; // GraphSupport

#define WCSW_TLW_KEYFRAME_SELECT	(0<<0)
#define WCSW_TLW_KEYFRAME_NEW 	(1<<0)
#define WCSW_TLW_KEYFRAME_MOVE	(1<<1)
#define WCSW_TLW_POINT_SELECTED	(1<<2)
#define WCSW_TLW_QUICK_DRAW	(1<<3)
#define WCSW_TLW_NO_CLEAR	(1<<4)

#ifdef _WIN32
#define WCSW_TLW_MADF_DRAWUPDATE		(1<<0)
#define WCSW_TLW_MADF_DRAWOBJECT		(1<<1)
#define WCSW_TLW_MADF_DRAWGRAPH			(1<<2)
#endif // _WIN32

/* *************************** ListView *********************** */

// Superclassed ListView with selectable column header
// longword Offsets into TimeLine class
enum
	{
	WCSW_CLV_OFF_ACTIVECOL =		0
	};

void ColumnedListViewSetActiveColumn(NativeControl NC, signed long ActiveCol);
signed long ColumnedListViewGetActiveColumn(NativeControl NC);

/* *************************** GridWidget *********************** */

// Messages supported by GridWidget Widget
#define WM_WCSW_GW_SETUP			WM_WCSW_GW_BASE
#define WM_WCSW_GW_SETSTYLE			(WM_WCSW_GW_BASE + 1)
#define WM_WCSW_GW_NMCDREFLECT		(WM_WCSW_GW_BASE + 2)
#define WM_WCSW_GW_LVNBLEREFLECT	(WM_WCSW_GW_BASE + 4) // LVN_BEGINLABELEDIT reflected from parent window for subclassing
#define WM_WCSW_GW_LVNELEREFLECT	(WM_WCSW_GW_BASE + 5) // LVN_ENDLABELEDIT reflected from parent window for subclassing
#define WM_WCSW_GW_LVNGDIREFLECT	(WM_WCSW_GW_BASE + 6) // LVN_GETDISPINFO reflected from parent window for subclassing
#define WM_WCSW_GW_LVNOCHREFLECT	(WM_WCSW_GW_BASE + 7) // LVN_ODCACHEHINT reflected from parent window for subclassing
#define WM_WCSW_GW_LVNBSREFLECT		(WM_WCSW_GW_BASE + 8) // LVN_BEGINSCROLL reflected from parent window for subclassing

/*
#define WM_WCSW_GW_SYNC		(WM_WCSW_GW_BASE + 0x01)
// Returned from Widget
#define WM_WCSW_GW_CHANGE	WM_WCSW_SMARTCHANGE
#define WM_WCSW_GW_PRECHANGE	WM_WCSW_SMARTPRECHANGE
#define WM_WCSW_GW_OPT1		(WM_WCSW_GW_BASE + 0x02)
#define WM_WCSW_GW_OPT2		(WM_WCSW_GW_BASE + 0x03)
#define WM_WCSW_GW_OPT3		(WM_WCSW_GW_BASE + 0x04)
#define WM_WCSW_GW_COLOR	(WM_WCSW_GW_BASE + 0x05)
#define WM_WCSW_GW_CHANGEARROW	(WM_WCSW_GW_BASE + 0x06) // same as WM_WCSW_GW_CHANGE, but changed via updown arrow
*/

// WPARAM values for WPARAM of WM_WCSW_GW_SYNC
//#define WP_FISYNC_NONOTIFY	0x01

// Style defines for GridWidget (not WS_ style styles, but used in GridWidgetInstance->GWStyles below.
#define WCSW_GW_STYLE_GREENBAR		0x01
#define WCSW_GW_STYLE_JOE			0x02 // this is really the only way we use it now...


// GridWidgetInstance is now defined in GridWidget.h


/* *************************** LinkWidget *********************** */

// Messages supported by LinkWidget Widget
#define WM_WCSW_LW_SETUP			WM_WCSW_LW_BASE
#define WM_WCSW_LW_SYNC				(WM_WCSW_LW_BASE + 1)

// Style defines for LinkWidget
//#define WCSW_LW_STYLE_	0x01

class LinkWidgetInstance
	{
	private:
		NativeControl TextField;
		NativeControl ActionButton;
		HMENU ActionPopupMenu;
		long NumJoes;
		
		void Sync(void);
		void HardLink(void);
		void HardLinkSelectedVectors(void);
		void FreeQuery(void);
		void EditQuery(void);
		void NewQuery(void);
		void EnableVectors(bool NewState);
		void SelectVectors(void);
		void DisplayPopup(int ScreenX, int ScreenY);

	public:
		GeneralEffect *ComponentHost;
		Database *DBHost;
		EffectsLib *EffectsHost;
		unsigned long int NewQueryFlags;
		LinkWidgetInstance() {NumJoes = 0; NewQueryFlags = NULL; EffectsHost = NULL; ComponentHost = NULL; DBHost = NULL; ActionPopupMenu = NULL; TextField = ActionButton = NULL;};
		~LinkWidgetInstance();		
		
		// Actual WndProc body, where it can access the private members, public, so it can be called from the PASCAL-linkage one
		long LinkWidgetInternalWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
	}; // LinkWidgetInstance

// Flags to tell LinkWidget what criteria to create new queries with
#define WM_WCSW_LW_NEWQUERY_FLAG_CONTROLPT		(1<<0)
#define WM_WCSW_LW_NEWQUERY_FLAG_VECTOR			(1<<1)
#define WM_WCSW_LW_NEWQUERY_FLAG_DEM			(1<<2)
#define WM_WCSW_LW_NEWQUERY_FLAG_ENABLED		(1<<3)
#define WM_WCSW_LW_NEWQUERY_FLAG_DISABLED		(1<<4)
#define WM_WCSW_LW_NEWQUERY_FLAG_LINE			(1<<5)
#define WM_WCSW_LW_NEWQUERY_FLAG_POINT			(1<<6)

/* *************************** TextColumnMarker *********************** */

// Messages supported by TextColumnMarker Widget
#define WM_WCSW_TC_SETUP		WM_WCSW_TC_BASE
#define WM_WCSW_TC_SETTEXT		(WM_WCSW_TC_BASE + 1)
#define WM_WCSW_TC_GETDATA		(WM_WCSW_TC_BASE + 2)
#define WM_WCSW_TC_DATACHANGED	(WM_WCSW_TC_BASE + 3)

// Style defines for TextColumnMarker
//#define WCSW_TC_STYLE_GREENBAR		0x01

class TextColumnMarkerWidgetInstance
	{
	// so the WndProc can access the private members
	friend long WINAPI TextColumnMarkerWndProc(NativeControl hwnd, UINT message, UINT wParam, LONG lParam);
	private:
		HFONT _MonospacedFont;
		HWND _HScroll, _VScroll;
		int _ScrollOffsetX, _ScrollOffsetY, _MaxTextLengthPixels, _NumLinesInBody, _StoredPixelWidthOfChar;

	public:
		const char *Text;
		std::vector<unsigned long int> ColumnMarkers;
		TextColumnMarkerWidgetInstance() {Text = NULL; _MonospacedFont = NULL; _HScroll = _VScroll = NULL; _ScrollOffsetX = _ScrollOffsetY = _MaxTextLengthPixels = _NumLinesInBody = 0;};
	}; // TextColumnMarkerWidgetInstance

class TrueColorIcon
	{
	public:
		unsigned int Width, Height;
		char *R, *G, *B;
		#ifdef _WIN32
		unsigned char *BitArray;
		unsigned long BASize;
		BITMAPINFO BMI;
		#endif // _WIN32

		TrueColorIcon();
		~TrueColorIcon();
	}; // TrueColorIcon

#endif //WCSWIDGETS_H
