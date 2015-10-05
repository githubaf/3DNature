// UsefulPathString.cpp
// Path and String related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulPathString.h"

/*===========================================================================*/

unsigned long int MakeIDFromString(const char *IDStr)
{
unsigned long int IDAcc = NULL, IDTemp;

if(IDStr)
	{
	IDTemp = IDStr[0];
	IDAcc |= (IDTemp << 24);
	IDTemp = IDStr[1];
	IDAcc |= (IDTemp << 16);
	IDTemp = IDStr[2];
	IDAcc |= (IDTemp << 8);
	IDAcc |= IDStr[3];
	} // if

return(IDAcc);
} // MakeIDFromString

/*===========================================================================*/

char *StripExtension(char *Source)
{
unsigned char CIndex, FoundDot;

if(Source)
	{
	for(FoundDot = CIndex = 0; Source[CIndex]; CIndex++)
		{
		if(Source[CIndex] == '.')
			{
			FoundDot = CIndex;
			} // if
		} // for
	if(FoundDot)
		{
		Source[FoundDot] = NULL;
		return(Source);
		} // if
	} // if
return(NULL);
} // StripExtension

/*===========================================================================*/

// adds extension if it doesn't already exist, returns Source string
// does not replace any existing extensions
char *AddExtension(char *Source, char *AddExt)
{
int ExtLen, SceLen;

if (! Source || ! AddExt)
	return (Source);

ExtLen = (int)strlen(AddExt);
SceLen = (int)strlen(Source);
if (SceLen > ExtLen)
	{
	if (Source[SceLen - ExtLen - 1] == '.' && ! stricmp(&Source[SceLen - ExtLen], AddExt))
		return (Source);
	} // if

if (AddExt[0] != '.')
	{
	strcat(Source, ".");
	} // if
strcat(Source, AddExt);

return (Source);

} // AddExtension

/*===========================================================================*/

static char BackEscapeBuf[1024];

// EscapeBackslashes turns each backslash into two (Adobe Illustrator file strings)
char *EscapeBackslashes(char *InputPath)
{
int InLoop, OutLoop = 0;

if(InputPath && InputPath[0])
	{
	for(InLoop = 0; InputPath[InLoop]; InLoop++)
		{
		BackEscapeBuf[OutLoop++] = InputPath[InLoop];
		if(InputPath[InLoop] == '\\')
			{
			// write one extra backslash
			BackEscapeBuf[OutLoop++] = '\\';
			} // if
		} // for
	} // if

BackEscapeBuf[OutLoop] = NULL;
return(BackEscapeBuf);
} // EscapeBackslashes

/*===========================================================================*/

char *UnifyPathGlyphs(char *Path)
{
int i, FoundColon = 0;

for(i = 0; Path[i]; i++)
	{
	if((Path[i] == ':'))
		{
		if(FoundColon)
			{
			Path[i] = '\\';
			} // if
		else
			{
			FoundColon = 1;
			} // else
		} // if
	if((Path[i] == '/') || (Path[i] == '\\'))
		{
		Path[i] = '\\';
		} // if
	} // for

return(Path);
} // UnifyPathGlyphs

/*===========================================================================*/

int IsPathGlyph(char Candidate)
{
return(Candidate == ':' || Candidate == '/' || Candidate == '\\');
} // IsPathGlyph

/*===========================================================================*/

char GetNativePathGlyph(void) {return('\\');};

/*===========================================================================*/

/*** Not used FPW2 10/04/07
// currently only used in Mac LW Export
char *ForceAllPathGlyphs(char *Path, char Glyph)
{
int i, FoundColon = 0;

for(i = 0; Path[i]; i++)
	{
	if((Path[i] == ':'))
		{
		Path[i] = Glyph;
		} // if
	if((Path[i] == '/') || (Path[i] == '\\'))
		{
		Path[i] = Glyph;
		} // if
	} // for

return(Path);
} // ForceAllPathGlyphs
***/

/*===========================================================================*/

// currently only used in LW Export
char *ForceAmigaPathGlyphs(char *Path)
{
int i, OrigLen, FoundColon = 0, FollowingSlashRemoved = 0;

OrigLen = (int)strlen(Path);

for(i = 0; Path[i]; i++)
	{
	if (Path[i] == ':')
		{
		if (FoundColon)
			Path[i - FollowingSlashRemoved] = '/';
		else
			FoundColon = 1;
		} // if
	else if (Path[i] == '\\')
		{
		if (FoundColon && Path[i - 1] == ':')
			FollowingSlashRemoved = 1;
		else
			Path[i - FollowingSlashRemoved] = '/';
		} // if
	else if (FollowingSlashRemoved)
		Path[i - 1] = Path[i];
	} // for

if (FollowingSlashRemoved)
	Path[OrigLen - 1] = 0;

return(Path);
} // ForceAmigaPathGlyphs

/*===========================================================================*/

void strmfp(char *name, const char *path, const char *node)
{
char EndTest;

name[0] = 0;

if(path[0])
	{
	strcpy(name, path);
	if (! node[0])
		return;
	EndTest = path[strlen(path) - 1];
	if(!((EndTest == '\\') || (EndTest == ':') || (EndTest == '/')))
		{ // add a directory seperator
		strcat(name, "\\");
		} // if
	} // if

strcat(name, node);
} // strmfp()


/*===========================================================================*/

int stcgfe(char *ext, const char *name)
{
int size = 0, nodesize = (int)strlen(name);

while (nodesize-- > 0)
	{
	if  (name[nodesize] == '.')
	 {
	 strcpy(ext, &name[nodesize + 1]);
	 break;
	 } // if dot found
	size ++;
	} // if

return(size);

} // stcgfe()


/*===========================================================================*/

#ifndef _MSC_VER // MSVC++ provides strupr() and strlwr()
char *strupr(char *Source)
{
int Skim;

if(Source)
   {
   for(Skim = 0; Source[Skim]; Skim++)
      {
      Source[Skim] = toupper(Source[Skim]);
      } // for
   } // if
return(Source);
} // strupr()

/*===========================================================================*/

char *strlwr(char *Source)
{
int Skim;

if(Source)
   {
   for(Skim = 0; Source[Skim]; Skim++)
      {
      Source[Skim] = tolower(Source[Skim]);
      } // for
   } // if
return(Source);
} // strlwr()
#endif // !_MSC_VER

/*===========================================================================*/

char *strncpyupr(char *Dest, const char *Source, size_t Len)
{
unsigned int Loop;
if(Source && Dest && Len)
	{
	for(Loop = 0; Source[Loop] && Loop < Len; Loop++)
		{
		Dest[Loop] = toupper(Source[Loop]);
		} // for
	Dest[Loop] = NULL;
	} // if
return(Dest);
} // strncpyupr

/*===========================================================================*/

char *strncpylwr(char *Dest, const char *Source, size_t Len)
{
unsigned int Loop;
if(Source && Dest && Len)
	{
	for(Loop = 0; Source[Loop] && Loop < Len; Loop++)
		{
		Dest[Loop] = tolower(Source[Loop]);
		} // for
	Dest[Loop] = NULL;
	} // if
return(Dest);
} // strncpylwr

/*===========================================================================*/

char *strdoublenullcopy(char *Dest, const char *Source)
{

int FullLen;

if(Source && Dest)
	{
	for(FullLen = 0; Source[FullLen] || Source[FullLen + 1]; FullLen++)
		{
		// Nothing to do but count.
		} // for
	FullLen += 2; // Two NULLs
	memcpy(Dest, Source, FullLen);
	} // if

return(Dest);
} // strdoublenullcopy

/*===========================================================================*/

// Function is similar to fgets except line is read until CR, CR/LF pair, or LF is read, and line terminator is stripped.
// ToN = Terminate on NULL - set to true to handle frickin ESRI
// NoEOF = return string if EOF found (thanks ESRI).  Check for EOF yourself if you enable this!
char *fgetline( char *string, int n, FILE *stream, char ToN, char NoEOF)
{
int ch, i = 0;
char *out = string;
bool more = true;

while (more && (i < (n - 1)))
	{
	ch = fgetc(stream);
	if (ch == EOF)		// error or end of file
		{
		if (NoEOF)
			{
			*out = 0;	// terminate the string
			return string;
			}
		else
			return NULL;
		}
	//*out++ = (char)ch;
	i++;
	if (ch == 0x0A)		// stop on LF
		more = false;
	else if ((ToN) && (ch == 0))
		more = false;
	else if ((ch == 0x0D) && (i < (n - 1)))
		{
		ch = fgetc(stream);
		if (ch == EOF)
			return NULL;
		if (ch != 0x0A)	// CR found, but not a CR/LF pair, so put back this character
			(void)ungetc(ch, stream);	// ignore any possibility of an error in this function
		//else
		//	{
		//	*out++ = 0x0A;
		//	i++;
		//	} // else
		more = false;	// we either have a CR or CR/LF pair now, so stop
		} // else if
	if (more)
		*out++ = (char)ch;
	} // while

*out = 0;	// terminate the created string
return string;

} // fgetline


/*===========================================================================*/

// C-linkage frontend for above for use by external C libraries that can't figure out C++ linkage
extern "C" {
char *fgetlineC( char *string, int n, FILE *stream, char ToN, char NoEOF)
{
return(fgetline(string, n, stream, ToN, NoEOF));
} // fgetlineC

} // extern "C"



/*===========================================================================*/

void BreakFileName(char *FullName, char *PathPart, int PathSize, char *FilePart, int FileSize)
{
int Scan;
char Backup;

if(FullName && PathPart && PathSize && FilePart && FileSize)
	{
	for(Scan = (int)(strlen(FullName) - 1); Scan > -1; Scan--)
		{
		if((FullName[Scan] == '\\') || (FullName[Scan] == '/') ||
		 (FullName[Scan] == ':'))
			{
			strncpy(FilePart, &FullName[Scan + 1], FileSize);
			FilePart[FileSize - 1] = NULL;
			Backup = FullName[Scan + 1];
			FullName[Scan + 1] = NULL;
			strncpy(PathPart, FullName, PathSize);
			PathPart[PathSize - 1] = NULL;
			FullName[Scan + 1] = Backup;
			return;
			} // if
		} // for
	strncpy(FilePart, FullName, FileSize);
	PathPart[0] = NULL;
	} // if

} // BreakFileName

/*===========================================================================*/

char *ScanToNextWord(char *String)
{
char *NextWord;
int BlankFound = 0, StartBlank = 0;

NextWord = String;
if (NextWord[0] == ' ')
	StartBlank = 1;
while (NextWord[0])
	{
	if (NextWord[0] == ' ')
		{
		if (! StartBlank)
			BlankFound = 1;
		} // if
	else if (StartBlank)
		StartBlank = 0;
	else if (BlankFound)
		return (NextWord);
	NextWord ++;
	} // while

return (String);

} // ScanToNextWord

/*===========================================================================*/

char *FindFileExtension(char *String)
{
char *FoundDot;

FoundDot = &String[strlen(String) - 1];
while (FoundDot >= String && *FoundDot != '.')
	{
	FoundDot --;
	} // while

if (FoundDot >= String)
	return (FoundDot + 1);
return (NULL);

} // FindFileExtension

/*===========================================================================*/

static char NameInsertTempBuf[15], BigNameInsertTempBuf[1024];
char *InsertNameNum(char *DestName, const char *NumStr, int Append)
{
int First, NumHash, Loop;
char AlphaDigits;

memset(NameInsertTempBuf, 0, 10);

if(DestName && NumStr)
	{
	for(First = NumHash = Loop = 0; DestName[Loop]; Loop++)
		{
		if(DestName[Loop] == '#')
			{
			if(NumHash == 0)
				{
				First = Loop;
				} // if
			NumHash++;
			} // if
		else
			{
			if(NumHash)
				{
				break;
				} // else
			} // else
		} // for
	Loop = (int)strlen(NumStr);
	if(NumHash)
		{ // Insert into DestName
		if(NumHash > 8)
			{
			NumHash = 8;
			} // if
		memcpy(&DestName[First], &NumStr[Loop - NumHash], NumHash);
		} // if
	else if (Append)
		{ // Append to end of DestName
		NumHash = 0;
		for(First = 0; NumStr[First]; First++)
			{
			if(NumHash)
				{
				NumHash++;
				} // if
			else
				{
				if(NumStr[First] != '0')
					{
					NumHash++;
					} // if
				} // else
			} // for
		if(NumHash < 3)
			{
			NumHash = 3;
			} // if
		AlphaDigits = 0;
		AlphaDigits += (isdigit((unsigned char)NumStr[Loop - 1]) == 0);
		AlphaDigits += (isdigit((unsigned char)NumStr[Loop - 2]) == 0);
		memcpy(NameInsertTempBuf, &NumStr[Loop - (NumHash + AlphaDigits)], NumHash + AlphaDigits);
		strcat(DestName, NameInsertTempBuf);
		} // else if
	return(DestName);
	} // if

return(NULL);
} // InsertNameNum

/*===========================================================================*/

char *InsertNameNumDigits(char *DestName, unsigned int Number, int Digits)
{
int /*First, */ NumHash, Loop;
char *Continue = NULL;
char *BeginNum = NULL;

sprintf(NameInsertTempBuf, "%010d", Number);
if(Digits > 10) Digits = 10;

BeginNum = &NameInsertTempBuf[10 - Digits];

if(DestName)
	{
	for(/*First = */NumHash = Loop = 0; DestName[Loop]; Loop++)
		{
		if(DestName[Loop] == '#')
			{
			if(NumHash == 0)
				{
				//First = Loop;
				DestName[Loop] = NULL;
				} // if
			NumHash++;
			} // if
		else
			{
			if(NumHash)
				{
				Continue = &DestName[Loop];
				break;
				} // else
			} // else
		} // for
	strcpy(BigNameInsertTempBuf, DestName); // copy all, or up to first #
	if(Digits)
		{
		strcat(BigNameInsertTempBuf, BeginNum); // Automagically gets the right number of digits
		} // if
	if(Continue)
		{ // re-add portion after # signs
		strcat(BigNameInsertTempBuf, Continue);
		} // if
	strcpy(DestName, BigNameInsertTempBuf);
	return(DestName);
	} // if

return(NULL);
} // InsertNameNumDigits

/*===========================================================================*/

char MatchChar(const char CheckChar, const char *MatchList)
{
for(int SkimChar = 0; MatchList[SkimChar]; SkimChar++)
	{
	if(CheckChar == MatchList[SkimChar]) return(CheckChar);
	} // for
return(0);
} // MatchChar

/*===========================================================================*/


char *SkipPastNextSpace(const char *InStr)
{
unsigned long int SkipLoop;
char FoundSpace = 0;

if(InStr)
	{
	for(SkipLoop = 0; InStr[SkipLoop] != 0; SkipLoop++)
		{
		if(FoundSpace)
			{
			if(!isspace((unsigned char)InStr[SkipLoop]))
				{
				return((char *)&InStr[SkipLoop]);
				} // if
			} // if
		else
			{
			if(isspace((unsigned char)InStr[SkipLoop]))
				{
				FoundSpace = 1;
				} // if
			} // else
		} // for
	} // if

return(NULL);
} // SkipPastNextSpace


/*===========================================================================*/

// I wrote this because strtok is so unpredictable

char *SkipSpaces(const char *InStr)
{
int Skip;

if(InStr)
	{
	for(Skip = 0; InStr[Skip]; Skip++)
		{
		if(InStr[Skip] != ' ')
			{
			return((char *)&InStr[Skip]);
			} // if
		} // for
	} // if

return(NULL);
} // SkipSpaces

/*===========================================================================*/

char *TrimTrailingSpaces(char *String)
{
signed int Search;

for(Search = (int)(strlen(String) - 1); Search >= 0; Search--)
	{ // Nuke trailing spaces
	if(String[Search] == ' ')
		{
		String[Search] = NULL;
		} // if
	else
		{
		break;
		} // else
	} // for
return(String);
} // TrimTrailingSpaces()

/*===========================================================================*/

char *TrimTrailingDigits(char *String)
{
signed int Search;

for(Search = (int)(strlen(String) - 1); Search >= 0; Search--)
	{ // Nuke trailing spaces
	if(isdigit((unsigned char)String[Search]))
		{
		String[Search] = NULL;
		} // if
	else
		{
		break;
		} // else
	} // for
return(String);
} // TrimTrailingDigits()

/*===========================================================================*/

void TrimZeros(char *String)
{
int i;

for(i = (int)(strlen(String) - 1); i > -1; i--)
	{
	switch (String[i])
		{
		case '.':
			{
			String[i] = NULL;
			i = -1; // end loop
			break;
			} // '.'
		case '0':
			{
			String[i] = NULL;
			break;
			} // '0'
		default:
			{
			i = -1; // end loop
			break;
			} // default
		} // switch
	} // for

return;
} // TrimZeros

/*===========================================================================*/

void TrimDecimalZeros(char *String)
{
int i, Decimal = 0;

for(i = 0; String[i]; i++)
	{
	if(String[i] == '.')
		{
		Decimal = 1;
		break;
		} // if
	} // for

if(!Decimal) return;

for(i = (int)(strlen(String) - 1); i > -1; i--)
	{
	switch (String[i])
		{
		case '.':
			{
			String[i] = NULL;
			i = -1; // end loop
			break;
			} // '.'
		case '0':
			{
			String[i] = NULL;
			break;
			} // '0'
		default:
			{
			i = -1; // end loop
			break;
			} // default
		} // switch
	} // for

return;
} // TrimDecimalZeros

/*===========================================================================*/

int CountIntDigits(const char *String)
{
int Digits = 0, Loop;
if(String)
	{
	for(Loop = 0;;Loop++)
		{
		if(isdigit((unsigned char)String[Loop]))
			{
			Digits++;
			} // if
		else if((String[Loop] == '.') || (String[Loop] == NULL))
			{
			break;
			} // if
		} // for
	} // if

return(Digits);
} // CountIntDigits

/*===========================================================================*/

int CountDecDigits(const char *String)
{
int Digits = 0, Loop, OkCount = 0;
if(String)
	{
	for(Loop = 0;;Loop++)
		{
		if(String[Digits] == '.')
			{
			OkCount = 1;
			} // if
		if(isdigit((unsigned char)String[Loop]))
			{
			if(OkCount)
				{
				Digits++;
				} // if
			} // if
		else if(String[Loop] == NULL)
			{
			break;
			} // if
		} // for
	} // if

return(Digits);
} // CountDecDigits

/*===========================================================================*/

char *strovly(char *Dest, char *Src)
{
int Ole;

if(Dest && Src)
	{
	for(Ole = 0; Src[Ole]; Ole++)
		{
		Dest[Ole] = Src[Ole];
		} // if
	} // if
return(NULL);
} // strovly()

/*===========================================================================*/

char *MakeCompletePath(char *Dest, char *Base, char *ObjName, char *ObjExt)
{
char Padname[20], Short[20];

if(!(Dest && Base && ObjName && ObjExt))
	{
	return(NULL);
	} // if

Dest[0] = NULL;
strcpy(Padname, "          ");
strncpy(Short, ObjName, 10);
Short[10] = NULL;
strovly(Padname, Short);

strcat(Padname, ObjExt);
strmfp(Dest, Base, Padname);

return(Dest);

} // MakeCompletePath()

/*===========================================================================*/

char *MakeNonPadPath(char *Dest, char *Base, char *ObjName, char *ObjExt)
{
char LongName[64];
int ExtLen;

if(!(Dest && Base && ObjName && ObjExt))
	{
	return(NULL);
	} // if

Dest[0] = NULL;
ExtLen = (int)strlen(ObjExt);
if(ExtLen > 10)
	{
	ExtLen = 10;
	} // if
strncpy(LongName, ObjName, 63);
LongName[63 - ExtLen] = NULL;
strncat(LongName, ObjExt, ExtLen);
LongName[63] = NULL;

strmfp(Dest, Base, LongName);

return(Dest);

} // MakeNonPadPath()

/*===========================================================================*/

char *ReplaceChar(char *String, char Replace, char ReplaceWith)
{
char *i;

for (i = String; *i; i ++)
	{
	if (*i == Replace)
		*i = ReplaceWith;
	} // for

return (String);

} // ReplaceChar

/*===========================================================================*/

// do an atoi conversion on a substring (style 0 = 0..n-1 numbering, style 1 = 1..n numbering
int atoisub(const char *string, unsigned short first, unsigned short last, unsigned char style)
{
char buffer[32];

assert(string != NULL);
assert(last > first);
assert((last - first) < 32);
if (style != 0)
	assert(first != 0);

if (style == 0)
	strncpy(buffer, string + first, last - first + 1);
else
	strncpy(buffer, string + first - 1, last - first + 1);
buffer[last - first + 1] = 0;
return atoi(buffer);

} // atoisub()

/*===========================================================================*/

// Used for Web URL/URI encoding/escaping

int ismark(int c)
{

switch(c)
	{
	case '$':
	case '-':
	case '_':
	case '.':
	case '!':
	case '~':
	case '*':
	case '\'':
	case '(':
	case ')':
	case ',':
		return(1);
	} // c

return(0);
} // ismark

/*===========================================================================*/

int EscapeURI(char *Inbuf, char *Outbuf, int OutbufSize)
{
int in, out;
char Escode[5];

if(!Inbuf || !Outbuf || OutbufSize == 0) return(0);

for(in = out = 0; Inbuf[in];)
	{
	if((Inbuf[in]) && (out < OutbufSize - 1))
		{
		if(isalpha(Inbuf[in]) || isdigit(Inbuf[in]) || ismark(Inbuf[in]))
			{
			// leave intact
			Outbuf[out++] = Inbuf[in++];
			} // if
		else
			{
			// escape it
			Outbuf[out++] = '%';
			sprintf(Escode, "%02x", Inbuf[in++]);
			Outbuf[out++] = Escode[0];
			Outbuf[out++] = Escode[1];
			} // else
		} // if
	} // for

Outbuf[out] = NULL;
return(out);
} // EscapeURI

/*===========================================================================*/

char *FindNextCharInstance(char *Str, char Search)
{
char *Place = Str;

if (Place)
	{
	while (*Place)
		{
		if (*Place == Search)
			return (Place);
		Place ++;
		} // while
	} // if

return (NULL);

} // FindNextCharInstance

/*===========================================================================*/

// Split string into tokens, allowing a qualifier to indicate that a substring should be treated as a single entity.
// Functions similar to strtok.
// qualifier is normally " or '
// qualifier must also appear in delims
char *strtok2(char *str, const char *delims, const char qualifier)
{
static char *lastToken = NULL;
char *tmp;
char qStr[2];

if (str == NULL)
	{
	str = lastToken;
	if (str == NULL)
		return(NULL);
	} // if

qStr[0] = qualifier;
qStr[1] = 0;
if (*str == qualifier)
	{
	++str;
	tmp = strpbrk(str, qStr);
	if (tmp)
		{
		*tmp = NULL;
		lastToken = tmp + 1;
		} // if
	else
		{
		// no matching qualifier was found - they're getting the rest of the string as a field
		lastToken = NULL;
		} // else
	} // if
else
	{
	str += strspn(str, delims);
	tmp = strpbrk(str, delims);
	if (tmp)
		{
		*tmp = NULL;
		lastToken = tmp + 1;
		} // if
	else
		lastToken = NULL;
	} // else if

return(str);

} // strtok2
