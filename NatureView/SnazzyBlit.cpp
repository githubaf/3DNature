// SnazzyBlit.cpp
// Blit support code 

#include "SnazzyBlit.h"

#ifndef ROUNDUP
        // Do NOT make b a zero, that would be dumb.
        #define ROUNDUP(a,b)    ((b) * (((a) + ((b) - 1)) / (b)))
#endif // ROUNDUP



BITMAPINFO BMIBlast;
// this ought to be big enough for individual blits up to 1k*1k pixels wide (4Mb of temporary memory)
#define SNAZZYBLIT_BLASTARRAY_SIZE		(1024*1024*4)
static unsigned char BlastCTBitArray[SNAZZYBLIT_BLASTARRAY_SIZE];

void RastBlastBlock(HDC BlastDest, unsigned long int X, unsigned long int Y,
 unsigned long int W, unsigned long int H, unsigned char *R, unsigned char *G, unsigned char *B)
{
signed long int XPaint, YPaint;
unsigned char RT, GT, BT;

if(R && G && B)
	{
	if(W * H * 3 > (SNAZZYBLIT_BLASTARRAY_SIZE - 4)) // -4 is for some sort of round-off protection
		{ // too big of a blit, we can't handle it
		return;
		} // if
	BMIBlast.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
	BMIBlast.bmiHeader.biWidth = W;
	BMIBlast.bmiHeader.biHeight = H;
	BMIBlast.bmiHeader.biPlanes = 1;
	// per this article:
	// http://bugs.sun.com/bugdatabase/view_bug.do?bug_id=4409306
	// "this is due to GDI's need for 32-bit aligned scanline strides.  This means that:
	//- All 32-bit images are aligned and will go at full speed
	// - One quarter (on average) of 24-bit images will go at full speed"
	// So, by padding our image to 32-bit, we retain the 32-bit alignment on each
	// scanline and realize significant speed imprvements
	BMIBlast.bmiHeader.biBitCount = 32;
	BMIBlast.bmiHeader.biCompression = BI_RGB;
	BMIBlast.bmiHeader.biSizeImage = 0;
	BMIBlast.bmiHeader.biXPelsPerMeter = 2835; // 72 dpi
	BMIBlast.bmiHeader.biYPelsPerMeter = 2835;
	BMIBlast.bmiHeader.biClrUsed = 0;
	BMIBlast.bmiHeader.biClrImportant = 0;
	// copy pixel data, interleaving as we go
	unsigned long int InterleaveIdx = 0, YBase = 0;
	for(YPaint = H - 1; YPaint >= 0; YPaint--) // scan-copy in bottom-up order
		{
		YBase = YPaint * W;
		for(XPaint = 0; XPaint < W; XPaint++)
			{
			RT = R[YBase + XPaint];
			GT = G[YBase + XPaint];
			BT = B[YBase + XPaint];
			BlastCTBitArray[InterleaveIdx++] = BT;
			BlastCTBitArray[InterleaveIdx++] = GT;
			BlastCTBitArray[InterleaveIdx++] = RT;
			BlastCTBitArray[InterleaveIdx++] = 255;
			} // for
		} // for
	SetDIBitsToDevice(BlastDest, X, Y, W, H, 0, 0, 0, H, BlastCTBitArray, &BMIBlast, DIB_RGB_COLORS);
	} // if


} // RastBlastBlock

/*
void RastBlastBlock(HDC BlastDest, unsigned long int X, unsigned long int Y,
 unsigned long int W, unsigned long int H, unsigned char *R, unsigned char *G, unsigned char *B)
{
unsigned char *LineR, *LineG, *LineB;

LineR = R;
LineG = G;
LineB = B;

for(unsigned short int i=0; i < H; i++)
	{
	RastBlastLine(BlastDest, X, Y + i, W, LineR, LineG, LineB);
	// adjust R, G, B input pointers ahead one line
	LineR += W;
	LineG += W;
	LineB += W;
	} // for

} // RastBlastBlock
*/

signed int BlitWhere(unsigned long int NumPixels, unsigned char *RDest, unsigned char *GDest, unsigned char *BDest,
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc, unsigned char *Mask, unsigned char MaskVal)
{
unsigned long int DonePixels = 0;
for(unsigned long int Pixel = 0; Pixel < NumPixels; Pixel++)
	{
	if(Mask[Pixel] == MaskVal)
		{
		RDest[Pixel] = RSrc[Pixel];
		GDest[Pixel] = GSrc[Pixel];
		BDest[Pixel] = BSrc[Pixel];
		DonePixels++;
		} // if
	} // for
return(DonePixels);
} // BlitWhere

signed int BlitSimple(unsigned long int NumPixels, unsigned char *RDest, unsigned char *GDest, unsigned char *BDest,
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc)
{
// this is brain-dead easy...
memcpy(RDest, RSrc, NumPixels);
memcpy(GDest, GSrc, NumPixels);
memcpy(BDest, BSrc, NumPixels);
return(NumPixels);
} // BlitSimple

signed int BlitAlpha(unsigned char *RDest, unsigned char *GDest, unsigned char *BDest, const unsigned long int &DestW, const unsigned long int &DestH, 
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc, unsigned char *Alpha, const unsigned long int &SrcW, const unsigned long int &SrcH,
 const unsigned long int &OffX, const unsigned long int &OffY)
{
unsigned long int DonePixels = 0, Pixel, DestPixel;

for(unsigned long int YCoord = 0; YCoord < SrcH; YCoord++)
	{
	unsigned long int DestY;
	DestY = YCoord + OffY;
	for(unsigned long int XCoord = 0; XCoord < SrcW; XCoord++)
		{
		unsigned long int DestX;
		DestX = XCoord + OffX;
		if(DestY > 0 && DestY < DestH && DestX > 0 && DestX < DestW)
			{
			Pixel = XCoord + (YCoord * SrcW);
			if(Alpha[Pixel] > 0)
				{
				float AlphaFrac, AlphaFracInv;
				unsigned long int RTemp, GTemp, BTemp;

				// we're NOT assuming premultiplied alpha at this point
				AlphaFrac = (Alpha[Pixel] * (1.0f / 255.0f));
				AlphaFracInv = 1.0f - AlphaFrac;
				DestPixel = DestX + (DestY * DestW);
				
				RTemp = (unsigned long int)(((float)RDest[DestPixel] * AlphaFracInv) + (AlphaFrac * (float)RSrc[Pixel]));
				GTemp = (unsigned long int)(((float)GDest[DestPixel] * AlphaFracInv) + (AlphaFrac * (float)GSrc[Pixel]));
				BTemp = (unsigned long int)(((float)BDest[DestPixel] * AlphaFracInv) + (AlphaFrac * (float)BSrc[Pixel]));
				
				RDest[DestPixel] = (unsigned char)RTemp;
				GDest[DestPixel] = (unsigned char)GTemp;
				BDest[DestPixel] = (unsigned char)BTemp;

				DonePixels++;
				} // if
			} // if
		} // for
	} // for

return(DonePixels);
} // BlitAlpha
