/*******************************************************************************
NAME                            MERCATOR

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the Mercator projection.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      Nov, 1991
T. Mittan               Mar, 1993

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

// Initialize the Mercator projection
long Projectoid::merinvint(
double r_maj,                   // major axis
double r_min,                   // minor axis
double center_lon,              // center longitude
double center_lat,              // center latitude
double false_east,              // x offset in meters
double false_north)             // y offset in meters
{
double temp;                    // temporary variable

// Place parameters in static storage for common use
r_major = r_maj;
r_minor = r_min;
lon_center = center_lon;
lat_origin = center_lat;
false_northing = false_north;
false_easting = false_east;

temp = r_minor / r_major;
es = 1.0 - SQUARE(temp);
e = sqrt(es);
m1 = cos(center_lat) / (sqrt(1.0 - es * sin(center_lat) * sin(center_lat)));

// Report parameters to the user
ptitle("MERCATOR"); 
radius2(r_major, r_minor);
cenlonmer(lon_center);
origin(lat_origin);
offsetp(false_easting, false_northing);
InverseOK[WCS_PROJECTIONCODE_MERCAT] = 1;
InverseTransform = &Projectoid::merinv;

return(OK);

}

/*===========================================================================*/

/* Mercator inverse equations--mapping x,y to lat/long
  --------------------------------------------------*/
long Projectoid::merinv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{
double ts;              // small t value
long flag;              // error flag

// Inverse equations
flag = 0;
x -= false_easting;
y -= false_northing;
const double tmp = 1.0 / (r_major * m1);	// <<<F2 OPT>>>
ts = exp(-y * tmp);	// <<<F2 OPT>>>
*lat = phi2z(e, ts, &flag);
if (flag != 0)
	return(flag);
*lon = adjust_lon(lon_center + x * tmp);	// <<<F2 OPT>>>

return(OK);

}
