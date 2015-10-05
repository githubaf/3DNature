// DataOpsUseful.cpp
// Common code used during reading and writing of data
// Written by Frank Weed II on 9/24/99
// Copyright 1999 3D Nature

#include "stdafx.h"
#include "ImportWizGUI.h"
#include "DataOpsUseful.h"
#include "Types.h"
#include "AppMem.h"
#include "Useful.h"
#include "Log.h"
#include "DEM.h"
#include "Joe.h"
#include "Database.h"
#include "Project.h"
#include "Interactive.h"
#include "ImageOutputEvent.h"

static double Rescale;
double DataRef, OutRef, VertScale;
double DataScale, DatumChange;
static long datazip, LastOutRow, LastOutCol;
//static long InValSize;
long InputDataSize;
long LastOutputCols, LastOutputRows, OCols, ORows, OutputCols, OutputDataSize, OutputRows;
short DupRow;
static short NoScaling;
void *InputData, *OutputData;
DEM *DOTopo;
static BusyWin *BWDO;
static char RGBComponent[5] = {0,0,0,0,0};

union ShortSwap
	{
	short sht;
	SBYTE 	byt[2];
	} *ShtSwp;

union LongSwap
	{
	long 	lng;
	short sht[2];
	SBYTE 	byt[4];
	} *LngSwp;

union DoubleSwap
	{
	double dbl;
	long	 lng[2];
	short  sht[4];
	SBYTE 	 byt[8];
	} *DblSwp;

/*===========================================================================*/

void ImportWizGUI::DataOpsCleanup(short error)
{

if (error)
	DataOpsErrMsg(error);
if (OutputData)
	AppMem_Free(OutputData, (size_t)OutputDataSize);
OutputData = NULL;
Importing->OutputData = NULL;
if (InputData)
	AppMem_Free(InputData, (size_t)InputDataSize);
InputData = NULL;
Importing->InputData = NULL;
if (DOTopo)
	{
	DOTopo->RawMap = NULL;
	delete DOTopo;
	DOTopo = NULL;
	} // if
OutputData1S = InputData1S = NULL;
OutputData1U = InputData1U = NULL;
OutputData2S = InputData2S = NULL;
OutputData2U = InputData2U = NULL;
OutputData4S = InputData4S = NULL;
OutputData4U = InputData4U = NULL;
OutputData4F = InputData4F = NULL;
OutputData8F = InputData8F = NULL;

} // ImportWizGUI::DataOpsCleanup

/*===========================================================================*/

short ImportWizGUI::DataOpsFloor(void)
{
short error = 0;
USHORT i, j;

BWDO = new BusyWin("Floor", (ULONG)INPUT_ROWS, 'BWDO', 0);
datazip = 0;
for (i=0; i<INPUT_ROWS; i++)
	{
	for (j=0; j<INPUT_COLS; j++)
		{
		switch (INVALUE_SIZE)
			{
			default:
				break;
			case DEM_DATA_VALSIZE_BYTE:
				{
				switch (INVALUE_FORMAT)
					{
					default:
						break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData1U[datazip] < INPUT_FLOOR)
							InputData1U[datazip] = (UBYTE)INPUT_FLOOR;
						break;
						} // unsigned byte
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData1S[datazip] < INPUT_FLOOR)
							InputData1S[datazip] = (SBYTE)INPUT_FLOOR;
						break;
						} // unsigned byte
					} // switch value format
				break;
				} // byte
			case DEM_DATA_VALSIZE_SHORT:
				{
				switch (INVALUE_FORMAT)
					{
					default:
						break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData2U[datazip] < INPUT_FLOOR)
							InputData2U[datazip] = (USHORT)INPUT_FLOOR;
						break;
						} // unsigned short
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData2S[datazip] < INPUT_FLOOR)
							InputData2S[datazip] = (SHORT)INPUT_FLOOR;
						break;
						} // signed short
					} // switch value format
				break;
				} // short
			case DEM_DATA_VALSIZE_LONG:
				{
				switch (INVALUE_FORMAT)
					{
					default:
						break;
					case DEM_DATA_FORMAT_FLOAT:
						{
						if (InputData4F[datazip] < INPUT_FLOOR)
							InputData4F[datazip] = (float)INPUT_FLOOR;
						break;
						} // floating point long
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData4U[datazip] < INPUT_FLOOR)
							InputData4U[datazip] = (ULONG)INPUT_FLOOR;
						break;
						} // unsigned long integer
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData4S[datazip] < INPUT_FLOOR)
							InputData4S[datazip] = (LONG)INPUT_FLOOR;
						break;
						} // signed long integer
					} // switch value format
				break;
				} // long word
			case DEM_DATA_VALSIZE_DOUBLE:
				{
				if (InputData8F[datazip] < INPUT_FLOOR)
					InputData8F[datazip] = INPUT_FLOOR;
				break;
				} // double
			} // switch input value size
		datazip ++;
		} // for j=0
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			break;
			} // if user abort
		BWDO->Update((ULONG)(i + 1));
		} // if
	} // for i=0
if (BWDO)
	delete BWDO;

return error;

} // ImportWizGUI::DataOpsFloor

/*===========================================================================*/

short ImportWizGUI::DataOpsCeiling(void)
{
short error = 0;
USHORT i, j;

BWDO = new BusyWin("Ceiling", (ULONG)INPUT_ROWS, 'BWDO', 0);
datazip = 0;
for (i=0; i<INPUT_ROWS; i++)
	{
	for (j=0; j<INPUT_COLS; j++)
		{
		switch (INVALUE_SIZE)
			{
			default: break;
			case DEM_DATA_VALSIZE_BYTE:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData1U[datazip] > INPUT_CEILING)
							InputData1U[datazip] = (UBYTE)INPUT_CEILING;
						break;
						} // unsigned byte
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData1S[datazip] > INPUT_CEILING)
							InputData1S[datazip] = (SBYTE)INPUT_CEILING;
						break;
						} // unsigned byte
					} // switch value format
				break;
				} // byte
			case DEM_DATA_VALSIZE_SHORT:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData2U[datazip] > INPUT_CEILING)
							InputData2U[datazip] = (USHORT)INPUT_CEILING;
						break;
						} // unsigned short
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData2S[datazip] > INPUT_CEILING)
							InputData2S[datazip] = (SHORT)INPUT_CEILING;
						break;
						} // signed short
					} // switch value format
				break;
				} // short
			case DEM_DATA_VALSIZE_LONG:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_FLOAT:
						{
						if (InputData4F[datazip] > INPUT_CEILING)
							InputData4F[datazip] = (float)INPUT_CEILING;
						break;
						} // floating point long
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (InputData4U[datazip] > INPUT_CEILING)
							InputData4U[datazip] = (ULONG)INPUT_CEILING;
						break;
						} // unsigned long integer
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (InputData4S[datazip] > INPUT_CEILING)
							InputData4S[datazip] = (LONG)INPUT_CEILING;
						break;
						} // signed long integer
					} // switch value format
				break;
				} // long word
			case DEM_DATA_VALSIZE_DOUBLE:
				{
				if (InputData8F[datazip] > INPUT_CEILING)
					InputData8F[datazip] = INPUT_CEILING;
				break;
				} // double
			} // switch input value size
		datazip ++;
		} // for j
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			break;
			} // if user abort
		BWDO->Update((ULONG)(i + 1));
		} // if
	} // for i
if (BWDO)
	delete BWDO;

return error;

} // ImportWizGUI::DataOpsCeiling

/*===========================================================================*/

short ImportWizGUI::DataOpsNull2Min(void)
{
double gnumin;
short error = 0;
USHORT i, j;

// Set NULL data to minimum minus 1% of elevation range
gnumin = Importing->TestMin - (Importing->TestMax - Importing->TestMin) / 100.0;
#ifdef WCS_BUILD_GARY
gnumin = WCS_floor(Importing->NullVal);
#endif // WCS_BUILD_GARY

BWDO = new BusyWin("Null->Min", (ULONG)INPUT_ROWS, 'BWDO', 0);
datazip = 0;
for (i=0; i<INPUT_ROWS; i++)
	{
	for (j=0; j<INPUT_COLS; j++)
		{
		switch (INVALUE_SIZE)
			{
			default:
				break;
			case DEM_DATA_VALSIZE_BYTE:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData1U[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData1U[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData1U[datazip] <= Importing->NullVal)))
							InputData1U[datazip] = (UBYTE)gnumin;
						break;
						} // unsigned byte
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData1S[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData1S[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData1S[datazip] <= Importing->NullVal)))
							InputData1S[datazip] = (SBYTE)gnumin;
						break;
						} // signed byte
					} // switch value format
				break;
				} // byte
			case DEM_DATA_VALSIZE_SHORT:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData2U[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData2U[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData2U[datazip] <= Importing->NullVal)))
							InputData2U[datazip] = (USHORT)gnumin;
						break;
						} // unsigned short
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData2S[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData2S[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData2S[datazip] <= Importing->NullVal)))
							InputData2S[datazip] = (SHORT)gnumin;
						break;
						} // signed short
					} // switch value format
				break;
				} // short
			case DEM_DATA_VALSIZE_LONG:
				{
				switch (INVALUE_FORMAT)
					{
					default: break;
					case DEM_DATA_FORMAT_FLOAT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4F[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4F[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4F[datazip] <= Importing->NullVal)))
							InputData4F[datazip] = (float)gnumin;
						break;
						} // floating point long
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4U[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4U[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4U[datazip] <= Importing->NullVal)))
							InputData4U[datazip] = (ULONG)gnumin;
						break;
						} // unsigned long integer
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4S[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4S[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4S[datazip] <= Importing->NullVal)))
							InputData4S[datazip] = (LONG)gnumin;
						break;
						} // signed long integer
					} // switch value format
				break;
				} // long word
			case DEM_DATA_VALSIZE_DOUBLE:
				{
				if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData8F[datazip] == Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData8F[datazip] >= Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData8F[datazip] <= Importing->NullVal)))
					InputData8F[datazip] = gnumin;
				break;
				} // double
			} // switch input value size
		datazip ++;
		} // for j
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			break;
			} // if user abort
		BWDO->Update((ULONG)(i + 1));
		} // if
	} // for i
if (BWDO)
	delete BWDO;

// set the appropriate NULL values
switch (INVALUE_SIZE)
	{
	case DEM_DATA_VALSIZE_BYTE:
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				Importing->SaveNull = (UBYTE)gnumin;
				break; // unsigned byte
			case DEM_DATA_FORMAT_SIGNEDINT:
				Importing->SaveNull = (SBYTE)gnumin;
				break; // signed byte
			default: break;
			} // switch value format
		break; // byte
	case DEM_DATA_VALSIZE_SHORT:
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				Importing->SaveNull = (USHORT)gnumin;
				break; // unsigned short
			case DEM_DATA_FORMAT_SIGNEDINT:
				Importing->SaveNull = (SHORT)gnumin;
				break; // signed short
			default: break;
			} // switch value format
		break; // short
	case DEM_DATA_VALSIZE_LONG:
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				Importing->SaveNull = (float)gnumin;
				break; // floating point long
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				Importing->SaveNull = (ULONG)gnumin;
				break; // unsigned long integer
			case DEM_DATA_FORMAT_SIGNEDINT:
				Importing->SaveNull = (LONG)gnumin;
				break; // signed long integer
			default: break;
			} // switch value format
		break; // long word
	case DEM_DATA_VALSIZE_DOUBLE:
		Importing->SaveNull = gnumin;
		break; // double
	default: break;
	} // switch input value size

return error;

} // ImportWizGUI::DataOpsNull2Min

/*===========================================================================*/

void ImportWizGUI::DataOpsInit(void)
{

OutputData1S = InputData1S = NULL;
OutputData1U = InputData1U = NULL;
OutputData2S = InputData2S = NULL;
OutputData2U = InputData2U = NULL;
OutputData4S = InputData4S = NULL;
OutputData4U = InputData4U = NULL;
OutputData4F = InputData4F = NULL;
OutputData8F = InputData8F = NULL;

} // ImportWizGUI::DataOpsInit

/*===========================================================================*/

short ImportWizGUI::DataOpsInvert(void)
{
short error = 0;
USHORT i, j;

BWDO = new BusyWin("Inverting", (ULONG)INPUT_ROWS, 'BWDO', 0);
datazip = 0;
for (i=0; i<INPUT_ROWS; i++)
	{
	for (j=0; j<INPUT_COLS; j++)
		{
		switch (INVALUE_SIZE)
			{
			default: break;
			case DEM_DATA_VALSIZE_SHORT:
				{
				swmem(&ShtSwp[datazip].byt[0], &ShtSwp[datazip].byt[1], 1);
				break;
				} // short
			case DEM_DATA_VALSIZE_LONG:
				{
				swmem(&LngSwp[datazip].byt[0], &LngSwp[datazip].byt[3], 1);
				swmem(&LngSwp[datazip].byt[1], &LngSwp[datazip].byt[2], 1);
				break;
				} // long
			case DEM_DATA_VALSIZE_DOUBLE:
				{
				swmem(&DblSwp[datazip].byt[0], &DblSwp[datazip].byt[7], 1);
				swmem(&DblSwp[datazip].byt[1], &DblSwp[datazip].byt[6], 1);
				swmem(&DblSwp[datazip].byt[2], &DblSwp[datazip].byt[5], 1);
				swmem(&DblSwp[datazip].byt[3], &DblSwp[datazip].byt[4], 1);
				break;
				} // double
			} // switch value size
		datazip ++;
		} // for j
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			break;
			} // if user abort
		BWDO->Update((ULONG)(i + 1));
		} // if
	} // for i
if (BWDO)
	delete BWDO;

return error;

} // ImportWizGUI::DataOpsInvert

/*===========================================================================*/

short ImportWizGUI::DataOpsResample(void)
{
double RowStep, ColStep, RowDelta, ColDelta, TP[4],
	P0 = 0.0, P1, P2, P3 = 0.0, D1, D2, S1, S2, S3, h1, h2, h3, h4;
long RowBase[4], LastInRow, LastInCol, FirstInRow, FirstInCol, CurRow[4], CurCol[4];
long i, j, k;
int error = 0, P0Null = 0, P1Null = 0, P2Null = 0, P3Null = 0, ConstrainTest = 1;

//OutputData = NULL;
//ORows = OUTPUT_ROWS;
//OCols = OUTPUT_COLS;
OutputDataSize = ORows * OCols;

//lint -save -e527
switch (INVALUE_SIZE)
	{
	default: break;
	case DEM_DATA_VALSIZE_BYTE:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				if ((OutputData1U = (UBYTE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					} // if out of memory
				OutputData = (UBYTE *)OutputData1U;
				break;
				} // unsigned byte
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				if ((OutputData1S = (SBYTE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					} // if out of memory
				OutputData = (SBYTE *)OutputData1S;
				break;
				} // signed byte
			default:
				{
				error = 11;
				goto Cleanup;
				break;
				} // floating point byte or unknown format
			} // switch value format
		break;
		} // byte
	case DEM_DATA_VALSIZE_SHORT:
		{
		OutputDataSize *= 2;
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				if ((OutputData2U = (USHORT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (USHORT *)OutputData2U;
				break;
				} // unsigned short
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				if ((OutputData2S = (SHORT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (SHORT *)OutputData2S;
				break;
				} // signed short
			default:
				{
				error = 11;
				goto Cleanup;
				break;
				} // floating point short or unknown format
			} // switch value format
		break;
		} // short
	case DEM_DATA_VALSIZE_LONG:
		{
		OutputDataSize *= 4;
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				{
				if ((OutputData4F = (FLOAT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (FLOAT *)OutputData4F;
				break;
				} // floating point long
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				if ((OutputData4U = (ULONG *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (ULONG *)OutputData4U;
				break;
				} // unsigned long
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				if ((OutputData4S = (LONG *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (LONG *)OutputData4S;
				break;
				} // signed long
			default:
				{
				error = 11;
				goto Cleanup;
				break;
				} // unknown format
			} // switch value format
		break;
		} // long
	case DEM_DATA_VALSIZE_DOUBLE:
		{
		OutputDataSize *= 8;
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				{
				if ((OutputData8F = (DOUBLE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
					{
					error = 1;
					goto Cleanup;
					}
				OutputData = (double *)OutputData8F;
				break;
				} // floating point double
			default:
				{
				error = 11;
				goto Cleanup;
				break;
				} // integer double or unknown format
			} // switch value format
		break;
		} // double
	} // switch value size
//lint -restore

RowStep = (double)(CROP_ROWS - 1) / (double)(ORows - 1);
ColStep = (double)(CROP_COLS - 1) / (double)(OCols - 1);
if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM || INPUT_FORMAT == DEM_DATA2_INPUT_DTED ||
	INPUT_FORMAT == DEM_DATA2_INPUT_MDEM || INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
	{
	LastInRow = INPUT_ROWS - CROP_RIGHT - 1;
	LastInCol = INPUT_COLS - CROP_TOP - 1;
	FirstInRow = CROP_LEFT;
	FirstInCol = CROP_BOTTOM;
	}
else
	{
	LastInRow = INPUT_ROWS - CROP_BOTTOM - 1;
	LastInCol = INPUT_COLS - CROP_RIGHT - 1;
	FirstInRow = CROP_TOP;
	FirstInCol = CROP_LEFT;
	} // else
LastOutRow = ORows - 1;
LastOutCol = OCols - 1;

BWDO = new BusyWin("Resample", (ULONG)ORows, 'BWDO', 0);
//	for (i=0; i<OCols; i++)
for (i=0; i<ORows; i++)
	{
	if (i == LastOutRow)
		CurRow[0] = CurRow[1] = CurRow[2] = CurRow[3] = LastInRow;
	else
		CurRow[0] = CurRow[1] = CurRow[2] = CurRow[3] = (long)(FirstInRow + i * RowStep);
	CurRow[0] --;
	CurRow[2] ++;
	CurRow[3] += 2;
	RowDelta = i * RowStep - (double)(CurRow[1] - FirstInRow);
	RowBase[1] = CurRow[1] * INPUT_COLS;
	RowBase[0] = RowBase[1] - INPUT_COLS;
	RowBase[2] = RowBase[1] + INPUT_COLS;
	RowBase[3] = RowBase[2] + INPUT_COLS;

	if (i == 0 || i == LastOutRow || RowDelta == 0.0)
		{
		for (j=0; j<OCols; j++)
			{
			if (j == LastOutCol)
				CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = LastInCol;
			else
				CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = (long)(FirstInCol + j * ColStep);
			CurCol[0] --;
			CurCol[2] ++;
			CurCol[3] += 2;
			ColDelta = j * ColStep - (double)(CurCol[1] - FirstInCol);
			if (j == 0 || j == LastOutCol || ColDelta == 0.0)
				{
				switch (INVALUE_SIZE)
					{
					default: break;
					case DEM_DATA_VALSIZE_BYTE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData1U[i * OCols + j] =
								InputData1U[RowBase[1] + CurCol[1]];
								break;
								} // unsigned byte
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData1S[i * OCols + j] =
								InputData1S[RowBase[1] + CurCol[1]];
								break;
								} // signed byte
							} // switch value format
						break;
						} // byte
					case DEM_DATA_VALSIZE_SHORT:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData2U[i * OCols + j] =
								InputData2U[RowBase[1] + CurCol[1]];
								break;
								} // unsigned short
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData2S[i * OCols + j] =
								InputData2S[RowBase[1] + CurCol[1]];
								break;
								} // signed short
							} // switch value format
						break;
						} // short
					case DEM_DATA_VALSIZE_LONG:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								OutputData4F[i * OCols + j] =
								InputData4F[RowBase[1] + CurCol[1]];
								break;
								} // floating point long
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData4U[i * OCols + j] =
								InputData4U[RowBase[1] + CurCol[1]];
								break;
								} // unsigned long
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData4S[i * OCols + j] =
								InputData4S[RowBase[1] + CurCol[1]];
								break;
								} // signed long
							} // switch value format
						break;
						} // long
					case DEM_DATA_VALSIZE_DOUBLE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								OutputData8F[i * OCols + j] =
								InputData8F[RowBase[1] + CurCol[1]];
								break;
								} // floating point double
							} // switch value format
						break;
						} // double
					} // switch value size
				} // set value directly
			else
				{
				switch (INVALUE_SIZE)
					{
					default: break;
					case DEM_DATA_VALSIZE_BYTE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								P1 = InputData1U[RowBase[1] + CurCol[1]];
								P2 = InputData1U[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData1U[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData1U[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // unsigned byte
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								P1 = InputData1S[RowBase[1] + CurCol[1]];
								P2 = InputData1S[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData1S[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData1S[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // signed byte
							} // switch value format
						break;
						} // byte
					case DEM_DATA_VALSIZE_SHORT:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								P1 = InputData2U[RowBase[1] + CurCol[1]];
								P2 = InputData2U[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData2U[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData2U[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // unsigned short
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								P1 = InputData2S[RowBase[1] + CurCol[1]];
								P2 = InputData2S[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData2S[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData2S[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // signed short
							} // switch value format
						break;
						} // short
					case DEM_DATA_VALSIZE_LONG:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								P1 = InputData4F[RowBase[1] + CurCol[1]];
								P2 = InputData4F[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData4F[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData4F[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // floating point long
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								P1 = InputData4U[RowBase[1] + CurCol[1]];
								P2 = InputData4U[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData4U[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData4U[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // unsigned long
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								P1 = InputData4S[RowBase[1] + CurCol[1]];
								P2 = InputData4S[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData4S[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData4S[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // signed long
							} // switch value format
						break;
						} // long
					case DEM_DATA_VALSIZE_DOUBLE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								P1 = InputData8F[RowBase[1] + CurCol[1]];
								P2 = InputData8F[RowBase[1] + CurCol[2]];
								if (CurCol[0] >= FirstInCol)
									{
									P0 = InputData8F[RowBase[1] + CurCol[0]];
									} // if
								if (CurCol[3] <= LastInCol)
									{
									P3 = InputData8F[RowBase[1] + CurCol[3]];
									} // if
								break;
								} // floating point double
							} // switch value format
						break;
						} // double
					} // switch value size

				if (Importing->HasNulls)
					{
					P0Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P0 == Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_GE) && (P0 >= Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_LE) && (P0 <= Importing->NullVal)));
					P1Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P1 == Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_GE) && (P1 >= Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_LE) && (P1 <= Importing->NullVal)));
					P2Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P2 == Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_GE) && (P2 >= Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_LE) && (P2 <= Importing->NullVal)));
					P3Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P3 == Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_GE) && (P3 >= Importing->NullVal)) ||
						((Importing->NullMethod == IMWIZ_NULL_LE) && (P3 <= Importing->NullVal)));
					ConstrainTest = 1;
					} // if

				if (! (P1Null || P2Null))
					{
					D1 = CurCol[0] >= FirstInCol && ! P0Null ? (.5 * (P2 - P0)): (P2 - P1);
					D2 = CurCol[3] <= LastInCol && ! P3Null ? (.5 * (P3 - P1)): (P2 - P1);
					S1 = ColDelta;
					S2 = S1 * S1;
					S3 = S1 * S2;
					h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
					h2 = -2.0 * S3 + 3.0 * S2;
					h3 = S3 - 2.0 * S2 + S1;
					h4 = S3 - S2;
					TP[0] = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
					} // if
				else
					{
					TP[0] = Importing->NullVal;
					ConstrainTest = 0;
					} // else
				if (SPLINE_CONSTRAIN && ConstrainTest)
					{
					if (TP[0] > P1 && TP[0] > P2)
						TP[0] = max(P1, P2);
					else if (TP[0] < P1 && TP[0] < P2)
						TP[0] = min(P1, P2);
					} // if spline constraint
				switch (INVALUE_SIZE)
					{
					default: break;
					case DEM_DATA_VALSIZE_BYTE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData1U[i * OCols + j] = (UBYTE)TP[0];
								break;
								} // unsigned byte
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData1S[i * OCols + j] = (SBYTE)TP[0];
								break;
								} // signed byte
							} // switch value format
						break;
						} // byte
					case DEM_DATA_VALSIZE_SHORT:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData2U[i * OCols + j] = (USHORT)TP[0];
								break;
								} // unsigned short
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData2S[i * OCols + j] = (SHORT)TP[0];
								break;
								} // signed short
							} // switch value format
						break;
						} // short
					case DEM_DATA_VALSIZE_LONG:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								OutputData4F[i * OCols + j] = (FLOAT)TP[0];
								break;
								} // floating point long
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								OutputData4U[i * OCols + j] = (ULONG)TP[0];
								break;
								} // unsigned long
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								OutputData4S[i * OCols + j] = (LONG)TP[0];
								break;
								} // signed long
							} // switch value format
						break;
						} // long
					case DEM_DATA_VALSIZE_DOUBLE:
						{
						switch (INVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								OutputData8F[i * OCols + j] = TP[0];
								break;
								} // floating point double
							} // switch value format
						break;
						} // double
					} // switch value size */
				} // else compute splined value
			} // for j
		} // if only one row needed, compute final values directly
	else
		{
		for (j=0; j<OCols; j++)
			{
			if (j == LastOutCol)
				CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = LastInCol;
			else
				CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = (long)(FirstInCol + j * ColStep);
			CurCol[0] --;
			CurCol[2] ++;
			CurCol[3] += 2;
			ColDelta = j * ColStep - (double)(CurCol[1] - FirstInCol);

			for (k=0; k<4; k++)
				{
				if (CurRow[k] < FirstInRow) continue;
				if (CurRow[k] > LastInRow) break;

				if (j == 0 || j == LastOutCol || ColDelta == 0.0)
					{
					switch (INVALUE_SIZE)
						{
						default: break;
						case DEM_DATA_VALSIZE_BYTE:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									TP[k] = InputData1U[RowBase[k] + CurCol[1]];
									break;
									} // unsigned byte
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									TP[k] = InputData1S[RowBase[k] + CurCol[1]];
									break;
									} // signed byte
								} // switch value format
							break;
							} // byte
						case DEM_DATA_VALSIZE_SHORT:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									TP[k] = InputData2U[RowBase[k] + CurCol[1]];
									break;
									} // unsigned short
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									TP[k] = InputData2S[RowBase[k] + CurCol[1]];
									break;
									} // signed short
								} // switch value format
							break;
							} // short
						case DEM_DATA_VALSIZE_LONG:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_FLOAT:
									{
									TP[k] = InputData4F[RowBase[k] + CurCol[1]];
									break;
									} // floating point long
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									TP[k] = InputData4U[RowBase[k] + CurCol[1]];
									break;
									} // unsigned long
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									TP[k] = InputData4S[RowBase[k] + CurCol[1]];
									break;
									} // signed long
								} // switch value format
							break;
							} // long
						case DEM_DATA_VALSIZE_DOUBLE:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_FLOAT:
									{
									TP[k] = InputData8F[RowBase[k] + CurCol[1]];
									break;
									} // floating point double
								} // switch value format
							break;
							} // double
						} // switch value size
					} // set value directly
				else
					{
					switch (INVALUE_SIZE)
						{
						default: break;
						case DEM_DATA_VALSIZE_BYTE:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									P1 = InputData1U[RowBase[k] + CurCol[1]];
									P2 = InputData1U[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData1U[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData1U[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // unsigned byte
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									P1 = InputData1S[RowBase[k] + CurCol[1]];
									P2 = InputData1S[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData1S[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData1S[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // signed byte
								} // switch value format
							break;
							} // byte
						case DEM_DATA_VALSIZE_SHORT:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									P1 = InputData2U[RowBase[k] + CurCol[1]];
									P2 = InputData2U[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData2U[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData2U[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // unsigned short
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									P1 = InputData2S[RowBase[k] + CurCol[1]];
									P2 = InputData2S[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData2S[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData2S[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // signed short
								} // switch value format
							break;
							} // short
						case DEM_DATA_VALSIZE_LONG:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_FLOAT:
									{
									P1 = InputData4F[RowBase[k] + CurCol[1]];
									P2 = InputData4F[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData4F[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData4F[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // floating point long
								case DEM_DATA_FORMAT_UNSIGNEDINT:
									{
									P1 = InputData4U[RowBase[k] + CurCol[1]];
									P2 = InputData4U[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData4U[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData4U[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // unsigned long
								case DEM_DATA_FORMAT_SIGNEDINT:
									{
									P1 = InputData4S[RowBase[k] + CurCol[1]];
									P2 = InputData4S[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData4S[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData4S[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // signed long
								} // switch value format
							break;
							} // long
						case DEM_DATA_VALSIZE_DOUBLE:
							{
							switch (INVALUE_FORMAT)
								{
								default: break;
								case DEM_DATA_FORMAT_FLOAT:
									{
									P1 = InputData8F[RowBase[k] + CurCol[1]];
									P2 = InputData8F[RowBase[k] + CurCol[2]];
									if (CurCol[0] >= FirstInCol)
										{
										P0 = InputData8F[RowBase[k] + CurCol[0]];
										} // if
									if (CurCol[3] <= LastInCol)
										{
										P3 = InputData8F[RowBase[k] + CurCol[3]];
										} // if
									break;
									} // floating point double
								} // switch value format
							break;
							} // double
						} // switch value size

					if (Importing->HasNulls)
						{
						P0Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P0 == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (P0 >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (P0 <= Importing->NullVal)));
						P1Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P1 == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (P1 >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (P1 <= Importing->NullVal)));
						P2Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P2 == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (P2 >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (P2 <= Importing->NullVal)));
						P3Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (P3 == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (P3 >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (P3 <= Importing->NullVal)));
						ConstrainTest = 1;
						} // if

					if (! (P1Null || P2Null))
						{
						D1 = CurCol[0] >= FirstInCol && ! P0Null ? (.5 * (P2 - P0)): (P2 - P1);
						D2 = CurCol[3] <= LastInCol && ! P3Null ? (.5 * (P3 - P1)): (P2 - P1);
						S1 = ColDelta;
						S2 = S1 * S1;
						S3 = S1 * S2;
						h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
						h2 = -2.0 * S3 + 3.0 * S2;
						h3 = S3 - 2.0 * S2 + S1;
						h4 = S3 - S2;
						TP[k] = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
						} // if
					else
						{
						TP[0] = Importing->NullVal;
						ConstrainTest = 0;
						} // else
					if (SPLINE_CONSTRAIN && ConstrainTest)
						{
						if (TP[k] > P1 && TP[k] > P2)
							TP[k] = max(P1, P2);
						else if (TP[k] < P1 && TP[k] < P2)
							TP[k] = min(P1, P2);
						} // if spline constraint
					} // else compute splined value
				} // for k

			if (Importing->HasNulls)
				{
				P0Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (TP[0] == Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_GE) && (TP[0] >= Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_LE) && (TP[0] <= Importing->NullVal)));
				P1Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (TP[1] == Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_GE) && (TP[1] >= Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_LE) && (TP[1] <= Importing->NullVal)));
				P2Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (TP[2] == Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_GE) && (TP[2] >= Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_LE) && (TP[2] <= Importing->NullVal)));
				P3Null = (((Importing->NullMethod == IMWIZ_NULL_EQ) && (TP[3] == Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_GE) && (TP[3] >= Importing->NullVal)) ||
					((Importing->NullMethod == IMWIZ_NULL_LE) && (TP[3] <= Importing->NullVal)));
				ConstrainTest = 1;
				} // if

			if (! (P1Null || P2Null))
				{
				D1 = CurRow[0] >= FirstInRow && ! P0Null ? (.5 * (TP[2] - TP[0])): (TP[2] - TP[1]);
				D2 = CurRow[3] <= LastInRow && ! P3Null ? (.5 * (TP[3] - TP[1])): (TP[2] - TP[1]);
				S1 = RowDelta;
				S2 = S1 * S1;
				S3 = S1 * S2;
				h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
				h2 = -2.0 * S3 + 3.0 * S2;
				h3 = S3 - 2.0 * S2 + S1;
				h4 = S3 - S2;
				TP[0] = TP[1] * h1 + TP[2] * h2 + D1 * h3 + D2 * h4;
				} // if
			else
				{
				TP[0] = Importing->NullVal;
				ConstrainTest = 0;
				} // else
			if (SPLINE_CONSTRAIN && ConstrainTest)
				{
				if (TP[0] > TP[1] && TP[0] > TP[2])
					TP[0] = max(TP[1], TP[2]);
				else if (TP[0] < TP[1] && TP[0] < TP[2])
					TP[0] = min(TP[1], TP[2]);
				} // if spline constraint
			switch (INVALUE_SIZE)
				{
				default: break;
				case DEM_DATA_VALSIZE_BYTE:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							OutputData1U[i * OCols + j] = (UBYTE)TP[0];
							break;
							} // unsigned byte
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							OutputData1S[i * OCols + j] = (SBYTE)TP[0];
							break;
							} // signed byte
						} // switch value format
					break;
					} // byte
				case DEM_DATA_VALSIZE_SHORT:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							OutputData2U[i * OCols + j] = (USHORT)TP[0];
							break;
							} // unsigned short
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							OutputData2S[i * OCols + j] = (SHORT)TP[0];
							break;
							} // signed short
						} // switch value format
					break;
					} // short
				case DEM_DATA_VALSIZE_LONG:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_FLOAT:
							{
							OutputData4F[i * OCols + j] = (FLOAT)TP[0];
							break;
							} // floating point long
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							OutputData4U[i * OCols + j] = (ULONG)TP[0];
							break;
							} // unsigned long
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							OutputData4S[i * OCols + j] = (LONG)TP[0];
							break;
							} // signed long
						} // switch value format
					break;
					} // long
				case DEM_DATA_VALSIZE_DOUBLE:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_FLOAT:
							{
							OutputData8F[i * OCols + j] = TP[0];
							break;
							} // floating point double
						} // switch value format
					break;
					} // double
				} // switch value size
			} // for j
		} // else compute four rows of spline pts as intermediates, then final values
	if (BWDO)
		{
		if (BWDO->CheckAbort())
			{
			error = 50;
			break;
			} // if user abort
		BWDO->Update((ULONG)(i + 1));
		} // if
	} // for i
if (BWDO)
	delete BWDO;

if (error)
	goto Cleanup;

AppMem_Free(InputData, InputDataSize);

switch (INVALUE_SIZE)
	{
	case DEM_DATA_VALSIZE_BYTE:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				InputData1U = OutputData1U;
				OutputData1U = NULL;
				break;
				} // unsigned byte
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				InputData1S = OutputData1S;
				OutputData1S = NULL;
				break;
				} // signed byte
			default:
				break;
			} // switch value format
		break;
		} // byte
	case DEM_DATA_VALSIZE_SHORT:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				InputData2U = OutputData2U;
				OutputData2U = NULL;
				break;
				} // unsigned short
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				InputData2S = OutputData2S;
				OutputData2S = NULL;
				break;
				} // signed short
			default:
				break;
			} // switch value format
		break;
		} // short
	case DEM_DATA_VALSIZE_LONG:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				{
				InputData4F = OutputData4F;
				OutputData4F = NULL;
				break;
				} // floating point long
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				InputData4U = OutputData4U;
				OutputData4U = NULL;
				break;
				} // unsigned long
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				InputData4S = OutputData4S;
				OutputData4S = NULL;
				break;
				} // signed long
			default:
				break;
			} // switch value format
		break;
		} // long
	case DEM_DATA_VALSIZE_DOUBLE:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				{
				InputData8F = OutputData8F;
				OutputData8F = NULL;
				break;
				} // floating point double
			default:
				break;
			} // switch value format
		break;
		} // double
	default:
		break;
	} // switch value size

InputDataSize = OutputDataSize;
InputData = OutputData;
OutputData = NULL;
INPUT_ROWS = ORows;
INPUT_COLS = OCols;
CROP_LEFT = CROP_RIGHT = CROP_TOP = CROP_BOTTOM = 0;

Cleanup:
return ((short)error);

} // ImportWizGUI::DataOpsResample

/*===========================================================================*/

// figure out scaling constants - applied later
short ImportWizGUI::DataOpsScale(void)
{
double DataMaxEl = -FLT_MAX, DataMinEl = FLT_MAX;
unsigned short i, j;
short error = 0;

//if (Importing->TestOnly || SCALEOP == DEM_DATA_SCALEOP_MAXMINSCALE ||
if (SCALEOP == DEM_DATA_SCALEOP_MAXMINSCALE ||
	SCALEOP == DEM_DATA_SCALEOP_UNIFIED ||
	(SCALEOP == DEM_DATA_SCALEOP_MATCHSCALE &&
	(SCALETYPE == DEM_DATA_SCALETYPE_MAXEL ||
	SCALETYPE == DEM_DATA_SCALETYPE_MINEL)))
	{
	long RightEdge, BottomEdge, LeftEdge, TopEdge, RowIncr;

	if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM || INPUT_FORMAT == DEM_DATA2_INPUT_DTED ||
		INPUT_FORMAT == DEM_DATA2_INPUT_MDEM || INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
		{
		RightEdge = INPUT_COLS - CROP_TOP;
		BottomEdge = INPUT_ROWS - CROP_RIGHT;
		LeftEdge = CROP_BOTTOM;
		TopEdge = CROP_LEFT;
		RowIncr = CROP_TOP;
		} // if
	else
		{
		RightEdge = INPUT_COLS - CROP_RIGHT;
		BottomEdge = INPUT_ROWS - CROP_BOTTOM;
		LeftEdge = CROP_LEFT;
		TopEdge = CROP_TOP;
		RowIncr = CROP_RIGHT;
		} // else

	datazip = 0;
	BWDO = new BusyWin("Extrema", (ULONG)BottomEdge, 'BWDO', 0);
	for (i=0; i<BottomEdge; i++)
		{
		if (i < TopEdge)
			{
			datazip += INPUT_COLS;
			continue;
			} // if cropped top
		for (j=0; j<RightEdge; j++)
			{
			if (j < LeftEdge)
				{
				datazip ++;
				continue;
				} // if cropped top
			switch (INVALUE_SIZE)
				{
				default: break;
				case DEM_DATA_VALSIZE_BYTE:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData1U[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData1U[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData1U[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData1U[datazip] > DataMaxEl)
								DataMaxEl = InputData1U[datazip];
							if (InputData1U[datazip] < DataMinEl)
								DataMinEl = InputData1U[datazip];
							break;
							} // unsigned byte
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData1S[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData1S[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData1S[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData1S[datazip] > DataMaxEl)
								DataMaxEl = InputData1S[datazip];
							if (InputData1S[datazip] < DataMinEl)
								DataMinEl = InputData1S[datazip];
							break;
							} // unsigned byte
						} // switch value format
					break;
					} // byte
				case DEM_DATA_VALSIZE_SHORT:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData2U[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData2U[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData2U[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData2U[datazip] > DataMaxEl)
								DataMaxEl = InputData2U[datazip];
							if (InputData2U[datazip] < DataMinEl)
								DataMinEl = InputData2U[datazip];
							break;
							} // unsigned short
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData2S[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData2S[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData2S[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData2S[datazip] > DataMaxEl)
								DataMaxEl = InputData2S[datazip];
							if (InputData2S[datazip] < DataMinEl)
								DataMinEl = InputData2S[datazip];
							break;
							} // signed short
						} // switch value format
					break;
					} // short
				case DEM_DATA_VALSIZE_LONG:
					{
					switch (INVALUE_FORMAT)
						{
						default: break;
						case DEM_DATA_FORMAT_FLOAT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4F[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4F[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4F[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData4F[datazip] > DataMaxEl)
								DataMaxEl = InputData4F[datazip];
							if (InputData4F[datazip] < DataMinEl)
								DataMinEl = InputData4F[datazip];
							break;
							} // floating point long
						case DEM_DATA_FORMAT_UNSIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4U[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4U[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4U[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData4U[datazip] > DataMaxEl)
								DataMaxEl = (double)InputData4U[datazip];
							if (InputData4U[datazip] < DataMinEl)
								DataMinEl = (double)InputData4U[datazip];
							break;
							} // unsigned long integer
						case DEM_DATA_FORMAT_SIGNEDINT:
							{
							if (Importing->HasNulls)
								{
								if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData4S[datazip] == Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData4S[datazip] >= Importing->NullVal)) ||
									((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData4S[datazip] <= Importing->NullVal)))
									break;
								}
							if (InputData4S[datazip] > DataMaxEl)
								DataMaxEl = (double)InputData4S[datazip];
							if (InputData4S[datazip] < DataMinEl)
								DataMinEl = (double)InputData4S[datazip];
							break;
							} // signed long integer */
						} // switch value format */
					break;
					} // long word */
				case DEM_DATA_VALSIZE_DOUBLE:
					{
					if (Importing->HasNulls)
						{
						if (((Importing->NullMethod == IMWIZ_NULL_EQ) && (InputData8F[datazip] == Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_GE) && (InputData8F[datazip] >= Importing->NullVal)) ||
							((Importing->NullMethod == IMWIZ_NULL_LE) && (InputData8F[datazip] <= Importing->NullVal)))
							break;
						}
					if (InputData8F[datazip] > DataMaxEl)
						DataMaxEl = InputData8F[datazip];
					if (InputData8F[datazip] < DataMinEl)
						DataMinEl = InputData8F[datazip];
					break;
					} // double
				} // switch input value size
			datazip ++;
			} // for j
		datazip += RowIncr;
		if (BWDO)
			{
			if (BWDO->CheckAbort())
				{
				error = 50;
				break;
				} // if user abort
			BWDO->Update((ULONG)(i + 1));
			} // if
		} // for i
	if (BWDO)
		delete BWDO;
	} // if scale operator...
if (error)
	goto Cleanup;

Importing->TestMax = DataMaxEl;
Importing->TestMin = DataMinEl;

if (Importing->TestOnly)
	return error;

if (Importing->Reference == 1) // X,Y
	{
	DatumChange = GlobalApp->MainProj->Interactive->GetProjRefCoords(WCS_INTERVEC_COMP_Z) - DataMinEl;
	Importing->DatumChange += DatumChange;
	}
else
	DatumChange = 0.0;

DataScale = 1.0;
switch (SCALEOP)
	{
	default:
	case DEM_DATA_SCALEOP_UNIFIED:
		{
		DataRef = DataMinEl;
		if (Importing->InvertData)
			{
			if (Importing->UserMaxSet)
				OutRef = Importing->UserMax;
//				OutRef = max(DataMaxEl, Importing->UserMax);  OK, what was this for anyways?
			else
				OutRef = DataMaxEl;
			}
		else
			{
			if (Importing->UserMinSet)
				OutRef = Importing->UserMin;
//				OutRef = min(DataMinEl, Importing->UserMin);
			else
				OutRef = DataMinEl;
			}
		DatumChange = Importing->DatumChange;
		VertScale = Importing->VertExag;
		if (DataMaxEl == DataMinEl)
			{
			break;	// use the DataScale of 1.0;
			}
		if ((Importing->UserMaxSet) && (Importing->UserMinSet))
			{
			DataScale = (Importing->UserMax - Importing->UserMin) /
				(DataMaxEl - DataMinEl);
			}
		else if (Importing->UserMaxSet)
			{
			DataScale = (Importing->UserMax - DataMinEl) /
				(DataMaxEl - DataMinEl);
			}
		else if (Importing->UserMinSet)
			{
			DataScale = (DataMaxEl - Importing->UserMin) /
				(DataMaxEl - DataMinEl);
			}
		// if the user didn't set Max or Min, use the DataScale of 1.0
		if (Importing->InvertData)
			DataScale *= -1.0;
		break;
		}
	case DEM_DATA_SCALEOP_MAXMINSCALE:
		{
		DataRef = DataMinEl;
		OutRef = SCALE_MINEL;
		if (DataMaxEl == DataMinEl || (SCALE_MAXEL == 0.0 && SCALE_MINEL == 0.0))
			{
			VertScale = 1.0f;
			NoScaling = 1;
			break;
			} // illegal scale
		VertScale = (SCALE_MAXEL - SCALE_MINEL) / (DataMaxEl - DataMinEl);
		break;
		} //
	case DEM_DATA_SCALEOP_MATCHSCALE:
		{
		switch (SCALETYPE)
			{
			default: break;
			case DEM_DATA_SCALETYPE_MAXEL:
				{
				DataRef = SCALE_VALU1;
				OutRef = SCALE_ELEV1;
				if (DataMaxEl == SCALE_VALU1 || (SCALE_SCALE == 0.0 && SCALE_ELEV1 == 0.0))
					{
					VertScale = 1.0f;
					NoScaling = 1;
					break;
					} // illegal scale
				VertScale = (SCALE_SCALE - SCALE_ELEV1) / (DataMaxEl - SCALE_VALU1);
				break;
				} //
			case DEM_DATA_SCALETYPE_MINEL:
				{
				DataRef = SCALE_VALU1;
				OutRef = SCALE_ELEV1;
				if (DataMinEl == SCALE_VALU1 || (SCALE_SCALE == 0.0 && SCALE_ELEV1 == 0.0))
					{
					VertScale = 1.0f;
					NoScaling = 1;
					break;
					} // illegal scale
				VertScale = (SCALE_ELEV1 - SCALE_SCALE) / (SCALE_VALU1 - DataMinEl);
				break;
				} //
			case DEM_DATA_SCALETYPE_SCALE:
				{
				DataRef = SCALE_VALU1;
				OutRef = SCALE_ELEV1;
				if (SCALE_SCALE == 0.0)
					{
					VertScale = 1.0f;
					NoScaling = 1;
					break;
					} // illegal scale
				VertScale = SCALE_SCALE;
				break;
				} //
			} // switch scale type
		break;
		} //
	case DEM_DATA_SCALEOP_MATCHMATCH:
		{
		DataRef = SCALE_VALU2;
		OutRef = SCALE_ELEV2;
		if (SCALE_VALU3 == SCALE_VALU2 || (SCALE_ELEV3 == 0.0 && SCALE_ELEV2 == 0.0))
			{
			VertScale = 1.0f;
			NoScaling = 1;
			break;
			} // illegal scale
		VertScale = (SCALE_ELEV3 - SCALE_ELEV2) / (SCALE_VALU3 - SCALE_VALU2);
		break;
		} //
	} // switch scale operator

SCALE_TESTMAX = DataMaxEl;
SCALE_TESTMIN = DataMinEl;

if (Importing->TestOnly)
	DataOpsCleanup(error);

return error;

Cleanup:
DataOpsCleanup(error);

return error;

} // ImportWizGUI::DataOpsScale

/*===========================================================================*/

void ImportWizGUI::DataOpsErrMsg(short error)
{

switch (error)
	{
	default:	// bad error code
		{
		UserMessageOK("Import Wizard", "Unknown error code returned.\r\r\nOperation terminated.");
		break;
		} // out of memory
	case 0:	// all's cool
		break;
	case 1:
		{
		UserMessageOK("Import Wizard", "Out of memory!\r\r\nOperation terminated.");
		break;
		} // out of memory
	case 2:
		{
		UserMessageOK("Import Wizard", "Unable to open file for input!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, "Import Wizard - source file");
		break;
		} // file open fail
	case 3:
		{
		UserMessageOK("Import Wizard", "Incorrect file size for specified header, width and height!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_SIZE, "Import Wizard - source file");
		break;
		} // file size fail
	case 4:
		{
		UserMessageOK("Import Wizard", "Unable to open file for output!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_OPEN_FAIL, "Import Wizard - destination file");
		break;
		} // file open fail
	case 5:
		{
		UserMessageOK("Import Wizard", "Error writing destination file!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, "Import Wizard - failed writing destination file");
		break;
		} // write failure
	case 6:
		{
		UserMessageOK("Import Wizard", "Error reading source file!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_SIZE, "Import Wizard - source file");
		break;
		} // file read fail
	case 7:
		{
		UserMessageOK("Import Wizard", "Not a compressed file!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_SIZE, "Import Wizard - source file");
		break;
		} // file open fail
	case 8:
		{
		UserMessageOK("Import Wizard", "Extended header!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_SIZE, "Import Wizard - source file");
		break;
		} // file open fail
	case 10:
		{
		UserMessageOK("Import Wizard", "Input file configuration not yet supported!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Import Wizard - source type");
		break;
		} // illogical input params
	case 11:
		{
		UserMessageOK("Import Wizard", "Input data format not supported!\nCheck your settings.\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "Import Wizard - source type");
		break;
		} // file type fail
	case 12:
		{
		UserMessageOK("Database Module", "Out of memory expanding database!\nOperation terminated.");
		break;
		} // out of memory for database expansion
	case 13:
		{
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_SIZE, "Import Wizard - source file");
		break;
		} // file open fail
	case 14:
		{
		UserMessageOK("Import Wizard", "Error saving \".Obj\" file!\nOperation terminated.");
		break;
		}
	case 15:
		{
		UserMessageOK("Import Wizard", "Input file not recognized as a DTED file!\r\r\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_TYPE, "DTED");
		break;
		}
	case 16:
		{
		UserMessageOK("Import Wizard", "This is only an 8 bit image file!\r\r\nOperation terminated.\r\r\nSet Input Value Bytes to One and retry.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRONG_TYPE, "DTED");
		break;
		}
	case 17:
		{
		UserMessageOK("Import Wizard", "An internal error has occurred.");
		break;
		}
	case 18:
		{
		UserMessageOK("Import Wizard: XYZ / WXYZ", "Out of memory!\nOperation terminated.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_MEM_FAIL, "Control Point Allocation");
		break;
		}
	case 19:
		{
		UserMessageOK("Import Wizard", "Warning!\nFile is not an IFF Z Buffer file.");
		GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_READ_FAIL, "ZBuffer load");
		break;
		}
	case 33:
		{
		UserMessageOK("Import Wizard", "Error!\nDEMs are in different UTM Zones");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "DEMs are in different UTM Zones.");
		break;
		}
	case 34:
		{
		UserMessageOK("Import Wizard", "Error!\n7.5' DEMs in region load need to be the same resolution.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "7.5' DEMs have different resolution.");
		break;
		}
	case 35:
		UserMessageOK("Import Wizard", "A problem was encountered while processing the SDTS files\n\
If you unarchived your data with WinZip, make sure the 'TAR file smart CR/LF conversion' option is disabled.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "7.5' DEMs have different resolution.");
		break;
	case 36:
		UserMessageOK("Import Wizard", "An error was found in the input data.");
		GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_ERR, "Input data error.");
		break;
	case 50:	// user cancelled - all's fine
		break;
	case 99:	// an error was encountered, but the message was already issued
		break;
	} // switch error

} // ImportWizGUI::DataOpsErrMsg

/*===========================================================================*/

short ImportWizGUI::DataOpsDEMInit(void)
{
short error = 0;
double deltaLat, deltaLon;

InputData = OutputData = NULL;
Importing->InputData = Importing->OutputData = NULL;
DOTopo = NULL;
NoScaling = 0;

if (Importing->WrapData)
	INPUT_COLS ++;

if (INPUT_FORMAT == DEM_DATA2_INPUT_VISTA)
	{
	if (INPUT_ROWS != 258 && INPUT_ROWS != 514 && INPUT_ROWS != 1026 && INPUT_ROWS != 2050)
		INPUT_ROWS = 258;
	INPUT_COLS = INPUT_ROWS;
	} // if Vista DEM input

if (OUTPUT_ROWS == 0)
	OUTPUT_ROWS = CROP_ROWS;
if (OUTPUT_COLS == 0)
	OUTPUT_COLS = CROP_COLS;

ORows = OUTPUT_ROWS;
OCols = OUTPUT_COLS;
if (Importing->InFormat == DEM_DATA2_INPUT_WCSDEM || Importing->InFormat == DEM_DATA2_INPUT_DTED ||
	Importing->InFormat == DEM_DATA2_INPUT_MDEM || Importing->InFormat == DEM_DATA2_INPUT_NTF_DTM)
	swmem(&OUTPUT_ROWS, &OUTPUT_COLS, sizeof (long));
// reset the bounds if there's any cropping going on
if (INPUT_ROWS != CROP_ROWS)
	{
	if ((strcmp(Importing->MyIdentity, "WCS_DEM") == 0) || (strcmp(Importing->MyIdentity, "WCS_DEM") == 0))
		{
//		deltaLon = (Importing->WBound - Importing->EBound) / (Importing->InCols - 1);
		deltaLon = (Importing->WBound - Importing->EBound) / (Importing->InRows - 1);
		if (!Importing->KeepBounds)
			{
			Importing->WBound -= Importing->CropLeft * deltaLon;
			Importing->EBound += Importing->CropRight * deltaLon;
			}
		}
	else
		{
		deltaLat = (Importing->NBound - Importing->SBound) / (Importing->InRows - 1);
		if (!Importing->KeepBounds)
			{
			Importing->NBound -= Importing->CropTop * deltaLat;
			Importing->SBound += Importing->CropBottom * deltaLat;
			}
		}
	}
if (INPUT_COLS != CROP_COLS)
	{
	if ((strcmp(Importing->MyIdentity, "WCS_DEM") == 0) || (strcmp(Importing->MyIdentity, "WCS_DEM") == 0))
		{
//		deltaLat = (Importing->NBound - Importing->SBound) / (Importing->InRows - 1);
		deltaLat = (Importing->NBound - Importing->SBound) / (Importing->InCols - 1);
		if (!Importing->KeepBounds)
			{
			Importing->NBound -= Importing->CropTop * deltaLat;
			Importing->SBound += Importing->CropBottom * deltaLat;
			}
		}
	else
		{
		deltaLon = (Importing->WBound - Importing->EBound) / (Importing->InCols - 1);
		if (!Importing->KeepBounds)
			{
			Importing->WBound -= Importing->CropLeft * deltaLon;
			Importing->EBound += Importing->CropRight * deltaLon;
			}
		}
	}

/*
** Compute output array sizes
*/
switch (OUTPUT_FORMAT)
	{
	default: break;
//	case DEM_DATA2_OUTPUT_COLORMAP:
	case DEM_DATA2_OUTPUT_WCSDEM:
		{
		LastOutputRows = OutputRows = 1 + (OUTPUT_ROWS - 1) / OUTPUT_COLMAPS;
		LastOutputCols = OutputCols = 1 + (OUTPUT_COLS - 1) / OUTPUT_ROWMAPS;
		DupRow = 1;
		break;
		} // WCS DEM output
	case DEM_DATA2_OUTPUT_BINARY:
//	case DEM_DATA2_OUTPUT_BINARY_TERRAIN:
	case DEM_DATA2_OUTPUT_COLORBMP:
	case DEM_DATA2_OUTPUT_COLORIFF:
	case DEM_DATA2_OUTPUT_COLORTGA:
	case DEM_DATA2_OUTPUT_COLORPICT:
	case DEM_DATA2_OUTPUT_CONTROLPTS:
	case DEM_DATA2_OUTPUT_GRAYIFF:
	case DEM_DATA2_OUTPUT_ZBUF:
		{
		LastOutputRows = OutputRows = OUTPUT_ROWS / OUTPUT_COLMAPS;
		LastOutputCols = OutputCols = OUTPUT_COLS / OUTPUT_ROWMAPS;
		DupRow = 0;
		break;
		} // IFF or Array output
	} // switch output format

if (((OUTPUT_ROWS - DupRow) % OUTPUT_COLMAPS) || ((OUTPUT_COLS - DupRow) % OUTPUT_ROWMAPS))
	{
	LastOutputRows = OutputRows - DupRow + OUTPUT_ROWS - (OutputRows - DupRow) * OUTPUT_COLMAPS;
	LastOutputCols = OutputCols - DupRow + OUTPUT_COLS - (OutputCols - DupRow) * OUTPUT_ROWMAPS;
	} // if too many rows or columns

if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM || INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM ||
	INPUT_FORMAT == DEM_DATA2_INPUT_MDEM || INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
	{
	if ((DOTopo = new DEM()) == NULL)
		{
		error = 1;
		goto Cleanup;
		} // if out of memory
	} // if WCS DEM input or output

InputDataSize = INPUT_COLS * INPUT_ROWS;

error = DataOpsAlloc();

Cleanup:
if (error)
	 DataOpsCleanup(error);

return ((short)(!error));

} // ImportWizGUI::DataOpsDEMInit

/*===========================================================================*/

short ImportWizGUI::DataOpsAlloc(void)
{

// USGS routine takes care of own memory allocations
if (Importing->InFormat == DEM_DATA2_INPUT_USGS_DEM)
	return 0;

//lint -save -e527
switch (INVALUE_SIZE)
	{
	default: break;
	case DEM_DATA_VALSIZE_BYTE:
		{
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_UNSIGNEDINT:
				{
				if ((InputData1U = (UBYTE *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
					{
					return 1;
					} // if out of memory
				InputData = InputData1U;
				break;
				} // unsigned byte
			case DEM_DATA_FORMAT_SIGNEDINT:
				{
				if ((InputData1S = (SBYTE *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
					{
					return 1;
					} // if out of memory
				InputData = InputData1S;
				break;
				} // signed byte
			default:
				{
				return 11;
				break;
				} // floating point byte or unknown format
			} // switch value format
		//InValSize = 1;
		break;
		} // byte
	case DEM_DATA_VALSIZE_SHORT:
		{
		InputDataSize *= 2;
		if (INPUT_FORMAT != DEM_DATA2_INPUT_WCSDEM)
			{
			switch (INVALUE_FORMAT)
				{
				case DEM_DATA_FORMAT_UNSIGNEDINT:
					{
					if ((InputData2U = (USHORT *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
						{
						return 1;
						}
					InputData = InputData2U;
					ShtSwp = (union ShortSwap *)InputData2U;
					break;
					} // unsigned short
				case DEM_DATA_FORMAT_SIGNEDINT:
					{
					if ((InputData2S = (SHORT *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
						{
						return 1;
						}
					InputData = InputData2S;
					ShtSwp = (union ShortSwap *)InputData2S;
					break;
					} // signed short
				default:
					{
					return 11;
					break;
					} // floating point short or unknown format
				} // switch value format
			} // if not WCS DEM input (DEMs will be read later)
		//InValSize = 2;
		break;
		} // short
	case DEM_DATA_VALSIZE_LONG:
		{
		InputDataSize *= 4;
		if (INPUT_FORMAT != DEM_DATA2_INPUT_WCSDEM)
			{
			switch (INVALUE_FORMAT)
				{
				case DEM_DATA_FORMAT_FLOAT:
					{
					if ((InputData4F = (FLOAT *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
						{
						return 1;
						}
					InputData = InputData4F;
					LngSwp = (union LongSwap *)InputData4F;
					break;
					} // floating point long
				case DEM_DATA_FORMAT_UNSIGNEDINT:
					{
					if ((InputData4U = (ULONG *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
						{
						return 1;
						}
					InputData = InputData4U;
					LngSwp = (union LongSwap *)InputData4U;
					break;
					} // unsigned long
				case DEM_DATA_FORMAT_SIGNEDINT:
					{
					if ((InputData4S = (LONG *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
						{
						return 1;
						}
					InputData = InputData4S;
					LngSwp = (union LongSwap *)InputData4S;
					break;
					} // signed long
				default:
					{
					return 11;
					break;
					} // unknown format
				} // switch value format
			} // if not WCS DEM - it gets allocated by LoadDEM()
		//InValSize = 4;
		break;
		} // long
	case DEM_DATA_VALSIZE_DOUBLE:
		{
		InputDataSize *= 8;
		switch (INVALUE_FORMAT)
			{
			case DEM_DATA_FORMAT_FLOAT:
				{
				if ((InputData8F = (DOUBLE *)AppMem_Alloc((size_t)InputDataSize, 0)) == NULL)
					{
					return 1;
					}
				InputData = InputData8F;
				DblSwp = (union DoubleSwap *)InputData8F;
				break;
				} // floating point double
			default:
				{
				return 11;
				break;
				} // integer double or unknown format
			} // switch value format
		//InValSize = 8;
		break;
		} // double
	case DEM_DATA_VALSIZE_UNKNOWN:
		{
		return 10;
		break;
		} // unknown value size
	} // switch value size
//lint -restore

Importing->InputData = InputData;

return 0;

} // ImportWizGUI::DataOpsAlloc

/*===========================================================================*/

/*** F2 NOTE: Saves NULL data when VNS, .pElMaxEl/.pElMinEl need to account for this ***/
short ImportWizGUI::DataOpsSaveOutput(void *OutputData, long OutputDataSize, short i, short j,
	long rows, long cols, long OutputRows, long OutputCols, char *RGBComp)
{
char HideNULLed = FALSE, tempfilename[64], OutFilename[256], BaseName[64];//, NameStr[80];
short error = 0;
//short Found = 0;
FILE *fOutput = NULL;
Joe *Clip, *Added = NULL;
JoeDEM *MyDEM = NULL;
//LayerEntry *LE = NULL;
//NotifyTag ChangeEvent[2];
DEM TempDEM;
unsigned char newname;
FileReq *FR;

strcpy(tempfilename, OUTPUT_NAMEBASE);

if (OUTPUT_ROWMAPS > 1 || OUTPUT_COLMAPS > 1)
	{
	char suffix[5] = { '.', 0, 0, 0, 0 };	// David Catts will blow out the old 676 suffix limit!
	short mapct;

	// Compute ASCII character(s) to append - A..Z
	mapct = (short)(i * OUTPUT_COLMAPS + j);
	if (OUTPUT_COLMAPS * OUTPUT_ROWMAPS > 675)
		{
		suffix[1] = (char)(65 + mapct / 676);	// This will give us up to 17576 tiles.  That should hold them for awhile :)
		suffix[2] = 65 + (mapct / 26) % 26;
		suffix[3] = 65 + mapct % 26;
		}
	else if (OUTPUT_COLMAPS * OUTPUT_ROWMAPS > 25)
		{
		suffix[1] = (char)(65 + mapct / 26);
		suffix[2] = 65 + mapct % 26;
		}
	else
		suffix[1] = (char)(65 + mapct);

	if ((OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM))
		{
//		if (strlen(tempfilename) > 10 - 3)
//			tempfilename[10 - 3] = 0;
		strcat(tempfilename, suffix);
		strcpy(BaseName, tempfilename);
//		while (strlen(tempfilename) < 10)
//			strcat(tempfilename, " ");
		strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
		if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM)
			strcat(OutFilename, ".elev");
		else
			strcat(OutFilename, RGBComp);
		} // if output DEM to database
	else
		{
		strcat(tempfilename, suffix);
		strcpy(BaseName, tempfilename);
		strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
		} // if no database output
	} // if more than one output map
else
	{
	strcpy(BaseName, tempfilename);
	if ((OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM))
		{
//		while (strlen(tempfilename) < 10)
//			strcat(tempfilename, " ");
//		tempfilename[10] = 0;
		strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
		if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM)
			strcat(OutFilename, ".elev");
		else
			strcat(OutFilename, RGBComp);
		} // if output DEM to database
	else
		{
		strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
		} // if no output to database
	} // else only one output map

// warn the user that they'll overwrite a file
do
	{
	static BOOL nag = TRUE;
	fOutput = PROJ_fopen(OutFilename, "rb");
	if ((fOutput != NULL) && nag)
		{
		char message[128];
		sprintf(message, "Overwrite existing file\n\n(%s)\n\n?", OutFilename);
		// buttons get laid out in NO / Yes / Yes to All / Abort order
		newname = UserMessageCustomQuad("WARNING", message, "No", "Abort", "Yes", "Yes to All", 1);	// no = 1, yes = 2, yall = 3, abort = 0
		if (newname == 0)
			{
			fclose(fOutput);
			error = 50;	// user abort
			return error;
			}
		else if (newname == 3)
			nag = FALSE;
		else if ((newname == 1) && (FR = new FileReq))
			{
			FR->SetTitle("SAVE");
			FR->SetDefPat(WCS_REQUESTER_WILDCARD);
			if (newname && FR->Request(WCS_REQUESTER_FILE_SAVE))
				{
				strncpy(OutFilename, FR->GetFirstName(), 255);
				OutFilename[255] = 0;
				if (!strstr(OutFilename, ".elev"))
					strcat(OutFilename, ".elev");
				BreakFileName(OutFilename, Importing->OutDir, 256, Importing->OutName, 80);
				strncpy(BaseName, Importing->OutName, 63);
				BaseName[63] = 0;
				(void)StripExtension(BaseName);
				}
			delete FR;
			} // else if NO
		} // if nagging
	} while (fOutput && (newname == 1));
if (fOutput)
	fclose(fOutput);
fOutput = NULL;

switch (OUTPUT_FORMAT)
	{
	default: break;
	case DEM_DATA2_OUTPUT_WCSDEM:
		{		
		TempDEM.pLonEntries	 = cols;
		TempDEM.pLatEntries	 = rows;
//		if (stricmp(Importing->MyIdentity, "GTOPO30") == 0)
//			{
//			TempDEM.pLatStep	 = (OUTPUT_HILAT - OUTPUT_LOLAT) / (OUTPUT_ROWS);
//			TempDEM.pLonStep	 = (OUTPUT_HILON - OUTPUT_LOLON) / (OUTPUT_COLS);
//			}
//		else
//			{
			TempDEM.pLatStep	 = (OUTPUT_HILAT - OUTPUT_LOLAT) / (OUTPUT_ROWS - 1);
			// added 09/14/04
			if (Importing->WrapData)
				TempDEM.pLonStep	 = (OUTPUT_HILON - OUTPUT_LOLON) / ((OUTPUT_COLS - 1) - 1);
			else
				TempDEM.pLonStep	 = (OUTPUT_HILON - OUTPUT_LOLON) / (OUTPUT_COLS - 1);
//			}
//		if (stricmp(Importing->MyIdentity, "GTOPO30") == 0)
//			{
//			TempDEM.pSouthEast.Lat	 = OUTPUT_LOLAT + j * OutputRows * TempDEM.pLatStep;
//			TempDEM.pNorthWest.Lon	 = OUTPUT_HILON - i * OutputCols * TempDEM.pLonStep;
//			}
//		else
//			{
			TempDEM.pSouthEast.Lat	 = OUTPUT_LOLAT + (double)j * (OutputRows - DupRow) * TempDEM.pLatStep;
			TempDEM.pNorthWest.Lon	 = OUTPUT_HILON - (double)i * (OutputCols - DupRow) * TempDEM.pLonStep;
//			}
		TempDEM.pSouthEast.Lon	 = TempDEM.pNorthWest.Lon - (TempDEM.pLonEntries - 1) * TempDEM.pLonStep;
		TempDEM.pNorthWest.Lat	 = TempDEM.pSouthEast.Lat + (TempDEM.pLatEntries - 1) * TempDEM.pLatStep;
		TempDEM.RawMap = (float *)OutputData;
		//TempDEM.size = OutputDataSize;

		TempDEM.pElScale = ELSCALE_METERS;	// v5 wants everything in meters only
/***	switch (ELEV_UNITS)
			{
			case DEM_DATA_UNITS_KILOM:
				{
				TempDEM.pElScale	 = ELSCALE_KILOM;
				break;
				}
			case DEM_DATA_UNITS_DECIM:
				{
				TempDEM.pElScale	 = ELSCALE_DECIM;
				break;
				}
			case DEM_DATA_UNITS_CENTIM:
				{
				TempDEM.pElScale	 = ELSCALE_CENTIM;
				break;
				}
			case DEM_DATA_UNITS_MILLIM:
				{
				TempDEM.pElScale	 = ELSCALE_MILLIM;
				break;
				}
			case DEM_DATA_UNITS_MILES:
				{
				TempDEM.pElScale	 = ELSCALE_MILES;
				break;
				}
			case DEM_DATA_UNITS_YARDS:
				{
				TempDEM.pElScale	 = ELSCALE_YARDS;
				break;
				}
			case DEM_DATA_UNITS_FEET:
				{
				TempDEM.pElScale	 = ELSCALE_FEET;
				break;
				}
			case DEM_DATA_UNITS_INCHES:
				{
				TempDEM.pElScale	 = ELSCALE_INCHES;
				break;
				}
			default:
			case DEM_DATA_UNITS_METERS:
				{
				TempDEM.pElScale	 = ELSCALE_METERS;
				break;
				}
			} // switch
***/

		if (INPUT_FORMAT != DEM_DATA2_INPUT_WCSDEM)
			{
			if (ROWS_EQUAL == DEM_DATA_ROW_LON)
				{
				swmem(&TempDEM.pLatStep, &TempDEM.pLonStep, 8);
				swmem(&TempDEM.pSouthEast.Lat, &TempDEM.pNorthWest.Lon, 8);
				swmem(&TempDEM.pSouthEast.Lon, &TempDEM.pNorthWest.Lat, 8);
				} // if rows = longitude
			} // if not WCS DEM input

		TempDEM.RawMap = (float *)OutputData;
#ifdef WCS_BUILD_VNS
		if (Importing->HasNulls)
			{
			TempDEM.SetNullReject(1);
			TempDEM.SetNullValue((float)Importing->SaveNull);
			}
#endif // WCS_BUILD_VNS

		TempDEM.FindElMaxMin();	// added by GRH 8/6/01
		TempDEM.PrecalculateCommonFactors(); // CXH 8/11/06

		#ifdef WCS_BUILD_VNS
		Clip = DBHost->AddDEMToDatabase("Import Wizard", BaseName, &TempDEM, NewCoordSys, ProjHost, EffectsHost);	// added by GRH 8/6/01
		#else // WCS_BUILD_VNS
		Clip = DBHost->AddDEMToDatabase("Import Wizard", BaseName, &TempDEM, NULL, ProjHost, EffectsHost);	// added by GRH 8/6/01
		#endif // WCS_BUILD_VNS

		if (! TempDEM.SaveDEM(OutFilename, LocalLog))
			{
			TempDEM.RawMap = NULL;
			error = 5;
			goto Cleanup;
			} // if

		TempDEM.RawMap = NULL;
		if (Importing->HideNULLed)
			{
			if (TempDEM.pElMaxEl < TempDEM.pElMinEl)	// found a completely NULL tile
				HideNULLed = TRUE;
			else
				HideNULLed = FALSE;
			}
		GlobalApp->StatusLog->PostStockError(WCS_LOG_MSG_DEM_SAVE, tempfilename);


		Clip->RecheckBounds();	// removed by GRH 8/6/01, reenabled by FW2 09/29/04

		break;
		} // WCS DEM
	case DEM_DATA2_OUTPUT_GRAYIFF:
	case DEM_DATA2_OUTPUT_COLORIFF:
	case DEM_DATA2_OUTPUT_COLORTGA:
	case DEM_DATA2_OUTPUT_COLORBMP:
	case DEM_DATA2_OUTPUT_COLORPICT:
		{
		if (! SaveImageFile(OUTPUT_FORMAT, OutFilename, cols, rows))
			error = 5;	// write failure
		break;
		} // iff
	case DEM_DATA2_OUTPUT_BINARY:
//	case DEM_DATA_OUTPUT_COLORMAP:
		{
		if ((fOutput = PROJ_fopen(OutFilename, "wb" /*IOFLAG_BINARY | IOFLAG_CREAT | IOFLAG_TRUNC | IOFLAG_WRONLY*/)) == 0)
			{
			error = 4;
			break;
			} // if open fail
		if ((fwrite((char *)OutputData, 1, OutputDataSize, fOutput)) != (unsigned)OutputDataSize)
			{
			error = 5;
			fclose(fOutput);
			goto Cleanup;
			} // if write fail
		fclose(fOutput);
		break;
		} // array
//	case DEM_DATA2_OUTPUT_BINARY_TERRAIN:
//		{
//		if ((fOutput = PROJ_fopen(OutFilename, "wb" /*IOFLAG_BINARY | IOFLAG_CREAT | IOFLAG_TRUNC | IOFLAG_WRONLY*/)) == 0)
//			{
//			error = 4;
//			break;
//			} // if open fail
//		if (ExportBT(fOutput, OUTPUT_COLS, OUTPUT_ROWS,
//			Importing->NBound, Importing->SBound, Importing->EBound, Importing->WBound, (float *)OutputData) != 0)
//			{
//			error = 5;
//			fclose(fOutput);
//			goto Cleanup;
//			} // if write fail
//		fclose(fOutput);
//		break;
//		}
	case DEM_DATA2_OUTPUT_ZBUF:
		{
		if (! SaveImageFile(OUTPUT_FORMAT, OutFilename, cols, rows))
			error = 5;	// write failure
		//SaveZBuf(0, 0, OutputDataSize / (long)sizeof(float), NULL,
		//	(float *)OutputData, OutFilename, (short)rows, (short)cols, 0, 1, (short)cols, (short)rows);
		break;
		} // Z Buffer
	case DEM_DATA2_OUTPUT_CONTROLPTS:
		{
		VectorPoint *MyPoints, *PLink;
		long TotalPoints, TotalCt, PointsThisObject, PointCt;
		double LatStep, LonStep;
		float *Data = (float *)OutputData;
#ifndef WCS_BUILD_VNS
		float MyNorth, MySouth, MyEast, MyWest;
#endif // !WCS_BUILD_VNS

		TotalPoints = rows * cols;
		LatStep = (OUTPUT_HILAT - OUTPUT_LOLAT) / (rows - 1);
		LonStep = (OUTPUT_HILON - OUTPUT_LOLON) / (cols - 1);
		for (TotalCt = 0; TotalCt < TotalPoints; )
			{
			// first point is reserved for label use
			PointsThisObject = TotalPoints + Joe::GetFirstRealPtNum() - TotalCt >= WCS_DXF_MAX_OBJPTS ? WCS_DXF_MAX_OBJPTS: TotalPoints + Joe::GetFirstRealPtNum() - TotalCt;
#ifndef WCS_BUILD_VNS
			MyNorth = -90.0f;
			MySouth = 90.0f;
			MyEast = 360.0f;
			MyWest = -360.0f;
#endif // !WCS_BUILD_VNS
			if (MyPoints = DBHost->MasterPoint.Allocate(PointsThisObject))
				{
				// first point is reserved for label use
				for (PLink = (Joe::GetFirstRealPtNum() ? MyPoints->Next: MyPoints), PointCt = Joe::GetFirstRealPtNum(); PLink && PointCt < PointsThisObject; PLink = PLink->Next, PointCt ++, TotalCt ++)
					{
					PLink->Latitude = OUTPUT_HILAT - (TotalCt / (float)cols) * LatStep;
					PLink->Longitude = OUTPUT_HILON - (TotalCt % cols) * LonStep;
					PLink->Elevation = Data[TotalCt];
#ifndef WCS_BUILD_VNS
					if(PLink->Latitude > MyNorth)
						{
						MyNorth = (float)PLink->Latitude;
						} // if
					if(PLink->Latitude < MySouth)
						{
						MySouth = (float)PLink->Latitude;
						} // if
					if(PLink->Longitude > MyWest)
						{
						MyWest = (float)PLink->Longitude;
						} // if
					if(PLink->Longitude < MyEast)
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
				if(Clip = new (BaseName) Joe)
					{
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
					Clip->SetRGB(221, 221, 221);
#ifdef WCS_BUILD_VNS
					if (NewCoordSys)
						Clip->AddEffect(NewCoordSys, -1);
					Clip->RecheckBounds();
#endif // WCS_BUILD_VNS
					Added = DBHost->AddJoe(Clip, WCS_DATABASE_STATIC, ProjHost);
					if (Added)
						{
						DBHost->SetActiveObj(Clip);
						DBHost->BoundUpTree(Clip);
						} // if
					else
						{
						UserMessageOK("Import Wizard", "Could not add object to Database.\nOperation terminated.");
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
		break;
		} // Control Points
	} // switch output file format

Cleanup:

return (error);

} // ImportWizGUI::DataOpsSaveOutput

/*===========================================================================*/

char ImportWizGUI::DataOpsEarlyDEMSave(void)
{

if (DataOpsSameFormat() && NoScaling
	&& Importing->OutDEMNS == 1 && Importing->OutDEMWE == 1 &&
	Importing->InRows == ORows && Importing->InCols == OCols)
	{
	DataOpsSaveOutput(InputData, InputDataSize, 0, 0,
		Importing->OutRows, Importing->OutCols, Importing->OutRows, Importing->OutCols, RGBComponent);
	return TRUE;
	} // if

return FALSE;

} // ImportWizGUI::DataOpsEarlyDEMSave

/*===========================================================================*/

char ImportWizGUI::DataOpsSameFormat(void)
{

if ((Importing->InFormat == DEM_DATA2_INPUT_WCSDEM) && (Importing->OutFormat == DEM_DATA2_OUTPUT_WCSDEM))
	return TRUE;
else if ((Importing->InFormat == DEM_DATA2_INPUT_ZBUF) && (Importing->OutFormat == DEM_DATA2_OUTPUT_ZBUF))
	return TRUE;
else
	return FALSE;

} // ImportWizGUI::DataOpsSameFormat

/*===========================================================================*/

short ImportWizGUI::DataOpsTransform(void)
{
double OutValue;
long BaseOff, colctr, cols, outzip, rows, rowctr;
short error = 0, i, j;

switch (OUTPUT_FORMAT)
	{
	default: break;
	case DEM_DATA2_OUTPUT_WCSDEM:
		{
		OUTVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
		OUTVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
		break;
		} // if WCS DEM output
	case DEM_DATA2_OUTPUT_COLORIFF:
	case DEM_DATA2_OUTPUT_GRAYIFF:
	case DEM_DATA2_OUTPUT_COLORTGA:
	case DEM_DATA2_OUTPUT_COLORBMP:
	case DEM_DATA2_OUTPUT_COLORPICT:
//	case DEM_DATA_OUTPUT_COLORMAP:
		{
		OUTVALUE_SIZE = DEM_DATA_VALSIZE_BYTE;
		OUTVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
		break;
		} // if Color Map or IFF output
	case DEM_DATA2_OUTPUT_ZBUF:
	case DEM_DATA2_OUTPUT_CONTROLPTS:
//	case DEM_DATA2_OUTPUT_BINARY_TERRAIN:
		{
		OUTVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
		OUTVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
		OUTPUT_ROWMAPS = OUTPUT_COLMAPS = 1;
		break;
		} // if Z Buffer output
	} // switch

// compute scaling factor to convert to meters for saving
switch (ELEV_UNITS)
	{
	case DEM_DATA_UNITS_KILOM:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_KILOMETER);
		break;
	case DEM_DATA_UNITS_DECIM:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_DECIMETER);
		break;
	case DEM_DATA_UNITS_CENTIM:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_CENTIMETER);
		break;
	case DEM_DATA_UNITS_MILLIM:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_MILLIMETER);
		break;
	case DEM_DATA_UNITS_MILES:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_MILE_US_STATUTE);
		break;
	case DEM_DATA_UNITS_YARDS:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_YARD);
		break;
	case DEM_DATA_UNITS_FEET:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET);
		break;
	case DEM_DATA_UNITS_SURVEY_FEET:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_FEET_US_SURVEY);
		break;
	case DEM_DATA_UNITS_INCHES:
		Rescale = ConvertToMeters(1.0, WCS_USEFUL_UNIT_INCH);
		break;
	default:
	case DEM_DATA_UNITS_METERS:
		Rescale = 1.0;
		break;
	} // switch

Importing->NullVal = ((Importing->NullVal - DataRef) * DataScale + OutRef) * VertScale + DatumChange;
Importing->NullVal *= Rescale;
Importing->SaveNull *= Rescale;
Importing->TestMax = ((Importing->TestMax - DataRef) * DataScale + OutRef) * VertScale + DatumChange;
//Importing->TestMax *= Rescale;
Importing->TestMin = ((Importing->TestMin - DataRef) * DataScale + OutRef) * VertScale + DatumChange;
//Importing->TestMin *= Rescale;
Importing->VScale = Rescale;

for (i = 0; i < OUTPUT_ROWMAPS; i++)	// W-E
	{
	if (i == OUTPUT_ROWMAPS - 1)
		cols = LastOutputCols;
	else
		cols = OutputCols;
	for (j = 0; j < OUTPUT_COLMAPS; j++)	// S-N
		{
		if (j == OUTPUT_COLMAPS - 1)
			rows = LastOutputRows;
		else
			rows = OutputRows;

		OutputDataSize = rows * cols;

		switch (OUTVALUE_SIZE)
			{
			default: break;
			case DEM_DATA_VALSIZE_BYTE:
				{
				switch (OUTVALUE_FORMAT)
					{
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if ((OutputData1U = (UBYTE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							} // if out of memory
						OutputData = OutputData1U;
						break;
						} // unsigned byte
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if ((OutputData1S = (SBYTE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							} // if out of memory
						OutputData = OutputData1S;
						break;
						} // signed byte
					default:
						{
						error = 11;
						break;
						} // floating point byte or unknown format
					} // switch output value format
				break;
				} // byte
			case DEM_DATA_VALSIZE_SHORT:
				{
				OutputDataSize *= 2;
				switch (OUTVALUE_FORMAT)
					{
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if ((OutputData2U = (USHORT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData2U;
						break;
						} // unsigned short
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if ((OutputData2S = (SHORT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData2S;
						break;
						} // signed short
					default:
						{
						error = 11;
						break;
						} // floating point short or unknown format
					} // switch output value format
				break;
				} // short
			case DEM_DATA_VALSIZE_LONG:
				{
				OutputDataSize *= 4;
				switch (OUTVALUE_FORMAT)
					{
					case DEM_DATA_FORMAT_FLOAT:
						{
						if ((OutputData4F = (FLOAT *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData4F;
						break;
						} // floating point long
					case DEM_DATA_FORMAT_UNSIGNEDINT:
						{
						if ((OutputData4U = (ULONG *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData4U;
						break;
						} // unsigned long
					case DEM_DATA_FORMAT_SIGNEDINT:
						{
						if ((OutputData4S = (LONG *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData4S;
						break;
						} // signed long
					default:
						{
						error = 11;
						break;
						} // unknown format
					} // switch output value format
				break;
				} // long
			case DEM_DATA_VALSIZE_DOUBLE:
				{
				OutputDataSize *= 8;
				switch (OUTVALUE_FORMAT)
					{
					case DEM_DATA_FORMAT_FLOAT:
						{
						if ((OutputData8F = (DOUBLE *)AppMem_Alloc((size_t)OutputDataSize, 0)) == NULL)
							{
							error = 1;
							goto Cleanup;
							}
						OutputData = OutputData8F;
						break;
						} // floating point double
					default:
						{
						error = 11;
						break;
						} // integer double or unknown format
					} // switch output value format
				break;
				} // double
			case DEM_DATA_VALSIZE_UNKNOWN:
				{
				error = 10;
				break;
				} // unknown value size
			} // switch output value size
		if (error)
			goto Cleanup;

		//
		// Scale and move data
		//

//		UserMessageOK("IM Debug <file=>", "At Scale & Move");

		// pointer to first corner
		switch (OUTPUT_FORMAT)
			{
			case DEM_DATA2_OUTPUT_WCSDEM:
//			case DEM_DATA_OUTPUT_COLORMAP:
				{
				if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM 
					|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
					|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
					{
					BaseOff = (CROP_LEFT + i * (OutputCols - DUPROW)) * INPUT_COLS
						+ j * (OutputRows - DUPROW) + CROP_BOTTOM;
					} // if elevs in same order as USGS (by column, starting with SW)
				else
					{
					BaseOff = (CROP_TOP + ((OUTPUT_COLMAPS - 1 - j) * (OutputRows - DUPROW)
						+ LastOutputRows - DUPROW)) * INPUT_COLS
						+ i * (OutputCols - DUPROW) + CROP_LEFT;
					} // if elevs in raster order
				break;
				} // if WCS DEM or Color Map output
			default:
				{
				if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
					|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
					{
					BaseOff = (CROP_LEFT + i * (OutputCols)) * INPUT_COLS
						+ j * (OutputRows) + rows - 1 + CROP_BOTTOM;
					} // if WCS DEM input
				else
					{
					if (OUTPUT_COLMAPS - 1 - j > 0)
						{
						BaseOff = (CROP_TOP + ((OUTPUT_COLMAPS - 2 - j) * OutputRows
							+ LastOutputRows)) * INPUT_COLS  + i * OutputCols + CROP_LEFT;
						} // if not top row of output maps
					else
						{
						BaseOff = CROP_TOP * INPUT_COLS + i * OutputCols + CROP_LEFT;
						} // else top row of output maps
					} // else not DEM input
				break;
				} // else not WCS DEM or Color Map output
			} // switch OUTPUT_FORMAT

		outzip = 0;

		BWDO = new BusyWin("Convert", (ULONG)cols, 'BWDO', 0);
		for (colctr = 0 ; colctr < cols; colctr++)
			{
			if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM)
//				|| OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP)
				{
				if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
					|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
					{
					//datazip = BaseOff + colctr * INPUT_ROWS;
					datazip = BaseOff + colctr * INPUT_COLS;
					outzip = colctr * rows;
					} // if WCS DEM input
				else
					{
					datazip = BaseOff + colctr;
					} // else not DEM input
				} // if WCS DEM or Color Map output
			else
				{
				if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
					|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
					|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
					{
					//datazip = BaseOff + colctr * INPUT_ROWS;
					datazip = BaseOff + colctr * INPUT_COLS;
					outzip = colctr;
					} // if WCS DEM input
				else
					{
					datazip = BaseOff + colctr;
					outzip = colctr;
					} // else not DEM input
				} // else not WCS DEM or Color Map output

			for (rowctr = 0; rowctr < rows; rowctr++)
				{
				switch (INVALUE_SIZE)
					{
					default:
						OutValue = 0.0;
						break;
					case DEM_DATA_VALSIZE_BYTE:
						{
						switch (INVALUE_FORMAT)
							{
							default:
								OutValue = 0.0;
								break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
//								OutValue = OutRef + (InputData1U[datazip] - DataRef) * VertScale;
								// equivalent to the old method when DataScale == 1.0 & DatumChange == 0.0
								OutValue = ((InputData1U[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // UBYTE input format
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
//								OutValue = OutRef + (InputData1S[datazip] - DataRef) * VertScale;
								OutValue = ((InputData1S[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // BYTE input format
							} // switch input format
						break;
						} // BYTE input size
					case DEM_DATA_VALSIZE_SHORT:
						{
						switch (INVALUE_FORMAT)
							{
							default:
								OutValue = 0.0;
								break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
//								OutValue = OutRef + (InputData2U[datazip] - DataRef) * VertScale;
								OutValue = ((InputData2U[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // USHORT input format
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
//								OutValue = OutRef + (InputData2S[datazip] - DataRef) * VertScale;
								OutValue = ((InputData2S[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // SHORT input format
							} // switch input format
						break;
						} // SHORT input size
					case DEM_DATA_VALSIZE_LONG:
						{
						switch (INVALUE_FORMAT)
							{
							default:
								OutValue = 0.0;
								break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
//								OutValue = OutRef + (InputData4U[datazip] - DataRef) * VertScale;
								OutValue = ((InputData4U[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // ULONG input format
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
//								OutValue = OutRef + (InputData4S[datazip] - DataRef) * VertScale;
								OutValue = ((InputData4S[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // LONG input format
							case DEM_DATA_FORMAT_FLOAT:
								{
//								OutValue = OutRef + (InputData4F[datazip] - DataRef) * VertScale;
								OutValue = ((InputData4F[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // FLOAT input format
							} // switch input format
						break;
						} // LONG input size
					case DEM_DATA_VALSIZE_DOUBLE:
						{
						switch (INVALUE_FORMAT)
							{
							default:
								OutValue = 0.0;
								break;
							case DEM_DATA_FORMAT_FLOAT:
								{
//								OutValue = OutRef + (InputData8F[datazip] - DataRef) * VertScale;
								OutValue = ((InputData8F[datazip] - DataRef) * DataScale + OutRef) *
									VertScale + DatumChange;
								break;
								} // DOUBLE input format
							} // switch input format
						break;
						} // DOUBLE input size
					} // switch input value size

				if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM)
					OutValue *= Rescale; // convert to meters

				switch (OUTVALUE_SIZE)
					{
					default: break;
					case DEM_DATA_VALSIZE_BYTE:
						{
						switch (OUTVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								if (OutValue > UCHAR_MAX)
									OutputData1U[outzip] = UCHAR_MAX;
								else if (OutValue < 0.0)
									OutputData1U[outzip] = 0;
								else
									OutputData1U[outzip] = (UBYTE)OutValue;
								break;
								} // UBYTE output format */
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								if (OutValue > SCHAR_MAX)
									OutputData1S[outzip] = SCHAR_MAX;
								else if (OutValue < SCHAR_MIN)
									OutputData1S[outzip] = (SBYTE)SCHAR_MIN;
								else
									OutputData1S[outzip] = (SBYTE)OutValue;
								break;
								} // BYTE output format
							} // switch output format
						break;
						} // BYTE output size
					case DEM_DATA_VALSIZE_SHORT:
						{
						switch (OUTVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								if (OutValue > USHRT_MAX)
									OutputData2U[outzip] = USHRT_MAX;
								else if (OutValue < 0.0)
									OutputData2U[outzip] = 0;
								else
									OutputData2U[outzip] = (USHORT)OutValue;
								break;
								} // USHORT output format
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								if (OutValue > SHRT_MAX)
									OutputData2S[outzip] = SHRT_MAX;
								else if (OutValue < SHRT_MIN)
									OutputData2S[outzip] = SHRT_MIN;
								else
									OutputData2S[outzip] = (SHORT)OutValue;
								break;
								} // SHORT output format
							} // switch output format
						break;
						} // SHORT output size
					case DEM_DATA_VALSIZE_LONG:
						{
						switch (OUTVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_UNSIGNEDINT:
								{
								if (OutValue > ULONG_MAX)
									OutputData4U[outzip] = ULONG_MAX;
								else if (OutValue < 0.0)
									OutputData4U[outzip] = 0;
								else
									OutputData4U[outzip] = (ULONG)OutValue;
								break;
								} // ULONG output format
							case DEM_DATA_FORMAT_SIGNEDINT:
								{
								if (OutValue > LONG_MAX)
									OutputData4S[outzip] = LONG_MAX;
								else if (OutValue < LONG_MIN)
									OutputData4S[outzip] = LONG_MIN;
								else
									OutputData4S[outzip] = (LONG)OutValue;
								break;
								} // LONG output format
							case DEM_DATA_FORMAT_FLOAT:
								{
								if (OutValue > FLT_MAX)
									OutputData4F[outzip] = FLT_MAX;
								else
									OutputData4F[outzip] = (FLOAT)OutValue;
								break;
								} // FLOAT output format
							} // switch output format
						break;
						} // LONG output size
					case DEM_DATA_VALSIZE_DOUBLE:
						{
						switch (OUTVALUE_FORMAT)
							{
							default: break;
							case DEM_DATA_FORMAT_FLOAT:
								{
								OutputData8F[outzip] = OutValue;
								break;
								} // DOUBLE output format
							} // switch output format
						break;
						} // DOUBLE output size
					} // switch output size

				if (OUTPUT_FORMAT == DEM_DATA2_OUTPUT_WCSDEM)
//					|| OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP)
					{
					if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM
						|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
						|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
						|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
						{
						outzip ++;
						datazip ++;
						} // if WCS DEM input
					else
						{
						outzip ++;
						datazip -= INPUT_COLS;
						} // else not DEM input
					} // if output WCS DEM
				else
					{
					if (INPUT_FORMAT == DEM_DATA2_INPUT_WCSDEM
						|| INPUT_FORMAT == DEM_DATA2_INPUT_DTED
						|| INPUT_FORMAT == DEM_DATA2_INPUT_MDEM
						|| INPUT_FORMAT == DEM_DATA2_INPUT_NTF_DTM)
						{
						outzip += cols;
						datazip --;
						} // if WCS DEM input
					else
						{
						outzip += cols;
						datazip += INPUT_COLS;
						} // else not DEM input
					} // else not WCS DEM output

				} // for rowctr
			if (BWDO)
				{
				if (BWDO->CheckAbort())
					{
					error = 50;
					break;
					} // if user abort
				BWDO->Update((ULONG)(colctr + 1));
				} // if
			} // for colctr
		if (BWDO)
			delete BWDO;
		if (error)
			break;
		/*
		** Output
		*/
//		ELEV_UNITS = DEM_DATA_UNITS_METERS;	// we converted all the data to meters
		// Convert what we know about the input values to the output value scale
		error = DataOpsSaveOutput(OutputData, OutputDataSize, i, j,
			rows, cols, OutputRows, OutputCols, RGBComponent);

		if (OutputData)
			{
			AppMem_Free(OutputData, (size_t)OutputDataSize);
			OutputData = NULL;
			} // if OutputData
		if (error) break;
		} // for j
	if (error) break;
	} // for i

Cleanup:
return error;

} // ImportWizGUI::DataOpsTransform

/*===========================================================================*/

void ImportWizGUI::DataOpsDimsFromBounds(void)
{

Importing->InHeight = fabs(Importing->NBound - Importing->SBound);
Importing->InWidth = fabs(Importing->WBound - Importing->EBound);

} // ImportWizGUI::DataOpsDimsFromBounds

/*===========================================================================*/

void ImportWizGUI::DataOpsFlipYAxis(UBYTE *buffer, long rowsize, long numrows)
{
UBYTE tmp, *to, *from;
long ii,jj;

from = buffer;
to = buffer + (numrows - 1) * rowsize;
for (ii = 0; ii < numrows / 2; ii++)
	{
	for (jj = 0; jj < rowsize; jj++, to++, from++)
		{
		tmp = *to;
		*to = *from;
		*from = tmp;
		} // for
	to -= rowsize + rowsize; // reset to beginning of previous row
	} // for

} // ImportWizGUI::DataOpsFlipYAxis

/*===========================================================================*/

int ImportWizGUI::SaveImageFile(long FileFormat, char *OutFileName, long Width, long Height)
{
BufferNode *CurBuf, *RootBuf;
ImageOutputEvent IOEvent;
int Success = 1;
char PathName[256], FileName[256], BufCt, BufType, *DefBuf, *BufA;

BreakFileName(OutFileName, PathName, 256, FileName, 256);

BufA = FileFormat == DEM_DATA2_OUTPUT_ZBUF ? (char*)"ZBUF": (char*)"RED";
BufType = FileFormat == DEM_DATA2_OUTPUT_ZBUF ? WCS_RASTER_BANDSET_FLOAT: WCS_RASTER_BANDSET_BYTE;

if (FileFormat == DEM_DATA2_OUTPUT_GRAYIFF)
	strcpy(IOEvent.FileType, "IFF");
else if (FileFormat == DEM_DATA2_OUTPUT_COLORIFF)
	strcpy(IOEvent.FileType, "IFF");
else if (FileFormat == DEM_DATA2_OUTPUT_COLORTGA)
	strcpy(IOEvent.FileType, "Targa");
else if (FileFormat == DEM_DATA2_OUTPUT_COLORBMP)
	strcpy(IOEvent.FileType, "BMP");
else if (FileFormat == DEM_DATA2_OUTPUT_COLORPICT)
	strcpy(IOEvent.FileType, "PICT");
else if (FileFormat == DEM_DATA2_OUTPUT_ZBUF)
	strcpy(IOEvent.FileType, "IFF ZBUF");
else
	return (0);

strcpy(IOEvent.OutBuffers[0], BufA);
if (FileFormat != DEM_DATA2_OUTPUT_GRAYIFF && FileFormat != DEM_DATA2_OUTPUT_ZBUF)
	{
	strcpy(IOEvent.OutBuffers[1], "GREEN");
	strcpy(IOEvent.OutBuffers[2], "BLUE");
	} // if
if (DefBuf = ImageSaverLibrary::GetNextCodec(IOEvent.FileType, NULL))
	strcpy(IOEvent.Codec, DefBuf);

IOEvent.PAF.SetPathAndName(PathName, FileName);
IOEvent.AutoExtension = 0;
IOEvent.AutoDigits = 0;

// create a set of Buffer Nodes
if (CurBuf = RootBuf = new BufferNode(BufA, BufType))
	{
	CurBuf->Buffer = OutputData;
	for (BufCt = 1; IOEvent.OutBuffers[BufCt] && BufCt < 3; BufCt ++)
		{
		if (CurBuf = CurBuf->AddBufferNode(IOEvent.OutBuffers[BufCt], BufType))
			{
			CurBuf->Buffer = OutputData;
			} // if
		} // for

	// this sets up some necessary format-specific allocations
	IOEvent.InitSequence(NULL, RootBuf, Width, Height);

	// prep to save
	for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
		{
		if (CurBuf->Buffer)
			{
			CurBuf->PrepToSave(NULL, 0, Width, 0);
			} // if
		} // for

	// Save it
	Success = IOEvent.SaveImage(NULL, RootBuf, Width, Height, 0, NULL);

	// Cleanup all prep work
	for (CurBuf = RootBuf; CurBuf; CurBuf = CurBuf->Next)
		{
		if (CurBuf->Buffer)
			{
			CurBuf->CleanupFromSave();
			} // if
		} // for

	while (RootBuf)
		{
		CurBuf = RootBuf->Next;
		delete RootBuf;
		RootBuf = CurBuf;
		} // if

	} // if

return (Success);

} // ImportWizGUI::SaveImageFile

/*===========================================================================*/

// valid range is -60..-1 and 1..60
void ImportWizGUI::UTMZoneNum2DBCode(long *utmzone)
{

if (*utmzone > 0)
	*utmzone = *utmzone * 2 - 2;
else
	*utmzone = *utmzone * -2 - 1;

} // ImportWizGUI::UTMZoneNum2DBCode

/*===========================================================================*/
