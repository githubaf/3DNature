/* UnscrambleSer.c
** Takes a scrambled, checked serial number on the command line (in
** hex digits), descrambles it, checks the check bytes and reports the
** final serial number if everything looks good.
** Created from scratch on 03 Aug 1994 by Christopher Eric "Xenon" Hanson
** Copyright ©1994 by CXH, Arcticus, and Questar.
*/

/* #define TEST_PHASE */
#define MAGIC_FOOBAR 0x00F00BA2

int main(int Count, char *Vector[])
{
unsigned long int InterMed, Scrambled, PlainSer;
unsigned char CheckByte, TestByte;

if(Count == 2)
	{
	if(stch_l(Vector[1], &Scrambled))
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
