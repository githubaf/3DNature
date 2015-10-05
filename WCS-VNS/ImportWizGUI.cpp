// ImportWizGUI.cpp
// The MegaOneStopImport Wizard code
// 09/09/99 FPW2 {the dreaded date}
// Copyright 3D Nature 1999

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Conservatory.h"
#include "Database.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Application.h"
#include "Toolbar.h"
#include "Fenetre.h"
#include "DataOpsDefs.h"
#include "requester.h"
#include "Useful.h"
#include "Interactive.h"
#include "AppMem.h"
#include "AppHelp.h"
#include "DXF.h"
//#include "ProjectDispatch.h"
#include "ImageInputFormat.h"
#include "WCSVersion.h"
#include "resource.h"
#include "FeatureConfig.h"	//lint -e766
#include "UsefulPathString.h"

//#include <boost/thread/thread.hpp>
//#include <iostream>
//using namespace std;

//void hello_world()
//	{
//	OutputDebugString("Yo world!\n");
//	}

// if you want to see import settings
//#define EXPORTPIERS

// values used by output scaling
static HFONT FontFixedWidth = NULL;

static double IWGridSizeNS, IWGridSizeWE;
static char *intype, knowhow;
static char IMWizElevMethod = 1, IMWizGridCP = 0;
static bool NewVertStyle = true, RandomCP = false;
static char *FormatNames[IW_INPUT_MAX_FORMATS + 1] = {
	"Arc ASCII Array",
	"Arc Grid",
	"Arc Export Grid",
	"ASCII Array",
	"Binary Array",
	"Bryce Terrain",
	"DTED",
	"DXF",
	"GTOPO30",
	"Image - BMP",
	"Image - ECW",
	"Image - IFF",
	"Image - JPEG",
	//"Image - MrSID",
	"Image - Pict",
	"Image - PNG",
	"Image - Targa",
	"Image - TIFF",
	"MicroDEM",
	"NTF DTM",
	//"NTF MERIDIAN2",
	"SDTS DEM",
	"SDTS DLG",
	"ShapeFile",
	"SRTM",
	"STM",
	#ifdef GO_SURFING
	"Surfer Grid",
	#endif // GO_SURFING
	"Terragen Terrain",
	"USGS ASCII DEM",
	"USGS DLG",
	"VistaPro DEM",
#ifdef WCS_BUILD_VNS
	"WCS/VNS DEM",
#else // WCS_BUILD_VNS
	"WCS DEM",
#endif // WCS_BUILD_VNS
	"WCS XYZ",
	"WCS ZBuffer",
	"XYZ",
	"ASCII GPS",
	"GPX",
	NULL };
static long FormatIdents[IW_INPUT_MAX_FORMATS] = {
	IW_INPUT_ARCASCII,
	IW_INPUT_ARCBINARYADF_GRID,
	//IW_INPUT_ARCEXPORT_COVER,
	IW_INPUT_ARCEXPORT_DEM,
	IW_INPUT_ASCII_ARRAY,
	IW_INPUT_BINARY,
	IW_INPUT_BRYCE,
	IW_INPUT_DTED,
	IW_INPUT_DXF,
	IW_INPUT_GTOPO30,
	IW_INPUT_IMAGE_BMP,
	IW_INPUT_IMAGE_ECW,
	IW_INPUT_IMAGE_IFF,
	IW_INPUT_IMAGE_JPEG,
	// IW_INPUT_IMAGE_MR_SID,
	IW_INPUT_IMAGE_PICT,
	IW_INPUT_IMAGE_PNG,
	IW_INPUT_IMAGE_TARGA,
	IW_INPUT_IMAGE_TIFF,
	IW_INPUT_MDEM,
	IW_INPUT_NTF_DTM,
	//IW_INPUT_NTF_MERIDIAN2,
	IW_INPUT_SDTS_DEM,
	IW_INPUT_SDTS_DLG,
	IW_INPUT_SHAPE,
	IW_INPUT_SRTM,
	IW_INPUT_STM,
	#ifdef GO_SURFING
	IW_INPUT_SURFER,
	#endif // GO_SURFING
	IW_INPUT_TERRAGEN,
	IW_INPUT_USGS_ASCII_DEM,
	IW_INPUT_USGS_ASCII_DLG,
	IW_INPUT_VISTAPRO,
	IW_INPUT_WCS_DEM,
	IW_INPUT_WCS_XYZ,
	IW_INPUT_WCS_ZBUFFER,
	IW_INPUT_XYZ,
	IW_INPUT_ASCII_GPS,
	IW_INPUT_GPX};
static short FormatPos[IW_INPUT_MAX_FORMATS];
#ifndef WCS_BUILD_VNS
static bool FormatInWCS[IW_INPUT_MAX_FORMATS] = {
	true,
	true,
	//true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	false,
	true,
	true,
	//true,
	true,
	true,
	true,
	true,
	true,
	false,
	//true,
	true,
	true,
	true,
	false,
	true,
	#ifdef GO_SURFING
	true,
	#endif // GO_SURFING
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true,
	true };
#endif // !WCS_BUILD_VNS

// Panel orders
#ifdef WCS_BUILD_VNS

// Panels 0, 1, 2, 3 always IDD_IMWIZ_IDENT, IDD_IMWIZ_LOADAS, IDD_IMWIZ_OUTTYPE, IDD_IMWIZ_COORDSYS
static unsigned long PanelsArcASCII[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsArcBinaryADFGrid[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsArcExportDEM[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsASCII[] = 
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsASCIIGPS[] =
	{IDD_IMWIZ_GPS_STEP1, IDD_IMWIZ_GPS_STEP2D, IDD_IMWIZ_GPS_STEP3};
static unsigned long PanelsBinary[] =
	{IDD_IMWIZ_BINSET, /***IDD_IMWIZ_CELLORDER,***/ IDD_IMWIZ_ICRHEAD, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//static unsigned long PanelsBinaryGeoRef[] =
//	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_BINSET, /***IDD_IMWIZ_CELLORDER,***/ IDD_IMWIZ_ICRHEAD, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsBryce[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsDTED[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsDXF[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_GRIDIT};
static unsigned long PanelsGTOPO30[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsImage[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_COLEL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsMDEM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsNTFDTM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//static unsigned long PanelsNTFMERIDIAN2[] =
//	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsSDTSDEM[] =
	{0};
static unsigned long PanelsSDTSDLG[] =
	{0};
static unsigned long PanelsShape[] =
	{IDD_IMWIZ_SHAPEOPTS, /***IDD_IMWIZ_ELEVUNITS, IDD_IMWIZ_WCSKNOWS ***/};
static unsigned long PanelsSRTM[] =
	{IDD_IMWIZ_BINSET, /***IDD_IMWIZ_CELLORDER,***/ IDD_IMWIZ_ICRHEAD, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsSTM[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//#ifdef GO_SURFING
static unsigned long PanelsSurfer[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//#endif // GO_SURFING
static unsigned long PanelsTerragen[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsUSGSASCII[] =
	{0};
static unsigned long PanelsUSGSDLG[] =
	{0};
static unsigned long PanelsVistaPro[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsWCSDEM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsWCSXYZ[] =
	{/***IDD_IMWIZ_ELEVUNITS,***/ IDD_IMWIZ_GRIDIT};
static unsigned long PanelsWCSZBuf[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsXYZ[] =
	{/***IDD_IMWIZ_ELEVUNITS,***/ IDD_IMWIZ_GRIDIT};

#else  // WCS_BUILD_VNS

// Panels 0, 1, 2, always IDD_IMWIZ_IDENT, IDD_IMWIZ_LOADAS, IDD_IMWIZ_OUTTYPE
static unsigned long PanelsArcASCII[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsArcBinaryADFGrid[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsArcExportDEM[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsASCII[] = 
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsBinary[] =
	{IDD_IMWIZ_BINSET, /***IDD_IMWIZ_CELLORDER,***/ IDD_IMWIZ_ICRHEAD, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//static unsigned long PanelsBinaryGeoRef[] =
//	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_BINSET, /***IDD_IMWIZ_CELLORDER,***/ IDD_IMWIZ_ICRHEAD, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsBryce[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsDTED[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsDXF[] =
	{IDD_IMWIZ_WCSUNITS, IDD_IMWIZ_REFCO, IDD_IMWIZ_GRIDIT};
static unsigned long PanelsGTOPO30[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_NULL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsImage[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_COLEL, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsMDEM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsNTFDTM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//static unsigned long PanelsNTFMERIDIAN2[] =
//	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsSDTSDEM[] =
	{0};
static unsigned long PanelsSDTSDLG[] =
	{0};
static unsigned long PanelsShape[] =
	{IDD_IMWIZ_SHAPEOPTS, /***IDD_IMWIZ_ELEVUNITS, IDD_IMWIZ_WCSKNOWS ***/};
static unsigned long PanelsSTM[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
//#ifdef GO_SURFING
//static unsigned long PanelsSurfer[] =
//	{};
//#endif // GO_SURFING
static unsigned long PanelsTerragen[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsUSGSASCII[] =
	{0};
static unsigned long PanelsUSGSDLG[] =
	{0};
static unsigned long PanelsVistaPro[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsWCSDEM[] =
	{IDD_IMWIZ_WCSKNOWS, IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsWCSXYZ[] =
	{/***IDD_IMWIZ_ELEVUNITS,***/ IDD_IMWIZ_GRIDIT};
static unsigned long PanelsWCSZBuf[] =
	{IDD_IMWIZ_REFCO, IDD_IMWIZ_HOREX, IDD_IMWIZ_PREPRO, IDD_IMWIZ_VERTEX, IDD_IMWIZ_OUTTILES};
static unsigned long PanelsXYZ[] =
	{IDD_IMWIZ_WCSUNITS, /***IDD_IMWIZ_ELEVUNITS,***/ IDD_IMWIZ_GRIDIT};
//	{IDD_IMWIZ_HORUNITS, IDD_IMWIZ_ELEVUNITS, IDD_IMWIZ_GRIDIT};

#endif // WCS_BUILD_VNS

struct DXF_Pens dxfpens;

/*===========================================================================*/

bool ImportWizGUI::IdentByExtension(char *ext)
{

// if the user selected a header or projection file, try to figure out the real data file
if ((stricmp(ext, ".hdr") == 0) || (stricmp(ext, ".prj") == 0))
	{
	ext = GuessBetter();
	if (ext == 0)
		return true;
	}
if (stricmp(ext, ".dem") == 0)
	{
	strcpy(whatitis, "USGS_DEM.VISTA_DEM.GTOPO30");
	return false;
	}
if (stricmp(ext, ".adf") == 0)
	{
	strcpy(whatitis, "ARC_BINARYADF_GRID");
	return false;
	}
if ((stricmp(ext, ".1") == 0) || (stricmp(ext, ".2") == 0))
	{
	strcpy(whatitis, "USGS_DEM");
	return false;
	}
if ((stricmp(ext, ".dlg") == 0) || (stricmp(ext, ".do") == 0) || (stricmp(ext, ".opt") == 0))
	{
	strcpy(whatitis, "USGS_DLG");
	return false;
	}
if (stricmp(ext, ".vdem") == 0)
	{
	strcpy(whatitis, "VISTA_DEM");
	return false;
	}
if (stricmp(ext, ".dxf") == 0)
	{
	strcpy(whatitis, "DXF");
	return false;
	}
if ((stricmp(ext, ".brc") == 0) || (stricmp(ext, ".br3") == 0) || (stricmp(ext, ".br4") == 0))
	{
	strcpy(whatitis, "BRYCE_DEM");
	return false;
	}
if (stricmp(ext, ".bil") == 0)
	{
	strcpy(whatitis, "BINARY");
	Importing->BandStyle = IMWIZ_DATA_LAYOUT_BIL;
	return false;
	}
if (stricmp(ext, ".bip") == 0)
	{
	strcpy(whatitis, "BINARY");
	Importing->BandStyle = IMWIZ_DATA_LAYOUT_BIP;
	return false;
	}
if (stricmp(ext, ".bsq") == 0)
	{
	strcpy(whatitis, "BINARY");
	Importing->BandStyle = IMWIZ_DATA_LAYOUT_BSQ;
	return false;
	}
if (stricmp(ext, ".flt") == 0)
	{
	strcpy(whatitis, "BINARY");
	Importing->BandStyle = IMWIZ_DATA_LAYOUT_BIL;
	return false;
	}
if (stricmp(ext, ".bmp") == 0)
	{
	strcpy(whatitis, "IMAGE_BMP");
	return false;
	}
if (stricmp(ext, ".ddf") == 0)
	{
	strcpy(whatitis, "SDTS_DEM.SDTS_DLG");
	return false;
	}
if (stricmp(ext, ".iff") == 0)
	{
	strcpy(whatitis, "IMAGE_IFF");
	return false;
	}
if (stricmp(ext, ".elev") == 0)
	{
	strcpy(whatitis, "WCS_DEM");
	return false;
	}
if (stricmp(ext, ".e00") == 0)
	{
	strcpy(whatitis, "ARC_EXPORT_DEM");
	return false;
	}
if ((stricmp(ext, ".gz") == 0) || (stricmp(ext, ".z") == 0))
	{
	strcpy(whatitis, "GZIP");
	return false;
	}
if ((stricmp(ext, ".pic") == 0) || (stricmp(ext, ".pict") == 0) || (stricmp(ext, ".pct") == 0))
	{
	strcpy(whatitis, "IMAGE_PICT");
	return false;
	}
if ((stricmp(ext, ".shp") == 0) || (stricmp(ext, ".dbf") == 0))
	{
	strcpy(whatitis, "SHAPEFILE");
	return false;
	}
if (stricmp(ext, ".stm") == 0)
	{
	strcpy(whatitis, "STM");
	return false;
	}
if (stricmp(ext, ".tga") == 0)
	{
	strcpy(whatitis, "IMAGE_TARGA");
	return false;
	}
if (stricmp(ext, ".xyz") == 0)
	{
	strcpy(whatitis, "XYZ_FILE");
	return false;
	}
if (stricmp(ext, ".wxyz") == 0)
	{
	strcpy(whatitis, "WCS_XYZ");
	return false;
	}
if ((stricmp(ext, ".dt0") == 0) || (stricmp(ext, ".dt1") == 0) || (stricmp(ext, ".dt2") == 0))
	{
	strcpy(whatitis, "DTED");
	return false;
	}
if ((stricmp(ext, ".terrain") == 0) || (stricmp(ext, ".ter") == 0))
	{
	strcpy(whatitis, "TERRAGEN");
	return false;
	}
if (stricmp(ext, ".zip") == 0)
	{
	strcpy(whatitis, "ZIP");
	return false;
	}
if ((stricmp(ext, ".w3o") == 0) || (stricmp(ext, ".w3d") == 0))
	{
	strcpy(whatitis, "WCS_3DOBJECT");
	return false;
	}
if (stricmp(ext, ".lwo") == 0)
	{
	strcpy(whatitis, "LWOB");
	return false;
	}
if (stricmp(ext, ".3ds") == 0)
	{
	strcpy(whatitis, "3DS");
	return false;
	}
if (stricmp(ext, ".proj") == 0)
	{
	strcpy(whatitis, "WCS_PROJ");
	return false;
	}
if (stricmp(ext, ".v4s") == 0)
	{
	strcpy(whatitis, "VISTA_DEM");
	return false;
	}
#ifdef GO_SURFING
if (stricmp(ext, ".grd") == 0)
	{
	strcpy(whatitis, "SURFER");
	return false;
	}
#endif // GO_SURFING
if (stricmp(ext, ".ntf") == 0)
	{
	strcpy(whatitis, "NTF_DTM");
//	strcpy(whatitis, "NTF_DTM.NTF_MERIDIAN2");
	return false;
	} //
if (stricmp(ext, ".hgt") == 0)
	{
	strcpy(whatitis, "SRTM");
	return false;
	} // if
if (stricmp(ext, ".wpt") == 0)
	{
	strcpy(whatitis, "ASCII_GPS");
	return false;
	} // if
if (stricmp(ext, ".gps") == 0)
	{
	strcpy(whatitis, "ASCII_GPS");
	Importing->forceGPS = true;
	return false;
	} // if

strcpy(whatitis, "UNKNOWN_EXT");
return false;

} // ImportWizGUI::IdentByExtension

/*===========================================================================*/

void ImportWizGUI::IdentByAssociation(char *fullbasename)
{
FILE *test;
char testname[256+32];

//lint -save -e550
strcpy(testname, fullbasename);
strcat(testname, ".hdr");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	{
	strcpy(whatitis, "NO_ASSOCIATES");
	return;
	}
strcpy(testname, fullbasename);
strcat(testname, ".blw");
if ((test = PROJ_fopen(testname, "rb")) != NULL)
	{
	strcpy(whatitis, "NED_BINARY");
	return;
	}
strcpy(testname, fullbasename);
strcat(testname, ".dmw");
if ((test = PROJ_fopen(testname, "rb")) != NULL)
	{
	strcpy(whatitis, "GTOPO30");
	return;
	}
strcpy(testname, fullbasename);
strcat(testname, ".prj");
if ((test = PROJ_fopen(testname, "rb")) != NULL)
	{
	strcpy(whatitis, "NED_GRIDFLOAT");
	return;
	}
strcpy(whatitis, "NO_ASSOCIATES");
return;
//lint -restore

} // ImportWizGUI::IdentByAssociation

/*===========================================================================*/

void ImportWizGUI::TryAllImportable(FILE *unknown)
{
FILE *hdrfile;
char hdrname[256+32], subtype[32];

// don't retest what we guessed at
//if (strstr(whatitis, "3DS") == NULL)
//	if (Sniff3DS(unknown))
//		{
//		strcpy(whatitis, "3DS");
//		return;
//		}
if (strstr(whatitis, "LWOB") == NULL)
	if (SniffLWOB(unknown))
		{
		strcpy(whatitis, "LWOB");
		return;
		} // if
if (strstr(whatitis, "WCS_3DOBJECT") == NULL)
	if (SniffWCS_3Dobj(unknown))
		{
		strcpy(whatitis, "WCS_3DOBJECT");
		return;
		} // if
if (strstr(whatitis, "BRYCE_DEM") == NULL)
	if (SniffBryce(unknown))
		{
		strcpy(whatitis, "BRYCE_DEM");
		return;
		} // if
if (strstr(whatitis, "IMAGE_BMP") == NULL)
	if (SniffBMP(unknown))
		{
		strcpy(whatitis, "IMAGE_BMP");
		return;
		} // if
if (strstr(whatitis, "DTED") == NULL)
	if (SniffDTED(unknown, completename))
		{
		strcpy(whatitis, "DTED");
		return;
		} // if
if (strstr(whatitis, "DXF") == NULL)
	if (SniffDXF(unknown))
		{
		strcpy(whatitis, "DXF");
		return;
		} // if
if (strstr(whatitis, "GTOPO30") == NULL)
	if (SniffGTOPO30(unknown, completename))
		{
		strcpy(whatitis, "GTOPO30");
		return;
		} // if
if (strstr(whatitis, "IMAGE_IFF") == NULL)
	if (SniffIFF(unknown))
		{
		strcpy(whatitis, "IMAGE_IFF");
		return;
		} // if
if (strstr(whatitis, "WCS_ZBUFFER") == NULL)
	if (SniffWCS_ZBuffer(unknown))
		{
		strcpy(whatitis, "WCS_ZBUFFER");
		return;
		} // if
if (strstr(whatitis, "MDEM") == NULL)
	if (SniffMDEM(unknown))
		{
		strcpy(whatitis, "MDEM");
		return;
		} // if
if (strstr(whatitis, "NTF_DTM") == NULL)
	if (SniffNTFDTM(unknown))
		{
		strcpy(whatitis, "NTF_DTM");
		return;
		} // if
//if (strstr(whatitis, "NTF_MERIDIAN2") == NULL)
//	if (SniffNTFDTM(unknown))
//		{
//		strcpy(whatitis, "NTF_MERIDIAN2");
//		return;
//		} // if
if (strstr(whatitis, "ARC_EXPORT_DEM") == NULL)
	if (SniffArcExportDEM(unknown))
		{
		strcpy(whatitis, "ARC_EXPORT_DEM");
		return;
		} // if
//if (strstr(whatitis, "NED_BINARY") == NULL)
//	if (SniffNEDBinary(unknown, completename))
//		{
//		strcpy(whatitis, "NED_BINARY");
//		return;
//		} // if
//if (strstr(whatitis, "NED_GRIDFLOAT") == NULL)
//	if (SniffNEDGridFloat(unknown, completename))
//		{
//		strcpy(whatitis, "NED_GRIDFLOAT");
//		return;
//		} // if
if (strstr(whatitis, "IMAGE_PICT") == NULL)
	if (SniffPICT(unknown))
		{
		strcpy(whatitis, "IMAGE_PICT");
		return;
		} // if
if (strstr(whatitis, "SDTS_DEM") == NULL)
	if (SniffSDTSDEM(unknown, completename))
		{
		strcpy(whatitis, "SDTS_DEM");
		return;
		} // if
if (strstr(whatitis, "SDTS_DLG") == NULL)
	if (SniffSDTSDLG(unknown, completename))
		{
		strcpy(whatitis, "SDTS_DLG");
		return;
		} // if
if (strstr(whatitis, "SHAPEFILE") == NULL)
	if (SniffShapefile(unknown, completename))
		{
		strcpy(whatitis, "SHAPEFILE");
		return;
		} // if
if (strstr(whatitis, "IMAGE_TARGA") == NULL)
	if (SniffTarga(completename))
		{
		strcpy(whatitis, "IMAGE_TARGA");
		return;
		} // if
if (strstr(whatitis, "STM") == NULL)
	if (SniffSTM(unknown))
		{
		strcpy(whatitis, "STM");
		return;
		} // if
#ifdef GO_SURFING
if (strstr(whatitis, "SURFER") == NULL)
	if (SniffSurfer(unknown))
		{
		strcpy(whatitis, "SURFER");
		return;
		} // if
#endif // GO_SURFING
if (strstr(whatitis, "TERRAGEN") == NULL)
	if (SniffTerragen(unknown))
		{
		strcpy(whatitis, "TERRAGEN");
		return;
		} // if
if (strstr(whatitis, "USGS_DEM") == NULL)
	if (SniffUSGS_DEM(unknown))
		{
		strcpy(whatitis, "USGS_DEM");
		return;
		} // if
if (strstr(whatitis, "USGS_DLG") == NULL)
	if (SniffUSGS_DLG(unknown))
		{
		strcpy(whatitis, "USGS_DLG");
		return;
		} // if
if (strstr(whatitis, "VISTA_DEM") == NULL)
	if (SniffVistaPro(unknown))
		{
		strcpy(whatitis, "VISTA_DEM");
		return;
		} // if
if (strstr(whatitis, "WCS_DEM") == NULL)
	if (SniffWCS_DEM(unknown))
		{
		strcpy(whatitis, "WCS_DEM");
		return;
		} // if
if (strstr(whatitis, "WCS_PROJ") == NULL)
	if (SniffWCS_Proj(unknown))
		{
		strcpy(whatitis, "WCS_PROJ");
		return;
		} // if
if (strstr(whatitis, "ARC_BINARYADF_GRID") == NULL)
	if (SniffArcBinaryADFGrid(unknown, completename))
		{
		strcpy(whatitis, "ARC_BINARYADF_GRID");
		return;
		} // if

// still don't have any idea
if (SniffArcASCIIArray(unknown))
	{
	strcpy(whatitis, "ARC_ASCII_ARRAY");
	return;
	} // if

if (SniffGPS(unknown))
	{
	strcpy(whatitis, "ASCII_GPS");
	return;
	} // if

if (SniffGPX(unknown))
	{
	strcpy(whatitis, "GPX");
	return;
	} // if

/*** F2 NOTE - CALL WCS IDENTIFY FOR OTHER FILE TYPES ***/
// Test to see if it's only ASCII chars
switch (SniffASCII(unknown))
	{
	default:	// keep lint happy
	case 0:
		strcpy(subtype, "BINARY");
		break;
	case 1:
		strcpy(subtype, "NUMERICFILE");
		break;
	case 2:
		strcpy(subtype, "EXPONENTFILE");
		break;
	case 3:
		strcpy(subtype, "TEXTFILE");
		break;
	} // switch

if (strstr(subtype, "NUMERICFILE") || strstr(subtype, "EXPONENTFILE"))
	{
	if (SniffASCIIArray(unknown))
		{
		if (Importing->InCols == 3)
			{
			if (SniffXYZ(unknown))
				{
				strcpy(whatitis, "XYZ_FILE");
				return;
				} // if
			} // if
		else if (Importing->InCols == 4)
			{
			if (SniffWXYZ(unknown))
				{
				strcpy(whatitis, "WCS_XYZ");
				return;
				} // if
			} // else if
		strcpy(whatitis, "ASCII_ARRAY");
		return;
		} // if
	} // if
else if (strstr(subtype, "TEXTFILE"))
	{
	if (SniffGPS(unknown))
		{
		strcpy(whatitis, "ASCII_GPS");
		return;
		} // if
	} // else if
else if (strstr(subtype, "BINARY"))
	{
//	if (!(tried & BINARY_BIL))
//		if (SniffBIL(unknown))
//			return BINARY_BIL;
//	if (!(tried & BINARY_BIP))
//		if (SniffBIP(unknown))
//			return BINARY_BIP;
//	if (!(tried & BINARY_BSQ))
//		if (SniffBSQ(unknown))
//			return BINARY_BSQ;
#ifdef WCS_BUILD_VNS
	strcpy(hdrname, noextname);
	strcat(hdrname, ".prj");
	ReadArcPrj(hdrname);	// attempt to set CoordSys via Arc projection file
#endif // WCS_BUILD_VNS
	if ((Importing->InCols <= 1) && (Importing->InRows <= 1))	// if we haven't read anything to tell us this yet
		{
		strcpy(hdrname, noextname);
		strcat(hdrname, ".hdr");
		if (hdrfile = PROJ_fopen(hdrname, "r"))
			{
			(void)ReadBinHeader(hdrfile);	// read what we can
			fclose(hdrfile);
			} // if
		} // if
	if ((Importing->NBound == Importing->SBound) || (Importing->WBound == Importing->EBound))
		{
		ReadWorldFiles();
		} // if
	strcpy(whatitis, "BINARY");
	return;
	} // else if
strcpy(whatitis, subtype);	// should be only a TEXTFILE at this point
return;

} // ImportWizGUI::TryAllImportable

/*===========================================================================*/

void ImportWizGUI::Snoopy(char *loadname, char *thisload, unsigned char *load_as)
{
FILE *fin;
//#ifdef WCS_BUILD_VNS
short ImageFound;
//#endif // WCS_BUILD_VNS
size_t exten;
char msg[512];
char DaPath[256],DaFile[64];

PanelNum = 0;

SECURITY_INLINE_CHECK(018, 18);
whatitis[0] = NULL;
if ((fin = PROJ_fopen(loadname, "rb")) == NULL)
	{
	sprintf(&msg[0], "Can't open %s", loadname);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
	return;
	} // if
strcpy(completename, loadname);
strcpy(noextname, loadname);
// added 6/3/03 since paths with extensions & files without was causing the noextname to end up being the path minus ext. only
BreakFileName(loadname, DaPath, sizeof(DaPath), DaFile, sizeof(DaFile));
if (strchr(DaFile, '.'))
	(void)StripExtension(&noextname[0]);
exten = strlen(noextname);
if (exten != strlen(completename))	// is there an extension in the filename?
	{
	if (IdentByExtension(&completename[exten]))	// filename was corrected
		{
		strcpy(loadname, completename);
		if ((fin = PROJ_fopen(loadname, "rb")) == NULL)
			{
			sprintf(&msg[0], "Can't open %s", loadname);
			GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
			return;
			} // if
		} // if
	// verify format indicated by extension (whatitis is 0 if name was corrected to a filename w/out an extension
	if ((*whatitis != 0) && (strstr(whatitis, "UNKNOWN_EXT") == NULL))
		{
		if (strstr(whatitis, "PIER1"))
			{
			rewind(fin);
			Importing->Load(fin, 0, false);
			strcpy(whatitis, Importing->MyIdentity);
			goto identified;
			} // if
		// warning - heavy use of GOTO ahead
		if (strstr(whatitis, "3DS"))
			if (Sniff3DS(fin))
				{
				strcpy(whatitis, "3DS");
				goto identified;
				} // if
		if (strstr(whatitis, "ARC_BINARYADF_GRID"))
			if (SniffArcBinaryADFGrid(fin, completename))
				{
				strcpy(whatitis, "ARC_BINARYADF_GRID");
				goto identified;
				} // if
		if (strstr(whatitis, "ARC_EXPORT_DEM"))
			if (SniffArcExportDEM(fin))
				{
				strcpy(whatitis, "ARC_EXPORT_DEM");
				goto identified;
				} // if
		if (strstr(whatitis, "ASCII_GPS"))
			if (SniffGPS(fin), Importing->forceGPS)
				{
				strcpy(whatitis, "ASCII_GPS");
				goto identified;
				}
		if (strstr(whatitis, "BINARY_BIL"))
			{
			if (SniffNEDBinary(fin, completename))
				{
				strcpy(whatitis, "NED_BINARY");
				goto identified;
				} // if
			if (SniffBIL(fin))
				goto identified;
			} // if
		if (strstr(whatitis, "BINARY_BIP"))
			{
			if (SniffNEDBinary(fin, completename))
				{
				strcpy(whatitis, "NED_BINARY");
				goto identified;
				} // if
			if (SniffBIP(fin))
				goto identified;
			} // if
		if (strstr(whatitis, "BINARY_BSQ"))
			{
			if (SniffNEDBinary(fin, completename))
				{
				strcpy(whatitis, "NED_BINARY");
				goto identified;
				} // if
			if (SniffBSQ(fin))
				goto identified;
			} // if
		if (strstr(whatitis, "BRYCE_DEM"))
			if (SniffBryce(fin))
				{
				strcpy(whatitis, "BRYCE_DEM");
				goto identified;
				} // if
		if (strstr(whatitis, "DTED"))
			if (SniffDTED(fin, completename))
				{
				strcpy(whatitis, "DTED");
				goto identified;
				} // if
		if (strstr(whatitis, "DXF"))
			if (SniffDXF(fin))
				{
				strcpy(whatitis, "DXF");
				goto identified;
				} // if
		if (strstr(whatitis, "GTOPO30"))
			if (SniffGTOPO30(fin, completename))
				{
				strcpy(whatitis, "GTOPO30");
				strcpy(loadname, completename);
				goto identified;
				} // if
		if (strstr(whatitis, "GZIP"))
			if (SniffGZIP(fin))
				{
				strcpy(whatitis, "GZIP");
				goto identified;
				} // if
		if (strstr(whatitis, "IMAGE_BMP"))
			if (SniffBMP(fin))
				{
				strcpy(whatitis, "IMAGE_BMP");
				goto identified;
				} // if
		if (strstr(whatitis, "IMAGE_IFF"))
			if (SniffIFF(fin))
				{
				strcpy(whatitis, "IMAGE_IFF");
				goto identified;
				} // if
		if (strstr(whatitis, "IMAGE_TARGA"))
			if (SniffTarga(completename))
				{
				strcpy(whatitis, "IMAGE_TARGA");
				goto identified;
				} // if
		if (strstr(whatitis, "IMAGE_PICT"))
			if (SniffPICT(fin))
				{
				strcpy(whatitis, "IMAGE_PICT");
				goto identified;
				} // if
		if (strstr(whatitis, "LWOB"))
			if (SniffLWOB(fin))
				{
				strcpy(whatitis, "LWOB");
				goto identified;
				} // if
		if (strstr(whatitis, "NTF_DTM"))
			if (SniffNTFDTM(fin))
				{
				strcpy(whatitis, "NTF_DTM");
				goto identified;
				} // if
//		if (strstr(whatitis, "NTF_MERIDIAN2"))
//			if (SniffNTFMeridian2(fin))
//				{
//				strcpy(whatitis, "NTF_MERIDIAN2");
//				goto identified;
//				} // if
		if (strstr(whatitis, "SDTS_DEM"))
			if (SniffSDTSDEM(fin, completename))
				{
				strcpy(whatitis, "SDTS_DEM");
				strcpy(loadname, completename);
				goto identified;
				} // if
		if (strstr(whatitis, "SDTS_DLG"))
			if (SniffSDTSDLG(fin, completename))
				{
				strcpy(whatitis, "SDTS_DLG");
				strcpy(loadname, completename);
				goto identified;
				} // if
		if (strstr(whatitis, "SHAPEFILE"))
			if (SniffShapefile(fin, completename))
				{
				strcpy(whatitis, "SHAPEFILE");
				goto identified;
				} // if
		if (strstr(whatitis, "SRTM"))
			if (SniffSRTM(fin, DaFile))
				{
				strcpy(whatitis, "SRTM");
				goto identified;
				} // if
		if (strstr(whatitis, "STM"))
			if (SniffSTM(fin))
				{
				strcpy(whatitis, "STM");
				goto identified;
				} // if
#ifdef GO_SURFING
		if (strstr(whatitis, "SURFER"))
			if (SniffSurfer(fin))
				{
				strcpy(whatitis, "SURFER");
				goto identified;
				} // if
#endif // GO_SURFING
		if (strstr(whatitis, "TERRAGEN"))
			if (SniffTerragen(fin))
				{
				strcpy(whatitis, "TERRAGEN");
				goto identified;
				} // if
		if (strstr(whatitis, "USGS_DEM"))
			if (SniffUSGS_DEM(fin))
				{
				strcpy(whatitis, "USGS_DEM");
				goto identified;
				} // if
		if (strstr(whatitis, "USGS_DLG"))
			if (SniffUSGS_DLG(fin))
				{
				strcpy(whatitis, "USGS_DLG");
				goto identified;
				} // if
		if (strstr(whatitis, "VISTA_DEM"))
			if (SniffVistaPro(fin))
				{
				strcpy(whatitis, "VISTA_DEM");
				goto identified;
				} // if
		if (strstr(whatitis, "WCS_3DOBJECT"))
			if (SniffWCS_3Dobj(fin))
				{
				strcpy(whatitis, "WCS_3DOBJECT");
				goto identified;
				} // if
		if (strstr(whatitis, "WCS_DEM"))
			if (SniffWCS_DEM(fin))
				{
				strcpy(whatitis, "WCS_DEM");
				goto identified;
				} // if
		if (strstr(whatitis, "WCS_PROJ"))
			if (SniffWCS_Proj(fin))
				{
				strcpy(whatitis, "WCS_PROJ");
				goto identified;
				} // if
		if (strstr(whatitis, "WCS_XYZ"))
			if (SniffWXYZ(fin))
				{
				strcpy(whatitis, "WCS_XYZ");
				goto identified;
				} // if
		if (strstr(whatitis, "XYZ_FILE"))
			if (SniffXYZ(fin))
				{
				strcpy(whatitis, "XYZ_FILE");
				goto identified;
				} // if
		if (strstr(whatitis, "ZIP"))
			if (SniffZip(fin))
				{
				strcpy(whatitis, "ZIP");
				goto identified;
				} // if
		} // if not unknown extension
	} // if has extension

//#ifdef WCS_BUILD_VNS
ImageFound = IdentImage(loadname);
switch (ImageFound)
	{
	case WCS_BITMAPS_IDENTIMAGE_TIF:
	case WCS_BITMAPS_IDENTIMAGE_TIFF:
		strcpy(whatitis, "IMAGE_TIFF");
		break;
	case WCS_BITMAPS_IDENTIMAGE_JPG:
	case WCS_BITMAPS_IDENTIMAGE_JPEG:
		strcpy(whatitis, "IMAGE_JPEG");
		break;
	case WCS_BITMAPS_IDENTIMAGE_PNG:
		strcpy(whatitis, "IMAGE_PNG");
		break;
#ifdef WCS_BUILD_VNS
	case WCS_BITMAPS_IDENTIMAGE_ECW:
		strcpy(whatitis, "IMAGE_ECW");
		break;
#endif // WCS_BUILD_VNS
//	case WCS_BITMAPS_IDENTIMAGE_MRSID:
//		strcpy(whatitis, "IMAGE_MR_SID");
//		break;
	default:
	case WCS_BITMAPS_IDENTIMAGE_ERROR:
	case WCS_BITMAPS_IDENTIMAGE_UNKNOWN:
		TryAllImportable(fin);
		break;
	}
//#else // WCS_BUILD_VNS
//TryAllImportable(fin);
//#endif // WCS_BUILD_VNS

identified:
	Importing->MyNum = IW_INPUT_BINARY;	// set a default
	if (strstr(whatitis, "3DS"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "ARC_ASCII_ARRAY"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_ARCASCII;
		}
	else if (strstr(whatitis, "ARC_BINARYADF_GRID"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_ARCBINARYADF_GRID;
		}
	else if (strstr(whatitis, "ARC_EXPORT_DEM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_ARCEXPORT_DEM;
		}
	else if (strstr(whatitis, "ASCII_ARRAY"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_ASCII_ARRAY;
		}
	else if (strstr(whatitis, "ASCII_GPS"))
		{
		*load_as = LAS_CP | LAS_VECT;
		Importing->MyNum = IW_INPUT_ASCII_GPS;
		} // else if
	else if (strstr(whatitis, "BRYCE_DEM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_BRYCE;
		}
	else if (strstr(whatitis, "DTED"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_DTED;
		}
	else if (strstr(whatitis, "DXF"))
		{
		*load_as = LAS_CP | LAS_VECT;
		Importing->MyNum = IW_INPUT_DXF;
		}
	else if (strstr(whatitis, "EXPONENTFILE"))
		{
		*load_as = LAS_DEM | LAS_CP;
		}
	else if (strstr(whatitis, "GTOPO30"))
		{
		strcpy(loadname, completename);	// file name may have been corrected
		*load_as = LAS_DEM;
#ifdef WCS_BUILD_VNS
		Importing->IWCoordSys.SetSystem("Geographic - WGS 84");
		Importing->CoordSysWarn = false;
#endif // WCS_BUILD_VNS
		Importing->MyNum = IW_INPUT_GTOPO30;
		}
	else if (strstr(whatitis, "GZIP"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "IMAGE_BMP"))
		{
		*load_as = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		Importing->MyNum = IW_INPUT_IMAGE_BMP;
		}
	else if (strstr(whatitis, "IMAGE_IFF"))
		{
		*load_as = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		Importing->MyNum = IW_INPUT_IMAGE_IFF;
		}
	else if (strstr(whatitis, "IMAGE_PICT"))
		{
		*load_as = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		Importing->MyNum = IW_INPUT_IMAGE_PICT;
		}
	else if (strstr(whatitis, "IMAGE_TARGA"))
		{
		*load_as = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		Importing->MyNum = IW_INPUT_IMAGE_TARGA;
		}
	else if (strstr(whatitis, "LWOB"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "MDEM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_MDEM;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "NED_BINARY"))
		{
		strcpy(loadname, completename);	// file name may have been corrected
		strcpy(whatitis, "BINARY");
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_BINARY;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "NED_GRIDFLOAT"))
		{
		strcpy(loadname, completename);	// file name may have been corrected
		strcpy(whatitis, "BINARY");
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_BINARY;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "NTF_DTM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_NTF_DTM;
		Importing->CoordSysWarn = false;
		}
//	else if (strstr(whatitis, "NTF_MERIDIAN2"))
//		{
//		*load_as = LAS_DEM | LAS_VECT;
//		Importing->MyNum = IW_INPUT_NTF_MERIDIAN2;
//		Importing->CoordSysWarn = false;
//		}
	else if (strstr(whatitis, "NO_ASSOCIATES"))
		{
		*load_as = LAS_ERR;
		}
	else if (strstr(whatitis, "NUMERICFILE"))
		{
		*load_as = LAS_DEM | LAS_CP;
//		*load_as = LAS_ERR;
//		sprintf(&msg[0], "%s", "Identified: Numeric file");
		Importing->MyNum = IW_INPUT_ASCII_ARRAY;
		}
	else if (strstr(whatitis, "SDTS_DEM"))
		{
		*load_as = LAS_DEM;
		Importing->MyNum = IW_INPUT_SDTS_DEM;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "SDTS_DLG"))
		{
		*load_as = LAS_VECT;
		Importing->MyNum = IW_INPUT_SDTS_DLG;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "SHAPEFILE"))
		{
		strcpy(loadname, completename);	// file name may have been corrected
		*load_as = LAS_VECT | LAS_CP;
		Importing->MyNum = IW_INPUT_SHAPE;
		} // else if
	else if (strstr(whatitis, "SRTM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_SRTM;
		} // else if
	else if (strstr(whatitis, "STM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_STM;
		} // else if
#ifdef GO_SURFING
	else if (strstr(whatitis, "SURFER"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_SURFER;
		}
#endif // GO_SURFING
	else if (strstr(whatitis, "TERRAGEN"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_TERRAGEN;
		}
	else if (strstr(whatitis, "TEXTFILE"))
		{
//		*load_as = LAS_ERR;
//		Importing->Signal = DEAD_END;
		*load_as = LAS_DEM | LAS_CP;
		}
	else if (strstr(whatitis, "BINARY"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_BINARY;
		}
	else if (strstr(whatitis, "NTF_DTM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_NTF_DTM;
		Importing->CoordSysWarn = false;
		}
//	else if (strstr(whatitis, "NTF_MERIDIAN2"))
//		{
//		*load_as = LAS_DEM | LAS_VECT;
//		Importing->MyNum = IW_INPUT_NTF_MERIDIAN2;
//		Importing->CoordSysWarn = false;
//		}
	else if (strstr(whatitis, "UNKNOWN_EXT"))
		{
		strcpy(whatitis, "BINARY");
		*load_as = LAS_DEM | LAS_CP;
		}
	else if (strstr(whatitis, "USGS_DEM"))
		{
		*load_as = LAS_DEM;
		Importing->MyNum = IW_INPUT_USGS_ASCII_DEM;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "USGS_DLG"))
		{
		*load_as = LAS_VECT;
		Importing->MyNum = IW_INPUT_USGS_ASCII_DLG;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "VISTA_DEM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_VISTAPRO;
		}
	else if (strstr(whatitis, "WCS_3DOBJECT"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "WCS_DEM"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_WCS_DEM;
		Importing->CoordSysWarn = false;
		}
	else if (strstr(whatitis, "WCS_PROJ"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "WCS_XYZ"))
		{
		*load_as = LAS_CP;
		Importing->MyNum = IW_INPUT_WCS_XYZ;
		}
	else if (strstr(whatitis, "WCS_ZBUFFER"))
		{
		*load_as = LAS_DEM | LAS_IMAGE;
		Importing->MyNum = IW_INPUT_WCS_ZBUFFER;
		}
	else if (strstr(whatitis, "XYZ_FILE"))
		{
		*load_as = LAS_CP;
		Importing->MyNum = IW_INPUT_XYZ;
		}
	else if (strstr(whatitis, "ZIP"))
		{
		*load_as = LAS_ERR;
		Importing->Signal = DEAD_END;
		}
	else if (strstr(whatitis, "IMAGE_TIFF"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_IMAGE_TIFF;
		}
	else if (strstr(whatitis, "IMAGE_JPEG"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_IMAGE_JPEG;
		}
	else if (strstr(whatitis, "IMAGE_ECW"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_IMAGE_ECW;
		}
	else if (strstr(whatitis, "IMAGE_PNG"))
		{
		*load_as = LAS_DEM | LAS_CP;
		Importing->MyNum = IW_INPUT_IMAGE_PNG;
		}
	else if (strstr(whatitis, "GPX"))
		{
		*load_as = LAS_VECT;
		Importing->MyNum = IW_INPUT_GPX;
		}
//	else if (strstr(whatitis, "IMAGE_MR_SID"))
//		{
//		*load_as = LAS_DEM | LAS_CP;
//		Importing->MyNum = IW_INPUT_IMAGE_MR_SID;
//		}
	strcpy(Importing->LoadName, loadname);
	BreakFileName(loadname, Importing->InDir, 256, Importing->InFile, 32);
	strcpy(thisload, whatitis);
	strcpy(Importing->MyIdentity, whatitis);
	fclose(fin);
	Importing->LoadOpt = *load_as;

//	ShowID(whatitis);

} // ImportWizGUI::Snoopy

/*===========================================================================*/

NativeGUIWin ImportWizGUI::Open(Project *Moi)
{
NativeGUIWin Success;

if (Success = GUIFenetre::Open(Moi))
	{
	GlobalApp->MCP->AddWindowToMenuList(this);
	} // if

return (Success);

} // ImportWizGUI::Open

/*===========================================================================*/

NativeGUIWin ImportWizGUI::Construct(void)
{

IWGlobeRad = GlobalApp->AppEffects->GetPlanetRadius();
ControlPts = 0;
RUTMMaxNorthing = -DBL_MAX;
RUTMMinNorthing = DBL_MAX;
RUTMMaxEasting = -DBL_MAX;
RUTMMinEasting = DBL_MAX;
RUTMBoundN = -DBL_MAX;
RUTMBoundS = DBL_MAX;
RUTMBoundE = DBL_MAX;
RUTMBoundW = -DBL_MAX;
RUTMZone = 0;
InitDXFPens(&dxfpens);

if (! NativeWin)
	{
	NativeWin = CreateWinFromTemplate(IDD_IMWIZ, LocalWinSys()->RootWin);

	(void)CreateSubWinFromTemplate(IDD_IMWIZ_BINSET, 0, 1, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_BUILDOPT, 0, 2, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_COLEL, 0, 3, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_ELEVUNITS, 0, 4, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_GRIDIT, 0, 5, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_HOREX, 0, 6, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_HORUNITS, 0, 7, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_ICRHEAD, 0, 8, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_IDENT, 0, 9, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_LOADAS, 0, 10, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_NULL, 0, 11, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_CELLORDER, 0, 12, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTFORM, 0, 13, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTPARAMS, 0, 14, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTREG, 0, 15, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTSCALE, 0, 16, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTTILES, 0, 17, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTTYPE, 0, 18, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_PIXCTR, 0, 19, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_PREPRO, 0, 20, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_REFCO, 0, 21, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_SAVEGEO, 0, 22, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_SHAPEOPTS, 0, 23, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_TEST, 0, 24, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_VERTEX, 0, 25, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_WCSKNOWS, 0, 26, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_WCSUNITS, 0, 0, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_WRAP, 0, 27);
#ifdef WCS_BUILD_VNS
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_COORDSYS, 0, 28, false);
#endif // WCS_BUILD_VNS
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_GPS_STEP1, 0, 29, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_GPS_STEP2F, 0, 30, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_GPS_STEP2D, 0, 31, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_GPS_STEP3, 0, 32, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTSCALE_TWOVAL, 1, 0, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTSCALE_ONEVAL, 1, 1, false);
	(void)CreateSubWinFromTemplate(IDD_IMWIZ_OUTSCALE_MINMAX, 1, 2, false);

	if (NativeWin)
		{
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEFORMAT, "Signed Integer");
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEFORMAT, "Unsigned Integer");
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEFORMAT, "Floating Point");

		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "One");
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Two");
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Four");

		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEFORMAT, "Signed Integer");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEFORMAT, "Unsigned Integer");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEFORMAT, "Floating Point");

		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "One");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Two");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Four");

		(void)WidgetCBAddEnd(IDC_CBBINSET_BYTEORDER, "High-Low/Motorola/Big");
		(void)WidgetCBAddEnd(IDC_CBBINSET_BYTEORDER, "Low-High/Intel/Little");

		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MAJORAXIS, "W --> E");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MAJORAXIS, "E --> W");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MAJORAXIS, "N --> S");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MAJORAXIS, "S --> N");

		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MINORAXIS, "N --> S");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MINORAXIS, "S --> N");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MINORAXIS, "W --> E");
		(void)WidgetCBAddEnd(IDC_CBCELLORDER_MINORAXIS, "E --> W");

		(void)WidgetCBAddEnd(IDC_CBOUTSCALEOVTYPE, "Max Out");
		(void)WidgetCBAddEnd(IDC_CBOUTSCALEOVTYPE, "Min Out");
		(void)WidgetCBAddEnd(IDC_CBOUTSCALEOVTYPE, "I/O Scale");

		(void)WidgetCBAddEnd(IDC_CBICRHEAD_BINFACTORS, " ");

		// need to create a fixed-width font and install it into the table widgets on the GPS pages
		if (FontFixedWidth = CreateFont(14, 0, 0, 0, FW_NORMAL, false, false, 0,
			ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY,
			FIXED_PITCH | FF_MODERN, "Courier New"))
			{
			NativeControl Wid;

			if (Wid = GetWidgetFromID(IDC_GPS_FILEPREVIEW1))
				{
				SendMessage(Wid, WM_SETFONT, (WPARAM)FontFixedWidth, 1);
				} // if
			if (Wid = GetWidgetFromID(IDC_GPS_FILEPREVIEW2))
				{
				SendMessage(Wid, WM_SETFONT, (WPARAM)FontFixedWidth, 1);
				} // if
			if (Wid = GetWidgetFromID(IDC_GPS_FILEPREVIEW2D))
				{
				SendMessage(Wid, WM_SETFONT, (WPARAM)FontFixedWidth, 1);
				} // if
			if (Wid = GetWidgetFromID(IDC_GPS_FILEPREVIEW3))
				{
				SendMessage(Wid, WM_SETFONT, (WPARAM)FontFixedWidth, 1);
				} // if
			} // if

		ConfigureWidgets();
		SelectPanel(IDD_IMWIZ_IDENT);

		} // if NativeWin
	} // if !NativeWin

return NativeWin;

} // ImportWizGUI::Construct

/*===========================================================================*/

ImportWizGUI::ImportWizGUI(Database *DBSource, Project *ProjSource, EffectsLib *EffectsSource, MessageLog *LogInit, bool ShowGUI)
: GUIFenetre('IMWZ', this, "Import Wizard") // Yes, I know...
{
FileReq *FR;
const char *loadname;
char LoadName[256+32], msg[256+34];
static char LastPath[256] = "";
char loading[32], thisload[32];
UBYTE load_as;
short i = 0;
unsigned char again = true;
HeadPier = NULL;
DXFinfo = NULL;
Pier1 *LastImport = NULL;
static NotifyTag AllEvents[] = {MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff), 0};

//long var1 = ftol(3.14);
//long var2 = ftol(3.95);
//long var3 = ftol(-3.14);
//long var4 = ftol(-3.95);

//double sealme = 2.1043327852243606e-009;
//double result = quickdblceil(sealme);

/***
static double magicdnum2 = 6755399441055744.0;	// 2^52 * 1.5
//static double magicdelta2 = 1.5e-11;
//static double magicround2 = 0.5 - magicdelta2;
static double magicround2 = 0.49999999999999994;	// as close to .5 as we can get

double qif = 27.999999995459518;
//int result = quickintfloor(qif);

double qif2 = 27.999999999999996;
double step1 = qif2 - magicround2;
double step2 = step1 + magicdnum2;
double qif3 = 27.499999;
double qif4 = qif2 - qif3;
***/

//double qdf = 39.999999999999943;
//double result = quickdblfloor(qdf);

//boost::thread my_thread(&hello_world);
//my_thread.join();

/***
double test1 = 3.14159, test2 = -3.14159, test3 = 7.9999, test4 = 7.0001, test5 = -7.9999, test6 = -7.0001, test7 = 11.0;
int test0;

test0 = quickdblceil(test1);
test0 = quickdblceil(test2);
test0 = quickdblceil(test3);
test0 = quickdblceil(test4);
test0 = quickdblceil(test5);
test0 = quickdblceil(test6);
test0 = quickdblceil(test7);
***/

ConstructError = 0;
FontFixedWidth = NULL;
minxhrs = minyhrs = DBL_MAX;
UseGUI = ShowGUI;
Templated = NULL;
RandomCP = false;
Num2Load = NumLoaded = 0;
if (TC = new ControlPointDatum())
	{
	CurDat = TC;
	} // if
else
	{
	ConstructError = 1;
	return;
	} // else

loading[0] = thisload[0] = NULL;
MergeLoad = false;
GlobalApp->AppEx->RegisterClient(this, AllEvents);

ProjHost = ProjSource;
DBHost = DBSource;
EffectsHost = EffectsSource;
LocalLog = LogInit;

if (UseGUI)
	{
	FR_loop:
	if (FR = new FileReq)
		{
		FR->SetTitle("Import Data");
		FR->SetDefPat(WCS_REQUESTER_WILDCARD);
		strcpy(LastPath, ProjHost->importdatapath);
		if (LastPath[0] == 0 || chdir(ProjHost->MungPath(ProjHost->importdatapath)))
			FR->SetDefPath("WCSProjects:");
		else
			FR->SetDefPath(LastPath);
		if (FR->Request(WCS_REQUESTER_FILE_MULTI))
			{
			while (loadname = FR->GetNextName())
				{
				if (i++ == 0)
					{
					if (!(HeadPier = new Pier1))
						{
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to create new Import object - aborting.");
						ConstructError = 1;
						delete FR;
						return;
						} // if
					Importing = HeadPier;
					} // if
				else
					{
					if (!(Importing->Next = new Pier1))
						{
						GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Unable to create new Import object - aborting.");
						// deallocate all
						while (HeadPier)
							{
							Importing = HeadPier;
							HeadPier = HeadPier->Next;
							delete Importing;
							} // while
						ConstructError = 1;
						delete FR;
						return;
						} // if
					LastImport = Importing;
					Importing = Importing->Next;
					} // else
				strcpy(LoadName, loadname);
				Snoopy(LoadName, &thisload[0], &load_as);
				strcpy(Importing->OutDir, ProjHost->dirname);
				strcpy(LastPath, Importing->InDir);
				strcpy(ProjHost->importdatapath, LastPath);
				Num2Load++;

				if (i == 1)		// we are only going to load data of similar types, set the first type
					strcpy(&loading[0], &thisload[0]);
				else if (strcmp(&loading[0], &thisload[0]) != 0)
					{
					sprintf(msg, "You can only import one type of data at a time. Skipping '%s'", LoadName);
					UserMessageOK("Import Wizard Error", msg);
					delete Importing;
					Importing = LastImport;
					Importing->Next = NULL;
					Num2Load--;
					} // else if
				} // while
			} // if
		else
			{
			if (i == 0)		// user cancelled on first requester
				ConstructError = 1;
			again = false;	// done multiloading
			} // else
		delete FR;
		} // if
	else
		ConstructError = 1;

	// allow muliple requesters for SDTS & USGS data so they can load from different directories
	if ((strcmp(thisload, "SDTS_DEM") == 0) && again)
		again = UserMessageYN("Import Wizard", "Import more SDTS DEMs from another directory?");
	else if ((strcmp(thisload, "USGS_DEM") == 0) && again)
		again = UserMessageYN("Import Wizard", "Import more USGS DEMs from another directory?");
	else
		again = false;

	if (again)
		goto FR_loop;

	if (! ConstructError)
		{
		Importing = HeadPier;	// reset to the beginning
		SetPanelOrder();
		SetWinManFlags(WCS_FENETRE_WINMAN_NODOCK | WCS_FENETRE_WINMAN_NOPOPUP);
		} // if
	} // if UseGUI

if (((strcmp(thisload, "SDTS_DEM") == 0) || (strcmp(thisload, "USGS_DEM") == 0)) && (Num2Load > 1))
	MergeLoad = true;

} // ImportWizGUI::ImportWizGUI

/*===========================================================================*/

//lint -save -e1740
ImportWizGUI::~ImportWizGUI()
{
Pier1 *TempImport, **TemplatedPtr, *Disposable = NULL, **DisposablePtr;

GlobalApp->AppEx->RemoveClient(this);

TC->DeleteAllChildren();
delete TC;

if (DXFinfo)
	AppMem_Free(DXFinfo, sizeof (struct ImportData));

// Pier1 list may have been deleted in DoLoad()
if (HeadPier)
	{
	// sort into two lists, auto-imports and disposable
	Templated = NULL;
	TemplatedPtr = &Templated;
	DisposablePtr = &Disposable;
	Importing = HeadPier;
	while (Importing)
		{
		TempImport = Importing;
		Importing = Importing->Next;
		TempImport->Next = NULL;
		if (TempImport->Automatic)
			{
			*TemplatedPtr = TempImport;
			TemplatedPtr = &(*TemplatedPtr)->Next;
			} // if
		else
			{
			*DisposablePtr = TempImport;
			DisposablePtr = &(*DisposablePtr)->Next;
			} // else
		} // while

	while (Disposable) // take out the trash
		{
		Importing = Disposable;
		Disposable = Disposable->Next;
		delete Importing;
		} // while
	HeadPier = Importing = NULL; // for form's sake
	} // if

if (Templated && UseGUI)
	ProjHost->AddPier(Templated);

GlobalApp->MCP->RemoveWindowFromMenuList(this);

ProjHost = NULL;
DBHost = NULL;
LocalLog = NULL;
if (RUTMZone != 0) // did somebody set the region load UTMZone?
	GlobalApp->MainProj->Prefs.LastUTMZone = RUTMZone;	// yes, so save it

if (FontFixedWidth)
	{
	DeleteObject((HGDIOBJ)FontFixedWidth);
	} // if

} // ImportWizGUI::~ImportWizGUI
//lint -restore

/*===========================================================================*/

void ImportWizGUI::ConfigureWidgets(void)
{
long i, position;
bool AddFormat;

//ShowID(Importing->MyIdentity);

ConfigureTB(NativeWin, IDC_TBHOREX_LOCKN, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBHOREX_LOCKS, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBHOREX_LOCKE, IDI_ICONLOCKINTERSM, NULL);
ConfigureTB(NativeWin, IDC_TBHOREX_LOCKW, IDI_ICONLOCKINTERSM, NULL);

ConfigureSC(NativeWin, IDC_CHECKGLOBALWRAP, &Importing->WrapData, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCDISABLENULLTILES, &Importing->HideNULLed, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCHASNULLS, &Importing->HasNulls, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCRENDERENABLED, &Importing->DBRenderFlag, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSWAPFACTORS, &Importing->SwapRC, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSPLINECONSTRAIN, &Importing->SplineConstrain, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCVERTEXINVDATUM, &Importing->InvertData, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_POSEAST, &Importing->PosEast, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_DOLAYERS, &Importing->ShapeLayers, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_DONAMES, &Importing->ShapeDBNames, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_DOELEVS, &Importing->ShapeElevs, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_LOADATTR, &Importing->ShapeAttribs, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_CONFORM, &Importing->Conform, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCSHAPEOPTS_FILELAYER, &Importing->FileLayer, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCOUTTYPE_TEMPLATE, &Importing->Automatic, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_GPS_DEL_TAB, &Importing->gpsDTab, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_GPS_DEL_SEMI, &Importing->gpsDSemicolon, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_GPS_DEL_COMMA, &Importing->gpsDComma, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_GPS_DEL_SPACE, &Importing->gpsDSpace, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_GPS_DEL_OTHER, &Importing->gpsDOther, SCFlag_Char, NULL, NULL);

ConfigureFI(NativeWin, IDC_FIOUTSCALEOVV1IN, &Importing->ScaleOVIn, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALEV1OUT, &Importing->ScaleOVOut, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALEOVSET, &Importing->ScaleOVSet, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALETVV1INPUT, &Importing->ScaleTVV1In, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALETVV1OUTPUT, &Importing->ScaleTVV1Out, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALETVV2INPUT, &Importing->ScaleTVV2In, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALETVV2OUTPUT, &Importing->ScaleTVV2Out, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FINULLVAL, &Importing->NullVal, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);

(void)WidgetCBAddEnd(IDC_CBICRHEAD_BINFACTORS, " ");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBELEVUNITS_ELEVUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_HUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Centimeters");
//(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Millimeters");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Miles");
//(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Yards");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBCOORDSYS_VUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_HUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_VUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "1 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "2 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "3 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "4 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "5 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "6 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "7 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "8 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "9 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "10 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "11 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "12 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "13 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "14 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "15 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "16 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "17 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "18 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "19 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "20 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "21 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "22 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "23 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "24 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "25 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "26 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "27 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "28 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "29 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "30 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "31 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "32 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "33 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "34 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "35 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "36 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "37 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "38 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "39 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "40 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "41 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "42 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "43 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "44 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "45 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "46 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "47 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "48 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "49 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "50 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "51 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "52 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "53 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "54 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "55 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "56 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "57 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "58 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "59 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "60 North");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "1 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "2 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "3 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "4 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "5 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "6 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "7 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "8 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "9 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "10 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "11 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "12 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "13 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "14 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "15 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "16 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "17 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "18 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "19 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "20 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "21 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "22 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "23 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "24 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "25 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "26 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "27 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "28 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "29 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "30 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "31 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "32 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "33 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "34 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "35 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "36 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "37 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "38 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "39 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "40 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "41 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "42 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "43 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "44 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "45 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "46 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "47 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "48 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "49 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "50 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "51 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "52 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "53 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "54 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "55 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "56 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "57 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "58 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "59 South");
(void)WidgetCBAddEnd(IDC_CBWCSUNITS_UTMZONE, "60 South");
(void)WidgetCBAddEnd(IDC_GPS_TEXTQUAL, " \" ");
(void)WidgetCBAddEnd(IDC_GPS_TEXTQUAL, " ' ");
(void)WidgetCBAddEnd(IDC_GPS_TEXTQUAL, "< none >");

// set up things related to the input formats drop box
#ifdef WCS_BUILD_VNS
AddFormat = true;
#endif // WCS_BUILD_VNS
for (i = 0; i < IW_INPUT_MAX_FORMATS; i++)
	{
	#ifndef WCS_BUILD_VNS
	AddFormat = FormatInWCS[i];
	#endif // !WCS_BUILD_VNS
	if (AddFormat)
		{
		position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, FormatNames[i]);
		WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)FormatIdents[i]);
		FormatPos[FormatIdents[i]] = (short)position;
		} // if
	else
		FormatPos[FormatIdents[i]] = -1;
	} // for

/***
position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Arc ASCII Array");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_ARCASCII);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Arc Grid");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_ARCBINARYADF_GRID);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Arc Export Grid");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_ARCEXPORT_DEM);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "ASCII Array");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_ASCII_ARRAY);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Binary Array");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_BINARY);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Bryce Terrain");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_BRYCE);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "DTED");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_DTED);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "DXF");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_DXF);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "GTOPO30");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_GTOPO30);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - BMP");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_BMP);

#ifdef WCS_BUILD_VNS
position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - ECW");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_ECW);
#endif // WCS_BUILD_VNS

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - IFF");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_IFF);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - JPEG");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_JPEG);

#ifdef WCS_BUILD_VNS
//position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - MrSID");
//WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_MR_SID);
#endif // WCS_BUILD_VNS

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - Pict");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_PICT);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - PNG");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_PNG);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - Targa");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_TARGA);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Image - TIFF");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_IMAGE_TIFF);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "MicroDEM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_MDEM);

#ifdef WCS_BUILD_VNS
position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "NTF DTM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_NTF_DTM);

//position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "NTF MERIDIAN2");
//WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_NTF_MERIDIAN2);
#endif // WCS_BUILD_VNS

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "SDTS DEM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_SDTS_DEM);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "SDTS DLG");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_SDTS_DLG);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "ShapeFile");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_SHAPE);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "STM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_STM);

#ifdef GO_SURFING
position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Surfer Grid");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_SURFER);
#endif // GO_SURFING

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "Terragen Terrain");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_TERRAGEN);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "USGS ASCII DEM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_USGS_ASCII_DEM);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "USGS DLG");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_USGS_ASCII_DLG);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "VistaPro DEM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_VISTAPRO);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "WCS DEM");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_WCS_DEM);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "WCS XYZ");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_WCS_XYZ);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "WCS ZBuffer");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_WCS_ZBUFFER);

position = WidgetCBAddEnd(IDC_CBIDENT_OVERRIDE, "XYZ");
WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)IW_INPUT_XYZ);
***/

(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBVERTEX_ELEVUNITS, "Inches");
(void)WidgetCBAddEnd(IDC_CBHOREX_BOUNDS, "Degrees");
(void)WidgetCBAddEnd(IDC_CBHOREX_BOUNDS, "Linear Units");
#ifdef WCS_BUILD_VNS
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Degrees");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Linear Units");
#else // WCS_BUILD_VNS
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Degrees");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Kilometers");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Meters");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Decimeters");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Centimeters");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Miles");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "International Foot");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "U.S. Survey Foot");
(void)WidgetCBAddEnd(IDC_CBHOREX_UNITS, "Inches");
#endif // WCS_BUILD_VNS

WidgetCBSetCurSel(IDC_CBBINSET_BYTEORDER, Importing->ByteOrder);
WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
WidgetCBSetCurSel(IDC_CBBINSET_VALUEFORMAT, IMWIZ_DATA_FORMAT_SIGNEDINT);
WidgetCBSetCurSel(IDC_CBCELLORDER_MAJORAXIS, 0);
WidgetCBSetCurSel(IDC_CBCELLORDER_MINORAXIS, 0);
WidgetCBSetCurSel(IDC_CBICRHEAD_BINFACTORS, 0);
WidgetCBSetCurSel(IDC_CBCOORDSYS_HUNITS, Importing->GridUnits);
#ifdef WCS_BUILD_VNS
WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 1);			// Linear
#else // WCS_BUILD_VNS
WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 0);			// Degrees
#endif // WCS_BUILD_VNS
WidgetCBSetCurSel(IDC_CBOUTFORM_VALUEBYTES, 0);
WidgetCBSetCurSel(IDC_CBOUTFORM_VALUEFORMAT, IMWIZ_DATA_FORMAT_SIGNEDINT);
WidgetCBSetCurSel(IDC_CBOUTSCALEOVTYPE, 0);
WidgetCBSetCurSel(IDC_CBVERTEX_ELEVUNITS, 1);		// Meters
WidgetCBSetCurSel(IDC_CBELEVUNITS_ELEVUNITS, 1);	// Meters
WidgetCBSetCurSel(IDC_CBWCSUNITS_UTMZONE, 0);		// 1 North
WidgetCBSetCurSel(IDC_CBWCSUNITS_HUNITS, 1);		// Meters
WidgetCBSetCurSel(IDC_CBWCSUNITS_VUNITS, 1);		// Meters

SCALEOP = 2;
ConfigureSR(NativeWin, IDC_SROUTSCALETWOVAL, IDC_SROUTSCALETWOVAL, &Importing->ScaleOp, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_SROUTSCALETWOVAL, IDC_SROUTSCALEONEVAL, &Importing->ScaleOp, SRFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_SROUTSCALETWOVAL, IDC_SROUTSCALEMINMAX, &Importing->ScaleOp, SRFlag_Char, 2, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRBINSETBIL, IDC_SRBINSETBIL, &Importing->BandStyle, SRFlag_Char, IMWIZ_DATA_LAYOUT_BIL, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRBINSETBIL, IDC_SRBINSETBIP, &Importing->BandStyle, SRFlag_Char, IMWIZ_DATA_LAYOUT_BIP, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRBINSETBIL, IDC_SRBINSETBSQ, &Importing->BandStyle, SRFlag_Char, IMWIZ_DATA_LAYOUT_BSQ, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRNULLEQ, IDC_SRNULLEQ, &Importing->NullMethod, SRFlag_Char, IMWIZ_NULL_EQ, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRNULLEQ, IDC_SRNULLLE, &Importing->NullMethod, SRFlag_Char, IMWIZ_NULL_LE, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRNULLEQ, IDC_SRNULLGE, &Importing->NullMethod, SRFlag_Char, IMWIZ_NULL_GE, NULL, NULL);

knowhow = 0;

WidgetSetDisabled(IDC_PREV, 1);
WidgetSetDisabled(IDC_NEXT, 0);
WidgetSetDisabled(IDCANCEL, 0);

WidgetSetDisabled(IDC_BUTTONVERTEXTEST, 1);

WidgetShow(IDC_BUTTONIDENT_UNARCHELP, 1);
WidgetSetText(IDC_BUTTONIDENT_UNARCHELP, "File format reference");

if (Importing->Signal & DEAD_END)
	WidgetSetDisabled(IDC_NEXT, 1);

} // ImportWizGUI::ConfigureWidgets

/*===========================================================================*/

void ImportWizGUI::HandleNotifyEvent(void)
{

// this function does nothing unless we're running VNS
#ifdef WCS_BUILD_VNS
GeneralEffect *matchEffect, *myEffect;
NotifyTag *Changes, Interested[3];

//int Done = 0;
long curPos, done, pos;

if (! NativeWin)
	return;
Changes = Activity->ChangeNotify->ChangeList;

Interested[0] = MAKE_ID(Importing->IWCoordSys.GetNotifyClass(), 0xff, 0xff, 0xff);
Interested[1] = NULL;
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
	SyncCoordSys();

Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, 0xff);
if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 1))
	{
	curPos = -1;
	matchEffect = &Importing->IWCoordSys;
	WidgetCBClear(IDC_IMWIZ_CBCOORDS);
	WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, "New Coordinate System...");
	for (myEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
		{
		pos = WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, myEffect->GetName());
		WidgetCBSetItemData(IDC_IMWIZ_CBCOORDS, pos, myEffect);
		if (Importing->IWCoordSys.Equals((CoordSys *)myEffect))
			curPos = pos;
		} // for
	// add a new CoordSys to the list if it hasn't been found in the effects list
	if (curPos < 0)
		{
		curPos = WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, matchEffect->GetName());
		WidgetCBSetItemData(IDC_IMWIZ_CBCOORDS, curPos, matchEffect);
		} // if
	WidgetCBSetCurSel(IDC_IMWIZ_CBCOORDS, curPos);
	Interested[0] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
	Interested[1] = MAKE_ID(WCS_EFFECTSSUBCLASS_COORDSYS, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
	Interested[2] = NULL;
	if (GlobalApp->AppEx->MatchNotifyClass(Interested, Changes, 0))
		{
		done = 1;
		} // if
	} // if Coordinate System name changed

if (! done)
	ConfigureWidgets();
#endif // WCS_BUILD_VNS

} // ImportWizGUI::HandleNotifyEvent

/*===========================================================================*/

long ImportWizGUI::HandleListViewColSel(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
long ActiveColumn = -1, pResult;
char disability;

switch (CtrlID)
	{
	case IDC_GPS_FILEPREVIEW3:
		ActiveColumn = WidgetColumnedListViewGetActiveColumn(IDC_GPS_FILEPREVIEW3);
		Importing->gpsColumn = (char)ActiveColumn;
		pResult = ParseField(ActiveColumn);
		WidgetCBSetCurSel(IDC_GPS_FIELDIS, pResult);
		disability = (6 != pResult);
		UpdateGPSAttribName();
		WidgetSetDisabled(IDC_GPS_ATTRIBNAME, disability);
		// <<<>>> Frank, do something with ActiveColumn here
		// You can also set the active column with
		// WidgetColumnedListViewSetActiveColumn(IDC_GPS_FILEPREVIEW1, long ActiveCol);
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // ImportWizGUI::HandleListViewColSel

/*===========================================================================*/

long ImportWizGUI::HandleStringEdit(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_EDITSHAPEOPTS_FILELAYERNAME:
		DoNewShapeLayerName();
		break;
	case IDC_EDITOUTTYPEOUTNAME:
		DoNewOutputName();
		break;
	case IDC_GPS_ATTRIBNAME:
		DoGPSAttribName();
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // ImportWizGUI::HandleStringEdit

/*===========================================================================*/

long ImportWizGUI::HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GPS_CONSECDELIM:
		break;
	case IDC_GPS_DATUMLINEENABLED:
		WidgetSetDisabled(IDC_GPS_DATUMINLINE, !Importing->gpsHasDatum);
		break;
	case IDC_GPS_DEL_OTHER:
		WidgetSetDisabled(IDC_OTHERDELIM, (char)(Importing->gpsDOther == 0));
		break;
	case IDC_SCOUTTYPE_TEMPLATE:
		break;
	case IDC_SCSWAPFACTORS:
		DoBinFactors();
		break;
	case IDC_SCHASNULLS:
		WidgetSetDisabled(IDC_FINULLVAL, (char)(Importing->HasNulls == 0));
		WidgetSetDisabled(IDC_SRNULLLE, (char)(Importing->HasNulls == 0));
		WidgetSetDisabled(IDC_SRNULLEQ, (char)(Importing->HasNulls == 0));
		WidgetSetDisabled(IDC_SRNULLGE, (char)(Importing->HasNulls == 0));
		break;
	case IDC_SCCEILING:
		WidgetSetDisabledRO(IDC_FICEILINGVAL, (char)!(Importing->UseCeiling));
		break;
	case IDC_SCFLOOR:
		WidgetSetDisabledRO(IDC_FIFLOORVAL, (char)!(Importing->UseFloor));
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // ImportWizGUI::HandleSCChange

/*===========================================================================*/

long ImportWizGUI::HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_GPS_DELIMITED:
	case IDC_GPS_FIXEDWIDTH:
		break;
	case IDC_GPS_DEL_COMMA:
	case IDC_GPS_DEL_OTHER:
	case IDC_GPS_DEL_SEMI:
	case IDC_GPS_DEL_SPACE:
	case IDC_GPS_DEL_TAB:
		break;
	case IDC_LOADASDEM:
	case IDC_LOADASCP:
	case IDC_LOADASIMAGE:
	case IDC_LOADASVECTOR:
		Importing->OutFormat = DEM_DATA2_OUTPUT_UNSET;
		break;
	case IDC_SCHORUNITS_ARBITRARY:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		Importing->AllowPosE = true;
		break;
	case IDC_SCHORUNITS_LATLON:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 0);	// Degrees
		Importing->AllowPosE = true;
		break;
	case IDC_SCHORUNITS_UTM:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		Importing->AllowPosE = false;
		break;
	case IDC_SRCOLEL_ADD:
	case IDC_SRCOLEL_16BIT:
	case IDC_SRCOLEL_24BIT:
		switch (CtrlID)
			{
			default: break;
			case IDC_SRCOLEL_ADD:
				Importing->Flags = ELEV_METHOD_ADD;
				break;
			case IDC_SRCOLEL_16BIT:
				Importing->Flags = ELEV_METHOD_16BIT;
				break;
			case IDC_SRCOLEL_24BIT:
				Importing->Flags = ELEV_METHOD_24BIT;
				break;
			} // switch
		SetElevMethodHeights();
		break;
	case IDC_SRGRID_CP:
	case IDC_SRLOAD_CP:
		// nothing done here - smartwidget update only
		break;
	case IDC_SROUTSCALEMINMAX:
		SCALEOP = DEM_DATA_SCALEOP_MAXMINSCALE;
		SelectPanel(IDD_IMWIZ_OUTSCALE);
		break;
	case IDC_SROUTSCALEONEVAL:
		SCALEOP = DEM_DATA_SCALEOP_MATCHSCALE;
		SelectPanel(IDD_IMWIZ_OUTSCALE);
		break;
	case IDC_SROUTSCALETWOVAL:
		SCALEOP = DEM_DATA_SCALEOP_MATCHMATCH;
		SelectPanel(IDD_IMWIZ_OUTSCALE);
		break;
	case IDC_SRKNOWLOAD:
	case IDC_SRKNOWCHANGE:
		if (knowhow == 0)
			WidgetSetText(IDC_NEXT, "&Import");
		else
			WidgetSetText(IDC_NEXT, "&Next -->");
		break;
	case IDC_SRWCSUNITS_ARBITRARY:
		Importing->AllowPosE = true;
		WidgetSetDisabled(IDC_CBWCSUNITS_HUNITS, false);
		WidgetSetDisabled(IDC_CBWCSUNITS_UTMZONE, true);
		break;
	case IDC_SRWCSUNITS_LATLON:
		Importing->AllowPosE = true;
		WidgetSetDisabled(IDC_CBWCSUNITS_HUNITS, true);
		WidgetSetDisabled(IDC_CBWCSUNITS_UTMZONE, true);
		break;
	case IDC_SRWCSUNITS_UTM:
		Importing->PosEast = true;
		Importing->AllowPosE = false;
		WidgetSetDisabled(IDC_CBWCSUNITS_HUNITS, false);
		WidgetSetDisabled(IDC_CBWCSUNITS_UTMZONE, false);
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // ImportWizGUI::HandleSRChange

/*===========================================================================*/

long ImportWizGUI::HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID)
{
char checked;

switch (ButtonID)
	{
#ifdef WCS_BUILD_VNS
//	case IDC_BUTCOORDSYS_MODIFY:
//		ChangeCoordSys();
//		break;
#endif
	case IDCANCEL:
		(void)HandleCloseWin(NW);
		break;
	case IDC_NEXT:
		NextButton(NW);
		break;
	case IDC_PREV:
		PrevButton();
		break;
	case IDC_BUTTONDOTEST:
		DoTestButton(NW);
		break;
	case IDC_BUTTONDOVERTEXPANEL:
		ShowPanel(0, -1);	// one or both of these isn't needed
		ShowPanel(1, -1);
		SelectPanel(IDD_IMWIZ_VERTEX);
		break;
	case IDC_BUTTONDOSCALEPANEL:
		ShowPanel(0, -1);	// one or both of these isn't needed
		ShowPanel(1, -1);
		SelectPanel(IDD_IMWIZ_OUTSCALE);
		break;
	case IDC_BUTTONIDENT_UNARCHELP:
		if ((strcmp(whatitis, "GZIP") == 0) || (strcmp(whatitis, "ZIP") == 0))
			(void)GlobalApp->HelpSys->OpenHelpTopic('ZIPS');
		else
			(void)GlobalApp->HelpSys->OpenHelpTopic('IMWZ');
		break;
	case IDC_BUTTONVERTEXTEST:
//		DoTestButton(NW);
		break;
	case IDC_IMWIZ_BEDITCOORDS:
		if (&Importing->IWCoordSys)
			Importing->IWCoordSys.EditRAHost();
		SetMetrics();
		break;
	case IDC_TBHOREX_LOCKN:
		checked = WidgetGetCheck(IDC_TBHOREX_LOCKN);
		WidgetEMSetReadOnly(IDC_FIHOREXN, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBHOREX_LOCKS)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_FIHOREXS, 0);
			WidgetSetCheck(IDC_TBHOREX_LOCKS, 0);
			}
		break;
	case IDC_TBHOREX_LOCKS:
		checked = WidgetGetCheck(IDC_TBHOREX_LOCKS);
		WidgetEMSetReadOnly(IDC_FIHOREXS, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBHOREX_LOCKN)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_FIHOREXN, 0);
			WidgetSetCheck(IDC_TBHOREX_LOCKN, 0);
			}
		break;
	case IDC_TBHOREX_LOCKE:
		checked = WidgetGetCheck(IDC_TBHOREX_LOCKE);
		WidgetEMSetReadOnly(IDC_FIHOREXE, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBHOREX_LOCKW)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_FIHOREXW, 0);
			WidgetSetCheck(IDC_TBHOREX_LOCKW, 0);
			}
		break;
	case IDC_TBHOREX_LOCKW:
		checked = WidgetGetCheck(IDC_TBHOREX_LOCKW);
		WidgetEMSetReadOnly(IDC_FIHOREXW, (int)checked);
		if (checked && (WidgetGetCheck(IDC_TBHOREX_LOCKE)))	// flip other off if enabled
			{
			WidgetEMSetReadOnly(IDC_FIHOREXE, 0);
			WidgetSetCheck(IDC_TBHOREX_LOCKE, 0);
			}
		break;
	default:
		break;
	} // switch ButtonID

return(0);

} // ImportWizGUI::HandleButtonClick

/*===========================================================================*/

long ImportWizGUI::HandleDDChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

//switch (CtrlID)
//	{
//	default: break;
//	} // switch CtrlID

return(0);

} // ImportWizGUI::HandleDDChange

/*===========================================================================*/

long ImportWizGUI::HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{
char restxt[80];

switch (CtrlID)
	{
	case IDC_BINHEADERSIZE:
		DoBinHdrSize();
		break;
	case IDC_FIOUTDEMNS:	// nothing to do
	case IDC_FIOUTDEMWE:	// for these two
		break;
	case IDC_FIDEMOUTCOLS:
	case IDC_FIDEMOUTROWS:
		if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
			{
			Importing->OutDEMWE = (Importing->OutRows - 1) / 301 + 1;
			Importing->OutDEMNS = (Importing->OutCols - 1) / 301 + 1;
			}
		else
			{
			Importing->OutDEMWE = (Importing->OutCols - 1) / 301 + 1;
			Importing->OutDEMNS = (Importing->OutRows - 1) / 301 + 1;
			}
		WidgetFISync(IDC_FIOUTDEMWE, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIOUTDEMNS, WP_FISYNC_NONOTIFY);
		SetGridSpacing();
		if (OUTPUT_COLS == 0)
			{
			OUTPUT_COLS = INPUT_COLS;
			WidgetFISync(IDC_FIDEMOUTCOLS, WP_FISYNC_NONOTIFY);
			}
		if (OUTPUT_ROWS == 0)
			{
			OUTPUT_ROWS = INPUT_ROWS;
			WidgetFISync(IDC_FIDEMOUTROWS, WP_FISYNC_NONOTIFY);
			}
		if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
			{
			if (Importing->KeepBounds)
				{
				sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE * Importing->InRows / Importing->OutRows);
				restxt[33] = 0;	// truncate any numbers past 7 digits
				WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
				sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS * Importing->InCols / Importing->OutCols);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
				}
			else
				{
				sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
				sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
				}
			} // InFormat == DEM_DATA2_INPUT_WCSDEM
		else
			{
			if (Importing->KeepBounds)
				{
				sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE);	// * Importing->InCols / Importing->OutCols);
				restxt[33] = 0;	// truncate any numbers past 7 digits
				WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
				sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS);	// * Importing->InRows / Importing->OutRows);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
				}
			else
				{
				sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE);	// * CROP_COLS / Importing->OutCols);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
				sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS);	// * CROP_ROWS / Importing->OutRows);
				restxt[33] = 0;
				WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
				}
			} // InFormat != DEM_DATA2_INPUT_WCSDEM
		if ((CROP_COLS != OUTPUT_COLS) || (CROP_ROWS != OUTPUT_ROWS))
			WidgetSetDisabled(IDC_SCSPLINECONSTRAIN, false);
		else
			WidgetSetDisabled(IDC_SCSPLINECONSTRAIN, true);
		break;
	case IDC_FIVERTEXHIGHEL:
		Importing->UserMaxSet = true;
		break;
	case IDC_FIVERTEXLOWEL:
		Importing->UserMinSet = true;
		break;
	case IDC_FIHOREXGRIDNS:
	case IDC_FIHOREXGRIDWE:
	case IDC_FIHOREXGRIDSIZENS:
	case IDC_FIHOREXGRIDSIZEWE:
	case IDC_FIHOREXHEIGHT:
	case IDC_FIHOREXWIDTH:
		Importing->GridSpaceNS = AnimPar[IW_ANIMPAR_GRIDNS].CurValue;
		Importing->GridSpaceWE = AnimPar[IW_ANIMPAR_GRIDWE].CurValue;
		IWGridSizeNS = AnimPar[IW_ANIMPAR_GRIDSIZENS].CurValue;
		IWGridSizeWE = AnimPar[IW_ANIMPAR_GRIDSIZEWE].CurValue;
		Importing->InHeight = AnimPar[IW_ANIMPAR_HEIGHT].CurValue;
		Importing->InWidth = AnimPar[IW_ANIMPAR_WIDTH].CurValue;
		DoHorExChange(CtrlID);
		break;
	case IDC_FIHOREXN:
	case IDC_FIHOREXS:
		Importing->NBound = AnimPar[IW_ANIMPAR_N].CurValue;
		Importing->SBound = AnimPar[IW_ANIMPAR_S].CurValue;
//		if ((CtrlID == IDC_FIHOREXS) && (Importing->SBound >= Importing->NBound))
//			{
//			Importing->NBound = Importing->SBound + Importing->InHeight;
//			WidgetFISync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
//			}
//		else
//			Importing->InHeight = Importing->NBound - Importing->SBound;
		DoHorExChange(CtrlID);
		break;
	case IDC_FIHOREXE:
	case IDC_FIHOREXW:
		Importing->EBound = AnimPar[IW_ANIMPAR_E].CurValue;
		Importing->WBound = AnimPar[IW_ANIMPAR_W].CurValue;
//		if ((CtrlID == IDC_FIHOREXE) && (Importing->EBound >= Importing->WBound))
//			{
//			Importing->WBound = Importing->EBound + Importing->InWidth;
//			WidgetFISync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
//			}
//		else
//			Importing->InWidth = Importing->WBound - Importing->EBound;
		DoHorExChange(CtrlID);
		break;
	default:
		break;
	} // switch CtrlID

return(0);

} // ImportWizGUI::HandleFIChange

/*===========================================================================*/

long ImportWizGUI::HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID)
{

switch (CtrlID)
	{
	case IDC_CBICRHEAD_BINFACTORS:
		DoBinFactors();
		break;
	case IDC_CBBINSET_BYTEORDER:
		Importing->ByteOrder = (UBYTE)WidgetCBGetCurSel(IDC_CBBINSET_BYTEORDER);
		break;
	case IDC_CBCOORDSYS_HUNITS:
		SetHScale(WidgetCBGetCurSel(IDC_CBCOORDSYS_HUNITS));
		break;
	case IDC_CBCOORDSYS_VUNITS:
		SetVScale(WidgetCBGetCurSel(IDC_CBCOORDSYS_VUNITS));
		break;
	case IDC_CBELEVUNITS_ELEVUNITS:
		SetVScale(WidgetCBGetCurSel(IDC_CBELEVUNITS_ELEVUNITS));
		break;
	case IDC_CBIDENT_OVERRIDE:
		DoOverride();
		break;
	case IDC_CBOUTTYPE_OUTFORMAT:
		DoNameBaseExtension();
		WidgetSetText(IDC_EDITOUTTYPEOUTNAME, Importing->NameBase);
		break;
	case IDC_CBVERTEX_ELEVUNITS:
		SetElevUnits();
		SetVScale(WidgetCBGetCurSel(IDC_CBVERTEX_ELEVUNITS));
		break;
	case IDC_CBHOREX_BOUNDS:
		Importing->BoundsType = (short)WidgetCBGetCurSel(IDC_CBHOREX_BOUNDS);
		if (Importing->BoundsType == VNS_BOUNDSTYPE_LINEAR)
			{
			Importing->GridUnits = VNS_IMPORTDATA_HUNITS_LINEAR;
			WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 1);	// Linear
			WidgetSetDisabled(IDC_CBHOREX_UNITS, 1);
			}
		else
			WidgetSetDisabled(IDC_CBHOREX_UNITS, 0);
//		if (WidgetCBGetCurSel(IDC_CBHOREX_BOUNDS) == 0)
//			{
//			Importing->BoundsType = VNS_BOUNDSTYPE_DEGREES;
//			WidgetShow(IDC_FIHOREXWIDTH, 1);
//			WidgetShow(IDC_FIHOREXHEIGHT, 1);
//			}
//		else
//			{
//			Importing->BoundsType = VNS_BOUNDSTYPE_LINEAR;
//			WidgetShow(IDC_FIHOREXWIDTH, 0);
//			WidgetShow(IDC_FIHOREXHEIGHT, 0);
//			}
		SetMetrics();
		DoHorExLabels();
		SyncGrid();
		break;
	case IDC_CBHOREX_UNITS:
		Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
#ifdef WCS_BUILD_VNS
		Importing->GridUnits += VNS_IMPORTDATA_HUNITS_DEGREES;
#endif // WCS_BUILD_VNS
		SetGridSpacing();
		SetMetrics();
		SyncGrid();
		break;
	case IDC_CBCELLORDER_MAJORAXIS:
	case IDC_CBCELLORDER_MINORAXIS:
		/*** F2 NOTE: Fix this ***/
		UserMessageOK("Be Alert - the world needs lerts", "You press the button, yet nothing happens");
		break;
	case IDC_CBBINSET_VALUEBYTES:
		DoValBytes();
		break;
	case IDC_CBBINSET_VALUEFORMAT:
		DoValFmt();
		break;
	case IDC_CBOUTFORM_VALUEBYTES:
		DoOutValueBytes();
		break;
	case IDC_CBOUTFORM_VALUEFORMAT:
		DoOutValueFormat();
		break;
	case IDC_CBOUTSCALEOVTYPE:
		Importing->ScaleType = (char)WidgetCBGetCurSel(IDC_CBOUTSCALEOVTYPE);
		break;
	case IDC_CBWCSUNITS_HUNITS:
		// SetHScale(WidgetCBGetCurSel(IDC_CBWCSUNITS_HUNITS));	// this really should be here, but DXF goes wonky if enabled
		break;
	case IDC_CBWCSUNITS_UTMZONE:
		Importing->UTMZone = (short)WidgetCBGetCurSel(IDC_CBWCSUNITS_UTMZONE);
		Importing->UTMZone += 1;
		if (Importing->UTMZone > 60)
			Importing->UTMZone = 60 - Importing->UTMZone;
		break;
	case IDC_CBWCSUNITS_VUNITS:
		// SetVScale(WidgetCBGetCurSel(IDC_CBWCSUNITS_VUNITS));	// this really should be here, but DXF goes wonky if enabled
		break;
	case IDC_GPS_FIELDIS:
		DoGPSFieldChange();
		break;
	case IDC_IMWIZ_CBCOORDS:
		#ifdef WCS_BUILD_VNS
		if (&Importing->IWCoordSys)
			Importing->IWCoordSys.EditRAHost();
		//SelectNewCoords();
		SetMetrics();
		#endif // WCS_BUILD_VNS
		break;
	default:
		break;
	} // switch CtrlID

return (0);

} // ImportWizGUI::HandleCBChange

/*===========================================================================*/

long ImportWizGUI::HandleCloseWin(NativeGUIWin NW)
{

AppScope->MCP->SetParam(1, WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_CLOSE_MOD,
	WCS_TOOLBAR_ITEM_IWG, 0);

return(0);

} // ImportWizGUI::HandleCloseWin

/*===========================================================================*/

void ImportWizGUI::SelectPanel(unsigned long PanelID)
{

ActivePanel = PanelID;
switch(PanelID)
	{
	default:
		ShowPanel(0, -1);
		break;
	case IDD_IMWIZ_BINSET:
		DoBinSetPanel();
		ShowPanel(0, 1);
		break;
	case IDD_IMWIZ_BUILDOPT:
		ShowPanel(0, 2);
		break;
	case IDD_IMWIZ_COLEL:
		DoElevMethodPanel();
		ShowPanel(0, 3);
		break;
#ifdef WCS_BUILD_VNS
	case IDD_IMWIZ_COORDSYS:
		DoCoordSysPanel();
		ShowPanel(0, 28);
		break;
#endif // WCS_BUILD_VNS
	case IDD_IMWIZ_ELEVUNITS:
		DoElevUnitsPanel();
		ShowPanel(0, 4);
		break;
	case IDD_IMWIZ_GPS_STEP1:
		DoGPSStep1Panel();
		ShowPanel(0, 29);
		break;
	case IDD_IMWIZ_GPS_STEP2F:
	case IDD_IMWIZ_GPS_STEP2D:
		if (Importing->gpsFieldType)
			{
			DoGPSStep2FPanel();
			ShowPanel(0, 30);
			} // if
		else
			{
			DoGPSStep2DPanel();
			ShowPanel(0, 31);
			} // else
		break;
	case IDD_IMWIZ_GPS_STEP3:
		DoGPSStep3Panel();
		ShowPanel(0, 32);
		break;
	case IDD_IMWIZ_GRIDIT:
		DoGridItPanel();
		ShowPanel(0, 5);
		break;
	case IDD_IMWIZ_HOREX:
		DoHorExPanel();
		ShowPanel(0, 6);
		break;
	case IDD_IMWIZ_HORUNITS:
		DoHorUnitsPanel();
		ShowPanel(0, 7);
		break;
	case IDD_IMWIZ_ICRHEAD:
		DoICRHeadPanel();
		ShowPanel(0, 8);
		break;
	case IDD_IMWIZ_IDENT:
		PanelNum = 0;
		ShowPanel(0, -1);	// close any previously open panel
		WidgetSetDisabled(IDC_PREV, 1);
		ShowID(Importing->MyIdentity);
		DoIdentPanel();
		ShowPanel(0, 9);
		break;
	case IDD_IMWIZ_LOADAS:
		WidgetSetDisabled(IDC_PREV, 0);
		DoLoadAsPanel();
		ShowPanel(0, 10);
		break;
	case IDD_IMWIZ_NULL:
		DoNullPanel();
		ShowPanel(0, 11);
		break;
	case IDD_IMWIZ_CELLORDER:
		DoCellOrderPanel();
		ShowPanel(0, 12);
		break;
	case IDD_IMWIZ_OUTFORM:
		DoOutFormPanel();
		ShowPanel(0, 13);
		break;
	case IDD_IMWIZ_OUTPARAMS:
		DoOutParamsPanel();
		ShowPanel(0, 14);
		break;
	//case IDD_IMWIZ_OUTREG:
	//	DoOutRegPanel();
	//	ShowPanel(0, 15);
	//	break;
	case IDD_IMWIZ_OUTSCALE:
		DoOutScalePanel();
		ShowPanel(0, 16);
		switch (Importing->ScaleOp)
			{
			case 0:
				ShowPanel(1, 0);
				break;
			case 1:
				ShowPanel(1, 1);
				break;
			case 2:
				ShowPanel(1, 2);
				break;
			default:
				break;
			} // switch
		break;
	case IDD_IMWIZ_OUTTILES:
		DoOutTilesPanel();
		ShowPanel(0, 17);
		break;
	case IDD_IMWIZ_OUTTYPE:
		DoOutTypePanel();
		ShowPanel(0, 18);
		break;
	case IDD_IMWIZ_PIXCTR:
		ShowPanel(0, 19);
		break;
	case IDD_IMWIZ_PREPRO:
		DoPreProPanel();
		ShowPanel(0, 20);
		break;
	case IDD_IMWIZ_REFCO:
		DoRefCoPanel();
		ShowPanel(0, 21);
		break;
	case IDD_IMWIZ_SAVEGEO:
		DoSaveGeoPanel();
		ShowPanel(0, 22);
		break;
//#ifdef WCS_BUILD_VNS
	case IDD_IMWIZ_SHAPEOPTS:
		DoShapeOptsPanel();
		ShowPanel(0, 23);
		break;
//#endif // WCS_BUILD_VNS
	case IDD_IMWIZ_TEST:
		DoTestPanel();
		ShowPanel(0, 24);
		break;
	case IDD_IMWIZ_VERTEX:
		DoVertExPanel();
		ShowPanel(0, 25);
		break;
	case IDD_IMWIZ_WCSKNOWS:
		DoWCSKnowsPanel();
		ShowPanel(0, 26);
		break;
	case IDD_IMWIZ_WCSUNITS:
		DoWCSUnitsPanel();
		ShowPanel(0, 0);
		break;
	case IDD_IMWIZ_WRAP:
		DoWrapPanel();
		ShowPanel(0, 27);
		break;
	} // switch PanelID

} // ImportWizGUI::SelectPanel

/*===========================================================================*/

void ImportWizGUI::NextButton(NativeGUIWin NW)
{
unsigned long ndx, ScanOnce = 1;
#ifdef WCS_BUILD_VNS
unsigned long FixedFlow = 4;
#else // WCS_BUILD_VNS
unsigned long FixedFlow = 3;
#endif // WCS_BUILD_VNS
class Pier1 *pier;
short result = 0;

SECURITY_INLINE_CHECK(021, 21);
WidgetSetDisabled(IDC_PREV, 0);

//if (GlobalApp->GUIWins->COS)
//	{
//	printf("Here");
//	} // if

//if (Importing->LoadAs == LAS_IMAGE)
//	FixedFlow = 3;

if (PanelNum == 0) // this is true at the beginning of a load
	{
	if (strcmp(Importing->MyIdentity, "GPX") == 0)
		{
		DoLoad(NULL);	// Just import it right now.  We won't return from this call.
		return;
		} // if

	// reset lock states
	WidgetEMSetReadOnly(IDC_FIHOREXN, 0);
	WidgetSetCheck(IDC_TBHOREX_LOCKN, 0);
	WidgetEMSetReadOnly(IDC_FIHOREXS, 0);
	WidgetSetCheck(IDC_TBHOREX_LOCKS, 0);
	WidgetEMSetReadOnly(IDC_FIHOREXE, 0);
	WidgetSetCheck(IDC_TBHOREX_LOCKE, 0);
	WidgetEMSetReadOnly(IDC_FIHOREXW, 0);
	WidgetSetCheck(IDC_TBHOREX_LOCKW, 0);

	pier = Importing;
	if ((strcmp(Importing->MyIdentity, "USGS_DEM") == 0) || (strcmp(Importing->MyIdentity, "SDTS_DEM") == 0))
		ScanOnce = 0;
	while ((result == 0) && (Importing != NULL)) // loop to get data bounds where possible
		{
		result = GetInputFile();	// 0 = no error, positive number is error message to display

		if (Importing->HasUTM)
			{
			// Compare data bounds with UTM region bounds
			if (Importing->NBound > RUTMBoundN)
				RUTMBoundN = Importing->NBound;
			if (Importing->SBound < RUTMBoundS)
				RUTMBoundS = Importing->SBound;
			if (Importing->EBound < RUTMBoundE)
				RUTMBoundE = Importing->EBound;
			if (Importing->WBound > RUTMBoundW)
				RUTMBoundW = Importing->WBound;
			} // if

		if (ScanOnce)
			break;

		Importing = Importing->Next;
		} // while
	Importing = pier;
	if (result == 0)
		{
//		PanelNum = 1;
		WidgetSetDisabled(IDC_PREV, 0);
//		SelectPanel(IDD_IMWIZ_LOADAS);
		} // if
	else if (result == -1)
		{
		UserMessageOK("Import Wizard", "Internal Error - No case for identified file type");
		return;
		} // else if
	else
		{
		DataOpsErrMsg(result);
		UserMessageOK("Import Wizard", "File error - check log for details");
		return;
		} // else
	} // if Ident Panel

switch (PanelNum)
	{
	case 0:		// IDD_IMWIZ_IDENT
		PanelNum++;
		SelectPanel(IDD_IMWIZ_LOADAS);
		break;
	case 1:		// IDD_IMWIZ_LOADAS
		PanelNum++;
		SelectPanel(IDD_IMWIZ_OUTTYPE);
		break;
#ifdef WCS_BUILD_VNS
	case 2:		// IDD_IMWIZ_OUTTYPE
		PanelNum++;
		if (Importing->LoadAs == LAS_IMAGE)
			ControlFlow(1, true);
		else
			{
			FinishOuttype();
			if ((stricmp(Importing->MyIdentity, "ASCII_GPS") == 0) || (stricmp(Importing->MyIdentity, "GPX") == 0))
				{
				ndx = 0;
				ControlFlow(ndx, true);
				} // if
			else
				SelectPanel(IDD_IMWIZ_COORDSYS);
			} // else
		break;
#endif // WCS_BUILD_VNS
	default:
		if (PanelNum >= FixedFlow)
			ndx = PanelNum - FixedFlow;
		else
			ndx = 0;
		ControlFlow(ndx, true);
		break;
	} // switch PanelNum

} // ImportWizGUI::NextButton

/*===========================================================================*/

void ImportWizGUI::PrevButton(void)
{
unsigned long ndx;
#ifdef WCS_BUILD_VNS
unsigned long FixedFlow = 4;
#else // WCS_BUILD_VNS
unsigned long FixedFlow = 3;
#endif // WCS_BUILD_VNS

//if (Importing->LoadAs == LAS_IMAGE)
//	FixedFlow = 3;

WidgetSetText(IDC_NEXT, "Next -->");

switch (PanelNum)
	{
	case 0:		// IDD_IMWIZ_IDENT
		// shouldn't ever get this case
		break;
	case 1:		// IDD_IMWIZ_LOADAS
		--PanelNum;
		SelectPanel(IDD_IMWIZ_IDENT);
		break;
	case 2:		// IDD_IMWIZ_OUTTYPE
		--PanelNum;
		SelectPanel(IDD_IMWIZ_LOADAS);
		break;
	case 3:		// IDD_IMWIZ_COORDSYS
		--PanelNum;
		SelectPanel(IDD_IMWIZ_OUTTYPE);
		break;
	default:
		if (PanelNum >= FixedFlow)
			ndx = PanelNum - FixedFlow;
		else
			ndx = 0;
		ControlFlow(ndx, false);
		break;
	} // switch PanelNum

} // ImportWizGUI::PrevButton

/*===========================================================================*/

void ImportWizGUI::ShowID(char *whatiam)
{
char msg[512], MyTitle[256];

// This next line is what I'd like, but the Windoze backslashes become control characters
//sprintf(msg, "The file :\r\r\n'%s'\r\r\nwas identified as ", Importing->LoadName);
strcpy(MyTitle, "Import Wizard: ");
strncat(MyTitle, Importing->InFile, 255-15);
MyTitle[255] = 0;
SetTitle(MyTitle);
sprintf(msg, "The file '%s' was identified as ", Importing->InFile);
intype = whatiam;
if (strcmp(whatiam, "3DS") == 0)
	{
	strcat(msg, "a 3D Studio object.\r\r\n\r\r\nUse the 3D Object Editor to load.");
	} // if
else if (strcmp(whatiam, "ARC_ASCII_ARRAY") == 0)
	{
	strcat(msg, "an Arc ASCII array.");
	} // else if
else if (strcmp(whatiam, "ARC_BINARYADF_GRID") == 0)
	{
	strcat(msg, "an Arc Grid.");
	} // else if
else if (strcmp(whatiam, "ARC_EXPORT_DEM") == 0)
	{
	strcat(msg, "an Arc Export Grid.");
	} // else if
else if (strcmp(whatiam, "ASCII_ARRAY") == 0)
	{
	strcat(msg, "an ASCII array.");
	} // else if
else if (strcmp(whatiam, "ASCII_GPS") == 0)
	{
	strcat(msg, "an ASCII GPS file.");
	} // else if
else if (strcmp(whatiam, "BRYCE_DEM") == 0)
	{
	strcat(msg, "a Bryce DEM.");
	} // else if
else if (strcmp(whatiam, "DTED") == 0)
	{
	strcat(msg, "a DTED file.");
	} // else if
else if (strcmp(whatiam, "DXF") == 0)
	{
	strcat(msg, "a DXF file.\r\r\n\r\r\nIf this DXF file is a 3D Object, use the 3D Object Editor to load.");
	} // else if
else if (strcmp(whatiam, "EXPONENTFILE") == 0)
	{
	strcat(msg, "an exponential numbers file.");
	} // else if
else if (strcmp(whatiam, "GTOPO30") == 0)
	{
	strcat(msg, "a GTOPO30 data set.");
	} // else if
else if (strcmp(whatiam, "GPX") == 0)
	{
	strcat(msg, "a GPX file.\r\r\n\r\r\nWhen you press the Import button, GPX data will be loaded & processed without any further wizards.");
	} // else if
else if (strcmp(whatiam, "GZIP") == 0)
	{
	strcat(msg, "a gzip compressed file.\r\r\n\r\r\nWCS can't read that file.\r\r\n\
Press the 'Get Help on Unarchiving' button for further instructions.");
	WidgetSetText(IDC_BUTTONIDENT_UNARCHELP, "Get Help on Unarchiving");
//	WidgetShow(IDC_BUTTONIDENT_UNARCHELP, 1); 
	} // else if
else if (strcmp(whatiam, "IMAGE_BMP") == 0)
	{
	strcat(msg, "a BMP image.");
	} // else if
else if (strcmp(whatiam, "IMAGE_IFF") == 0)
	{
	strcat(msg, "an IFF image.");
	} // else if
else if (strcmp(whatiam, "IMAGE_PICT") == 0)
	{
	strcat(msg, "a PICT image.");
	} // else if
else if (strcmp(whatiam, "IMAGE_TARGA") == 0)
	{
	strcat(msg, "a TARGA image.");
	} // else if
else if (strcmp(whatiam, "LWOB") == 0)
	{
	strcat(msg, "a Lightwave object.\r\r\n\r\r\nUse the 3D Object Editor to load.");
	} // else if
else if (strcmp(whatiam, "MDEM") == 0)
	{
	strcat(msg, "a MicroDEM DEM.");
	} // else if
/***
else if (strcmp(whatiam, "NED_BINARY") == 0)
	{
	strcpy(Importing->NameBase, Importing->InFile);		// filename_bil to filename
	if (ch = strstr(Importing->NameBase, "_"))
		*ch = 0;
	strcat(msg, "a NED Binary data set.");
	}
else if (strcmp(whatiam, "NED_GRIDFLOAT") == 0)
	{
	strcpy(Importing->NameBase, Importing->InFile);		// filename_fp to filename
	if (ch = strstr(Importing->NameBase, "_"))
		*ch = 0;
	strcat(msg, "a NED GridFloat data set.");
	}
***/
else if (strcmp(whatiam, "NO_ASSOCIATES") == 0)
	{
	strcat(msg, ".");
	} // else if
else if (strcmp(whatiam, "NTF_DTM") == 0)
	{
	strcat(msg, "a NTF DTM.");
	} // else if
else if (strcmp(whatiam, "NUMERICFILE") == 0)
	{
	strcat(msg, "a numeric file.");
	} // else if
else if (strcmp(whatiam, "SDTS_DEM") == 0)
	{
	strcat(msg, "a SDTS DEM.");
	} // else if
else if (strcmp(whatiam, "SDTS_DLG") == 0)
	{
	strcat(msg, "a SDTS DLG.");
	} // else if
else if (strcmp(whatiam, "SHAPEFILE") == 0)
	{
	strcat(msg, "an Arc Shapefile.");
	} // else if
else if (strcmp(whatiam, "SRTM") == 0)
	{
	strcat(msg, "a SRTM (Shuttle Radar Topography Mission) file.");
	} // else if
else if (strcmp(whatiam, "STM") == 0)
	{
	strcat(msg, "a STM (Simple Terrain Model) file.");
	} // else if
#ifdef GO_SURFING
else if (strcmp(whatiam, "SURFER") == 0)
	{
	strcat(msg, "a Surfer Grid file.");
	} // else if
#endif // GO_SURFING
else if (strcmp(whatiam, "TERRAGEN") == 0)
	{
	strcat(msg, "a Terragen DEM.");
	} // else if
else if (strcmp(whatiam, "TEXTFILE") == 0)
	{
	strcat(msg, "a text file.");
	} // else if
else if (strcmp(whatiam, "IMAGE_TIFF") == 0)
	{
	strcat(msg, "a TIFF file.");
	} // else if
else if (strcmp(whatiam, "BINARY") == 0)
	{
	strcat(msg, "a binary file.");
	} // else if
else if (strcmp(whatiam, "UNKNOWN_EXT") == 0)
	{
	// we shouldn't see this!
	strcat(msg, "an unknown extension.");
	} // else if
else if (strcmp(whatiam, "USGS_DEM") == 0)
	{
	if (Importing->Signal == ARC_VARIANT)
		strcat(msg, "an Arc USGS DEM.");
	else
		strcat(msg, "a USGS DEM.");
	} // else if
else if (strcmp(whatiam, "USGS_DLG") == 0)
	{
	strcat(msg, "a USGS DLG.");
	} // else if
else if (strcmp(whatiam, "VISTA_DEM") == 0)
	{
	strcat(msg, "a VistaPro DEM.");
	} // else if
else if (strcmp(whatiam, "WCS_3DOBJECT") == 0)
	{
	strcat(msg, "a "APP_TLA" 3D Object.\r\r\n\r\r\nUse the 3D Object Editor to load.");
	} // else if
else if (strcmp(whatiam, "WCS_DEM") == 0)
	{
#ifdef WCS_BUILD_VNS
	strcat(msg, "a WCS/VNS DEM.");
#else // WCS_BUILD_VNS
	strcat(msg, "a WCS DEM.");
#endif // WCS_BUILD_VNS
	} // else if
else if (strcmp(whatiam, "WCS_PROJ") == 0)
	{
	strcat(msg, "a "APP_TLA" Project file.\r\r\n\r\r\nUse the Open command to load a project.");
	} // else if
else if (strcmp(whatiam, "WCS_XYZ") == 0)
	{
	strcat(msg, "a "APP_TLA" XYZ+ file.");
	} // else if
else if (strcmp(whatiam, "WCS_ZBUFFER") == 0)
	{
	strcat(msg, "a "APP_TLA" ZBuffer.");
	} // else if
else if (strcmp(whatiam, "XYZ_FILE") == 0)
	{
	strcat(msg, "a XYZ file.");
	} // else if
else if (strcmp(whatiam, "ZIP") == 0)
	{
	strcat(msg, "a Zip compressed file.\r\r\n\r\r\nWCS can't read that file.\r\r\n\
Press the 'Get Help on Unarchiving' button for further instructions.");
	WidgetSetText(IDC_BUTTONIDENT_UNARCHELP, "Get Help on Unarchiving");
//	WidgetShow(IDC_BUTTONIDENT_UNARCHELP, 1); 
	} // else if
else if (strcmp(whatiam, "IMAGE_JPEG") == 0)
	{
	strcat(msg, "a JPEG file.");
	} // else if
else if (strcmp(whatiam, "IMAGE_PNG") == 0)
	{
	strcat(msg, "a PNG file.");
	} // else if
else if (strcmp(whatiam, "IMAGE_ECW") == 0)
	{
	strcat(msg, "a ECW file.");
	} // else if
else
	{
	strcat(msg, "\r\r\n\r\r\nInternal Error in ShowID (whatiam = '");
	strcat(msg, whatiam);
	strcat(msg, "').");
	} // else

strcat(msg, "\r\r\n\r\r\n\r\r\nUse the Override button to force "APP_TLA" to read the file as another type if needed.\r\r\n");
WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::ShowID

/*===========================================================================*/

void ImportWizGUI::DoBinFactors(void)
{
FILE *phyle;
long fact1, fact2, fsize;
long Current, found = 0;
char msg[256];

phyle = PROJ_fopen(Importing->LoadName, "rb");
if (phyle == NULL)
	{
	sprintf(msg, "Unable to open file %s", Importing->LoadName);
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, msg);
	} // if
else
	{
	fseek(phyle, 0, SEEK_END);
	fsize = ftell(phyle);
	Importing->InFileSize = fsize;
	sprintf(msg, "%ld (%d Byte Data)", fsize, Importing->ValueBytes);
	WidgetSetText(IDC_INPUTSIZE, msg);
	Current = WidgetCBGetCurSel(IDC_CBICRHEAD_BINFACTORS);
	WidgetCBClear(IDC_CBICRHEAD_BINFACTORS);
	ConfigureFI(NativeWin, IDC_BINHEADERSIZE, &Importing->HdrSize, 1.0, 0.0, (double)fsize, FIOFlag_Long, NULL, NULL);
	fsize -= Importing->HdrSize ;
	fsize /= Importing->ValueBytes;
	fact2 = 0;
	fact1 = (long) (sqrt((double)fsize) + 0.5);
	if (! Importing->Factored && (Importing->InRows > Importing->InCols))
		{
		Importing->SwapRC = true;
		WidgetSCSync(IDC_SCSWAPFACTORS, WP_SCSYNC_NONOTIFY);
		} // if
	do
		{
		if ((fsize % fact1) == 0)	// found a prime factor
			{
			fact2 = fsize / fact1;	// get it's partner
			if ((! Importing->Factored) && (Importing->InCols != 1) && (Importing->InRows != 1))
				{
				// see if we have a match for either of these factors
				if (fact1 == Importing->InCols)
					Current = found;
				else if (fact1 == Importing->InRows)
					Current = found;
				} // if
			else if (Current == found)	// we're at the list position that was previously selected
				{
				if (Importing->SwapRC)
					{
					Importing->InCols = fact2;
					Importing->InRows = fact1;
					} // if
				else
					{
					Importing->InCols = fact1;
					Importing->InRows = fact2;
					} // else
				} // else if
			found++;
			if (Importing->SwapRC)
				sprintf(msg, "%ld * %ld", fact2, fact1);
			else
				sprintf(msg, "%ld * %ld", fact1, fact2);
			(void)WidgetCBAddEnd(IDC_CBICRHEAD_BINFACTORS, msg);
			} // if
		fact1++;
		} while (fact2 != 1);
	ConfigureFI(NativeWin, IDC_FIICRINCOLS, &Importing->InCols, 1.0, 1.0, (double)fsize, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIICRINROWS, &Importing->InRows, 1.0, 1.0, (double)fsize, FIOFlag_Long, NULL, NULL);
	WidgetCBSetCurSel(IDC_CBICRHEAD_BINFACTORS, Current);
	fclose(phyle);
	Importing->Factored = true;
	} // else

} // ImportWizGUI::DoBinFactors

/*===========================================================================*/

void ImportWizGUI::DoHorExLabels(void)
{

if (Importing->BoundsType != VNS_BOUNDSTYPE_LINEAR)
	{
	if (Importing->PosEast)
		{
		WidgetSetText(IDC_FIHOREXN, "North Lat ");
		WidgetSetText(IDC_FIHOREXS, "South Lat ");
		WidgetSetText(IDC_FIHOREXE, "East Lon (+) ");
		WidgetSetText(IDC_FIHOREXW, "West Lon ");
		} // if
	else
		{
		WidgetSetText(IDC_FIHOREXN, "North Lat ");
		WidgetSetText(IDC_FIHOREXS, "South Lat ");
		WidgetSetText(IDC_FIHOREXE, "East Lon ");
		WidgetSetText(IDC_FIHOREXW, "West Lon (+) ");
		} // else
	} // if
else
	{
	WidgetSetText(IDC_FIHOREXN, "High Y ");
	WidgetSetText(IDC_FIHOREXS, "Low Y ");
	WidgetSetText(IDC_FIHOREXE, "High X ");
	WidgetSetText(IDC_FIHOREXW, "Low X ");
	} // else

WidgetSNSync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXWIDTH, WP_FISYNC_NONOTIFY);

} // ImportWizGUI::DoHorExLabels

/*===========================================================================*/

void ImportWizGUI::SyncGrid(void)
{

WidgetSNSync(IDC_FIHOREXGRIDNS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDWE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXHEIGHT, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDSIZENS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDSIZEWE, WP_FISYNC_NONOTIFY);

} // ImportWizGUI::SyncGrid

/*===========================================================================*/

void ImportWizGUI::LoadingMsg(void)
{
char msg[] = "LOADING\r\r\n\r\r\nThe data is loading.  This window will disappear when the data is \
finished loading and processing.";

WidgetSetText(IDC_IMWIZTEXT, msg);

ShowPanel(0, -1);

} // ImportWizGUI::LoadingMsg

/*===========================================================================*/

void ImportWizGUI::GenericBinary(void)
{

SelectPanel(IDD_IMWIZ_BINSET);

} // ImportWizGUI::GenericBinary

/*===========================================================================*/

#ifdef WCS_BUILD_VNS
void ImportWizGUI::ChangeCoordSys(void)
{

Importing->IWCoordSys.EditModal();

} // ImportWizGUI::ChangeCoordSys
#endif // WCS_BUILD_VNS

/*===========================================================================*/

void ImportWizGUI::DoNameBaseExtension(void)
{
long ident, selected;
char *ext;

selected = WidgetCBGetCurSel(IDC_CBOUTTYPE_OUTFORMAT);
ident = (long)WidgetCBGetItemData(IDC_CBOUTTYPE_OUTFORMAT, selected);

if (Importing->LoadAs == LAS_DEM)
	{
	if ((strcmp(intype, "SDTS_DEM") == 0) || (strcmp(intype, "USGS_DEM") == 0))
		return;
	if (INPUT_FORMAT == DEM_DATA2_INPUT_IFF)
		selected = 5;
	else if (selected == 6)
		selected--;
	} // if
else if (Importing->LoadAs != LAS_IMAGE)
	return;

// append an appropriate extension on to the output name
ext = FindFileExtension(Importing->NameBase);
if ((ext != NULL) && ((stricmp(ext, "bmp") == 0) || (stricmp(ext, "iff") == 0) || (stricmp(ext, "raw") == 0)
	|| (stricmp(ext, "pct") == 0) || (stricmp(ext, "pic") == 0) || (stricmp(ext, "pict") == 0) || (stricmp(ext, "tga") == 0)))
	(void)StripExtension(Importing->NameBase);

switch (ident)
	{
	default:
		break;
	case DEM_DATA2_OUTPUT_BINARY:	// binary array
		strcat(Importing->NameBase, ".raw");
		break;
	case DEM_DATA2_OUTPUT_COLORBMP:
		strcat(Importing->NameBase, ".bmp");
		break;
	case DEM_DATA2_OUTPUT_COLORIFF:
		strcat(Importing->NameBase, ".iff");
		break;
	case DEM_DATA2_OUTPUT_COLORPICT:
		strcat(Importing->NameBase, ".pct");
		break;
	case DEM_DATA2_OUTPUT_COLORTGA:
		strcat(Importing->NameBase, ".tga");
		break;
	case DEM_DATA2_OUTPUT_GRAYIFF:
		strcat(Importing->NameBase, ".iff");
		break;
	case DEM_DATA2_OUTPUT_ZBUF:
		break;
	} // switch

} // ImportWizGUI::DoNameBaseExtension

/*===========================================================================*/

void ImportWizGUI::DoValBytes(void)
{
long CurrentVB, CurrentVF;
unsigned char numbytes = 0;

CurrentVB = WidgetCBGetCurSel(IDC_CBBINSET_VALUEBYTES);
CurrentVF = WidgetCBGetCurSel(IDC_CBBINSET_VALUEFORMAT);

switch (CurrentVF)
	{
	case IMWIZ_DATA_FORMAT_UNSIGNEDINT:
	case IMWIZ_DATA_FORMAT_SIGNEDINT:
		if (CurrentVB == 0)
			numbytes = 1;
		else if (CurrentVB == 1)
			numbytes = 2;
		else
			numbytes = 4;
		break;
	case IMWIZ_DATA_FORMAT_FLOAT:
		numbytes = (UBYTE)(CurrentVB * 4 + 4);	// 4 or 8
		break;
	default:
		break;
	} // switch

Importing->ValueBytes = numbytes;
if (Importing->ValueBytes == 1)
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, true);
else
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, false);

} // ImportWizGUI::DoValBytes

/*===========================================================================*/

void ImportWizGUI::DoOutValueBytes(void)
{
long CurrentVB, CurrentVF;
unsigned char numbytes = 0;

CurrentVB = WidgetCBGetCurSel(IDC_CBOUTFORM_VALUEBYTES);
CurrentVF = WidgetCBGetCurSel(IDC_CBOUTFORM_VALUEFORMAT);

switch (CurrentVF)
	{
	case IMWIZ_DATA_FORMAT_UNSIGNEDINT:
	case IMWIZ_DATA_FORMAT_SIGNEDINT:
		if (CurrentVB == 0)
			numbytes = 1;
		else if (CurrentVB == 1)
			numbytes = 2;
		else
			numbytes = 4;
		break;
	case IMWIZ_DATA_FORMAT_FLOAT:
		numbytes = (UBYTE)(CurrentVB * 4 + 4);	// 4 or 8
		break;
	default:
		break;
	} // switch

Importing->OutValBytes = numbytes;

} // ImportWizGUI::DoOutValueBytes

/*===========================================================================*/

void ImportWizGUI::DoValFmt(void)
{
long Current;
unsigned char numbytes = 0;

Current = WidgetCBGetCurSel(IDC_CBBINSET_VALUEFORMAT);
switch (Current)
	{
	case IMWIZ_DATA_FORMAT_SIGNEDINT:
	case IMWIZ_DATA_FORMAT_UNSIGNEDINT:
		if (Importing->ValueFmt == 2)	// did they change from a float?
			{
			WidgetCBClear(IDC_CBBINSET_VALUEBYTES);
			(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "One");
			(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Two");
			(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Four");
			WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
			numbytes = 1;
			} // if
		else
			numbytes = Importing->ValueBytes;	// keep the last value
		break;
	case IMWIZ_DATA_FORMAT_FLOAT:
		WidgetCBClear(IDC_CBBINSET_VALUEBYTES);
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Four");
		(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Eight");
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
		numbytes = 4;
		break;
	default:
		break;
	} // switch
Importing->ValueFmt = (unsigned char)Current;
Importing->ValueBytes = numbytes;
if (Importing->ValueBytes == 1)
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, true);
else
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, false);

} // ImportWizGUI::DoValFmt

/*===========================================================================*/

void ImportWizGUI::DoOutValueFormat(void)
{
long Current;
unsigned char numbytes = 0;

Current = WidgetCBGetCurSel(IDC_CBOUTFORM_VALUEFORMAT);
switch (Current)
	{
	case IMWIZ_DATA_FORMAT_UNSIGNEDINT:
	case IMWIZ_DATA_FORMAT_SIGNEDINT:
		WidgetCBClear(IDC_CBOUTFORM_VALUEBYTES);
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "One");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Two");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Four");
		WidgetCBSetCurSel(IDC_CBOUTFORM_VALUEBYTES, 0);
		numbytes = 1;
		break;
	case IMWIZ_DATA_FORMAT_FLOAT:
		WidgetCBClear(IDC_CBOUTFORM_VALUEBYTES);
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Four");
		(void)WidgetCBAddEnd(IDC_CBOUTFORM_VALUEBYTES, "Eight");
		WidgetCBSetCurSel(IDC_CBOUTFORM_VALUEBYTES, 0);
		numbytes = 4;
		break;
	default:
		break;
	} // switch
Importing->OutValFmt = (unsigned char)Current;
Importing->OutValBytes = numbytes;

} // ImportWizGUI::DoOutValueFormat

/*===========================================================================*/

void ImportWizGUI::DoBinSetPanel(void)
{
const char msg[] = "BINARY INPUT SETTINGS\r\r\nWCS needs help on how to interpret raw data\r\r\n\r\r\n� Value Format is \
used to indicate the kind of numbers stored, and may be a signed integer, an unsigned integer, or a floating \
point number.\r\r\n\r\r\n� Value Bytes is the size of the stored number. Integers may be 1, 2, or 4 bytes in size. \
Floats are 4 (single precision) or 8 (double precision) bytes in size.\r\r\n\r\r\n� Byte Order is only relevant if \
Value Bytes is two or more. Each byte holds a piece of the number. This indicates whether the pieces are stored \
in ascending or descending order. Motorola CPU's normally write in High-Low or Big-Endian order, while Intel \
CPU's normally do the opposite.\r\r\n\r\r\n� Signed Byte = -128..+127\r\r\n� Unsigned Byte = 0..+255\r\r\n� Signed 2 \
Bytes = -32768..+32767\r\r\n� Unsigned 2 Bytes = 0..65535\r\r\n� Signed 4 Bytes = +/- 2 Billion\r\r\n� Unsigned 4 \
Bytes = 0..4 Billion\r\r\n� 4 Byte Float = 6 Digits Precision\r\r\n� 8 Byte Float = 15 Digits Precision";

WidgetSetText(IDC_IMWIZTEXT, msg);

WidgetCBSetCurSel(IDC_CBBINSET_VALUEFORMAT, Importing->ValueFmt);
if (Importing->ValueFmt == IMWIZ_DATA_FORMAT_FLOAT)
	{
	WidgetCBClear(IDC_CBBINSET_VALUEBYTES);
	(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Four");
	(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Eight");
	if (Importing->ValueBytes == 8)
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 1);
	else
		{
		Importing->ValueBytes = 4;
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
		} // else
	} // if
else
	{
	WidgetCBClear(IDC_CBBINSET_VALUEBYTES);
	(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "One");
	(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Two");
	(void)WidgetCBAddEnd(IDC_CBBINSET_VALUEBYTES, "Four");
	WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
	if (Importing->ValueBytes == 4)
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 2);
	else if (Importing->ValueBytes == 2)
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 1);
	else
		{
		Importing->ValueBytes = 1;
		WidgetCBSetCurSel(IDC_CBBINSET_VALUEBYTES, 0);
		} // else
	} // else
if (Importing->ValueBytes == 1)
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, true);
else
	WidgetSetDisabled(IDC_CBBINSET_BYTEORDER, false);

} // ImportWizGUI::DoBinSetPanel

/*===========================================================================*/

void ImportWizGUI::DoICRHeadPanel(void)
{
const char msg[] = "INPUT COLUMNS, ROWS, AND HEADER\r\r\n\r\r\nThe Cols and Rows values indicate the number of rows and \
columns present in the input array. If the input file has a portion of data that has to be skipped, set the \
Header Bytes field to the number of bytes to skip. The File Size must always be greater than or equal to \
Header + Rows x Columns x Value Size.\r\r\n\r\r\n� The Factor List drop box will contain all the possible \
combinations of Rows & Cols based on the File Size and the Header size.";

WidgetSetText(IDC_IMWIZTEXT, msg);

DoBinFactors();

} // ImportWizGUI::DoICRHeadPanel

/*===========================================================================*/

void ImportWizGUI::DoCellOrderPanel(void)
{
const char msg[] = "CELL ORDER\r\r\n\r\r\nCell Order determines how a DEM file is read.\r\r\n\r\r\n� The Major Axis & Minor Axis controls \
determine how the DEM is read into memory.  The data will be read along the Major Axis first, and once that row or column of data is \
filled, it will step to the next row or column.  Data orderered from left to right should use W --> E on the appropriate axis.  Data \
ordered from top to bottom should use N --> S on the appropriate axis.\r\r\n";

WidgetSetText(IDC_IMWIZTEXT, msg);

WidgetSetDisabled(IDC_PREV, 0);

} // ImportWizGUI::DoCellOrderPanel

/*===========================================================================*/

void ImportWizGUI::DoOverride(void)
{
long ident, selected;
FILE *phil = NULL;

phil = PROJ_fopen(Importing->LoadName, "rb");
selected = WidgetCBGetCurSel(IDC_CBIDENT_OVERRIDE);
ident = (long)WidgetCBGetItemData(IDC_CBIDENT_OVERRIDE, selected);
WidgetSetDisabled(IDC_NEXT, 0);

Importing->MyNum = ident;
switch (ident)
	{
	default:
		break;
	case IW_INPUT_ARCASCII:
		strcpy(Importing->MyIdentity, "ARC_ASCII_ARRAY");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_ARCBINARYADF_GRID:
		strcpy(Importing->MyIdentity, "ARC_BINARYADF_GRID");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_ASCII_ARRAY:
		strcpy(Importing->MyIdentity, "ASCII_ARRAY");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_BINARY:
		strcpy(Importing->MyIdentity, "BINARY_ARRAY");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_BRYCE:
		strcpy(Importing->MyIdentity, "BRYCE_DEM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
//	case 4:
//		Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;	// set defaults - will be changed if .HDR is found
//		Importing->ValueBytes = DEM_DATA_VALSIZE_LONG;
//		(void)SniffBIL(phil);
//		strcpy(Importing->MyIdentity, "BINARY_BIL");
//		Importing->LoadOpt = LAS_DEM | LAS_CP;
//		break;
	case IW_INPUT_DTED:
		strcpy(Importing->MyIdentity, "DTED");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_DXF:
		strcpy(Importing->MyIdentity, "DXF");
		Importing->LoadOpt = LAS_CP | LAS_VECT;
		break;
	case IW_INPUT_GTOPO30:
		strcpy(Importing->MyIdentity, "GTOPO30");
		Importing->LoadOpt = LAS_DEM;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_IMAGE_BMP:
		strcpy(Importing->MyIdentity, "IMAGE_BMP");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_ECW:
		strcpy(Importing->MyIdentity, "IMAGE_ECW");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_IFF:
		strcpy(Importing->MyIdentity, "IMAGE_IFF");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_JPEG:
		strcpy(Importing->MyIdentity, "IMAGE_JPEG");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_PICT:
		strcpy(Importing->MyIdentity, "IMAGE_PICT");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_PNG:
		strcpy(Importing->MyIdentity, "IMAGE_PNG");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_TARGA:
		strcpy(Importing->MyIdentity, "IMAGE_TARGA");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_IMAGE_TIFF:
		strcpy(Importing->MyIdentity, "IMAGE_TIFF");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		break;
	case IW_INPUT_MDEM:
		strcpy(Importing->MyIdentity, "MicroDEM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;	// | LAS_IMAGE;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_NTF_DTM:
		strcpy(Importing->MyIdentity, "NTF_DTM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		Importing->CoordSysWarn = false;
		break;
//	case IW_INPUT_NTF_MERIDIAN2:
//		strcpy(Importing->MyIdentity, "NTF_MERIDIAN2");
//		Importing->LoadOpt = LAS_DEM | LAS_VECT;
//		Importing->CoordSysWarn = false;
//		break;
	case IW_INPUT_SDTS_DEM:
		strcpy(Importing->MyIdentity, "SDTS_DEM");
		Importing->LoadOpt = LAS_DEM;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_SDTS_DLG:
		strcpy(Importing->MyIdentity, "SDTS_DLG");
		Importing->LoadOpt = LAS_VECT;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_SHAPE:
		strcpy(Importing->MyIdentity, "SHAPEFILE");
		Importing->LoadOpt = LAS_VECT;
		break;
	case IW_INPUT_STM:
		strcpy(Importing->MyIdentity, "STM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
#ifdef GO_SURFING
	case IW_INPUT_SURFER:
		strcpy(Importing->MyIdentity, "SURFER");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
#endif // GO_SURFING
	case IW_INPUT_TERRAGEN:
		strcpy(Importing->MyIdentity, "TERRAGEN");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_USGS_ASCII_DEM:
		strcpy(Importing->MyIdentity, "USGS_DEM");
		Importing->LoadOpt = LAS_DEM;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_USGS_ASCII_DLG:
		strcpy(Importing->MyIdentity, "USGS_DLG");
		Importing->LoadOpt = LAS_VECT;
		Importing->CoordSysWarn = true;
		break;
	case IW_INPUT_VISTAPRO:
		strcpy(Importing->MyIdentity, "VISTA_DEM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		break;
	case IW_INPUT_WCS_DEM:
		strcpy(Importing->MyIdentity, "WCS_DEM");
		Importing->LoadOpt = LAS_DEM | LAS_CP;
		Importing->CoordSysWarn = false;
		break;
	case IW_INPUT_WCS_XYZ:
		strcpy(Importing->MyIdentity, "WCS_XYZ");
		Importing->LoadOpt = LAS_CP;
		break;
	case IW_INPUT_WCS_ZBUFFER:
		strcpy(Importing->MyIdentity, "WCS_ZBUFFER");
		Importing->LoadOpt = LAS_DEM | LAS_IMAGE;
		break;
	case IW_INPUT_XYZ:
		strcpy(Importing->MyIdentity, "XYZ_FILE");
		Importing->LoadOpt = LAS_CP;
		break;
	} // switch

Importing->LoadAs = 0;	// reset the choice to none
strcpy(whatitis, Importing->MyIdentity);
SetPanelOrder();
if (phil)
	fclose(phil);

} // ImportWizGUI::DoOverride

/*===========================================================================*/

void ImportWizGUI::DoText(void)
{
const char msg[] = "That appears to be a Text file.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoText

/*** All the Check functions are called after the file has been identified ***/
/*===========================================================================*/

short ImportWizGUI::CheckGTOPO30(void)
{
long flags;

Importing->AllowRef00 = Importing->AllowPosE = false;
//Importing->UseFloor = true;
//Importing->Floor = -425;		// Dead Sea is -411m and dropping!
// see if we ended up with all the georef data we should have
flags = Importing->Flags;
if ((flags & FOUND_BITS) && (flags & FOUND_BYTEORDER) && (flags & FOUND_ROWS) && (flags & FOUND_COLS) &&
	(flags & FOUND_ULX) && (flags & FOUND_ULY) && (flags & FOUND_XDIM) && (flags & FOUND_YDIM))
	return IDD_IMWIZ_WCSKNOWS;
else
	return IDD_IMWIZ_PREPRO;

} // ImportWizGUI::CheckGTOPO30

/*===========================================================================*/

short ImportWizGUI::CheckNED_Bin(void)
{
long flags;

Importing->AllowRef00 = Importing->AllowPosE = false;
// see if we ended up with all the georef data we should have
flags = Importing->Flags;
if ((flags & FOUND_BITS) && (flags & FOUND_BYTEORDER) && (flags & FOUND_TFWDATA))
	return IDD_IMWIZ_WCSKNOWS;
else
	return IDD_IMWIZ_PREPRO;

} // ImportWizGUI::CheckNED_Bin

/*===========================================================================*/

short ImportWizGUI::CheckNED_FP(void)
{
long flags;

Importing->AllowRef00 = Importing->AllowPosE = false;
Importing->ValueFmt = DEM_DATA_FORMAT_FLOAT;
Importing->ValueBytes = DEM_DATA_VALSIZE_LONG;
Importing->ByteOrder = DEM_DATA_BYTEORDER_HILO;
Importing->ElevUnits = DEM_DATA_UNITS_METERS;
// see if we ended up with all the georef data we should have
flags = Importing->Flags;
if ((flags & FOUND_COLS) && (flags & FOUND_ROWS) && (flags & FOUND_XLL) &&
	(flags & FOUND_YLL) && (flags & FOUND_CELLSIZE) && (flags & FOUND_BYTEORDER))
	return IDD_IMWIZ_WCSKNOWS;
else
	return IDD_IMWIZ_PREPRO;

} // ImportWizGUI::CheckNED_FP

/*===========================================================================*/

void ImportWizGUI::DoOutTypePanel(void)
{
char buffer[768];
const char msg[] = "OUTPUT FILE TYPE AND NAME\r\r\n\r\r\nYou need to choose a format that the input data will be \
converted to, and the name of the file.\r\r\n\r\r\n";
#ifdef WCS_BUILD_VNS
const char remsg[] = "� Reimport when Project Loads - Shapefiles can be auto-imported when you load your project, remembering all settings \
you use.  This allows you to make changes on the shapefile attributes, or in the shape itself as a project evolves and have the changes \
transfer to VNS automatically.";
#endif // WCS_BUILD_VNS

long position;

strcpy(buffer, msg);
#ifdef WCS_BUILD_VNS
strcat(buffer, remsg);
#endif // WCS_BUILD_VNS
WidgetSetText(IDC_IMWIZTEXT, buffer);

// Reimport currently only OK for shapefiles
if (strcmp(Importing->MyIdentity, "SHAPEFILE") == 0)
	WidgetSetDisabled(IDC_SCOUTTYPE_TEMPLATE, 0);
else
	WidgetSetDisabled(IDC_SCOUTTYPE_TEMPLATE, 1);

if (Importing->OutFormat == DEM_DATA2_OUTPUT_UNSET)
	{
	WidgetCBClear(IDC_CBOUTTYPE_OUTFORMAT);
	switch (Importing->LoadAs)
		{
		case LAS_CP:
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Database Control Points");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_CONTROLPTS); 
			break;
		case LAS_DEM:
#ifdef WCS_BUILD_VNS
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "VNS DEM");
#else // WCS_BUILD_VNS
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "WCS DEM");
#endif // WCS_BUILD_VNS
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_WCSDEM); 
			if ((INPUT_FORMAT == DEM_DATA2_INPUT_IFF)  ||	// any image really
				(strcmp(intype, "SDTS_DEM") == 0) || (strcmp(intype, "USGS_DEM") == 0))
				break;	// no image to image conversion, or USGS/SDTS DEM to image conversion
			// else fall through to image
			//lint -fallthrough
		case LAS_IMAGE:
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Binary Array");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_BINARY); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Color BMP");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_COLORBMP); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Color IFF");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_COLORIFF); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Color PICT");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_COLORPICT); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Color Targa");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_COLORTGA); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Gray IFF");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_GRAYIFF); 
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "WCS ZBuffer");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_ZBUF); 
			break;
		case LAS_VECT:
			position = WidgetCBAddEnd(IDC_CBOUTTYPE_OUTFORMAT, "Database Vector");
			WidgetCBSetItemData(IDC_CBOUTTYPE_OUTFORMAT, position, (void *)DEM_DATA2_OUTPUT_CONTROLPTS); 
			break;
		default:
			break;
		} // switch LoadAs
	if ((Importing->LoadAs == LAS_DEM) && (INPUT_FORMAT != DEM_DATA2_INPUT_IFF)
		&& (strcmp(intype, "SDTS_DEM") != 0) && (strcmp(intype, "USGS_DEM") != 0))
		WidgetCBSetCurSel(IDC_CBOUTTYPE_OUTFORMAT, 6);	// default to WCS DEM
	else if (Importing->LoadAs == LAS_IMAGE)
		WidgetCBSetCurSel(IDC_CBOUTTYPE_OUTFORMAT, 2);	// image defaults to Color IFF
	else
		WidgetCBSetCurSel(IDC_CBOUTTYPE_OUTFORMAT, 0);
	Importing->OutFormat = DEM_DATA2_OUTPUT_SET;			// Dummy so we don't init again
	} // if

if (Importing->LoadAs != LAS_DEM)
	WidgetShow(IDC_DDOUTTYPEOUTDIR, 0);
else
	WidgetShow(IDC_DDOUTTYPEOUTDIR, 1);

WidgetSetText(IDC_NEXT, "&Next -->");
if (Importing->NameBase[0] == 0)
	WidgetSetDisabled(IDC_NEXT, 1);
WidgetSetModified(IDC_EDITOUTTYPEOUTNAME, false);
DoNameBaseExtension();
WidgetSetText(IDC_EDITOUTTYPEOUTNAME, Importing->NameBase);
ConfigureDD(NativeWin, IDC_DDOUTTYPEOUTDIR, Importing->OutDir, 255, NULL, 0, NULL);
#ifndef WCS_BUILD_VNS
WidgetShow(IDC_SCOUTTYPE_TEMPLATE, 0);
if (PanelOrder[0] == 0)
	WidgetSetText(IDC_NEXT, "Import");
#endif // !WCS_BUILD_VNS

} // ImportWizGUI::DoOutTypePanel

/*===========================================================================*/

void ImportWizGUI::DoPreProPanel(void)
{
char msg[1024];
const char text1[] = "PRE-PROCESSOR SETTINGS\r\r\n\r\r\n";
const char text2[] = "� Use the checkbox by Floor and Ceiling to enable these fields. When Floor is enabled, any input \
value below the floor value is replaced with the floor value. When Ceiling is enabled, any input value above the \
ceiling value is replaced with the ceiling value.\r\r\n\r\r\n";
const char text3[] = "� Use the Crop fields to eliminate rows or columns from the input array while loading. A number \
other than zero means to ignore that many rows or columns from that edge.\r\r\n\r\r\n";
const char text4[] = "� The checkbox to Keep Original Geo Bounds will cause the cropped area to use the same bounds \
as the uncropped area.  With this box unchecked, the bounds will be computed from the original bounds & the crop \
settings.";

strcpy(msg, text1);
if (Importing->LoadAs != LAS_IMAGE)
	strcat(msg, text2);
strcat(msg, text3);
if (Importing->LoadAs != LAS_IMAGE)
	strcat(msg, text4);
WidgetSetText(IDC_IMWIZTEXT, msg);

ConfigureFI(NativeWin, IDC_FICEILINGVAL, &Importing->Ceiling, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIFLOORVAL, &Importing->Floor, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCCEILING, &Importing->UseCeiling, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCFLOOR, &Importing->UseFloor, SCFlag_Char, NULL, NULL);
ConfigureSC(NativeWin, IDC_SCKEEPBOUNDS, &Importing->KeepBounds, SCFlag_Char, NULL, NULL);
if (Importing->LoadAs == LAS_IMAGE)
	{
	WidgetSetDisabledRO(IDC_FICEILINGVAL, true);
	WidgetSetDisabledRO(IDC_FIFLOORVAL, true);
	WidgetSetDisabled(IDC_SCCEILING, true);
	WidgetSetDisabled(IDC_SCFLOOR, true);
	WidgetSetDisabled(IDC_SCKEEPBOUNDS, true);
	}
else
	{
	WidgetSetDisabledRO(IDC_FICEILINGVAL, (char)!(Importing->UseCeiling));
	WidgetSetDisabledRO(IDC_FIFLOORVAL, (char)!(Importing->UseFloor));
	}
/*** F2 NOTE - What about load rotation? ***/
ConfigureFI(NativeWin, IDC_FICROPTOP, &Importing->CropTop, 1.0, 0.0, (double)Importing->InRows, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_FICROPBOTTOM, &Importing->CropBottom, 1.0, 0.0, (double)Importing->InRows, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_FICROPLEFT, &Importing->CropLeft, 1.0, 0.0, (double)Importing->InCols, FIOFlag_Long, NULL, NULL);
ConfigureFI(NativeWin, IDC_FICROPRIGHT, &Importing->CropRight, 1.0, 0.0, (double)Importing->InCols, FIOFlag_Long, NULL, NULL);

} // ImportWizGUI::DoPreProPanel

/*===========================================================================*/

#ifdef WCS_BUILD_VNS
void ImportWizGUI::DoCoordSysPanel(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *testObj;
GeneralEffect *myEffect;
long index;
#endif // WCS_BUILD_VNS
long select;
char buffer[2048];
const char msg[] = "COORDINATE SYSTEM SUMMARY\r\r\nThis is the coordinate system that will be used on this data set.\r\r\nIMPORTANT: This should \
always be set to the system that your data is distributed in.  This is not used to reproject your data between systems.";
const char warn[] = "\r\r\n\r\r\nNOTE: "APP_TLA" can't positively determine the coordinate system that this file uses.  Check with your data provider for \
metadata or usage guidelines.";

strcpy(buffer, msg);
if (Importing->CoordSysWarn)
	strcat(buffer, warn);
if (Importing->FileInfo)
	strcat(buffer, Importing->FileInfo);
WidgetSetText(IDC_IMWIZTEXT, buffer);
if (Importing->FileInfo)
	WidgetEMScroll(IDC_IMWIZTEXT, 0, 99);	// This scroll value currently works, since it doesn't seem to work as M$ indicates (which is good here)

switch (Importing->ElevUnits)
	{
	case DEM_DATA_UNITS_KILOM:
		select = 0;
		break;
	default:
	case DEM_DATA_UNITS_METERS:
		select = 1;
		break;
	case DEM_DATA_UNITS_DECIM:
		select = 2;
		break;
	case DEM_DATA_UNITS_CENTIM:
		select = 3;
		break;
	case DEM_DATA_UNITS_MILES:
		select = 4;
		break;
	case DEM_DATA_UNITS_FEET:
		select = 5;
		break;
	case DEM_DATA_UNITS_SURVEY_FEET:
		select = 6;
		break;
	case DEM_DATA_UNITS_INCHES:
		select = 7;
		break;
	} // Importing->ElevUnits
WidgetCBSetCurSel(IDC_CBCOORDSYS_VUNITS, select);
WidgetCBSetCurSel(IDC_CBCOORDSYS_HUNITS, Importing->GridUnits - 1);

if (Importing->IWCoordSys.GetGeographic())	// Geographic
	Importing->GridUnits = VNS_IMPORTDATA_HUNITS_DEGREES;
else
	Importing->GridUnits = VNS_IMPORTDATA_HUNITS_LINEAR;

if ((Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES))
	WidgetSetDisabled(IDC_CBCOORDSYS_HUNITS, 1);
else
	WidgetSetDisabled(IDC_CBCOORDSYS_HUNITS, 0);

#ifdef WCS_BUILD_VNS
WidgetCBClear(IDC_IMWIZ_CBCOORDS);
WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, "New Coordinate System...");
for (myEffect = EffectsHost->GetListPtr(WCS_EFFECTSSUBCLASS_COORDSYS); myEffect; myEffect = myEffect->Next)
	{
	index = WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, myEffect->GetName());
	WidgetCBSetItemData(IDC_IMWIZ_CBCOORDS, index, myEffect);
	} // for
if ((&Importing->IWCoordSys) && (Importing->FileInfo[1023] != '~'))
	{
	long listPos, numEntries;

	listPos = -1;
	numEntries = WidgetCBGetCount(IDC_IMWIZ_CBCOORDS);
	for (long ct = 0; ct < numEntries; ct++)
		{
		testObj = (CoordSys *)WidgetCBGetItemData(IDC_IMWIZ_CBCOORDS, ct);
		if (testObj != (CoordSys *)LB_ERR && testObj && Importing->IWCoordSys.Equals(testObj))
			{
			listPos = ct;
			break;
			} // for
		} // for
	if (listPos >= 0)
		WidgetCBSetCurSel(IDC_IMWIZ_CBCOORDS, listPos);	// we found a match - select it
	else
		{
		// add ours in & select it
		index = WidgetCBInsert(IDC_IMWIZ_CBCOORDS, -1, Importing->IWCoordSys.GetName());
		WidgetCBSetItemData(IDC_IMWIZ_CBCOORDS, index, &Importing->IWCoordSys);
		WidgetCBSetCurSel(IDC_IMWIZ_CBCOORDS, index);
		} // else
	} // if
else
	WidgetCBSetCurSel(IDC_IMWIZ_CBCOORDS, -1);
#endif // WCS_BUILD_VNS

if (Importing->CoordSysWarn)
	{
	ChangeCoordSys();					// make the user do something
	Importing->CoordSysWarn = false;	// don't warn again on this file
	} // if

SyncCoordSys();

if (PanelOrder[0] == 0)
	WidgetSetText(IDC_NEXT, "Import");

//Importing->GridUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
//Importing->AllowPosE = false;
//Importing->PosEast = false;

} // ImportWizGUI::DoCoordSysPanel
#endif // WCS_BUILD_VNS

/*===========================================================================*/

void ImportWizGUI::DoWrapPanel(void)
{
const char msg[] = "GLOBAL WRAP\r\r\nIf your input file is to wrap around the entire planet, check this box.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoWrapPanel

/*===========================================================================*/

void ImportWizGUI::DoTestPanel(void)
{
const char msg[] = "TEST\r\r\nYou can see the results of your settings for the input file here.  Min and Max will indicate \
the minimum & maximum values found in the input file, taking into account any pre-processing options selected.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoTestPanel

/*===========================================================================*/

void ImportWizGUI::DoRefCoPanel(void)
{
#ifdef WCS_BUILD_VNS
const char msg[] = "REFERENCE COORDINATES\r\r\nThe Project Reference Coordinates are normally set to zero, but can \
be used to indicate where data without a geographic reference is to load, or where you wish to reposition data \
to.\r\r\n\r\r\n� Ignore Reference puts the data on the planet using the Horizontal Extents Low X and Low Y fields. \
Use this option to put a "APP_TLA" DEM back to the same position on the planet.\r\r\n\r\r\n� Place 0,0 at \
Reference puts the data set origin at the Reference Coordinates.\
\r\r\n\r\r\n� Place Low X,Y at Reference puts the low coordinates of your data onto the planet at the Reference \
Coordinates.\r\r\n\r\r\n\r\r\n� The X (Lon) + to East field is used to indicate that the data is in a coordinate \
system which has the units increasing to the right. This keeps the data from being flipped in the E-W direction.";
#else // WCS_BUILD_VNS
const char msg[] = "REFERENCE COORDINATES\r\r\nThe Project Reference Coordinates are normally set to zero, but can \
be used to indicate where data without a geographic reference is to load, or where you wish to reposition data \
to.\r\r\n\r\r\n� Ignore Reference puts the data on the planet using the Horizontal Extents Low X and Low Y fields. \
Use this option to put a "APP_TLA" DEM back to the same position on the planet.\r\r\n\r\r\n� Place 0,0 at \
Reference puts the data set origin at the Reference Coordinates. This is recommended for DXF files or for files \
with unknown origins. "APP_TLA" will disable this field if it can determine that the file contains Lat/Lon or UTM \
coordinates.\r\r\n\r\r\n� Place Low X,Y at Reference puts the low coordinates of your data onto the planet at the Reference \
Coordinates.\r\r\n\r\r\n\r\r\n� The X (Lon) + to East field is used to indicate that the data is in a coordinate \
system which has the units increasing to the right. This keeps the data from being flipped in the E-W direction.";
#endif // WCS_BUILD_VNS

// Make sure this is being set from previous panel (CoordSys)
Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
#ifdef WCS_BUILD_VNS
Importing->GridUnits += VNS_IMPORTDATA_HUNITS_DEGREES;
#endif // WCS_BUILD_VNS

WidgetSetText(IDC_IMWIZTEXT, msg);

ConfigureSC(NativeWin, IDC_SCPOSEAST, &Importing->PosEast, SCFlag_Char, NULL, NULL);
WidgetSetDisabled(IDC_RADIOREF00, (char)!(Importing->AllowRef00));
WidgetSetDisabled(IDC_SCPOSEAST, (char)!(Importing->AllowPosE));

#ifdef WCS_BUILD_VNS
Importing->Reference = 3;
#else // WCS_BUILD_VNS
if (Importing->Reference == 0)	// no user choice made yet
	{
	if (strcmp(whatitis, "DXF") == 0)
		Importing->Reference = 2;	// 0,0
	else if (Importing->WantGridUnits)
		Importing->Reference = 1;	// X,Y
	else
		Importing->Reference = 3;	// Ignore
	}
#endif // WCS_BUILD_VNS

if ((strcmp(whatitis, "DXF") == 0) && (Importing->LoadAs == LAS_VECT))
	WidgetSetText(IDC_NEXT, "&Import");

ConfigureSR(NativeWin, IDC_RADIOREFXY, IDC_RADIOREFXY, &Importing->Reference, SCFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOREFXY, IDC_RADIOREF00, &Importing->Reference, SCFlag_Char, 2, NULL, NULL);
ConfigureSR(NativeWin, IDC_RADIOREFXY, IDC_RADIOREFIGNORE, &Importing->Reference, SCFlag_Char, 3, NULL, NULL);

} // ImportWizGUI::DoRefCoPanel

/*===========================================================================*/

void ImportWizGUI::DoElevUnitsPanel(void)
{
const char msg[] = "ELEVATION UNITS\r\r\n\r\r\nSet the units of elevation that the data is in.";
const char shpmsg[] = "ELEVATION UNITS\r\r\n\r\r\nIf you plan to assign elevations from an attribute record to your 2D shapefile, \
set the units of elevation that the data is in.";

if ((Importing->InFormat == VECT_DATA2_INPUT_SHAPEFILE) && (Importing->Flags != GET_ELEV_UNITS))
	WidgetSetText(IDC_IMWIZTEXT, shpmsg);
else
	WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoElevUnitsPanel

/*===========================================================================*/

void ImportWizGUI::DoGridItPanel(void)
{
const char msg[] = "GRID DATA?\r\r\n\r\r\nGridding is the process of generating a DEM from Control Points.\r\r\n\r\r\n\
"APP_TLA" only uses Control Points to create DEMs. You can either load your Control Points now so that you can either \
edit them, or add more Control Points, or you can create a DEM now by Gridding the control points.";

WidgetSetText(IDC_IMWIZTEXT, msg);

ConfigureSR(NativeWin, IDC_SRLOAD_CP, IDC_SRLOAD_CP, &IMWizGridCP, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRLOAD_CP, IDC_SRGRID_CP, &IMWizGridCP, SRFlag_Char, 1, NULL, NULL);

if (IMWizGridCP == 0)
	WidgetSetText(IDC_NEXT, "&Import");
else
	WidgetSetText(IDC_NEXT, "&Next -->");

} // ImportWizGUI::DoGridItPanel

/*===========================================================================*/

void ImportWizGUI::DoGPSStep1Panel(void)
{
const char msg[] = "GPS STEP 1\r\r\n\r\r\nTell the Import Wizard whether your file is Delimited (charaters such as commas \
separate the fields) or is a Fixed Width type (fields start in certain columns).\r\r\n\r\r\n\
Set the Start Import field to the first line of GPS data.\r\r\n\r\r\n\
Set the Datum checkbox and line number if appropriate.";

WidgetSetText(IDC_IMWIZTEXT, msg);

ConfigureFI(NativeWin, IDC_GPS_DATUMINLINE, &Importing->gpsDatum, 1.0, 1.0, 65535.0, FIOFlag_Unsigned | FIOFlag_Short, NULL, NULL);
if (Importing->gpsDatum)
	WidgetSetDisabled(IDC_GPS_DATUMINLINE, 0);
ConfigureFI(NativeWin, IDC_GPS_IMPORTATROW, &Importing->gpsStart, 1.0, 1.0, 65535.0, FIOFlag_Unsigned | FIOFlag_Short, NULL, NULL);
WidgetSetDisabled(IDC_GPS_IMPORTATROW, 0);	// should change the resource to always be enabled
WidgetSetDisabled(IDC_GPS_DATUMINLINE, !Importing->gpsHasDatum);

ConfigureSC(NativeWin, IDC_GPS_DATUMLINEENABLED, &Importing->gpsHasDatum, SCFlag_Char, NULL, NULL);

ConfigureSR(NativeWin, IDC_GPS_DELIMITED, IDC_GPS_DELIMITED, &Importing->gpsFieldType, SRFlag_Char, IMWIZ_GPS_DELIMITED, NULL, NULL);
ConfigureSR(NativeWin, IDC_GPS_DELIMITED, IDC_GPS_FIXEDWIDTH, &Importing->gpsFieldType, SRFlag_Char, IMWIZ_GPS_FIXED, NULL, NULL);

ConfigureGPSFieldsList1();

} // ImportWizGUI::DoGPSStep1Panel

/*===========================================================================*/

void ImportWizGUI::DoGPSStep2FPanel(void)
{
const char msg[] = "GPS STEP 2F\r\r\n\r\r\nMark the columns to be used for making fields.  Click to set a column marker, \
or to clear a previously set marker.  Press the NEXT button when you're done setting columns.";

WidgetSetText(IDC_IMWIZTEXT, msg);

ConfigureGPSFieldsList2F();

} // ImportWizGUI::DoGPSStep2FPanel

/*===========================================================================*/

void ImportWizGUI::DoGPSStep2DPanel(void)
{
const char msg[] = "GPS STEP 2D\r\r\n\r\r\nSet the Delimiter (the character which separates the fields).\r\r\n\r\r\n\
Collapse Consecutive Delimiters will eliminate fields if the same Delimiter appears consecutively.  This is a good way \
to eliminate empty fields if the same field is empty in the whole file.\r\r\n\r\r\nThe Text Qualifier should be set if \
a field is quoted in order to treat it as a single item.";

WidgetSetText(IDC_IMWIZTEXT, msg);
WidgetCBSetCurSel(IDC_GPS_TEXTQUAL, Importing->gpsTextQualifier);
WidgetSetDisabled(IDC_OTHERDELIM, (char)(Importing->gpsDOther == 0));
WidgetSCSync(IDC_GPS_DEL_TAB, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_GPS_DEL_SEMI, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_GPS_DEL_COMMA, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_GPS_DEL_SPACE, WP_SCSYNC_NONOTIFY);
WidgetSCSync(IDC_GPS_DEL_OTHER, WP_SCSYNC_NONOTIFY);

ConfigureGPSFieldsList2D();

} // ImportWizGUI::DoGPSStep2DPanel

/*===========================================================================*/

void ImportWizGUI::DoGPSStep3Panel(void)
{
const char msg[] = "GPS STEP 3\r\r\n\r\r\nSelect a column and change the 'Field is' selection to change the way a field \
is handled.";

WidgetSetText(IDC_IMWIZTEXT, msg);
WidgetSetText(IDC_NEXT, "Import");

ConfigureGPSFieldsList3();

} // ImportWizGUI::DoGPSStep3Panel

/*===========================================================================*/

/***
#ifndef WCS_BUILD_VNS

void ImportWizGUI::DoHorExPanel(void)
{
char demmsg[] = "HORIZONTAL EXTENTS\r\r\nThis shows the number of elevation points in the DEM, the area that \
each elevation point covers, and where the DEM is positioned on the planet.\r\r\n\r\r\n� The size of the DEM is \
shown in the Cols & Rows field.\r\r\n\r\r\n� Grid Units indicate the unit of measure used for Grid Spacing.\r\r\n\
\r\r\n� Grid Spacing is the distance between elevation points.  Changing Grid Spacing modifies the Width or \
Height.\r\r\n\r\r\n� Total DEM Size is the edge to edge distance covered by the DEM.\r\r\n\r\r\n\
� Changing Width or Height adjusts the area that this DEM covers, and will change the Grid Spacing.\r\r\n\r\r\n\
� Entering values directly into the High & Low Lat/Lon fields will change the corresponding Width, Height and \
Grid Spacing fields.";
char nextmsg[] = "\r\r\n\r\r\n� The NEXT button will be disabled unless the North bound is greater than the South \
bound and the West bound is greater than the East bound.";
char imgmsg[] = "HORIZONTAL EXTENTS\r\r\n\r\r\nThe size of the input image is shown in the Cols & Rows field.";
char msg[1200];
char gridsize[32];
char disability;	// really boolean
long inCols, inRows;

if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
	{
	inCols = Importing->InRows;
	inRows = Importing->InCols;
	}
else
	{
	inCols = Importing->InCols;
	inRows = Importing->InRows;
	}

if (strcmp(Importing->MyIdentity, "TERRAGEN") == 0)
	{
	Importing->NBound = ConvertMetersToDeg(Importing->North * (inRows - 1), Importing->SBound, IWGlobeRad)
		+ Importing->SBound;
	Importing->WBound = ConvertMetersToDeg(Importing->West * (inCols - 1), Importing->SBound, IWGlobeRad)
		+ Importing->EBound;
	}

if (Importing->LoadAs == LAS_IMAGE)
	{
	WidgetSetText(IDC_IMWIZTEXT, imgmsg);
	WidgetSetDisabled(IDC_NEXT, 0);
	}
else
	{
	if (GoodBounds())
		{
		WidgetSetDisabled(IDC_NEXT, 1);
		strcpy(msg, demmsg);
		strcat(msg, nextmsg);
		WidgetSetText(IDC_IMWIZTEXT, msg);
		}
	else
		WidgetSetText(IDC_IMWIZTEXT, demmsg);
	}

if ((Importing->InWidth == 0) || (Importing->InHeight == 0))
	DataOpsDimsFromBounds();
sprintf(gridsize, "Cols: %d, Rows: %d", inCols, inRows);
WidgetSetText(IDC_TXTHOREXSIZE, gridsize);
SetGridSpacing();
ConfigureFI(NativeWin, IDC_FIHOREXGRIDNS, &Importing->GridSpaceNS, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXGRIDWE, &Importing->GridSpaceWE, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXGRIDSIZENS, &IWGridSizeNS, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXGRIDSIZEWE, &IWGridSizeWE, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXWIDTH, &Importing->InWidth, 0.025, 0.0, 720.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXHEIGHT, &Importing->InHeight, 0.025, 0.0, 180.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXN, &Importing->NBound, 0.025, -90.0, 90.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXS, &Importing->SBound, 0.025, -90.0, 90.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXE, &Importing->EBound, 0.025, -1000.0, 1000.0, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIHOREXW, &Importing->WBound, 0.025, -1000.0, 1000.0, FIOFlag_Double, NULL, NULL);

if (Importing->LoadAs == LAS_IMAGE)
	disability = true;
else if ((Importing->LoadAs == LAS_VECT) && ((Importing->HasUTM) || !(Importing->WantGridUnits)))
	disability = true;
else if ((Importing->LoadAs == LAS_CP) && ((Importing->HasUTM) || !(Importing->WantGridUnits)))
	disability = true;
else
	disability = false;
WidgetSetDisabled(IDC_CBHOREX_UNITS, disability);
// disable horizontal extents unless it's a DEM
if (Importing->LoadAs == LAS_DEM)
	disability = false;
else
	disability = true;
WidgetSetDisabledRO(IDC_FIHOREXGRIDNS, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDWE, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDSIZENS, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDSIZEWE, disability);
WidgetSetDisabledRO(IDC_FIHOREXWIDTH, disability);
WidgetSetDisabledRO(IDC_FIHOREXHEIGHT, disability);
WidgetSetDisabledRO(IDC_FIHOREXN, disability);
WidgetSetDisabledRO(IDC_FIHOREXS, disability);
WidgetSetDisabledRO(IDC_FIHOREXE, disability);
WidgetSetDisabledRO(IDC_FIHOREXW, disability);

if (Importing->WantGridUnits)
	{
	WidgetSetText(IDC_FIHOREXN, "High Y ");
	WidgetSetText(IDC_FIHOREXS, "Low Y ");
	WidgetSetText(IDC_FIHOREXE, "Low X ");
	WidgetSetText(IDC_FIHOREXW, "High X ");
	}

} // ImportWizGUI::DoHorExPanel

***/

//#else // !WCS_BUILD_VNS

void ImportWizGUI::DoHorExPanel(void)
{
const char demmsg[] = "HORIZONTAL EXTENTS\r\r\nThis shows the number of elevation points in the DEM, the area that \
each elevation point covers, and where the DEM is positioned on the planet.\r\r\n\r\r\n� The size of the DEM is \
shown in the Cols & Rows field.\r\r\n\r\r\n� Grid Units indicate the unit of measure used for Grid Spacing.\r\r\n\
\r\r\n� Grid Spacing is the distance between elevation points.  Changing Grid Spacing modifies the Width or \
Height.\r\r\n\r\r\n� Total DEM Size is the edge to edge distance covered by the DEM.\r\r\n\r\r\n\
� Changing Width or Height adjusts the area that this DEM covers, and will change the Grid Spacing.\r\r\n\r\r\n\
� Entering values directly into the High & Low Lat/Lon fields will change the corresponding Width, Height and \
Grid Spacing fields.";
/***
const char nextmsg[] = "\r\r\n\r\r\n� The NEXT button will be disabled unless the North bound is greater than the South \
bound and the West bound is greater than the East bound.";
***/
const char imgmsg[] = "HORIZONTAL EXTENTS\r\r\n\r\r\nThe size of the input image is shown in the Cols & Rows field.";
//char msg[1200];
char gridsize[32];
char disability;	// really boolean
long inCols, inRows;

// need to scale DEM bounds to meters (F2 070302)
if (strcmp(whatitis, "ARC_EXPORT_DEM") == 0)
	{
	Importing->NBound *= Importing->HScale;
	Importing->SBound *= Importing->HScale;
	Importing->WBound *= Importing->HScale;
	Importing->EBound *= Importing->HScale;
	SetHScale(1);	// reset HScale to unity so we don't rescale later
	} // if

IWGlobeRad = Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
AnimPar[IW_ANIMPAR_GRIDNS].CurValue = Importing->GridSpaceNS;
AnimPar[IW_ANIMPAR_GRIDWE].CurValue = Importing->GridSpaceWE;
AnimPar[IW_ANIMPAR_GRIDSIZENS].CurValue = IWGridSizeNS;
AnimPar[IW_ANIMPAR_GRIDSIZEWE].CurValue = IWGridSizeWE;
AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight;
AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth;

Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
#ifdef WCS_BUILD_VNS
Importing->GridUnits += VNS_IMPORTDATA_HUNITS_DEGREES;
if (Importing->IWCoordSys.GetGeographic())
	SetBoundsType(VNS_BOUNDSTYPE_DEGREES);
else
	SetBoundsType(VNS_BOUNDSTYPE_LINEAR);
#endif // WCS_BUILD_VNS

if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) || (Importing->InFormat == DEM_DATA2_INPUT_MDEM))
	{
	inCols = Importing->InRows;
	inRows = Importing->InCols;
	} // if
else
	{
	inCols = Importing->InCols;
	inRows = Importing->InRows;
	} // else

// Hide Degree Width & Height if VNS & Linear
//if (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR)
//	{
//	WidgetShow(IDC_FIHOREXWIDTH, 0);
//	WidgetShow(IDC_FIHOREXHEIGHT, 0);
//	}
//else
//	{
//	WidgetShow(IDC_FIHOREXWIDTH, 1);
//	WidgetShow(IDC_FIHOREXHEIGHT, 1);
//	}

if (strcmp(Importing->MyIdentity, "TERRAGEN") == 0)
	{
	Importing->NBound = ConvertMetersToDeg(Importing->North * (inRows - 1), Importing->SBound, IWGlobeRad)
		+ Importing->SBound;
	if (Importing->PosEast)
		Importing->WBound = Importing->EBound - ConvertMetersToDeg(Importing->West * (inCols - 1), Importing->SBound, IWGlobeRad);
	else
		Importing->WBound = ConvertMetersToDeg(Importing->West * (inCols - 1), Importing->SBound, IWGlobeRad) + Importing->EBound;
	} // if

if (Importing->LoadAs == LAS_IMAGE)
	{
	WidgetSetText(IDC_IMWIZTEXT, imgmsg);
	WidgetSetDisabled(IDC_NEXT, 0);
	} // if
else
	{
	WidgetSetText(IDC_IMWIZTEXT, demmsg);
	} // else

#ifdef WCS_BUILD_VNS
if (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES)
	WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 0);
else
	WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 1);
#else // WCS_BUILD_VNS
WidgetCBSetCurSel(IDC_CBHOREX_UNITS, Importing->GridUnits);
#endif // WCS_BUILD_VNS

if ((Importing->InWidth == 0) || (Importing->InHeight == 0))
	DataOpsDimsFromBounds();
sprintf(gridsize, "Cols: %d, Rows: %d", inCols, inRows);
WidgetSetText(IDC_TXTHOREXSIZE, gridsize);
//SetGridSpacing();
//ConfigureFI(NativeWin, IDC_FIHOREXGRIDNS, &Importing->GridSpaceNS, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXGRIDWE, &Importing->GridSpaceWE, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXGRIDSIZENS, &IWGridSizeNS, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXGRIDSIZEWE, &IWGridSizeWE, 0.1, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXWIDTH, &Importing->InWidth, 0.025, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXHEIGHT, &Importing->InHeight, 0.025, 0.0, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXN, &Importing->NBound, 5.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXS, &Importing->SBound, 5.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXE, &Importing->EBound, 5.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
//ConfigureFI(NativeWin, IDC_FIHOREXW, &Importing->WBound, 5.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
SetSNDefaults();
WidgetSNConfig(IDC_FIHOREXGRIDNS, &AnimPar[IW_ANIMPAR_GRIDNS]);
WidgetSNConfig(IDC_FIHOREXGRIDWE, &AnimPar[IW_ANIMPAR_GRIDWE]);
WidgetSNConfig(IDC_FIHOREXGRIDSIZENS, &AnimPar[IW_ANIMPAR_GRIDSIZENS]);
WidgetSNConfig(IDC_FIHOREXGRIDSIZEWE, &AnimPar[IW_ANIMPAR_GRIDSIZEWE]);
WidgetSNConfig(IDC_FIHOREXWIDTH, &AnimPar[IW_ANIMPAR_WIDTH]);
WidgetSNConfig(IDC_FIHOREXHEIGHT, &AnimPar[IW_ANIMPAR_HEIGHT]);
WidgetSNConfig(IDC_FIHOREXN, &AnimPar[IW_ANIMPAR_N]);
WidgetSNConfig(IDC_FIHOREXS, &AnimPar[IW_ANIMPAR_S]);
WidgetSNConfig(IDC_FIHOREXE, &AnimPar[IW_ANIMPAR_E]);
WidgetSNConfig(IDC_FIHOREXW, &AnimPar[IW_ANIMPAR_W]);

WidgetCBSetCurSel(IDC_CBHOREX_BOUNDS, Importing->BoundsType);
if (Importing->BoundTypeLocked)
	WidgetSetDisabled(IDC_CBHOREX_BOUNDS, 1);
else
	WidgetSetDisabled(IDC_CBHOREX_BOUNDS, 0);

// Are they allowed to alter the Bounds Degree/Linear drop box?
//disability = false;
//if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
//	disability = true;
//WidgetSetDisabled(IDC_CBHOREX_BOUNDS, disability);

// Are they allowed to alter the Grid drop box?
disability = false;
if (Importing->LoadAs == LAS_IMAGE)
	disability = true;
else if ((Importing->LoadAs == LAS_VECT) && ((Importing->HasUTM) || !(Importing->WantGridUnits)))
	disability = true;
else if ((Importing->LoadAs == LAS_CP) && ((Importing->HasUTM) || !(Importing->WantGridUnits)))
	disability = true;
else if (Importing->BoundsType == VNS_BOUNDSTYPE_LINEAR)
	disability = true;
else
	disability = false;
WidgetSetDisabled(IDC_CBHOREX_UNITS, disability);

// disable horizontal extents unless it's a DEM
if (Importing->LoadAs == LAS_DEM)
	disability = false;
else
	disability = true;
WidgetSetDisabledRO(IDC_FIHOREXGRIDNS, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDWE, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDSIZENS, disability);
WidgetSetDisabledRO(IDC_FIHOREXGRIDSIZEWE, disability);
WidgetSetDisabledRO(IDC_FIHOREXWIDTH, disability);
WidgetSetDisabledRO(IDC_FIHOREXHEIGHT, disability);
WidgetSetDisabledRO(IDC_FIHOREXN, disability);
WidgetSetDisabledRO(IDC_FIHOREXS, disability);
WidgetSetDisabledRO(IDC_FIHOREXE, disability);
WidgetSetDisabledRO(IDC_FIHOREXW, disability);

DoHorExLabels();
SetGridSpacing();

if (Importing->LoadAs == LAS_IMAGE)
	disability = 0;
else
	disability = (char)VNS_HorexCheckFails();
WidgetSetDisabled(IDC_NEXT, disability);

} // ImportWizGUI::DoHorExPanel

//#endif // !WCS_BUILD_VNS

/*===========================================================================*/

void ImportWizGUI::DoHorUnitsPanel(void)
{
char msg[1024];
const char intromsg[] = "HORIZONTAL COORDINATES\r\r\nSelect the coordinate system your data is using.\r\r\n\r\r\n� If your \
coordinate values are in decimal degrees, use Lat/Lon.\r\r\n\r\r\n� If your data is projected into the UTM coordinate \
system, select UTM.\r\r\n\r\r\n� If you coordinates are in meters and not UTM projected, or are in another unit of \
measure, such as feet, use arbitrary.\r\r\n\r\r\n";
const char latlonmsg[] = "Your data appears to be in Lat/Lon coordinates.  If it's in UTM or arbitrary units, select that \
option instead.";
const char utmmsg[] = "Your data appears to be in UTM coordinates.  If it's in Lat/Lon or arbitrary units, select that \
option instead.";
const char othermsg[] = "You have told "APP_TLA" that your data is in arbitrary coordinates.  If it's in Lat/Lon or UTM \
coordinates, use that option instead.";

strcpy(msg, intromsg);
switch (Importing->Mode)
	{
	default:
	case IMWIZ_HORUNITS_LATLON:
		strcat(msg, latlonmsg);
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 0);	// Degrees
		Importing->AllowPosE = true;
		break;
	case IMWIZ_HORUNITS_UTM:
		strcat(msg, utmmsg);
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		Importing->AllowPosE = false;
		break;
	case IMWIZ_HORUNITS_ARBITRARY:
		strcat(msg, othermsg);
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		Importing->AllowPosE = true;
		break;
	} // switch

ConfigureSR(NativeWin, IDC_SCHORUNITS_LATLON, IDC_SCHORUNITS_LATLON, &Importing->Mode, SRFlag_Char,
			IMWIZ_HORUNITS_LATLON, NULL, NULL);
ConfigureSR(NativeWin, IDC_SCHORUNITS_LATLON, IDC_SCHORUNITS_UTM, &Importing->Mode, SRFlag_Char,
			IMWIZ_HORUNITS_UTM, NULL, NULL);
ConfigureSR(NativeWin, IDC_SCHORUNITS_LATLON, IDC_SCHORUNITS_ARBITRARY, &Importing->Mode, SRFlag_Char,
			IMWIZ_HORUNITS_ARBITRARY, NULL, NULL);

if (RandomCP)
	WidgetSetDisabled(IDC_SCHORUNITS_ARBITRARY, 1);

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoHorUnitsPanel

/*===========================================================================*/

void ImportWizGUI::DoLoadAsPanel(void)
{
char msg[1024] = "LOAD AS\r\r\nThis file type can be loaded as any of the following "APP_TLA" objects:\r\r\n";
const char demstr[] = "\r\r\n� DEM - This is a 2D grid of regularly spaced elevation values.  Arrays or tables of data \
must be ordered to be loaded as a DEM.\r\r\n\r\r\n� The Disable NULLed flag controls whether 'tiles' composed entirely of NULL data are \
disabled.\r\r\n";
const char cpstr[] = "\r\r\n� Control Points - These represent spot elevations.  They don't have to be spaced regularly and \
can be unordered. The control points have to be converted to a DEM through the process of gridding, where the \
data is reorganized, resampled, and interpolated.\r\r\n";
const char imagestr[] = "\r\r\n� Image - You wish to save the input file as an image format, or a raw binary output.\r\r\n";
const char vectstr[] = "\r\r\n� Vector - These are objects that represent points, lines, or connected lines or points in the \
world. They are typically output from GIS systems or drawn manually.\r\r\n\r\r\n� The Render Enabled flag allows you to set or clear \
the corresponding Database flag for this entire set of data.\r\r\n";

if (! Importing->LoadAs)
	{
	switch (Importing->LoadOpt)
		{
		case LAS_CP:
		case LAS_IMAGE:
		case LAS_VECT:
			Importing->LoadAs = Importing->LoadOpt;		// if they only have one choice, set it
			break;
		case (LAS_VECT | LAS_CP):
			Importing->LoadAs = LAS_VECT;
			break;
		default:
			// all other multiple choices allow DEM, set as default (unless better default for type - such as vector for DXF)
			if (strcmp(intype, "DXF") == 0)
				{
				if (Importing->Flags == DXF_3DFACE)
					Importing->LoadAs = LAS_CP;
				else
					Importing->LoadAs = LAS_VECT;
				} // if
			else
				Importing->LoadAs = LAS_DEM;
			break;
		} // switch
	} // if

ConfigureSR(NativeWin, IDC_LOADASDEM, IDC_LOADASDEM, &Importing->LoadAs, SRFlag_Char, LAS_DEM, NULL, NULL);
ConfigureSR(NativeWin, IDC_LOADASDEM, IDC_LOADASCP, &Importing->LoadAs, SRFlag_Char, LAS_CP, NULL, NULL);
ConfigureSR(NativeWin, IDC_LOADASDEM, IDC_LOADASIMAGE, &Importing->LoadAs, SRFlag_Char, LAS_IMAGE, NULL, NULL);
ConfigureSR(NativeWin, IDC_LOADASDEM, IDC_LOADASVECTOR, &Importing->LoadAs, SRFlag_Char, LAS_VECT, NULL, NULL);
// Enable all
WidgetSetDisabled(IDC_LOADASDEM, 0);
WidgetSetDisabled(IDC_LOADASCP, 0);
WidgetSetDisabled(IDC_LOADASIMAGE, 0);
WidgetSetDisabled(IDC_LOADASVECTOR, 0);
// Disable irrelevant options
if (!(Importing->LoadOpt & LAS_DEM))
	WidgetSetDisabled(IDC_LOADASDEM, 1);
if (!(Importing->LoadOpt & LAS_CP))
	WidgetSetDisabled(IDC_LOADASCP, 1);
if (!(Importing->LoadOpt & LAS_IMAGE))
	WidgetSetDisabled(IDC_LOADASIMAGE, 1);
if (!(Importing->LoadOpt & LAS_VECT))
	WidgetSetDisabled(IDC_LOADASVECTOR, 1);
// Text for enabled options
if (Importing->LoadOpt & LAS_DEM)
	strcat(msg, demstr);
if (Importing->LoadOpt & LAS_CP)
	strcat(msg, cpstr);
if (Importing->LoadOpt & LAS_IMAGE)
	strcat(msg, imagestr);
if (Importing->LoadOpt & LAS_VECT)
	strcat(msg, vectstr);
// kill the last \r\r\n
msg[strlen(msg) - 3] = 0;
WidgetSetText(IDC_IMWIZTEXT, msg);
if (Importing->LoadAs == LAS_DEM)
	WidgetSetDisabled(IDC_SCDISABLENULLTILES, 0);
else
	WidgetSetDisabled(IDC_SCDISABLENULLTILES, 1);
if ((Importing->LoadAs == LAS_VECT) || (Importing->LoadAs == LAS_CP))
	WidgetSetDisabled(IDC_SCRENDERENABLED, 0);
else
	WidgetSetDisabled(IDC_SCRENDERENABLED, 1);

WidgetSetDisabled(IDC_NEXT, 0);

if (Importing->express)
	WidgetSetText(IDC_NEXT, "&Import");
else
	WidgetSetText(IDC_NEXT, "&Next -->");

} // ImportWizGUI::DoLoadAsPanel

/*===========================================================================*/

void ImportWizGUI::DoLonDirPanel(void)
{
const char msg[] = "LONGITUDE POSITIVE DIRECTION\r\r\n\r\r\n"APP_TLA" considers positive longitude to be west of \
the Prime Meridian internally. Set this to inform "APP_TLA" on the direction your data considers positive.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoLonDirPanel

/*===========================================================================*/

void ImportWizGUI::DoBinHdrSize(void)
{

WidgetCBSetCurSel(IDC_CBICRHEAD_BINFACTORS, 0); // brand new list
DoBinFactors();

} // ImportWizGUI::DoBinHdrSize

/*===========================================================================*/

void ImportWizGUI::DoElevMethodPanel(void)
{
const char msg[] = "COLOR TO ELEVATION METHOD\r\r\n\r\r\nWCS needs to know which method of computing elevation you \
wish to use.  The red, green, and blue (RGB) components of the image are used to determine elevation.\r\r\n\r\r\n\
� Additive - Elevation is equal to R+G+B and therefore ranges from 0 to 765.\r\r\n\r\r\n� 16 bit - Elevation is \
treated like a 16 bit unsigned value in POVray format (R * 256 + G), therefore the elevation range is 0 to 65535.\
\r\r\n\r\r\n� 24 bit - Elevation is treated as an unsigned 24 bit RGB value (R * 65536 + G * 256 + B), therefore \
the elevation range is 0 to 16777215.";

WidgetSetText(IDC_IMWIZTEXT, msg);

switch (IMWizElevMethod)
	{
	default: break;
	case 1:
		Importing->Flags = ELEV_METHOD_ADD;
		break;
	case 2:
		Importing->Flags = ELEV_METHOD_16BIT;
		break;
	case 3:
		Importing->Flags = ELEV_METHOD_24BIT;
		break;
	} // switch

ConfigureSR(NativeWin, IDC_SRCOLEL_ADD, IDC_SRCOLEL_ADD, &IMWizElevMethod, SCFlag_Char, 1, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRCOLEL_ADD, IDC_SRCOLEL_16BIT, &IMWizElevMethod, SCFlag_Char, 2, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRCOLEL_ADD, IDC_SRCOLEL_24BIT, &IMWizElevMethod, SCFlag_Char, 3, NULL, NULL);

SetElevMethodHeights();

} // ImportWizGUI::DoElevMethodPanel

/*===========================================================================*/

void ImportWizGUI::DoWCSKnowsPanel(void)
{
const char msgdem[] = "DATA POSITIONING\r\r\n\r\r\n"APP_TLA" knows all it needs in order to place this DEM on your \
planet. You may now load it as is, or you may change the placement of the DEM. You may also alter the elevations, \
the cell size, or the cropping.";
const char msgmerge[] = "DATA POSITIONING\r\r\n\r\r\n"APP_TLA" is now ready to load your region.";
const char msgdata[] = "DATA POSITIONING\r\r\n\r\r\n"APP_TLA" knows all it needs in order to place this data on your \
planet. You may now load them as is, or you may change their positioning.";

if (Importing->LoadAs == LAS_DEM)
	{
	if (MergeLoad)
		WidgetSetText(IDC_IMWIZTEXT, msgmerge);
	else
		WidgetSetText(IDC_IMWIZTEXT, msgdem);
	} // if
else
	WidgetSetText(IDC_IMWIZTEXT, msgdata);

if (Importing->WantGridUnits)
	knowhow = 1;

if (knowhow == 0)
	WidgetSetText(IDC_NEXT, "&Import");
else
	WidgetSetText(IDC_NEXT, "&Next -->");

ConfigureSR(NativeWin, IDC_SRKNOWLOAD, IDC_SRKNOWLOAD, &knowhow, SRFlag_Char, 0, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRKNOWLOAD, IDC_SRKNOWCHANGE, &knowhow, SRFlag_Char, 1, NULL, NULL);

if ((Importing->InFormat == VECT_DATA2_INPUT_SHAPEFILE) || (strcmp(Importing->MyIdentity, "USGS_DLG") == 0) ||
	(strcmp(Importing->MyIdentity, "SDTS_DLG") == 0))
	WidgetSetDisabled(IDC_SRKNOWCHANGE, 1);

if (MergeLoad)
	{
	WidgetSetDisabled(IDC_SRKNOWCHANGE, 1);
	} // if
else if ((strcmp(Importing->MyIdentity, "SDTS_DEM") == 0) || (strcmp(Importing->MyIdentity, "USGS_DEM") == 0))
	{
	WidgetSetDisabled(IDC_SRKNOWCHANGE, 1);
	} // else if

WidgetSetDisabled(IDC_NEXT, 0);

} // ImportWizGUI::DoWCSKnowsPanel

/*===========================================================================*/

void ImportWizGUI::DoWCSUnitsPanel(void)
{
char buffer[2048];
const char msg[] = "UNITS\r\r\nSelect the coordinate system your data is using, and the units of measure.\r\r\n\r\r\n� If your \
coordinate values are in decimal degrees, use Lat/Lon.\r\r\n\r\r\n� If your data is projected into the UTM coordinate \
system, select UTM.\r\r\n\r\r\n� If you coordinates are in meters and not UTM projected, or are in another unit of \
measure, such as feet, use arbitrary.\r\r\n\r\r\n";
const char latlonmsg[] = "Your data appears to be in Lat/Lon coordinates.  If it's in UTM or arbitrary units, select that \
option instead.";
const char utmmsg[] = "Your data appears to be in UTM coordinates.  If it's in Lat/Lon or arbitrary units, select that \
option instead.";
const char othermsg[] = "You have told "APP_TLA" that your data is in arbitrary coordinates.  If it's in Lat/Lon or UTM \
coordinates, use that option instead.";
char disability;

strcpy(buffer, msg);
ConfigureSR(NativeWin, IDC_SRWCSUNITS_LATLON, IDC_SRWCSUNITS_LATLON, &Importing->Mode, SRFlag_Char, IMWIZ_HORUNITS_LATLON, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRWCSUNITS_LATLON, IDC_SRWCSUNITS_UTM, &Importing->Mode, SRFlag_Char, IMWIZ_HORUNITS_UTM, NULL, NULL);
ConfigureSR(NativeWin, IDC_SRWCSUNITS_LATLON, IDC_SRWCSUNITS_ARBITRARY, &Importing->Mode, SRFlag_Char, IMWIZ_HORUNITS_ARBITRARY, NULL, NULL);
if (Importing->Mode == IMWIZ_HORUNITS_LATLON)
	disability = true;
else
	disability = false;
WidgetSetDisabled(IDC_CBWCSUNITS_HUNITS, disability);
if (Importing->Mode == IMWIZ_HORUNITS_UTM)
	disability = false;
else
	disability = true;
WidgetSetDisabled(IDC_CBWCSUNITS_UTMZONE, disability);
switch (Importing->Mode)
	{
	default:
	case IMWIZ_HORUNITS_LATLON:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 0);	// Degrees
		strcat(buffer, latlonmsg);
		break;
	case IMWIZ_HORUNITS_UTM:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		strcat(buffer, utmmsg);
		if (Importing->UTMZone == 0)
			{
			if (GlobalApp->MainProj->Prefs.LastUTMZone == 0)
				GlobalApp->MainProj->Prefs.LastUTMZone = 13;
			Importing->UTMZone = GlobalApp->MainProj->Prefs.LastUTMZone;
			}
		if (Importing->UTMZone > 0)
			WidgetCBSetCurSel(IDC_CBWCSUNITS_UTMZONE, Importing->UTMZone - 1);
		else
			WidgetCBSetCurSel(IDC_CBWCSUNITS_UTMZONE, -Importing->UTMZone + 59);
		break;
	case IMWIZ_HORUNITS_ARBITRARY:
		WidgetCBSetCurSel(IDC_CBHOREX_UNITS, 2);	// Meters
		strcat(buffer, othermsg);
		break;
	}
if (Importing->FileInfo)
	strcat(buffer, Importing->FileInfo);
WidgetSetText(IDC_IMWIZTEXT, buffer);
if (Importing->FileInfo)
	WidgetEMScroll(IDC_IMWIZTEXT, 0, 99);	// This scroll value currently works, since it doesn't seem to work as M$ indicates (which is good here)

} // ImportWizGUI::DoWCSUnitsPanel

/*===========================================================================*/

void ImportWizGUI::DoVertExPanel(void)
{
const char msg[] = "VERTICAL EXTENTS\r\r\nThese settings control the interpretation of the elevations.\r\r\n\r\r\n\
톉ou can change what unit of elevation "APP_TLA" interprets the data as here.\r\r\n\r\r\n\
톉ou can change the elevation range by changing the Low Elevation and/or High Elevation fields. This allows \
you to flatten, exaggerate, or invert the data.\r\r\n\r\r\n톂he Elev Modifier will adjust both fields by the \
same amount.\r\r\n\r\r\n� You can switch to an Output Scaling page if you'd rather establish input/output \
relationships.";

WidgetSetText(IDC_IMWIZTEXT, msg);

Importing->UserMax = SCALE_TESTMAX;
Importing->UserMin = SCALE_TESTMIN;
if (Importing->UserMax == FLT_MIN)	//lint !e777
	Importing->UserMax = 0.0;
if (Importing->UserMin == FLT_MAX)	//lint !e777
	Importing->UserMin = 0.0;
Importing->UserMinSet = false;
Importing->UserMaxSet = false;
ConfigureFI(NativeWin, IDC_FIVERTEXSCALE, &Importing->VertExag, 0.1, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIVERTEXDATUM, &Importing->DatumChange, 100.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIVERTEXHIGHEL, &Importing->UserMax, 100.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIVERTEXLOWEL, &Importing->UserMin, 100.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);

NewVertStyle = true;

//if (stricmp(Importing->MyIdentity, "BINARY") == 0)
//	WidgetSetDisabled(IDC_BUTTONVERTEXTEST, 0);

} // ImportWizGUI::DoVertExPanel

/*===========================================================================*/

void ImportWizGUI::DoSaveGeoPanel(void)
{
char msg[] = "SAVE GEOGRAPHIC REFERENCING INFORMATION?\r\r\n\r\r\nYour input file contains Geographic \
referencing information.  Do you wish to save this info with the output image?";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoSaveGeoPanel

/*===========================================================================*/

//#ifdef WCS_BUILD_VNS
void ImportWizGUI::DoShapeOptsPanel(void)
{
char buffer[4096];
const char msg[] = "SHAPE OPTIONS\r\r\n\r\r\n텾ositive E Longitude: When checked, eastern hemisphere data is positive, \
western hemisphere data is negative.\r\r\n\r\r\n텮ttach Layers: Layers group objects together in the database. Database \
items can be enabled & disabled via their layers.\r\r\n\r\r\n텮ssign DB Names from an Attribute Field: Normally database \
names are derived from the file name and the shape number.  Checking this will prompt you to select a field to obtain the \
database name from.\r\r\n\r\r\n텮ssign Elevations from Attribute Field: With this option, you can use a field to set the vertex \
elevation from a 2D Shapefile.\r\r\n\r\r\n";
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
const char attmsg[] = "텹oad Attributes: Attributes attached to database entries can be queried to set up Thematic Maps.\r\r\n\r\r\n";
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
const char msg2[] = "텰onform to Terrain: Vector vertices will be conformed to the terrain that already exists in the database.\
\r\r\n\r\r\n텮dd File Layer: All entities in the file will be added to a layer with the name from the adjacent text field.\r\r\n\r\r\n";

if (Importing->IWCoordSys.GetGeographic())
	WidgetSetDisabled(IDC_SCSHAPEOPTS_POSEAST, 1);
if (Importing->FileInfo && strstr(Importing->FileInfo, "3D shape"))
	WidgetSetDisabled(IDC_SCSHAPEOPTS_DOELEVS, 1);
	
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
WidgetShow(IDC_SCSHAPEOPTS_LOADATTR, 1);
#endif // WCS_SUPPORT_GENERIC_ATTRIBS

strcpy(buffer, msg);
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
strcat(buffer, attmsg);
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
strcat(buffer, msg2);
strcat(buffer, AttributeList);
WidgetSetText(IDC_IMWIZTEXT, buffer);
#ifdef WCS_SUPPORT_GENERIC_ATTRIBS
WidgetEMScroll(IDC_IMWIZTEXT, 0, 25);	// set to start of attribute list
#endif // WCS_SUPPORT_GENERIC_ATTRIBS
WidgetSetText(IDC_EDITSHAPEOPTS_FILELAYERNAME, Importing->LayerName);

WidgetSetText(IDC_NEXT, "Import");

} // ImportWizGUI::DoShapeOptsPanel
//#endif // WCS_BUILD_VNS

/*===========================================================================*/

void ImportWizGUI::DoOutTilesPanel(void)
{
const char demmsg[] = "OUTPUT DEMS\r\r\n\r\r\nEach DEM is loaded into memory as it's being rendered. These values are \
closest to the recommended DEM size of  300 x 300. The render engine will have to load the entire DEM into \
memory if any portion of that DEM is visible. Breaking even a \"normal\" sized DEM into smaller pieces can \
sometimes increase rendering speed.\r\r\n\r\r\n� If for example, you're just NE of the DEM center and looking \
towards the NE corner, if this DEM had been subdivided into 2 x 2 DEMs, only the NE tile from this DEM would have \
to be loaded.\r\r\n\r\r\n� The Output Columns & Output Rows normally match the Input Columns & Input Rows. If you \
change these values, the data will be resampled.  Entering a value of zero will reset the number to match the \
input number.\r\r\n\r\r\n� Spline Constrain will only be enabled if the input files are resampled to create the \
output files.";
const char imgmsg[] = "OUTPUT IMAGE\r\r\n\r\r\nSet the size of the output image here.  The default is the size of the \
input image.";
char restxt[80];

if (Importing->LoadAs == LAS_IMAGE)
	WidgetSetText(IDC_IMWIZTEXT, imgmsg);
else
	WidgetSetText(IDC_IMWIZTEXT, demmsg);

if ((OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM) &&
	(Importing->InCols * Importing->InRows) > (301 * 301)) // will output DEM size be greater than recommended?
	{
	if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
		{
		Importing->OutDEMWE = (Importing->InRows - 1) / 301 + 1;
		Importing->OutDEMNS = (Importing->InCols - 1) / 301 + 1;
		} // if
	else
		{
		Importing->OutDEMWE = (Importing->InCols - 1) / 301 + 1;
		Importing->OutDEMNS = (Importing->InRows - 1) / 301 + 1;
		} // else
	} // if

if (Importing->OutCols == 0)
	Importing->OutCols = CROP_COLS;
if (Importing->OutRows == 0)
	Importing->OutRows = CROP_ROWS;
if (!Importing->KeepBounds)
	{
	Importing->OutCols = CROP_COLS;
	Importing->OutRows = CROP_ROWS;
	} // if

if (Importing->LoadAs == LAS_IMAGE)
	{
	WidgetSetDisabledRO(IDC_FIOUTDEMNS, true);
	WidgetSetDisabledRO(IDC_FIOUTDEMWE, true);
	WidgetSetDisabled(IDC_SCSPLINECONSTRAIN, true);
	} // if
else
	{
	ConfigureFI(NativeWin, IDC_FIOUTDEMWE, &Importing->OutDEMWE, 1.0, 1.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIOUTDEMNS, &Importing->OutDEMNS, 1.0, 1.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	if ((CROP_ROWS == OUTPUT_ROWS) && (CROP_COLS == OUTPUT_COLS))
		WidgetSetDisabled(IDC_SCSPLINECONSTRAIN, true);
	} // else

if (Importing->InFormat != DEM_DATA2_INPUT_WCSDEM)
	{
	ConfigureFI(NativeWin, IDC_FIDEMOUTCOLS, &Importing->OutCols, 1.0, 0.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIDEMOUTROWS, &Importing->OutRows, 1.0, 0.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	} // if
else
	{
	ConfigureFI(NativeWin, IDC_FIDEMOUTCOLS, &Importing->OutRows, 1.0, 0.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	ConfigureFI(NativeWin, IDC_FIDEMOUTROWS, &Importing->OutCols, 1.0, 0.0, (double)SHRT_MAX, FIOFlag_Long, NULL, NULL);
	} // else

Importing->GridUnits = VNS_IMPORTDATA_HUNITS_LINEAR;
SetGridSpacing();
if (Importing->KeepBounds)
	{
	sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE * Importing->InCols / Importing->OutCols);
	restxt[33] = 0;	// truncate any numbers past 7 digits
	WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
	sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS * Importing->InRows / Importing->OutRows);
	restxt[33] = 0;
	WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
	} // if
else
	{
	sprintf(restxt, " Output WE Cell Size (m): %7g", Importing->GridSpaceWE * CROP_COLS / Importing->OutCols);
	restxt[33] = 0;
	WidgetSetText(IDC_TXTOUTTILESWERES, restxt);
	sprintf(restxt, " Output NS Cell Size (m): %7g", Importing->GridSpaceNS * CROP_ROWS / Importing->OutRows);
	restxt[33] = 0;
	WidgetSetText(IDC_TXTOUTTILESNSRES, restxt);
	} // else

WidgetSetText(IDC_NEXT, "&Import");

} // ImportWizGUI::DoOutTilesPanel

/*===========================================================================*/

void ImportWizGUI::DoOutScalePanel(void)
{
const char msg[] = "OUTPUT DATA SCALING\r\r\nThese settings control the scaling of input values to output values.\r\r\n\
\r\r\n� Min/Max: Values from the input file will be scaled between these output values.\r\r\n\r\r\n� TwoVal: \
Values from the input file that are between the Value1 and Value2 Input numbers will be scaled between the Value1 \
and Value2 Output numbers. Values outside of the input range will be scaled proportionately.\r\r\n\r\r\n� OneVal: \
Use this to correlate a known input value with a desired output value (such as to assign sea level to a given \
value). Value1 Input and Output sets the related values. Then you need to choose how the rest of the data is scaled. \
Choices are Min Out, Max Out, and I/O Scale.\r\r\n\r\r\n렁 I/O Scale: This value is used to scale data around the \
central related value.\r\r\n\r\r\n렁 MinOut: The minimum input value is set to this value, while the rest of the \
file is scaled proportionally to the relation set by the minimum value and the Value1 relationship.\r\r\n\r\r\n렁 \
MaxOut: This works the same way as MinOut except the maximum values control the scaling.\r\r\n\r\r\n� You can switch \
back to the Vertical Extents page.";

if (Importing->LoadAs == LAS_IMAGE)
	{
	Importing->ScaleMMMax = 255.0;
	Importing->ScaleMMMin = 0.0;
	}

ConfigureFI(NativeWin, IDC_FIOUTSCALEMMMAX, &Importing->ScaleMMMax, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FIOUTSCALEMMMIN, &Importing->ScaleMMMin, 1.0, -DBL_MAX, DBL_MAX, FIOFlag_Double, NULL, NULL);

WidgetSetText(IDC_IMWIZTEXT, msg);

NewVertStyle = false;

} // ImportWizGUI::DoOutScalePanel

/*===========================================================================*/

// keep Lint happy
#ifdef SOMETHINGUNDEFINED
void ImportWizGUI::DoOutRegPanel(void)
{
const char msg[] = "OUTPUT DATA REGISTRATION\r\r\nThis controls the placement of your output data on the planet. These \
are the minimum and	maximum Latitude and Longitude values for your data. You may override these values if they \
are	preset if you wish to load your data at another point on the planet.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoOutRegPanel
#endif // SOMETHINGUNDEFINED

/*===========================================================================*/

void ImportWizGUI::DoOutFormPanel(void)
{
const char msg[] = "BINARY OUTPUT SETTINGS\r\r\nThese controls only apply to Binary file types, and MUST be set.\r\r\n\
� Value Format controls the type of data (signed integer, unsigned integer, or floating point).\r\r\n� Value Bytes \
controls the data element size & precision. Both types of integers can have 1, 2, or 4 bytes. Floats are either \
4 bytes (single precision) or 8 bytes (double precision).\r\r\n� Signed Byte = -128..+127\r\r\n� Unsigned Byte = \
0..+255\r\r\n� Signed 2 Bytes = -32768..+32767\r\r\n� Unsigned 2 Bytes = 0..65535\r\r\n� Signed 4 Bytes = +/- 2 \
Billion\r\r\n� Unsigned 4 Bytes = 0..4 Billion\r\r\n� 4 Byte Float = 6 Digits Precision\r\r\n� 8 Byte Float = 15 \
Digits Precision";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoOutFormPanel

/*===========================================================================*/

void ImportWizGUI::DoNullPanel(void)
{
const char msg[] = "NULL DATA\r\r\n\r\r\n� Null data is used when a DEM array has points that don't represent actual \
elevation values. This is normally done when a DEM is skewed inside of the array, or when only a portion of \
a DEM is to be used (i.e. - the DEM is being masked).\r\r\n\r\r\n� Enable this field if needed. If the field is \
enabled, the value entered in the Value field is used to test each input DEM cell to see if it contains \
valid elevation data.\r\r\n\r\r\n� Use the Comparison Method to set how the data values are compared to the NULL \
value (normally it should be set to equal).";

WidgetSetText(IDC_IMWIZTEXT, msg);

WidgetSetDisabled(IDC_FINULLVAL, (char)(Importing->HasNulls == 0));
WidgetSetDisabled(IDC_SRNULLLE, (char)(Importing->HasNulls == 0));
WidgetSetDisabled(IDC_SRNULLEQ, (char)(Importing->HasNulls == 0));
WidgetSetDisabled(IDC_SRNULLGE, (char)(Importing->HasNulls == 0));
WidgetSCSync(IDC_SCHASNULLS, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FINULLVAL, WP_FISYNC_NONOTIFY);
WidgetSRSync(IDC_SRNULLLE, WP_FISYNC_NONOTIFY);
WidgetSRSync(IDC_SRNULLEQ, WP_FISYNC_NONOTIFY);
WidgetSRSync(IDC_SRNULLGE, WP_FISYNC_NONOTIFY);

} // ImportWizGUI::DoNullPanel

/*===========================================================================*/

void ImportWizGUI::DoIdentPanel(void)
{
long Select;

if (FormatPos[Importing->MyNum] < 0)
	Select = FormatPos[IW_INPUT_BINARY];	// select binary if it's not supported
else
	Select = FormatPos[Importing->MyNum];
WidgetCBSetCurSel(IDC_CBIDENT_OVERRIDE, Select);

if (strcmp(Importing->MyIdentity, "GPX") == 0)
	WidgetSetText(IDC_NEXT, "&Import");

//WidgetCBSetCurSel(IDC_CBIDENT_OVERRIDE, Importing->MyNum);

} // ImportWizGUI::DoIdentPanel

/*===========================================================================*/

void ImportWizGUI::DoOutParamsPanel(void)
{
const char msg[] = "DEM CELL AND TILE SETTINGS\r\r\n	These values control the number of DEMs created, and the size \
of each DEM. These have been preset to recommended values (based on the recommended size of approximately 301 \
x 301).\r\r\n� E-W DEMs & N-S DEMs - These fields tell "APP_TLA" how many DEMs to grid in each direction.	Generating \
multiple DEMs will conserve memory during rendering.\r\r\n� Columns & Rows - These fields are the number of grid \
cells generated along each axis. Columns controls the E-W axis while Rows controls the N-S axis. Increasing these \
values will result in higher resolution DEMs, but will add to gridding time and rendering time.\r\r\n텴rid Cell \
Size - These display fields show the resulting grid size in meters for your settings.";

WidgetSetText(IDC_IMWIZTEXT, msg);

} // ImportWizGUI::DoOutParamsPanel

/*===========================================================================*/

void ImportWizGUI::DoTestButton(NativeGUIWin NW)
{
short error;
FILE *input;
UBYTE *buffer;

if ((input = PROJ_fopen(Importing->LoadName, "rb")) == NULL)
	{
	error = 2;
//	goto Cleanup;
	return;
	}

if (!(buffer = (UBYTE *)AppMem_Alloc(Importing->InRows * Importing->ValueBytes, 0L)))
	{
	fclose(input);
	error = 1;
//	goto Cleanup;
	return;
	}

ConfigureFI(NativeWin, IDC_FITESTMAX, &Importing->TestMax, 1.0, Importing->TestMax, Importing->TestMax, FIOFlag_Double, NULL, NULL);
ConfigureFI(NativeWin, IDC_FITESTMIN, &Importing->TestMin, 1.0, Importing->TestMin, Importing->TestMin, FIOFlag_Double, NULL, NULL);
WidgetFISync(IDC_FITESTMAX, WP_FISYNC_NONOTIFY);
WidgetFISync(IDC_FITESTMIN, WP_FISYNC_NONOTIFY);

AppMem_Free(buffer, Importing->InRows * Importing->ValueBytes);
fclose(input);

} // ImportWizGUI::DoTestButton

/*===========================================================================*/

bool ImportWizGUI::ControlFlow(unsigned long PanelIndex, bool forward)
{
const char ctrlmsg[] = "Internal Error: Control Flow";
bool err = false;

switch (ActivePanel)
	{
	case IDD_IMWIZ_BINSET:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			/*** F2 NOTE: Need to go to WCS_KNOWS when georeferencing flag stuff is fully implemented ***/
			--PanelNum;
			SelectPanel(IDD_IMWIZ_COORDSYS);
			} // else
		break;
	case IDD_IMWIZ_BUILDOPT:
	case IDD_IMWIZ_CELLORDER:
	case IDD_IMWIZ_COLEL:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_COORDSYS:	// we should only get this in a VNS build
		if (forward)
			{
			PanelNum++;
			if (PanelOrder[0] == 0)
				DoLoad(NULL);
			else
				SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum;
			SelectPanel(IDD_IMWIZ_OUTTYPE);
			} // else
		break;
	case IDD_IMWIZ_ELEVUNITS:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			if ((stricmp(Importing->MyIdentity, "DXF") == 0) ||
				(stricmp(Importing->MyIdentity, "XYZ_FILE") == 0) || (stricmp(Importing->MyIdentity, "WCS_XYZ") == 0))
				SelectPanel(IDD_IMWIZ_COORDSYS);
			else
				SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_GPS_STEP1:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			//if (PanelNum > 0)
			//	{
			//	--PanelNum; --PanelIndex;
			//	SelectPanel(PanelOrder[PanelIndex]);
			//	} // if
			//else
			//	{
				--PanelNum;
				SelectPanel(IDD_IMWIZ_COORDSYS);
			//	} // else
			} // else
		break;
	case IDD_IMWIZ_GPS_STEP2F:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_GPS_STEP2D:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_GPS_STEP3:
		if (forward)
			{
			DoLoad(NULL);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_GRIDIT:
		if (forward)
			{
			DoLoad(NULL);
			} // if
		else
			{
			if ((stricmp(Importing->MyIdentity, "XYZ_FILE") == 0) || (stricmp(Importing->MyIdentity, "WCS_XYZ") == 0))
				{
				--PanelNum;
				SelectPanel(IDD_IMWIZ_COORDSYS);
				} // if
			else
				{
				--PanelNum; --PanelIndex;
				SelectPanel(PanelOrder[PanelIndex]);
				} // else
			} // else
		break;
	case IDD_IMWIZ_HOREX:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			if (Importing->LoadAs == LAS_IMAGE)
				{
				PanelNum = 2;
				SelectPanel(IDD_IMWIZ_OUTTYPE);
				} // if
			else
				{
				--PanelNum; --PanelIndex;
				SelectPanel(PanelOrder[PanelIndex]);
				WidgetSetDisabled(IDC_NEXT, 0);
				} // else
			} // else
		break;
	case IDD_IMWIZ_HORUNITS:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum;
			SelectPanel(IDD_IMWIZ_COORDSYS);
			} // else
		break;
	case IDD_IMWIZ_ICRHEAD:
	case IDD_IMWIZ_IDENT:
	case IDD_IMWIZ_LOADAS:
	case IDD_IMWIZ_NULL:
	case IDD_IMWIZ_OUTFORM:
	case IDD_IMWIZ_OUTPARAMS:
	//case IDD_IMWIZ_OUTREG:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_OUTSCALE:
		ShowPanel(1, -1);	// close previous
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_OUTTILES:
		if (forward)
			{
			if (NewVertStyle)
				SCALEOP = DEM_DATA_SCALEOP_UNIFIED;
			DoLoad(NULL);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			if (Importing->LoadAs == LAS_IMAGE)
				SelectPanel(IDD_IMWIZ_OUTSCALE);
			else
				SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_OUTTYPE:		// we should only get this in non-VNS builds, or if VNS is doing a Z-buffer to image conversion.
		if (forward)
			{
			FinishOuttype();
			PanelNum++;
			if (PanelOrder[0] == 0)
				DoLoad(NULL);
			else
				SelectPanel(PanelOrder[PanelIndex]);
//			if (Importing->LoadAs == LAS_IMAGE)
//				SelectPanel(PanelOrder[PanelIndex]);
//			else
//				SelectPanel(PanelOrder[PanelNum]);
			} // if
		else
			{
			--PanelNum;
			SelectPanel(IDD_IMWIZ_LOADAS);
			} // else
		break;
	case IDD_IMWIZ_PIXCTR:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_PREPRO:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			if (Importing->LoadAs == LAS_IMAGE)
				SelectPanel(IDD_IMWIZ_OUTSCALE);
			else
				SelectPanel(PanelOrder[PanelIndex]);
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			SelectPanel(PanelOrder[PanelIndex]);
			} // else
		break;
	case IDD_IMWIZ_REFCO:
		if (forward)
			{
			if ((stricmp(Importing->MyIdentity, "DXF") == 0) && (Importing->LoadAs == LAS_VECT))
				DoLoad(NULL);
			else
				{
				++PanelNum; PanelIndex++;
				SelectPanel(PanelOrder[PanelIndex]);
				} // else
			} // if
		else
			{
			if (stricmp(Importing->MyIdentity, "DXF") == 0)
				{
				--PanelNum;
				SelectPanel(IDD_IMWIZ_COORDSYS);
				} // if
			else
				{
				--PanelNum; --PanelIndex;
				SelectPanel(PanelOrder[PanelIndex]);
				} // else
			} // else
		break;
	case IDD_IMWIZ_SAVEGEO:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_SHAPEOPTS:
		if (forward)
			{
			DoLoad(NULL);
			} // if
		else
			{
			--PanelNum;
			SelectPanel(IDD_IMWIZ_COORDSYS);
			} // else
		break;
	case IDD_IMWIZ_TEST:
	case IDD_IMWIZ_VERTEX:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_WCSKNOWS:
		if (forward)
			{
			// Geographic WCS DEM?
			#ifdef WCS_BUILD_VNS
			if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) && (Importing->IWCoordSys.Method.GCTPMethod == 0))
			#else // WCS_BUILD_VNS
			if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
			#endif // WCS_BUILD_VNS
				Importing->PosEast = false;

			if (knowhow == 0)
				{
				SCALEOP = DEM_DATA_SCALEOP_UNIFIED;
				if ((OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM) &&
					(Importing->InCols * Importing->InRows) > (301 * 301)) // will output DEM size be greater than recommended?
					{
					if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM)
						{
						Importing->OutDEMWE = (Importing->InRows - 1) / 301 + 1;
						Importing->OutDEMNS = (Importing->InCols - 1) / 301 + 1;
						} // if
					else
						{
						Importing->OutDEMWE = (Importing->InCols - 1) / 301 + 1;
						Importing->OutDEMNS = (Importing->InRows - 1) / 301 + 1;
						} // else
					} // if
				DoLoad(NULL);
				} // if
			else
				{
				PanelNum++; PanelIndex++;
				SelectPanel(PanelOrder[PanelIndex]);
				} // else
			} // if forward
		else
			{
			if (PanelIndex > 0)
				{
				--PanelNum; --PanelIndex;
				if ((Importing->InFormat == VECT_DATA2_INPUT_SHAPEFILE) && (PanelOrder[PanelIndex] == IDD_IMWIZ_ELEVUNITS) && (! Importing->ShapeElevs))
					{
					--PanelNum; --PanelIndex;
					} // if
				SelectPanel(PanelOrder[PanelIndex]);
				} // if
			else
				{
				--PanelNum;
				SelectPanel(IDD_IMWIZ_COORDSYS);
				} // else
			} // else reverse
		break;
	case IDD_IMWIZ_WCSUNITS:
		if (forward)
			{
			PanelNum++; PanelIndex++;
			} // if
		else
			{
			--PanelNum; --PanelIndex;
			} // else
		SelectPanel(PanelOrder[PanelIndex]);
		break;
	case IDD_IMWIZ_WRAP:
		break;
	default:
		err = true;
		break;
	} // switch

if (err)
	{
	WidgetSetText(IDC_IMWIZTEXT, ctrlmsg);
	return err;
	} // if
	
return 0;

} // ImportWizGUI::ControlFlow

/*===========================================================================*/

// see if it's valid, get all GeoRef settings possible
// (basically, any values needed that are not returned to the Importing object have to be supplied by the user)
// returns 0 if file validates OK, -1 if we didn't handle the string passed, any other result is error
short ImportWizGUI::GetInputFile(void)
{

// set default database name to file name - the load routine will replace this with a better name where applicable
strcpy(Importing->NameBase, Importing->InFile);
(void)StripExtension(Importing->NameBase);

if ((strcmp(intype, "EXPONENTFILE") == 0) || (strcmp(intype, "TEXTFILE") == 0) || (strcmp(intype, "NUMERICFILE") == 0))
	{
	// it's probably not a binary file, but the user didn't override
	strcpy(Importing->MyIdentity, "BINARY");
	} // if

if (strcmp(intype, "ARC_ASCII_ARRAY") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_ASCII;
	return LoadASCII_DEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "ASCII_ARRAY") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_ASCII;
	return LoadASCII_DEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "ASCII_GPS") == 0)
	{
	Importing->InFormat = IW_INPUT_ASCII_GPS;
	return LoadASCII_GPS(Importing->LoadName, true);
	} // if

if (strcmp(intype, "ARC_BINARYADF_GRID") == 0)
	{
	Importing->InFormat = IW_INPUT_ARCBINARYADF_GRID;
	return LoadARCBinaryADFDEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "ARC_EXPORT_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_ARC_EXPORT_GRID;
	return LoadARCExportDEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "WCS_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_WCSDEM;
	return GetWCSInputFile();
	} // if

if (strcmp(intype, "GPX") == 0)
	{
	Importing->InFormat = IW_INPUT_GPX;
	return LoadGPX(Importing->LoadName, true);
	} // if

if (strcmp(intype, "SDTS_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_SDTS_DEM;
	return LoadSDTS_DEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "SDTS_DLG") == 0)
	{
	Importing->InFormat = VECT_DATA2_INPUT_SDTS_DLG;
	return LoadSDTS_DLG(true);
	} // if

if (strcmp(intype, "TERRAGEN") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_TERRAGEN;
	return LoadTerragen(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "DTED") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_DTED;
	return LoadDTED(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "DXF") == 0)
	{
	Importing->InFormat = VECT_DATA2_INPUT_DXF;
	return VectLoad(true);
	} // if

if (strcmp(intype, "BINARY") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_BINARY;
	return LoadBinary(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "GTOPO30") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_BINARY;
	return LoadBinary(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "MDEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_MDEM;
	return LoadMDEM(Importing->LoadName, NULL, true);
	} // if

#ifdef WCS_BUILD_VNS
if (strcmp(intype, "NTF_DTM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_NTF_DTM;
	return LoadNTF_DTM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "NTF_MERIDIAN2") == 0)
	{
	Importing->InFormat = VECT_DATA2_INPUT_NTF_MERIDIAN2;
	return VectLoad(true);
	} // if
#endif // WCS_BUILD_VNS

if (strcmp(intype, "BRYCE_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_BRYCE;
	return LoadBryceDEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "VISTA_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_VISTA;
	return LoadVistaDEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "USGS_DEM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_USGS_DEM;
	return LoadUSGS_DEM(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "USGS_DLG") == 0)
	{
	Importing->InFormat = VECT_DATA2_INPUT_USGS_DLG;
	return VectLoad(true);
	} // if

if (strcmp(intype, "SHAPEFILE") == 0)
	{
	Importing->InFormat = VECT_DATA2_INPUT_SHAPEFILE;
	return VectLoad(true);
	} // if

if (strcmp(intype, "SRTM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_BINARY;
	return LoadBinary(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "STM") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_BINARY;
	return LoadBinary(Importing->LoadName, NULL, true);
	} // if

#ifdef GO_SURFING
if (strcmp(intype, "SURFER") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_SURFER;
	return LoadSurfer(Importing->LoadName, NULL, true);
	}
#endif // GO_SURFING

//if ((strcmp(intype, "IMAGE_BMP") == 0) || (strcmp(intype, "IMAGE_IFF") == 0) ||
//	(strcmp(intype, "IMAGE_PICT") == 0) || (strcmp(intype, "IMAGE_TARGA") == 0) ||
//	(strcmp(intype, "IMAGE_ECW") == 0) || (strcmp(intype, "IMAGE_JPEG") || (strcmp(intype, "IMAGE_PNG") || (strcmp(intype, "IMAGE_TIFF"))
if (strncmp(intype, "IMAGE_", 6) == 0)
	{
	if (strcmp(intype, "IMAGE_TIFF") == 0)
		{
		if (IsTIFFDEM(Importing->LoadName))
			{
			return LoadTIFFDEM(Importing->LoadName, NULL, true);
			} // if
		} // if
	Importing->InFormat = DEM_DATA2_INPUT_IFF;
// F2NOTE: 080501 - Not sure when/why next line was changed to use value passed to function.  Causes nasty crash though :)
// So we're overriding in function back to this: DEM_DATA_FORMAT_UNSIGNEDINT
	return LoadDEMImage(Importing->LoadName, NULL, true, DEM_DATA_FORMAT_SIGNEDINT);
	} // if

if (strcmp(intype, "WCS_XYZ") == 0)
	{
	return LoadWXYZ_CP(Importing->LoadName, true);
	} // if

if (strcmp(intype, "WCS_ZBUFFER") == 0)
	{
	Importing->InFormat = DEM_DATA2_INPUT_ZBUF;
	return LoadWCS_ZBuffer(Importing->LoadName, NULL, true);
	} // if

if (strcmp(intype, "XYZ_FILE") == 0)
	{
	return LoadXYZ_CP(Importing->LoadName, true);
	} // if

return -1;

} // ImportWizGUI::GetInputFile

/*===========================================================================*/

void ImportWizGUI::DoHorExChange(int CtrlID)
{
double avglat, degrees, meters;
//#ifdef WCS_BUILD_VNS
double tmp;
//#endif // WCS_BUILD_VNS
long inCols, inRows;

if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) || (Importing->InFormat == DEM_DATA2_INPUT_MDEM))
	{
	inCols = Importing->InRows;
	inRows = Importing->InCols;
	} // if
else
	{
	inCols = Importing->InCols;
	inRows = Importing->InRows;
	} // else

switch (CtrlID)
	{
	case IDC_FIHOREXN:
	case IDC_FIHOREXS:
	case IDC_FIHOREXW:
	case IDC_FIHOREXE:
		#ifdef WCS_BUILD_VNS
		AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = Importing->NBound - Importing->SBound;
		AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = fabs(Importing->WBound - Importing->EBound);
		if (GoodBounds())
			WidgetSetDisabled(IDC_NEXT, 0);
		else
			WidgetSetDisabled(IDC_NEXT, 1);
		SetGridSpacing();
		#else // WCS_BUILD_VNS
		AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = Importing->NBound - Importing->SBound;
		AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = fabs(Importing->WBound - Importing->EBound);
		if ((Importing->NBound <= Importing->SBound) || (Importing->WBound >= Importing->EBound))
			WidgetSetDisabled(IDC_NEXT, 1);
		else
			WidgetSetDisabled(IDC_NEXT, 0);
		SetGridSpacing();
		#endif // WCS_BUILD_VNS
		break;
	case IDC_FIHOREXGRIDSIZENS:
		#ifdef WCS_BUILD_VNS
		if (Importing->InRows <= 1)
			break;
		Importing->GridSpaceNS = IWGridSizeNS / inRows;
		WidgetFISync(IDC_FIHOREXGRIDNS, WP_FISYNC_NONOTIFY);
		// fall through to IDC_FIHOREXGRIDNS
		#else // WCS_BUILD_VNS
		if (Importing->InRows == 0)
			break;
		Importing->GridSpaceNS = IWGridSizeNS / Importing->InRows;
		WidgetFISync(IDC_FIHOREXGRIDNS, WP_FISYNC_NONOTIFY);
		// fall through to IDC_FIHOREXGRIDNS
		#endif // WCS_BUILD_VNS
		//lint -fallthrough
	case IDC_FIHOREXGRIDNS:
		#ifdef WCS_BUILD_VNS
		Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
		Importing->GridUnits += VNS_IMPORTDATA_HUNITS_DEGREES;
		avglat = (Importing->NBound + Importing->SBound) / 2.0;
		if ((Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES))
			{
			AnimPar[IW_ANIMPAR_GRIDSIZENS].CurValue = IWGridSizeNS = Importing->GridSpaceNS * inRows;
			AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = Importing->GridSpaceNS * (inRows - 1);
			DoHorExChange(IDC_FIHOREXHEIGHT);
			} // if
		else if ((Importing->BoundsType == VNS_BOUNDSTYPE_DEGREES) && (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR))
			{
			IWGlobeRad = Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
			meters = Importing->GridSpaceNS * (inRows - 1);
			degrees = ConvertMetersToDeg(meters, avglat, IWGlobeRad);
			AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = degrees;
			DoHorExChange(IDC_FIHOREXHEIGHT);
			} // else if
		else if (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR)
			{
			degrees = Importing->GridSpaceNS * (inRows - 1);
			AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = degrees;
			DoHorExChange(IDC_FIHOREXHEIGHT);
			} // else if
		else
			{
			meters = Importing->GridSpaceNS * inRows; // meters is really in units of measure here
			switch (Importing->GridUnits)
				{
				case 1: // kilometers
					meters *= 1000;
					break;
				default:
				case 2: // meters
					// don't need to change
					break;
				case 3: // decimeters
					meters *= 0.1;
					break;
				case 4: // centimeters
					meters *= 0.01;
					break;
				case 5: // miles
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_MILE_US_STATUTE);
					break;
				case 6: // International foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET);
					break;
				case 7: // U.S. Survey foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET_US_SURVEY);
					break;
				case 8: // inches
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_INCH);
					break;
				} // switch
			degrees = ConvertMetersToDeg(meters, 0.0, IWGlobeRad);
			AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = degrees;
			DoHorExChange(IDC_FIHOREXHEIGHT);
			} // else
		/***
		Importing->NBound = Importing->SBound + degrees;
		IWGridSizeNS = Importing->InRows * Importing->GridSpaceNS;
		WidgetSNSync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXGRIDSIZENS, WP_FISYNC_NONOTIFY);
		if (GoodBounds())
			WidgetSetDisabled(IDC_NEXT, 0);
		else
			WidgetSetDisabled(IDC_NEXT, 1);
		***/
		#else // WCS_BUILD_VNS
		Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
		if (Importing->GridUnits == 0) // degrees
			degrees = Importing->GridSpaceNS * Importing->InRows;
		else
			{
			meters = Importing->GridSpaceNS * (Importing->InRows - 1); // meters is really in units of measure here
			switch (Importing->GridUnits)
				{
				case 1: // kilometers
					meters *= 1000;
					break;
				default:
				case 2: // meters
					// don't need to change
					break;
				case 3: // decimeters
					meters *= 0.1;
					break;
				case 4: // centimeters
					meters *= 0.01;
					break;
				case 5: // miles
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_MILE_US_STATUTE);
					break;
				case 6: // International foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET);
					break;
				case 7: // U.S. Survey foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET_US_SURVEY);
					break;
				case 8: // inches
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_INCH);
					break;
				}
			degrees = ConvertMetersToDeg(meters, 0.0, IWGlobeRad);
			}
		AnimPar[IW_ANIMPAR_HEIGHT].CurValue = Importing->InHeight = degrees;
		DoHorExChange(IDC_FIHOREXHEIGHT);
		/***
		Importing->InHeight = degrees;
		Importing->NBound = Importing->SBound + degrees;
		IWGridSizeNS = Importing->InRows * Importing->GridSpaceNS;
		WidgetFISync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXHEIGHT, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXGRIDSIZENS, WP_FISYNC_NONOTIFY);
		if ((Importing->NBound <= Importing->SBound) || (Importing->WBound <= Importing->EBound))
			WidgetSetDisabled(IDC_NEXT, 1);
		else
			WidgetSetDisabled(IDC_NEXT, 0);
		***/
		#endif // WCS_BUILD_VNS
		break;
	case IDC_FIHOREXGRIDSIZEWE:
		#ifdef WCS_BUILD_VNS
		if (Importing->InCols <= 1)
			break;
		Importing->GridSpaceWE = IWGridSizeWE / inCols;
		WidgetFISync(IDC_FIHOREXGRIDWE, WP_FISYNC_NONOTIFY);
		// fall through to IDC_FIHOREXGRIDWE
		#else // WCS_BUILD_VNS
		if (Importing->InCols == 0)
			break;
		Importing->GridSpaceWE = IWGridSizeWE / Importing->InCols;
		WidgetFISync(IDC_FIHOREXGRIDWE, WP_FISYNC_NONOTIFY);
		// fall through to IDC_FIHOREXGRIDWE
		#endif // WCS_BUILD_VNS
		//lint -fallthrough
	case IDC_FIHOREXGRIDWE:
#ifdef WCS_BUILD_VNS
		Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
		Importing->GridUnits += VNS_IMPORTDATA_HUNITS_DEGREES;
		avglat = (Importing->NBound + Importing->SBound) / 2.0;
		if ((Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES))
			{
			AnimPar[IW_ANIMPAR_GRIDSIZEWE].CurValue = IWGridSizeWE = Importing->GridSpaceWE * inCols;
			AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = Importing->GridSpaceWE * (inCols - 1);
			DoHorExChange(IDC_FIHOREXWIDTH);
			} // if
		else if ((Importing->BoundsType == VNS_BOUNDSTYPE_DEGREES) && (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR))
			{
			IWGlobeRad = Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].CurValue;
			meters = Importing->GridSpaceWE * (inCols - 1);
			degrees = ConvertMetersToDeg(meters, avglat, IWGlobeRad);
			AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = degrees;
			DoHorExChange(IDC_FIHOREXWIDTH);
			} // else if
		else if (Importing->BoundsType == VNS_BOUNDSTYPE_LINEAR)
			{
			degrees = Importing->GridSpaceWE * (inCols - 1);
			AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = degrees;
			DoHorExChange(IDC_FIHOREXWIDTH);
			} // else if
		else
			{
			meters = Importing->GridSpaceWE * inCols; // meters is really in units of measure here
			switch (Importing->GridUnits)
				{
				case 1: // kilometers
					meters *= 1000;
					break;
				default:
				case 2: // meters
					// don't need to change
					break;
				case 3: // decimeters
					meters *= 0.1;
					break;
				case 4: // centimeters
					meters *= 0.01;
					break;
				case 5: // miles
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_MILE_US_STATUTE);
					break;
				case 6: // International foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET);
					break;
				case 7: // U.S. Survey foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET_US_SURVEY);
					break;
				case 8: // inches
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_INCH);
					break;
				} // switch
			degrees = ConvertMetersToDeg(meters, avglat, IWGlobeRad);
			AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = degrees;
			DoHorExChange(IDC_FIHOREXWIDTH);
			} // else
		/***
		Importing->WBound = Importing->EBound + degrees;
		IWGridSizeWE = Importing->InCols * Importing->GridSpaceWE;
		WidgetSNSync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXWIDTH, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXGRIDSIZEWE, WP_FISYNC_NONOTIFY);
		if (GoodBounds())
			WidgetSetDisabled(IDC_NEXT, 0);
		else
			WidgetSetDisabled(IDC_NEXT, 1);
		***/
		#else // WCS_BUILD_VNS
		Importing->GridUnits = (short)WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
		avglat = (Importing->NBound + Importing->SBound) / 2.0;
		if (Importing->GridUnits == 0) // degrees
			degrees = Importing->GridSpaceWE * Importing->InCols;
		else
			{
			meters = Importing->GridSpaceWE * (Importing->InCols - 1); // meters is really in units of measure here
			switch (Importing->GridUnits)
				{
				case 1: // kilometers
					meters *= 1000;
					break;
				default:
				case 2: // meters
					// don't need to change
					break;
				case 3: // decimeters
					meters *= 0.1;
					break;
				case 4: // centimeters
					meters *= 0.01;
					break;
				case 5: // miles
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_MILE_US_STATUTE);
					break;
				case 6: // International foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET);
					break;
				case 7: // U.S. Survey foot
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_FEET_US_SURVEY);
					break;
				case 8: // inches
					meters = ConvertToMeters(meters, WCS_USEFUL_UNIT_INCH);
					break;
				}
			degrees = ConvertMetersToDeg(meters, avglat, IWGlobeRad);
			}
		AnimPar[IW_ANIMPAR_WIDTH].CurValue = Importing->InWidth = degrees;
		DoHorExChange(IDC_FIHOREXWIDTH);
		/***
		Importing->InWidth = degrees;
		Importing->WBound = Importing->EBound + degrees;
		IWGridSizeWE = Importing->InCols * Importing->GridSpaceWE;
		WidgetFISync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXWIDTH, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXGRIDSIZEWE, WP_FISYNC_NONOTIFY);
		if ((Importing->NBound <= Importing->SBound) || (Importing->WBound <= Importing->EBound))
			WidgetSetDisabled(IDC_NEXT, 1);
		else
			WidgetSetDisabled(IDC_NEXT, 0);
		***/
		#endif // WCS_BUILD_VNS
		break;
//#ifdef WCS_BUILD_VNS
	case IDC_FIHOREXWIDTH:
		if (WidgetGetCheck(IDC_TBHOREX_LOCKW))
			{
			if (Importing->PosEast)
				Importing->EBound = Importing->WBound + Importing->InWidth;
			else
				Importing->EBound = Importing->WBound - Importing->InWidth;
			} // if
		else if (WidgetGetCheck(IDC_TBHOREX_LOCKE))
			{
			if (Importing->PosEast)
				Importing->WBound = Importing->EBound - Importing->InWidth;
			else
				Importing->WBound = Importing->EBound + Importing->InWidth;
			} // else if
		else
			{
			tmp = (Importing->WBound + Importing->EBound) / 2;
			if (Importing->PosEast)
				{
				Importing->EBound = tmp + (Importing->InWidth / 2);
				Importing->WBound = tmp - (Importing->InWidth / 2);
				} // if
			else
				{
				Importing->EBound = tmp - (Importing->InWidth / 2);
				Importing->WBound = tmp + (Importing->InWidth / 2);
				} // else
			} // else
		AnimPar[IW_ANIMPAR_E].CurValue = Importing->EBound;
		AnimPar[IW_ANIMPAR_W].CurValue = Importing->WBound;
		SetGridSpacing();
		WidgetSNSync(IDC_FIHOREXE, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
		if (GoodBounds())
			WidgetSetDisabled(IDC_NEXT, 0);
		else
			WidgetSetDisabled(IDC_NEXT, 1);
		break;
	case IDC_FIHOREXHEIGHT:
		if (WidgetGetCheck(IDC_TBHOREX_LOCKN))
			Importing->SBound = Importing->NBound - Importing->InHeight;
		else if (WidgetGetCheck(IDC_TBHOREX_LOCKS))
			Importing->NBound = Importing->SBound + Importing->InHeight;
		else
			{
			tmp = (Importing->NBound + Importing->SBound) / 2;
			Importing->NBound = tmp + (Importing->InHeight / 2);
			Importing->SBound = tmp - (Importing->InHeight / 2);
			} // else
		if ((Importing->BoundsType == VNS_BOUNDSTYPE_DEGREES) && (Importing->NBound > 90.0))
			{
			Importing->NBound = 90.0;
			Importing->SBound = 90.0 - Importing->InHeight;
			} // if
		AnimPar[IW_ANIMPAR_N].CurValue = Importing->NBound;
		AnimPar[IW_ANIMPAR_S].CurValue = Importing->SBound;
		SetGridSpacing();
		WidgetSNSync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
		WidgetSNSync(IDC_FIHOREXS, WP_FISYNC_NONOTIFY);
		if (GoodBounds())
			WidgetSetDisabled(IDC_NEXT, 0);
		else
			WidgetSetDisabled(IDC_NEXT, 1);
		break;
/***
#else // WCS_BUILD_VNS
	case IDC_FIHOREXWIDTH:
	case IDC_FIHOREXHEIGHT:
		Importing->NBound = Importing->SBound + Importing->InHeight;
		if (Importing->NBound > 90.0)
			{
			Importing->NBound = 90.0;
			Importing->SBound = 90.0 - Importing->InHeight;
			}
		Importing->WBound = Importing->EBound + Importing->InWidth;
		SetGridSpacing();
		WidgetFISync(IDC_FIHOREXN, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXS, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXE, WP_FISYNC_NONOTIFY);
		WidgetFISync(IDC_FIHOREXW, WP_FISYNC_NONOTIFY);
		if ((Importing->NBound <= Importing->SBound) || (Importing->WBound <= Importing->EBound))
			WidgetSetDisabled(IDC_NEXT, 1);
		else
			WidgetSetDisabled(IDC_NEXT, 0);
		break;
#endif // WCS_BUILD_VNS
***/
		default:
			break;
	} // switch

} // ImportWizGUI::DoHorExChange

/*===========================================================================*/

short ImportWizGUI::DoLoad(NativeGUIWin NW)
{
double LonSpacing;
int DEMsLoaded = 0, CtrlPtsLoaded = 0, VectsLoaded = 0;
short error;
NotifyTag Changes[2];
//ULONG loaded;
#ifdef WCS_BUILD_DEMO
bool restricted = false;
#endif // WCS_BUILD_DEMO
#ifdef WCS_BUILD_VNS
CoordSys *MatchSys = &Importing->IWCoordSys;
FILE *ExportLog = NULL;
#endif // WCS_BUILD_VNS
Pier1 *TempImport, **TemplatedPtr, *Disposable = NULL, **DisposablePtr;
char msg[80], TemplateLoadStash;

#ifdef WCS_BUILD_DEMO
if ((Importing->InFormat == DEM_DATA2_INPUT_IFF) && (Importing->LoadAs == LAS_DEM))
	{
	Importing->OutDEMNS = Importing->OutDEMWE = 1;
	if ((Importing->InRows > 300) && (Importing->OutRows > 300))
		{
		restricted = true;
		Importing->OutRows = 300;
		} // if
	if ((Importing->InCols > 300) && (Importing->OutCols > 300))
		{
		restricted = true;
		Importing->OutCols = 300;
		} // if
	} // if
#endif // WCS_BUILD_DEMO

#ifdef EXPORTPIERS
ExportLog = fopen("C:/IMWizSettings.Txt", "w");
#endif // EXPORTPIERS

LoadingMsg();
WidgetSetDisabled(IDC_PREV, 1);
WidgetSetDisabled(IDC_NEXT, 1);
WidgetSetDisabled(IDCANCEL, 1);
// put S@G to sleep
Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_FREEZE, 0xff, 0xff);
Changes[1] = NULL;
GlobalApp->AppEx->GenerateNotify(Changes, NULL);
// some vector files can create a ton of notification events...
GlobalApp->AppDB->SuppressNotifiesDuringLoad = 1;

if (Importing->Reference == 1) // X,Y
	{
	Importing->SBound = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
	Importing->NBound = Importing->SBound + Importing->InHeight;
	Importing->EBound = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
	Importing->WBound = Importing->EBound + Importing->InWidth;
	} // if
else if (Importing->Reference == 2) // 0,0
	{
	Importing->SBound = 0;
	Importing->NBound = Importing->SBound + Importing->InHeight;
	Importing->EBound = 0;
	Importing->WBound = Importing->EBound + Importing->InWidth;
	} // else if

#ifdef WCS_BUILD_DEMO
if (restricted)
	UserMessageDemo("The output resolution has been reduced.");
#endif // WCS_BUILD_DEMO

#ifdef WCS_BUILD_VNS
if (Importing->FileInfo[1023] != '~')
	NewCoordSys = GlobalApp->AppEffects->CompareMakeCoordSys(MatchSys, 1);
Importing->FileInfo[1023] = 0;
#endif // WCS_BUILD_VNS

TemplateLoadStash = GlobalApp->TemplateLoadInProgress;
GlobalApp->TemplateLoadInProgress = (Importing->Automatic && ! Importing->Embed);

if (strstr(whatitis, "GPX"))
	{
	Importing->LoadAs = LAS_VECT;
	Importing->InFormat = VECT_DATA2_INPUT_GPX;
	} // if

// is this a global geographic DEM?
#ifdef WCS_BUILD_VNS
if ((Importing->LoadAs == LAS_DEM) && (Importing->InCols > 1) && (Importing->IWCoordSys.Method.GCTPMethod == 0))
#else // WCS_BUILD_VNS
if ((Importing->LoadAs == LAS_DEM) && (Importing->InCols > 1))
#endif // WCS_BUILD_VNS
	{
	double epsilon;
	if (Importing->PosEast)
		{
		LonSpacing = (Importing->EBound - Importing->WBound) / (Importing->InCols - 1.0);
		epsilon = LonSpacing * (1.0 / 10000.0);
		if ((Importing->EBound - Importing->WBound + LonSpacing + epsilon) >= 360.0)
			{
			Importing->WrapData = true;
			Importing->OutCols++;
			} // if
		} // if
	else
		{
		LonSpacing = (Importing->WBound - Importing->EBound) / (Importing->InCols - 1.0);
		epsilon = LonSpacing * (1.0 / 10000.0);
		if ((Importing->WBound - Importing->EBound + LonSpacing + epsilon) >= 360.0)
			{
			Importing->WrapData = true;
			Importing->OutCols++;
			} // if
		} // else
	} // if global DEM

switch (Importing->LoadAs)
	{
	case LAS_CP:
		if (strstr(whatitis, "ASCII_GPS"))
			{
			error = LoadASCII_GPS(Importing->LoadName, false);
			break;
			} // if
		else if (strstr(whatitis, "DXF"))
			{
			if (!(error = VectLoad(false)) && IMWizGridCP)
				AutoGrid();
			break;
			} // else if
		else if (strstr(whatitis, "SHAPEFILE"))
			{
			error = VectLoad(false);
			} // else if
		else if (strstr(whatitis, "WCS_XYZ"))
			{
			if (!(error = LoadWXYZ_CP(Importing->LoadName, false)) && IMWizGridCP)
				AutoGrid();
			break;
			} // else if
		else if (strstr(whatitis, "XYZ_FILE"))
			{
			if (!(error = LoadXYZ_CP(Importing->LoadName, false)) && IMWizGridCP)
				AutoGrid();
			break;
			} // else if
		else
			error = DEMLoad(false, NW);
		break;
	case LAS_DEM:
		error = DEMLoad(false, NW);
		break;
	case LAS_VECT:
		error = VectLoad(false);
		break;
	case LAS_IMAGE:
		error = DEMLoad(false, NW);
		break;
	default:
		break;
	} // switch LoadAs

if (! Importing->TestOnly)
	{
#ifdef WCS_BUILD_VNS
	if (ExportLog)
		ExportSettings(ExportLog, Importing);
#endif // WCS_BUILD_VNS
	if (HeadPier->LoadAs == LAS_DEM)
		{
		sprintf(msg, "Max elev = %.2f, Min elev = %.2f", Importing->TestMax * Importing->VScale, Importing->TestMin * Importing->VScale);
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_MSG, msg);
		} // if
	if (MergeLoad || (Importing->Next == NULL))
		{
		// find out which items were loaded
		Importing = HeadPier;
		while (Importing)
			{
			if (Importing->LoadAs == LAS_DEM)
				{
				DEMsLoaded = 1;
				if (Importing->Corners)
					VectsLoaded = 1;	// USGS Quadrangle bounds
				} // if
			else if (Importing->LoadAs == LAS_VECT)
				VectsLoaded = 1;
			else if (Importing->LoadAs == LAS_CP)
				CtrlPtsLoaded = 1;
			Importing = Importing->Next;
			} // while

		// sort into two lists, auto-imports and disposable
		Templated = NULL;
		TemplatedPtr = &Templated;
		DisposablePtr = &Disposable;
		Importing = HeadPier;
		while (Importing)
			{
			TempImport = Importing;
			Importing = Importing->Next;
			TempImport->Next = NULL;
			if (TempImport->Automatic)
				{
				*TemplatedPtr = TempImport;
				TemplatedPtr = &(*TemplatedPtr)->Next;
				} // if
			else
				{
				*DisposablePtr = TempImport;
				DisposablePtr = &(*DisposablePtr)->Next;
				} // else
			} // while

		while (Disposable)	// take out the trash
			{
			Importing = Disposable;
			Disposable = Disposable->Next;
			delete Importing;
			} // while
		HeadPier = Importing = NULL;		// for form's sake

		// send notifications if this is not an auto-import operation
		if (NativeWin)
			{
			GlobalApp->AppDB->SuppressNotifiesDuringLoad = 0;
			// wake up S@G
			Changes[0] = MAKE_ID(WCS_NOTIFYCLASS_FREEZE, WCS_NOTIFYSUBCLASS_THAW, 0xff, 0xff);
			Changes[1] = NULL;
			GlobalApp->AppEx->GenerateNotify(Changes, NULL);
			if (DEMsLoaded)
				{
				Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_DEM, 0xff, 0xff,	WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				(void)GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			if (VectsLoaded)
				{
				Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_VECTOR, 0xff, 0xff,	WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				(void)GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			if (CtrlPtsLoaded)
				{
				Changes[0] = MAKE_ID(WCS_RAHOST_OBJTYPE_CONTROLPT, 0xff, 0xff,	WCS_NOTIFYCOMP_OBJECT_COUNTCHANGED);
				(void)GlobalApp->AppEx->GenerateNotify(Changes, NULL);
				} // if
			(void)HandleCloseWin(NW);
			} // if
#ifdef WCS_BUILD_VNS
		if (ExportLog)
			fclose(ExportLog);
#endif // WCS_BUILD_VNS
		} // if
	else
		{
		NumLoaded++;
		Importing = Importing->Next;
		while (Importing && ! Importing->Enabled && ! Importing->Embed)
			Importing = Importing->Next;
		// NativeWin might be NULL if this is an Auto-Import, Importing might be NULL from above loop
		if (NativeWin && Importing)
			{
			ShowPanel(0, -1);
			ConfigureWidgets();
			SelectPanel(IDD_IMWIZ_IDENT);
			PanelNum = 0;
			strcpy(whatitis, Importing->MyIdentity);
			WidgetSetText(IDC_NEXT, "&Next -->");
			ShowID(Importing->MyIdentity);
			} // if
		else if (! Importing)
			{
#ifdef WCS_BUILD_VNS
			// not sure if this log can be open for auto-import operations
			if (ExportLog)
				fclose(ExportLog);
#endif // WCS_BUILD_VNS
			} // else if
		} // else
	} // if !TestOnly

GlobalApp->TemplateLoadInProgress = TemplateLoadStash;

return error;

} // ImportWizGUI::DoLoad

/*===========================================================================*/

short ImportWizGUI::CPSave(void)
{
#ifndef WCS_BUILD_VNS
float MyNorth, MySouth, MyEast, MyWest;
#endif // !WCS_BUILD_VNS
VectorPoint *MyPoints, *PLink;
Joe *Clip, *Added = NULL;
long TotalPoints, TotalCt, PointsThisObject, PointCt;
short error = 0;

TotalPoints = ControlPts;
CurDat = TC;
for (TotalCt = 0; TotalCt < TotalPoints; )
	{
	// first point is reserved for label use
	PointsThisObject = TotalPoints + Joe::GetFirstRealPtNum() - TotalCt >= WCS_DXF_MAX_OBJPTS ? WCS_DXF_MAX_OBJPTS: TotalPoints + Joe::GetFirstRealPtNum() - TotalCt;
#ifndef WCS_BUILD_VNS
	MyNorth = (float)(-90);
	MySouth = (float)(90);
	MyEast = (float)(360);
	MyWest = (float)(-360);
#endif // !WCS_BUILD_VNS
	if (MyPoints = DBHost->MasterPoint.Allocate(PointsThisObject))
		{
		// first point is reserved for label use
		for (PLink = (Joe::GetFirstRealPtNum() > 0 ? MyPoints->Next: MyPoints), PointCt = Joe::GetFirstRealPtNum(); PLink && PointCt < PointsThisObject; PLink = PLink->Next, PointCt ++, TotalCt ++)
			{
			CurDat = CurDat->nextdat;
			PLink->Latitude = CurDat->values[1];
			PLink->Longitude = CurDat->values[0];
			PLink->Elevation = (float)CurDat->values[2];
#ifndef WCS_BUILD_VNS
			if (PLink->Latitude > MyNorth)
				{
				MyNorth = (float)PLink->Latitude;
				} // if
			if (PLink->Latitude < MySouth)
				{
				MySouth = (float)PLink->Latitude;
				} // if
			if (PLink->Longitude > MyWest)
				{
				MyWest = (float)PLink->Longitude;
				} // if
			if (PLink->Longitude < MyEast)
				{
				MyEast = (float)PLink->Longitude;
				} // if
#endif // !WCS_BUILD_VNS
			} // for
		#ifdef WCS_JOE_LABELPOINTEXISTS
		MyPoints->Latitude = MyPoints->Next->Latitude;
		MyPoints->Longitude = MyPoints->Next->Longitude;
		MyPoints->Elevation = MyPoints->Next->Elevation;
		#endif // WCS_JOE_LABELPOINTEXISTS
		if (Clip = new (Importing->NameBase) Joe)
			{
			Clip->TemplateItem = GlobalApp->TemplateLoadInProgress;
			Clip->Points(MyPoints);
			MyPoints = NULL;
			Clip->NumPoints(PointsThisObject);

#ifndef WCS_BUILD_VNS
			Clip->NWLat = MyNorth;
			Clip->NWLon = MyWest;
			Clip->SELat = MySouth;
			Clip->SELon = MyEast;
#endif // !WCS_BUILD_VNS

			Clip->SetFlags(WCS_JOEFLAG_ACTIVATED | WCS_JOEFLAG_DRAWENABLED | WCS_JOEFLAG_RENDERENABLED | WCS_JOEFLAG_ISCONTROL);
			Clip->SetLineStyle(3);
			Clip->SetLineWidth((unsigned char)1);
			Clip->SetRGB((unsigned char)221, (unsigned char)221, (unsigned char)221);
#ifdef WCS_BUILD_VNS
			if (NewCoordSys)
				Clip->AddEffect(NewCoordSys, -1);
			Clip->RecheckBounds();
#endif // WCS_BUILD_VNS
			if (! Importing->DBRenderFlag)
				Clip->ClearFlags(WCS_JOEFLAG_RENDERENABLED);
			Added = DBHost->AddJoe(Clip, WCS_DATABASE_STATIC, ProjHost);
			if (Added)
				{
				DBHost->SetActiveObj(Clip);
				} // if
			else
				{
				UserMessageOK("Import Wizard", "Could not add control points to Database.\nOperation terminated.");
				delete Clip;
				error = 1;
				} // else
			} // if Joe allocated
		else
			{
			error = 1;
			DBHost->MasterPoint.DeAllocate(MyPoints);
			} // else
		} // if points allocated
	else
		{
		error = 1;
		break;
		} // else
	} // for

return error;

} // ImportWizGUI::CPSave

/*===========================================================================*/

short ImportWizGUI::DEMLoad(char TestOnly, NativeGUIWin NW)
{
extern long OCols, ORows;
bool Skip2Cleanup = false;
PlanetOpt *PO;
long cc, cr;	// crop debug vars
short error = 0;

Importing->TestOnly = TestOnly;
if (!TestOnly && (Importing->NewPlanetRadius != 0.0))
	{
	IWGlobeRad = Importing->NewPlanetRadius;
	if (PO = (PlanetOpt *)GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_PLANETOPT))
		PO->AnimPar[WCS_EFFECTS_PLANETOPT_ANIMPAR_RADIUS].SetCurValue(IWGlobeRad);
	} // if
//#ifdef WCS_BUILD_VNS
//if ((Importing->AllowPosE) && (Importing->PosEast) && (Importing->IWCoordSys.Method == WCS_PROJECTIONCODE_GEO))
//	{
//	Importing->WBound = -Importing->WBound;
//	Importing->EBound = -Importing->EBound;
//	}
//#endif // WCS_BUILD_VNS
DataOpsInit();
if (!(DataOpsDEMInit()))
	goto DEMCleanup;

switch (Importing->InFormat)
	{
	default:
		error = 17;
		goto EndDEMLoad;	// Internal error
	case DEM_DATA2_INPUT_ASCII:
		error = LoadASCII_DEM(Importing->LoadName, (float *)Importing->InputData, false);
		break;
	case IW_INPUT_ARCBINARYADF_GRID:
		error = LoadARCBinaryADFDEM(Importing->LoadName, (float *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_ARC_EXPORT_GRID:
		error = LoadARCExportDEM(Importing->LoadName, (float *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_BINARY:
		error = LoadBinary(Importing->LoadName, (UBYTE *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_BRYCE:
		error = LoadBryceDEM(Importing->LoadName, (USHORT *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_DTED:
		error = LoadDTED(Importing->LoadName, (short *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_IFF:	// Any image we read actually
		if (IsTIFFDEM(Importing->LoadName))
			error = LoadTIFFDEM(Importing->LoadName, (float *)Importing->InputData, false);
		else
			{
			if (strcmp(intype, "IMAGE_TIFF") == 0)
				error = LoadDEMImage(Importing->LoadName, (unsigned long *)Importing->InputData, false, DEM_DATA_FORMAT_SIGNEDINT);
			else
				error = LoadDEMImage(Importing->LoadName, (unsigned long *)Importing->InputData, false);
			} // else
		break;
	case DEM_DATA2_INPUT_MDEM:
		error = LoadMDEM(Importing->LoadName, (short *)Importing->InputData, false);
		break;
#ifdef WCS_BUILD_VNS
	case DEM_DATA2_INPUT_NTF_DTM:
		error = LoadNTF_DTM(Importing->LoadName, (float *)Importing->InputData, false);
		break;
#endif // WCS_BUILD_VNS
	case DEM_DATA2_INPUT_SDTS_DEM:
		strcpy(MergeName, Importing->NameBase);
		error = LoadSDTS_DEM(Importing->LoadName, (float *)Importing->InputData, false);
		Skip2Cleanup = true;
		break;
#ifdef GO_SURFING
	case DEM_DATA2_INPUT_SURFER:
		switch (Importing->Flags)
			{
			default:
			case IMWIZ_SURFER_ASCII:
				error = LoadASCII_DEM(Importing->LoadName, (float *)Importing->InputData, false);
				break;
			case IMWIZ_SURFER_BINARY:
				error = LoadBinary(Importing->LoadName, (unsigned char *)Importing->InputData, false);
				break;
			case IMWIZ_SURFER_TAGGED:
				error = LoadSurfer(Importing->LoadName, (float *)Importing->InputData, false);
				break;
			} // Flags
		break;
#endif // GO_SURFING
	case DEM_DATA2_INPUT_TERRAGEN:
		error = LoadTerragen(Importing->LoadName, (float *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_USGS_DEM:
		error = LoadUSGS_DEM(Importing->LoadName, (float *)Importing->InputData, false);
		Skip2Cleanup = true;
		break;
	case DEM_DATA2_INPUT_VISTA:
		error = LoadVistaDEM(Importing->LoadName, (short *)Importing->InputData, false);
		break;
	case DEM_DATA2_INPUT_WCSDEM:
		error = LoadWCSDEM();
		break;
	case DEM_DATA2_INPUT_ZBUF:
		error = LoadWCS_ZBuffer(Importing->LoadName, (char *)Importing->InputData, false);
		break;
	} // InFormat

EndDEMLoad:

	if (error || Skip2Cleanup)
		goto DEMCleanup;

	// change geographic sign notation?
	if (Importing->PosEast)
		{
#ifdef WCS_BUILD_VNS
		if (Importing->IWCoordSys.GetGeographic())	// Geographic
			{
			Importing->WBound = -Importing->WBound;
			Importing->EBound = -Importing->EBound;
			}
#else // WCS_BUILD_VNS
		Importing->WBound = -Importing->WBound;
		Importing->EBound = -Importing->EBound;
#endif // WCS_BUILD_VNS
		} // if

	// Invert data if moving between platforms
	#ifdef BYTEORDER_LITTLEENDIAN
	if (Importing->ByteOrder == DEM_DATA_BYTEORDER_HILO
		&& Importing->ValueBytes != 1
		&& (Importing->InFormat == DEM_DATA2_INPUT_BINARY 
		|| Importing->InFormat == DEM_DATA2_INPUT_ZBUF
		|| Importing->InFormat == DEM_DATA2_INPUT_BRYCE))			// all others byte swapped during loading
	#else
	if (Importing->ByteOrder == DEM_DATA_BYTEORDER_LOHI
		&& Importing->ValueBytes != 1)
	#endif // BYTEORDER_LITTLEENDIAN
		error = DataOpsInvert();

//	// Check for illegal float/double values, which most likely means the user got the byte order wrong
//	if (I
//	DataOpsCheckIllegal();

	if (!error && Importing->UseFloor)
		error = DataOpsFloor();
	if (!error && Importing->UseCeiling)
		error = DataOpsCeiling();

	if (!error && ((cr = CROP_ROWS) != ORows || (cc = CROP_COLS) != OCols))	//lint -e550
		error = DataOpsResample();

	if (!error)
		error = DataOpsScale();
	if (!error && Importing->HasNulls)
		error = DataOpsNull2Min();

	if (TestOnly)
		goto DEMCleanup;

	if (!error && DataOpsEarlyDEMSave())
		goto DEMCleanup;

	if (!error)
		error = DataOpsTransform();

DEMCleanup:
DataOpsCleanup(error);

return ((short)(! error));

} // ImportWizGUI::DEMLoad

/*===========================================================================*/

short ImportWizGUI::VectLoad(char TestOnly)
{
FILE *phyle;
ULONG loaded;
char FileName[256+32];

strcpy(FileName, Importing->LoadName);	// copy this, since it may be fiddled with
Importing->TestOnly = TestOnly;
switch (Importing->InFormat)
	{
	default:
		return -1;
		break;	//lint !e527
	case VECT_DATA2_INPUT_GPX:
		loaded = LoadGPX(FileName, TestOnly);
		break;
	case VECT_DATA2_INPUT_SDTS_DLG:
		loaded = LoadSDTS_DLG(false);
		strcpy(Importing->InFile, "WCS_Temp.dlg");
		strcpy(FileName, Importing->InDir);
		strcat(FileName, Importing->InFile);
		// fall through and load the ASCII DLG we just created
		//lint -fallthrough
	case VECT_DATA2_INPUT_USGS_DLG:
		loaded = Import(FileName, GlobalApp->AppDB->StaticRoot, WCS_DATABASE_IMPORT_DLG);
		break;
	case VECT_DATA2_INPUT_DXF:
		if (TestOnly)
			{
			if (DXFinfo == NULL)
				DXFinfo = (struct ImportData *)AppMem_Alloc(sizeof (struct ImportData), APPMEM_CLEAR);
			loaded = ScanDXF();
			} // if
		else
			{
#ifdef WCS_BUILD_VNS
			//printf("foo");
#else // WCS_BUILD_VNS
			if (Importing->Mode == IMWIZ_HORUNITS_ARBITRARY)
				{
				long hunits = WidgetCBGetCurSel(IDC_CBHOREX_UNITS);
				switch (hunits)
					{
					case 0:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DEGREES;
						break;
					case 1:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_KILOMETERS;
						break;
					default:
					case 2:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_METERS;
						break;
					case 3:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_DECIMETERS;
						break;
					case 4:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_CENTIMETERS;
						break;
					case 5:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_MILES;
						break;
					case 6:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_FEET;
						break;
					case 7:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_SURVEY_FEET;
						break;
					case 8:
						DXFinfo->InputHUnits = WCS_IMPORTDATA_HUNITS_INCHES;
						break;
					}
				Importing->GridUnits = DXFinfo->InputHUnits;
				} // Arbitrary
#endif // WCS_BUILD_VNS
			if (phyle = PROJ_fopen(FileName, "r"))
				{
//				setvbuf(phyle, NULL, _IOFBF, 16384);
				loaded = ImportGISDXF(phyle, GlobalApp->AppDB->StaticRoot, NULL, DXFinfo, NULL, this);
				fclose(phyle);
				}
			else
				loaded = 0;
			AppMem_Free(DXFinfo, sizeof (struct ImportData));
			DXFinfo = NULL;
//			loaded = Import(FileName, GlobalApp->AppDB->StaticRoot, WCS_DATABASE_IMPORT_DXF);
			} // else
		break;
	case VECT_DATA2_INPUT_NTF_MERIDIAN2:
		loaded = Import(FileName, GlobalApp->AppDB->StaticRoot, WCS_DATABASE_IMPORT_MERIDIAN2);
		break;
	case VECT_DATA2_INPUT_SHAPEFILE:
		loaded = Import(FileName, GlobalApp->AppDB->StaticRoot, WCS_DATABASE_IMPORT_SHAPE);
		break;
	} // InFormat

if (loaded == 0)
	return 1;	// an error
else
	return 0;	// no error
	
} // ImportWizGUI::VectLoad

/*===========================================================================*/

void ImportWizGUI::DoNewOutputName(void)
{
char Name[80];

if (WidgetGetModified(IDC_EDITOUTTYPEOUTNAME))
	{
	(void)WidgetGetText(IDC_EDITOUTTYPEOUTNAME, 79, Name);
	strcpy(Importing->NameBase, Name);
	if (Importing->NameBase[0] == 0)
		WidgetSetDisabled(IDC_NEXT, 1);
	else
		WidgetSetDisabled(IDC_NEXT, 0);
	} // if

} // ImportWizGUI::DoNewOutputName

/*===========================================================================*/

void ImportWizGUI::DoNewShapeLayerName(void)
{
char Name[80];

if (WidgetGetModified(IDC_EDITSHAPEOPTS_FILELAYERNAME))
	{
	(void)WidgetGetText(IDC_EDITSHAPEOPTS_FILELAYERNAME, 79, Name);
	strcpy(Importing->LayerName, Name);
	} // if

} // ImportWizGUI::DoNewShapeLayerName

/*===========================================================================*/

void ImportWizGUI::SetElevMethodHeights(void)
{
extern ULONG MaxHite8, MaxHiteAdd, MaxHite16, MaxHite24;
extern ULONG MinHite8, MinHiteAdd, MinHite16, MinHite24;

switch (Importing->Flags)
	{
	default:	// 8 bit
		SCALE_TESTMAX = MaxHite8;
		SCALE_TESTMIN = MinHite8;
		break;
	case ELEV_METHOD_ADD:
		SCALE_TESTMAX = MaxHiteAdd;
		SCALE_TESTMIN = MinHiteAdd;
		break;
	case ELEV_METHOD_16BIT:
		SCALE_TESTMAX = MaxHite16;
		SCALE_TESTMIN = MinHite16;
		break;
	case ELEV_METHOD_24BIT:
		SCALE_TESTMAX = MaxHite24;
		SCALE_TESTMIN = MinHite24;
		break;
	} // switch Flags

} // ImportWizGUI::SetElevMethodHeights

/*===========================================================================*/

void ImportWizGUI::SetElevUnits(void)
{
long choice;

choice = WidgetCBGetCurSel(IDC_CBVERTEX_ELEVUNITS);
switch (choice)
	{
	case 0: // kilometers
		ELEV_UNITS = DEM_DATA_UNITS_KILOM;
		break;
	case 1: // meters
	default:
		ELEV_UNITS = DEM_DATA_UNITS_METERS;
		break;
	case 2: // decimeters
		ELEV_UNITS = DEM_DATA_UNITS_DECIM;
		break;
	case 3: // centimeters
		ELEV_UNITS = DEM_DATA_UNITS_CENTIM;
		break;
	case 4: // miles
		ELEV_UNITS = DEM_DATA_UNITS_MILES;
		break;
	case 5: // International foot
		ELEV_UNITS = DEM_DATA_UNITS_FEET;
		break;
	case 6: // U.S. Survey foot
		ELEV_UNITS = DEM_DATA_UNITS_SURVEY_FEET;
		break;
	case 7: // inches
		ELEV_UNITS = DEM_DATA_UNITS_INCHES;
		break;
	} // switch choice

} // ImportWizGUI::SetElevUnits

/*===========================================================================*/

void ImportWizGUI::SetGridSpacing(void)
{
double NSDimension, WEDimension, Lon_Scale;
long LatGaps, LonGaps;
long tCols, tRows;

if (Importing->OutCols == 0)
	Importing->OutCols = Importing->InCols;

if (Importing->OutRows == 0)
	Importing->OutRows = Importing->InRows;

if ((Importing->OutCols == 1) || (Importing->OutRows == 1))
	return;		// avoid divide by zero

if (Importing->KeepBounds)
	{
	if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) || (Importing->InFormat == DEM_DATA2_INPUT_MDEM))
		{
		tCols = Importing->InRows;
		tRows = Importing->InCols;
		} // if
	else
		{
		tCols = Importing->InCols;
		tRows = Importing->InRows;
		} // else
	} // if KeepBounds
else
	{
	if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) || (Importing->InFormat == DEM_DATA2_INPUT_MDEM))
		{
		tCols = Importing->OutRows;
		tRows = Importing->OutCols;
		} // if
	else
		{
		tCols = Importing->OutCols;
		tRows = Importing->OutRows;
		} // else
	} // else

LatGaps = tRows - 1;
LonGaps = tCols - 1;
NSDimension = (Importing->NBound - Importing->SBound) / LatGaps;
if (Importing->PosEast)
	WEDimension = (Importing->EBound - Importing->WBound) / LonGaps;
else
	WEDimension = (Importing->WBound - Importing->EBound) / LonGaps;
if ((Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES))
	{
	Importing->GridSpaceNS = NSDimension;
	Importing->GridSpaceWE = WEDimension;
	} // if
else if ((Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR) && (Importing->BoundsType == VNS_BOUNDSTYPE_DEGREES))
	{
	Lon_Scale = cos(((Importing->NBound + Importing->SBound) / 2.0) * PiOver180);
	NSDimension *= LatScale(IWGlobeRad);
	WEDimension *= LatScale(IWGlobeRad) * Lon_Scale;
	Importing->GridSpaceNS = NSDimension;
	Importing->GridSpaceWE = WEDimension;
	} // else if
else if ((Importing->GridUnits == VNS_IMPORTDATA_HUNITS_LINEAR) && (Importing->BoundsType == VNS_BOUNDSTYPE_LINEAR))
	{
	Importing->GridSpaceNS = NSDimension;
	Importing->GridSpaceWE = WEDimension;
	} // else if
else
	{
	Lon_Scale = cos(((Importing->NBound + Importing->SBound) / 2.0) * PiOver180);
	NSDimension *= LatScale(IWGlobeRad);
	WEDimension *= LatScale(IWGlobeRad) * Lon_Scale;
	switch (Importing->GridUnits)
		{
		case 1:	// kilometers
			NSDimension *= 0.001;
			WEDimension *= 0.001;
			break;
		default:
		case 2: // meters
			// units are already in meters
			break;
		case 3: // decimeters
			NSDimension *= 10;
			WEDimension *= 10;
			break;
		case 4: // centimeters
			NSDimension *= 100;
			WEDimension *= 100;
			break;
		case 5: // miles
			NSDimension = ConvertFromMeters(NSDimension, WCS_USEFUL_UNIT_MILE_US_STATUTE);
			WEDimension = ConvertFromMeters(WEDimension, WCS_USEFUL_UNIT_MILE_US_STATUTE);
			break;
		case 6: // International foot
			NSDimension = ConvertFromMeters(NSDimension, WCS_USEFUL_UNIT_FEET);
			WEDimension = ConvertFromMeters(WEDimension, WCS_USEFUL_UNIT_FEET);
			break;
		case 7: // U.S. Survey foot
			NSDimension = ConvertFromMeters(NSDimension, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			WEDimension = ConvertFromMeters(WEDimension, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			break;
		case 8: // inches
			NSDimension = ConvertFromMeters(NSDimension, WCS_USEFUL_UNIT_INCH);
			WEDimension = ConvertFromMeters(WEDimension, WCS_USEFUL_UNIT_INCH);
			break;
		} // switch GridUnits
	Importing->GridSpaceNS = NSDimension;
	Importing->GridSpaceWE = WEDimension;
	} // else
AnimPar[IW_ANIMPAR_GRIDNS].CurValue = Importing->GridSpaceNS;
AnimPar[IW_ANIMPAR_GRIDWE].CurValue = Importing->GridSpaceWE;
AnimPar[IW_ANIMPAR_GRIDSIZENS].CurValue = IWGridSizeNS = Importing->GridSpaceNS * tRows;
AnimPar[IW_ANIMPAR_GRIDSIZEWE].CurValue = IWGridSizeWE = Importing->GridSpaceWE * tCols;
WidgetSNSync(IDC_FIHOREXGRIDNS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDWE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDSIZENS, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXGRIDSIZEWE, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXWIDTH, WP_FISYNC_NONOTIFY);
WidgetSNSync(IDC_FIHOREXHEIGHT, WP_FISYNC_NONOTIFY);

} // ImportWizGUI::SetGridSpacing

/*===========================================================================*/

// cursel = WidgetCBGetCurSel on some horizontal units widget
void ImportWizGUI::SetHScale(long cursel)
{

switch (cursel)
	{
	case 0: // kilometers
		Importing->HScale = 1000;
		break;
	default:
	case 1:	// meters
		Importing->HScale = 1.0;
		break;
	case 2: // decimeters
		Importing->HScale = 0.1;
		break;
	case 3: // centimeters
		Importing->HScale = 0.01;
		break;
	case 4: // miles
		Importing->HScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_MILE_US_STATUTE);
		break;
	case 5: // International foot
		Importing->HScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
		break;
	case 6: // U.S. Survey foot
		Importing->HScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
		break;
	case 7: // inches
		Importing->HScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_INCH);
		break;
	} // switch units

WidgetCBSetCurSel(IDC_CBCOORDSYS_HUNITS, cursel);	// In IDD_IMWIZ_COORDSYS
Importing->GridUnits = (short)(cursel + 1);

} // ImportWizGUI::SetHScale

/*===========================================================================*/

// cursel = WidgetCBGetCurSel on some vertical units widget
void ImportWizGUI::SetVScale(long cursel)
{

switch (cursel)
	{
	case 0: // kilometers
		Importing->VScale = 1000.0;
		ELEV_UNITS = DEM_DATA_UNITS_KILOM;
		break;
	default:
		cursel = 1;	// then fall through
		//lint -fallthrough
	case 1:	// meters
		Importing->VScale = 1.0;
		ELEV_UNITS = DEM_DATA_UNITS_METERS;
		break;
	case 2: // decimeters
		Importing->VScale = 0.1;
		ELEV_UNITS = DEM_DATA_UNITS_DECIM;
		break;
	case 3: // centimeters
		Importing->VScale = 0.01;
		ELEV_UNITS = DEM_DATA_UNITS_CENTIM;
		break;
	case 4: // miles
		Importing->VScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_MILE_US_STATUTE);
		ELEV_UNITS = DEM_DATA_UNITS_MILES;
		break;
	case 5: // International foot
		Importing->VScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
		ELEV_UNITS = DEM_DATA_UNITS_FEET;
		break;
	case 6: // U.S. Survey foot
		Importing->VScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
		ELEV_UNITS = DEM_DATA_UNITS_SURVEY_FEET;
		break;
	case 7: // inches
		Importing->VScale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_INCH);
		ELEV_UNITS = DEM_DATA_UNITS_INCHES;
		break;
	} // switch units

// keep the two in sync
WidgetCBSetCurSel(IDC_CBCOORDSYS_VUNITS, cursel);	// In IDD_IMWIZ_COORDSYS
WidgetCBSetCurSel(IDC_CBVERTEX_ELEVUNITS, cursel);	// In IDD_IMWIZ_VERTEX

} // ImportWizGUI::SetVScale

/*===========================================================================*/

#ifdef WCS_BUILD_VNS
void ImportWizGUI::SyncCoordSys(void)
{
char text[80];

text[79] = 0;
strncpy(text, Importing->IWCoordSys.ProjSysName, 79);
WidgetSetText(IDC_TXTCOORDSYS_PSYSTEM, text);
strncpy(text, Importing->IWCoordSys.ZoneName, 79);
WidgetSetText(IDC_TXTCOORDSYS_ZONE, text);
strncpy(text, Importing->IWCoordSys.Method.MethodName, 79);
WidgetSetText(IDC_TXTCOORDSYS_PMETHOD, text);
strncpy(text, Importing->IWCoordSys.Datum.DatumName, 79);
WidgetSetText(IDC_TXTCOORDSYS_GDATUM, text);
strncpy(text, Importing->IWCoordSys.Datum.Ellipse.EllipsoidName, 79);
WidgetSetText(IDC_TXTCOORDSYS_ELLIPSOID, text);

if (Importing->IWCoordSys.GetGeographic())	// Geographic
	Importing->GridUnits = VNS_IMPORTDATA_HUNITS_DEGREES;
else
	Importing->GridUnits = VNS_IMPORTDATA_HUNITS_LINEAR;

if ((Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES))
	WidgetSetDisabled(IDC_CBCOORDSYS_HUNITS, 1);
else
	WidgetSetDisabled(IDC_CBCOORDSYS_HUNITS, 0);

// disable modifying units for SDTS DEMs
if (Importing->InFormat == DEM_DATA2_INPUT_SDTS_DEM)
	{
	WidgetSetDisabled(IDC_CBCOORDSYS_HUNITS, 1);
	WidgetSetDisabled(IDC_CBCOORDSYS_VUNITS, 1);
	} // if

} // ImportWizGUI::SyncCoordSys
#endif // WCS_BUILD_VNS

/*===========================================================================*/

void ImportWizGUI::AutoGrid(void)
{
double lat[2], lon[2];
#ifdef WCS_BUILD_DEMO
bool restricted = false;
#endif // WCS_BUILD_DEMO
char filename[256];

// save the project in case something goes terribly wrong
if (AppScope->MainProj->ProjectLoaded)
	{
	strmfp(filename, AppScope->MainProj->projectpath, AppScope->MainProj->projectname);
	#ifndef WCS_BUILD_DEMO
	if (! AppScope->MainProj->Save(NULL, filename, DBHost, EffectsHost, AppScope->AppImages, NULL, 0xffffffff))
		{
		UserMessageOK("Project: Save", "An error occurred saving the file.\n File could not be opened for writing or an\n error occurred while writing the file. \nCheck disk access privileges and amount of free space.");
		} // if
	AppScope->MainProj->SavePrefs(AppScope->GetProgDir());
	#endif // WCS_BUILD_DEMO
	} // if

TGinfo = (TerraGridder *)GlobalApp->AppEffects->AddEffect(WCS_EFFECTSSUBCLASS_GRIDDER, "LastAutoGrid", NULL);

cpstats.psdX = sqrt((cpstats.sumX2 - ((cpstats.sumX * cpstats.sumX) / cpstats.n)) / cpstats.n);			// RPN would be _SO_ much easer
cpstats.ssdX = sqrt((cpstats.sumX2 - ((cpstats.sumX * cpstats.sumX) / cpstats.n)) / (cpstats.n - 1));	// maybe even APL?
cpstats.meanX = cpstats.sumX / cpstats.n;
cpstats.psdY = sqrt((cpstats.sumY2 - ((cpstats.sumY * cpstats.sumY) / cpstats.n)) / cpstats.n);
cpstats.ssdY = sqrt((cpstats.sumY2 - ((cpstats.sumY * cpstats.sumY) / cpstats.n)) / (cpstats.n - 1));
cpstats.meanY = cpstats.sumY / cpstats.n;
#ifdef DEBUG
printf("X = %lf : %lf, Y = %lf : %lf\n", cpstats.minX, cpstats.maxX, cpstats.minY, cpstats.maxY);
printf("X:\npsd = %lf\nssd = %lf\nmean = %lf\n", cpstats.psdX, cpstats.ssdX, cpstats.meanX);
printf("Y:\npsd = %lf\nssd = %lf\nmean = %lf\n", cpstats.psdY, cpstats.ssdY, cpstats.meanY);
#endif // DEBUG
// are the means dead center?  {OK, now FORTRAN's epsilon would come in handy...}
if ((fabs(cpstats.meanX - ((cpstats.maxX + cpstats.minX) / 2.0)) < 0.000001) &&
	(fabs(cpstats.meanY - ((cpstats.maxY + cpstats.minY) / 2.0)) < 0.000001))
	{
#ifdef DEBUG
	printf("Method: Equal distribution\n");
#endif // DEBUG
	if (cpstats.psdY != 0.0)
		{
		cpstats.ratio = cpstats.psdX / cpstats.psdY;
		cpstats.yn = sqrt((double)cpstats.n) / sqrt(cpstats.ratio);
		cpstats.xn = cpstats.n / cpstats.yn;
		} // if
	else
		{
		cpstats.ratio = 0.0;
		cpstats.yn = 0;
		cpstats.xn = cpstats.n;
		} // else
	cpstats.pctX = 10.0;
	cpstats.pctY = 10.0;
#ifdef DEBUG
	printf("Ratio = %lf, xn = %lf, yn = %lf\n", cpstats.ratio, cpstats.xn, cpstats.yn);
#endif // DEBUG
	cpstats.gridX = LatScale(IWGlobeRad) * (cpstats.maxX - cpstats.minX) / (cpstats.xn - 1);	// the simple answer - equal distribution
	cpstats.avglat = (cpstats.maxY + cpstats.minY) / 2.0;
	cpstats.gridY = cos(cpstats.avglat * Pi / 180) * LatScale(IWGlobeRad) * (cpstats.maxY - cpstats.minY) / (cpstats.yn - 1);
	} // if
else
	{
#ifdef DEBUG
	printf("Method: Special recipe\n");
#endif // DEBUG
	cpstats.offsetX = (cpstats.maxX + cpstats.minX) / 2 - cpstats.meanX;	// center to mean distance
	cpstats.deltaX = 2 * cpstats.psdX + cpstats.offsetX;					// 2 Standard Deviations + offset
	cpstats.offsetY = (cpstats.maxY + cpstats.minY) / 2 - cpstats.meanY;	// center to mean distance
	cpstats.deltaY = 2 * cpstats.psdY + cpstats.offsetY;					// 2 Standard Deviations + offset
	printf("offsetX = %lf, deltaX = %lf\n", cpstats.offsetX, cpstats.deltaX);
	if ((cpstats.meanX + cpstats.deltaX - cpstats.maxX) > (cpstats.minX - (cpstats.meanX - cpstats.deltaX)))
		cpstats.pctX = (cpstats.meanX + cpstats.deltaX - cpstats.maxX) / (cpstats.maxX - cpstats.minX) * 100.0;
	else
		cpstats.pctX = (cpstats.minX - (cpstats.meanX - cpstats.deltaX)) / (cpstats.maxX - cpstats.minX) * 100.0;
	if ((cpstats.meanY + cpstats.deltaY - cpstats.maxY) > (cpstats.minY - (cpstats.meanY - cpstats.deltaY)))
		cpstats.pctY = (cpstats.meanY + cpstats.deltaY - cpstats.maxY) / (cpstats.maxY - cpstats.minY) * 100.0;
	else
		cpstats.pctY = (cpstats.minY - (cpstats.meanY - cpstats.deltaY)) / (cpstats.maxY - cpstats.minY) * 100.0;
	if (cpstats.psdY != 0.0)
		{
		cpstats.ratio = cpstats.psdX / cpstats.psdY;
		cpstats.yn = sqrt((double)cpstats.n) / sqrt(cpstats.ratio);
		cpstats.xn = cpstats.n / cpstats.yn;
		} // if
	else
		{
		cpstats.ratio = 0.0;
		cpstats.yn = 0;
		cpstats.xn = cpstats.n;
		} // else
#ifdef DEBUG
	printf("Ratio = %lf, xn = %lf, yn = %lf\n", cpstats.ratio, cpstats.xn, cpstats.yn);
#endif // DEBUG
	cpstats.gridX = LatScale(IWGlobeRad) * (cpstats.maxX - cpstats.minX) / (cpstats.xn - 1);
	cpstats.avglat = (cpstats.maxY + cpstats.minY) / 2.0;
	cpstats.gridY = cos(cpstats.avglat * Pi / 180) * LatScale(IWGlobeRad) * (cpstats.maxY - cpstats.minY) / (cpstats.yn - 1);
	// These seem to be more in line with what V4 would give
	cpstats.gridX /= 2;
	cpstats.gridY /= 2;
	if (cpstats.pctX < 10.0)
		cpstats.pctX = 10.0;
	if (cpstats.pctY < 10.0)
		cpstats.pctY = 10.0;
	} // else
#ifdef DEBUG
printf("N = %ld\nGrid size X = %lf\nGrid size Y = %lf\n", cpstats.n, cpstats.gridX, cpstats.gridY);
printf("X%% = %lf, Y%% = %lf\n", cpstats.pctX, cpstats.pctY);
#endif // DEBUG

lat[0] = cpstats.minY; lat[1] = cpstats.maxY;
lon[0] = cpstats.minX; lon[1] = cpstats.maxX;
TGinfo->SetBounds(lat, lon);
TGinfo->NCols = long(cpstats.xn + 0.5);
TGinfo->NRows = long(cpstats.yn + 0.5);

#ifdef WCS_BUILD_DEMO
if (TGinfo->NCols > 300)
	{
	restricted = true;
	TGinfo->NCols = 300;
	}
if (TGinfo->NRows > 300)
	{
	restricted = true;
	TGinfo->NRows = 300;
	}
#endif // WCS_BUILD_DEMO

TGinfo->NNG.x_nodes = TGinfo->NCols;
TGinfo->NNG.y_nodes = TGinfo->NRows;
TGinfo->NNG.igrad = true;
TGinfo->NNG.extrap = false;

// set overlap to 0. Change if tiling is implemented in the future.
TGinfo->NNG.horilap = 0.0;
TGinfo->NNG.vertlap = 0.0;

strncpy(TGinfo->NNG.grd_file, Importing->NameBase, sizeof(TGinfo->NNG.grd_file) - 5);
strcat(TGinfo->NNG.grd_file, ".DEM");

#ifdef WCS_BUILD_DEMO
if (restricted)
	UserMessageDemo("The gridding size has been reduced.");
#endif // WCS_BUILD_DEMO

// It looks like MakeGrid returns a boolean success (true = OK, false = failed)
if (TGinfo->MakeGrid(EffectsHost, ProjHost, DBHost))
	{ // nothing to do anymore
	} // if
else
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR,
		"Auto-Gridding failed.");
	UserMessageOK("Import Wizard Error",
		"Auto-Gridding failed.  Try setting the Terrain Gridder values yourself and regrid.");
	} // else

} // ImportWizGUI::AutoGrid

/*===========================================================================*/

short ImportWizGUI::ExportSettings(FILE *iwlog, Pier1 *DumpMe)
{

return DumpMe->DocumentSelf(iwlog);

} // ImportWizGUI::ExportSettings

/*===========================================================================*/

void ImportWizGUI::SetPanelOrder(void)
{

switch (Importing->MyNum)
	{
	case IW_INPUT_ARCASCII:
		PanelOrder = PanelsArcASCII;
		break;
	case IW_INPUT_ARCBINARYADF_GRID:
		PanelOrder = PanelsArcBinaryADFGrid;
		break;
	case IW_INPUT_ARCEXPORT_DEM:
		PanelOrder = PanelsArcExportDEM;
		break;
	case IW_INPUT_ASCII_ARRAY:
		PanelOrder = PanelsASCII;
		break;
	case IW_INPUT_ASCII_GPS:
		PanelOrder = PanelsASCIIGPS;
		break;
	case IW_INPUT_BINARY:
		PanelOrder = PanelsBinary;
		break;
	case IW_INPUT_BRYCE:
		PanelOrder = PanelsBryce;
		break;
	case IW_INPUT_DTED:
		PanelOrder = PanelsDTED;
		break;
	case IW_INPUT_DXF:
		PanelOrder = PanelsDXF;
		break;
	case IW_INPUT_GTOPO30:
		PanelOrder = PanelsGTOPO30;
		break;
	case IW_INPUT_IMAGE_BMP:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_ECW:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_IFF:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_JPEG:
		PanelOrder = PanelsImage;
		break;
//	case IW_INPUT_IMAGE_MR_SID:
//		PanelOrder = PanelsImage;
//		break;
	case IW_INPUT_IMAGE_PICT:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_PNG:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_TARGA:
		PanelOrder = PanelsImage;
		break;
	case IW_INPUT_IMAGE_TIFF:
		if (IsTIFFDEM(Importing->LoadName))
			PanelOrder = PanelsBinary;
		else
			PanelOrder = PanelsImage;
		break;
	case IW_INPUT_MDEM:
		PanelOrder = PanelsMDEM;
		break;
	case IW_INPUT_NTF_DTM:
		PanelOrder = PanelsNTFDTM;
		break;
//	case IW_INPUT_NTF_MERIDIAN2:
//		PanelOrder = PanelsNTFMERIDIAN2;
//		break;
	case IW_INPUT_SDTS_DEM:
		PanelOrder = PanelsSDTSDEM;
		break;
	case IW_INPUT_SDTS_DLG:
		PanelOrder = PanelsSDTSDLG;
		break;
	case IW_INPUT_SHAPE:
		PanelOrder = PanelsShape;
		break;
	case IW_INPUT_SRTM:
		PanelOrder = PanelsSRTM;
		break;
	case IW_INPUT_STM:
		PanelOrder = PanelsSTM;
		break;
	#ifdef GO_SURFING
	case IW_INPUT_SURFER:
		PanelOrder = PanelsSurfer;
		break;
	#endif // GO_SURFING
	case IW_INPUT_TERRAGEN:
		PanelOrder = PanelsTerragen;
		break;
	case IW_INPUT_USGS_ASCII_DEM:
		PanelOrder = PanelsUSGSASCII;
		break;
	case IW_INPUT_USGS_ASCII_DLG:
		PanelOrder = PanelsUSGSDLG;
		break;
	case IW_INPUT_VISTAPRO:
		PanelOrder = PanelsVistaPro;
		break;
	case IW_INPUT_WCS_DEM:
		PanelOrder = PanelsWCSDEM;
		break;
	case IW_INPUT_WCS_XYZ:
		PanelOrder = PanelsWCSXYZ;
		break;
	case IW_INPUT_WCS_ZBUFFER:
		PanelOrder = PanelsWCSZBuf;
		break;
	case IW_INPUT_XYZ:
		PanelOrder = PanelsXYZ;
		break;
	} // switch

} // ImportWizGUI::SetPanelOrder

/*===========================================================================*/

void ImportWizGUI::FinishOuttype(void)
{
long ident, outtype;
char str[80];

intype = Importing->MyIdentity;
outtype = WidgetCBGetCurSel(IDC_CBOUTTYPE_OUTFORMAT);
ident = (long)WidgetCBGetItemData(IDC_CBOUTTYPE_OUTFORMAT, outtype);
RandomCP = false;

if (PROJ_chdir(Importing->OutDir))
	{
	if (UserMessageYN("Import Wizard", "Output directory doesn't exist - create it?"))
		PROJ_mkdir(Importing->OutDir);
	} // if

switch (Importing->LoadAs)
	{
	default:
		break;
	case LAS_CP:
		Importing->OutFormat = DEM_DATA2_OUTPUT_CONTROLPTS;

		if (strcmp(intype, "ASCII_ARRAY") == 0)
			{
			if (Importing->InRows != 1)
				sprintf(str, "%d", Importing->InRows);
			else
				str[0] = 0;
			if (GetInputString("Number of Rows", ":;*/?`#%", str))
				Importing->InRows = atoi(str);
			if (Importing->InCols != 1)
				sprintf(str, "%d", Importing->InCols);
			else
				str[0] = 0;
			if (GetInputString("Number of Cols", ":;*/?`#%", str))
				Importing->InCols = atoi(str);
			} // if

		else if (strcmp(intype, "WCS_XYZ") == 0)
			RandomCP = true;

		else if (strcmp(intype, "XYZ_FILE") == 0)
			RandomCP = true;

		break;
	case LAS_DEM:
		// if USGS or image input, write WCS
		if ((INPUT_FORMAT == DEM_DATA2_INPUT_IFF) || (strcmp(Importing->MyIdentity, "SDTS_DEM") == 0)
			|| (strcmp(Importing->MyIdentity, "USGS_DEM") == 0))
			Importing->OutFormat = DEM_DATA2_OUTPUT_WCSDEM;
		else
			Importing->OutFormat = (short)ident;

		if (strcmp(Importing->MyIdentity, "ASCII_ARRAY") == 0)
			{
			if (Importing->InRows != 1)
				sprintf(str, "%d", Importing->InRows);
			else
				str[0] = 0;
			if (GetInputString("Number of Rows", ":;*/?`#%", str))
				Importing->InRows = atoi(str);
			if (Importing->InCols != 1)
				sprintf(str, "%d", Importing->InCols);
			else
				str[0] = 0;
			if (GetInputString("Number of Cols", ":;*/?`#%", str))
				Importing->InCols = atoi(str);
			} // if

		else if (strcmp(Importing->MyIdentity, "ARC_ASCII_ARRAY") == 0)
			{
#ifndef WCS_BUILD_VNS
			if (Importing->HasUTM)
				{
				UserMessageOK("Import Wizard", "Your data doesn't appear to be in Geographic coordinates (Lat / Lon).\r\n\
Only Geographic data is supported.");
				} // if UTM
#endif // !WCS_BUILD_VNS
			} // if

		break;
	case LAS_IMAGE:
		switch (outtype)
			{
			case 0:
				Importing->OutFormat = DEM_DATA2_OUTPUT_BINARY;
				break;
			case 1:
				Importing->OutFormat = DEM_DATA2_OUTPUT_COLORBMP;
				break;
			case 2:
				Importing->OutFormat = DEM_DATA2_OUTPUT_COLORIFF;
				break;
			case 3:
				Importing->OutFormat = DEM_DATA2_OUTPUT_COLORTGA;
				break;
			case 4:
				Importing->OutFormat = DEM_DATA2_OUTPUT_GRAYIFF;
				break;
			case 5:
				Importing->OutFormat = DEM_DATA2_OUTPUT_ZBUF;
				break;
			default:
				break;
			} // outtype
		break;
	case LAS_VECT:
		break;
	} // switch LoadAs

} // ImportWizGUI::FinishOuttype

/*===========================================================================*/

bool ImportWizGUI::VNS_HorexCheckFails(void)
{
bool failed = true;

if (Importing->NBound > Importing->SBound)
	{
	if (Importing->PosEast)
		{
		if (Importing->EBound > Importing->WBound)
			failed = false;
		} // if
	else
		{
		if (Importing->WBound > Importing->EBound)
			failed = false;
		} // else
	} // if

return failed;

} // ImportWizGUI::VNS_HorexCheckFails

/*===========================================================================*/

void ImportWizGUI::SetSNDefaults(void)
{
double EffectDefault[IW_NUMANIMPAR] = {Importing->GridSpaceNS, Importing->GridSpaceWE, IWGridSizeNS, IWGridSizeWE,
	Importing->InWidth, Importing->InHeight, Importing->NBound, Importing->SBound, Importing->EBound, Importing->WBound};
double RangeDefaults[IW_NUMANIMPAR][3] =
	{FLT_MAX, 0.0, 5.0,			// GridNS
	FLT_MAX, 0.0, 5.0,			// GridWE
	FLT_MAX, 0.0, 5.0,			// GridSizeNS
	FLT_MAX, 0.0, 5.0,			// GridSizeWE
	FLT_MAX, 0.0, 5.0,			// Width
	FLT_MAX, 0.0, 5.0,			// Height
	FLT_MAX, -FLT_MAX, 5.0,		// N
	FLT_MAX, -FLT_MAX, 5.0,		// S
	FLT_MAX, -FLT_MAX, 5.0,		// E
	FLT_MAX, -FLT_MAX, 5.0};	// W
long i;

for (i = 0; i < IW_NUMANIMPAR; i++)
	{
	AnimPar[i].SetDefaults(NULL, (char)i, EffectDefault[i]);
	AnimPar[i].SetRangeDefaults(RangeDefaults[i]);
	} // for

AnimPar[IW_ANIMPAR_GRIDNS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[IW_ANIMPAR_GRIDWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[IW_ANIMPAR_GRIDSIZENS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[IW_ANIMPAR_GRIDSIZEWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
AnimPar[IW_ANIMPAR_WIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
AnimPar[IW_ANIMPAR_HEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
SetMetrics();

} // ImportWizGUI::SetSNDefaults

/*===========================================================================*/
// note that this function has a local variable of the same name as a member so it will overprint the member
// within this function's scope.
//int ImportWizGUI::AutoImport(Pier1 *HeadPier)
int ImportWizGUI::AutoImport(Pier1 *piers)
{
int Success = 1;

if (HeadPier = piers)
	{
	while (HeadPier && ! HeadPier->Enabled && ! HeadPier->Embed)
		HeadPier = HeadPier->Next;
	Importing = HeadPier;
	while (Importing)
		DoLoad(NULL);
	} // if

return (Success);

} // ImportWizGUI::AutoImport

/*===========================================================================*/

// Set any metric that is changable here
void ImportWizGUI::SetMetrics(void)
{

if (Importing->BoundsType == VNS_BOUNDSTYPE_LINEAR)
	{
	AnimPar[IW_ANIMPAR_N].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_S].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_E].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_W].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_WIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_HEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	} // if
else
	{
	AnimPar[IW_ANIMPAR_N].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_S].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_E].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_W].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_WIDTH].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_HEIGHT].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	} // else

if ((Importing->GridUnits == WCS_IMPORTDATA_HUNITS_DEGREES) || (Importing->GridUnits == VNS_IMPORTDATA_HUNITS_DEGREES))
	{
	AnimPar[IW_ANIMPAR_GRIDNS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_GRIDWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_GRIDSIZENS].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	AnimPar[IW_ANIMPAR_GRIDSIZEWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_ANGLE);
	} // if
else
	{
	AnimPar[IW_ANIMPAR_GRIDNS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_GRIDWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_GRIDSIZENS].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	AnimPar[IW_ANIMPAR_GRIDSIZEWE].SetMetricType(WCS_ANIMDOUBLE_METRIC_DISTANCE);
	} // else

} // ImportWizGUI::SetMetrics

/*===========================================================================*/

bool ImportWizGUI::GoodBounds(void)
{

if ((Importing->NBound <= Importing->SBound) || (Importing->PosEast && (Importing->WBound >= Importing->EBound)) ||
	(! Importing->PosEast && (Importing->WBound <= Importing->EBound)))
	return false;
else
	return true;

} // ImportWizGUI::GoodBounds

/*===========================================================================*/

void ImportWizGUI::SetBoundsType(char BoundTypeID)
{

if (BoundTypeID == VNS_BOUNDSTYPE_DEGREES)
	{
	Importing->BoundsType = VNS_BOUNDSTYPE_DEGREES;
	WidgetSetDisabled(IDC_CBHOREX_UNITS, 0);
	} // if
else if (BoundTypeID == VNS_BOUNDSTYPE_LINEAR)
	{
	Importing->BoundsType = VNS_BOUNDSTYPE_LINEAR;
	Importing->GridUnits = VNS_IMPORTDATA_HUNITS_LINEAR;
	WidgetSetDisabled(IDC_CBHOREX_UNITS, 1);
	} // else if

} // ImportWizGUI::SetBoundsType

/*===========================================================================*/

void ImportWizGUI::WidgetSetDisabledRO(WIDGETID WidgetID, char Disabled, NativeGUIWin DestWin)
{

WidgetSetDisabled(WidgetID, Disabled, DestWin);
WidgetEMSetReadOnly(WidgetID, Disabled, DestWin);

} // ImportWizGUI::WidgetSetDisabledRO

/*===========================================================================*/

void ImportWizGUI::SelectNewCoords(void)
{
#ifdef WCS_BUILD_VNS
CoordSys *newObj;
long current;

current = WidgetCBGetCurSel(IDC_IMWIZ_CBCOORDS);
if (((newObj = (CoordSys *)WidgetCBGetItemData(IDC_IMWIZ_CBCOORDS, current, 0)) != (CoordSys *)LB_ERR && newObj)
	|| (newObj = (CoordSys *)EffectsHost->AddEffect(WCS_EFFECTSSUBCLASS_COORDSYS, NULL, NULL)))
	{
	Importing->IWCoordSys.Copy(&Importing->IWCoordSys, newObj);
	} // if
#endif // WCS_BUILD_VNS
} // ImportWizGUI::SelectNewCoords
