// TerrainWriter.h
// Header file for class
// Created from scratch on 07/08/03 by FPW2
// Copyright 2003 3D Nature, LLC. All rights reserved.

#include "stdafx.h"

enum TW_HANDEDNESS
	{
	WCS_TERRAINWRITER_RIGHTHANDED,
	WCS_TERRAINWRITER_LEFTHANDED
	};

enum TW_CLOCKORDER
	{
	WCS_TERRAINWRITER_CW,
	WCS_TERRAINWRITER_CCW
	};

enum TW_UPAXIS
	{
	WCS_TERRAINWRITER_UPAXIS_X,
	WCS_TERRAINWRITER_UPAXIS_Y,
	WCS_TERRAINWRITER_UPAXIS_Z
	};

enum TW_EMITTYPE
	{
	WCS_TERRAINWRITER_ASCII,
	WCS_TERRAINWRITER_BINARY
	};

enum TW_BYTEORDER
	{
	WCS_TERRAINWRITER_LITTLEENDIAN,
	WCS_TERRAINWRITER_BIGENDIAN
	};

enum TW_COLROW_EMITORDER
	{
	WCS_TERRAINWRITER_COLROW,	// a row of columns
	WCS_TERRAINWRITER_ROWCOL	// a column of rows
	};

enum TW_COL_DIRECTION
	{
	WCS_TERRAINWRITER_W2E,	// West to East
	WCS_TERRAINWRITER_E2W
	};

enum TW_ROW_DIRECTION
	{
	WCS_TERRAINWRITER_N2S,	// North to South
	WCS_TERRAINWRITER_S2N
	};

#define FLAPFLAG_EMIT_N 1
#define FLAPFLAG_EMIT_S 2
#define FLAPFLAG_EMIT_E 4
#define FLAPFLAG_EMIT_W 8

class TerrainWriter
	{
	public:
		TW_HANDEDNESS Handedness;
		TW_CLOCKORDER ClockOrder;
		TW_UPAXIS UpAxis;
		TW_EMITTYPE EmitType;
		TW_BYTEORDER ByteOrder;
		TW_COLROW_EMITORDER ColRowEmitOrder;
		TW_COL_DIRECTION ColDirection;
		TW_ROW_DIRECTION RowDirection;
		float EmitScale;
		float EmitOffset;

		TerrainWriter();
		~TerrainWriter();

		unsigned long EmitTerrain(long xdim, long ydim, FILE *RawData, FILE *output, float Delta,
			float FlapElev = 0.0f, long EmitFlaps = 0, long FlapFlags = 0);
		void ReadRow(long xdim, long Row, float *buffer, FILE *RawElevs);
		void EmitColumns(long StartCol, long FinishCol, long ColStep, float *buffer, FILE *Output, float Delta,
			float FlapElev, long EmitFlaps, long FlapFlags);
		void EmitPoint(float value, FILE *Output);
		void RowFlap(long xdim, FILE *Output, float FlapElev);
	}; // TerrainWriter
