// PathAndFile.h
// Header file for PathAndFile object for World Construction Set v4
// Built from scratch on 4/13/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_PATHANDFILE_H
#define WCS_PATHANDFILE_H

#include "Types.h"

class PathAndFile;
class Project;

#define WCS_PATHANDFILE_PATH_LEN				256
#define WCS_PATHANDFILE_PATH_LEN_MINUSONE		255
#define WCS_PATHANDFILE_NAME_LEN				64
#define WCS_PATHANDFILE_NAME_LEN_MINUSONE		63
#define WCS_PATHANDFILE_PATH_PLUS_NAME_LEN		(WCS_PATHANDFILE_PATH_LEN + WCS_PATHANDFILE_NAME_LEN)
#define WCS_PATHANDFILE_P_PLUS_N_LEN_MINUSONE	((WCS_PATHANDFILE_PATH_LEN + WCS_PATHANDFILE_NAME_LEN) - 1)

class PathAndFile
	{
	public:
		char Path[WCS_PATHANDFILE_PATH_LEN], Name[WCS_PATHANDFILE_NAME_LEN];

		PathAndFile();
		void Copy(PathAndFile *CopyTo, PathAndFile *CopyFrom);
		const char *SetPath(const char *NewPath);
		const char *SetName(const char *NewName);
		void SetPathAndName(const char *NewPath, const char *NewName);
		const char *GetPath(void);
		const char *GetName(void);

		const char *AppendDirToPath(char *NewDir);
		const char *RemoveLowestDirFromPath(void);

		char *GetFramePathAndName(char *Dest, char *FileExt, int FrameNum, long StringLen, char NumDigits);
		char *GetValidPathAndName(char *Dest);
		int ValidatePath(void);
		char *GetPathAndName(char *Dest);
		int MatchPathAndName(char *MatchPath, char *MatchName);
		void MungPath(Project *ProjHost);
		void UnMungPath(Project *ProjHost);
		int SelectFile(void);
		int SelectPath(void);
		int SearchDirectoryTree(char *RootDir);
		ULONG Save(FILE *ffile);
		unsigned long Load(FILE *ffile, unsigned long ReadSize, short ByteFlip);

	}; // class PathAndFile

#define WCS_PATHANDFILE_PATH			0x00010000
#define WCS_PATHANDFILE_NAME			0x00020000

#endif // WCS_PATHANDFILE_H
