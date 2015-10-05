/* GrammarTable.h
** Extra glue to hold GrammarTable.c together to whatever you use it with.
** Copyright 1995 by Questar Productions
*/


struct CmdContext
	{
	char *RawText;
	unsigned long int WordToken[10], LastToken;
	char InlineArg[40], ArgStr[255];
	}; /* struct CmdContext */

struct MWS_Entry
	{
	unsigned long int Word, Follow;
	int (*DispFunc)(struct CmdContext *Call);
	};

#include "CmdCallProtos.h"

