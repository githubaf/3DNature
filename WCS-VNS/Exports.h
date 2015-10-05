// Exports.h
// Exported file types
// Code created on 09/29/00 by Frank Weed II
// Copyright 2000 3D Nature

#include "stdafx.h"

#ifndef WCS_EXPORTS_H
#define WCS_EXPORTS_H

#include "Types.h"

/*****************************************************************************/

struct BinaryTerrain_Hdr	// intel ordering
{
char BT_Ident[10];
long BT_Cols, BT_Rows;
short BT_DataSize, BT_FloatFlag, BT_UTMFlag, BT_UTMZone, BT_Datum;
double BT_Left, BT_Right, BT_Bottom, BT_Top;
short BT_ExtProj;
float BT_VertScale;
char BT_Reserved[190];
};

/*****************************************************************************/

short ExportBT(FILE *bt_output, long bt_cols, long bt_rows,
	float bt_north, float bt_south, float bt_east, float bt_west, float *bt_dem);
short WriteBT_Hdr(FILE *bt_output, long bt_cols, long bt_rows, bool IsProjected);
short WriteBT_Line(FILE *bt_output, long bt_cols, float *bt_elev_line);
short FinishBT_File(FILE *bt_output, double bt_north, double bt_south, double bt_east, double bt_west);

#endif //  WCS_EXPORTS_H
