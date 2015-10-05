// Color.h
// Color Class for World Construction Set
// By GRH
// Copyright Questar Productions 1996

#ifndef WCS_COLOR_H
#define WCS_COLOR_H

#include "Types.h"

#ifdef RGB
#undef RGB // stupid Microsoft makes macros with hazardously-common names
#endif // RGB

class WCSRGBColor
	{

	public:
		void Set (double Red, double Grn, double Blu) {RGB[0] = (short)Red;
														RGB[1] = (short)Grn;
														RGB[2] = (short)Blu;};
		void Set (short Red, short Grn, short Blu) {RGB[0] = Red;
													RGB[1] = Grn;
													RGB[2] = Blu;};
		void Set (UBYTE Red, UBYTE Grn, UBYTE Blu) {RGB[0] = Red;
													RGB[1] = Grn;
													RGB[2] = Blu;};
		void Set (double *Source) {RGB[0] = (short)Source[0];
									RGB[1] = (short)Source[1];
									RGB[2] = (short)Source[2];};
		void Set (WCSRGBColor *Source) {RGB[0] = Source->RGB[0];
									RGB[1] = Source->RGB[1];
									RGB[2] = Source->RGB[2];};
		void operator+= (double Value) {RGB[0] = (short)(RGB[0] + Value);
										RGB[1] = (short)(RGB[1] + Value);
										RGB[2] = (short)(RGB[2] + Value);};
		void operator*= (double Value) {RGB[0] = (short)(RGB[0] * Value);
										RGB[1] = (short)(RGB[1] * Value);
										RGB[2] = (short)(RGB[2] * Value);};

		void Multiply(double Red, double Grn, double Blu) {RGB[0] = (short)(RGB[0] * Red);
															RGB[1] = (short)(RGB[1] * Grn);
															RGB[2] = (short)(RGB[2] * Blu);};
		void Increment(double Value) {RGB[0] = (short)(RGB[0] + RGB[0] * Value);
										RGB[1] = (short)(RGB[1] + RGB[1] * Value);
										RGB[2] = (short)(RGB[2] + RGB[2] * Value);};
		void Diminish(double Value)  {RGB[0] = (short)(RGB[0] - RGB[0] * Value);
										RGB[1] = (short)(RGB[1] - RGB[1] * Value);
										RGB[2] = (short)(RGB[2] - RGB[2] * Value);};

		void Add3(short V0, short V1, short V2)  {	RGB[0] += V0;
													RGB[1] += V1;
													RGB[2] += V2;};
		void Blend(WCSRGBColor *BCol, double Value) {RGB[0] = (short)(RGB[0] + (BCol->RGB[0] - RGB[0]) * Value);
												  RGB[1] = (short)(RGB[1] + (BCol->RGB[1] - RGB[1]) * Value);
												  RGB[2] = (short)(RGB[2] + (BCol->RGB[2] - RGB[2]) * Value);};
		void Blend(double *BCol, double Value) {RGB[0] = (short)(RGB[0] + (BCol[0] - RGB[0]) * Value);
												  RGB[1] = (short)(RGB[1] + (BCol[1] - RGB[1]) * Value);
												  RGB[2] = (short)(RGB[2] + (BCol[2] - RGB[2]) * Value);};
		void DiminishBlend(double *BCol, double Value)  {RGB[0] = (short)(RGB[0] - RGB[0] * Value + BCol[0]);
										RGB[1] = (short)(RGB[1] - RGB[1] * Value + BCol[1]);
										RGB[2] = (short)(RGB[2] - RGB[2] * Value + BCol[2]);};


		short RGB[3];

	}; // WCSRGBColor

#endif // WCS_COLOR_H
