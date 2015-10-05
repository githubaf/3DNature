// Script.cpp
// Script parsing and dispatching header
// Created from markov/ParseTest.c on 28 May 1997 by Chris 'Xenon' Hanson
// Copyright 1997

#include "stdafx.h"
#include "Script.h"
#include "AppMem.h"
#include "VocabTable.h"
#include "Requester.h"
#include "Log.h"
#include "Project.h"
#include "WCSVersion.h"

#define ENABLE_NETWORK_SCRIPT
// add link to  WSOCK32.LIB 

//extern unsigned long MarkovTable[];
extern struct MWS_Entry SentLookUp[];

static char ScriptErrorFmtBuf[WCS_SCRIPT_MAX_ERROR_LEN], CommandIn[WCS_SCRIPT_MAX_INPUT_LEN];
//static char NetCommandIn[WCS_SCRIPT_MAX_INPUT_LEN];
static char NetCommandBuf[WCS_SCRIPT_MAX_INPUT_LEN];

/*===========================================================================*/

ScriptEntity::ScriptEntity(ScriptParser *Host)
{

NetCommandBufSize = 0;
OneShotBuf = NULL;

Next = NULL; ScriptFile = NULL; ScriptStatus = NULL;
ScriptSocket = NULL;
ID = ScriptFlags = CommandsProcessed = LinesProcessed = NULL;
ScriptHost = Host;
ScriptErrors = Host->ScriptErrors;
ScriptDebugLevel = Host->GlobalDebugLevel;
LastResult = 0;
};

/*===========================================================================*/

ScriptEntity::~ScriptEntity()
{

if (ScriptStatus) delete ScriptStatus;
ScriptStatus = NULL;

// <<<>>> May not want to fclose this.
if (ScriptFile) fclose(ScriptFile);
ScriptFile = NULL;

} // ScriptEntity::~ScriptEntity

/*===========================================================================*/

static char OutMessage[200], RemHost[200];

/*===========================================================================*/

AppEvent *ScriptEntity::DoNextCommand(int NetMode)
{
AppEvent *Success = NULL;
int TrimEnd;
int GoodGo = 0, OneShot = 0;
long Status, CurFilePos;
#ifdef ENABLE_NETWORK_SCRIPT
long NetReadLen;
char *NetRetPos;
BOOL SockStat;
int NewScriptSocket, OptLen;
SOCKADDR_IN RemoteLink;
int RLSize;
//hostent *HE = NULL;
#endif // ENABLE_NETWORK_SCRIPT

// ReInitialize event-processing structures.
ScriptHost->ScriptEvent.EventClear();
memset(&ScriptHost->CurrentCommand, 0, sizeof(struct CmdContext));

// Do it...
errno = 0;
if (NetMode)
	{
	#ifdef ENABLE_NETWORK_SCRIPT
	OptLen = sizeof(SockStat);
	getsockopt(ScriptSocket, SOL_SOCKET, SO_ACCEPTCONN, (char FAR *)&SockStat, &OptLen);
	if (SockStat)
		{
		RLSize = sizeof(RemoteLink);
		if ((NewScriptSocket = accept(ScriptSocket, (struct sockaddr *)&RemoteLink, &RLSize)) != -1)
			{
			//getpeername(NewScriptSocket, (struct sockaddr *)&RemoteLink, &RLSize);
			WSAAsyncSelect(NewScriptSocket, (HWND)GlobalApp->WinSys->GetRoot(), 0xC000, FD_READ | FD_CLOSE | FD_CONNECT | FD_ACCEPT);
			//HE = gethostbyaddr();
			sprintf(RemHost, "%s", inet_ntoa(RemoteLink.sin_addr));
			if (ScriptHost->RemoteIP != INADDR_NONE)
			//if (1) // we need to permit connections from localhost
				{
				if (ScriptHost->RemoteIP == RemoteLink.sin_addr.S_un.S_addr)
					{
					ScriptSocket = NewScriptSocket;
					sprintf(OutMessage, "\n\rConnected to %s V%s Remote Network Scripting\n\rIdentified Remote host %s.\n\r", ExtTitle, ExtVersion, RemHost);
					send(NewScriptSocket, OutMessage, (int)strlen(OutMessage), 0);
					} // if
				else
					{
					sprintf(OutMessage, "\n\rConnection from %s refused.\n\r", RemHost);
					send(NewScriptSocket, OutMessage, (int)strlen(OutMessage), 0);
					closesocket(NewScriptSocket);
					} // else
				} // if
			else
				{
				} // else
			} // if
		else
			{
			return(NULL);
			} // else
		} // if
	//if (fgets(CommandIn, WCS_SCRIPT_MAX_INPUT_LEN, ScriptFile))
	if ((NetReadLen = recv(ScriptSocket, &NetCommandBuf[NetCommandBufSize], WCS_SCRIPT_MAX_INPUT_LEN - NetCommandBufSize, 0)) != SOCKET_ERROR)
		{
		if (NetReadLen == 0)
			{ // socket closed, re-init
			closesocket(ScriptSocket);
			// get master socket again
			ScriptSocket = ScriptHost->ScriptSocket;
			//listen(ScriptSocket, 1);
			} // if
		else
			{
			NetCommandBufSize += NetReadLen;
			if ((NetCommandBufSize > 0) && ( (NetRetPos = strchr(NetCommandBuf, '\n')) || (NetRetPos = strchr(NetCommandBuf, '\r')) ) )
				{
				NetRetPos[0] = NULL;
				strcpy(CommandIn, NetCommandBuf);
				memcpy(NetCommandBuf, NetRetPos, WCS_SCRIPT_MAX_INPUT_LEN - strlen(CommandIn));
				// echo
				sprintf(OutMessage, "%s\r\n", CommandIn);
				send(ScriptSocket, OutMessage, (int)strlen(OutMessage), 0);
				NetCommandBufSize -= (int)(strlen(CommandIn) + 1);
				GoodGo = 1;
				NetCommandBufSize = 0; // clear for next input
				} // if
			} // else
		} // if
	else
		{
		int SockErr;
		// get the error
		SockErr = WSAGetLastError();
		if (SockErr != WSAEWOULDBLOCK)
			{
			// other error
			} // if
		if (SockErr == WSAESHUTDOWN)
			{
			// re-init socket?
			} // if
		} // else
	#endif // ENABLE_NETWORK_SCRIPT
	} // if
else
	{
	if (OneShotBuf)
		{
		// One-Shot script command
		OneShot = 1;
		// Ensure scipt dies when we're done.
		ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
		strncpy(CommandIn, OneShotBuf, WCS_SCRIPT_MAX_INPUT_LEN);
		CommandIn[WCS_SCRIPT_MAX_INPUT_LEN - 1] = NULL;
		OneShotBuf = NULL;
		GoodGo = 1;
		} // if
	else if (fgets(CommandIn, WCS_SCRIPT_MAX_INPUT_LEN, ScriptFile))
		{
		GoodGo = 1;
		} // if
	} // else


if (GoodGo)
   {
   LinesProcessed++;
   for (TrimEnd = (int)(strlen(CommandIn) - 1); TrimEnd >= 0;)
      {
      if (!isprint((unsigned char)CommandIn[TrimEnd])) // gnaw off any EOL junk
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
   if (CommandIn[0] == '\"')
      {
      CommandIn[0] = ' ';
      if (CommandIn[strlen(CommandIn) - 1] == '\"')
         CommandIn[strlen(CommandIn) - 1] = NULL;
      } /* if */
   if (ScriptDebugLevel > 0)
      {
	  if (ScriptErrors)
		{
		sprintf(ScriptErrorFmtBuf, "[%d]DBG: %s", GetLastResult(), CommandIn);
		ScriptErrors->PostError(WCS_LOG_SEVERITY_MSG, ScriptErrorFmtBuf);
		ReportResult("DEBUGIN", CommandIn);
		sprintf(ScriptErrorFmtBuf, "%d", GetLastResult());
		ReportResult("DEBUGRESULT", ScriptErrorFmtBuf);
		} // if
      } // if
   SetLastResult(0);
   Status = 0;
   if (CommandIn[0])
      {
	  Status = ParseDispatch(CommandIn);
      } // if
   if (Status == -1)
      {
      if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Script Interpreter problem in ParseDispatch.");
	  ReportResult("ERROR", "Script Interpreter problem in ParseDispatch.");
      LogErrorLine(WCS_LOG_SEVERITY_ERR, CommandIn);
      ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
      } // if
   else
      {
	  Success = &ScriptHost->ScriptEvent;
	  } // else
   if ((!NetMode) && (ScriptFile) && ((CurFilePos = ftell(ScriptFile)) != -1))
	{
	if (ScriptStatus)
		{
		ScriptStatus->Update(CurFilePos);
		if (ScriptStatus->CheckAbort())
			{
			sprintf(ScriptErrorFmtBuf, "Script Aborted by user.");
   			if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_MSG, ScriptErrorFmtBuf);
			ReportResult("ERROR", "Script Aborted by user.");
			ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
			} // if
		} // if
	} // if
   return(Success);
   } /* if */
else // Error or EOF
   {
   if (errno)
   	{
	if (ScriptErrors)
		{
		sprintf(ScriptErrorFmtBuf, "Script Interpreter Input File Error, %s", strerror(errno));
   		ScriptErrors->PostStockError(WCS_LOG_WNG_READ_FAIL, ScriptErrorFmtBuf);
		sprintf(ScriptErrorFmtBuf, "at line %d", LinesProcessed);
   		ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, ScriptErrorFmtBuf);
		} // if
   	} // if
   else
   	{
	if (NetMode)
		{
		Success = 0;
		} // if
	else
		{
		Success = &ScriptHost->ScriptEvent;
		} // else
   	} // else
   ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
   } // else

return(Success);

} // ScriptEntity::DoNextCommand

/*===========================================================================*/

void ScriptEntity::LogErrorLine(unsigned char Magnitude, char *RawLine)
{

if (ScriptErrors)
	{
	ScriptErrors->PostError(Magnitude, RawLine);
	} // if

} // ScriptEntity::LogErrorLine

/*===========================================================================*/

long ScriptEntity::ParseDispatch(char *InLine)
{
char *CurLinePtr, *NewLinePtr, *ArgPart, WordOut[WCS_SCRIPT_MAX_SINGLE_LEN];
int TermNGo, Words, ResolvedWord /*, SentLen*/;
struct CmdContext *CallFrame;
struct MWS_Entry *Descent, *Found;

CurLinePtr = InLine;
CallFrame = &ScriptHost->CurrentCommand;

CallFrame->RawText = InLine;
for (ResolvedWord = 0; ResolvedWord < WCS_SCRIPT_MAX_WORD_LEN; ResolvedWord++)
	{
	CallFrame->WordToken[ResolvedWord] = 0;
	} /* for */
CallFrame->LastToken = 0xffffffff;

memset(CallFrame->ArgStr, 0, WCS_SCRIPT_MAX_ARG_LEN);
memset(CallFrame->InlineArg, 0, WCS_SCRIPT_MAX_INLINE_LEN);

Found = Descent = &SentLookUp[0];
TermNGo = Words = 0;
ArgPart = NULL;

// we'll return from somewhere else...
while (1)	//lint !e716
	{
	NewLinePtr = PullWord(CurLinePtr, WordOut, WCS_SCRIPT_MAX_SINGLE_LEN - 2);
	if ((NewLinePtr != NULL) && (NewLinePtr != CurLinePtr))
		{
		if (Words == WCS_SCRIPT_MAX_WORD_LEN)
			{
			sprintf(ScriptErrorFmtBuf, "Script Interpreter Command string longer than %d words.", WCS_SCRIPT_MAX_WORD_LEN);
			if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, ScriptErrorFmtBuf);
			ReportResult("ERROR", ScriptErrorFmtBuf);
			LogErrorLine(WCS_LOG_SEVERITY_ERR, InLine);
			ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
			return(0);
			} /* if */
		ResolvedWord = LookupWord(WordOut);
		if (ResolvedWord != -1)
			{
			CallFrame->WordToken[Words++] = ResolvedWord;
			CallFrame->LastToken = ResolvedWord;
			if ((Found = SearchMe(Descent, ResolvedWord)) == NULL)
				{
				if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Unrecognised command.");
				ReportResult("ERROR", "Unrecognised command.");
				LogErrorLine(WCS_LOG_SEVERITY_ERR, InLine);
				ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
				return(0);
				} // if
			else
				{
				Descent = &SentLookUp[Found->Follow];
				if (Found->Word & RECOG_WILD)
					{
					NewLinePtr = FetchInlineArg(NewLinePtr, CallFrame->InlineArg, 40);
					} // if
				// The following lets us process unambiguous commands that have args,
				// even without the equals sign.
				if (Found->Follow == 0) // End of valid command, nowhere to go. Process
					{
					TermNGo = 1;
					} // if
				} // else
			} // if
		else
			{
			if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Unrecognised word in command.");
			ReportResult("ERROR", "Unrecognised word in command.");
			LogErrorLine(WCS_LOG_SEVERITY_ERR, InLine);
			ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
			return(0);
			} // else
		} // if
	else
		{
		TermNGo = 1;
		} // else
	if (TermNGo)
		{
		ArgPart = NewLinePtr; // Not obvious, but correct.
		//printf("DEBUG:\nNEW:%s\nCUR:%s\nARG:%s\n", NewLinePtr, CurLinePtr, ArgPart);
		if (Words)
			{ // Process the sentance now
			//if (Found->DispFunc)
			if (Found->EventCode)
				{
				if (ArgPart)
					{
					TrimArg(CallFrame->ArgStr, ArgPart, 250);
					} // if
				//InsertEvent(Found->EventCode, CallFrame);
				CallFrame->EventCode = Found->EventCode;
				CallFrame->NumTokens = Words;
				ScriptHost->ScriptEvent.Type = WCS_APP_EVENTTYPE_SCRIPT;
				ScriptHost->ScriptEvent.GenericData = CallFrame;
				CallFrame->SourceScript = this;
				// Tell the world.
				ScriptHost->BroadCastEvent(this);
				return((long)CallFrame->LastToken);
				} // if
			else
				{
				if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Incomplete command.");
				ReportResult("ERROR", "Incomplete command.");
				LogErrorLine(WCS_LOG_SEVERITY_ERR, InLine);
				ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
				return(0);
				} // else
			} // if
		else
			{
			return(0);
			} // else
		} // if
	CurLinePtr = NewLinePtr;
	} // while

// You can't get here from there, but...
//lint -e{527} because Lint sees you can't get here too
return((long)CallFrame->LastToken);

} // ScriptEntity::ParseDispatch

/*===========================================================================*/

char *ScriptEntity::PullWord(char *Source, char *WordBuf, int WBufSize)
{
int WordSize, Index;

memset(WordBuf, 0, WBufSize);
for (Index = WordSize = 0; WordSize < (WBufSize - 1); Index++)
	{
	switch(Source[Index])
		{
		case 0:		// NULL
		case '\n':	// EOL
		case '\r':	// Or whatever fires your rocket
			{
			if (WordSize == 0)
				{ // EOL before getting word, bail with 0
				return(0);
				} // if
			// no break here, return or fall through to next
			} // EOLs
			//lint -fallthrough
		case '\t':	// Tab
		case ' ':	// space
			{
			if (WordSize > 0)
				{ // have some kind of word, bail with it
				return(&Source[Index]);
				} // if
			else
				{
				// Just don't do anything, Index will ++
				} // else
			break;
			} // whitespace
		default:
			{
			if (isalpha((unsigned char)Source[Index]))
				{
				WordBuf[WordSize++] = Source[Index];
				} // if
			else
				{
				//if (WordSize == 0)
					{
					if (Source[Index] == '=')
						{
						return(Source); // this'll be clever
						} // if
					} // if
				if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Script Interpreter Non-alphabetic character found in command word.");
				ReportResult("ERROR", "Script Interpreter Non-alphabetic character found in command word.");
				LogErrorLine(WCS_LOG_SEVERITY_ERR, Source);
				return(0);
				} // else
			} // default
		} // switch
	} // for

sprintf(ScriptErrorFmtBuf, "Script Interpreter Command word size (%d characters) exceeded.", WBufSize - 1);
ReportResult("ERROR", ScriptErrorFmtBuf);
if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, ScriptErrorFmtBuf);
return(0);

} // ScriptEntity::PullWord

/*===========================================================================*/

long ScriptEntity::LookupWord(char *Word)
{
unsigned long MIndex, CurEntry, LetIndex, WordLen;
char CurSym;

// printf("DEBUG: \"%s\"...", Word);
WordLen = (unsigned long)strlen(Word);
for (MIndex = 0; MIndex < WordLen; MIndex++)
	{
	Word[MIndex] = toupper(Word[MIndex]);
	} // for

MIndex = LetIndex = 0;
 // We won't be leaving by the usual way
while (1)	//lint !e716
	{
	CurEntry = MarkovTable[MIndex];
	if ((CurEntry & 0xff000000) == 0xff000000)
		{ // Code ChristmasTree: Run out of index before end of word. Fail/Bail.
		//printf("\nDEBUG: Ran into index end before completing resolution.\n");
		return(-1);
		} // if
	
	CurSym = (char)((CurEntry & 0x1f000000) >> 24);
	if (CurSym == (Word[LetIndex] - 'A'))
		{
		//printf("[%c] ", Word[LetIndex]);
		if (LetIndex == (WordLen - 1))
			{ // out of letters, see if we terminate here
			if (CurEntry & VE_TERM_FLAG)
				{ // Perfect match, return it.
				//printf("\nDEBUG: Good match %08lx.\n", CurEntry);
				return((long)(CurEntry & 0x00ffffff));
				} // if
			else
				{ // More likely a shortened match, hunt for it
				return(HuntShort(CurEntry & 0x00ffffff));
				} // else
			} // if
		else
			{ // More letters to go...
			MIndex = CurEntry & 0x00ffffff; // Where to go next
			LetIndex++;
			} // else
		} // if
	else
		{
		if (CurEntry & VE_LAST_ENTRY)
			{ // Unable to match word. Bail.
			//printf("ERROR: Reached end of table level without finding letter '%c'.\n", Word[LetIndex]);
			return(-1);
			} // if
		else
			{ // Look at next entry.
			MIndex++;
			} // if
		} // else
	} // whilever

} // ScriptEntity::LookupWord

/*===========================================================================*/

long ScriptEntity::HuntShort(unsigned long BeginPoint)
{

while (1)	//lint !e716
	{
	if (MarkovTable[BeginPoint] & VE_LAST_ENTRY)
		{
		if (MarkovTable[BeginPoint] & VE_TERM_FLAG)
			{ // Unique enough for me.
			//printf("\nDEBUG: HuntShort found a good match.\n");
			return((long)(MarkovTable[BeginPoint] & 0x00ffffff));
			} // if
		else
			{
			BeginPoint = (MarkovTable[BeginPoint] & 0x00ffffff);
			} // else
		} // if
	else
		{
		//printf("\nDEBUG: HuntShort found an ambiguous entry.\n");
		return(-1); /* couldn't resolve into a unique word */
		} // else
	} // whilever

} // ScriptEntity::HuntShort

/*===========================================================================*/

struct MWS_Entry *ScriptEntity::SearchMe(struct MWS_Entry *FromHere, long WordUp)
{

// why not?
while (1)	//lint !e716
	{
	if ((FromHere->Word & 0x00ffffff) == (unsigned)WordUp)
		{
		return(FromHere); // Success
		} // if
	else
		{
		if (FromHere->Word & RECOG_ENDLEVEL)
			{ // You've failed, my son.
			return(NULL);
			} // if
		else
			{
			FromHere = &FromHere[1]; // Move on...
			} // else
		} // else
	} // whilever

} // ScriptEntity::SearchMe

/*===========================================================================*/

void ScriptEntity::TrimArg(char *Dest, char *Source, int DestLen)
{
// Trims off leading spaces and equals signs, and copies to dest string.
int Skim, Out, Trimming;

Trimming = 1;
for (Out = Skim = 0;;Skim++)
	{
	if ((Out == DestLen) || (Source[Skim] == NULL))
		{
		Dest[Out] = NULL;
		return;
		} /* if */
	if (Trimming)
		{
		if ((Source[Skim] != '=') && (Source[Skim] != ' '))
			{
			Trimming = 0; /* continues below... */
			}
		} /* if */
	if (!Trimming) /* there's a reason this isn't just an else clause... */
		{
		if ((Source[Skim] != '\n') && (Source[Skim] != '\r'))
			{
			Dest[Out++] = Source[Skim];
			} /* if */
		} /* else */
	} /* for */
} /* ScriptEntity::TrimArg() */

/*===========================================================================*/

char *ScriptEntity::FetchInlineArg(char *Inline, char *ArgDest, int ArgSize)
{ // just slightly copied and hacked from PullWord()
int WordSize, Index, Quoted;

// printf("DEBUG: Input to InlineArg={%s}.\n", Inline);

memset(ArgDest, 0, ArgSize);
for (Quoted = Index = WordSize = 0; WordSize < (ArgSize - 1); Index++)
	{
	switch(Inline[Index])
		{
		case '\"': // quote mode
		case '\'':
			{
			if (!Quoted)
				{
				Quoted = 1;
				break;
				} // if
			// else fall through to next
			Quoted = 2; // Opened, and closed
			} // quotes
			//lint -fallthrough
		case 0:		// NULL
		case '\n':	// EOL
		case '\r':	// Or whatever fires your rocket
			{
			if (WordSize == 0)
				{ // EOL before getting word, bail with 0
				return(Inline); // never return an invalid pointer
				} // if
			// no break here, return or fall through to next
			} // EOLs
			//lint -fallthrough
		case '\t':	// Tab
		case ' ':	// space
			{
			if (Quoted)
				{
				if ((Inline[Index] == ' ') || (Inline[Index] == '\t'))
					{ // other chars will fall past the break even if in quoted
					ArgDest[WordSize++] = Inline[Index];
					break; // skip the rest of this case
					} // if
				} // if
			if (WordSize > 0)
				{ // have some kind of word, bail with it
				if (Quoted == 2)
					{
					return(&Inline[Index + 1]);
					} // if
				else
					{
					return(&Inline[Index]);
					} // else
				} // if
			else
				{
				// Just don't do anything, Index will ++
				} // else
			break;
			} // whitespace
		default:
			{
			ArgDest[WordSize++] = Inline[Index];
			} // default
		} // switch
	} // for

printf("ERROR: Inline Arg size (%d characters) exceeded.\n", ArgSize - 1);
return(0);

} // ScriptEntity::FetchInlineArg

/*===========================================================================*/

void ScriptEntity::LogScriptError(unsigned char Magnitude, const char *ErrorMsg)
{

if (ScriptErrors) ScriptErrors->PostError(Magnitude, ErrorMsg);
ReportResult("ERROR", (char *)ErrorMsg);

} // ScriptEntity::LogScriptError

/*===========================================================================*/

char ScriptEntity::IsWordInSentance(struct CmdContext *Sent, unsigned long TestWord)
{
int Sloop;

if (Sent)
	{
	for (Sloop = 0; Sloop < Sent->NumTokens; Sloop++)
		{
		if (Sent->WordToken[Sloop] == TestWord)
			{
			return(Sloop);
			} // if
		} // for
	} // if

return(-1);
} // ScriptEntity::IsWordInSentance

/*===========================================================================*/

int ScriptEntity::ReportResult(char *KeyWord, char *Value)
{

if (IsNetwork())
	{
	// output to socket
	sprintf(OutMessage, "%s=\"%s\"\r\n", KeyWord, Value);
	#ifdef ENABLE_NETWORK_SCRIPT
	send(ScriptSocket, OutMessage, (int)strlen(OutMessage), 0);
	#endif // ENABLE_NETWORK_SCRIPT
	} // if
else
	{
	// <<<>>> log to somewhere else
	} // else

return(1); // success, always, as far as you know

} // ScriptEntity::ReportResult

/*===========================================================================*/

char OutValue[50];

/*===========================================================================*/

int ScriptEntity::ReportResult(char *KeyWord, signed long Value)
{

sprintf(OutValue, "%d", Value);
return(ReportResult(KeyWord, OutValue));

} // ScriptEntity::ReportResult

/*===========================================================================*/

int ScriptEntity::ReportResult(char *KeyWord, unsigned long Value)
{

sprintf(OutValue, "%d", Value);
return(ReportResult(KeyWord, OutValue));

} // ScriptEntity::ReportResult

/*===========================================================================*/

int ScriptEntity::ReportResult(char *KeyWord, double Value)
{

sprintf(OutValue, "%f", Value);
return(ReportResult(KeyWord, OutValue));


} // ScriptEntity::ReportResult

// ********************************* ScriptParser Code ********************************* 

ScriptParser::ScriptParser(MessageLog *LogOutput)
{
int i;

NetScriptHandle = ScriptStack = NULL;
ScriptErrors = LogOutput;
NextScriptID = 1;
TopConsumer = 0;
GlobalDebugLevel = 0;
LocalPort = 0;
RemoteIP = 0;
RemoteName[0] = NULL;
memset(&CurrentCommand, 0, sizeof(struct CmdContext));

for (i = 0; i < WCS_SCRIPT_MAX_EVENT_CONSUMERS; i++)
	{
	EventConsumers[i] = NULL;
	} // for

} // ScriptParser::ScriptParser

/*===========================================================================*/

ScriptParser::~ScriptParser()
{
ScriptEntity *NextSE = NULL;

while (ScriptStack)
	{
	NextSE = ScriptStack->Next;
	delete ScriptStack;
	ScriptStack = NextSE;
	} // while

#ifdef ENABLE_NETWORK_SCRIPT
while (NetScriptHandle)
	{
	NextSE = NetScriptHandle->Next;
	delete NetScriptHandle;
	NetScriptHandle = NextSE;
	} // while

if (LocalPort && ScriptSocket)
	{
    closesocket (ScriptSocket);
    WSACleanup ();
	} // if
#endif // ENABLE_NETWORK_SCRIPT

} // ScriptParser::~ScriptParser

/*===========================================================================*/

AppEvent *ScriptParser::CheckScriptEvent(void)
{
AppEvent *Result = NULL;

while (ScriptStack)
	{
	if (ScriptStack->ScriptFlags & WCS_SCRIPT_FLAG_TERMINATED)
		{
		TerminateScript(ScriptStack->ID);
		} // if
	else
		{
		return(ScriptStack->DoNextCommand());
		} // else
	Result = &ScriptEvent;
	} // if

return(Result);

} // ScriptParser::CheckScriptEvent

/*===========================================================================*/

AppEvent *ScriptParser::CheckNetScriptEvent(void)
{

while (NetScriptHandle)
	{
	return(NetScriptHandle->DoNextCommand(1));
	} // if

return(NULL);

} // ScriptParser::CheckScriptEvent

/*===========================================================================*/

static char ScriptIDBuf[12];
unsigned long ScriptParser::StartScript(const char *ScriptName, int SuppressStatus)
{
ScriptEntity *NuScript = NULL;
unsigned long FileLen = 100;

if (NuScript = new ScriptEntity(this))
	{
	NuScript->ID = NextScriptID++;
	if (NuScript->ScriptFile = PROJ_fopen((char *)ScriptName, "r"))
		{
		memset(ScriptIDBuf, 0, 12);
		fread(ScriptIDBuf, 1, 8, NuScript->ScriptFile);
		if (!strnicmp(ScriptIDBuf, "WCSCRIPT", 8))
			{
			if (fseek(NuScript->ScriptFile, 0, SEEK_END) != -1)
				{
				FileLen = ftell(NuScript->ScriptFile);
				fseek(NuScript->ScriptFile, 8, SEEK_SET);
				} // if
			if (!SuppressStatus)
				{
				NuScript->ScriptStatus = new BusyWin("Script:", FileLen, 'SCRP');
				} // if
			NuScript->Next = ScriptStack;
			ScriptStack = NuScript;
			return(NuScript->ID); // Success
			} // if
		else
			{
			if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Script file lacks WCSCRIPT header.");
			} // else
		} // if
	else
		{
		if (ScriptErrors) ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, "Unable to open script file.");
		} // else
	if (ScriptErrors) ScriptErrors->PostStockError(WCS_LOG_ERR_SCRIPT_FAIL, (char *)ScriptName);
	delete NuScript;
	NuScript = NULL;
	} // if

return(NULL); // Error, unable to start

} // ScriptParser::StartScript

/*===========================================================================*/

unsigned long ScriptParser::RunSingleCommand(const char *ScriptCommand)
{
ScriptEntity *NuScript = NULL;

if (ScriptCommand && ScriptCommand[0])
	{
	if (NuScript = new ScriptEntity(this))
		{
		NuScript->ID = NextScriptID++;
		NuScript->Next = ScriptStack;
		ScriptStack = NuScript;
		NuScript->OneShotBuf = ScriptCommand;
		return(NuScript->ID); // Success
		} // if
	} // if

return(NULL); // Error, unable to start

} // ScriptParser::RunSingleCommand

/*===========================================================================*/

unsigned long ScriptParser::TerminateScript(unsigned long ScriptID)
{
ScriptEntity *Reaper, *CleanUp;

for (Reaper = ScriptStack; Reaper; Reaper = Reaper->Next)
	{
	if (Reaper->ID == ScriptID)
		{
		// Unlink from list
		if (ScriptStack == Reaper)
			{
			ScriptStack = Reaper->Next;
			} // if
		else
			{
			for (CleanUp = ScriptStack; CleanUp; CleanUp = CleanUp->Next)
				{
				if (CleanUp->Next == Reaper)
					{
					CleanUp->Next = Reaper->Next;
					} // if
				} // for
			} // else
		Reaper->Next = NULL;
		delete Reaper;
		Reaper = NULL;
		return(ScriptID);
		} // if
	} // for

return(NULL);

} // ScriptParser::TerminateScript

/*===========================================================================*/

unsigned long ScriptParser::TerminateAllScripts(void)
{
ScriptEntity *Genocide, *NextVictim = NULL;
unsigned long DeathToll = 0;

for (Genocide = ScriptStack; Genocide;)
	{
	++DeathToll;
	NextVictim = Genocide->Next;
	Genocide->Next = NULL;
	delete Genocide;
	Genocide = NextVictim;
	} // for

ScriptStack = NULL;
return(DeathToll);

} // ScriptParser::TerminateAllScripts

/*===========================================================================*/

void ScriptParser::RemoveHandler(WCSModule *ScriptConsumer)
{
unsigned long Search;

// Removes any and ALL occurances of a Handler that it finds in the
// consumers list -- since duplicate entries are a bad thing.

for (Search = 0; Search < TopConsumer; ++Search)
	{
	if (EventConsumers[Search] == ScriptConsumer)
		{
		EventConsumers[Search] = NULL;
		} // if
	} // if

UpdateTop();

} // ScriptParser::RemoveHandler

/*===========================================================================*/

unsigned long ScriptParser::AddHandler(WCSModule *ScriptConsumer)
{
unsigned long Search;

// First, we'll make sure that we're not already IN the list...
RemoveHandler(ScriptConsumer);

for (Search = 0; Search < WCS_SCRIPT_MAX_EVENT_CONSUMERS; Search++)
	{
	if (EventConsumers[Search] == NULL)
		{
		EventConsumers[Search] = ScriptConsumer;
		UpdateTop();
		return(1);
		} // if
	} // if

return(0);

} // ScriptParser::AddHandler

/*===========================================================================*/

unsigned int ScriptParser::UpdateTop(void)
{
int Search;

TopConsumer = 0;

for (Search = 0; Search < WCS_SCRIPT_MAX_EVENT_CONSUMERS; Search++)
	{
	if (EventConsumers[Search])
		{
		TopConsumer = Search + 1;
		} // if
	} // for

return(TopConsumer);

} // ScriptParser::UpdateTop

/*===========================================================================*/

unsigned int ScriptParser::BroadCastEvent(ScriptEntity *Source)
{
unsigned int ListeningConsumers = 0, TargetConsumer;
long InvokeResult, FinalResult = LONG_MIN;

if (Source)
	{
	Source->ReportResult("PARSESTATUS", "OK");
	for (TargetConsumer = 0; TargetConsumer < TopConsumer; TargetConsumer++)
		{
		if (EventConsumers[TargetConsumer])
			{
			// Return 0 or >0 if ok, <0 for error
			EventConsumers[TargetConsumer]->SetCommonVars(&ScriptEvent, GlobalApp, NULL, NULL);
			if ((InvokeResult = EventConsumers[TargetConsumer]->HandleEvent()) < 0)
				{ // Error return from script command, halt script.
				Source->ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;
				Source->SetLastResult(InvokeResult);
				if (ScriptErrors)
					{
					sprintf(ScriptErrorFmtBuf, "Script aborted, error code %d on line %d.", InvokeResult, Source->LinesProcessed);
					ScriptErrors->PostError(WCS_LOG_SEVERITY_ERR, ScriptErrorFmtBuf);
					} // if
				Source->ReportResult("ERRORCODE", InvokeResult);
				Source->ReportResult("ERRORLINE", Source->LinesProcessed);
				return(ListeningConsumers);
				} // if
			else
				{
				if (FinalResult == LONG_MIN)
					{
					FinalResult = InvokeResult;
					} // if
				} // else
			ListeningConsumers++;
			} // if
		} // for
	} // if

Source->SetLastResult(FinalResult);
Source->ReportResult("RESULT", FinalResult);
return(ListeningConsumers);

} // ScriptParser::BroadCastEvent

/*===========================================================================*/

int winsockInit (void)
{
#ifdef ENABLE_NETWORK_SCRIPT

    WORD version = MAKEWORD(2,2);
    WSADATA data;
    int ret;

    ret = WSAStartup (version, &data);
    
    if (ret != 0) /* There was an error in Winsock startup */
        return FALSE;
#endif // ENABLE_NETWORK_SCRIPT

    return TRUE;
}

/*===========================================================================*/

int ScriptParser::SetupNetworkScript(int Port, char *PermitRemote)
{
int ret = 0;
#ifdef ENABLE_NETWORK_SCRIPT
//int optionValue = SO_SYNCHRONOUS_NONALERT;
u_long NonBlock;

SOCKADDR_IN saServer;

// getpeername, setsockopt

if (Port && PermitRemote)
	{
	if (winsockInit())
		{
		strncpy(RemoteName, PermitRemote, 250);
		RemoteName[250] = NULL;
		RemoteIP = inet_addr(PermitRemote);
		//setsockopt(INVALID_SOCKET, SOL_SOCKET, SO_OPENTYPE, (char *)&optionValue, sizeof(optionValue));

		ScriptSocket = socket (AF_INET, SOCK_STREAM, IPPROTO_TCP);
    
		if (!ScriptSocket) /* Error socket was not created */
			return FALSE;

		saServer.sin_family = AF_INET;
		saServer.sin_addr.s_addr = INADDR_ANY; 
		saServer.sin_port = htons (Port);

		ret = bind (ScriptSocket, (SOCKADDR *)&saServer, sizeof(SOCKADDR));
		if (ret) /* There was an error binding this socket */
			{ // most likely port number in use -- autoincrement a few (8) times and see if that solves it
			int PortNumRetry = 1;
			while (PortNumRetry < 8)
				{
				// set up to try again
				saServer.sin_family = AF_INET;
				saServer.sin_addr.s_addr = INADDR_ANY; 
				// increment port number
				saServer.sin_port = htons (Port + PortNumRetry);
				// and try binding again
				ret = bind (ScriptSocket, (SOCKADDR *)&saServer, sizeof(SOCKADDR));
				if (ret)
					{ // still an error
					PortNumRetry++;
					continue; // keep trying
					} // if
				else
					{ // we're ok, go ahead
					break; // out of while
					} // else
				} // while
			if (ret)
				{ // bail out if we didn't get anywhere
				return FALSE;
				} // if
			} // if
		ret = listen(ScriptSocket, 1);
		if (ret) /* There was an error binding this socket */
			return FALSE;

		// accept() somewhere
		// _open_osf_handle
		// _fdopen

		NonBlock = 1;
		ioctlsocket(ScriptSocket, FIONBIO, &NonBlock); //lint !e569
		WSAAsyncSelect(ScriptSocket, (HWND)GlobalApp->WinSys->GetRoot(), 0xC000, FD_CLOSE | FD_CONNECT | FD_ACCEPT);

   		LocalPort = Port;
		if (NetScriptHandle = new ScriptEntity(this))
			{
			NetScriptHandle->ScriptSocket = ScriptSocket;
			return TRUE;
			} // if
		} // if
	} // if
#endif // ENABLE_NETWORK_SCRIPT
return(FALSE);

} // ScriptParser::SetupNetworkScript

/*===========================================================================*/

int ScriptParser::NetReportResult(char *KeyWord, char *Value)
{
#ifdef ENABLE_NETWORK_SCRIPT

if (NetScriptHandle && NetScriptHandle->IsNetwork() && NetScriptHandle->ScriptSocket)
	{
	// output to socket
	sprintf(OutMessage, "%s=\"%s\"\r\n", KeyWord, Value);
	send(NetScriptHandle->ScriptSocket, OutMessage, (int)strlen(OutMessage), 0);
	} // if
#endif // ENABLE_NETWORK_SCRIPT

return(1); // success, always, as far as you know

} // ScriptParser::NetReportResult

/*===========================================================================*/

int ScriptParser::NetSendString(char *Value)
{
#ifdef ENABLE_NETWORK_SCRIPT

if (NetScriptHandle && NetScriptHandle->IsNetwork() && NetScriptHandle->ScriptSocket)
	{
	// output to socket
	send(NetScriptHandle->ScriptSocket, Value, (int)strlen(Value), 0);
	} // if
#endif // ENABLE_NETWORK_SCRIPT

return(1); // success, always, as far as you know

} // ScriptParser::NetSendString

/*===========================================================================*/

int ScriptParser::NetCheckAbort(void)
{
#ifdef ENABLE_NETWORK_SCRIPT
char InputChar;
long NetReadLen;

if (NetScriptHandle && NetScriptHandle->IsNetwork() && NetScriptHandle->ScriptSocket)
	{
	// output to socket
	//send(NetScriptHandle->ScriptSocket, Value, strlen(Value), 0);
	if ((NetReadLen = recv(NetScriptHandle->ScriptSocket, &InputChar, 1, 0)) != SOCKET_ERROR)
		{
		if (InputChar == 0x1B) // escape, I hope
			{
			return(1);
			} // if
		} // if
	} // if
#endif // ENABLE_NETWORK_SCRIPT

return(0);

} // ScriptParser::NetCheckAbort
