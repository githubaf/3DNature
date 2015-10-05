// VectorProfGUI.h
// Header file for Vector Profile GUI
// Built from EffectGradProfEditGUI.h on 6/27/97 by Gary Huber
// Copyright 1997 Questar Productions

#ifndef WCS_VECPROFGUI_H
#define WCS_VECPROFGUI_H

class Joe;
//class Database;
//class Param;
class DEM;
union KeyFrame;
class VectorScaleGUI;

#include "Application.h"
#include "Fenetre.h"
#include "WCSWidgets.h"
#include "GraphData.h"
#include "GraphWidget.h"

// Very important that we list the base classes in this order.
class VectorProfileGUI : public WCSModule, public GUIFenetre, public GraphDialog
	{
	private:
		Joe *Active, *Backup;
		Database *HostDB;
		NativeGUIWin GraphWidget;
		double Distance, Elevation, Value, Slope, MaxSlope, MinSlope;
		VectorScaleGUI *VecScaleGUI;
		AnimDoubleTime MaxValueADT, MinValueADT, ValueADT, DistanceADT;
		AnimDoubleDistance DistanceADD, ElevationADD;

	public:

		int ConstructError, Disabled, IgnoreNotify;
		struct GRData WidgetLocal;

		VectorProfileGUI(Database *DBSource, Joe *ActiveSource);
		~VectorProfileGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		int NewActiveVector(Joe *NewActive);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void Name(void);
		void SignalNewPoints(short Conform);
		void SignalPreChangePoints(void);
		void Undo(void);
		void Restore(void);
		void NewActiveNode(GraphNode *NewActive);
		void NextNode(void);
		void PrevNode(void);
		void SelectAllNodes(void);
		void ClearSelectNodes(void);
		void ToggleSelectNodes(void);
		void FetchElevation(void);
		void FetchSlope(void);
		void NewValue(void);
		void SetValueBySlope(void);
		void SetValuesBySlopeLimits(void);
		int SetDistanceADD(void);
		void UpdateNodeSelection(void);
		void UpdateVectorSelection(void);
		void UpdateVectorElevations(void);
		void RefreshGraph(void);
		void VectorScale(void);
		void ExportGraph(void);
		Joe *GetActive(void)	{return (Active);};

	}; // class VectorProfileGUI

#endif // WCS_VECPROFGUI_H
