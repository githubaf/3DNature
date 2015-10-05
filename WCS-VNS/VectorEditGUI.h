// VectorEditGUI.h
// Header file for  Vector Editor
// Created from ColorEditGUI.h on 11/12/97 by GRH
// Copyright 1997 Questar Productions

#ifndef WCS_VECTOREDITGUI_H
#define WCS_VECTOREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

class Database;
class Joe;
class InterCommon;
class Project;
class DigitizeGUI;
class EffectsLib;
class VectorPoint;

// Very important that we list the base classes in this order.
class VectorEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		Database *DBHost;
		Project *ProjHost;
		EffectsLib *EffectsHost;
		InterCommon *InterStash;
		Joe *Active, *Backup;
		DigitizeGUI *DigInfo;
		short IamResponsible;
		short Enabled, DrawEnabled, RenderEnabled, LineWeight, Red, Green, Blue, LineStyle, Class;
		double OldWidth, OldHeight, OldMaxEl, OldMinEl;
		AnimDoubleTime MaxEl, MinEl, VecNorth, VecSouth, VecWest, VecEast, PtX, PtY, PtZ, VecWidth, VecHeight,
			ShiftAmtX, ShiftAmtY, ShiftAmtZ, RotateAmtZ, ArbLat, ArbLon, ArbElev, ProjLat, ProjLon, ProjElev;
		AnimColorTime VectorColor;
	public:

		int ConstructError;
		static long ActivePage;

		VectorEditGUI(Database *DBSource, Project *ProjSource, EffectsLib *EffectsSource, InterCommon *ISource);
		~VectorEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void NewActive(void);
		void ConfigureWidgets(void);
		void ConfigureCoords(void);
		void DisableWidgets(void);
		void SelectPanel(short PanelID);
		short GetClass(Joe *Me);
		void SetScale(short Component, unsigned short WidID);
		void SetShift(short Component, unsigned short WidID);
		void SetRotate(unsigned short WidID);
		void ChangeNorth(void);
		void ChangeSouth(void);
		void ChangeWest(void);
		void ChangeEast(void);
		void ChangeLowElev(void);
		void ChangeHighElev(void);
		void SignalNewPoints(short Conform);
		void SignalPreChangePoints(void);
		void UndoPoints(void);
		void RestorePoints(void);
		void SetHorUnits(void);
		void ChangePointLat(void);
		void ChangePointLon(void);
		void ChangePointElev(void);
		void ChangeWidth(void);
		void ChangeHeight(void);
		void ScaleRotateShift(void);
		void NoJoeMessage(void);
		void InitDigitize(void);
		void CreateCopy(void);

		void DoLineWeight(void);
		void DoEnable(void);
		void DoDrawEnable(void);
		void DoRenderEnable(void);
		void DoObjectClass(void);
		void DoLineStyle(void);
		void DoRGBTwiddle(void);

		void CloseOrigin(void);
		void SetOrigin(void);
		void ReversePoints(void);
		void DeletePoints(void);
		void VectorToPath(void);
		void PathToVector(void);
		void InterpolatePoints(void);
		void SmoothPoints(void);
		void InsertPoints(void);
		void EvenSpacing(void);
		VectorPoint *Interpolate(VectorPoint *DataPts, unsigned long OrigNumPts, unsigned long NewNumPts,
			short EvenOutSpacing, short ConsiderElev, short Linear, short Geographic);
		VectorPoint *Insert(VectorPoint *OrigPtList, unsigned long OrigNumPts, unsigned long InsertAfter,
			unsigned long InsertNumPts, short InsertSpline, short Geographic);
		void SelectNewCoords(void);

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	}; // class VectorEditGUI

#endif // WCS_VECTOREDITGUI_H
