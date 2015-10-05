// UsefulArray.cpp
// Array related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"

#ifdef BUILD_LIB
#define _TEMP_DEBUG // so we can restore after AppMem.h
#undef _DEBUG
#endif // BUILD_LIB

#include "AppMem.h" // for AppMem*()

#ifdef _TEMP_DEBUG
#define _DEBUG // after AppMem.h
#endif // _TEMP_DEBUG


long *Zip_New(long Width, long Height)
{
long *Me = NULL;
long Offset = 0;
short Y;

if(Width && Height)
	{
	if(Me = (long *)AppMem_Alloc(Height * sizeof(long), 0))
		{
		for(Y = 0; Y < Height; Y++, Offset += Width)
			{
			Me[Y] = Offset;
			} // for
		} // if
	} // if
return(Me);
} // Zip_New

/*===========================================================================*/

void Zip_Del(long *Me, long Width, long Height)
{

if(Me)
	{
	AppMem_Free(Me, Height * sizeof(long));
	} // if

} // Zip_Del

/*===========================================================================*/

double ArrayPointExtract(short *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows)
{
double RemX = 0.0, RemY = 0.0, P1, P2, rVal = 0.0;
long Row, Col, Col_p, Row_p, TrueRow, TrueCol;

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
		Row_p ++;
		} /* if not last row */
	if (Col < Cols - 1 && TrueCol >= 0)
		{ 
		RemX = (X - (Col * IntX + MinX)) / IntX;
		if (RemX < 0.0)
			RemX = 0.0;
		Col_p ++;
		} /* if not last column */

	P1 = RemX * (Data[Col_p * Rows + Row] - Data[Col * Rows + Row])
		+ Data[Col * Rows + Row];
	P2 = RemX * (Data[Col_p * Rows + Row_p] - Data[Col * Rows + Row_p])
		+ Data[Col * Rows + Row_p];

	rVal = RemY * (P2 - P1) + P1;
	} // if

return (rVal);

} // ArrayPointExtract()

/*===========================================================================*/

double ArrayPointExtract(float *Data, double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, long Cols, long Rows)
{
double RemX = 0.0, RemY = 0.0, P1, P2, rVal = 0.0;
long Row, Col, Col_p, Row_p, TrueRow, TrueCol;

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
		Row_p ++;
		} /* if not last row */
	if (Col < Cols - 1 && TrueCol >= 0)
		{ 
		RemX = (X - (Col * IntX + MinX)) / IntX;
		if (RemX < 0.0)
			RemX = 0.0;
		Col_p ++;
		} /* if not last column */

	P1 = RemX * (Data[Col_p * Rows + Row] - Data[Col * Rows + Row])
		+ Data[Col * Rows + Row];
	P2 = RemX * (Data[Col_p * Rows + Row_p] - Data[Col * Rows + Row_p])
		+ Data[Col * Rows + Row_p];

	rVal = RemY * (P2 - P1) + P1;
	} // if

return (rVal);

} // ArrayPointExtract()
