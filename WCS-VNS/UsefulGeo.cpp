// UsefulGeo.cpp
// Geographic and lat/lon code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulGeo.h"
#include "UsefulMath.h"

//#if defined WCS_BUILD_FRANK || defined WCS_BUILD_GARY
//extern unsigned __int64 DevCounter[50];
//#endif // FRANK or GARY

void sincos(double val, double *sin_val, double *cos_val);

void DecimalToDMS(double GeoDecimal, double &Degrees, double &Minutes, double &Seconds)
{
double CalcTemp;

Degrees  = quickftol(GeoDecimal);
CalcTemp = fabs(((GeoDecimal - Degrees) * 60));
Minutes  = fabs((double)(quickftol(CalcTemp)));
Seconds  = fabs(((CalcTemp - Minutes) * 60));

} // DecimalToDMS

/*===========================================================================*/

double DMSToDecimal(double Degrees, double Minutes, double Seconds)
{

if (Degrees >= 0.0)
	return(Degrees + fabs(Minutes / 60) + fabs(Seconds / 3600));
else
	return(Degrees - fabs(Minutes / 60) - fabs(Seconds / 3600));

} // DMSToDecimal

/*===========================================================================*/

double DecimalToPackedDMS(double GeoDecimal)
{
double Sign, CalcTemp, Degrees, Minutes, Seconds;

Sign = GeoDecimal >= 0.0 ? 1.0: -1.0;
GeoDecimal = fabs(GeoDecimal);
Degrees  = quickftol(GeoDecimal);
CalcTemp = fabs(((GeoDecimal - Degrees) * 60));
Minutes  = fabs((double)(quickftol(CalcTemp)));
Seconds  = fabs(((CalcTemp - Minutes) * 60));

CalcTemp = Degrees * 1000000.0 + Minutes * 1000.0 + Seconds;
return (CalcTemp * Sign);

} // DecimalToPackedDMS

/*===========================================================================*/

// For converting a coordinate string of the form dddmmss to decimal degrees 

double DegMinSecToDegrees(char *str)
{
long Deg, Min, DegMinSec;

DegMinSec = atoi(str);

Deg = DegMinSec / 10000;
DegMinSec -= Deg * 10000;
Min = DegMinSec / 100;
DegMinSec -= Min * 100;

return ((double)Deg + (double)Min / 60.0 + (double)DegMinSec / 3600.0);
 
} // DegMinSecToDegrees

/*===========================================================================*/

double LonScale(double SphereRad, double Latitude)
{

return((Pi * (SphereRad + SphereRad) * (1.0 / 360.0)) * cos(Latitude * PiOver180));

} // LonScale

/*===========================================================================*/

// returns Km
double FindDistance(double Lat1, double Lon1, double Lat2, double Lon2, double PlanetRad)
{
double XOne, YOne, ZOne, XTwo, YTwo, ZTwo;

MakeTempCart(Lat1, Lon1, PlanetRad, &XOne, &YOne, &ZOne);
MakeTempCart(Lat2, Lon2, PlanetRad, &XTwo, &YTwo, &ZTwo);

return (LatScale(PlanetRad) * PiUnder180 * 2.0 * SolveArcAng(
		SolveDistCart(XOne, YOne, ZOne, XTwo, YTwo, ZTwo), PlanetRad));

} // FindDistance

/*===========================================================================*/

// returns whatever units the original data is in. Input data is in cartesian form
double FindDistanceCartesian(double Y1, double X1, double Y2, double X2)
{
double L1, L2;

L1 = Y2 - Y1;
L2 = X2 - X1;

return (sqrt(L1 * L1 + L2 * L2));

} // FindDistanceCartesian

/*===========================================================================*/

void MakeTempCart(double Lat, double Lon, double SphereRad, double *X, double *Y, double *Z)
{
double sLat, cLat, sLon, cLon;
double TempCos;

// this could be optimized to use sincos if it were performance-critical
//TempCos = cos(Lat * PiOver180) * SphereRad;
//*X = sin(Lon * PiOver180) * TempCos;
//*Y = sin(Lat * PiOver180) * SphereRad;
//*Z = cos(Lon * PiOver180) * TempCos;

// FPW2: Changed to sincos Nov 19, 2007 since it can make a BIG difference for large vector databases
sincos(Lat * PiOver180, &sLat, &cLat);
sincos(Lon * PiOver180, &sLon, &cLon);
TempCos = cLat * SphereRad;
*X = sLon * TempCos;
*Y = sLat * SphereRad;
*Z = cLon * TempCos;

} // MakeTempCart

/*===========================================================================*/

double SolveDistCart(double XJ, double YJ, double ZJ, double XK, double YK, double ZK)
{

return(sqrt(((XK-XJ)*(XK-XJ)) + ((YK-YJ)*(YK-YJ)) + ((ZK-ZJ)*(ZK-ZJ))));

} // SolveDistCart

/*===========================================================================*/

double SolveArcAng(double CartDist, double SphereRad)
{

return(asin((CartDist * 0.5) / SphereRad));

} // SolveArcArg

/*===========================================================================*/

double ConvertMetersToDeg(double MeterVal, double Latitude, double GlobeRad)
{
double RadAtLat;

Latitude = fabs(Latitude);
if(Latitude > 90.0) Latitude = 90.0;

if(Latitude == 0.0) RadAtLat = GlobeRad * (1.0 / 1000.0);
else RadAtLat = (cos(Latitude * Pi * (1.0 / 180.0)) * GlobeRad * (1.0 / 1000.0));

return(MeterVal * (360 / (2000 * Pi * RadAtLat)));

} // ConvertMetersToDeg
