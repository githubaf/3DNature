// TableListGUI.h
// Header file for TableList GUI
// Created from scratch on 1/4/01 by Gary R. Huber. Happy birthday!
// Copyright 2001 Questar Productions. All rights reserved.

#ifndef WCS_TABLELISTGUI_H
#define WCS_TABLELISTGUI_H

#include "Application.h"
#include "Fenetre.h"

class CoordSys;
class GenericTable;

// Very important that we list the base classes in this order.
class TableListGUI : public WCSModule, public GUIFenetre
	{
	private:
		CoordSys *Active;
		GenericTable *TableHost;
		char ApplyTo, Fields[4][64], DataType[64];
		long FirstSelection, GoneModal;


	public:

		int ConstructError;

		TableListGUI(CoordSys *ActiveSource, GenericTable *TableSource, char *FieldSource[4], char ApplyToSource, 
			char *DataTypeSource, long CurrentSelection);
		~TableListGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		CoordSys *GetActive(void) {return (Active);};
		void SetModal(void)	{GoneModal = 1; GoModal();};

	private:

		void ApplySelection(void);
		void UpdateUsage(void);
		void BuildList(void);

	}; // class TableListGUI

#endif // WCS_TABLELISTGUI_H
