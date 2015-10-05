// PathTransferGUI.h
// Header file for PathTransferGUI
// Created from ProjNewGUI.h on 7/14/01 GRH
// Copyright 2001 Questar Productions

#ifndef WCS_PATHTRANSFERGUI_H
#define WCS_PATHTRANSFERGUI_H

//class Project;
//class Database;
//class EffectsLib;
class RasterAnimHost;

#include "Application.h"
#include "Fenetre.h"
#include "GraphData.h"

enum
	{
	WCS_TRANSFER_KEYPLACE_EACHVERTEX,
	WCS_TRANSFER_KEYPLACE_EACHINTERVAL,
	WCS_TRANSFER_KEYPLACE_FIXEDCOUNT
	}; // vertex positioning

enum
	{
	WCS_TRANSFER_CONSTUNIT_INTERVAL,
	WCS_TRANSFER_CONSTUNIT_VELOCITY
	}; // constant unit types

enum
	{
	WCS_TRANSFER_INTERPTYPE_SPLINE,
	WCS_TRANSFER_INTERPTYPE_LINEAR
	}; // interpolation types

enum
	{
	WCS_TRANSFER_VELOCITYTYPE_NOCHANGE,
	WCS_TRANSFER_VELOCITYTYPE_CONSTANT
	}; // velocity types

enum
	{
	WCS_TRANSFER_VERTCOUNT_NOCHANGE,
	WCS_TRANSFER_VERTCOUNT_CHANGE
	}; // velocity types

enum
	{
	WCS_TRANSFER_VERTSPACE_NOCHANGE,
	WCS_TRANSFER_VERTSPACE_EVEN
	}; // velocity types

class AnimPath
	{
	public:
		AnimDoubleTime *Lat, *Lon, *Elev;

		AnimPath()	{Lat = Lon = Elev = NULL;};
		double MeasureSplinedPathLength(int ConsiderElev, double FrameRate, double PlanetRad);
		long CountUniqueKeys(void);
		int GetKeyFrameRange(double &MinDist, double &MaxDist);
		int GetNextUniqueKey(double &LastTime);
		int Smooth(int ConsiderElev, double FrameRate, double LatScaleMeters, long NumKeys);
	}; // class AnimPath

class TransferData
	{
	public:
		char VP_KeyPlace, VP_ConstUnit, ConsiderElev, InterpType, PV_VertPlace, ConformTopo, ConsiderTfx, PP_VelocityType,
			VV_VertType, VV_VertSpacing;
		AnimDoubleTime OutPathTime, OutVelocity, OutFrameInt, OutFrameRate, OutVertInt, ElevAdd, OutFirstKey;
		double InFrameRate, InPathTime, InVelocity, InFirstKey, InPathLength, AlternateElev;
		long OutNumKeys, OutNumVerts, InNumVerts;
		AnimPath InPath, OutPath;

		TransferData();

	}; // class TransferData

// Very important that we list the base classes in this order.
class PathTransferGUI : public WCSModule, public GUIFenetre
	{
	public:

		int ConstructError, MakeNew;
		long InType, OutType;
		char InCamTarget, OutCamTarget;
		Project *ProjHost;
		Database *DBHost;
		EffectsLib *EffectsHost;
		RasterAnimHost *TransferFrom, *TransferTo;
		TransferData TransData;

		PathTransferGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource,
			RasterAnimHost *TransferFromSource, RasterAnimHost *TransferToSource);
		~PathTransferGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		void HandleNotifyEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void ConfigureWidgets(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void BuildList(int InputOrOutput);
		void SelectPanel(void);
		void NewInputSelection(void);
		void NewOutputSelection(void);
		void ComputeIntervalFromPathTime(void);
		void ComputePathTimeFromInterval(void);
		void ComputeVelocityFromPathTime(void);
		void ComputePathTimeFromVelocity(void);
		void ComputeNumVerticesFromVertexSpacing(void);
		void ComputeVertexSpacingFromNumVertices(void);
		void ComputeIntervalFromNumVertices(void);
		void ComputeNumVerticesFromInterval(void);
		void Transfer(void);
		int TransferVectorToVector(void);
		int TransferVectorToPath(void);
		int TransferPathToVector(void);
		int TransferPathToPath(void);

	}; // class PathTransferGUI

#endif // WCS_PATHTRANSFERGUI_H
