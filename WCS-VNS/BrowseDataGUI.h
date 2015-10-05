// BrowseDataGUI.h
// Header file for Atmosphere Editor
// Created from scratch on 6/12/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#ifndef WCS_BROWSEDATAGUI_H
#define WCS_BROWSEDATAGUI_H

#include "Application.h"
#include "Fenetre.h"

class EffectsLib;
class RasterAnimHost;
class Raster;
class BrowseData;
class Thumbnail;

#define WCS_BROWSEDATA_NUMNAILS	6

// Very important that we list the base classes in this order.
class BrowseDataGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *EffectsHost;
		RasterAnimHost *Active;
		char **CategoryNames, DefaultExt[8], ActiveDir[256], DefaultName[256], BackupPath[512], BackupImage[256];
		long NumCategories, ActiveObjType, ActiveCategory, Listening;
		BrowseData *AuthorChunk;
		Raster *MyRast;

	public:

		int ConstructError, GoneModal;

		BrowseDataGUI(EffectsLib *EffectsSource, RasterAnimHost *ActiveSource);
		~BrowseDataGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RasterAnimHost *GetActive(void) {return (Active);};

	private:

		void ClearAll(void);
		void ConfigureImage(void);
		void FillCategories(BrowseData *BrowseList, long NumItems);
		long CountCategories(BrowseData *BrowseList, long NumItems);
		void BuildCategoryList(void);
		int SaveData(int SaveIt);
		void SetUserData(void);
		void NewImage(void);
		void CopyThumbnail(Thumbnail *NewThumb);
		void ClearThumbnail(void);
		int ImageNameChanged(void);
		void NewCategory(void);
		void CaptureUserData(void);

	}; // class BrowseDataGUI

#endif // WCS_BROWSEDATAGUI_H
