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
	unsigned long int Word, Follow, EventCode;
	};

#include "CmdCallProtos.h"

enum
	{
	WCS_SCRIPT_EVENT_PROJECT = 1,
	WCS_SCRIPT_EVENT_SCRIPT,
	WCS_SCRIPT_EVENT_QUIT,
	WCS_SCRIPT_EVENT_RENDER,
	WCS_SCRIPT_EVENT_WINDOW,
	WCS_SCRIPT_EVENT_MAPVIEW,
	WCS_SCRIPT_EVENT_CAMVIEW,
	WCS_SCRIPT_EVENT_RENDSET,
	WCS_SCRIPT_EVENT_DATABASE,
	WCS_SCRIPT_EVENT_GISIMPORT
	}; // Script Event types
