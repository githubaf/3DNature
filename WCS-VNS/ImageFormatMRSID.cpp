// ImageFormatMRSID.cpp

#include "stdafx.h"
#include "ImageFormatConfig.h"
#include "AppMem.h"
#include "Useful.h"
#include "Application.h"
#include "Project.h"
#include "Requester.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "ImageFormatIFF.h"
#include "Raster.h"

/*===========================================================================*/

#ifdef WCS_BUILD_MRSID_SUPPORT
short Raster::LoadMRSID(char *Name, short SupressWng, ImageCacheControl *ICC)
{
if(ICC) ICC->QueryOnlyFlags = NULL; // clear all flags

#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
unsigned char Success = 0;
return(Success);
#endif // !DEMO
} // Raster::LoadMRSID()
#endif // WCS_BUILD_MRSID_SUPPORT
