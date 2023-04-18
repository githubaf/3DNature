/*
 * sasc_functions.c
 *
 *  Created on: Jun 7, 2021
 *      Author: Alexander Fritsch
 */

#include <stdio.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <ctype.h>
#include <stdlib.h>

#include "sasc_functions.h"

#ifndef max
   #define max(a,b) ((a)>(b)?(a):(b))  // SAS/C library reference 3357
#endif
#ifndef min
   #define min(a,b) ((a)<=(b)?(a):(b))  // SAS/C library reference 347
#endif



/***
*
* The following symbols define the sizes of file names and node names.
*
***/
#ifndef FMSIZE   // SAS/C define
    #define FMSIZE 256 /* maximum file name size      - DOS limit */
#endif
#ifndef FNSIZE
    #define FNSIZE 108  /* maximum file node size      - DOS limit */
#endif
#ifndef FESIZE
    #define FESIZE 32   /* maximum file extension size - arbitrary */
#endif


#ifdef __GNUC__
/*   SAS/C-only function
void swmem( void *a, void *b, unsigned n) ;

void *a,*b; // block pointers
unsigned n; // number of bytes

This function swaps two blocks of memory. This function neither
recognizes nor produces the NULL terminator byte usually found at the
end of strings.

This function is not available if the _STRICT_ANSI flag has been
defined.
*/

#ifdef SWMEM_INLINE   // define in Makefile if swmem() should be inlined
// SAS/C-only function, own re-implementation AF
// Body is moved to header-File as inline function
#elif SWMEM_FAST_INLINE
// // SAS/C-only function, own re-implementation AF
// special macro/functions for 1, 8 and other sizes. Compiler decides which one to use if n is a constant
// Body is moved to header-File as inline function
extern void swmem_1(void *a, void *b);
extern void swmem_8(void *a, void *b);
extern void swmem_other(void *a, void *b, unsigned n);


#else
void swmem(void *a, void *b, unsigned n)  // SAS/C-only function, own re-implementation AF
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
#endif
#endif

#ifdef __GNUC__
/*   SAS/C-only function

Make a filename from the path or node

((include <string.h>

void strmfp( name, path, node) ;

char *name; // file name
const char *path; // directory path
const char *node; // node

This function copies the path string (if it is not NULL) to the file name
area and appends the SLASH separator if the path string does not end
with a slash or colon. Then, the node string is appended to the file
name. SLASH is an external character variable that defaults to a slash
(/).

The name area must be large enough to accept the filename string.
This function is not available if the STRICT ANSI flag has been
defined.
*/

void strmfp(char *name, const char *path, const char *node)
{
    char *slash;
    if(path)
    {
        if((strlen(path)>=1) && ((path[strlen(path)-1]!=':') && (path[strlen(path)-1]!='/')))
        {
            slash="/";
        }
        else
        {
            slash="";
        }
    }
    else
    {
        path="";
        slash="";
    }
    sprintf(name,"%s%s%s",path,slash,node);
}
#endif

#ifdef __GNUC__
/*   SAS/C-only function

Strmfn Make a filename from components

void strmfn(f ile, drive, path, node, ext)
char *file;        // file name pointer
const char *drive  // drive code pointer
const char *path;  // directory path pointer
const char *node;  // node pointer
const char *ext;   // extension pointer

Description This function makes a filename from four possible components. The name
is constructed as follows:

drive : path/node . ext

If the drive pointer is not NULL, the drive pointer is moved to the
area pointed to by the file argument. Then, a colon is inserted unless
one is already there. Next, if the path pointer is not NULL, it is
appended to file, and the directory separator specified by SLASH is
added if necessary. The node string is appended next, unless it is NULL.
Finally, if the ext pointer is not NULL, a period is appended to file,
followed by the ext string.

Make sure that the file pointer refers to an area which is large
enough to hold the result.
*/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
void strmfn(char *file, const char *drive, const char *path, const char *node, const char *ext)
{

    char *colon, *slash, *dot;
    if(drive)
    {
        if((strlen(drive)>=1) && ((drive[strlen(drive)-1]!=':')))
        {
            colon=":";
        }
        else
        {
            colon="";
        }
    }
    else
    {
        drive="";
        colon="";
    }

    if(path)
     {
         if((strlen(path)>=1) && (path[strlen(path)-1]!='/'))
         {
             slash="/";
         }
         else
     {
             slash="";
         }
     }
     else
     {
         path="";
         slash="";
     }

    if(ext)
    {
        dot=".";
    }
    else
    {
        dot="";
        ext="";
    }

    sprintf(file,"%s%s%s%s%s%s%s",drive,colon,path,slash,node,dot,ext);
}
#endif
#endif

#ifdef __GNUC__
/*   SAS/C-only function

Split the filename

void strsfn(const char *file,    // file name pointer
            char *drive,         // drive code pointer
            char *path,          // directory path pointer
            char *node,          // node pointer
            char *ext)           // extension pointer


This function splits a filename into four possible components and places
them into the drive, path, node, and ext strings. If any of the
arguments is NULL, then that component is discarded.

A complete filename is constructed as follows:

drive : path/ node . ext

When the strsfn function splits the file name, it leaves the colon
attached to the drive string, but drops the leading or trailing
punctuation characters. In other words, the path component does not
end with a slash, and the ext component does not begin with a period.
Slashes within the path component are preserved.

Make sure that the drive, path, node, and ext pointers refer to
areas that are large enough to hold the largest string that might be
generated. This function does not check that any component lengths are
exceeded, although it does copy the file string to an internal buffer of size
FMSIZE and truncate it if it is too long. If you want to be absolutely sure
that no overflows occur, make each component area FMSIZE bytes long.

This function is not available if the _STRICT_ANSI flag has been
defined.
*/

void strsfn(const char *file,    // file name pointer
        char *drive,         // drive code pointer
        char *path,          // directory path pointer
        char *node,          // node pointer
        char *ext)           // extension pointer
{

    if(file)
    {
        if(strlen(file)>=1)
        {
            char buffer[FMSIZE];
            char *Ptr;
            strcpy(buffer,file);

            Ptr=buffer+strlen(buffer); // points to the final 0
            while(Ptr>buffer)
            {
                Ptr--;
                if(*Ptr=='/' || *Ptr==':')
                {
                    break;
                }
                if(*Ptr=='.')
                {
                    //found!
                    *Ptr=0;        // new end of string in buffer (without ext)
                    if(ext)
                    {
                        sprintf(ext,"%s",Ptr+1);
                    }
                    break;
                }
            }
            if(*Ptr!=0)   // not found
            {
                if(ext)
                {
                    sprintf(ext,"%s","");
                }
            }

            // look for the next '/', node starts there
            while(Ptr>buffer && *Ptr!=':' && *Ptr!='/')
            {
                Ptr--;
            }
                if(*Ptr=='/')
                {
                    //found!
                    *Ptr=0;        // new end of string in buffer (without node and ext)
                    if(node)
                    {
                        sprintf(node,"%s",Ptr+1);
                    }
                }
                else if(*Ptr==':')
                {
                    if(node)
                    {
                        sprintf(node,"%s",Ptr+1);
                        *(Ptr+1)=0;
                    }
                }
                else  // begin of buffer
                {

                    if(node)
                    {
                        sprintf(node,"%s",Ptr);
                        *Ptr=0;
                    }
                }

            // look for the next ':' or begin of filename, path starts there
            while(Ptr>buffer && *Ptr!=':')
            {
                Ptr--;
            }
                if(*Ptr==':')
                {
                    //found!
                    if(path)
                    {
                        sprintf(path,"%s",Ptr+1);
                    }
                    *(Ptr+1)=0;        // new end of string in buffer after(!) the : (without path,node and ext)
                }
                else
                if(path)
                {
                    sprintf(path,"%s",Ptr);
                    buffer[0]=0;
                }
            // rest is drive
            if(drive)
            {
                sprintf(drive,"%s",buffer);
            }
        }
    }
}
#endif


#ifdef __GNUC__
/*   SAS/C-only function

stcgfe Get the filename extension
Synopsis #include <string.h>

         size = stcgf e( ext, name ) ;

         int size;         // size of result string
         char *ext;        // extension area pointer
         const char *name; // file name pointer

Description This function isolates the extension portion of a filename from the path
and node. The node is the rightmost portion of the filename that is
separated from the rest of the name by a colon or a slash. The extension
is the final part of the node that begins with a period, and the path is the
leading part of the name up to the node. The following table contains
examples of how you can isolate the parts of a filename using the
stcgfe, stcgfn, and stcgfp functions.


Name                Path            Node            Extension
---------------------------------------------------------------
myprog.c            NULL string     myprog .c       c
/abc.dir/def        /abc.dir        def             NULL string    AF: Path starts with / (wrong in manual p 448)
/abc.dir/def.ghi    /abc.dir        def.ghi         ghi            AF: Path starts with / (wrong in manual p 448)
df0:yourfile        df0:            yourfile        NULL string
/abc/               /abc            NULL string     NULL string


AF: I checked with SAS/C test program: "NULL string" means "", i.e. empty string

The maximum number of bytes copied into your array is defined in the
file dos.h as FESIZE. You should provide a buffer at least FESIZE
bytes long.

This function is not available if the _STRICT__ANSI flag has been
defined.

Portability SAS/C

Returns The size value is the same as would be returned by the strlen
function. That is, if size is 0, then the desired portion of the filename
could not be found, and the result area contains a null string.

*/

int stcgfe(char *ext, const char *name)
{
    int ret=0;
    if(name)
    {
        if(strlen(name)>=1)
        {
            char buffer[FMSIZE];
            char *Ptr;
            strcpy(buffer,name);

            Ptr=buffer+strlen(buffer); // points to the final 0
            while(Ptr>buffer)
            {
                Ptr--;
                if(*Ptr=='/' || *Ptr==':')
                {
                    break;
                }
                if(*Ptr=='.')
                {
                    //found!
                    *Ptr=0;        // new end of string in buffer (without ext)
                    if(ext)
                    {
                        sprintf(ext,"%s",Ptr+1);
                        ret=strlen(ext);
                    }
                    break;
                }
            }
            if(*Ptr!=0)   // not found
            {
                if(ext)
                {
                    sprintf(ext,"%s","");
                }
            }
        }
    }
    return ret;
}
#endif

#ifdef __GNUC__
/*   SAS/C-only function
C Library Reference 487

stcul_d Convert an unsigned long integer to a decimal string

Synopsis #include <string.h>

         length = stcul_d(out, uvalue);

         int length;            // output length
         char *out;             // output buffer pointer
         unsigned long uvalue;  // unsigned long integer value

This function converts an unsigned long integer into an ASCII string that
is the decimal equivalent of the integer. The output area should be at
least 12 bytes long, which is large enough to accommodate the maximum
possible string, including the terminating NULL byte that each function
appends.

Leading zeroes are suppressed, and a single 0 character is generated if
the input value is 0.

This function is not available if the _STRICT_ANSI flag has been
defined.

The return value is the number of characters actually placed into the
output area, not including the final NULL byte.
*/

int stcul_d(char *out, unsigned long uvalue)
{
    int length=sprintf(out,"%lu",uvalue);
    return length;
}
#endif

#ifdef __GNUC__
/*   SAS/C-only function

376 Chapter 7

pow2 Raise 2 to a power

Synopsis     #include <math.h>
             r = pow2( x) ;
             double r, x;

Description The pow2 function computes 2**x by calling the pow function.
            This function is not available if the _STRICT_ANSI flag has been
            defined.

Portability SAS/C
Returns     The return value r is the value 2**x.
See Also    __matherr
*/

double pow2(double x)
{
    return pow(2,x);
}
#endif


#ifdef __GNUC__
/*   SAS/C-only function
C Library Reference 489

stpblk Skip blanks

Synopsis    # include <string.h>
            q = stpblk(p);
            char *q; // updated string pointer
            const char *p; // string pointer

Description This function advances the string pointer past blank characters, that is,
past all the characters for which the is space function is true.

This function is not available if the _STRICT_ANSI flag has been
defined.

Portability SAS/C

Returns The function returns a pointer to the next nonblank character. The NULL
terminator byte is not considered a blank, and so the function will not go
past the end of the string.
*/
#ifdef UNUSED_FUNCTIONS  // AF, not used 26.July 2021
const char * stpblk(const char *p)
{
    const char *q=p;
    while (*q && isspace(*q))
    {
        q++;
    }
    return q;
}
#endif
#endif

#ifdef __GNUC__
/*   SAS/C-only function
 C Library Reference 565

tell Get the level 1 file position

Synopsis #include <fcntl.h>
         apos = tell(fh) ;
         long apos; // absolute file position
         int fh; // file handle

Description The tell function is equivalent to:
            apos = lseek( fh, OL, 1 ) ;

            The tell function returns a file position that can be used in a
            subsequent call to the lseek function to restore the file to the position at
            the time of the tell call.



Portability UNIX

Returns This function returns -1L if an error occurs, in which case the external
        integers err no and _OSERR contain additional error information.
 */
long tell(int fh)
{
    return lseek(fh,0L,1);;
}


/*
 * 25.July2022, AF, selco, HGW
 * drand48() behaves differently on SAS/C compared to gcc and m68k-amigaos-gcc.
 * We need the same random values if we want to create pixel-identical pictures.
 *(Dithering, Clouds...)
 * After a long disassembling and debugging session it turned out, that after
 * the calculation of a random value,SAS/C adds  the last two 16bit values of
 * the seed together to and store that as the last seed word.
 * So we have to do that in gcc, too. We use erand48() from gcc to provide an
 * own seed-buffer. Therefore we must provide an srand48() function as well.
 *
 */

#ifdef SASC_DRAND48_USING_ERAND48

// I had problems with this one
unsigned short Drand48SeedBuffer[3]={0,0,0x330e};

void srand48(long int seedval)
{
    Drand48SeedBuffer[3]=(seedval&0xffff0000)>>16;
    Drand48SeedBuffer[2]=(seedval&0xffff);
    Drand48SeedBuffer[3]=0x330e;
}

double drand48(void)
{
    double Random;
    Random=erand48(Drand48SeedBuffer);
    Drand48SeedBuffer[2]+=Drand48SeedBuffer[1];  // <-- Additional calculation by SAS/C
    return Random;
}

#else
/*
AF, selco, 25. Juli 2022,HGW
drand48() behaves on SAS/C differently than on gcc/m68k-amigaos-gcc
Even m68k-amigaos-gcc with and without -noixemul result in different values.

gcc af_drand48.c -DSASC_DRAND48_TEST -o af_drand48_linux && ./af_drand48_linux
m68k-amigaos-gcc af_drand48.c -DSASC_DRAND48_TEST -noixemul -o af_drand48_amiga -lm && vamos af_drand48_amiga
*/
unsigned long long AF_Drand48Seed=0;


__stdargs void srand48(long int seedval)  // __stdargs  -> same prototype as in stdlib.h
{
    AF_Drand48Seed=seedval*65536LL+0x330e;
}

double drand48()
{
    AF_Drand48Seed = (0x5DEECE66DL * AF_Drand48Seed + 0xBL) & ((1LL << 48) - 1);
    unsigned short seed_0= AF_Drand48Seed&0xffff;
    unsigned short seed_1=(AF_Drand48Seed&0xffff0000) >> 16;

    //printf("            Seed_0=%04hx, Seed_1=%04hx\n",seed_0, seed_1);

    seed_0=(short)seed_0+(short)seed_1;
    AF_Drand48Seed=(AF_Drand48Seed&0xffffffff0000)+ seed_0;
    //printf("SASC-Fixed: Seed_0=%04hx, Seed_1=%04hx\n",seed_0, seed_1);

    return (double)AF_Drand48Seed / (1LL << 48);
}

#endif
#endif

#ifdef __GNUC__
// ALEXANDER
// SAS/C has a mkdir() function with path parameter only. (gcc has additional mode parameter)
// This is a modified version from projects/libnix/sources/nix/extra/mkdir.c
extern void __seterrno(void);
#include <dos/dos.h>
#include <clib/dos_protos.h>
int Mkdir(const char *name)
{
  BPTR fl;
  int ret;

  if ((fl=CreateDir((STRPTR)name)))
  {
	UnLock(fl);  // CreateDir returns an exclusive lock on the new directory if it succeeds.
    ret=0;
  }
  else
  {
     #ifndef __AROS__
        __seterrno();
     #endif
     ret=-1;
  }
  return ret;
}
#else
int Mkdir(const char *name)
{
    return mkdir(name);
}
#endif

// ------------- Test -------------------
// compile this file alone and define TESTING_SASC_FUNCTIONS on compiler call to run the tests
//#define TESTING_SASC_FUNCTIONS
#ifdef TESTING_SASC_FUNCTIONS

#include <assert.h>
#include <limits.h>

int main(void)
{

    {
        char buffer [FMSIZE];
        /* The next statements both place "abc/def/ghi" */
        /* into the buffer. */

        printf("'', 'abc/def', 'ghi', '' \n" );

        strmfn(buffer,NULL, "abc/def" , "ghi" ,NULL ) ;
        printf( "result = %s\n\n", buffer);

        if(strcmp(buffer,"abc/def/ghi"))
        {
            printf("strmfn() test_01 failed!\n");
            return 20;
        }

        printf("'', 'abc/def/', 'ghi', ''\n");
        strmfn(buffer, NULL, "abc/def/" , "ghi" , NULL ) ;
        printf ( "result = %s\n\n", buffer);

        if(strcmp(buffer,"abc/def/ghi"))
        {
            printf("strmfn() test_02 failed!\n");
            return 20;
        }

        /* The next statements both generate "dfO:myfile.str" */

        printf ("'df0', '', 'myfile', 'str'\n");
        strmfn (buffer , "df0" ,NULL, "myfile" , "str" ) ;
        printf ( "result = %s\n\n", buffer);

        if(strcmp(buffer,"df0:myfile.str"))
        {
            printf("strmfn() test_03 failed!\n");
            return 20;
        }


        printf ("'df0:' , '', 'myfile', 'str'\n");
        strmfn (buffer , "df0:" ,NULL, "myfile" , "str" ) ;
        printf ( "result = %s\n\n", buffer);

        if(strcmp(buffer,"df0:myfile.str"))
        {
            printf("strmfn() test_04 failed!\n");
            return 20;
        }

        printf ("strmfn(): All tests passed.\n");
        printf("----------------------------\n\n");
    }

    // -----------------------------------------------------------------------------------------

    {
        typedef struct
        {
                char *Input;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {
                {"myfile","D= P= N=myfile E="},
                {".txt","D= P= N= E=txt"},
                {"myfile.txt","D= P= N=myfile E=txt"},
                {"abc/def","D= P=abc N=def E="},
                {"df0:", "D=df0: P= N= E="},
                {"df0:myfile", "D=df0: P= N=myfile E="},
                {"df0:myfile.str", "D=df0: P= N=myfile E=str"},
                {"df0:abc/myfile.str", "D=df0: P=abc N=myfile E=str"},
                {"df0:abc/def/ghi/myfile.str", "D=df0: P=abc/def/ghi N=myfile E=str"},
                {"/abc/","D= P=/abc N= E="},
                {"/abc.dir/def","D= P=/abc.dir N=def E="},
                {"/abc.dir/def.ghi","D= P=/abc.dir N=def E=ghi"},

                {NULL, NULL}                // Input==NULL marks end
        };



        /* After the next statement, the component strings are: */
        /* drive => "" path => "abc/def" */
        /* node => "ghi" ext => "" */
        /* strsfn( "abc/def /ghi" , drive, path, node, ext) ; */
        /* */
        /* After the next statement, the component strings are: */
        /* drive => "dfO:" path => "" */
        /* node => "myfile" ext => "str" */
        /* strsfn("dfO:myfile.str", drive, path, node, ext) ; */

        char drive [FNSIZE] ,path[FMSIZE] ,
        node[FNSIZE] ,ext[FESIZE] ;
        char result[512];   // enough space3 for our test result-string
        char *file;
        int i=0;

        while(InputOutputStrings[i].Input!=NULL)
        {
            printf("Testing \"%s\"...",InputOutputStrings[i].Input);
            file=InputOutputStrings[i].Input;
            strsfn ( file, drive , path , node , ext );
            sprintf(result,"D=%s P=%s N=%s E=%s",drive , path , node , ext);
            printf(" Result: %s\n",result);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("strsfn(): All tests passed.\n");
        printf("----------------------------\n\n");


    }
    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                char *Input;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {"myprog.c","E=c S=1"},
                {"/abc.dir/def","E= S=0"},
                {"/abc.dir/def.ghi","E=ghi S=3"},
                {"df0:yourfile","E= S=0"},
                {"/abc/","E= S=0"},
                {NULL,NULL}                         // Input==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        char ext[FESIZE];
        int i=0;
        int size;

        while(InputOutputStrings[i].Input!=NULL)
        {
            printf("Testing \"%s\"...",InputOutputStrings[i].Input);
            size=stcgfe (ext, InputOutputStrings[i].Input);
            sprintf(result,"E=%s S=%d",ext,size);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("stcgfe(): All tests passed.\n");
        printf("----------------------------\n\n");

    }

    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                unsigned long int Input;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {0,"O=0 S=1"},
                {1,"O=1 S=1"},
                {123,"O=123 S=3"},
                {(0x7fffffffL * 2UL + 1),"O=4294967295 S=10"},    // on Amiga is defined:  #define ULONG_MAX (0x7fffffffL * 2UL + 1)
                {0,NULL}    // ExpectedOutput==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        char out[12];
        int i=0;
        int length;

        while(InputOutputStrings[i].ExpectedOutput!=NULL)
        {
            printf("Testing \"%lu\"...",InputOutputStrings[i].Input);
            length=stcul_d (out, InputOutputStrings[i].Input);
            sprintf(result,"O=%s S=%d",out,length);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("stcul_d(): All tests passed.\n");
        printf("----------------------------\n\n");

    }

    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                double Input;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {0, "R=1.000000"},
                {1, "R=2.000000"},
                {2, "R=4.000000"},
                {16,"R=65536.000000"},
                {0.5,"R=1.414214"},
                {0,NULL}    // ExpectedOutput==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        int i=0;
        double resultval;

        while(InputOutputStrings[i].ExpectedOutput!=NULL)
        {
            printf("Testing \"%f\"...",InputOutputStrings[i].Input);
            resultval=pow2 (InputOutputStrings[i].Input);
            sprintf(result,"R=%f",resultval);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("pow2(): All tests passed.\n");
        printf("----------------------------\n\n");

    }
    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                char *Input;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {"", "Q="},
                {" ", "Q="},
                {"   ", "Q="},
                {"Test", "Q=Test"},
                {" Test", "Q=Test"},
                {"   Test", "Q=Test"},
                {0,NULL}    // ExpectedOutput==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        int i=0;
        const char *q;

        while(InputOutputStrings[i].ExpectedOutput!=NULL)
        {
            printf("Testing \"%s\"...",InputOutputStrings[i].Input);
            q=stpblk(InputOutputStrings[i].Input);
            sprintf(result,"Q=%s",q);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("stpblk(): All tests passed.\n");
        printf("----------------------------\n\n");

    }

    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                double Input1;
                double Input2;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {0,0, "M=0.000000"},
                {-1,1, "M=1.000000"},
                {1.123,2, "M=2.000000"},
                {2,1.123, "M=2.000000"},
                {0,0,NULL}    // ExpectedOutput==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        int i=0;
        double m;

        while(InputOutputStrings[i].ExpectedOutput!=NULL)
        {
            printf("Testing \"%f,%f\"...",InputOutputStrings[i].Input1,InputOutputStrings[i].Input2);
            m=max(InputOutputStrings[i].Input1,InputOutputStrings[i].Input2);
            sprintf(result,"M=%f",m);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("max(): All tests passed.\n");
        printf("----------------------------\n\n");

    }

    // -----------------------------------------------------------------------------------------
    {
        typedef struct
        {
                double Input1;
                double Input2;
                char *ExpectedOutput;
        }InputOutputString;

        InputOutputString InputOutputStrings[]=
        {

                {0,0, "M=0.000000"},
                {-1,1, "M=-1.000000"},
                {1.123,2, "M=1.123000"},
                {2,1.123, "M=1.123000"},
                {0,0,NULL}    // ExpectedOutput==NULL marks end
        };

        char result[512];   // enough space3 for our test result-string
        int i=0;
        double m;

        while(InputOutputStrings[i].ExpectedOutput!=NULL)
        {
            printf("Testing \"%f,%f\"...",InputOutputStrings[i].Input1,InputOutputStrings[i].Input2);
            m=min(InputOutputStrings[i].Input1,InputOutputStrings[i].Input2);
            sprintf(result,"M=%f",m);
            //printf("\n");
            printf(" Result: %s\n",result);
            //printf(" Expect: %s\n",InputOutputStrings[i].ExpectedOutput);
            assert(!strcmp(result,InputOutputStrings[i].ExpectedOutput));
            i++;
        }
        printf ("min(): All tests passed.\n");
        printf("----------------------------\n\n");

        // -----------------------------------------------------------------------------------------
        {
            printf("Still need a test for tell()\n");
            printf("----------------------------\n\n");
        }
    }

    return 0;
}

#endif

