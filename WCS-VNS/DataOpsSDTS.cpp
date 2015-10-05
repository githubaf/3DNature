// DataOpsSDTS.cpp
// Code for SDTS files
// Adapted from DEMExtractGUI.cpp on 10/15/99 by Frank Weed II
// Copyright 1999 3D Nature
// SDTS routines adapated from Bob Lazar (sdtsdump.c) & Sol Katz (sdts2dem.c) by FPW2

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Projection.h"
#include "Database.h"
#include "AppMem.h"
#include "Project.h"
#include "Joe.h"
#include "stc123.h"
#include "DEM.h"
#include "Log.h"
#include "Notify.h"

#ifdef WCS_BUILD_VNS
extern NotifyTag DBLoadEvent[];
extern NotifyTag DBPreLoadEvent[];
#endif // WCS_BUILD_VNS

// this will save out the untiled elev file
#define SDTS_UNTILED

//#define STDSDEM_DEBUG

#ifdef DEBUG	// never allow in release build!
// used for debugging
//#define ASCIIDUMP
// enable timing tests
//#define TIMING
#endif

#ifndef SDTS_UNTILED
extern short DupRow;
#endif // SDTS_UNTILED
static bool HosedData, MemErr;

/*===========================================================================*/

// names of files that hold these pieces of the transfer (always 12 chars)
static char Ident[16];				// ****IDEN.DDF
static char InternalSR[16];			// ****IREF.DDF
static char ExternalSR[16];			// ****XREF.DDF
static char SpatialDomain[16];		// ****SPDM.DDF
static char Lineage[16];			// ****DQHL.DDF
static char PositionAccy[16];		// ****DQPA.DDF
static char AttributeAccy[16];		// ****DQAA.DDF
static char LogicalConsist[16];		// ****DQLC.DDF
static char Completeness[16];		// ****DQCG.DDF
static char DataDictDef[16];		// ****DDDF.DDF
static char DataDictDomain[16];		// ****DDOM.DDF
static char DataDictSchema[16];		// ****DDSH.DDF
static char RasterDef[16];			// ****RSDF.DDF
static char LayerDef[16];			// ****LDEF.DDF
static char SDTSCell[16];				// ****CE**.DDF (always L0 ???)
static char CatSpatialDomain[16];	// ****CATS.DDF
static char TransferStats[16];		// ****STAT.DDF
//static char CatDir[16];				// ****CATD.DDF (where the above names come from)

static char		ccs[4];
static char		DAID[100];						// what's reasonable?
static char		date[11];
#ifdef WCS_BUILD_VNS
static int		datum = 1;
#endif // WCS_BUILD_VNS
static char		descr[5000];					// what's reasonable?
static double	ElevFactor = ELSCALE_METERS;	// set default
static char		sdtsfname[256+32];				// what's reasonable?
static int		fillvalue = -32766;
static char		FloatElevs;
static FILE		*fpin;
static char		frmts[500];						// what's reasonable?
static char		have_l, have_x, have_y;		// corrupt gov't - what else is new?
static double	height;
static char		ice;
static long		int_level;
static long		izone;
static char		leadid;
static char		logmsg[256];
static long		ncols, nrows, nxy;
static int		order;
static int		recid;
static char		rsnm[5];
static long		sadr_x, sadr_y;
static long		scale;
static int		seq = 0;	//lint -e551
static double	sfax, sfay;
static int		stat2;	//lint -e551
static int		status;
//static char		str[80];
static long		str_len;
static char		stringy[5000];		// uh- yeah
static char		tag[10];
//static double	upperlx, upperly;
static int		voidvalue = -32767;
static double	width;
static double	x[5], xhrs, xorg, y[5], yhrs, yorg;
static struct	DEMExtractData SDTSDEMExtract;
static long		RasterRows, RasterCols;

/*===========================================================================*/

// This is the "correct" way to get the file names for all the files in the SDTS xfer
// returns true if successful
/***
   ACK!  A nasty side effect of this is that if the files are renamed manually (since adjacent DEM's
   sometimes have the same first four characters!), is that the original file names are still looked for!
***/
bool ImportWizGUI::SetFileNames(void)
{
bool success = false;
char rtype[4];

// Clear file names
Ident[0] = 0;
InternalSR[0] = 0;
ExternalSR[0] = 0;
SpatialDomain[0] = 0;
Lineage[0] = 0;
PositionAccy[0] = 0;
AttributeAccy[0] = 0;
LogicalConsist[0] = 0;
Completeness[0] = 0;
DataDictDef[0] = 0;
DataDictDomain[0] = 0;
DataDictSchema[0] = 0;
RasterDef[0] = 0;
LayerDef[0] = 0;
SDTSCell[0] = 0;
CatSpatialDomain[0] = 0;
TransferStats[0] = 0;

g123order(&order); // SDTS library needs byte order of this machine
strcpy(sdtsfname, GlobalApp->MainProj->MungPath(Importing->InDir));
strcpy(Importing->InDir, sdtsfname);
strcat(sdtsfname, Importing->InFile);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	sprintf(logmsg, "Can't open SDTS Catalog/Directory file %s!", sdtsfname);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
	goto done;
	}
if (! rd123ddrec
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	sprintf(logmsg, "\nERROR READING DATA DESCRIPTIVE RECORD in %s", sdtsfname);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
	goto done;
	}
status = -1;
/* Loop to process each subfield */
do
	{
	/* Read data record subfield */
	if (! rd123sfld
		(fpin,          /* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,         /* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		sprintf(logmsg, "\nERROR READING DATA RECORD SUBFIELD in %s", sdtsfname);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		goto done;
		}
	/* Retrieve description of current subfield */
	if (! chk123sfld
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */
		{
		sprintf(logmsg, "\nERROR CHECKING DATA RECORD SUBFIELD in %s", sdtsfname);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		goto done;
		}
	if (strnicmp(descr, "NAME", 4) == 0)
		strncpy(rtype, stringy, 4);
	else if (strnicmp(descr, "FILE", 4) == 0)
		{
		if (str_len != 12)
			{
			sprintf(logmsg, "\nCorrupt file name found in catalog (%s)", sdtsfname);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
			goto done;
			}
		if (rtype)
			{
//lint -save -e645
			if (strnicmp(rtype, "IDEN", 4) == 0)
				strncpy(Ident, stringy, 15);
			else if (strnicmp(rtype, "IREF", 4) == 0)
				strncpy(InternalSR, stringy, 15);
			else if (strnicmp(rtype, "XREF", 4) == 0)
				strncpy(ExternalSR, stringy, 15);
			else if (strnicmp(rtype, "SPDM", 4) == 0)
				strncpy(SpatialDomain, stringy, 15);
			else if (strnicmp(rtype, "DQHL", 4) == 0)
				strncpy(Lineage, stringy, 15);
			else if (strnicmp(rtype, "DQPA", 4) == 0)
				strncpy(PositionAccy, stringy, 15);
			else if (strnicmp(rtype, "DQAA", 4) == 0)
				strncpy(AttributeAccy, stringy, 15);
			else if (strnicmp(rtype, "DQLC", 4) == 0)
				strncpy(LogicalConsist, stringy, 15);
			else if (strnicmp(rtype, "DQCG", 4) == 0)
				strncpy(Completeness, stringy, 15);
			else if (strnicmp(rtype, "DDDF", 4) == 0)
				strncpy(DataDictDef, stringy, 15);
			else if (strnicmp(rtype, "DDOM", 4) == 0)
				strncpy(DataDictDomain, stringy, 15);
			else if (strnicmp(rtype, "DDSH", 4) == 0)
				strncpy(DataDictSchema, stringy, 15);
			else if (strnicmp(rtype, "RSDF", 4) == 0)
				strncpy(RasterDef, stringy, 15);
			else if (strnicmp(rtype, "LDEF", 4) == 0)
				strncpy(LayerDef, stringy, 15);
			else if (strnicmp(rtype, "CEL0", 4) == 0) // are there ever any other cells?
				strncpy(SDTSCell, stringy, 15);
			else if (strnicmp(rtype, "CATS", 4) == 0)
				strncpy(CatSpatialDomain, stringy, 15);
			else if (strnicmp(rtype, "STAT", 4) == 0)
				strncpy(TransferStats, stringy, 15);
//lint -restore
			} // if rtype
		} // else FILE
	} while (status != 4);   /* Break out of loop at end of file */
success = true;

done:
status = end123file(&fpin);
return success;

} // ImportWizGUI::SetFileNames

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_ElevUnits(void)
{

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, DataDictSchema);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	sprintf(logmsg, "%s", sdtsfname);
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, logmsg);
	return false;
    }
if (! rd123ddrec      
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
FloatElevs = false;
/*   Loop to process each subfield in Identification module            */
/*   default elevation to meters  */
do 
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          /* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,			/* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(IDEN MODULE)");
		stat2 = end123file(&fpin);
		return false;
		}
	//      Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	//    check subfield name and extract contents for each subfield
	if (!strcmp(tag, "DDSH") && !strcmp(descr, "UNIT"))
		{
		if		(!strcmp("FEET"  , stringy))
			{
			ElevFactor = ELSCALE_FEET / ELSCALE_METERS;
			Importing->ElevUnits = DEM_DATA_UNITS_FEET;
			}
		else if (!strcmp("METERS", stringy))
			{
			ElevFactor = 1.0;
			Importing->ElevUnits = DEM_DATA_UNITS_METERS;
			}
		}
	else if (!strcmp(tag, "DDSH") && !strcmp(descr, "PREC"))
		{
		ElevFactor *= atof(stringy);
		}
	else if (!strcmp(tag, "DDSH") && !strcmp(descr, "FMT"))
		{
		if (!strcmp(stringy, "BFP32"))
			FloatElevs = true;
		}
	} while (status != 4);   /* Break out of loop at end of file */
stat2 = end123file(&fpin);
Importing->VScale = ElevFactor;
return true;

} // ImportWizGUI::ReadSDTS_ElevUnits

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_Ident(void)
{
size_t namelen;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Ident);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
// Read Identification module data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	status = end123file(&fpin);
	return false;
	}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld    
	(fpin,          /* file pointer */
	tag,            /* field tag returned */
	&leadid,        /* leader identifier returned */
	stringy,         /* subfield contents returned */ 
	&str_len,       /* string length */
	&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(IDEN MODULE)");
		status = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		status = end123file(&fpin);
		return false;
		}
	// Display subfield name and contents for each subfield
	if (!strcmp(tag, "IDEN") && !strcmp(descr, "TITL")) /* DEM title */
		{
		if (Importing->TestOnly)
			{
			// copy DEM TITLE up to comma, 79 chars max
			namelen = strcspn(stringy, ",");
			if (namelen > 79)
				namelen = 79;
			strncpy(Importing->NameBase, stringy, namelen);
			Importing->NameBase[namelen] = 0;
			}
		}
	/*** F2 NOTE - The next line shouldn't be in here according to USGS SDTS data mapping guide ***/
	else if (!strcmp(tag, "IDEN") && !strcmp(descr, "SCAL"))
		scale = atol(stringy);
	else if (!strcmp (tag, "IDEN") && !strcmp(descr, "DAID"))
		strncpy(DAID, stringy, 99);
	else if (!strcmp (tag, "IDEN") && !strcmp(descr, "DCDT"))
		strncpy(date, stringy, 10);
	// else if (!strcmp (tag, "IDEN") && !strcmp (descr, "MPDT"))
	} while (status != 4);   // Break out of loop at end of file
// Close input Identification module
status = end123file(&fpin);
return true;

} // ImportWizGUI::ReadSDTS_Ident

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_CellRange(void)
{
double	maxvalue, minvalue;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, DataDictDomain);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
if (! rd123ddrec 
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
// Loop to process each subfield in CATS module
do
	{  
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          /* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,         /* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(AHDR MODULE)");
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */    
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "DDOM") && !strcmp(descr, "RCID"))
		{
		recid = atoi(stringy); 
		}      
	else if (recid == 1  && !strcmp(descr, "DVAL"))
		{
		voidvalue = atoi(stringy);
		}
	else if (recid == 2  && !strcmp(descr, "DVAL"))
		{
		fillvalue = atoi(stringy);
		}
	else if (recid == 3  && !strcmp(descr, "DVAL"))
		{
		minvalue = atof(stringy);
		Importing->TestMin = minvalue;
		}
	else if (recid == 4  && !strcmp(descr, "DVAL"))
		{
		maxvalue = atof(stringy);
		Importing->TestMax = maxvalue;
		}
	} while (status != 4);   // Break out of loop at end of file
stat2 = end123file (&fpin);
return true;

} // ImportWizGUI::ReadSDTS_CellRange

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_IRef(void)
{

/* set some default values */
sfax = 1.0;
sfay = 1.0;
xorg = 0.0;
yorg = 0.0;
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, InternalSR);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
if (! rd123ddrec      
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	}
status = -1;
// Loop to process each subfield in Identification module
do 
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          /* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,			/* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, "in IREF.");
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))			/* subfield format control */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "IREF") && !strcmp(descr, "SFAX"))
		sfax = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "SFAY"))
		sfay = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "XORG"))
		xorg = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "YORG"))
		yorg = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "XHRS"))
		{
		xhrs = atof(stringy);
		if (xhrs < minxhrs)
			minxhrs = xhrs;
		}
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "YHRS"))
		{
		yhrs = atof(stringy);
		if (yhrs < minyhrs)
			minyhrs = yhrs;
		}
	} while (status != 4);   // Break out of loop at end of file
width  = xhrs;	//lint -e551
height = yhrs;	//lint -e551
stat2 = end123file(&fpin);
return true;

} // ImportWizGUI::ReadSDTS_IRef

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_XRef(void)
{
#ifdef WCS_BUILD_VNS
short foo;
char cfoo[80];
#endif // WCS_BUILD_VNS

/* set some default values */
/* strcpy(rsnm, "??1"); */
/* strcpy(hdat,"??2"); */
/* strcpy(rdoc,"??3"); */
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, ExternalSR);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
if (! rd123ddrec 
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, "in XREF.");
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld  
		(fpin,			/* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,         /* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(IDEN MODULE)");
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld      
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
#ifdef STDSDEM_DEBUG
	sprintf(logmsg, "Tag = %s, Descr = %s\n, String = %s\n", tag, descr, stringy);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
#endif // STDSDEM_DEBUG
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "XREF") && !strcmp(descr, "RSNM"))
		{
		strncpy(rsnm, stringy, 4);
		if (strcmp(rsnm, "UTM") == 0)
			Importing->HasUTM = true;
		}
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "ZONE"))
		{
		izone = atoi(stringy);
		Importing->UTMZone = (short)izone;
		if (RUTMZone == 0)					// if the region zone is unset
			RUTMZone = Importing->UTMZone;	// use this zone
#ifdef WCS_BUILD_VNS
		UTMZoneNum2DBCode(&izone);
		Importing->IWCoordSys.SetZoneByCode(izone);
#endif // WCS_BUILD_VNS
		}
#ifdef WCS_BUILD_VNS
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "HDAT"))
		{ 
		if (!strcmp(stringy, "NAS" ))	// NAD 27
			{
			Importing->IWCoordSys.SetSystem("UTM - NAD 27");
			}
		else if (!strcmp(stringy, "WGC")) datum = 2 ;	// WGS 72
		else if (!strcmp(stringy, "WGE")) datum = 3 ;	// WGS 84
		else if (!strcmp(stringy, "NAX"))	// NAD 83
			{
			Importing->IWCoordSys.SetSystem("UTM - NAD 83");
			}
		else if (!strcmp(stringy, "OHD" )) datum = 5 ;	// Old Hawaii Datum
		else if (!strcmp(stringy, "PRD" )) datum = 6 ;	// Puerto Rico Datum
		}
	// we're doing absolutely nothing with these next two yet
	else if (!strcmp(tag, "VATT") && !strcmp(descr, "VDAT"))
		{ 
		// strcpy(vdat, stringy);
		if      (!strcmp(stringy, "LMSL" )) foo = 1 ;	// Local Means Sea Level
		else if (!strcmp(stringy, "NGVD" )) foo = 2 ;	// National Geodetic Vertical Datum 1929
		else if (!strcmp(stringy, "NAVD" )) foo = 3 ;	// North American Vertical Datum 1988
		}
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "COMT"))
		{ 
		strncpy(cfoo, stringy, 79);
		cfoo[79] = 0;
		}
//	else if (!strcmp(tag, "XREF") && !strcmp(descr, "RDOC"))     
//		strcpy(rdoc, stringy);
#endif // WCS_BUILD_VNS
	} while (status != 4);   /* Break out of loop at end of file */
stat2 = end123file(&fpin);
return true;

} // ImportWizGUI::ReadSDTS_XRef

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_RasterDef(void)
{

// I've found corrupt SDTS RSDF files, so this is the workaround
have_l = false;
have_x = false;
have_y = false;
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, RasterDef);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
if (! rd123ddrec      
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
// Loop to process each subfield in Raster Definition module
do 
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,			// subfield contents returned
		&str_len,       // string length
		&status))       // status returned
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(IDEN MODULE)");
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "SADR") && !strcmp(descr, "X"))		// raster y (northings)
		{   
		// Binary data, convert character string returned by rd123sfld to
		// a long integer, changing bit order if necessary
		if (strstr(frmts, "R") != NULL)
			{
			have_x = true;
			sadr_x = atol(stringy);
			}
		else if (strstr(frmts, "B") != NULL)
			{
			have_x = true;
			if (!order)
				s123tol(stringy, &sadr_x, 1);
			else
				s123tol(stringy, &sadr_x, 0);
			}
		}
	else if (!strcmp(tag, "SADR") && !strcmp(descr, "Y"))	// raster y (eastings)
		{
		// Binary data, convert character string returned by rd123sfld to a
		// long integer, changing bit order if necessary
		if (strstr(frmts, "R") != NULL)
			{
			have_y = true;
			sadr_y = atol(stringy);
			}
		else if (strstr(frmts, "B") != NULL)
			{
			have_y = true;
			if (!order)
				s123tol(stringy, &sadr_y, 1);
			else
				s123tol(stringy, &sadr_y, 0);
			}
		}
	else if (!strcmp(tag, "RSDF") && !strcmp(descr, "OBRP"))
		{
		have_l = true;
//		strcpy(Hdr->ElPattern, stringy);
		}
	else if (!strcmp(tag, "RSDF") && !strcmp(descr, "RWXT"))	// raster rows
		{
		if (strstr(frmts, "I") != NULL)
			{
			RasterRows = atol(stringy);
			}
		}
	else if (!strcmp(tag, "RSDF") && !strcmp(descr, "CLXT"))	// raster cols
		{
		if (strstr(frmts, "I") != NULL)
			{
			RasterCols = atol(stringy);
			}
		}
	if ((have_l) && (have_x) && (have_y))
		status = 4;		// We've got everything I need - fake an EOF status
	} while (status != 4);   // Break out of loop at end of file

stat2 = end123file(&fpin);

//upperlx = (sadr_x * sfax ) + xorg;
//upperly = (sadr_y * sfay ) + yorg;

// try to auto-correct base UTM coords if the raster northing or easting isn't in range, or if it has an incorrect modulo
// almost all SDTS DEM's prior to October 2001 are incorrect
if ((sadr_x < Importing->West) || (sadr_x > Importing->East) || ((sadr_x % (long)minxhrs) != 0) ||
	(sadr_y > Importing->North) || (sadr_y < Importing->South) || ((sadr_y % (long)minyhrs) != 0))
	{
	// see if we're west of the prime meridian
	if (Importing->West < 500000.0)
		{
		// round x up, round y down
		sadr_x = long(x[0] / minxhrs + 1.0) * long(minxhrs);	// SW corner is farthest west
		sadr_y = long(y[1] / minyhrs) * long(minyhrs);			// NW corner is farthest north
		}
	else
		{
		// round x up, round y down
		sadr_x = long(x[1] / minxhrs + 1.0) * long(minxhrs);	// NW corner is farthest west
		sadr_y = long(y[2] / minyhrs) * long(minyhrs);			// NE corner is farthest north
		}
	if (! Importing->TestOnly)
		{
		sprintf(logmsg, "The data set '%s',", Importing->LoadName);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		sprintf(logmsg, "which is identified as '%s',", Importing->NameBase);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		sprintf(logmsg, "contains incorrect georeferencing information and a guess has been made on it's actual location.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		sprintf(logmsg, "You should try to obtain an updated and corrected version of that DEM.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, logmsg);
		HosedData = true;
		}
	}
if (sadr_x < RUTMMinEasting)
	RUTMMinEasting = sadr_x;
if ((sadr_x + (RasterCols - 1) * minxhrs) > RUTMMaxEasting)
	RUTMMaxEasting = sadr_x + (RasterCols - 1) * minxhrs;
if (sadr_y > RUTMMaxNorthing)
	RUTMMaxNorthing = sadr_y;
if ((sadr_y - (RasterRows - 1) * minyhrs) < RUTMMinNorthing)
	RUTMMinNorthing = sadr_y - (RasterRows - 1) * minyhrs;

return true;

} // ImportWizGUI::ReadSDTS_RasterDef

/*===========================================================================*/

// returns true if successful (haven't you figured that out by now? :)
bool ImportWizGUI::ReadSDTS_DEM_SpatDomain(void)
{
double value;
struct UTMLatLonCoords coords;
short i;

seq = 0;
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, SpatialDomain);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	stat2 = end123file(&fpin);
	return false;
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
nxy = 0;	// number of coordinate pairs
// Loop to process each subfield
do
	{ // Read data record subfield
	if (! rd123sfld      
		(fpin,			// file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))		// status returned
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// Process based on field and subfield tags
	if (!strcmp(tag, "DMSA"))
		{
		value = atof(stringy);
		// Process if X spatial address
		if ((!strcmp(descr, "!X")) || (!strcmp(descr, "X")))	// original SDTS files with FIPS123 library return !X, new files return X
			{
			x[nxy] = value;
			if ((nxy == 0) && (Importing->Corners == NULL))
				Importing->Corners = (struct QuadCorners *)AppMem_Alloc(sizeof (struct QuadCorners), APPMEM_CLEAR);
			Importing->Corners->Easting[nxy] = value;
			}
		// Process if Y spatial address
		else if ((!strcmp(descr, "!Y")) || (!strcmp(descr, "Y")))
			{
			y[nxy] = value;
			Importing->Corners->Northing[nxy] = value;
			nxy++;
			}
		}
	} while (status != 4);	// Break out of loop at end of file
stat2 = end123file(&fpin);

Importing->NBound = DBL_MIN;
Importing->SBound = DBL_MAX;
Importing->EBound = DBL_MAX;
Importing->WBound = DBL_MIN;
Importing->North = DBL_MIN;	// We'll put Northings & Eastings into these
Importing->South = DBL_MAX;
Importing->East = DBL_MAX;
Importing->West = DBL_MIN;
// convert UTM bounds to degrees, set Northings & Eastings
UTMLatLonCoords_Init(&coords, Importing->UTMZone);
for (i = 0; i < 4; i++)
	{
	coords.East = x[i];
	coords.North = y[i];
	if (coords.North < Importing->South)
		Importing->South = coords.North;
	if (coords.North > Importing->North)
		Importing->North = coords.North;
	if (coords.East > Importing->East)
		Importing->East = coords.East;
	if (coords.East < Importing->West)
		Importing->West = coords.East;
	UTM_LatLon(&coords);
	if (coords.Lat > Importing->NBound)
		Importing->NBound = coords.Lat;
	if (coords.Lat < Importing->SBound)
		Importing->SBound = coords.Lat;
	if (coords.Lon < Importing->EBound)
		Importing->EBound = coords.Lon;
	if (coords.Lon > Importing->WBound)
		Importing->WBound = coords.Lon;
	}

return true;

} // ImportWizGUI::ReadSDTS_DEM_SpatDomain

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_DEM_LayerDef(void)
{

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, LayerDef);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	stat2 = end123file(&fpin);
	return false;
	}
// Read data descriptive record (DDR)
if (! rd123ddrec      
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
// Loop to process each subfield
do 
	{
	// Read data record subfield
	if (! rd123sfld
		(fpin,			/* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,         /* subfield contents returned */
		&str_len,       /* length of subfield */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld
		(fpin,			/* file pointer */
		tag,			/* tag output */
		descr,			/* subfield descriptions output */
		frmts))			/* subfield format control */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		return false;
		}
	// Process based on field and subfield tags
	if (!strcmp(tag, "LDEF") && !strcmp(descr, "NROW"))
		{
		nrows = atol(stringy);
		Importing->InRows = nrows;
		}
	else if (!strcmp(tag, "LDEF") && !strcmp(descr, "NCOL"))
		{
		ncols = atol(stringy);
		Importing->InCols = ncols;
		}
	} while (status != 4);
// Break out of loop at end of file
stat2 = end123file(&fpin);
return true;

} // ImportWizGUI::ReadSDTS_DEM_LayerDef

/*===========================================================================*/

float ImportWizGUI::GaussFix(float *UTMarray, long UTMrows, long CtrCol, long CtrRow)
{
double GaussSum = 0.0, GaussWeight = 0.0, weight;
float *qwik;
short xkern, ykern;
double GaussKernal[5][5] =
	{{1.0, 2.0, 3.0, 2.0, 1.0},
	{2.0, 7.0, 11.0, 7.0, 2.0},
	{3.0, 11.0, 17.0, 11.0, 3.0},
	{2.0, 7.0, 11.0, 7.0, 2.0},
	{1.0, 2.0, 3.0, 2.0, 1.0}};

for (ykern = 0; ykern < 5; ykern++)
	for (xkern = 0; xkern < 5; xkern++)
		{
		qwik = UTMarray + (CtrCol - 2 + xkern) * UTMrows + CtrRow - 2 + ykern;
		if (*qwik > -32766.0f)
			{
			weight = GaussKernal[ykern][xkern];
			GaussWeight += weight;
			GaussSum += *qwik * weight;
			}
		}

if (GaussWeight == 0.0)
	return *qwik;	// we're stuck in a sea of unset data - return what was there
else
	return (float)(-(40000.0 + GaussSum / GaussWeight));

} // ImportWizGUI::GaussFix

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_DEM(struct DEMExtractData *DEMExtract)
{
//char elevbase[64]
char elevname[256+32], *MsgHdr = "Import Wizard: SDTS DEM";
// short *MapPtr, Elev;
bool error = false, success = false;
short k;
#ifndef WCS_BUILD_VNS
short UTMZone;
long VoidCol, VoidRow;
#endif // !WCS_BUILD_VNS
//short namelen;
//double MinE, MinN;
long i, j, Rows, Columns, Row, Col;
float *MapPtr;
//double roundval;
// double ElevDatum;
DEM Topo;
FILE *DEMFile;
#ifndef WCS_BUILD_VNS
BusyWin *BWRE;
long GaussCol, GaussRow;
#endif // WCS_BUILD_VNS
double min_elev = DBL_MAX;

#ifdef ASCIIDUMP
char *outpos, outstr[4096];
short outptr;
#endif // ASCIIDUMP
#ifdef EXTRUDING
float Elev;
double Slope;
short ColSign, RowSign;
long CornerPtX, CornerPtY, MapCtrX, MapCtrY, ColQuit, RowQuit;
#endif // EXTRUDING

//double elrange, elmax, elmin;

DBHost->ResetGeoClip();

DEMExtract->UTMData = NULL;
DEMExtract->LLData 	= NULL;

strcpy(elevname, Importing->InDir);
strcat(elevname, SDTSCell);
if ((DEMFile = PROJ_fopen(elevname, "rb")) == NULL)
	return false;

// Allocate some basic structures
if ((DEMExtract->Info = (struct DEMInfo *)AppMem_Alloc(Num2Load * sizeof (struct DEMInfo), APPMEM_CLEAR)) == NULL)
	{
	MemErr = true;
	UserMessageOK(MsgHdr, "Out of memory allocating SDTS DEM Info Header!\nOperation terminated.");
	return false;
	} // if
if ((DEMExtract->Convert = (struct UTMLatLonCoords *)AppMem_Alloc(sizeof (struct UTMLatLonCoords), APPMEM_CLEAR)) == NULL)
	{
	MemErr = true;
	UserMessageOK(MsgHdr, "Out of memory allocating UTM Coords!\nOperation terminated.");
	goto EndExtract;
	} // if

// if at least one 7.5 minute DEM found, process it first
if (Importing->UTMZone >= 0)
	{
	//long tmp;

	DEMExtract->UTMColInt = minxhrs;
	DEMExtract->UTMRowInt = minyhrs;
	DEMExtract->MaxLat = RUTMBoundN;
	DEMExtract->MinLat = RUTMBoundS;
	DEMExtract->MinLon = RUTMBoundE;
	DEMExtract->MaxLon = RUTMBoundW;

	DEMExtract->MinEast[0] = RUTMMinEasting;
	DEMExtract->MinNorth[0] = RUTMMinNorthing;
	DEMExtract->MaxEast[0] = RUTMMaxEasting;
	DEMExtract->MaxNorth[0] = RUTMMaxNorthing;

	/***
	tmp = 1 + (long)(DEMExtract->MinEast[0] / DEMExtract->UTMColInt);
	DEMExtract->FirstCol = tmp * DEMExtract->UTMColInt;
	DEMExtract->UTMCols = 1 + (long)(fabs((DEMExtract->MaxEast[0] - DEMExtract->FirstCol) / DEMExtract->UTMColInt)); 
	tmp = 1 + (long)(DEMExtract->MinNorth[0] / DEMExtract->UTMRowInt);
	DEMExtract->FirstRow = tmp * DEMExtract->UTMRowInt;
	DEMExtract->UTMRows = 1 + (long)(fabs((DEMExtract->MaxNorth[0] - DEMExtract->FirstRow) / DEMExtract->UTMRowInt));
	***/
	DEMExtract->FirstCol = RUTMMinEasting;
	DEMExtract->UTMCols = (long)((RUTMMaxEasting - RUTMMinEasting) / minxhrs + 1.0);
	DEMExtract->FirstRow = RUTMMaxNorthing;
	DEMExtract->UTMRows = (long)((RUTMMaxNorthing - RUTMMinNorthing) / minyhrs + 1.0);

	// derive the output intervals from the input intervals
	DEMExtract->LLRowInt = DEMExtract->UTMRowInt / (1000.0 * EARTHLATSCALE);

	DEMExtract->LLColInt = DEMExtract->UTMColInt /
		(1000.0 * (EARTHLATSCALE * cos(PiOver180 * (DEMExtract->MaxLat + DEMExtract->MinLat) / 2.0)));

	DEMExtract->LLRows = (long)(fabs((DEMExtract->MaxLat - DEMExtract->MinLat) / DEMExtract->LLRowInt));
	DEMExtract->LLCols = (long)(fabs((DEMExtract->MaxLon - DEMExtract->MinLon) / DEMExtract->LLColInt));

	// recompute the output intervals based on the total spread of data
	DEMExtract->LLRowInt = (DEMExtract->MaxLat - DEMExtract->MinLat) / (DEMExtract->LLRows - 1);
	DEMExtract->LLColInt = (DEMExtract->MaxLon - DEMExtract->MinLon) / (DEMExtract->LLCols - 1);

	DEMExtract->UTMSize = DEMExtract->UTMRows * DEMExtract->UTMCols * sizeof(float);
	DEMExtract->LLSize = DEMExtract->LLRows * DEMExtract->LLCols * sizeof(float);

	DEMExtract->UTMData = (float *)AppMem_Alloc(DEMExtract->UTMSize, 0);
	DEMExtract->LLData = (float *)AppMem_Alloc(DEMExtract->LLSize, APPMEM_CLEAR);

	if (! DEMExtract->UTMData || ! DEMExtract->LLData)
		{
		MemErr = true;
		UserMessageOK(MsgHdr, "Out of memory allocating SDTS DEM Arrays!\nOperation terminated.");
		error = 1;
		goto EndPhase1s;
		} // if

	MapPtr = DEMExtract->UTMData;
	for (i = 0; i < DEMExtract->UTMCols; i++)
		{
		for (j = 0; j < DEMExtract->UTMRows; j++, MapPtr++)
			*MapPtr = (float)-32767.0;
		} // for i=0...

	// Extract 7.5 Min DEM's
	// NOTE: *** _ONLY_ 7.5 Min DEM's supported right now! ***
	for (k = 0; k < Num2Load; k++)
		{
//*		if (DEMExtract->Info[k].Code != 1)
//*			{
//*			if (k == 0)
//*				DEMExtract->FR->GetFirstName();
//*			else
//*				DEMExtract->FR->GetNextName();
//*			continue;
//*			} // if

//*		if ((DEMFile = SDTSDEMFile_Init(DEMExtract, k, MsgHdr)) == NULL)
//*			break;

		Columns = Importing->InCols;
		Rows = Importing->InRows;

		/***
		// get this topos bounds
		MinN = Importing->South;
		MinE = Importing->East;

//		Col = (long)((FCvt(DEMExtract->ProfHdr.Coords[0]) +.5 - DEMExtract->FirstCol) / DEMExtract->UTMColInt);
//		Row = (long)((FCvt(DEMExtract->ProfHdr.Coords[1]) +.5 - DEMExtract->FirstRow) / DEMExtract->UTMRowInt);
		roundval = minxhrs / 2;
		Col = (long)((MinE + roundval - DEMExtract->FirstCol) / minxhrs);
		roundval = minyhrs / 2;
		Row = (long)((MinN + roundval - DEMExtract->FirstRow) / minyhrs);
		***/
		// compute which column & row is the NW corner of the raster we're loading
		Col = (long)((sadr_x - RUTMMinEasting) / minxhrs);
		Row = (long)((RUTMMaxNorthing - sadr_y) / minyhrs);

		ReadSDTS_Elevations(DEMFile, &DEMExtract->UTMData[0], (short)Rows, (short)Columns,
			Row, Col, ElevFactor, voidvalue, fillvalue, DEMExtract);

/***
		elmax = Importing->TestMax;
		elmin = Importing->TestMin;
		elrange = elmax - elmin;

// code to generate grayscale raw image
		double elrange = Importing->TestMax - Importing->TestMin;
		float tmpelev;
		long ijk;
		unsigned char val;
		FILE *frawelevs;
		if (frawelevs = fopen("D:/SDTS.raw", "wb"))
			{
			for (ijk = 0; ijk < DEMExtract->UTMCols * DEMExtract->UTMRows; ijk++)
				{
				tmpelev = DEMExtract->UTMData[ijk];
				if (tmpelev < elmin)
					val = 0;
				else if (tmpelev > elmax)
					val = 255;
				else
					val = (unsigned char)((tmpelev - elmin) / elrange * 255.0);
				fwrite(&val, 1, 1, frawelevs);
				} // for
			fclose(frawelevs);
			} // if
***/

		fclose (DEMFile);

		if (Importing->TestMin < min_elev)
			min_elev = Importing->TestMin;
		Importing = Importing->Next;
		if (Importing)
			{
			error = 1;
			if (! SetFileNames())
				break;
			if (! ReadSDTS_ElevUnits())
				break;
			if (! ReadSDTS_Ident())
				break;
			if (! ReadSDTS_CellRange())
				break;
			if (! ReadSDTS_IRef())
				break;
			if (! ReadSDTS_XRef())
				break;
			if (! ReadSDTS_DEM_SpatDomain())
				break;
			if (! ReadSDTS_RasterDef())
				break;
			if (! ReadSDTS_DEM_LayerDef())
				break;
			error = 0;
			} // if Importing
		} // for k

	if (error)
		goto EndPhase1s;

#ifdef ASCIIDUMP
	// set config var log_file=path&file
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ">>> Original data");
	for (GaussRow = 0; GaussRow < DEMExtract->UTMRows; GaussRow++)
		{
		outptr = 0;
		for (GaussCol = 0; GaussCol < DEMExtract->UTMCols; GaussCol++)
			{
			outpos = &outstr[outptr];
			sprintf(outpos, "%11.3f", *(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow));
			outptr += 11;
			}
		sprintf(outpos, "\n");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, outstr);
		}
#endif // ASCIIDUMP

#ifndef WCS_BUILD_VNS
	// Scan for missing data cells in the inner region.  Use Guassian weighted filter of valid cells to fix.
	// Since we're going to fix this in situ, "fixed" data will be written with a bias so we can distinguish it
	// from unfixed data.  After all cells are fixed, a 2nd pass to remove the bias is made.
	BWRE = new BusyWin("Blank Patch 1a", DEMExtract->UTMRows - 4, 'BWRE', 0);
	for (GaussRow = 2; GaussRow < (DEMExtract->UTMRows - 2); GaussRow++)
		{
		for (GaussCol = 2; GaussCol < (DEMExtract->UTMCols - 2); GaussCol++)
			{
			// look for void or fill
			if ((*(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow) >= -32767.0f) &&
				(*(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow) <= -32766.0f))
				{
				*(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow) =
					GaussFix(DEMExtract->UTMData, DEMExtract->UTMRows, GaussCol, GaussRow);
				} // if
			} // for GaussCol

		if (BWRE)
		  	{
			BWRE->Update(GaussRow - 2);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if

		} // for GaussRow

	if (BWRE) delete BWRE;
	BWRE = NULL;
#endif // WCS_BUILD_VNS

#ifdef ASCIIDUMP
	// set config var log_file=path&file
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ">>> Add bias");
	for (GaussRow = 0; GaussRow < DEMExtract->UTMRows; GaussRow++)
		{
		outptr = 0;
		for (GaussCol = 0; GaussCol < DEMExtract->UTMCols; GaussCol++)
			{
			outpos = &outstr[outptr];
			sprintf(outpos, "%11.3f", *(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow));
			outptr += 11;
			}
		sprintf(outpos, "\n");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, outstr);
		}
#endif

#ifndef WCS_BUILD_VNS
	// now remove the bias
	BWRE = new BusyWin("Blank Patch 1b", DEMExtract->UTMRows - 4, 'BWRE', 0);
	for (GaussRow = 2; GaussRow < (DEMExtract->UTMRows - 2); GaussRow++)
		{
		for (GaussCol = 2; GaussCol < (DEMExtract->UTMCols - 2); GaussCol++)
			{
			if (*(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow) < -32767.0f) // modified data
				{
				*(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow) =
					-40000.0f - *(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow);
				} // if
			} // for GaussCol

		if (BWRE)
		  	{
			BWRE->Update(GaussRow - 2);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if

		} // for GaussRow

	if (BWRE) delete BWRE;
	BWRE = NULL;
#endif // !WCS_BUILD_VNS

#ifdef ASCIIDUMP
	// set config var log_file=path&file
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ">>> Bias removed");
	for (GaussRow = 0; GaussRow < DEMExtract->UTMRows; GaussRow++)
		{
		outptr = 0;
		for (GaussCol = 0; GaussCol < DEMExtract->UTMCols; GaussCol++)
			{
			outpos = &outstr[outptr];
			sprintf(outpos, "%11.3f", *(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow));
			outptr += 11;
			}
		sprintf(outpos, "\n");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, outstr);
		}
#endif // ASCIIDUMP

#ifndef EXTRUDING

#ifndef WCS_BUILD_VNS
	// change voids to minimum elevation
	BWRE = new BusyWin("Void to minimum", DEMExtract->UTMRows, 'BWRE', 0);
	for (VoidRow = 0; VoidRow < DEMExtract->UTMRows; VoidRow++)
		{
		for (VoidCol = 0; VoidCol < DEMExtract->UTMCols; VoidCol++)
			{
			if (*(DEMExtract->UTMData + VoidCol * DEMExtract->UTMRows + VoidRow) <= -32767.0f) // void data
				{
				*(DEMExtract->UTMData + VoidCol * DEMExtract->UTMRows + VoidRow) =
					(float)(min_elev * ElevFactor);
				} // if
			} // for VoidCol

		if (BWRE)
		  	{
			BWRE->Update(VoidRow);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if

		} // for VoidRow

	if (BWRE) delete BWRE;
	BWRE = NULL;
#endif // !WCS_BUILD_VNS

#ifdef ASCIIDUMP
	// set config var log_file=path&file
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, ">>> Void->Min");
	for (GaussRow = 0; GaussRow < DEMExtract->UTMRows; GaussRow++)
		{
		outptr = 0;
		for (GaussCol = 0; GaussCol < DEMExtract->UTMCols; GaussCol++)
			{
			outpos = &outstr[outptr];
			sprintf(outpos, "%11.3f", *(DEMExtract->UTMData + GaussCol * DEMExtract->UTMRows + GaussRow));
			outptr += 11;
			}
		sprintf(outpos, "\n");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, outstr);
		}
#endif

	if (error)
		goto EndPhase1s;

#endif // !EXTRUDING
#ifdef EXTRUDING

	// fill any missing cells with adjacent elevations (by "extrusion")
	BWRE = new BusyWin("Blank Patch 2", 4, 'BWRE', 0);

	MapCtrX = DEMExtract->UTMCols / 2;  
	MapCtrY = DEMExtract->UTMRows / 2;  

	for (k = 0; k < 4; k++)
		{
		switch (k)
			{
			default:
				break;
			case 0:
				{
				CornerPtX 	= 0;
				CornerPtY	= 0;
				RowSign	= -1;
				ColSign	= -1;
				RowQuit	= -1;
				ColQuit	= -1;
				break;
				} // SouthWest
			case 1:
				{
				CornerPtX 	= 0;
				CornerPtY	= DEMExtract->UTMRows - 1;
				RowSign	= +1;
				ColSign	= -1;
				RowQuit	= DEMExtract->UTMRows;
				ColQuit	= -1;
				break;
				} // NorthWest
			case 2:
				{
				CornerPtX 	= DEMExtract->UTMCols - 1;
				CornerPtY	= DEMExtract->UTMRows - 1;
				RowSign	= +1;
				ColSign	= +1;
				RowQuit	= DEMExtract->UTMRows;
				ColQuit	= DEMExtract->UTMCols;
				break;
				} // NorthEast
			case 3:
				{
				CornerPtX 	= DEMExtract->UTMCols - 1;
				CornerPtY	= 0;
				RowSign	= -1;
				ColSign	= +1;
				RowQuit	= -1;
				ColQuit	= DEMExtract->UTMCols;
				break;
				} // SouthEast
			} // switch

		Slope = ((double)CornerPtY - MapCtrY) / ((double)CornerPtX - MapCtrX);

		Elev = DEMExtract->UTMData[MapCtrX * DEMExtract->UTMRows + MapCtrY];
		for (Row=MapCtrY, i=0; Row!=RowQuit; Row+=RowSign, i+=RowSign)
			{
			Col = (long)(MapCtrX + i / Slope);
			MapPtr = DEMExtract->UTMData + Col * DEMExtract->UTMRows + Row;
			while (Col != ColQuit)
				{
				if ((*MapPtr == -32767.0) || (*MapPtr == -32766.0))
					*MapPtr = Elev;
				else
					Elev = *MapPtr;
				Col += ColSign;
				MapPtr += ColSign * DEMExtract->UTMRows;
				} // while 
			} // for Row

		Elev = DEMExtract->UTMData[MapCtrX * DEMExtract->UTMRows + MapCtrY];
		for (Col=MapCtrX, i=0; Col!=ColQuit; Col+=ColSign, i+=ColSign)
			{
			Row = (long)(MapCtrY + i * Slope);
			MapPtr = DEMExtract->UTMData + Col * DEMExtract->UTMRows + Row;
			while (Row != RowQuit)
				{
				if ((*MapPtr == -32767.0) || (*MapPtr == -32766.0))
					*MapPtr = Elev;
				else
					Elev = *MapPtr;
				Row += RowSign;
				MapPtr += RowSign;
				} // while 
			} // for Col
		if (BWRE)
		  	{
			BWRE->Update(k + 1);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if

		} // for k

	if (BWRE) delete BWRE;
	BWRE = NULL;

	if (error)
		goto EndPhase1s;

#endif // EXTRUDING

#ifndef WCS_BUILD_VNS
	// Resample UTM grid into Lat/Lon grid
	BWRE = new BusyWin("Resample", (ULONG)DEMExtract->LLCols, 'BWRE', 0);

	UTMZone = HeadPier->UTMZone;

	DEMExtract->Convert->Lon = DEMExtract->MaxLon;
	for (i = 0; i < DEMExtract->LLCols; i++, DEMExtract->Convert->Lon -= DEMExtract->LLColInt)
		{
		DEMExtract->Convert->Lat = DEMExtract->MinLat;
		for (j = 0; j < DEMExtract->LLRows; j++, DEMExtract->Convert->Lat += DEMExtract->LLRowInt)
			{
			LatLon_UTM(DEMExtract->Convert, UTMZone);

			DEMExtract->LLData[i * DEMExtract->LLRows + j] = (float)(Point_Extract
				(DEMExtract->Convert->East, DEMExtract->Convert->North,	DEMExtract->FirstCol, RUTMMinNorthing,
				DEMExtract->UTMColInt, DEMExtract->UTMRowInt, DEMExtract->UTMData, DEMExtract->UTMRows, DEMExtract->UTMCols));
			/***
			DEMExtract->LLData[i * DEMExtract->LLRows + j] = (float)(Point_Extract
				(DEMExtract->Convert->East, DEMExtract->Convert->North,	DEMExtract->FirstCol, DEMExtract->FirstRow,
				DEMExtract->UTMColInt, DEMExtract->UTMRowInt, DEMExtract->UTMData, DEMExtract->UTMRows, DEMExtract->UTMCols));
			***/
			} // for j
		if (BWRE)
		  	{
			BWRE->Update((ULONG)i + 1);
			if (BWRE->CheckAbort())
				{
				error = 50;
				break;
				} // if
			} // if
		} // for i
	if (BWRE) delete BWRE;
	BWRE = NULL;

	if (error)
		goto EndPhase1s;

	// 7.5 Minute file save
	Topo.pLatEntries 	= (ULONG)DEMExtract->LLRows;
	Topo.pLonEntries 	= (ULONG)DEMExtract->LLCols;
	Topo.pLatStep 	= DEMExtract->LLRowInt;
	Topo.pLonStep 	= DEMExtract->LLColInt;
	Topo.pSouthEast.Lat = RUTMBoundS;	// Importing->SBound;
	Topo.pNorthWest.Lon = RUTMBoundW;	// Importing->WBound;
//	Topo.pNorthWest.Lat = RBoundN;
//	Topo.pSouthEast.Lon = RBoundE;
	Topo.pNorthWest.Lat = RUTMBoundS + (Topo.pLatEntries - 1) * Topo.pLatStep;
	Topo.pSouthEast.Lon = RUTMBoundW - (Topo.pLonEntries - 1) * Topo.pLonStep;
	Topo.pElScale = ELSCALE_METERS;
	Topo.pElDatum = 0.0;
	Topo.PrecalculateCommonFactors();
//	MapHdr.columns 	= DEMExtract->LLRows;
//	MapHdr.rows 	= DEMExtract->LLCols - 1;

	if (MergeLoad)
		error = DBAddSDTS_DEM(MergeName, &Topo, DEMExtract->LLData, DEMExtract->LLSize);
	else
		error = DBAddSDTS_DEM(HeadPier->NameBase, &Topo, DEMExtract->LLData, DEMExtract->LLSize);
#else // !WCS_BUILD_VNS
	Importing = HeadPier;
	Topo.pLatEntries 	= DEMExtract->UTMRows;
	Topo.pLonEntries 	= DEMExtract->UTMCols;
	/***
	Topo.pSouthEast.Lat	= DEMExtract->MinNorth[0];
	Topo.pNorthWest.Lon	= DEMExtract->MinEast[0];
	Topo.pSouthEast.Lon	= DEMExtract->MaxEast[0];
	Topo.pNorthWest.Lat	= DEMExtract->MaxNorth[0];
	***/
	Topo.pNorthWest.Lon = RUTMMinEasting;
	Topo.pSouthEast.Lat = RUTMMinNorthing;
	Topo.pSouthEast.Lon = RUTMMaxEasting;
	Topo.pNorthWest.Lat = RUTMMaxNorthing;
	DEMExtract->MinEast[0] = Topo.pNorthWest.Lon;
	DEMExtract->MaxNorth[0] = Topo.pNorthWest.Lat;
	DEMExtract->MaxEast[0] = Topo.pSouthEast.Lon;
	DEMExtract->MinNorth[0] = Topo.pSouthEast.Lat;
	//Topo.pLatStep 	= (Topo.pNorthWest.Lat -  Topo.pSouthEast.Lat ) / (Topo.pLatEntries - 1);
	//Topo.pLonStep 	= (Topo.pNorthWest.Lon - Topo.pSouthEast.Lon) / (Topo.pLonEntries - 1);
	Topo.pLatStep = minyhrs;
	Topo.pLonStep = -minxhrs;
	Topo.pElScale = ELSCALE_METERS;
	Topo.pElDatum = 0.0;
	Topo.PrecalculateCommonFactors();
	Topo.SetNullReject(1);
	Topo.SetNullValue((float)voidvalue);

	error = DBAddSDTS_DEM(HeadPier->NameBase, &Topo, DEMExtract->UTMData, DEMExtract->UTMSize);
#endif // !WCS_BULD_VNS
	if (HosedData)
		UserMessageOK("SDTS DEM", "There was a problem with a SDTS file - check the log for details.", 1);
	} // if at least one 7.5 minute DEM

EndPhase1s:

if (DEMExtract->UTMData)
	AppMem_Free(DEMExtract->UTMData, DEMExtract->UTMSize);
if (DEMExtract->LLData)
	AppMem_Free(DEMExtract->LLData, DEMExtract->LLSize);
DEMExtract->UTMData = DEMExtract->LLData = NULL;
DEMExtract->UTMSize = DEMExtract->LLSize = 0;
success = true;

EndExtract:

if (HeadPier->FileParent)
	CloseSDTSDBGroup();

if (DEMExtract->Info)
	AppMem_Free(DEMExtract->Info, Num2Load * sizeof (struct DEMInfo));
if (DEMExtract->Convert)
	AppMem_Free(DEMExtract->Convert, sizeof (struct UTMLatLonCoords));

DEMExtract->Info = NULL;
DEMExtract->Convert = NULL;

Importing = HeadPier;

return success;

} // ImportWizGUI::ReadSDTS_DEM

/*===========================================================================*/

// returns true if successful
bool ImportWizGUI::ReadSDTS_Elevations(FILE *DEM, float *ProfPtr, short Rows, short Columns, long start_y,
									   long start_x, double ElevFactor, int voidvalue, int fillvalue,
									   struct DEMExtractData *DEMExtract)
{
BusyWin *BWSI=NULL;
//char	ccs[4];
//char	descr[5000];	// what's reasonable?
SHORT	elevation;
//char	sdtsfname[256];	// what's reasonable?
float	felevation;
float	floaty;
float	*fltptr;
//FILE	*fpin;
//char	frmts[500];		// what's reasonable?
//char	ice;
long	index;
//long	int_level;
//char	leadid;
//int		order;
float	outelev;
short	sdts_x, sdts_y;	// column & row in file
SHORT	shorty;
SHORT	*sptr;
//int		stat2, status;
//long	str_len;
//char	stringy[5000];	// uh- yeah
//char	tag[10];
float	*TruePtr = &ProfPtr[0];	// keep Lint happy
#ifdef TIMING
double	TimeF, TimeS;
char	timemsg[80];
#endif // TIMING
char	busytitle[80];
//#ifdef DEBUG
//char	debugmsg[80];
//#endif // DEBUG

g123order(&order);		// determine byte order of this machine
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, SDTSCell);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_XFER, sdtsfname);
	return false;
	}
if (! rd123ddrec 
	(fpin,          /* file pointer */
	stringy,         /* DDR record returned */
	&status))       /* status returned */
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_DDR, NULL);
	stat2 = end123file(&fpin);
	return false;
	}
status = -1;
sdts_y = 0;
sdts_x = Columns;	// initialize this to pass first sanity check
sprintf(busytitle, "Reading rows (file %d of %d)", ++NumLoaded, Num2Load);
BWSI = new BusyWin(busytitle, (unsigned)(long int)Rows, 'BWSI', 0);
#ifdef TIMING
TimeS = GetSystemTimeFP();
#endif // TIMING
// Loop to process each subfield in CEL0 module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          /* file pointer */
		tag,            /* field tag returned */
		&leadid,        /* leader identifier returned */
		stringy,         /* subfield contents returned */
		&str_len,       /* string length */
		&status))       /* status returned */
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_RSUB, "(CEL0 module)");
		stat2 = end123file(&fpin);
		delete BWSI;
		return false;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          /* file pointer */
		tag,            /* tag output */
		descr,          /* subfield descriptions output */
		frmts))         /* subfield format control */    
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_CSUB, NULL);
		stat2 = end123file(&fpin);
		delete BWSI;
		return false;
		}
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "CVLS"))
		{
		//sprintf(debugmsg, "CVLS - 0x%x", stringy);
		//DEBUGOUT(debugmsg);
		if (FloatElevs)
			{
			fltptr = (float *)stringy;
			floaty = *fltptr;
			if (!order)
				BlindSimpleEndianFlip32F(&floaty, &felevation);
			else
				felevation = floaty;
			// DANGER WILL ROBINSON:
			// SDTS transfers define voidvalue & fillvalue at the time of transfer!  They "normally"
			// should be -32767 & -32766.
			// ACK!  I've found a corrupt SDTS file which stores voids as +32767 instead of -32767!!!
			// The next two lines are a hack to fix this, but shouldn't effect anyone with real data.
			if (felevation == 32767.0)
				felevation = (float)voidvalue;
#ifdef WCS_BUILD_VNS
			if (felevation == (float)fillvalue)
				felevation = (float)voidvalue;
#endif // WCS_BUILD_VNS
			// store scaled file elevations, but DON'T scale VOID & FILL!
			if ((felevation != (float)voidvalue) && (felevation != (float)fillvalue))
				{
				outelev = (float)(felevation * ElevFactor * Importing->VertExag + Importing->DatumChange);
				*TruePtr = outelev;
				}
//			else if (felevation == (float)voidvalue)
//				*TruePtr = -32767.0;
//			else
//				*TruePtr = -32766.0;
			}
		else
			{
			sptr = (SHORT *)stringy;
			shorty = *sptr;
			if (!order)
				SimpleEndianFlip16S(shorty, &elevation);
			else
				elevation = shorty;
			// DANGER WILL ROBINSON:
			// SDTS transfers define voidvalue & fillvalue at the time of transfer!  They "normally"
			// should be -32767 & -32676.
			// ACK!  I've found a corrupt SDTS file which stores voids as +32767 instead of -32767!!!
			// The next two lines are a hack to fix this, but shouldn't effect anyone with real data.
			if (elevation == 32767)
				elevation = voidvalue;
#ifdef WCS_BUILD_VNS
			if (elevation == fillvalue)	// caused run-time error - is this correct now?
				felevation = (float)voidvalue;
#endif // WCS_BUILD_VNS
			// store scaled file elevations, but DON'T scale VOID & FILL!
			if ((elevation != voidvalue) && (elevation != fillvalue))
				{
				outelev = (float)(elevation * ElevFactor * Importing->VertExag + Importing->DatumChange);
				*TruePtr = outelev;
				}
//			else if (elevation == voidvalue)
//				*TruePtr = -32767.0;
//			else
//				*TruePtr = -32766.0;
			}
		sdts_x++;			// got me another one
		TruePtr += DEMExtract->UTMRows;	// store in WCS order
		}
	else if (!strcmp(tag, "CELL") && (!strcmp(descr, "RCID")))
		{
		//DEBUGOUT("CELL / RCID");
		if (sdts_x != Columns)
			{
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_MANGLED, "Bad Record ID");
			stat2 = end123file(&fpin);
			delete BWSI;
			return false;
			}
		}
	else if (!strcmp(tag, "CELL") && (!strcmp(descr, "ROWI")))
		{
		//DEBUGOUT("CELL / ROWI");
		sdts_y++;
		if (sdts_y != atoi(stringy))	// make sure we're in sync with records
			{
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_MANGLED, "Bad Row");
			stat2 = end123file(&fpin);
			delete BWSI;
			return false;
			}
		if (BWSI)
			{
			BWSI->Update(sdts_y);
			if (BWSI->CheckAbort())
				{
				status = 4;
				break;
				}
			}
		}
	else if (!strcmp(tag, "CELL") && (!strcmp(descr, "COLI")))
		{
		//DEBUGOUT("CELL / COLI");
		if (atoi(stringy) != 1)	// SDTS defines this as 1 always!
			{
			GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_SDTSDEM_MANGLED, "Bad Column");
			stat2 = end123file(&fpin);
			delete BWSI;
			return false;
			}
		sdts_x = 0;	// keep track of how many come in
		// Set the base address for the memory writes of the elevations to come in next
		// SDTS DEM is stored in raster order, so put it into WCS order
		//index = DEMExtract->UTMCols - sdts_y + start_y;
		//TruePtr = &ProfPtr[index] + (start_x * DEMExtract->UTMRows);
		index = DEMExtract->UTMRows * (start_x + 1) - start_y - sdts_y;
		TruePtr = &ProfPtr[index];
		}
	if ((sdts_y > Rows) && (sdts_x == Columns))
		status = 4;				// we're done, break out of loop
	} while (status != 4);		// Break out of loop at end of file
#ifdef TIMING
TimeF = GetSystemTimeFP();
sprintf(&timemsg[0], "Time in secs: %f", TimeF-TimeS);
GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, timemsg);
#endif // TIMING
stat2 = end123file (&fpin);
if (BWSI) delete BWSI;
BWSI = NULL;

return true;

} // ImportWizGUI::ReadSDTS_Elevations

/*===========================================================================*/

/***
void ImportWizGUI::CreateSDTSDBGroup(char *Name)
{

if (Importing->FileParent = new (Name) Joe)
	{
	Importing->FileParent->SetFlags(WCS_JOEFLAG_HASKIDS);
	DBHost->AddJoe(Importing->FileParent, WCS_DATABASE_STATIC);
	} // if

} // ImportWizGUI::CreateSDTSDBGroup
***/

/*===========================================================================*/

void ImportWizGUI::CloseSDTSDBGroup(void)
{

if (Importing->FileParent)
	{
	if (Importing->LastJoe)
		DBHost->BoundUpTree(Importing->LastJoe);
	Importing->LastJoe = NULL;
	Importing->FileParent = NULL;
	} // if

} // ImportWizGUI::CloseSDTSDBGroup

/*===========================================================================*/

#ifndef SDTS_UNTILED

short ImportWizGUI::DBAddSDTS_DEM(char *BaseName, DEM *Topo, float *MapArray, long MapSize)
{
char RGBComponent[5] = {0,0,0,0,0};
short error, i, j, ii, jj;
long cols, rows, colmod, rowmod, firstcol, lastcol, firstrow, lastrow;
float *destptr, *srcptr, *TileData;
long ndx, ndx2, TileSize;
long bumpcols, bumprows, lastcols, lastrows;

DupRow = 1;
Importing = HeadPier;
OUTPUT_COLMAPS = (short)((SDTSDEMExtract.LLCols - 1) / 300.0 + 1.0);
OUTPUT_ROWMAPS = (short)((SDTSDEMExtract.LLRows - 1) / 300.0 + 1.0);

OUTPUT_COLS = SDTSDEMExtract.LLCols;
OUTPUT_ROWS = SDTSDEMExtract.LLRows;
ELEV_UNITS = DEM_DATA_UNITS_METERS;

lastcols = cols = SDTSDEMExtract.LLCols / OUTPUT_COLMAPS;
lastrows = rows = SDTSDEMExtract.LLRows / OUTPUT_ROWMAPS;

// figure out if the size of the final tiles should be bumped up (714 / 3 = 238 + 1, 238 + 1, 238, 715 -> 238+1, 238+1, 239, 716 -> 239+1, 238+1, 239)
if ((cols * OUTPUT_COLMAPS) != SDTSDEMExtract.LLCols)
	lastcols++;
if ((rows * OUTPUT_ROWMAPS) != SDTSDEMExtract.LLRows)
	lastrows++;

// see how many column tiles need to be bumped up
bumpcols = SDTSDEMExtract.LLCols - ((OUTPUT_COLMAPS - 1) * cols + lastcols);
for (j = 0; j < OUTPUT_COLMAPS; j++)
	{
	bumprows = SDTSDEMExtract.LLRows - ((OUTPUT_ROWMAPS - 1) * rows + lastrows);
	for (i = 0; i < OUTPUT_ROWMAPS; i++)
		{
		/***
		cols = SDTSDEMExtract.LLCols / OUTPUT_COLMAPS;
		colmod = SDTSDEMExtract.LLCols % (OUTPUT_COLMAPS * cols);
		if (colmod != 0)
			cols++;
		if (j != (OUTPUT_COLMAPS - 1))
			firstcol = cols * j;
		else
			firstcol = SDTSDEMExtract.LLCols - cols;
		lastcol = firstcol + cols - 1;

		rows = SDTSDEMExtract.LLRows / OUTPUT_ROWMAPS;
		rowmod = SDTSDEMExtract.LLRows % (OUTPUT_ROWMAPS * rows);
		if (rowmod != 0)
			rows++;
		if (i != (OUTPUT_ROWMAPS - 1))
			firstrow = rows * i;
		else
			firstrow = SDTSDEMExtract.LLRows - rows;
		lastrow = firstrow + rows - 1;
		***/

		if (j != (OUTPUT_COLMAPS - 1))
			{
			firstcol = cols * j;
			lastcol = firstcol + cols - 1 + 1;	// overlap onto next tile
			if (bumpcols)
				{
				bumpcols--;
				lastcol++;
				}
			}
		else
			{
			firstcol = SDTSDEMExtract.LLCols - lastcols;
			lastcol = SDTSDEMExtract.LLCols - 1;
			}

		if (i != (OUTPUT_ROWMAPS - 1))
			{
			firstrow = rows * i;
			lastrow = firstrow + rows - 1 + 1;	// overlap onto next tile
			if (bumprows)
				{
				bumprows--;
				lastrow++;
				}
			}
		else
			{
			firstrow = SDTSDEMExtract.LLRows - lastrows;
			lastrow = SDTSDEMExtract.LLRows - 1;
			}

		TileSize = (lastcol - firstcol + 1) * (lastrow - firstrow + 1) * sizeof(float);
		if ((TileData = (float *)AppMem_Alloc(TileSize, 0)) == NULL)
			{
			MemErr = true;
			return 1;
			}
		destptr = TileData;
		srcptr = SDTSDEMExtract.LLData;
		OUTPUT_HILAT = RUTMBoundN;
		OUTPUT_LOLAT = RUTMBoundS;
		OUTPUT_HILON = RUTMBoundW;
		OUTPUT_LOLON = RUTMBoundE;

		for (jj = 0; jj < rows; jj++)
			{
			for (ii = 0; ii < cols; ii++)
				{
				ndx = (firstcol + ii) * SDTSDEMExtract.LLRows + firstrow + jj;
				ndx2 = ii * rows + jj;
				destptr[ndx2] = srcptr[ndx];
				}
			}

		ARNIE_SWAP(OUTPUT_COLMAPS, OUTPUT_ROWMAPS);
		error = DataOpsSaveOutput(TileData, TileSize, j, i,
			 rows, cols, rows, cols, RGBComponent);
		ARNIE_SWAP(OUTPUT_COLMAPS, OUTPUT_ROWMAPS);
		AppMem_Free(TileData, TileSize);
		}
	}

Topo->FreeRawElevs();

return (0);

} // ImportWizGUI::DBAddSDTS_DEM

#endif // !SDTS_UNTILED

/*===========================================================================*/

#ifdef SDTS_UNTILED

bool ImportWizGUI::DBAddSDTS_DEM(char *BaseName, DEM *Topo, float *MapArray, long MapSize)
{
char filename[256], FileBase[32];
//char NameStr[80];
short Found = 0;
long Ct, MaxCt = (long)Topo->MapSize();
Joe *Clip, *Added = NULL;
//JoeDEM *MyDEM;
LayerEntry *LE = NULL;
//NotifyTag ChangeEvent[2];
#ifdef WCS_BUILD_VNS
Joe *AverageGuy, *LoadRoot;
VectorPoint *PLink;
long Point;
char Ident[80];
Pier1 *Piers;
#endif // WCS_BUILD_VNS

strcpy(FileBase, BaseName);
strcat(FileBase, ".elev");
strmfp(filename, GlobalApp->MainProj->dirname, FileBase);

if (Topo->AllocRawMap())
	{
	for (Ct = 0; Ct < MaxCt; Ct ++)
		{
		Topo->RawMap[Ct] = (float)MapArray[Ct];
		} // for
	} // if

Topo->FindElMaxMin();	// added by GRH 8/6/01

#ifdef WCS_BUILD_VNS
Clip = DBHost->AddDEMToDatabase("SDTS DEM Extract", BaseName, Topo, NewCoordSys, ProjHost, EffectsHost);	// added by GRH 8/6/01
#else // WCS_BUILD_VNS
Clip = DBHost->AddDEMToDatabase("SDTS DEM Extract", BaseName, Topo, NULL, ProjHost, EffectsHost);	// added by GRH 8/6/01
#endif // WCS_BUILD_VNS

if (! Topo->SaveDEM(filename, GlobalApp->StatusLog))
	{
	Topo->FreeRawElevs();
	return (true);
	} // if
Topo->FreeRawElevs();
GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DEM_SAVE, FileBase);

#ifdef WCS_BUILD_VNS
// Add Quadrangle bounds object
Piers = HeadPier;
if (LoadRoot = new ("Quad Boundaries") Joe)
	{
	if (DBHost->AddJoe(LoadRoot, GlobalApp->AppDB->StaticRoot, ProjHost))
		{
		DBHost->GenerateNotify(DBPreLoadEvent);
		while (Piers)
			{
			strncpy(Ident, Piers->NameBase, 74);
			strcat(Ident, " Quad");
			if (AverageGuy = new (Ident) Joe)
				{
				unsigned long AllocPts;
				AllocPts = 5 + Joe::GetFirstRealPtNum();	// 4 + repeat to close + label?
				if (AverageGuy->Points(GlobalApp->AppDB->MasterPoint.Allocate(AllocPts)))	// label + 4 + repeat to close
					{
					AverageGuy->NumPoints(AllocPts);
					PLink = AverageGuy->Points();
					#ifdef WCS_JOE_LABELPOINTEXISTS
					// Set label point
					PLink->Latitude = Piers->Corners->Northing[0];
					PLink->Longitude = Piers->Corners->Easting[0];
					PLink->Elevation = 0.0f;
					PLink = PLink->Next;
					#endif // WCS_JOE_LABELPOINTEXISTS
					// Set Corners
					for (Point = 0; Point < 4; Point++)
						{
						PLink->Latitude = Piers->Corners->Northing[Point];
						PLink->Longitude = Piers->Corners->Easting[Point];
						PLink->Elevation = 0.0f;
						PLink = PLink->Next;
						} // for
					// Connect to start point
					PLink->Latitude = Piers->Corners->Northing[0];
					PLink->Longitude = Piers->Corners->Easting[0];
					PLink->Elevation = 0.0f;

					AverageGuy->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED);
					AverageGuy->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
					AverageGuy->SetLineStyle(4);
					AverageGuy->SetLineWidth((unsigned char)1);
					AverageGuy->SetRGB(0, 106, 83);
					if (NewCoordSys)
						AverageGuy->AddEffect(NewCoordSys, -1);
					AverageGuy->RecheckBounds();
					LoadRoot->AddChild(AverageGuy, NULL);
					} // if
				GlobalApp->AppDB->DBLayers.AddObjectToLayer(AverageGuy, "Quad Names");
				} // if
			Piers = Piers->Next;
			}
		DBHost->GenerateNotify(DBLoadEvent);
		} // if
	else
		{
		delete LoadRoot;
		LoadRoot = NULL;
		} // else
	} // if
#endif // WCS_BUILD_VNS


return (false);

} // ImportWizGUI::DBAddSDTS_DEM

#endif // SDTS_UNTILED

/*===========================================================================*/

// the filename we are sent is the CATD file, all other filenames in the transfer are stored there
short ImportWizGUI::LoadSDTS_DEM(char *filename, float *Output, char TestOnly)
{
short error = 35;

MemErr = false;
HosedData = false;
Importing->AllowRef00 = false;
Importing->AllowPosE = false;
Importing->AskNull = false;
Importing->TestOnly = TestOnly;

if (! SetFileNames())
	goto Cleanup;
if (! ReadSDTS_ElevUnits())
	goto Cleanup;
if (! ReadSDTS_Ident())
	goto Cleanup;
if (! ReadSDTS_CellRange())
	goto Cleanup;
if (! ReadSDTS_IRef())
	goto Cleanup;
if (! ReadSDTS_XRef())
	goto Cleanup;
if (Importing->HasUTM && (Importing->UTMZone != RUTMZone))
	{
	error = 33;	// different UTM zones
	goto Cleanup;
	}
if (xhrs != minxhrs)
	{
	error = 34;	// differing 7.5' resolutions for region load
	goto Cleanup;
	}
if (! ReadSDTS_DEM_SpatDomain())
	goto Cleanup;
if (! ReadSDTS_RasterDef())
	goto Cleanup;
if (! ReadSDTS_DEM_LayerDef())
	goto Cleanup;
if ((!TestOnly) && (! ReadSDTS_DEM(&SDTSDEMExtract)))
	goto Cleanup;
error = 0;

Cleanup:
if (MemErr)
	error = 1;
if (error)
	DataOpsErrMsg(error);

return error;

} // ImportWizGUI::LoadSDTS_DEM

/*===========================================================================*/

//#ifdef SDTS_DLG
// DLG code adapted from sdtsldlg version .007 by Sol Katz (July 1998)

/*
         Revisions
         =========
12/02/98 sol katz, added point_flag and code to dlgnode to process degenerative lines (points) if they have record numbers that indicate
that they come after the line records. also bumped several arrays up to 10k after finding out that man-made-features have > 1800 nodes
and > 3600 points in the test file.
*/

static FILE *fpdlg;
static FILE *fpdat;
static double xtemp, ytemp;
int p;
static long dlg_x[3000];
static long dlg_y[3000];
long px[10000];
long py[10000];
static long node_id[10000];
static long record_id;
static long att_record_id;
long li;
static long pid_l = 0, pid_r = 0;
static long node_start= 0, node_end= 0;
static long att_dat = 1;
static long att_dlg = 1;
long two = 2;
static short natt;
static int point_flag = -1;
long i;
static char mapname[41];
static char theme[21];
//char fdlen[10];
//char *fdname;
static char mod_name[10];
static char out_file[100];
static char dat_file[100];
double lat; 
double lon;
int zone = 0;
int level = 3;
int utm = 1; 
int meters = 2;
double resolution = 0.61;
static int file2map = 4;
int zero = 0;
int sides = 4;
static int catas = 1;
int one = 1;
static double rem;
static double  NE_Latitude ;
static double  NE_Longitude;
static double  NW_Latitude ;
static double  NW_Longitude;
static double  SW_Latitude ;
static double  SW_Longitude;
static double  SE_Latitude ;
static double  SE_Longitude;

static double  NEX, NEY, NWX, NWY;
static double  SWX, SWY, SEX, SEY;

/*char hdat[4] ;*/
/*char rdoc[4] ;*/
static char oh1oh[4];
static char layerid[3];

static int klines = 0;
static int knodes = 0;
static int kareas = 0;
static int kpoints = 0;

static char errmsg[80];

short ImportWizGUI::LoadSDTS_DLG(bool TestOnly)
{

SetFileNames();

if (TestOnly)
	{
	SDTS_dlg_get_xref();
	return 0;
	}

Importing->InFile[4] = 0;	// use this for the SDTS 4 character base name

// set base output name for external write during testing
strcpy(out_file, Importing->InDir);
strcat(out_file, "WCS_Temp");
strcpy(dat_file, out_file);
strcat(dat_file,".dla");
strcat(out_file,".dlg");

/*** F2 NOTE: Fix this ***/
/***
  if (argc < 4)  // prompt for 2 digit layer number
  {
    printf (
"\nEnter the 2 digits in position 7 and 8 of the base name, include leading 0:\n ");
    scanf ("%s",layerid); 
  }
  else
    strcpy (layerid, argv[3]);
***/
strcpy(layerid, "01");	/*** F2 NOTE: Fix this ***/

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "LE");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".DDF");

if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf ("\nCAN'T OPEN 'DLG3' LINE FILE %s",sdtsfname);
	printf ("Perhaps this is a sdts DEM directory?\n");
	printf ("If you have a file %sCEL0.DDF, then it is a DEM!\n", Importing->InDir);
	printf ("You need sdts DLG data for this program\n");
	dlg_exit(0);
	}
stat2 = end123file(&fpin);
// Open output .dlg file
fpdlg = fopen(out_file, "w");
if (fpdlg == NULL)
	{
	printf("\nERROR OPENING .DLG FILE %s", out_file);
	stat2 = end123file(&fpin);
	dlg_exit(1);
	}
// Open output .dla  file
fpdat = fopen(dat_file, "w");
if (fpdat == NULL)
	{
	printf("\nERROR OPENING .DLA FILE %s", dat_file);
	stat2 = end123file(&fpin);
	dlg_exit(1);
	}
strcpy(oh1oh, "010");
SDTS_dlg_head(status);
SDTS_dlg_more_header(status);
SDTS_dlg_get_iref();
SDTS_dlg_get_xref();
SDTS_dlg_proj(status);
SDTS_dlg_mbr(status);
SDTS_dlg_geo_mbr(status);
SDTS_dlg_loadpoints(status);
// printf(" point flag = %d\n", point_flag);
SDTS_dlg_linecount(status);
// getcounts(status);
//fprintf (fpdlg, "NO LINKAGE RECORDS: SDTS to DLG3, By Sol Katz, July 1998, ver .007\n");
#ifdef WCS_BUILD_VNS
fprintf(fpdlg,  "Visual Nature Studio temporary SDTS to DLG file\n");
#else // !VNS
fprintf(fpdlg,  "World Construction Set temporary SDTS to DLG file\n");
#endif // !VNS
printf("map name = %s \ntheme =  %s\ndate = %s \nscale = %d \n", mapname, theme, date, scale);
fprintf(fpdlg, "%-40s %-10s %8d\n", mapname, date, scale);
// will leave line 3 blank
fprintf(fpdlg,"\n%6d%6d%6d%6d%18.11f%6d%6d%6d%6d\n", level, utm, Importing->UTMZone, meters, resolution, file2map, zero, sides, catas);
lat = (SW_Latitude  + NE_Latitude) / 2;
lon = (SW_Longitude + NE_Longitude) / 2;
fprintf(fpdlg,"%24.15f%24.15f%24.15f\n", lon, lat, (float)zero);
fprintf(fpdlg,"%24.15f%24.15f%24.15f\n", (float)zero, (float)zero, (float)zero);
fprintf(fpdlg,"%24.15f%24.15f%24.15f\n", (float)zero, (float)zero, (float)zero);
fprintf(fpdlg,"%24.15f%24.15f%24.15f\n", (float)zero, (float)zero, (float)zero);
fprintf(fpdlg,"%24.15f%24.15f%24.15f\n", (float)zero, (float)zero, (float)zero);
fprintf(fpdlg,"%18.11f%18.11f%18.11f%18.11f\n", (float)one, (float)zero, (float)zero, (float)zero);

fprintf(fpdlg,"SW    %12.6lf%12.6lf%18.2lf%12.2lf\n", SW_Latitude, SW_Longitude, SWX, SWY);
fprintf(fpdlg,"NW    %12.6lf%12.6lf%18.2lf%12.2lf\n", NW_Latitude, NW_Longitude, NWX, NWY);
fprintf(fpdlg,"NE    %12.6lf%12.6lf%18.2lf%12.2lf\n", NE_Latitude, NE_Longitude, NEX, NEY); 
fprintf(fpdlg,"SE    %12.6lf%12.6lf%18.2lf%12.2lf\n", SE_Latitude, SE_Longitude, SEX, SEY);
fprintf(fpdlg,"%-20s%4d%6d%6d %s%6d%6d %s%6d%6d%6d\n", theme, zero, knodes, knodes, oh1oh, kareas, kareas, oh1oh, klines, klines, one);

printf("processing %d nodes \n", knodes);
knodes = 0;
SDTS_dlg_nodes(status);

printf("processing %d areas \n", kareas);
kareas = 0;
SDTS_dlg_areas(status);

if (kpoints > 0  && point_flag ==  1) 
	{
	printf("processing %d points\n", kpoints);
	SDTS_degenlines(status);
	}

printf("processing %d lines \n", klines);
klines = 0;
SDTS_dlg_lines(status);

if (kpoints > 0  && point_flag ==  0) 
	{
	printf("processing %d points\n", kpoints);
	SDTS_degenlines(status);
	}

printf("\nUse dlgbld to add node-to-line and area-to-line linkage records\n");

// Close files and end
fclose(fpdat);
fclose(fpdlg);

return 0;	/*** F2 NOTE: Fix this ***/

}

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_lines(int status)
{

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "le");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
// strcat (sdtsfname,module);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nCAN'T OPEN 'DLG3' LINE FILE %s", sdtsfname);
	dlg_exit(0);
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d", status);
		fprintf(fpdlg, "\n*** ERROR READING DDR *** %d", status);
		goto done;
		}
status = -1;
nxy = 0;         // number of coordinate pairs
att_record_id = 0;
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
		{
		printf("\nERROR READING DATA RECORD SUBFIELD");
		fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
		goto done;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
		{
		printf("\nERROR CHECKING DATA RECORD SUBFIELD");
		fprintf(fpdlg,"\nERROR CHECKING DATA RECORD SUBFIELD");
		goto done;
		}
	// Process based on field and subfield tags
	if (!strcmp(tag, "LINE") && !strcmp(descr, "RCID"))
		{
		record_id = atol(stringy);
		}
	else if (!strcmp(tag, "PIDL") && !strcmp(descr, "RCID"))
		pid_l = atol(stringy);
	else if (!strcmp(tag, "PIDR") && !strcmp(descr, "RCID"))
		pid_r = atol(stringy);
	else if (!strcmp(tag, "SNID") && !strcmp(descr, "RCID"))
		node_start = atol(stringy);
	else if (!strcmp(tag, "ENID") && !strcmp(descr, "RCID"))
		node_end = atol(stringy);
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!RCID"))
		{
		att_record_id = atol(stringy);
		natt++;
		fprintf(fpdat, "L, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		fprintf(fpdat, ", %s\n", mod_name);
		}
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "RCID"))
		{
		att_record_id = atol(stringy);
		natt++;
		fprintf(fpdat, "L, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		fprintf(fpdat, ", %s\n", mod_name);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts, "B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// Process if X spatial address
		if (!strcmp(descr, "!X"))
				{
				dlg_x[nxy] = li;
				}
		// Process if Y spatial address
		else if (!strcmp(descr, "!Y"))
				{
				dlg_y[nxy] = li;
				nxy++;
				}
		}
	// If end of record, write out record and reinitialize
	if (status == 3 || status == 4)
		{
		fprintf(fpdlg, "L%5ld%6ld%6ld%6ld%6ld            %6ld%6d%6d\n", record_id, node_start, node_end, pid_l, pid_r, nxy, natt, zero);
		klines++ ;
		for (i = 0; i < nxy; i++)
			{
			// Convert internal coordinates to external system
			xtemp = ((double) dlg_x[i] * sfax) + xorg;
			ytemp = ((double) dlg_y[i] * sfay) + yorg;
			fprintf(fpdlg, "%12.2f%12.2f", xtemp, ytemp);
			rem = (i+1) % 3;
			if ( rem == 0 )
				fprintf(fpdlg, "\n");
			}
		if ( rem != 0 )
			fprintf(fpdlg, "\n");
		if (natt > 0 )
			{
			for (i = 0; i < natt; i++)
				{  
				fprintf(fpdlg, "   003%6ld", att_dlg++);
				rem = (i+1) % 6;
				if ( rem == 0 )
					fprintf(fpdlg, "\n");
				}
			if ( rem != 0 )
				fprintf(fpdlg, "\n");
			}
		// fprintf (fpdat,"%7ld,%7ld,%7ld\n",att_dat++,record_id,att_record_id);
		natt = 0;
		nxy  = 0;
		att_record_id = 0;
		}
	} while (status != 4);   // Break out of loop at end of file

done:
// done2:
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_lines

/*===========================================================================*/

void ImportWizGUI::SDTS_degenlines(int status)
{

// degenerate lines sored as points
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "ne");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
p=0;
// strcat(sdtsfname, module);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nCAN'T OPEN POINTS FILE %s, continuing....\n", sdtsfname);
	return;
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d", status);
		fprintf(fpdlg,"\n*** ERROR READING DDR *** %d", status);
		goto done;
		}
status = -1;
nxy = 0;         // number of coordinate pairs
att_record_id = 0;
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// Process based on field and subfield tags
	if (!strcmp(tag, "PNTS") && !strcmp(descr, "RCID"))
		{
		record_id = atol(stringy);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "RCID"))
		{
		pid_l = atol(stringy);
		pid_r = atol(stringy);
		}
	/*
	else if ( !strcmp(tag,"ARID") && !strcmp(descr,"RCID"))
		{
		att_record_id = atol (stringy);
		fprintf (fpdat,"P, %7ld,%7ld,%7d ",record_id,att_record_id,att_dat++);
		fprintf (fpdat,", %s\n",mod_name);
		natt++;
		}
	*/
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!RCID"))
		{
		att_record_id = atol(stringy);
		fprintf(fpdat, "P, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		fprintf(fpdat, ", %s\n", mod_name);
		natt++;
		}
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts,"B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// Process if X spatial address
		if (!strcmp(descr, "X"))
			{
			dlg_x[nxy] = li;
			}
		// Process if Y spatial address
		else if (!strcmp(descr, "Y"))
			{
			dlg_y[nxy] = li;
			nxy++;
			}
		}
	// If end of record, write out record and reinitialize
	if (status == 3 || status == 4)
		{
		node_start = node_id[p];
		node_end = node_id[p];
		p++;
		// klines++;
		fprintf(fpdlg,"L%5ld%6ld%6ld%6ld%6ld            %6ld%6d%6d\n", record_id, node_start, node_end, (long)one, (long)one, two, natt, zero);
		/*       klines, node_start,node_end,pid_l,pid_r,two,natt,zero);*/
		for (i = 0; i < nxy; i++)
			{
			// Convert internal coordinates to external system
			xtemp = ((double) dlg_x[i] * sfax) + xorg;
			ytemp = ((double) dlg_y[i] * sfay) + yorg;
			fprintf(fpdlg, "%12.2f%12.2f", xtemp, ytemp);
			fprintf(fpdlg, "%12.2f%12.2f\n", xtemp, ytemp);
			}
		if (natt > 0 )
			{
			for (i = 0; i < natt; i++) 
				{
				fprintf(fpdlg, "   004%6ld", att_dlg++);
				rem = (i+1) % 6;
				if ( rem == 0 )
					fprintf(fpdlg, "\n");
				}
			if ( rem != 0 )
				fprintf(fpdlg, "\n");
			}
		natt = 0;
		nxy  = 0;
		att_record_id = 0;
		}
	} while (status != 4);	// Break out of loop at end of file

done:
/*  done2: */
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_degenlines

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_get_iref(void)
{
// set some default values
sfax = 1.0;
sfay = 1.0;
xorg = 0.0;
yorg = 0.0;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, InternalSR);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nERROR OPENING FILE %s",sdtsfname);
	dlg_exit(0);
	}
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // string length
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD (IDEN MODULE)");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// check subfield name and extract contents for each subfield
	if (!strcmp(tag, "IREF") && !strcmp(descr, "SFAX"))
	sfax = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "SFAY"))
	sfay = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "XORG"))
	xorg = atof(stringy);
	else if (!strcmp(tag, "IREF") && !strcmp(descr, "YORG"))
	yorg = atof(stringy);
	} while (status != 4);	// Break out of loop at end of file

done:
stat2 = end123file (&fpin);

// printf("\nsfax:                              %f", sfax);
// printf("\nsfay:                              %f", sfay);
// printf("\nxorg:                              %f", xorg);
// printf("\nyorg:                              %f", yorg);

} // ImportWizGUI::SDTS_dlg_get_iref

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_get_xref(void)
{
#ifdef WCS_BUILD_VNS
short foo;
char cfoo[80];
#endif // WCS_BUILD_VNS

// set some default values
strcpy(rsnm,"??1");
// strcpy(hdat,"??2");
// strcpy(rdoc,"??3");

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, ExternalSR);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	sprintf(errmsg, "Error Opening File: %s", sdtsfname);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, errmsg);
	dlg_exit(0);
	}
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}

status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // string length
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD (IDEN MODULE)");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// check subfield name and extract contents for each subfield
	if (!strcmp     (tag, "XREF") && !strcmp(descr, "RSNM"))
		{
		strncpy(rsnm, stringy, 4);
		if (strcmp(rsnm, "UTM") == 0)
			Importing->HasUTM = true;
		}
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "ZONE"))
		{
		izone = atoi(stringy);
		Importing->UTMZone = (short)izone;
		if (RUTMZone == 0)					// if the region zone is unset
			RUTMZone = Importing->UTMZone;	// use this zone
#ifdef WCS_BUILD_VNS
		UTMZoneNum2DBCode(&izone);
		Importing->IWCoordSys.SetZoneByCode(izone);
#endif // WCS_BUILD_VNS
		}
#ifdef WCS_BUILD_VNS
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "HDAT"))
		{ 
		if (!strcmp(stringy, "NAS" ))	// NAD 27
			{
			Importing->IWCoordSys.SetSystem("UTM - NAD 27");
			}
		else if (!strcmp(stringy, "WGC")) datum = 2 ;	// WGS 72
		else if (!strcmp(stringy, "WGE")) datum = 3 ;	// WGS 84
		else if (!strcmp(stringy, "NAX"))	// NAD 83
			{
			Importing->IWCoordSys.SetSystem("UTM - NAD 83");
			}
		else if (!strcmp(stringy, "OHD" )) datum = 5 ;	// Old Hawaii Datum
		else if (!strcmp(stringy, "PRD" )) datum = 6 ;	// Puerto Rico Datum
		}
	// we're doing absolutely nothing with these next two yet
	else if (!strcmp(tag, "VATT") && !strcmp(descr, "VDAT"))
		{ 
		// strcpy(vdat, stringy);
		if      (!strcmp(stringy, "LMSL" )) foo = 1 ;	// Local Means Sea Level
		else if (!strcmp(stringy, "NGVD" )) foo = 2 ;	// National Geodetic Vertical Datum 1929
		else if (!strcmp(stringy, "NAVD" )) foo = 3 ;	// North American Vertical Datum 1988
		}
	else if (!strcmp(tag, "XREF") && !strcmp(descr, "COMT"))
		{ 
		strncpy(cfoo, stringy, 79);
		cfoo[79] = 0;
		}
//	else if (!strcmp (tag, "XREF") && !strcmp (descr, "RDOC"))
//		strcpy(rdoc, stringy);
#endif // WCS_BUILD_VNS
	} while (status != 4);   // Break out of loop at end of file

// printf("rsnm:                              %s\n", rsnm);
// printf("hdat:                              %s\n", hdat);
// printf("rdoc:                              %s\n", rdoc);
// printf("zone:                              %d\n", zone);

done:
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_get_xref

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_head(int status)
{

// Open Identification module
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Ident);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nERROR OPENING FILE %s", sdtsfname);
	dlg_exit(0);
	}
// printf ("\n\nIdentification module:  %s\n", sdtsfname);
// Read Identification module data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // string length
		&status))       // status returned
		{
		printf("\nERROR READING DATA RECORD SUBFIELD (IDEN MODULE)");
		goto done;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
		{
		printf("\nERROR CHECKING DATA RECORD SUBFIELD");
		goto done;
		}
	// Display subfield name and contents for each subfield
	// if (!strcmp (tag, "IDEN") && !strcmp (descr, "TITL"))
	// title
	// fprintf (fpdlg,"%-41s",stringy);
	if (!strcmp(tag, "IDEN") && !strcmp(descr, "MPDT"))
		{
		// map date
		// fprintf (fpdlg,"%-11s",stringy);
		strcpy(date, stringy);
		}
	else if (!strcmp(tag, "IDEN") && !strcmp(descr, "SCAL"))
		{
		// scale
		scale = atol(stringy);
		}
	/*
		fprintf (fpdlg,"%8s\n",stringy);
	else if (!strcmp (tag, "IDEN") && !strcmp (descr, "DAID"))
		fprintf (fpdlg,"\nData ID:                           %s",stringy);
	else if (!strcmp (tag, "IDEN") && !strcmp (descr, "DAST"))
		fprintf (fpdlg,"\nData structure:                    %s",stringy);
	else if (!strcmp (tag, "IDEN") && !strcmp (descr, "DCDT"))
		fprintf (fpdlg,"\nData set creation date:            %s",stringy);
	*/
	} while (status != 4);	// Break out of loop at end of file

done:
status = end123file(&fpin);	// Close input Identification module

} // ImportWizGUI::SDTS_dlg_head

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_proj(int status)
{

if (!strcmp(rsnm, "UTM"))
	{
	// calculate the central lon from the zone
	lon = -((180 - ((short)zone * 6)) + 3);
	// fprintf(fpdlg,"zone = %d , lon=  %f, lat = 0.00000  \n",zone,lon);
	// printf("in UTM code \n" );
	}
else if (!strcmp(rsnm, "GEO")) 
	{
	fprintf(fpdlg, "GEOgraphic coodinates \n" );
	// printf("in GEO code \n" );
	printf("DLG'S can not be created from lat/lon coordinates, sorry\n");
	/*** F2 NOTE: Fix this ***/
	exit(1); 
	}
else
	{ 
	printf("coordsys %s is not UTM and not GEO, faking it with Albers.\n", rsnm); 
	printf("The projection information will be wrong. \n" );
	// printf("in ALBERS code \n" );
	}

status = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_proj

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_nodes(int status)
{

/*
these are the saved points (degen lines) to be added as nodes
for (i=0; i < kpoints; i++)
{
printf ("%ld %ld %ld\n", i , px[i], py[i]);
}
*/
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "no");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
p = 0;
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf ("\nCAN'T OPEN 'DLG3' NODES FILE %s", sdtsfname);
	dlg_exit(0);
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d", status);
		fprintf(fpdlg, "\n*** ERROR READING DDR *** %d", status);
		goto done;
		}
status = -1;
nxy = 0;         // number of coordinate pairs
att_record_id = 0;
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer */
		tag,            // tag output */
		descr,          // subfield descriptions output */
		frmts))         // subfield format control */
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			} // if
	// Process based on field and subfield tags
	if (!strcmp(tag, "PNTS") && !strcmp(descr, "RCID"))
		{
		record_id = atol(stringy);
		} // if
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "RCID"))
		{
		pid_l = atol(stringy);
		pid_r = atol(stringy);
		} // else if
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!RCID"))
		{
		att_record_id = atol(stringy);
		fprintf(fpdat, "N, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		fprintf(fpdat, ", %s\n", mod_name);
		natt++;
		} // else if
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!MODN"))
		{
		strcpy(mod_name,stringy);
		} // else if
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "RCID"))
		{
		att_record_id = atol(stringy);
		fprintf(fpdat, "N, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		fprintf(fpdat, ", %s\n", mod_name);
		natt++;
		} // else if
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "MODN"))
		{
		strcpy(mod_name, stringy);
		} // else if
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts, "B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// Process if X spatial address
		if (!strcmp(descr, "X"))
			{
			dlg_x[nxy] = li;
			} // if
		// Process if Y spatial address
		else if (!strcmp(descr, "Y"))
			{
			dlg_y[nxy] = li;
			nxy++;
			} // else if
		} // else if
	// If end of record, write out record and reinitialize
	if (status == 3 || status == 4)
		{
		knodes++; 
		// if the degen lines are numbered starting at 1, we need to put in the points (degen lines) as nodes at the top of the node list
		while (record_id > knodes)
			{
			/*
			printf ("%d %ld %ld\n", p , px[p], py[p]);
			*/
			xtemp = ((double) px[p] * sfax) + xorg;
			ytemp = ((double) py[p] * sfay) + yorg;
			fprintf(fpdlg,"N%5d%12.2f%12.2f%6d%6d%6d%6d%6d%6d\n", knodes, xtemp, ytemp, zero, zero, zero, zero, zero, zero);
			node_id[p] =knodes; 
			p++;
			knodes++;
			}
		// Convert internal coordinates to external system
		// now we process the actual node
		xtemp = ((double) dlg_x[0] * sfax) + xorg;
		ytemp = ((double) dlg_y[0] * sfay) + yorg;
		fprintf(fpdlg,"N%5ld%12.2f%12.2f%6d%6d%6d%6d%6d%6d\n", record_id, xtemp, ytemp, zero, zero, zero, natt, zero, zero);
		if (natt > 0 ) 
			{
			for (i = 0; i < natt; i++)
				{
				fprintf(fpdlg, "   001%6ld", att_dlg++);
				/* need to count attributes per line */
				rem = (i+1) % 6;
				if ( rem == 0 )
					fprintf(fpdlg,"\n");
				}
			if ( rem != 0 )
				fprintf(fpdlg,"\n");
			}
		/*     printf ("%7ld,%7ld,%7ld\n",att_dat,record_id,att_record_id);*/
		natt = 0;
		nxy  = 0;
		att_record_id = 0;
		}
	} while (status != 4);	// Break out of loop at end of file

	// if the degen lines are numbered starting anywhere after 1, we need to put the points (degen lines) as nodes at the bottom of the node list
	if (point_flag == 0)
		{
		knodes++;
		for (i = 0; i < kpoints; i++)
			{
			//printf ("%d %ld %ld\n", p , px[p], py[p]);
			xtemp = ((double) px[p] * sfax) + xorg;
			ytemp = ((double) py[p] * sfay) + yorg;
			fprintf(fpdlg, "N%5d%12.2f%12.2f%6d%6d%6d%6d%6d%6d\n", knodes, xtemp, ytemp, zero, zero, zero, zero, zero, zero);
			node_id[p] = knodes; 
			p++;
			knodes++;
			}
	}

done:
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_nodes

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_areas(int status)
{

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "na");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
// strcat (sdtsfname,module);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nCAN'T OPEN 'DLG3' AREAS FILE %s",sdtsfname);
	dlg_exit(0);
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d",status);
		fprintf(fpdlg,"\n*** ERROR READING DDR *** %d",status);
		goto done;
		}

natt = 0;
xtemp = 0.0;
ytemp = 0.0;
fprintf(fpdlg, "A%5d%12.2f%12.2f%6d%6d%6d%6d%6d%6d\n", one, xtemp, ytemp, zero, zero, zero, one, zero, zero);
fprintf(fpdlg, "   000   000\n");
status = -1;
nxy = 0;         // number of coordinate pairs
att_record_id = 0;
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// Process based on field and subfield tags
	if (!strcmp(tag, "PNTS") && !strcmp(descr, "RCID"))
		{
		record_id = atol(stringy);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "RCID"))
		{
		att_record_id = atol(stringy);
		fprintf(fpdat, "A, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		natt++;
		fprintf(fpdat, ", %s\n", mod_name);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "ATID") && !strcmp(descr, "!RCID"))
		{
		att_record_id = atol(stringy);
		fprintf(fpdat, "A, %7ld,%7ld,%7d ", record_id, att_record_id, att_dat++);
		natt++;
		fprintf(fpdat, ", %s\n", mod_name);
		}
	else if (!strcmp(tag, "ARID") && !strcmp(descr, "!MODN"))
		{
		strcpy(mod_name, stringy);
		}
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts, "B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// Process if X spatial address
		if (!strcmp(descr, "X"))
			{
			dlg_x[nxy] = li;
			}
		// Process if Y spatial address
		else if (!strcmp(descr, "Y"))
			{
			dlg_y[nxy] = li;
			nxy++;
			}
		}
	// If end of record, write out record and reinitialize
	if (status == 3 || status == 4)
		{
		// Convert internal coordinates to external system
		xtemp = ((double) dlg_x[0] * sfax) + xorg;
		ytemp = ((double) dlg_y[0] * sfay) + yorg;
		// printf(" x y %ld %ld %f %f \n", dlg_x[0], dlg_y[0], xtemp, ytemp);
		fprintf(fpdlg,"A%5ld%12.2f%12.2f%6d%6d%6d%6d%6d%6d\n", record_id, xtemp, ytemp, zero, zero, zero, natt, zero, zero);
		kareas++;
		if (natt > 0 ) 
			{
			for (i=0; i < natt; i++)
				{
				fprintf(fpdlg, "   002%6ld", att_dlg++);
				// need to count attributes per line
				rem = (i+1) % 6 ;
				if ( rem == 0 )
					fprintf(fpdlg, "\n");
				}
			if ( rem != 0 )
				fprintf(fpdlg, "\n");
			}
		// printf ("%7ld,%7ld,%7ld\n",att_dat,record_id,att_record_id);
		natt = 0;
		nxy  = 0;
		att_record_id = 0;
		}
	} while (status != 4);	// Break out of loop at end of file

done:
/*  done2: */
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_areas

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_mbr(int status)
{
int seq=0;
int label;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "np");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
// strcat (sdtsfname,module);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nCAN'T OPEN 'DLG3' 'MBR (np)' FILE %s", sdtsfname);
	dlg_exit(0);
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d", status);
		fprintf(fpdlg, "\n*** ERROR READING DDR *** %d", status);
		goto done;
		}
status = -1;
nxy = 0;		// number of coordinate pairs
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	//  Process based on field and subfield tags
	if (!strcmp(tag, "PNTS") && !strcmp(descr, "RCID"))
		label = atoi(stringy);
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts, "B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// printf( "%2d %ld\n",seq,li);
		// Process if X spatial address
		if (!strcmp(descr, "X"))
			{
			dlg_x[nxy] = li;
			}
		// Process if Y spatial address
		else if (!strcmp(descr, "Y"))
			{
			dlg_y[nxy] = li;
			nxy++;
			}
		}
	// If end of record, write out record and reinitialize
	if (status == 3 || status == 4)
		{
		seq++;
		// for (i = 0; i < nxy; i++)
			{
			// Convert internal coordinates to external system
			xtemp = ((double) dlg_x[0] * sfax) + xorg;
			ytemp = ((double) dlg_y[0] * sfay) + yorg;
			if      ( label==1 )
				{
				SWX = xtemp;
				SWY = ytemp;
				}
			else if ( label==2 ) /*printf( "NW= ");*/
				{
				NWX = xtemp;
				NWY = ytemp;
				}
			else if ( label==3 ) /*printf( "NE= ");*/
				{
				NEX = xtemp;
				NEY = ytemp;
				}
			else if ( label==4 ) /* printf( "SE= ");*/
				{
				SEX = xtemp;
				SEY = ytemp;
				}
			// printf (" %12.2f %12.2f\n", xtemp, ytemp);
			// fprintf (fpdlg,"mbr %d %12.2f %12.2f\n", label, xtemp, ytemp);
			}
		nxy=0;
		}
	} while (status != 4);	// Break out of loop at end of file

done:
/*  done2: */
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_mbr


/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_geo_mbr(int status)
{
int seq=0;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "AHDR.DDF");
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nERROR OPENING FILE %s",sdtsfname);
	dlg_exit(0);
	}
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // string length
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD (AHDR MODULE)");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// check subfield name and extract contents for each subfield
	if		(!strcmp(tag, "ATTP") && !strncmp(descr, "SW_LATITUDE", 11))
		SW_Latitude =  atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "SW_LONGITUDE", 12))
		SW_Longitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "NW_LATITUDE", 11))
		NW_Latitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "NW_LONGITUDE", 12))
		NW_Longitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "NE_LATITUDE", 11))
		NE_Latitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "NE_LONGITUDE", 12))
		NE_Longitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "SE_LATITUDE", 11))
		SE_Latitude = atof(stringy);
	else if (!strcmp(tag, "ATTP") && !strncmp(descr, "SE_LONGITUDE", 12))
		SE_Longitude = atof(stringy);
	// printf("%d %s %s %s\n",seq++,tag,descr,stringy);
	// fprintf(fpdlg,"%d >%s< >%s< >%s<\n",seq,tag,descr,stringy);
	} while (status != 4);   // Break out of loop at end of file

/*
fprintf(fpdlg,"NE_LATITUDE = %f\n",NE_LATITUDE );
fprintf(fpdlg,"NE_LONGITUDE= %f\n",NE_LONGITUDE);
fprintf(fpdlg,"NW_LATITUDE = %f\n",NW_LATITUDE );
fprintf(fpdlg,"NW_LONGITUDE= %f\n",NW_LONGITUDE);
fprintf(fpdlg,"SW_LATITUDE = %f\n",SW_LATITUDE );
fprintf(fpdlg,"SW_LONGITUDE= %f\n",SW_LONGITUDE);
fprintf(fpdlg,"SE_LATITUDE = %f\n",SE_LATITUDE );
fprintf(fpdlg,"SE_LONGITUDE= %f\n",SE_LONGITUDE);

printf("NE_LATITUDE = %f\n",NE_LATITUDE );
printf("NE_LONGITUDE= %f\n",NE_LONGITUDE);
printf("NW_LATITUDE = %f\n",NW_LATITUDE );
printf("NW_LONGITUDE= %f\n",NW_LONGITUDE);
printf("SW_LATITUDE = %f\n",SW_LATITUDE );
printf("SW_LONGITUDE= %f\n",SW_LONGITUDE);
printf("SE_LATITUDE = %f\n",SE_LATITUDE );
printf("SE_LONGITUDE= %f\n",SE_LONGITUDE);
*/

done:
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_geo_mbr

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_more_header(int status)
{
int seq=0;

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, CatSpatialDomain);
// printf( "looking for more header info in cats file\n");
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nERROR OPENING FILE %s", sdtsfname);
	dlg_exit(0);
	}
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}

status = -1;
// Loop to process each subfield in CATS module
do
	{
	// Read data record subfield
	if (! rd123sfld 
	(fpin,          // file pointer
	tag,            // field tag returned
	&leadid,        // leader identifier returned
	stringy,         // subfield contents returned
	&str_len,       // string length
	&status))       // status returned
		{
		printf("\nERROR READING DATA RECORD SUBFIELD (AHDR MODULE)");
		goto done;
		}
	// Retrieve description of current subfield
	if (! chk123sfld 
	(fpin,          // file pointer
	tag,            // tag output
	descr,          // subfield descriptions output
	frmts))         // subfield format control
		{
		printf("\nERROR CHECKING DATA RECORD SUBFIELD");
		goto done;
		}
	// check subfield name and extract contents for each subfield
	if      (!strcmp(tag, "CATS") && !strncmp(descr, "MAP", 3))
		strcpy(mapname, stringy);
	else if (!strcmp(tag, "CATS") && !strncmp(descr, "THEM", 4))
		strcpy(theme, stringy);
	} while (status != 4);   // Break out of loop at end of file

done:
stat2 = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_more_header

/*===========================================================================*/

void ImportWizGUI::SDTS_dlg_loadpoints(int status)
{

// these are the degenerate lines
strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, Importing->InFile);
strcat(sdtsfname, "ne");
strcat(sdtsfname, layerid);
strcat(sdtsfname, ".ddf");
nxy = 0;
// strcat(sdtsfname, module);
if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nPOINT FILE %s not found, continuing...", sdtsfname);
	return;
	}
// Read data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR *** %d", status);
		fprintf(fpdlg, "\n*** ERROR READING DDR *** %d", status);
		goto done;
		}
status = -1;
nxy = 0;         // number of coordinate pairs
att_record_id = 0;
// Loop to process each subfield
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // length of subfield
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR READING DATA RECORD SUBFIELD");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			fprintf(fpdlg, "\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// Process based on field and subfield tags
	if (!strcmp(tag, "PNTS") && !strcmp(descr, "RCID"))
		{
		record_id = atol(stringy);
		kpoints++;
		if (kpoints == 1)
			{
			if (record_id == 1)
				point_flag = 1 ;
			else
				point_flag = 0 ;
			// printf("point flag = %d, record_id = %ld\n", point_flag, record_id);
			}
		}
	else if (!strcmp(tag, "SADR"))
		{
		// Binary data, convert character string returned by rd123sfld to a long integer, changing bit order if necessary
		if (strstr(frmts, "B") != NULL)
			{
			if (!order)
				s123tol(stringy, &li, 1);
			else
				s123tol(stringy, &li, 0);
			}
		// ASCII data
		else
			li = atol(stringy);
		// Process if X spatial address
		if (!strcmp(descr, "X"))
			{
			px[nxy] = li;
			}
		// Process if Y spatial address
		else if (!strcmp(descr, "Y"))
			{
			py[nxy] = li;
			nxy++;
			}
		}
	} while (status != 4);	// Break out of loop at end of file

done:
stat2 = end123file(&fpin);

/*
for (i=0; i < kpoints; i++)
{
printf ("%ld %ld %ld\n", i , px[i], py[i]);
}
*/

} // ImportWizGUI::SDTS_dlg_loadpoints

/*===========================================================================*/

// read statistics module
void ImportWizGUI::SDTS_dlg_linecount(int status)
{
unsigned long gothold = 0;
char hold[5], leid[5], naid[5], neid[5], noid[5];

strcpy(sdtsfname, Importing->InDir);
strcat(sdtsfname, TransferStats);

strcpy(neid, "NE");
strcat(neid, layerid);

strcpy(noid, "NO");
strcat(noid, layerid);

strcpy(leid, "LE");
strcat(leid, layerid);

strcpy(naid, "NA");
strcat(naid, layerid);

if (! beg123file(sdtsfname, 'R', &int_level, &ice, ccs, &fpin))
	{
	printf("\nERROR OPENING FILE %s",sdtsfname);
	dlg_exit(0);
	}
// printf ("\n\nstatistics module:  %s\n",sdtsfname);
// Read module data descriptive record (DDR)
if (! rd123ddrec 
	(fpin,          // file pointer
	stringy,         // DDR record returned
	&status))       // status returned
		{
		printf("\n*** ERROR READING DDR ***");
		goto done;
		}
status = -1;
// Loop to process each subfield in Identification module
do
	{
	// Read data record subfield
	if (! rd123sfld 
		(fpin,          // file pointer
		tag,            // field tag returned
		&leadid,        // leader identifier returned
		stringy,         // subfield contents returned
		&str_len,       // string length
		&status))       // status returned
			{
			printf("\nERROR READING DATA RECORD SUBFIELD (IDEN MODULE)");
			goto done;
			}
	// Retrieve description of current subfield
	if (! chk123sfld 
		(fpin,          // file pointer
		tag,            // tag output
		descr,          // subfield descriptions output
		frmts))         // subfield format control
			{
			printf("\nERROR CHECKING DATA RECORD SUBFIELD");
			goto done;
			}
	// Display subfield name and contents for each subfield
	if (!strcmp(tag, "STAT") && !strcmp(descr, "MNRF"))
		{
		strcpy(hold, stringy);
		gothold = 1;
		// printf("hold = %s \n", hold);
		}
//lint -save -e645
	else if (!strcmp(descr, "NREC") && gothold && !strcmp(hold, neid))
		{
		kpoints = atoi(stringy);
		// printf("%d lines\n", kpoints);
		}
	else if (!strcmp(descr, "NREC") && gothold && !strcmp(hold, leid))
		{
		klines = atoi(stringy);
		// printf("%d lines\n",klines);
		}
	else if (!strcmp(descr, "NREC") && gothold && !strcmp(hold, noid))
		{
		knodes = atoi(stringy);
		// printf("%d nodes\n", knodes);
		}
	else if (!strcmp(descr, "NREC") && gothold && !strcmp(hold, naid))
		{
		kareas = atoi(stringy);
		// printf("%d lines\n", kareas);
		}
//lint -restore
	} while (status != 4);	// Break out of loop at end of file

done:
printf("\nnodes %d, areas %d, lines %d, points %d\n", knodes, kareas, klines, kpoints);

knodes = knodes + kpoints;	// for degenerate lines
klines = klines + kpoints;	// for degenerate lines
kareas++;					// add in the universe polygon

// Close input statistics module
status = end123file(&fpin);

} // ImportWizGUI::SDTS_dlg_linecount

/*===========================================================================*/

void ImportWizGUI::dlg_exit(int code)
{

printf("< Hit DLG exit >\n");

} // ImportWizGUI::dlg_exit

//#endif // SDTS_DLG
