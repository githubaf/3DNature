/* ParseTest.c
** Sample program to demonstrate usage and function of Markov tree
** sentance parsing.
** Copyright 1995 by Questar Productions
** Link with GrammarTable.o and VocabTable.o
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "VocabTable.h"

#define EXT
#include "GrammarTable.h"

extern struct MWS_Entry SentLookUp[];

#define VE_TERM_FLAG				(1 << 31)
#define VE_TERM_FLAG_EIGHT		(1 << 7)
#define VE_LAST_ENTRY			(1 << 30)
#define VE_LAST_ENTRY_EIGHT	(1 << 6)

#define RECOG_WILD				(1 << 29) /* Means word requires arg */
#define RECOG_ENDLEVEL			(1 << 28) /* End of this level of table */

extern unsigned long int MarkovTable[];

void main(int Count, char *Vector[]);
long int ParseDispatch(char *InLine);
char *PullWord(char *Source, char *WordBuf, int WBufSize);
long int LookupWord(char *Word);
long int HuntShort(unsigned long int BeginPoint);
struct MWS_Entry *SearchMe(struct MWS_Entry *FromHere, long int WordUp);
void TrimArg(char *Dest, char *Source, int DestLen);
void DemoFunc(struct CmdContext *Call);
char *FetchInlineArg(char *Inline, char *ArgDest, int ArgSize);
int InsertEvent(unsigned long int EventCode, struct CmdContext *Call);


int QuitStatus;

void main(int Count, char *Vector[])
{
int Status, CrunchOn = 1, TrimEnd;
char CommandIn[255];

printf("\nParseTest v1.0 Copyright 1995 by Questar Productions.\n");

if((Count == 2) && (Vector[1][0] == '?'))
	{
	printf("Usage: Type complete command lines, hitting [Enter] at the end\n");
	printf("of each. The program will interpret and execute the command.\n");
	printf("The command to exit is [PROJECT] QUIT.\n");
	} /* if */
else
	{
	printf("Begin entering commands, type [PROJECT] QUIT to exit.\n");
	} /* else */

while(CrunchOn)
	{
	printf("CMD: ");
	fflush(stdout);
	if(fgets(CommandIn, 250, stdin))
		{
		for(TrimEnd = strlen(CommandIn) - 1; TrimEnd >= 0;)
			{
			if(!isprint(CommandIn[TrimEnd]))
				{
				CommandIn[TrimEnd] = NULL;
				TrimEnd--;
				} // if
			else
				{
				break;
				} // else
			} /* for */
		/* Trash leading quote if found, and trailing quote only if leading
		** quote was also found */
		if(CommandIn[0] == '\"')
			{
			CommandIn[0] = ' ';
			if(CommandIn[strlen(CommandIn) - 1] == '\"')
				CommandIn[strlen(CommandIn) - 1] = NULL;
			} /* if */
		Status = ParseDispatch(CommandIn);
		if(QuitStatus == 1)
			{ /* we're gone... */
			printf("\nExiting.\n");
			return;
			} /* if */
		if(Status == -1)
			{
/*			printf("ERROR: Problem in ParseDispatch.\n"); */
			return;
			} /* if */
		} /* if */
	else
		{
		printf("\nERROR: Input file problem.\n");
/*		printf("\nERROR: %s\n", strerror(errno)); */
		CrunchOn = 0;
		} /* if */
	} /* while */

} /* main() */

long int ParseDispatch(char *InLine)
{
char *CurLinePtr, *NewLinePtr, *ArgPart, WordOut[32];
int TermNGo, Words, ResolvedWord /*, SentLen*/;
struct CmdContext CallFrame;
struct MWS_Entry *Descent, *Found;

CurLinePtr = InLine;

CallFrame.RawText = InLine;
for(ResolvedWord = 0; ResolvedWord < 10; ResolvedWord++)
	{
	CallFrame.WordToken[ResolvedWord] = 0;
	} /* for */
CallFrame.LastToken = -1;

memset(CallFrame.ArgStr, 0, 250);
memset(CallFrame.InlineArg, 0, 40);

/* What was I _thinking_? */
/* strncpy(CallFrame.ArgStr, InLine, 250); */

Found = Descent = &SentLookUp[0];
TermNGo = Words = 0;
ArgPart = NULL;

while(1) /* we'll return from somewhere else... */
	{
	NewLinePtr = PullWord(CurLinePtr, WordOut, 30);
	if((NewLinePtr != NULL) && (NewLinePtr != CurLinePtr))
		{
		if(Words == 10)
			{
			printf("ERROR: Command string longer than 10 words.\n");
			return(0);
			} /* if */
		ResolvedWord = LookupWord(WordOut);
		if(ResolvedWord != -1)
			{
			CallFrame.WordToken[Words++] = ResolvedWord;
			CallFrame.LastToken = ResolvedWord;
			if((Found = SearchMe(Descent, ResolvedWord)) == NULL)
				{
				printf("ERROR: Unrecognised command.\n");
				return(0);
				} /* if */
			else
				{
				Descent = &SentLookUp[Found->Follow];
				if(Found->Word & RECOG_WILD)
					{
					NewLinePtr = FetchInlineArg(NewLinePtr, CallFrame.InlineArg, 40);
					} /* if */
				/* The following lets us process unambiguous commands that have args,
				** even without the equals sign. */
				if(Found->Follow == 0) /* End of valid command, nowhere to go. Process */
					{
					TermNGo = 1;
					} /* if */
				} /* else */
			} /* if */
		else
			{
			printf("ERROR: Unrecognised word in command.\n");
			return(0);
			} /* else */
		} /* if */
	else
		{
		TermNGo = 1;
		} /* else */
	if(TermNGo)
		{
		ArgPart = NewLinePtr; /* Not obvious, but correct. */
/*		printf("DEBUG:\nNEW:%s\nCUR:%s\nARG:%s\n", NewLinePtr, CurLinePtr, ArgPart); */
		if(Words)
			{ /* Process the sentance now */
			//if(Found->DispFunc)
			if(Found->EventCode)
				{
				if(ArgPart)
					{
					TrimArg(CallFrame.ArgStr, ArgPart, 250);
					} /* if */
				//(*(Found->DispFunc))(&CallFrame); /* <<<>>> */
				InsertEvent(Found->EventCode, &CallFrame);
				return((long)CallFrame.LastToken);
				} /* if */
			else
				{
				printf("ERROR: Incomplete command.\n");
				return(0);
				} /* else */
			} /* if */
		else
			{
			return(0);
			} /* else */
		} /* if */
	CurLinePtr = NewLinePtr;
	} /* while */

/* You can't get here from there, but... */
return((long)CallFrame.LastToken);
} /* ParseDispatch() */

char *PullWord(char *Source, char *WordBuf, int WBufSize)
{
int WordSize, Index;

memset(WordBuf, 0, WBufSize);
for(Index = WordSize = 0; WordSize < (WBufSize - 1); Index++)
	{
	switch(Source[Index])
		{
		case 0:		/* NULL */
		case '\n':	/* EOL */
		case '\r':	/* Or whatever fires your rocket */
			{
			if(WordSize == 0)
				{ /* EOL before getting word, bail with 0 */
				return(0);
				} /* if */
			/* no break here, return or fall through to next */
			} /* EOLs */
		case '\t':	/* Tab */
		case ' ':	/* space */
			{
			if(WordSize > 0)
				{ /* have some kind of word, bail with it */
				return(&Source[Index]);
				} /* if */
			else
				{
				/* Just don't do anything, Index will ++ */
				} /* else */
			break;
			} /* whitespace */
		default:
			{
			if(isalpha(Source[Index]))
				{
				WordBuf[WordSize++] = Source[Index];
				} /* if */
			else
				{
				if(WordSize == 0)
					{
					if(Source[Index] == '=')
						{
						return(Source); /* this'll be clever */
						} /* if */
					} /* if */
				printf("ERROR: Non-alphabetic character found in command word.\n");
				return(0);
				} /* else */
			} /* default */
		} /* switch */
	} /* for */

printf("ERROR: Command word size (%d characters) exceeded.\n", WBufSize - 1);
return(0);

} /* PullWord */

long int LookupWord(char *Word)
{
unsigned long int MIndex, CurEntry;
int LetIndex, WordLen;
char CurSym;

/* printf("DEBUG: \"%s\"...", Word); */
WordLen = strlen(Word);
for(MIndex = 0; MIndex < WordLen; MIndex++)
	{
	Word[MIndex] = toupper(Word[MIndex]);
	} /* for */

MIndex = LetIndex = 0;
while(1) /* We won't be leaving by the usual way */
	{
	CurEntry = MarkovTable[MIndex];
	if((CurEntry & 0xff000000) == 0xff000000)
		{ /* Code ChristmasTree: Run out of index before end of word. Fail/Bail. */
/*		printf("\nDEBUG: Ran into index end before completing resolution.\n"); */
		return(-1);
		} /* if */
	
	CurSym = ((CurEntry & 0x1f000000) >> 24);
	if(CurSym == (Word[LetIndex] - 'A'))
		{
/*		printf("[%c] ", Word[LetIndex]); */
		if(LetIndex == (WordLen - 1))
			{ /* out of letters, see if we terminate here */
			if(CurEntry & VE_TERM_FLAG)
				{ /* Perfect match, return it. */
/*				printf("\nDEBUG: Good match %08lx.\n", CurEntry); */
				return((long)(CurEntry & 0x00ffffff));
				} /* if */
			else
				{ /* More likely a shortened match, hunt for it */
				return(HuntShort(CurEntry & 0x00ffffff));
				} /* else */
			} /* if */
		else
			{ /* More letters to go... */
			MIndex = CurEntry & 0x00ffffff; /* Where to go next */
			LetIndex++;
			} /* else */
		} /* if */
	else
		{
		if(CurEntry & VE_LAST_ENTRY)
			{ /* Unable to match word. Bail. */
/*			printf("ERROR: Reached end of table level without finding letter '%c'.\n", Word[LetIndex]); */
			return(-1);
			} /* if */
		else
			{ /* Look at next entry. */
			MIndex++;
			} /* if */
		} /* else */
	} /* whilever */

} /* LookupWord() */

long int HuntShort(unsigned long int BeginPoint)
{

while(1)
	{
	if(MarkovTable[BeginPoint] & VE_LAST_ENTRY)
		{
		if(MarkovTable[BeginPoint] & VE_TERM_FLAG)
			{ /* Unique enough for me. */
/*			printf("\nDEBUG: HuntShort found a good match.\n"); */
			return((long)(MarkovTable[BeginPoint] & 0x00ffffff));
			} /* if */
		else
			{
			BeginPoint = (MarkovTable[BeginPoint] & 0x00ffffff);
			} /* else */
		} /* if */
	else
		{
/*		printf("\nDEBUG: HuntShort found an ambiguous entry.\n"); */
		return(-1); /* couldn't resolve into a unique word */
		} /* else */
	} /* whilever */

} /* HuntShort() */

struct MWS_Entry *SearchMe(struct MWS_Entry *FromHere, long int WordUp)
{

while(1) /* why not? */
	{
	if((FromHere->Word & 0x00ffffff)== WordUp)
		{
		return(FromHere); /* Success */
		} /* if */
	else
		{
		if(FromHere->Word & RECOG_ENDLEVEL)
			{ /* You've failed, my son. */
			return(NULL);
			} /* if */
		else
			{
			FromHere = &FromHere[1]; /* Move on... */
			} /* else */
		} /* else */
	} /* whilever */

} /* SearchMe() */

void TrimArg(char *Dest, char *Source, int DestLen)
{

/* Trims off leading spaces and equals signs, and copies to dest string. */

int Skim, Out, Trimming;

Trimming = 1;
for(Out = Skim = 0;;Skim++)
	{
	if((Out == DestLen) || (Source[Skim] == NULL))
		{
		Dest[Out] = NULL;
		return;
		} /* if */
	if(Trimming)
		{
		if((Source[Skim] != '=') && (Source[Skim] != ' '))
			{
			Trimming = 0; /* continues below... */
			}
		} /* if */
	if(!Trimming) /* there's a reason this isn't just an else clause... */
		{
		if((Source[Skim] != '\n') && (Source[Skim] != '\r'))
			{
			Dest[Out++] = Source[Skim];
			} /* if */
		} /* else */
	} /* for */
} /* TrimArg() */

char *FetchInlineArg(char *Inline, char *ArgDest, int ArgSize)
{ /* just slightly copied and hack from PullWord() */
int WordSize, Index, Quoted;

/* printf("DEBUG: Input to InlineArg={%s}.\n", Inline); */

memset(ArgDest, 0, ArgSize);
for(Quoted = Index = WordSize = 0; WordSize < (ArgSize - 1); Index++)
	{
	switch(Inline[Index])
		{
		case '\"': /* quote mode */
		case '\'':
			{
			if(!Quoted)
				{
				Quoted = 1;
				break;
				} /* if */
			/* else fall through to next */
			Quoted = 2; /* Opened, and closed */
			} /* quotes */
		case 0:		/* NULL */
		case '\n':	/* EOL */
		case '\r':	/* Or whatever fires your rocket */
			{
			if(WordSize == 0)
				{ /* EOL before getting word, bail with 0 */
				return(Inline); /* never return an invalid pointer */
				} /* if */
			/* no break here, return or fall through to next */
			} /* EOLs */
		case '\t':	/* Tab */
		case ' ':	/* space */
			{
			if(Quoted)
				{
				if((Inline[Index] == ' ') || (Inline[Index] == '\t'))
					{ /* other chars will fall past the break even if in quoted */
					ArgDest[WordSize++] = Inline[Index];
					break; /* skip the rest of this case */
					} /* if */
				} /* if */
			if(WordSize > 0)
				{ /* have some kind of word, bail with it */
				if(Quoted == 2)
					{
					return(&Inline[Index + 1]);
					} /* if */
				else
					{
					return(&Inline[Index]);
					} /* else */
				} /* if */
			else
				{
				/* Just don't do anything, Index will ++ */
				} /* else */
			break;
			} /* whitespace */
		default:
			{
			ArgDest[WordSize++] = Inline[Index];
			} /* default */
		} /* switch */
	} /* for */

printf("ERROR: Inline Arg size (%d characters) exceeded.\n", ArgSize - 1);
return(0);

} /* FetchInlineArg() */


int InsertEvent(unsigned long int EventCode, struct CmdContext *Call)
{
switch(EventCode)
	{
	case WCS_SCRIPT_EVENT_PROJECT:
	case WCS_SCRIPT_EVENT_SCRIPT:
		{
		return(ProjectOps(Call));
		} //
	case WCS_SCRIPT_EVENT_QUIT:
		{
		return(Quit(Call));
		} //
	case WCS_SCRIPT_EVENT_RENDER:
		{
		return(RenderOps(Call));
		} //
	case WCS_SCRIPT_EVENT_WINDOW:
		{
		return(WinOps(Call));
		} //
	case WCS_SCRIPT_EVENT_MAPVIEW:
		{
		return(MapOps(Call));
		} //
	case WCS_SCRIPT_EVENT_CAMVIEW:
		{
		return(ViewOps(Call));
		} //
	case WCS_SCRIPT_EVENT_RENDSET:
		{
		return(RenderSet(Call));
		} //
	case WCS_SCRIPT_EVENT_DATABASE:
		{
		return(DataBase(Call));
		} //
	case WCS_SCRIPT_EVENT_GISIMPORT:
		{
		return(GISImport(Call));
		} //
	} // EventCode

return(0);
} /* InsertEvent() */




/* Beginning of implemented functions */

int DataBase(struct CmdContext *Call)
{
printf("DataBase: ");
DemoFunc(Call);
return(0);
} /* DataBase() */

int ParamIO(struct CmdContext *Call)
{

printf("ParamIO: ");
DemoFunc(Call);
return(0);
} /* ParamIO */

int KeyOps(struct CmdContext *Call)
{

printf("KeyOps: ");
DemoFunc(Call);
return(0);
} /* KeyOps() */

int MotionKey(struct CmdContext *Call)
{

printf("MotionKey: ");
DemoFunc(Call);
return(0);
} /* MotionKey() */

int RenderSet(struct CmdContext *Call)
{

printf("RenderSet: ");
DemoFunc(Call);
return(0);
} /* RenderSet() */

int ColorKey(struct CmdContext *Call)
{

printf("ColorKey: ");
DemoFunc(Call);
return(0);
} /* ColorKey() */

int EcoKey(struct CmdContext *Call)
{

printf("EcoKey: ");
DemoFunc(Call);
return(0);
} /* EcoKey() */

int ViewOps(struct CmdContext *Call)
{

printf("ViewOps: ");
DemoFunc(Call);
return(0);
} /* ViewOps() */

int ProjectOps(struct CmdContext *Call)
{

printf("ProjectOps: ");
DemoFunc(Call);
return(0);
} /* ProjectOps() */

int MapOps(struct CmdContext *Call)
{

printf("MapOps: ");
DemoFunc(Call);
return(0);
} /* MapOps() */

int Quit(struct CmdContext *Call)
{

printf("Quit: ");
DemoFunc(Call);
QuitStatus = 1;
return(0);
} /* Quit() */

int RenderOps(struct CmdContext *Call)
{

printf("RenderOps: ");
DemoFunc(Call);
return(0);
} /* RenderOps() */

int Status(struct CmdContext *Call)
{

printf("Status: ");
DemoFunc(Call);
return(0);
} /* Status() */

int WinOps(struct CmdContext *Call)
{

printf("WinOps: ");
DemoFunc(Call);
return(0);
} /* WinOps() */

int GISImport(struct CmdContext *Call)
{

printf("GISImport: ");
DemoFunc(Call);
return(0);
} /* GISImport() */



void DemoFunc(struct CmdContext *Call)
{

printf("LastToken=%d", Call->LastToken);
if(strlen(Call->InlineArg))
	{
	printf(" InlineArg=%s", Call->InlineArg);
	} /* if */
if(strlen(Call->ArgStr))
	{
	printf(" Arg=%s", Call->ArgStr);
	} /* if */
printf(".\n");
return;
} /* DemoFunc() */

