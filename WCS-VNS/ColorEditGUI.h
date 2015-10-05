// ColorEditGUI.h
// Header file for ColorEditor
// Created from scratch on 6/15/99 gy Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#ifndef WCS_COLOREDITGUI_H
#define WCS_COLOREDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"
#include "GraphData.h"
#include "RasterDrawWidget.h"

#undef RGB // stupid Microsoft makes macros with hazardously-common names

class DiagnosticData;

enum
	{
	WCS_COLOREDIT_RESPOND_ONCE,
	WCS_COLOREDIT_RESPOND_MANY
	}; // response modes

enum
	{
	WCS_COLOREDIT_RESPOND_RGB,
	WCS_COLOREDIT_RESPOND_RED,
	WCS_COLOREDIT_RESPOND_GRN,
	WCS_COLOREDIT_RESPOND_BLU,
	WCS_COLOREDIT_RESPOND_HUE,
	WCS_COLOREDIT_RESPOND_SAT,
	WCS_COLOREDIT_RESPOND_VAL
	}; // response types

#define HueChanged (fabs(HSV[0] - LastHSV[0]) > .5)
#define SatValChanged (fabs(HSV[1] - LastHSV[1]) > .1 || fabs(HSV[2] - LastHSV[2]) > .1)

// Very important that we list the base classes in this order.
class ColorEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		double RGB[3], HSV[3], LastHSV[3];
		AnimColorTime *Active, Backup;
		char Configured, ResponseEnabled, RespondMode, RespondToWhat;
		unsigned long NumColorSamples;
		long SwatchItems;
		char *SwatchTable;
			
		void HSVTwiddle(void);
		void RGBTwiddle(void);
		void Cancel(void);

	public:

		int ConstructError, Sampling;
		struct RasterWidgetData HueWidData, ValueWidData, SwatchData;

		ColorEditGUI(AnimColorTime *ActiveSource);
		~ColorEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		AnimColorTime *GetActive(void) {return (Active);};
		void SelectSampleType(void);
		void RespondColorNotify(DiagnosticData *Data);
		char GetResponseEnabled(void)	{return (ResponseEnabled);};
		void ComputeColorPickerA(void);
		void ComputeColorPickerB(void);
		void ComputeOverlayA(void);
		void ComputeOverlayB(void);
		void SetRGBFromHue(double X, double Y);
		void SetRGBFromSatVal(double X, double Y);
		void SetRGBFromSwatch(double X, double Y);
		void SelectColorSwatch();
		void FillSwatchDrop(void);
		void MatchSetColorSwatch(void);
		void StoreLastColorSwatch(char *Name);

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	}; // class ColorEditGUI

#endif // WCS_COLOREDITGUI_H
