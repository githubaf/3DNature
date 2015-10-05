/*******************************************************************************
NAME                        INTERRUPTED MOLLWEIDE

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the Interrupted Mollweide projection.  The
                longitude and latitude must be in radians.  The Easting
                and Northing values will be returned in meters.

PROGRAMMER              DATE            REASON
----------              ----            ------
D. Steinwand, EROS      June, 1991      Initial development
T. Mittan, EDC          Feb, 1993       Adapted format to be used by the "C"
                                        version of GCTP.
S. Nelson, EDC          June, 1993      Changed precisian of radians in
                                        assigning the region, and in the
                                        conversion algorithm.
S. Nelson, EDC          Feb, 1994       changed perror to p_error.

ALGORITHM REFERENCES

1.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.

2.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.
*******************************************************************************/
#include "cproj.h"

// Initialize the Interrupted Mollweide projection
long Projectoid::imolwforint(
double r)                       // (I) Radius of the earth (sphere)
{
// Place parameters in static storage for common use
R = r;

// Initialize central meridians for each of the 6 regions
lon_center6[0] = 1.0471975512;           //   60.0 degrees
lon_center6[1] = -2.96705972839;         // -170.0 degrees
lon_center6[2] = -0.523598776;           //  -30.0 degrees
lon_center6[3] =  1.57079632679;         //   90.0 degrees
lon_center6[4] = -2.44346095279;         // -140.0 degrees
lon_center6[5] = -0.34906585;            //  -20.0 degrees

// Initialize false eastings for each of the 6 regions
feast6[0] = R * -2.19988776387;
feast6[1] = R * -0.15713484;
feast6[2] = R * 2.04275292359;
feast6[3] = R * -1.72848324304;
feast6[4] = R * 0.31426968;
feast6[5] = R * 2.19988776387;

// Report parameters to the user
ptitle("INTERRUPTED MOLLWEIDE EQUAL-AREA"); 
radius(r);
ForwardOK[WCS_PROJECTIONCODE_IMOLL] = 1;
ForwardTransform = &Projectoid::imolwfor;

return(OK);

}

/*===========================================================================*/

// Interrupted Mollweide forward equations--mapping lat,long to x,y
long Projectoid::imolwfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double delta_lon;       // Delta longitude (Given longitude - center)
double theta;
double delta_theta;
double con;
double f2_cos, f2_sin;	// F2_MOD: changed to atomic cosine/sine for theta
unsigned long i;
long region;

// Forward equations
// *** Note:  PI has been adjusted so that the correct region will be assigned when lon = 180 deg.
if (lat >= 0.0)
	{
	if (lon >= 0.34906585 && lon < 1.91986217719)
		region = 0;
	else if ((lon >= 1.919862177 && lon <= (PI + 1.0E-14)) || (lon >= (-PI - 1.0E-14) && lon < -1.745329252))
		region=1;
	else
		region = 2;
	}
else
	{
	if (lon >= 0.34906585 && lon < 2.44346095279)
		region = 3; 
	else if ((lon >= 2.44346095279 && lon <= (PI +1.0E-14)) || (lon >= (-PI - 1.0E-14) && lon<-1.2217304764))
		region=4; 
	else
		region = 5;
	}

delta_lon = adjust_lon(lon - lon_center6[region]);
theta = lat;
con = PI * sin(lat);

// Iterate using the Newton-Raphson method to find theta
//for (i=0;;i++)
for (i = 50; ; --i)	// F2_MOD: 071702 - faster loops
	{
	delta_theta = -(theta + sin(theta) - con) / (1.0 + cos(theta));
	theta += delta_theta;
	if (fabs(delta_theta) < EPSLN)
		break;
	//if (i >= 50)
	if (i == 50)	// F2_MOD: 071702 - faster loops
		p_error("Iteration failed to converge", "IntMoll-forward");
	}
theta *= (1.0 / 2.0);	// <<<F2 OPT>>>

// If the latitude is 90 deg, force the x coordinate to be "0 + false easting"
// this is done here because of percision problems with "cos(theta)"
if (PI / 2 - fabs(lat) < EPSLN)
	delta_lon = 0;
/***
*x = feast6[region] + 0.900316316158 * R * delta_lon * cos(theta);
*y = R * 1.4142135623731 * sin(theta);
***/
sincos(theta, &f2_sin, &f2_cos);
*x = feast6[region] + 0.900316316158 * R * delta_lon * f2_cos;
*y = R * 1.4142135623731 * f2_sin;

return(OK);

}
