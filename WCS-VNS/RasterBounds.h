// RasterBounds.h
// Header file for RasterBounds class
// Class for keeping track of an image or DEM's geographic boundaries for 
// use in writing out world files, prj files and ECW headers among other things.
// Created from scratch 06/12/03 by Gary R Huber
// Copyright 2003 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_RASTERBOUNDS_H
#define WCS_RASTERBOUNDS_H

#include "Types.h"
#include "GeoRegister.h"

class CoordSys;

class RasterBounds: public GeoRegister
	{
	public:
		RasterPointD ULcorner, LRcorner, ULcenter, LRcenter;
		double CellSizeX, CellSizeY;
		float NullValue;
		int CoordsValid, BoundsSet, IsGeographic, NullReject;
		CoordSys *MyCoords;

		RasterBounds();
		void SetDefaults(void);
		void SetCoords(CoordSys *NewCoords)	{MyCoords = NewCoords;};
		void SetOutsideBounds(double North, double South, double West, double East);
		void SetOutsideBounds(GeoRegister *Source);
		// DIRE WARNING- Rows & Cols are "backwards"
		void SetOutsideBoundsFromCenters(double North, double South, double West, double East, long Rows, long Cols);
		void SetOutsideBoundsFromCenters(GeoRegister *Source, long Rows, long Cols);
		int DeriveCoords(long Rows, long Cols);
		int DeriveTileCoords(long Rows, long Cols, 
			long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap);
		int FetchBoundsEdgesGIS(double &N, double &S, double &W, double &E);
		int FetchBoundsCentersGIS(double &N, double &S, double &W, double &E);
		int FetchCellSizeGIS(double &CellNS, double &CellWE);
		int FetchBoundsEdgesWCS(double &N, double &S, double &W, double &E);
		int FetchBoundsCentersWCS(double &N, double &S, double &W, double &E);
		int FetchCellSizeWCS(double &CellNS, double &CellWE);
		int FetchRegionCenterGIS(double &NS, double &WE);
		int FetchRegionCenterWCS(double &NS, double &WE);
		CoordSys *FetchCoordSys(void)	{return (MyCoords);};
		void operator= (RasterBounds Source);
		int IsDefGeoPointBounded(double Lat, double Lon);
		int TestBoundsOverlap(double NNorthing, double WEasting, double SNorthing, double EEasting);
		void DefDegToRBounds(double Lat, double Lon, double &X, double &Y);
		void RBoundsToDefDeg(double X, double Y, double &Lat, double &Lon);

	}; // class RasterBounds

#endif // WCS_RASTERBOUNDS_H
