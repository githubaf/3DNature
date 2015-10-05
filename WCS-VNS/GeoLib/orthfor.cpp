/*******************************************************************************
NAME                             ORTHOGRAPHIC 

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the Orthographic projection.  The
                longitude and latitude must be in radians.  The Easting
                and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
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

// Initialize the Orthographic projection
long Projectoid::orthforint(
double r_maj,                   // major axis
double center_lon,              // center longitude
double center_lat,              // center latitude
double false_east,              // x offset in meters
double false_north)             // y offset in meters
{

// Place parameters in static storage for common use
r_major = r_maj;
lon_center = center_lon;
lat_origin = center_lat;
false_northing = false_north;
false_easting = false_east;

sincos(center_lat, &sin_p14, &cos_p14);

// Report parameters to the user
ptitle("ORTHOGRAPHIC"); 
radius(r_major);
cenlonmer(lon_center);
origin(lat_origin);
offsetp(false_easting, false_northing);
ForwardOK[WCS_PROJECTIONCODE_ORTHO] = 1;
ForwardTransform = &Projectoid::orthfor;

return(OK);

}

/*===========================================================================*/

// Orthographic forward equations--mapping lat,long to x,y
long Projectoid::orthfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double sinphi, cosphi;  // sin and cos value
double dlon;            // delta longitude value
double coslon;          // cos of longitude
double ksp;             // scale factor
double g;               

// Forward equations
dlon = adjust_lon(lon - lon_center);
sincos(lat, &sinphi, &cosphi);
coslon = cos(dlon);
g = sin_p14 * sinphi + cos_p14 * cosphi * coslon;
ksp = 1.0;
if ((g > 0) || (fabs(g) <= EPSLN))
	{
	*x = false_easting + r_major * ksp * cosphi * sin(dlon);
	*y = false_northing + r_major * ksp * (cos_p14 * sinphi - sin_p14 * cosphi * coslon);
	}
else
	{
	p_error("Point can not be projected", "orth-for");
	return(143);
	}

return(OK);

}
