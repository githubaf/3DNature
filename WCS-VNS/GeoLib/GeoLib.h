// GeoLib.h
// Header file for projection library code
// Adapted from GCTPC2 code 12/04/00 FPW2
// Copyright 2000 3D Nature LLC

// NOTE for those as confused as I am: GCTP is the original Fortran code, GCTPc is the C port of the GCTP version 2 code,
// and GCTPc2 is the second version of the C code.

enum ProjectionCode
		{
		WCS_PROJECTIONCODE_GEO, 				//	0 = Geographic
		WCS_PROJECTIONCODE_UTM, 				//	1 = Universal Transverse Mercator (UTM)
		WCS_PROJECTIONCODE_SPCS,				//	2 = State Plane Coordinates
		WCS_PROJECTIONCODE_ALBERS,				//	3 = Albers Conical Equal Area
		WCS_PROJECTIONCODE_LAMCC,				//	4 = Lambert Conformal Conic
		WCS_PROJECTIONCODE_MERCAT,				//	5 = Mercator
		WCS_PROJECTIONCODE_PS,					//	6 = Polar Stereographic
		WCS_PROJECTIONCODE_POLYC,				//	7 = Polyconic
		WCS_PROJECTIONCODE_EQUIDC,				//	8 = Equidistant Conic
		WCS_PROJECTIONCODE_TM,					//	9 = Transverse Mercator
		WCS_PROJECTIONCODE_STEREO,				// 10 = Stereographic
		WCS_PROJECTIONCODE_LAMAZ,				// 11 = Lambert Azimuthal Equal Area
		WCS_PROJECTIONCODE_AZMEQD,				// 12 = Azimuthal Equidistant
		WCS_PROJECTIONCODE_GNOMON,				// 13 = Gnomonic
		WCS_PROJECTIONCODE_ORTHO,				// 14 = Orthographic
		WCS_PROJECTIONCODE_GVNSP,				// 15 = General Vertical Near-Side Perspective
		WCS_PROJECTIONCODE_SNSOID,				// 16 = Sinusoidal
		WCS_PROJECTIONCODE_EQRECT,				// 17 = Equirectangular
		WCS_PROJECTIONCODE_MILLER,				// 18 = Miller Cylindrical
		WCS_PROJECTIONCODE_VGRINT,				// 19 = Van der Grinten
		WCS_PROJECTIONCODE_HOM, 				// 20 = (Hotine) Oblique Mercator
		WCS_PROJECTIONCODE_ROBIN,				// 21 = Robinson
		WCS_PROJECTIONCODE_SOM, 				// 22 = Space Oblique Mercator (SOM)
		WCS_PROJECTIONCODE_ALASKA,				// 23 = Alaska Conformal
		WCS_PROJECTIONCODE_GOOD,				// 24 = Interrupted Goode Homolosine
		WCS_PROJECTIONCODE_MOLL,				// 25 = Mollweide
		WCS_PROJECTIONCODE_IMOLL,				// 26 = Interrupted Mollweide
		WCS_PROJECTIONCODE_HAMMER,				// 27 = Hammer
		WCS_PROJECTIONCODE_WAGIV,				// 28 = Wagner IV
		WCS_PROJECTIONCODE_WAGVII,				// 29 = Wagner VII
		WCS_PROJECTIONCODE_OBEQA,				// 30 = Oblated Equal Area
		WCS_PROJECTIONCODE_NZMG,				// 31 = New Zealand Map Grid
		WCS_PROJECTIONCODE_USDEF = 99			// 99 = User defined
		};

enum UnitCode
		{
		WCS_PROJECTIONUNITCODE_RADIAN,			// 0 = Radians
		WCS_PROJECTIONUNITCODE_FEET,			// 1 = Feet
		WCS_PROJECTIONUNITCODE_METER,			// 2 = Meters
		WCS_PROJECTIONUNITCODE_SECOND,			// 3 = Seconds
		WCS_PROJECTIONUNITCODE_DEGREE,			// 4 = Decimal degrees
		WCS_PROJECTIONUNITCODE_INT_FEET			// 5 = International Feet
		};

// The STPLN_TABLE unit value is specifically used for State Plane -- if units
// equals STPLN_TABLE and Datum is NAD83--actual units are retrieved from
// a table according to the zone.  If Datum is NAD27--actual units will be feet.
// An error will occur with this unit if the projection is not State Plane.
#define STPLN_TABLE 6

// General code numbers
//#define IN_BREAK	-2 		//	Return status if the interupted projection
							//	point lies in the break area
#define COEFCT		15		//	projection coefficient count
#define PROJCT		30		//	projection count
#define SPHDCT		31		//	spheroid count

#define MAXPROJ		31		//	Maximum projection number
#define MAXUNIT		 5		//	Maximum unit code number
#define GEO_TERM	 0		//	Array index for print-to-term flag
#define GEO_FILE	 1		//	Array index for print-to-file flag
#define GEO_TRUE	 1		//	True value for geometric true/false flags
#define GEO_FALSE	-1		//	False val for geometric true/false flags

// Functions residing in cproj.c
void sincos( double val, double *sin_val, double *cos_val);
double asinz (double con);
double msfnz (double eccent, double sinphi, double cosphi);
double qsfnz (double eccent, double sinphi, double cosphi);
double phi1z (double eccent, double qs, long  *flag);
double phi2z(double eccent, double ts, long *flag);
double phi3z(double ml, double e0, double e1, double e2, double e3, long *flag);
//double phi4z (double eccent, double e0, double e1, double e2, double e3, double a, double b, double *c, double *phi);
// F2_MOD: 071702 - Changed to long return value
long phi4z (double eccent, double e0, double e1, double e2, double e3, double a, double b, double *c, double *phi);
double pakcz(double pak);
double pakr2dm(double pak);
double tsfnz(double eccent, double phi, double sinphi);
long sign(double x);	// F2_MOD: 071702 - changed from int to long
double adjust_lon(double x);
double e0fn(double x);
double e1fn(double x);
double e2fn(double x);
double e3fn(double x);
double e4fn(double x);
double mlfn(double e0, double e1, double e2, double e3, double phi);
long calc_utm_zone(double lon);
// End of functions residing in cproj.h

// functions in report.c
void close_file(void);
long init(long ipr, long jpr, char *efile, char *pfile);
void ptitle(char *A);
void radius(double A);
void radius2(double A, double B);
void cenlon( double A);
void cenlonmer(double A);
void cenlat(double A);
void origin(double A);
void stanparl(double A,double B);
void stparl1(double A);
void offsetp(double A, double B);
void genrpt(double A, char *S);
void genrpt_long(long A, char *S);
void pblank(void);
void p_error(char *what, char *where);
// End of the report.c functions

class Projectoid
	{
protected:
	// variables that can appear in GCTP's array of 15 projection parameters
	double Angle;
	double AscendingLong;
	double AzimuthAngle;
	double AzimuthPoint;
	double CenterLat;
	double CenterLong;
	double CentralMeridian;
	double Factor;
	double FalseEast;
	double FalseNorth;
	double Height;
	double InclinationAngle;
	double LandsatRatio;
	double Lat1, Lat2, LatZ;
	double Long1, Long2;
	double LongPole;
	double LonZ;
	double OriginLat;
	double PathFlag;
	double PathNum;
	double PeriodSatRevolution;
	double SatelliteNum;
	double ShapeParamM, ShapeParamN;
	double SMajor;
	double SMinor;
	double Sphere;
	double StdPar, StdPar1, StdPar2;
	double TrueScale;

private:
	char ForwardOK[MAXPROJ + 1];
	char InverseOK[MAXPROJ + 1];
	long (Projectoid::*ForwardTransform)(double, double, double *, double *);
	long (Projectoid::*InverseTransform)(double, double, double *, double *);
	// variables that can be used by the projection functions
	double a, a2, a4;
	double al;
	double azimuth;
	double acoef[7];
	double b;
	double bcoef[7];
	double bl;
	double c, c1, c3;
	double ca;
	double center_lat;			// center latitude
	double center_lon;			// center longituted
	double cos_lat_o;			// Cosine of the center latitude
	double cos_p10;				// cos of center latitude
	double cos_p12;				// cos of center latitude
	double cos_p13;				// Cosine of the center latitude
	double cos_p14;				// cos of center latitude
	double cos_p15;				// Cosine of the center latitude
	double cos_p20;				// sin and cos values
	double cos_p26;
	double cosaz;
	double cosgam;
	double d;
	double e;					// eccentricity constant
	double e0, e1, e2 ,e3;		// eccentricity constants
	double e4;					// e4 calculated from eccentricity
	double el;
	double es,esp;				// eccentricity constants
	double f0;					// flattening of ellipsoid
	double fac;					// sign variable
	double false_easting;		// x offset in meters
	double false_northing;		// y offset in meters
	double feast6[6]; 			// False easting, one for each region
	double feast12[12];			// False easting, one for each region
	double g;
	long id; 					// indicates which projection is to be transformed
	double ind;					// spherical flag
	//long ind;					// sphere flag value
	// set the initialized values for zone and spheroid.  This value determines
	// whether to initialize or not
/*** FIX IN CONSTRUCTOR ***/
	long insphere; // = -1;		// previous spheroid value
	long inzone; // = 0; 		// previous zone value
	double lat_center;			// center latitude
	double lat_o;
	double lat_origin;			// center latitude
	double lon_center;			// center longitude
	double lon_center6[6];		// Central meridians, one for each region
	double lon_center12[12];	// Central meridians, one for each region
	double lon_origin;			// center longitude
	double m;
	double m1;					// small value m
	double ml0;					// small value m
	double mcs;					// small m
	double n;
	long ndx;
	double ns;					// ratio of angle between meridian
	double ns0;					// ratio between meridians
	double p;					// Height above sphere
	double p21;
	double pr[21];
	double q;
	double R;					// Radius of the earth (sphere)
	double r_major;				// major axis
	double r_minor;				// minor axis
	double rh;					// height above elipsoid
	double s, sa;
	double scale_factor;	 	// scale factor
	double sin_lat_o;			// Sine of the center latitude
	double sin_p10;				// sin of center latitude
	double sin_p12;				// sin of center latitude
	double sin_p13;				// Sine of the center latitude
	double sin_p14;				// sin of center latitude
	double sin_p15;				// Sine of the center latitude
	double sin_p20;
	double sin_p26;
	double sinaz;
	double singam;
	double start;
	double t;
	double tcs;					// small t
	double theta;
	double ts;
	double u;
	double w;
	double xj;
	double xlr[21];

public:
	// create & destroy
	Projectoid(void);
	~Projectoid(void);

	// the main routines
	void for_init(long outsys, long outzone, double *outparm, long outspheroid, char *fn27, char *fn83, long *iflg);
	void gctp(double *incoor, long *insys, long *inzone, double *inparm, long *inunit, long *inspheroid, long *ipr, char *efile, long *jpr,
		char *pfile, double *outcoor, long *outsys, long *outzone, double *outparm, long *outunit, long *outspheroid,
		char fn27[], char fn83[], long *iflg);
	void inv_init(long insys, long inzone, double *inparm, long inspheroid, char *fn27, char *fn83, long *iflg);
	double paksz(double ang, long *iflg);
	void sphdz(long isph, double *parm, double *r_major, double *r_minor, double *radius);
	long untfz(long inunit, long outunit, double *factor);
	long CoordsForward(long ProjectionID, double lon, double lat, double *x, double *y);
	long CoordsInverse(long ProjectionID, double x, double y, double *lon, double *lat);

private:
	// now we have the actual projection functions
	// AlaskaConformalProjection
	long alconforint(double r_maj, double r_min, double false_east, double false_north);
	long alconfor(double lon, double lat, double *x, double *y);
	long alconinvint(double r_maj, double r_min, double false_east, double false_north);
	long alconinv( double x, double y, double *lon, double *lat);
	// AlbersConEqAreaProjection
	long alberforint(double r_maj, double r_min, double lat1, double lat2, double lon0, double lat0, double false_east, double false_north);
	long alberfor(double lon, double lat, double *x, double *y);
	long alberinvint( double r_maj, double r_min, double lat1, double lat2, double lon0, double lat0, double false_east, double false_north);
	long alberinv( double x, double y, double *lon, double *lat);
	// AzimuthalEqProjection
	long azimforint( double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long azimfor( double lon, double lat, double *x, double *y);
	long aziminvint( double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long aziminv( double x, double y, double *lon, double *lat);
	// EquiConicProjection
	long eqconforint(double r_maj, double r_min, double lat1, double lat2, double center_lon, double center_lat,
		double false_east, double false_north, long mode);
	long eqconfor(double lon, double lat, double *x, double *y);
	long eqconinvint(double r_maj, double r_min, double lat1, double lat2, double center_lon, double center_lat,
		double false_east, double false_north, long mode);
	long eqconinv(double x, double y, double *lon, double *lat);
	// EquirectangularProjection
	long equiforint(double r_maj, double center_lon, double lat1, double false_east, double false_north);
	long equifor(double lon, double lat, double *x, double *y);
	long equiinvint(double r_maj, double center_lon, double lat1, double false_east, double false_north);
	long equiinv(double x, double y, double *lon, double *lat);
	// GeneralVertNearSideProjection
	long gvnspforint(double r, double h, double center_long, double center_lat, double false_east, double false_north);
	long gvnspfor(double lon, double lat, double *x, double *y);
	long gvnspinvint(double r, double h, double center_long, double center_lat, double false_east, double false_north);
	long gvnspinv(double x, double y, double *lon, double *lat);
	// GnomonicProjection
	long gnomforint(double r, double center_long, double center_lat, double false_east, double false_north);
	long gnomfor(double lon, double lat, double *x, double *y);
	long gnominvint(double r, double center_long, double center_lat, double false_east, double false_north);
	long gnominv(double x, double y, double *lon, double *lat);
	// HammerProjection
	long hamforint(double r, double center_long, double false_east, double false_north);
	long hamfor(double lon, double lat, double *x, double *y);
	long haminvint(double r, double center_long, double false_east, double false_north);
	long haminv(double x, double y, double *lon, double *lat);
	// HotineOblMercProjection
	long omerforint(double r_maj, double r_min, double scale_fact, double azimuth, double lon_orig, double lat_orig, double false_east,
		double false_north, double lon1, double lat1, double lon2, double lat2, long mode);
	long omerfor(double lon, double lat, double *x, double *y);
	long omerinvint(double r_maj, double r_min, double scale_fact, double azimuth, double lon_orig, double lat_orig, double false_east,
		double false_north, double lon1, double lat1, double lon2, double lat2, long mode);
	long omerinv(double x, double y, double *lon, double *lat);
	// IntGoodeHomolosineProjection
	long goodforint(double r);
	long goodfor(double lon, double lat, double *x, double *y);
	long goodinvint(double r);
	long goodinv(double x, double y, double *lon, double *lat);
	// IntMollwiedeProjection
	long imolwforint(double r);
	long imolwfor(double lon, double lat, double *x, double *y);
	long imolwinvint(double r);
	long imolwinv(double x, double y, double *lon, double *lat);
	// LambertAzEqAreaProjection
	long lamazforint(double r, double center_long, double center_lat, double false_east, double false_north);
	long lamazfor(double lon, double lat, double *x, double *y);
	long lamazinvint(double r, double center_long, double center_lat, double false_east, double false_north);
	long lamazinv(double x, double y, double *lon, double *lat);
	// LambertCCProjection 
	long lamccforint(double r_maj, double r_min, double lat1, double lat2, double c_lon, double c_lat, double false_east, double false_north);
	long lamccfor(double lon, double lat, double *x, double *y);
	long lamccinvint(double r_maj, double r_min, double lat1, double lat2, double c_lon, double c_lat, double false_east, double false_north);
	long lamccinv(double x, double y, double *lon, double *lat);
	// MercatorProjection
	long merforint(double r_maj, double r_min, double center_lon, double center_lat, double false_east, double false_north);
	long merfor(double lon, double lat, double *x, double *y);
	long merinvint(double r_maj, double r_min, double center_lon, double center_lat, double false_east, double false_north);
	long merinv(double x, double y, double *lon, double *lat);
	// MillerCylindricalProjection
	long millforint(double r, double center_long, double false_east, double false_north);
	long millfor(double lon, double lat, double *x, double *y);
	long millinvint(double r, double center_long, double false_east, double false_north);
	long millinv(double x, double y, double *lon, double *lat);
	// MollweideProjection
	long molwforint(double r, double center_long, double false_east, double false_north);
	long molwfor(double lon, double lat, double *x, double *y);
	long molwinvint(double r, double center_long, double false_east, double false_north);
	long molwinv(double x, double y, double *lon, double *lat);
	// New Zealand Map Grid
	long nzmgforint(void);
	long nzmgfor(double lon, double lat, double *x, double *y);
	long nzmginvint(void);
	long nzmginv(double x, double y, double *lon, double *lat);
	// OblEqAreaProjection
	long obleqforint(double r, double center_long, double center_lat, double shape_m, double shape_n, double angle, double false_east,
		double false_north);
	long obleqfor(double lon, double lat, double *x, double *y);
	long obleqinvint(double r, double center_long, double center_lat, double shape_m, double shape_n, double angle, double false_east,
		double false_north);
	long obleqinv(double x, double y, double *lon, double *lat);
	// OrthographicProjection
	long orthforint(double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long orthfor( double lon, double lat, double *x, double *y);
	long orthinvint(double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long orthinv(double x, double y, double *lon, double *lat);
	// PolarStereographicProjection
	long psforint(double r_maj, double r_min, double c_lon, double c_lat, double false_east, double false_north);
	long psfor( double lon, double lat, double *x, double *y);
	long psinvint(double r_maj, double r_min, double c_lon, double c_lat, double false_east, double false_north);
	long psinv( double x, double y, double *lon, double *lat);
	// PolyconicProjection
	long polyforint(double r_maj, double r_min, double center_lon, double center_lat, double false_east, double false_north);
	long polyfor(double lon, double lat, double *x, double *y);
	long polyinvint(double r_maj, double r_min, double center_lon, double center_lat, double false_east, double false_north);
	long polyinv(double x, double y, double *lon, double *lat);
	// RobinsonProjection
	long robforint(double r, double center_long, double false_east, double false_north);
	long robfor( double lon, double lat, double *x, double *y);
	long robinvint(double r, double center_long, double false_east, double false_north);
	long robinv(double x, double y, double *lon, double *lat);
	// SinusoidalProjection
	long sinforint(double r, double center_long, double false_east, double false_north);
	long sinfor(double lon, double lat, double *x, double *y);
	long sininvint(double r, double center_long, double false_east, double false_north);
	long sininv(double x, double y, double *lon, double *lat);
	// SpaceOblMercProjection
	long somforint(double r_major, double r_minor, long satnum, long path, double alf_in, double lon, double false_east, double false_north,
		double time, long start1, long flag);
	long somfor(double lon, double lat, double *x, double *y);
	long sominvint(double  r_major, double r_minor, long satnum, long path, double alf_in, double lon, double false_east, double false_north,
		double time, long start1, long flag);
	long sominv(double x, double y, double *lon, double *lat);
	void som_series(double *fb, double *fa2, double *fa4, double *fc1, double *fc3, double *dlam);
	// StatePlaneProjection
	long stplnforint(long zone, long sphere, char *fn27, char *fn83);
	long stplnfor(double lon, double lat, double *x, double *y);
	long stplninvint(long zone, long sphere, char *fn27, char *fn83);
	long stplninv(double x, double y, double *lon, double *lat);
	// StereographicProjection
	long sterforint(double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long sterfor(double lon, double lat, double *x, double *y);
	long sterinvint(double r_maj, double center_lon, double center_lat, double false_east, double false_north);
	long sterinv(double x, double y, double *lon, double *lat);
	// TransverseMercatorProjection
	long tmforint(double r_maj, double r_min, double scale_fact, double center_lon, double center_lat, double false_east, double false_north);
	long tmfor(double lon, double lat, double *x, double *y);
	long tminvint(double r_maj, double r_min, double scale_fact, double center_lon, double center_lat, double false_east, double false_north);
	long tminv(double x, double y, double *lon, double *lat);
	// UTMProjection
	long utmforint(double r_maj, double r_min, double scale_fact, long zone); 
	long utmfor(double lon, double lat, double *x, double *y);
	long utminvint(double r_maj, double r_min, double scale_fact, long zone);
	long utminv(double x, double y, double *lon, double *lat);
	// VanDerGrintenProjection
	long vandgforint(double r, double center_long, double false_east, double false_north);
	long vandgfor( double lon, double lat, double *x, double *y);
	long vandginvint(double r, double center_long, double false_east, double false_north);
	long vandginv(double x, double y, double *lon, double *lat);
	// WagnerIVProjection
	long wivforint(double r, double center_long, double false_east, double false_north);
	long wivfor(double lon, double lat, double *x, double *y);
	long wivinvint(double r, double center_long, double false_east, double false_north);
	long wivinv(double x, double y, double *lon, double *lat);
	// WagnerVIIProjection
	long wviiforint(double r, double center_long, double false_east, double false_north);
	long wviifor(double lon, double lat, double *x, double *y);
	long wviiinvint( double r, double center_long, double false_east, double false_north);
	long wviiinv(double x, double y, double *lon, double *lat);
	};	// class Projectoid

/***
 ***  The rest of this file is purely for reference purposes (things WILL go wrong, eh?)
 ***

// AlaskaConformalProjection
	// forward & inverse vars
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_center;		// center latitude
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	static double acoef[7];
	static double bcoef[7];
	static double sin_p26;
	static double cos_p26;
	static double e;
	static long n;

// AlbersConEqAreaProjection
	{
	// forward vars
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double c;				// constant c
	static double e3;				// eccentricity
	static double rh;				// heigth above elipsoid
	static double ns0;				// ratio between meridians
	static double lon_center;		// center longitude
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	// inverse vars - above plus this
	static double es;				// eccentricity squared
	}; // AlbersProjection

// AzimuthalEqProjection
	{
	// forward vars
	static double r_major;			// major axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e0,e1,e2,e3;		// eccentricity constants
	static double e,es,esp; 		// eccentricity constants
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double sin_p12;			// sin of center latitude
	static double cos_p12;			// cos of center latitude
	// inverse vars
	static double r_major;			// major axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double sin_p12;			// sin of center latitude
	static double cos_p12;			// cos of center latitude
	}; // AzimuthalEqProjection

// EquiConicProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e0,e1,e2,e3;		// eccentricity constants
	static double e,es,esp; 		// eccentricity constants
	static double ml0;				// small value m
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double ns;
	static double g;
	static double rh;
	}; // EquiConicProjection

// EquirectangularProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	}; // EquirectangularProjection

// GeneralVertNearSideProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double lat_center;		// Center latitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double p;				// Height above sphere
	static double sin_p15;			// Sine of the center latitude
	static double cos_p15;			// Cosine of the center latitude
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // GeneralVertNearSideProjection

// GeographicProjection
	{
	}; // GeographicProjection

// GnomonicProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double lat_center;		// Center latitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double sin_p13;			// Sine of the center latitude
	static double cos_p13;			// Cosine of the center latitude
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // GnomonicProjection

// HammerProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // HammerProjection

// HotineOblMercProjection
	{
	// forward
	static double azimuth;
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double scale_factor; 	// scale factor
	static double lon_origin;		// center longitude
	static double lat_origin;		// center latitude
	static double e,es; 			// eccentricity constants
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double sin_p20,cos_p20;	// sin and cos values
	static double bl;
	static double al;
	static double d;
	static double el,u;
	static double singam,cosgam;
	static double sinaz,cosaz;
	// inverse - above plus this
	static double ts;
	}; // HotineOblMercProjection

// IntGoodeHomolosineProjection
	{
	// forward & inverse
	static double R;				// Radius of the earth (sphere)
	static double lon_center[12];	// Central meridians, one for each region
	static double feast[12];		// False easting, one for each region
	}; // IntGoodeHomolosineProjection

// IntMollwiedeProjection
	{
	// forward & inverse
	static double R;				// Radius of the earth (sphere)
	static double lon_center[6];	// Central meridians, one for each region
	static double feast[6]; 		// False easting, one for each region
	}; // IntMollwiedeProjection

// LambertAzEqAreaProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double lat_center;		// Center latitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double sin_lat_o;		// Sine of the center latitude
	static double cos_lat_o;		// Cosine of the center latitude
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // LambertAzEqAreaProjection

// LambertCCProjection 
	{
	// forward & inverse
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double es;				// eccentricity squared
	static double e;				// eccentricity
	static double center_lon;		// center longituted
	static double center_lat;		// cetner latitude
	static double ns;				// ratio of angle between meridian
	static double f0;				// flattening of ellipsoid
	static double rh;				// height above ellipsoid
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // LambertCCProjection

// MercatorProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e,es; 			// eccentricity constants
	static double m1;				// small value m
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	}; // MercatorProjection

// MillerCylindricalProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // MillerCylindricalProjection

// MollweideProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // MollweideProjection

// OblEqAreaProjection
	// forward & inverse
	static double lon_center;
	static double lat_o;
	static double theta;
	static double m;
	static double n;
	static double R;
	static double sin_lat_o;
	static double cos_lat_o;
	static double false_easting;
	static double false_northing;

// OrthographicProjection
	// forward & inverse
	static double r_major;			// major axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double sin_p14;			// sin of center latitude
	static double cos_p14;			// cos of center latitude

// PolarStereographicProjection
	// forward
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double es;				// eccentricity squared
	static double e;				// eccentricity
	static double e4;				// e4 calculated from eccentricity
	static double center_lon;		// center longitude
	static double center_lat;		// center latitude
	static double fac;				// sign variable
	static double ind;				// flag variable
	static double mcs;				// small m
	static double tcs;				// small t
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	// inverse - above minus es
	}; // PolarStereographicProjection

// PolyconicProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e0,e1,e2,e3;		// eccentricity constants
	static double e,es,esp; 		// eccentricity constants
	static double ml0;				// small value m
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	}; // PolyconicProjection

// RobinsonProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	static double pr[21];
	static double xlr[21];
	}; // RobinsonProjection

// SinusoidalProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // SinusoidalProjection

// SpaceOblMercProjection
	{
	// forward
	static double lon_center,a,b,a2,a4,c1,c3,q,t,u,w,xj,p21,sa,ca,es,s,start;
	static double som_series();
	static double false_easting;
	static double false_northing;
	// inverse - above minus som_series()
	}; // SpaceOblMercProjection

// StatePlaneProjection
	{
	// forward & inverse
	static long id; 				// indicates which projection is to be transformed
	// set the initialized values for zone and spheroid.  This value determines
	// whether to initialize or not
	static long inzone = 0; 		// previous zone value
	static long insphere = -1;		// previous spheroid value
	// the State Plane Zones are set in these arrays
	static long NAD27[134] =
		{ 101, 102, 5010, 5300, 201, 202, 203, 301, 302, 401, 402, 403, 404,
		  405, 406, 407, 501, 502, 503, 600, 700, 901, 902, 903, 1001, 1002, 5101,
		  5102, 5103, 5104, 5105, 1101, 1102, 1103, 1201, 1202, 1301, 1302, 1401,
		  1402, 1501, 1502, 1601, 1602, 1701, 1702, 1703, 1801, 1802, 1900, 2001,
		  2002, 2101, 2102, 2103, 2111, 2112, 2113, 2201, 2202, 2203, 2301, 2302,
		  2401, 2402, 2403, 2501, 2502, 2503, 2601, 2602, 2701, 2702, 2703, 2800,
		  2900, 3001, 3002, 3003, 3101, 3102, 3103, 3104, 3200, 3301, 3302, 3401,
		  3402, 3501, 3502, 3601, 3602, 3701, 3702, 3800, 3901, 3902, 4001, 4002,
		  4100, 4201, 4202, 4203, 4204, 4205, 4301, 4302, 4303, 4400, 4501, 4502,
		  4601, 4602, 4701, 4702, 4801, 4802, 4803, 4901, 4902, 4903, 4904, 5001,
		  5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5201, 5202, 5400 };
	static long NAD83[134] =
		{ 101, 102, 5010, 5300, 201, 202, 203, 301, 302, 401, 402, 403,
		  404, 405, 406, 0000, 501, 502, 503, 600, 700, 901, 902, 903, 1001, 1002,
		  5101, 5102, 5103, 5104, 5105, 1101, 1102, 1103, 1201, 1202, 1301, 1302,
		  1401, 1402, 1501, 1502, 1601, 1602, 1701, 1702, 1703, 1801, 1802, 1900,
		  2001, 2002, 2101, 2102, 2103, 2111, 2112, 2113, 2201, 2202, 2203, 2301,
		  2302, 2401, 2402, 2403, 2500, 0000, 0000, 2600, 0000, 2701, 2702, 2703,
		  2800, 2900, 3001, 3002, 3003, 3101, 3102, 3103, 3104, 3200, 3301, 3302,
		  3401, 3402, 3501, 3502, 3601, 3602, 3701, 3702, 3800, 3900, 0000, 4001,
		  4002, 4100, 4201, 4202, 4203, 4204, 4205, 4301, 4302, 4303, 4400, 4501,
		  4502, 4601, 4602, 4701, 4702, 4801, 4802, 4803, 4901, 4902, 4903, 4904,
		  5001, 5002, 5003, 5004, 5005, 5006, 5007, 5008, 5009, 5200, 0000, 5400 };
	}; // StatePlaneProjection

// StereographicProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double sin_p10;			// sin of center latitude
	static double cos_p10;			// cos of center latitude
	}; // StereographicProjection

// TransverseMercatorProjection
	{
	// forward & inverse
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double scale_factor; 	// scale factor
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e0,e1,e2,e3;		// eccentricity constants
	static double e,es,esp; 		// eccentricity constants
	static double ml0;				// small value m
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double ind;				// spherical flag
	}; // TransverseMercatorProjection

// UserDefProjection
	{
	}; // UserDefProjection

// UTMProjection
	{
	// forward
	static double r_major;			// major axis
	static double r_minor;			// minor axis
	static double scale_factor; 	// scale factor
	static double lon_center;		// Center longitude (projection center)
	static double lat_origin;		// center latitude
	static double e0,e1,e2,e3;		// eccentricity constants
	static double e,es,esp; 		// eccentricity constants
	static double ml0;				// small value m
	static double false_northing;	// y offset in meters
	static double false_easting;	// x offset in meters
	static double ind;				// spherical flag
	// inverse
	static long ind;				// sphere flag value
	}; // UTMProjection

// VanDerGrintenProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset in meters
	static double false_northing;	// y offset in meters
	}; // VanDerGrintenProjection

// WagnerIVProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset
	static double false_northing;	// y offset
	}; // WagnerIVProjection

// WagnerVIIProjection
	{
	// forward & inverse
	static double lon_center;		// Center longitude (projection center)
	static double R;				// Radius of the earth (sphere)
	static double false_easting;	// x offset
	static double false_northing;	// y offset
	}; // WagnerVIIProjection

***/
