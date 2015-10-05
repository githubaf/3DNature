// WCSVersion.h
// Defines for version, revision, and copyright information.
// Built out of v1 Version.h on 23 May 1995 by CXH
// Stuff that doesn't commonly change from build to build.

#ifndef WCS_WCSVERSION_H
#define WCS_WCSVERSION_H

#ifdef WCS_BUILD_VNS

// VNS
#define APP_TLA			"VNS"
#define APP_CLASSPREFIX	"WCS"
#define APP_VERS		"3"
#define APP_VERS_NUM    3
#define APP_TITLE      "Visual Nature Studio"
#define APP_SHORTTITLE      "VNS"

#else // !WCS_BUILD_VNS

// WCS
#define APP_TLA			"WCS"
#define APP_CLASSPREFIX	APP_TLA // WCS
#define APP_VERS		"7"
#define APP_VERS_NUM    7
#define APP_TITLE      "World Construction Set"
#define APP_SHORTTITLE	APP_TLA // WCS

#endif // !WCS_BUILD_VNS

// Common
#define APP_AUTHOR    "Gary R. Huber and Chris \"Xenon\" Hanson"
#define APP_COPYRIGHT_BASEDATE	"1992-2009"
#define APP_PUBLISHER "3D Nature, LLC"
#define APP_COPYRIGHT	"©" APP_COPYRIGHT_BASEDATE " " APP_PUBLISHER
#define APP_CONTACTPHONE	"303-659-4028"
#define APP_CONTACTWEB		"www.3DNature.com"
#define APP_CONTACTURL		"http://"APP_CONTACTWEB
#define APP_CONTACTEMAIL	"3DNature@3DNature.com"

#define APP_AUTHPHONE		APP_CONTACTPHONE
#define APP_AUTHEMAIL		"Authorize@3dnature.com"

extern char ExtTLA[], ExtTitle[], ExtVersion[], ExtLabelVersion[], ExtCopyright[], ExtDate[],
 ExtCopyrightFull[], ExtPublisher[], ExtAboutBuild[], ExtAmiVersionTag[];

#endif // WCS_WCSVERSION_H
