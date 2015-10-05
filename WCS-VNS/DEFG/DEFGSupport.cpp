// DEFGSupport.cpp
// support code for running DEFG outside of WCS/VNS
// created from scratch and from DEFG.cpp on
// Fri Aug 24, 2001 by CXH

#include <malloc.h>
#include "DEFGSupport.h"
#include "DEFG.h"

void *AppMem_Alloc(size_t AllocSize, unsigned long int Flags, char *Topic, unsigned char Optional)
{
if(Flags & APPMEM_CLEAR)
	{
	return(calloc(1, AllocSize));
	} // if
else
	{
	return(malloc(AllocSize));
	} // else

} // AppMem_Alloc

void AppMem_Free(void *Me, size_t DummyVal)
{
free(Me);
} // AppMem_Free



int LoadPoints(char *InFile, DEFG *DG)
{
FILE *In;
int PointsLoaded = 0;

float A, B, C;

if(In = fopen(InFile, "r"))
	{
	while(fscanf(In, "%f %f %f", &A, &B, &C) != EOF)
		{
		//InPoints[PointsLoaded].X = -A; // if using WCS pos=west longitude
		//InPoints[PointsLoaded].Y = -B; // always, to convert UR positive map form to LR positive array form
		//InPoints[PointsLoaded].Z = C;
		DG->AddPoint(-A, -B, C);
		PointsLoaded++;
		} // while
	fclose(In);
	In = NULL;
	} // if

return(PointsLoaded);
} // LoadPoints

double GetSystemTimeFP(void)
{
double Now = 0.0;
#ifndef __MACH__
struct _timeb TimeBuf;
_ftime(&TimeBuf);
Now = TimeBuf.time + TimeBuf.millitm / 1000.0;
#else // !__MACH__
struct timeval TimeBuf;
gettimeofday( &TimeBuf, NULL );
Now = TimeBuf.tv_sec + TimeBuf.tv_usec / 1000.0;
#endif // __MACH__

return(Now);
} // GetSystemTimeFP

