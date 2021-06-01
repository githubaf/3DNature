/*
** Header junk for RexxSupport.c
** Might go into WCS.h. Might not.
**/

#ifndef	REXX_SUPPORT_H
#define	REXX_SUPPORT_H

#include	<exec/types.h>
#include	<exec/nodes.h>
#include	<exec/lists.h>
#include	<exec/ports.h>

#include	<rexx/storage.h>
#include	<rexx/rxslib.h>

#include <stdio.h>
#include <libraries/mui.h>

#define	REXX_RETURN_ERROR	((struct RexxMsg *)-1L)

/*
 * A structure for the ARexx handler context
 * This is *VERY* *PRIVATE* and should not be touched...
 */

/*EXTERN*/ struct	ARexxContext
{
struct	MsgPort	*ARexxPort;	/* The port messages come in at... */
struct	Library	*RexxSysBase;	/* We will hide the library pointer here... */
	long	Outstanding;	/* The count of outstanding ARexx messages... */
	char	PortName[24];	/* The port name goes here... */
	char	ErrorName[28];	/* The name of the <base>.LASTERROR... */
	char	Extension[8];	/* Default file name extension... */
};

extern ULONG Rexx_SigMask(struct ARexxContext *This);
extern struct RexxMsg *Rexx_GetMsg(struct ARexxContext *This);
extern void Rexx_ReplyMsg(struct RexxMsg *This, char *RString, LONG Error);
extern short Rexx_SetLastError(struct ARexxContext *This, struct RexxMsg *rmsg,
 char *ErrorString);
extern short Rexx_SendMsg(struct ARexxContext *This, char *RString, short StringFile);
extern void Rexx_Del(struct ARexxContext *This);
extern struct ARexxContext *Rexx_New(char *AppName);

extern long int Cmd_ParseDispatch(struct ARexxContext *Rexx, struct RexxMsg *CmdMsg);
extern char *Cmd_PullWord(char *Source, char *WordBuf, int WBufSize);
extern long int Cmd_LookupWord(char *Word);
extern long int Cmd_HuntShort(unsigned long int BeginPoint);
extern struct MWS_Entry *Cmd_SearchMe(struct MWS_Entry *FromHere, long int WordUp);
extern void Cmd_TrimArg(char *Dest, char *Source, int DestLen);
extern void DemoFunc(struct CmdContext *Call, char *FromZone);
extern char *Cmd_FetchInlineArg(char *Inline, char *ArgDest, int ArgSize);
#endif	/* REXX_SUPPORT_H */
