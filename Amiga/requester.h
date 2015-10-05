// requester.h
// Common GUI requester classes
// built from scratch on 11 Jun 1995 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_REQUESTER_H
#define WCS_REQUESTER_H

#include "GUI.h"

#define WCS_REQUESTER_FILE_MULTI		1 << 0
#define WCS_REQUESTER_FILE_SAVE		1 << 1


class FileReq
	{
	private:
		#ifdef AMIGA
		struct FileRequester *NativeReq;
		#endif
		#ifdef WIN32
		OPENFILENAME NativeReq;
		#endif // WIN32
		GUIContext *StashedGUI;
		unsigned long int NameIndex, NumFiles;
		char Title[22], IDefFName[34], IDefPath[260], IDefPat[42], FinalCombo[260];
	
	public:
		#ifdef AMIGA
		FileReq(GUIContext *GUI);
		#endif // AMIGA
		#ifdef WIN32
		FileReq(HWND ParentWin);
		#endif // WIN32
		~FileReq();
		void SetDefFile(char *DefFName);
		void SetDefPath(char *DefPath);
		void SetDefPat(char  *DefPat);
		void SetTitle(char *NewTitle);
		int Request(int Flags);
		const char *GetFirstName(void);
		const char *GetNextName(void);
	
	}; // FileReq

#endif // WCS_REQUESTER_H
