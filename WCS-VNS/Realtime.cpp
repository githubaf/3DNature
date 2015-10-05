// Realtime.cpp
// code file for Realtime Data classes
// Created 12/17/01 by Gary R. Huber
// Copyright 2001 3D Nature LLC. All rights reserved.

#include "stdafx.h"
#include "Realtime.h"
#include "EffectsLib.h"
#include "Raster.h"
#include "Useful.h"
#include "Application.h"
#include "Project.h"
#include "Requester.h"

int RealtimeFoliageData::WriteFoliageRecordVF(FILE *ffile, RealtimeFoliageIndex *RFI)
{
double HeightNum;
//double TexRefLat, TexRefLon /*, TexRefElev */;
//double CoordFlip[2];
long HtInterp, HeightBits;
float FloatHeight;
short Padding = 0;

// VF1.1 needs these, VF2.0 doesn't
//TexRefLat = RFI->RefXYZ[1];	//GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Y);
//TexRefLon = RFI->RefXYZ[0];	//GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_X);
//TexRefElev = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z);

if (ElementID)
	{
	if (ElementID > 0)
		{
		short VFElementID, ShortHeight;
		float LocalX, LocalY;

		HtInterp = ((Height & ~0x3fff) >> 14);
		HeightBits = (Height & 0x3fff);
		HeightNum = (double)HeightBits;
		FloatHeight = (float)(HtInterp == 0x00 ? Height :
		HtInterp == 0x01 ? HeightNum * .1 :
		HtInterp == 0x02 ? HeightNum * .01 :
		HeightNum * .001);

		// grh modified coords
		//CoordFlip[0] = -(XYZ[0] + TexRefLon);
		// according to grh:
		// modifed coords, now they are in the export coord sys with positive always to east even if geographic
		
		// VF Version 1.1 wanted coords in absolute reference, new 2.0 version uses
		// a local offset -- JUST LIKE US!
		//CoordFlip[0] = XYZ[0] + TexRefLon;
		//CoordFlip[1] = XYZ[1] + TexRefLat;

		//CoordFlip[0] = XYZ[0];
		LocalX = XYZ[0];
		//CoordFlip[1] = XYZ[1];
		LocalY = XYZ[1];

		// XY floats (formerly doubles in VF1.1)
		#ifdef BYTEORDER_BIGENDIAN
		//SimpleEndianFlip64(&CoordFlip[0], &CoordFlip[0]);
		//SimpleEndianFlip64(&CoordFlip[1], &CoordFlip[1]);
		SimpleEndianFlip32F(&LocalX, &LocalX);
		SimpleEndianFlip32F(&LocalY, &LocalY);
		#endif // BYTEORDER_BIGENDIAN
		//fwrite((char *)&CoordFlip[0], sizeof (double), 1, ffile);
		//fwrite((char *)&CoordFlip[1], sizeof (double), 1, ffile);
		fwrite((char *)&LocalX, sizeof (float), 1, ffile);
		fwrite((char *)&LocalY, sizeof (float), 1, ffile);

		// NOTE! VF2.0 switched to centimeters for Height. I know not why...
		FloatHeight *= 100.0f;
		ShortHeight = (short)FloatHeight;

		// Height
		//if (! fwrite((char *)&FloatHeight, sizeof (float), 1, ffile)) // VF1.1...
		if (! fwrite((char *)&ShortHeight, sizeof (short), 1, ffile)) // VF2.0
			return (0);
		// Species
		VFElementID = ElementID - 1; //; VF starts at 0, we start at 1
		if (! fwrite((char *)&VFElementID, sizeof (short), 1, ffile))
			return (0);
		// padding (VF2.0 omits this field)
		//if (! fwrite((char *)&Padding, sizeof (unsigned short), 1, ffile))
		//	return (0);
		} // if
	else
		{
		// 3D Object, skip for now
		} // else
	} // if

return (1);

} // RealtimeFoliageData::WriteFoliageRecordVF

/*===========================================================================*/

int RealtimeFoliageData::WriteFoliageRecord(FILE *ffile)
{

if (ElementID)
	{
	if (! fwrite((char *)&ElementID, sizeof (short), 1, ffile))
		return (0);
	fwrite((char *)&XYZ[0], sizeof (float), 1, ffile);
	fwrite((char *)&XYZ[1], sizeof (float), 1, ffile);
	fwrite((char *)&XYZ[2], sizeof (float), 1, ffile);
	if (! fwrite((char *)&Height, sizeof (unsigned short), 1, ffile))
		return (0);
	#ifdef WCS_BUILD_SX2
	if (WCS_FOLIAGELIST_FILE_VERSION > 1)
		{
		fwrite((char *)&BitInfo, sizeof (unsigned char), 1, ffile);
		// two mutually exclusive bits
		if (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY)
			{
			fwrite((char *)&MyEffect, sizeof (GeneralEffect *), 1, ffile);
			fwrite((char *)&MyVec, sizeof (Joe *), 1, ffile);
			} // if
		else if (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_VECTORPRESENT)
			{
			fwrite((char *)&MyVec, sizeof (Joe *), 1, ffile);
			} // if
		} // if
	#endif // WCS_BUILD_SX2
	if (ElementID > 0)
		{
		#ifdef WCS_BUILD_SX2
		if (WCS_FOLIAGELIST_FILE_VERSION == 1)
		#endif // WCS_BUILD_SX2
			fwrite((char *)&BitInfo, sizeof (unsigned char), 1, ffile);
		fwrite((char *)&ImageColorOpacity, sizeof (unsigned char), 1, ffile);
		if (ImageColorOpacity < 255)
			{
			fwrite((char *)&TripleValue[0], sizeof (unsigned char), 1, ffile);
			fwrite((char *)&TripleValue[1], sizeof (unsigned char), 1, ffile);
			fwrite((char *)&TripleValue[2], sizeof (unsigned char), 1, ffile);
			} // if
		} // if
	else
		{
		fwrite((char *)&TripleValue[0], sizeof (unsigned char), 1, ffile);
		fwrite((char *)&TripleValue[1], sizeof (unsigned char), 1, ffile);
		if (! fwrite((char *)&TripleValue[2], sizeof (unsigned char), 1, ffile))
			return (0);
		} // else
	} // if

return (1);
} // RealtimeFoliageData::WriteFoliageRecord

/*===========================================================================*/

int RealtimeFoliageData::ReadFoliageRecord(FILE *ffile, char FileVersion)
{
if (! fread((char *)&ElementID, sizeof (short), 1, ffile))
	return (0);
fread((char *)&XYZ[0], sizeof (float), 1, ffile);
fread((char *)&XYZ[1], sizeof (float), 1, ffile);
fread((char *)&XYZ[2], sizeof (float), 1, ffile);
if (! fread((char *)&Height, sizeof (unsigned short), 1, ffile))
	return (0);
#ifdef WCS_BUILD_SX2
if (FileVersion > 1)
	{
	fread((char *)&BitInfo, sizeof (unsigned char), 1, ffile);
	// two mutually exclusive bits
	if (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_CLICKQUERY)
		{
		fread((char *)&MyEffect, sizeof (GeneralEffect *), 1, ffile);
		fread((char *)&MyVec, sizeof (Joe *), 1, ffile);
		} // if
	else if (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_VECTORPRESENT)
		{
		fread((char *)&MyVec, sizeof (Joe *), 1, ffile);
		MyEffect = NULL;
		} // if
	else
		{
		MyVec = NULL;
		MyEffect = NULL;
		} // else
	} // if
else
	{
	MyVec = NULL;
	MyEffect = NULL;
	} // else
#endif // WCS_BUILD_SX2
if (ElementID > 0)
	{
	#ifdef WCS_BUILD_SX2
	if (FileVersion == 1)
	#endif // WCS_BUILD_SX2
		fread((char *)&BitInfo, sizeof (unsigned char), 1, ffile);
	fread((char *)&ImageColorOpacity, sizeof (unsigned char), 1, ffile);
	if (ImageColorOpacity < 255)
		{
		fread((char *)&TripleValue[0], sizeof (unsigned char), 1, ffile);
		fread((char *)&TripleValue[1], sizeof (unsigned char), 1, ffile);
		fread((char *)&TripleValue[2], sizeof (unsigned char), 1, ffile);
		} // if
	} // if
else
	{
	fread((char *)&TripleValue[0], sizeof (unsigned char), 1, ffile);
	fread((char *)&TripleValue[1], sizeof (unsigned char), 1, ffile);
	if (! fread((char *)&TripleValue[2], sizeof (unsigned char), 1, ffile))
		return (0);
	} // else

return (1);
} // RealtimeFoliageData::ReadFoliageRecord

/*===========================================================================*/

int RealtimeFoliageData::InterpretFoliageRecord(EffectsLib *Effects, ImageLib *Images, FoliagePreviewData *PointData)
{
long HtInterp, HeightBits;
double HeightNum;

if (ElementID)
	{
	HtInterp = ((Height & ~0x3fff) >> 14);
	HeightBits = (Height & 0x3fff);
	HeightNum = (double)HeightBits;
	PointData->Height = HtInterp == 0x00 ? Height :
		HtInterp == 0x01 ? HeightNum * .1 :
		HtInterp == 0x02 ? HeightNum * .01 :
		HeightNum * .001;

	if (ElementID > 0)
		{
		PointData->Object3D = NULL;
		if(Images)
			{ // skip this step if IOL not provided
			PointData->CurRast = Images->FindByID((unsigned)(long)ElementID);
			PointData->Width = PointData->Height * ((double)PointData->CurRast->Cols / (double)PointData->CurRast->Rows);
			} // if
		else
			{
			PointData->CurRast = NULL;
			PointData->Width = PointData->Height;	// don't know what else to make it
			} // else
		PointData->FlipX = (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_FLIPX) ? 1: 0;
		PointData->Shade3D = (BitInfo & WCS_REALTIME_FOLDAT_BITINFO_SHADE3D) ? 1: 0;
		if (ImageColorOpacity < 255)
			{
			PointData->ColorImageOpacity = ImageColorOpacity * .003921569;	//	1 / 255.0
			PointData->RGB[0] = TripleValue[0] * .003921569;
			PointData->RGB[1] = TripleValue[1] * .003921569;
			PointData->RGB[2] = TripleValue[2] * .003921569;
			} // if
		else
			PointData->ColorImageOpacity = 1.0;
		} // if
	else
		{
		PointData->CurRast = NULL;
		if(Effects)
			{ // skip this step if Effects library not provided
			PointData->Object3D = (Object3DEffect *)Effects->FindByID(WCS_EFFECTSSUBCLASS_OBJECT3D, (unsigned)(long)(-ElementID));
			} // if
		else
			{
			PointData->Object3D = NULL;
			} // else
		PointData->Rotate[0] = TripleValue[0] * 1.411764706;	// 360.0 / 255.0;
		PointData->Rotate[1] = TripleValue[1] * 1.411764706;
		PointData->Rotate[2] = TripleValue[2] * 1.411764706;
		} // else
	} // if

return (1);

} // RealtimeFoliageData::InterpretFoliageRecord

/*===========================================================================*/
/*===========================================================================*/

RealtimeFoliageCellData::~RealtimeFoliageCellData()
{

FreeFolData();

} // RealtimeFoliageCellData::~RealtimeFoliageCellData()

/*===========================================================================*/

void RealtimeFoliageCellData::FreeFolData(void)
{

if(FolData && NumDatLoaded)
	{
	delete [] FolData;
	FolData = NULL;
	} // if

} // RealtimeFoliageCellData::FreeFolData

/*===========================================================================*/

RealtimeFoliageData *RealtimeFoliageCellData::AllocFolData(int NumFolData)
{

FreeFolData();

if(NumFolData)
	{
	if(FolData = new RealtimeFoliageData[NumFolData])
		{
		NumDatLoaded = NumFolData;
		} // if
	} // if

return(FolData);
} // RealtimeFoliageCellData::RealtimeFoliageData

/*===========================================================================*/

int RealtimeFoliageCellData::LoadFolData(RealtimeFoliageIndex *Index, long FileCt)
{
int DatPt = 0;
FILE *ffile;
char FileName[512], TestFileVersion;

FreeFolData();
if (AllocFolData(Index->CellDat[FileCt].DatCt))
	{
	strmfp(FileName, GlobalApp->MainProj->dirname, Index->CellDat[FileCt].FileName);
	if (ffile = PROJ_fopen(FileName, "rb"))
		{
		fgets(FileName, 64, ffile);
		// version
		fread((char *)&TestFileVersion, sizeof (char), 1, ffile);
		// test to see if same version file as index
		if (TestFileVersion == Index->FileVersion)
			{
			for (DatPt = 0; DatPt < Index->CellDat[FileCt].DatCt; DatPt ++)
				{
				if (! FolData[DatPt].ReadFoliageRecord(ffile, Index->FileVersion))
					{
					break; // error
					} // if
				} // for
			} // if
		fclose(ffile);
		ffile = NULL;
		} // if
	} // if

return(DatPt);
} // RealtimeFoliageCellData::LoadFolData

/*===========================================================================*/
/*===========================================================================*/


void RealtimeFoliageIndex::FreeAllFoliage(void)
{
if (CellDat)
	delete [] CellDat;
CellDat = NULL;
if (RasterInterp)
	delete [] RasterInterp;
RasterInterp = NULL;
} // RealtimeFoliageIndex::FreeAllFoliage

/*===========================================================================*/

RealtimeFoliageIndex::~RealtimeFoliageIndex()
{
FreeAllFoliage();
} // RealtimeFoliageIndex::~RealtimeFoliageIndex

/*===========================================================================*/

// Pathname can be NULL
int RealtimeFoliageIndex::LoadFoliageIndex(char *Filename, char *Pathname)
{
char FileName[512], BaseName[512], *Ext;
FILE *ffile;
long FileCt = 0;

// find and open the index file
if (! Pathname)
	{
	strcpy(BaseName, Filename); // to make it modifyable
	Pathname = GlobalApp->MainProj->dirname;
	// lop off any extension if provided -- we will add ours
	if(Ext = FindFileExtension(BaseName))
		{
		Ext[0] = NULL;
		} // if
	strcat(BaseName, ".dat");
	strmfp(FileName, Pathname, BaseName);
	} // if
else
	strmfp(FileName, Pathname, Filename);

FreeAllFoliage();

if (ffile = PROJ_fopen(FileName, "rb"))
	{
	// read file descriptor, no need to keep it around unless you want to
	fgets(FileName, 256, ffile);
	// version
	fread((char *)&FileVersion, sizeof (char), 1, ffile);
	if (FileVersion <= WCS_FOLIAGELIST_FILE_VERSION)
		{
		// number of files
		fread((char *)&NumCells, sizeof (long), 1, ffile);
		// reference XYZ
		fread((char *)&RefXYZ[0], sizeof (double), 1, ffile);
		fread((char *)&RefXYZ[1], sizeof (double), 1, ffile);
		fread((char *)&RefXYZ[2], sizeof (double), 1, ffile);

		if (NumCells > 0)
			{
			// allocate cell data
			if (CellDat = new RealtimeFoliageCellData[NumCells])
				{
				// for each file
				for (FileCt = 0; FileCt < NumCells; FileCt ++)
					{
					// file name
					fgets(CellDat[FileCt].FileName, 64, ffile);
					// center XYZ
					fread((char *)&CellDat[FileCt].CellXYZ[0], sizeof (double), 1, ffile);
					fread((char *)&CellDat[FileCt].CellXYZ[1], sizeof (double), 1, ffile);
					fread((char *)&CellDat[FileCt].CellXYZ[2], sizeof (double), 1, ffile);
					// half cube cell dimension
					fread((char *)&CellDat[FileCt].CellRad, sizeof (double), 1, ffile);
					// number of trees in file
					if (fread((char *)&CellDat[FileCt].DatCt, sizeof (long), 1, ffile) != 1)
						break;
					} // for
				} // if
			} // if some cells to read
		} // if OK version
	else
		UserMessageOK("Realtime Foliage File", "Foliage file is a newer version than is supported by this version of the program. It cannot be read.");
	fclose(ffile);

	// see if there is a foliage image name file
	strmfp(FileName, Pathname, Filename);
	StripExtension(FileName);
	strcat(FileName, "ImageList.dat");
	if (ffile = PROJ_fopen(FileName, "r"))
		{
		long ScanNum, NumOldRast, NameLen;
		Raster *FoundRast;
		char TempName[256];

		// initialize current raster stable
		GlobalApp->AppImages->InitRasterIDs();
		// read number of rasters
		//fscanf(ffile, "%s", TempName);
		fgets(TempName, 256, ffile);
		NumOldRast = atoi(TempName);
		// allocate the number of slots for the old images
		if (NumOldRast > 1 && (RasterInterp = new long[NumOldRast + 1]))
			{
			ScanNum = 1;
			// can't use fscanf here because it stops reading at blank spaces as well as end of lines
			// need fgets and strip off end of line. Check for NULL return which could indicate an error or EOF
			//while ((fscanf(ffile, "%s", TempName) != EOF) && ScanNum <= NumOldRast)
			while (fgets(TempName, 256, ffile) && ScanNum <= NumOldRast)
				{
				NameLen = strlen(TempName) - 1;
				if (TempName[NameLen] == '\n')
					TempName[NameLen] = 0;
				RasterInterp[ScanNum] = 0;
				if (FoundRast = GlobalApp->AppImages->FindByUserName(TempName))
					{
					RasterInterp[ScanNum] = FoundRast->GetID();
					} // if
				ScanNum ++;
				} // while
			} // if
		fclose(ffile);
		} // if
	} // if index opened

return(FileCt);
} // RealtimeFoliageIndex::LoadFoliageIndex


/*===========================================================================*/


int RealtimeFoliageIndex::LoadAllFoliage(void)
{
long int FileCount, ObjCount = 0, TempCt;

if(CellDat)
	{
	for(FileCount = 0; FileCount < NumCells; FileCount++)
		{
		TempCt = LoadFoliageCellData(FileCount);
		if (! TempCt)
			break;
		ObjCount += TempCt;
		} // for
	} // if

return(FileCount);
} // RealtimeFoliageIndex::LoadAllFoliage


/*===========================================================================*/

int RealtimeFoliageIndex::LoadFoliageCellData(long FileCt)
{
int NumSuccess = 0;

if(FileCt <= NumCells)
	{
	NumSuccess = CellDat[FileCt].LoadFolData(this, FileCt);
	} // if

return(NumSuccess);
} // RealtimeFoliageIndex::LoadFoliageCellData

/*===========================================================================*/

RealtimeFoliageData *RealtimeFoliageIndex::FirstWalk(void)
{
RealtimeFoliageData *NextRTFD = NULL;

WalkCellNum = 0; WalkDatNum = 0;

if(NumCells)
	{
	if(CellDat)
		{
		if(WalkDatNum < CellDat[WalkCellNum].NumDatLoaded)
			{
			NextRTFD = &CellDat[WalkCellNum].FolData[WalkDatNum];
			} // if
		} // if
	} // if

return(NextRTFD);
} // RealtimeFoliageIndex::FirstWalk


/*===========================================================================*/

RealtimeFoliageData *RealtimeFoliageIndex::NextWalk(void)
{
RealtimeFoliageData *NextRTFD = NULL;

WalkDatNum++;
if(WalkDatNum == CellDat[WalkCellNum].NumDatLoaded)
	{
	WalkDatNum = 0;
	WalkCellNum++;
	} // if

if(WalkCellNum < NumCells)
	{
	if(CellDat)
		{
		if(WalkDatNum < CellDat[WalkCellNum].NumDatLoaded)
			{
			NextRTFD = &CellDat[WalkCellNum].FolData[WalkDatNum];
			} // if
		} // if
	} // if

return(NextRTFD);
} // RealtimeFoliageIndex::NextWalk

/*===========================================================================*/

RealTimeFoliageWriteConfig::RealTimeFoliageWriteConfig()
{
long Ct;
unsigned char MetricTypes[WCS_REALTIME_CONFIG_MAX] = {WCS_ANIMDOUBLE_METRIC_HEIGHT, WCS_ANIMDOUBLE_METRIC_HEIGHT, WCS_ANIMDOUBLE_METRIC_DISTANCE, WCS_ANIMDOUBLE_METRIC_DISTANCE};
double EffectDefault[WCS_REALTIME_CONFIG_MAX] = {1.0, 1000.0, 0.0, 1000000}; // one MILLION meters (1Mm)
double RangeDefaults[WCS_REALTIME_CONFIG_MAX][3] =
	{
	DBL_MAX, 0, 1.0,
	DBL_MAX, 0, 1.0, 
	DBL_MAX, 0, 1.0,
	DBL_MAX, 0, 1.0,
	};

for (Ct = 0; Ct < WCS_REALTIME_CONFIG_MAX; Ct ++)
	{
	ConfigParams[Ct].SetMetricType(MetricTypes[Ct]);
	ConfigParams[Ct].SetDefaults(NULL, (char)Ct, EffectDefault[Ct]);
	ConfigParams[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	ConfigParams[Ct].SetNoNodes(1);
	} // for

Include3DO = 1;
IncludeImage = 1;
IncludeLabels = 0;

NumFiles = 20;
StemsPerCell = 1000;

strcpy(BaseName, "RealtimeData");
DirName[0] = 0;

} // RealTimeFoliageWriteConfig::RealTimeFoliageWriteConfig

/*===========================================================================*/

ULONG RealTimeFoliageWriteConfig::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
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
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_VIEWINIT_FOLWRITE_BASENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)BaseName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_INCLUDE3DO:
						{
						BytesRead = ReadBlock(ffile, (char *)&Include3DO, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_INCLUDEIMAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&IncludeImage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_NUMFILES:
						{
						BytesRead = ReadBlock(ffile, (char *)&NumFiles, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_STEMSPERCELL:
						{
						BytesRead = ReadBlock(ffile, (char *)&StemsPerCell, WCS_BLOCKTYPE_LONGINT + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_MINHEIGHT:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_MAXHEIGHT:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_NEARDIST:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLWRITE_FARDIST:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_FARDIST].Load(ffile, Size, ByteFlip);
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

} // RealTimeFoliageWriteConfig::Load

/*===========================================================================*/

unsigned long int RealTimeFoliageWriteConfig::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_REALTIME_CONFIG_MAX] = {WCS_VIEWINIT_FOLWRITE_MINHEIGHT, 
															WCS_VIEWINIT_FOLWRITE_MAXHEIGHT,
															WCS_VIEWINIT_FOLWRITE_NEARDIST,
															WCS_VIEWINIT_FOLWRITE_FARDIST};

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLWRITE_BASENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(BaseName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)BaseName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLWRITE_INCLUDE3DO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Include3DO)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLWRITE_INCLUDEIMAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&IncludeImage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLWRITE_NUMFILES, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&NumFiles)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLWRITE_STEMSPERCELL, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_LONG,
	WCS_BLOCKTYPE_LONGINT, (char *)&StemsPerCell)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < WCS_REALTIME_CONFIG_MAX; Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ConfigParams[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

return (TotalWritten);

WriteError:

return (0L);

} // RealTimeFoliageWriteConfig::Save

/*===========================================================================*/
/*===========================================================================*/

RealTimeFoliageDisplayConfig::RealTimeFoliageDisplayConfig()
{
long Ct;
unsigned char MetricTypes[WCS_REALTIME_CONFIG_MAX] = {WCS_ANIMDOUBLE_METRIC_HEIGHT, WCS_ANIMDOUBLE_METRIC_HEIGHT, WCS_ANIMDOUBLE_METRIC_DISTANCE, WCS_ANIMDOUBLE_METRIC_DISTANCE};
double EffectDefault[WCS_REALTIME_CONFIG_MAX] = {1.0, 1000.0, 0.0, 1000000}; // one MILLION meters (1Mm)
double RangeDefaults[WCS_REALTIME_CONFIG_MAX][3] =
	{
	DBL_MAX, 0, 1.0,
	DBL_MAX, 0, 1.0, 
	DBL_MAX, 0, 1.0,
	DBL_MAX, 0, 1.0,
	};

for (Ct = 0; Ct < WCS_REALTIME_CONFIG_MAX; Ct ++)
	{
	ConfigParams[Ct].SetMetricType(MetricTypes[Ct]);
	ConfigParams[Ct].SetDefaults(NULL, (char)Ct, EffectDefault[Ct]);
	ConfigParams[Ct].SetRangeDefaults(RangeDefaults[Ct]);
	ConfigParams[Ct].SetNoNodes(1);
	} // for

Display3DO = 1;
DisplayImage = 1;

strcpy(BaseName, "RealtimeData");
} // RealTimeFoliageDisplayConfig::RealTimeFoliageDisplayConfig

/*===========================================================================*/

ULONG RealTimeFoliageDisplayConfig::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file 
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file 
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
					} // switch 

				switch (ItemTag & 0xffff0000)
					{
					case WCS_VIEWINIT_FOLDISPLAY_BASENAME:
						{
						BytesRead = ReadBlock(ffile, (char *)BaseName, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_DISPLAY3DO:
						{
						BytesRead = ReadBlock(ffile, (char *)&Display3DO, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_DISPLAYIMAGE:
						{
						BytesRead = ReadBlock(ffile, (char *)&DisplayImage, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_MINHEIGHT:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_MINHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_MAXHEIGHT:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_MAXHEIGHT].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_NEARDIST:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_NEARDIST].Load(ffile, Size, ByteFlip);
						break;
						}
					case WCS_VIEWINIT_FOLDISPLAY_FARDIST:
						{
						BytesRead = ConfigParams[WCS_REALTIME_CONFIG_FARDIST].Load(ffile, Size, ByteFlip);
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

} // RealTimeFoliageDisplayConfig::Load

/*===========================================================================*/

unsigned long int RealTimeFoliageDisplayConfig::Save(FILE *ffile)
{
ULONG ItemTag, TotalWritten = 0;
long BytesWritten, Ct;
unsigned long int AnimItemTag[WCS_REALTIME_CONFIG_MAX] = {WCS_VIEWINIT_FOLDISPLAY_MINHEIGHT, 
															WCS_VIEWINIT_FOLDISPLAY_MAXHEIGHT,
															WCS_VIEWINIT_FOLDISPLAY_NEARDIST,
															WCS_VIEWINIT_FOLDISPLAY_FARDIST};

if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLDISPLAY_BASENAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(BaseName) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)BaseName)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLDISPLAY_DISPLAY3DO, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&Display3DO)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
if ((BytesWritten = PrepWriteBlock(ffile, WCS_VIEWINIT_FOLDISPLAY_DISPLAYIMAGE, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (char *)&DisplayImage)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;

for (Ct = 0; Ct < WCS_REALTIME_CONFIG_MAX; Ct ++)
	{
	ItemTag = AnimItemTag[Ct] + WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT;
	if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
		{
		TotalWritten += BytesWritten;

		ItemTag = 0;
		if (BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
			WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
			{
			TotalWritten += BytesWritten;

			if (BytesWritten = ConfigParams[Ct].Save(ffile))
				{
				TotalWritten += BytesWritten;
				fseek(ffile, -(BytesWritten + WCS_BLOCKSIZE_LONG), SEEK_CUR);
				if (WriteBlock(ffile, (char *)&BytesWritten,
					WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT))
					{
					fseek(ffile, 0, SEEK_END);
					} // if wrote size of block 
				else
					goto WriteError;
				} // if anim param saved 
			else
				goto WriteError;
			} // if size written 
		else
			goto WriteError;
		} // if tag written 
	else
		goto WriteError;
	} // for

ItemTag = WCS_PARAM_DONE;
if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
	goto WriteError;
TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return(TotalWritten);

} // RealTimeFoliageDisplayConfig::Save

/*===========================================================================*/
