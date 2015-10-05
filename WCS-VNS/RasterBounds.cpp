// RasterBounds.cpp
// Source file for RasterBounds class
// Class for keeping track of an image or DEM's geographic boundaries for 
// use in writing out world files, prj files and ECW headers among other things.
// Created from scratch 06/12/03 by Gary R Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RasterBounds.h"
#include "EffectsLib.h"
#include "Render.h"

RasterBounds::RasterBounds()
: GeoRegister(NULL)
{

SetDefaults();

} // RasterBounds::RasterBounds

/*===========================================================================*/

void RasterBounds::SetDefaults(void)
{

ULcorner.x = ULcenter.x = -360.0;
ULcorner.y = ULcenter.y = 90.0;
LRcorner.x = LRcenter.x = 360.0;
LRcorner.y = LRcenter.y = -90.0;

CellSizeX = CellSizeY = 0.0;
CoordsValid = BoundsSet = 0;
IsGeographic = 1;
MyCoords = NULL;
NullReject = 0;
NullValue = -9999.0f;

// in WCS convention
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(90.0);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(-90.0);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(360.0);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-360.0);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetFlags(WCS_ANIMCRITTER_FLAG_NONODES | WCS_ANIMCRITTER_FLAG_SUPPRESSNOTIFY | WCS_ANIMCRITTER_FLAG_SUPPRESSACTIVATE);

} // RasterBounds::SetDefaults

/*===========================================================================*/

void RasterBounds::operator= (RasterBounds Source)
{

ULcorner.x = Source.ULcorner.x;
ULcorner.y = Source.ULcorner.y;
LRcorner.x = Source.LRcorner.x;
LRcorner.y = Source.LRcorner.y;
ULcenter.x = Source.ULcenter.x;
ULcenter.y = Source.ULcenter.y;
LRcenter.x = Source.LRcenter.x;
LRcenter.y = Source.LRcenter.y;
CellSizeX = Source.CellSizeX;
CellSizeY = Source.CellSizeY;
CoordsValid = Source.CoordsValid;
BoundsSet = Source.BoundsSet;
IsGeographic = Source.IsGeographic;
MyCoords = Source.MyCoords;

GeoRegister::Copy(this, &Source);

} // RasterBounds::operator=

/*===========================================================================*/

// geographic coords are either projected or specified in WCS convention of pos. west longitude
void RasterBounds::SetOutsideBounds(double North, double South, double West, double East)
{

AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(North);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(South);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(West);
AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(East);
BoundsSet = 1;

} // RasterBounds::SetOutsideBounds

/*===========================================================================*/

void RasterBounds::SetOutsideBounds(GeoRegister *Source)
{
double North, South, West, East;

North = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
South = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
West = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
East = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;

SetOutsideBounds(North, South, West, East);

} // RasterBounds::SetOutsideBounds

/*===========================================================================*/

// geographic coords are either projected or specified in WCS convention of pos. west longitude
void RasterBounds::SetOutsideBoundsFromCenters(double North, double South, double West, double East, long Rows, long Cols)
{
long Intervals;
double Increment;

if (Rows > 1 && Cols > 1)
	{
	Intervals = Rows - 1;
	Increment = .5 * (North - South) / Intervals;	// 1/2 the cell size used for incrementing bounds
	North += Increment;
	South -= Increment;
	Intervals = Cols - 1;
	Increment = .5 * (East - West) / Intervals;	// 1/2 the cell size used for incrementing bounds
	East += Increment;
	West -= Increment;
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(North);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(South);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(West);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(East);
	} // if
else
	{
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(North);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(North);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(West);
	AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(West);
	} // else

BoundsSet = 1;

} // RasterBounds::SetOutsideBoundsFromCenters

/*===========================================================================*/

void RasterBounds::SetOutsideBoundsFromCenters(GeoRegister *Source, long Rows, long Cols)
{
double North, South, West, East;

North = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
South = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;
West = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
East = Source->AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;

SetOutsideBoundsFromCenters(North, South, West, East, Rows, Cols);

} // RasterBounds::SetOutsideBoundsFromCenters

/*===========================================================================*/

// output is either projected or in standard GIS convention of east longitude positive
// NOTE "BACKWARDS" ARG ORDER !!!
int RasterBounds::DeriveCoords(long Rows, long Cols)
{
long Intervals;

if (MyCoords)
	{
	IsGeographic = MyCoords->GetGeographic();
	} // if
else
	IsGeographic = 1;

if (BoundsSet && Cols > 1 && Rows > 1)
	{
	Intervals = Rows; // - 1;
	CellSizeY = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue) / Intervals;

	// geographic coords are either projected or specified in WCS convention of pos. west longitude
	Intervals = Cols; // - 1;
	CellSizeX = (AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue - AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue) / Intervals;
	// reversed sign for output to standard GIS convention
	if (IsGeographic)
		CellSizeX = -CellSizeX;

	ULcorner.x = IsGeographic ? -AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue: AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].CurValue;
	ULcorner.y = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].CurValue;
	LRcorner.x = IsGeographic ? -AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue: AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].CurValue;
	LRcorner.y = AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].CurValue;

	ULcenter.x = ULcorner.x;
	ULcenter.x += .5 * CellSizeX;
	ULcenter.y = ULcorner.y;
	ULcenter.y -= .5 * CellSizeY;
	LRcenter.x = LRcorner.x;
	LRcenter.x -= .5 * CellSizeX;
	LRcenter.y = LRcorner.y;
	LRcenter.y += .5 * CellSizeY;

	CoordsValid = 1;
	} // if
else
	CoordsValid = 0;

return (CoordsValid);

} // RasterBounds::DeriveCoords

/*===========================================================================*/

int RasterBounds::DeriveTileCoords(long Rows, long Cols, 
	long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap)
{
double ULYpos, ULXpos, LRYpos, LRXpos;
long TileRows, TileCols, TileStartRow, TileStartCol;

TileRows = (Rows + (TilesY - 1) * Overlap) / TilesY;
TileCols = (Cols + (TilesX - 1) * Overlap) / TilesX;

// which row and column of resampled raster would this tile start on
TileStartRow = CurTileY * (TileRows - Overlap);
if (TileStartRow < 0)
	return (0);	// illegal, overlap larger than tile
TileStartCol = CurTileX * (TileCols - Overlap);
if (TileStartCol < 0)
	return (0);	// illegal, overlap larger than tile

ULYpos = (double)TileStartRow / (Rows - 1);
ULXpos = (double)TileStartCol / (Cols - 1);
LRYpos = (double)(TileStartRow + TileRows - 1) / (Rows - 1);
LRXpos = (double)(TileStartCol + TileCols - 1) / (Cols - 1);

if (DeriveCoords(Rows, Cols))
	{
	ULYpos = ULcorner.y + ULYpos * (LRcorner.y - ULcorner.y); 
	ULXpos = ULcorner.x + ULXpos * (LRcorner.x - ULcorner.x); 
	LRYpos = ULcorner.y + LRYpos * (LRcorner.y - ULcorner.y); 
	LRXpos = ULcorner.x + LRXpos * (LRcorner.x - ULcorner.x);
	ULcorner.y = ULYpos;
	ULcorner.x = ULXpos;
	LRcorner.y = LRYpos;
	LRcorner.x = LRXpos;

	ULcenter.x = ULcorner.x;
	ULcenter.x += .5 * CellSizeX;
	ULcenter.y = ULcorner.y;
	ULcenter.y -= .5 * CellSizeY;
	LRcenter.x = LRcorner.x;
	LRcenter.x -= .5 * CellSizeX;
	LRcenter.y = LRcorner.y;
	LRcenter.y += .5 * CellSizeY;
	} // if

return (CoordsValid);

} // RasterBounds::DeriveTileCoords

/*===========================================================================*/

// coords in north-south direction are returned as increasing to north.
// coords in west-east direction are returned as increasing to east.
int RasterBounds::FetchBoundsEdgesGIS(double &N, double &S, double &W, double &E)
{

if (CoordsValid)
	{
	N = ULcorner.y;
	S = LRcorner.y;
	W = ULcorner.x;
	E = LRcorner.x;
	return (1);
	} // if

return (0);

} // RasterBounds::FetchBoundsEdgesGIS

/*===========================================================================*/

// coords in north-south direction are returned as increasing to north.
// coords in west-east direction are returned as increasing to east.
int RasterBounds::FetchBoundsCentersGIS(double &N, double &S, double &W, double &E)
{

if (CoordsValid)
	{
	N = ULcenter.y;
	S = LRcenter.y;
	W = ULcenter.x;
	E = LRcenter.x;
	return (1);
	} // if

return (0);

} // RasterBounds::FetchBoundsCentersGIS

/*===========================================================================*/

// cell size in north-south direction is returned as increasing to north.
// cell size in west-east direction is returned as increasing to east.
int RasterBounds::FetchCellSizeGIS(double &CellNS, double &CellWE)
{

if (CoordsValid)
	{
	CellNS = CellSizeY;
	CellWE = CellSizeX;
	return (1);
	} // if

return (0);

} // RasterBounds::FetchCellSizeGIS

/*===========================================================================*/

// coords in north-south direction are returned as increasing to north.
// coords in west-east direction are returned as increasing to west.
int RasterBounds::FetchBoundsEdgesWCS(double &N, double &S, double &W, double &E)
{

if (FetchBoundsEdgesGIS(N, S, W, E))
	{
	if (IsGeographic)
		{
		W = -W;
		E = -E;
		} // if
	return (1);
	} // if

return (0);

} // RasterBounds::FetchBoundsEdgesWCS

/*===========================================================================*/

// coords in north-south direction are returned as increasing to north.
// coords in west-east direction are returned as increasing to west.
int RasterBounds::FetchBoundsCentersWCS(double &N, double &S, double &W, double &E)
{

if (FetchBoundsCentersGIS(N, S, W, E))
	{
	if (IsGeographic)
		{
		W = -W;
		E = -E;
		} // if
	return (1);
	} // if

return (0);

} // RasterBounds::FetchBoundsCentersWCS

/*===========================================================================*/

// cell size in north-south direction is returned as increasing to north.
// cell size in west-east direction is returned as increasing to west.
int RasterBounds::FetchCellSizeWCS(double &CellNS, double &CellWE)
{

if (FetchCellSizeGIS(CellNS, CellWE))
	{
	// WCS DEMs prefer to have projected W-E cell size negative and geographic positive
	// CellWE is always positive in the RasterBounds structure so needs negation for projected data
	// This correlates with the difinition of DEM header data in DEM version 1.02
	if (! IsGeographic)
		CellWE = -CellWE;
	return (1);
	} // if

return (0);

} // RasterBounds::FetchCellSizeWCS

/*===========================================================================*/

int RasterBounds::FetchRegionCenterGIS(double &NS, double &WE)
{
double N, S, W, E;

if (FetchBoundsCentersGIS(N, S, W, E))
	{
	NS = (N + S) * .5;
	WE = (W + E) * .5;
	} // if

return (0);

} // RasterBounds::FetchRegionCenterGIS

/*===========================================================================*/

int RasterBounds::FetchRegionCenterWCS(double &NS, double &WE)
{
double N, S, W, E;

if (FetchBoundsCentersWCS(N, S, W, E))
	{
	NS = (N + S) * .5;
	WE = (W + E) * .5;
	} // if

return (0);

} // RasterBounds::FetchRegionCenterWCS

/*===========================================================================*/

int RasterBounds::IsDefGeoPointBounded(double Lat, double Lon)
{
VertexDEM Vert;

Vert.xyz[0] = Vert.Lon = Lon;
Vert.xyz[1] = Vert.Lat = Lat;
if (MyCoords)
	{
	MyCoords->DefDegToProj(&Vert);
	} // if

if (IsGeographic)
	{
	// corners are stored in GIS convention of east positive
	if (-Vert.xyz[0] <= LRcorner.x && -Vert.xyz[0] >= ULcorner.x
		&& Vert.xyz[1] >= LRcorner.y && Vert.xyz[1] <= ULcorner.y)
		return (1);
	} // if
else
	{
	if (Vert.xyz[0] <= LRcorner.x && Vert.xyz[0] >= ULcorner.x
		&& Vert.xyz[1] >= LRcorner.y && Vert.xyz[1] <= ULcorner.y)
		return (1);
	} // else

return (0);

} // RasterBounds::IsDefGeoPointBounded

/*===========================================================================*/

int RasterBounds::TestBoundsOverlap(double NNorthing, double WEasting, double SNorthing, double EEasting)
{

if (SNorthing > ULcorner.y)
	return (0);
if (NNorthing < LRcorner.y)
	return (0);

// bounds must be presented in GIS convention
if (WEasting > LRcorner.x)
	return (0);
if (EEasting < ULcorner.x)
	return (0);

return (1);

} // RasterBounds::TestBoundsOverlap

/*===========================================================================*/

void RasterBounds::DefDegToRBounds(double Lat, double Lon, double &X, double &Y)
{
VertexDEM Vert;

Vert.xyz[0] = Vert.Lon = Lon;
Vert.xyz[1] = Vert.Lat = Lat;
if (MyCoords)
	{
	MyCoords->DefDegToProj(&Vert);
	} // if

if (IsGeographic)
	{
	// .xyz[0] is longitude in WCS convention of positive west
	X = -Vert.xyz[0];
	} // if
else
	{
	// .xyz[0] is metric in GIS convention of positive east
	X = Vert.xyz[0];
	} // else
Y = Vert.xyz[1];

} // RasterBounds::DefDegToRBounds

/*===========================================================================*/

void RasterBounds::RBoundsToDefDeg(double X, double Y, double &Lat, double &Lon)
{
VertexDEM Vert;

if (IsGeographic)
	{
	// X is longitude in GIS convention of positive east
	Vert.xyz[0] = Vert.Lon = -X;
	} // if
else
	{
	// X is metric in GIS convention of positive east
	Vert.xyz[0] = Vert.Lon = X;
	} // else
Vert.xyz[1] = Vert.Lat = Y;
if (MyCoords)
	{
	MyCoords->ProjToDefDeg(&Vert);
	} // if

Lon = Vert.Lon;
Lat = Vert.Lat;

} // RasterBounds::RBoundsToDefDeg
