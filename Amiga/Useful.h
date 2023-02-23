// Useful.h
// Just generally useful code I thought I'd put in one file.
// Written from memory on 3/23/95 by Chris "Xenon" Hanson
// Copyright 1995

#ifndef WCS_USEFUL_H
#define WCS_USEFUL_H

#include <stddef.h>
#include <exec/types.h>

#ifndef ROUNDUP
        // Do NOT make b a zero, that would be dumb.
        #define ROUNDUP(a,b)    (b * (a + (b - 1)) / b)

#endif // ROUNDUP

// AF, 12.Dec.22
#if defined  __AROS__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   #define ENDIAN_CHANGE_IF_NEEDED(x) x
#else
   #define ENDIAN_CHANGE_IF_NEEDED(x)
#endif

#if defined  __AROS__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
inline void SimpleEndianFlip64 (            double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
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

inline void SimpleEndianFlip32F(             float Source32, float  *Dest32)  // AF, 10Dec22 for i386-aros
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
inline       void SimpleEndianFlip32U( ULONG Source32, ULONG *Dest32)  // AF, 10Dec22 for i386-aros
       {
           (*Dest32) = (ULONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
                       (ULONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
       }


inline      void SimpleEndianFlip32S( LONG Source32, LONG *Dest32)  //AF, 10Dec22 for i386-aros
       {
           (*Dest32) = ( LONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
                       ( LONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
       }

inline void SimpleEndianFlip16U(USHORT Source16, USHORT *Dest16) {(*Dest16) = (USHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
inline void SimpleEndianFlip16S(SHORT Source16, SHORT *Dest16) {(*Dest16) = ( SHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
#endif


char *StripExtension(char *Source);

#endif // WCS_USEFUL_H

