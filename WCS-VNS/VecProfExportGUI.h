// VecProfExportGUI.h
// Header file for Vector Profile Export window
// Built from scratch 10/31/02 Gary Huber
// Copyright 2002 Questar Productions. all rights reserved.

#ifndef WCS_VECPROFEXPORTGUI_H
#define WCS_VECPROFEXPORTGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

class Joe;
class Project;
class VectorExportData;

#define WCS_VECPROF_EXPORT_NUMCOLORS	9

// Very important that we list the base classes in this order.
class VecProfExportGUI : public WCSModule, public GUIFenetre
	{
	private:
		Joe *Active;
		VectorExportData *ExportData;
		FILE *fHandle;
		AnimColorTime ColorStandIns[WCS_VECPROF_EXPORT_NUMCOLORS];
		AnimDoubleDistance ElevationADD;

	public:

		int ConstructError;
		double VecLen;
		float VecMaxEl, VecMinEl;
		double ElevMinEl, ElevMaxEl;

		VecProfExportGUI(Project *ProjSource, Joe *ActiveSource);
		~VecProfExportGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		Joe *GetActive(void) {return (Active);};

	private:
		void BuildElevProfile(void);
		void FetchColors(void);
		void CalcVecLenHeight(void);
		void SetUnitsTexts(void);
		void CalcSizeFromScale(void);
		void CalcScaleFromSize(void);
		void ExportProfile(void);
		long IllustratorInit(double VecDPI, double RasDPI, long screenwidth, long screenheight);
		long IllustratorEnd(void);
		void LaunchIllustrator(void);

	}; // class VecProfExportGUI

#endif // WCS_VECPROFEXPORTGUI_H
