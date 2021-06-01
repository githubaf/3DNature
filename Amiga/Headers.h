/* Headers.h
** One #include to rule them all, one #include to find them,
** one #include to bring them all, and in the preprocessor bind them.
** Built from map.h on 24 Jul 1993 by Chris "Xenon" Hanson
** Original code by the one and only Gary R. Huber.
** Cleaned up and rearranged on 22 May 1995 by CXH
*/

/* ANSI headers */
//#include <dos.h>
#include <dos/dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <ctype.h>
#include <limits.h>
#include <float.h>
#include <math.h>

/* comment out the ieee math library for 040 version and the 68881 library
** for nocoproc version */
/*#include <mieeedoub.h>*/
//#include <m68881.h>

/* Amiga-specific headers */
#include <devices/serial.h>
#include <devices/printer.h>
#include <exec/memory.h>
#include <graphics/gfxbase.h>
#include <graphics/display.h>
#include <graphics/displayinfo.h>
#include <graphics/gfxmacros.h>
#include <hardware/blit.h>
#include <intuition/intuition.h>
#include <intuition/screens.h>
#include <libraries/asl.h>
#include <libraries/mui.h>
#include <workbench/startup.h>
#include <workbench/workbench.h>
#include	<rexx/storage.h>
#include	<rexx/rxslib.h>


/* Prototype headers */
#include <clib/alib_protos.h>
#ifdef __GNUC__
    //#include <proto/muimaster_lib.h>   // ALEXANDER: Fehler in GCC-MUI-SDK???
    #include <libraries/mui.h>
    //#warning "GCC MUI-Include problem in Header.h??? "
#else
    #include <proto/muimaster.h>
#endif
//#include <proto/all.h>


/* All of these are covered by proto/all.h */

#include <proto/exec.h>
//#include <clib/exec_protos.h> // proto/exec.h
#include <proto/asl.h> //#include <clib/asl_protos.h>
#include <proto/intuition.h> //#include <clib/intuition_protos.h>
#include <proto/graphics.h>   //#include <clib/graphics_protos.h>
#include <proto/dos.h>     //#include <clib/dos_protos.h>
#include <proto/icon.h>    //#include <clib/icon_protos.h>
#include <proto/gadtools.h>  //#include <clib/gadtools_protos.h>



/* Currently unused or redundant headers */
/*
#include <exec/io.h> // devices/[serial|printer]
#include <exec/types.h>  // i/i.h
#include <graphics/gfx.h> // i/i.h
#include <graphics/text.h> // i/i.h
#include <libraries/dos.h> // dos.h
*/
