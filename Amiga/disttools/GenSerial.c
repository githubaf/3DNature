/* GenSerial.c
** Sucks in a serial number from a file called ".LastSer", increments it,
** and spits it out to stdout and to the ".LastSer" file.
** Created from scratch on 03 Aug 1994 by Christopher Eric "Xenon" Hanson
** Copyright ©1994 by CXH, Arcticus, and Questar
*/

#include <stdio.h>
#include <stdlib.h>

int main(void)
{

FILE *LastSer, *NextSer;
char SerAsc[20];
int CurSer;

LastSer = fopen(".LastSer", "r+");
if(LastSer)
	{
	if(fgets(SerAsc, 15, LastSer))
		{
		CurSer = atoi(SerAsc);
		if(CurSer)
			{
			CurSer++;
			printf("%d\n", CurSer);
			rewind(LastSer);
			fprintf(LastSer, "%d\nIGNORE THIS LINE, IT IS PADDING\n", CurSer);
			fclose(LastSer);
			NextSer = fopen("ENV:UNSCRAM", "w");
			if (NextSer)
				{
				fprintf(NextSer, "%d\n", CurSer);
				fclose(NextSer);
				} /* if */
			NextSer = fopen("ENVARC:UNSCRAM", "w");
			if (NextSer)
				{
				fprintf(NextSer, "%d\n", CurSer);
				fclose(NextSer);
				} /* if */
			exit(0);
			} /* if */
		} /* if */
	} /* if */

exit(30);

} /* main() */
