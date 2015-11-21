// SnazzyBlit.h
// Blit support code 

#include <windows.h>

void RastBlastBlock(HDC BlastDest, unsigned long int X, unsigned long int Y,
 unsigned long int W, unsigned long int H, unsigned char *R, unsigned char *G, unsigned char *B);

// copies RGB values from Src to Dest only where Mask value equals MaskVal
signed int BlitWhere(unsigned long int NumPixels, unsigned char *RDest, unsigned char *GDest, unsigned char *BDest,
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc, unsigned char *Mask, unsigned char MaskVal);

// just memcopies the whole thing over
signed int BlitSimple(unsigned long int NumPixels, unsigned char *RDest, unsigned char *GDest, unsigned char *BDest,
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc);

// blits using source alpha channel and x/y offset
signed int BlitAlpha(unsigned char *RDest, unsigned char *GDest, unsigned char *BDest, const unsigned long int &DestW, const unsigned long int &DestH, 
 unsigned char *RSrc, unsigned char *GSrc, unsigned char *BSrc, unsigned char *Alpha, const unsigned long int &SrcW, const unsigned long int &SrcH,
 const unsigned long int &OffX, const unsigned long int &OffY);
