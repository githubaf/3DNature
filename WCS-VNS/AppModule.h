// AppModule.h
// Root class definition for input-capable sections of the program
// Created from scratch on 24 May 1995 by Chris "Xenon" Hanson
// Copyright 1995

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_APPMODULE_H
#define WCS_APPMODULE_H

#include "Types.h"

class AppEvent;
class WCSApp;
class WCSModule;
class Fenetre;
class RasterAnimHost;

class WCSModule
	{
	protected:
		AppEvent *Activity;
		WCSApp *AppScope;
		NativeAnyWin OriginWindow;
		Fenetre *OriginFen;
	public:
		// Generic Event Handling, somewhat obsoleted...
		virtual long HandleEvent(void) {return(0);};
		virtual void HandleNotifyEvent(void) {return;};
		
		// These are no longer passed as variables because they're common to all...
		inline void SetCommonVars(AppEvent *Act, WCSApp *AppS, NativeAnyWin NativeOri = NULL, Fenetre *NativeFen = NULL) {Activity = Act; AppScope = AppS; OriginWindow = NativeOri; OriginFen = NativeFen;};
		void SetEvent(AppEvent *NewEvent) {Activity = NewEvent;};
		AppEvent *GetEvent(void) const {return(Activity);};
		
		// Specific virtual platform-independent events
		virtual long HandleKeyDown(int Key, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleKeyUp(int Key, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleKeyPress(int Key, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleCloseWin(NativeGUIWin NW) {return(0);};
		virtual long HandleInquireScrollColor(NativeControl Widget) {return(0);};
		virtual long HandleDrawItem(int CtrlID, void *DrawStuff) {return(0);};
		virtual long HandleGetMinMaxInfo(void *MinMaxData) {return(0);};
		virtual long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID) {if(!ScrollCode) return(5); else return(0);};
		virtual long HandleWindowInit(void) {return(0);};
		virtual long HandleLeftButtonDown(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleLeftButtonUp(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleLeftButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMiddleButtonDown(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMiddleButtonUp(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMiddleButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleRightButtonDown(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleRightButtonUp(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleRightButtonDoubleClick(long X, long Y, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right) {return(0);};
		virtual long HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right) {return(0);};
		virtual long HandleMouseWheelVert(long X, long Y, float Amount, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMouseWheelHoriz(long X, long Y, float Amount, char Alt, char Control, char Shift) {return(0);};
		virtual long HandleMeasureItem(int CtrlID, void *MeasureData) {return(0);};
		virtual long HandleRepaint(NativeGUIWin NW, NativeDrawContext NDC) {return(0);};
		virtual long HandlePreReSize(void *ResizeData) {return(0);};
		virtual long HandleReSized(int ReSizeType, long NewWidth, long NewHeight) {return(0);};
		virtual long HandleMoved(long NewX, long NewY) {return(0);};
		virtual long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID) {return(0);};
		virtual long HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long OldTreeItem, unsigned long NewTreeItem, void *TreeItemData) {return(0);};
		virtual long HandleTreeExpand(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char Pre, char Expand) {return(0);};
		virtual long HandleTreeBeginDrag(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, long DragX, long DragY) {return(0);};
		virtual long HandleTreeBeginLabelEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData) {return(0);};
		virtual long HandleTreeEndLabelEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData, char *NewText) {return(0);};
		virtual long HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived) {return(0);};
		virtual long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIPreChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIChangeArrow(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIOpt1(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIOpt2(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleFIOpt3(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID) {return(0);};
		virtual long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID) {return(0);};
		virtual long HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleStringLoseFocus(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData) {return(0);};
		virtual long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData) {return(0);};
		virtual long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData) {return(0);};
		virtual long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleListViewColSel(NativeControl Handle, NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleTextColumnMarkerChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long ChangeMode) {return(0);};
		virtual long HandleShowAdvanced(NativeGUIWin NW, bool NewState) {return(0);};
		virtual long HandleMenuSelect(int MenuID) {return(0);};
		virtual long HandlePopupMenuSelect(int MenuID) {return(0);};
		virtual long HandleTitlebarButtonSelect(NativeGUIWin NW, int CtrlID) {return(0);};
		virtual long HandleBackgroundCrunch(int Siblings) {return(0);}; // return 1 when complete
		virtual long HandleGainFocus(void) {return(0);};
		virtual long HandleLoseFocus(void) {return(0);};
	}; // WCSModule

#endif // WCS_APPMODULE_H
