/*******************************************************************************
NAME                            STATE PLANE 

PURPOSE:        Transforms input Easting and Northing to longitude and
                latitude for the State Plane projection.  The
                Easting and Northing must be in meters.  The longitude
                and latitude values will be returned in radians.

PROGRAMMER      DATE
----------      ----
T. Mittan       Apr, 1993
S. Nelson       Jan, 1998       Took out reinitialization test.  Initialization
                                will be performed in all cases.  Added comments
F. Weed II		Jan, 2001		Moved NAD tables to nads.h

ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include <stdio.h>
#include "cproj.h"
#include "nads.h"

// Initialize the State Plane projection
long Projectoid::stplninvint(
long   zone,            // zone number
long   sphere,          // spheroid number
char   *fn27,           // name of file containing the NAD27 parameters
char   *fn83)           // name of file containing the NAD83 parameters
{
long ind;               // index for the zone
long i;                 // loop control variable
long nadval;            // datum value for the report (27 or 83)
double table[9];        // array containing the projection information
char pname[33];         // projection name
char buf[100];          // buffer for error messages
                        // projection parameters
double r_maj, r_min, scale_fact, center_lon;
double center_lat, false_east, false_north;
double azimuth, lat_orig, lon_orig, lon1, lat1, lon2, lat2;
long mode,iflg;
long status;
FILE *ptr;

ind = -1;

// Find the index for the zone
if (zone > 0)
	{
	if (sphere == 0)
		{
		for (i = 0; i < 134; i++)
			{
			if (zone == NAD27[i])
				{
				ind = i;
				break;
				}
			}
		}
	else if (sphere == 8)
		{
		for (i = 0; i < 134; i++)
			{
			if (zone == NAD83[i])
				{
				ind = i;
				break;
				}
			}
		}
	else
		{
		sprintf(buf, "Illegal spheroid #%4d", sphere);
		p_error(buf, "state-spheroid");
		return(23);
		}
	}

if (ind == -1)
	{
	sprintf(buf, "Illegal zone #%4d  for spheroid #%4d", zone, sphere);
	p_error(buf, "state-init");
	return(21);
	}

// Open and read the parameter file to get this zone's parameters
if (sphere == 0)
	ptr = fopen(fn27, "rb");
else
	ptr = fopen(fn83, "rb");
if (ptr == NULL)
	{
	p_error("Error opening State Plane parameter file", "state-inv");
	return(22);
	}
fseek(ptr, ind * 432, 0);
status = ftell(ptr);
fread(pname, sizeof(char), 32, ptr);
fread(&id, sizeof(long), 1, ptr);
fread(table, sizeof(double), 9, ptr);
fclose(ptr);

if (id <= 0)
	{
	sprintf(buf, "Illegal zone #%4d  for spheroid #%4d", zone, sphere);
	p_error(buf, "state-init");
	return(21);
	}

// Report parameters to the user
ptitle("STATE PLANE"); 
genrpt_long(zone,"Zone:     ");
if (sphere == 0)
	nadval = 27;
else
	nadval = 83;
genrpt_long(nadval,"Datum:     NAD");

// initialize proper projection
r_maj = table[0];
r_min = (sqrt(1.0 - table[1])) * table[0];

if (id == 1)
	{
	scale_fact = table[3];
	center_lon = paksz(pakcz(table[2]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	center_lat = paksz(pakcz(table[6]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	false_east = table[7];
	false_north = table[8];
	tminvint(r_maj, r_min, scale_fact, center_lon, center_lat, false_east, false_north);
	}
else if (id == 2)
	{
	lat1 = paksz(pakcz(table[5]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	lat2 = paksz(pakcz(table[4]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	center_lon = paksz(pakcz(table[2]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	center_lat = paksz(pakcz(table[6]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	false_east = table[7];
	false_north = table[8];
	lamccinvint(r_maj, r_min, lat1, lat2, center_lon, center_lat, false_east, false_north);
	}
else if (id == 3)
	{
	center_lon = paksz(pakcz(table[2]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	center_lat = paksz(pakcz(table[3]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	false_east = table[4];
	false_north = table[5];
	polyinvint(r_maj, r_min, center_lon, center_lat, false_east, false_north);
	}
else if (id == 4)
	{
	scale_fact = table[3];
	azimuth = paksz(pakcz(table[5]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	lon_orig = paksz(pakcz(table[2]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	lat_orig = paksz(pakcz(table[6]), &iflg) * D2R;
	if (iflg != 0)
		return(iflg);
	false_east = table[7];
	false_north = table[8];
	mode = 1;
	omerinvint(r_maj, r_min, scale_fact, azimuth, lon_orig, lat_orig, false_east, false_north, lon1, lat1, lon2, lat2, mode);
	}
InverseOK[WCS_PROJECTIONCODE_SPCS] = 1;
InverseTransform = &Projectoid::stplninv;

return(OK);

}

/*===========================================================================*/

// State Plane inverse equations--mapping x,y to lat/long
long Projectoid::stplninv(
double x,                       // (O) X projection coordinate
double y,                       // (O) Y projection coordinate
double *lon,                    // (I) Longitude
double *lat)                    // (I) Latitude
{
long iflg;

// Inverse equations
if (id == 1)
	{
	if ((iflg = tminv(x, y, lon, lat)) != 0)
		return(iflg);
	}
else if (id == 2)
	{
	if ((iflg = lamccinv(x, y, lon, lat)) != 0)
		return(iflg);
	}
else if (id == 3)
	{
	if ((iflg = polyinv(x, y, lon, lat)) != 0)
		return(iflg);
	}
else if (id == 4)
	{
	if ((iflg = omerinv(x, y, lon, lat)) != 0)
		return(iflg);
	}

return(OK);

}
