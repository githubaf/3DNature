// WCSVersion.cpp
// Strings and numbers for version, revision, and copyright information.
// Built out of v1 Version.c on 23 May 1995 by CXH

#include "stdafx.h"
#include "WCSVersion.h"
#include "WCSVersionMinor.h"

// APP_VERS is in WCSVersion.h
// APP_REV/APP_FLAVOR is now in WCSVersionMinor.h

#ifdef WCS_BUILD_DEMO
#undef APP_FLAVOR
#define APP_FLAVOR "Demo"
#endif // WCS_BUILD_DEMO

#define APP_VERSION		APP_VERS "." APP_REV " " APP_FLAVOR
//#define APP_VERSION		"Build " APP_REV " " APP_FLAVOR
#define APP_BUILDDATE	__DATE__ " " __TIME__

char ExtTLA[] = APP_TLA;
#ifdef WCS_BUILD_DEMO
char ExtTitle[] = APP_TITLE " Demo";
#else
char ExtTitle[] = APP_TITLE;
#endif // WCS_BUILD_DEMO
char ExtVersion[]		= APP_VERSION;
//char ExtLabelVersion[]		= "Version " APP_VERSION;
char ExtLabelVersion[]		= APP_VERSION;
char ExtCopyright[] = APP_COPYRIGHT;
char ExtCopyrightFull[] = "Copyright " APP_COPYRIGHT_BASEDATE;
char ExtPublisher[] = "by " APP_PUBLISHER;
char ExtAboutBuild[]	= APP_BUILDDATE;
char ExtDate[] = __DATE__;
char ExtAmiVersionTag[]		= "$VER: " APP_TLA " " APP_VERSION "(" APP_BUILDDATE ")";
