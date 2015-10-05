// UsefulClip.cpp
// VectorClipper related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulClip.h"
#include "UsefulSwap.h"
#include "Fenetre.h"

// Line clipping code

void VectorClipper::SetClipBounds(DrawingFenetre *DF)
{
unsigned short  WinW, WinH;
if(DF)
	{
	DF->GetDrawingAreaSize(WinW, WinH);
	LowX = LowY = 0;
	HighX = WinW;
	HighY = WinH;
	} // if
} // VectorClipper::SetClipBounds

/*===========================================================================*/

void VectorClipper::MinimizeClipBounds(int X1, int Y1, int X2, int Y2)
{

if (X1 > X2)
	swmem(&X1, &X2, sizeof (int));
if (Y1 > Y2)
	swmem(&Y1, &Y2, sizeof (int));

X1 = min(X1, HighX);
LowX = max(LowX, X1);
X2 = max(X2, LowX);
HighX = min(HighX, X2);
X1 = min(Y1, HighY);
LowY = max(LowY, Y1);
Y2 = max(Y2, LowY);
HighY = min(HighY, Y2);

} // VectorClipper::MinimizeClipBounds()

/*===========================================================================*/

int VectorClipper::ClipSeg(double &XS, double &YS, double &XE, double &YE)
{
double icptx[4], icpty[4];
double m;
short outxlow, outxhigh, outylow, outyhigh, firstin, lastin;
short found = 0, swap = 0;

outxlow = outxhigh = outylow = outyhigh = 0;
firstin = lastin = 1;

/*
** Determine which pts are in draw region. Reject segment if both are on one
**   side and outside of draw region.
*/

// Attempt trivial discard on X axis
if (XS < LowX)
	{
	firstin = 0;
	outxlow ++;
	}
else if (XS > HighX)
	{
	firstin = 0;
	outxhigh ++;
	}
if (XE < LowX)
	{
	lastin = 0;
	outxlow ++;
	}
else if (XE > HighX)
	{
	lastin = 0;
	outxhigh ++;
	}

// If both points are off left side 
if (outxlow  == 2) return(0);

// If both points are off right side 
if (outxhigh == 2) return(0);

// Attempt trivial clip off right side
if (YS < LowY)
	{
	firstin = 0;
	outylow ++;
	}
else if (YS > HighY)
	{
	firstin = 0;
	outyhigh ++;
	}
if (YE < LowY)
	{
	lastin = 0;
	outylow ++;
	}
else if (YE > HighY)
	{
	lastin = 0;
	outyhigh ++;
	}

// Are both points off top of clip bounds?
if (outylow == 2) return(0);
// Are both points off bottom of clip bounds?
if (outyhigh == 2) return(0);

/*
** If both pts are in the draw region draw the segment. Otherwise
**   compute bounds intercept(s).
*/

if (firstin && lastin)
	{
	if((XS < 0) || (XE < 0) || (YS < 0) || (YE < 0))
		{
		//OutputDebugStr("Internal Clip Error.\n");
		} // if
	return(1);
	} // if both pts within draw region

// Make sure that we're always going to modifying the XE,YE point
if (lastin)
	{ // Swap first and last points using m as temp
	m = XS;
	XS = XE;
	XE = m;
	m = YS;
	YS = YE;
	YE = m;
	swap = 1;
	} // if

// Note to paranoid, if XE == XS, either both points are in, and
// we've already exited the function, or both are out and we've
// already trivially discarded the line.

if (XE != XS)
	m = (YE - YS) / (XE - XS);

 if (firstin || lastin)
  {
  if ((outxhigh + outxlow) && !(outyhigh + outylow))
   {
   if (outxhigh) XE = HighX;
   else XE = LowX;
   YE = YS + m * (XE - XS);
   } // if last pt x value is outside bounds
  else if ((outyhigh + outylow) && !(outxhigh + outxlow))
   {
   if (outyhigh) YE = HighY;
   else YE = LowY;
   if (XE != XS)
    XE = XS + (YE - YS) / m;
   } // if last pt y value is outside bounds
  else
   {
   if (outxhigh) XE = HighX;
   else XE = LowX;
   YE = YS + m * (XE - XS);
   if (YE < LowY || YE > HighY)
    {
    if (outyhigh) YE = HighY;
    else YE = LowY;
    XE = XS + (YE - YS) / m;
    }
   } // if last pt x and y values are outside bounds
  } // if first pt only is within draw region
 else
  { // Neither point is in the clip bounds, but the line
    // connecting them may cross the area.

  icptx[0] = LowX;
  icptx[1] = HighX;
  if (XS != XE)
   {
   icpty[0] = YS + m * (icptx[0] - XS);
   icpty[1] = YS + m * (icptx[1] - XS);
   } // if
  else
   {
   icpty[0] = -10000.;	// just some number that will always be outside the clip bounds
   icpty[1] = -10000.;
   } // else

  if (icpty[0] >= LowY && icpty[0] <= HighY) found +=1;
  if (icpty[1] >= LowY && icpty[1] <= HighY) found +=2;

  if (found < 3)
   {
   icpty[2] = LowY;
   icpty[3] = HighY;
   if (XS != XE)
    {
    icptx[2] = XS + (icpty[2] - YS) / m;
    icptx[3] = XS + (icpty[3] - YS) / m;
    } // if
   else
    {
    icptx[2] = XS;
    icptx[3] = XS;
    } // else

   if (icptx[2] >= LowX && icptx[2] <= HighX) found +=4;
   if (icptx[3] >= LowX && icptx[3] <= HighX) found +=8;
   } // if one or both of the y intercepts was out of bounds

  switch (found)
   {
   case 3:
    XS = icptx[0];
    YS = icpty[0];
    XE = icptx[1];
    YE = icpty[1];
    break;
   case 5:
    XS = icptx[0];
    YS = icpty[0];
    XE = icptx[2];
    YE = icpty[2];
    break;
   case 6:
    XS = icptx[1];
    YS = icpty[1];
    XE = icptx[2];
    YE = icpty[2];
    break;
   case 9:
    XS = icptx[0];
    YS = icpty[0];
    XE = icptx[3];
    YE = icpty[3];
    break;
   case 10:
    XS = icptx[1];
    YS = icpty[1];
    XE = icptx[3];
    YE = icpty[3];
    break;
   case 12:
    XS = icptx[2];
    YS = icpty[2];
    XE = icptx[3];
    YE = icpty[3];
    break;
   default:
    return(0);
   } // switch
  } // else both pts outside draw region

if (swap)
	{ // Swap first and last points using m as temp
	m = XS;
	XS = XE;
	XE = m;
	m = YS;
	YS = YE;
	YE = m;
	} // if

/*
** Approve the segment.
*/
if((XS < 0) || (XE < 0) || (YS < 0) || (YE < 0))
	{
	//OutputDebugStr("Internal Clip Error.\n");
	} // if

return(1);

} // VectorClipper::ClipSeg
