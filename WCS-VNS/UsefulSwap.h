// UsefulSwap.h
// Non-Endian swap code from Useful.h
// Built from Useful.h on 060403 by CXH

#ifndef WCS_USEFULSWAP_H
#define WCS_USEFULSWAP_H

#ifdef _WIN32
// Some OSes and compilers don't have the decency to implement useful UNIX
// library functions like swmem()
void swmem(void *a, void *b, unsigned n);
#endif // _WIN32

// Used in J. Jone's image loader/saver code
// Arnie's neato byte-swapping macros:
#define ARNIE_SWAP(a,b) { a^=b; b^=a; a^=b; }  // this cool in-place swap came from a graphics gem...

inline void Swap64(double &A, double &B) {register double C; C=A;A=B;B=C;};
inline void Swap32S(signed int &A, signed int &B) {register signed int C; C=A;A=B;B=C;};
inline void Swap32U(unsigned int &A, unsigned int &B) {register unsigned int C; C=A;A=B;B=C;};
inline void Swap32F(float &A, float &B) {register float C; C=A;A=B;B=C;};
inline void SwapV(void **A, void **B) {register void *C; C=*A;*A=*B;*B=C;};
inline void Swap16S(signed short &A, signed short &B) {register signed short C; C=A;A=B;B=C;};
inline void Swap16U(unsigned short &A, unsigned short &B) {register unsigned short C; C=A;A=B;B=C;};

// This turns ABCDEFGH into HGFEDCBA
// Useful in TARGA files that may be stored horizontal-flipped
void SimpleDataFlip(void *a, unsigned long n);

#endif // WCS_USEFULSWAP_H
