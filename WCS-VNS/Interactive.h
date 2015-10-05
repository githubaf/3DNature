// Interactive.h
// Header file for common Interactive elements
// Created from MapViewGUI.h on 5/13/96 by CXH
// Copyright 1996

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_INTERACTIVE_H
#define WCS_INTERACTIVE_H

#undef RGB // stupid Microsoft makes macros with hazardously-common names

class CamViewGUI;
class ViewGUI;
class DigitizeGUI;
class VectorPoint;
class VertexData;
class PolygonData;
struct ChunkIODetail;
class Joe;
class DEM;
class Project;

#include "Application.h"
#include "Types.h"
#include "Notify.h"
#include "Database.h"
#include "GraphData.h"

#define WCS_MAXINTER_NOTIFY_CHANGES		20
#define WCS_MAX_ECOLEGEND_VALUES		6

#define WCS_FRAMERATE_NTSC_VIDEO	30
#define WCS_FRAMERATE_PAL_VIDEO		25
#define WCS_FRAMERATE_FILM			24
#define WCS_FRAMERATE_SINGLEFRAME	1

// Camera View Flags
#define CAMVIEW_GRID_LON				(1 << 0)
#define CAMVIEW_GRID_LAT				(1 << 1)

#define CAMVIEW_BOUNDS_COMPASS			(1 << 0)
#define CAMVIEW_BOUNDS_LAND				(1 << 1)
#define CAMVIEW_BOUNDS_PROFILE			(1 << 2)
#define CAMVIEW_BOUNDS_BOX				(1 << 3)
#define CAMVIEW_BOUNDS_GRID				(1 << 4)
#define CAMVIEW_BOUNDS_OPENGL			(1 << 5)
#define CAMVIEW_BOUNDS_OPENGLOBJECTS	(1 << 6)

#define CAMVIEW_MOVE_CONSTRAIN_X		(1 << 0)
#define CAMVIEW_MOVE_CONSTRAIN_Y		(1 << 1)
#define CAMVIEW_MOVE_CONSTRAIN_Z		(1 << 2)
#define CAMVIEW_MOVE_RECTRADIAL			(1 << 3)
#define CAMVIEW_MOVE_ECOREGION			(1 << 4)
#define CAMVIEW_MOVE_LOCKINTER			(1 << 5)

#define CAMVIEW_AUTO_WIRE				(1 << 0)
#define CAMVIEW_AUTO_SOLID				(1 << 1)
#define CAMVIEW_AUTO_SUNSHADE			(1 << 2)
#define CAMVIEW_AUTO_ELSHADE			(1 << 3)
#define CAMVIEW_AUTO_ECOSHADE			(1 << 4)
#define CAMVIEW_AUTO_DIAGNOSTIC			(1 << 5)

#define CAMVIEW_OPENGL_FOG				(1 << 0)
#define CAMVIEW_OPENGL_AA				(1 << 1)
#define CAMVIEW_OPENGL_SMOOTH			(1 << 2)

// Map View Inter Flags
#define WCS_MAPVIEW_INTER_CAMFOCUS		(1 << 0)
#define WCS_MAPVIEW_INTER_SUNLIGHT		(1 << 1)
#define WCS_MAPVIEW_INTER_CELEST		(1 << 2)
#define WCS_MAPVIEW_INTER_HAZE			(1 << 3)
#define WCS_MAPVIEW_INTER_CLOUDS		(1 << 4)
#define WCS_MAPVIEW_INTER_WAVES			(1 << 5)
#define WCS_MAPVIEW_INTER_MSTRING		(1 << 6)
#define WCS_MAPVIEW_INTER_RULER			(1 << 7)

// for vector editing
enum
	{	// don't mess with this order! Add more if necessary at the end
	WCS_VECTOR_REFERENCE_GEOCOORD,
	WCS_VECTOR_REFERENCE_PROJCOORD,
	WCS_VECTOR_REFERENCE_ARBCOORD,
	WCS_VECTOR_REFERENCE_VECORIGIN,
	WCS_VECTOR_REFERENCE_VECCENTER,
	WCS_VECTOR_REFERENCE_ACTIVEPT,
	WCS_VECTOR_REFERENCE_SELPTAVG
	}; // Vector Reference Control

enum
	{	// don't mess with this order! Add more if necessary at the end
	WCS_VECTOR_INSERT_LINEAR,
	WCS_VECTOR_INSERT_SPLINE
	}; // Vector Insert method

enum
	{	// don't mess with this order! Add more if necessary at the end
	WCS_VECTOR_DISPLAYMODE_ABSOLUTE,
	WCS_VECTOR_DISPLAYMODE_RELATIVE
	}; // Vector Coord Display mode

enum
	{	// don't mess with this order! Add more if necessary at the end
	WCS_VECTOR_PTOPERATE_ALLPTS,
	WCS_VECTOR_PTOPERATE_MAPSELECTED,
	WCS_VECTOR_PTOPERATE_SINGLEPT,
	WCS_VECTOR_PTOPERATE_PTRANGE
	}; // Vector Operation points

enum
	{
	WCS_VECTOR_HUNITS_DEGREES = 0,
	WCS_VECTOR_HUNITS_KILOMETERS,
	WCS_VECTOR_HUNITS_METERS,
	WCS_VECTOR_HUNITS_DECIMETERS,
	WCS_VECTOR_HUNITS_CENTIMETERS,
	WCS_VECTOR_HUNITS_MILLIMETERS,
	WCS_VECTOR_HUNITS_MILES,
	WCS_VECTOR_HUNITS_YARDS,
	WCS_VECTOR_HUNITS_FEET,
	WCS_VECTOR_HUNITS_INCHES
	}; // Hor Units

enum
	{
	WCS_VECTOR_VUNITS_KILOMETERS = 0,
	WCS_VECTOR_VUNITS_METERS,
	WCS_VECTOR_VUNITS_DECIMETERS,
	WCS_VECTOR_VUNITS_CENTIMETERS,
	WCS_VECTOR_VUNITS_MILLIMETERS,
	WCS_VECTOR_VUNITS_MILES,
	WCS_VECTOR_VUNITS_YARDS,
	WCS_VECTOR_VUNITS_FEET,
	WCS_VECTOR_VUNITS_INCHES
	}; // Vert Units

// for import data
enum
	{
	WCS_IMPORTDATA_REFERENCE_IGNORE = 0,
	WCS_IMPORTDATA_REFERENCE_LOWXY,
	WCS_IMPORTDATA_REFERENCE_ZERO
	};


// Notify class
#define WCS_INTERCLASS_CAMVIEW		220
#define WCS_INTERCLASS_MAPVIEW		221
#define WCS_INTERCLASS_VECTOR		223
#define WCS_INTERCLASS_MISC			224
#define WCS_INTERCLASS_TIME			226

// Time subclasses
enum
	{
	WCS_TIME_SUBCLASS_TIME,
	WCS_TIME_SUBCLASS_FRAME
	}; // Time subclasses

enum
	{
	WCS_INTERMAP_SUBCLASS_MISC
	}; // map view subclasses

enum
	{
	WCS_INTERCAM_SUBCLASS_GRANULARITY,
	WCS_INTERCAM_SUBCLASS_SETTINGS
	}; // cam view subclasses

// draw subclasses
enum
	{
	WCS_INTERCAM_SUBCLASS_CLOUDS,
	WCS_INTERCAM_SUBCLASS_WAVES,
	WCS_INTERCAM_SUBCLASS_DEMBUILD,
	WCS_INTERCAM_SUBCLASS_DEMFRACTAL
	}; // draw subclasses

// Item codes for CamView
enum
	{
	// granularity
	WCS_INTERCAM_ITEM_GRIDSAMPLE,
	// settings
	WCS_INTERCAM_ITEM_SETTINGS_TFXPREVIEW,
	WCS_INTERCAM_ITEM_SETTINGS_TFXREALTIME
	}; // item


// Item codes for MapView
enum
	{
	WCS_INTERMAP_ITEM_FOLLOWTERRAIN,
	WCS_INTERMAP_ITEM_POINTSMODE
	}; // items for map view

// Subclass codes for Vectors
enum
	{
	WCS_INTERVEC_SUBCLASS_VALUE,
	WCS_INTERVEC_SUBCLASS_PREOPERATE,
	WCS_INTERVEC_SUBCLASS_OPERATE,
	WCS_INTERVEC_SUBCLASS_BACKUP
	}; // subclass

// Item Codes for Vectors
enum
	{
	WCS_INTERVEC_ITEM_SCALE,
	WCS_INTERVEC_ITEM_SCALEAMOUNT,
	WCS_INTERVEC_ITEM_SHIFT,
	WCS_INTERVEC_ITEM_SHIFTAMOUNT,
	WCS_INTERVEC_ITEM_ROTATE,
	WCS_INTERVEC_ITEM_ROTATEAMOUNT,
	WCS_INTERVEC_ITEM_PRESERVEXY,
	WCS_INTERVEC_ITEM_SMOOTH,
	WCS_INTERVEC_ITEM_SMOOTHING,
	WCS_INTERVEC_ITEM_CONSIDERELEV,
	WCS_INTERVEC_ITEM_INTERPPTS,
	WCS_INTERVEC_ITEM_INSERTPTS,
	WCS_INTERVEC_ITEM_INSERTAFTER,
	WCS_INTERVEC_ITEM_ORIGINVERTEX,
	WCS_INTERVEC_ITEM_FIRSTRANGEPT,
	WCS_INTERVEC_ITEM_LASTRANGEPT,
	WCS_INTERVEC_ITEM_PTRELATIVE,
	WCS_INTERVEC_ITEM_PTOPERATE,
	WCS_INTERVEC_ITEM_TOPOCONFORM,
	WCS_INTERVEC_ITEM_INSERTSPLINE,
	WCS_INTERVEC_ITEM_REFCONTROL,
	WCS_INTERVEC_ITEM_ARBREFCOORD,
	WCS_INTERVEC_ITEM_PROJREFCOORD,
	WCS_INTERVEC_ITEM_REFCOORD,
	WCS_INTERVEC_ITEM_HORUNITS,
	WCS_INTERVEC_ITEM_VERTUNITS,
	WCS_INTERVEC_ITEM_SETPOINTSELECT,
	WCS_INTERVEC_ITEM_ADDPOINTSELECT,
	WCS_INTERVEC_ITEM_TOGGLEPOINTSELECT,
	WCS_INTERVEC_ITEM_PROJREFCOORDFLOAT
	}; // Value items

enum
	{
	WCS_INTERVEC_ITEM_SHIFTPTLAT,
	WCS_INTERVEC_ITEM_SHIFTPTLON,
	WCS_INTERVEC_ITEM_SHIFTPTELEV,
	WCS_INTERVEC_ITEM_SHIFTPTLONLAT,
	WCS_INTERVEC_ITEM_SHIFTPTLONLATELEV,
	WCS_INTERVEC_ITEM_SCALEPTLAT,
	WCS_INTERVEC_ITEM_SCALEPTLON,
	WCS_INTERVEC_ITEM_SCALEPTLONLAT,
	WCS_INTERVEC_ITEM_ROTATEPTDEG
	}; // items for vector operations

enum
	{
	WCS_INTERVEC_ITEM_BACKUP,
	WCS_INTERVEC_ITEM_UNDO,
	WCS_INTERVEC_ITEM_FREEZE,
	WCS_INTERVEC_ITEM_RESTORE
	}; // Backup items

// Components for Vectors
enum
	{
	WCS_INTERVEC_COMP_X,
	WCS_INTERVEC_COMP_Y,
	WCS_INTERVEC_COMP_Z
	}; // coord components

// Misc class
enum
	{
	WCS_INTERMISC_SUBCLASS_MISC
	}; // subclasses for misc

enum
	{
	WCS_INTERMISC_ITEM_PROJFRAMERATE,
	WCS_INTERMISC_ITEM_DIGITIZEDRAWMODE
	}; // items for misc

// Diagnostic Data
#define WCS_NOTIFYCLASS_DIAGNOSTICDATA			240
#define WCS_SUBCLASS_DIAGNOSTIC_DATA			1
#define WCS_SUBCLASS_DIAGNOSTIC_DISPLAYBUF		2
#define WCS_SUBCLASS_DIAGNOSTIC_THRESHOLD		3
#define WCS_SUBCLASS_DIAGNOSTIC_UPDATETHRESHOLD	4
#define WCS_SUBCLASS_DIAGNOSTIC_ENDDIGITIZE		5
#define WCS_SUBCLASS_DIAGNOSTIC_THUMBNAIL		6

#define WCS_DIAGNOSTIC_ITEM_MOUSEDOWN	1
#define WCS_DIAGNOSTIC_ITEM_MOUSEDRAG	2


enum
	{
	WCS_DIAGNOSTIC_ELEVATION,
	WCS_DIAGNOSTIC_LATITUDE,
	WCS_DIAGNOSTIC_LONGITUDE,
	WCS_DIAGNOSTIC_SLOPE,
	WCS_DIAGNOSTIC_ASPECT,
	WCS_DIAGNOSTIC_RELEL,
	WCS_DIAGNOSTIC_ILLUMINATION,
	WCS_DIAGNOSTIC_REFLECTION,
	WCS_DIAGNOSTIC_Z,
	WCS_DIAGNOSTIC_DISTANCE,
	WCS_DIAGNOSTIC_NORMALX,
	WCS_DIAGNOSTIC_NORMALY,
	WCS_DIAGNOSTIC_NORMALZ,
	WCS_DIAGNOSTIC_RGB,
	WCS_DIAGNOSTIC_ALPHA,
	WCS_DIAGNOSTIC_OBJECT,
	WCS_DIAGNOSTIC_NUMBUFFERS
	};

#define WCS_MAX_DIAGNOSTIC_VALUES	WCS_DIAGNOSTIC_RGB

class DiagnosticData
	{
	public:
		double Value[WCS_MAX_DIAGNOSTIC_VALUES], RefLat, RefLon;
		RasterAnimHost *Object;
		long PixelX, PixelY, PixelZ, MoveX, MoveY, MoveZ, DimX, DimY, ViewSource;
		unsigned char DataRGB[3], Alpha, DisplayedBuffer;
		char ValueValid[WCS_DIAGNOSTIC_NUMBUFFERS], ThresholdValid;

		DiagnosticData();

	}; // class DiagnosticData

class VectorEdit
	{
	friend class InterCommon;
	friend class Project;
	friend class ImageFormatRAW;

	private:

		short Scale[3], Shift[3], Rotate, Smooth[3], PreserveXY, ConsiderElev, TopoConform, PtRelative, PtOperate,
			InsertSpline, RefControl, HorUnits, VertUnits, ProjRefCoordsFloating;
		unsigned long InterpPts, InsertPts, InsertAfter, OriginVertex, FirstRangePt, LastRangePt,
			RestoreNumPts, UndoNumPts, RefNumPts, SelectedNumPts;
		double ScaleAmt[3], ShiftAmt[3], RotateAmt, Smoothing, ArbRefCoord[3], ProjRefCoord[3], RefCoord[3],
			HUnitScale, VUnitScale;
		VectorPoint *RestorePts, *UndoPts, *RefPts, *SelPts[WCS_DXF_MAX_OBJPTS], *SelRefPts[WCS_DXF_MAX_OBJPTS];
		unsigned short SelPtsIndex[WCS_DXF_MAX_OBJPTS];
		Joe *Active;
		VectorPoint *ActivePoint;

		void SetHUnitScale(void);
		void SetVUnitScale(void);
	public:
		VectorEdit();
		~VectorEdit();

	}; // class VectorEdit

class InterCommon : public SetCritter, public NotifyEx, public WCSModule
	{
	friend class CamViewGUI;
	friend class ViewGUI;
	friend class DigitizeGUI;
	friend class VectorEditGUI;
	friend class DefaultGUI;
	friend class DEMMerger;
	friend class Project;
	friend class ExportFormatWCSVNS;
	friend class ImageFormatRAW;
	private:
		double ProjFrameRate, ActiveTime;
		char FollowTerrain, TfxPreview, TfxRealtime;
		unsigned short EditPointsMode;
		short DigitizeDrawMode;
		unsigned long GridSample;
		VectorEdit VE;
		Database *DBHost;
		Project *ProjHost;
		DEM *ActiveDEM;

		void SetNewObj(Joe *NewGuy);
		void PointsChanged(Joe *Active);
		DEM *FindAndLoadDEM(double Lat, double Lon);
		void ConformToTopo(void);

	public:

		// DigitizeGUI
		AnimDoubleTime DigElevation, DigSmooth, DigSpace, DigSpeed, GridOverlaySize;

		void SetDigDrawMode(short NewMode) {DigitizeDrawMode = NewMode;};
		short GetDigDrawMode(void) {return(DigitizeDrawMode);};

		unsigned short MVPointsMode(void) {return (EditPointsMode);};

		// Get functions - mostly inline
		unsigned long GetGridSample(void) {return(GridSample);};
		short GetHorUnits(void) {return(VE.HorUnits);};
		short GetVertUnits(void) {return(VE.VertUnits);};
		short GetScale(short Component) {return(VE.Scale[Component]);};
		double GetScaleAmt(short Component) {return(VE.ScaleAmt[Component]);};
		short GetShift(short Component) {return(VE.Shift[Component]);};
		double GetShiftAmt(short Component) {return(VE.ShiftAmt[Component]);};
		short GetRotate(void) {return(VE.Rotate);};
		double GetRotateAmt(void) {return(VE.RotateAmt);};
		short GetPointOperate(void) {return(VE.PtOperate);};
		short GetPreserveXY(void) {return(VE.PreserveXY);};
		void GetActivePointCoords(double &Lon, double &Lat, float &Elev);
		unsigned long GetNumSelectedPoints(void);
		int GetPointSelState(unsigned long Pt);
		int GetPointSelState(VectorPoint *Pt);
		int SetPointSelState(unsigned long Select, int State);
		double GetProjRefCoords(int Component) {return(VE.ProjRefCoord[Component]);};
		double GetRefCoords(int Component) {return(VE.RefCoord[Component]);};
		short GetPtRelative(void) {return(VE.PtRelative);};
		unsigned long GetInterpPts(void) {return(VE.InterpPts);};
		unsigned long GetInsertPts(void) {return(VE.InsertPts);};
		unsigned long GetInsertAfter(void) {return(VE.InsertAfter);};
		short GetInsertSpline(void) {return(VE.InsertSpline);};
		double GetSmoothing(void) {return(VE.Smoothing);};
		short GetSmooth(short Component) {return(VE.Smooth[Component]);};
		short GetConsiderElev(void) {return(VE.ConsiderElev);};
		char GetFollowTerrain(void) {return(FollowTerrain);};
		double GetFrameRate(void) {return(ProjFrameRate);};
		double GetActiveTime(void) {return(ActiveTime);};
		void SetActiveTime(double NewTime);
		void SetActiveTimeAndRate(double NewTime, double NewRate);
		long GetActiveFrame(void)	{return ((long)(ActiveTime * ProjFrameRate + .5));};
		void SetActiveFrame(long Frame)	{(SetActiveTime(ProjFrameRate > 0.0 ? Frame / ProjFrameRate: 0.0));};
		unsigned long GetOriginVertex(void) {return(VE.OriginVertex);};
		VectorPoint *GetFirstSelectedPt(unsigned long &Marker) {if (VE.SelectedNumPts > 0) return (VE.SelPts[Marker = 0]); else return (NULL);};
		VectorPoint *GetNextSelectedPt(unsigned long &Marker) {if (VE.SelectedNumPts > ++Marker) return (VE.SelPts[Marker]); else return (NULL);};
		void SyncFloaters(Database *CurDB);
		void SetFloating(short NewFloating, Database *CurDB);
		short GetFloating(void) {return (VE.ProjRefCoordsFloating);};
		char GetTfxPreview(void)	{return (TfxPreview);};
		char GetTfxRealtime(void)	{return (TfxRealtime);};

		// vector point stuff
		void ClearAllPts(void);
		void UndoPoints(Joe *Active);
		void RestorePoints(Joe *Active);
		int SelectPoint(Joe *Active, unsigned long Select);
		int UnSelectPoint(Joe *Active, unsigned long Select);
		void SelectAllPoints(Joe *Active);
		void UnSelectAllPoints(void);
		int CopyRefToUndo(void);
		int CopyRestoreToActive(Joe *Active);
		int CopyActiveToRef(Joe *Active);
		void RebuildSelectedList(Joe *Active);
		void SetActivePoint(Joe *Active);
		VectorPoint *GetActivePoint(void) {return(VE.ActivePoint);};
		void SetPointsSelected(void);

		// moving vectors
		void ChangePointLat(double NewLat);
		void ChangePointLon(double NewLon);
		void ChangePointElev(float NewElev);
		void ScaleRotateShift(double PlanetRad, double LocalLonScale, double LocalLatScale, double ElevScale, double RotateDeg,
			double LonShift, double LatShift, float ElevShift, short UseDefaultUnits);
		void ShiftPointLat(double PlanetRad, double LatShift, short UseDefaultUnits);
		void ShiftPointLon(double PlanetRad, double LonShift, short UseDefaultUnits);
		void ShiftPointLonLat(double PlanetRad, double LonShift, double LatShift, short UseDefaultUnits);
		void ShiftPointLonLatElev(double PlanetRad, double LonShift, double LatShift, double Elev, short UseDefaultUnits);
		void ShiftPointElev(float ElevShift);
		void ScalePointLat(double PlanetRad, double LocalLatScale, short UseDefaultUnits);
		void ScalePointLon(double PlanetRad, double LocalLonScale, short UseDefaultUnits);
		void ScalePointLonLat(double PlanetRad, double LocalLonScale, double LocalLatScale, short UseDefaultUnits);
		void ScalePointElev(double ElevScale);
		void RotatePointDegXY(double PlanetRad, double RotateDeg, short UseDefaultUnits);
		void DeleteSelectedPoints(void);

		// vector support
		void ComputeReferenceCoords(void);
		void ConvertPointToXYZ(double PlanetRad, double &X, double &Y, float &Z, short Relative);
		void ConvertPointFromXYZ(double PlanetRad, double &X, double &Y, float &Z, short Relative);
		void ConvertDegToDist(double PlanetRad, double &Y);
		void ConvertDistToDeg(double PlanetRad, double &Y);
		void ConvertDegToDist(double PlanetRad, double Lat, double &X);
		void ConvertDistToDeg(double PlanetRad, double Lat, double &X);
		void ConvertDistToDegNoOptions(double PlanetRad, double &Y);
		void ConvertDistToDegNoOptions(double PlanetRad, double Lat, double &X);
		void ConvertElev(float &Z);
		void UnConvertElev(float &Z);

		// DEM functions
		double ElevationPoint(double Lat, double Lon);
		double ElevationPointNULLReject(double Lat, double Lon, int &Reject);
		double RelativeElevationPoint(double Lat, double Lon);
		double SQElevationPointNULLReject(double Lat, double Lon, int &Reject, CoordSys *TestSys);
		int VertexDataPoint(RenderData *Rend, VertexData *Vert, PolygonData *Poly, unsigned long Flags);

		// Data functions
		void DrillDownPoint(double Lat, double Lon);

		InterCommon(Database *DBSource, Project *ProjSource);
		~InterCommon();
		void HandleNotifyEvent(void);

		virtual void SetParam(int Notify, ...);
		virtual void GetParam(void *Value, ...);
		ULONG Save(FILE *ffile, struct ChunkIODetail *Detail = NULL);
	}; // InterCommon 

#endif // WCS_INTERACTIVE_H
