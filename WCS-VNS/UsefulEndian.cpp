// UsefulEndian.cpp
// Endian-related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulEndian.h"

/*===========================================================================*/

void BlindSimpleEndianFlip64 (void *Source64, void *Dest64)
{
unsigned long i, j;
char *Src = (char *)Source64;
char *Dst = (char *)Dest64;
char buf[8];

for (i = 0; i < 8; i++)
	buf[i] = Src[i];

for (i = 0, j = 7; i < 8; i++, j--)
	Dst[i] = buf[j];

} // BlindSimpleEndianFlip64

/*===========================================================================*/

void BlindSimpleEndianFlip32F(void *Source32, void *Dest32)
{
unsigned long i, j;
char *Src = (char *)Source32;
char *Dst = (char *)Dest32;
char buf[4];

for (i = 0; i < 4; i++)
	buf[i] = Src[i];

for (i = 0, j = 3; i < 4; i++, j--)
	Dst[i] = buf[j];

} // BlindSimpleEndianFlip32F

/*===========================================================================*/

/***
void SimpleEndianFlip64(double Source64, double *Dest64)
{
char *SwapBuf, *SrcBuf;

SrcBuf = (char *)&Source64;
SwapBuf = (char *)Dest64; // Trusssssssst me...
SwapBuf[0] = SrcBuf[7]; SwapBuf[7] = SrcBuf[0];
SwapBuf[1] = SrcBuf[6]; SwapBuf[6] = SrcBuf[1];
SwapBuf[2] = SrcBuf[5]; SwapBuf[5] = SrcBuf[2];
SwapBuf[3] = SrcBuf[4]; SwapBuf[4] = SrcBuf[3];

} // SimpleEndianFlip64

void SimpleEndianFlip32F(float Source32, float *Dest32)
{
char *SwapBuf, *SrcBuf;

SrcBuf = (char *)&Source32;
SwapBuf = (char *)Dest32; // Trusssssssst me...
SwapBuf[0] = SrcBuf[3]; SwapBuf[3] = SrcBuf[0];
SwapBuf[1] = SrcBuf[2]; SwapBuf[2] = SrcBuf[1];
} // SimpleEndianFlip32F
***/

/*===========================================================================*/

void SimpleEndianFlip64(void *Source64, double *Dest64)
{
char *SwapBuf, *SrcBuf, tmp;

SrcBuf = (char *)Source64;
SwapBuf = (char *)Dest64; // Trusssssssst me...
tmp = SrcBuf[0]; SwapBuf[0] = SrcBuf[7]; SwapBuf[7] = tmp;
tmp = SrcBuf[1]; SwapBuf[1] = SrcBuf[6]; SwapBuf[6] = tmp;
tmp = SrcBuf[2]; SwapBuf[2] = SrcBuf[5]; SwapBuf[5] = tmp;
tmp = SrcBuf[3]; SwapBuf[3] = SrcBuf[4]; SwapBuf[4] = tmp;

} // SimpleEndianFlip64

/*===========================================================================*/

void SimpleEndianFlip32F(void *Source32, float *Dest32)
{
char *SwapBuf, *SrcBuf, tmp;

SrcBuf = (char *)Source32;
SwapBuf = (char *)Dest32; // Trusssssssst me...
tmp = SrcBuf[0]; SwapBuf[0] = SrcBuf[3]; SwapBuf[3] = tmp;
tmp = SrcBuf[1]; SwapBuf[1] = SrcBuf[2]; SwapBuf[2] = tmp;

} // SimpleEndianFlip32F

/*===========================================================================*/

void SimpleEndianFlip32U(unsigned long int Source32, unsigned long int *Dest32)
{
char *SwapBuf, *SrcBuf;

SrcBuf = (char *)&Source32;
SwapBuf = (char *)Dest32; // Trusssssssst me...
SwapBuf[0] = SrcBuf[3]; SwapBuf[3] = SrcBuf[0];
SwapBuf[1] = SrcBuf[2]; SwapBuf[2] = SrcBuf[1];

} // SimpleEndianFlip32U

/*===========================================================================*/

void SimpleEndianFlip32S(signed long int Source32, signed long int *Dest32)
{
char *SwapBuf, *SrcBuf;

SrcBuf = (char *)&Source32;
SwapBuf = (char *)Dest32; // Trusssssssst me...
SwapBuf[0] = SrcBuf[3]; SwapBuf[3] = SrcBuf[0];
SwapBuf[1] = SrcBuf[2]; SwapBuf[2] = SrcBuf[1];

} // SimpleEndianFlip32S
