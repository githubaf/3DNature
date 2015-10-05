/*******************************************************************************
NAME                            NEW ZEALAND MAP GRID

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the New Zealand Map Grid.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER              DATE
----------              ----
F. Weed II              10/16/01 - adapted code that came from http://www.linz.govt.nz/services/surveysystem/geodetic/index.html

ALGORITHM REFERENCES

OSG Technical Report 4.1 (Office of the Surveyor-General)

*******************************************************************************/
#include "cproj.h"
#include "nzmg.h"

// Initialize the New Zealand Map Grid projection
long Projectoid::nzmginvint(void)
{

r_major = 6378388.0;
false_easting = 2510000.0;
false_northing = 6023150.0;
center_lat = -41.0;
center_lon = 173.0;

// Report parameters to the user
ptitle("NEW ZEALAND MAP GRID"); 
InverseOK[WCS_PROJECTIONCODE_NZMG] = 1;
InverseTransform = &Projectoid::nzmginv;

return(OK);

} // Projectoid::nzmginvint

/*===========================================================================*/

// New Zealand Map Grid inverse equations--mapping x,y to lat/long
long Projectoid::nzmginv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{
geolib_complex z0, z1, zn, zd, tmp1, tmp2;
double sum,tmp;
short i, it;

z0.real = (y - false_northing) / r_major;   z0.imag = (x - false_easting) / r_major;
z1.real = nzmg_cfb2[5].real;   z1.imag = nzmg_cfb2[5].imag;
for (i = 5; i--; )
	cadd(&z1, cmult(&z1, &z1, &z0), nzmg_cfb2 + i );
cmult(&z1, &z1, &z0);

for (it = 2; it--; )
	{
	cscale(&zn, nzmg_cfb1 + 5, 5.0);
	cscale(&zd, nzmg_cfb1 + 5, 6.0);
	for (i = 4; i; i--)
		{
		cadd(&zn, cmult(&tmp1, &zn, &z1), cscale(&tmp2, nzmg_cfb1 + i, (double) i));
		cadd(&zd, cmult(&tmp1, &zd, &z1), cscale(&tmp2, nzmg_cfb1 + i, (double) (i + 1)));
		}
	cadd(&zn, &z0, cmult(&zn, cmult(&zn, &zn, &z1), &z1));
	cadd(&zd, nzmg_cfb1, cmult(&zd, &zd, &z1 ));
	cdiv(&z1, &zn, &zd );
	}

*lon = center_lon / R2D + z1.imag;

tmp = z1.real;
sum = nzmg_cfl[8];
for (i = 8; i--; )
	sum = sum * tmp + nzmg_cfl[i];
sum *= tmp * (1.0 / 3600.0e-5);				// <<<F2 OPT>>>
*lat = (center_lat + sum) * (1.0 / R2D);	// <<<F2 OPT>>>

return(OK);

} // Projectoid::nzmginv
