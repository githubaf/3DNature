// UsefulSwap.cpp
// Non-Endian swap code from Useful.cpp
// Built from Useful.cpp on 060403 by CXH

#include "stdafx.h"
#include "UsefulSwap.h"

#ifdef _WIN32
void swmem(void *a, void *b, unsigned n)
{
char *Source, *Dest, Temp;
unsigned Index;

Source = (char *)a;
Dest   = (char *)b;

for(Index = 0; Index < n; Index++)
	{
	Temp = Source[Index];
	Source[Index] = Dest[Index];
	Dest[Index] = Temp;
	} // for

} // swmem()
#endif // _WIN32

/*===========================================================================*/

void SimpleDataFlip(void *a, unsigned long int n)
{
unsigned long FlipLoop, Top, Idx;
unsigned char TempByte, *ByteBlock;

Top = n/2;
ByteBlock = (unsigned char *)a;
for(FlipLoop = 0; FlipLoop < Top; FlipLoop++)
	{
	Idx = (n - FlipLoop) - 1;
	TempByte = ByteBlock[FlipLoop];
	ByteBlock[FlipLoop] = ByteBlock[Idx];
	ByteBlock[Idx] = TempByte;
	} // for

} // SimpleDataFlip
