// RawOSGImageAccessSupport.h

#ifndef RAWOSGIMAGEACCESSSUPPORT_H
#define RAWOSGIMAGEACCESSSUPPORT_H

#include <osg/Image>

// for performance reasons, virtually NO error checking is done in these functions. You were warned!


// the actual read methods
unsigned long int OSGImageReadPixelRGBA(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha);
unsigned long int OSGImageReadPixelRGB(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue);
unsigned long int OSGImageReadPixelBGRA(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha);
unsigned long int OSGImageReadPixelBGR(unsigned char *data, unsigned char &Red, unsigned char &Green, unsigned char &Blue);

// the actual write methods
unsigned long int OSGImageWritePixelRGBA(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);
unsigned long int OSGImageWritePixelRGB(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue);
unsigned long int OSGImageWritePixelBGRA(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);
unsigned long int OSGImageWritePixelBGR(unsigned char *data, unsigned char Red, unsigned char Green, unsigned char Blue);

// front-end, chooses and uses one of the above
unsigned long int OSGImageReadPixel(osg::Image *SourceImage, unsigned long int X, unsigned long int Y, unsigned char &Red, unsigned char &Green, unsigned char &Blue, unsigned char &Alpha);
unsigned long int OSGImageWritePixel(osg::Image *SourceImage, unsigned long int X, unsigned long int Y, unsigned char Red, unsigned char Green, unsigned char Blue, unsigned char Alpha);

// These do the real work
void OSGImageBlitRGBA(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY);
void OSGImageBlitRGB(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY);
void OSGImageBlitBGR(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY);
void OSGImageBlitBGRA(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY);

// front-end, tries to choose and use one of the above
void OSGImageBlit(osg::Image *SourceImage, unsigned long int SrcX, unsigned long int SrcY, unsigned long int Width, unsigned long int Height, osg::Image *DestImage, unsigned long int DestX, unsigned long int DestY);




#endif // RAWOSGIMAGEACCESSSUPPORT_H

