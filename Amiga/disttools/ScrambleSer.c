/* ScrambleSer.c
** Takes a serial number as ascii on the command line, performs horrible
** butchery on it to add a check byte and scramble it. Spits the result
** to stdout.
** Because last byte is used as check, only serial #'s up to 16,777,215
** can be used.
** Created from scratch on 03 Aug 1994 by Christopher Eric "Xenon" Hanson
** Copyright ©1994 by CXH, Arcticus, and Questar.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* #define TEST_PHASE */
#define MAGIC_FOOBAR 0x00F00BA2

int main(int Count, char *Vector[])
{
FILE *fScram;
unsigned long int PlainSer, OrLong;
unsigned char B0, B1, B2, CheckByte;

if (Count == 2)
	{
	if(stcd_l(Vector[1], &PlainSer))
		{
		if((PlainSer > 0) && (PlainSer < 0xFFFFFF))
			{
			B0 = PlainSer & 0x000000FF;
			B1 = (PlainSer & 0x0000FF00) >> 8;
			B2 = (PlainSer & 0x00FF0000) >> 16;
			CheckByte = (B0 ^ B1 ^ B2);
			#ifdef TEST_PHASE
			printf("Serial: %d, B0: %x, B1: %x, B2: %x, Check: %x.\n",
			 PlainSer, B0, B1, B2, CheckByte);
			#endif /* TEST_PHASE */
			OrLong = CheckByte;
			OrLong = OrLong << 24;
			OrLong |= PlainSer;
			OrLong ^= MAGIC_FOOBAR;
			printf("%lX\n", OrLong);
			fScram = fopen("ENV:NEXTSERIAL", "w");
			if (fScram)
				{
				fprintf(fScram, "%lX\n", OrLong);
				fclose(fScram);
				} /* if */
			fScram = fopen("ENVARC:NEXTSERIAL", "w");
			if (fScram)
				{
				fprintf(fScram, "%lX\n", OrLong);
				fclose(fScram);
				} /* if */
			exit(0);
			} /* if */
		else
			{
			printf("Serial number out of range.\n");
			} /* else */
		} /* if */
	else
		{
		printf("Problems converting ascii to int.\n");
		} /* else */
	} /* if */
else
	{
	printf("Wrong number of args. Read the source code for more info.\n");
	} /* else */

exit(30);
} /* main() */
