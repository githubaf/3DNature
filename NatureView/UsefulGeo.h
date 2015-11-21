// UsefulGeo.h
// Geographic and lat/lon code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULGEO_H
#define WCS_USEFULGEO_H

#ifndef Pi
#include "Defines.h"
#endif // Pi

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
