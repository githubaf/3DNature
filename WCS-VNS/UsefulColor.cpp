// UsefulColor.cpp
// Color-related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulColor.h"
#include "UsefulMath.h"
#include "FeatureConfig.h"

struct AICMYKcolor ToCMYK(const struct AIRGBcolor &rgb)
{
struct AICMYKcolor cmyk;

cmyk.c = 255 - rgb.r;
cmyk.m = 255 - rgb.g;
cmyk.y = 255 - rgb.b;
cmyk.k = __min(cmyk.c, cmyk.m);
cmyk.k = __min(cmyk.k, cmyk.y);
cmyk.c -= cmyk.k;
cmyk.m -= cmyk.k;
cmyk.y -= cmyk.k;
return cmyk;
} // ToCMYK

/*===========================================================================*/

// HSV[0]=H, HSV[1]=S, HSV[2]=V
// RGB[0]=R, RGB[1]=G, RGB[2]=B
// R:0-255 G:0-255 B:0-255
// H:0-360 S:0-100 V:0-100

#ifdef WCS_COLORMODEL_HSL
// HSL
double HuetoRGB(double v1, double v2, double vH);

void HSVtoRGB(double *HSV, double *RGB)
{
double H, S, L, v1, v2;

H = HSV[0] * (1.0 / 360.0);
S = HSV[1] * .01;
L = HSV[2] * .01;

if (HSV[1] == 0.0 )
	{
	RGB[0] = RGB[1] = RGB[2] = L;
	} // if
else
	{
	if ( L < 0.5 )
		v2 = L * ( 1.0 + S);
	else
		v2 = ( L + S ) - ( S * L );

	v1 = 2.0 * L - v2;

	RGB[0] = HuetoRGB(v1, v2, H + (1.0 / 3.0));
	RGB[1] = HuetoRGB(v1, v2, H);
	RGB[2] = HuetoRGB(v1, v2, H - (1.0 / 3.0));
	} // else

} // HSVtoRGB

/*===========================================================================*/

void RGBtoHSV(double *HSV, double *RGB)
{
double vMax, vMin, dMax, dR, dG, dB, H, S, L;

vMin = MIN3(RGB[0], RGB[1], RGB[2]);
vMax = MAX3(RGB[0], RGB[1], RGB[2]);
dMax = vMax - vMin;

L = (vMax + vMin) / 2.0;

if (dMax == 0.0)
	{
	H = 0.0;
	S = 0.0;
	} // if
else
	{
	if (L < 0.5)
		S = dMax / (vMax + vMin);
	else
		S = dMax / (2.0 - vMax - vMin);

	dR = (((vMax - RGB[0]) * (1.0 / 6.0)) + (dMax * .5)) / dMax;
	dG = (((vMax - RGB[1]) * (1.0 / 6.0)) + (dMax * .5)) / dMax;
	dB = (((vMax - RGB[2]) * (1.0 / 6.0)) + (dMax * .5)) / dMax;

	if (RGB[0] == vMax)
		H = dB - dG;
	else if (RGB[1] == vMax)
		H = (1.0 / 3.0) + dR - dB;
	else
		H = (2.0 / 3.0) + dG - dR;

	if (H < 0.0)
		H += 1.0;
	if (H > 1.0)
		H -= 1.0;
	} // else

HSV[0] = H * 360.0;
HSV[1] = S * 100.0;
HSV[2] = L * 100.0;

} // RGBtoHSV

/*===========================================================================*/

double HuetoRGB(double v1, double v2, double vH)
{

if (vH < 0.0)
	vH += 1.0;
if (vH > 1.0)
	vH -= 1.0;
if ((6.0 * vH) < 1.0)
	return (v1 + (v2 - v1) * 6.0 * vH);
if ((2.0 * vH) < 1.0)
	return (v2);
if ((3.0 * vH) < 2.0)
	return (v1 + (v2 - v1) * ((2.0 / 3.0) - vH) * 6.0);
return (v1);

} // 

/*===========================================================================*/

#else // WCS_COLORMODEL_HSL
// HSV

void HSVtoRGB(double *HSV, double *RGB)
{
double mmax, mmin, hueshift;

mmax = HSV[2] / 100;
mmin = mmax - (HSV[1] * mmax) / 100;
if (HSV[0] >= 60.0 && HSV[0] < 180.0)
	{
	RGB[1] = mmax;
	hueshift = HSV[0] - 120.0;
	if (hueshift < 0)
		{
		RGB[2] = mmin;
		RGB[0] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60.0);
		} // if shift toward red 
	else
		{
		RGB[0] = mmin;
		RGB[2] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60.0);
		} // else shift toward blu 
	} // if dominant hue grn 
else if (HSV[0] >= 180.0 && HSV[0] < 300.0)
	{
	RGB[2] = mmax;
	hueshift = HSV[0] - 240.0;
	if (hueshift < 0)
		{
		RGB[0] = mmin;
		RGB[1] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // if shift toward grn 
	else
		{
		RGB[1] = mmin;
		RGB[0] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // else shift toward red 
	} // else if dominant hue blu 
else
	{
	RGB[0] = mmax;
	hueshift = HSV[0] < 120.0 ? HSV[0]: HSV[0] - 360.0;
	if (hueshift < 0)
		{
		RGB[1] = mmin;
		RGB[2] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // if shift toward blu 
	else
		{
		RGB[2] = mmin;
		RGB[1] = (mmin + ((mmax - mmin) * fabs(hueshift)) / 60);
		} // else shift toward grn 
	} // else dominant hue red 

} // HSVtoRGB()

/*===========================================================================*/

void RGBtoHSV(double *HSV, double *RGB)
{
double mmax, mmin, mmid;
int sign;

mmax = MAX3(RGB[0], RGB[1], RGB[2]);
mmin = MIN3(RGB[0], RGB[1], RGB[2]);
mmid = MID3(RGB[0], RGB[1], RGB[2]);
if (mmax >= 1.0)
	HSV[2] = 100.0;
else
	HSV[2] = (100 * mmax);
if (mmax <= 0.0)
	HSV[1] = 0.0;
else
	HSV[1] = ((100 * (mmax - mmin)) / mmax);
if (mmax == mmin)
	{
	HSV[0] = 0.0;
	return;
	} // if R=G=B 
if (mmax == RGB[0])
	{
	if (mmid == RGB[1])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (0.0 + (sign * 60.0 * (mmid - mmin)) / (mmax - mmin));
	if (HSV[0] < 0.0)
		HSV[0] += 360.0;
	} // if red max 
else if (mmax == RGB[1])
	{
	if (mmid == RGB[2])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (120.0 + (sign * 60.0 * (mmid - mmin)) / (mmax - mmin));
	} // else if grn max 
else
	{
	if (mmid == RGB[0])
		sign = +1;
	else
		sign = -1;
	HSV[0] = (240.0 + (sign * 60.0 * (mmid - mmin)) / (mmax - mmin));
	} // else blu max 

} // RGBtoHSV()

#endif // WCS_COLORMODEL_HSL
/*===========================================================================*/

void ScaleHSV(double *RGB, double RotateHue, double AddSaturation, double AddValue)
{
double HSV[3];

RGBtoHSV(HSV, RGB);

HSV[0] += RotateHue;
while (HSV[0] > 360.0)
	HSV[0] -= 360.0;
while (HSV[0] < 0.0)
	HSV[0] += 360.0;
// adding saturation should only be done if there is some already. Otherwise pure gray turns pink
if (HSV[1] > 0.0)
	{
	HSV[1] += AddSaturation;
	if (HSV[1] > 100.0)
		HSV[1] = 100.0;
	if (HSV[1] < 0.0)
		HSV[1] = 0.0;
	}
HSV[2] += AddValue;
if (HSV[2] > 100.0)
	HSV[2] = 100.0;
if (HSV[2] < 0.0)
	HSV[2] = 0.0;

HSVtoRGB(HSV, RGB);

} // ScaleHSV

/*===========================================================================*/

char *EncodeColorAsHTML(double Red, double Green, double Blue, char *OutputBuf)
{
if(OutputBuf)
	{
	unsigned char R, G, B;

	R = (unsigned char)(Red * 255.0);
	G = (unsigned char)(Green * 255.0);
	B = (unsigned char)(Blue * 255.0);
	sprintf(OutputBuf, "#%02x%02x%02x", R, G, B);
	} // if

return(NULL);

} // EncodeColorAsHTML
