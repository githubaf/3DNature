// ScriptArg.h

#include "stdafx.h"

// NOTE: Arg templates must all be uppercase for matching to work!

// Argument decoding settings
// Max number of different types of Args on a line
#define WCS_SCRIPT_ARG_MAX_TOTAL_ARGS			60
// Max number of different types of Args on a line
#define WCS_SCRIPT_ARG_MAX_ARGS					60
// Max occurances of the same arg in a line
#define WCS_SCRIPT_ARG_MAX_ONETYPE				20
// Max length of argument template
#define WCS_SCRIPT_ARG_MAX_TEMPLATE_LEN			2048

// Flags to indicate ArgEntry options
#define WCS_SCRIPT_ARG_DEFAULT		(1 << 0) // DFLT
#define WCS_SCRIPT_ARG_MULTIPLE		(1 << 1) // MULT
#define WCS_SCRIPT_ARG_BOOLEAN		(1 << 2) // BOOL


class ArgEntry
	{
	public:
		ArgEntry() {TextName = NULL; ArgFlags = NULL;};
		char *TextName, MatchFailed;
		unsigned long ArgFlags;
		char *Instances[WCS_SCRIPT_ARG_MAX_ONETYPE];
	}; // ArgEntry

class ArgRipper
	{
	private:
		const char *OrigArgLine;
		int LineSize, NumTemplates, NumDecodedArgs;
		char *ProcessArgLine, *SpecCopy;
		char *SplitArgs[WCS_SCRIPT_ARG_MAX_TOTAL_ARGS];
		ArgEntry ArgTable[WCS_SCRIPT_ARG_MAX_ARGS];
		unsigned long DecodeArgFlags(char *FlagsString);
	public:
		ArgRipper(char *InputSpec);
		~ArgRipper();

		int Rip(char *InputArg);
		char *GetArg(int TemplateSubscript, int &ArgNumber);
		char *GetArgText(int ArgNumber);
		char IsBooleanTrue(int TemplateSubscript);
	}; // ArgRipper
