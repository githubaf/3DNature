// UsefulEndian.h
// Endian-related code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULENDIAN_H
#define WCS_USEFULENDIAN_H

       void BlindSimpleEndianFlip64 (        void *Source64, void *Dest64);
       void BlindSimpleEndianFlip32F(        void *Source32, void *Dest32);
//       void SimpleEndianFlip64 (            double Source64, double *Dest64);
//       void SimpleEndianFlip32F(             float Source32, float  *Dest32);
       void SimpleEndianFlip64 (             void *Source64, double *Dest64);	// it isn't safe to load a double/float before
       void SimpleEndianFlip32F(             void *Source32, float  *Dest32);	// it's been normalized
       void SimpleEndianFlip32U( unsigned long int Source32, unsigned long int *Dest32);
       void SimpleEndianFlip32S(   signed long int Source32, signed long int   *Dest32);
inline void SimpleEndianFlip16U(unsigned short int Source16, unsigned short int *Dest16) {(*Dest16) = (unsigned short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};
inline void SimpleEndianFlip16S(  signed short int Source16, signed short int   *Dest16) {(*Dest16) = (  signed short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );};

// Used in J. Jone's image loader/saver code
#ifdef BYTEORDER_LITTLEENDIAN
#define	BSWAP_W(w) ARNIE_SWAP( (((unsigned char *)&w)[0]), (((unsigned char *)&w)[1]) )
#define	BSWAP_L(L) { BSWAP_W( (((unsigned short *)&L)[0])); BSWAP_W( (((unsigned short *)&L)[1])); ARNIE_SWAP( (((unsigned short *)&L)[0]), (((unsigned short *)&L)[1]) ) }
#else // BIGENDIAN
#define	BSWAP_W(w)	(w)	
#define	BSWAP_L(L)	(L)
#endif // BIGENDIAN

#endif // WCS_USEFULENDIAN_H
