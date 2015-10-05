// GeoRegister.h
// Header file for GeoRegister.cpp
// Snagged from EffectsLib.h on 10/25/99 by Gary R. Huber
// Copyright 1999 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_GEOREGISTER_H
#define WCS_GEOREGISTER_H

#include "GraphData.h"

class CoordSys;
class DEMBounds;
class Database;
class Project;

#define WCS_SUBCLASS_GEOREGISTER  140

enum
	{
	WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH,
	WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH,
	WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST,
	WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST,
	WCS_EFFECTS_GEOREGISTER_NUMANIMPAR
	}; // animated GeoRegister params

class GeoRegister : public RasterAnimHost
	{
	public:
		AnimDoubleTime AnimPar[WCS_EFFECTS_GEOREGISTER_NUMANIMPAR];

		GeoRegister(RasterAnimHost *RAHost);
		void Copy(GeoRegister *CopyTo, GeoRegister *CopyFrom);
		unsigned long Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

		long GetNumAnimParams(void) {return (WCS_EFFECTS_GEOREGISTER_NUMANIMPAR);};
		AnimDoubleTime *GetAnimPtr(long AnimNum) {return (AnimNum < WCS_EFFECTS_GEOREGISTER_NUMANIMPAR ? &AnimPar[AnimNum]: NULL);};
		int SetToTime(double Time);
		unsigned long GetRAFlags(unsigned long Mask = ~0);
		int GetRAHostAnimated(void);
		short AnimateShadows(void);
		long GetKeyFrameRange(double &FirstKey, double &LastKey);
		char GetRAHostDropOK(long DropType);
		int ProcessRAHostDragDrop(RasterAnimHostProperties *DropSource);
		long GetRAHostTypeNumber(void)					{return (WCS_RAHOST_OBJTYPE_GEOREGISTER);};
		int SnapToDEMBounds(CoordSys *MyCoords);
		int SnapToDBObjs(CoordSys *MyCoords);
		void ScaleToDEMBounds(DEMBounds *OldBounds, DEMBounds *CurBounds);
		int SnapToDatabaseBounds(Database *SnapBase);
		int SnapToBounds(Database *SnapBase, Project *CurProj, CoordSys *MyCoords, int DEMOnly);
		void ValidateBoundsOrder(CoordSys *MyCoords);
		int TestBoundsOrder(void);	// 1 = bounds OK, 0 = bounds reversed
		int GetCenterLatLon(CoordSys *MyCoords, double &CenterLat, double &CenterLon);
		int GetMetricWidthHeight(CoordSys *MyCoords, double PlanetRad, double &MetricWidth, double &MetricHeight);
		int SetBoundsFromMetricWidthHeight(CoordSys *MyCoords, double PlanetRad, double MetricWidth, double MetricHeight,
			int FixedNorth, int FixedSouth, int FixedEast, int FixedWest);
		int SetBoundsFromGeographicWidthHeight(double GeographicWidth, double GeographicHeight,
			int FixedNorth, int FixedSouth, int FixedEast, int FixedWest);
		void SetBounds(CoordSys *MyCoords, double LatRange[2], double LonRange[2]);
		void ShiftBounds(CoordSys *MyCoords, double LatShift[2], double LonShift[2]); // to conform with Lat/Lon order of SetBounds, above
		int GetPopClassFlags(RasterAnimHostProperties *Prop);
		int GetAffiliates(RasterAnimHost *ChildA, RasterAnimHost **ChildB, AnimCritter *&AnimAffil);
		void SetMetricType(CoordSys *MyCoords);
		void FindOutsideBoundsFromDefDeg(CoordSys *MyCoords, double &Nrth, double &Sth, double &Wst, double &Est);
		int GeographicPointContained(CoordSys *MyCoords, CoordSys *PointCoords, double PointLat, double PointLon);
		int TestForDefaultBounds(void);

		// inherited from RasterAnimHost
		virtual unsigned char GetNotifySubclass(void) {return((unsigned char)WCS_SUBCLASS_GEOREGISTER);};
		virtual char *GetRAHostTypeString(void) {return ("(Geographic Registration)");};
		virtual void GetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual int SetRAHostProperties(RasterAnimHostProperties *Prop);
		virtual RasterAnimHost *GetRAHostChild(RasterAnimHost *Current, long ChildTypeFilter);
		virtual char *GetCritterName(RasterAnimHost *Test);
		virtual RasterAnimHost *GetNextGroupSibling(RasterAnimHost *FindMyBrother);
		virtual int AddSRAHBasePopMenus(PopMenuAdder *PMA, unsigned long MenuClassFlags, RasterAnimHost *ChildA, RasterAnimHost **ChildB);
		virtual int HandleSRAHPopMenuSelection(void *Action, RasterAnimHost *ChildA, RasterAnimHost **ChildB);

	}; // class GeoRegister

#endif // WCS_GEOREGISTER_H
