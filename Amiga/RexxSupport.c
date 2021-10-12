/* RexxSupport.c
** Built from SimpleRexx.c
** Copyright 1995 by Questar Productions
*/

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>
#include	<exec/memory.h>

#include	<proto/exec.h>
#include 	<proto/rexxsyslib.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

#include	<string.h>
#include	<ctype.h>

/* Local includes */
#define EXTERN
#include "RexxSupport.h"
#include "GrammarTable.h"
#include "VocabTable.h"
#include "Proto.h"
#include "Defines.h"

#include <clib/alib_protos.h>

extern struct MWS_Entry SentLookUp[];

#define VE_TERM_FLAG			(1 << 31)
#define VE_TERM_FLAG_EIGHT		(1 << 7)
#define VE_LAST_ENTRY			(1 << 30)
#define VE_LAST_ENTRY_EIGHT		(1 << 6)

#define RECOG_WILD			(1 << 29) /* Means word requires arg */
#define RECOG_ENDLEVEL			(1 << 28) /* End of this level of table */

extern unsigned long int MarkovTable[];

STATIC_FCN struct MWS_Entry *Cmd_SearchMe(struct MWS_Entry *FromHere, long int WordUp); // used locally only -> static, AF 20.7.2021
STATIC_FCN char *Cmd_PullWord(char *Source, char *WordBuf, int WBufSize); // used locally only -> static, AF 20.7.2021
STATIC_FCN long int Cmd_HuntShort(unsigned long int BeginPoint); // used locally only -> static, AF 20.7.2021
STATIC_FCN long int Cmd_LookupWord(char *Word); // used locally only -> static, AF 20.7.2021
STATIC_FCN char *Cmd_FetchInlineArg(char *Inline, char *ArgDest, int ArgSize); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Rexx_ReplyMsg(struct RexxMsg *This, char *RString, LONG Error); // used locally only -> static, AF 20.7.2021
STATIC_FCN void Cmd_TrimArg(char *Dest, char *Source, int DestLen); // used locally only -> static, AF 20.7.2021

/*
 * This function returns the signal mask that the Rexx port is
 * using.  It returns NULL if there is no signal...
 *
 * Use this signal bit in your Wait() loop...
 */
ULONG Rexx_SigMask(struct ARexxContext *This)
{
register	ULONG	tmp=0;

if (This)
	{
	tmp = 1L << (This->ARexxPort->mp_SigBit);
	} /* if */
return(tmp);
} /* Rexx_SigMask() */



/*
 * This function returns a structure that contains the commands sent from
 * ARexx...  You will need to parse it and return the structure back
 * so that the memory can be freed...
 *
 * This returns NULL if there was no message...
 */
struct RexxMsg *Rexx_GetMsg(struct ARexxContext *This)
{
register	struct RexxMsg *tmp=NULL;
register	short	flag;

if (This)
	{
	if (tmp = (struct RexxMsg *)GetMsg(This->ARexxPort))
		{
		if (tmp->rm_Node.mn_Node.ln_Type==NT_REPLYMSG)
			{
			/* If we had sent a command, it would come this way... */

			flag=FALSE;
			if (tmp->rm_Result1)
				{
				flag=TRUE;
				} /* if */

			/* Free the arguments and the message... */
			DeleteArgstring(tmp->rm_Args[0]);
			DeleteRexxMsg(tmp);
			This->Outstanding -= 1;

			/* Return the error if there was one... */
			tmp=flag ? REXX_RETURN_ERROR : NULL;
			} /* if */
		} /* if */
	} /* if */
return(tmp);
} /* Rexx_GetMsg() */




/*
 * Use this to return a ARexx message...
 *
 * If you wish to return something, it must be in the RString.
 * If you wish to return an Error, it must be in the Error.
 * If there is an error, the RString is ignored.
 */
STATIC_FCN void Rexx_ReplyMsg(struct RexxMsg *This, char *RString, LONG Error) // used locally only -> static, AF 20.7.2021
{
if ((This) && (This != REXX_RETURN_ERROR))
	{
	This->rm_Result2 = 0;
	if (!(This->rm_Result1 = Error))
		{
		/* if you did not have an error we return the string */
		if (This->rm_Action & (1L << RXFB_RESULT))
			{
			if (RString)
				{
				This->rm_Result2=(LONG)CreateArgstring((STRPTR)RString,
				 (LONG)strlen(RString));
				} /* if */
			} /* if */
		} /* if */
	/* Reply the message to ARexx... */
	ReplyMsg((struct Message *)This);
	} /* if */
} /* Rexx_ReplyMsg() */




/*
 * This function will set an error string for the ARexx
 * application in the variable defined as <appname>.LASTERROR
 *
 * Note that this can only happen if there is an ARexx message...
 *
 * This returns TRUE if it worked, FALSE if it did not...
 */
STATIC_FCN short Rexx_SetLastError(struct ARexxContext *This, struct RexxMsg *rmsg,
 char *ErrorString) // used locally only -> static, AF 20.7.2021
{
register	short	OkFlag=FALSE;

if ((This) && (rmsg))
	{
	if (CheckRexxMsg(rmsg))
		{
		if(!SetRexxVar(rmsg, (STRPTR)This->ErrorName, (STRPTR)ErrorString, (long)strlen(ErrorString)))
			{
			OkFlag=TRUE;
			} /* if */
		} /* if */
	} /* if */
return(OkFlag);
} /* Rexx_SetLastError() */



/*
** This function will send a string to ARexx, the default host port will be
** that of your task. If you set StringFile to TRUE, it will set that bit for
** the message. Returns TRUE if it send the message, FALSE if it did not...
**/
#ifdef UNUSED_FUNCTIONS  // AF, not used 17.July 2021
short Rexx_SendMsg(struct ARexxContext *This, char *RString, short StringFile)
{
register	struct MsgPort *RexxPort;
register	struct RexxMsg *rmsg;
register short flag=FALSE;

if ((This) && (RString))
	{
	if (rmsg = CreateRexxMsg(This->ARexxPort, This->Extension, This->PortName))
		{
		rmsg->rm_Action = RXCOMM | (StringFile ? (1L << RXFB_STRING):0);
		if((rmsg->rm_Args[0] = CreateArgstring(RString, (LONG)strlen(RString))))
			{
			/* We need to find the RexxPort from within a Forbid() */
			Forbid();
			if(RexxPort = FindPort(RXSDIR))
				{
				/* Found the port, so put the message to ARexx... */
				PutMsg(RexxPort,(struct Message *)rmsg);
				This->Outstanding+=1;
				flag=TRUE;
				} /* if */
			Permit();
			if(!flag)
				{
				/* No port, so clean up... */
				DeleteArgstring(rmsg->rm_Args[0]);
				DeleteRexxMsg(rmsg);
				} /* if */
			} /* if */
		else
			{
			DeleteRexxMsg(rmsg);
			} /* else */
		} /* if */
	} /* if */
return(flag);
} /* Rexx_SendMsg() */
#endif




/* This function closes down the ARexx context that was opened
** with Rexx_New()... */
void Rexx_Del(struct ARexxContext *This)
{
register	struct RexxMsg *rmsg;

if (This)
	{
	/* Clear port name so it can't be found... */
	This->PortName[0]='\0';

	/* Clean out any outstanding messages we had sent out... */
	while (This->Outstanding)
		{
		WaitPort(This->ARexxPort);
		while((rmsg=Rexx_GetMsg(This)))
			{
			if(rmsg != REXX_RETURN_ERROR)
				{
				/* Any messages that come now are blown away... */
				Rexx_SetLastError(This, rmsg, "WCS is shutting down.");
				Rexx_ReplyMsg(rmsg, NULL, 100);
				} /* if */
			} /* while */
		} /* while */

	/* Clean up the port and delete it... */
	if (This->ARexxPort)
		{
		while ((rmsg=Rexx_GetMsg(This)))
			{
			/* Any messages that still are coming in are "dead". We just set
			** the LASTERROR and reply an error of 100... */
			Rexx_SetLastError(This, rmsg, "WCS is shutting down.");
			Rexx_ReplyMsg(rmsg, NULL, 100);
			} /* while */
		DeletePort(This->ARexxPort);
		} /* if */

	/* Make sure we close the library... */
	if (This->RexxSysBase)
		{
		CloseLibrary(This->RexxSysBase);
		} /* if */

	/* Free the memory of the RexxContext */
	FreeMem(This, sizeof(struct ARexxContext));
	} /* if */
} /* Rexx_Del() */




/*
 * This routine initializes an ARexx port for your process
 * This should only be done once per process.  You must call it
 * with a valid application name and you must use the handle it
 * returns in all other calls...
 *
 * NOTE:  The AppName should not have spaces in it...
 *        Example AppNames:  "MyWord" or "FastCalc" etc...
 *        The name *MUST* be less that 16 characters...
 *        If it is not, it will be trimmed...
 *        The name will also be UPPER-CASED...
 *
 * NOTE:  The Default file name extension, if NULL will be
 *        "rexx"  (the "." is automatic)
 */
struct ARexxContext *Rexx_New(char *AppName)
{
register	struct ARexxContext *This=NULL;
register	short loop;
/* register	short count; */
register	char *tmp;

if((This = AllocMem(sizeof(struct ARexxContext), MEMF_PUBLIC|MEMF_CLEAR)))
	{
	if(This->RexxSysBase = OpenLibrary((STRPTR)"rexxsyslib.library", 0))
		{
		/* Set up the extension... */
		strcpy(This->Extension, "WCS");

		/* Set up a port name... */
		tmp=This->PortName;
		for (loop=0;(loop<16)&&(AppName[loop]);loop++)
			{
			*tmp++=toupper(AppName[loop]);
			} /* for */
		*tmp='\0';

		/* Set up the last error RVI name... */
		/* This is <appname>.LASTERROR */
		strcpy(This->ErrorName, This->PortName);
		strcat(This->ErrorName,".LASTERROR");

		Forbid();
		This->ARexxPort = CreatePort((STRPTR)This->PortName, (ULONG)NULL);
		Permit();
		} /* if */

	
	if ((!(This->RexxSysBase)) || (!(This->ARexxPort)))
		{
		Rexx_Del(This);
		This = NULL;
		} /* if */
	} /* if */
return(This);
} /* Rexx_New() */
#ifdef IGNORE_THIS_CRAP

void main(int Count, char *Vector[])
{
int Status, CrunchOn = 1;
char CommandIn[255];

printf("\nParseTest v1.0 Copyright 1995 by Questar Productions.\n");

if((Count == 2) && (Vector[1][0] == '?'))
	{
	printf("Usage: Type complete command lines, hitting [Enter] at the end\n");
	printf("of each. The program will interpret and execute the command.\n");
	printf("The command to exit is QUIT.\n");
	} /* if */
else
	{
	printf("Begin entering commands, type PROJECT QUIT to exit.\n");
	} /* else */

while(CrunchOn)
	{
	printf("CMD: ");
	fflush(stdout);
	if(fgets(CommandIn, 250, stdin))
		{
		/* Trash leading quote if found, and trailing quote only if leading
		** quote was also found */
		if(CommandIn[0] == '\"')
			{
			CommandIn[0] = ' ';
			if(CommandIn[strlen(CommandIn) - 1] == '\"')
				CommandIn[strlen(CommandIn) - 1] = ' ';
			} /* if */
		Status = ParseDispatch(CommandIn);
		if(Status == VE_QUIT)
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

#endif /* IGNORE_THIS_CRAP */



long int Cmd_ParseDispatch(struct ARexxContext *Rexx, struct RexxMsg *CmdMsg)
{
char *InLine, *CurLinePtr, *NewLinePtr, *ArgPart, WordOut[32];
int TermNGo, Words, ResolvedWord /*, SentLen*/;
struct CmdContext CallFrame;
struct MWS_Entry *Descent, *Found;

CurLinePtr = InLine = (char*)ARG0(CmdMsg);

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
	NewLinePtr = Cmd_PullWord(CurLinePtr, WordOut, 30);
	if((NewLinePtr != NULL) && (NewLinePtr != CurLinePtr))
		{
		if(Words == 10)
			{
			Rexx_SetLastError(Rexx, CmdMsg, "WCS currently does not support commands longer than 10 words.");
			Rexx_ReplyMsg(CmdMsg, "", 20);
			return(0);
			} /* if */
		ResolvedWord = Cmd_LookupWord(WordOut);
		if(ResolvedWord != -1)
			{
			CallFrame.WordToken[Words++] = ResolvedWord;
			CallFrame.LastToken = ResolvedWord;
			if((Found = Cmd_SearchMe(Descent, ResolvedWord)) == NULL)
				{
				Rexx_SetLastError(Rexx, CmdMsg, "WCS did not recognise the commands in the order supplied.");
				Rexx_ReplyMsg(CmdMsg, "", 20);
				return(0);
				} /* if */
			else
				{
				Descent = &SentLookUp[Found->Follow];
				if(Found->Word & RECOG_WILD)
					{
					NewLinePtr = Cmd_FetchInlineArg(NewLinePtr, CallFrame.InlineArg, 40);
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
			Rexx_SetLastError(Rexx, CmdMsg, "WCS did not recognise one of the command words.");
			Rexx_ReplyMsg(CmdMsg, "", 20);
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
			{ /* Process the sentence now */
			if(Found->DispFunc)
				{
				if(ArgPart)
					{
					Cmd_TrimArg(CallFrame.ArgStr, ArgPart, 250);
					} /* if */
				if ((*(Found->DispFunc))(&CallFrame))
					{
					Rexx_SetLastError(Rexx, CmdMsg, CallFrame.ArgStr);
					Rexx_ReplyMsg(CmdMsg, "", 10);
					return((long)CallFrame.LastToken);
					} /* if error */
				else
					{
					Rexx_ReplyMsg(CmdMsg, CallFrame.ArgStr, 0);
					return((long)CallFrame.LastToken);
					} /* else no error */
				} /* if */
			else
				{
				Rexx_SetLastError(Rexx, CmdMsg, "WCS recognised a partial but incomplete command.");
				Rexx_ReplyMsg(CmdMsg, "", 20);
				return(0);
				} /* else */
			} /* if */
		else
			{
			Rexx_ReplyMsg(CmdMsg, "", 20);
			return(0);
			} /* else */
		} /* if */
	CurLinePtr = NewLinePtr;
	} /* while */

/* You can't get here from there, but... */
Rexx_ReplyMsg(CmdMsg, "", 20);
return((long)CallFrame.LastToken);
} /* Cmd_ParseDispatch() */

STATIC_FCN char *Cmd_PullWord(char *Source, char *WordBuf, int WBufSize) // used locally only -> static, AF 20.7.2021
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
				Log(ERR_NULL, (CONST_STRPTR)"Non-alphabetic character found in command word."); /* <<<>>> */
				return(0);
				} /* else */
			} /* default */
		} /* switch */
	} /* for */

Log(ERR_NULL, (CONST_STRPTR)"Command word size exceeded."); /* <<<>>> */
return(0);

} /* Cmd_PullWord */

STATIC_FCN long int Cmd_LookupWord(char *Word) // used locally only -> static, AF 20.7.2021
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
				return(Cmd_HuntShort(CurEntry & 0x00ffffff));
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

} /* Cmd_LookupWord() */

STATIC_FCN long int Cmd_HuntShort(unsigned long int BeginPoint) // used locally only -> static, AF 20.7.2021
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

} /* Cmd_HuntShort() */

STATIC_FCN struct MWS_Entry *Cmd_SearchMe(struct MWS_Entry *FromHere, long int WordUp) // used locally only -> static, AF 20.7.2021
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

} /* Cmd_SearchMe() */

STATIC_FCN void Cmd_TrimArg(char *Dest, char *Source, int DestLen) // used locally only -> static, AF 20.7.2021
{

/* Trims off leading spaces and equals signs, and copies to dest string. */

int Skim, Out, Trimming;

Trimming = 1;
for(Out = Skim = 0;;Skim++)
	{
	if((Out == DestLen) || (Source[Skim] == 0))
		{
		Dest[Out] = 0;
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
} /* Cmd_TrimArg() */

STATIC_FCN char *Cmd_FetchInlineArg(char *Inline, char *ArgDest, int ArgSize) // used locally only -> static, AF 20.7.2021
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

Log(ERR_NULL, (CONST_STRPTR)"Inline Arg size exceeded.");
return(0);

} /* Cmd_FetchInlineArg() */
