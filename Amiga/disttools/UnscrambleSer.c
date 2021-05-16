/* UnscrambleSer.c
** Takes a scrambled, checked serial number on the command line (in
** hex digits), descrambles it, checks the check bytes and reports the
** final serial number if everything looks good.
** Created from scratch on 03 Aug 1994 by Christopher Eric "Xenon" Hanson
** Copyright ©1994 by CXH, Arcticus, and Questar.
*/

#include <stdio.h>
#include <stdlib.h>

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


/* #define TEST_PHASE */
#define MAGIC_FOOBAR 0x00F00BA2

int main(int Count, char *Vector[])
{
unsigned long int InterMed, Scrambled, PlainSer;
unsigned char CheckByte, TestByte;

if(Count == 2)
	{
	if(stch_l(Vector[1], (long*)&Scrambled))
		{
		InterMed = Scrambled ^= MAGIC_FOOBAR;
		CheckByte = (InterMed & 0xFF000000) >> 24;
		TestByte = (((InterMed & 0x00FF0000) >> 16) ^ ((InterMed & 0x0000FF00) >> 8) ^
		 (InterMed & 0x000000FF));
		PlainSer = InterMed & 0x00FFFFFF;
		#ifdef TEST_PHASE
		printf("Plain: %ld, Check: %d, TestByte: %d.\n", PlainSer, CheckByte,
		 TestByte);
		#endif /* TEST_PHASE */
		if(TestByte == CheckByte)
			{
			printf("Serial number validated. Serial: %ld.\n", PlainSer);
			exit(0);
			} /* if */
		else
			{
			printf("\007"); /* Agent 007: A beep. */
			printf("*** WARNING: Serial number does not validate.\n");
			printf("*** Serial number would be %ld, but user-supplied\n", PlainSer);
			printf("*** check portion %X does not match calculated %X.\n\n",
			 CheckByte, TestByte);
			exit(15);
			} /* else */
		exit(0);
		} /* if */
	else
		{
		printf("Error in hex to integer conversion.\n");
		} /* else */
	} /* if */
else
	{
	printf("Wrong number of args. Read source code for usage.\n");
	} /* else */

return(30);

} /* main() */
