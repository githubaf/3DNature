// UsefulIO.cpp
// IO-related code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulIO.h"
#include "UsefulEndian.h"

static size_t fileResult;

/*===========================================================================*/

// Hack for now
size_t GetFileResult(void)
{

return fileResult;

} // GetFileResult

/*===========================================================================*/

// byte order smart file reading routines
double Get64(char byteorder, FILE *fp)
{
double val;

fileResult = fread((void *)&val, 8, 1, fp);
if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip64(&val, &val);
	#endif
	} // if
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip64(&val, &val);
	#endif
	} // else

return val;

} // Get64

/*===========================================================================*/

float Get32F(char byteorder, FILE *fp)
{
float val;

fileResult = fread((void *)&val, 4, 1, fp);
if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32F(&val, &val);
	#endif
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip32F(&val, &val);
	#endif
	}

return val;

} // Get32F

/*===========================================================================*/

unsigned long Get32U(char byteorder, FILE *fp)
{
unsigned long val;

#ifdef BYTEORDER_LITTLEENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (unsigned long)(fgetc(fp) & 0xFF);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (unsigned long)(fgetc(fp) & 0xFF);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x08);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x10);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x18);
	}
#else // BYTEORDER_BIGENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (unsigned long)(fgetc(fp) & 0xFF);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((unsigned long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (unsigned long)(fgetc(fp) & 0xFF);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x08);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x10);
	val |= ((unsigned long)(fgetc(fp) & 0xFF) << 0x18);
	}
#endif

return val;

} // Get32U

/*===========================================================================*/

long Get32S(char byteorder, FILE *fp)
{
long val;

#ifdef BYTEORDER_LITTLEENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (long)(fgetc(fp) & 0xFF);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (long)(fgetc(fp) & 0xFF);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x08);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x10);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x18);
	}
#else // BYTEORDER_BIGENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (long)(fgetc(fp) & 0xFF);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	val = ((long)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (long)(fgetc(fp) & 0xFF);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x08);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x10);
	val |= ((long)(fgetc(fp) & 0xFF) << 0x18);
	}
#endif

return val;

} // Get32S

/*===========================================================================*/

unsigned short Get16U(char byteorder, FILE *fp)
{
unsigned short val;

#ifdef BYTEORDER_LITTLEENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (unsigned short)(fgetc(fp) & 0xFF);
	val = ((unsigned short)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (unsigned short)(fgetc(fp) & 0xFF);
	val |= ((unsigned short)(fgetc(fp) & 0xFF) << 0x08);
	}
#else // BYTEORDER_BIGENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (unsigned short)(fgetc(fp) & 0xFF);
	val = ((unsigned short)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (unsigned short)(fgetc(fp) & 0xFF);
	val |= ((unsigned short)(fgetc(fp) & 0xFF) << 0x08);
	}
#endif

return val;

} // Get16U

/*===========================================================================*/

short Get16S(char byteorder, FILE *fp)
{
short val;

#ifdef BYTEORDER_LITTLEENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (short)(fgetc(fp) & 0xFF);
	val = ((short)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (short)(fgetc(fp) & 0xFF);
	val |= ((short)(fgetc(fp) & 0xFF) << 0x08);
	}
#else // BYTEORDER_BIGENDIAN
if (byteorder == BIG_END_DATA)
	{
	val = (short)(fgetc(fp) & 0xFF);
	val = ((short)(fgetc(fp) & 0xFF)) | (val << 0x08);
	}
else	// LITTLE_END_DATA
	{
	val = (short)(fgetc(fp) & 0xFF);
	val |= ((short)(fgetc(fp) & 0xFF) << 0x08);
	}
#endif

return val;

} // Get16S

/*===========================================================================*/

// read BigEndian data
double  GetB64(FILE *fp)	{return Get64(BIG_END_DATA, fp);};
float	GetB32F(FILE *fp)	{return Get32F(BIG_END_DATA, fp);};
unsigned long	GetB32U(FILE *fp)	{return	Get32U(BIG_END_DATA, fp);};
long	GetB32S(FILE *fp)	{return	Get32S(BIG_END_DATA, fp);};
unsigned short	GetB16U(FILE *fp)	{return Get16U(BIG_END_DATA, fp);};
short	GetB16S(FILE *fp)	{return	Get16S(BIG_END_DATA, fp);};

// read LittleEndian data
double	GetL64(FILE *fp)	{return Get64(LITTLE_END_DATA, fp);};
float	GetL32F(FILE *fp)	{return Get32F(LITTLE_END_DATA, fp);};
unsigned long	GetL32U(FILE *fp)	{return Get32U(LITTLE_END_DATA, fp);};
long	GetL32S(FILE *fp)	{return Get32S(LITTLE_END_DATA, fp);};
unsigned short	GetL16U(FILE *fp)	{return Get16U(LITTLE_END_DATA, fp);};
short	GetL16S(FILE *fp)	{return Get16S(LITTLE_END_DATA, fp);};

/*===========================================================================*/

// byte order smart file writing routines {0 = no error}
int Put64(char byteorder, double *dval, FILE *fp)
{
char buf[8];

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	BlindSimpleEndianFlip64(dval, buf);
	#else
	memcpy((void *)buf, (const void *)dval, 8);
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	BlindSimpleEndianFlip64(dval, buf);
	#else
	memcpy((void *)buf, (const void *)dval, 8);
	#endif // BYTEORDER_BIGENDIAN
	}

fwrite((void *)buf, 1, 8, fp);

return ferror(fp);

} // Put64

/*===========================================================================*/

int Put32F(char byteorder, float *fval, FILE *fp)
{
char buf[4];

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	BlindSimpleEndianFlip32F(fval, buf);
	#else
	memcpy((void *)buf, (const void *)fval, 4);
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	BlindSimpleEndianFlip32F(fval, buf);
	#else
	memcpy((void *)buf, (const void *)fval, 4);
	#endif // BYTEORDER_BIGENDIAN
	}
fwrite((void *)buf, 1, 4, fp);

return ferror(fp);

} // Put32F

/*===========================================================================*/

int Put32U(char byteorder, unsigned long ulval, FILE *fp)
{
unsigned long realval;

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32U(ulval, &realval);
	#else
	realval = ulval;
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip32U(ulval, &realval);
	#else
	realval = ulval;
	#endif // BYTEORDER_BIGENDIAN
	}
fwrite((void *)&realval, 1, 4, fp);

return ferror(fp);

} // Put32U

/*===========================================================================*/

int Put32S(char byteorder, long lval, FILE *fp)
{
long realval;

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip32S(lval, &realval);
	#else
	realval = lval;
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip32S(lval, &realval);
	#else
	realval = lval;
	#endif // BYTEORDER_BIGENDIAN
	}
fwrite((void *)&realval, 1, 4, fp);

return ferror(fp);

} // Put32S

/*===========================================================================*/

int Put16U(char byteorder, unsigned short usval, FILE *fp)
{
unsigned short realval;

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16U(usval, &realval);
	#else
	realval = usval;
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip16U(usval, &realval);
	#else
	realval = usval;
	#endif // BYTEORDER_BIGENDIAN
	}
fwrite((void *)&realval, 1, 2, fp);

return ferror(fp);

} // Put16U

/*===========================================================================*/

int Put16S(char byteorder, short sval, FILE *fp)
{
short realval;

if (byteorder == BIG_END_DATA)
	{
	#ifdef BYTEORDER_LITTLEENDIAN
	SimpleEndianFlip16S(sval, &realval);
	#else
	realval = sval;
	#endif // BYTEORDER_LITTLEENDIAN
	}
else	// LITTLE_END_DATA
	{
	#ifdef BYTEORDER_BIGENDIAN
	SimpleEndianFlip16S(sval, &realval);
	#else
	realval = sval;
	#endif // BYTEORDER_BIGENDIAN
	}
fwrite((void *)&realval, 1, 2, fp);

return ferror(fp);

} // Put16S

// write data to file in BigEndian format
int PutB64(double *dval, FILE *fp)			{return Put64(BIG_END_DATA, dval, fp);};
int	PutB32F(float *fval, FILE *fp)			{return Put32F(BIG_END_DATA, fval, fp);};
int	PutB32U(unsigned long ulval, FILE *fp)	{return	Put32U(BIG_END_DATA, ulval, fp);};
int	PutB32S(long lval, FILE *fp)			{return	Put32S(BIG_END_DATA, lval, fp);};
int	PutB16U(unsigned short usval, FILE *fp)	{return Put16U(BIG_END_DATA, usval, fp);};
int	PutB16S(short sval, FILE *fp)			{return	Put16S(BIG_END_DATA, sval, fp);};

// write data to file in LittleEndian format
int	PutL64(double *dval, FILE *fp)			{return Put64(LITTLE_END_DATA, dval, fp);};
int	PutL32F(float *fval, FILE *fp)			{return Put32F(LITTLE_END_DATA, fval, fp);};
int	PutL32U(unsigned long ulval, FILE *fp)	{return Put32U(LITTLE_END_DATA, ulval, fp);};
int	PutL32S(long lval, FILE *fp)			{return Put32S(LITTLE_END_DATA, lval, fp);};
int	PutL16U(unsigned short usval, FILE *fp)	{return Put16U(LITTLE_END_DATA, usval, fp);};
int	PutL16S(short sval, FILE *fp)			{return Put16S(LITTLE_END_DATA, sval, fp);};

/*===========================================================================*/

// code below adapted from http://czyborra.com/utf/
// see http://en.wikipedia.org/wiki/Utf-8#Description for description of algorithm
void UTF8Encode(FILE *out, const char *string)
{
unsigned int c;
unsigned long i = 0;
unsigned char ch;

ch = string[i];
c = ch;
while (ch != 0)
	{
	if (c < 0x80)
		{
		fputc(c, out);
		} // if
	else if (c < 0x800)
		{
		fputc(0xC0 | (c >> 6), out);
		fputc(0x80 | (c & 0x3F), out);
		} // else if
	else if (c < 0x10000)
		{
		fputc(0xE0 | (c >> 12), out);
		fputc(0x80 | ((c >> 6) & 0x3F), out);
		fputc(0x80 | (c & 0x3F), out);
		} // else if
	else if (c < 0x200000)
		{
		fputc(0xF0 | (c >> 18), out);
		fputc(0x80 | ((c >> 12) & 0x3F), out);
		fputc(0x80 | ((c >> 6) & 0x3F), out);
		fputc(0x80 | (c & 0x3F), out);
		} // else if
	i++;
	ch = string[i];
	c = ch;
	} // while

} // UTF8Encode

/*===========================================================================*/

unsigned long ReadBlock(FILE *ffile, char *Block, unsigned long Flags, short ByteFlip)
{
double Dbl;
unsigned long SizeRead, SizeBlock;
unsigned long Lng;
float Flt;
unsigned short Sht;

SizeBlock = Flags & 0x00ff;

SizeRead = SizeBlock * fread(Block, SizeBlock, 1, ffile);

// byte swapping
if (ByteFlip)
	{
	if (SizeRead)
		{
		switch (Flags & 0xff00)
			{
			case WCS_BLOCKTYPE_SHORTINT:
				{
				memcpy(&Sht, Block, 2);
				SimpleEndianFlip16U((unsigned short)Sht, (unsigned short *)&Sht);
				memcpy(Block, &Sht, 2);
				break;
				}
			case WCS_BLOCKTYPE_LONGINT:
				{
				memcpy(&Lng, Block, 4);
				SimpleEndianFlip32U((unsigned long)Lng, (unsigned long *)&Lng);
				memcpy(Block, &Lng, 4);
				break;
				}
			case WCS_BLOCKTYPE_FLOAT:
				{
				memcpy(&Flt, Block, 4);
				SimpleEndianFlip32F((void *)&Flt, (float *)&Flt);
				memcpy(Block, &Flt, 4);
				break;
				}
			case WCS_BLOCKTYPE_DOUBLE:
				{
				memcpy(&Dbl, Block, 8);
				SimpleEndianFlip64((void *)&Dbl, (double *)&Dbl);
				memcpy(Block, &Dbl, 8);
				break;
				}
			default:
				break;
			} // switch
		} // if something read
	} // if ByteFlip

return (SizeRead);

} // ReadBlock

/*===========================================================================*/

unsigned long ReadLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock)
{

return(SizeBlock * fread(Block, SizeBlock, 1, ffile));

} // ReadLongBlock

/*===========================================================================*/

unsigned long WriteBlock(FILE *ffile, char *Block, unsigned long Flags)
{
//unsigned long SizeWritten, SizeBlock;
unsigned long SizeBlock;
//char CopyBlock[256];
//unsigned short Sht;
//unsigned long Lng;
//float Flt;
//double Dbl;

SizeBlock = Flags & 0x00ff;

// byte swapping 
/*
 if (SizeBlock)
  {
  memcpy(CopyBlock, Block, SizeBlock);
#ifdef BYTEORDER_LITTLEENDIAN
  switch (Flags & 0xff00)
   {
   case WCS_BLOCKTYPE_SHORTINT:
    {
	memcpy(&Sht, CopyBlock, 2);
    SimpleEndianFlip16U((unsigned short)Sht, (unsigned short *)&Sht);
	memcpy(CopyBlock, &Sht, 2);
    break;
    }
   case WCS_BLOCKTYPE_LONGINT:
    {
	memcpy(&Lng, CopyBlock, 4);
    SimpleEndianFlip32U((unsigned long)Lng, (unsigned long *)&Lng);
	memcpy(CopyBlock, &Lng, 4);
    break;
    }
   case WCS_BLOCKTYPE_FLOAT:
    {
	memcpy(&Flt, CopyBlock, 4);
    SimpleEndianFlip32F((void *)&Flt, (float *)&Flt);
	memcpy(CopyBlock, &Flt, 4);
    break;
    }
   case WCS_BLOCKTYPE_DOUBLE:
    {
	memcpy(&Dbl, CopyBlock, 8);
    SimpleEndianFlip64((void *)&Dbl, (double *)&Dbl);
	memcpy(CopyBlock, &Dbl, 8);
    break;
    }
   } // switch 
#endif // BYTEORDER_LITTLEENDIAN  
  }// if something to write 
*/


// SizeWritten = SizeBlock * fwrite(CopyBlock, SizeBlock, 1, ffile);	// for byte-swapped data
return (SizeBlock * fwrite(Block, SizeBlock, 1, ffile));

// return (SizeWritten);

} // WriteBlock

/*===========================================================================*/

unsigned long WriteLongBlock(FILE *ffile, char *Block, unsigned long SizeBlock)
{

return (SizeBlock * fwrite(Block, SizeBlock, 1, ffile));

} // WriteLongBlock

/*===========================================================================*/

unsigned long PrepWriteBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
	unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr)
{
unsigned long LongSize, BytesWritten, TotalWritten = 0;
unsigned short ShortSize;
unsigned char CharSize;

ItemTag += SizeSize + SizeType;

if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG)) == NULL)
	{
	goto EndSave;
	} // if
TotalWritten += BytesWritten;

switch (SizeSize)
	{
	case WCS_BLOCKSIZE_CHAR:
		{
		CharSize = (unsigned char)FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&CharSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	case WCS_BLOCKSIZE_SHORT:
		{
		ShortSize = (unsigned short)FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&ShortSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	case WCS_BLOCKSIZE_LONG:
		{
		LongSize = FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&LongSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	} // switch
TotalWritten += BytesWritten;

if ((BytesWritten = WriteBlock(ffile, (char *)FieldAddr,
	FieldSize + FieldType)) == NULL)
	{
	goto EndSave;
	} // if
TotalWritten += BytesWritten;

return (TotalWritten);

EndSave:

return (0L);

} // PrepWriteBlock

/*===========================================================================*/

unsigned long PrepWriteLongBlock(FILE *ffile, unsigned long ItemTag, unsigned long SizeSize,
	unsigned long SizeType, unsigned long FieldSize, unsigned long FieldType, char *FieldAddr)
{
unsigned long LongSize, BytesWritten, TotalWritten = 0;
unsigned short ShortSize;
unsigned char CharSize;

ItemTag += SizeSize + SizeType;

if ((BytesWritten = WriteBlock(ffile, (char *)&ItemTag,
	WCS_BLOCKTYPE_LONGINT + WCS_BLOCKSIZE_LONG)) == NULL)
	{
	goto EndSave;
	} // if
TotalWritten += BytesWritten;

switch (SizeSize)
	{
	case WCS_BLOCKSIZE_CHAR:
		{
		CharSize = (unsigned char)FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&CharSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	case WCS_BLOCKSIZE_SHORT:
		{
		ShortSize = (unsigned short)FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&ShortSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	case WCS_BLOCKSIZE_LONG:
		{
		LongSize = FieldSize;
		if ((BytesWritten = WriteBlock(ffile, (char *)&LongSize,
			SizeSize + SizeType)) == NULL)
			{
			goto EndSave;
			} // if
		break;
		}
	} // switch
TotalWritten += BytesWritten;

if ((BytesWritten = WriteLongBlock(ffile, (char *)FieldAddr, FieldSize)) == NULL)
	{
	goto EndSave;
	} // if
TotalWritten += BytesWritten;

return (TotalWritten);

EndSave:
return (0L);

} // PrepWriteLongBlock
