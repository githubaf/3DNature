/* USDEMExtract.c
** A small program for extracting a block of data from the US 30 arc-sec
** DEM data set.
** compile: sc math=68881 link USDEMExtract
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#ifdef __SASC
    #include <m68881.h>
#endif
#include <clib/exec_protos.h>
#include <exec/memory.h>
#include <proto/exec.h>

#define HILAT 49.991666667
#define HILON 129.991666667
#define LOLAT 24.0
#define LOLON 60.0
#define STEPSIZE .008333333
#define ROWSIZE 16800
#define LASTROW 3119
#define LASTCOL 8399

int main(void)
{
char InFile[256], OutFile[256], ans[80];
FILE *fInput, *fOutput;
long i, StartRow, StartCol, EndRow, EndCol, RowSize, ReadSize, ReadPos,
	ReadRows, ReadCols, Ans, error;
double Hilat, Lolat, Hilon, Lolon;
short *Data;

ReSet:

 printf("Input File: ");
 scanf("%s", InFile);
 printf("Output File: ");
 scanf("%s", OutFile);

 printf("High Latitude: ");
 scanf("%le", &Hilat);
 printf("Low Latitude: ");
 scanf("%le", &Lolat);
 printf("High Longitude: ");
 scanf("%le", &Hilon);
 printf("Low Longitude: ");
 scanf("%le", &Lolon);

 if (Hilat < Lolat)
  swmem(&Hilat, &Lolat, sizeof (double));
 if (Hilon < Lolon)
  swmem(&Hilon, &Lolon, sizeof (double));

 if (Hilat > HILAT)
  Hilat = HILAT;
 if (Hilon > HILON)
  Hilon = HILON;
 if (Lolat < LOLAT)
  Lolat = LOLAT;
 if (Lolon < LOLON)
  Lolon = LOLON;

/* compute row and column offsets */

 StartRow = (HILAT - Hilat) / STEPSIZE;
 StartCol = (HILON - Hilon) / STEPSIZE;
 EndRow = (HILAT - Lolat) / STEPSIZE;
 EndCol = (HILON - Lolon) / STEPSIZE;

 ReadRows = EndRow - StartRow + 1;
 ReadCols = EndCol - StartCol + 1;
 RowSize = ReadCols * sizeof (short);
 ReadSize = ReadRows * RowSize;

 printf("Output file will be %d bytes in size.\n", ReadSize);
 printf("Input File: %s\n", InFile);
 printf("Output File: %s\n", OutFile);
 printf("0: Abort,   1: Continue,   2: Reset....: ");
 scanf("%s", ans);
 Ans = atoi(ans);
 if (Ans == 0)
  goto EndAll;
 if (Ans == 2)
  goto ReSet;

 if ((Data = (short *)AllocMem(RowSize, MEMF_ANY)) == NULL)
  {
  printf("Out of Memory allocating %d bytes! Operation terminated.\n", RowSize);
  goto EndAll;
  } /* if out of memory */

 printf("Reading %d rows x %d columns beginning at row %d, col %d\n",
	ReadRows, ReadCols, StartRow, StartCol);

 ReadPos = StartRow * ROWSIZE + StartCol * sizeof (short);

 if ((fInput = fopen(InFile, "rb")) != NULL)
  {
  if ((fOutput = fopen(OutFile, "wb")) != NULL)
   {
   for (i=StartRow; i<=EndRow; i++)
    {
    if ((error = fseek(fInput, ReadPos, SEEK_SET)) < 0)
     {
     printf("Input File SEEK ERROR! Operation terminated.\n");
     break;
     } /* if seek error */
    if ((fread((char *)Data, RowSize, 1, fInput)) != 1)
     {
     printf("Input File READ ERROR! Operation terminated.\n");
     break;
     } /* if read error */
    if ((fwrite((char *)Data, RowSize, 1, fOutput)) != 1)
     {
     printf("Output File WRITE ERROR! Operation terminated.\n");
     break;
     } /* if read error */
    ReadPos += ROWSIZE;
    } /* for i=0... */
   fclose(fOutput);
   } /* if output file opened */
  else
   {
   printf("Can't open output file! Operation terminated.\n");
   } /* else no output file */
  fclose(fInput);
  } /* if input file opened */
 else
  {
  printf("Can't open input file! Operation terminated.\n");
  } /* else no input file */
EndAll:

return 0;

} /* main() */
