/*******************************************************************************
NAME                            Krovak Oblique Conformal Conic

PURPOSE:        Transforms input longitude and latitude to Easting and
                Northing for the Krovak system.  The
				longitude and latitude must be in radians.  The Easting
				and Northing values will be returned in meters.

PROGRAMMER              DATE
----------              ----
F. Weed II              03/23/04

ALGORITHM REFERENCES:

	POSC docs
	ASPRS Grids & Datums (Jan 2000)

NOTES:

	_MAJOR NOTE_: POSC HTML formula for 'B' is WRONG (as of 03/26/04)

	Transform should be able to handle all variants, but the user is responsible for
making sure they have the parameters for the right epoch & usage (military or civilian).
OK - issue to resolve - several prime meridians are involved in the various defs!!!

*******************************************************************************/
#include "cproj.h"

static double kocc_PST, qtr_pi, kocc_B, kocc_A, kocc_gamma_o, kocc_t_o, kocc_n, kocc_r_o;

// Initialize the Krovak Oblique Conformal Conic projection
long Projectoid::krovakoccforint(
double r_maj,                   // major axis
double r_min,                   // minor axis
double c_lon,					// center longitude
double c_lat,					// center latitude
double azmth,					// azimuth
double lat_pst,					// latitude of psuedo standard parallel
double sf,						// scale factor
double fe,						// false easting
double fn)						// false northing
{
double sclat, cclat, es, e, numerator, denominator;

// init here
r_major = r_maj;
r_minor = r_min;
center_lon = c_lon;
center_lat = c_lat;
azimuth = azmth;
kocc_PST = lat_pst;
scale_factor = sf;
false_easting = fe;
false_northing = fn;

sincos(c_lat, &sclat, &cclat);
es = 1.0 - SQUARE(r_minor / r_major);
e = sqrt(es);
qtr_pi = PI * 0.25;

kocc_B = sqrt(1.0 + (es * pow(cclat, 4.0)) / (1.0 - es));

kocc_A = r_major * sqrt(1.0 - es) / (1.0 - SQUARE(e) * SQUARE(sclat));

kocc_gamma_o = asin(sclat / kocc_B);

numerator = pow((1.0 + e * sclat) / (1.0 - e * sclat), e * kocc_B * 0.5);
denominator = pow(tan(qtr_pi + c_lat * 0.5), kocc_B);
kocc_t_o = tan(qtr_pi + kocc_gamma_o * 0.5) * numerator / denominator;

kocc_n = sin(lat_pst);

kocc_r_o = sf * kocc_A / tan(lat_pst);

// Report parameters to the user
ptitle("KROVAK"); 
ForwardOK[WCS_PROJECTIONCODE_KROVAK] = 1;
ForwardTransform = &Projectoid::krovakoccfor;

return(OK);

} // Projectoid::krovakoccforint

/*===========================================================================*/

// Krovak Oblique Conformal Conic forward equations--mapping lat,long to x,y
long Projectoid::krovakoccfor(
double lon,                     // (I) Longitude
double lat,                     // (I) Latitude
double *x,                      // (O) X projection coordinate
double *y)                      // (O) Y projection coordinate
{
double slat, tmp1, tmp2, U, V, S, D, theta, r;
// sine/cosine pairs below
double sac, cac, sU, cU, sV, cV, st, ct;
double lon_adjust = -(17.0 + 40.0 / 60.0) * D2R;	// Ferro to Greenwich adjustment


// for debugging only!
lon = 0.294083999;
lat = 0.876312566;


slat = sin(lat);
tmp1 = pow(tan(lat * 0.5 + qtr_pi), kocc_B);
tmp2 = pow((1.0 + e * slat) / (1.0 - e * slat), e * kocc_B * 0.5);
U = 2.0 * (atan(kocc_t_o * tmp1 / tmp2) - qtr_pi);		// this isn't ending up equal to their answer!!!

V = kocc_B * (center_lon + lon_adjust - lon);

sincos(U, &sU, &cU);
sincos(V, &sV, &cV);
sincos(azimuth, &sac, &cac);

S = asin(cac * sU + sac * cU * cV);

D = asin(cU * sV / cos(S));

theta = kocc_n * D;
sincos(theta, &st, &ct);

r = kocc_r_o * pow(tan(qtr_pi + kocc_PST * 0.5), kocc_n) / pow(tan(S * 0.5 + qtr_pi), kocc_n);

// these equations may only be valid for S-JTSK(Ferro) variant
*x = r * ct;	// coded for E sub c as zero
*y = r * st;	// coded for N sub c as zero

return(OK);

} // Projectoid::krovakoccfor
