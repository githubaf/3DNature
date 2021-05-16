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

#ifdef __GNUC__
/* replacement function for "SAS/C stch_l()" (SAS/C Library Reference page 455)
   built analogous to stcd_l() in ScrambleSer.c
*/
int stch_l(const char *in, long *value)
{
    if (in)
    {
        char *ptr;

        switch (*in)
        {
            case '0':
            case '1':
            case '2':
            case '3':
            case '4':
            case '5':
            case '6':
            case '7':
            case '8':
            case '9':
            case 'A':
            case 'B':
            case 'C':
            case 'D':
            case 'E':
            case 'F':
            case 'a':
            case 'b':
            case 'c':
            case 'd':
            case 'e':
            case 'f':
                *value = strtol(in, &ptr, 16);
                return ptr - in;
                break;
        }
    }

    *value = 0;

    return 0;
}
#endif


int main(int Count, char *Vector[])
{
unsigned long int BinOffset, AscOffset, Serial;
FILE *ToBe;
char Buffer[20];

memset(Buffer, 0, 18); /* overkill */
strcpy(Buffer, Vector[3]);

if(Count == 5)
	{
	if((ToBe = fopen(Vector[4], "r+")))
		{
		BinOffset = atol(Vector[1]);
		AscOffset = atol(Vector[2]);
		if(BinOffset && AscOffset)
			{
			if(!fseek(ToBe, BinOffset, SEEK_SET))
				{
				if(stch_l(Vector[3], (long*)&Serial))
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
			printf("Invalid offset. %lu %lu %s %s\n",
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

