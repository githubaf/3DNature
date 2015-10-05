/*******************************************************************************
NAME                    MILLER CYLINDRICAL 

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the Miller Cylindrical projection.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER              DATE            
----------              ----           
T. Mittan               March, 1993

This function was adapted from the Miller Cylindrical projection code
(FORTRAN) in the General Cartographic Transformation Package software
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
long Projectoid::millinvint(
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
InverseOK[WCS_PROJECTIONCODE_MILLER] = 1;
InverseTransform = &Projectoid::millinv;

return(OK);

}

/*===========================================================================*/

/* Miller Cylindrical inverse equations--mapping x,y to lat/long
  ------------------------------------------------------------*/
long Projectoid::millinv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{

// Inverse  equations
x -= false_easting;
y -= false_northing;

*lon = adjust_lon(lon_center + x / R);
*lat = 2.5 * (atan(exp(y / (R * 1.25))) - PI * (1.0 / 4.0));

return(OK);

}
