// TerrainWriter.cpp
// Code module for class
// Created from scratch 07/08/03 by FPW2
// Copyright 2003 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "TerrainWriter.h"
#include "AppMem.h"
#include "UsefulIO.h"

TerrainWriter::TerrainWriter()
{
Handedness = WCS_TERRAINWRITER_RIGHTHANDED;
ClockOrder = WCS_TERRAINWRITER_CW;
UpAxis = WCS_TERRAINWRITER_UPAXIS_Z;
EmitType = WCS_TERRAINWRITER_BINARY;
ByteOrder = WCS_TERRAINWRITER_LITTLEENDIAN;
ColRowEmitOrder = WCS_TERRAINWRITER_COLROW;
ColDirection = WCS_TERRAINWRITER_W2E;
RowDirection = WCS_TERRAINWRITER_N2S;
EmitScale = 1.0f;
EmitOffset = 0.0f;

} // TerrainWriter::TerrainWriter

/*===========================================================================*/

TerrainWriter::~TerrainWriter()
{
} // TerrainWriter::~TerrainWriter

/*===========================================================================*/

unsigned long TerrainWriter::EmitTerrain(long xdim, long ydim, FILE *RawElevs, FILE *Output, float Delta,
										 float FlapElev, long EmitFlaps, long EmitFlags)
{
long Col, Row, ColStep, RowStep, StartCol, StartRow, FinishCol, FinishRow;
long ColsDone, RowsDone;
long EarlyFlap, LateFlap, XtraFlap;
float *buffer = NULL;

switch (ColDirection)
	{
	case WCS_TERRAINWRITER_W2E:
		StartCol = 0;
		FinishCol = xdim - 1;
		ColStep = 1;
		break;
	case WCS_TERRAINWRITER_E2W:
		StartCol = xdim - 1;
		FinishCol = 0;
		ColStep = -1;
		break;
	default:
		break;
	} // switch ColDirection

EarlyFlap = LateFlap = 0;
switch (RowDirection)
	{
	case WCS_TERRAINWRITER_N2S:
		StartRow = 0;
		FinishRow = ydim - 1;
		RowStep = 1;
		if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_N))
			EarlyFlap = 1;
		if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_S))
			LateFlap = 1;
		break;
	case WCS_TERRAINWRITER_S2N:
		StartRow = ydim - 1;
		FinishRow = 0;
		RowStep = -1;
		if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_S))
			EarlyFlap = 1;
		if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_N))
			LateFlap = 1;
		break;
	default:
		break;
	} // switch RowDirection

XtraFlap = 0;
if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_W))
	XtraFlap++;
if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_E))
	XtraFlap++;

ColsDone = RowsDone = 0;
switch (ColRowEmitOrder)
	{
	case WCS_TERRAINWRITER_COLROW:	// a row of columns
		{
		buffer = (float *)AppMem_Alloc(xdim * sizeof(float), 0);
		Row = StartRow;
		if (EarlyFlap)
			RowFlap(xdim + XtraFlap, Output, FlapElev);
		do
			{
			if (Row == FinishRow)
				RowsDone = 1;
			ReadRow(xdim, Row, buffer, RawElevs);
			EmitColumns(StartCol, FinishCol, ColStep, buffer, Output, Delta, FlapElev, EmitFlaps, EmitFlags);
			if (!RowsDone && (EmitType == WCS_TERRAINWRITER_ASCII))
				fprintf(Output, "\n");
			Row += RowStep;
			} while (! RowsDone);
		if (LateFlap)
			RowFlap(xdim + XtraFlap, Output, FlapElev);
		AppMem_Free(buffer, xdim * sizeof(float));
		} // WCS_TERRAINWRITER_COLROW
		break;
	case WCS_TERRAINWRITER_ROWCOL:	// a column of rows
		{
		Col = StartCol;
		} // WCS_TERRAINWRITER_ROWCOL
		break;
	default:
		break;
	} // switch ColRowEmitOrder

return 0;

} // TerrainWriter::EmitTerrain

/*===========================================================================*/

void TerrainWriter::ReadRow(long xdim, long Row, float *buffer, FILE *RawElevs)
{
long offset = xdim * Row * sizeof(float);

fseek(RawElevs, offset, SEEK_SET);
fread((void *)buffer, xdim, sizeof(float), RawElevs);

} // TerrainWriter::ReadRow

/*===========================================================================*/

void TerrainWriter::RowFlap(long xdim, FILE *Output, float FlapElev)
{
long i;

for (i = 0; i < xdim; i++)
	EmitPoint(FlapElev, Output);

if (EmitType == WCS_TERRAINWRITER_ASCII)
	fprintf(Output, "\n");

} // TerrainWriter::ReadRow

/*===========================================================================*/

void TerrainWriter::EmitColumns(long StartCol, long FinishCol, long ColStep, float *buffer, FILE *Output, float Delta,
						   float FlapElev, long EmitFlaps, long EmitFlags)
{
float value;
long Col = StartCol, done = 0;

if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_W))
	EmitPoint(FlapElev, Output);
do
	{
	if (Col == FinishCol)
		done = 1;
	value = buffer[Col];
	EmitPoint(value + Delta, Output);
	Col += ColStep;
	} while (! done);
if (EmitFlaps && (EmitFlags & FLAPFLAG_EMIT_E))
	EmitPoint(FlapElev, Output);

} // TerrainWriter::EmitColumns

/*===========================================================================*/

void TerrainWriter::EmitPoint(float value, FILE *Output)
{
char dataorder = LITTLE_END_DATA;
float newvalue = value * EmitScale + EmitOffset;

switch (EmitType)
	{
	case WCS_TERRAINWRITER_ASCII:
		fprintf(Output, "%f,", newvalue);
		break;
	case WCS_TERRAINWRITER_BINARY:
		if (ByteOrder == WCS_TERRAINWRITER_BIGENDIAN)
			dataorder = BIG_END_DATA;
		Put32F(dataorder, &newvalue, Output);
		break;
	default:
		break;
	} // switch EmitType

} // TerrainWriter::EmitPoint

/*===========================================================================*/

/*
TerrainWriter::ReturnTriangle(long (*callback)(*float[3]), float *verts[3])
{

callback(verts);

} // TerrainWriter::ReturnTriangle
*/

/*===========================================================================*/

/*
unsigned long TerrainWriter::EmitTriangleMesh(long xdim, long ydim, long (*callback)(*float[3]), FILE *RawElevs, FILE *Output)
{
unsigned long success = 1;
long Col, Row, ColStep, RowStep, StartCol, StartRow, FinishCol, FinishRow;
long ColsDone, RowsDone;
float *buffer1 = NULL, *buffer2 = NULL;

switch (ColDirection)
	{
	case WCS_TERRAINWRITER_W2E:
		StartCol = 0;
		FinishCol = xdim - 2;
		ColStep = 1;
		break;
	case WCS_TERRAINWRITER_E2W:
		StartCol = xdim - 2;
		FinishCol = 0;
		ColStep = -1;
		break;
	default:
		break;
	} // switch ColDirection

switch (RowDirection)
	{
	case WCS_TERRAINWRITER_N2S:
		StartRow = 0;
		FinishRow = ydim - 2;
		RowStep = 1;
		break;
	case WCS_TERRAINWRITER_S2N:
		StartRow = ydim - 2;
		FinishRow = 0;
		RowStep = -1;
		break;
	default:
		break;
	} // switch RowDirection

ColsDone = RowsDone = 0;
switch (ColRowEmitOrder)
	{
	case WCS_TERRAINWRITER_COLROW:	// a row of columns
		{
		buffer1 = (float *)AppMem_Alloc(xdim * sizeof(float), 0);
		buffer2 = (float *)AppMem_Alloc(xdim * sizeof(float), 0);
		if (buffer1 && buffer2)
			{
			Row = StartRow;
			ReadRow(xdim, Row, buffer1, RawElevs);
			do
				{
				if (Row == FinishRow)
					RowsDone = 1;
				ReadRow(xdim, Row + RowStep, buffer2, RawElevs);
				//EmitColumns(StartCol, FinishCol, ColStep, buffer, Output);
				Row += RowStep;
				memcpy(buffer1, buffer2, xdim * sizeof(float));
				} while (! RowsDone);
			} // if
		else
			success = 0;
		if (buffer2)
			AppMem_Free(buffer2, xdim * sizeof(float));
		if (buffer1)
			AppMem_Free(buffer1, xdim * sizeof(float));
		} // WCS_TERRAINWRITER_COLROW
		break;
	case WCS_TERRAINWRITER_ROWCOL:	// a column of rows
		{
		buffer1 = (float *)AppMem_Alloc(ydim * sizeof(float), 0);
		buffer2 = (float *)AppMem_Alloc(ydim * sizeof(float), 0);
		Col = StartCol;
		} // WCS_TERRAINWRITER_ROWCOL
		break;
	default:
		break;
	} // switch ColRowEmitOrder

return success;

} // TerrainWriter::EmitTriangleMesh
*/

/*===========================================================================*/
