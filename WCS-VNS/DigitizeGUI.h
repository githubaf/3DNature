// DigitizeGUI.h
// Header file for DEM Designer window
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_DIGITIZEGUI_H
#define WCS_DIGITIZEGUI_H

class Database;
class Project;
class EffectsLib;
class Joe;
class DiagnosticData;
class RasterAnimHost;

#include "Application.h"
#include "Fenetre.h"
#include "Notify.h"
#include "GraphData.h"
#include "GraphWidget.h"

#define WCS_DIGITIZE_MAX_OBJPTS 		4000	// this could be raised to 32767 since that is now WCS_DXF_MAX_OBJPTS
#define WCS_DIGITIZE_ADDPOINTS_NOT			0
#define WCS_DIGITIZE_ADDPOINTS_SINGLE		1
#define WCS_DIGITIZE_ADDPOINTS_SKETCH		2
#define WCS_DIGITIZE_ADDPOINTS_CONNECT		3

enum
	{
	WCS_DIGITIZE_ELEVTYPE_VECTOR = 0,
	WCS_DIGITIZE_ELEVTYPE_ANT,
	WCS_DIGITIZE_ELEVTYPE_PERSON,
	WCS_DIGITIZE_ELEVTYPE_SPARROW,
	WCS_DIGITIZE_ELEVTYPE_HAWK,
	WCS_DIGITIZE_ELEVTYPE_EAGLE,
	WCS_DIGITIZE_ELEVTYPE_SMALLPLANE,
	WCS_DIGITIZE_ELEVTYPE_JUMBOJET,
	WCS_DIGITIZE_ELEVTYPE_SATELLITE,
	WCS_DIGITIZE_ELEVTYPE_CELESTIAL
	}; // Elevation type
enum
	{
	WCS_DIGITIZE_DIGOBJTYPE_VECTOR = 0,
	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_REPLACE,
	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_APPEND,
	WCS_DIGITIZE_DIGOBJTYPE_VECTOR_EFFECT,
	WCS_DIGITIZE_DIGOBJTYPE_PATH, // camera, target, light, celestial
	WCS_DIGITIZE_DIGOBJTYPE_TARGETPATH
	}; // Digitize object type - also known as AddInfo.Mode

#define WCS_MAXDIG_NOTIFY_CHANGES	10

#define WCS_DIGCLASS_POINTS			220		
#define WCS_DIGCLASS_OUTPUT			221

#define WCS_SUBCLASS_FIRSTPOINT		0
#define WCS_SUBCLASS_PTSADDED		1
#define WCS_SUBCLASS_CURRENTPOINT	2
#define WCS_SUBCLASS_DRAWMODE		3
//#define WCS_SUBCLASS_PENNUM			4
#define WCS_SUBCLASS_DIGSTART		5
#define WCS_SUBCLASS_DIGDONE		6

#define WCS_ITEM_DIGDONE_ENDCAP		0
#define WCS_ITEM_DIGDONE_ABORT		1
#define WCS_ITEM_DIGDONE_ACCEPT		2

#define WCS_SUBCLASS_ADDTOOBJECT	0
#define WCS_SUBCLASS_ELEVTYPE		1
#define WCS_SUBCLASS_OBJECTTYPE		2
#define WCS_SUBCLASS_ELEV			3
#define WCS_SUBCLASS_SMOOTH			4
#define WCS_SUBCLASS_PTSPACE		5
#define WCS_SUBCLASS_SPEED			6

enum
	{
	WCS_DEMDESIGN_ELSOURCE_ELEVCTRL,
	WCS_DEMDESIGN_ELSOURCE_ENDPOINTS,
	WCS_DEMDESIGN_ELSOURCE_DEM
	}; // Point Mode

enum
	{
	WCS_DEMDESIGN_DMODE_ISOLINE,
	WCS_DEMDESIGN_DMODE_GRADIENT
	}; // Point Mode

struct DigitizePoints
	{
	Joe *AddToObj;
	RasterAnimHost *HostObj;
	long Point, FirstPoint, PointsAdded, NewCat;
	short Mode, DigObjectType, ElevType, Append;
	short DrawEnabled, RenderEnabled;
	short LineWeight;
	short LineStyle;
	short Red, Green, Blue;
	short Class;
	double Datum, Smooth, PtSpace, GroundSpeed, FirstLat, FirstLon, FirstElev;
	double Lat[WCS_DIGITIZE_MAX_OBJPTS];
	double Lon[WCS_DIGITIZE_MAX_OBJPTS];
	double Elev[WCS_DIGITIZE_MAX_OBJPTS];
	short ControlPtMode, ElSource, NoReversal;
	double Tension1, Tension2;
	AnimDoubleTime Elev1ADT, Elev2ADT, StdDev;
	AnimDoubleDistance ElevADD;
	}; // DigitizePoints


// Very important that we list the base classes in this order.
class DigitizeGUI : public WCSModule, public GUIFenetre, public GraphDialog, public SetCritter, public NotifyEx
	{
	private:
		Database *DBHost;
		Project *ProjHost;
		EffectsLib *EffectsHost;
		AnimColorTime VectorColor;
		long ActivePage, Profile;
		char PaletteTitle[200];
		char SummaryTitle[200];
		void SetAttribByCat(long Cat);
		void UpdatePaletteTitle(void);

	public:

		int ConstructError;
		short Initialized, Digitizing;
		struct DigitizePoints AddInfo;
		struct GRData WidgetLocal;

		DigitizeGUI(Project *ProjSource, EffectsLib *EffectsSource, Database *DBSource, short DigitizeActive = 0);
		~DigitizeGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		int ConfigureDigitize(short Mode, long ForceCategory = 0);
		void ConfigureWidgets(void);
		void ConfigProfile(void);
		void UpdateVecColor(void);
		void DoClose(void);
		int InitDigitize(int DigitizeActive = 0);
		long AddNextPoint(DiagnosticData *Data, int MouseMode);
		void EndAddPoints(void);
		void SetPaletteTitle(char *Synopsis, char *Suffix = NULL);
		void SetSummary(char *Synopsis);
		void SelectElevSource(void);
		void DoLineStyle(void);
		virtual void SetParam(int Notify, ...);
		virtual void GetParam(void *Value, ...);
		virtual void NewActiveNode(GraphNode *NewActive) {return;};

	}; // class DigitizeGUI

#endif // WCS_DIGITIZEGUI_H
