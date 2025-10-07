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
(303) 659-4028\n\n"
"Maintenance 2017-2025 by Alexander Fritsch (selco)\n\n";


#ifdef __SASC
char toolchain_ver[] = "\0$TLCN: " "SAS/C";
#else
    char toolchain_ver[] = "\0$TLCN: " TOOLCHAIN_VER;   /* This macro contains git hashes for Bebbos's gcc, AF, 11.11.2021*/
#endif


// --------------------------------------------------------------------
    // AF, 32.Feb32
    // wrappers for time() and cvt_TIME. time() cannot be called directly in WCS.c because it needs time.h
    // and that include breaks GST on SAS/C (AGUI.c(!) no longer compiler clean when WCS.c included time.h)

#include <stdio.h>
#include <string.h>
#include <time.h>
    // for beta timeout calculation. Converts Date-String "20 Jan 23" to epoch
    // see https://stackoverflow.com/questions/1765014/convert-string-from-date-into-a-time-t
    unsigned long cvt_TIME(char const *time) {
    	char s_month[5];
    	int month, day, year;
    	struct tm t = {0};
    	static const char month_names[] = "JanFebMarAprMayJunJulAugSepOctNovDec";

    	sscanf(time, "%s %d %d", s_month, &day, &year);

    	month = (strstr(month_names, s_month)-month_names)/3;

    	t.tm_mon = month;
    	t.tm_mday = day;
    	t.tm_year = year - 1900;
    	t.tm_isdst = -1;

    	return (unsigned long)mktime(&t);
    }

    unsigned long get_time(unsigned long *result)
    {
    	return (unsigned long) time((time_t*)result);
    }
