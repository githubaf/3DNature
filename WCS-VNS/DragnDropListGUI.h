// DragnDropListGUI.h
// Header file for ...Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_DRAGNDROPLISTGUI_H
#define WCS_DRAGNDROPLISTGUI_H

class RasterAnimHost;
class RasterAnimHostProperties;
class RootTextureParent;
class Texture;

#include "Application.h"
#include "Fenetre.h"

// Very important that we list the base classes in this order.
class DragnDropListGUI : public WCSModule, public GUIFenetre
	{
	public:

		int ConstructError;

		DragnDropListGUI(unsigned char IDSource, RasterAnimHostProperties *PropSource, RasterAnimHost **ListSource, 
			long NumItems, RootTextureParent *RootTextureCallBackSource, Texture *TextureCallBackSource, long *ListItemSource = NULL);
		~DragnDropListGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void BuildList(void);
		int DoApply(void);

	private:
		unsigned char MyListID;
		long NumListItems;
		long *ListItems;
		RasterAnimHost **RAHostList;
		RootTextureParent *RootTextureCallBackHost;
		Texture *TextureCallBackHost;
		RasterAnimHostProperties *DropSource;

	}; // class DragnDropListGUI

#endif // WCS_DRAGNDROPLISTGUI_H
