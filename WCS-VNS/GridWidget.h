// GridWidget.h
// Implementation of the Grid/Listview widget used by the Database Editor
// Built from WCSWidgets.h on 5/6/2008 by Chris 'Xenon' Hanson
// Copyright 2008

#include "stdafx.h"

#include <vector>
#include <algorithm>
#include "GraphData.h"

#ifndef WCS_GRIDWIDGET_H
#define WCS_GRIDWIDGET_H

class Database;
class Joe;
class DBEditGUI;
class EffectsLib;
class Project;


class GridWidgetInstance
	{
	public:
		unsigned long GWStyles;
		std::vector<Joe *> *GridWidgetCache;
		HMENU HeaderPopupMenu;
		HICON CheckedIcon, UnCheckedIcon, DiasbledCheckIcon;
		HWND MyControl;
		HWND EditControl;
		HWND ComboControl;
		int ActiveEditColumn;
		unsigned long int ActiveEditItem;
		Database *DBHost;
		DBEditGUI *DBEHost;
		EffectsLib *HostLib;
		Project *ProjHost;
		AnimDoubleTime ElevationADT, DistanceADT, LatitudeADT, LongitudeADT;

		GridWidgetInstance();
		bool CreateEdit(int Item, int WhichColumn, bool ReadOnly, const char *InitialString);
		bool CreateCombo(int Item, int WhichColumn, std::vector<std::string> &ComboChoices, int CurSel);
		void DestroySubControls(void);
		const char *FetchTextForColumn(int Item, int WhichColumn, bool NoDecorations = false);
		bool FillColumnChoices(int Item, int WhichColumn, std::vector<std::string> &ComboChoices);
		int QueryActiveColumnChoice(int Item, int WhichColumn);
		bool HandleNewTextForColumn(int Item, int WhichColumn, const char *NewText);
		bool HandleNewChoiceForColumn(int Item, int WhichColumn, int NewChoice);
		int FetchColumnWidthPixels(int ColumnNum);
	}; // GridWidgetInstance


#endif // WCS_GRIDWIDGET_H
