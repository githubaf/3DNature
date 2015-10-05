/*******************************************************************************
NAME                            RD / Netherlands New

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the RD system.  The
				longitude and latitude must be in radians.  The Easting
				and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
F. Weed II              02/16/04 - coded from translation of equations sent to me

ALGORITHM REFERENCES

Schreutelkamp and Strang van Hees, Geodesia 2001-2

*******************************************************************************/
#include "cproj.h"
#include "rd.h"

// Initialize the Lambert Conformal conic projection
long Projectoid::rdforint(void)
{

false_easting = rd_fe;
false_northing = rd_fn;
lat_origin = rd_lat_origin;
lon_origin = rd_lon_origin;

// Report parameters to the user
ptitle("RD / NETHERLANDS NEW");
ForwardOK[WCS_PROJECTIONCODE_RD] = 1;
ForwardTransform = &Projectoid::rdfor;

return(OK);

} // Projectoid::rdforint

/*===========================================================================*/

// Lambert Conformal conic forward equations--mapping lat,long to x,y
long Projectoid::rdfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double sum, dlat, dlon, tmp1, tmp2;
unsigned long i;

dlat = 0.36 * ((lat * R2D) - lat_origin);
dlon = 0.36 * ((lon * R2D) - lon_origin);

sum = 0.0;
for (i = 0; i < 9; i++)
	{
	tmp1 = pow(dlat, (double)RD_R_P[i]);
	tmp2 = pow(dlon, (double)RD_R_Q[i]);
	sum += RD_Rpq[i] * tmp1 * tmp2;
	} // for

*x = false_easting + sum;

sum = 0.0;
for (i = 0; i < 10; i++)
	{
	tmp1 = pow(dlat, (double)RD_S_P[i]);
	tmp2 = pow(dlon, (double)RD_S_Q[i]);
	sum += RD_Spq[i] * tmp1 * tmp2;
	} // for

*y = false_northing + sum;

return(OK);

} // Projectoid::rdfor
