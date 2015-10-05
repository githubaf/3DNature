// UsefulUnit.cpp
// Unit conversion code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulUnit.h"
#include "UsefulPathString.h"

// Multiply meters by these factors to convert to the specified unit.
// These are in the same order as the enum in Useful.h
// Items with question marks have been calculated but not verified.
// Items that are negative are stored as inverse (1/x) for easier storage and better precision
double UsefulUnitMeterFactors[] =
	{
	// SI Units
	1e13,		// WCS_USEFUL_UNIT_XUNIT
	1e10,		// WCS_USEFUL_UNIT_ANGSTROM
	1e9,		// WCS_USEFUL_UNIT_MILLIMICRON
	1e6,		// WCS_USEFUL_UNIT_MICRON
	1000,		// WCS_USEFUL_UNIT_MILLIMETER
	100,		// WCS_USEFUL_UNIT_CENTIMETER
	10,			// WCS_USEFUL_UNIT_DECIMETER
	1,			// WCS_USEFUL_UNIT_METER
	0.001,		// WCS_USEFUL_UNIT_KILOMETER
	1e-6,		// WCS_USEFUL_UNIT_MEGAMETER

	// English units
//#ifdef WCS_BUILD_VNS
	-.0254,		// WCS_USEFUL_UNIT_INCH (1" = 2.54 cm)
	-.3048,		// WCS_USEFUL_UNIT_FEET (3.280839895)
	-.9144,		// WCS_USEFUL_UNIT_YARD (1.0936)		made consistent with feet and inches 12/27/00 by GRH
	-5.0292,	// WCS_USEFUL_UNIT_ROD		made consistent with feet and inches 12/27/00 by GRH
	-1609.344,	// WCS_USEFUL_UNIT_MILE_US_STATUTE (.0006213699953)		made consistent with feet and inches 12/27/00 by GRH
/***
#else // WCS_BUILD_VNS
	-.025400048,// WCS_USEFUL_UNIT_INCH (39.3701)
	-.30480058,	// WCS_USEFUL_UNIT_FEET (3.280833652)
	-.91440174,	// WCS_USEFUL_UNIT_YARD (1.0936)
	0.198838403,// WCS_USEFUL_UNIT_ROD		made consistent with feet and inches 12/27/00 by GRH
	-1609.3471,	// WCS_USEFUL_UNIT_MILE_US_STATUTE (.0006213699953)
#endif // WCS_BUILD_VNS
***/
	-.304800609601219,	// WCS_USEFUL_UNIT_FEET_US_SURVEY (NOT THE COMMON FOOT) {1m = 39.37"}

	// More exotic units
	0.00497,	// WCS_USEFUL_UNIT_FURLONG
	0.04971,	// WCS_USEFUL_UNIT_CHAIN_GUNTER
	0.03281,	// WCS_USEFUL_UNIT_CHAIN_RAMDEN
	4.97097,	// WCS_USEFUL_UNIT_LINK_GUNTER
	3.28084,	// WCS_USEFUL_UNIT_LINK_RAMDEN
	0.54681,	// WCS_USEFUL_UNIT_FATHOM
	0.0005396,	// WCS_USEFUL_UNIT_MILE_NAUT_BRIT
	-1852,		// WCS_USEFUL_UNIT_MILE_NAUT_INT
	-5559.5,	// WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT
	-5556,		// WCS_USEFUL_UNIT_LEAGUE_NAUT_INT
	-4828,		// WCS_USEFUL_UNIT_LEAGUE
	39310.08,	// WCS_USEFUL_UNIT_MIL
	-6.096,		// WCS_USEFUL_UNIT_ROPE
	-.4572,		// WCS_USEFUL_UNIT_CUBIT
	// according to http://physics.nist.gov/cuu/Units/outside.html
	// The astronomical unit is a unit of length. Its value is such that, when used to describe the
	// motion of bodies in the solar system, the heliocentric gravitation constant is
	// (0.017 202 098 95)2 ua3·d-2. The value must be obtained by experiment, and is therefore not known exactly. 
	-1.49598e11,	// WCS_USEFUL_UNIT_AU
	-9.46073e15,// WCS_USEFUL_UNIT_LIGHTYEAR
	-3.08568e16	// WCS_USEFUL_UNIT_PARSEC
	};

char UsefulUnitTextSuffixes[33][10] =
	{
	// SI Units
	"xu",		// WCS_USEFUL_UNIT_XUNIT
	"A",		// WCS_USEFUL_UNIT_ANGSTROM
	"mu",		// WCS_USEFUL_UNIT_MILLIMICRON
	"u",		// WCS_USEFUL_UNIT_MICRON
	"mm",		// WCS_USEFUL_UNIT_MILLIMETER
	"cm",		// WCS_USEFUL_UNIT_CENTIMETER
	"dm",		// WCS_USEFUL_UNIT_DECIMETER
	"m",		// WCS_USEFUL_UNIT_METER
	"Km",		// WCS_USEFUL_UNIT_KILOMETER
	"Mm",		// WCS_USEFUL_UNIT_MEGAMETER

	// English units
	"in",		// WCS_USEFUL_UNIT_INCH
	"ft",		// WCS_USEFUL_UNIT_FEET
	"yd",		// WCS_USEFUL_UNIT_YARD
	"rd",		// WCS_USEFUL_UNIT_ROD
	"mi",		// WCS_USEFUL_UNIT_MILE_US_STATUTE
	"ft_uss",	// WCS_USEFUL_UNIT_FEET_US_SURVEY (NOT THE COMMON FOOT)

	// More exotic units
	"fur",		// WCS_USEFUL_UNIT_FURLONG
	"ch_g",		// WCS_USEFUL_UNIT_CHAIN_GUNTER
	"ch_r",		// WCS_USEFUL_UNIT_CHAIN_RAMDEN
	"li_g",		// WCS_USEFUL_UNIT_LINK_GUNTER
	"li_r",		// WCS_USEFUL_UNIT_LINK_RAMDEN
	"fath",		// WCS_USEFUL_UNIT_FATHOM
	"naut_b",	// WCS_USEFUL_UNIT_MILE_NAUT_BRIT
	"naut",		// WCS_USEFUL_UNIT_MILE_NAUT_INT
	"lg_b",		// WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT
	"lg_i",		// WCS_USEFUL_UNIT_LEAGUE_NAUT_INT
	"lg",		// WCS_USEFUL_UNIT_LEAGUE
	"mil_",		// WCS_USEFUL_UNIT_MIL
	"rop",		// WCS_USEFUL_UNIT_ROPE
	"cb",		// WCS_USEFUL_UNIT_CUBIT
	"AU",		// WCS_USEFUL_UNIT_AU
	"ly",		// WCS_USEFUL_UNIT_LIGHTYEAR
	"pc"		// WCS_USEFUL_UNIT_PARSEC
	};

char UsefulUnitTextNames[33][25] =
	{
	// SI Units
	"X Unit",			// WCS_USEFUL_UNIT_XUNIT
	"Angstrom",			// WCS_USEFUL_UNIT_ANGSTROM
	"Millimicron",		// WCS_USEFUL_UNIT_MILLIMICRON
	"Micron",			// WCS_USEFUL_UNIT_MICRON
	"Millimeter",		// WCS_USEFUL_UNIT_MILLIMETER
	"Centimeter",		// WCS_USEFUL_UNIT_CENTIMETER
	"Decimeter",		// WCS_USEFUL_UNIT_DECIMETER
	"Meter",			// WCS_USEFUL_UNIT_METER
	"Kilometer",		// WCS_USEFUL_UNIT_KILOMETER
	"Megameter",		// WCS_USEFUL_UNIT_MEGAMETER

	// English units
	"Inch",		// WCS_USEFUL_UNIT_INCH
	"Foot",		// WCS_USEFUL_UNIT_FEET
	"Yard",		// WCS_USEFUL_UNIT_YARD
	"Rod",		// WCS_USEFUL_UNIT_ROD
	"Mile",		// WCS_USEFUL_UNIT_MILE_US_STATUTE
	"Foot (US Survey)",		// WCS_USEFUL_UNIT_FEET_US_SURVEY (NOT THE COMMON FOOT)

	// More exotic units
	"Furlong",				// WCS_USEFUL_UNIT_FURLONG
	"Chain (Gunter)",		// WCS_USEFUL_UNIT_CHAIN_GUNTER
	"Chain (Ramden)",		// WCS_USEFUL_UNIT_CHAIN_RAMDEN
	"Link (Gunter)",		// WCS_USEFUL_UNIT_LINK_GUNTER
	"Link (Ramden)",		// WCS_USEFUL_UNIT_LINK_RAMDEN
	"Fathom",				// WCS_USEFUL_UNIT_FATHOM
	"Nautical Mile (Brit)",	// WCS_USEFUL_UNIT_MILE_NAUT_BRIT
	"Nautical Mile (Int)",	// WCS_USEFUL_UNIT_MILE_NAUT_INT
	"League (Naut Brit)",	// WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT
	"League (Naut Int)",	// WCS_USEFUL_UNIT_LEAGUE_NAUT_INT
	"League",				// WCS_USEFUL_UNIT_LEAGUE
	"Mil",					// WCS_USEFUL_UNIT_MIL
	"Rope",					// WCS_USEFUL_UNIT_ROPE
	"Cubit",				// WCS_USEFUL_UNIT_CUBIT
	"Astronomical Unit",	// WCS_USEFUL_UNIT_AU
	"Lightyear",			// WCS_USEFUL_UNIT_LIGHTYEAR
	"Parsec"				// WCS_USEFUL_UNIT_PARSEC
	};

int UsefulUnitUpConvert[] =
	{
	// SI Units
	WCS_USEFUL_UNIT_ANGSTROM,			// WCS_USEFUL_UNIT_XUNIT
	WCS_USEFUL_UNIT_MILLIMICRON,		// WCS_USEFUL_UNIT_ANGSTROM
	WCS_USEFUL_UNIT_MICRON,				// WCS_USEFUL_UNIT_MILLIMICRON
	WCS_USEFUL_UNIT_MILLIMETER,			// WCS_USEFUL_UNIT_MICRON
	WCS_USEFUL_UNIT_METER,				// WCS_USEFUL_UNIT_MILLIMETER
	WCS_USEFUL_UNIT_METER,				// WCS_USEFUL_UNIT_CENTIMETER
	WCS_USEFUL_UNIT_METER,				// WCS_USEFUL_UNIT_DECIMETER
	WCS_USEFUL_UNIT_KILOMETER,			// WCS_USEFUL_UNIT_METER
	WCS_USEFUL_UNIT_MEGAMETER,			// WCS_USEFUL_UNIT_KILOMETER
	WCS_USEFUL_UNIT_AU,					// WCS_USEFUL_UNIT_MEGAMETER

	// English units
	WCS_USEFUL_UNIT_FEET,				// WCS_USEFUL_UNIT_INCH
	WCS_USEFUL_UNIT_MILE_US_STATUTE,	// WCS_USEFUL_UNIT_FEET
	WCS_USEFUL_UNIT_MILE_US_STATUTE,	// WCS_USEFUL_UNIT_YARD
	WCS_USEFUL_UNIT_MILE_US_STATUTE,	// WCS_USEFUL_UNIT_ROD
	WCS_USEFUL_UNIT_AU,					// WCS_USEFUL_UNIT_MILE_US_STATUTE
	// This does not (currently) upconvert
	WCS_USEFUL_UNIT_FEET_US_SURVEY,		// WCS_USEFUL_UNIT_FEET_US_SURVEY (NOT THE COMMON FOOT)

	// More exotic units
	// These cannot usefully be upconverted
	WCS_USEFUL_UNIT_FURLONG,			// WCS_USEFUL_UNIT_FURLONG
	WCS_USEFUL_UNIT_CHAIN_GUNTER,		// WCS_USEFUL_UNIT_CHAIN_GUNTER
	WCS_USEFUL_UNIT_CHAIN_RAMDEN,		// WCS_USEFUL_UNIT_CHAIN_RAMDEN
	WCS_USEFUL_UNIT_LINK_GUNTER,		// WCS_USEFUL_UNIT_LINK_GUNTER
	WCS_USEFUL_UNIT_LINK_RAMDEN,		// WCS_USEFUL_UNIT_LINK_RAMDEN
	WCS_USEFUL_UNIT_FATHOM,				// WCS_USEFUL_UNIT_FATHOM
	WCS_USEFUL_UNIT_MILE_NAUT_BRIT,		// WCS_USEFUL_UNIT_MILE_NAUT_BRIT
	WCS_USEFUL_UNIT_MILE_NAUT_INT,		// WCS_USEFUL_UNIT_MILE_NAUT_INT
	WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT,	// WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT
	WCS_USEFUL_UNIT_LEAGUE_NAUT_INT,	// WCS_USEFUL_UNIT_LEAGUE_NAUT_INT
	WCS_USEFUL_UNIT_LEAGUE,				// WCS_USEFUL_UNIT_LEAGUE
	WCS_USEFUL_UNIT_MIL,				// WCS_USEFUL_UNIT_MIL
	WCS_USEFUL_UNIT_ROPE,				// WCS_USEFUL_UNIT_ROPE
	WCS_USEFUL_UNIT_CUBIT,				// WCS_USEFUL_UNIT_CUBIT
	WCS_USEFUL_UNIT_PARSEC,				// WCS_USEFUL_UNIT_AU
	WCS_USEFUL_UNIT_LIGHTYEAR,			// WCS_USEFUL_UNIT_LIGHTYEAR
	WCS_USEFUL_UNIT_LIGHTYEAR			// WCS_USEFUL_UNIT_PARSEC
	};

int UsefulUnitDownConvert[] =
	{
	// SI Units
	WCS_USEFUL_UNIT_XUNIT,				// WCS_USEFUL_UNIT_XUNIT
	WCS_USEFUL_UNIT_XUNIT,				// WCS_USEFUL_UNIT_ANGSTROM
	WCS_USEFUL_UNIT_ANGSTROM,			// WCS_USEFUL_UNIT_MILLIMICRON
	WCS_USEFUL_UNIT_MILLIMICRON,		// WCS_USEFUL_UNIT_MICRON
	WCS_USEFUL_UNIT_MICRON,				// WCS_USEFUL_UNIT_MILLIMETER
	WCS_USEFUL_UNIT_MILLIMETER,			// WCS_USEFUL_UNIT_CENTIMETER
	WCS_USEFUL_UNIT_MILLIMETER,			// WCS_USEFUL_UNIT_DECIMETER
	WCS_USEFUL_UNIT_CENTIMETER,			// WCS_USEFUL_UNIT_METER
	WCS_USEFUL_UNIT_METER,				// WCS_USEFUL_UNIT_KILOMETER
	WCS_USEFUL_UNIT_KILOMETER,			// WCS_USEFUL_UNIT_MEGAMETER

	// English units
	WCS_USEFUL_UNIT_INCH,				// WCS_USEFUL_UNIT_INCH
	WCS_USEFUL_UNIT_INCH,				// WCS_USEFUL_UNIT_FEET
	WCS_USEFUL_UNIT_FEET,				// WCS_USEFUL_UNIT_YARD
	WCS_USEFUL_UNIT_FEET,				// WCS_USEFUL_UNIT_ROD
	WCS_USEFUL_UNIT_FEET,				// WCS_USEFUL_UNIT_MILE_US_STATUTE
	// This does not (currently) upconvert
	WCS_USEFUL_UNIT_FEET_US_SURVEY,		// WCS_USEFUL_UNIT_FEET_US_SURVEY (NOT THE COMMON FOOT)

	// More exotic units
	// These cannot usefully be upconverted
	WCS_USEFUL_UNIT_FURLONG,			// WCS_USEFUL_UNIT_FURLONG
	WCS_USEFUL_UNIT_CHAIN_GUNTER,		// WCS_USEFUL_UNIT_CHAIN_GUNTER
	WCS_USEFUL_UNIT_CHAIN_RAMDEN,		// WCS_USEFUL_UNIT_CHAIN_RAMDEN
	WCS_USEFUL_UNIT_LINK_GUNTER,		// WCS_USEFUL_UNIT_LINK_GUNTER
	WCS_USEFUL_UNIT_LINK_RAMDEN,		// WCS_USEFUL_UNIT_LINK_RAMDEN
	WCS_USEFUL_UNIT_FATHOM,				// WCS_USEFUL_UNIT_FATHOM
	WCS_USEFUL_UNIT_MILE_NAUT_BRIT,		// WCS_USEFUL_UNIT_MILE_NAUT_BRIT
	WCS_USEFUL_UNIT_MILE_NAUT_INT,		// WCS_USEFUL_UNIT_MILE_NAUT_INT
	WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT,	// WCS_USEFUL_UNIT_LEAGUE_NAUT_BRIT
	WCS_USEFUL_UNIT_LEAGUE_NAUT_INT,	// WCS_USEFUL_UNIT_LEAGUE_NAUT_INT
	WCS_USEFUL_UNIT_LEAGUE,				// WCS_USEFUL_UNIT_LEAGUE
	WCS_USEFUL_UNIT_MIL,				// WCS_USEFUL_UNIT_MIL
	WCS_USEFUL_UNIT_ROPE,				// WCS_USEFUL_UNIT_ROPE
	WCS_USEFUL_UNIT_CUBIT,				// WCS_USEFUL_UNIT_CUBIT
	WCS_USEFUL_UNIT_AU,					// WCS_USEFUL_UNIT_AU
	WCS_USEFUL_UNIT_LIGHTYEAR,			// WCS_USEFUL_UNIT_LIGHTYEAR
	WCS_USEFUL_UNIT_PARSEC				// WCS_USEFUL_UNIT_PARSEC
	};

int GetUpconvertUnit(int Unit) {return(UsefulUnitUpConvert[Unit]);};

/*===========================================================================*/

int GetDownconvertUnit(int Unit) {return(UsefulUnitDownConvert[Unit]);};

/*===========================================================================*/

char *GetUnitName(int Unit) {return(UsefulUnitTextNames[Unit]);};

/*===========================================================================*/

char *GetUnitSuffix(int Unit) {return(UsefulUnitTextSuffixes[Unit]);};

/*===========================================================================*/

int MatchUnitSuffix(char *Suffix)
{
int Matched = -1, Search;

// try case-sensitive match first
for (Search = 0; Search < WCS_USEFUL_UNIT_MAXIMUM; ++Search)
	{
	if (strcmp(Suffix, UsefulUnitTextSuffixes[Search]) == 0)
		{ // found a match
		Matched = Search;
		return(Matched);
		} // if
	} // for

for (Search = 0; Search < WCS_USEFUL_UNIT_MAXIMUM; ++Search)
	{
	if (stricmp(Suffix, UsefulUnitTextSuffixes[Search]) == 0)
		{ // found a match
		Matched = Search;
		return(Matched);
		} // if
	} // for

// Didn't get a complete match, try single-letter short-forms
if (Suffix[1] == NULL)
	{
	switch(tolower((unsigned char)Suffix[0]))
		{
		case 'i': return(WCS_USEFUL_UNIT_INCH);
		case 'f': return(WCS_USEFUL_UNIT_FEET);
		case 'y': return(WCS_USEFUL_UNIT_YARD);
		case 'k': return(WCS_USEFUL_UNIT_KILOMETER);
		} // switch
	} // if

return(Matched);

} // MatchUnitSuffix

/*===========================================================================*/

static char UsefulNormalizeUnitBuf[50];
int GetNormalizedUnit(int Unit, double Meters)
{
double Converted;
int PreferredUnit, TestUnit, NewTestUnit;
int Digits, UpConvDigits, ConvertOption, ConvertTrend = 0;
char Done = 0;

TestUnit = Unit;
PreferredUnit = Unit; // in case of failure, we are at least initialized

while (!Done)
	{
	ConvertOption = 0;
	// Convert to Prefs Display Units
	Converted = ConvertFromMeters(Meters, TestUnit);
	sprintf(UsefulNormalizeUnitBuf, WCSW_FP_CONVERT_STRING, Converted);
	TrimZeros(UsefulNormalizeUnitBuf);

	Digits = CountIntDigits(UsefulNormalizeUnitBuf);
	// ConvertTrend prevents us from trying a DownConvert once
	// we've begun upconverting. Should prevent unit bounce.
	// Upconvert?

	// Distances in Feet are rational up to about 100,000ft, so we
	// will refrain from upconverting until we exceed 5 digits.
	if ((TestUnit == WCS_USEFUL_UNIT_FEET) || (TestUnit == WCS_USEFUL_UNIT_FEET_US_SURVEY))
		{
		UpConvDigits = 5;
		} // if
	else
		{
		UpConvDigits = 4;
		} // else

	if ((ConvertTrend != -1) && (Digits > UpConvDigits))
		{
		ConvertTrend = ConvertOption = 1;
		} // if
	// DownConvert?
	else if ((ConvertTrend != 1) && (Converted < 1.0))
		{
		Digits = CountDecDigits(UsefulNormalizeUnitBuf);
		if (Digits > 4)
			{
			ConvertTrend = ConvertOption = -1;
			} // if
		} // else

	if (ConvertOption == 0)
		{
		PreferredUnit = TestUnit;
		Done = 1;
		} // if
	else
		{
		if (ConvertOption == 1)
			{
			NewTestUnit = GetUpconvertUnit(TestUnit);
			} // if
		else if (ConvertOption == -1)
			{
			NewTestUnit = GetDownconvertUnit(TestUnit);
			} // if
		} // else
	if (!Done)
		{
		if (NewTestUnit == TestUnit)
			{
			PreferredUnit = TestUnit;
			Done = 1; // can't get anywhere from here
			} // if
		else
			{
			TestUnit = NewTestUnit;
			} // else
		} // if
	} // while

return(PreferredUnit);

} // GetNormalizedUnit

/*===========================================================================*/

double ConvertToMeters(double OrigVal, int FromUnit)
{
double Factor = 1.0;

if (FromUnit < WCS_USEFUL_UNIT_MAXIMUM)
	{
	Factor = UsefulUnitMeterFactors[FromUnit];
	} // if

if (Factor == 1.0) return(OrigVal);
// Do we need to invert factor while we convert?
if (Factor <= 0)
	{
	return(-Factor * OrigVal);
	} // if
else
	{
	return(OrigVal / Factor);
	} // else

} // ConvertToMeters

/*===========================================================================*/

double ConvertFromMeters(double MeterVal, int ToUnit)
{
double Factor = 1.0;

if (ToUnit < WCS_USEFUL_UNIT_MAXIMUM)
	{
	Factor = UsefulUnitMeterFactors[ToUnit];
	} // if

// Do we need to invert factor while we convert?
if (Factor == 1.0) return(MeterVal);
if (Factor >= 0)
	{
	return(Factor * MeterVal);
	} // if
else
	{
	return(MeterVal / -Factor);
	} // else

} // ConvertFromMeters
