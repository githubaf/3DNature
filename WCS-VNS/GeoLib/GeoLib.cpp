/*******************************************************************************
NAME                           GCTP 

VERSION	PROGRAMMER      DATE
-------	----------      ----
		T. Mittan		2-26-93			Conversion from FORTRAN to C
		S. Nelson		12-14-93		Added assignments to inunit and 
										outunit for State Plane purposes.
c.1.0	S. Nelson		9-15-94			Added outdatum parameter call.
c.1.1	S. Nelson		11-94			Modified code so that PC_UTM can accept
										any spheroid code.  Changed State
										Plane legislated distance units,
										for NAD83, to be consistant with
										FORTRAN version of GCTP.  Unit codes
										are specified by state laws as of
										2/1/92.
c.1.5	S. Nelson		 1-98			Changed the name datum to spheroid.

										In initialize test, before the init
										functions were called for State Plane
										every time.  Now, initialization will
										only be done if zone, spheroid, or
										the parameter array changes which is
										consistant with the other projections.

										For the PC_UTM inverse projections the 1st
										2 elements of a temporary projection
										array were being assigned to a lat and
										long that lies in that zone area.  This
										has been eliminated.  PC_UTM will be
										initialized the same as the other
										projections for inverse transformations.
										For forward transformations the
										temporary array still exists.
  
ALGORITHM REFERENCES

1.  Snyder, John P., "Map Projections--A Working Manual", U.S. Geological
    Survey Professional Paper 1395 (Supersedes USGS Bulletin 1532), United
    State Government Printing Office, Washington D.C., 1987.

2.  Snyder, John P. and Voxland, Philip M., "An Album of Map Projections",
    U.S. Geological Survey Professional Paper 1453 , United State Government
    Printing Office, Washington D.C., 1989.
*******************************************************************************/
#include "cproj.h"

#define TRUE 1
#define FALSE 0

static long iter = 0;						// First time flag
static long inpj[MAXPROJ + 1];				// input projection array
static long indat[MAXPROJ + 1];				// input dataum array
static long inzn[MAXPROJ + 1];				// input zone array
static double pdin[MAXPROJ + 1][COEFCT];	// input projection parm array
static long outpj[MAXPROJ + 1];				// output projection array
static long outdat[MAXPROJ + 1];			// output dataum array
static long outzn[MAXPROJ + 1];				// output zone array
static double pdout[MAXPROJ+1][COEFCT];		// output projection parm array

						// Table of unit codes as specified by state
						// laws as of 2/1/92 for NAD 1983 State Plane
						// projection, 1 = U.S. Survey Feet, 2 = Meters,
						// 5 = International Feet

static long NADUT[134] = { 1, 5, 1, 1, 5, 1, 1, 1, 1, 2, 1, 1, 1, 2, 1, 1, 2, 2,
						   1, 1, 5, 2, 1, 2, 5, 1, 2, 2, 2, 1, 1, 1, 5, 2, 1, 5,
						   2, 2, 5, 2, 1, 1, 5, 2, 2, 1, 2, 1, 2, 2, 1, 2, 2, 2 };

// <<<>>> Perhaps these could be static? -CXH
//MemberFuncFor (for_trans[MAXPROJ + 1]);	// forward function pointer array
//MemberFuncInv (inv_trans[MAXPROJ + 1]);	// inverse function pointer array

void Projectoid::gctp(
double *incoor,			// input coordinates
long *insys,			// input projection code
long *inzone,			// input zone number
double *inparm,			// input projection parameter array
long *inunit,			// input units
long *inspheroid,		// input spheroid
long *ipr,				// printout flag for error messages. 0=screen, 1=file, 2=both
char *efile,			// error file name
long *jpr,				// printout flag for projection parameters 0=screen, 1=file, 2 = both
char *pfile,			// error file name
double *outcoor,		// output coordinates
long *outsys,			// output projection code
long *outzone,			// output zone
double *outparm,		// output projection array
long *outunit,			// output units
long *outspheroid,		// output spheroid
char fn27[],			// file name of NAD 1927 parameter file
char fn83[],			// file name of NAD 1983 parameter file
long *iflg)				// error flag
{
double x;				// x coordinate
double y;				// y coordinate
double factor;			// conversion factor
double lon;				// longitude
double lat;				// latitude
long i,j;				// loop counters
long ininit_flag;		// input initilization flag
long outinit_flag;		// output initilization flag
long unit;				// temporary unit variable
double temparr[COEFCT];	// temporary projection array

// setup initilization flags and output message flags
ininit_flag = FALSE;
outinit_flag = FALSE;
*iflg = 0;

*iflg = init(*ipr, *jpr, efile, pfile);
if (*iflg != 0)
	return;

// check to see if initilization is required
// only the first 13 projection parameters are currently used.
// If more are added the loop should be increased.
if (iter == 0)
	{
	for (i = 0; i < MAXPROJ + 1; i++)
		{
		inpj[i] = 0;
		indat[i] = 0;
		inzn[i] = 0;
		outpj[i] = 0;
		outdat[i] = 0;
		outzn[i] = 0;
		for (j = 0; j < COEFCT; j++)
			{
			pdin[i][j] = 0.0;
			pdout[i][j] = 0.0;
			}
		}
	ininit_flag = TRUE;
	outinit_flag = TRUE;
	iter = 1;
	}
else
	{
	if (*insys != WCS_PROJECTIONCODE_GEO)
		{
		if ((inzn[*insys] != *inzone) || (indat[*insys] != *inspheroid) || (inpj[*insys] != *insys))
			{
			ininit_flag = TRUE;
			}
		else
			for (i = 0; i < 13; i++)
				if (pdin[*insys][i] != inparm[i])
					{
					ininit_flag = TRUE;
					break;
					}
		}
	if (*outsys != WCS_PROJECTIONCODE_GEO)
		{
		if ((outzn[*outsys] != *outzone) || (outdat[*outsys] != *outspheroid) || (outpj[*outsys] != *outsys))
			{
			outinit_flag = TRUE;
			}
		else
			for (i = 0; i < 13; i++)
				if (pdout[*outsys][i] != outparm[i])
					{
					outinit_flag = TRUE;
					break;
					}
		}
	}

// Check input and output projection numbers
if ((*insys < WCS_PROJECTIONCODE_GEO) || (*insys > MAXPROJ))
	{
	p_error("Insys is illegal", "GCTP-INPUT");
	*iflg = 1;
	return;
	}
if ((*outsys < WCS_PROJECTIONCODE_GEO) || (*outsys > MAXPROJ))
	{
	p_error("Outsys is illegal", "GCTP-OUTPUT");
	*iflg = 2;
	return;
	}

// find the correct conversion factor for units
unit = *inunit;

// use legislated unit table for State Plane
if ((*inspheroid == 0) && (*insys == WCS_PROJECTIONCODE_SPCS) && (*inunit == STPLN_TABLE)) 
		unit = WCS_PROJECTIONUNITCODE_FEET;
if ((*inspheroid == 8) && (*insys == WCS_PROJECTIONCODE_SPCS) && (*inunit == STPLN_TABLE))
		unit = NADUT[(*inzone)/100];

// find the factor unit conversions--all transformations are in radians or meters
if (*insys == WCS_PROJECTIONCODE_GEO)
	*iflg = untfz(unit, WCS_PROJECTIONUNITCODE_RADIAN, &factor); 
else
	*iflg = untfz(unit, WCS_PROJECTIONUNITCODE_METER, &factor); 
if (*insys == WCS_PROJECTIONCODE_SPCS)
	*inunit = unit;
if (*iflg != 0)
	{
	close_file();
	return;
	}

x = incoor[0] * factor;
y = incoor[1] * factor;

// Initialize inverse transformation
if (ininit_flag)
	{
	inpj[*insys] = *insys;
	indat[*insys] = *inspheroid;
	inzn[*insys] = *inzone;
	for (i = 0;i < COEFCT; i++)
		pdin[*insys][i] = inparm[i];

	// Call the initialization function
	inv_init(*insys, *inzone, inparm, *inspheroid, fn27, fn83, iflg);
	if (*iflg != 0)
		{
		close_file();
		return;
		}
	}

// Do actual transformations

// Inverse transformations
if (*insys == WCS_PROJECTIONCODE_GEO)
	{
	lon = x;
	lat = y;
	}
else if ((*iflg = CoordsInverse(*insys, x, y, &lon, &lat)) != 0)
	{
	close_file();
	return;
	}

// DATUM conversion should go here

// The datum conversion facilities should go here 

// Initialize forward transformation
if (outinit_flag)
	{
	outpj[*outsys] = *outsys;
	outdat[*outsys] = *outspheroid;
	outzn[*outsys] = *outzone;
	for (i = 0;i < COEFCT; i++)
		pdout[*outsys][i] = outparm[i];
	// If the projection is WCS_PROJECTIONCODE_UTM, copy to a temporary array.  This way, the
	// user does not have to enter the zone nor a lat/long within the zone.
	// The program will calculate the zone from the input lat/long.
	if (*outsys == WCS_PROJECTIONCODE_UTM)
		{
		for (i = 2; i < COEFCT; i++)
			temparr[i] = outparm[i];
		temparr[0] = 0;
		temparr[1] = 0;
		if (outparm[0] == 0.0)
			{
			temparr[0] = pakr2dm(lon);
			temparr[1] = pakr2dm(lat);
			}
		else
			{
			temparr[0] = outparm[0];
			temparr[1] = outparm[1];
			}
		for_init(*outsys, *outzone, temparr, *outspheroid, fn27, fn83, iflg);
		}
	else
		for_init(*outsys, *outzone, outparm, *outspheroid, fn27, fn83, iflg);
	if (*iflg != 0)
		{
		close_file();
		return;
		}
	}

// Forward transformations
if (*outsys == WCS_PROJECTIONCODE_GEO)
	{
	outcoor[0] = lon;
	outcoor[1] = lat;
	}
else if ((*iflg = CoordsForward(*outsys, lon, lat, &outcoor[0], &outcoor[1])) != 0)
	{
	close_file();
	return;
	}

// find the correct conversion factor for units
unit = *outunit;
// use legislated unit table
if ((*outspheroid == 0) && (*outsys == WCS_PROJECTIONCODE_SPCS) && (*outunit == STPLN_TABLE)) 
	unit = 1;
if ((*outspheroid == 8) && (*outsys == WCS_PROJECTIONCODE_SPCS) && (*outunit == STPLN_TABLE))
	unit = NADUT[(*outzone) / 100];

if (*outsys == WCS_PROJECTIONCODE_GEO)
	*iflg = untfz(WCS_PROJECTIONUNITCODE_RADIAN, unit, &factor); 
else
	*iflg = untfz(WCS_PROJECTIONUNITCODE_METER, unit, &factor); 

if (*outsys == WCS_PROJECTIONCODE_SPCS)
	*outunit = unit;

outcoor[0] *= factor;
outcoor[1] *= factor;
close_file();

}

/*===========================================================================*/

Projectoid::Projectoid(void)
{
short loop;

Angle = AscendingLong = AzimuthAngle = AzimuthPoint = 0.0;
CenterLat = CenterLong = CentralMeridian = 0.0;
Factor = FalseEast = FalseNorth = 0.0;
Height = InclinationAngle = LandsatRatio = 0.0;
Lat1 = Lat2 = LatZ = 0.0;
Long1 = Long2 = LongPole = LonZ = 0.0;
OriginLat = PathFlag = PathNum = PeriodSatRevolution = 0.0;
SatelliteNum = ShapeParamM = ShapeParamN = 0.0;
SMajor = SMinor = Sphere = 0.0;
StdPar = StdPar1 = StdPar2 = 0.0;
TrueScale = 0.0;

a = a2 = a4 = al = azimuth = 0.0;
b = bl = c = c1 = c3 = ca = 0.0;
center_lat = center_lon = cos_lat_o = 0.0;
cos_p10 = cos_p12 = cos_p13 = cos_p14 = 0.0;
cos_p15 = cos_p20 = cos_p26 = cosaz = cosgam = 0.0;
d = e = e0 = e1 = e2 = e3 = e4 = el = es = esp = 0.0;
f0 = fac = false_easting = false_northing = g = 0.0;
id = 0;

ind = 0.0;
//long ind;			// sphere flag value
insphere = -1;		// previous spheroid value
inzone = 0; 		// previous zone value

lat_center = lat_o = lat_origin = lon_center = lon_origin = 0.0;
m = m1 = ml0 = mcs = n = 0.0;
ndx = 0;
ns = ns0 = p = p21 = 0.0;
q = R = r_major = r_minor = 0.0;
rh = s = sa = scale_factor = 0.0;
sin_lat_o = sin_p10 = sin_p12 = sin_p13 = 0.0;
sin_p14 = sin_p15 = sin_p20 = sin_p26 = 0.0;
sinaz = singam = start = 0.0;
t = tcs = theta = ts = u = w = xj = 0.0;

for (loop = 0; loop < 6; loop++)
	{
	feast6[loop] = 0.0;
	lon_center6[loop] = 0.0;
	}

for (loop = 0; loop < 7; loop++)
	{
	acoef[loop] = 0.0;
	bcoef[loop] = 0.0;
	}

for (loop = 0; loop < 12; loop++)
	{
	feast12[loop] = 0.0;
	lon_center12[loop] = 0.0;
	}

for (loop = 0; loop < 21; loop++)
	{
	pr[loop] = 0.0;
	xlr[loop] = 0.0;
	}

for (loop = 0; loop < (MAXPROJ + 1) ; loop++)
	{
	ForwardOK[loop] = 0;
	InverseOK[loop] = 0;
	}

}

/*===========================================================================*/

Projectoid::~Projectoid(void)
{
}

/*===========================================================================*/

long Projectoid::CoordsForward(long ProjectionID, double lon, double lat, double *x, double *y)
{

if (ForwardOK[ProjectionID])
	return (this->*ForwardTransform)(lon, lat, x, y);
else
	return ERROR;

}

/*===========================================================================*/

long Projectoid::CoordsInverse(long ProjectionID, double x, double y, double *lon, double *lat)
{

if (InverseOK[ProjectionID])
	return (this->*InverseTransform)(x, y, lon, lat);
else
	return ERROR;

}
