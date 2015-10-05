// DEMCore.cpp
// The very essential pieces necessary to load a WCS/VNS ELEV format DEM file

#include "stdafx.h"

#include "zlib.h"
#include "UsefulIO.h"

#ifdef BUILD_LIB // building as library pulls less headers
#include "AppMemStub.h"
#include "DEMCore.h"
#include "UsefulEndian.h"
#include "UsefulMath.h"
#include "UsefulArray.h"
#include "Types.h"
#include "ProjectIODetail.h" // for ProjectIODetailSearchStandAlone()
#else // !BUILD_LIB 
#include "DEM.h" // DEM.h will pull DEMCore.h where needed
#include "AppMem.h"
#include "Requester.h"
#include "Application.h"
#include "Project.h"
#endif // !BUILD_LIB 

#ifdef DEM_TRACE
#define CRUMB(x) x
#else
#define CRUMB(x)
#endif

DEM::DEM()
{

Flags = NULL;
RelElMap = NULL;
RawMap = NULL;
FractalMap = NULL;
#ifndef BUILD_LIB
Vertices = NULL;
MoJoe = NULL;
#endif // BUILD_LIB
XIndex = YIndex = NULL;
ScreenWidth = ScreenHeight = 0;
RawCount = 0;
pNorthWest.Lat = pSouthEast.Lat = pNorthWest.Lon = pSouthEast.Lon = 0.0;
pLatEntries = pLonEntries = pElSamples = pRelElSamples = NULL;
pDSLatEntries = pDSLonEntries = pDSSamples = NULL;
pElMaxEl = pElMinEl = pRelElMaxEl = pRelElMinEl = 0.0f;
pElSumElDif = pElSumElDifSq = pRelElSumElDif = pRelElSumElDifSq = pFileVersion = 0.0f;
pLatStep = pLonStep = pElScale = pElDatum = pRelElScale = pRelElDatum = 0.0;
Lr = Lc = 0;
pNullValue = -9999.f;
NullReject = 0;
#ifndef BUILD_LIB
InvLatStep = InvLonStep = 0.0;
#endif // !BUILD_LIB
RelativeEffectSubtract = RelativeEffectAdd = 0.0;
AbsoluteEffectMax = -FLT_MAX;
AbsoluteEffectMin = FLT_MAX;

#ifdef WCS_VECPOLY_EFFECTS
VPData = NULL;
LastTouchedTime = 0;
#endif // WCS_VECPOLY_EFFECTS

} // DEM::DEM

/*===========================================================================*/

DEM::~DEM()
{

#ifndef BUILD_LIB
FreeScreenData();
#endif // BUILD_LIB

FreeRawElevs();

} // DEM::~DEM

/*===========================================================================*/

void DEM::SetNullPointers(void)
{

RelElMap = NULL;
RawMap = NULL;
FractalMap = NULL;

} // DEM::SetNullPointers

/*===========================================================================*/

void DEM::FreeRawElevs(void)
{

FreeRawMap();
#ifndef BUILD_LIB
FreeFractalMap();
FreeRelElMap();
FreeVertices();
#ifdef WCS_VECPOLY_EFFECTS
FreeVPData(false);
#endif // WCS_VECPOLY_EFFECTS
#endif // BUILD_LIB
pFileVersion = 0.0f;
NullReject = 0;

} // DEM::FreeRawElevs

/*===========================================================================*/

void DEM::FreeRawMap(void)
{

if (RawMap)
	{
	AppMem_Free(RawMap, RawMapSize());
	RawMap = NULL;
	} // if

} // DEM::FreeRawMap

/*===========================================================================*/

short DEM::AllocRawMap(void)
{

if (RawMap = (float *)AppMem_Alloc(RawMapSize(), APPMEM_CLEAR))
	return (1);

return (0);

} // DEM::AllocRawMap

/*===========================================================================*/

short DEM::FindElMaxMin(void)
{
unsigned long Zip;
long FEMMLr, FEMMLc, LastCol;
float ElDif;

if (RawMap == NULL)
	return (0);

pElMaxEl = -1000000.0f;
pElMinEl = 1000000.0f;
pElSamples = 0;
pElSumElDifSq = pElSumElDif = 0.0f;
LastCol = pLatEntries - 1;

for (FEMMLr = 0, Zip = 0; FEMMLr < (long)pLonEntries; FEMMLr ++)
	{
	for (FEMMLc = 0; FEMMLc < (long)pLatEntries; FEMMLc ++, Zip ++)
		{
		if (! NullReject || RawMap[Zip] != pNullValue)
			{
			if (RawMap[Zip] > pElMaxEl)
				{
				pElMaxEl = RawMap[Zip];
				} // if 
			if (RawMap[Zip] < pElMinEl)
				{
				pElMinEl = RawMap[Zip];
				} // else if 
			if (FEMMLr != 0 && FEMMLc != LastCol)
				{
				if (! NullReject || RawMap[Zip - LastCol] != pNullValue)
					{
					ElDif = (float)fabs(RawMap[Zip] - RawMap[Zip - LastCol]);
					pElSumElDif += ElDif;
					pElSumElDifSq += (ElDif * ElDif);
					pElSamples ++;
					} // if
				} // if not first row 
			} // if
		} // for FEMMLc
	} // for FEMMLr

// V5 expects MinEl & MaxEl to be in meters
pElMaxEl = (float)(pElMaxEl * pElScale * 1000.);
pElMinEl = (float)(pElMinEl * pElScale * 1000.);

return (1);

} // DEM::FindElMaxMin

/*===========================================================================*/

int DEM::CalcPolys(void)
{

return(2 * (LonEntries() - 1) * (LatEntries() - 1));

} // DEM::CalcPolys

/*===========================================================================*/

unsigned long DEM::LoadDEM(FILE *ffile, short LoadRelEl, CoordSys **MyCoords, struct ProjectIODetail *Detail)
{
unsigned long ByteOrder;
unsigned long Size, BytesRead, TotalRead = 0;
short ReadError = 0, ByteFlip;
unsigned char Version, Revision;
char Title[12], ReadBuf[32];

CRUMB(UserMessageOK("DEM Debug", "FreeRawElevs");)
FreeRawElevs();

fread((char *)ReadBuf, 14, 1, ffile);

CRUMB(UserMessageOK("DEM Debug", "MemCopies");)
memcpy(Title, &ReadBuf[0], 8);
memcpy(&Version, &ReadBuf[8], 1);
memcpy(&Revision, &ReadBuf[9], 1);
memcpy(&ByteOrder, &ReadBuf[10], 4);

ByteFlip = 0;
if (ByteOrder == 0xaabbccdd)
	ByteFlip = 0;
else if (ByteOrder == 0xddccbbaa)
	ByteFlip = 1;
else
	return (0);

Title[11] = '\0';
if (strnicmp(Title, "WCS File", 8))
	{
	return (0);
	} // if

TotalRead = BytesRead = 14;

CRUMB(UserMessageOK("DEM Debug", "while BytesRead");)
while (BytesRead)
	{
	// read block descriptor tag from file 
	CRUMB(UserMessageOK("DEM Debug", "block desc tag");)
	if (BytesRead = ReadBlock(ffile, (char *)ReadBuf,
		WCS_BLOCKTYPE_CHAR + WCS_BLOCKSIZE_DOUBLE, ByteFlip))
		{
		TotalRead += BytesRead;
		ReadBuf[8] = 0;
		// read block size from file 
		CRUMB(UserMessageOK("DEM Debug", "block size");)
		if (BytesRead = ReadBlock(ffile, (char *)&Size,
			WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
			{
			TotalRead += BytesRead;
			BytesRead = 0;
			//LocalDetail = NULL;
			#ifdef BUILD_LIB
			// use ProjectIODetailSearchStandAlone instead of stub in Project class
			if (! Detail || (/*LocalDetail = */ProjectIODetailSearchStandAlone(Detail, ReadBuf)))
			#else // !BUILD_LIB
			if (! Detail || (/*LocalDetail = */GlobalApp->MainProj->ProjectIODetailSearch(Detail, ReadBuf)))
			#endif // !BUILD_LIB
				{
				if (! strnicmp(ReadBuf, "CoordSys", 8))
					{
					CRUMB(UserMessageOK("DEM Debug", "Load Coords");)
					#ifdef BUILD_LIB
					// <<<>>> skip CoordSys for now
					if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					#else // !BUILD_LIB
					if (MyCoords && (*MyCoords = new CoordSys()))
						{
						BytesRead = (*MyCoords)->Load(ffile, Size, ByteFlip, false);
						} // if
					else if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
					#endif // BUILD_LIB
					} // if
				else if (! strnicmp(ReadBuf, "DEMData", 8))
					{
					CRUMB(UserMessageOK("DEM Debug", "Load");)
					BytesRead = Load(ffile, ByteFlip, LoadRelEl);
					} // if
				else if (! fseek(ffile, Size, SEEK_CUR))
					BytesRead = Size;
				}
			else if (! fseek(ffile, Size, SEEK_CUR))
				BytesRead = Size;
			TotalRead += BytesRead;
			if (BytesRead != Size)
				{
				ReadError = 1;
				break;
				} // if error
			} // if size block read 
		else
			break;
		} // if tag block read 
	else
		break;
	} // while 

if (! ReadError)
	{
	return (TotalRead);
	} // if no error
else
	{
	#ifndef BUILD_LIB
	UserMessageOK("DEM: Load", "A chunk size error was detected in loading a DEM file. Not all data was read correctly.");
	#endif // !BUILD_LIB
	} // else

return (0);

} // DEM::LoadDEM

/*===========================================================================*/

unsigned long DEM::Load(FILE *ffile, short ByteFlip, short LoadRelEl)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;
char DEMName[256];

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	CRUMB(UserMessageOK("DEM Debug", "3 block tag");)
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			CRUMB(UserMessageOK("DEM Debug", "3 block size");)
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_DEMDATA_NAME:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEMDATA_NAME");)
						BytesRead = ReadBlock(ffile, (char *)DEMName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DEMDATA_COVERAGE:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEMDATA_COVERAGE");)
						BytesRead = Coverage_Load(ffile, ByteFlip, LoadRelEl);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

return (TotalRead);

} // DEM::Load

/*===========================================================================*/

unsigned long DEM::Coverage_Load(FILE *ffile, short ByteFlip, short LoadRelEl)
{
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	CRUMB(UserMessageOK("DEM Debug", "CL block tag");)
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			CRUMB(UserMessageOK("DEM Debug", "CL block size");)
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_DEM_COVERAGE_ROWS:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_ROWS");)
						BytesRead = ReadBlock(ffile, (char *)&pLonEntries, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_COLUMNS:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_COLUMNS");)
						BytesRead = ReadBlock(ffile, (char *)&pLatEntries, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_NWLAT:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_NWLAT");)
						BytesRead = ReadBlock(ffile, (char *)&pNorthWest.Lat, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_NWLON:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_NWLON");)
						BytesRead = ReadBlock(ffile, (char *)&pNorthWest.Lon, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_SELAT:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_SELAT");)
						BytesRead = ReadBlock(ffile, (char *)&pSouthEast.Lat, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_SELON:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_SELON");)
						BytesRead = ReadBlock(ffile, (char *)&pSouthEast.Lon, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_LATSTEP:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_LATSTEP");)
						BytesRead = ReadBlock(ffile, (char *)&pLatStep, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_LONSTEP:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_LONSTEP");)
						BytesRead = ReadBlock(ffile, (char *)&pLonStep, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						#ifdef BUILD_LIB
						// 01/13/04 CXH:
						// Added code to detect improper negative LonStep values in Geographic DEMs written by SX
						// and fabs() them. This code will only be built into NVE and other 3DNDEM.lib users, as
						// WCS/VNS are not likely to ever need to read these old DEMs.
						
						// 3DNDEM.lib users can't read projection data from DEMs yet anyway. Assume all are geographic,
						// and update this when we later add projection-reading ability to 3DNDEM.lib
						//if(!Projected)
						if(1)
							{
							pLonStep = fabs(pLonStep);
							} // if
						#endif // BUILD_LIB
						break;
						}
					case WCS_DEM_COVERAGE_ELEVATION:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_ELEVATION");)
						BytesRead = Elevation_Load(ffile, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_RELEL:
						{
						CRUMB(UserMessageOK("DEM Debug", "CL WCS_DEM_COVERAGE_RELEL");)
#ifdef BUILD_LIB
// skip over any Relel -- shouldn't be there anyway
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
#else // BUILD_LIB
						if (LoadRelEl)
							BytesRead = RelEl_Load(ffile, ByteFlip);
						else if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
#endif // BUILD_LIB
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
		else
			break;
	} // while 

// precalculate some common factors that we don't need to store
PrecalculateCommonFactors();

return (TotalRead);

} // DEM::Coverage_Load

/*===========================================================================*/

unsigned long DEM::Elevation_Load(FILE *ffile, short ByteFlip)
{
int zlib_result;
unsigned long ItemTag = 0, Size, BytesRead, TotalRead = 0, Ct, MaxCt = MapSize();
short *TempMap;
unsigned char *CompBuf;
union MultiVal MV;
char Precision = -1, Compression = -1;
CRUMB(char debug[40];)

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	CRUMB(UserMessageOK("DEM Debug", "EL block tag");)
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
			CRUMB(UserMessageOK("DEM Debug", "EL block size");)
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_DEM_COVERAGE_DATA_PRECISION:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_PRECISION");)
						BytesRead = ReadBlock(ffile, (char *)&Precision, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_COMPRESSION:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_COMPRESSION");)
						BytesRead = ReadBlock(ffile, (char *)&Compression, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_DATUM:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_DATUM");)
						BytesRead = ReadBlock(ffile, (char *)&pElDatum, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_ELSCALE:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_ELSCALE");)
						BytesRead = ReadBlock(ffile, (char *)&pElScale, WCS_BLOCKTYPE_DOUBLE + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_MAXEL:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_MAXEL");)
						BytesRead = ReadBlock(ffile, (char *)&pElMaxEl, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_MINEL:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_MINEL");)
						BytesRead = ReadBlock(ffile, (char *)&pElMinEl, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SAMPLES:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_SAMPLES");)
						BytesRead = ReadBlock(ffile, (char *)&pElSamples, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SUMELDIF:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_SUMELDIF");)
						BytesRead = ReadBlock(ffile, (char *)&pElSumElDif, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_SUMELDIFSQ:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_SUMELDIFSQ");)
						BytesRead = ReadBlock(ffile, (char *)&pElSumElDifSq, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_NULLREJECT:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_NULLREJECT");)
						BytesRead = ReadBlock(ffile, (char *)&NullReject, WCS_BLOCKTYPE_SHORTINT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_NULLVALUE:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_NULLVALUE");)
						BytesRead = ReadBlock(ffile, (char *)&pNullValue, WCS_BLOCKTYPE_FLOAT + Size, ByteFlip);
						break;
						}
					case WCS_DEM_COVERAGE_DATA_DATABLOCK:
						{
						CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_COVERAGE_DATA_DATABLOCK");)
						if (Compression == WCS_DEM_COMPRESSION_ZLIB)
							{
							CRUMB(UserMessageOK("DEM Debug", "Zlib Compression");)
							// only WCS_DEM_PRECISION_FLOAT is supported here
							if (RawMap = (float *)AppMem_Alloc(MapSize() * sizeof(float), 0))
								{
								if (CompBuf = (unsigned char *)AppMem_Alloc(Size, APPMEM_CLEAR))
									{
									if (BytesRead = ReadLongBlock(ffile, (char *)CompBuf, Size))
										{
										unsigned long OutSize = MapSize() * sizeof(float);

										zlib_result = uncompress((unsigned char *)RawMap, &OutSize, CompBuf, Size);
										if (zlib_result != Z_OK)
											{
											CRUMB(UserMessageOK("DEM Debug", "Decompression Failure");)
											FreeRawElevs();
											} // else read error
										else
											{
											if (ByteFlip)
												{
												for (Ct = 0; Ct < MaxCt; Ct ++)
													{
													SimpleEndianFlip32F(&RawMap[Ct], (float *)&RawMap[Ct]);
													} // for
												} // if
											} // else
										} // if
									else
										{
										CRUMB(UserMessageOK("DEM Debug", "Decompression Buffer Allocation Failure");)
										FreeRawElevs();
										} // else read error
									AppMem_Free(CompBuf, Size);
									CompBuf = NULL;
									} //if
								} // if
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // zlib compression
						else
							{
							if (Precision == WCS_DEM_PRECISION_FLOAT)
								{
								CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_PRECISION_FLOAT");)
								if (RawMap = (float *)AppMem_Alloc(MapSize() * sizeof (float), 0))
									{
									CRUMB(UserMessageOK("DEM Debug", "EL ReadLongBlock");)
									if (BytesRead = ReadLongBlock(ffile, (char *)RawMap, Size))
										{
										if (ByteFlip)
											{
											for (Ct = 0; Ct < MaxCt; Ct ++)
												{
												SimpleEndianFlip32F(&RawMap[Ct], (float *)&RawMap[Ct]);
												} // for
											} // if
										} // if
									else
										{
										CRUMB(UserMessageOK("DEM Debug", "EL FreeRawElevs");)
										FreeRawElevs();
										} // else read error
									} // if memory
								else if (! fseek(ffile, Size, SEEK_CUR))
									BytesRead = Size;
								} // if float
							else if (Precision == WCS_DEM_PRECISION_SHORT)
								{
								CRUMB(UserMessageOK("DEM Debug", "WCS_DEM_PRECISION_SHORT");)
								CRUMB(sprintf(debug, "MapSize() = %d", MapSize());)
								CRUMB(UserMessageOK("DEM Debug", debug);)
								if (TempMap = (short *)AppMem_Alloc(MapSize() * sizeof (short), 0))
									{
									if (RawMap = (float *)AppMem_Alloc(MapSize() * sizeof (float), 0))
										{
										CRUMB(UserMessageOK("DEM Debug", "EL ReadLongBlock");)
										CRUMB(sprintf(debug, "Size() = %d", Size);)
										CRUMB(UserMessageOK("DEM Debug", debug);)
										if (BytesRead = ReadLongBlock(ffile, (char *)TempMap, Size))
											{
											if (ByteFlip)
												{
												for (Ct = 0; Ct < MaxCt; Ct ++)
													{
													SimpleEndianFlip16S((short)TempMap[Ct], (short *)&TempMap[Ct]);
													} // for
												} // if
											for (Ct = 0; Ct < MaxCt; Ct ++)
												{
												RawMap[Ct] = (float)TempMap[Ct];
												} // for
											} // if
										else
											{
											CRUMB(UserMessageOK("DEM Debug", "EL FreeRawElevs");)
											FreeRawElevs();
											} // else read error
										} // if rawmap memory
									else if (! fseek(ffile, Size, SEEK_CUR))
										BytesRead = Size;
									CRUMB(UserMessageOK("DEM Debug", "EL AppMem_Free");)
									AppMem_Free(TempMap, MapSize() * sizeof (short));
									TempMap = NULL;
									} // if temp memory
								else if (! fseek(ffile, Size, SEEK_CUR))
									BytesRead = Size;
								} // else if short
							else if (! fseek(ffile, Size, SEEK_CUR))
								BytesRead = Size;
							} // uncompressed
						break;
						}
					default:
						{
						CRUMB(UserMessageOK("DEM Debug", "EL default");)
						if (! fseek(ffile, Size, SEEK_CUR))
							BytesRead = Size;
						break;
						} 
					} // switch 

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read 
			else
				break;
			} // if not done flag 
		} // if tag block read 
	else
		break;
	} // while 

return (TotalRead);

} // DEM::Elevation_Load

/*===========================================================================*/

// used for DEM merging - don't call unless you ensure there's valid Map & entry settings
// get 0 based column & row as seen in DEM Editor (note: Editor is 1 based)
float DEM::GetCell(unsigned long col, unsigned long row)
{
float rval = 0.0f;

rval = *(Map() + col * pLatEntries + pLatEntries - row - 1);

return rval;

} // DEM::GetCell

/*===========================================================================*/

double DEM::DEMArrayPointExtract(CoordSys *SourceCoords, double Lat, double Lon)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
#endif // WCS_BUILD_VNS]
#ifndef BUILD_LIB
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = Lat;
Vert.Lon = Vert.xyz[0] = Lon;
#endif // !BUILD_LIB

#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

#ifdef BUILD_LIB
if (! TestPointNULL(Lon, Lat))
#else // !BUILD_LIB
if (! TestPointNULL(Vert.xyz[0], Vert.xyz[1]))
#endif // !BUILD_LIB
	{
	#ifdef BUILD_LIB
	// do it without referring to toxically-dependent VertexDEM structure
	return (ArrayPointExtract(Map(), Lon, Lat,
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries()));
	#else // !BUILD_LIB
	return (ArrayPointExtract(Map(), Vert.xyz[0], Vert.xyz[1],
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries()));
	#endif // !BUILD_LIB
	} // if

return (NullValue());

} // DEM::DEMArrayPointExtract

/*===========================================================================*/

// Used by DEM Merger.  Whole rows of valid data were being dropped in merges under some conditions where
// they should've been generated due to the fact that a NULL cell was adjacent, even though the samples footprint
// would never interpolate from those adjacent cells.
double DEM::DEMArrayPointExtract2(CoordSys *SourceCoords, double Lat, double Lon)
{
double rVal = NullValue();
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
#endif // WCS_BUILD_VNS]
#ifndef BUILD_LIB
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = Lat;
Vert.Lon = Vert.xyz[0] = Lon;
#endif // !BUILD_LIB

#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

if (NullReject)
	{
	// Attempt to compute a weighted sample
	rVal = ArrayPointExtract2(Vert.xyz[0], Vert.xyz[1],
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries());
	} // if
else
	{
	// Standard routine works well if no NULLs
	rVal = ArrayPointExtract(Map(), Vert.xyz[0], Vert.xyz[1],
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries());
	} // else

return (rVal);

} // DEM::DEMArrayPointExtract2

/***
double DEM::DEMArrayPointExtract2(CoordSys *SourceCoords, double Lat, double Lon)
{
#ifdef WCS_BUILD_VNS
JoeCoordSys *MyAttr;
CoordSys *HostCoords;
#endif // WCS_BUILD_VNS]
#ifndef BUILD_LIB
VertexDEM Vert;

Vert.Lat = Vert.xyz[1] = Lat;
Vert.Lon = Vert.xyz[0] = Lon;
#endif // !BUILD_LIB


#ifdef WCS_BUILD_VNS

if (! SourceCoords)
	SourceCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
if (MoJoe && (MyAttr = (JoeCoordSys *)MoJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
	HostCoords = MyAttr->Coord;
else
	HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();

if (SourceCoords != HostCoords)
	{
	if (SourceCoords)
		{
		if (! SourceCoords->DegToCart(&Vert))
			return (0);
		} // if
	if (HostCoords)
		{
		if (! HostCoords->CartToProj(&Vert))
			return (0);
		} // else
	} // if

#endif // WCS_BUILD_VNS

#ifdef BUILD_LIB
if (! TestPointNULL(Lon, Lat))
#else // !BUILD_LIB
if (! TestPointNULL2(Vert.xyz[0], Vert.xyz[1]))
#endif // !BUILD_LIB
	{
	#ifdef BUILD_LIB
	// do it without referring to toxically-dependent VertexDEM structure
	return (ArrayPointExtract(Map(), Lon, Lat,
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries()));
	#else // !BUILD_LIB
	return (ArrayPointExtract(Map(), Vert.xyz[0], Vert.xyz[1],
		Westest(), Southest(),
		-LonStep(), LatStep(), 
		LonEntries(), LatEntries()));
	#endif // !BUILD_LIB
	} // if

return (NullValue());

} // DEM::DEMArrayPointExtract2
***/

/*===========================================================================*/

int DEM::TestPointNULL(double X, double Y)
{
double dRow, dCol;
#ifdef DEBUG
float cellVal;
#endif // DEBUG
long Row, Col, PointCell;

if (NullReject)
	{
	dCol = ((X - Westest()) / (Eastest() - Westest())) * (LonEntries() - 1);
	dRow = ((Y - Southest()) / (Northest() - Southest())) * (LatEntries() - 1);

	Row = quicklongfloor(dRow);
	Col = quicklongfloor(dCol);
	if (Row >= 0 && Row < (long)LatEntries() && Col >= 0 && Col < (long)LonEntries())
		{
		PointCell = Col * LatEntries() + Row;
	#ifdef DEBUG
		cellVal = RawMap[PointCell];
	#endif // DEBUG
		if (TestNullReject(PointCell))
			return (1);
		} // if
	Row ++;
	if (Row >= 0 && Row < (long)LatEntries() && Col >= 0 && Col < (long)LonEntries())
		{
		PointCell = Col * LatEntries() + Row;
	#ifdef DEBUG
		cellVal = RawMap[PointCell];
	#endif // DEBUG
		if (TestNullReject(PointCell))
			return (1);
		} // if
	Col ++;
	if (Row >= 0 && Row < (long)LatEntries() && Col >= 0 && Col < (long)LonEntries())
		{
		PointCell = Col * LatEntries() + Row;
	#ifdef DEBUG
		cellVal = RawMap[PointCell];
	#endif // DEBUG
		if (TestNullReject(PointCell))
			return (1);
		} // if
	Row --;
	if (Row >= 0 && Row < (long)LatEntries() && Col >= 0 && Col < (long)LonEntries())
		{
		PointCell = Col * LatEntries() + Row;
	#ifdef DEBUG
		cellVal = RawMap[PointCell];
	#endif // DEBUG
		if (TestNullReject(PointCell))
			return (1);
		} // if
	} // if

return (0);

} // DEM::TestPointNULL

/*===========================================================================*/

// Used by DEM Merger.  Tests only the single point, and not the adjacent cells
int DEM::TestPointNULL2(double X, double Y)
{
double dRow, dCol;
#ifdef DEBUG
float cellVal;
#endif // DEBUG
long Row, Col, PointCell;

dCol = ((X - Westest()) / (Eastest() - Westest())) * (LonEntries() - 1);
dRow = ((Y - Southest()) / (Northest() - Southest())) * (LatEntries() - 1);

Row = quicklongfloor(dRow);
Col = quicklongfloor(dCol);
if (Row >= 0 && Row < (long)LatEntries() && Col >= 0 && Col < (long)LonEntries())
	{
	PointCell = Col * LatEntries() + Row;
#ifdef DEBUG
	cellVal = RawMap[PointCell];
#endif // DEBUG
	if (TestNullReject(PointCell))
		return (1);
	} // if

return (0);

} // DEM::TestPointNULL2

/*===========================================================================*/

// used by DEM Merger to attempt to create a weighted sample
double DEM::ArrayPointExtract2(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows)
{
double RemX = 0.0, RemY = 0.0, P1, P2, rVal = 0.0;
float *Data = Map();
float cell0, cell1, cell2, cell3;
long Row, Col, Col_p, Row_p, TrueRow, TrueCol, nonNULL = 0;

if (Data)
	{
	Row = TrueRow = (long)((Y - MinY) / IntY);
	Col = TrueCol = (long)((X - MinX) / IntX);
	if (Row < 0)
		Row = 0;
	else if (Row >= Rows)
		Row = Rows - 1;
	if (Col < 0)
		Col = 0;
	if (Col >= Cols)
		Col = Cols - 1;

	Row_p = Row;
	Col_p = Col;

	if (Row < Rows - 1 && TrueRow >= 0)
		{
		RemY = (Y - (Row * IntY + MinY)) / IntY;
		if (RemY < 0.0)
			RemY = 0.0;
		Row_p++;
		} // if not last row
	if (Col < Cols - 1 && TrueCol >= 0)
		{ 
		RemX = (X - (Col * IntX + MinX)) / IntX;
		if (RemX < 0.0)
			RemX = 0.0;
		Col_p++;
		} // if not last column

	if ((cell0 = Data[Col_p * Rows + Row]) != pNullValue)
		nonNULL++;
	if ((cell1 = Data[Col * Rows + Row]) != pNullValue)
		nonNULL++;
	if ((cell2 = Data[Col_p * Rows + Row_p]) != pNullValue)
		nonNULL++;
	if ((cell3 = Data[Col * Rows + Row_p]) != pNullValue)
		nonNULL++;

	if (nonNULL == 4)
		{
		// compute weighted sample from all four cells
		P1 = RemX * (cell0 - cell1)	+ cell1;
		P2 = RemX * (cell2 - cell3)	+ cell3;
		rVal = RemY * (P2 - P1) + P1;
		} // if
	else if (nonNULL == 3)
		{
		// compute a weighted sample from 3 valid cells
		if (cell0 == pNullValue)
			{
			P1 = cell1;
			P2 = RemX * (cell2 - cell3)	+ cell3;
			rVal = RemY * (P2 - P1) + P1;
			} // if cell0
		else if (cell1 == pNullValue)
			{
			P1 = cell0;
			P2 = RemX * (cell2 - cell3)	+ cell3;
			rVal = RemY * (P2 - P1) + P1;
			} // else if cell1
		else if (cell2 == pNullValue)
			{
			P1 = RemX * (cell0 - cell1)	+ cell1;
			P2 = cell3;
			rVal = RemY * (P2 - P1) + P1;
			} // else if cell2
		else if (cell3 == pNullValue)
			{
			P1 = RemX * (cell0 - cell1)	+ cell1;
			P2 = cell2;
			rVal = RemY * (P2 - P1) + P1;
			} // else if cell3
		} // else if
	else if (nonNULL == 2)
		{
		// compute a weighted sample if two values are along same row or column
		// if first row NULLs
		if ((cell0 == pNullValue) && (cell1 == pNullValue))
			{
			rVal = RemX * (cell2 - cell3) + cell3;
			} // if
		// if second row NULLs
		else if ((cell2 == pNullValue) && (cell3 == pNullValue))
			{
			rVal = RemX * (cell0 - cell1) + cell1;
			} // else if
		// if first col NULLs
		else if ((cell0 == pNullValue) && (cell2 == pNullValue))
			{
			rVal = RemY * (cell1 - cell3) + cell3;
			} // else if
		// if second col NULLs
		else if ((cell1 == pNullValue) && (cell3 == pNullValue))
			{
			rVal = RemY * (cell0 - cell2) + cell2;
			} // else if
		else
			rVal = NullValue();
		} // else if
	else
		{
		// return NULL for 0 or 1 valid cells
		rVal = NullValue();
		} // else

	} // if

return (rVal);

} // ArrayPointExtract()

/*===========================================================================*/

void DEM::PrecalculateCommonFactors(void)
{

// precalculate some common factors that we don't need to store
InvLatStep = 1.0 / pLatStep;
InvLonStep = 1.0 / pLonStep;

} // DEM::PrecalculateCommonFactors

/*===========================================================================*/

// You need to save the DEM after you call this routine
long DEM::FillVoids(void)
{
float GaussSum, GaussWeight, weight;
float GaussKernal[5][8] =	// 5x5 kernal in 5x8 array so compiler can optimize better
	{{1.0f, 2.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f},
	{2.0f, 7.0f, 11.0f, 7.0f, 2.0f, 0.0f, 0.0f, 0.0f},
	{3.0f, 11.0f, 17.0f, 11.0f, 3.0f, 0.0f, 0.0f, 0.0f},
	{2.0f, 7.0f, 11.0f, 7.0f, 2.0f, 0.0f, 0.0f, 0.0f},
	{1.0f, 2.0f, 3.0f, 2.0f, 1.0f, 0.0f, 0.0f, 0.0f}};
/***
double GaussKernal[9][9] = {
	{ 0.0,  0.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.0,  0.0},
	{ 0.0,  1.0,  2.0,  3.0,  3.0,  3.0,  2.0,  1.0,  0.0},
	{ 1.0,  2.0,  3.0,  6.0,  7.0,  6.0,  3.0,  2.0,  1.0},
	{ 1.0,  3.0,  6.0,  9.0, 11.0,  9.0,  6.0,  3.0,  1.0},
	{ 1.0,  3.0,  7.0, 11.0, 12.0, 11.0,  7.0,  3.0,  1.0},
	{ 1.0,  3.0,  6.0,  9.0, 11.0,  9.0,  6.0,  3.0,  1.0},
	{ 1.0,  2.0,  3.0,  6.0,  7.0,  6.0,  3.0,  2.0,  1.0},
	{ 0.0,  1.0,  2.0,  3.0,  3.0,  3.0,  2.0,  1.0,  0.0},
	{ 0.0,  0.0,  1.0,  1.0,  1.0,  1.0,  1.0,  0.0,  0.0}};
***/
float *cellIn, *cellOut, *curCell, *fixedElevs;	// cell accessed via kernal, cell we're writing to, cell that may be fixed
long LatEntries = (long)pLatEntries;	// avoid signed/unsigned mismatch
long LonEntries = (long)pLonEntries;
long x, y;			// the DEM cell we're working on
long xkern, ykern;
long xpos, ypos;	// the kernal cell index into the DEM
long success = 0;

// attempt to fill voids iff NULLs are enabled & we can allocate another array to hold the new elev data
if (NullReject && (fixedElevs = (float *)AppMem_Alloc(MapSize() * sizeof (float), APPMEM_CLEAR)))
	{
	curCell = RawMap;
	cellOut = fixedElevs;
	// scan DEM in memory order for faster access
	for (y = (LatEntries - 1); y >= 0; y--)
		{
		for (x = 0; x < LonEntries; x++, cellOut++, curCell++)
			{
			float writeVal;

			// if void, try to fill it, otherwise copy it
			if (*curCell != NullValue())
				{
				writeVal = *curCell;
				} // if
			else
				{
				for (ykern = -2; ykern <= 2; ykern++)
					{
					ypos = y + ykern;
					if ((ypos >= 0) && (ypos < LatEntries))
						{
						GaussSum = 0.0f;
						GaussWeight = 0.0f;
						for (xkern = -2; xkern <= 2; xkern++)
							{
							xpos = x + xkern;
							if ((xpos >= 0) && (xpos < LonEntries))
								{
								// array + x * rows + rows - y - 1
								cellIn = RawMap + xpos * LatEntries + LatEntries - ypos - 1;
								if (*cellIn != NullValue())
									{
									weight = GaussKernal[ykern + 2][xkern + 2];
									GaussWeight += weight;
									GaussSum += *cellIn * weight;
									} // if
								} // if valid xpos
							} // for xkern
						} // if valid ypos
					} // for ykern
				// see if we're stuck in a sea of unset data
				if (GaussWeight != 0.0f)
					writeVal = GaussSum / GaussWeight;
				else
					writeVal = NullValue();
				} // else
			*cellOut = writeVal;
			} // for x
		} // for y
	memcpy(RawMap, fixedElevs, MapSize() * sizeof (float));
	// FindElMaxMin(); -- unneeded - saver does this
	ComputeRelEl();
	AppMem_Free(fixedElevs, MapSize() * sizeof (float));
	} // if

return(success);

} // DEM::FillVoids

/*===========================================================================*/
