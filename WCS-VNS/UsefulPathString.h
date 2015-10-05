// UsefulPathString.h
// Path and String related code from Useful.h
// Built from Useful.h on 060403 by CXH

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_USEFULPATHSTRING_H
#define WCS_USEFULPATHSTRING_H

// Good ol' IFF-style IDs.
#    ifndef MAKE_ID
// Augh! Lint made me do it!
#      define MAKE_ID(a,b,c,d)	(const unsigned long)((((unsigned)(unsigned char)(a)) << 24) | (((unsigned)(unsigned char)(b)) << 16) | (((unsigned)(unsigned char)(c)) << 8) | ((unsigned)(unsigned char)(d)))
#    endif // !MAKE_ID


unsigned long MakeIDFromString(const char *IDStr);

// This does the obvious.
char *StripExtension(char *Source);

// adds extension if it doesn't already exist, returns Source string
char *AddExtension(char *Source, char *AddExt);

// EscapeBackslashes turns each backslash into two (Adobe Illustrator file strings)
char *EscapeBackslashes(char *InputPath);

// Performs slash, backslash, colon translation on paths
char *UnifyPathGlyphs(char *Path);

int IsPathGlyph(char Candidate);

char GetNativePathGlyph(void);

// Performs slash, backslash, colon translation to a sepcified glyph on paths
// currently only used in Mac LW Export
//char *ForceAllPathGlyphs(char *Path, char Glyph);
// currently only used in LW Export
char *ForceAmigaPathGlyphs(char *Path);

// This assembles a full path/file name from components.
// These were included in the standard library on the Ami.
void strmfp(char *name, const char *path, const char *node);
// This copies the existing extension from name to ext
int stcgfe(char *ext, const char *name);

#ifdef _MSC_VER // Microsoft provides _str(upr|lwr)
// it doesn't seem we need to define this anymore
//#define strupr(Source) _strupr(Source)
//#define strlwr(Source) _strlwr(Source)
#endif // _MSC_VER
// These copy and upper/lower case in one whack
char *strncpyupr(char *Dest, const char *Source, size_t Len);
char *strncpylwr(char *Dest, const char *Source, size_t Len);

char *strdoublenullcopy(char *Dest, const char *Source);

// Function is similar to fgets except line is read until CR, CR/LF pair, or LF is read, and line terminator is stripped.
char *fgetline( char *string, int n, FILE *stream, char TerminateOnNull = 0, char ReturnLineIfEOF = 0);


// This breaks apart a fully qualified path/file into seperate path and file
void BreakFileName(char *FullName, char *PathPart, int PathSize, char *FilePart, int FileSize);
char *ScanToNextWord(char *String);
char *FindFileExtension(char *String);


// This writes characters from the right side NumStr over top of the first
// block of # symbols in the DestName string. If no # symbols are found,
// it appends up to two letters and three digits from the right side of
// NumStr onto the end of DestName.
//
// Note: Make sure DestName is big enough to handle up to 8 extra characters!
char *InsertNameNum(char *DestName, const char *NumStr, int Append = 1);
char *InsertNameNumDigits(char *DestName, unsigned int Number, int Digits);

char MatchChar(const char CheckChar, const char *MatchList);
char *SkipPastNextSpace(const char *InStr);
char *SkipSpaces(const char *InStr);
char *TrimTrailingSpaces(char *String);
char *TrimTrailingDigits(char *String);
void TrimZeros(char *String);
void TrimDecimalZeros(char *String);
int CountIntDigits(const char *String);
int CountDecDigits(const char *String);
char *strovly(char *Dest, char *Src);
char *MakeCompletePath(char *Dest, char *Base, char *ObjName, char *ObjExt);
char *MakeNonPadPath(char *Dest, char *Base, char *ObjName, char *ObjExt);
char *ReplaceChar(char *String, char Replace, char ReplaceWith);
char *FindNextCharInstance(char *String, char Search);

char *strtok2(char *str, const char *delims, const char qualifier);

// do an atoi conversion on a substring (style 0 = 0..n-1 numbering, style 1 = 1..n numbering
int atoisub(const char *string, unsigned short first, unsigned short last, unsigned char style);

// Used for Web URL/URI encoding/escaping
int ismark(int c);
int EscapeURI(char *Inbuf, char *Outbuf, int OutbufSize);

#endif // WCS_USEFULPATHSTRING_H
