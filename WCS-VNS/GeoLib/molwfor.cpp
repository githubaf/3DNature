/*******************************************************************************
NAME                            MOLLWEIDE

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the MOllweide projection.  The
                longitude and latitude must be in radians.  The Easting
                and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
D. Steinwand, EROS      May, 1991;  Updated Sept, 1992; Updated Feb, 1993
S. Nelson, EDC          Jun, 2993;      Made corrections in precision and
                                        number of iterations.

ALGORITHM REFERENCES

1.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.
*******************************************************************************/
#include "cproj.h"

// Initialize the Mollweide projection
long Projectoid::molwforint(
double r,                       // (I) Radius of the earth (sphere)
double center_long,             // (I) Center longitude
double false_east,              // x offset in meters
double false_north)             // y offset in meters
{

// Place parameters in static storage for common use
false_easting = false_east;
false_northing = false_north;
R = r;
lon_center = center_long;

// Report parameters to the user
ptitle("MOLLWEIDE"); 
radius(r);
cenlon(center_long);
offsetp(false_easting, false_northing);
ForwardOK[WCS_PROJECTIONCODE_MOLL] = 1;
ForwardTransform = &Projectoid::molwfor;

return(OK);

}

/*===========================================================================*/

// Mollweide forward equations--mapping lat,long to x,y
long Projectoid::molwfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double delta_lon;       // Delta longitude (Given longitude - center
double theta;
double delta_theta;
double con;
double f2_cos, f2_sin;	// F2_MOD: 071702
unsigned long i;

// Forward equations
delta_lon = adjust_lon(lon - lon_center);
theta = lat;
con = PI * sin(lat);

// Iterate using the Newton-Raphson method to find theta
//for (i=0;;i++)
for (i = 50; ; --i)	// F2_MOD: 071702 - faster loops
	{
	delta_theta = -(theta + sin(theta) - con)/ (1.0 + cos(theta));
	theta += delta_theta;
	if (fabs(delta_theta) < EPSLN) break;
	//if (i >= 50) 
	if (i == 0)	// F2_MOD: 071702 - faster loops
		{
		p_error("Iteration failed to converge", "Mollweide-forward");
		return(241);
		}
	}
theta *= 0.5;	// F2 OPT;

// If the latitude is 90 deg, force the x coordinate to be "0 + false easting"
// this is done here because of precision problems with "cos(theta)"
if (HALF_PI  - fabs(lat) < EPSLN)	// <<<F2 OPT>>>
	delta_lon = 0;
/***
*x = 0.900316316158 * R * delta_lon * cos(theta) + false_easting;
*y = 1.4142135623731 * R * sin(theta) + false_northing;
***/
// F2_MOD: 071702 - compute atomic sine/cosine for theta
sincos(theta, &f2_sin, &f2_cos);
*x = 0.900316316158 * R * delta_lon * f2_cos + false_easting;
*y = 1.4142135623731 * R * f2_sin + false_northing;

return(OK);

}
