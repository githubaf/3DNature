// AppMem.h
// Prototypes for AppMem.cpp
// Created from scratch on 4/2/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_APPMEM_H
#define WCS_APPMEM_H

// Enable this to make us use malloc/calloc for everythin on any platform
// Mostly used for debugging
//#define APPMEM_USE_MALLOC

// Enable this to track usage statistics
#define APPMEM_TRACK_USAGE

#include <stddef.h>
#ifdef AMIGA
#include <exec/memory.h>
#include <proto/exec.h>

#define APPMEM_CLEAR 						 MEMF_CLEAR
#define APPMEM_TEMP_CLEAR					 MEMF_CLEAR
#define APPMEM_GRAPHICS 					 MEMF_CHIP
#define APPMEM_TEMP_GRAPHICS	  MEMF_CHIP

#endif // AMIGA

#ifdef WIN32
#include <windows.h>

#define APPMEM_CLEAR 						 GMEM_ZEROINIT
#define APPMEM_TEMP_CLEAR					 LMEM_ZEROINIT
// Amiga implementation:
// Do not allocate graphics mem with AllocTemp()
// #define APPMEM_GRAPHICS 							NULL
#define APPMEM_GRAPHICS 					 NULL
#define APPMEM_TEMP_GRAPHICS	  NULL

#endif // WIN32


void *App_AllocTemp(size_t AllocSize, unsigned long int Flags);
void *App_Alloc(size_t AllocSize, unsigned long int Flags);
void App_FreeTemp(void *Me, size_t BlockSize);
void App_Free(void *Me, size_t BlockSize);
void App_ReportUsage(void);

#endif // WCS_APPMEM_H
