/* Version.h
** Defines for version, revision, and copyright information.
** Built out of GUI.c on 14 Jan 1994 by CXH
*/

#define APP_VERS    "2"
#define APP_REV    "031"
/*#define APP_REV    "00 beta"*/
#define APP_FLAVOR "(Emerald)"
#define APP_TLA    "WCS"
#define APP_TITLE      "World Construction Set"
#define APP_AUTHOR    "Gary R. Huber and Chris \"Xenon\" Hanson"
#define APP_DESCRIPTION  "Unleash your terraforming urges!"

/* Don't mess with these. */
#define APP_VERSION		APP_TLA " " APP_VERS "." APP_REV " " __DATE__
#define APP_COPYR			"©1992-1996"
#define APP_COPYRIGHT	APP_COPYR", "APP_AUTHOR

#ifndef EXTERNAL_VERSION_HACK
extern char ExtVersion[], ExtAboutVers[], ExtAboutBuild[], ExtCreditText[];
#endif /* EXTERNAL_VERSION_HACK */

/* Sapphire 2.00, 2.01, 2.02
   Emerald  2.03		Feb 2, 1996
*/
