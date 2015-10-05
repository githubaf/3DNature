// ExportControlGUI.cpp
// Code for RasterResampler
// Built from scratch on 6/16/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "RasterResampler.h"
#include "AppMem.h"
#include "Project.h"

// function can resample any size raster to any other size using barycentric weighted sampling.
// All data resides in files, only a few lines of data reside in memory at one time.
// Byte and Float data types are currently supported.

RasterResampler::RasterResampler()
{

InputRowSize = OutputRowSize = InputRows = OutputRows = InputCols = OutputCols = TileStartRow = TileStartCol = 
	TileRows = TileCols = TileOutputCols = TileOutputRows = 0;
LowRowLoaded = HighRowLoaded = -1;
LowRow = HighRow = OutputRow = NULL;
InputFile = OutputFile = NULL;
NullValue = 0.0f;
HonorNull = 0;

} // RasterResampler::RasterResampler

/*===========================================================================*/

RasterResampler::~RasterResampler()
{

CloseFiles();
FreeRows();

} // RasterResampler::~RasterResampler

/*===========================================================================*/

int RasterResampler::Resample(const char *InputPath, const char *OutputPath, long InRows, long InCols, long OutRows, long OutCols)
{
long RowCt;
int Success = 1;

// done here as well as constructor in case Resampler is reused
LowRowLoaded = HighRowLoaded = -1;
TileStartRow = TileStartCol = 0;
InputRows = InRows;
OutputRows = OutRows;
InputCols = InCols;
OutputCols = OutCols;
TileOutputRows = OutputRows;
TileOutputCols = OutputCols;

if (Success = AllocRows())
	{
	if (Success = OpenFiles(InputPath, OutputPath))
		{
		for (RowCt = 0; Success && RowCt < TileOutputRows; RowCt ++)
			{
			if (Success = SampleRow(RowCt))
				{
				if (! (Success = WriteRow()))
					{
					break;
					} // if
				} // if
			} // for
		CloseFiles();
		} // if
	FreeRows();
	} // if

return (Success);

} // RasterResampler::Resample

/*===========================================================================*/

int RasterResampler::ResampleTile(const char *InputPath, const char *OutputPath, long InRows, long InCols, long OutRows, long OutCols,
	long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap)
{
long RowCt;
int Success = 1;

// how many rows and columns in each tile?

TileRows = (OutRows + (TilesY - 1) * Overlap) / TilesY;
TileCols = (OutCols + (TilesX - 1) * Overlap) / TilesX;

// which row and column of resampled raster would this tile start on
TileStartRow = CurTileY * (TileRows - Overlap);
if (TileStartRow < 0)
	return (0);	// illegal, overlap larger than tile
TileStartCol = CurTileX * (TileCols - Overlap);
if (TileStartCol < 0)
	return (0);	// illegal, overlap larger than tile

// done here as well as constructor in case Resampler is reused
LowRowLoaded = HighRowLoaded = -1;
InputRows = InRows;
OutputRows = OutRows;
InputCols = InCols;
OutputCols = OutCols;
TileOutputRows = TileRows;
TileOutputCols = TileCols;

if (Success = AllocRows())
	{
	if (Success = OpenFiles(InputPath, OutputPath))
		{
		for (RowCt = 0; Success && RowCt < TileOutputRows; RowCt ++)
			{
			if (Success = SampleRow(RowCt))
				{
				if (! (Success = WriteRow()))
					{
					break;
					} // if
				} // if
			} // for
		CloseFiles();
		} // if
	FreeRows();
	} // if

return (Success);

} // RasterResampler::ResampleTile

/*===========================================================================*/

int RasterResampler::OpenFiles(const char *InputPath, const char *OutputPath)
{

if (InputFile = PROJ_fopen(InputPath, "rb"))
	{
	if (OutputFile = PROJ_fopen(OutputPath, "wb"))
		{
		return (1);
		} // if
	} // if

CloseFiles();
return (0);

} // RasterResampler::OpenFiles

/*===========================================================================*/

void RasterResampler::CloseFiles(void)
{

if (InputFile)
	fclose(InputFile);
if (OutputFile)
	fclose(OutputFile);
InputFile = OutputFile = NULL;

} // RasterResampler::CloseFiles

/*===========================================================================*/

int RasterResampler::AllocRows(void)
{

CalcRowSizes();

if (LowRow = AppMem_Alloc(InputRowSize, 0))
	{
	if (HighRow = AppMem_Alloc(InputRowSize, 0))
		{
		if (OutputRow = AppMem_Alloc(OutputRowSize, 0))
			{
			return (1);
			} // if
		} // if
	} // if

FreeRows();
return (0);

} // RasterResampler::AllocRows

/*===========================================================================*/

void RasterResampler::FreeRows(void)
{

if (LowRow)
	AppMem_Free(LowRow, InputRowSize);
if (HighRow)
	AppMem_Free(HighRow, InputRowSize);
if (OutputRow)
	AppMem_Free(OutputRow, OutputRowSize);

LowRow = HighRow = OutputRow = NULL;

} // RasterResampler::FreeRows

/*===========================================================================*/

int RasterResampler::ReadRow(void *ReadTo, long RowNum, long &RowLoaded)
{

if (! fseek(InputFile, RowNum * InputRowSize, SEEK_SET))
	{
	if (fread(ReadTo, InputRowSize, 1, InputFile) == 1)
		{
		RowLoaded = RowNum;
		return (1);
		} // if
	} // if

return (0);

} // RasterResampler::ReadRow

/*===========================================================================*/

int RasterResampler::WriteRow(void)
{

if (fwrite(OutputRow, OutputRowSize, 1, OutputFile) == 1)
	return (1);

return (0);

} // RasterResampler::WriteRow

/*===========================================================================*/

int RasterResampler::SampleRow(long RowNum)
{
float InRowPos, OutRowPos, InRowFloor;
long LowRowNeeded, HighRowNeeded, PtCt;
int LoadLowRow, LoadHighRow, Success = 1;

OutRowPos = (float)(RowNum + TileStartRow + .5f) / (OutputRows);
InRowPos = OutRowPos * InputRows - .5f;

if (InRowPos < 0.0)
	InRowPos = 0.0f;
if (InRowPos > InputRows - 1.0f)
	InRowPos = InputRows - 1.0f;

// calculate the rows that need to be loaded
InRowFloor = (float)WCS_floor((double)InRowPos);
LowRowNeeded = quicklongfloor((double)InRowFloor);
HighRowNeeded = LowRowNeeded + 1;

// don't try to load some row outside the image
if (LowRowNeeded < 0)
	LowRowNeeded = 0;
if (LowRowNeeded >= InputRows)
	LowRowNeeded = InputRows - 1;
if (HighRowNeeded < 0)
	HighRowNeeded = 0;
if (HighRowNeeded >= InputRows)
	HighRowNeeded = InputRows - 1;

// check to see if those rows are already loaded
// sampler requires both rows be loaded in all cases, even if one of them is not used
// In practice the unneeded row will be used on the next or previous sample so does not waste time loading
if (LowRowNeeded == LowRowLoaded && HighRowNeeded == HighRowLoaded)
	{
	LoadLowRow = LoadHighRow = 0;
	} // everything ready to go as is
else
	{
	if (LowRowNeeded != LowRowLoaded)
		{
		if (LowRowNeeded == HighRowLoaded)
			{
			memcpy(LowRow, HighRow, InputRowSize);
			LoadLowRow = 0;
			LowRowLoaded = HighRowLoaded;
			} // swap
		else
			LoadLowRow = 1;
		} // if
	if (HighRowNeeded != HighRowLoaded)
		LoadHighRow = 1;
	else
		LoadHighRow = 0;
	} // else

// load new rows if necessary
if (LoadLowRow)
	{
	Success = ReadRow(LowRow, LowRowNeeded, LowRowLoaded);
	} // if
if (Success && LoadHighRow)
	{
	Success = ReadRow(HighRow, HighRowNeeded, HighRowLoaded);
	} // if

// sample each point along the row
if (Success)
	{
	for (PtCt = 0; PtCt < TileOutputCols; PtCt ++)
		{
		SamplePoint(PtCt, InRowPos - InRowFloor);
		} // for
	} // if

return (Success);

} // RasterResampler::SampleRow

/*===========================================================================*/

void RasterResampler::SamplePoint(long PointNum, float RowPointOffset)
{
float InPointPos, OutPointPos, InPointFloor, ColPointOffset;
float PointOffsets[4];
long LowPointNeeded, HighPointNeeded;

OutPointPos = (float)(PointNum + TileStartCol + .5) / (OutputCols);
InPointPos = OutPointPos * InputCols - .5f;

if (InPointPos < 0.0)
	InPointPos = 0.0f;
if (InPointPos > InputCols - 1.0f)
	InPointPos = InputCols - 1.0f;

// calculate the points we will use
InPointFloor = (float)WCS_floor((double)InPointPos);
LowPointNeeded = quickftol((double)InPointFloor);
HighPointNeeded = LowPointNeeded + 1;

// don't try to load some row outside the image
if (LowPointNeeded < 0)
	LowPointNeeded = 0;
if (LowPointNeeded >= InputCols)
	LowPointNeeded = InputCols - 1;
if (HighPointNeeded < 0)
	HighPointNeeded = 0;
if (HighPointNeeded >= InputCols)
	HighPointNeeded = InputCols - 1;

ColPointOffset = InPointPos - InPointFloor;
if (RowPointOffset < 0.0)
	RowPointOffset = 0.0f;
if (RowPointOffset > 1.0)
	RowPointOffset = 1.0f;
if (ColPointOffset < 0.0)
	ColPointOffset = 0.0f;
if (ColPointOffset > 1.0)
	ColPointOffset = 1.0f;

PointOffsets[0] = ColPointOffset;
PointOffsets[1] = 1.0f - ColPointOffset;
PointOffsets[2] = RowPointOffset;
PointOffsets[3] = 1.0f - RowPointOffset;

SamplePoint(PointNum, LowPointNeeded, HighPointNeeded, PointOffsets);

} // RasterResampler::SamplePoint

/*===========================================================================*/
/*===========================================================================*/

void UByteRasterResampler::CalcRowSizes(void)
{

InputRowSize = InputCols * sizeof (unsigned char);
OutputRowSize = TileOutputCols * sizeof (unsigned char);

} // UByteRasterResampler::CalcRowSizes

/*===========================================================================*/

void UByteRasterResampler::SamplePoint(long PointNum, long LowInPt, long HighInPt, float PointOffsets[4])
{
unsigned char *ULowRow, *UHighRow, *UOutputRow;
float Samples[4], NewSample;

ULowRow = (unsigned char *)LowRow;
UHighRow = (unsigned char *)HighRow;
UOutputRow = (unsigned char *)OutputRow;

Samples[0] = ULowRow[LowInPt];
Samples[1] = ULowRow[HighInPt];
Samples[2] = UHighRow[LowInPt];
Samples[3] = UHighRow[HighInPt];

NewSample = Samples[0] * PointOffsets[1] * PointOffsets[3]
	+ Samples[1] * PointOffsets[0] * PointOffsets[3]
	+ Samples[2] * PointOffsets[1] * PointOffsets[2]
	+ Samples[3] * PointOffsets[0] * PointOffsets[2];

UOutputRow[PointNum] = (unsigned char)NewSample;

} // UByteRasterResampler::SamplePoint

/*===========================================================================*/
/*===========================================================================*/

void FloatRasterResampler::CalcRowSizes(void)
{

InputRowSize = InputCols * sizeof (float);
OutputRowSize = TileOutputCols * sizeof (float);

} // FloatRasterResampler::CalcRowSizes

/*===========================================================================*/

void FloatRasterResampler::SamplePoint(long PointNum, long LowInPt, long HighInPt, float PointOffsets[4])
{
float *FLowRow, *FHighRow, *FOutputRow;
float Samples[4], NewSample, PtWt, Wt[4];
int UseIt[4];

FLowRow = (float *)LowRow;
FHighRow = (float *)HighRow;
FOutputRow = (float *)OutputRow;

Samples[0] = FLowRow[LowInPt];
Samples[1] = FLowRow[HighInPt];
Samples[2] = FHighRow[LowInPt];
Samples[3] = FHighRow[HighInPt];

if (HonorNull)
	{
	UseIt[0] = UseIt[1] = UseIt[2] = UseIt[3] = 1;
	PtWt = 0.0f;

	if (Samples[0] == NullValue)
		UseIt[0] = 0;
	else
		PtWt = (Wt[0] = PointOffsets[1] * PointOffsets[3]);

	if (Samples[1] == NullValue)
		UseIt[1] = 0;
	else
		PtWt += (Wt[1] = PointOffsets[0] * PointOffsets[3]);

	if (Samples[2] == NullValue)
		UseIt[2] = 0;
	else
		PtWt += (Wt[2] = PointOffsets[1] * PointOffsets[2]);

	if (Samples[3] == NullValue)
		UseIt[3] = 0;
	else
		PtWt += (Wt[3] = PointOffsets[0] * PointOffsets[2]);

	if (PtWt > 0.0)
		{
		NewSample = (UseIt[0] ? Samples[0] * Wt[0]: 0.0f)
			+ (UseIt[1] ? Samples[1] * Wt[1]: 0.0f)
			+ (UseIt[2] ? Samples[2] * Wt[2]: 0.0f)
			+ (UseIt[3] ? Samples[3] * Wt[3]: 0.0f);
		NewSample /= PtWt;
		} // if
	else
		NewSample = NullValue;
	} // if
else
	{
	NewSample = Samples[0] * PointOffsets[1] * PointOffsets[3]
		+ Samples[1] * PointOffsets[0] * PointOffsets[3]
		+ Samples[2] * PointOffsets[1] * PointOffsets[2]
		+ Samples[3] * PointOffsets[0] * PointOffsets[2];
	} // else

FOutputRow[PointNum] = (float)NewSample;

} // FloatRasterResampler::SamplePoint
