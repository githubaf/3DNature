// AppMemStub.h
// Stub to thunk AppMem calls down to C standard library calls when outside of WCS/VNS

#include "stdafx.h"

#ifndef WCS_APPMEM_H
#define WCS_APPMEM_H

// we define WCS_APPMEM_H to prevent it from taking over in case it's also included by accident

#define APPMEM_CLEAR 						 0

void *AppMem_Alloc(size_t AllocSize, unsigned long Flags, char *Topic = NULL, unsigned char Optional = 0) {return(calloc(1, AllocSize));};
void AppMem_Free(void *Me, size_t DummyVal = 0) {free(Me);};

#endif // WCS_APPMEM_H
