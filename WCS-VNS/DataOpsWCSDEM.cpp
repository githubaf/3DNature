// DataOpsWCSDEM.cpp
// Code for reading & writing WCS DEMs
// Written by Frank Weed II on 9/24/99
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Useful.h"
#include "Log.h"
#include "Requester.h"
#include "WCSWidgets.h"
#include "Joe.h"
#include "Project.h"
#include "DEM.h"

extern void *InputData;
extern long InputDataSize;
extern DEM *DOTopo;

/*===========================================================================*/

short ImportWizGUI::GetWCSInputFile(void)
{
CoordSys *MyCoords = NULL;
FILE *fh = NULL;
long filesize;
DEM Topo;
char filename[255];

strcpy(filename, Importing->LoadName);
if ((fh = PROJ_fopen(filename, "rb")) == 0)
	{
	INPUT_FILESIZE = 0;
	UserMessageOK(INPUT_FILENAME, "Unable to open file for input!");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_OPEN_FAIL, INPUT_FILENAME);
	return 1;
	} // if open error
fseek(fh, 0L, SEEK_END);
if ((filesize = ftell(fh)) < 0)
	{
	INPUT_FILESIZE = 0;
	UserMessageOK(INPUT_FILENAME, "Unable to obtain file size!");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_READ_FAIL, INPUT_FILENAME);
	return 2;
	} // if file size read fail
else
	{
	INPUT_FILESIZE = filesize;
	} // else file size read OK

fseek(fh, 0L, SEEK_SET);

fclose(fh);
fh = NULL;

if (Topo.LoadDEM(filename, 0, &MyCoords))
	{
	if (MyCoords == NULL)	// DEM must've originated in WCS
		{
		Importing->PosEast = FALSE;
		Importing->AllowPosE = FALSE;
#ifdef WCS_BUILD_VNS
		Importing->BoundsType = VNS_BOUNDSTYPE_DEGREES;
		//Importing->IWCoordSys.SetSystemByCode(8);	// Back compatible sphere
		Importing->FileInfo[1023] = '~';
#endif // WCS_BUILD_VNS
		} // if
#ifndef WCS_BUILD_VNS
	else if (MyCoords && (MyCoords->Method.GCTPMethod != 0))
		{
		delete MyCoords;
		MyCoords = NULL;
		UserMessageOK(INPUT_FILENAME, "Can't load VNS Non-Geographic DEM");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_READ_FAIL, INPUT_FILENAME);
		return 50;
		}
#endif // !WCS_BUILD_VNS
	// Use the filename as the default database name
	strcpy(Importing->NameBase, Importing->InFile);
	(void)StripExtension(Importing->NameBase);
	INPUT_HEADER = 0;
	INPUT_ROWS = (long)Topo.pLonEntries; 
	INPUT_COLS = (long)Topo.pLatEntries;
	INVALUE_SIZE = DEM_DATA_VALSIZE_LONG; 
	INVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
	#ifdef BYTEORDER_LITTLENDIAN
	INBYTE_ORDER = DEM_DATA_BYTEORDER_LOHI;
	#else
	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
	#endif
	READ_ORDER = DEM_DATA_READORDER_COLS;
	ROWS_EQUAL = DEM_DATA_ROW_LON;
	if (fabs (Topo.pElScale - ELSCALE_KILOM) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_KILOM;
	else if (fabs (Topo.pElScale - ELSCALE_METERS) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_METERS;
	else if (fabs (Topo.pElScale - ELSCALE_DECIM) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_DECIM;
	else if (fabs (Topo.pElScale - ELSCALE_CENTIM) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_CENTIM;
	else if (fabs (Topo.pElScale - ELSCALE_MILLIM) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_MILLIM;
	else if (fabs (Topo.pElScale - ELSCALE_MILES) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_MILES;
	else if (fabs (Topo.pElScale - ELSCALE_YARDS) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_YARDS;
	else if (fabs (Topo.pElScale - ELSCALE_FEET) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_FEET; 
	else if (fabs (Topo.pElScale - ELSCALE_INCHES) < .0001)
		ELEV_UNITS = DEM_DATA_UNITS_INCHES;
	else
		ELEV_UNITS = DEM_DATA_UNITS_OTHER;
	OUTPUT_LOLAT = Topo.pSouthEast.Lat;
	OUTPUT_HILON = Topo.pNorthWest.Lon;
	OUTPUT_HILAT = Topo.pNorthWest.Lat;
	OUTPUT_LOLON = Topo.pSouthEast.Lon;
	Importing->InHeight = OUTPUT_HILAT - OUTPUT_LOLAT;
	if (Importing->PosEast)
		Importing->InWidth = Importing->EBound - Importing->WBound;
	else
		Importing->InWidth = Importing->WBound - Importing->EBound;
	Importing->GridSpaceNS = Importing->InHeight / (INPUT_ROWS - 1);
	Importing->GridSpaceWE = Importing->InWidth / (INPUT_COLS - 1);
	Importing->AllowRef00 = FALSE;
#ifdef WCS_BUILD_VNS
	Importing->AskNull = TRUE;
	Importing->HasNulls = (char)Topo.NullReject;
	Importing->NullVal = Topo.pNullValue;
#else // WCS_BUILD_VNS
	Importing->AllowPosE = FALSE;
	Importing->AskNull = FALSE;
#endif // WCS_BUILD_VNS
	Importing->FullGeoRef = TRUE;
	Importing->TestMin = Topo.pElMinEl;
	Importing->TestMax = Topo.pElMaxEl;
	if (MyCoords)
		{
#ifdef WCS_BUILD_VNS
		Importing->IWCoordSys.Copy(&Importing->IWCoordSys, MyCoords);
#endif // WCS_BUILD_VNS
		delete MyCoords;
		MyCoords = NULL;
		} // if
	} // if Topo.LoadDEM
else
	{
	if (MyCoords)
		{
		delete MyCoords;
		MyCoords = NULL;
		} // if
	UserMessageOK(INPUT_FILENAME, "Unable to obtain file size!");
	GlobalApp->StatusLog->PostStockError(WCS_LOG_WNG_READ_FAIL, INPUT_FILENAME);
	return 3;
	} // else Topo.LoadDEM

return 0;

} // ImportWizGUI::GetWCSInputFile

/*===========================================================================*/

short ImportWizGUI::LoadWCSDEM(void)
{
CoordSys *MyCoords = NULL;
short error = 0;

if (! DOTopo->LoadDEM(Importing->LoadName, 0, &MyCoords))
	{
	error = 1;
	goto Cleanup;
	} // if read fail
InputData = InputData4F = DOTopo->RawMap;
if (InputDataSize != (long)(DOTopo->MapSize() * sizeof (float)))
	{
	InputDataSize = (long)(DOTopo->MapSize() * sizeof (float));
	error = 3;
	goto Cleanup;
	} // if wrong file size
InputDataSize = (long)(DOTopo->MapSize() * sizeof (float));
INPUT_ROWS = (long)DOTopo->pLonEntries;
INPUT_COLS = (long)DOTopo->pLatEntries;
if (OUTPUT_LOLAT == OUTPUT_HILAT || OUTPUT_HILON == OUTPUT_LOLON)
	{
	OUTPUT_LOLAT = DOTopo->pSouthEast.Lat;
	OUTPUT_HILAT = DOTopo->pNorthWest.Lat;
	OUTPUT_HILON = DOTopo->pNorthWest.Lon;
	OUTPUT_LOLON = DOTopo->pSouthEast.Lon;
	} // if no useful values entered in lat/lon table
#ifndef WCS_BUILD_VNS
if (OUTPUT_LOLAT > OUTPUT_HILAT)
	swmem(&OUTPUT_HILAT, &OUTPUT_LOLAT, sizeof (double));
if (OUTPUT_LOLON > OUTPUT_HILON)
	swmem(&OUTPUT_HILON, &OUTPUT_LOLON, sizeof (double));
if (OUTPUT_HILAT > 90.0 && OUTPUT_HILON < 90.0)
	{
	swmem(&OUTPUT_HILAT, &OUTPUT_HILON, sizeof (double));
	swmem(&OUTPUT_LOLAT, &OUTPUT_LOLON, sizeof (double));
	} // user got latitude and longitude reversed
#endif // !WCS_BUILD_VNS

Cleanup:

if (MyCoords)
	{
#ifdef WCS_BUILD_VNS
	Importing->IWCoordSys.Copy(&Importing->IWCoordSys, MyCoords);
#endif // WCS_BUILD_VNS
	delete MyCoords;
	MyCoords = NULL;
	} // if
return error;

} // ImportWizGUI::LoadWCSDEM
