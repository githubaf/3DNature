/* SerInsert.c
** Inserts a serial number into a file at two specified offsets.
** The first offset specifies the location to insert the 4-byte
** binary version. The second offset will be used to insert the
** 8-byte ascii hexadecimal version. Note: At this time, there
** are ten bytes reserved for the ascii version -- the last two
** are currently filled with NULLs. MUI will understand.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

int main(int Count, char *Vector[])
{
unsigned long int BinOffset, AscOffset, Serial;
FILE *ToBe;
char Buffer[20];

memset(Buffer, 0, 18); /* overkill */
strcpy(Buffer, Vector[3]);

if(Count == 5)
	{
	if(ToBe = fopen(Vector[4], "r+"))
		{
		BinOffset = atol(Vector[1]);
		AscOffset = atol(Vector[2]);
		if(BinOffset && AscOffset)
			{
			if(!fseek(ToBe, BinOffset, SEEK_SET))
				{
				if(stch_l(Vector[3], &Serial))
					{
					if(fwrite(&Serial, 1, 4, ToBe) == 4)
						{
						if(!fseek(ToBe, AscOffset, SEEK_SET))
							{
							if(fwrite(Buffer, 1, 10, ToBe) == 10)
								{
								exit(0);
								} /* if */
							else
								{
								printf("Error writing ascii serial number. File now probably corrupt.\n");
								} /* else */
							} /* if */
						else
							{
							printf("Error in ascii seek.\n");
							} /* else */
						} /* if */
					else
						{
						printf("Error writing binary serial number. File now probably corrupt.\n");
						} /* else */
					} /* if */
				else
					{
					printf("Serial hex to integer conversion error.\n");
					} /* else */
				} /* if */
			else
				{
				printf("Binary seek error.\n");
				} /* else */
			} /* if */
		else
			{
			printf("Invalid offset. %d %d %s %s\n",
			 BinOffset, AscOffset, Vector[1], Vector[2]);
			} /* else */
		} /* if */
	else
		{
		printf("Unable to open file \"%s\".\n", Vector[4]);
		} /* else */
	} /* if */
else
	{
	printf("Usage: %s <binoffset> <ascoffset> <hexserial> <filename>\n", Vector[0]);
	} /* else */

exit(30);
} /* main() */

