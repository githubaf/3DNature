// SnazzyWidgetJPGRC.h
// Interface between JPEG-in-Resource/PNG-in-Resource and SnazzyWidgetImageCollection

#include "SnazzyWidget.h"

// SnazzyWidgetImage class
bool LoadPNGImageToSnazzyWidgetNormal(unsigned short ImageID, SnazzyWidgetImage *Destination); // used for Slider Knob, loads Alpha

// SnazzyWidgetImageCollection class
bool LoadJPGImageToSnazzyWidgetNormal(unsigned short ImageID, SnazzyWidgetImageCollection *Destination); // RGB only, no Alpha
bool LoadJPGImageToSnazzyWidgetNormalHover(unsigned short ImageID, SnazzyWidgetImageCollection *Destination); // RGB only, no Alpha
bool LoadJPGImageToSnazzyWidgetSelected(unsigned short ImageID, SnazzyWidgetImageCollection *Destination); // RGB only, no Alpha
bool LoadJPGImageToSnazzyWidgetSelectedHover(unsigned short ImageID, SnazzyWidgetImageCollection *Destination); // RGB only, no Alpha
bool LoadPNGImageToSnazzyWidgetIndex(unsigned short ImageID, SnazzyWidgetImageCollection *Destination); // Index (R) only

// only loads R band to destination (PNG only, used for index)
bool LoadPNGImageToSingleBand(unsigned short ImageID, unsigned char *DestBand);
// loads RGB bands
bool LoadJPGImageToTripleBand(unsigned short ImageID, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB);
// loads RGBA bands (PNG only, used for slider knobs)
bool LoadPNGImageToQuadBand(unsigned short ImageID, unsigned char *DestR, unsigned char *DestG, unsigned char *DestB, unsigned char *DestA);
