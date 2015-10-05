// AppMem.h
// Prototypes for AppMem.cpp
// Created from scratch on 4/2/95 by Chris "Xenon" Hanson
// Copyright 1995

#include "stdafx.h"

#ifndef WCS_APPMEM_H
#define WCS_APPMEM_H

// Enable this to make us use malloc/calloc for everything on any platform
// Mostly used for debugging
//#define APPMEM_USE_MALLOC

// Enable this to track usage statistics
#ifdef _DEBUG
#define APPMEM_TRACK_LOCATION
class AppMem_Tracker
	{
	public:
		void *AppMem_AllocReal(size_t AllocSize, unsigned long Flags, char *Topic = NULL, unsigned char Optional = 0);
	};
#endif // _DEBUG

#ifdef APPMEM_TRACK_LOCATION
// Enable this to track usage statistics
// Only enable if you have APPMEM_TRACK_LOCATION on...
#define APPMEM_TRACK_USAGE
#endif // APPMEM_TRACK_LOCATION

#define APPMEM_ANY	0

#ifdef _WIN32
//#define APPMEM_CLEAR 						 GMEM_ZEROINIT
#define APPMEM_CLEAR 						 HEAP_ZERO_MEMORY
#endif // _WIN32


// One of these tokens is passed to AppMem_ReportStatus to ask about
// current system memory conditions.
enum
	{
	WCS_APPMEM_REPORT_ALLFREE,
	WCS_APPMEM_REPORT_ALLVIRT,
	WCS_APPMEM_REPORT_FREEVIRT,
	WCS_APPMEM_REPORT_ALLPHYS,
	WCS_APPMEM_REPORT_FREEPHYS,
	WCS_APPMEM_REPORT_MAINHEAPSIZE, // only in debug builds
	WCS_APPMEM_REPORT_MAINMAXSIZE, // only in debug builds
	WCS_APPMEM_REPORT_MAINUSAGE // only in debug builds
	}; // ReportStatus

unsigned long AppMem_Prep(void);
#ifdef APPMEM_TRACK_LOCATION
AppMem_Tracker *AppMem_Track(const char *WhatFile, unsigned long WhatLine);
// AppMem_Track(__FILE__, __LINE__)
#define AppMem_Alloc AppMem_Track(__FILE__, __LINE__)->AppMem_AllocReal
#else // !APPMEM_TRACK_LOCATION
void *AppMem_Alloc(size_t AllocSize, unsigned long Flags, char *Topic = NULL, unsigned char Optional = 0);
#endif // !APPMEM_TRACK_LOCATION
void AppMem_Free(void *Me, size_t DummyVal = 0);
size_t AppMem_QueryAllocatedSize(void *Me);
size_t AppMem_QueryRealSize(void *Me);
void AppMem_ReportUsage(void);
void AppMem_ReportTrackLeftovers(void);
unsigned long AppMem_ReportStatus(unsigned long ReqType = WCS_APPMEM_REPORT_ALLFREE);

void AppMem_LogDEMSizes(int Width, int Height);
void AppMem_ClearDEMSizes(void);

void AppMem_Cleanup(void);

void *operator new(size_t);
void operator delete(void *);
#endif // WCS_APPMEM_H
