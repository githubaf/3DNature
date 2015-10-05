
/*******************************************************************************
NAME                            NEW ZEALAND MAP GRID

PURPOSE:		Transforms input longitude and latitude to Easting and
				Northing for the New Zealand Map Grid projection.  The
				longitude and latitude must be in radians.  The Easting
				and Northing values will be returned in meters.

PROGRAMMER		DATE		REASON
----------		----		------
F. Weed II		10/16/01	because... adapted come that came from http://www.linz.govt.nz/services/surveysystem/geodetic/index.html

ALGORITHM REFERENCES

OSG Technical Report 4.1 (Office of the Surveyor-General)

*******************************************************************************/
#include "cproj.h"
#include "nzmg.h"

// Initialize the Lambert Conformal conic projection
long Projectoid::nzmgforint(void)
{

r_major = 6378388.0;
center_lat = -41.0;
center_lon = 173.0;
false_northing = 6023150.0;
false_easting = 2510000.0;


// Report parameters to the user
ptitle("NEW ZEALAND MAP GRID");
ForwardOK[WCS_PROJECTIONCODE_NZMG] = 1;
ForwardTransform = &Projectoid::nzmgfor;

return(OK);

} // Projectoid::nzmgforint

/*===========================================================================*/

// Lambert Conformal conic forward equations--mapping lat,long to x,y
long Projectoid::nzmgfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double sum;
short i;
geolib_complex z0, z1;

lat = (lat * R2D - center_lat) * 3600.0e-5;
sum = nzmg_cfi[9];
for (i = 9; i--; )
	sum = sum * lat + nzmg_cfi[i];
sum *= lat;

z1.real = sum; z1.imag = lon - center_lon / R2D;
z0.real = nzmg_cfb1[5].real; z0.imag = nzmg_cfb1[5].imag;
for (i = 5; i--; )
	cadd(&z0, cmult(&z0, &z0, &z1), nzmg_cfb1 + i);
cmult(&z0, &z0, &z1);

*y = false_northing + z0.real * r_major;
*x = false_easting + z0.imag * r_major;

return(OK);

} // Projectoid::nzmgfor
