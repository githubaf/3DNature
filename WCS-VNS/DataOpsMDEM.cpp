// DataOpsMDEM.cpp
// Microdem code
// Code created on 01/19/01 by Frank Weed II
// Copyright 2001 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "Project.h"
//#include "AppMem.h"

bool ImportWizGUI::ReadMDEMHdr(MDEM_Hdr *hdr, FILE *fin)
{

if (fread(&hdr->NumCol, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->NumRow, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->OSquareID, 3, 1, fin) != 1)
	return false;
if (fread(&hdr->MaxElev, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->MinElev, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->LatInterval, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->LongInterval, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->UnusedWhereMax, 9, 1, fin) != 1)
	return false;
if (fread(&hdr->UnusedWhereMin, 9, 1, fin) != 1)
	return false;
if (fread(&hdr->WhichSphere, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->UnusedOldDatum, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->BaseXUTM24K, 4, 1, fin) != 1)
	return false;
if (fread(&hdr->BaseYUTM24K, 4, 1, fin) != 1)
	return false;
if (fread(&hdr->ElevUnits, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->Unused2, 8, 1, fin) != 1)
	return false;
if (fread(&hdr->UTMZone, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->DEMUsed, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->DataSpacing, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->DigitizeDatum, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->DigitizeLong0, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DigitizeLat0, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DigitizeScale, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DigitizeRadEarth, 4, 1, fin) != 1)
	return false;
if (fread(&hdr->ElevOffset, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->LatHemisph, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->unusedDatumNumber, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->unusedEllpsoidNumber, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.h_Adat, 8, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.h_f, 8, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.h_XDat, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.h_YDat, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.h_ZDat, 2, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.DatumCode, 6, 1, fin) != 1)
	return false;
if (fread(&hdr->DMAMapDefinition.EllipsCode, 3, 1, fin) != 1)
	return false;
if (fread(&hdr->FutureUse, 1, 1, fin) != 1)
	return false;
if (fread(&hdr->CreationInformation, 30, 1, fin) != 1)
	return false;

/*** F2 NOTE: Must do byte flipping if on Motorola ***/

return true;

} // ImportWizGUI::ReadMDEMHdr

/*===========================================================================*/

short ImportWizGUI::LoadMDEM(char *filename, short *Output, char TestOnly)
{
FILE *fin = NULL;
char data[32];
MDEM_Hdr hdr;
//long headpos = 0;
long dempos = 0;
#ifdef WCS_BUILD_VNS
long zonecode;
#endif // WCS_BUILD_VNS
short error;
size_t datasize;

if ((fin = PROJ_fopen(filename, "rb")) == NULL)
	{
	error = 2;
	goto Cleanup;
	}

rewind(fin);
// find signature
error = 6;	// read error
if (fread(data, 1, 14, fin) != 14)
	goto Cleanup;
if (strnicmp(data, "*MICRODEM DEM\r", 14) != 0)
	goto Cleanup;
// read header offset
if (fread(data, 1, 22, fin) != 22)
	goto Cleanup;
if (strnicmp(&data[5], "Offset to Header\r", 17) != 0)
	goto Cleanup;
data[5] = 0;
//headpos = atoi(data);
// read DEM offset
if (fread(data, 1, 19, fin) != 19)
	goto Cleanup;
if (strnicmp(&data[5], "Offset to DEM\r", 14) != 0)
	goto Cleanup;
data[5] = 0;
dempos = atoi(data);
// read END marker
if (fread(data, 1, 5, fin) != 5)
	goto Cleanup;
if (strnicmp(data, "*END\r", 5) != 0)
	goto Cleanup;
// read header
if (!ReadMDEMHdr(&hdr, fin))
	goto Cleanup;
error = 0;	// reset

Importing->InCols = hdr.NumRow;
Importing->InRows = hdr.NumCol;
Importing->TestMax = hdr.MaxElev;
Importing->TestMin = hdr.MinElev;
Importing->HasNulls = TRUE;
Importing->NullVal = 32767.0;
Importing->HdrSize = dempos;
Importing->ValueFmt = DEM_DATA_FORMAT_SIGNEDINT;
Importing->ValueBytes = 2;
#ifdef WCS_BUILD_VNS
if (hdr.DEMUsed == MDEM_TYPE_UTMDEM)	// UTM based
	{
	switch (hdr.DigitizeDatum)
		{
		case 0:	// WGS72
			Importing->IWCoordSys.Method.SetMethodByCode(28);			// Transverse Mercator
			Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(42);	// WGS 72
			break;
		case 1:	// WGS84
			Importing->IWCoordSys.SetSystemByCode(10);	// UTM - WGS84
			break;
		case 2:	// NAD27
			Importing->IWCoordSys.SetSystemByCode(2);	// UTM - NAD27
			break;
		case 3:	// NAD83
			Importing->IWCoordSys.SetSystemByCode(3);	// UTM - NAD83
			break;
		default:
		case 4:	// spherical
			Importing->IWCoordSys.SetSystemByCode(8);	// Back Compatible Sphere
			break;
		} // switch
	} // if
else	// arc second based
	{
	switch (hdr.DigitizeDatum)
		{
		case 0:	// WGS72
			Importing->IWCoordSys.Method.SetMethodByCode(6);			// Geographic
			Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(42);	// WGS 72
			break;
		case 1:	// WGS84
			Importing->IWCoordSys.SetSystemByCode(12);	// Geographic WGS84
			break;
		case 2:	// NAD27
			Importing->IWCoordSys.SetSystemByCode(13);	// Geo - NAD27
			break;
		case 3:	// NAD83
			Importing->IWCoordSys.SetSystemByCode(11);	// Geo - NAD83
			break;
		default:
		case 4:	// spherical
			Importing->IWCoordSys.SetSystemByCode(8);	// Back Compatible Sphere
			break;
		} // switch
	} // else
/***
if (!strnicmp(&hdr.DMAMapDefinition.DatumCode[1], "NAS-C", 5))
	Importing->IWCoordSys.SetSystemByCode(2);
else if (!strnicmp(&hdr.DMAMapDefinition.DatumCode[1], "NAR-C", 5))
	Importing->IWCoordSys.SetSystemByCode(3);
else if (!strnicmp(&hdr.DMAMapDefinition.DatumCode[1], "WGS72", 5))
	Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(42);
else if (!strnicmp(&hdr.DMAMapDefinition.DatumCode[1], "WGS84", 5))
	{
	Importing->IWCoordSys.SetSystemByCode(12);	// Geographic WGS84
//	Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(43);
	}
***/
if (hdr.DEMUsed == MDEM_TYPE_UTMDEM)
	{
	Importing->HasUTM = TRUE;
	if (hdr.LatHemisph == 'S')
		Importing->UTMZone = -hdr.UTMZone;
	else
		Importing->UTMZone = hdr.UTMZone;
	zonecode = Importing->UTMZone;
	UTMZoneNum2DBCode(&zonecode);
	Importing->IWCoordSys.SetZoneByCode(zonecode);
	Importing->WBound = hdr.BaseXUTM24K;
	Importing->SBound = hdr.BaseYUTM24K;
	Importing->NBound = Importing->SBound + hdr.LatInterval * (hdr.NumRow - 1);
	Importing->EBound = Importing->WBound + hdr.LongInterval * (hdr.NumCol - 1);
	Importing->GridSpaceNS = hdr.LatInterval;
	Importing->GridSpaceWE = hdr.LongInterval;
	SetBoundsType(VNS_IMPORTDATA_HUNITS_LINEAR);
	}
else
	{
	SetBoundsType(VNS_IMPORTDATA_HUNITS_DEGREES);
	switch (hdr.DataSpacing)
		{
		default:
			break;
		case 1:	// SpaceSeconds
			Importing->SBound = hdr.BaseYUTM24K / 3600.0;
			Importing->WBound = hdr.BaseXUTM24K / 3600.0;
			Importing->NBound = Importing->SBound + ((hdr.NumRow - 1) * hdr.LatInterval) / 3600.0;
			Importing->EBound = Importing->WBound + ((hdr.NumCol - 1) * hdr.LongInterval) / 3600.0;
			break;
		}
	}
#endif // WCS_BUILD_VNS

switch (hdr.ElevUnits)
	{
	default:
	case 0: // Meters
		Importing->ElevUnits = DEM_DATA_UNITS_METERS;
		break;
	case 1: // Feet
		Importing->ElevUnits = DEM_DATA_UNITS_FEET;
		break;
/***
	case 2: // TenthMgal
		break;
	case 3: // Milligal
		break;
	case 4: // TenthGamma
		break;
	case 5: // Decimeters
		break;
	case 6: // Gammas
		break;
	case 7: // HundredthMGal
		break;
	case 8: // DeciFeet
		break;
	case 9: // Centimeters
		break;
	case 10: // OtherElev
		break;
	case 11: // HundredthMa
		break;
	case 12: // HundredthPercentSlope
		break;
	case 13: // Undefined
		break;
	case 14: // zDegrees
		break;
	case 15: // UndefinedHundredth
		break;
***/
	}

if (TestOnly)
	return 0;

//ReadColumnDEM();
datasize = Importing->InCols * Importing->InRows * Importing->ValueBytes;
if (fread(Output, datasize, 1, fin) != 1)
	return 6;	// read error
strcpy(Importing->MyIdentity, "BINARY");

Cleanup:
if (fin)
	fclose(fin);
return error;

} // ImportWizGUI::LoadMDEM
