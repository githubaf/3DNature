// Script.h
// Script parsing and dispatching header
// Created from markov/GrammarTable.h and markov/ParseTest.c
// on 27 May 1997 by Chris 'Xenon' Hanson
// Copyright 1997

#include "stdafx.h"

#ifndef WCS_SCRIPT_H
#define WCS_SCRIPT_H
#include "Application.h"
#include "ScriptArg.h"

class ScriptParser;
class ScriptEntity;
class BusyWin;
class MessageLog;
class WCSModule;

// Configure the maximum number of Script-Event consumers
#define WCS_SCRIPT_MAX_EVENT_CONSUMERS		30
// Max words in a sentance
#define WCS_SCRIPT_MAX_WORD_LEN				20
// Max chars in a single word
#define WCS_SCRIPT_MAX_SINGLE_LEN			40
// Max length chars of an inline arg
#define WCS_SCRIPT_MAX_INLINE_LEN			40
// Max length of arg string
#define WCS_SCRIPT_MAX_ARG_LEN				4096
// Size of command-line input buffer
#define WCS_SCRIPT_MAX_INPUT_LEN			8192
// Size of error-output buffer
#define WCS_SCRIPT_MAX_ERROR_LEN			(WCS_SCRIPT_MAX_INPUT_LEN + 1024)


// Flag definitions for decoding various tables
#define VE_TERM_FLAG			(1U << 31)
#define VE_TERM_FLAG_EIGHT		(1 << 7)
#define VE_LAST_ENTRY			(1 << 30)
#define VE_LAST_ENTRY_EIGHT		(1 << 6)

#define RECOG_WILD				(1 << 29) // Means word requires arg
#define RECOG_ENDLEVEL			(1 << 28) // End of this level of table

struct CmdContext
	{
	char *RawText, NumTokens;
	unsigned long EventCode, WordToken[WCS_SCRIPT_MAX_WORD_LEN], LastToken;
	char InlineArg[WCS_SCRIPT_MAX_INLINE_LEN], ArgStr[WCS_SCRIPT_MAX_ARG_LEN];
	ScriptEntity *SourceScript;
	}; // struct CmdContext

struct MWS_Entry
	{
	unsigned long Word, Follow, EventCode;
	};

enum
	{
	WCS_SCRIPT_EVENT_PROJECT = 1,
	WCS_SCRIPT_EVENT_SCRIPT,
	WCS_SCRIPT_EVENT_RENDER,
	WCS_SCRIPT_EVENT_WINDOW,
	WCS_SCRIPT_EVENT_MAPVIEW,
	WCS_SCRIPT_EVENT_CAMVIEW,
	WCS_SCRIPT_EVENT_RENDSET,
	WCS_SCRIPT_EVENT_PARAM,
	WCS_SCRIPT_EVENT_EFFECT,
	WCS_SCRIPT_EVENT_DATABASE,
	WCS_SCRIPT_EVENT_IMAGELIB,
	WCS_SCRIPT_EVENT_GISIMPORT,
	WCS_SCRIPT_EVENT_QUIT
	}; // Script Event types

// Boolean states a ScriptEntity can have.
#define WCS_SCRIPT_FLAG_TERMINATED		(1 << 0)



class ScriptEntity
	{
	friend class ScriptParser;
	private:
		ScriptEntity *Next;
		FILE *ScriptFile;
		int ScriptSocket;
		unsigned long ID, ScriptFlags, CommandsProcessed, LinesProcessed;
		long LastResult;
		BusyWin *ScriptStatus;
		MessageLog *ScriptErrors;
		ScriptParser *ScriptHost;
		char ScriptDebugLevel;
		int NetCommandBufSize;
		const char *OneShotBuf;
		
		AppEvent *DoNextCommand(int NetMode = 0);

		void LogErrorLine(unsigned char Magnitude, char *RawLine);
		long ParseDispatch(char *InLine);
		char *PullWord(char *Source, char *WordBuf, int WBufSize);
		long LookupWord(char *Word);
		long HuntShort(unsigned long BeginPoint);
		struct MWS_Entry *SearchMe(struct MWS_Entry *FromHere, long WordUp);
		void TrimArg(char *Dest, char *Source, int DestLen);
		char *FetchInlineArg(char *Inline, char *ArgDest, int ArgSize);

	public:
		ScriptEntity(ScriptParser *Host);
		~ScriptEntity();
		inline unsigned long GetID(void) {return(ID);};
		inline void SetTerminate(void) {ScriptFlags |= WCS_SCRIPT_FLAG_TERMINATED;};
		inline char GetDebugLevel(void) {return(ScriptDebugLevel);};
		inline void SetDebugLevel(char Level) {if(ScriptDebugLevel > -1) ScriptDebugLevel = Level;};
		void LogScriptError(unsigned char Magnitude, const char *ErrorMsg);
		inline ScriptParser *GetHost(void) {return(ScriptHost);};
		inline int GetFeedback(void) {return(ScriptStatus ? 1 : 0);};
		inline void SetLastResult(long LastRes) {LastResult = LastRes;};
		inline long GetLastResult(void) {return(LastResult);};
		char IsWordInSentance(struct CmdContext *Sent, unsigned long TestWord);

		int ReportResult(char *KeyWord, char *Value);
		int ReportResult(char *KeyWord, signed long Value);
		int ReportResult(char *KeyWord, unsigned long Value);
		int ReportResult(char *KeyWord, double Value);

		int IsNetwork(void) {return(ScriptSocket != NULL);};
	
	}; // ScriptEntity

class ScriptParser
	{
	friend class ScriptEntity;
	private:
		ScriptEntity *ScriptStack;
		ScriptEntity *NetScriptHandle;
		unsigned long NextScriptID;
		struct CmdContext CurrentCommand;
		AppEvent ScriptEvent;
		MessageLog *ScriptErrors;
		WCSModule *EventConsumers[WCS_SCRIPT_MAX_EVENT_CONSUMERS];
		unsigned int TopConsumer;
		char GlobalDebugLevel;
		
		unsigned int UpdateTop(void);
		unsigned int BroadCastEvent(ScriptEntity *Source);
		unsigned long RemoteIP;
		char RemoteName[255];
		int ScriptSocket, LocalPort;
		
	public:
		ScriptParser(MessageLog *LogOutput);
		~ScriptParser();
		AppEvent *CheckScriptEvent(void);
		AppEvent *CheckNetScriptEvent(void);
		unsigned long StartScript(const char *ScriptName, int SuppressStatus = 0);
		unsigned long RunSingleCommand(const char *ScriptCommand);
		unsigned long TerminateScript(unsigned long ScriptID);
		unsigned long TerminateAllScripts(void);
		unsigned long AddHandler(WCSModule *ScriptConsumer);
		void RemoveHandler(WCSModule *ScriptConsumer);
		inline void SetDebugLevel(char NewLevel) {GlobalDebugLevel = NewLevel;};
		inline char GetDebugLevel(void) {return(GlobalDebugLevel);};

		int SetupNetworkScript(int Port, char *PermitRemote);
		int NetReportResult(char *KeyWord, char *Value);
		int NetSendString(char *Value);
		int NetCheckAbort(void);

	}; // ScriptParser

#endif // WCS_SCRIPT_H
