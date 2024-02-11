#ifndef WCS_LOCALE_H
#define WCS_LOCALE_H 1

/*
** WCS_locale.h
**
** (c) 2006 by Guido Mersmann
**
** Object source created by SimpleCat
*/

/*************************************************************************/

#include "WCS_strings.h" /* change name to correct locale header if needed */

/*
** Prototypes
*/

BOOL   Locale_Open( STRPTR catname, ULONG version, ULONG revision);
void   Locale_Close(void);
STRPTR GetString(long ID);

/*************************************************************************/

#endif /* WCS_LOCALE_H */
