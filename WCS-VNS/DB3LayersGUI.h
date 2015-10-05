// DB3LayersGUI.h
// Header file for DB3 Layer Selection Editor
// Created  on 6/11/97 by GRH
// Copyright 1997 Questar Productions

#ifndef WCS_DB3LAYERSGUI_H
#define WCS_DB3LAYERSGUI_H


#include "Application.h"
#include "Fenetre.h"
#include "Types.h"

// Very important that we list the base classes in this order.
class DB3LayersGUI : public WCSModule, public GUIFenetre
	{
	private:
		DB3LayerList *LayerList;
		long *FieldsSelected, NumItems;
		int AbortStatus, ProcessData;
		
	public:

		int ConstructError;

		DB3LayersGUI(DB3LayerList *ListSource, long ItemSource, long *SelectedSource);
		~DB3LayersGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		int CheckAbort(void);

	}; // class DB3LayersGUI

#endif // WCS_DB3LAYERSGUI_H
