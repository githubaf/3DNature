/*
 * sasc_functions.c
 *
 *  Created on: Jun 7, 2021
 *      Author: ALexander Fritsch
 */

#include <stdio.h>
#include <string.h>

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

void swmem(void *a, void *b, unsigned n)  // SAS/C-only function, own re-implementation AF
{
    unsigned char temp;
    unsigned int i;

    for(i=0;i<n;i++)
    {
        temp=*(unsigned char*)b;
        *(unsigned char*)b++=*(unsigned char*)a;
        *(unsigned char*)a++=temp;
    }
}
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
area and appends the —SLASH separator if the path string does not end
with a slash or colon. Then, the node string is appended to the file
name. —SLASH is an external character variable that defaults to a slash
(/).

The name area must be large enough to accept the filename string.
This function is not available if the —STRICT— ANSI flag has been
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
appended to file, and the directory separator specified by —SLASH is
added if necessary. The node string is appended next, unless it is NULL.
Finally, if the ext pointer is not NULL, a period is appended to file,
followed by the ext string.

Make sure that the file pointer refers to an area which is large
enough to hold the result.
*/

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


// ------------- Test -------------------
// compile this file alone and define TESTING_SASC_FUNCTIONS on compiler call to run the tests
#define TESTING_SASC_FUNCTIONS
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



    return 0;
}

#endif

