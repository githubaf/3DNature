// SnazzyWidgetOSG.cpp
// Interface between OSG Image class and SnazzyWidgetImageCollection

#include "SnazzyWidgetOSG.h"

bool CopyImageToSnazzyWidgetNormal(osg::Image *Input, SnazzyWidgetImage *Destination)
{
return(CopyImageToQuadBand(Input, Destination->GetR_Normal(), Destination->GetG_Normal(), Destination->GetB_Normal(), Destination->GetA_Normal()));
} // CopyImageToSnazzyWidgetNormal


bool CopyImageToSnazzyWidgetNormal(osg::Image *Input, SnazzyWidgetImageCollection *Destination)
{
return(CopyImageToTripleBand(Input, Destination->GetR_Normal(), Destination->GetG_Normal(), Destination->GetB_Normal()));
} // CopyImageToSnazzyWidgetNormal

bool CopyImageToSnazzyWidgetNormalHover(osg::Image *Input, SnazzyWidgetImageCollection *Destination)
{
return(CopyImageToTripleBand(Input, Destination->GetR_NormalHover(), Destination->GetG_NormalHover(), Destination->GetB_NormalHover()));
} // CopyImageToSnazzyWidgetNormalHover

bool CopyImageToSnazzyWidgetSelected(osg::Image *Input, SnazzyWidgetImageCollection *Destination)
{
return(CopyImageToTripleBand(Input, Destination->GetR_Selected(), Destination->GetG_Selected(), Destination->GetB_Selected()));
} // CopyImageToSnazzyWidgetSelected

bool CopyImageToSnazzyWidgetSelectedHover(osg::Image *Input, SnazzyWidgetImageCollection *Destination)
{
return(CopyImageToTripleBand(Input, Destination->GetR_SelectedHover(), Destination->GetG_SelectedHover(), Destination->GetB_SelectedHover()));
} // CopyImageToSnazzyWidgetSelectedHover

bool CopyImageToSnazzyWidgetIndex(osg::Image *Input, SnazzyWidgetImageCollection *Destination)
{
return(CopyImageToSingleBand(Input, Destination->Get_Index()));
} // CopyImageToSnazzyWidgetIndex

// only copies R band to destination
bool CopyImageToSingleBand(osg::Image *Input, unsigned char *DestBand)
{
if(Input && DestBand)
	{
	GLint PixelFormat;
	PixelFormat = Input->getPixelFormat();
	if(PixelFormat == GL_RGB || PixelFormat == GL_RGBA)
		{
		unsigned char *PixelData;
		for(int YLoop = 0; YLoop < Input->t(); YLoop++)
			{
			// OGL images are stored with 0,0 in LL corner, use t() - YLoop to flip over
			PixelData = Input->data(0, Input->t() - (YLoop + 1), 0);
			for(int XLoop = 0; XLoop < Input->s(); XLoop++)
				{
				*DestBand = *PixelData++;
				PixelData++; // skip G
				PixelData++; // skip B
				if(PixelFormat == GL_RGBA)
					{
					PixelData++; // skip alpha
					} // if
				DestBand++;
				} // for
			} // for
		return(true);
		} // if
	} // if
return(false);
} // CopyImageToSingleBand

// copies RGB bands over
bool CopyImageToTripleBand(osg::Image *Input, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB)
{
if(Input && DestR && DestG && DestB)
	{
	GLint PixelFormat;
	PixelFormat = Input->getPixelFormat();
	if(PixelFormat == GL_RGB || PixelFormat == GL_RGBA)
		{
		unsigned char *PixelData;
		for(int YLoop = 0; YLoop < Input->t(); YLoop++)
			{
			// OGL images are stored with 0,0 in LL corner, use t() - YLoop to flip over
			PixelData = Input->data(0, Input->t() - (YLoop + 1), 0);
			for(int XLoop = 0; XLoop < Input->s(); XLoop++)
				{
				*DestR = *PixelData++;
				*DestG = *PixelData++;
				*DestB = *PixelData++;
				if(PixelFormat == GL_RGBA)
					{
					PixelData++; // skip alpha
					} // if
				DestR++; DestG++; DestB++;
				} // for
			} // for
		return(true);
		} // if
	} // if
return(false);
} // CopyImageToTripleBand


// copies RGBA bands over
bool CopyImageToQuadBand(osg::Image *Input, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB, unsigned char *DestA)
{
if(Input && DestR && DestG && DestB && DestA)
	{
	GLint PixelFormat;
	PixelFormat = Input->getPixelFormat();
	if(PixelFormat == GL_RGB || PixelFormat == GL_RGBA)
		{
		unsigned char *PixelData;
		for(int YLoop = 0; YLoop < Input->t(); YLoop++)
			{
			// OGL images are stored with 0,0 in LL corner, use t() - YLoop to flip over
			PixelData = Input->data(0, Input->t() - (YLoop + 1), 0);
			for(int XLoop = 0; XLoop < Input->s(); XLoop++)
				{
				*DestR = *PixelData++;
				*DestG = *PixelData++;
				*DestB = *PixelData++;
				if(PixelFormat == GL_RGBA)
					{
					*DestA = *PixelData++; // copy alpha
					} // if
				else
					{
					*DestA = 255; // full opacity
					} // else
				DestR++; DestG++; DestB++; DestA++;
				} // for
			} // for
		return(true);
		} // if
	} // if
return(false);
} // CopyImageToQuadBand
