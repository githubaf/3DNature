// RawOSGImageAccessSupport.cpp
// Some quick functions for accessing pixel data within osg::Image objects
// Probably will be phased out later

#include "RawOSGImageAccessSupport.h"

// for performance reasons, NO error checking is done here. You were warned.

// Read

unsigned long int OSGImageReadPixelRGBA(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha)
{
Red = data[0];
Green = data[1];
Blue = data[2];
Alpha = data[3];
return(4);
} // OSGImageReadPixelRGBA


unsigned long int OSGImageReadPixelRGB(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue)
{
Red = data[0];
Green = data[1];
Blue = data[2];
return(3);
} // OSGImageReadPixelRGB

unsigned long int OSGImageReadPixelBGRA(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha)
{
Blue = data[0];
Green = data[1];
Red = data[2];
Alpha = data[3];
return(4);
} // OSGImageReadPixelBGRA


unsigned long int OSGImageReadPixelBGR(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue)
{
Blue = data[0];
Green = data[1];
Red = data[2];
return(3);
} // OSGImageReadPixelBGR




// Write

unsigned long int OSGImageWritePixelRGBA(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
{
data[0] = Red;
data[1] = Green;
data[2] =  Blue;
data[3] = Alpha;
return(4);
} // OSGImageWritePixelRGBA


unsigned long int OSGImageWritePixelRGB(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue)
{
data[0] = Red;
data[1] = Green;
data[2] =Blue;
return(3);
} // OSGImageWritePixelRGB

unsigned long int OSGImageWritePixelBGRA(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
{
data[0] = Blue;
data[1] = Green;
data[2] = Red;
data[3] = Alpha;
return(4);
} // OSGImageWritePixelBGRA


unsigned long int OSGImageWritePixelBGR(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue)
{
data[0] = Blue;
data[1] = Green;
data[2] = Red;
return(3);
} // OSGImageWritePixelBGR




unsigned long int OSGImageReadPixel(osg::Image *SourceImage, unsigned long int X, unsigned long int Y, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha)
{
unsigned char *data;
data = SourceImage->data(X, Y, 0);
switch(SourceImage->getPixelFormat())
	{
	case GL_RGB: Alpha = 255; return(OSGImageReadPixelRGB(data, Red, Green, Blue));
	case GL_RGBA: return(OSGImageReadPixelRGBA(data, Red, Green, Blue, Alpha));
	case GL_BGR_EXT: Alpha = 255; return(OSGImageReadPixelBGR(data, Red, Green, Blue));
	case GL_BGRA_EXT: return(OSGImageReadPixelBGRA(data, Red, Green, Blue, Alpha));
	} // switch
return(0);
} // OSGImageReadPixel


unsigned long int OSGImageWritePixel(osg::Image *SourceImage, unsigned long int X, unsigned long int Y, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha)
{
unsigned char *data;
data = SourceImage->data(X, Y, 0);
switch(SourceImage->getPixelFormat())
	{
	case GL_RGB: return(OSGImageWritePixelRGB(data, Red, Green, Blue));
	case GL_RGBA: return(OSGImageWritePixelRGBA(data, Red, Green, Blue, Alpha));
	case GL_BGR_EXT: return(OSGImageWritePixelBGR(data, Red, Green, Blue));
	case GL_BGRA_EXT: return(OSGImageWritePixelBGRA(data, Red, Green, Blue, Alpha));
	} // switch
return(0);
} // OSGImageWritePixel


// no error-checking or out-of-bounds checking. Don't be dumb.
void OSGImageBlitRGBA(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY)
{
unsigned long int X, Y;
unsigned char *SrcData, *DestData;
unsigned char R, G, B, A;

for(Y = 0; Y < Height; Y++)
	{
	SrcData  = SourceImage->data(SrcX, SrcY + Y, 0);
	DestData = DestImage->data(DestX, DestY + Y, 0);
	for(X = 0; X < Width; X++)
		{
		SrcData += OSGImageReadPixelRGBA(SrcData, R, G, B, A);
		DestData += OSGImageWritePixelRGBA(DestData, R, G, B, A);
		} // for
	} // for

} // OSGImageBlitRGBA


// no error-checking or out-of-bounds checking. Don't be dumb.
void OSGImageBlitRGB(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY)
{
unsigned long int X, Y;
unsigned char *SrcData, *DestData;
unsigned char R, G, B;

for(Y = 0; Y < Height; Y++)
	{
	SrcData  = SourceImage->data(SrcX, SrcY + Y, 0);
	DestData = DestImage->data(DestX, DestY + Y, 0);
	for(X = 0; X < Width; X++)
		{
		SrcData += OSGImageReadPixelRGB(SrcData, R, G, B);
		DestData += OSGImageWritePixelRGB(DestData, R, G, B);
		} // for
	} // for

} // OSGImageBlitRGB

// no error-checking or out-of-bounds checking. Don't be dumb.
void OSGImageBlitBGR(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY)
{
unsigned long int X, Y;
unsigned char *SrcData, *DestData;
unsigned char R, G, B;

for(Y = 0; Y < Height; Y++)
	{
	SrcData  = SourceImage->data(SrcX, SrcY + Y, 0);
	DestData = DestImage->data(DestX, DestY + Y, 0);
	for(X = 0; X < Width; X++)
		{
		SrcData += OSGImageReadPixelBGR(SrcData, R, G, B);
		DestData += OSGImageWritePixelBGR(DestData, R, G, B);
		} // for
	} // for

} // OSGImageBlitBGR


// no error-checking or out-of-bounds checking. Don't be dumb.
void OSGImageBlitBGRA(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY)
{
unsigned long int X, Y;
unsigned char *SrcData, *DestData;
unsigned char R, G, B, A;

for(Y = 0; Y < Height; Y++)
	{
	SrcData  = SourceImage->data(SrcX, SrcY + Y, 0);
	DestData = DestImage->data(DestX, DestY + Y, 0);
	for(X = 0; X < Width; X++)
		{
		SrcData += OSGImageReadPixelBGRA(SrcData, R, G, B, A);
		DestData += OSGImageWritePixelBGRA(DestData, R, G, B, A);
		} // for
	} // for

} // OSGImageBlitBGRA


// no error-checking or out-of-bounds checking. Don't be dumb.
void OSGImageBlit(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY)
{
unsigned long int X, Y;
unsigned char R, G, B, A;

// see if input and output formats match, and if so, perform the proper matched copy
if(SourceImage->getPixelFormat() == GL_RGB && DestImage->getPixelFormat() == GL_RGB)
	{
	OSGImageBlitRGB(SourceImage, SrcX, SrcY, Width, Height, DestImage, DestX, DestY);
	} // if
else if(SourceImage->getPixelFormat() == GL_RGBA && DestImage->getPixelFormat() == GL_RGBA)
	{
	OSGImageBlitRGBA(SourceImage, SrcX, SrcY, Width, Height, DestImage, DestX, DestY);
	} // else if
else if(SourceImage->getPixelFormat() == GL_BGR_EXT && DestImage->getPixelFormat() == GL_BGR_EXT)
	{
	OSGImageBlitBGR(SourceImage, SrcX, SrcY, Width, Height, DestImage, DestX, DestY);
	} // else if
else if(SourceImage->getPixelFormat() == GL_BGRA_EXT && DestImage->getPixelFormat() == GL_BGRA_EXT)
	{
	OSGImageBlitBGRA(SourceImage, SrcX, SrcY, Width, Height, DestImage, DestX, DestY);
	} // else if
else // formats don't match (avoid this!)
	{ // do it the slower way with per-pixel format switches
	for(Y = 0; Y < Height; Y++)
		{
		for(X = 0; X < Width; X++)
			{
			OSGImageReadPixel(SourceImage, SrcX + X, SrcY + Y, R, G, B, A);
			OSGImageWritePixel(DestImage, DestX + X, DestY + Y, R, G, B, A);
			} // for
		} // for
	} // else

} // OSGImageBlit

