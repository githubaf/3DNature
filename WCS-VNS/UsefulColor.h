// UsefulColor.h
// Color-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULCOLOR_H
#define WCS_USEFULCOLOR_H

#ifdef RGB
#undef RGB // stupid Microsoft makes macros with hazardously-common names
#endif // RGB

// functions and structures for illustrator output
struct AIRGBcolor
	{
	unsigned char r, g, b;
	};	// AIRGBcolor

struct AICMYKcolor
	{
	unsigned char c, m, y, k;
	}; // AICMYKcolor

struct AICMYKcolor ToCMYK(const struct AIRGBcolor &rgb);

void HSVtoRGB(double *HSV, double *RGB);
void RGBtoHSV(double *HSV, double *RGB);
void ScaleHSV(double *RGB, double RotateHue, double AddSaturation, double AddValue);

// OutputBuf needs room for 7 chars plus a trailing NULL
char *EncodeColorAsHTML(double Red, double Green, double Blue, char *OutputBuf);

#endif // WCS_USEFULCOLOR_H
