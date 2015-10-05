// UsefulClip.h
// VectorClipper related code from Useful.h
// Built from Useful.h on 060403 by CXH

#include "stdafx.h"

#ifndef WCS_USEFULCLIP_H
#define WCS_USEFULCLIP_H

class DrawingFenetre;

// Used to be V1 clipbounds
class VectorClipper
	{
	public:

		short LowX, HighX, LowY, HighY;
		VectorClipper() {LowX = LowY = 0; HighX = HighY = SHRT_MAX;};
		void SetClipBounds(DrawingFenetre *DF);
		void MinimizeClipBounds(int X1, int Y1, int X2, int Y2);
		int ClipSeg(double &XS, double &YS, double &XE, double &YE);
		

	}; // VectorClipper

#endif // WCS_USEFULCLIP_H
