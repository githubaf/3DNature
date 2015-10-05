// Requester.h
// Common GUI requester classes
// built from scratch on 11 Jun 1995 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_REQUESTER_H
#define WCS_REQUESTER_H

// define FASTCALL if Microsoft compiler & generating Win32 code
#ifndef FASTCALL
#if defined(_MSC_VER) && defined(_WIN32)
#define FASTCALL __fastcall
#else // _MSC_VER
#define FASTCALL
#endif // _MSC_VER
#endif // FASTCALL

#include "RequesterBasic.h"

#include "AppModule.h"
#include "Application.h"
#include "Fenetre.h"
#include "GUI.h"

class GUIContext;
class BusyHandler;
class BusyWin;
class FileReq;
class AnimDoubleTime;

#define WCS_REQUESTER_FILE_MULTI		(1 << 0)
#define WCS_REQUESTER_FILE_SAVE			(1 << 1)
#define WCS_REQUESTER_FILE_DIRONLY		(1 << 2)
#define WCS_REQUESTER_FILE_NOMUNGE		(1 << 3)

#define WCS_REQUESTER_POSINTEGERS_ONLY 	"abcdefghijklmnopqrstuvwxyz.-"
#define WCS_REQUESTER_FILESAFE_ONLY 	"!#$%&*()\'\"\\/,?[]{}"
#define WCS_REQUESTER_POSDIGITS_ONLY 	"abcdefghijklmnopqrstuvwxyz-"
#define WCS_REQUESTER_JUSTLETTERS	 	"0123456789!#$%&*()\'\"\\/,?[]{}.-+=_|;:,><~`@^"
#define WCS_REQUESTER_NUMERIC_ONLY	 	"!#$%&*()\'\"\\/,?[]{}=_|;:,><~`@^abcdefghijklmnopqrstuvwxyz"

#define WCS_REQUESTER_MAX_STUFFEDNAMES	32

#define WCS_REQUESTER_MAX_BUSYSTACK		16

#define WCS_REQUESTER_POPUPNOTIFIER_AUTOHIDEDELAY	30


#ifdef _WIN32
#define WCS_REQUESTER_WILDCARD			"*.*"
#define WCS_REQUESTER_PARTIALWILD(a)	"*."a
#endif // _WIN32

class FileReq
	{
	private:
		#ifdef _WIN32
		OPENFILENAME NativeReq;
		char *FNameList, *FNameListStore;
		#endif // _WIN32

		unsigned long NameIndex, NumFiles;
		char Title[42], IDefFName[66], IDefPath[260], CustPat[42], IDefPat[100], FinalCombo[16384];
		char *StuffedNames[WCS_REQUESTER_MAX_STUFFEDNAMES];
		char *GetNextStuffedName(void);

	
	public:
		FileReq();
		~FileReq();
		void SetDefFile(char *DefFName);
		void SetDefPath(char *DefPath);
		void SetDefPat(char  *DefPat);
		void SetTitle(char *NewTitle);
		int StuffNames(int NumNames, ...);
		inline void ClearStuffedNames(void) {int Init; for(Init = 0; Init < WCS_REQUESTER_MAX_STUFFEDNAMES; Init++) StuffedNames[Init] = NULL;};
		int Request(int Flags);
		unsigned long FilesSelected(void) {return (NumFiles);};
		const char *GetFirstName(void);
		const char *GetNextName(void);
	
	}; // FileReq

// This is only called from the GUIContext, to set up the requester
// code with a convenient pointer to the GUIContext, so you don't
// have to keep supplying it.
void SetupReqLocalGUI(GUIContext *GUIInit);

// this is just just a front-end for FileReq->Request that works like the V1 function
short GetFileNamePtrn(long Mode, char *ReqName, char *PathName, char *FileName, char *Ptrn, int NameLen = 32);

// return value indicates button pressed, 0=rightmost, increasing to left
// DefButton follows same numbering scheme.
// Should be 0 = rightmost, 1 = leftmost, increasing to right
unsigned char UserMessageNOMEMLargeDEM(char *Topic, unsigned long ReqSize, unsigned long AvailSize, int Width, int Height);
unsigned char UserMessageCustom(char *Topic, char *Message,
 char *ButtonA, char *ButtonB, char *ButtonC, unsigned char DefButton = 0);
unsigned char UserMessageCustomQuad(char *Topic, char *Message,
 char *ButtonA, char *ButtonB, char *ButtonC, char *ButtonD, unsigned char DefButton = 0);
void          UserMessageDemo(char *Message);

class BusyWin : public WCSModule, public GUIFenetre
	{
	private:
		time_t StartSeconds;
		unsigned long MaxSteps, CurSteps, CurPos, LastTickCount;
		int AbortStatus;
		char *TitleStore;

		static BusyWin *BusyStack[WCS_REQUESTER_MAX_BUSYSTACK];

		#ifdef _WIN32
		HWND PrevActive;
		void RepaintFuelGauge(void);
		#endif // _WIN32
	public:

		BusyWin(char *Title, unsigned long Steps, unsigned long Section, int TimeEst = 0);
		~BusyWin();
#ifdef SPEED_MOD
		int FASTCALL Update(unsigned long Step); // now returns abort status since we have to check anyway
#else // SPEED_MOD
		int Update(unsigned long Step); // now returns abort status since we have to check anyway
#endif // SPEED_MOD
		void SetNewMaxSteps(unsigned long NewMaxSteps) {MaxSteps = NewMaxSteps; Update(1);};
		void SetNewTitle(char *NewTitle);
		void Reconfigure(char *NewTitle, unsigned long NewMaxSteps, unsigned long NewCurStep);
		inline int CheckAbort(void);
		NativeGUIWin Construct(void);
		unsigned long GetCurSteps(void) {return(CurSteps);};
		char *GetReqTitle(void) {return(TitleStore);};
	}; // BusyWin


short GetInputString(char *Message, char *Reject, char *String);
int GetInputValue(char *Message, AnimDoubleTime *ADT);
double GetInputTime(char *Message);
#ifndef WCS_REQUESTER_CPP
extern GUIContext *ReqLocalGUI;
extern WCSApp *GlobalApp;
extern char BusyWinAbort;
#endif // !WCS_REQUESTER_CPP

int BusyWin::CheckAbort(void)
{
#ifdef _WIN32
MSG BusyEvent;
#endif // _WIN32
unsigned long CurTickCount;

//if(this && NativeWin)
if(this)
	{
	// timing should limit abort checks to every 63 milliseconds or 15 times per second max
	CurTickCount = GetTickCount(); // counter in milliseconds, coarse-grained to about 20ms
	CurTickCount &= 0xffffffc0; // mask off lowest 6 bits as insignificant
	if(CurTickCount != LastTickCount)
		{
		// Special event checking
		#ifdef _WIN32
		while(ReqLocalGUI->CheckNoWait(&BusyEvent))
			{
			GlobalApp->ProcessOSEvent(&BusyEvent);
			} // while
		#endif // _WIN32
		} // if

	return(BusyWinAbort);
	} // if
return(0);
} // BusyWin::CheckAbort

class PopupNotifier
	{
	friend class Fenetre;
	private:
		bool Visible, Added;
		NOTIFYICONDATA NID;
		double LastDisplayedTime;
		
		void HandleNotificationEvent(AppEvent *Activity, WCSApp *AppScope);
		
	public:
		PopupNotifier();
		~PopupNotifier();
		bool ShowNotification(const char *Title, const char *Message, int Icon = NULL, bool CloseButton = false);
		bool HideNotification(void);
		bool QueryIsVisible(void) const {return(Added && Visible);};
		void HideIfNeeded(void);
	}; // PopupNotifier

#endif // WCS_REQUESTER_H
