// WCSVersionMinor.h
// Strings and numbers for version, revision, and copyright information.
// Built out of WCSVersion.cpp on 27 Nov 2006 by CXH

// Stuff that commonly changes from build to build.
// APP_REV should now be purely numeric, put textual suffixes in APP_FLAVOR
// this is because we incorporate APP_REV into the Version resource

#include "BuildMagic.h"

#ifdef WCS_BUILD_VNS
#ifdef WCS_BUILD_V3
#define APP_REV    "10"
#define APP_REV_NUM	0
#define APP_FLAVOR ""
#else // WCS_BUILD_V2
#define APP_REV    "62"
#define APP_REV_NUM	62
#define APP_FLAVOR ""
#endif // WCS_BUILD_V2
#else // !WCS_BUILD_VNS
#ifdef WCS_BUILD_W7
#define APP_REV    "80"
#define APP_FLAVOR "[W7]"
#else // WCS_BUILD_W6
#define APP_REV    "62"
#define APP_FLAVOR "t"
#endif // WCS_BUILD_W6
#endif // !WCS_BUILD_VNS

#define FILEVER			APP_VERS_NUM,APP_REV_NUM,APP_YEAR_NUM,APP_MONTHDAY_NUM
#define PRODUCTVER		APP_VERS_NUM,APP_REV_NUM,APP_YEAR_NUM,APP_MONTHDAY_NUM
