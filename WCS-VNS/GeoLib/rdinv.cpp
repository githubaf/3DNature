/*******************************************************************************
NAME                            RD / Netherlands New

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the RD system.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
F. Weed II              02/16/04 - coded from translation of equations sent to me

ALGORITHM REFERENCES

Schreutelkamp and Strang van Hees, Geodesia 2001-2

*******************************************************************************/
#include "cproj.h"
#include "rd.h"

// Initialize the RD projection
long Projectoid::rdinvint(void)
{

false_easting = rd_fe;
false_northing = rd_fn;
lat_origin = rd_lat_origin;
lon_origin = rd_lon_origin;

// Report parameters to the user
ptitle("RD / NETHERLANDS NEW"); 
InverseOK[WCS_PROJECTIONCODE_RD] = 1;
InverseTransform = &Projectoid::rdinv;

return(OK);

} // Projectoid::rdinvint

/*===========================================================================*/

// RD inverse equations--mapping x,y to lat/long
long Projectoid::rdinv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{
double sum, dx, dy, tmp1, tmp2;
unsigned long i;

dx = (x - false_easting) * (1.0 / 100000.0);
dy = (y - false_northing) * (1.0 / 100000.0);

sum = 0.0;
for (i = 0; i < 11; i++)
	{
	tmp1 = pow(dx, (double)RD_K_P[i]);
	tmp2 = pow(dy, (double)RD_K_Q[i]);
	sum += RD_Kpq[i] * tmp1 * tmp2;
	} // for

*lat = (lat_origin + sum * (1.0 / 3600.0)) * D2R;

sum = 0.0;
for (i = 0; i < 12; i++)
	{
	tmp1 = pow(dx, (double)RD_L_P[i]);
	tmp2 = pow(dy, (double)RD_L_Q[i]);
	sum += RD_Lpq[i] * tmp1 * tmp2;
	} // for

*lon = (lon_origin + sum * (1.0 / 3600.0)) * D2R;

return(OK);

} // Projectoid::rdinv
