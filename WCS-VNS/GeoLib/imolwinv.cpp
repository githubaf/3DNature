/*******************************************************************************
NAME                        INTERRUPTED MOLLWEIDE

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the Interrupted Mollweide projection.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER              DATE            REASON
----------              ----            ------
D. Steinwand, EROS      June, 1991      Initial Development
S. Nelson, EDC          June, 1993      Changed precision for values to
                                        determine regions and if coordinates
                                        are in the break, and for the values
                                        in the conversion algorithm.

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
long Projectoid::imolwinvint(
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
InverseOK[WCS_PROJECTIONCODE_IMOLL] = 1;
InverseTransform = &Projectoid::imolwinv;

return(OK);

}

/*===========================================================================*/

long Projectoid::imolwinv(
double x,               // (I) X projection coordinate
double y,               // (I) Y projection coordinate
double *lon,            // (O) Longitude
double *lat)            // (O) Latitude
{
double theta;
long region;

// Inverse equations
if (y >= 0.0)
	{
	if (x <= R * -1.41421356248) region = 0;
	else if (x <= R * 0.942809042) region = 1;
	else region = 2;
	}
else
	{
	if (x <= R * -0.942809042) region = 3;
	else if (x <= R * 1.41421356248) region = 4;
	else region = 5;
	}
x = x - feast6[region];

theta = asin(y / (1.4142135623731 * R));
*lon = adjust_lon(lon_center6[region] + (x / (0.900316316158 * R * cos(theta))));
*lat = asin((2.0 * theta + sin(2.0 * theta)) * INV_PI);	// <<<F2 OPT>>>

// Are we in a interrupted area?  If so, return status code of IN_BREAK.
if (region == 0 && (*lon < 0.34906585 || *lon > 1.91986217719))
	return(IN_BREAK);
if (region == 1 && ((*lon < 1.91986217719 && *lon > 0.34906585) || (*lon > -1.74532925199 && *lon < 0.34906585)))
	return(IN_BREAK);
if (region == 2 && (*lon < -1.745329252 || *lon > 0.34906585))
	return(IN_BREAK);
if (region == 3 && (*lon < 0.34906585 || *lon > 2.44346095279))
	return(IN_BREAK);
if (region == 4 && ((*lon < 2.44346095279 && *lon > 0.34906585) || (*lon > -1.2217304764 && *lon < 0.34906585)))
	return(IN_BREAK);
if (region == 5 && (*lon < -1.2217304764 || *lon> 0.34906585))
	return(IN_BREAK);

return(OK);

}
