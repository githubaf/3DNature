// UsefulGeo.h
// Geographic and lat/lon code from Useful.h
// Built from Useful.h on 060403 by CXH

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_USEFULGEO_H
#define WCS_USEFULGEO_H

#include "UsefulMath.h"

// moved here from Defines.h on 2/20/06 by CXH since it makes more sense here
// We now supply LatScale() to calculate your
// Km-per-one-degree-Latitude values for any radius of sphere/planet.
#define EARTHLATSCALE 111.049771	/* km, miles = 69.003 we're goin' metric! */
#define EARTHLATSCALE_METERS 111049.771	/* meters of course */
#define EARTHRAD 6362.683195	/* km, miles = 3953.580673 we done gone metric! */


void DecimalToDMS(double GeoDecimal, double &Degrees, double &Minutes, double &Seconds);
double DMSToDecimal(double Degrees, double Minutes, double Seconds);
double DecimalToPackedDMS(double GeoDecimal);
double DegMinSecToDegrees(char *str);

inline double LatScale(double SphereRad) {return(Pi * (SphereRad + SphereRad) * (1.0 / 360.0));};
double LonScale(double SphereRad, double Latitude);
double FindDistance(double Lat1, double Lon1, double Lat2, double Lon2, double PlanetRad);
double FindDistanceCartesian(double Y1, double X1, double Y2, double X2);
void MakeTempCart(double Lat, double Lon, double SphereRad,	double *X, double *Y, double *Z);
double SolveDistCart(double XJ, double YJ, double ZJ, double XK, double YK, double ZK);
double SolveArcAng(double CartDist, double SphereRad);

double ConvertMetersToDeg(double MeterVal, double Latitude, double GlobeRad);

// Adjusts for the effect of Exag/Datum
inline double CalcExag(double Elev, double Datum, double Exag)
{
return(Datum + (Elev - Datum) * Exag);
} // CalcExag

inline double UnCalcExag(double Elev, double Datum, double Exag)
{
if(Exag == 0.0) return(Elev);
return(Datum + (Elev - Datum) / Exag);
} // UnCalcExag

#endif // WCS_USEFULGEO_H
