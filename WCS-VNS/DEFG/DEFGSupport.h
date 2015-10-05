// DEFGSupport.h
// support for running DEFG outside of WCS/VNS
// created from scratch and from DEFG.cpp on
// Fri Aug 24, 2001 by CXH


#include <stddef.h>
#include <stdio.h>

class DEFG;

#define APPMEM_CLEAR 						 1

void *AppMem_Alloc(size_t AllocSize, unsigned long int Flags, char *Topic = NULL, unsigned char Optional = 0);
void AppMem_Free(void *Me, size_t DummyVal = 0);
double GetSystemTimeFP(void);


int LoadPoints(char *InFile, DEFG *DG);


