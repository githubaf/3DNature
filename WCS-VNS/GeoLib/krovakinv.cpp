/*******************************************************************************
NAME                            Krovak Oblique Conformal Conic

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the Krovak system.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

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

	Easting & Northing are the traditional names for the vars, but in reality, the values
these refer to are southings & westings (due to 90 degree clockwise rotation)

*******************************************************************************/
#include "cproj.h"

static double kocc_PST, qtr_pi, kocc_B, kocc_A, kocc_gamma_o, kocc_t_o, kocc_n, kocc_r_o;

// Initialize the Krovak Oblique Conformal Conic projection
long Projectoid::krovakoccinvint(
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
InverseOK[WCS_PROJECTIONCODE_KROVAK] = 1;
InverseTransform = &Projectoid::krovakoccinv;

return(OK);

} // Projectoid::krovakoccinvint

/*===========================================================================*/

// Krovak inverse equations--mapping x,y to lat/long
long Projectoid::krovakoccinv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{
double r_prime, theta_prime, D_prime, S_prime, U_prime, V_prime, precomp1, precomp2, s_last_lat, tmp, last_lat, tmp_lat;
double lon_adjust = (17.0 + 40.0 / 60.0) * D2R;	// Ferro to Greenwich adjustment
// sine/cosine pairs below
double sac, cac, sDp, cDp, sSp, cSp;
bool looping = true;
long i;

r_prime = sqrt(SQUARE(x - false_easting) + SQUARE(y - false_northing));

theta_prime = atan(y / x);	// see docs - may not be correct for all epochs or usages
// uhhh - docs say atan[(E - Ec) / (N - Nc)], but since Ec & Nc are zero, this would
// reduce to atan[E / N].  However, the grid is rotated 90 degrees, so it appears that
// this really ends up as atan[N / E].  C'est La Vie

D_prime = theta_prime / sin(kocc_PST);

S_prime = 2.0 * (atan(pow(kocc_r_o / r_prime, 1.0 / kocc_n) * tan(qtr_pi + kocc_PST * 0.5)) - qtr_pi);

sincos(azimuth, &sac, &cac);
sincos(D_prime, &sDp, &cDp);
sincos(S_prime, &sSp, &cSp);

U_prime = asin(cac * sSp - sac * cSp * cDp);

V_prime = asin(cSp * sDp / cos(U_prime));

// nasty equation for latiude, so compute the constant parts once
precomp1 = pow(kocc_t_o, -1.0 / kocc_B);
precomp2 = pow(tan(U_prime * 0.5 + qtr_pi), 1.0 / kocc_B);

//s_last_lat = sin(U_prime);	// don't really know where the initial latitude "guess" comes from, but Proj4
//s_last_lat = U_prime;
// code uses something similar to this equation for it's initial guess
//tmp = pow((1.0 + e * s_last_lat) / (1.0 - e * s_last_lat), e * 0.5);
//last_lat = 2.0 * (atan(precomp1 * precomp2 * tmp) - qtr_pi);
last_lat = U_prime;

// compute by iteration
i = 0;
do
	{
	s_last_lat = sin(last_lat);
	tmp = pow((1.0 + e * s_last_lat) / (1.0 - e * s_last_lat), e * 0.5);
	tmp_lat = 2.0 * (atan(precomp1 * precomp2 * tmp) - qtr_pi);
	i++;
	looping = (i < 8) && (fabs(tmp_lat - last_lat) > 0.00000001);
	last_lat = tmp_lat;
	} while (looping);

*lat = tmp_lat;

*lon = (center_lon - lon_adjust) - V_prime / kocc_B;
// again, the docs are rather confusing on this issue, as they seem to use the same symbol
// (lambda - sub c) for the projection center in both Ferro & Greenwich adjust forms.  And
// worse still, it seems like both forms are used in the equations.  Where's Kreskin when
// you need him?

return(OK);

} // Projectoid::krovakoccinv
