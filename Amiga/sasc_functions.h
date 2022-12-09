/*
 * sasc_functions.h
 *
 *  Created on: Jul 22, 2021
 *      Author: AF
 */

#ifndef SASC_FUNCTIONS_H_
#define SASC_FUNCTIONS_H_

int Mkdir(const char *name);  // calls mkdir(name) because mkdir on gcc has two parameters

#ifdef SWMEM_INLINE   // define in Makefile if swmem should be inlined
inline void swmem(void *a, void *b, unsigned n)  // SAS/C function, needs to be re-implemented for gcc, Swap two memory blocks
{
    unsigned char temp;
    unsigned int i;
    unsigned char *p1=(unsigned char *)a, *p2=(unsigned char *)b;

    for(i=0;i<n;i++)
    {
        temp=*p2;
        *p2++=*(unsigned char*)p1;
        *p1++=temp;
    }
}
#elif SWMEM_FAST_INLINE


// special functions for 1, 8 and random number of bytes. 1 and 8 are by far the MOSTLY USED sizes in wcs.
// The compiler decides at compiler time, which one to use/inline if n is a constant.

#define swmem(a,b,n) \
    (__builtin_constant_p(n) && n==1) ? swmem_1(a,b) :  \
        ((__builtin_constant_p(n) && n==8) ? swmem_8(a,b) : swmem_other(a,b,n))



inline void swmem_1(void *a, void *b)
{
    unsigned char temp, *p1=(unsigned char *)a, *p2=(unsigned char *)b;
    temp=*p2;
    *p2 = *p1;
    *p1 = temp;
}

inline void swmem_8(void *a, void *b)
{
    unsigned int temp, *p1=(unsigned int *)a, *p2=(unsigned int *)b;
    temp=*p2;
    *p2++ = *p1;
    *p1++ = temp;

    temp=*p2;
    *p2 = *p1;
    *p1 = temp;
}

inline void swmem_other(void *a, void *b, unsigned n)
{
    unsigned char temp;
    unsigned int i;
    unsigned char *p1=(unsigned char *)a, *p2=(unsigned char *)b;

    for(i=0;i<n;i++)
    {
        temp=*p2;
        *p2++=*(unsigned char*)p1;
        *p1++=temp;
    }
}

#else
    void swmem(void *a, void *b, unsigned n);  // SAS/C function, needs to be re-implemented for gcc, Swap two memory blocks
#endif

void strmfp(char *name, const char *path, const char *node); // SAS/C function, needs to be re-implemented for gcc, Make a filename from the path or node
void strmfn(char *file, const char *drive, const char *path, const char *node, const char *ext); // SAS/C function, needs to be re-implemented for gcc, Make a filename from components
void strsfn(const char *file, char *drive, char *path, char *node, char *ext); // SAS/C function, needs to be re-implemented for gcc, Split the filename
int stcgfe(char *ext, const char *name); // SAS/C function, needs to be re-implemented for gcc, Get the filename extension
int stcul_d(char *out, unsigned long uvalue); // SAS/C function, needs to be re-implemented for gcc, Convert an unsigned long integer to a decimal string
double pow2(double x); // SAS/C function, needs to be re-implemented for gcc, Raise 2 to a power
#ifndef __AROS__
// in AROS this defined in /usr/local/amiga/i386-aros/include/aros/stdc/string.h
const char * stpblk(const char *p); // SAS/C function, needs to be re-implemented for gcc, Skip blanks
#endif
#ifndef max
   #define max(a,b) ((a)>(b)?(a):(b))  // SAS/C library reference 3357
#endif
#ifndef min
   #define min(a,b) ((a)<=(b)?(a):(b))  // SAS/C library reference 347
#endif
#ifndef tell
long tell(int fh); // SAS/C function, needs to be re-implemented for gcc, Get the level 1 file position
#endif

#endif /* SASC_FUNCTIONS_H_ */
