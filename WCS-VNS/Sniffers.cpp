// Sniffers.cpp
// Import Wizard file identification code
// 08/23/00 FPW2 - code yanked from ImportWizGUI.cpp
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
#include "DEM.h"
#include "DataOpsDefs.h"
#include "requester.h"
#include "ImageInputFormat.h"
#include "Useful.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "CoordSys.h"

//extern WCSApp *GlobalApp;

void ImportWizGUI::CopyCoordSys(CoordSys *TempCS)
{
CoordSys *curCS = EffectsHost->Coord;
bool found = false;

while (curCS)
	{
	if (curCS->Equals(TempCS))
		{
		found = true;
		curCS->Copy(&Importing->IWCoordSys, curCS);
		break;
		} // if
	if (curCS == EffectsHost->LastCoord)
		break;
	curCS = (CoordSys *)curCS->Next;
	} // while

if (! found)
	TempCS->Copy(&Importing->IWCoordSys, TempCS);

} // ImportWizGUI::CopyCoordSys

/*===========================================================================*/

bool ImportWizGUI::Sniff3DS(FILE *phyle)
{

// simple test - not called by TryAllImportable
rewind(phyle);
if (((char)fgetc(phyle) == 'M') && ((char)fgetc(phyle) == 'M'))
	return true;
return false;

} // ImportWizGUI::Sniff3DS

/*===========================================================================*/

bool ImportWizGUI::SniffArcASCIIArray(FILE *phyle)
{
double XCellSize = 0.0, YCellSize = 0.0, NullData = -9999.0;
char *NextWord;
BYTE found = 0;
short i;
#ifdef WCS_BUILD_VNS
char prjname[256+32];
#endif // WCS_BUILD_VNS
char line[128];
unsigned char NextChar;

rewind(phyle);
for (i = 0; i < 10; i++)
	{
	(void)fgetline(line, 100, phyle);
	if (! strnicmp(line, "ncols", 4))
		{
		found += 1;
		NextWord = ScanToNextWord(line);
		INPUT_COLS = atoi(NextWord);
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "nrows", 4))
		{
		found += 2;
		NextWord = ScanToNextWord(line);
		INPUT_ROWS = atoi(NextWord);
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "xllc", 4))
		{
		found += 4;
		NextWord = ScanToNextWord(line);
		OUTPUT_HILON = atof(NextWord);
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "yllc", 4))
		{
		found += 8;
		NextWord = ScanToNextWord(line);
		OUTPUT_LOLAT = atof(NextWord);
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "cell", 4))
		{
		found += 16;
		NextWord = ScanToNextWord(line);
		XCellSize = YCellSize = atof(NextWord);	// only NIMA stores Y separate
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "ycel", 4))		// NIMA version
		{
		NextWord = ScanToNextWord(line);
		YCellSize = atof(NextWord);
		INPUT_HEADER = ftell(phyle);
		} // if
	else if (! strnicmp(line, "noda", 4))		// "nodata_value"
		{
		NextWord = ScanToNextWord(line);
		NullData = atof(NextWord);
		INPUT_HEADER = ftell(phyle);
		Importing->HasNulls = TRUE;
		} // if
	else
		{
		NextWord = ScanToNextWord(line);
		NextChar = *NextWord;
		if (isdigit((int)NextChar))
			break;
		else if ((NextChar == '-') || (NextChar == '.'))
			break;
		else
			return FALSE;
		} // else
	} // for

if (found != 31)	// we didn't encounter all the above required fields
	return FALSE;

Importing->AllowRef00 = FALSE;
Importing->InFormat = DEM_DATA2_INPUT_ASCII;
Importing->NullVal = NullData;

#ifdef WCS_BUILD_VNS
OUTPUT_LOLON = OUTPUT_HILON + (INPUT_COLS - 1) * XCellSize;
OUTPUT_HILAT = OUTPUT_LOLAT + (INPUT_ROWS - 1) * YCellSize;
Importing->GridSpaceNS = YCellSize;
Importing->GridSpaceWE = XCellSize;
strcpy(prjname, noextname);
strcat(prjname, ".prj");
ReadArcPrj(prjname);	// attempt to set CoordSys via Arc projection file
#else // WCS_BUILD_VNS
if ((OUTPUT_LOLAT > 100000) || (OUTPUT_HILON > 100000))
	Importing->HasUTM = TRUE;

if (YCellSize < 0.0)
	YCellSize = -YCellSize;

if ((XCellSize > 0.0) && (!(Importing->HasUTM)))
	{
	OUTPUT_LOLON = OUTPUT_HILON + (INPUT_COLS - 1) * XCellSize;
	OUTPUT_HILAT = OUTPUT_LOLAT + (INPUT_ROWS - 1) * YCellSize;
	} // if
#endif // WCS_BUILD_VNS

READ_ORDER = DEM_DATA_READORDER_ROWS; 
ROWS_EQUAL = DEM_DATA_ROW_LAT; 
ELEV_UNITS = DEM_DATA_UNITS_METERS; 

return true;

} // ImportWizGUI::SniffArcASCIIArray

/*===========================================================================*/

bool ImportWizGUI::SniffArcBinaryADFGrid(FILE *phyle, char *fullname)
{
FILE *test;
long fpos;
char buffer[32], fPath[256], fFile[32], testname[256+32];

rewind(phyle);
// we really only care about the directory this file is in
BreakFileName(fullname, fPath, 256, fFile, 32);
// check for hdr.adf in directory
strcpy(testname, fPath);
strcat(testname, "hdr.adf");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if ((fread(buffer, 1, 8, test) != 8) || (strcmp(buffer, "GRID1.2") != 0))
	return false;
fclose(test);
// check for dblbnd.adf & correct size	
strcpy(testname, fPath);
strcat(testname, "dblbnd.adf");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
fseek(test, 0, SEEK_END);
fpos = ftell(test);
fclose(test);
if (fpos != 32)
	return false;
// check for sta.adf & correct size
strcpy(testname, fPath);
strcat(testname, "sta.adf");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
fseek(test, 0, SEEK_END);
fpos = ftell(test);
fclose(test);
if (fpos != 32)
	return false;
// check for w001001.adf
strcpy(testname, fPath);
strcat(testname, "w001001.adf");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
fclose(test);
// check for w001001x.adf
strcpy(testname, fPath);
strcat(testname, "w001001x.adf");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
fclose(test);
return true;

} // ImportWizGUI::SniffArcBinaryADFGrid

/*===========================================================================*/

bool ImportWizGUI::SniffArcExportDEM(FILE *phyle)
{
char buffer[256];

rewind(phyle);
fgetline(buffer, 255, phyle);
if (strncmp(buffer, "EXP  0", 6) != 0)
	return false;
fgetline(buffer, 255, phyle);
if (strncmp(buffer, "GRD  2", 6) != 0)
	return false;

return true;

} // ImportWizGUI::SniffArcExportDEM

/*===========================================================================*/

// is the file 100% usable ASCII?
// return 0 = no, 1 = digits only, 2 = possibly exponential float, 3 = text found
/*** F2 NOTE - We should meter this sucker too ***/
SBYTE ImportWizGUI::SniffASCII(FILE *phyle)
{
int ch;
bool foundtext = false;
bool floating = true;	// exponent char - only valid if foundtext is TRUE

rewind(phyle);
ch = fgetc(phyle);
if (ch == EOF)
	return 0;
while (ch != EOF)
	{
	if (isdigit(ch) || isspace(ch))
		;	// do nothing - quite fast actually
	else if (isalpha(ch))
		{
		if (((char) ch != 'e') && ((char) ch != 'E'))	// possibly exponent char
			floating = FALSE;
		foundtext = TRUE;
		}
	else if (((char) ch == '.') || ((char) ch == ',') || ((char) ch == '+') || ((char) ch == '-'))
		;	// nothing again
	else
		return 0;
	ch = fgetc(phyle);
	}
if (!foundtext)
	return 1;
if (floating)
	return 2;
return 3;

} // ImportWizGUI::SniffASCII

/*===========================================================================*/

bool ImportWizGUI::SniffASCIIArray(FILE *phyle)
{
short between, lines = 0, spacing = 0;
char *got, line[8192], *token;
char seps[] = " ,\t";

rewind(phyle);
got = fgetline(line, 8191, phyle);
if (got == NULL)
	return false;
while (got != NULL)
	{
	lines++;
	between = 0;
	token = strtok(line, seps);
	while (token != NULL)
		{
		between++;	// how many separators between numbers
		token = strtok(NULL, seps);
		};
	if (spacing == 0)
		spacing = between;
	else
		{
		if (spacing != between)	// file is irregular
			return false;
		}
	got = fgetline(line, 8191, phyle);
	}
/*** F2 Note: rows = lines, cols = spacing ***/
Importing->InCols = spacing;
Importing->InRows = lines;
return true;

} // ImportWizGUI::SniffASCIIArray

/*===========================================================================*/

bool ImportWizGUI::SniffBIL(FILE *phyle)
{
FILE *test;
char testname[256+32];

strcpy(testname, noextname);
strcat(testname, ".prj");	// look for the projection file
if ((test = PROJ_fopen(testname, "rb")) != NULL)
	{
	(void)ReadUSGSProjection(test);	// read what we can
	fclose(test);
	}
strcpy(testname, noextname);
strcat(testname, ".hdr");	// look for the header file
if ((test = PROJ_fopen(testname, "rb")) != NULL)
	{
	(void)ReadBinHeader(test);
	fclose(test);
	}
if (SniffBinary(phyle))
	return true;
else
	return false;

} // ImportWizGUI::SniffBIL

/*===========================================================================*/

bool ImportWizGUI::SniffBinary(FILE *phyle)
{

// make sure the file is as big as we think it needs to be
if (fseek(phyle, Importing->HdrSize + Importing->InRows * Importing->InCols * 
	Importing->Bands * Importing->ValueBytes, SEEK_SET) != 0)
	return false;
else
	return true;

} // ImportWizGUI::SniffBinary

/*===========================================================================*/

bool ImportWizGUI::SniffBIP(FILE *phyle)
{

if (SniffBinary(phyle))
	return true;
else
	return false;

} // ImportWizGUI::SniffBIP

/*===========================================================================*/

bool ImportWizGUI::SniffBMP(FILE *phyle)
{
long fsize, start;
char data[14];

#ifdef BYTEORDER_BIGENDIAN
signed long int tmp;
#endif // BYTEORDER_BIGENDIAN

rewind(phyle);
if (fread(data, 1, 14, phyle) != 14)
	return FALSE;
if ((data[0] == 'B') && (data[1] == 'M'))	// magic markers
	{
	// next 4 bytes are filesize
	#ifdef BYTEORDER_BIGENDIAN
	memcpy(&tmp, &data[2], 4);
	SimpleEndianFlip32S(tmp, &fsize);
	#else
	memcpy(&fsize, &data[2], 4);
	#endif // BYTEORDER_BIGENDIAN
	fseek(phyle, 0, SEEK_END);
	if (ftell(phyle) != fsize)
		return FALSE;
	if ((data[6] != 0) || (data[7] != 0) || (data[8] != 0) || (data[9] != 0))
		return FALSE;
	// next 4 bytes are offset to start of image data
	#ifdef BYTEORDER_BIGENDIAN
	memcpy(&tmp, &data[10], 4);
	SimpleEndianFlip32S(tmp, &start);
	#else
	memcpy(&start, &data[10], 4);
	#endif // BYTEORDER_BIGENDIAN
	if (start >= fsize)
		return false;
	else
		return true;
	} // if "BM"
return false;

} // ImportWizGUI::SniffBMP

/*===========================================================================*/

bool ImportWizGUI::SniffBryce(FILE *phyle)
{
char str[128];

rewind(phyle);
if (fgetline(str, 128, phyle))
	{
	if ((strncmp(str, "CCmF - Universal - Axiom", 24) == 0) ||	// pre-Corel Bryce
		(strncmp(&str[94], "CCmFile::kIdentify4", 19) == 0))	// Corel Bryce 5
		return true;
	}
return false;

} // ImportWizGUI::SniffBryce

/*===========================================================================*/

bool ImportWizGUI::SniffBSQ(FILE *phyle)
{

if (SniffBinary(phyle))
	return true;
else
	return false;

} // ImportWizGUI::SniffBSQ

/*===========================================================================*/

bool ImportWizGUI::SniffDTED(FILE *phyle, const char *filename)
{
size_t slen;
long StartPt = 0, ct;
char test[4];

slen = strlen(filename);
if (slen < 4)
	return false;
if ((strnicmp(&filename[slen - 4], ".dt", 3) != 0) && (strnicmp(&filename[slen - 4], ".tar", 4) != 0))
	return false;
fseek(phyle, 160, SEEK_SET);
(void)fread(test, 1, 3, phyle);
test[3] = 0;
if (strcmp(test, "DSI"))
	{
	fseek(phyle, 80, SEEK_SET);
	(void)fread(test, 1, 3, phyle);
	test[3] = 0;
	if (! strcmp(test, "DSI"))
		StartPt = -80;
	else
		{
		StartPt = -30000;
		fseek(phyle, 0, SEEK_SET);
		for (ct=0; ct<20000; ct++)
			{
			(void)fread(test, 1, 1, phyle);
			if (test[0] == 'D')
				{
				(void)fread(test, 1, 1, phyle);
				if (test[0] == 'S')
					{
					(void)fread(test, 1, 1, phyle);
					if (test[0] == 'I')
						{
						StartPt = ftell(phyle) - 163;
						break;
						} // if
					} // if
				} // if
			} // for col
		} // else look for DSI
	} // if
if (StartPt <= -30000)
	return false;

return true;

} // ImportWizGUI::SniffDTED

/*===========================================================================*/

bool ImportWizGUI::SniffDXF(FILE *phyle)
{
short code;
char line[4096];
#ifdef WCS_BUILD_VNS
char prjname[256+32];
#endif // WCS_BUILD_VNS

rewind(phyle);
if (fscanf(phyle, "%hd", &code) != 1)
	return false;
// skip comment lines
while (code == 999)
	{
	if (fgetline(line, 4096, phyle) == NULL)	// pick up CR/LF
		return false;
	if (fgetline(line, 4096, phyle) == NULL)
		return false;
	if (fscanf(phyle, "%hd", &code) != 1)
		return false;
	} // while
if (code != 0)
	return false;
if (fscanf(phyle, "%s", line) != 1)
	return false;
if (stricmp(line, "SECTION") != 0)
	return false;
if (fscanf(phyle, "%hd", &code) != 1)
	return false;
if (code != 2)
	return false;

#ifdef WCS_BUILD_VNS
strcpy(prjname, noextname);
strcat(prjname, ".prj");
ReadArcPrj(prjname);	// attempt to set CoordSys via Arc projection file
#endif // WCS_BUILD_VNS

return true;

} // ImportWizGUI::SniffDXF

/*===========================================================================*/

bool ImportWizGUI::SniffGPS(FILE *phyle, bool force)
{
char gpsLine[256];

rewind(phyle);
fgetline(gpsLine, 256, phyle);

if (! strncmp(gpsLine, "OziExplorer", 11))
	return true;

if (! strcmp(gpsLine, "Version 2"))
	return true;

if (force)
	return true;
else
	return false;

} // ImportWizGUI::SniffGPS

/*===========================================================================*/

bool ImportWizGUI::SniffGPX(FILE *phyle)
{
bool rVal = false;
char gpsLine[256];

rewind(phyle);
fgetline(gpsLine, 256, phyle);

if (strncmp(gpsLine, "<?xml version", 13))
	return false;

for (long i = 0; i < 10; ++i)
	{
	fgetline(gpsLine, 256, phyle);
	if (strncmp(gpsLine, "<gpx", 4) == 0)
		return true;
	} // for

return rVal;

} // ImportWizGUI::SniffGPX

/*===========================================================================*/

//lint -save -e731
bool ImportWizGUI::SniffGTOPO30(FILE *phyle, char *fullname)
{
char *ext;
FILE *test;
char testname[256+32];

strcpy(testname, noextname);
strcat(testname, ".dmw");	// look for the world file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (ReadWorldFile(test) == false)
	{
	fclose(test);
	return false;
	} // if
fclose(test);
strcpy(testname, noextname);
strcat(testname, ".prj");	// look for the projection file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (ReadUSGSProjection(test) == false)
	{
	fclose(test);
	return false;
	} // if
fclose(test);
strcpy(testname, noextname);
strcat(testname, ".hdr");	// look for the header file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (! ReadBinHeader(test))
	{
	fclose(test);
	return false;
	} // if
ext = FindFileExtension(fullname);
if ((stricmp(ext, "dem") == 0) && SniffBinary(phyle))
	return true;										// correct file checks out
strcpy(testname, noextname);							// set to correct file name
strcat(testname, ".dem");
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;										// can't open correct file
if (SniffBinary(test))
	{
	fclose(test);
	strcpy(fullname, testname);							// send back the correct name
	return true;										// correct file checks out
	} // if
else
	return false;

} // ImportWizGUI::SniffGTOPO30

/*===========================================================================*/

bool ImportWizGUI::SniffGZIP(FILE *phyle)
{
UBYTE ident[4];

rewind(phyle);
(void)fread(ident, 1, 4, phyle);
if ((ident[0] == 0x1F) && (ident[1] == 0x8B))
	return true;
else
	return false;

} // ImportWizGUI::SniffGZIP

/*===========================================================================*/

bool ImportWizGUI::SniffIFF(FILE *phyle)
{
char data[16];
long fsize;	// technically, fsize is a ULONG, but ftell returns longs
#ifdef BYTEORDER_LITTLEENDIAN
long tmp;
#endif // BYTEORDER_LITTLEENDIAN

rewind(phyle);
if (fread(data, 1, 16, phyle) != 16)
	return false;
// look for magic markers
if ((data[0] != 'F') || (data[1] != 'O') || (data[2] != 'R') || (data[3] != 'M') ||
	(data[8] != 'I') || (data[9] != 'L') || (data[10] != 'B') || (data[11] != 'M') ||
	(data[12] != 'B') || (data[13] != 'M') || (data[14] != 'H') || (data[15] != 'D'))
	return false;
// read chunksize
#ifdef BYTEORDER_LITTLEENDIAN
memcpy(&tmp, &data[4], 4);
SimpleEndianFlip32S(tmp, &fsize);
#else
memcpy(&fsize, &data[4], 4);
#endif // BYTEORDER_LITTLEENDIAN
fseek(phyle, 0, SEEK_END);
fsize += 8;	// chunksize + first 8 bytes
if ((ftell(phyle) == fsize) || (ftell(phyle) == (fsize + 1)))
	return true;
else
	return false;

} // ImportWizGUI::SniffIFF

/*===========================================================================*/

bool ImportWizGUI::SniffLWOB(FILE *phyle)
{
char data[12];

rewind(phyle);
if (fread(data, 1, 12, phyle) != 12)
	return false;
// look for magic markers
if ((data[0] != 'F') || (data[1] != 'O') || (data[2] != 'R') || (data[3] != 'M') ||
	(data[8] != 'L') || (data[9] != 'W') || (data[10] != 'O') || (data[11] != 'B'))
	return false;
return true;

} // ImportWizGUI::SniffLWOB

/*===========================================================================*/

bool ImportWizGUI::SniffMDEM(FILE *phyle)
{
char data[32];

rewind(phyle);
// find signature
if (fread(data, 1, 14, phyle) != 14)
	return false;
if (strnicmp(data, "*MICRODEM DEM\r", 14) != 0)
	return false;
// read header offset
if (fread(data, 1, 22, phyle) != 22)
	return false;
if (strnicmp(&data[5], "Offset to Header\r", 17) != 0)
	return false;
// read DEM offset
if (fread(data, 1, 19, phyle) != 19)
	return false;
if (strnicmp(&data[5], "Offset to DEM\r", 14) != 0)
	return false;
// read END marker
if (fread(data, 1, 5, phyle) != 5)
	return false;
if (strnicmp(data, "*END\r", 5) != 0)
	return false;

return true;

} // ImportWizGUI::SniffMDEM

/*===========================================================================*/

bool ImportWizGUI::SniffNEDBinary(FILE *phyle, char *fullname)
{
char *ext;
FILE *test;
char testname[256+32];

strcpy(testname, noextname);
strcat(testname, ".blw");	// look for a world file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (ReadWorldFile(test) == false)
	return false;
fclose(test);
strcpy(testname, noextname);
strcat(testname, ".hdr");	// look for the header file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (!(ReadBinHeader(test)))
	return false;
fclose(test);
ext = FindFileExtension(fullname);
if ((ext == NULL) && SniffBinary(phyle))
	return true;										// file was apparently renamed
if ((stricmp(ext, "bil") == 0) && SniffBinary(phyle))	// they have the right file name
	return true;
if ((stricmp(ext, "blw") == 0) ||
	(stricmp(ext, "hdr") == 0) || 
	(stricmp(ext, "stx") == 0))							// if they choose an associated file
	{
	strcpy(fullname, noextname);						// set to correct file name
	strcat(fullname, ".bil");
	if ((test = PROJ_fopen(testname, "rb")) == NULL)
		return false;									// can't open correct file
	if (SniffBinary(test))
		{
		fclose(test);
		return true;									// correct file checks out
		} // if
	else
		return false;
	} // if
return false;

} // ImportWizGUI::SniffNEDBinary

/*===========================================================================*/

bool ImportWizGUI::SniffNEDGridFloat(FILE *phyle, char *fullname)
{
char *ext;
FILE *test;
char testname[256+32];
unsigned char bytes;

strcpy(testname, noextname);
strcat(testname, ".prj");			// look for the projection file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (ReadUSGSProjection(test) == false)
	return false;
fclose(test);
strcpy(testname, noextname);
strcat(testname, ".hdr");			// look for the header file
if ((test = PROJ_fopen(testname, "rb")) == NULL)
	return false;
if (ReadBinHeader(test) == false)
	return false;
fclose(test);
ext = FindFileExtension(fullname);
bytes = Importing->ValueBytes;		// save old value
Importing->ValueBytes = 4;			// size of float
if ((ext == NULL) && SniffBinary(phyle))
	return true;					// they have the right file name
if (ext == NULL)
	{
	Importing->ValueBytes = bytes;	// restore old value
	return false;
	} // if
if ((stricmp(ext, "prj") == 0) ||
	(stricmp(ext, "hdr") == 0))		// if they choose an associated file
	{
	strcpy(fullname, noextname);	// set to correct file name
	if ((test = PROJ_fopen(fullname, "rb")) == NULL)
		{
		Importing->ValueBytes = bytes;
		return false;				// can't open correct file
		} // if
	if (SniffBinary(test))
		{
		fclose(test);
		return true;				// correct file checks out
		} // if
	} // if
Importing->ValueBytes = bytes;
return false;

} // ImportWizGUI::SniffNEDGridFloat
//lint -restore

/*===========================================================================*/

bool ImportWizGUI::SniffNTFDTM(FILE *phyle)
{
char buffer[84];

#ifndef WCS_BUILD_VNS

return FALSE;

#else // !WCS_BUILD_VNS

rewind(phyle);
fgetline(buffer, 67+2, phyle);
if (strnicmp(buffer, "01", 2) != 0)	// the remaining string can be almost anything
	return false;
fgetline(buffer, 81+2, phyle);
if ((strnicmp(buffer, "02OS_LANDRANGER_DTM", 19) != 0) && (strnicmp(buffer, "02L-F_PROFILE_DTM", 17) != 0))
	return false;

return true;

#endif // !WCS_BUILD_VNS

} // ImportWizGUI::SniffNTFDTM

/*===========================================================================*/

bool ImportWizGUI::SniffNTFMeridian2(FILE *phyle)
{
char buffer[80];

#ifndef WCS_BUILD_VNS

return FALSE;

#else // !WCS_BUILD_VNS

rewind(phyle);
fgetline(buffer, 66+2, phyle);
if (strnicmp(buffer, "01ORDNANCE SURVEY", 17) != 0)
	return false;
fgetline(buffer, 80+2, phyle);
if (strnicmp(buffer, "02Meridian_02.00", 16) != 0)
	return false;
fgetline(buffer, 80+2, phyle);
if (strnicmp(buffer, "00Meridian_02.00", 16) != 0)
	return false;

return true;

#endif // !WCS_BUILD_VNS

} // ImportWizGUI::SniffNTFMeridian2

/*===========================================================================*/

bool ImportWizGUI::SniffPICT(FILE *phyle)
{
unsigned char data[4];

rewind(phyle);
if (fseek(phyle, 512 + 10, SEEK_SET) == 0)
	{
	if (fread(data, 1, 4, phyle) == 4)
		{
		// look for PICT v2.0 signature
		if ((data[0] == 0x00) && (data[1] == 0x11) &&
			(data[2] == 0x02) && (data[3] == 0xff))
			return true;
		} // if
	} // if
return false;

} // ImportWizGUI::SniffPICT

/*===========================================================================*/

bool ImportWizGUI::SniffSDTSDEM(FILE *phyle, char *fullname)
{
FILE *catfile;
long last8;
char catname[256+32];
char data[18];	// pad to 18

last8 = (long)(strlen(fullname) - 8);
if (last8 < 0)
	return false;	// all SDTS files are at least 8 character names!!!
if ((strnicmp(&fullname[last8], "CATD.DDF", 8)) == 0)	// the *CATD.DDF file
	{
	rewind(phyle);
	(void)fread(data, 1, 17, phyle);
	if (strncmp(data, "001762L   0600052", 17) == 0)	// original integer format
		{	// looks like SDTS - do one more check
		fseek(phyle, 42, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, phyle);
		if (strncmp(data, "CATD", 4) == 0)
			return true;
		} // if integer
	if (strncmp(data, "001682L 1 0600049", 17) == 0)	// new floating point format
		{	// looks like SDTS - do one more check
		fseek(phyle, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, phyle);
		if (strncmp(data, "CATD", 4) == 0)
			return true;
		} // if floating point
	if (strncmp(data, "001672L 1 0600049", 17) == 0)	// new floating point format2
		{	// looks like SDTS - do one more check
		fseek(phyle, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, phyle);
		if (strncmp(data, "CATD", 4) == 0)
			return true;
		} // if floating point
	} // if "CATD.DDF"
else	// some other *.DDF file, so open *CATD.DDF for testing
	{
	strcpy(&catname[0], fullname);
	memcpy(&catname[last8], "CATD.DDF", 8);
	catfile = PROJ_fopen(&catname[0], "rb");
	if (catfile == NULL)
		return false;
	(void)fread(data, 1, 17, catfile);
	if (strncmp(data, "001762L   0600052", 17) == 0)
		{	// looks like SDTS - do one more check
		fseek(catfile, 42, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, catfile);
		if (strncmp(data, "CATD", 4) == 0)
			{
			strcpy(completename, &catname[0]);
			return true;
			} // if "CATD"
		} // if integer
	if (strncmp(data, "001682L 1 0600049", 17) == 0)	// new floating point format
		{	// looks like SDTS - do one more check
		fseek(catfile, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, catfile);
		if (strncmp(data, "CATD", 4) == 0)
			{
			strcpy(completename, &catname[0]);
			return true;
			} // if "CATD"
		} // if floating point
	if (strncmp(data, "001672L 1 0600049", 17) == 0)	// new floating point format2
		{	// looks like SDTS - do one more check
		fseek(catfile, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, catfile);
		if (strncmp(data, "CATD", 4) == 0)
			{
			strcpy(completename, &catname[0]);
			return true;
			} // if "CATD"
		} // if floating point
	} // else

return FALSE;

} // ImportWizGUI::SniffSDTSDEM

/*===========================================================================*/

bool ImportWizGUI::SniffSDTSDLG(FILE *phyle, char *fullname)
{
FILE *catfile;
long last8;
char catname[256+32];
char data[18];	// pad to 18

last8 = (long)(strlen(fullname) - 8);
if (last8 < 0)
	return false;	// all SDTS files are at least 8 character names!!!
if ((strnicmp(&fullname[last8], "CATD.DDF", 8)) == 0)	// the *CATD.DDF file
	{
	rewind(phyle);
	(void)fread(data, 1, 17, phyle);
	if (strncmp(data, "001602L   0600049", 17) == 0)
		{	// looks like SDTS - do one more check
		fseek(phyle, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, phyle);
		if (strncmp(data, "CATD", 4) == 0)
			return true;
		} // if "CATD"
	} // if "CATD.DDF"
else	// some other *.DDF file, so open *CATD.DDF for testing
	{
	strcpy(&catname[0], fullname);
	memcpy(&catname[last8], "CATD.DDF", 8);
	catfile = PROJ_fopen(&catname[0], "rb");
	if (catfile == NULL)
		return false;
	(void)fread(data, 1, 17, catfile);
	if (strncmp(data, "001602L   0600049", 17) == 0)
		{	// looks like SDTS - do one more check
		fseek(catfile, 40, SEEK_SET);	// from the beginning
		(void)fread(data, 1, 4, catfile);
		if (strncmp(data, "CATD", 4) == 0)
			{
			strcpy(completename, &catname[0]);
			return true;
			} // if "CATD"
		} // if
	} // else

return false;

} // ImportWizGUI::SniffSDTSDLG

/*===========================================================================*/

bool ImportWizGUI::SniffShapefile(FILE *phyle, char *fullname)
{
char data[100], *ext;
long shape;
ULONG val;
double xmin, ymin, zmin, mmin, xmax, ymax, zmax, mmax;
FILE *shpfile;
char shpname[256+32];
#ifdef WCS_BUILD_VNS
char prjname[256+32];
#endif // WCS_BUILD_VNS
bool local = FALSE, success = FALSE;

ext = FindFileExtension(fullname);
if ((ext == NULL) || (stricmp(ext, "shp") == 0))	// if *.shp or no extension
	shpfile = phyle;	// use the file the user selected
else												// try to find a matching .shp file	
	{
	strcpy(&shpname[0], fullname);
	(void)StripExtension(shpname);
	strcat(shpname, ".shp");
	shpfile = PROJ_fopen(&shpname[0], "rb");
	if (shpfile == NULL)
		return FALSE;
	strcpy(completename, &shpname[0]);
	local = TRUE;
	} // else if

rewind(shpfile);
if (fread(data, 1, 100, shpfile) != 100)
	goto done;
memcpy(&val, &data[0], 4);
#ifdef BYTEORDER_BIGENDIAN
if (val != 0x0000270A)	// big endian 9994
	goto done;
#else
if (val != 0x0A270000)	// big endian 9994
	goto done;
#endif // BYTEORDER_BIGENDIAN
memcpy(&val, &data[4], 4);
if (val != 0)	// first of five unused, but set to 0
	goto done;
memcpy(&val, &data[8], 4);
if (val != 0)
	goto done;
memcpy(&val, &data[12], 4);
if (val != 0)
	goto done;
#ifndef WCS_BUILD_EDSS
memcpy(&val, &data[16], 4);
if (val != 0)
	goto done;
#endif // WCS_BUILD_EDSS
memcpy(&val, &data[20], 4);
if (val != 0)
	goto done;
memcpy(&val, &data[28], 4);
#ifdef BYTEORDER_LITTLEENDIAN
if (val != 0x000003E8)	// little endian 1000 {hey I'm not making this stuff up!}
	goto done;
#else
if (val != 0xE8030000)	// little endian 1000
	goto done;
#endif // BYTEORDER_LITTLEENDIAN
memcpy(&val, &data[24], 4);	// big endian filesize in # of words
#ifdef BYTEORDER_LITTLEENDIAN
SimpleEndianFlip32U(val, &val);
#endif // BYTEORDER_LITTLEENDIAN
val *= 2;	// filelength in bytes
fseek(shpfile, 0, SEEK_END);
if (ftell(shpfile) != (signed long)val)
	goto done;
memcpy(&shape, &data[32], 4);	// back to little endian mode {I kid you not}
memcpy(&xmin, &data[36], 8);
memcpy(&ymin, &data[44], 8);
memcpy(&xmax, &data[52], 8);
memcpy(&ymax, &data[60], 8);
memcpy(&zmin, &data[68], 8);
memcpy(&zmax, &data[76], 8);
memcpy(&mmin, &data[84], 8);
memcpy(&mmax, &data[92], 8);
#ifdef BYTEORDER_BIGENDIAN
SimpleEndianFlip32S(shape, &shape);
SimpleEndianFlip64(&xmin, &xmin);
SimpleEndianFlip64(&ymin, &ymin);
SimpleEndianFlip64(&xmax, &xmax);
SimpleEndianFlip64(&ymax, &ymax);
SimpleEndianFlip64(&zmin, &zmin);
SimpleEndianFlip64(&zmax, &zmax);
SimpleEndianFlip64(&mmin, &mmin);
SimpleEndianFlip64(&mmax, &mmax);
#endif // BYTEORDER_BIGENDIAN
// the shape type that exists in this file
if ((shape == WCS_ARCVIEW_SHAPETYPE_POINTZ) || (shape == WCS_ARCVIEW_SHAPETYPE_POLYLINEZ) ||
	(shape == WCS_ARCVIEW_SHAPETYPE_POLYGONZ) || (shape == WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ))
	Importing->Flags = GET_ELEV_UNITS;	// there's Z data
// set default load type
if ((shape == WCS_ARCVIEW_SHAPETYPE_POINT) || (shape == WCS_ARCVIEW_SHAPETYPE_MULTIPOINT) ||
	(shape == WCS_ARCVIEW_SHAPETYPE_POINTZ) || (shape == WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ) ||
	(shape == WCS_ARCVIEW_SHAPETYPE_POINTM) || (shape == WCS_ARCVIEW_SHAPETYPE_MULTIPOINTM))
	Importing->LoadAs = LAS_CP;
else
	Importing->LoadAs = LAS_VECT;
#ifdef WCS_BUILD_VNS
Importing->CoordSysWarn = TRUE;
#endif // WCS_BUILD_VNS
if ((xmin > xmax) || (ymin > ymax))
	goto done;
#ifdef WCS_BUILD_VNS
// WTF - doesn't ESRI read their own documentation!?!  Unused fields are supposed to be 0.0, not -DBL_MAX!
if ((zmin < -1e20) && (zmax < -1e20))
	{
	zmin = 0.0;
	zmax = 0.0;
	} // if
if ((mmin < -1e20) && (mmax < -1e20))
	{
	mmin = 0.0;
	mmax = 0.0;
	} // if
if ((shape == WCS_ARCVIEW_SHAPETYPE_POINTZ) || (shape == WCS_ARCVIEW_SHAPETYPE_POLYLINEZ) ||
	(shape == WCS_ARCVIEW_SHAPETYPE_POLYGONZ) || (shape == WCS_ARCVIEW_SHAPETYPE_MULTIPOINTZ))
	sprintf(Importing->FileInfo,
		"\r\r\n\r\r\n3D shape\r\nX range: %.1f  -  %.1f\r\nY range: %.1f  -  %.1f\r\nZ range: %.1f  -  %.1f\r\nM range: %.1f  -  %.1f",
		xmin, xmax, ymin, ymax, zmin, zmax, mmin, mmax);
else
	sprintf(Importing->FileInfo, "\r\r\n\r\r\n2D shape\r\nX range: %.1f  -  %.1f\r\nY range: %.1f  -  %.1f", xmin, xmax, ymin, ymax);
#else // WCS_BUILD_VNS
if ((xmax > 1000) || (ymax > 1000))
	{
	Importing->HasUTM = TRUE;
//#ifdef WCS_BUILD_VNS
//	Importing->IWCoordSys.SetSystem("UTM - NAD 27");
//	utmzone = RUTMZone;
//	utmzone = 11;
//	UTMZoneNum2DBCode(&utmzone);
//	Importing->IWCoordSys.SetZoneByCode(utmzone);
//#endif // WCS_BUILD_VNS
	}
#endif // WCS_BUILD_VNS

success = TRUE;

#ifdef WCS_BUILD_VNS
strcpy(prjname, noextname);
strcat(prjname, ".prj");
ReadArcPrj(prjname);	// attempt to set CoordSys via Arc projection file
#endif // WCS_BUILD_VNS

done:
if (local)
	fclose(shpfile);

return success;

} // ImportWizGUI::SniffShapefile

/*===========================================================================*/

bool ImportWizGUI::SniffSRTM(FILE *phyle, char *filename)
{
long size;
bool success = false;

rewind(phyle);
fseek(phyle, 0, SEEK_END);
size = ftell(phyle);
if (size == 25934402)		// SRTM 1 arc second
	{
	success = TRUE;
	size = 3601;
	} // if
else if (size == 2884802)	// SRTM 3 arc second
	{
	success = TRUE;
	size = 1201;
	} // else if

if (success)
	{
	// decode georeferencing
	if ((strlen(filename) == 11) &&
		((filename[0] == 'N') || (filename[0] == 'S')) && 
		isdigit(filename[1]) && isdigit(filename[2]) &&
		((filename[3] == 'E') || (filename[3] == 'W')) &&
		isdigit(filename[4]) && isdigit(filename[5]) && isdigit(filename[6]) &&
		filename[7] == '.')
		{
		double var;
		char degrees[4];

		memset(degrees, 0, sizeof(degrees));
		strncpy(degrees, &filename[1], 2);
		var = atof(degrees);
		if (filename[0] == 'S')
			var = -var;
		Importing->SBound = var;
		Importing->NBound = var + 1.0;
		memset(degrees, 0, sizeof(degrees));
		strncpy(degrees, &filename[4], 3);
		var = atof(degrees);
		if (filename[3] == 'W')
			var = -var;
		Importing->WBound = var;
		Importing->EBound = var + 1.0;
		} // if
	else
		success = FALSE;	// abort
	} // if

if (success)
	{
	INPUT_ROWS = size;
	INPUT_COLS = size;
	INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT; 
	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
	READ_ORDER = DEM_DATA_READORDER_ROWS; 
	ROWS_EQUAL = DEM_DATA_ROW_LAT;
	Importing->CoordSysWarn = FALSE;
	Importing->HasNulls = TRUE;
	Importing->NullVal = -32768.0;
	} // if

return(success);

} // ImportWizGUI::SniffSRTM

/*===========================================================================*/

bool ImportWizGUI::SniffSTM(FILE *phyle)
{
unsigned char data[4];
long width, height;
unsigned long *magic;
char ch;

rewind(phyle);
if (fread(data, 1, 4, phyle) != 4)
	return false;
if ((data[0] != 'S') || (data[1] != 'T') || (data[2] != 'M') || (data[3] != ' '))
	return false;
if (fscanf(phyle, "%ld %ld%c", &width, &height, &ch) != 3)
	return false;
if (fread(data, 1, 4, phyle) != 4)
	return false;
magic = (unsigned long *)data;
if (*magic == 0x01020304)
	INBYTE_ORDER = DEM_DATA_BYTEORDER_HILO;
else if (*magic != 0x04030201)
	return false;
else
	INBYTE_ORDER = DEM_DATA_BYTEORDER_LOHI;
ch = (char)fgetc(phyle);
if (ch != 0x0A)
	return false;
Importing->InCols = width;
Importing->InRows = height;
Importing->HdrSize = ftell(phyle);

Importing->AllowRef00 = FALSE;
Importing->AllowPosE = FALSE;
Importing->AskNull = FALSE;
INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT; 
READ_ORDER = DEM_DATA_READORDER_ROWS; 
ROWS_EQUAL = DEM_DATA_ROW_LAT;
GRID_UNITS = DEM_DATA_UNITS_METERS;
ELEV_UNITS = DEM_DATA_UNITS_METERS;
return true;

} // ImportWizGUI::SniffSTM

/*===========================================================================*/

#ifdef GO_SURFING
bool ImportWizGUI::SniffSurfer(FILE *phyle)
{
char data[4];
//char gtype;
//float flt;
//double dbl;

rewind(phyle);
if (fread(data, 1, 4, phyle) != 4)
	return FALSE;
if ((strncmp(data, "DSAA", 4) != 0) && (strncmp(data, "DSBB", 4) != 0) && (strncmp(data, "DSRB", 4) != 0))
	return FALSE;

/***
if (data[3] == 'A')			// Version 5 & prior
	gtype = 0;
else if (data[3] == 'B')	// Version 6
	gtype = 1;
else						// Version 7 & later
	gtype = 2;

switch (gtype)
	{
	default:
	case 0:
		break;
	case 1:
		break;
	case 2:
		break;
	}
***/

return TRUE;

} // ImportWizGUI::SniffSurfer
#endif // GO_SURFING

/*===========================================================================*/

bool ImportWizGUI::SniffTarga(char *fullname)
{

if (IdentImage(fullname) == WCS_BITMAPS_IDENTIMAGE_TARGA)
	return true;
else
	return false;

} // ImportWizGUI::SniffTarga

/*===========================================================================*/

bool ImportWizGUI::SniffTerragen(FILE *phyle)
{
char data[20];
float planetrad = 0.0f, x, y, z;
long foo;
bool GotSize = false, done = false;
#ifdef BYTEORDER_BIGENDIAN
short fliptmp;
#else  // !BYTEORDER_BIGENDIAN
short *value;
#endif // !BYTEORDER_BIGENDIAN

rewind(phyle);
if (fread(data, 1, 16, phyle) != 16)
	return false;
if (strncmp(&data[0], "TERRAGENTERRAIN ", 16) != 0)
	return false;

// look for known segments - ALTW must follow SIZE
do
	{
	if (fread(data, 1, 4, phyle) != 4)
		return false;
	if (strncmp(data, "SIZE", 4) == 0)
		{
		if (fread(data, 1, 4, phyle) != 4)
			return false;
		GotSize = true;
		Importing->Flags |= FOUND_COLS;
		Importing->Flags |= FOUND_ROWS;
		#ifdef BYTEORDER_BIGENDIAN
		fliptmp = *(short *)data;
		SimpleEndianFlip16S(fliptmp, &fliptmp);
		Importing->InCols = fliptmp + 1;
		Importing->InRows = fliptmp + 1;
		#else // !BYTEORDER_BIGENDIAN
		value = (short *)data;
		Importing->InCols = *value + 1;
		Importing->InRows = *value + 1;
		#endif // !BYTEORDER_BIGENDIAN
		} // if "SIZE"
	else if (strncmp(data, "XPTS", 4) == 0)
		{
		if (fread(data, 1, 4, phyle) != 4)
			return FALSE;
		Importing->Flags |= FOUND_COLS;
		#ifdef BYTEORDER_BIGENDIAN
		fliptmp = *(short *)data;
		SimpleEndianFlip16S(fliptmp, &fliptmp);
		Importing->InCols = fliptmp;
		#else // !BYTEORDER_BIGENDIAN
		value = (short *)data;
		Importing->InCols = *value;
		#endif // !BYTEORDER_BIGENDIAN
		} // else if "XPTS"
	else if (strncmp(data, "YPTS", 4) == 0)
		{
		if (fread(data, 1, 4, phyle) != 4)
			return false;
		Importing->Flags |= FOUND_ROWS;
		#ifdef BYTEORDER_BIGENDIAN
		fliptmp = *(short *)data;
		SimpleEndianFlip16S(fliptmp, &fliptmp);
		Importing->InRows = fliptmp;
		#else // !BYTEORDER_BIGENDIAN
		value = (short *)data;
		Importing->InRows = *value;
		#endif // !BYTEORDER_BIGENDIAN
		} // else if "YPTS"
	else if (strncmp(data, "ALTW", 4) == 0)
		{
		Importing->Flags |= FOUND_ALTW;
		if (!GotSize)
			return false;
		done = true;
		Importing->HdrSize = ftell(phyle);
		} // else if "ALTW"
	else if (strncmp(data, "CRAD", 4) == 0)
		{
		if (fread(&planetrad, 1, 4, phyle) != 4)
			return false;
		#ifdef BYTEORDER_BIGENDIAN
		SimpleEndianFlip32F(&planetrad, &planetrad);
		#endif // BYTEORDER_BIGENDIAN
		Importing->NewPlanetRadius = planetrad * 1000;
		} // else if "CRAD"
	else if (strncmp(data, "SCAL", 4) == 0)
		{
		if (fread(&x, 1, 4, phyle) != 4)	// x (longitude) cell size in meters
			return false;
		if (fread(&y, 1, 4, phyle) != 4)	// y (latitude) cell size in meters
			return false;
		if (fread(&z, 1, 4, phyle) != 4)	// z (elevation) cell size in meters
			return false;
		#ifdef BYTEORDER_BIGENDIAN
		SimpleEndianFlip32F(&x, &x);
		SimpleEndianFlip32F(&y, &y);
		SimpleEndianFlip32F(&z, &z);
		#endif // BYTEORDER_BIGENDIAN
		Importing->North = y;	// no real good place to store these, since planet radius may not be known yet
		Importing->West = x;
		} // else if "SCAL"
	else if (strncmp(data, "CRVM", 4) == 0)	// curve mode - not used
		{
		if (fread(&foo, 4, 1, phyle) != 1)
			return false;
		} // else if "CRVM"
	} while (!done);

#ifdef WCS_BUILD_VNS
if (planetrad == 0.0)
	{
	Importing->IWCoordSys.SetSystemByCode(8);
	}
else
	{
	Importing->IWCoordSys.SetSystemByCode(0);					// Custom
	Importing->IWCoordSys.Method.SetMethodByCode(6);			// Geographic
	Importing->IWCoordSys.Datum.SetDatumByCode(357);			// Back Compatible Sphere - really just want to init deltas to zeros
	Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(44);	// Custom
	Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMAJOR].SetValue(planetrad * 1000.0);
	Importing->IWCoordSys.Datum.Ellipse.AnimPar[WCS_EFFECTS_GEOELLIPSOID_ANIMPAR_SEMIMINOR].SetValue(planetrad * 1000.0);
	}
#endif // WCS_BUILD_VNS

return true;

} // ImportWizGUI::SniffTerragen

/*===========================================================================*/

bool ImportWizGUI::SniffUSGS_DEM(FILE *phyle)
{
int  ch;
long fpos, i, tocheck;
char data[6];

// look for valid DEM level code
if (fseek(phyle, 144, SEEK_SET) != 0)
	return false;
if (fread(data, 1, 6, phyle) != 6)
	return false;
if (!((strnicmp(data, "     1", 6) == 0) ||
	(strnicmp(data, "     2", 6) == 0) ||
	(strnicmp(data, "     3", 6) == 0)))
	return false;

// look for valid elevation pattern
if (fseek(phyle, 150, SEEK_SET) != 0)
	return false;
if (fread(data, 1, 6, phyle) != 6)
	return false;
if (!((strnicmp(data, "     1", 6) == 0) ||
	(strnicmp(data, "     2", 6) == 0)))
	return false;

// look for valid ground planimetric ref sys
if (fseek(phyle, 156, SEEK_SET) != 0)
	return false;
if (fread(data, 1, 6, phyle) != 6)
	return false;
if (!((strnicmp(data, "     0", 6) == 0) ||
	(strnicmp(data, "     1", 6) == 0) ||
	(strnicmp(data, "     2", 6) == 0)))
	return false;

// look for valid unit of measure for above
if (fseek(phyle, 528, SEEK_SET) != 0)
	return false;
if (fread(data, 1, 6, phyle) != 6)
	return false;
if (!((strnicmp(data, "     0", 6) == 0) ||
	(strnicmp(data, "     1", 6) == 0) ||
	(strnicmp(data, "     2", 6) == 0) ||
	(strnicmp(data, "     3", 6) == 0)))
	return false;

// if polysides is 4, it's a USGS DEM (or variant)
if (fseek(phyle, 540, SEEK_SET) != 0)
	return false;
if (fread(data, 1, 6, phyle) != 6)
	return false;
if (strnicmp(data, "     4", 6) != 0)
	return false;

// check for those pesky Arc variants (scan for LF to indicate end of A record)
fpos = ftell(phyle);	// this is always 546, but this is more readable this way
tocheck = 1024 - fpos;
for (i = 0; i < tocheck; i++)
	{
	ch = fgetc(phyle);
	if (ch == EOF)
		return false;
	if (ch == 0x0A)		// LF
		{
		Importing->Signal = ARC_VARIANT;
		Importing->Flags = ftell(phyle);	// hey, it's an available LONG in this situation
		return true;
		} // if
	} // for

return true;	// standard USGS variant

} // ImportWizGUI::SniffUSGS_DEM

/*===========================================================================*/

/*** F2 NOTE - This needs to be much better, but this should work for now.
     The DLG files I have don't meet the specs I have! ***/
bool ImportWizGUI::SniffUSGS_DLG(FILE *phyle)
{
char data[80];

rewind(phyle);
if (fread(data, 1, 80, phyle) != 80)
	return false;
data[79] = 0;
if (strstr(data, "DLG") == NULL)
	return false;
else
	return true;

} // ImportWizGUI::SniffUSGS_DLG

/*===========================================================================*/

bool ImportWizGUI::SniffVistaPro(FILE *phyle)
{
char str[15];

rewind(phyle);
if (fgetline(str, 15, phyle))
	{
	if (strcmp(str, "Vista DEM File") == 0)
		return true;
	}
return false;

} // ImportWizGUI::SniffVistaPro

/*===========================================================================*/

bool ImportWizGUI::SniffWCS_3Dobj(FILE *phyle)
{
char data[22];

rewind(phyle);
if ((fread(data, 1, 22, phyle) == 22) &&
	(strncmp(data, "WCS File", 8) == 0) && (strncmp(&data[14], "Object3D", 8) == 0))
	return true;
return false;

} // ImportWizGUI::SniffWCS_3Dobj

/*===========================================================================*/

bool ImportWizGUI::SniffWCS_DEM(FILE *phyle)
{
float version;
ULONG order;
char data[32];

rewind(phyle);
if (fread(data, 1, 22, phyle) != 22)
	return false;
if (strncmp(data, "WCS File", 8) == 0)	// new file format?
	{
	memcpy(&order, &data[10], 4);
	if ((order != 0xAABBCCDD) && (order != 0xDDCCBBAA))
		return false;
	if ((strncmp(&data[14], "DEMData\0", 8) != 0) && (strncmp(&data[14], "CoordSys", 8) != 0))
		return false;
	} // if
else
	{
	rewind(phyle);
	if (fread(&version, 4, 1, phyle) != 1)
		return false;
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32F(&version, &version);
	#endif // BYTEORDER_LITTLEENDIAN
	// We expect a float 1.020 or similar.  Accept any file with a 1.0xx version. 
	if (!(((version - WCS_DEM_CURVERSION) < .1) && ((version - WCS_DEM_CURVERSION) > -.1)))
		return false;
	} // else

return true;

} // ImportWizGUI::SniffWCS_DEM

/*===========================================================================*/

bool ImportWizGUI::SniffWCS_Proj(FILE *phyle)
{
char data[22];

rewind(phyle);
if ((fread(data, 1, 22, phyle) == 22) &&
	(strncmp(data, "WCS File", 8) == 0) && (strncmp(&data[14], "Paths", 5) == 0))
	return true;
else
	return false;

} // ImportWizGUI::SniffWCS_Proj

/*===========================================================================*/

bool ImportWizGUI::SniffWCS_ZBuffer(FILE *phyle)
{
char data[16];
/***
long fsize;	// technically, fsize is a ULONG, but ftell returns longs
#ifdef BYTEORDER_LITTLEENDIAN
long tmp;
#endif // BYTEORDER_LITTLEENDIAN
***/

rewind(phyle);
if (fread(data, 1, 16, phyle) != 16)
	return false;
// look for magic markers
if ((data[0] != 'F') || (data[1] != 'O') || (data[2] != 'R') || (data[3] != 'M') ||
	(data[8] != 'I') || (data[9] != 'L') || (data[10] != 'B') || (data[11] != 'M') ||
	(data[12] != 'Z') || (data[13] != 'B') || (data[14] != 'U') || (data[15] != 'F'))
	return false;

return true;

/***
// read chunksize
#ifdef BYTEORDER_LITTLEENDIAN
memcpy(&tmp, &data[4], 4);
SimpleEndianFlip32S(tmp, &fsize);
#else
memcpy(&fsize, &data[4], 4);
#endif // BYTEORDER_LITTLEENDIAN
fseek(phyle, 0, SEEK_END);
fsize += 8;	// chunksize + first 8 bytes
if (ftell(phyle) == fsize)
	return TRUE;
else
	return FALSE;
***/

} // ImportWizGUI::SniffWCS_ZBuffer

/*===========================================================================*/

// WCS's own Lon / Lat / Elev / Link file format as written by DEM Designer in prior versions
// ASCII file is in decimal degrees only, space delimited, with link being 0 or 1
bool ImportWizGUI::SniffWXYZ(FILE *phyle)
{
double x, y, elev;
long link;
double minx, miny, maxx, maxy;
int count;

rewind(phyle);
minx = miny = DBL_MAX;
maxx = maxy = -DBL_MAX;
count = fscanf(phyle, "%lf %lf %lf %ld", &x, &y, &elev, &link);
if (count != 4)
	return false;
do
	{
	if (count != 4)
		return false;
	if (x < minx)
		minx = x;
	if (x > maxx)
		maxx = x;
	if (y < miny)
		miny = y;
	if (y > maxy)
		maxy = y;
	count = fscanf(phyle, "%lf %lf %lf %ld", &x, &y, &elev, &link);
	} while (count != EOF);
return true;

} // ImportWizGUI::SniffWXYZ

/*===========================================================================*/

bool ImportWizGUI::SniffXYZ(FILE *phyle)
{
double x, y;	//, elev;
double minx, miny, maxx, maxy;
int  count, lines = 0;
char *status, *text;
char buffer[255], str1[51], str2[51], str3[51], str4[51], str5[51];

rewind(phyle);
minx = miny = DBL_MAX;
maxx = maxy = -DBL_MAX;
//count = fscanf(phyle, "%lf %lf %lf", &x, &y, &elev);
// %[^ ,\t] reads everything except space comma or tab (string is implied)
// %[ ,\t]  reads space comma or tab (string is implied)
status = fgetline(buffer, 255, phyle);
if (status == NULL)
	return false;
text = buffer;
while (isspace(*text))
	text++;
count = sscanf(text, "%[^ ,\t]%[ ,\t]%[^ ,\t]%[ ,\t]%s", str1, str2, str3, str4, str5);
if (count != 5)
	return false;
x = atof(str1); y = atof(str3);	//, elev = atof(str5);
do
	{
	lines++;
	if (x < minx)
		minx = x;
	if (x > maxx)
		maxx = x;
	if (y < miny)
		miny = y;
	if (y > maxy)
		maxy = y;
//	count = fscanf(phyle, "%lf %lf %lf", &x, &y, &elev);
	if (status = fgetline(buffer, 255, phyle))
		{
		text = buffer;
		while (isspace(*text))
			text++;
		count = sscanf(text, "%[^ ,\t]%[ ,\t]%[^ ,\t]%[ ,\t]%s", str1, str2, str3, str4, str5);
		if (count != 5)
			return FALSE;
		x = atof(str1); y = atof(str3);	//, elev = atof(str5);
		}
	} while (status && (lines < 1500)); // if it still looks like an XYZ file after 1.5k lines, say it's so
if ((maxx > 1000) || (maxy > 1000))
	{
	Importing->HasUTM = TRUE;
	Importing->Mode = IMWIZ_HORUNITS_UTM;
	}
return true;

} // ImportWizGUI::SniffXYZ

/*===========================================================================*/

bool ImportWizGUI::SniffZip(FILE *phyle)
{
char ident[4];

rewind(phyle);
if ((fread(ident, 1, 4, phyle) == 4) &&
	(ident[0] == 'P') && (ident[1] == 'K') && (ident[2] == 0x03) && (ident[3] == 0x04))
	return true;
else
	return false;

} // ImportWizGUI::SniffZip

/*===========================================================================*/

// try to find a world file, attempt to read it if found
bool ImportWizGUI::ReadWorldFiles(void)
{
static char Extension[][4] = {"BLW", "DMW", "JGW", "SDW", "TFW", "BQW", NULL};
char worldname[256+32];
short i = 0;
bool found = false;

while (*Extension[i] && !found)
	{
	strcpy(worldname, noextname);
	strcat(worldname, ".");
	strcat(worldname, Extension[i]);
	found = ReadWorldFile(worldname);
	i++;
	} // while

return found;

} // ImportWizGUI::ReadWorldFiles

/*===========================================================================*/

bool ImportWizGUI::ReadWorldFile(char *FilePath)
{
FILE *fHandle = NULL;
bool result = false;

if (fHandle = PROJ_fopen(FilePath, "r"))
	{
	result = ReadWorldFile(fHandle);
	fclose(fHandle);
	fHandle = NULL;
	} // if

return result;

} // ImportWizGUI::ReadWorldFile

/*===========================================================================*/

#ifdef WCS_BUILD_VNS

// Read .TFW family of files
bool ImportWizGUI::ReadWorldFile(FILE *tfw)
{
int err;
struct TFWdata
	{
	double xdim, rot1, rot2, ydim, xcoord, ycoord;
	} wref;

// xdim in meters, rotation angles, negative ydim in meters, UL x & y coords
err = fscanf(tfw, "%lf %lf %lf %lf %lf %lf",
			 &wref.xdim, &wref.rot1, &wref.rot2, &wref.ydim, &wref.xcoord, &wref.ycoord);
if (err == EOF)
	return false;	// what? so soon?

if (err == 6)	// we read all the above ok
	{
	if ((wref.rot1 != 0) || (wref.rot2 != 0))
		return false;	// we don't handle any rotation
	if ((wref.xdim == 0) || (wref.ydim == 0))
		return false;	// this is just plain nonsense
	Importing->Flags |= FOUND_TFWDATA;
	Importing->GridSpaceWE = wref.xdim;
	Importing->GridSpaceNS = -wref.ydim;
	Importing->NBound = wref.ycoord;
	Importing->SBound = wref.ycoord + (Importing->InRows - 1) * wref.ydim;
	Importing->WBound = wref.xcoord;
	Importing->EBound = wref.xcoord + (Importing->InCols - 1) * wref.xdim;
	return true;
	}

return false;

} // ImportWizGUI::ReadWorldFile

#else // WCS_BUILD_VNS

// Read .TFW family of files
// Currently, that means files with a BLW, DMW, JGW, SDW, or TFW extension
bool ImportWizGUI::ReadWorldFile(FILE *tfw)
{
int err, zone;
char line[128];
struct TFWdata
	{
	double xdim, rot1, rot2, ydim, xcoord, ycoord;
	} wref;

// xdim in meters, rotation angles, negative ydim in meters, UL x & y coords
err = fscanf(tfw, "%lf %lf %lf %lf %lf %lf",
			 &wref.xdim, &wref.rot1, &wref.rot2, &wref.ydim, &wref.xcoord, &wref.ycoord);
if (err == EOF)
	return FALSE;	// what? so soon?

if (err == 6)	// we read all the above ok
	{
	if ((wref.rot1 != 0) || (wref.rot2 != 0))
		return FALSE;	// we don't handle any rotation
	if ((wref.xdim == 0) || (wref.ydim == 0))
		return FALSE;	// this is just plain nonsense
	Importing->Flags |= FOUND_TFWDATA;
	Importing->GridSpaceWE = wref.xdim;
	Importing->GridSpaceNS = -wref.ydim;

/*** changed 04/17/03 
	Importing->North = wref.ycoord;
	Importing->West = wref.xcoord;
***/
	Importing->NBound = wref.ycoord;
	Importing->SBound = wref.ycoord + (Importing->InRows - 1) * wref.ydim;
	Importing->WBound = wref.xcoord;
	Importing->EBound = wref.xcoord + (Importing->InCols - 1) * wref.xdim;

	Importing->Signal = IMSIG_GRIDFLOAT;
	// look for more info just in case we have a GPSy extended file
	if (fgetline(line, 127, tfw) != NULL)
		{
		zone = atoi(line);
		if ((zone < 1) || (zone > 60))
			{
			// we won't consider this an error - somebody else may be adding some other fields
			return TRUE;
			} // if
		SetUTMZone(zone);
		if (fgetline(line, 127, tfw) != NULL)
			{
			if (strnicmp(line, "NAD27 CONUS", 11) == 0)
				SetDatum("NAD27_CONUS");
			else if (strnicmp(line, "NAD83", 5) == 0)
				SetDatum("NAD83");
			// again, no error
			return TRUE;
			} // if
		} // if
	return TRUE;
	} // if (err == 6)
else	// we didn't read the TFW correctly
	return FALSE;

} // ImportWizGUI::ReadWorldFile

#endif // WCS_BUILD_VNS

/*===========================================================================*/

// Read various header types that travel with binary files
bool ImportWizGUI::ReadBinHeader(FILE *binhdr)
{
int  err, lines = 0, tmp;
char buf[132], ident[132], val[132];
double TmpX = 0.0, TmpY = 0.0;
CoordSys *TempCS;

if (fgets(buf, 132, binhdr) == NULL)
	return false;

if ((strcmp(buf, "ENVI\x0a") == 0) || (strcmp(buf, "ENVI") == 0))
	{
	// found an ENVI header
	while (fgets(buf, 132, binhdr) && !feof(binhdr))
		{
		int v;
		unsigned long copy1, copy2;
		char *equals;

		equals = strchr(buf, '=');
		if (equals == NULL)
			return false;	// bail - somethings wrong
		copy1 = (unsigned long)(equals - buf);
		copy2 = (unsigned long)((buf + strlen(buf)) - equals);
		memset(ident, 0, 132);
		memset(val, 0, 132);
		strncpy(ident, buf, copy1);
		strncpy(val, equals + 1, copy2);
		if (strncmp(ident, "description", 11) == 0)
			{
			while ((strchr(val, '}') == NULL) && !feof(binhdr))
				fgets(val, 132, binhdr);
			} // description
		else if (strncmp(ident, "samples", 7) == 0)
			{
			Importing->InCols = atoi(val);
			} // samples
		else if (strncmp(ident, "lines", 5) == 0)
			{
			Importing->InRows = atoi(val);
			} // lines
		else if (strncmp(ident, "bands", 5) == 0)
			{
			} // bands
		else if (strncmp(ident, "header offset", 13) == 0)
			{
			Importing->HdrSize = atol(val);
			} // header offset
		else if (strncmp(ident, "file type", 9) == 0)
			{
			} // file type
		else if (strncmp(ident, "data type", 9) == 0)
			{
			v = atoi(val);

			if (v == 1)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_BYTE;
			else if ((v == 2) || (v == 12))
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_SHORT;
			else if ((v == 3) || (v == 4) || (v == 13))
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_LONG;
			else if (v == 5)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_DOUBLE;

			if ((v == 2) || (v == 3))
				Importing->ValueFmt = IMWIZ_DATA_FORMAT_SIGNEDINT;
			else if ((v == 12) || (v == 13))
				Importing->ValueFmt = IMWIZ_DATA_FORMAT_UNSIGNEDINT;
			else if ((v == 4) || (v == 5))
				Importing->ValueFmt = IMWIZ_DATA_FORMAT_FLOAT;
			} // data type
		else if (strncmp(ident, "interleave", 10) == 0)
			{
			if (strncmp(val, "bil", 3) == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BIL;
			else if (strncmp(val, "bip", 3) == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BIP;
			else if (strncmp(val, "bsq", 3) == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BSQ;
			} // interleave
		else if (strncmp(ident, "sensor type", 11) == 0)
			{
			} // sensor type
		else if (strncmp(ident, "byte order", 10) == 0)
			{
			v = atoi(val);

			if (v == 0)
				Importing->ByteOrder = DEM_DATA_BYTEORDER_LOHI;
			else if (v == 1)
				Importing->ByteOrder = DEM_DATA_BYTEORDER_HILO;
			} // byte order
		else if (strncmp(ident, "wavelength units", 16) == 0)
			{
			} // wavelength units
		else if (strncmp(ident, "band names", 10) == 0)
			{
			while ((strchr(val, '}') == NULL) && !feof(binhdr))
				fgets(val, 132, binhdr);
			} // band names
		else
			{
			// more stuff we don't know about :)
			}
		} // while
	} // if ENVI
else
	{
	if (buf[0] == '#')
		{
		HeaderType = HEADER_ARCINFO;
		lines++;
		err = fscanf(binhdr, "%s %s", ident, val);
		} //
	// presume it's an ESRI header
	// see if it's a comment
	else if (buf[0] == ';')
		{
		int ch1, ch2;

		HeaderType = HEADER_ARCVIEW;
		rewind(binhdr);
		ch1 = fgetc(binhdr);
		while (ch1 == ';')
			{
			do
				{
				ch2 = fgetc(binhdr);
				} while (ch2 != 0x0a);
			lines++;
			ch1 = fgetc(binhdr);
			} // while
		ungetc(ch1, binhdr);

		err = fscanf(binhdr, "%s %s", ident, val);
		} // if comment
	else
		{
		err = sscanf(buf, "%s %s", ident, val);
		} // else comment
	if (err == 0)
		return false;	// an error so soon?
	do
		{
		lines++;
		if (stricmp(ident, "BYTEORDER") == 0)
			{
			Importing->Flags |= FOUND_BYTEORDER;
			if (lines == 1)
				HeaderType = HEADER_GTOPO30;
			if ((stricmp(val, "M") == 0) || (stricmp(val, "MSBFIRST") == 0))
				Importing->ByteOrder = DEM_DATA_BYTEORDER_HILO;
			else if ((stricmp(val, "I") == 0) || (stricmp(val, "LSBFIRST") == 0))
				Importing->ByteOrder = DEM_DATA_BYTEORDER_LOHI;
			} // if "BYTEORDER"
		else if (stricmp(ident, "LAYOUT") == 0)
			{
			if (stricmp(val, "BIL") == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BIL;
			else if (stricmp(val, "BIP") == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BIP;
			else if (stricmp(val, "BSQ") == 0)
				Importing->Mode = IMWIZ_DATA_LAYOUT_BSQ;
			} // else if "LAYOUT"
		else if (stricmp(ident, "NROWS") == 0)
			{
			Importing->Flags |= FOUND_ROWS;
			Importing->InRows = atoi(val);
			} // else if "NROWS"
		else if (stricmp(ident, "NCOLS") == 0)
			{
			Importing->Flags |= FOUND_COLS;
			if (lines == 1)
				HeaderType = HEADER_NED;
			Importing->InCols = atoi(val);
	//		if (Importing->Signal == IMSIG_GRIDFLOAT)
	//			{
	//			Importing->South = Importing->North + (Importing->InRows - 1) * Importing->GridSpaceNS;
	//			Importing->East = Importing->West - (Importing->InCols - 1) * Importing->GridSpaceWE;
	//			Importing->NBound = Importing->North - Importing->GridSpaceNS / 2.0;
	//			Importing->SBound = Importing->South + Importing->GridSpaceNS / 2.0;
	//			Importing->WBound = Importing->West + Importing->GridSpaceWE / 2.0;
	//			Importing->EBound = Importing->East - Importing->GridSpaceWE / 2.0;
	//			}
			/*** disabled 07/06/04 F2
			if (Importing->Signal == IMSIG_GRIDFLOAT)
				{
				Importing->South = Importing->North - (Importing->InRows - 1) * Importing->GridSpaceNS;
				Importing->East = Importing->West + (Importing->InCols - 1)  * Importing->GridSpaceWE;
				Importing->NBound = Importing->North;
				Importing->SBound = Importing->South;
				Importing->WBound = Importing->West;
				Importing->EBound = Importing->East;
				}
			***/
			} // else if "NCOLS"
		else if (stricmp(ident, "NBANDS") == 0)
			{
			Importing->Bands = (short)atoi(val);
			} // else if "NBANDS"
		else if (stricmp(ident, "NBITS") == 0)
			{
			Importing->Flags |= FOUND_BITS;
			tmp = atoi(val);
			if (tmp == 8)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_BYTE;
			else if (tmp == 16)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_SHORT;
			else if (tmp == 32)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_LONG;
			else if (tmp == 64)
				Importing->ValueBytes = IMWIZ_DATA_VALSIZE_DOUBLE;
			} // else if "NBITS"
		else if (stricmp(ident, "BANDROWBYTES") == 0)
			{
			Importing->BandRowBytes = atoi(val);
			} // else if "BANDROWBYTES"
		else if (stricmp(ident, "TOTALROWBYTES") == 0)
			{
			Importing->TotalRowBytes = atoi(val);
			} // else if "TOTALROWBYTES"
		else if (stricmp(ident, "BANDGAPBYTES") == 0)
			{
			Importing->BandGapBytes = atoi(val); // supposed to be 0 for DEM's
			} // else if "BANDGAPBYTES"
		else if (strnicmp(ident, "NODATA", 6) == 0)
			{
			Importing->HasNulls = TRUE;
			Importing->NullVal = atof(val);
			} // else if "NODATA"
		else if (stricmp(ident, "ULXMAP") == 0)
			{
			Importing->Flags |= FOUND_ULX;
			TmpX = atof(val);
			Importing->West = TmpX;
			} // else if "ULXMAP"
		else if (stricmp(ident, "ULYMAP") == 0)
			{
			Importing->Flags |= FOUND_ULY;
			TmpY = atof(val);
			Importing->North = TmpY;
			} // else if "ULYMAP"
		else if (stricmp(ident, "XDIM") == 0)
			{
			Importing->Flags |= FOUND_XDIM;
			Importing->GridSpaceWE = atof(val);
			} // else if "XDIM"
		else if (stricmp(ident, "YDIM") == 0)
			{
			Importing->Flags |= FOUND_YDIM;
			Importing->GridSpaceNS = atof(val);
			if (lines == 6)	// it looks like it's a GRIDFLOAT
				{
				Importing->ValueFmt = IMWIZ_DATA_FORMAT_FLOAT;
				Importing->ValueBytes = 4;
				} // if
			if (HeaderType == HEADER_NED)
				{
				double HalfX, HalfY;

				Importing->Flags |= FOUND_CELLSIZE;
				Importing->GridSpaceNS = Importing->GridSpaceWE = atof(val);
				if (Importing->Flags & FOUND_YLL)
					{
					HalfY = Importing->GridSpaceNS * 0.5;
					Importing->SBound = TmpY + HalfY;
					Importing->NBound = TmpY - HalfY + Importing->InRows * Importing->GridSpaceNS;
					} // if
				else
					{
					Importing->SBound = TmpY;
					Importing->NBound = TmpY + Importing->InRows * Importing->GridSpaceNS;
					} // else
				if (Importing->Flags & FOUND_XLL)
					{
					HalfX = Importing->GridSpaceWE * 0.5;
					Importing->WBound = TmpX + HalfX;
					Importing->EBound = TmpX - HalfX + Importing->InCols * Importing->GridSpaceWE;
					} // if
				else
					{
					Importing->WBound = TmpX;
					Importing->EBound = TmpX + Importing->InCols * Importing->GridSpaceWE;
					} // else
				} // if
			/*** disabled 07/06/04 F2
			if (HeaderType == HEADER_GTOPO30)
				{
	//			Importing->North = TmpY;
	//			Importing->South = Importing->North - Importing->GridSpaceNS * (Importing->InRows - 1);
	//			Importing->West = -TmpX;
	//			Importing->East = -(TmpX + Importing->GridSpaceWE * (Importing->InCols - 1));
	//			Importing->NBound = TmpY + Importing->GridSpaceNS / 2.0;
	//			Importing->SBound =
	//				Importing->North - Importing->GridSpaceNS * Importing->InRows + Importing->GridSpaceNS / 2.0;
	//			Importing->WBound = -(TmpX - Importing->GridSpaceWE / 2.0);
	//			Importing->EBound = -(TmpX + Importing->GridSpaceWE * Importing->InCols - Importing->GridSpaceWE / 2.0);
				Importing->NBound = Importing->North = TmpY;
				Importing->SBound = Importing->South = Importing->North - Importing->GridSpaceNS * (Importing->InRows - 1);
				Importing->WBound = Importing->West = TmpX;
				Importing->EBound = Importing->East = TmpX + Importing->GridSpaceWE * (Importing->InCols - 1);
				}
			***/
			} // else if "YDIM"
		else if (stricmp(ident, "xllcorner") == 0)
			{
			Importing->Flags |= FOUND_XLL;
			TmpX = atof(val);
			} // else if "xllcorner"
		else if (stricmp(ident, "yllcorner") == 0)
			{
			Importing->Flags |= FOUND_YLL;
			TmpY = atof(val);
			} // else if "yllcorner"
		/*** F2 NOTE: account for these too ***/
		// xllcenter [=xllcorner+cellsize/2])
		// yllcenter [=yllcorner+cellsize/2])
		else if (stricmp(ident, "cellsize") == 0)
			{
			if (lines == 5)	// it looks like it's a GRIDFLOAT
				{
				Importing->ValueFmt = IMWIZ_DATA_FORMAT_FLOAT;
				Importing->ValueBytes = 4;
				} // if
			if (HeaderType == HEADER_NED)
				{
				Importing->Flags |= FOUND_CELLSIZE;
				Importing->GridSpaceNS = Importing->GridSpaceWE = atof(val);
				Importing->SBound = TmpY;
				Importing->NBound = TmpY + Importing->InRows * Importing->GridSpaceNS;
				Importing->WBound = TmpX;
				Importing->EBound = TmpX + Importing->InCols * Importing->GridSpaceWE;
				} // if
			} // else if "cellsize"
		else if (stricmp(ident, "skipbytes") == 0)
			{
			Importing->HdrSize = atol(val);
			} // else if "skipbytes"
		//else if (stricmp(ident, "MAPUNITS") == 0)
		//	{
		//	} // else if "mapunits"
		else if (stricmp(ident, "PROJ_ID") == 0)
			{
			if (stricmp(val, "\x22Universal") == 0)
				{
				err = fscanf(binhdr, "%s %s", ident, val);
				if ((err != EOF) && (err != 0) && (stricmp(ident, "Transverse") == 0) && (stricmp(val, "Mercator\x22") == 0))
					{
					err = fscanf(binhdr, "%s %s", ident, val);
					if ((err != EOF) && (err != 0) && (stricmp(ident, "PROJ_ZONE") == 0))
						{
						long zone;

						zone = atoi(val);
						if ((zone != 0) && (zone >= -60) && (zone <= 60))
							{
							if (TempCS = new CoordSys)
								{
								TempCS->SetSystemByCode(10);	// UTM - WGS 84
								UTMZoneNum2DBCode(&zone);
								TempCS->SetZoneByCode(zone);
								CopyCoordSys(TempCS);
								//TempCS->Copy(&Importing->IWCoordSys, TempCS);
								delete TempCS;
								TempCS = NULL;
								} // if TempCS
							} // if zone
						} // if err
					} // if err
				} // if UTM
			} // else if "PROJ_ID"
		err = fscanf(binhdr, "%s %s", ident, val);
		} while ((err != EOF) && (err != 0));
	if ((HeaderType == HEADER_ARCVIEW) || (HeaderType == HEADER_ARCINFO))
		{
		Importing->Flags |= FOUND_CELLSIZE;
		Importing->NBound = Importing->North = TmpY;
		Importing->SBound = Importing->South = Importing->North - Importing->GridSpaceNS * (Importing->InRows - 1);
		Importing->WBound = Importing->West = TmpX;
		Importing->EBound = Importing->East = TmpX + Importing->GridSpaceWE * (Importing->InCols - 1);
		} // if ArcView
	// moved here 07/06/04 F2
	else if (HeaderType == HEADER_GTOPO30)
		{
		Importing->NBound = Importing->North = TmpY;
		Importing->SBound = Importing->South = Importing->North - Importing->GridSpaceNS * (Importing->InRows - 1);
		Importing->WBound = Importing->West = TmpX;
		Importing->EBound = Importing->East = TmpX + Importing->GridSpaceWE * (Importing->InCols - 1);
		} // else if HEADER_GTOPO30
	else if (Importing->Signal == IMSIG_GRIDFLOAT)
		{
		Importing->South = Importing->North - (Importing->InRows - 1) * Importing->GridSpaceNS;
		Importing->East = Importing->West + (Importing->InCols - 1)  * Importing->GridSpaceWE;
		Importing->NBound = Importing->North;
		Importing->SBound = Importing->South;
		Importing->WBound = Importing->West;
		Importing->EBound = Importing->East;
		} // else if IMSIG_GRIDFLOAT

	} // else ESRI

return true;

} // ImportWizGUI::ReadBinHeader

/*===========================================================================*/

#ifdef WCS_BUILD_VNS
void ImportWizGUI::ReadArcPrj(char *PathAndName)
{
CoordSys *TempCS;
CSLoadInfo *CSLI = NULL;
char *ElevStr, *UnitStr;

if (TempCS = new CoordSys)
	{
	if (CSLI = TempCS->LoadFromArcPrj(PathAndName))
		{
		CopyCoordSys(TempCS);
		//TempCS->Copy(&Importing->IWCoordSys, TempCS);
		if (ElevStr = CSLI->FindEntry(WCS_COORDSYS_SLOT_ZUNITS))
			{
			if (stricmp(ElevStr, "FEET") == 0)
				SetVScale(6);	// US foot!!!
			else if (stricmp(ElevStr, "CENTIMETER") == 0)
				SetVScale(3);
			else if (stricmp(ElevStr, "DECIMETER") == 0)
				SetVScale(2);
			else
				SetVScale(1);	// else use meters
			} // if
		if (UnitStr = CSLI->FindEntry(WCS_COORDSYS_SLOT_UNITS))
			{
			if (stricmp(UnitStr, "FEET") == 0)
				SetHScale(6);	// US foot!!!
			else if (stricmp(UnitStr, "CENTIMETER") == 0)
				SetHScale(3);
			else if (stricmp(UnitStr, "DECIMETER") == 0)
				SetHScale(2);
			else
				SetHScale(1);	// else use meters
			} // if
		Importing->CoordSysWarn = FALSE;
		delete CSLI;
		CSLI = NULL;
		} // if
	delete TempCS;
	TempCS = NULL;
	} // if

} // ImportWizGUI::ReadArcPrj
#endif // WCS_BUILD_VNS

/*===========================================================================*/

#ifdef WCS_BUILD_VNS
void ImportWizGUI::ReadArcPrj(FILE *input)
{
CoordSys *TempCS;
CSLoadInfo *CSLI = NULL;
char *ElevStr, *UnitStr;

if (TempCS = new CoordSys)
	{
	if (CSLI = TempCS->LoadFromArcPrj(input))
		{
		CopyCoordSys(TempCS);
		//TempCS->Copy(&Importing->IWCoordSys, TempCS);
		if (ElevStr = CSLI->FindEntry(WCS_COORDSYS_SLOT_ZUNITS))
			{
			if (stricmp(ElevStr, "FEET") == 0)
				SetVScale(5);
			else if (stricmp(ElevStr, "CENTIMETER") == 0)
				SetVScale(3);
			else if (stricmp(ElevStr, "DECIMETER") == 0)
				SetVScale(2);
			else
				SetVScale(1);	// else use meters
			} // if
		if (UnitStr = CSLI->FindEntry(WCS_COORDSYS_SLOT_UNITS))
			{
			// changes made 092705 due to new type of PRJ file found in the wild
			if ((stricmp(UnitStr, "FEET") == 0) || (stricmp(UnitStr, "3.28084") == 0))
				SetHScale(5);
			else if (stricmp(UnitStr, "CENTIMETER") == 0)
				SetHScale(3);
			else if (stricmp(UnitStr, "DECIMETER") == 0)
				SetHScale(2);
			else
				SetHScale(1);	// else use meters
			} // if
		Importing->CoordSysWarn = FALSE;
		delete CSLI;
		CSLI = NULL;
		} // if
	delete TempCS;
	TempCS = NULL;
	} // if

} // ImportWizGUI::ReadArcPrj
#endif // WCS_BUILD_VNS

/*===========================================================================*/

// Projection file that goes with GTOPO30 or NED BIL/FP
bool ImportWizGUI::ReadUSGSProjection(FILE *usgsproj)
{
char line[128];
char seps[] = " \t\n";
char *token;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Projection") != 0)
	return false;
token = strtok(NULL, seps);
if (stricmp(token, "GEOGRAPHIC") != 0)
	return false;
Importing->ProjectionFlags |= PROJECTION_FLAG_PROJECTION;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Datum") != 0)
	return false;
token = strtok(NULL, seps);
if (stricmp(token, "NAD27") == 0)	// Guessed string
	SetDatum("NAD27_CONUS");
else if (stricmp(token, "WGS84") == 0)
	SetDatum("WGS84");
else if (stricmp(token, "NAD83") == 0)
	SetDatum("NAD83");
else
	return false;
Importing->ProjectionFlags |= PROJECTION_FLAG_DATUM;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Zunits") != 0)
	return false;
token = strtok(NULL, seps);
if (stricmp(token, "METERS") == 0)
	ELEV_UNITS = DEM_DATA_UNITS_METERS;
else if (stricmp(token, "FEET") == 0)
	ELEV_UNITS = DEM_DATA_UNITS_FEET;
else if (stricmp(token, "NO") == 0)		// haven't a clue as to why they do this...
	ELEV_UNITS = DEM_DATA_UNITS_METERS;	// guessing default back to meters
else
	return false;
Importing->ProjectionFlags |= PROJECTION_FLAG_ZUNITS;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Units") != 0)
	return false;
token = strtok(NULL, seps);
if (stricmp(token, "DD") != 0)
	return false;
Importing->ProjectionFlags |= PROJECTION_FLAG_ZUNITS;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Spheroid") != 0)
	return false;
token = strtok(NULL, seps);
if (stricmp(token, "WGS84") == 0)
	{
	#ifdef WCS_BUILD_VNS
	Importing->IWCoordSys.SetSystemByCode(12);	// Geographic - WGS84
//	Importing->IWCoordSys.Datum.SetDatumByCode(352);
	#endif // WCS_BUILD_VNS
	}
else if (stricmp(token, "GRS1980") == 0)
	{
	#ifdef WCS_BUILD_VNS
	Importing->IWCoordSys.SetSystemByCode(11);	// Geographic - NAD83
//	Importing->IWCoordSys.Datum.SetDatumByCode(231);	// assume NAD83
//	Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(27);
	#endif // WCS_BUILD_VNS
	}
else if (stricmp(token, "CLARKE1866") == 0)
	{
	#ifdef WCS_BUILD_VNS
	Importing->IWCoordSys.SetSystemByCode(13);	// Geographic - NAD27
//	Importing->IWCoordSys.Datum.SetDatumByCode(220);	// assume NAD27
//	Importing->IWCoordSys.Datum.Ellipse.SetEllipsoidByCode(8);
	#endif // WCS_BUILD_VNS
	}
else
	return false;
Importing->ProjectionFlags |= PROJECTION_FLAG_SPHEROID;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Xshift") != 0)
	return false;
token = strtok(NULL, seps);
if (token == NULL)
	return false;
XShift = atof(token);
Importing->ProjectionFlags |= PROJECTION_FLAG_XSHIFT;

if (!(fgetline(line, 128, usgsproj)))
	return false;
token = strtok(line, seps);
if (stricmp(token, "Yshift") != 0)
	return false;
token = strtok(NULL, seps);
if (token == NULL)
	return false;
YShift = atof(token);
Importing->ProjectionFlags |= PROJECTION_FLAG_YSHIFT;

return true;

} // ImportWizGUI::ReadUSGSProjection

/*===========================================================================*/

void ImportWizGUI::SetDatum(char *name)
{

// nothing useful happens unless we're running VNS
#ifdef WCS_BUILD_VNS
// thse aren't quite correct yet, since the USGS projection file is apparently showing what they were previously
//if (stricmp(name, "NAD27_CONUS") == 0)
//	{
//	Importing->IWCoordSys.SetSystem("UTM - NAD 27");
//	}
//else if (stricmp(name, "NAD83") == 0)
//	{
//	Importing->IWCoordSys.SetSystem("UTM - NAD 83");
//	}
//else if (stricmp(name, "WGS84") == 0)
//	{
//	Importing->IWCoordSys.Set???(???);
//	}
#endif // WCS_BUILD_VNS

} // ImportWizGUI::SetDatum

/*===========================================================================*/

void ImportWizGUI::SetUTMZone(int zone)
{

Importing->UTMZone = (short)zone;

} // ImportWizGUI::SetUTMZone

/*===========================================================================*/

char *ImportWizGUI::GuessBetter(void)
{
FILE *exists;
size_t exten;
char lookname[256+32];

strcpy(lookname, noextname);
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".bil");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".bip");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".bsq");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".dem");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".shp");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;
strcpy(lookname, noextname);
strcat(lookname, ".dxf");
if (exists = PROJ_fopen(lookname, "rb"))
	goto FoundAlt;

return NULL;

FoundAlt:

fclose(exists);
strcpy(completename, lookname);
exten = strlen(noextname);
return &completename[exten];

} // ImportWizGUI::GuessBetter

/*===========================================================================*/
