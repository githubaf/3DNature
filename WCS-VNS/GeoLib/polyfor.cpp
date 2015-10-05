/*******************************************************************************
NAME                             POLYCONIC 

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the Polyconic projection.  The
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

// Initialize the POLYCONIC projection
long Projectoid::polyforint(
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
e0 = e0fn(es);
e1 = e1fn(es);
e2 = e2fn(es);
e3 = e3fn(es);
ml0 = mlfn(e0, e1, e2, e3, lat_origin);

// Report parameters to the user
ptitle("POLYCONIC"); 
radius2(r_major, r_minor);
cenlonmer(lon_center);
origin(lat_origin);
offsetp(false_easting, false_northing);
ForwardOK[WCS_PROJECTIONCODE_POLYC] = 1;
ForwardTransform = &Projectoid::polyfor;

return(OK);

}

/*===========================================================================*/

// Polyconic forward equations--mapping lat,long to x,y
long Projectoid::polyfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double sinphi, cosphi;  // sin and cos value
double con, ml;         // cone constant, small m
double ms;              // small m
double f2_cos, f2_sin;	// F2_MOD: 071702 - change to atomic sine/cosine for cone constant also

// Forward equations
con = adjust_lon(lon - lon_center);
if (fabs(lat) <= .0000001)
	{
	*x = false_easting + r_major * con;
	*y = false_northing - r_major * ml0;
	}
else
	{
	sincos(lat, &sinphi, &cosphi);
	ml = mlfn(e0, e1, e2, e3, lat);
	ms = msfnz(e, sinphi, cosphi);
	con *= sinphi;
	/***
	*x = false_easting + r_major * ms * sin(con) / sinphi;
	*y = false_northing + r_major * (ml - ml0 + ms * (1.0 - cos(con)) / sinphi);
	***/
	// F2_MOD: 071702
	sincos(con, &f2_sin, &f2_cos);
	*x = false_easting + r_major * ms * f2_sin / sinphi;
	*y = false_northing + r_major * (ml - ml0 + ms * (1.0 - f2_cos) / sinphi);
	}

return(OK);

}
