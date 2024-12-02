/*
 * BigEndianReadWrite.h
 *
 *  Created on: Mar 22, 2023
 *      Author: Alexander Fritsch, selco, HGW
 *
 *      Read/write basic datatypes and special structures from/into big endian format
 */

#ifndef BIGENDIANREADWRITE_H_
#define BIGENDIANREADWRITE_H_

#include "WCS.h"

// ########################################################################################################################
// swap primitives
// ########################################################################################################################

// AF, 12.Dec.22
#if defined  __AROS__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   #define ENDIAN_CHANGE_IF_NEEDED(x) x
#else
   #define ENDIAN_CHANGE_IF_NEEDED(x)
#endif

#if defined  __AROS__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline void SimpleEndianFlip64 (double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
{
	double retVal;
	char *doubleToConvert = ( char* ) & Source64;
	char *returnDouble = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnDouble[0] = doubleToConvert[7];
	returnDouble[1] = doubleToConvert[6];
	returnDouble[2] = doubleToConvert[5];
	returnDouble[3] = doubleToConvert[4];
	returnDouble[4] = doubleToConvert[3];
	returnDouble[5] = doubleToConvert[2];
	returnDouble[6] = doubleToConvert[1];
	returnDouble[7] = doubleToConvert[0];

	*Dest64=retVal;

}

inline void SimpleEndianFlip32F(float Source32, float  *Dest32)  // AF, 10Dec22 for i386-aros
{
	float retVal;
	char *floatToConvert = ( char* ) & Source32;
	char *returnFloat = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];

	*Dest32=retVal;
}
inline void SimpleEndianFlip32U( ULONG Source32, ULONG *Dest32)  // AF, 10Dec22 for i386-aros
{
	(*Dest32) = (ULONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			(ULONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}


inline void SimpleEndianFlip32S( LONG Source32, LONG *Dest32)  //AF, 10Dec22 for i386-aros
{
	(*Dest32) = ( LONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			( LONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}

inline void SimpleEndianFlip16U(USHORT Source16, USHORT *Dest16) {(*Dest16) = (USHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
inline void SimpleEndianFlip16S(SHORT Source16, SHORT *Dest16) {(*Dest16) = ( SHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
#endif

// ########################################################################################################################
// Write functions
// ########################################################################################################################

// basic types types Upper/lowercase should match really used type, e.g LONG vs long!

// AF: 9.Jan23, Write short in Big-Endian (i.e. native Amiga-) format
int fwrite_short_BE(const short *Value, FILE *file);

// AF, 17.12.2022
ssize_t write_UShort_BigEndian (int filedes, const void *buffer, size_t size);

// AF: 16.Feb23, Write LONG in Big-Endian (i.e. native Amiga-) format
int fwrite_LONG_BE(const LONG *Value, FILE *file);

// AF: 22.Mar23, Write float in Big-Endian (i.e. native Amiga-) format
int fwrite_float_BE(const float *Value, FILE *file);

// AF: 21.Mar23, Write float in Big-Endian (i.e. native Amiga-) format
int write_float_BE(int fh, const float *Value);

// ------------------------------------------------------------------------------------------------------------------------

// basic arrays
// AF, HGW, 19.Jan23, returns number of Bytes written
ssize_t write_float_Array_BE(int filehandle, float *FloatArray, size_t size);

// AF, HGW, 16.Feb23, the fwrite()-version
ssize_t fwrite_float_Array_BE(float *FloatArray, size_t size, FILE *file);

// AF, HGW, 16.Feb23, returns 1 if all bytes written, otherwise 0
ssize_t fwrite_SHORT_Array_BE(SHORT *SHORTArray, size_t size, FILE *file);

// AF, HGW, 20.Mar23, returns number of Bytes written
ssize_t write_double_Array_BE(int filehandle, double *DoubleArray, size_t size);

// AF, HGW, 20.Mar23, returns number of Bytes written
ssize_t fwrite_double_Array_BE(double *DoubleArray, size_t size, FILE *file);

// AF, HGW, 20.Mar23, returns number of Bytes written
long write_short_Array_BE(int filehandle, short *ShortArray, size_t size);

// AF, HGW, 20.Mar23
long write_ushort_Array_BE(int filehandle, short *UShortArray, size_t size);

// AF, HGW, 20.Mar23
long write_LONG_Array_BE(int filehandle, LONG *LongArray, size_t size);

// AF, HGW, 20.Mar23, returns number of Bytes written
long write_ULONG_Array_BE(int filehandle, ULONG *ULongArray, size_t size);

// ------------------------------------------------------------------------------------------------------------------------
// special structures

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteParHeader_BE(const struct ParHeader *ParHdr,FILE *file);

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteMotion_BE(const struct Motion *Value, FILE *file);

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteMoPar_BE(const struct Animation *MoPar, FILE *file);

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteCoPar_BE(const struct Palette *CoPar, FILE *file);

// AF: 5.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteColorItem_BE(const struct Color *CoItem, FILE *file);

// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteEcoPar_BE(const union Environment *EcoPar, FILE *file);

// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteEcoParItem_BE(const struct Ecosystem *EcoParItem, FILE *file);

// AF: 6.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteSettings_BE(const struct Settings *settings, FILE *file);

// AF: 9.Jan23, Write struct in Big-Endian (i.e. native Amiga-) format
int fwriteKeyFrames_BE(const union KeyFrame *KeyFrames, short NumKeyframes, FILE *file);

// AF, 20.Mar23 writes the DEM-Buffer in Big Endian Format, cares for int, unsigned and float, 1,2,4,8 Bytes size
long writeDemArray_BE(long fOutput,void *OutputData,long OutputDataSize,short outvalue_format,short outvalue_size);

// AF, 19.Jan23, always write BigEndian
ssize_t writeILBMHeader_BE(int filehandle, struct ILBMHeader *Hdr);

// AF, 19.Jan23, always write BigEndian
ssize_t writeZBufferHeader_BE(int filehandle, struct ZBufferHeader *ZBufHdr);

// AF: 21.Mar.23 correct endian if necessary and write
long writeElMapHeaderV101_BE(int fh, struct elmapheaderV101 *Hdr);

// AF: 22.Mar.23 correct endian if necessary and write
long fwriteVectorheaderV100_BE(struct vectorheaderV100 *Hdr, FILE *file);

// ########################################################################################################################
// Read functions
// ########################################################################################################################

// basic types

// AF, HGW, 21.Apr23
int fread_float_BE(float *Value, FILE *file);

// AF, HGW, 22.Jan23
int fread_double_BE(double *Value, FILE *file);

int fread_short_BE(short *Value, FILE *file);

// AF, HGW, 19.Oct23
int fread_LONG_BE(LONG *Value, FILE *file);

// basic arrays
// AF, HGW, 19.Oct23, returns number of Bytes read
int fread_SHORT_Array_BE(SHORT *FloatArray, ssize_t size, ssize_t cnt, FILE *file);

// AF, HGW, 29.Mar23, returns number of Bytes read
ssize_t read_float_Array_BE(int filehandle, float *FloatArray, ssize_t size);

// AF, HGW, 10.Oct23
ssize_t read_double_Array_BE(int filehandle, double *DoubleArray, ssize_t size);

// AF, HGW, 10.Oct23
ssize_t read_int_Array_BE(int filehandle, int *IntArray, ssize_t size);

// ------------------------------------------------------------------------------------------------------------------------
// special structures

// AF, HGW, 20.Jan23
ssize_t readZBufHdr_BE(int filehandle, struct ZBufferHeader *ZBufHdr);

// AF: 20-Mar.23 read and correct endian if necessary
long readElMapHeaderV101_BE(int fh, struct elmapheaderV101 *Hdr);

// AF, 11.Oct23 reads the DEM-Buffer in Big Endian Format, cares for int, unsigned and float, 1,2,4,8 Bytes size
long readDemArray_BE(long fInput,void *InputData,long InputDataSize,short invalue_format,short invalue_size);


#endif /* BIGENDIANREADWRITE_H_ */
