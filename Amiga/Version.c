/* Version.c
** Strings and numbers for version, revision, and copyright information.
** Built out of GUI.c on 14 Jan 1994 by CXH
*/

#define EXTERNAL_VERSION_HACK
#include "Version.h"

char ExtVersion[]		= "$VER: " APP_VERSION;
char ExtAboutVers[]	= "\33cVersion " APP_VERS "." APP_REV " " APP_FLAVOR;
/*char ExtAboutVers[]	= "\33cVersion " APP_VERS "." APP_REV;*/
#define BUILDHOST "selco"
/*#define BUILDID   "Thanks 3DNature!"*/  /* provide git hash on command line! git describe --always --dirty*/
char ExtAboutBuild[]	= "\33c("__DATE__ " " __TIME__ " " BUILDHOST ")\n""Serial: " BUILDID ;
char Date[]=__DATE__; // used for Beta timeout
char ExtCreditText[] =
"\n\
Concept, initial implementation, algorithms, renderer, good ideas, principal programming:\n\
Gary R. Huber\n\
\n\
Interface style, initial GUI rework, optimization, weird ideas, additional programming:\n\
Christopher Eric \"Xenon\" Hanson\n\
\n"
"Toolbar icons by Chris Hurtt\
\n"
"\n\
World Construction Set uses MUI - MagicUserInterface.\n\
MUI is copyright ©1993 by Stefan Stuntz.\n"
"\n\
\n\
This is an Ent-free product, and is made from 100% \
recycleable electrons.\n"
"\n\
World Construction Set is copyright ©1993-1995 by Questar Productions,\n\
1058 Weld County Road 23.5\n\
Brighton, Colorado 80601\n\
(303) 659-4028\n\n";

#ifdef __SASC
char toolchain_ver[] = "\0$TLCN: " "SAS/C";
#else
    char toolchain_ver[] = "\0$TLCN: " TOOLCHAIN_VER;   /* This macro contains git hashes for Bebbos's gcc, AF, 11.11.2021*/
#endif

