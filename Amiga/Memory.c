/* Memory.c (ne gismemory.c 14 Jan 1994 CXH)
** Memory allocation and clearing functions for WCS.
** Original code by Gary R. Huber, August 1, 1993.
*/

#include "WCS.h"

/*#define MEMCHECK*/

void *get_Memory(long zsize, long attributes)
{
 APTR memblock;

#ifdef AMIGA_GUI
 if ((memblock = (void *)AllocMem(zsize,attributes)) == NULL)
  {
  sprintf(str, "Bytes = %ld", zsize);
  Log(ERR_MEM_FAIL, (CONST_STRPTR)str);
/*
  sprintf(str, "Allocation: %d, Free: %d, Largest: %d\n", zsize, 
	AvailMem(MEMF_FAST), AvailMem(MEMF_FAST | MEMF_LARGEST));
  User_Message("Memory Alloc Fail", str, "OK", "o");
*/
  } /* out of memory */
#ifdef MEMTRACK
 else
  {
  MemTrack += zsize;
  }
#endif /* MEMTRACK */
#ifdef MEMCHECK
 printf("GETMEM: %d @ 0x%x\n", zsize, memblock);
#endif /* MEMCHECK */

#else
 memblock = NULL;
#endif /* AMIGA_GUI */

 return (memblock);

} /* get_Memory() */

/*********************************************************************/

void free_Memory(void *memblock, long zsize)
{

#ifdef AMIGA_GUI
 FreeMem(memblock, zsize);
#ifdef MEMTRACK
 MemTrack -= zsize;
#endif /* MEMTRACK */
#ifdef MEMCHECK
 printf("FREEMEM: %d @ 0x%x\n", zsize, memblock);
#endif /* MEMCHECK */
#endif /* AMIGA_GUI */

} /* free_Memory */

