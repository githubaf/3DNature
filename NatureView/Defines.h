/* Defines.h
** 
** Stolen and trimmed from WCS V1 Defines.h
**
** Chock full of delicious, nutritious #define statements that probably
** are bad for you. (And your code.)
** Built from map.h and gis.h and gisam.c and gis.c and all that
** on 24 Jul 1993 by Chris "Xenon" Hanson.
** Original code and subsequent perturbations by Gary R. Huber.
*/

#ifndef WCS_DEFINES_H
#define WCS_DEFINES_H

enum
	{
	WCS_X,
	WCS_Y,
	WCS_Z,
	WCS_W
	}; // 

enum
	{
	WCS_R,
	WCS_G,
	WCS_B,
	WCS_A
	}; //

#ifdef __SASC_60
	#define AMIGA_GUI
	/* #define ROUNDING */
#else /* __SASC_60 */
	#ifdef LATTICE_50
		#define AMIGA_GUI
		#define SAS_5
		#define ROUNDING
	#else /* !LATTICE_50 */
		/* #define AMIGA_GUI */
		/* #define ROUNDING */
	#endif /* LATTICE_50 */
#endif /* __SASC_60 */

#ifdef ROUNDING 
 #define ROUNDING_KLUDGE -.5
#else
 #define ROUNDING_KLUDGE 0
#endif /* ifdef ROUNDING */

#ifdef SAS_5
 #define FGETS_KLUDGE 0
#else
 #define FGETS_KLUDGE 1
#endif /* ifdef LATTICE_50 */


// Geographic and other math defines

// We now supply LatScale() in Useful.cpp to calculate your
// Km-per-one-degree-Latitude values for any radius of sphere/planet.
#define EARTHLATSCALE 111.049771	/* km, miles = 69.003 we're goin' metric! */
#define EARTHLATSCALE_METERS 111049.771	/* meters of course */
#define EARTHRAD 6362.683195	/* km, miles = 3953.580673 we done gone metric! */
#define Pi     3.1415926535898
#define HalfPi 1.5707963267949
#define TwoPi  6.2831853071796
#define OneAndHalfPi 4.7123889803847
#define PiOver180 1.74532925199433E-002
#define PiUnder180 5.72957795130823E+001

// Picked up from V1 Fractal.c. Seemed useful to have around.
#define FRAMES_PER_HOUR		108000.0


// Used in V1 MapUtil.c, V2 RenderUtil.cpp
#define PROJPLANE 1000

// Used in V2 RenderGlobe.cpp, globemap() and probably others
#define HORSCALEFACTOR 90.219

#define ELSCALE_KILOM	 1.0
#define ELSCALE_METERS	 1.0E-003
#define ELSCALE_DECIM	 1.0E-004
#define ELSCALE_CENTIM	 1.0E-005
#define ELSCALE_MILLIM	 1.0E-006
#define ELSCALE_MILES	 1.6093471
#define ELSCALE_YARDS	 9.1440176E-004
#define ELSCALE_FEET	 3.0480058E-004
#define ELSCALE_INCHES	 2.5400048E-005
#define FEET_PER_METER	 3.28083451			// don't know about the relevant precision here.
											// This is consistent with the values for feet and kilometers above.

#define ELEVHDRLENV101 64

/* colors for illuminated objects */
#define ILLUMVECRED	235
#define ILLUMVECGRN	235
#define ILLUMVECBLU	210

#define LARGENUM 32000		/* 10E+12 */

#define DEM_CURRENT_VERSION	1.02

#ifdef _WIN32
#ifdef _MAC
#define WCS_SYMBOL_DEGREE		"\xa1"
#define WCS_SYMBOL_DEGREE_CHAR	0xa1
#else // !_MAC
#define WCS_SYMBOL_DEGREE		"\xb0"
#define WCS_SYMBOL_DEGREE_CHAR	0xb0
#endif // !_MAC
#endif // _WIN32
#ifdef AMIGA
//<<<A>>>
#define WCS_SYMBOL_DEGREE		"\xb0"
#define WCS_SYMBOL_DEGREE_CHAR	0xb0
#endif // AMIGA

// I weeded out a bunch of old obsolete #defines from V1 right here. 10/14/96 -CXH

#endif /* WCS_DEFINES_H */
