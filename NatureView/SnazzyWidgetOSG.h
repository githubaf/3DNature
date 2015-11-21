// SnazzyWidgetOSG.h
// Interface between OSG Image class and SnazzyWidgetImageCollection

#include <osg/Image>
#include "SnazzyWidget.h"

// SnazzyWidgetImage class
bool CopyImageToSnazzyWidgetNormal(osg::Image *Input, SnazzyWidgetImage *Destination);

// SnazzyWidgetImageCollection class
bool CopyImageToSnazzyWidgetNormal(osg::Image *Input, SnazzyWidgetImageCollection *Destination);
bool CopyImageToSnazzyWidgetNormalHover(osg::Image *Input, SnazzyWidgetImageCollection *Destination);
bool CopyImageToSnazzyWidgetSelected(osg::Image *Input, SnazzyWidgetImageCollection *Destination);
bool CopyImageToSnazzyWidgetSelectedHover(osg::Image *Input, SnazzyWidgetImageCollection *Destination);
bool CopyImageToSnazzyWidgetIndex(osg::Image *Input, SnazzyWidgetImageCollection *Destination);

// only copies R band to destination
bool CopyImageToSingleBand(osg::Image *Input, unsigned char *DestBand);
// copies RGB bands over
bool CopyImageToTripleBand(osg::Image *Input, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB);
// copies RGBA bands over
bool CopyImageToQuadBand(osg::Image *Input, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB, unsigned char *DestA);
