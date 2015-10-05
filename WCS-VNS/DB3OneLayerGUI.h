// DB3OneLayerGUI.h
// Header file for DB3 single layer selection editor
// Copied from DB3LayersGUI.h on 02/17/00 FPW2
// Copyright 2000 3D Nature

#ifndef WCS_DB3OneLayerGUI_H
#define WCS_DB3OneLayerGUI_H


#include "Application.h"
#include "Fenetre.h"
#include "Types.h"

// Very important that we list the base classes in this order.
class DB3OneLayerGUI : public WCSModule, public GUIFenetre
	{
	private:
		DB3LayerList *LayerList;
		long *FieldsSelected, NumItems;
		int AbortStatus, ProcessData;
		
	public:

		int ConstructError;

		DB3OneLayerGUI(DB3LayerList *ListSource, long ItemSource, long *SelectedSource);
		~DB3OneLayerGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		int CheckAbort(void);
		void SetCaption(char *Label);

	}; // class DB3LayersGUI

#endif // WCS_DB3LAYERSGUI_H
