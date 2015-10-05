// ExportControlGUI.h
// Header file for ExportControlGUI
// Created from scratch on 4/23/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_RASTERRESAMPLER_H
#define WCS_RASTERRESAMPLER_H

// function can resample any size raster to any other size using barycentric weighted sampling.
// All data resides in files, only a few lines of data reside in memory at one time.

class RasterResampler
	{
	public:
		float NullValue;
		long InputRowSize, OutputRowSize, InputRows, OutputRows, InputCols, OutputCols, LowRowLoaded, HighRowLoaded, 
			TileRows, TileCols, TileStartRow, TileStartCol, TileOutputCols, TileOutputRows, HonorNull;
		void *LowRow, *HighRow, *OutputRow;
		FILE *InputFile, *OutputFile;

		RasterResampler();
		~RasterResampler();
		void SetNull(float NewNull) {NullValue = NewNull; HonorNull = 1;};
		int Resample(const char *InputPath, const char *OutputPath, long InRows, long InCols, long OutRows, long OutCols);
		int ResampleTile(const char *InputPath, const char *OutputPath, long InRows, long InCols, long OutRows, long OutCols,
			long TilesY, long TilesX, long CurTileY, long CurTileX, long Overlap);
		int OpenFiles(const char *InputPath, const char *OutputPath);
		void CloseFiles(void);
		int AllocRows(void);
		void FreeRows(void);
		int ReadRow(void *ReadTo, long RowNum, long &RowLoaded);
		int WriteRow(void);
		int SampleRow(long RowNum);
		void SamplePoint(long PointNum, float RowPointOffset);
		virtual void SamplePoint(long PointNum, long LowInPt, long HighInPt, float PointDist[4]) = 0;
		virtual void CalcRowSizes(void) = 0;

	}; // class RasterResampler

class UByteRasterResampler : public RasterResampler
	{
	public:
		UByteRasterResampler()	{};
		virtual void SamplePoint(long PointNum, long LowInPt, long HighInPt, float PointOffsets[4]);
		virtual void CalcRowSizes(void);

	}; // class UByteRasterResampler

class FloatRasterResampler : public RasterResampler
	{
	public:
		FloatRasterResampler()	{};
		virtual void SamplePoint(long PointNum, long LowInPt, long HighInPt, float PointDist[4]);
		virtual void CalcRowSizes(void);

	}; // class FloatRasterResampler

enum
	{
	WCS_RESAMPLE_BANDTYPE_UBYTE,
	WCS_RESAMPLE_BANDTYPE_FLOAT
	}; // band sizes for resampling

#endif // WCS_RASTERRESAMPLER_H
