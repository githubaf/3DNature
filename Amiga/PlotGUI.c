/* PlotGUI.c
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
USHORT AltDither128Colors[256];



//  Color ordered dither using 3 bits per pixel (1 bit per color plane)
//void makeDitherBayerRgb3bpp( unsigned char* pixels, int width, int height )
//{
//    int col = 0;
//    int row = 0;
//
//    for( int y = 0; y < height; y++ )
//    {
//        row = y & 15;   //  y % 16
//
//        for( int x = 0; x < width; x++ )
//        {
//            col = x & 15;   //  x % 16
//
//            pixels[x * 3 + 0]   = (pixels[x * 3 + 0] > BAYER_PATTERN_16X16[col][row] ? 255 : 0);
//            pixels[x * 3 + 1]   = (pixels[x * 3 + 1] > BAYER_PATTERN_16X16[col][row] ? 255 : 0);
//            pixels[x * 3 + 2]   = (pixels[x * 3 + 2] > BAYER_PATTERN_16X16[col][row] ? 255 : 0);
//        }
//
//        pixels  += width * 3;
//    }
//}

unsigned char makeDitherBayerRgb1bpp( unsigned char pixel, int x, int y )
{
    int col = 0;
    int row = 0;
    unsigned char NewValue;

        row = y & 15;   //  y % 16
        col = x & 15;   //  x % 16

            NewValue = pixel > BAYER_PATTERN_16X16[col][row] ? 255 : 0;

            return NewValue;
}

#include <intuition/intuition.h>
void BayerDither8ColorsScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
//	static int Init=TRUE;
	unsigned char PixelComponenR;
	unsigned char PixelComponenG;
	unsigned char PixelComponenB;
    short Col;

// Experiment Gamma Correction
//	if(Init==TRUE)
//	{
//        int x=0,y=0;
//		Init=FALSE;
//        for(y=0;y<16;y++)
//        {
//                for(x=0;x<16;x++)
//                {
//                //      BAYER_PATTERN_16X16[x][y]=(BAYER_PATTERN_16X16[x][y]/255.0*BAYER_PATTERN_16X16[x][y]/255.0)*255;
//                BAYER_PATTERN_16X16[x][y]=pow(BAYER_PATTERN_16X16[x][y]/255.0,0.5)*255;
//                }
//        }
//	}

	PixelComponenR=makeDitherBayerRgb1bpp(Bitmap[0][zip],x,y);
	PixelComponenG=makeDitherBayerRgb1bpp(Bitmap[1][zip],x,y);
	PixelComponenB=makeDitherBayerRgb1bpp(Bitmap[2][zip],x,y);

	// We have 8 RGB-Colors. Each RGs is either 255 or 0 after Bayer-Mapping. Convert that value to pen-Number 8...15
	// 123 321 132 213 231 312
	Col=((PixelComponenR == 255) ? 4:0) +
	     ((PixelComponenG == 255) ? 2:0) +
		 ((PixelComponenB == 255) ? 1:0);


	Col=15-Col; // Pens 8...15 are for render window

	SetAPen(win->RPort, Col);
	WritePixel(win->RPort, x, y);
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

// RGB-Test
//void ScreenPixelPlotNew(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
//{
//#ifndef __AROS__
//	if(P96Base)
//	{
//		if(p96GetBitMapAttr(win->RPort->BitMap, P96BMA_ISP96))
//		{
//			if(p96GetBitMapAttr(win->RPort->BitMap, P96BMA_BITSPERPIXEL)>=15)
//			{
//				p96WritePixel(win->RPort, x, y,(Bitmap[0][zip]<<16) + (Bitmap[1][zip]<<8) + Bitmap[2][zip]);
//			}
//			else
//			{
//				ScreenPixelPlotClassic(win, Bitmap, x, y, zip);  // the old way
//			}
//		}
//		else
//		{
//			ScreenPixelPlotClassic(win, Bitmap, x, y, zip);  // the old way
//		}
//	}
//	else
//#endif
//	if(CyberGfxBase)
//	{
//		if(GetCyberMapAttr(win->RPort->BitMap,CYBRMATTR_ISCYBERGFX))
//		{
//			if(GetCyberMapAttr(win->RPort->BitMap, CYBRMATTR_DEPTH)>=15)
//			{
//				WriteRGBPixel(win->RPort, x, y,(Bitmap[0][zip]<<16) + (Bitmap[1][zip]<<8) + Bitmap[2][zip]);
//			}
//			else
//			{
//				ScreenPixelPlotClassic(win, Bitmap, x, y, zip);  // the old way
//			}
//		}
//		else
//		{
//			ScreenPixelPlotClassic(win, Bitmap, x, y, zip);  // the old way
//		}
//	}
//	else
//	{
//		ScreenPixelPlotClassic(win, Bitmap, x, y, zip);  // the old way
//	}
//}

/***********************************************************************/
void getGfxInformation(void)
{
	printf("WCSScrn->RastPort.BitMap->Depth=%d\n",WCSScrn->RastPort.BitMap->Depth);
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
			// check if RTG screen and if 256 colors
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

				  if(BitsPerPixel<=8)  // 8-Bit CyberGraphX screen
				  {
					  printf("ScreenPixelPlotP96Dither256\n");
					  //ScreenPixelPlot=ScreenPixelPlotP96Dither256;
				  }
				  else            // true or high color CyberGraphX screen
				  {
					  printf("ScreenPixelPlotP96 full color\n");
					  ScreenPixelPlot=ScreenPixelPlotP96;
					  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
    				  SetRast(RenderWind0->RPort, 8); // 8=white
				  }
			  }
    		  else
    		  {
				  printf("ScreenPixelPlotDither8\n");
				  ScreenPixelPlot=BayerDither8ColorsScreenPixelPlot; //dither 111 (8 colors in upper half of 16 color color table)
				  LoadRGB4(&WCSScrn->ViewPort, &AltDither8Colors[0], 16);
				  SetRast(RenderWind0->RPort, 8); // 8=white
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
    					  printf("ScreenPixelPlotCgfxDither256\n");
    					  //ScreenPixelPlot=ScreenPixelPlotCgfxDither256;
    				  }
    				  else            // true or high color CyberGraphX screen
    				  {
    					  printf("ScreenPixelPlotCgfx full color\n");
    					  ScreenPixelPlot=ScreenPixelPlotCGFX;
    					  LoadRGB4(&WCSScrn->ViewPort, &AltColors[0], 16);
        				  SetRast(RenderWind0->RPort, 8);
    				  }
    			  }
        		  else
        		  {
    				  printf("ScreenPixelPlotDither8\n");
    				  // Palette noch setzen!
    				  ScreenPixelPlot=BayerDither8ColorsScreenPixelPlot; //dither 111 (8 colors in upper half of 16 color color table)
    				  LoadRGB4(&WCSScrn->ViewPort, &AltDither8Colors[0], 16);
    				  SetRast(RenderWind0->RPort, 8); // 8color dithered -> 8=white
        		  }
    		  }
    		  else // no RTG system installed
    		  {
    			  printf("Nono-RTG Screen\n");
    			  if (WCSScrn->RastPort.BitMap->Depth==8)  // 256 colors
    			  {
    				  printf("ScreenPixelPlotDither128\n");
    				  //ScreenPixelPlot=ScreenPixelPlotDither128;  //dither 322 (128 colors in upper half of 256 color table)
    			  }
    			  else
    			  {
    				  printf("ScreenPixelPlotDither8\n");
    				  ScreenPixelPlot=BayerDither8ColorsScreenPixelPlot; //dither 111 (8 colors in upper half of 16 color color table)
    				  LoadRGB4(&WCSScrn->ViewPort, &AltDither8Colors[0], 16);
    				  SetRast(RenderWind0->RPort, 8); // 8color dithered -> 8=white
    			  }
    		  }
			break;
		default:
			// don't touch
			printf("should not plot at all\n");
	}
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
