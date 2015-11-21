// ScriptArg.cpp

#include "ScriptArg.h"
#include <string.h>
#include <ctype.h>

#ifdef BUILD_LIB // building as library
#include <malloc.h>
#else // !BUILD_LIB
#include "AppMem.h"
#endif // !BUILD_LIB

// ********************************* ArgRipper Code ********************************* 








ArgRipper::ArgRipper(char *InputSpec)
{
int Scan, Chop, Chopped;
char TemplateStr[WCS_SCRIPT_ARG_MAX_TEMPLATE_LEN];

NumDecodedArgs = NumTemplates = LineSize = 0;
OrigArgLine = ProcessArgLine = SpecCopy = NULL;

for(int TableClear = 0; TableClear < WCS_SCRIPT_ARG_MAX_ARGS; TableClear++)
	{
	for(int InstanceClear = 0; InstanceClear < WCS_SCRIPT_ARG_MAX_ONETYPE; InstanceClear++)
		{
		ArgTable[TableClear].Instances[InstanceClear] = NULL;
		} // for
	} // for


if(InputSpec && InputSpec[0])
	{
	// Visual 6 doesn't like performing modifications to static strings
	strcpy(TemplateStr, InputSpec);
	InputSpec = TemplateStr;
	// Remove any leading whitespace, reduce multiple consecutive whitespace to a single space
	for(Chopped = 1, Chop = Scan = 0; InputSpec[Scan]; Scan++)
		{
		if(isspace((unsigned char)InputSpec[Scan]))
			{
			if(Chopped)
				{
				// Do nothing.
				} // if
			else
				{
				InputSpec[Chop++] = InputSpec[Scan];
				Chopped = 1;
				} // else
			} // if
		else
			{
			InputSpec[Chop++] = InputSpec[Scan];
			Chopped = 0;
			} // else
		} // for
	InputSpec[Chop] = NULL;

	// bail out if nothing's left
	if(InputSpec[0] == 0) return;

	// Chop off trailing space if present
	if(isspace((unsigned char)InputSpec[strlen(InputSpec) - 1])) InputSpec[strlen(InputSpec) - 1] = NULL;

	// bail out if nothing's left
	if(InputSpec[0] == 0) return;

	// Copy compacted string to new buffer
	LineSize = strlen(InputSpec) + 1;
#ifdef BUILD_LIB // building as library
	if(SpecCopy = (char *)malloc(LineSize))
#else // !BUILD_LIB
	if(SpecCopy = (char *)AppMem_Alloc(LineSize, 0))
#endif // !BUILD_LIB
		{
		strcpy(SpecCopy, InputSpec);
		// Count remaining spaces to determine number of args in template
		for(Scan = NumTemplates = 0; SpecCopy[Scan]; Scan++)
			{
			if(isspace((unsigned char)SpecCopy[Scan]))
				{
				NumTemplates++;
				} // if
			} // for
		NumTemplates += 1;
		// Build argument/template table by chopping up copied string
		Chop = 0;
		ArgTable[Chop].TextName = SpecCopy;
		if(NumTemplates > 1)
			{
			for(Chop = 1, Scan = 0; SpecCopy[Scan]; Scan++)
				{
				if(isspace((unsigned char)SpecCopy[Scan]))
					{
					SpecCopy[Scan] = NULL;
					ArgTable[Chop].TextName = &SpecCopy[Scan + 1];
					ArgTable[Chop].Instances[0] = NULL;
					Chop++;
					} // if
				} // for
			} // if
		// Decode Argument flags for args we found.
		for(Scan = 0; Scan < NumTemplates; Scan++)
			{
			if(ArgTable[Scan].TextName)
				{
				ArgTable[Scan].ArgFlags = DecodeArgFlags(ArgTable[Scan].TextName);
				} // if
			else
				{
				break;
				} // else
			} // for
		} // if
	} // if

} // ArgRipper::ArgRipper

ArgRipper::~ArgRipper()
{
if(SpecCopy && LineSize)
	{
#ifdef BUILD_LIB // building as library
	free(SpecCopy);
#else // !BUILD_LIB
	AppMem_Free(SpecCopy, LineSize);
#endif // !BUILD_LIB
	LineSize = 0;
	SpecCopy = NULL;
	} // if

if(ProcessArgLine)
	{
#ifdef BUILD_LIB // building as library
	free(ProcessArgLine);
#else // !BUILD_LIB
	AppMem_Free(ProcessArgLine, strlen(ProcessArgLine) + 1);
#endif // !BUILD_LIB
	ProcessArgLine = NULL;
	} // if
} // ArgRipper::~ArgRipper

unsigned long int ArgRipper::DecodeArgFlags(char *FlagsString)
{
int ArgTextLen;
unsigned long int FlagsSet = NULL;

ArgTextLen = strlen(FlagsString);
while(ArgTextLen > 6)
	{
	if(FlagsString[ArgTextLen - 5] == '/')
		{
		if(FlagsString[ArgTextLen - 4] == 'D') FlagsSet |= WCS_SCRIPT_ARG_DEFAULT;
		else if(FlagsString[ArgTextLen - 4] == 'M') FlagsSet |= WCS_SCRIPT_ARG_MULTIPLE;
		else if(FlagsString[ArgTextLen - 4] == 'B') FlagsSet |= WCS_SCRIPT_ARG_BOOLEAN;

		FlagsString[ArgTextLen - 5] = NULL;
		ArgTextLen -= 5;
		} // if
	else
		{
		break;
		} // else
	} // while

return(FlagsSet);
} // ArgRipper::DecodeArgFlags


// Return codes:
// 0 or positive: Number of args successfully parsed
// -1 : can't-match-to-arg
// -2 : multiple args not allowed by template
// -3 : too many args
// -4 : unidentified-keyword

int ArgRipper::Rip(char *InputArg)
{
int Wipe, Process, Test, LetterIdx, LastMatch, ArgsFound = 0;
char *ThisArg, *ArgData, InQuote, EscapedQuote, Letter, AllFailed, NoMatch, CommentedOut;

if(InputArg && InputArg[0])
	{
	// Set up
	for(Wipe = 0; Wipe < WCS_SCRIPT_ARG_MAX_TOTAL_ARGS; Wipe++)
		{
		SplitArgs[Wipe] = NULL;
		} // for

	// Skip over any leading whitespaces
	while(isspace((unsigned char)InputArg[0]))
		{
		if(InputArg[0])
			{
			InputArg = &InputArg[1];
			} // if
		else
			{
			break;
			} // else
		} // while

	// Chop off unquoted trailing spaces
	LetterIdx = strlen(InputArg);
	while(isspace((unsigned char)InputArg[LetterIdx - 1]))
		{
		InputArg[LetterIdx - 1] = NULL;
		LetterIdx -= 1;
		} // if

	// Split Args up at unquoted whitespace, quote escape char is the caret ^
	ThisArg = InputArg;
	for(Process = NumDecodedArgs = InQuote = 0; ; Process++)
		{
		EscapedQuote = 0;
		if((isspace((unsigned char)InputArg[Process])) || (InputArg[Process] == NULL))
			{
			if(!InQuote)
				{
				SplitArgs[NumDecodedArgs++] = ThisArg;
				ThisArg = &InputArg[Process + 1];
				if(InputArg[Process] == NULL)
					{
					break;
					} // if
				else
					{
					InputArg[Process] = NULL;
					} // else
				} // if
			} // if
		else if (InputArg[Process] == '\"')
			{
			if(Process > 0)
				{
				if(InputArg[Process] == '^')
					{
					EscapedQuote = 1;
					} // if
				} // if
			if(!EscapedQuote) InQuote = !InQuote;
			} // else if
		} // for

	// Try to match each arg to the template
	for(Process = 0; Process < NumDecodedArgs; Process++)
		{
		ThisArg = SplitArgs[Process];
		// Clear out the matchfailed indicators
		for(Wipe = 0; Wipe < WCS_SCRIPT_ARG_MAX_ARGS; Wipe++)
			{
			if(ArgTable[Wipe].TextName) ArgTable[Wipe].MatchFailed = 0;
			else break;
			} // for
		ArgData = ThisArg;
		for(LetterIdx = LastMatch = NoMatch = 0; !NoMatch && (Letter = toupper(ThisArg[LetterIdx])); LetterIdx++)
			{
			ArgData = &ThisArg[LetterIdx];
			if(!isalpha((unsigned char)Letter))
				{
				if(LetterIdx == 0)
					{
					NoMatch = 1;
					break;
					} // if
				Letter = 0; // Only match keywords that end here.
				} // if
			// optimization: Only scan up to where Wipe loop halted...
			for(AllFailed = 1, Test = 0; Test < Wipe; Test++)
				{
				if(ArgTable[Test].MatchFailed == 0)
					{
					if(Letter != ArgTable[Test].TextName[LetterIdx])
						{
						ArgTable[Test].MatchFailed = 1;
						} // 
					else
						{
						LastMatch = Test;
						AllFailed = 0;
						} // else
					} // if
				} // for
			if(AllFailed)
				{
				NoMatch = 1;
				} // if
			if(Letter == 0)
				{
				break;
				} // if
			} // for
		// Skip up to the non-alpha char that made us bail out
		ArgData = &ArgData[1];
		if(NoMatch)
			{
			// Try to identify if it looks like an unknown KEY=VAL arg, IE first non-alpha char is an =
			for(CommentedOut = Test = 0; ThisArg[Test]; Test++)
				{
				if(!isalpha((unsigned char)ThisArg[Test]))
					{
					if(ThisArg[Test] == '=')
						{
						// deal with unidentified-keyword error
						return(-4);
						} // if
					else if(ThisArg[Test] == ';') // commented-out keyword, ignore
						{
						CommentedOut = 1;
						break;
						} // else if
					else
						{
						break; // probably a default arg
						} // else
					} // if
				} // for
			if(!CommentedOut)
				{
				// look for a default arg to stick this under

				for(AllFailed = 1, Test = 0; Test < WCS_SCRIPT_ARG_MAX_ARGS; Test++)
					{
					if(ArgTable[Test].TextName)
						{
						if(ArgTable[Test].ArgFlags & WCS_SCRIPT_ARG_DEFAULT)
							{
							LastMatch = Test;
							AllFailed = 0;
							break;
							} // 
						} // if
					} // for
				if(AllFailed)
					{
					NoMatch = 1;
					} // if
				else
					{
					ArgData = ThisArg;
					NoMatch = 0;
					} // else
				} // if
			} // if
		if(NoMatch && !CommentedOut)
			{ // STILL no match
			// Deal with can't-match-to-arg error
			return(-1);
			} // if
		else if (!NoMatch)
			{
			// Find a slot for this matched arg
			for(Test = 0; Test < WCS_SCRIPT_ARG_MAX_ONETYPE - 1; Test++)
				{
				if(ArgTable[LastMatch].Instances[Test] == 0)
					{
					if((Test > 0) && !(ArgTable[LastMatch].ArgFlags & WCS_SCRIPT_ARG_MULTIPLE))
						{
						// Deal with no-multiples situation
						return(-2);
						} // if
					ArgTable[LastMatch].Instances[Test] = ArgData;
					ArgsFound++;
					// Move ending NULL up a notch
					if(Test < WCS_SCRIPT_ARG_MAX_ONETYPE)
						{
						ArgTable[LastMatch].Instances[Test + 1] = NULL;
						break;
						} // if
					} // if
				} // for
			if(Test == WCS_SCRIPT_ARG_MAX_ONETYPE)
				{
				// Deal with no-more-args condition
				return(-3);
				} // if
			} // else
		} // for
	} // if

return(ArgsFound);
} // ArgRipper::Rip

char *ArgRipper::GetArg(int TemplateSubscript, int &ArgNumber)
{
char *RetVal = NULL;
int QuoteReplace, ArgRewrite;

if(ArgNumber < WCS_SCRIPT_ARG_MAX_ONETYPE)
	{
	if(ArgTable[TemplateSubscript].Instances[ArgNumber])
		{
		RetVal = ArgTable[TemplateSubscript].Instances[ArgNumber];
		ArgNumber++;
		} // if
	} // if

if(RetVal)
	{
	// Need to do some string processing to remove
	// equals sign, and/or matched begin/end quotes
	
	// remove a single leading equals if present
	if(RetVal[0] == '=') RetVal = &RetVal[1];

	// Remove unescaped quotes, replace escaped quotes with non-escaped
	for(QuoteReplace = 0; RetVal[QuoteReplace]; QuoteReplace++)
		{
		if(RetVal[QuoteReplace] == '\"')
			{
			if((QuoteReplace > 0) && (RetVal[QuoteReplace - 1] == '^'))
				{
				RetVal[QuoteReplace - 1] = -1;
				} // if
			else
				{
				RetVal[QuoteReplace] = -1;
				} // else
			} // if
		} // for

	for(ArgRewrite = QuoteReplace = 0; RetVal[QuoteReplace]; QuoteReplace++)
		{
		if(RetVal[QuoteReplace] != -1)
			{
			RetVal[ArgRewrite++] = RetVal[QuoteReplace];
			} // if
		} // for
	RetVal[ArgRewrite] = NULL;
	} // if

return(RetVal);
} // ArgRipper::GetArg



char *ArgRipper::GetArgText(int ArgNumber)
{
return(SplitArgs[ArgNumber]);

} // ArgRipper::GetArgText


char ArgRipper::IsBooleanTrue(int TemplateSubscript)
{
int BoolScan, DummyNum = 0;
char *TestString, *YesString, *TrueString;
char RetCode = 0, TestLetter;

if(TestString = GetArg(TemplateSubscript, DummyNum))
	{
	RetCode = 1;
	YesString = "YES "; // space pads string to four characters...
	TrueString = "TRUE";
	for(BoolScan = 0; (BoolScan < 4) && (TestLetter = TestString[BoolScan]); BoolScan++)
		{
		TestLetter = toupper(TestLetter);
		if((TestLetter != YesString[BoolScan]) && (TestLetter != TrueString[BoolScan]))
			{
			RetCode = 0;
			break;
			} // if
		} // if
	} // if

return(RetCode);
} // ArgRipper::IsBooleanTrue

