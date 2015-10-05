// GalleryGUI.h
// Header file for Atmosphere Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_GALLERYGUI_H
#define WCS_GALLERYGUI_H

#include "Application.h"
#include "Fenetre.h"

class EffectsLib;
class RasterAnimHost;
class Raster;
class BrowseData;

#define WCS_GALLERY_NUMNAILS	14

// Very important that we list the base classes in this order.
class GalleryGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		RasterAnimHost *Active;
		char **NameList, **CategoryNames, DefaultExt[8], ActiveDir[256], ContentDir[256];
		long NumItems, NumValidItems, NumCategories, NumRanges, ActiveRange, ActiveCategory, ActiveItem, FirstItemInRange, LastItemInRange, ActiveObjType, NumObjTypes,
			ObjectType[100], DisplayedItem[WCS_GALLERY_NUMNAILS];	// should be enough to keep us for a while
		long *NumCategoryItems, *InCategory;
		BrowseData *AuthorChunk, *BrowseList;
		Raster *RasterList;

	public:

		int ConstructError, GoneModal;

		GalleryGUI(EffectsLib *EffectsSource, RasterAnimHost *ActiveSource);
		~GalleryGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RasterAnimHost *GetActive(void) {return (Active);};

	private:

		void ClearAll(void);
		void ConfigureImages(long ScrollPos);
		void Cancel(void);
		void SetActiveItem(long NewActive);
		void CaptureActiveItem(long NewActive);
		// obsolete now
		//void NewObjectType(long NewPage);

		void FillCategories(BrowseData *LocalBrowseList, long LocalNumValidItems);
		long CountCategories(BrowseData *LocalBrowseList, long LocalNumValidItems);
		void BuildCategoryList(void);
		void NewCategory(long NewPage);

		void WriteHTMLComponentIndex(char *OutputPath);
		void WriteHTMLComponentIndex(FILE *HTML, char *OutputPath, char *ComponentPath, char *ComponentWild);

	}; // class GalleryGUI

#endif // WCS_GALLERYGUI_H
