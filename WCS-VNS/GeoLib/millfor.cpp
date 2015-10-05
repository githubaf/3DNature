/*******************************************************************************
NAME                    MILLER CYLINDRICAL 

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the Miller Cylindrical projection.  The
                longitude and latitude must be in radians.  The Easting
                and Northing values will be returned in meters.

PROGRAMMER              DATE            
----------              ----           
T. Mittan               March, 1993

This function was adapted from the Lambert Azimuthal Equal Area projection
code (FORTRAN) in the General Cartographic Transformation Package software
which is available from the U.S. Geological Survey National Mapping Division.
 
ALGORITHM REFERENCES

1.  "New Equal-Area Map Projections for Noncircular Regions", John P. Snyder,
    The American Cartographer, Vol 15, No. 4, October 1988, pp. 341-355.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

3.  "Software Documentation for GCTP General Cartographic Transformation
    Package", U.S. Geological Survey National Mapping Division, May 1982.
*******************************************************************************/
#include "cproj.h"

// Initialize the Miller Cylindrical projection
long Projectoid::millforint(
double r,                       // (I) Radius of the earth (sphere)
double center_long,             // (I) Center longitude
double false_east,              // x offset in meters
double false_north)             // y offset in meters
{

// Place parameters in static storage for common use
R = r;
lon_center = center_long;
false_easting = false_east;
false_northing = false_north;

// Report parameters to the user
ptitle("MILLER CYLINDRICAL");
radius(r);
cenlon(center_long);
offsetp(false_easting, false_northing);
ForwardOK[WCS_PROJECTIONCODE_MILLER] = 1;
ForwardTransform = &Projectoid::millfor;

return(OK);

}

/*===========================================================================*/

// Miller Cylindrical forward equations--mapping lat,long to x,y
long Projectoid::millfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double dlon;

// Forward equations
dlon = adjust_lon(lon - lon_center);
*x = false_easting + R * dlon;
*y = false_northing + R * log(tan((PI * (1.0 / 4.0)) + (lat * (1.0 / 2.5)))) * 1.25;	// <<<F2 OPT>>>

return(OK);

}
