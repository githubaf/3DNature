/* PlotGUI.cvels
** Functions for plotting pixels.
** Copyright Questar Productions, October, 1995
*/

#include "WCS.h"
#ifndef __AROS__
   #include <proto/Picasso96.h>
   #include <cybergraphx/cybergraphics.h>
   #include <proto/cybergraphics.h>
#else
   #include <cybergraphx/cybergraphics.h>
   #include <proto/cybergraphics.h>
#endif

#include <Proto.h>

extern struct Library *CyberGfxBase;

#define RENDER_SCREEN_DITHER_SIZE 4096

// https://stackoverflow.com/questions/54372456/is-this-a-correct-implementation-of-ordered-dithering
int BAYER_PATTERN_16X16[16][16] =   {   //  16x16 Bayer Dithering Matrix.  Color levels: 256
		{     0, 191,  48, 239,  12, 203,  60, 251,   3, 194,  51, 242,  15, 206,  63, 254  },
		{   127,  64, 175, 112, 139,  76, 187, 124, 130,  67, 178, 115, 142,  79, 190, 127  },
		{    32, 223,  16, 207,  44, 235,  28, 219,  35, 226,  19, 210,  47, 238,  31, 222  },
		{   159,  96, 143,  80, 171, 108, 155,  92, 162,  99, 146,  83, 174, 111, 158,  95  },
		{     8, 199,  56, 247,   4, 195,  52, 243,  11, 202,  59, 250,   7, 198,  55, 246  },
		{   135,  72, 183, 120, 131,  68, 179, 116, 138,  75, 186, 123, 134,  71, 182, 119  },
		{    40, 231,  24, 215,  36, 227,  20, 211,  43, 234,  27, 218,  39, 230,  23, 214  },
		{   167, 104, 151,  88, 163, 100, 147,  84, 170, 107, 154,  91, 166, 103, 150,  87  },
		{     2, 193,  50, 241,  14, 205,  62, 253,   1, 192,  49, 240,  13, 204,  61, 252  },
		{   129,  66, 177, 114, 141,  78, 189, 126, 128,  65, 176, 113, 140,  77, 188, 125  },
		{    34, 225,  18, 209,  46, 237,  30, 221,  33, 224,  17, 208,  45, 236,  29, 220  },
		{   161,  98, 145,  82, 173, 110, 157,  94, 160,  97, 144,  81, 172, 109, 156,  93  },
		{    10, 201,  58, 249,   6, 197,  54, 245,   9, 200,  57, 248,   5, 196,  53, 244  },
		{   137,  74, 185, 122, 133,  70, 181, 118, 136,  73, 184, 121, 132,  69, 180, 117  },
		{    42, 233,  26, 217,  38, 229,  22, 213,  41, 232,  25, 216,  37, 228,  21, 212  },
		{   169, 106, 153,  90, 165, 102, 149,  86, 168, 105, 152,  89, 164, 101, 148,  85  }
};

USHORT AltDither8Colors[16]
={
#ifndef DAVE_WARNER
 0x89b,	/* 0, gray-blue */
 0x000,	/* 1, black */
 0xddd,	/* 2, almost white */
#else /* DAVE_WARNER */
 0x000,	/* 0, gray-blue, now black */
 0xfff,	/* 1, black, now white */
 0xbbb,	/* 2, almost white, now greyish */
#endif /* DAVE_WARNER */
 0xb10, /* 3, red */
 0x348,	/* 4, dark blue */
 0x392,	/* 5, green */
 0x37c,	/* 6, med blue */
 0xdd2,	/* 7, yellow */
 0xfff,	/* 8-15, 3 BPpP Dither colors */  //     Red+Blue+Green
 0xff0, //    Red+Green
 0xf0f, //    Red+Blue
 0xf00, //    Red
 0x0ff, //    Blue+Green
 0x0f0, //    Green
 0x00f, //    Blue
 0x000  //    Black
 };

// Fuellen mit AltColors[16], dann 128-155 mit 7Bit Dithercolors fuellen
WORD Alt256Colors[256];

USHORT Used[256]={0};  // Table of used colors, i.e. color has already been used for color sorings

// we shuffle the colors a bit to fit the dithercolortable better to the original 16 colors.This table translates the access
// of the Plot-Functions to the sfuffled table
WORD DitherColorTranslationTable[256]={0};

#include <intuition/intuition.h>


#ifndef MIN
#define MIN(a,b)            (((a) < (b)) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef CLAMP
//  This produces faster code without jumps
#define     CLAMP( x, xmin, xmax )      (x) = MAX( (xmin), (x) );   \
                                        (x) = MIN( (xmax), (x) )
#define     CLAMPED( x, xmin, xmax )    MAX( (xmin), MIN( (xmax), (x) ) )
#endif


unsigned int makeDitherBayerRgbnLevels(unsigned char pixel, int x, int y, int Levels )
{
    int row = y & 15;
    const int col = x & 15;   //  x % 16

    const int t     = BAYER_PATTERN_16X16[col][row];
    const int corr  = t / (Levels-1);

    int ncolors = Levels-1;  //(1 << BitsPerPlane) -1;
    int divider = 256 / ncolors;

    int i1  = (pixel   + corr) / divider; CLAMP( i1, 0, ncolors );

    return i1;
}

//// Alexander: Find best matching color
//// returns index to best matching free color in AltDitherColors
//unsigned int FindBestColor(USHORT *AltColors, SHORT *AltDitherColors, USHORT *Used, USHORT Index)
//{
//	unsigned int i;
//	unsigned int MinDiff2=USHRT_MAX;
//	unsigned int MinI=0;
//	unsigned int Diff2;
//
//	unsigned int  Red1,Grn1,Blu1,Red2,Grn2,Blu2;
//	Red1=(AltColors[Index] & 0xf00) >>8;
//	Grn1=(AltColors[Index] & 0x0f0) >>4;
//	Blu1=(AltColors[Index] & 0x00f) >>0;
//
////	printf("Index=%2d AltColors[%d]=%03x Red1 = %x Grn1=%x Blu1=%x\n",Index,Index,AltColors[Index],Red1,Grn1,Blu1);
//
//	for(i=0;i<256;i++)
//	{
//		if(AltDitherColors[i]==-1)	break;    // end of table reached
//		Red2=(AltDitherColors[i] & 0xf00) >>8;
//		Grn2=(AltDitherColors[i] & 0x0f0) >>4;
//		Blu2=(AltDitherColors[i] & 0x00f) >>0;
//
//
//		Diff2=(Red1-Red2)*(Red1-Red2)+(Grn1-Grn2)*(Grn1-Grn2)+(Blu1-Blu2)*(Blu1-Blu2);
//
////		printf("i=%2d Red2 = %x Grn2=%x Blue=%x Diff2=%d\n",i,Red2,Grn2,Blu2,Diff2);
//
//		if(Diff2<MinDiff2 && Used[i]!=TRUE)
//		{
//			MinDiff2=Diff2;
//			MinI=i;
//		}
//	}
//
//	Used[MinI]=TRUE;
//
//
//	Red2=(AltDitherColors[MinI] & 0xf00) >>8;
//	Grn2=(AltDitherColors[MinI] & 0x0f0) >>4;
//	Blu2=(AltDitherColors[MinI] & 0x00f) >>0;
//
//	//printf("Soll: %2d: %x %x %x  --- Bester %3d: %x %x %x --- Diff2: %2d\n",Index,Red1,Grn1,Blu1,MinI,Red2,Grn2,Blu2, MinDiff2);
//	return MinI;
//}


//void SortColorTable(SHORT *TargetDitherColorTable,USHORT *OriginalColorTable,SHORT *DitherColorTranslationTable, UWORD LastColor)
//{
////	USHORT Temp;
////	Temp=TargetDitherColorTable[8];
////	// Color 8 should be white, Last color in new Colortable is white. Exchange them
////	TargetDitherColorTable[8]=TargetDitherColorTable[LastColor];
////	TargetDitherColorTable[LastColor]=Temp;
////
////	// swap indices in translation table as well so that access to last color is translated to color 8 now
////	Temp=DitherColorTranslationTable[8];
////	DitherColorTranslationTable[8]=DitherColorTranslationTable[LastColor];  // Color 8 should be white, Last color in new Colortable is white. Exchange them
////	DitherColorTranslationTable[LastColor]=Temp;
////
////	// Colors 8-15 grayscale should remain unchanged.
////	Used[ 8]=TRUE;
////
////    // Test for 16-Color-Screeen, first 8 colors need to be matched
////	{
////	int i;
////	for (i=0;i<7;i++)
////	{
////		unsigned int BestColor=FindBestColor(OriginalColorTable, TargetDitherColorTable, Used, i);
////		printf("Color %d: %03x   Best match: Color %d %03x \n",i,OriginalColorTable[i],BestColor, TargetDitherColorTable[BestColor]);
////
////		Temp=DitherColorTranslationTable[BestColor];
////		DitherColorTranslationTable[BestColor]=DitherColorTranslationTable[i];
////		DitherColorTranslationTable[i]=Temp;
////
////		// swap indices in translation table as well so that access to color points to the right one
////		Temp=DitherColorTranslationTable[BestColor];
////		DitherColorTranslationTable[BestColor]=DitherColorTranslationTable[i];
////		DitherColorTranslationTable[i]=Temp;
////
////	}
////	}
//	// fixed and hand crafted for 2x2x2
//	USHORT TempTargetDitherColorTable[]={
//			0x89b,  //  0 soll: gray-blue
//			0x000,  //  1       black
//			0xf0f,  //  2       almost white !bad!
//			0xf00,  //  3       red
//			0x0ff,  //  4       dark blue
//			0x0f0,  //  5       green
//			0x00f,  //  6       med blue
//			0xff0,  //  7       yellow    000 -> 111 und fff -> 8 fff
//
//			0xfff,  //  8  gray scale, don't move
//			0xddd,  //  9
//			0xbbb,  // 10
//			0x999,  // 11
//			0x777,  // 12
//			0x555,  // 13
//			0x333,  // 14
//			0x111   // 15
//	};
//	USHORT TempDitherColorTranslationTable[]={
//			1,    //  0  must point to 000
//			6,    //  1                00f
//			5,    //  2                0f0
//			4,    //  3                0ff
//			3,    //  4                f00
//	     	2,    //  5                f0f
//			7,    //  6                ff0
//			8,    //  7                fff  0k
//
//			8,    //  8                fff grauwerte ab hier, dont move
//			9,    //  9                ddd
//			10,   // 10                bbb
//			11,   // 11                999
//			12,   // 12                777
//			13,   // 13                444
//			14,   // 14                333
//			15    // 15                111
//	};
//	memcpy(TargetDitherColorTable,TempTargetDitherColorTable,sizeof(TempTargetDitherColorTable));
//	memcpy(DitherColorTranslationTable,TempDitherColorTranslationTable,sizeof(TempDitherColorTranslationTable));
//}


// ALEXANDER: 21_Oct Make Color table for 2x2x2 (8 colors) ... 6x6x6 Levels (216 colors) RGB
void Make_N_Levels_Palette4( WORD *NewColorTable, unsigned int Levels, unsigned int AvailColors, unsigned int Offset, USHORT *OldTable16)
{
	unsigned int i=0;
	unsigned int r,g,b;
	unsigned int LastColor;

	memset(NewColorTable,-1,256*sizeof(USHORT));
	memcpy(NewColorTable,OldTable16,16*sizeof(USHORT));  // copy old 16-Color table to begin of new color table

		if(Levels>6)
		{
			printf("Illegal Levels value /d!\n",Levels);
			Levels=2;
		}

	for (r=0;r<Levels;r++)
		for(g=0;g<Levels;g++)
			for(b=0;b<Levels;b++)
			{
				NewColorTable[i+Offset]= ((r*(255/(Levels-1))) /16)   << 8 |
						                 ((g*(255/(Levels-1))) /16)   << 4 |
						                  (b*(255/(Levels-1))  /16);
				//KPrintF("Colors[%ld]=0x%03lx\n",i,NewColorTable[i+Offset]);
				i++;
			}
//	LastColor=Offset+i-1;
//
	for (i=0;i<Levels*Levels*Levels;i++)
	{
		DitherColorTranslationTable[i]=i+Offset;
	}
//
//    // adapt new Dithered Color Table to original 16 Color table . Windows/Icons/Gray-Images should look similar
//	SortColorTable(NewColorTable,OldTable16,DitherColorTranslationTable,LastColor);

	switch (Levels)
	{
		case 2: // Spezialfall, handoptimiert. 8 dither colors. Original gray colors 8-15 remain unchanged
		{
			USHORT TempTargetDitherColorTable[]={
					0x89b,  //  0 soll: gray-blue
					0x000,  //  1       black
					0xf0f,  //  2       almost white !bad!
					0xf00,  //  3       red
					0x0ff,  //  4       dark blue
					0x0f0,  //  5       green
					0x00f,  //  6       med blue
					0xff0,  //  7       yellow    000 -> 111 und fff -> 8 fff

					0xfff,  //  8  gray scale, don't move
					0xddd,  //  9
					0xbbb,  // 10
					0x999,  // 11
					0x777,  // 12
					0x555,  // 13
					0x333,  // 14
					0x111   // 15
			};
			USHORT TempDitherColorTranslationTable[]={
					1,    //  0  must point to 000
					6,    //  1                00f
					5,    //  2                0f0
					4,    //  3                0ff
					3,    //  4                f00
					2,    //  5                f0f
					7,    //  6                ff0
					8,    //  7                fff  0k

					8,    //  8                fff grauwerte ab hier, dont move
					9,    //  9                ddd
					10,   // 10                bbb
					11,   // 11                999
					12,   // 12                777
					13,   // 13                444
					14,   // 14                333
					15    // 15                111
			};
			memcpy(NewColorTable,TempTargetDitherColorTable,sizeof(TempTargetDitherColorTable));
			memcpy(DitherColorTranslationTable,TempDitherColorTranslationTable,sizeof(TempDitherColorTranslationTable));
			break;
		}
		case 3:
		{
			if(AvailColors<64)
			{
				USHORT TempTargetDitherColorTable[]={
						0x89b,    //  0 soll: gray-blue
						0x000,    //  1       black
						0xf0f,    //  2       almost white
						0xf00,    //  3       red
						0x0ff,    //  4       dark blue
						0x0f0,    //  5       green
						0x70f,    //  6       med blue
						0xff0,    //  7       yellow

						0xfff,    //  8  gray scale
						0xddd,    //  9
						0xbbb,    // 10
						0x999,    // 11
						0x777,    // 12
						0x555,    // 13
						0x00f,    // 14 original 333
						0x007,    // 15 original 111

						0x070,    // 16
						0x077,    // 17
						0x07f,    // 18
						0x0f7,    // 19
						0x700,    // 20
						0x707,    // 21
						0x770,    // 22
						0x77f,    // 23
						0x7f0,    // 24
						0x7f7,    // 25
						0x7ff,    // 26
						0xf07,    // 27
						0xf70,    // 28
						0xf77,    // 29
						0xf7f,    // 30
						0xff7,    // 31
				};

				USHORT TempDitherColorTranslationTable[]={
						1,   //  0  must point to 000
						15,   //  1                007
						14,   //  2                00f
						16,   //  3                070
						17,   //  4                077
						10,   //  5                07f
						5,   //  6                0f0
						19,   //  7                0f7

						4,   //  8                0ff
						20,   //  9                700
						21,   // 10                707
						6,   // 11                70f
						22,   // 12                770
						12,   // 13                777
						23,   // 14                77f
						24,   // 15                7f0
						25,   // 16                7f7
						26,	  // 17				   7ff
						3,   // 18                f00
						27,   // 19                f07
						2,   // 20                f0f
						28,   // 21                f70
						29,   // 22                f77
						30,   // 23                f7f
						7,   // 24                ff0
						31,   // 25                ff7
						8    // 26                fff
				};


				memcpy(NewColorTable,TempTargetDitherColorTable,sizeof(TempTargetDitherColorTable));
				memcpy(DitherColorTranslationTable,TempDitherColorTranslationTable,sizeof(TempDitherColorTranslationTable));
			}
			break;
		}


	}

}

// ALEXANDER: 21_Oct Dithers to 2x2x2 RGB-levels (Levels, not bits), i.e. 8 colors
void BayerDither_2_2_2_ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	static int Init=TRUE;
	unsigned char PixelComponentR;
	unsigned char PixelComponentG;
	unsigned char PixelComponentB;
    UBYTE Col;

    if(Init)
    {
    	printf("using %s()...\n",__func__);
    	Init=FALSE;
    }

	PixelComponentR=makeDitherBayerRgbnLevels(Bitmap[0][zip],x,y,2);  // 2 Levels Red
	PixelComponentG=makeDitherBayerRgbnLevels(Bitmap[1][zip],x,y,2);  // 2 Levels Green
	PixelComponentB=makeDitherBayerRgbnLevels(Bitmap[2][zip],x,y,2);  // 2 Levels Blue

	// 1 Bit per RGB component
	// We have 8 RGB-Colors. Convert that value to pen-Number 8...15

	Col=  4*PixelComponentR +
	      2*PixelComponentG +
		    PixelComponentB;

	Col=0+Col; // 8 Pens are used by the original program, dither colors start with offset 0, we want to keep the gray values exactly
	Col=DitherColorTranslationTable[Col];  // and translate in case Colortable has been shuffled to match original 16 colors better

	//KPrintF("0x%08lx\n",Col);

	WriteChunkyPixels(win->RPort, x, y,x,y,&Col,4);  // Only one pixel, so BytesPerRow is irrelevant
}



// ALEXANDER: 21_Oct Dithers to 3x3x3 RGB-levels (Levels, not bits), i.e. 27 colors
void BayerDither_3_3_3_ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	static int Init=TRUE;
	unsigned char PixelComponentR;
	unsigned char PixelComponentG;
	unsigned char PixelComponentB;
    UBYTE Col;

    if(Init)
    {
    	printf("using %s()...\n",__func__);
    	Init=FALSE;
    }

	PixelComponentR=makeDitherBayerRgbnLevels(Bitmap[0][zip],x,y,3);  // 3 Levels Red
	PixelComponentG=makeDitherBayerRgbnLevels(Bitmap[1][zip],x,y,3);  // 3 Levels Green
	PixelComponentB=makeDitherBayerRgbnLevels(Bitmap[2][zip],x,y,3);  // 3 Levels Blue

	// 2 Bits per RGB component
	// We have 27 RGB-Colors. Convert that value to pen-Number 5...31

	Col=  9*PixelComponentR +
	      3*PixelComponentG +
		    PixelComponentB;

//	Col=5+Col; // no Pens are used by the original program, dither colors start with offset 0
	Col=DitherColorTranslationTable[Col];  // and translate in case Colortable has been shuffled to match original 16 colors better

	//KPrintF("0x%08lx\n",Col);

	WriteChunkyPixels(win->RPort, x, y,x,y,&Col,4);  // Only one pixel, so BytesPerRow is irrelevant
}

// ALEXANDER: 21_Oct Dithers to 4x4x4 RGB-levels (Levels, not bits), i.e. 64 colors
void BayerDither_4_4_4_ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	static int Init=TRUE;
	unsigned char PixelComponentR;
	unsigned char PixelComponentG;
	unsigned char PixelComponentB;
    UBYTE Col;

    if(Init)
    {
    	printf("using %s()...\n",__func__);
    	Init=FALSE;
    }

	PixelComponentR=makeDitherBayerRgbnLevels(Bitmap[0][zip],x,y,4);  // 4 Levels Red
	PixelComponentG=makeDitherBayerRgbnLevels(Bitmap[1][zip],x,y,4);  // 4 Levels Green
	PixelComponentB=makeDitherBayerRgbnLevels(Bitmap[2][zip],x,y,4);  // 4 Levels Blue

	// 2 Bits per RGB component
	// We have 64 RGB-Colors. Convert that value to pen-Number 0...63

	Col= 16*PixelComponentR +
	      4*PixelComponentG +
		    PixelComponentB;

//	Col=0+Col; // no Pens are used by the original program, dither colors start with offset 0
	Col=DitherColorTranslationTable[Col];  // and translate in case Colortable has been shuffled to match original 16 colors better

	//KPrintF("0x%08lx\n",Col);

	WriteChunkyPixels(win->RPort, x, y,x,y,&Col,4);  // Only one pixel, so BytesPerRow is irrelevant
}


// ALEXANDER: 21_Oct Dithers to 5x5x5 RGB-levels (Levels, not bits), i.e. 125 colors
void BayerDither_5_5_5_ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	static int Init=TRUE;
	unsigned char PixelComponentR;
	unsigned char PixelComponentG;
	unsigned char PixelComponentB;
    UBYTE Col;

    if(Init)
    {
    	printf("using %s()...\n",__func__);
    	Init=FALSE;
    }

	PixelComponentR=makeDitherBayerRgbnLevels(Bitmap[0][zip],x,y,5);  // 5 Levels Red
	PixelComponentG=makeDitherBayerRgbnLevels(Bitmap[1][zip],x,y,5);  // 5 Levels Green
	PixelComponentB=makeDitherBayerRgbnLevels(Bitmap[2][zip],x,y,5);  // 5 Levels Blue

	// 3 Bits per RGB component (for 5 levels, 3 Bits are needed. 3 values not used
	// We have 125 RGB-Colors. Convert that value to pen-Number 3...127

	Col= 25*PixelComponentR +
	      5*PixelComponentG +
		    PixelComponentB;

//	Col=3+Col; // Pens 0.1 are used by the original program, dither colors start with offset 16
	Col=DitherColorTranslationTable[Col];  // and translate in case Colortable has been shuffled to match original 16 colors better

	//KPrintF("0x%08lx\n",Col);

	WriteChunkyPixels(win->RPort, x, y,x,y,&Col,4);  // Only one pixel, so BytesPerRow is irrelevant
}


// ALEXANDER: 21_Oct Dithers to 6x6x6 RGB-levels (Levels, not bits), i.e. 216 colors
void BayerDither_6_6_6_ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	static int Init=TRUE;
	unsigned char PixelComponentR;
	unsigned char PixelComponentG;
	unsigned char PixelComponentB;
    UBYTE Col;

    if(Init)
    {
    	printf("using %s()...\n",__func__);
    	Init=FALSE;
    }

	PixelComponentR=makeDitherBayerRgbnLevels(Bitmap[0][zip],x,y,6);  // 6 Levels Red
	PixelComponentG=makeDitherBayerRgbnLevels(Bitmap[1][zip],x,y,6);  // 6 Levels Green
	PixelComponentB=makeDitherBayerRgbnLevels(Bitmap[2][zip],x,y,6);  // 6 Levels Blue

	// 3 Bits per RGB component (for 6 levels, 3 Bits are needed. 2 values not used
	// We have 216 RGB-Colors. Convert that value to pen-Number 16 + 0...215

	Col= 36*PixelComponentR +
	      6*PixelComponentG +
		    PixelComponentB;

//	Col=16+Col; // Pens 0...15 are used by the original program, dither colors start with offset 16
	Col=DitherColorTranslationTable[Col];  // and translate in case Colortable has been shuffled to match original 16 colors better

	//KPrintF("0x%08lx\n",Col);

	WriteChunkyPixels(win->RPort, x, y,x,y,&Col,4);  // Only one pixel, so BytesPerRow is irrelevant

}

// fills 256 entries color table with 16 original WCS colors 0-15 and 128 Dither colors 128-255 format "232"
void fill256ColorsPalette128(USHORT *ColorTable256, USHORT *ColorTable16)
{
	unsigned int i;
	unsigned int r,g,b;
	unsigned char ColorComponent2Bit[]={0,5,10,15};  // n x 15/3,  4 Values == 3 segements
	unsigned char ColorComponent3Bit[]={0,2,4,6,8,10,12,15};  // n x 15/7,  8 Values == 7 segements

	memcpy(ColorTable256,ColorTable16,16*sizeof(USHORT));  // 16 entries a 2 bytes
	i=128;
	for (r=0;r<4;r++)
		for(g=0;g<8;g++)
			for(b=0;b<4;b++)
			{
				// 4 bit Color entries
				ColorTable256[i]=(ColorComponent2Bit[r]<<8)+(ColorComponent3Bit[g]<<4)+ColorComponent2Bit[b];
				i++;
			}
}

void ScreenPixelPlotClassic(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
short Col;
double FloatCol;

 FloatCol = 8.0 + 7.999 * (765 - Bitmap[0][zip] - Bitmap[1][zip] - Bitmap[2][zip]) / 765.0;
 Col = FloatCol + ROUNDING_KLUDGE;
 if ((x + y) & 0x01)
  {
  if((FloatCol - (double)Col) > .5)
   {
   if (Col < 15)
    Col++;
   } /* if color less than max for specific gradient range */
  } /* if */
 SetAPen(win->RPort, Col);
 WritePixel(win->RPort, x, y);

} /* PixelPlot */


/***********************************************************************/
void getGfxInformation(void)
{
	WORD rect[4]={0};

	printf("WCSScrn->ViewPort = 0x%08x\n",&WCSScrn->ViewPort);
	printf("WCSScrn->RastPort.BitMap->Depth = %d\n",WCSScrn->RastPort.BitMap->Depth);
	printf("WCSScrn->Flags=0x%04x\n",WCSScrn->Flags);
	printf("WCSScrn->ViewPort.DWidth %d x WCSScrn->ViewPort.DHeight %d\n",WCSScrn->ViewPort.DWidth,WCSScrn->ViewPort.DHeight);

#ifndef __AROS__
    	  if(P96Base)
    	  {
    		  ULONG IsP96Screen=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_ISP96);
    		  printf("Screen is %s a P96 Screen\n",IsP96Screen? "" : "not ");
    		  if(IsP96Screen)
    		  {
    			  ULONG Value=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_BITSPERPIXEL);
    			  printf("Screen has %d Bits per Pixel\n",Value);
    			  Value=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_BYTESPERPIXEL);
    			  printf("Screen has %d Bytes per Pixel\n",Value);
    		  }
    	  }
    	  else
#endif
    		  if(CyberGfxBase)
    		  {
    			  ULONG IsCgfxScreen=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_ISCYBERGFX);
    			  printf("Screen is %s a CGFX Screen\n",IsCgfxScreen? "" : "not ");
    			  if(IsCgfxScreen)
    			  {
    				  ULONG Value=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_DEPTH);
    				  printf("Screen has %d Bits per Pixel\n",Value);
    				  Value=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_BPPIX);
    				  printf("Screen has %d Bytes per Pixel\n",Value);
    			  }
    		  }
}

// AF, initis the ScreenPixelPlot function pointer to original function (gray scaled)
void initScreenPixelPlotFnct()
{
	printf("Alexander: %s %s()called\n",__FILE__,__func__);
	ScreenPixelPlot=ScreenPixelPlotClassic;
}

// P96 high/True color
void ScreenPixelPlotP96(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
#ifndef __AROS__
	p96WritePixel(win->RPort, x, y,(Bitmap[0][zip]<<16) + (Bitmap[1][zip]<<8) + Bitmap[2][zip]);
#endif
}

// CyberGraphX high/True color
void ScreenPixelPlotCGFX(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	WriteRGBPixel(win->RPort, x, y,(Bitmap[0][zip]<<16) + (Bitmap[1][zip]<<8) + Bitmap[2][zip]);
}


// AF, set ScreenPixelPlot function pointer to old function, new color-dithered function or RTG function
void setScreenPixelPlotFnct(struct Settings settings)
{
	printf("Alexander: %s %s()called\n",__FILE__,__func__);
	printf("settings.renderopts=%04x",settings.renderopts);

	switch(settings.renderopts & 0x30)
	{
		case 0x10:  // render Screen, gray
			printf("should plot gray scaled\n");
			ScreenPixelPlot=ScreenPixelPlotClassic;
			LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
			SetRast(RenderWind0->RPort, 8); // 8=white
			break;
		case 0x20:   // render Screen, color
		case 0x30:   // render screen gray + color
			printf("should plot colored\n");

			switch(WCSScrn->RastPort.BitMap->Depth)
			{
				case 4:
				{
					Make_N_Levels_Palette4(Alt256Colors,2,8,0,AltColors);  // prepare Color Table  for 4-Bit-Screens 2x2x2 leves = 8 colors // ALEXANDER
					ScreenPixelPlot=BayerDither_2_2_2_ScreenPixelPlot; //dither 222 (8 colors 0...7), original gray colors 8..15 unchanged
					LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
					SetRast(RenderWind0->RPort, 8); // 8=white
					break;
				}

				case 5:
				{
					Make_N_Levels_Palette4(Alt256Colors,3,32,0,AltColors);  // prepare Color Table  for 5-Bit-Screens 3x3x3 leves = 27 colors // ALEXANDER
					ScreenPixelPlot=BayerDither_3_3_3_ScreenPixelPlot; //dither 333 (27 colors 5...31), original color 0..4 unchanged
					LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
					SetRast(RenderWind0->RPort, 8); // 8=white
					break;
				}
				case 6:
				{
					Make_N_Levels_Palette4(Alt256Colors,3,64,16,AltColors);  // // prepare Color Table  for 6-Bit-Screens 3x3x3 leves = 27 colors, keep original 16 colors unchanged // ALEXANDER
					ScreenPixelPlot=BayerDither_3_3_3_ScreenPixelPlot; //dither 333 (64 colors 16...43), all original colors kept
					LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
					SetRast(RenderWind0->RPort, 8); // 8=white
					break;
				}
				case 7:
				{
					Make_N_Levels_Palette4(Alt256Colors,4,128,16,AltColors);  // // prepare Color Table  for 7-Bit-Screens 4x4x4 leves = 64 colors // ALEXANDER
					ScreenPixelPlot=BayerDither_4_4_4_ScreenPixelPlot; //dither 444 (125 colors) original color 0...16 unchanged
					LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
					SetRast(RenderWind0->RPort, 8); // 8=white
					break;
				}
				case 8:
				{
					Make_N_Levels_Palette4(Alt256Colors,6,256,16,AltColors);  // // prepare Color Table  for 8-Bit-Screens 6x6x6 leves = 216 colors, original remain unchanged // ALEXANDER
                    #ifndef __AROS__
					if(P96Base)
					{
						ULONG IsP96Screen=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_ISP96);
						printf("Screen is %s a P96 Screen\n",IsP96Screen? "" : "not ");
						if(IsP96Screen)
						{
							ULONG BitsPerPixel, BytesPerPixel;

							BitsPerPixel=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_BITSPERPIXEL);
							printf("Screen has %d Bits per Pixel\n",BitsPerPixel);
							BytesPerPixel=p96GetBitMapAttr(WCSScrn->RastPort.BitMap, P96BMA_BYTESPERPIXEL);
							printf("Screen has %d Bytes per Pixel\n",BytesPerPixel);

							if(BitsPerPixel==8)  // 8-Bit P96 screen
							{
								printf("BayerDither_6_6_6_ScreenPixelPlot\n");
								//ScreenPixelPlot=BayerDither128ColorsScreenPixelPlot;
								ScreenPixelPlot=BayerDither_6_6_6_ScreenPixelPlot;
								LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
								SetRast(RenderWind0->RPort, 8); // 8=white
							}
							else
							{
								// High/true color
								printf("ScreenPixelPlotP96 full color\n");
								ScreenPixelPlot=ScreenPixelPlotP96;
								LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
								SetRast(RenderWind0->RPort, 8); // 8=white
							}
							break;
						}
        				default:
        				{
        					printf("Alexander: Amiga 256 Color Screen\n");
        					printf("ScreenPixelPlotDither256\n");
        					AF_DEBUG("default: setze BayerDither_6_6_6_ScreenPixelPlot()\n");
        					//ScreenPixelPlot=BayerDither128ColorsScreenPixelPlot; //dither 232 (128 colors in upper half of 256 color color table)
        					ScreenPixelPlot=BayerDither_6_6_6_ScreenPixelPlot;
        					AF_DEBUG("default: BayerDither_6_6_6_ScreenPixelPlot() gesetzt");
        					AF_DEBUG("ScreenPixelPlotDither256 gesetzt\n");
        					LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);  // ALEXANDER
        					SetRast(RenderWind0->RPort, 8); // 8=white
        					AF_DEBUG("LoadRGB4() und SetRast() aufgerufen.\n");
        				}
					}
					else
                    #endif
					if(CyberGfxBase)
					{
						ULONG IsCgfxScreen=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_ISCYBERGFX);
						printf("Screen is %s a CGFX Screen\n",IsCgfxScreen? "" : "not ");
						if(IsCgfxScreen)
						{
							ULONG BitsPerPixel, BytesPerPixel;
							BitsPerPixel=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_DEPTH);
							printf("Screen has %d Bits per Pixel\n",BitsPerPixel);
							BytesPerPixel=GetCyberMapAttr(WCSScrn->RastPort.BitMap, CYBRMATTR_BPPIX);
							printf("Screen has %d Bytes per Pixel\n",BytesPerPixel);

							if(BitsPerPixel==8)  // 8-Bit CyberGraphX screen
							{
								printf("BayerDither_6_6_6_ScreenPixelPlot\n");
								//ScreenPixelPlot=BayerDither128ColorsScreenPixelPlot;
								ScreenPixelPlot=BayerDither_6_6_6_ScreenPixelPlot;
								LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
								SetRast(RenderWind0->RPort, 8); // 8=white
							}
							else            // true or high color CyberGraphX screen
							{
								printf("ScreenPixelPlotCgfx full color\n");
								ScreenPixelPlot=ScreenPixelPlotCGFX;
								LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
								SetRast(RenderWind0->RPort, 8);
							}
						}
						else // neither P96 nor CyberGraphiX, i.e. Amiga 8-Bit Screen
						{
							printf("Alexander: Amiga 256 Color Screen\n");
							printf("ScreenPixelPlotP96Dither256\n");
							//ScreenPixelPlot=BayerDither128ColorsScreenPixelPlot;
							ScreenPixelPlot=BayerDither_6_6_6_ScreenPixelPlot;
							LoadRGB4(&WCSScrn->ViewPort, &Alt256Colors[0], 256);
							SetRast(RenderWind0->RPort, 8); // 8=white
						}
					} // if(CyberGfxBase)
				} // case 8:
			} // switch(WCSScrn->RastPort.BitMap->Depth)
	} // switch(settings.renderopts & 0x30)
}

/***********************************************************************/
/***********************************************************************/

void NoRGBScreenPixelPlot(struct Window *win,
	double FloatCol, short ColMax, short x, short y)
{
short Col;

 Col = FloatCol + ROUNDING_KLUDGE;
 if(DTable)
  {
  if((FloatCol - (double)Col) > DTable[(DMod++ % RENDER_SCREEN_DITHER_SIZE)])
   {
   if (Col < ColMax)
    Col++; 
   } /* if color less than max for specific gradient range */
  } /* if */
 SetAPen(win->RPort, AltPen[Col]);
 WritePixel(win->RPort, x, y);

} /* NoRGBScreenPixelPlot() */

/**********************************************************************/

void NoDitherScreenPixelPlot(struct Window *win, short Col, short x, short y)
{

 SetAPen(win->RPort, Col);
 WritePixel(win->RPort, x, y);

} /* NoDitherScreenPixelPlot() */
