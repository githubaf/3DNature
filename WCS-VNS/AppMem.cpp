// AppMem.cpp
// Application-level memory allocation
// Hopefully a portable layer over all the weird memory systems
// we're running into.
// Created from scratch on 4/2/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"
#include "AppMem.h"
#include "Requester.h"
#include "Useful.h"
#ifdef APPMEM_USE_MALLOC
#include <malloc.h>
#endif // APPMEM_USE_MALLOC

#ifdef _DEBUG
#define APPMEM_MUNG_CHECK
#define APPMEM_MUNG_EXTRA	20
#endif // _DEBUG

// Used for reporting a variety of local errors.
#if defined (APPMEM_TRACK_USAGE) || defined (APPMEM_MUNG_CHECK)
static char OutLine[255];
#ifndef APPMEM_TRACK_LOCATION
static char OutComp[80];
#endif // APPMEM_TRACK_LOCATION
#endif // APPMEM_TRACK_USAGE || APPMEM_MUNG_CHECK

#define APPMEM_WARN_DEMSIZE	500

// Used for warning of excessive DEM sizes.
static int WarnMaxDEMWidth, WarnMaxDEMHeight;

#ifdef APPMEM_TRACK_USAGE
#ifdef APPMEM_TRACK_LOCATION
#define APPMEM_TRACK_LOCATION_MAXFILES	1024
static char MemTrackDebugStr[255];
static void *AppMem_CurMemBlock;
static int CurrentLine, CurrentFileNum;
static char AppMem_FileNumTrack[APPMEM_TRACK_LOCATION_MAXFILES][40];
#endif // APPMEM_TRACK_LOCATION

static unsigned long AppMem_MainHeapSize,
		  AppMem_MainMaxSize,
		  AppMem_MainUsage;
#endif // APPMEM_TRACK_USAGE

//lint --e{528}

// from http://www.codeguru.com/Cpp/misc/misc/threadsprocesses/article.php/c6945__2/#more
#if (_MSC_VER >= 1310) // 1310 == VC++ 7.1
#   pragma data_seg(push, old_seg)
#endif
int init1(void);
#pragma data_seg(".CRT$XIU")
static int (*initptr)(void) = init1;
#pragma data_seg()    /* reset data segment */

int init1(void)
{
AppMem_Prep();
return 0;
} // init1

int term1(void);
#pragma data_seg(".CRT$XTU")
static int (*termptr)(void) = term1;
#pragma data_seg()    /* reset data segment */

int term1(void)
{
AppMem_Cleanup();
return 0;
} // term1

#if (_MSC_VER >= 1310) // 1310 == VC++ 7.1
#   pragma data_seg(pop, old_seg)
#endif

/*===========================================================================*/

HANDLE AppMemHeap;
unsigned long AppMem_Prep(void)
{

if (!AppMemHeap)
	{
#ifdef _OPENMP
	AppMemHeap = HeapCreate(NULL, 0, 0); // initialsize=0 maxsize=0
#else // _OPENMP
	AppMemHeap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0); // initialsize=0 maxsize=0
#endif // _OPENMP
	} // if

return(0);

} // AppMem_Prep

/*===========================================================================*/

void AppMem_Cleanup(void)
{

if (AppMemHeap)
	{
	HeapDestroy(AppMemHeap);
	AppMemHeap = NULL;
	} // if

} // AppMem_Cleanup

/*===========================================================================*/

#ifdef APPMEM_TRACK_LOCATION
static char LocationTopic[200];
void *AppMem_Tracker::AppMem_AllocReal(size_t AllocSize, unsigned long Flags, char *Topic, unsigned char Optional)
#else // !APPMEM_TRACK_LOCATION
void *AppMem_Alloc(size_t AllocSize, unsigned long Flags, char *Topic, unsigned char Optional)
#endif // !APPMEM_TRACK_LOCATION
{
void *Me = NULL;
#ifdef APPMEM_MUNG_CHECK
size_t RealSize;
time_t NowTime;
unsigned long *Toi, *ReLink;
unsigned char *Vous;
unsigned long Fluff;
#ifdef APPMEM_TRACK_LOCATION
unsigned long CopyFileNum, CopyLineNum;
#endif // APPMEM_TRACK_LOCATION

RealSize = AllocSize;
AllocSize += APPMEM_MUNG_EXTRA;
time(&NowTime);
Fluff = (unsigned long int)NowTime;
#endif // APPMEM_MUNG_CHECK

if (Topic == NULL)
	{
	Topic = (char *)"Memory Allocation Failure";
	} // if

if (AllocSize == 0)
	{
#ifdef APPMEM_TRACK_LOCATION
char *TopicSkim;
sprintf(LocationTopic, "%s %d", AppMem_FileNumTrack[CurrentFileNum], CurrentLine);
// try to trim leading path info
Topic = &LocationTopic[0];
for (TopicSkim = Topic; TopicSkim[0]; ++TopicSkim)
	{
	if ((TopicSkim[0] == '\\') || (TopicSkim[0] == '/') || (TopicSkim[0] == ':'))
		{
		Topic = &TopicSkim[1]; // next character
		} // if
	} // for
#endif // !APPMEM_TRACK_LOCATION
	UserMessageOK(Topic, "Attempt to allocate zero\nbytes -- irrational. Aborting.");
	return(NULL);
	} // if

RetryAlloc:
#ifdef APPMEM_USE_MALLOC
if (Flags & APPMEM_CLEAR)
	{
	Me = calloc(1, AllocSize);
	} // if
else
	{
	Me = malloc(AllocSize);
	} // else
#else // !APPMEM_USE_MALLOC

#ifdef _WIN32
//HGLOBAL TempHandle;
//if (TempHandle = GlobalAlloc(Flags, AllocSize)) Me = GlobalLock(TempHandle);
#ifdef _OPENMP
Me = HeapAlloc(AppMemHeap, NULL | Flags, AllocSize);
#else // _OPENMP
Me = HeapAlloc(AppMemHeap, HEAP_NO_SERIALIZE | Flags, AllocSize);
#endif // _OPENMP
#endif // _WIN32

#endif // !APPMEM_USE_MALLOC

if (Me)
	{
#ifdef APPMEM_MUNG_CHECK
	Toi = (unsigned long *)Me;
	Vous = (unsigned char *)Me;
	Toi[0] = RealSize;
#ifdef APPMEM_TRACK_LOCATION
	CopyFileNum = CurrentFileNum;
	CopyLineNum = CurrentLine;
	Toi[1] = (unsigned long int)AppMem_CurMemBlock;
	Toi[2] = NULL;
	if (AppMem_CurMemBlock)
		{
		ReLink = (unsigned long *)AppMem_CurMemBlock;
		ReLink[2] = (unsigned long int)Toi;
		} // if
	AppMem_CurMemBlock = Toi;
	memcpy(&Vous[RealSize + 12], &CopyFileNum, 4);
	memcpy(&Vous[RealSize + 16], &CopyLineNum, 4);
#else // !APPMEM_TRACK_LOCATION
	Toi[1] = Fluff;
	Toi[2] = Fluff;
	memcpy(&Vous[RealSize + 12], &Fluff, 4);
	memcpy(&Vous[RealSize + 16], &Fluff, 4);
#endif // !APPMEM_TRACK_LOCATION
	Me = (void *)&Toi[3];
#endif // APPMEM_MUNG_CHECK
#ifdef APPMEM_TRACK_USAGE
	AppMem_MainHeapSize += AllocSize;
	if (AppMem_MainHeapSize > AppMem_MainMaxSize)
		{
		AppMem_MainMaxSize = AppMem_MainHeapSize;
		} // if
	AppMem_MainUsage += AllocSize;
#endif // APPMEM_TRACK_USAGE
	} // if
else
	{
	if (!Optional)
		{
#ifdef APPMEM_TRACK_LOCATION
char *TopicSkim;
sprintf(LocationTopic, "%s %d", AppMem_FileNumTrack[CurrentFileNum], CurrentLine);
// try to trim leading path info
Topic = &LocationTopic[0];
for (TopicSkim = Topic; TopicSkim[0]; ++TopicSkim)
	{
	if ((TopicSkim[0] == '\\') || (TopicSkim[0] == '/') || (TopicSkim[0] == ':'))
		{
		Topic = &TopicSkim[1]; // next character
		} // if
	} // for
#endif // !APPMEM_TRACK_LOCATION
		if (WarnMaxDEMWidth >= APPMEM_WARN_DEMSIZE || WarnMaxDEMWidth >= APPMEM_WARN_DEMSIZE)
			{
			if (UserMessageNOMEMLargeDEM(Topic, (unsigned long)AllocSize,
			 AppMem_ReportStatus(WCS_APPMEM_REPORT_ALLFREE), WarnMaxDEMWidth, WarnMaxDEMHeight))
				{
				// I'll burn in Hell unless I rewrite this...
				goto RetryAlloc;
				} // if
			} // if
		else
			{
			if (UserMessageNOMEM(Topic, (unsigned long)AllocSize,
			 AppMem_ReportStatus(WCS_APPMEM_REPORT_ALLFREE)))
				{
				// I'll burn in Hell unless I rewrite this...
				goto RetryAlloc;
				} // if
			} // else
		} // if
	} // else

return(Me);

} // AppMem_Alloc()/AppMem_AllocReal()

/*===========================================================================*/

size_t AppMem_QueryRealSize(void *Me)
{
size_t RealSize = 0;
#ifdef APPMEM_MUNG_CHECK
unsigned long *Toi;
unsigned char *Vous;
#endif // APPMEM_MUNG_CHECK

if (Me)
	{
#ifdef APPMEM_MUNG_CHECK
	Toi = (unsigned long *)Me;
	Toi -= 3; // back up in RAM
	Vous = (unsigned char *)Toi;
	RealSize = Toi[0];
#else // !APPMEM_MUNG_CHECK, usually release builds
#ifdef APPMEM_USE_MALLOC
	RealSize = _msize(Me);
#else // !APPMEM_USE_MALLOC
	RealSize = GlobalSize(Me);
#endif // !APPMEM_USE_MALLOC
#endif // APPMEM_MUNG_CHECK
	} // if

return(RealSize);

} // AppMem_QueryRealSize()

/*===========================================================================*/

size_t AppMem_QueryAllocatedSize(void *Me)
{
size_t RealSize = 0;
if (Me)
	{
#ifdef APPMEM_USE_MALLOC
	RealSize = _msize(Me);
#else // !APPMEM_USE_MALLOC
	RealSize = GlobalSize(Me);
#endif // !APPMEM_USE_MALLOC
	} // if

return(RealSize);

} // AppMem_QueryAllocatedSize()

/*===========================================================================*/

void AppMem_Free(void *Me, size_t DummyVal)
{
#ifdef APPMEM_MUNG_CHECK
size_t BlockSize;
#elif APPMEM_TRACK_USAGE
size_t BlockSize;
#endif
#ifdef APPMEM_MUNG_CHECK
size_t RealSize;
unsigned long *Toi;
unsigned char *Vous;
char PreTrash = 1, PostTrash = 1;
#ifdef APPMEM_TRACK_LOCATION
unsigned long *DeLinkNext, *DeLinkPrev;
#endif // APPMEM_TRACK_LOCATION
#endif // APPMEM_MUNG_CHECK

if (Me)
	{

#ifdef APPMEM_MUNG_CHECK
	Toi = (unsigned long *)Me;
	Toi -= 3; // back up in RAM
	Vous = (unsigned char *)Toi;
	RealSize = Toi[0];

	BlockSize = RealSize;
	if (DummyVal && (DummyVal != RealSize))
		{
		sprintf(OutLine, "Attempt to AppMem_Free with incorrect size.\nAllocated as %d, freed as %d.", RealSize, DummyVal);
		UserMessageOK("AppMem Debug", OutLine);
		} // if
	//else
		{
		#ifdef APPMEM_TRACK_LOCATION
		DeLinkNext = (unsigned long *)Toi[1];
		DeLinkPrev = (unsigned long *)Toi[2];
		if (((unsigned long *)AppMem_CurMemBlock) == Toi)
			{
			AppMem_CurMemBlock = (void *)DeLinkNext;
			} // if
		if (DeLinkNext)
			{
			DeLinkNext[2] = (unsigned long int)DeLinkPrev;
			} // if
		if (DeLinkPrev)
			{
			DeLinkPrev[1] = (unsigned long int)DeLinkNext;
			} // if
		#else // !APPMEM_TRACK_LOCATION

		if (memcmp(&Vous[4], &Vous[12 + RealSize], 8))
			{
			if (!memcmp(&Vous[4], &Vous[8], 4))
				{
				PreTrash = 0;
				} // if
			if (!memcmp(&Vous[12 + RealSize], &Vous[16 + RealSize], 4))
				{
				PostTrash = 0;
				} // if
			sprintf(OutLine, "AppMem_Free detected corruption.\n");
			if (PreTrash)
				{
				sprintf(OutComp, "PreBlock Wall=%x%x%x%x %x%x%x%x.\n",
				 Vous[4], Vous[5], Vous[6], Vous[7],
				 Vous[8], Vous[9], Vous[10], Vous[11]);
				strcat(OutLine, OutComp);
				} // if
			else
				{
				sprintf(OutComp, "PreBlock Intact=%x%x%x%x.\n",
				 Vous[4], Vous[5], Vous[6], Vous[7]);
				strcat(OutLine, OutComp);
				} // else
			if (PostTrash)
				{
				sprintf(OutComp, "PostBlock Wall=%x%x%x%x %x%x%x%x.\n",
				 Vous[8 + RealSize], Vous[9 + RealSize], Vous[10 + RealSize], Vous[11 + RealSize],
				 Vous[12 + RealSize], Vous[13 + RealSize], Vous[14 + RealSize], Vous[15 + RealSize]);
				strcat(OutLine, OutComp);
				} // if
			else
				{
				sprintf(OutComp, "PostBlock Intact=%x%x%x%x.\n",
				 Vous[8 + RealSize], Vous[9 + RealSize], Vous[10 + RealSize], Vous[11 + RealSize]);
				strcat(OutLine, OutComp);
				} // else
			UserMessageOK("AppMem Debug", OutLine);
			} // if
		#endif // !APPMEM_TRACK_LOCATION
		} // else (not any longer)
	
	BlockSize += APPMEM_MUNG_EXTRA;
	Me = (void *)Toi;
	
#endif // APPMEM_MUNG_CHECK

	#ifdef APPMEM_USE_MALLOC
	free(Me);
	#else // !APPMEM_USE_MALLOC
	
	#ifdef _WIN32
	//(void)GlobalFree(GlobalHandle(Me));
#ifdef _OPENMP
	HeapFree(AppMemHeap, NULL, Me);
#else // _OPENMP
	HeapFree(AppMemHeap, HEAP_NO_SERIALIZE, Me);
#endif // _OPENMP
	#endif // _WIN32
	
	#endif // !APPMEM_USE_MALLOC
	
	#ifdef APPMEM_TRACK_USAGE
	AppMem_MainHeapSize -= BlockSize;
	#endif // APPMEM_TRACK_USAGE
	} // if
else
	{
	#ifdef _DEBUG
	UserMessageOK("AppMem Debug", "Attempt to AppMem_Free a NULL pointer.\nIrrational. Ignoring.");
	//UserMessageCustom("AppMem Debug", "Attempt to AppMem_Free a NULL pointer.\nIrrational. Ignoring.",
	// "Yah", "Ignore", "Whatever", 2);
	#endif // _DEBUG
	} // else

} // AppMem_Free()

/*===========================================================================*/

void AppMem_ReportUsage(void)
{

#ifdef APPMEM_TRACK_USAGE
sprintf(OutLine, "\n[AppMem.cpp: Memory Usage Information]\n-------------------------------------\n");

DEBUGOUT(OutLine);

sprintf(OutLine, "\nSize  ");
DEBUGOUT(OutLine);
sprintf(OutLine, "%10lu  ", AppMem_MainHeapSize);
DEBUGOUT(OutLine);

sprintf(OutLine, "\nMax   ");
DEBUGOUT(OutLine);
sprintf(OutLine, "%10lu  ", AppMem_MainMaxSize);
DEBUGOUT(OutLine);

sprintf(OutLine, "\nUsage ");
DEBUGOUT(OutLine);
sprintf(OutLine, "%10lu  ", AppMem_MainUsage);
DEBUGOUT(OutLine);

sprintf(OutLine, "\n");
DEBUGOUT(OutLine);
#endif // APPMEM_TRACK_USAGE

} // AppMem_ReportUsage()

/*===========================================================================*/

unsigned long AppMem_ReportStatus(unsigned long ReqType)
{
#ifdef _WIN32
   MEMORYSTATUS MemInfo;
   
MemInfo.dwLength = sizeof(MemInfo);
GlobalMemoryStatus(&MemInfo);
#endif // _WIN32

// Ignore flags in this switch, we'll deal with them on a
// case by case basis.
switch (ReqType & 0x0F)
	{
	case WCS_APPMEM_REPORT_ALLVIRT:
		{
		#ifdef _WIN32
		return((unsigned long)MemInfo.dwTotalPageFile);
		#endif // _WIN32
		} // 
	case WCS_APPMEM_REPORT_FREEVIRT:
		{
		#ifdef _WIN32
		return((unsigned long)MemInfo.dwAvailPageFile);
		#endif // _WIN32
		} // 
	case WCS_APPMEM_REPORT_ALLPHYS:
		{
		#ifdef _WIN32
		return((unsigned long)MemInfo.dwTotalPhys);
		#endif // _WIN32
		} // 
	case WCS_APPMEM_REPORT_FREEPHYS:
		{
		#ifdef _WIN32
		return((unsigned long)MemInfo.dwAvailPhys);
		#endif // _WIN32
		} // 
	case WCS_APPMEM_REPORT_MAINHEAPSIZE:
		{
		#ifdef _DEBUG
		return(AppMem_MainHeapSize);
		#else // !_DEBUG
		return(0);
		#endif // !_DEBUG
		} // 
	case WCS_APPMEM_REPORT_MAINMAXSIZE:
		{
		#ifdef _DEBUG
		return(AppMem_MainMaxSize);
		#else // !_DEBUG
		return(0);
		#endif // !_DEBUG
		} // 
	case WCS_APPMEM_REPORT_MAINUSAGE:
		{
		#ifdef _DEBUG
		return(AppMem_MainUsage);
		#else // !_DEBUG
		return(0);
		#endif // !_DEBUG
		} // 
	default: // case WCS_APPMEM_REPORT_ALLFREE:
		{
		#ifdef _WIN32
		return((unsigned long)(MemInfo.dwAvailPhys + MemInfo.dwAvailPageFile));
		// return(MemInfo.dwAvailVirtual);
		#endif // _WIN32
		} // 
	} // switch ReqType

return(0);	//lint !e527

} // AppMem_ReportStatus

/*===========================================================================*/

void AppMem_ReportTrackLeftovers(void)
{
#ifdef APPMEM_TRACK_LOCATION
unsigned long RealSize, FileID, LineID, NextLink, *NL;
unsigned char *Vous;

for (NL = (unsigned long *)AppMem_CurMemBlock; NL; NL = (unsigned long *)NextLink)
	{
	Vous = (unsigned char *)NL;
	RealSize = NL[0];
	memcpy(&FileID, &Vous[RealSize + 12], 4);
	memcpy(&LineID, &Vous[RealSize + 16], 4);
	sprintf(MemTrackDebugStr, "0x%08X File=%s, Line=%d, Size=%d.\n", Vous, AppMem_FileNumTrack[FileID], LineID, RealSize);
	DEBUGOUT(MemTrackDebugStr);
	NextLink = NL[1];
	} // for
#endif // APPMEM_TRACK_LOCATION

} // AppMem_ReportTrackLeftovers

/*===========================================================================*/

#ifdef APPMEM_TRACK_LOCATION
AppMem_Tracker *AppMem_Track(const char *WhatFile, unsigned long WhatLine)
{
int FileFind, FoundIt = 0;

for (FileFind = 1; FileFind < APPMEM_TRACK_LOCATION_MAXFILES; ++FileFind)
	{
	if (AppMem_FileNumTrack[FileFind][0])
		{
		if (!stricmp(WhatFile, AppMem_FileNumTrack[FileFind]))
			{
			FoundIt = FileFind;
			break;
			} // if
		} // if
	else
		{
		strcpy(AppMem_FileNumTrack[FileFind], WhatFile);
		FoundIt = FileFind;
		break;
		} // else
	} // for
if (FoundIt)
	{
	CurrentLine = WhatLine;
	CurrentFileNum = FoundIt;
	} // if
else
	{
	CurrentLine = WhatLine;
	CurrentFileNum = 0;
	} // else

return(0); // Ooooh, voodooo

} // AppMem_Track
#endif // APPMEM_TRACK_LOCATION

/*===========================================================================*/

void *operator new(size_t Bytes)
{

return(AppMem_Alloc(Bytes, NULL));

} // operator new

/*===========================================================================*/

void operator delete(void *Mem)
{

if (Mem)
	AppMem_Free(Mem);

} // operator delete

/*===========================================================================*/

void AppMem_LogDEMSizes(int Width, int Height)
{

if (Width > WarnMaxDEMWidth) WarnMaxDEMWidth = Width;
if (Height > WarnMaxDEMHeight) WarnMaxDEMHeight = Height;

} // AppMem_LogDEMSizes

/*===========================================================================*/

void AppMem_ClearDEMSizes(void)
{

WarnMaxDEMWidth = 0;
WarnMaxDEMHeight = 0;

} // AppMem_ClearDEMSizes
