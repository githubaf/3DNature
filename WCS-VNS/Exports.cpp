// Exports.cpp
// Exported file types
// Code created on 09/29/00 by Frank Weed II
// Copyright 2000 3D Nature

#include "stdafx.h"
#include "Exports.h"
#include "AppMem.h"
#include "Useful.h"

struct BinaryTerrain_Hdr *bt_hdr;
#ifdef BYTEORDER_BIGENDIAN
float *tmp_linebuf;
#endif // BYTEORDER_BIGENDIAN

/*===========================================================================*/

// format specs at http://vterrain.org/Implementation/BT.html, also archived on Avalanche
// longitude values should be the internal WCS positive west values
// returns 0 if no error, 1 if unsuccessful
short ExportBT(FILE *bt_output, long bt_cols, long bt_rows,
	float bt_north, float bt_south, float bt_east, float bt_west, float *bt_dem)
{
float *bt_elev;
long x, y;

// allocate the header
bt_hdr = (BinaryTerrain_Hdr *)AppMem_Alloc((size_t)sizeof(BinaryTerrain_Hdr), APPMEM_CLEAR);
if (bt_hdr == NULL)
	return 1;

// jam our values in the header
strncpy(bt_hdr->BT_Ident, "binterr1.0", 10);
bt_hdr->BT_Cols = bt_cols;
bt_hdr->BT_Rows = bt_rows;
bt_hdr->BT_DataSize = 4;
bt_hdr->BT_Left = -bt_west;
bt_hdr->BT_Right = -bt_east;
bt_hdr->BT_Bottom = bt_south;
bt_hdr->BT_Top = bt_north;
bt_hdr->BT_FloatFlag = 1;

// write the header
if ((fwrite(&bt_hdr->BT_Ident[0], sizeof(bt_hdr->BT_Ident), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Cols, sizeof(bt_hdr->BT_Cols), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Rows, sizeof(bt_hdr->BT_Rows), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_DataSize, sizeof(bt_hdr->BT_DataSize), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_UTMFlag, sizeof(bt_hdr->BT_UTMFlag), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_UTMZone, sizeof(bt_hdr->BT_UTMZone), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Left, sizeof(bt_hdr->BT_Left), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Right, sizeof(bt_hdr->BT_Right), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Bottom, sizeof(bt_hdr->BT_Bottom), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Top, sizeof(bt_hdr->BT_Top), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_FloatFlag, sizeof(bt_hdr->BT_FloatFlag), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Reserved[0], sizeof(bt_hdr->BT_Reserved), 1, bt_output) != 1))
	return 1;

// save the DEM
for (x = 0; x < bt_cols; x++)
	{
	for (y = bt_rows - 1; y >= 0; y--)
		{
		bt_elev = &bt_dem[(y * bt_cols) + x];
		if (fwrite(bt_elev, sizeof(float), 1, bt_output) != 1)
			return 1;
		}
	}

return 0;	// no error

}

/*===========================================================================*/

// returns 0 if no error, 1 if unsuccessful
short WriteBT_Hdr(FILE *bt_output, long bt_cols, long bt_rows, bool IsProjected)
{

#ifdef BYTEORDER_BIGENDIAN
tmp_linebuf = NULL;
#endif // BYTEORDER_BIGENDIAN

// allocate the header
bt_hdr = (BinaryTerrain_Hdr *)AppMem_Alloc((size_t)sizeof(BinaryTerrain_Hdr), APPMEM_CLEAR);
if (bt_hdr == NULL)
	return 1;

#ifdef BYTEORDER_BIGENDIAN
// allocate a temp line buffer
tmp_linebuf = (float *)AppMem_Alloc((size_t)(bt_cols * sizeof(float)), NULL);
if (tmp_linebuf == NULL)
	return 1;
#endif // BYTEORDER_BIGENDIAN

// jam our values in the header
strncpy(bt_hdr->BT_Ident, "binterr1.3", 10);
bt_hdr->BT_Cols = bt_cols;
bt_hdr->BT_Rows = bt_rows;
bt_hdr->BT_DataSize = 4;
bt_hdr->BT_FloatFlag = 1;

if(IsProjected) // VNS, other than geoWGS84 or geo-BCS
	{
	bt_hdr->BT_UTMFlag = 1; // 0:HUnits = degrees, 1:HUnits=Meters, 2:HUnits=Int'l Foot, 3:HUnits=USSurvey Foot
	bt_hdr->BT_UTMZone = 0; // no-op when using external PRJ
	bt_hdr->BT_Datum = -1;  // -2=nodatum, -1=unknown, 23 or 6326 are WGS84
	bt_hdr->BT_ExtProj = 1; // 0=not external, 1=projection via external WKT PRJ
	} // if
else
	{
	bt_hdr->BT_UTMFlag = 0; // 0:HUnits = degrees, 1:HUnits=Meters, 2:HUnits=Int'l Foot, 3:HUnits=USSurvey Foot
	bt_hdr->BT_UTMZone = 0; // no-op when using geographic
	bt_hdr->BT_Datum = 6326;  // -2=nodatum, -1=unknown, 23 or 6326 are WGS84
	bt_hdr->BT_ExtProj = 0; // 0=not external, 1=projection via external WKT PRJ
	} // else

bt_hdr->BT_Left = 1.0;
bt_hdr->BT_Right = 0.0;
bt_hdr->BT_Bottom = 0.0;
bt_hdr->BT_Top = 1.0;
bt_hdr->BT_VertScale = 1.0f;
memset(bt_hdr->BT_Reserved, 0, sizeof(bt_hdr->BT_Reserved));
#ifdef BYTEORDER_BIGENDIAN
SimpleEndianFlip32S(bt_hdr->BT_Cols, &bt_hdr->BT_Cols);
SimpleEndianFlip32S(bt_hdr->BT_Rows, &bt_hdr->BT_Rows);
SimpleEndianFlip16S(bt_hdr->BT_DataSize, &bt_hdr->BT_DataSize);
SimpleEndianFlip16S(bt_hdr->BT_FloatFlag, &bt_hdr->BT_FloatFlag);
SimpleEndianFlip16S(bt_hdr->BT_UTMFlag, &bt_hdr->BT_UTMFlag);
SimpleEndianFlip16S(bt_hdr->BT_UTMZone, &bt_hdr->BT_UTMZone);
SimpleEndianFlip16S(bt_hdr->BT_Datum, &bt_hdr->BT_Datum);
SimpleEndianFlip64(&bt_hdr->BT_Left, &bt_hdr->BT_Left);
SimpleEndianFlip64(&bt_hdr->BT_Right, &bt_hdr->BT_Right);
SimpleEndianFlip64(&bt_hdr->BT_Top, &bt_hdr->BT_Top);
SimpleEndianFlip64(&bt_hdr->BT_Bottom, &bt_hdr->BT_Bottom);
SimpleEndianFlip16S(bt_hdr->BT_ExtProj, &bt_hdr->BT_ExtProj);
SimpleEndianFlip32F(&bt_hdr->BT_VertScale, &bt_hdr->BT_VertScale);
#endif // BYTEORDER_BIGENDIAN

// write the header
if ((fwrite(&bt_hdr->BT_Ident[0], sizeof(bt_hdr->BT_Ident), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Cols, sizeof(bt_hdr->BT_Cols), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Rows, sizeof(bt_hdr->BT_Rows), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_DataSize, sizeof(bt_hdr->BT_DataSize), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_FloatFlag, sizeof(bt_hdr->BT_FloatFlag), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_UTMFlag, sizeof(bt_hdr->BT_UTMFlag), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_UTMZone, sizeof(bt_hdr->BT_UTMZone), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Datum, sizeof(bt_hdr->BT_Datum), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Left, sizeof(bt_hdr->BT_Left), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Right, sizeof(bt_hdr->BT_Right), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Bottom, sizeof(bt_hdr->BT_Bottom), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Top, sizeof(bt_hdr->BT_Top), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_ExtProj, sizeof(bt_hdr->BT_ExtProj), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_VertScale, sizeof(bt_hdr->BT_VertScale), 1, bt_output) != 1) ||
	(fwrite(&bt_hdr->BT_Reserved[0], sizeof(bt_hdr->BT_Reserved), 1, bt_output) != 1))
	return 1;

return 0;	// no error

}

/*===========================================================================*/

// returns 0 if no error, 1 if unsuccessful
short WriteBT_Line(FILE *bt_output, long bt_cols, float *bt_elev_line)
{
#ifdef BYTEORDER_BIGENDIAN
float *val;
long i;

if (tmp_linebuf)
	{
	memcpy(tmp_linebuf, bt_elev_line, bt_cols * sizeof(float));
	val = &tmp_linebuf[0];
	for (i = 0; i < bt_cols; i++, val++)
		{
		SimpleEndianFlip32F(val, val);
		}
	if (fwrite(tmp_linebuf, sizeof(float), bt_cols, bt_output) != (size_t)bt_cols)
		return 1;
	}
#endif // BYTEORDER_BIGENDIAN

#ifdef BYTEORDER_LITTLEENDIAN
if (fwrite(bt_elev_line, sizeof(float), bt_cols, bt_output) != (size_t)bt_cols)
	return 1;
#endif // BYTEORDER_LITTLEENDIAN

return 0;

}

/*===========================================================================*/

// longitude values should be the internal WCS positive west values
// returns 0 if no error, 1 if unsuccessful
short FinishBT_File(FILE *bt_output, double bt_north, double bt_south, double bt_east, double bt_west)
{
short result = 1;	// init to failure
double tmp_north, tmp_south, tmp_west, tmp_east;

tmp_north = bt_north;
tmp_south = bt_south;
tmp_east = bt_east;
tmp_west = bt_west;
#ifdef BYTEORDER_BIGENDIAN
SimpleEndianFlip64(&tmp_north, &tmp_north);
SimpleEndianFlip64(&tmp_south, &tmp_south);
SimpleEndianFlip64(&tmp_east, &tmp_east);
SimpleEndianFlip64(&tmp_west, &tmp_west);
#endif

if (fseek(bt_output, 28, SEEK_SET) == 0)
	{
	if ((fwrite(&tmp_west, sizeof(double), 1, bt_output) == 1) &&
		(fwrite(&tmp_east, sizeof(double), 1, bt_output) == 1) &&
		(fwrite(&tmp_south, sizeof(double), 1, bt_output) == 1) &&
		(fwrite(&tmp_north, sizeof(double), 1, bt_output) == 1))
		result = 0;	// success
	}

if (bt_hdr)
	AppMem_Free(bt_hdr, sizeof(BinaryTerrain_Hdr));
#ifdef BYTEORDER_BIGENDIAN
if (tmp_linebuf)
	//AppMem_Free(tmp_linebuf, sizeof((size_t)(bt_cols * sizeof(float))));
	// This hack works for now, but might throw a harmless debugging error,
	// though the memory subsystem will free the proper size anyway.
	// <<<>>> needs a rewrite because bt_cols isn't available here!
	AppMem_Free(tmp_linebuf, 1);
#endif

return result;

}
