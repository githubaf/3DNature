/* FindOffset.c
** Seeks through a file called "WCS.noserial" looking for a particular
** 10-byte sequence specified on the command line.
** Prints to stdout the byte offset of the beginning of that sequence.
** Created from scratch on 03 Aug 1994 by Christopher Eric "Xenon" Hanson
** Copyright 1994 by CXH, Arcticus, and Questar.
*/

/* #define MAGIC_COOKIE "KUBLA_KHAN" */
/* #define TEST_PHASE */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char *memstr(char *Block, unsigned long int BlockSize, char *Cookie);

int main (int Count, char *Vector[])
{
FILE *ExecutableFile, *fOffset;
char *ExecutableMem, *CookiePtr;
unsigned long int CookieOffset, ExecLen;

if((ExecutableFile = fopen("WCS.MATH", "r+")))
	{
	fseek(ExecutableFile, 0, SEEK_END);
	ExecLen = ftell(ExecutableFile);
	if(ExecLen > 500000) /* sanity check */
		{
		if((ExecutableMem = malloc(ExecLen)))
			{
			rewind(ExecutableFile);
			if(fread(ExecutableMem, 1, ExecLen, ExecutableFile) == ExecLen)
				{
				if((CookiePtr = memstr(ExecutableMem, ExecLen, Vector[1])))
					{
					CookieOffset = (CookiePtr - ExecutableMem);
					if(CookieOffset)
						{
						printf("%ld\n", CookieOffset);
						#ifdef TEST_PHASE
						memset(ExecutableMem, 0, 100); /* gimme some room */
						fseek(ExecutableFile, CookieOffset, SEEK_SET);
						fread(ExecutableMem, 1, 10, ExecutableFile);
						printf("[%s]\n", ExecutableMem);
						#endif /* TEST_PHASE */
						if (! strcmp(Vector[1], "KUBLA_KHAN"))
							{
							fOffset = fopen("ENV:ASCOFFSETMATH", "w");
							if (fOffset)
								{
								fprintf(fOffset, "%ld\n", CookieOffset);
								fclose(fOffset);
								} /* if */
							fOffset = fopen("ENVARC:ASCOFFSETMATH", "w");
							if (fOffset)
								{
								fprintf(fOffset, "%ld\n", CookieOffset);
								fclose(fOffset);
								} /* if */
							} /* if ascii offset */
						else if (! strcmp(Vector[1], "C0DEF00F"))
							{
							fOffset = fopen("ENV:BINOFFSETMATH", "w");
							if (fOffset)
								{
								fprintf(fOffset, "%ld\n", CookieOffset);
								fclose(fOffset);
								} /* if */
							fOffset = fopen("ENVARC:BINOFFSETMATH", "w");
							if (fOffset)
								{
								fprintf(fOffset, "%ld\n", CookieOffset);
								fclose(fOffset);
								} /* if */
							} /* if binary offset */
						exit(0);
						} /* if */
					else
						{
						printf("Something funny about cookie offset.\n");
						} /* else */
					} /* if */
				else
					{
					printf("Couldn't locate the magic cookie: \"%s\".\n", Vector[1]);
					} /* else */
				} /* if */
			else
				{
				printf("Couldn't slurp in the whole %ld byte file.\n", ExecLen);
				} /* else */
			} /* if */
		else
			{
			printf("Could allocate %ld bytes of slurp memory.\n", ExecLen);
			} /* else */
		} /* if */
	else
		{
		printf("File appears too short.\n");
		} /* else */
	} /* if */
else
	{
	printf("Couldn't open \"WCS.noserial\".\n");
	} /* else */

exit(30);

} /* main() */


char *memstr(char *Block, unsigned long int BlockSize, char *Cookie)
{
char *SearchPtr;
unsigned long int SearchOffset, CookieOffset, CookieSize;

CookieSize = strlen(Cookie);
SearchPtr = Block;

for(CookieOffset = SearchOffset = 0; SearchOffset < BlockSize; SearchOffset++)
	{
	if(SearchPtr[SearchOffset] == Cookie[CookieOffset])
		{
		CookieOffset++;
		if(CookieOffset == CookieSize)
			{
			return(&SearchPtr[SearchOffset - (CookieOffset - 1)]);
			} /* if */
		} /* if */
	else
		{
		CookieOffset = 0;
		}
	} /* for */

return(NULL);
} /* memstr() */

