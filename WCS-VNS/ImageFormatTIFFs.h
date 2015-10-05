// ImageFormatTIFFs.h
// Created 05/09/06 by FW2
// Copyright 3D Nature, LLC 2006

#ifndef WCS_IMAGEFORMATTIFFS_H
#define WCS_IMAGEFORMATTIFFS_H

#ifdef WCS_BUILD_TIFF_SUPPORT
extern "C" {
#include "tiffio.h"
} // extern C
#ifdef WCS_BUILD_GEOTIFF_SUPPORT
#include "geotiff.h"
#include "xtiffio.h"
#include "geo_normalize.h"
#include "geovalues.h"

struct ArcWorldInfo
	{
	double cellSizeX;
	double rowRot;
	double colRot;
	double cellSizeY;
	double originX;
	double originY;
	}; // ArcWorldInfo

enum
	{
	GeoLib_Latstdparallel = 1,
	GeoLib_Lat1ststdparallel,
	GeoLib_Lat2ndstdparallel,
	GeoLib_Loncentralmeridian,
	GeoLib_Latprojectionorigin,
	GeoLib_Falseeasting,
	GeoLib_Falsenorthing,
	GeoLib_Lattruescale,
	GeoLib_Lonbelowpole,
	GeoLib_Scalefactor,
	GeoLib_Loncenterprojection,
	GeoLib_Latcenterprojection,
	GeoLib_Heightperspectivepoint,
	GeoLib_Lon1stpoint,
	GeoLib_Lon2ndpoint,
	GeoLib_Lat1stpoint,
	GeoLib_Lat2ndpoint,
	GeoLib_Azimuthangle,
	GeoLib_Lonazimuthpoint,
	GeoLib_Inclinationoforbit,
	GeoLib_Lonascendingorbit,
	GeoLib_Periodofrevolution,
	GeoLib_Landsatnorhernratio,
	GeoLib_Endofpathflag,
	GeoLib_Landsatsatellitenumber,
	GeoLib_Landsatpathnumber,
	GeoLib_Shapeparameterm,
	GeoLib_Shapeparametern,
	GeoLib_Rotationangle,
	GeoLib_PARAMMAP_MAX // no such value used
	}; // GeoLib Param Map enums

enum
	{
	GTIF_GeographicTypeGeoKey,
	GTIF_GeogGeodeticDatumGeoKey,
	GTIF_GeogPrimeMeridianGeoKey,
	GTIF_GeogLinearUnitsGeoKey,
	GTIF_GeogLinearUnitSizeGeoKey,
	GTIF_GeogAngularUnitsGeoKey,
	GTIF_GeogAngularUnitSizeGeoKey,
	GTIF_GeogEllipsoidGeoKey,
	GTIF_GeogSemiMajorAxisGeoKey,
	GTIF_GeogSemiMinorAxisGeoKey,
	GTIF_GeogInvFlatteningGeoKey,
	GTIF_GeogAzimuthUnitsGeoKey,
	GTIF_GeogPrimeMeridianLongGeoKey,
	GTIF_GeogKeysMax // no real key of this value
	}; // GeoTIFF Geographic keys

// 	GTIF_GeogCitationGeoKey

enum
	{
	GTIF_ProjectedCSTypeGeoKey,
	GTIF_ProjectionGeoKey,
	GTIF_ProjCoordTransGeoKey,
	GTIF_ProjLinearUnitsGeoKey,
	GTIF_ProjLinearUnitSizeGeoKey,
	GTIF_ProjStdParallel1GeoKey,
	GTIF_ProjStdParallel2GeoKey,
	GTIF_ProjNatOriginLongGeoKey,
	GTIF_ProjNatOriginLatGeoKey,
	GTIF_ProjFalseEastingGeoKey,
	GTIF_ProjFalseNorthingGeoKey,
	GTIF_ProjFalseOriginLongGeoKey,
	GTIF_ProjFalseOriginLatGeoKey,
	GTIF_ProjFalseOriginEastingGeoKey,
	GTIF_ProjFalseOriginNorthingGeoKey,
	GTIF_ProjCenterLongGeoKey,
	GTIF_ProjCenterLatGeoKey,
	GTIF_ProjCenterEastingGeoKey,
	GTIF_ProjCenterNorthingGeoKey,
	GTIF_ProjScaleAtNatOriginGeoKey,
	GTIF_ProjScaleAtCenterGeoKey,
	GTIF_ProjAzimuthAngleGeoKey,
	GTIF_ProjStraightVertPoleLongGeoKey,
	GTIF_ProjRectifiedGridAngleGeoKey,
	GTIF_ProjKeysMax // no real key of this value
	}; // GeoTIFF Projection keys 

//	GTIF_PCSCitationGeoKey

enum
	{
	GTIF_VerticalCSTypeGeoKey,
	GTIF_VerticalDatumGeoKey,
	GTIF_VerticalUnitsGeoKey,
	GTIF_VertKeysMax // no real key of this value
	}; // GeoTIFF Vertical Datum keys

// 	GTIF_VerticalCitationGeoKey

int IsValidGeoTIFF(GTIF *geotiff);
GeoRefShell* ReadTIFFGeoref(ArcWorldInfo &awi, GeoRefShell *loaderGeoRefShell, GTIF *GTI, const long cols, const long rows, const int prjGood, const int tfwValid, const int gtifValid, bool askUnits = false);
int ReadWorldFile(ArcWorldInfo &awi, const char *filename);

#endif // WCS_BUILD_GEOTIFF_SUPPORT

#endif // WCS_BUILD_TIFF_SUPPORT

#endif // WCS_IMAGEFORMATTIFFS_H
