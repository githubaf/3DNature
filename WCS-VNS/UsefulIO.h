// UsefulIO.h
// IO-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#include "stdafx.h"

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_USEFULIO_H
#define WCS_USEFULIO_H

#define BIG_END_DATA	1
#define LITTLE_END_DATA	2

size_t GetFileResult(void);	// hack for now

// byte order smart file reading routines
double			Get64(char byteorder, FILE *fp);
float			Get32F(char byteorder, FILE *fp);
unsigned long	Get32U(char byteorder, FILE *fp);
long			Get32S(char byteorder, FILE *fp);
unsigned short	Get16U(char byteorder, FILE *fp);
short			Get16S(char byteorder, FILE *fp);
inline unsigned char	Get8U(char byteorder, FILE *fp) {return((unsigned char)fgetc(fp));}; // these are basically dummies to make the API pretty
inline char			Get8S(char byteorder, FILE *fp) {return((char)fgetc(fp));}; // these are basically dummies to make the API pretty

// read BigEndian ordered data
double			GetB64(FILE *fp);
float			GetB32F(FILE *fp);
unsigned long	GetB32U(FILE *fp);
long			GetB32S(FILE *fp);
unsigned short	GetB16U(FILE *fp);
short			GetB16S(FILE *fp);
inline unsigned char	GetB8U(FILE *fp) {return Get8U(BIG_END_DATA, fp);}; // these are basically dummies to make the API pretty
inline char			GetB8S(FILE *fp) {return Get8S(BIG_END_DATA, fp);}; // these are basically dummies to make the API pretty

// read LittleEndian ordered data
double			GetL64(FILE *fp);
float			GetL32F(FILE *fp);
unsigned long	GetL32U(FILE *fp);
long			GetL32S(FILE *fp);
unsigned short	GetL16U(FILE *fp);
short			GetL16S(FILE *fp);
inline unsigned char	GetL8U(FILE *fp) {return Get8U(LITTLE_END_DATA, fp);}; // these are basically dummies to make the API pretty
inline char			GetL8S(FILE *fp) {return Get8S(LITTLE_END_DATA, fp);}; // these are basically dummies to make the API pretty

// byte order smart file writing routines {0 = no error}
int	Put64(char byteorder, double *dval, FILE *fp);
int	Put32F(char byteorder, float *fval, FILE *fp);
int Put32U(char byteorder, unsigned long ulval, FILE *fp);
int	Put32S(char byteorder, long lval, FILE *fp);
int	Put16U(char byteorder, unsigned short usval, FILE *fp);
int	Put16S(char byteorder, short sval, FILE *fp);
inline int	Put8S(char byteorder, char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty
inline int	Put8U(char byteorder, unsigned char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty

// write data to file in BigEndian format
int PutB64(double *dval, FILE *fp);
int	PutB32F(float *fval, FILE *fp);
int	PutB32U(unsigned long ulval, FILE *fp);
int	PutB32S(long lval,  FILE *fp);
int	PutB16U(unsigned short usval, FILE *fp);
int	PutB16S(short sval, FILE *fp);
inline int	PutB8S(char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty
inline int	PutB8U(unsigned char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty

// write data to file in LittleEndian format
int	PutL64(double *dval, FILE *fp);
int	PutL32F(float *fval, FILE *fp);
int	PutL32U(unsigned long ulval, FILE *fp);
int	PutL32S(long lval, FILE *fp);
int	PutL16U(unsigned short usval, FILE *fp);
int	PutL16S(short sval, FILE *fp);
inline int	PutL8S(char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty
inline int	PutL8U(unsigned char cval, FILE *fp) {fputc(cval, fp); return ferror(fp);}; // these are basically dummies to make the API pretty

void UTF8Encode(FILE *out, const char *string);

// these are used generically for reading and writing to V2 files

/* general block size and type descriptors */

/* variable sizes */
#define WCS_BLOCKSIZE_CHAR		0x01
#define WCS_BLOCKSIZE_SHORT		0x02
#define WCS_BLOCKSIZE_LONG		0x04
#define WCS_BLOCKSIZE_DOUBLE	0x08
/* variable types */
#define WCS_BLOCKTYPE_CHAR		0x0100
#define WCS_BLOCKTYPE_SHORTINT	0x0200
#define WCS_BLOCKTYPE_LONGINT	0x0300
#define WCS_BLOCKTYPE_FLOAT		0x0400
#define WCS_BLOCKTYPE_DOUBLE	0x0500

unsigned long ReadBlock(FILE *ffile, char *Block, unsigned long Flags, short ByteFlip);
unsigned long WriteBlock(FILE *ffile, char *Block, unsigned long Flags);
unsigned long PrepWriteBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
		unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr);
unsigned long ReadLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock);
unsigned long WriteLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock);
unsigned long PrepWriteLongBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
		unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr);

#endif // WCS_USEFULIO_H
