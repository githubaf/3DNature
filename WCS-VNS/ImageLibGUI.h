// ImageLibGUI.h
// Header file for Effects Library Editor
// Created from ColorEditGUI.h on 5/1/97 by CXH & GRH
// Copyright 1997 Questar Productions

#ifndef WCS_IMAGELIBGUI_H
#define WCS_IMAGELIBGUI_H

class EffectsLib;
class ImageLib;
class Project;
class GeneralEffect;
class Param;
class Raster;
class SequenceShell;
class DissolveShell;
class ColorControlShell;
class ImageManagerShell;
class RasterShell;
class GeoRefShell;
class DiagnosticData;

#include "Application.h"
#include "Fenetre.h"

enum
	{
	WCS_IMAGELIB_ADJUSTENDFRAME,
	WCS_IMAGELIB_ADJUSTENDIMAGE,
	WCS_IMAGELIB_ADJUSTSPEED,
	WCS_IMAGELIB_ADJUSTNUMLOOPS
	}; // sequence adjust values for radio buttons

// Very important that we list the base classes in this order.
class ImageLibGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *HostEffectsLib, *BrowseEffectsLib;
		ImageLib *HostLib, *BrowseLib;
		Project *HostProj;
		long ListItems, FormulaOutBand, FormulaInBand, SequenceAdjust;
		int Frozen;
		Raster *Active;
		RasterShell *ActiveShell;
		SequenceShell *ActiveSequence;
		DissolveShell *ActiveDissolve;
		Raster *ImageRast, *SequenceRast, *DissolveRast, *DissolveRast2, *ColorControlRast, *BrowseRast, *TileRast;
		long TVItemHandles[100], TVNumItems;
		char StashPath[512], StashName[512];
		unsigned char ReceivingDiagnostics;
		unsigned char ReceivingDiagnosticsShift;
		double LatEvent[2], LonEvent[2];
		//DrawingFenetre *Pane;

		#ifdef _WIN32
		HWND SeqDisPanel;
		#endif // _WIN32
		short CurPanel, CurSeqDisPanel;


	public:

		int ConstructError;

		ImageLibGUI(ImageLib *ImageSource, EffectsLib *EffectsSource, Project *ProjSource);
		~ImageLibGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleTreeChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, void *TreeItemData);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DisableButtons(void);
		void ListSelectOneItem(WIDGETID ListView, Raster *Active);
		void ListSelectOneItemByNumber(WIDGETID ListView, long NumSelection);
		void BuildList(ImageLib *SourceLib, WIDGETID ListView);
		long FindInList(Raster *FindMe);
		void BuildListEntry(char *ListName, Raster *Me);
		void BuildAttributeList(void);
		long BuildSequenceList(Raster *Rast, void *Parent);
		void UpdateTVText(void);
		void ConfigureSequence(SequenceShell *MyShell);
		void ConfigureDissolve(DissolveShell *MyShell);
		void ConfigureColorControl(ColorControlShell *MyShell);
		void ConfigureGeoRef(GeoRefShell *MyShell);
		void ConfigureImageManager(ImageManagerShell *MyShell);
		void ConfigureImageManagerCacheEst(void);
		void SetBounds(DiagnosticData *Data);
		void ShiftBounds(DiagnosticData *Data);
		void ConfigureSequenceColors(ColorControlShell *MyShell);
		void AddSequenceDissolve(void);
		void AddSequence(void);
		void AddDissolve(void);
		void AddColorControl(void);
		void AddGeoRef(void);
		void AddImageManager(void);
		void SelectPanel(short PanelID, short TabRemoved);
		long SetTabByTitle(char *MatchText);
		int CheckTabExists(char *MatchText);
		void AddImage(char UseSimpleRequester = 0);
		void SelectImage(void);
		void SelectBrowseImage(void);
		void SelectSequenceOrDissolve(RasterShell *NewShell);
		void StartBehavior(void);
		void EndBehavior(void);
		void SelectDissolveTarget(void);
		void RevealThumbnail(void);
		void DoBandAssignment(short ButtonID);
		void DoFormulaOutBand(void);
		void DoFormulaInBand(void);
		void ResyncColorFormula(ColorControlShell *MyShell);
		void ResyncTransparency(ColorControlShell *MyShell);
		void DoTransparencySelect(char Item);
		void DisableColorControlWidgets(ColorControlShell *MyShell);
		void RemoveAttribute(char RemoveAttrType);
		void ChangeEnabled(void);
		void NewSequenceEvent(void);
		void NewSequenceStartFrame(void);
		void NewSequenceStartSpace(void);
		void NewDissolveStartFrame(void);
		void NewDissolveEndFrame(void);
		void NewDissolveDuration(void);
		void Name(void);
		void RemovePartialSequenceDissolve(void);
		void RemovePartialSequence(void);
		void RemovePartialDissolve(void);
		void AdjustSequenceDissolvePosition(int Direction);
		void EditApplication(void);
		void ApplyMemoryToAll(void);
		void ApplyLoadFastToAll(void);
		void SelectNewCoords(void);
		Raster *GetNextSelected(long &LastOBN);
		unsigned long EstimateIMMaxCacheBytes(void);

		void ResetListNames(void);
		void ResetListEnabled(void);
		void BrowseFile(void);
		void SelectAll(void);
		void CopyToProj(void);
		void RemoveImage(int RemoveAll, int RemoveUnusedOnly);
		void UpdateImage(int UpdateCoordSys);
		void Freeze(void)	{Frozen = 1;};
		void UnFreeze(void)	{Frozen = 0;};
		bool GetFrozen(void) const {return(Frozen == 1);};
		Raster *GetNextSelected(Raster *CurSelection);

		void CreateTileableImage(void);

	}; // class ImageLibGUI

#endif // WCS_IMAGELIBGUI_H
