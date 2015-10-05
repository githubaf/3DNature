// for_init.cpp
// Forward initialization file for GCTP2cpp
// Adapted from GCTPC2 code 12/04/00 FPW2
// Copyright 2000 3D Nature LLC

/*******************************************************************************
NAME                           FOR_INIT 

PURPOSE:                Initializes forward projection transformation parameters

PROGRAMMER              DATE                    REASON
----------              ----                    ------
T. Mittan               3-09-93                 Initial Development
S. Nelson               11-94                   Added Clarke spheroid default to UTM
S. Nelson                1-98                   Changed datum to spheroid

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

void Projectoid::for_init(
long outsys,				// output system code
long outzone,				// output zone number
double *outparm,			// output array of projection parameters
long outspheroid,			// output spheroid
char *fn27,					// NAD 1927 parameter file
char *fn83,					// NAD 1983 parameter file
long *iflg)					// status flag
{
long zone;					// zone number
double azimuth;				// azimuth
double alf;					// SOM angle
double angle;				// rotation angle
double lon1;				// longitude point in utm scene
double lon2;				// 2nd longitude
double lat1;				// 1st standard parallel
double lat2;				// 2nd standard parallel
double center_long;			// center longitude
double center_lat;			// center latitude
double h;					// height above sphere
double lon_origin;			// longitude at origin
double lat_origin;			// latitude at origin
double r_major;				// major axis in meters
double r_minor;				// minor axis in meters
double scale_factor;		// scale factor
double false_easting;		// false easting in meters
double false_northing;		// false northing in meters
double shape_m;				// constant used for Oblated Equal Area
double shape_n;				// constant used for Oblated Equal Area
long   start;				// where SOM starts beginning or end
double time;				// SOM time
double radius;				// radius of sphere
long tmpspheroid;			// temporary spheroid for UTM
long path;					// SOM path number
long satnum;				// SOM satellite number
long mode;					// which initialization method to use A or B

// Initialize forward transformations

// find the correct major and minor axis
sphdz(outspheroid, outparm, &r_major, &r_minor, &radius);
false_easting  = outparm[6];
false_northing = outparm[7];

if (outsys == WCS_PROJECTIONCODE_UTM)	// initialize U T M
	{
	// set Clarke 1866 spheroid if negative spheroid code  
	if (outspheroid < 0)
		{
		tmpspheroid = 0;
		sphdz(tmpspheroid, outparm, &r_major, &r_minor, &radius);
		}
	zone = outzone;
	if (zone == 0)
		{
		lon1 = paksz(outparm[0], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lat1 = paksz(outparm[1], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		zone = calc_utm_zone(lon1 * R2D);
		if (lat1 < 0)
			zone = -zone;
		}
	scale_factor = .9996;
	*iflg = utmforint(r_major, r_minor, scale_factor, zone);
//	for_trans[outsys] = this->utmfor;
	}
else if (outsys == WCS_PROJECTIONCODE_SPCS)	// initialize STATE PLANE 
	{
	*iflg = stplnforint(outzone, outspheroid, fn27, fn83);
	if (*iflg != 0)
		return;
//	for_trans[outsys] = this->stplnfor;
	}
else if (outsys == WCS_PROJECTIONCODE_ALBERS)	// initialize ALBERS CONICAL EQUAL AREA 
	{
	lat1 = paksz(outparm[2], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat2 = paksz(outparm[3], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat_origin = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = alberforint(r_major, r_minor, lat1, lat2, center_long, lat_origin, false_easting, false_northing);
//	for_trans[outsys] = this->alberfor;
	}
else if (outsys == WCS_PROJECTIONCODE_LAMCC)	// initialize LAMBERT CONFORMAL CONIC 
	{
	lat1 = paksz(outparm[2], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat2 = paksz(outparm[3], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat_origin  = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = lamccforint(r_major, r_minor, lat1, lat2, center_long, lat_origin, false_easting, false_northing);
//	for_trans[outsys] = this->lamccfor;
	}
else if (outsys == WCS_PROJECTIONCODE_MERCAT)	// initialize MERCATOR
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat1   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = merforint(r_major, r_minor, center_long, lat1, false_easting, false_northing);
//	for_trans[outsys] = this->merfor;
	}
else if (outsys == WCS_PROJECTIONCODE_PS)	// initialize POLAR STEREOGRAPHIC 
	{
	center_long = paksz(outparm[4],iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat1  = paksz(outparm[5],iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = psforint(r_major, r_minor, center_long, lat1, false_easting, false_northing);
//	for_trans[outsys] = this->psfor;
	}
else if (outsys == WCS_PROJECTIONCODE_POLYC)	// initialize POLYCONIC
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat_origin   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = polyforint(r_major, r_minor, center_long, lat_origin, false_easting, false_northing); 
//	for_trans[outsys] = this->polyfor;
	}
else if (outsys == WCS_PROJECTIONCODE_EQUIDC)	// initialize EQUIDISTANT CONIC 
	{
	lat1 = paksz(outparm[2], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat2 = paksz(outparm[3], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat_origin   = paksz(outparm[5],iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	if (outparm[8] == 0)
		mode = 0;
	else
		mode = 1;
	*iflg = eqconforint(r_major, r_minor, lat1, lat2, center_long, lat_origin, false_easting, false_northing, mode);
//	for_trans[outsys] = this->eqconfor;
	}
else if (outsys == WCS_PROJECTIONCODE_TM)	// initialize TRANSVERSE MECTAR
	{
	scale_factor = outparm[2];
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat_origin   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = tmforint(r_major, r_minor, scale_factor, center_long, lat_origin, false_easting, false_northing);
//	for_trans[outsys] = this->tmfor;
	}
else if (outsys == WCS_PROJECTIONCODE_STEREO)	// initialize STEREOGRAPHIC
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = sterforint(radius, center_long, center_lat, false_easting, false_northing); 
//	for_trans[outsys] = this->sterfor;
	}
else if (outsys == WCS_PROJECTIONCODE_LAMAZ)	// initialize LAMBERT AZIMUTHAL
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat  = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = lamazforint(radius, center_long, center_lat, false_easting, false_northing);
//	for_trans[outsys] = this->lamazfor;
	}
else if (outsys == WCS_PROJECTIONCODE_AZMEQD)	// initialize AZIMUTHAL EQUIDISTANT
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = azimforint(radius, center_long, center_lat, false_easting, false_northing); 
//	for_trans[outsys] = this->azimfor;
	}
else if (outsys == WCS_PROJECTIONCODE_GNOMON)	// initialize GNOMONIC 
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = gnomforint(radius, center_long, center_lat, false_easting, false_northing);
//	for_trans[outsys] = this->gnomfor;
	}
else if (outsys == WCS_PROJECTIONCODE_ORTHO)	// initalize ORTHOGRAPHIC
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = orthforint(radius, center_long, center_lat, false_easting, false_northing); 
//	for_trans[outsys] = this->orthfor;
	}
else if (outsys == WCS_PROJECTIONCODE_GVNSP)	// initalize GENERAL VERTICAL NEAR-SIDE PERSPECTIVE
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	h = outparm[2];
	*iflg = gvnspforint(radius, h, center_long, center_lat, false_easting, false_northing);
//	for_trans[outsys] = this->gvnspfor;
	}
else if (outsys == WCS_PROJECTIONCODE_SNSOID)	// initialize SINUSOIDAL 
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = sinforint(radius, center_long, false_easting, false_northing);
//	for_trans[outsys] = this->sinfor;
	}
else if (outsys == WCS_PROJECTIONCODE_EQRECT)	// initialize EQUIRECTANGULAR
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	lat1   = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = equiforint(radius,center_long,lat1,false_easting,false_northing); 
//	for_trans[outsys] = this->equifor;
	}
else if (outsys == WCS_PROJECTIONCODE_MILLER)	// initialize MILLER CYLINDRICAL 
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = millforint(radius, center_long, false_easting, false_northing); 
//	for_trans[outsys] = this->millfor;
	}
else if (outsys == WCS_PROJECTIONCODE_VGRINT)	// initialize VAN DER GRINTEN 
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = vandgforint(radius, center_long, false_easting, false_northing); 
//	for_trans[outsys] = this->vandgfor;
	}
else if (outsys == WCS_PROJECTIONCODE_HOM)	// initialize HOTLINE OBLIQUE MERCATOR
	{
	scale_factor = outparm[2];
	lat_origin = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	if (outparm[12] != 0)
		{	// HOM B
		mode = 1;
		azimuth = paksz(outparm[3], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lon_origin = paksz(outparm[4], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lon1 = lat1 = lon2 = lat2 = 0.0;	// make sure they're initialized
		}
	else	// HOM A
		{
		mode = 0;
		lon1 = paksz(outparm[8], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lat1 = paksz(outparm[9], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lon2 = paksz(outparm[10], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lat2 = paksz(outparm[11], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		}
	*iflg = omerforint(r_major, r_minor, scale_factor, azimuth, lon_origin, lat_origin, false_easting, false_northing, lon1, lat1, lon2, lat2, mode);
//	for_trans[outsys] = this->omerfor;
	}
else if (outsys == WCS_PROJECTIONCODE_SOM)	// initialize SOM 
	{
	path = (long)outparm[3];
	satnum = (long)outparm[2];
	if (outparm[12] == 0)
		{
		mode = 1;
		alf = paksz(outparm[3], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		lon1 = paksz(outparm[4], iflg) * 3600 * S2R;
		if (*iflg != 0)
			return;
		time = outparm[8];
		start = (long)outparm[10];
		}
	else
		mode = 0;
//	*iflg = somforint(r_major,r_minor,satnum,path,false_easting,false_northing);
	*iflg = somforint(r_major ,r_minor, satnum, path, alf, lon1, false_easting, false_northing, time, start, mode);
//	for_trans[outsys] = this->somfor;
	}
else if (outsys == WCS_PROJECTIONCODE_HAMMER)	// initialize HAMMER 
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = hamforint(radius, center_long, false_easting, false_northing); 
//	for_trans[outsys] = this->hamfor;
	}
else if (outsys == WCS_PROJECTIONCODE_ROBIN)	// initialize ROBINSON 
	{
	center_long  = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = robforint(radius, center_long, false_easting, false_northing); 
//	for_trans[outsys] = this->robfor;
	}
else if (outsys == WCS_PROJECTIONCODE_GOOD)	// initialize GOODE'S HOMOLOSINE
	{
	*iflg = goodforint(radius);
//	for_trans[outsys] = this->goodfor;
	}
else if (outsys == WCS_PROJECTIONCODE_MOLL)	// initialize MOLLWEIDE
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = molwforint(radius, center_long, false_easting, false_northing);
//	for_trans[outsys] = this->molwfor;
	}
else if (outsys == WCS_PROJECTIONCODE_IMOLL)	// initialize INTERRUPTED MOLLWEIDE
	{
	*iflg = imolwforint(radius);
//	for_trans[outsys] = this->imolwfor;
	}
else if (outsys == WCS_PROJECTIONCODE_ALASKA)	// initialize ALASKA CONFORMAL 
	{
	*iflg = alconforint(r_major, r_minor, false_easting, false_northing);
//	for_trans[outsys] = this->alconfor;
	}
else if (outsys == WCS_PROJECTIONCODE_WAGIV)	// initialize WAGNER IV 
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = wivforint(radius, center_long, false_easting, false_northing);
//	for_trans[outsys] = this->wivfor;
	}
else if (outsys == WCS_PROJECTIONCODE_WAGVII)	// initialize WAGNER VII 
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = wviiforint(radius, center_long, false_easting, false_northing);
//	for_trans[outsys] = this->wviifor;
	}
else if (outsys == WCS_PROJECTIONCODE_OBEQA)	// initialize OBLATED EQUAL AREA 
	{
	center_long = paksz(outparm[4], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	center_lat  = paksz(outparm[5], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	shape_m = outparm[2];
	shape_n = outparm[3];
	angle = paksz(outparm[8], iflg) * 3600 * S2R;
	if (*iflg != 0)
		return;
	*iflg = obleqforint(radius, center_long, center_lat, shape_m, shape_n, angle, false_easting, false_northing);
//	for_trans[outsys] = this->obleqfor;
	}
else if (outsys == WCS_PROJECTIONCODE_NZMG)	// initialize NEW ZEALAND MAP GRID
	{
	*iflg = nzmgforint();
	}
else if (WCS_PROJECTIONCODE_RD == outsys)
	{
	*iflg = rdforint();
	} // else if

} // Projectoid::for_init
