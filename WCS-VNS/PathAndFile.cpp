// PathAndFile.cpp
// PathAndFile object for World Construction Set v4
// Built from scratch on 4/13/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved

#include "stdafx.h"
#include "ImageFormatConfig.h"
#include "PathAndFile.h"
#include "Useful.h"
#include "Requester.h"
#include "Project.h"
#include "Application.h"
#include "FSSupport.h"

PathAndFile::PathAndFile()
{

Path[0] = 0;
Name[0] = 0;

} // PathAndFile::PathAndFile

/*===========================================================================*/

void PathAndFile::Copy(PathAndFile *CopyTo, PathAndFile *CopyFrom)
{

strcpy(CopyTo->Path, CopyFrom->Path);
strcpy(CopyTo->Name, CopyFrom->Name);

} // PathAndFile::Copy

/*===========================================================================*/

const char *PathAndFile::SetPath(const char *NewPath)
{

strncpy(Path, NewPath, WCS_PATHANDFILE_PATH_LEN_MINUSONE);
Path[WCS_PATHANDFILE_PATH_LEN_MINUSONE] = 0;
return ((const char *)Path);

} // PathAndFile::SetPath

/*===========================================================================*/

const char *PathAndFile::SetName(const char *NewName)
{

strncpy(Name, NewName, WCS_PATHANDFILE_NAME_LEN_MINUSONE);
Name[WCS_PATHANDFILE_NAME_LEN_MINUSONE] = 0;
return ((const char *)Name);

} // PathAndFile::SetName

/*===========================================================================*/

void PathAndFile::SetPathAndName(const char *NewPath, const char *NewName)
{

strncpy(Path, NewPath, WCS_PATHANDFILE_PATH_LEN_MINUSONE);
Path[WCS_PATHANDFILE_PATH_LEN_MINUSONE] = 0;
strncpy(Name, NewName, WCS_PATHANDFILE_NAME_LEN_MINUSONE);
Name[WCS_PATHANDFILE_NAME_LEN_MINUSONE] = 0;

} // PathAndFile::SetPathAndName

/*===========================================================================*/

const char *PathAndFile::GetPath(void)
{

return ((const char *)Path);

} // PathAndFile::GetPath

/*===========================================================================*/

const char *PathAndFile::GetName(void)
{

return ((const char *)Name);

} // PathAndFile::GetName

/*===========================================================================*/


const char *PathAndFile::AppendDirToPath(char *NewDir)
{
int OrigLen;

OrigLen = (int)strlen(Path);
if(OrigLen > 0)
	{
	if(!IsPathGlyph(Path[OrigLen - 1]))
		{
		// do we have room to append anything?
		if(OrigLen < WCS_PATHANDFILE_PATH_LEN_MINUSONE)
			{
			Path[OrigLen] = GetNativePathGlyph();
			OrigLen++; // Account for our '/' in current length
			// Put new NULL on end
			Path[OrigLen] = NULL;
			} // if
		} // if
	strncat(Path, NewDir, WCS_PATHANDFILE_PATH_LEN_MINUSONE - OrigLen);
	// ensure no unterminated strings
	Path[WCS_PATHANDFILE_PATH_LEN_MINUSONE] = 0;
	return(Path);
	} // if
else
	{
	return(SetPath(NewDir));
	} // else

} // PathAndFile::AppendDirToPath


/*===========================================================================*/


const char *PathAndFile::RemoveLowestDirFromPath(void)
{
int OrigLen, ScanPos;

OrigLen = (int)strlen(Path);

if(OrigLen > 0)
	{
	// work backwards looking for a path glyph
	for(ScanPos = OrigLen - 1; ScanPos > -1; ScanPos--)
		{
		if(IsPathGlyph(Path[ScanPos]))
			{
			Path[ScanPos] = NULL;
			return(Path);
			} // if
		} // for
	return(NULL);
	} // if
else
	{
	return(NULL);
	} // else

} // PathAndFile::RemoveLowestDirFromPath


/*===========================================================================*/

char *PathAndFile::GetValidPathAndName(char *Dest)
{
FILE *fTest;

if (Path[0] && Name[0])
	{
	strmfp(Dest, Path, Name);
	if (fTest = PROJ_fopen(Dest, "r"))
		{
		fclose(fTest);
		return (Dest);
		} // if
#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
#ifdef WCS_BUILD_ECW_SUPPORT
	else if(!strnicmp("ecwp:", Dest, 5))
		{ // we assume ecwp: remote files are valid since it's not efficient to check
		return(Dest);
		} // if
#endif // WCS_BUILD_ECW_SUPPORT
#endif // WCS_BUILD_REMOTEFILE_SUPPORT
	} // if

return (NULL);

} // PathAndFile::GetValidPathAndName

/*===========================================================================*/

int PathAndFile::ValidatePath(void)
{
int Result;

RepeatCheck:

if (PROJ_chdir(Path))
	{
	if ((Result = UserMessageYNCAN("Output Event Path", "An invalid output directory is specified for one of the Output Events. Select a new directory or cancel the rendering?")) == 0)
		return (0);
	if (Result == 2)
		{
		if (SelectPath())
			goto RepeatCheck;
		return (0);
		} // if
	return (1);
	} // if

return (1);

} // PathAndFile::ValidatePath

/*===========================================================================*/

char *PathAndFile::GetFramePathAndName(char *Dest, char *FileExt, int FrameNum, long StringLen, char NumDigits)
{
unsigned long int TempLen;

if (Path[0] && Name[0])
	{
	TempLen = (unsigned long)(strlen(Path) + strlen(Name) + 10);
	if(FileExt) TempLen += (unsigned long)strlen(FileExt);
	if(TempLen < (unsigned long)StringLen)
		{
		strmfp(Dest, Path, Name);
		if(InsertNameNumDigits(Dest, (unsigned int)FrameNum, NumDigits))
			{
			if(FileExt && FileExt[0])
				{
				strcat(Dest, FileExt);
				} // if
			return(Dest);
			} // if
		} // if
	} // if

return (NULL);

} // PathAndFile::GetFramePathAndName

/*===========================================================================*/

char *PathAndFile::GetPathAndName(char *Dest)
{

if (Path[0] && Name[0])
	{
	strmfp(Dest, Path, Name);
	} // if
else
	Dest[0] = 0;

return (Dest);

} // PathAndFile::GetPathAndName

/*===========================================================================*/

int PathAndFile::MatchPathAndName(char *MatchPath, char *MatchName)
{

return ((! stricmp(Path, MatchPath)) && (! stricmp(Name, MatchName)));

} // PathAndFile::MatchPathAndName

/*===========================================================================*/

void PathAndFile::MungPath(Project *ProjHost)
{

strcpy(Path, ProjHost->MungPath(Path));

} // ImageLib::MungPath

/*===========================================================================*/

void PathAndFile::UnMungPath(Project *ProjHost)
{

strcpy(Path, ProjHost->UnMungPath(Path));

} // PathAndFile::UnMungPath

/*===========================================================================*/

int PathAndFile::SelectFile(void)
{
char Ptrn[32];

strcpy(Ptrn, WCS_REQUESTER_WILDCARD);

return (GetFileNamePtrn(0, "Select File", Path, Name, Ptrn, WCS_PATHANDFILE_NAME_LEN));

} // PathAndFile::SelectFile

/*===========================================================================*/

int PathAndFile::SelectPath(void)
{
char Ptrn[32];

strcpy(Ptrn, WCS_REQUESTER_WILDCARD);

// uses the save flag so that a valid file name is not required sinc it might be for image saving
return (GetFileNamePtrn(1, "Select File", Path, Name, Ptrn, WCS_PATHANDFILE_NAME_LEN));

} // PathAndFile::SelectPath

/*===========================================================================*/

int PathAndFile::SearchDirectoryTree(char *RootDir)
{
char SearchStr[512];
WIN32_FIND_DATA FileData;
HANDLE Hand;

strmfp(SearchStr, RootDir, "*.*");
if ((Hand = FindFirstFile(SearchStr, &FileData)) != INVALID_HANDLE_VALUE)
	{
	if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
		{
		if (FileData.cFileName[0] != '.')
			{
			strmfp(SearchStr, RootDir, FileData.cFileName);
			if (SearchDirectoryTree(SearchStr))
				{
				FindClose(Hand);
				return (1);
				} // if
			} // if
		} // if
	else if (! stricmp(GetName(), FileData.cFileName))
		{
		SetPath(GlobalApp->MainProj->UnMungPath(RootDir));
		FindClose(Hand);
		return (1);
		} // else

	while (FindNextFile(Hand, &FileData))
		{
		if (FileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
			{
			if (FileData.cFileName[0] != '.')
				{
				strmfp(SearchStr, RootDir, FileData.cFileName);
				if (SearchDirectoryTree(SearchStr))
					{
					FindClose(Hand);
					return (1);
					} // if
				} // if
			} // if
		else if (! stricmp(GetName(), FileData.cFileName))
			{
			SetPath(GlobalApp->MainProj->UnMungPath(RootDir));
			FindClose(Hand);
			return (1);
			} // else
		} // while
	FindClose(Hand);
	} // if

return (0);

} // PathAndFile::SearchDirectoryTree

/*===========================================================================*/

ULONG PathAndFile::Load(FILE *ffile, unsigned long ReadSize, short ByteFlip)
{
ULONG ItemTag = 0, Size, BytesRead, TotalRead = 0;
union MultiVal MV;

while (ItemTag != WCS_PARAM_DONE)
	{
	// read block descriptor tag from file
	if (BytesRead = ReadBlock(ffile, (char *)&ItemTag,
		WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG, ByteFlip))
		{
		TotalRead += BytesRead;
		if (ItemTag != WCS_PARAM_DONE)
			{
			// read block size from file
			if (BytesRead = ReadBlock(ffile, (char *)&MV, ItemTag & 0x0000ffff, ByteFlip))
				{
				TotalRead += BytesRead;
				BytesRead = 0;
				switch (ItemTag & 0xff)
					{
					case WCS_BLOCKSIZE_CHAR:
						{
						Size = MV.Char[0];
						break;
						}
					case WCS_BLOCKSIZE_SHORT:
						{
						Size = MV.Short[0];
						break;
						}
					case WCS_BLOCKSIZE_LONG:
						{
						Size = MV.Long;
						break;
						}
					default:
						break;
					} // switch

				switch (ItemTag & 0xffff0000)
					{
					case WCS_PATHANDFILE_PATH:
						{
						BytesRead = ReadBlock(ffile, (char *)Path, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					case WCS_PATHANDFILE_NAME:
						{
						BytesRead = ReadBlock(ffile, (char *)Name, WCS_BLOCKTYPE_CHAR + Size, ByteFlip);
						break;
						}
					default:
						{
						if (! fseek(ffile, Size, SEEK_CUR))
						BytesRead = Size;
						break;
						} 
					} // switch

				TotalRead += BytesRead;
				if (BytesRead != Size)
					break;
				} // if size block read
			else
				break;
			} // if not done flag
		} // if tag block read
	else
		break;
	} // while

return (TotalRead);

} // PathAndFile::Load

/*===========================================================================*/

ULONG PathAndFile::Save(FILE *ffile)
{
ULONG BytesWritten, ItemTag, TotalWritten = 0;

if ((BytesWritten = PrepWriteBlock(ffile, WCS_PATHANDFILE_PATH, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Path) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Path)) == NULL)
	goto WriteError;
  TotalWritten += BytesWritten;
  if ((BytesWritten = PrepWriteBlock(ffile, WCS_PATHANDFILE_NAME, WCS_BLOCKSIZE_CHAR,
	WCS_BLOCKTYPE_CHAR, (unsigned long)(strlen(Name) + 1),
	WCS_BLOCKTYPE_CHAR, (char *)Name)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;

  ItemTag = WCS_PARAM_DONE;
  if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKSIZE_LONG + WCS_BLOCKTYPE_LONGINT)) == NULL)
   goto WriteError;
  TotalWritten += BytesWritten;
goto WriteIt;

WriteError:
TotalWritten = 0UL;

WriteIt:
return (TotalWritten);

} // PathAndFile::Save
