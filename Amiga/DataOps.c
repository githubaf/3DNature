/* DataOps.c
** Data manipulations for WCS.
** Original code written by Gary R. Huber, March 1994.
*/

#include "WCS.h"
#include "GUIDefines.h"
#include "BigEndianReadWrite.h"

STATIC_FCN short SaveConvertOutput(struct DEMConvertData *data, struct elmapheaderV101 *DEMHdr,
        void *OutputData, long OutputDataSize, short i, short j,
        long rows, long cols, long OutputRows, long OutputCols, char *RGBComp); // used locally only -> static, AF 26.7.2021



/*
EXTERN struct DEMConvertData {
 short	FormatCy[10],
	FormatInt[5],
	OutputMaps[2],
	WrapLon,
	VSOperator,
	ScaleType,
	SplineConstrain,
	ActiveFC[2],
	Crop[4];
 float	LateralScale[4],
	VertScale[9],
	FloorCeiling[2],
	MaxMin[2];
 char	NameBase[24];	
	OutputDir[256];	
} *DEMData;
*/

#define DEM_DATA_INPUT_ARRAY		0
#define DEM_DATA_INPUT_WCSDEM		1
#define DEM_DATA_INPUT_ZBUF		2
#define DEM_DATA_INPUT_ASCII		3
#define DEM_DATA_INPUT_VISTA		4
#define DEM_DATA_INPUT_IFF		5
#define DEM_DATA_INPUT_DTED		6
#define DEM_DATA_OUTPUT_ARRAY		0
#define DEM_DATA_OUTPUT_WCSDEM		1
#define DEM_DATA_OUTPUT_ZBUF		2
#define DEM_DATA_OUTPUT_COLORMAP	3
#define DEM_DATA_OUTPUT_GRAYIFF		4
#define DEM_DATA_OUTPUT_COLORIFF	5
#define DEM_DATA_FORMAT_SIGNEDINT	0
#define DEM_DATA_FORMAT_UNSIGNEDINT	1
#define DEM_DATA_FORMAT_FLOAT		2
#define DEM_DATA_FORMAT_UNKNOWN		3
#define DEM_DATA_VALSIZE_BYTE		0
#define DEM_DATA_VALSIZE_SHORT		1
#define DEM_DATA_VALSIZE_LONG		2
#define DEM_DATA_VALSIZE_DOUBLE		3
#define DEM_DATA_VALSIZE_UNKNOWN	4	
#define DEM_DATA_BYTEORDER_HILO		0
#define DEM_DATA_BYTEORDER_LOHI		1
#define DEM_DATA_BYTEORDER_UNKNOWN	2
#define DEM_DATA_ROW_LAT		0
#define DEM_DATA_ROW_LON		1
#define DEM_DATA_UNITS_KILOM		0
#define DEM_DATA_UNITS_METERS		1
#define DEM_DATA_UNITS_CENTIM		2
#define DEM_DATA_UNITS_MILES		3
#define DEM_DATA_UNITS_FEET		4
#define DEM_DATA_UNITS_INCHES		5
#define DEM_DATA_UNITS_OTHER		6
#define DEM_DATA_READORDER_ROWS		0
#define DEM_DATA_READORDER_COLS		1
#define DEM_DATA_SCALEOP_MATCHMATCH	0
#define DEM_DATA_SCALEOP_MATCHSCALE	1
#define DEM_DATA_SCALEOP_MAXMINSCALE	2
#define DEM_DATA_SCALETYPE_MAXEL	0	
#define DEM_DATA_SCALETYPE_MINEL	1
#define DEM_DATA_SCALETYPE_SCALE	2
#define INPUT_FORMAT		data->FormatCy[0]
#define OUTPUT_FORMAT		data->FormatCy[1]
#define INVALUE_FORMAT		data->FormatCy[2]
#define INVALUE_SIZE		data->FormatCy[3]
#define INBYTE_ORDER		data->FormatCy[4]
#define READ_ORDER		data->FormatCy[5]
#define ROWS_EQUAL		data->FormatCy[6]
#define DATA_UNITS		data->FormatCy[7]
#define INPUT_HEADER		data->FormatInt[0]
#define INPUT_ROWS		data->FormatInt[1]
#define INPUT_COLS		data->FormatInt[2]
#define INPUT_FLOOR		data->FloorCeiling[0]
#define INPUT_CEILING		data->FloorCeiling[1]
#define INPUT_WRAP		data->WrapLon
#define ACTIVE_FLOOR		data->ActiveFC[0]
#define ACTIVE_CEILING		data->ActiveFC[1]
#define CROP_LEFT		data->Crop[0]
#define CROP_RIGHT		data->Crop[1]
#define CROP_TOP		data->Crop[2]
#define CROP_BOTTOM		data->Crop[3]
#define CROP_ROWS		INPUT_ROWS-CROP_TOP-CROP_BOTTOM		
#define CROP_COLS		INPUT_COLS-CROP_LEFT-CROP_RIGHT		

#define OUTPUT_ROWMAPS		data->OutputMaps[0]
#define OUTPUT_COLMAPS		data->OutputMaps[1]
#define OUTPUT_HILAT		data->LateralScale[0]
#define OUTPUT_LOLAT		data->LateralScale[1]
#define OUTPUT_HILON		data->LateralScale[2]
#define OUTPUT_LOLON		data->LateralScale[3]
#define OUTPUT_NAMEBASE		data->NameBase
#define OUTPUT_DIRECTORY	data->OutputDir
#define OUTVALUE_FORMAT		data->FormatCy[8]
#define OUTVALUE_SIZE		data->FormatCy[9]
#define OUTPUT_ROWS		data->FormatInt[3]
#define OUTPUT_COLS		data->FormatInt[4]
#define SPLINE_CONSTRAIN	data->SplineConstrain
#define SCALEOP			data->VSOperator
#define SCALETYPE		data->ScaleType
#define SCALE_MAXEL		data->VertScale[0]
#define SCALE_MINEL		data->VertScale[1]
#define SCALE_VALU1		data->VertScale[2]
#define SCALE_ELEV1		data->VertScale[3]
#define SCALE_SCALE		data->VertScale[4]
#define SCALE_VALU2		data->VertScale[5]
#define SCALE_ELEV2		data->VertScale[6]
#define SCALE_VALU3		data->VertScale[7]
#define SCALE_ELEV3		data->VertScale[8]
#define SCALE_TESTMIN		data->MaxMin[0]
#define SCALE_TESTMAX		data->MaxMin[1]
#define DUPROW			1

void ConvertDEM(struct DEMConvertData *data, char *filename, short TestOnly)
{
 BYTE   *OutputData1S = NULL, *InputData1S = NULL;
 UBYTE  *OutputData1U = NULL, *InputData1U = NULL;
 SHORT	*OutputData2S = NULL, *InputData2S = NULL;
 USHORT *OutputData2U = NULL, *InputData2U = NULL;
 LONG   *OutputData4S = NULL, *InputData4S = NULL;
 ULONG  *OutputData4U = NULL, *InputData4U = NULL;
 FLOAT  *OutputData4F = NULL, *InputData4F = NULL;
 DOUBLE *OutputData8F = NULL, *InputData8F = NULL;

 union ShortSwap {
  short sht;
  BYTE 	byt[2];
 } *ShtSwp;

 union LongSwap {
  long 	lng;
  short sht[2];
  BYTE 	byt[4];
 } *LngSwp;

 union DoubleSwap {
  double dbl;
  long	 lng[2];
  short  sht[4];
  BYTE 	 byt[8];
 } *DblSwp;

 char RGBComponent[5] = {0,0,0,0,0};
 struct elmapheaderV101 *DEMHdr = NULL;
 short i, j, error = 0, DupRow, NoScaling = 0, InValSize;
 long fInput = -1, InputDataSize, OutputDataSize, OutputRows, OutputCols,
	LastOutputRows, LastOutputCols, rows, cols, rowctr, colctr, datazip,
	outzip, BaseOff, FileSize, ORows, OCols;
 float	VertScale, DataRef, OutRef, DataMaxEl = -FLT_MAX, DataMinEl = FLT_MAX;
 double OutValue;
 void *InputData = NULL, *OutputData = NULL;
 FILE *fAscii = NULL;
 struct BusyWindow *BWDC;

//#define  PRINT_CONVERTDEM_PARAMS  // used to create convert-testcases, AF, 17.April 23
#ifdef PRINT_CONVERTDEM_PARAMS
 {
	 printf("AF: %s() %d Filename=%s TestOnly=%d\n",__func__,__LINE__,filename,TestOnly);

	 for(i=0;i< 2;i++) { printf("data->ActiveFC[%d]=%d;\n",i,data->ActiveFC[i]); }
	 for(i=0;i< 4;i++) { printf("data->Crop[%d]=%d;\n",i,data->Crop[i]); }
	 for(i=0;i< 2;i++) { printf("data->FloorCeiling[%d]=%f;\n",i,data->FloorCeiling[i]); }
	 for(i=0;i<10;i++) { printf("data->FormatCy[%d]=%d;\n",i,data->FormatCy[i]); }
	 for(i=0;i< 5;i++) { printf("data->FormatInt[%d]=%d;\n",i,data->FormatInt[i]); }
	 for(i=0;i< 4;i++) { printf("data->LateralScale[%d]=%f;\n",i,data->LateralScale[i]); }
	 for(i=0;i< 2;i++) { printf("data->MaxMin[%d]=%f;\n",i,data->MaxMin[i]); }
	 printf("snprintf(data->NameBase,24,\"%s\");\n",data->NameBase);
	 printf("snprintf(data->OutputDir,256,\"%s\");\n",data->OutputDir);
	 for(i=0;i< 2;i++) { printf("data->OutputMaps[%d]=%d;\n",i,data->OutputMaps[i]); }
	 printf("data->ScaleType=%d;\n",data->ScaleType);
	 printf("data->SplineConstrain=%d;\n",data->SplineConstrain);
	 printf("data->VSOperator=%d;\n",data->VSOperator);
	 for(i=0;i< 9;i++) { printf("data->VertScale[%d]=%f;\n",i,data->VertScale[i]); }
	 printf("data->WrapLon=%d;\n",data->WrapLon);
	 printf("\n");
	 }
#endif

 if (! filename || ! filename[0])
  {
  User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
          (CONST_STRPTR)"You must specify a file to convert!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no row/col sizes */
 if (! OUTPUT_NAMEBASE || ! data->NameBase[0])
  {
  User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
          (CONST_STRPTR)"You must specify an output file name!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no row/col sizes */
 if (INPUT_ROWS == 0 || INPUT_COLS == 0)
  {
  User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
          (CONST_STRPTR)"You must specify input rows and columns!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  return;
  } /* if no row/col sizes */
 if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM)
  {
  if (! dbaseloaded)
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"There is no Database to direct output entities to!\nOperation terminated.",
           (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   return;
   } /* if no database */
  } /* if output DEM to database */

 if (INPUT_WRAP)
	INPUT_COLS ++;
 if (INPUT_FORMAT == DEM_DATA_INPUT_VISTA)
  {
  if (INPUT_ROWS != 258 && INPUT_ROWS != 514 && INPUT_ROWS != 1026)
   INPUT_ROWS = 258;
  INPUT_COLS = INPUT_ROWS;
  } /* if Vista DEM input */

 if (OUTPUT_ROWS == 0)
	OUTPUT_ROWS = CROP_ROWS;
 if (OUTPUT_COLS == 0)
	OUTPUT_COLS = CROP_COLS;

 if (OUTPUT_ROWMAPS <= 0)
	OUTPUT_ROWMAPS = 1;
 if (OUTPUT_COLMAPS <= 0)
	OUTPUT_COLMAPS = 1;

 ORows = OUTPUT_ROWS;
 OCols = OUTPUT_COLS;
 if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM || INPUT_FORMAT == DEM_DATA_INPUT_DTED)
  swmem(&data->FormatInt[3], &data->FormatInt[4], sizeof (short));

/*
** Compute output array sizes
*/
 switch (OUTPUT_FORMAT)
  {
  case DEM_DATA_OUTPUT_WCSDEM:
  case DEM_DATA_OUTPUT_COLORMAP:
   {
   LastOutputRows = OutputRows = 1 + (OUTPUT_ROWS - 1) / OUTPUT_COLMAPS;
   LastOutputCols = OutputCols = 1 + (OUTPUT_COLS - 1) / OUTPUT_ROWMAPS;
   DupRow = 1;
   break;
   } /* WCS DEM output */
  case DEM_DATA_OUTPUT_ZBUF:
  case DEM_DATA_OUTPUT_GRAYIFF:
  case DEM_DATA_OUTPUT_COLORIFF:
  case DEM_DATA_OUTPUT_ARRAY:
   {
   LastOutputRows = OutputRows = OUTPUT_ROWS / OUTPUT_COLMAPS;
   LastOutputCols = OutputCols = OUTPUT_COLS / OUTPUT_ROWMAPS;
   DupRow = 0;
   break;
   } /* IFF or Array output */
  } /* switch output format */


 if (((OUTPUT_ROWS - DupRow) % OUTPUT_COLMAPS) || ((OUTPUT_COLS - DupRow) % OUTPUT_ROWMAPS))
  {
  short ans;

  LastOutputRows = OutputRows - DupRow + OUTPUT_ROWS - (OutputRows - DupRow) * OUTPUT_COLMAPS;
  LastOutputCols = OutputCols - DupRow + OUTPUT_COLS - (OutputCols - DupRow) * OUTPUT_ROWMAPS;
  sprintf(str, "Input data cannot be equally divided among output maps.\nLast Column of maps will have %ld columns.\nLast Row of maps will have %ld rows.",
	LastOutputCols, LastOutputRows);
  if ((ans = User_Message_Def((CONST_STRPTR)"Data Ops: Convert DEM", (CONST_STRPTR)str,
          (CONST_STRPTR)"Continue|Truncate|Cancel", (CONST_STRPTR)"ntc", 1)) == 0)
   {
   return;
   } /* if cancel */
  if (ans == 2)
   {
   LastOutputRows = OutputRows;
   LastOutputCols = OutputCols;
   } /* if truncate input */
  } /* if too many rows or columns */

 if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP)
  {
#ifdef OLD_COLORMAP  // AF, 18.April 23 Old code does NOT generate 3 files with .red, .grn and .blu but only one without suffix. See also below!
  short length;

  length = strlen(filename) - 3;             // file is the INPUT(!) file name. Has nothing to do with the resulting file! And what if filename-length is shorter 3 ???
  if (! stricmp("red", &filename[length]))   // INPUT filename will usually not end in "red"! So no red, grn and blu suffixes will ever be generated
   {
   strcpy(RGBComponent, ".red");
   } /* if red component file */
#else
  strcpy(RGBComponent, ".red");
#endif
  } /* if output color map */

RepeatRGB:

 if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM
	 || INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM)
  {
  if ((DEMHdr = (struct elmapheaderV101 *)
		get_Memory(sizeof (struct elmapheaderV101), MEMF_CLEAR)) == NULL)
   {
   error = 1;
   goto Cleanup;
   } /* if out of memory */
  } /* if WCS DEM input or output */


/*
** Input data
*/
 if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_VISTA
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
  {
  INVALUE_SIZE = DEM_DATA_VALSIZE_SHORT;
  INVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT;
  } /* if WCS DEM input */
 else if (INPUT_FORMAT == DEM_DATA_INPUT_IFF)
  {
  INVALUE_SIZE = DEM_DATA_VALSIZE_BYTE;
  INVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
  } /* if WCS DEM input */

 InputDataSize = INPUT_ROWS * INPUT_COLS;

 switch (INVALUE_SIZE)
  {
  case DEM_DATA_VALSIZE_BYTE:
   {
   switch (INVALUE_FORMAT)
    {
    case DEM_DATA_FORMAT_UNSIGNEDINT:
     {
     if ((InputData1U = (UBYTE *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      } /* if out of memory */
     InputData = InputData1U;
     break;
     } /* unsigned byte */
    case DEM_DATA_FORMAT_SIGNEDINT:
     {
     if ((InputData1S = (BYTE *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      } /* if out of memory */
     InputData = InputData1S;
     break;
     } /* signed byte */
    default:
     {
     error = 11;
     goto Cleanup;
     break;
     } /* floating point byte or unknown format */
    } /* switch value format */
   InValSize = 1;
   break;
   } /* byte */
  case DEM_DATA_VALSIZE_SHORT:
   {
   InputDataSize *= 2;
   if (INPUT_FORMAT != DEM_DATA_INPUT_WCSDEM)
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      if ((InputData2U = (USHORT *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      InputData = InputData2U;
      ShtSwp = (union ShortSwap *)InputData2U;
      break;
      } /* unsigned short */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      if ((InputData2S = (SHORT *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      InputData = InputData2S;
      ShtSwp = (union ShortSwap *)InputData2S;
      break;
      } /* signed short */
     default:
      {
      error = 11;
      goto Cleanup;
      break;
      } /* floating point short or unknown format */
     } /* switch value format */
    } /* if not WCS DEM input (DEMs will be read later) */
   InValSize = 2;
   break;
   } /* short */
  case DEM_DATA_VALSIZE_LONG:
   {
   InputDataSize *= 4;
   switch (INVALUE_FORMAT)
    {
    case DEM_DATA_FORMAT_FLOAT:
     {
     if ((InputData4F = (FLOAT *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      }
     InputData = InputData4F;
     LngSwp = (union LongSwap *)InputData4F;
     break;
     } /* floating point long */
    case DEM_DATA_FORMAT_UNSIGNEDINT:
     {
     if ((InputData4U = (ULONG *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      }
     InputData = InputData4U;
     LngSwp = (union LongSwap *)InputData4U;
     break;
     } /* unsigned long */
    case DEM_DATA_FORMAT_SIGNEDINT:
     {
     if ((InputData4S = (LONG *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      }
     InputData = InputData4S;
     LngSwp = (union LongSwap *)InputData4S;
     break;
     } /* signed long */
    default:
     {
     error = 11;
     goto Cleanup;
     break;
     } /* unknown format */
    } /* switch value format */
   InValSize = 4;
   break;
   } /* long */
  case DEM_DATA_VALSIZE_DOUBLE:
   {
   InputDataSize *= 8;
   switch (INVALUE_FORMAT)
    {
    case DEM_DATA_FORMAT_FLOAT:
     {
     if ((InputData8F = (DOUBLE *)get_Memory(InputDataSize, MEMF_ANY)) == NULL)
      {
      error = 1;
      goto Cleanup;
      }
     InputData = InputData8F;
     DblSwp = (union DoubleSwap *)InputData8F;
     break;
     } /* floating point double */
    default:
     {
     error = 11;
     goto Cleanup;
     break;
     } /* integer double or unknown format */
    } /* switch value format */
   InValSize = 8;
   break;
   } /* double */
  case DEM_DATA_VALSIZE_UNKNOWN:
   {
   error = 10;
   goto Cleanup;
   break;
   } /* unknown value size */
  } /* switch value size */

 if (INPUT_FORMAT == DEM_DATA_INPUT_VISTA)
  {
  error = LoadVistaDEM(filename, (short *)InputData, (short)INPUT_COLS);
  goto EndLoad;
  } /* if Vista DEM */

 if (INPUT_FORMAT == DEM_DATA_INPUT_DTED)
  {
  error = LoadDTED(filename, (short *)InputData, InputDataSize);
  goto EndLoad;
  } /* if Vista DEM */

 if (INPUT_FORMAT == DEM_DATA_INPUT_IFF)
  {
  UBYTE *BitMap[3];
  short DummyShort;
  long ct, planes = 24;

/* try 24 bit */
  if ((error = openbitmaps(BitMap, InputDataSize)) == 0)
   {
   if (LoadImage(filename, 1, BitMap,
	 (INPUT_WRAP ? INPUT_COLS - 1: INPUT_COLS), INPUT_ROWS, 0,
	 &DummyShort, &DummyShort, &DummyShort))
    {
    for (ct=0; ct<InputDataSize; ct++)
     {
     InputData1U[ct] = ((unsigned int)*(BitMap[0] + ct)
	 + (unsigned int)*(BitMap[1] + ct) + (unsigned int)*(BitMap[2] + ct)) / 3;
     } /* for ct=0... */
    } /* if 24 bit image loaded OK */
   else
    {
    planes = 8;
    } /* else */
   closebitmaps(BitMap, InputDataSize);
   } /* if allocated 24 bit image */  
  else
   planes = 8;

/* 24 bit failed, try 8 bit */
  if (planes == 8)
   {
   if ((BitMap[0] = (UBYTE *)get_Memory(InputDataSize, MEMF_ANY)) != NULL)
    {
    if (LoadImage(filename, 0, BitMap,
	 (INPUT_WRAP ? INPUT_COLS - 1: INPUT_COLS), INPUT_ROWS, 0,
	 &DummyShort, &DummyShort, &DummyShort))
     {
     for (ct=0; ct<InputDataSize; ct++)
      {
      InputData1U[ct] = *(BitMap[0] + ct);
      } /* for ct=0... */
     } /* if image loaded OK */
    else
     {
     error = 6;
     free_Memory(BitMap[0], InputDataSize);
     goto EndLoad;
     } /* else */
    free_Memory(BitMap[0], InputDataSize);
    } /* if 8 bit memory allocated */
   else
    {
    error = 1;
    goto EndLoad;
    } /* else */
   } /* if planes = 8 */

  if (INPUT_WRAP)
   {
   IPTR Source, Dest;
   ULONG InputRowSize, FullRowSize;

   InputRowSize = (INPUT_COLS - 1) * InValSize;
   FullRowSize = InputRowSize + InValSize;
   for (i=INPUT_ROWS-1; i>=0; i--)
    {
    Source = (IPTR)InputData + i * InputRowSize;
    Dest = (IPTR)InputData + i * FullRowSize;
    if (i > 0)
     memmove((char *)Dest, (char *)Source, InputRowSize);
    memcpy((char *)(Dest + InputRowSize), (char *)Dest, InValSize);
    } /* for i=0... */
   } /* if wrap longitude */
  goto EndLoad;
  } /* if IFF */

 if (INPUT_FORMAT == DEM_DATA_INPUT_ASCII)
  {
  if ((fAscii = fopen(filename, "r")) == NULL)
   {
   error = 2;
   goto Cleanup;
   } /* if open fail */
  } /* if ascii file */
 else
  {
  if ((fInput = open(filename, O_RDONLY)) < 0)
   {
   error = 2;
   goto Cleanup;
   } /* if open fail */
  FileSize = lseek(fInput, 0L, 2);
  } /* else binary file */

/* check file size */
 if (((! INPUT_WRAP && FileSize != INPUT_HEADER + INPUT_ROWS * INPUT_COLS * InValSize)
	|| (INPUT_WRAP && FileSize != INPUT_HEADER + INPUT_ROWS * (INPUT_COLS - 1) * InValSize))
	&& (INPUT_FORMAT != DEM_DATA_INPUT_ASCII))
  {
  if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM)
   {
   long tempversion;

   lseek(fInput, 0L, 0);
   read(fInput, &tempversion, 4);
   if (tempversion < 1.0 || tempversion > DEM_CURRENT_VERSION)
    {
    lseek(fInput, 0L, 0);
    read(fInput, DEMHdr, ELEVHDRLENV100);
    if (DEMHdr->rows != INPUT_ROWS - 1 || DEMHdr->columns != INPUT_COLS)
     {
     error = 3;
     } /* if wrong header info */    
    } /* if very old DEM file version */
   else
    {
    error = 3;
    } /* else recent version */
   } /* if WCS DEM input */
  else
   {
   if ((! INPUT_WRAP && FileSize > INPUT_HEADER + INPUT_ROWS * INPUT_COLS * InValSize)
	|| (INPUT_WRAP && FileSize > INPUT_HEADER + INPUT_ROWS * (INPUT_COLS - 1) * InValSize))
    {
    if (! User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
            (CONST_STRPTR)"Incorrect file size for specified header, width and height!\nProceed anyway?.", (CONST_STRPTR)"OK|Cancel", (CONST_STRPTR)"oc"))
     {
     error = 13;
     } /* if cancel operation */
    } /* if file size greater than needed */
   else
    {
    error = 3;
    } /* else file size less than specified dimensions */
   } /* else not WCS DEM input */
  if (error)
   {
   close(fInput);
   goto Cleanup;
   } /* if wrong file size */
  } /* if file size incorrect */

 if (INPUT_HEADER)
  {
  if (INPUT_FORMAT == DEM_DATA_INPUT_ASCII)
   {
   if (fseek(fAscii, INPUT_HEADER, SEEK_SET) == 0)
    {
    error = 3;
    fclose(fAscii);
    } /* if wrong file size */
   } /* if ascii file */
  else
   {
   if (lseek(fInput, INPUT_HEADER, 0) < 0)
    {
    error = 3;
    close(fInput);
    } /* if wrong file size */
   } /* else binary file */
  } /* if header */
 else if (INPUT_FORMAT != DEM_DATA_INPUT_ASCII)
  {
  lseek(fInput, 0L, 0);
  } /* else no binary header - ascii files do not need to be rewound */
 if (error)
  goto Cleanup;

 switch (INPUT_FORMAT)
  {
  case DEM_DATA_INPUT_ARRAY:
  case DEM_DATA_INPUT_ZBUF:
   {
   if (! INPUT_WRAP)
    {
	   int ReadResult;
	   if(INPUT_FORMAT==DEM_DATA_INPUT_ZBUF)  // AF: ZBUF is an IFF format format with float values
	   {
		   ReadResult=read_float_Array_BE(fInput, InputData, InputDataSize);
	   }
	   else // Format is DEM_DATA_INPUT_ARRAY. ToDO: Distinguish between 2,4,8 bytes (un)signed, float, double!
	   {
		   ReadResult=read(fInput, InputData, InputDataSize);
	   }

	   if (ReadResult != InputDataSize)  // AF: 29.Mar.23
	   {
		   error = 3;
	   } /* if read fail */
    } /* if no wrap longitude */
   else // INPUT_WRAP
   {
	   IPTR Source, Dest;
	   ULONG InputRowSize, FullRowSize;

	   int ReadResult;
	   if(INPUT_FORMAT==DEM_DATA_INPUT_ZBUF)  // AF: ZBUF is an IFF format format with float values
	   {
		   ReadResult=read_float_Array_BE(fInput, InputData, (InputDataSize - INPUT_ROWS * InValSize));
	   }
	   else // Format is DEM_DATA_INPUT_ARRAY. ToDO: Distinguish between 2,4,8 bytes (un)signed, float, double!
	   {
		   ReadResult=read(fInput, InputData, (InputDataSize - INPUT_ROWS * InValSize));
	   }

    if (ReadResult != InputDataSize - INPUT_ROWS * InValSize)
     {
     error = 3;
     } /* if read fail */
    InputRowSize = (INPUT_COLS - 1) * InValSize;
    FullRowSize = InputRowSize + InValSize;
    for (i=INPUT_ROWS-1; i>=0; i--)
     {
     Source = (IPTR)InputData + i * InputRowSize;
     Dest = (IPTR)InputData + i * FullRowSize;
     if (i > 0)
      memmove((char *)Dest, (char *)Source, InputRowSize);
     memcpy((char *)(Dest + InputRowSize), (char *)Dest, InValSize);
     } /* for i=0... */
    } /* else wrap longitude */
   close(fInput);
   break;
   } /* array */
  case DEM_DATA_INPUT_WCSDEM:
   {
   close(fInput);
   if (readDEM(filename, DEMHdr))
    {
    error = 1;
    break;
    } /* if read fail */
   InputData = InputData2S = DEMHdr->map;
   if (InputDataSize != DEMHdr->size)
    {
    InputDataSize = DEMHdr->size;
    error = 3;
    goto Cleanup;
    } /* if wrong file size */
   InputDataSize = DEMHdr->size;
   INPUT_ROWS = DEMHdr->rows + 1;
   INPUT_COLS = DEMHdr->columns;
   if (OUTPUT_LOLAT == OUTPUT_HILAT || OUTPUT_HILON == OUTPUT_LOLON)
    {
    OUTPUT_LOLAT = DEMHdr->lolat;
    OUTPUT_HILAT = DEMHdr->lolat + (DEMHdr->columns - 1) * DEMHdr->steplat;
    OUTPUT_HILON = DEMHdr->lolong;
    OUTPUT_LOLON = DEMHdr->lolong - DEMHdr->rows * DEMHdr->steplong;
    } /* if no useful values entered in lat/lon table */
   if (OUTPUT_LOLAT > OUTPUT_HILAT)
    swmem(&data->LateralScale[0], &data->LateralScale[1], sizeof (float));
   if (OUTPUT_LOLON > OUTPUT_HILON)
    swmem(&data->LateralScale[2], &data->LateralScale[3], sizeof (float));
   if (OUTPUT_HILAT > 90.0 && OUTPUT_HILON < 90.0)
    {
    swmem(&data->LateralScale[0], &data->LateralScale[2], sizeof (float));
    swmem(&data->LateralScale[1], &data->LateralScale[3], sizeof (float));
    } /* user got latitude and longitude reversed */
   break;
   } /* WCS DEM */
  case DEM_DATA_INPUT_IFF:
   {
   error = 10;
   close(fInput);
   break;
   } /* iff */
  case DEM_DATA_INPUT_ASCII:
   {
   long InputValues, ct;
   char InputChar, InValue[64];

   if (! INPUT_WRAP)
    InputValues = INPUT_ROWS * INPUT_COLS;
   else
    InputValues = INPUT_ROWS * (INPUT_COLS - 1);
   for (ct=0; ct<InputValues; ct++)
    {
ReadMore:
    if ((InputChar = fgetc(fAscii)) == EOF)
     {
     error = 3;
     break;
     }
    if (InputChar < 45 || InputChar > 57)  // AF: < '-'   || >  '9'  i.e. in "-0123456789"
     goto ReadMore;
    InValue[0] = InputChar;
    j = 0;
    while (j < 63 && InValue[j] > 44 && InValue[j] < 58 && InValue[j] != EOF)  //AF:  > ','  && < ':' i.e. in "-0123456789"
     {
     j ++;
     InValue[j] = fgetc(fAscii);
     } /*while */
    if (j >= 63) // max 63 characters + final \0
     {
     error = 3;
     break;
     }
    InValue[j] = '\0';

    switch (INVALUE_SIZE)
     {
     case DEM_DATA_VALSIZE_BYTE:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        InputData1U[ct] = (UBYTE)(atoi(InValue));
        break;
        } /* unsigned byte */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        InputData1S[ct] = (BYTE)(atoi(InValue));
        break;
        } /* signed byte */
       } /* switch value format */
      break;
      } /* byte */
     case DEM_DATA_VALSIZE_SHORT:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        InputData2U[ct] = (USHORT)(atoi(InValue));
        break;
        } /* unsigned short */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        InputData2S[ct] = (SHORT)(atoi(InValue));
        break;
        } /* signed short */
       } /* switch value format */
      break;
      } /* short */
     case DEM_DATA_VALSIZE_LONG:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_FLOAT:
        {
        InputData4F[ct] = (FLOAT)(atof(InValue));
        break;
        } /* floating point long */
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        InputData4U[ct] = (ULONG)(atoi(InValue));
        break;
        } /* unsigned long */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        InputData4S[ct] = (LONG)(atoi(InValue));
        break;
        } /* signed long */
       } /* switch value format */
      break;
      } /* long */
     case DEM_DATA_VALSIZE_DOUBLE:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_FLOAT:
        {
        InputData8F[ct] = (DOUBLE)(atof(InValue));
        break;
        } /* floating point double */
       } /* switch value format */
      break;
      } /* double */
     } /* switch value size */

    } /* for i=0... */
   fclose(fAscii);
   if (INPUT_WRAP)
    {
    IPTR Source, Dest;
    ULONG InputRowSize, FullRowSize;

    InputRowSize = (INPUT_COLS - 1) * InValSize;
    FullRowSize = InputRowSize + InValSize;
    for (i=INPUT_ROWS-1; i>=0; i--)
     {
     Source = (IPTR)InputData + i * InputRowSize;
     Dest = (IPTR)InputData + i * FullRowSize;
     if (i > 0)
      memmove((char *)Dest, (char *)Source, InputRowSize);
     memcpy((char *)(Dest + InputRowSize), (char *)Dest, InValSize);
     } /* for i=0... */
    } /* if wrap longitude */

/* invert file if it is stored SE corner to NW */
   if (User_Message((CONST_STRPTR)"Data Ops: Convert DEM", (CONST_STRPTR)"Invert Data order?", (CONST_STRPTR)"Yes|No", (CONST_STRPTR)"yn"))
    {
    long DataPts, ct;
    char *LowPtr, *HighPtr;

    DataPts = INPUT_ROWS * INPUT_COLS;
    LowPtr = (char *)InputData;
    HighPtr = LowPtr + (DataPts - 1) * InValSize;
    DataPts /= 2;
    for (ct=0; ct<DataPts; ct++, LowPtr+=InValSize, HighPtr-=InValSize)
     {
     swmem(LowPtr, HighPtr, InValSize);
     } /* for ct=0... */
    } /* if invert data */
   break;
   } /* ascii array */

  } /* switch input file format */

EndLoad:

 if (error)
  goto Cleanup;

/*
** Invert data if Intel format (Low-High byte order)
*/
 if (INBYTE_ORDER == DEM_DATA_BYTEORDER_LOHI
	 && INVALUE_SIZE != DEM_DATA_VALSIZE_BYTE)
  {
  BWDC = BusyWin_New("Inverting", INPUT_ROWS, 0, MakeID('B','W','D','C'));
  datazip = 0;
  for (i=0; i<INPUT_ROWS; i++)
   {
   for (j=0; j<INPUT_COLS; j++)
    {
    switch (INVALUE_SIZE)
     {
     case DEM_DATA_VALSIZE_SHORT:
      {
      swmem(&ShtSwp[datazip].byt[0], &ShtSwp[datazip].byt[1], 1);
      break;
      } /* short */
     case DEM_DATA_VALSIZE_LONG:
      {
      swmem(&LngSwp[datazip].byt[0], &LngSwp[datazip].byt[3], 1);
      swmem(&LngSwp[datazip].byt[1], &LngSwp[datazip].byt[2], 1);
      break;
      } /* short */
     case DEM_DATA_VALSIZE_DOUBLE:
      {
      swmem(&DblSwp[datazip].byt[0], &DblSwp[datazip].byt[7], 1);
      swmem(&DblSwp[datazip].byt[1], &DblSwp[datazip].byt[6], 1);
      swmem(&DblSwp[datazip].byt[2], &DblSwp[datazip].byt[5], 1);
      swmem(&DblSwp[datazip].byt[3], &DblSwp[datazip].byt[4], 1);
      break;
      } /* short */
     } /* switch value size */
    datazip ++;
    } /* for j=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
   if (BWDC)
    BusyWin_Update(BWDC, i + 1);
   } /* for i=0... */
  if (BWDC) BusyWin_Del(BWDC);

  } /* if must reverse byte order for Motorola processor */
 if (error)
  goto Cleanup;

/*
** Apply floor and ceiling values
*/
 if (ACTIVE_FLOOR)
  {
  BWDC = BusyWin_New("Floor", INPUT_ROWS, 0, MakeID('B','W','D','C'));
  datazip = 0;
  for (i=0; i<INPUT_ROWS; i++)
   {
   for (j=0; j<INPUT_COLS; j++)
    {
    switch (INVALUE_SIZE)
     {
     case DEM_DATA_VALSIZE_BYTE:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData1U[datazip] < INPUT_FLOOR)
		InputData1U[datazip] = INPUT_FLOOR;
        break;
	} /* unsigned byte */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData1S[datazip] < INPUT_FLOOR)
		InputData1S[datazip] = INPUT_FLOOR;
        break;
	} /* unsigned byte */
       } /* switch value format */
      break;
      } /* byte */
     case DEM_DATA_VALSIZE_SHORT:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData2U[datazip] < INPUT_FLOOR)
		InputData2U[datazip] = INPUT_FLOOR;
        break;
	} /* unsigned short */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData2S[datazip] < INPUT_FLOOR)
		InputData2S[datazip] = INPUT_FLOOR;
        break;
	} /* signed short */
       } /* switch value format */
      break;
      } /* short int */
     case DEM_DATA_VALSIZE_LONG:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_FLOAT:
        {
        if (InputData4F[datazip] < INPUT_FLOOR)
		InputData4F[datazip] = INPUT_FLOOR;
        break;
	} /* floating point long */
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData4U[datazip] < INPUT_FLOOR)
		InputData4U[datazip] = INPUT_FLOOR;
        break;
	} /* unsigned long integer */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData4S[datazip] < INPUT_FLOOR)
		InputData4S[datazip] = INPUT_FLOOR;
        break;
	} /* signed long integer */
       } /* switch value format */
      break;
      } /* long word */
     case DEM_DATA_VALSIZE_DOUBLE:
      {
      if (InputData8F[datazip] < INPUT_FLOOR)
		InputData8F[datazip] = INPUT_FLOOR;
      break;
      } /* double */
     } /* switch input value size */
    datazip ++;
    } /* for j=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
   if (BWDC)
    BusyWin_Update(BWDC, i + 1);
   } /* for i=0... */
  if (BWDC) BusyWin_Del(BWDC);
  } /* if active floor */
 if (error)
  goto Cleanup;

 if (ACTIVE_CEILING)
  {
  BWDC = BusyWin_New("Ceiling", INPUT_ROWS, 0, MakeID('B','W','D','C'));
  datazip = 0;
  for (i=0; i<INPUT_ROWS; i++)
   {
   for (j=0; j<INPUT_COLS; j++)
    {
    switch (INVALUE_SIZE)
     {
     case DEM_DATA_VALSIZE_BYTE:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData1U[datazip] > INPUT_CEILING)
		InputData1U[datazip] = INPUT_CEILING;
        break;
	} /* unsigned byte */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData1S[datazip] > INPUT_CEILING)
		InputData1S[datazip] = INPUT_CEILING;
        break;
	} /* unsigned byte */
       } /* switch value format */
      break;
      } /* byte */
     case DEM_DATA_VALSIZE_SHORT:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData2U[datazip] > INPUT_CEILING)
		InputData2U[datazip] = INPUT_CEILING;
        break;
	} /* unsigned short */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData2S[datazip] > INPUT_CEILING)
		InputData2S[datazip] = INPUT_CEILING;
        break;
	} /* signed short */
       } /* switch value format */
      break;
      } /* short int */
     case DEM_DATA_VALSIZE_LONG:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_FLOAT:
        {
        if (InputData4F[datazip] > INPUT_CEILING)
		InputData4F[datazip] = INPUT_CEILING;
        break;
	} /* floating point long */
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData4U[datazip] > INPUT_CEILING)
		InputData4U[datazip] = INPUT_CEILING;
        break;
	} /* unsigned long integer */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData4S[datazip] > INPUT_CEILING)
		InputData4S[datazip] = INPUT_CEILING;
        break;
	} /* signed long integer */
       } /* switch value format */
      break;
      } /* long word */
     case DEM_DATA_VALSIZE_DOUBLE:
      {
      if (InputData8F[datazip] > INPUT_CEILING)
		InputData8F[datazip] = INPUT_CEILING;
      break;
      } /* double */
     } /* switch input value size */
    datazip ++;
    } /* for j=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
   if (BWDC)
    BusyWin_Update(BWDC, i + 1);
   } /* for i=0... */
  if (BWDC) BusyWin_Del(BWDC);
  } /* if active ceiling */
 if (error)
  goto Cleanup;

/*
** Re-Sample to output size
*/
 if (CROP_ROWS != ORows || CROP_COLS != OCols)
  {
  short k;
  long RowBase[4], LastInRow, LastInCol, LastOutRow, LastOutCol, CurRow[4], CurCol[4];
  double RowStep, ColStep, RowDelta, ColDelta, TP[4],
	P0, P1, P2, P3, D1, D2, S1, S2, S3, h1, h2, h3, h4;

  OutputDataSize = ORows * OCols;

  switch (INVALUE_SIZE)
   {
   case DEM_DATA_VALSIZE_BYTE:
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      if ((OutputData1U = (UBYTE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       } /* if out of memory */
      OutputData = (UBYTE *)OutputData1U;
      break;
      } /* unsigned byte */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      if ((OutputData1S = (BYTE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       } /* if out of memory */
      OutputData = (BYTE *)OutputData1S;
      break;
      } /* signed byte */
     default:
      {
      error = 11;
      goto Cleanup;
      break;
      } /* floating point byte or unknown format */
     } /* switch value format */
    break;
    } /* byte */
   case DEM_DATA_VALSIZE_SHORT:
    {
    OutputDataSize *= 2;
/*    if (INPUT_FORMAT != DEM_DATA_INPUT_WCSDEM)
     {*/
     switch (INVALUE_FORMAT)
      {
      case DEM_DATA_FORMAT_UNSIGNEDINT:
       {
       if ((OutputData2U = (USHORT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = (USHORT *)OutputData2U;
       break;
       } /* unsigned short */
      case DEM_DATA_FORMAT_SIGNEDINT:
       {
       if ((OutputData2S = (SHORT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = (SHORT *)OutputData2S;
       break;
       } /* signed short */
      default:
       {
       error = 11;
       goto Cleanup;
       break;
       } /* floating point short or unknown format */
      } /* switch value format */
/*     } // if not WCS DEM input (DEMs will be read later)
*/
    break;
    } /* short */
   case DEM_DATA_VALSIZE_LONG:
    {
    OutputDataSize *= 4;
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_FLOAT:
      {
      if ((OutputData4F = (FLOAT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      OutputData = (FLOAT *)OutputData4F;
      break;
      } /* floating point long */
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      if ((OutputData4U = (ULONG *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      OutputData = (ULONG *)OutputData4U;
      break;
      } /* unsigned long */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      if ((OutputData4S = (LONG *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      OutputData = (LONG *)OutputData4S;
      break;
      } /* signed long */
     default:
      {
      error = 11;
      goto Cleanup;
      break;
      } /* unknown format */
     } /* switch value format */
    break;
    } /* long */
   case DEM_DATA_VALSIZE_DOUBLE:
    {
    OutputDataSize *= 8;
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_FLOAT:
      {
      if ((OutputData8F = (DOUBLE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
       {
       error = 1;
       goto Cleanup;
       }
      OutputData = (double *)OutputData8F;
      break;
      } /* floating point double */
     default:
      {
      error = 11;
      goto Cleanup;
      break;
      } /* integer double or unknown format */
     } /* switch value format */
    break;
    } /* double */
   } /* switch value size */

  RowStep = (float)(CROP_ROWS - 1) / (float)(ORows - 1);
  ColStep = (float)(CROP_COLS - 1) / (float)(OCols - 1);
  LastInRow = INPUT_ROWS - CROP_BOTTOM - 1;
  LastInCol = INPUT_COLS - CROP_RIGHT - 1;
  LastOutRow = ORows - 1;
  LastOutCol = OCols - 1;

  BWDC = BusyWin_New("Resample", ORows, 0, MakeID('B','W','D','C'));
  for (i=0; i<OCols; i++)
   {
   if (i == LastOutRow)
    CurRow[0] = CurRow[1] = CurRow[2] = CurRow[3] = LastInRow;
   else
    CurRow[0] = CurRow[1] = CurRow[2] = CurRow[3] = CROP_TOP + i * RowStep;
   CurRow[0] --;
   CurRow[2] ++;
   CurRow[3] += 2;
   RowDelta = i * RowStep - (float)(CurRow[1] - CROP_TOP);
   RowBase[1] = CurRow[1] * INPUT_COLS;
   RowBase[0] = RowBase[1] - INPUT_COLS;
   RowBase[2] = RowBase[1] + INPUT_COLS;
   RowBase[3] = RowBase[2] + INPUT_COLS;

   if (i == 0 || i == LastOutRow || RowDelta == 0.0)
    {
    for (j=0; j<OUTPUT_COLS; j++)
     {
     if (j == LastOutCol)
      CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = LastInCol;
     else
      CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = CROP_LEFT + j * ColStep;
     CurCol[0] --;
     CurCol[2] ++;
     CurCol[3] += 2;
     ColDelta = j * ColStep - (float)(CurCol[1] - CROP_LEFT);
     if (j == 0 || j == LastOutCol || ColDelta == 0.0)
      {
      switch (INVALUE_SIZE)
       {
       case DEM_DATA_VALSIZE_BYTE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData1U[i * OUTPUT_COLS + j] =
		 InputData1U[RowBase[1] + CurCol[1]];
          break;
          } /* unsigned byte */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData1S[i * OUTPUT_COLS + j] =
		 InputData1S[RowBase[1] + CurCol[1]];
          break;
          } /* signed byte */
         } /* switch value format */
        break;
        } /* byte */
       case DEM_DATA_VALSIZE_SHORT:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData2U[i * OUTPUT_COLS + j] =
		 InputData2U[RowBase[1] + CurCol[1]];
          break;
          } /* unsigned short */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData2S[i * OUTPUT_COLS + j] =
		 InputData2S[RowBase[1] + CurCol[1]];
          break;
          } /* signed short */
         } /* switch value format */
        break;
        } /* short */
       case DEM_DATA_VALSIZE_LONG:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          OutputData4F[i * OUTPUT_COLS + j] =
		 InputData4F[RowBase[1] + CurCol[1]];
          break;
          } /* floating point long */
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData4U[i * OUTPUT_COLS + j] =
		 InputData4U[RowBase[1] + CurCol[1]];
          break;
          } /* unsigned long */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData4S[i * OUTPUT_COLS + j] =
		 InputData4S[RowBase[1] + CurCol[1]];
          break;
          } /* signed long */
         } /* switch value format */
        break;
        } /* long */
       case DEM_DATA_VALSIZE_DOUBLE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          OutputData8F[i * OUTPUT_COLS + j] =
		 InputData8F[RowBase[1] + CurCol[1]];
          break;
          } /* floating point double */
         } /* switch value format */
        break;
        } /* double */
       } /* switch value size */
      } /* set value directly */
     else
      {
      switch (INVALUE_SIZE)
       {
       case DEM_DATA_VALSIZE_BYTE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          P1 = InputData1U[RowBase[1] + CurCol[1]];
          P2 = InputData1U[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData1U[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData1U[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* unsigned byte */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          P1 = InputData1S[RowBase[1] + CurCol[1]];
          P2 = InputData1S[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData1S[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData1S[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* signed byte */
         } /* switch value format */
        break;
        } /* byte */
       case DEM_DATA_VALSIZE_SHORT:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          P1 = InputData2U[RowBase[1] + CurCol[1]];
          P2 = InputData2U[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData2U[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData2U[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* unsigned short */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          P1 = InputData2S[RowBase[1] + CurCol[1]];
          P2 = InputData2S[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData2S[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData2S[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* signed short */
         } /* switch value format */
        break;
        } /* short */
       case DEM_DATA_VALSIZE_LONG:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          P1 = InputData4F[RowBase[1] + CurCol[1]];
          P2 = InputData4F[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData4F[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData4F[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* floating point long */
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          P1 = InputData4U[RowBase[1] + CurCol[1]];
          P2 = InputData4U[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData4U[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData4U[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* unsigned long */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          P1 = InputData4S[RowBase[1] + CurCol[1]];
          P2 = InputData4S[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData4S[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData4S[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* signed long */
         } /* switch value format */
        break;
        } /* long */
       case DEM_DATA_VALSIZE_DOUBLE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          P1 = InputData8F[RowBase[1] + CurCol[1]];
          P2 = InputData8F[RowBase[1] + CurCol[2]];
          if (CurCol[0] >= CROP_LEFT)
           {
           P0 = InputData8F[RowBase[1] + CurCol[0]];
           } /* if */
          if (CurCol[3] <= LastInCol)
           {
           P3 = InputData8F[RowBase[1] + CurCol[3]];
           } /* if */
          break;
          } /* floating point double */
         } /* switch value format */
        break;
        } /* double */
       } /* switch value size */
      D1 = CurCol[0] >= CROP_LEFT ? (.5 * (P2 - P0)): (P2 - P1);
      D2 = CurCol[3] <= LastInCol ? (.5 * (P3 - P1)): (P2 - P1);
      S1 = ColDelta;
      S2 = S1 * S1;
      S3 = S1 * S2;
      h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
      h2 = -2.0 * S3 + 3.0 * S2;
      h3 = S3 - 2.0 * S2 + S1;
      h4 = S3 - S2;
      TP[0] = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;

      if (SPLINE_CONSTRAIN)
       {
       if (TP[0] > P1 && TP[0] > P2) TP[0] = max(P1, P2);
       else if (TP[0] < P1 && TP[0] < P2) TP[0] = min(P1, P2);
       } /* if spline constraint */
      switch (INVALUE_SIZE)
       {
       case DEM_DATA_VALSIZE_BYTE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData1U[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* unsigned byte */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData1S[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* signed byte */
         } /* switch value format */
        break;
        } /* byte */
       case DEM_DATA_VALSIZE_SHORT:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData2U[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* unsigned short */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData2S[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* signed short */
         } /* switch value format */
        break;
        } /* short */
       case DEM_DATA_VALSIZE_LONG:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          OutputData4F[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* floating point long */
         case DEM_DATA_FORMAT_UNSIGNEDINT:
          {
          OutputData4U[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* unsigned long */
         case DEM_DATA_FORMAT_SIGNEDINT:
          {
          OutputData4S[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* signed long */
         } /* switch value format */
        break;
        } /* long */
       case DEM_DATA_VALSIZE_DOUBLE:
        {
        switch (INVALUE_FORMAT)
         {
         case DEM_DATA_FORMAT_FLOAT:
          {
          OutputData8F[i * OUTPUT_COLS + j] = TP[0];
          break;
          } /* floating point double */
         } /* switch value format */
        break;
        } /* double */
       } /* switch value size */
      } /* else compute splined value */
     } /* for j=0... */
    } /* if only one row needed, compute final values directly */
   else
    {
    for (j=0; j<OUTPUT_COLS; j++)
     {
     if (j == LastOutCol)
      CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = LastInCol;
     else
      CurCol[0] = CurCol[1] = CurCol[2] = CurCol[3] = CROP_LEFT + j * ColStep;
     CurCol[0] --;
     CurCol[2] ++;
     CurCol[3] += 2;
     ColDelta = j * ColStep - (float)(CurCol[1] - CROP_LEFT);

     for (k=0; k<4; k++)
      {
      if (CurRow[k] < CROP_TOP) continue;
      if (CurRow[k] > LastInRow) break;

      if (j == 0 || j == LastOutCol || ColDelta == 0.0)
       {
       switch (INVALUE_SIZE)
        {
        case DEM_DATA_VALSIZE_BYTE:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           TP[k] = InputData1U[RowBase[k] + CurCol[1]];
           break;
           } /* unsigned byte */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           TP[k] = InputData1S[RowBase[k] + CurCol[1]];
           break;
           } /* signed byte */
          } /* switch value format */
         break;
         } /* byte */
        case DEM_DATA_VALSIZE_SHORT:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           TP[k] = InputData2U[RowBase[k] + CurCol[1]];
           break;
           } /* unsigned short */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           TP[k] = InputData2S[RowBase[k] + CurCol[1]];
           break;
           } /* signed short */
          } /* switch value format */
         break;
         } /* short */
        case DEM_DATA_VALSIZE_LONG:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_FLOAT:
           {
           TP[k] = InputData4F[RowBase[k] + CurCol[1]];
           break;
           } /* floating point long */
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           TP[k] = InputData4U[RowBase[k] + CurCol[1]];
           break;
           } /* unsigned long */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           TP[k] = InputData4S[RowBase[k] + CurCol[1]];
           break;
           } /* signed long */
          } /* switch value format */
         break;
         } /* long */
        case DEM_DATA_VALSIZE_DOUBLE:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_FLOAT:
           {
           TP[k] = InputData8F[RowBase[k] + CurCol[1]];
           break;
           } /* floating point double */
          } /* switch value format */
         break;
         } /* double */
        } /* switch value size */
       } /* set value directly */
      else
       {
       switch (INVALUE_SIZE)
        {
        case DEM_DATA_VALSIZE_BYTE:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           P1 = InputData1U[RowBase[k] + CurCol[1]];
           P2 = InputData1U[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData1U[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData1U[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* unsigned byte */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           P1 = InputData1S[RowBase[k] + CurCol[1]];
           P2 = InputData1S[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData1S[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData1S[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* signed byte */
          } /* switch value format */
         break;
         } /* byte */
        case DEM_DATA_VALSIZE_SHORT:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           P1 = InputData2U[RowBase[k] + CurCol[1]];
           P2 = InputData2U[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData2U[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData2U[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* unsigned short */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           P1 = InputData2S[RowBase[k] + CurCol[1]];
           P2 = InputData2S[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData2S[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData2S[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* signed short */
          } /* switch value format */
         break;
         } /* short */
        case DEM_DATA_VALSIZE_LONG:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_FLOAT:
           {
           P1 = InputData4F[RowBase[k] + CurCol[1]];
           P2 = InputData4F[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData4F[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData4F[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* floating point long */
          case DEM_DATA_FORMAT_UNSIGNEDINT:
           {
           P1 = InputData4U[RowBase[k] + CurCol[1]];
           P2 = InputData4U[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData4U[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData4U[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* unsigned long */
          case DEM_DATA_FORMAT_SIGNEDINT:
           {
           P1 = InputData4S[RowBase[k] + CurCol[1]];
           P2 = InputData4S[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData4S[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData4S[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* signed long */
          } /* switch value format */
         break;
         } /* long */
        case DEM_DATA_VALSIZE_DOUBLE:
         {
         switch (INVALUE_FORMAT)
          {
          case DEM_DATA_FORMAT_FLOAT:
           {
           P1 = InputData8F[RowBase[k] + CurCol[1]];
           P2 = InputData8F[RowBase[k] + CurCol[2]];
           if (CurCol[0] >= CROP_LEFT)
            {
            P0 = InputData8F[RowBase[k] + CurCol[0]];
            } /* if */
           if (CurCol[3] <= LastInCol)
            {
            P3 = InputData8F[RowBase[k] + CurCol[3]];
            } /* if */
           break;
           } /* floating point double */
          } /* switch value format */
         break;
         } /* double */
        } /* switch value size */
       D1 = CurCol[0] >= CROP_LEFT ? (.5 * (P2 - P0)): (P2 - P1);
       D2 = CurCol[3] <= LastInCol ? (.5 * (P3 - P1)): (P2 - P1);
       S1 = ColDelta;
       S2 = S1 * S1;
       S3 = S1 * S2;
       h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
       h2 = -2.0 * S3 + 3.0 * S2;
       h3 = S3 - 2.0 * S2 + S1;
       h4 = S3 - S2;
       TP[k] = P1 * h1 + P2 * h2 + D1 * h3 + D2 * h4;
       if (SPLINE_CONSTRAIN)
        {
        if (TP[k] > P1 && TP[k] > P2) TP[k] = max(P1, P2);
        else if (TP[k] < P1 && TP[k] < P2) TP[k] = min(P1, P2);
        } /* if spline constraint */
       } /* else compute splined value */
      } /* for k=0... */
     D1 = CurRow[0] >= CROP_TOP ? (.5 * (TP[2] - TP[0])): (TP[2] - TP[1]);
     D2 = CurRow[3] <= LastInRow ? (.5 * (TP[3] - TP[1])): (TP[2] - TP[1]);
     S1 = RowDelta;
     S2 = S1 * S1;
     S3 = S1 * S2;
     h1 = 2.0 * S3 - 3.0 * S2 + 1.0;
     h2 = -2.0 * S3 + 3.0 * S2;
     h3 = S3 - 2.0 * S2 + S1;
     h4 = S3 - S2;
     TP[0] = TP[1] * h1 + TP[2] * h2 + D1 * h3 + D2 * h4;
     if (SPLINE_CONSTRAIN)
      {
      if (TP[0] > TP[1] && TP[0] > TP[2]) TP[0] = max(TP[1], TP[2]);
      else if (TP[0] < TP[1] && TP[0] < TP[2]) TP[0] = min(TP[1], TP[2]);
      } /* if spline constraint */
     switch (INVALUE_SIZE)
      {
      case DEM_DATA_VALSIZE_BYTE:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutputData1U[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* unsigned byte */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutputData1S[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* signed byte */
        } /* switch value format */
       break;
       } /* byte */
      case DEM_DATA_VALSIZE_SHORT:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutputData2U[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* unsigned short */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutputData2S[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* signed short */
        } /* switch value format */
       break;
       } /* short */
      case DEM_DATA_VALSIZE_LONG:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_FLOAT:
         {
         OutputData4F[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* floating point long */
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutputData4U[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* unsigned long */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutputData4S[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* signed long */
        } /* switch value format */
       break;
       } /* long */
      case DEM_DATA_VALSIZE_DOUBLE:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_FLOAT:
         {
         OutputData8F[i * OUTPUT_COLS + j] = TP[0];
         break;
         } /* floating point double */
        } /* switch value format */
       break;
       } /* double */
      } /* switch value size */
     } /* for j=0... */
    } /* else compute four rows of spline pts as intermediates, then final values */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
   if (BWDC)
    BusyWin_Update(BWDC, i + 1);
   } /* for i=0... */
  if (BWDC) BusyWin_Del(BWDC);
  if (error)
   goto Cleanup;

  free_Memory(InputData, InputDataSize);

  switch (INVALUE_SIZE)
   {
   case DEM_DATA_VALSIZE_BYTE:
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      InputData1U = OutputData1U;
      OutputData1U = NULL;
      break;
      } /* unsigned byte */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      InputData1S = OutputData1S;
      OutputData1S = NULL;
      break;
      } /* signed byte */
     } /* switch value format */
    break;
    } /* byte */
   case DEM_DATA_VALSIZE_SHORT:
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      InputData2U = OutputData2U;
      OutputData2U = NULL;
      break;
      } /* unsigned short */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      InputData2S = OutputData2S;
      OutputData2S = NULL;
      break;
      } /* signed short */
     } /* switch value format */
    break;
    } /* short */
   case DEM_DATA_VALSIZE_LONG:
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_FLOAT:
      {
      InputData4F = OutputData4F;
      OutputData4F = NULL;
      break;
      } /* floating point long */
     case DEM_DATA_FORMAT_UNSIGNEDINT:
      {
      InputData4U = OutputData4U;
      OutputData4U = NULL;
      break;
      } /* unsigned long */
     case DEM_DATA_FORMAT_SIGNEDINT:
      {
      InputData4S = OutputData4S;
      OutputData4S = NULL;
      break;
      } /* signed long */
     } /* switch value format */
    break;
    } /* long */
   case DEM_DATA_VALSIZE_DOUBLE:
    {
    switch (INVALUE_FORMAT)
     {
     case DEM_DATA_FORMAT_FLOAT:
      {
      InputData8F = OutputData8F;
      OutputData8F = NULL;
      break;
      } /* floating point double */
     } /* switch value format */
    break;
    } /* double */
   } /* switch value size */

  InputDataSize = OutputDataSize;
  InputData = OutputData;
  OutputData = NULL;
  INPUT_ROWS = ORows;
  INPUT_COLS = OCols;
  CROP_LEFT = CROP_RIGHT = CROP_TOP = CROP_BOTTOM = 0;

  } /* if need to resample data array to new size */


/*
** Compute scale
*/
 if (TestOnly || SCALEOP == DEM_DATA_SCALEOP_MAXMINSCALE ||
  	(SCALEOP == DEM_DATA_SCALEOP_MATCHSCALE &&
	   (SCALETYPE == DEM_DATA_SCALETYPE_MAXEL ||
	    SCALETYPE == DEM_DATA_SCALETYPE_MINEL)))
  {
  long RightEdge = INPUT_COLS - CROP_RIGHT;
  long BottomEdge = INPUT_ROWS - CROP_BOTTOM;

  datazip = 0;
  BWDC = BusyWin_New("Extrema", BottomEdge, 0, MakeID('B','W','D','C'));
  for (i=0; i<BottomEdge; i++)
   {
   if (i < CROP_TOP)
    {
    datazip += INPUT_COLS;
    continue;
    } /* if cropped top */
   for (j=0; j<RightEdge; j++)
    {
    if (j < CROP_LEFT)
     {
     datazip ++;
     continue;
     } /* if cropped top */
    switch (INVALUE_SIZE)
     {
     case DEM_DATA_VALSIZE_BYTE:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData1U[datazip] > DataMaxEl)
		DataMaxEl = InputData1U[datazip];
        if (InputData1U[datazip] < DataMinEl)
		DataMinEl = InputData1U[datazip];
        break;
	} /* unsigned byte */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData1S[datazip] > DataMaxEl)
	   DataMaxEl = InputData1S[datazip];
        if (InputData1S[datazip] < DataMinEl)
	   DataMinEl = InputData1S[datazip];
        break;
	} /* unsigned byte */
       } /* switch value format */
      break;
      } /* byte */
     case DEM_DATA_VALSIZE_SHORT:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData2U[datazip] > DataMaxEl)
	   DataMaxEl = InputData2U[datazip];
        if (InputData2U[datazip] < DataMinEl)
	   DataMinEl = InputData2U[datazip];
        break;
	} /* unsigned short */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData2S[datazip] > DataMaxEl)
	   DataMaxEl = InputData2S[datazip];
        if (InputData2S[datazip] < DataMinEl)
	   DataMinEl = InputData2S[datazip];
        break;
	} /* signed short */
       } /* switch value format */
      break;
      } /* short int */
     case DEM_DATA_VALSIZE_LONG:
      {
      switch (INVALUE_FORMAT)
       {
       case DEM_DATA_FORMAT_FLOAT:
        {
        if (InputData4F[datazip] > DataMaxEl)
	   DataMaxEl = InputData4F[datazip];
        if (InputData4F[datazip] < DataMinEl)
	   DataMinEl = InputData4F[datazip];
        break;
	} /* floating point long */
       case DEM_DATA_FORMAT_UNSIGNEDINT:
        {
        if (InputData4U[datazip] > DataMaxEl)
	   DataMaxEl = InputData4U[datazip];
        if (InputData4U[datazip] < DataMinEl)
	   DataMinEl = InputData4U[datazip];
        break;
	} /* unsigned long integer */
       case DEM_DATA_FORMAT_SIGNEDINT:
        {
        if (InputData4S[datazip] > DataMaxEl)
	   DataMaxEl = InputData4S[datazip];
        if (InputData4S[datazip] < DataMinEl)
	   DataMinEl = InputData4S[datazip];
        break;
	} /* signed long integer */
       } /* switch value format */
      break;
      } /* long word */
     case DEM_DATA_VALSIZE_DOUBLE:
      {
      if (InputData8F[datazip] > DataMaxEl)
	 DataMaxEl = InputData8F[datazip];
      if (InputData8F[datazip] < DataMinEl)
	 DataMinEl = InputData8F[datazip];
      break;
      } /* double */
     } /* switch input value size */
    datazip ++;
    } /* for j=0... */
   datazip += CROP_RIGHT;
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
   if (BWDC)
    BusyWin_Update(BWDC, i + 1);
   } /* for i=0... */
  if (BWDC) BusyWin_Del(BWDC);
  } /* if scale operator... */
 if (error)
  goto Cleanup;

 switch (SCALEOP)
  {
  case DEM_DATA_SCALEOP_MAXMINSCALE:
   {
   DataRef = DataMinEl;
   OutRef = SCALE_MINEL;
   if (DataMaxEl == DataMinEl || (SCALE_MAXEL == 0.0 && SCALE_MINEL == 0.0))
    {
    VertScale = 1.0;
    NoScaling = 1;
    break;
    } /* illegal scale */
   VertScale = (SCALE_MAXEL - SCALE_MINEL) / (DataMaxEl - DataMinEl);
   break;
   } /*  */
  case DEM_DATA_SCALEOP_MATCHSCALE:
   {
   switch (SCALETYPE)
    {
    case DEM_DATA_SCALETYPE_MAXEL:
     {
     DataRef = SCALE_VALU1;
     OutRef = SCALE_ELEV1;
     if (DataMaxEl == SCALE_VALU1 || (SCALE_SCALE == 0.0 && SCALE_ELEV1 == 0.0))
      {
      VertScale = 1.0;
      NoScaling = 1;
      break;
      } /* illegal scale */
     VertScale = (SCALE_SCALE - SCALE_ELEV1) / (DataMaxEl - SCALE_VALU1);
     break;
     } /*  */
    case DEM_DATA_SCALETYPE_MINEL:
     {
     DataRef = SCALE_VALU1;
     OutRef = SCALE_ELEV1;
     if (DataMinEl == SCALE_VALU1 || (SCALE_SCALE == 0.0 && SCALE_ELEV1 == 0.0))
      {
      VertScale = 1.0;
      NoScaling = 1;
      break;
      } /* illegal scale */
     VertScale = (SCALE_ELEV1 - SCALE_SCALE) / (SCALE_VALU1 - DataMinEl);
     break;
     } /*  */
    case DEM_DATA_SCALETYPE_SCALE:
     {
     DataRef = SCALE_VALU1;
     OutRef = SCALE_ELEV1;
     if (SCALE_SCALE == 0.0)
      {
      VertScale = 1.0;
      NoScaling = 1;
      break;
      } /* illegal scale */
     VertScale = SCALE_SCALE;
     break;
     } /*  */
    } /* switch scale type */
   break;
   } /*  */
  case DEM_DATA_SCALEOP_MATCHMATCH:
   {
   DataRef = SCALE_VALU2;
   OutRef = SCALE_ELEV2;
   if (SCALE_VALU3 == SCALE_VALU2 || (SCALE_ELEV3 == 0.0 && SCALE_ELEV2 == 0.0))
    {
    VertScale = 1.0;
    NoScaling = 1;
    break;
    } /* illegal scale */
   VertScale = (SCALE_ELEV3 - SCALE_ELEV2) / (SCALE_VALU3 - SCALE_VALU2);
   break;
   } /*  */
  } /* switch scale operator */

 SCALE_TESTMAX = DataMaxEl;   // Test Button and Min and Max text field in DEM Converter window
 SCALE_TESTMIN = DataMinEl;

 if (TestOnly)
	goto Cleanup;

/*
** If no scaling required and only one map for output - as in resampling only -
**  and same input/output formats, simply save the current array.
*/
 if (INPUT_FORMAT == OUTPUT_FORMAT && NoScaling
	 && OUTPUT_ROWMAPS == 1 && OUTPUT_COLMAPS == 1
	 && INPUT_ROWS == ORows && INPUT_COLS == OCols)
  {
  SaveConvertOutput(data, DEMHdr, InputData, InputDataSize, 0, 0,
	OUTPUT_ROWS, OUTPUT_COLS, OUTPUT_ROWS, OUTPUT_COLS, RGBComponent);
  goto Cleanup;
  }


/*
** Transform
*/
 switch (OUTPUT_FORMAT)
  {
  case DEM_DATA_OUTPUT_WCSDEM:
   {
   OUTVALUE_SIZE = DEM_DATA_VALSIZE_SHORT;
   OUTVALUE_FORMAT = DEM_DATA_FORMAT_SIGNEDINT;
   break;
   } /* if WCS DEM output */
  case DEM_DATA_OUTPUT_COLORIFF:
  case DEM_DATA_OUTPUT_GRAYIFF:
  case DEM_DATA_OUTPUT_COLORMAP:
   {
   OUTVALUE_SIZE = DEM_DATA_VALSIZE_BYTE;
   OUTVALUE_FORMAT = DEM_DATA_FORMAT_UNSIGNEDINT;
   break;
   } /* if Color Map or IFF output */
  case DEM_DATA_OUTPUT_ZBUF:
   {
   OUTVALUE_SIZE = DEM_DATA_VALSIZE_LONG;
   OUTVALUE_FORMAT = DEM_DATA_FORMAT_FLOAT;
   break;
   } /* if Z Buffer output */
  } /* switch */

 for (i=0; i<OUTPUT_ROWMAPS; i++)	/* W-E */
  {
  if (i == OUTPUT_ROWMAPS - 1)
	cols = LastOutputCols;
  else
	cols = OutputCols;
  for (j=0; j<OUTPUT_COLMAPS; j++)	/* S-N */
   {
   if (j == OUTPUT_COLMAPS - 1)
	rows = LastOutputRows;
   else
	rows = OutputRows;

   OutputDataSize = rows * cols;

   switch (OUTVALUE_SIZE)
    {
    case DEM_DATA_VALSIZE_BYTE:
     {
     switch (OUTVALUE_FORMAT)
      {
      case DEM_DATA_FORMAT_UNSIGNEDINT:
       {
       if ((OutputData1U = (UBYTE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        } /* if out of memory */
       OutputData = OutputData1U;
       break;
       } /* unsigned byte */
      case DEM_DATA_FORMAT_SIGNEDINT:
       {
       if ((OutputData1S = (BYTE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        } /* if out of memory */
       OutputData = OutputData1S;
       break;
       } /* signed byte */
      default:
       {
       error = 11;
       goto Cleanup;
       break;
       } /* floating point byte or unknown format */
      } /* switch output value format */
     break;
     } /* byte */
    case DEM_DATA_VALSIZE_SHORT:
     {
     OutputDataSize *= 2;
     switch (OUTVALUE_FORMAT)
      {
      case DEM_DATA_FORMAT_UNSIGNEDINT:
       {
       if ((OutputData2U = (USHORT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData2U;
       break;
       } /* unsigned short */
      case DEM_DATA_FORMAT_SIGNEDINT:
       {
       if ((OutputData2S = (SHORT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData2S;
       break;
       } /* signed short */
      default:
       {
       error = 11;
       goto Cleanup;
       break;
       } /* floating point short or unknown format */
      } /* switch output value format */
     break;
     } /* short */
    case DEM_DATA_VALSIZE_LONG:
     {
     OutputDataSize *= 4;
     switch (OUTVALUE_FORMAT)
      {
      case DEM_DATA_FORMAT_FLOAT:
       {
       if ((OutputData4F = (FLOAT *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData4F;
       break;
       } /* floating point long */
      case DEM_DATA_FORMAT_UNSIGNEDINT:
       {
       if ((OutputData4U = (ULONG *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData4U;
       break;
       } /* unsigned long */
      case DEM_DATA_FORMAT_SIGNEDINT:
       {
       if ((OutputData4S = (LONG *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData4S;
       break;
       } /* signed long */
      default:
       {
       error = 11;
       goto Cleanup;
       break;
       } /* unknown format */
      } /* switch output value format */
     break;
     } /* long */
    case DEM_DATA_VALSIZE_DOUBLE:
     {
     OutputDataSize *= 8;
     switch (OUTVALUE_FORMAT)
      {
      case DEM_DATA_FORMAT_FLOAT:
       {
       if ((OutputData8F = (DOUBLE *)get_Memory(OutputDataSize, MEMF_ANY)) == NULL)
        {
        error = 1;
        goto Cleanup;
        }
       OutputData = OutputData8F;
       break;
       } /* floating point double */
      default:
       {
       error = 11;
       goto Cleanup;
       break;
       } /* integer double or unknown format */
      } /* switch output value format */
     break;
     } /* double */
    case DEM_DATA_VALSIZE_UNKNOWN:
     {
     error = 10;
     goto Cleanup;
     break;
     } /* unknown value size */
    } /* switch output value size */

/*
** Scale and move data
*/

/* pointer to first corner */
   switch (OUTPUT_FORMAT)
    {
    case DEM_DATA_OUTPUT_WCSDEM:
    case DEM_DATA_OUTPUT_COLORMAP:
     {
     if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM 
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
      {
      BaseOff = (CROP_LEFT + i * (OutputCols - DUPROW)) * INPUT_COLS
		+ j * (OutputRows - DUPROW) + CROP_BOTTOM;
      } /* if WCS DEM input */
     else
      {
      BaseOff = (CROP_TOP + ((OUTPUT_COLMAPS - 1 - j) * (OutputRows - DUPROW)
		+ LastOutputRows - DUPROW)) * INPUT_COLS
		+ i * (OutputCols - DUPROW) + CROP_LEFT;
      } /* else not DEM input */
     break;
     } /* if WCS DEM or Color Map output */
    default:
     {
     if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
      {
      BaseOff = (CROP_LEFT + i * (OutputCols)) * INPUT_COLS
		+ j * (OutputRows) + rows - 1 + CROP_BOTTOM;
      } /* if WCS DEM input */
     else
      {
      if (OUTPUT_COLMAPS - 1 - j > 0)
       {
       BaseOff = (CROP_TOP + ((OUTPUT_COLMAPS - 2 - j) * OutputRows
		+ LastOutputRows)) * INPUT_COLS  + i * OutputCols + CROP_LEFT;
       } /* if not top row of output maps */
      else
       {
       BaseOff = CROP_TOP * INPUT_COLS + i * OutputCols + CROP_LEFT;
       } /* else top row of output maps */
      } /* else not DEM input */
     break;
     } /* else not WCS DEM or Color Map output */
    } /* switch OUTPUT_FORMAT */

   outzip = 0;

   BWDC = BusyWin_New("Convert", cols, 0, MakeID('B','W','D','C'));
   for (colctr=0; colctr<cols; colctr++)
    {
    if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM
	|| OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP)
     {
     if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
      {
/*      datazip = BaseOff + colctr * INPUT_ROWS;*/
      datazip = BaseOff + colctr * INPUT_COLS;
      outzip = colctr * rows;
      } /* if WCS DEM input */
     else
      {
      datazip = BaseOff + colctr;
      } /* else not DEM input */
     } /* if WCS DEM or Color Map output */
    else
     {
     if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
      {
/*      datazip = BaseOff + colctr * INPUT_ROWS;*/
      datazip = BaseOff + colctr * INPUT_COLS;
      outzip = colctr;
      } /* if WCS DEM input */
     else
      {
      datazip = BaseOff + colctr;
      outzip = colctr;
      } /* else not DEM input */
     } /* else not WCS DEM or Color Map output */

    for (rowctr=0; rowctr<rows; rowctr++)
     {
     switch (INVALUE_SIZE)
      {
      case DEM_DATA_VALSIZE_BYTE:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutValue = OutRef + (InputData1U[datazip] - DataRef) * VertScale;
         break;
	 } /* UBYTE input format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutValue = OutRef + (InputData1S[datazip] - DataRef) * VertScale;
         break;
	 } /* BYTE input format */
	} /* switch input format */
       break;
       } /* BYTE input size */
      case DEM_DATA_VALSIZE_SHORT:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutValue = OutRef + (InputData2U[datazip] - DataRef) * VertScale;
         break;
	 } /* USHORT input format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutValue = OutRef + (InputData2S[datazip] - DataRef) * VertScale;
         break;
	 } /* SHORT input format */
	} /* switch input format */
       break;
       } /* SHORT input size */
      case DEM_DATA_VALSIZE_LONG:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         OutValue = OutRef + (InputData4U[datazip] - DataRef) * VertScale;
         break;
	 } /* ULONG input format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         OutValue = OutRef + (InputData4S[datazip] - DataRef) * VertScale;
         break;
	 } /* LONG input format */
        case DEM_DATA_FORMAT_FLOAT:
         {
         OutValue = OutRef + (InputData4F[datazip] - DataRef) * VertScale;
         break;
	 } /* FLOAT input format */
	} /* switch input format */
       break;
       } /* LONG input size */
      case DEM_DATA_VALSIZE_DOUBLE:
       {
       switch (INVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_FLOAT:
         {
         OutValue = OutRef + (InputData8F[datazip] - DataRef) * VertScale;
         break;
	 } /* DOUBLE input format */
	} /* switch input format */
       break;
       } /* DOUBLE input size */
      } /* switch input value size */ 

     switch (OUTVALUE_SIZE)
      {
      case DEM_DATA_VALSIZE_BYTE:
       {
       switch (OUTVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         if (OutValue > UCHAR_MAX)
          OutputData1U[outzip] = UCHAR_MAX;
         else if (OutValue < 0.0)
          OutputData1U[outzip] = 0;
         else
          OutputData1U[outzip] = OutValue;
         break;
	 } /* UBYTE output format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         if (OutValue > SCHAR_MAX)
          OutputData1S[outzip] = SCHAR_MAX;
         else if (OutValue < SCHAR_MIN)
          OutputData1S[outzip] = SCHAR_MIN;
         else
          OutputData1S[outzip] = OutValue;
         break;
	 } /* BYTE output format*/
	} /* switch output format */
       break;
       } /* BYTE output size */
      case DEM_DATA_VALSIZE_SHORT:
       {
       switch (OUTVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         if (OutValue > USHRT_MAX)
          OutputData2U[outzip] = USHRT_MAX;
         else if (OutValue < 0.0)
          OutputData2U[outzip] = 0;
         else
          OutputData2U[outzip] = OutValue;
         break;
	 } /* USHORT output format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         if (OutValue > SHRT_MAX)
          OutputData2S[outzip] = SHRT_MAX;
         else if (OutValue < SHRT_MIN)
          OutputData2S[outzip] = SHRT_MIN;
         else
          OutputData2S[outzip] = OutValue;
         break;
	 } /* SHORT output format*/
	} /* switch output format */
       break;
       } /* SHORT output size */
      case DEM_DATA_VALSIZE_LONG:
       {
       switch (OUTVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_UNSIGNEDINT:
         {
         if (OutValue > ULONG_MAX)
          OutputData4U[outzip] = ULONG_MAX;
         else if (OutValue < 0.0)
          OutputData4U[outzip] = 0;
         else
          OutputData4U[outzip] = OutValue;
         break;
	 } /* ULONG output format */
        case DEM_DATA_FORMAT_SIGNEDINT:
         {
         if (OutValue > LONG_MAX)
          OutputData4S[outzip] = LONG_MAX;
         else if (OutValue < LONG_MIN)
          OutputData4S[outzip] = LONG_MIN;
         else
          OutputData4S[outzip] = OutValue;
         break;
	 } /* LONG output format*/
        case DEM_DATA_FORMAT_FLOAT:
         {
         if (OutValue > FLT_MAX)
          OutputData4F[outzip] = FLT_MAX;
         else
          OutputData4F[outzip] = OutValue;
         break;
	 } /* FLOAT output format*/
	} /* switch output format */
       break;
       } /* LONG output size */
      case DEM_DATA_VALSIZE_DOUBLE:
       {
       switch (OUTVALUE_FORMAT)
        {
        case DEM_DATA_FORMAT_FLOAT:
         {
         OutputData8F[outzip] = OutValue;
         break;
	 } /* DOUBLE output format*/
	} /* switch output format */
       break;
       } /* DOUBLE output size */
      } /* switch output size */

     if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM
	|| OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP)
      {
      if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
       {
       outzip ++;
       datazip ++;
       } /* if WCS DEM input */
      else
       {
       outzip ++;
       datazip -= INPUT_COLS;
       } /* else not DEM input */
      } /* if output WCS DEM */
     else
      {
      if (INPUT_FORMAT == DEM_DATA_INPUT_WCSDEM
	|| INPUT_FORMAT == DEM_DATA_INPUT_DTED)
       {
       outzip += cols;
       datazip --;
       } /* if WCS DEM input */
      else
       {
       outzip += cols;
       datazip += INPUT_COLS;
       } /* else not DEM input */
      } /* else not WCS DEM output */

     } /* for rowctr=0... */
   if (CheckInput_ID() == ID_BW_CLOSE)
    {
    error = 50;
    break;
    } /* if user abort */
    if (BWDC)
     BusyWin_Update(BWDC, colctr + 1);
    } /* for colctr=0... */
   if (BWDC) BusyWin_Del(BWDC);
   if (error)
    break;
/*
** Output
*/
   error = SaveConvertOutput(data, DEMHdr, OutputData, OutputDataSize, i, j,
	rows, cols, OutputRows, OutputCols, RGBComponent);


   if (OutputData)
    {
    free_Memory(OutputData, OutputDataSize);
    OutputData = NULL;
    } /* if OutputData */
   if (error) break;
   } /* for j=0... */
  if (error) break;
  } /* for i=0... */

Cleanup:
 switch (error)
  {
  case 1:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Out of memory!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* out of memory */
  case 2:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Unable to open file for input!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file open fail */
  case 3:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Incorrect file size for specified header, width and height!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_SIZE, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file size fail */
  case 4:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Unable to open file for output!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)"Convert DEM destination file");
   break;
   } /* file open fail */
  case 5:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Error writing destination file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRITE_FAIL, (CONST_STRPTR)"Convert DEM destination file");
   break;
   } /* file open fail */
  case 6:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Error reading source file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_SIZE, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file open fail */
  case 7:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Not a compressed file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_SIZE, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file open fail */
  case 8:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Extended header!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_SIZE, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file open fail */
  case 10:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Input file configuration not yet supported!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)"Convert DEM source type");
   break;
   } /* file type fail */
  case 11:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Input data format not supported!\nCheck your settings.\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_READ_FAIL, (CONST_STRPTR)"Convert DEM source type");
   break;
   } /* file type fail */
  case 12:
   {
   User_Message((CONST_STRPTR)"Database Module",
           (CONST_STRPTR)"Out of memory expanding database!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   } /* out of memory for database expansion */
  case 13:
   {
   Log(ERR_WRONG_SIZE, (CONST_STRPTR)"Convert DEM source file");
   break;
   } /* file open fail */
  case 14:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Error saving \".Obj\" file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   break;
   }
  case 15:
   {
   User_Message((CONST_STRPTR)"Data Ops: Convert DEM",
           (CONST_STRPTR)"Input file not recognized as a DTED file!\nOperation terminated.", (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
   Log(ERR_WRONG_TYPE, (CONST_STRPTR)"DTED");
   break;
   }
  case 50:
   {
   break;
   } /* operation canceled */
  } /* switch error */

 if (OutputData)
	 free_Memory(OutputData, OutputDataSize);
 if (InputData)
	 free_Memory(InputData, InputDataSize);
 if (DEMHdr)
	 free_Memory(DEMHdr, sizeof (struct elmapheaderV101));

 if ((! error && OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP) && ! TestOnly)
  {
#ifdef OLD_COLORMAP_CODE  // AF, 18.April 23 Old code does NOT generate 3 files with .red, .grn and .blu but only one without suffix. See also above!
  short length;

  length = strlen(filename) - 3;
  if (! stricmp("red", &filename[length]))
   {
   filename[length] = 0;
   strcat(filename, "grn");
   strcpy(RGBComponent, ".grn");
   goto RepeatRGB;
   } /* if filename ends in "red" */
  else if (! stricmp("grn", &filename[length]))
   {
   filename[length] = 0;
   strcat(filename, "blu");
   strcpy(RGBComponent, ".blu");
   goto RepeatRGB;
   } /* else if filename ends in "grn" */
#else
  if(!strcmp(RGBComponent,".red"))
  {
	  strcpy(RGBComponent, ".grn");
	  goto RepeatRGB;
  }
  else if(!strcmp(RGBComponent,".grn"))
  {
	  strcpy(RGBComponent, ".blu");
	  goto RepeatRGB;
  }
#endif
  } /* if may need to repeat process for other color components */

} /* ConvertDEM() */

/***********************************************************************/

STATIC_FCN short SaveConvertOutput(struct DEMConvertData *data, struct elmapheaderV101 *DEMHdr,
	void *OutputData, long OutputDataSize, short i, short j,
	long rows, long cols, long OutputRows, long OutputCols, char *RGBComp) // used locally only -> static, AF 26.7.2021
{
 char tempfilename[32], OutFilename[256];
 short error = 0, OBNexists, Elev[6];
 long	fOutput,
	ProtFlags = FIBB_OTR_READ | FIBB_OTR_WRITE | FIBB_OTR_DELETE,
	OutFlags = O_WRONLY | O_CREAT | O_TRUNC;
 float	Version = DEM_CURRENT_VERSION;
 double Lon[6], Lat[6];

 strcpy(tempfilename, OUTPUT_NAMEBASE);

 if (OUTPUT_ROWMAPS > 1 || OUTPUT_COLMAPS > 1)
  {
  char suffix[4] = { '.', 0, 0, 0 };
  short mapct;

  mapct = i * OUTPUT_COLMAPS + j;
  if (OUTPUT_COLMAPS * OUTPUT_ROWMAPS > 25)
   {
   suffix[1] = 65 + mapct / 26;
   suffix[2] = 65 + mapct % 26;
   }
  else
   suffix[1] = 65 + mapct;  // 'A' + count

  if ((OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM)
	|| (OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP && RGBComp[0]))
   {
   if (strlen(tempfilename) > length[0] - 3)
     	tempfilename[length[0] - 3] = 0;
   strcat(tempfilename, suffix);
   while (strlen(tempfilename) < length[0])
	strcat(tempfilename, " ");
   strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
   if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM)
    strcat(OutFilename, ".elev");
   else
    strcat(OutFilename, RGBComp);
   } /* if output DEM to database */
  else
   {
   strcat(tempfilename, suffix);
   strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
   } /* if no database output */
  } /* if more than one output map */
 else
  {
  if ((OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM)
	|| (OUTPUT_FORMAT == DEM_DATA_OUTPUT_COLORMAP && RGBComp[0]))
   {
   while (strlen(tempfilename) < length[0])
	strcat(tempfilename, " ");
   tempfilename[length[0]] = 0;
   strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
   if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_WCSDEM)
    strcat(OutFilename, ".elev");
   else
    strcat(OutFilename, RGBComp);
   } /* if output DEM to database */
  else
   {
   strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
   } /* if no output to database */
  } /* else only one output map */

 switch (OUTPUT_FORMAT)
  {
  case DEM_DATA_OUTPUT_WCSDEM:
   {
   if ((fOutput = open(OutFilename, OutFlags, ProtFlags)) < 0)
    {
    error = 4;
    break;
    } /* if open fail */
   DEMHdr->rows	 = cols - 1;
   DEMHdr->columns	 = rows;
   DEMHdr->steplat	 = (OUTPUT_HILAT - OUTPUT_LOLAT) / (OUTPUT_ROWS - 1);
   DEMHdr->steplong	 = (OUTPUT_HILON - OUTPUT_LOLON) / (OUTPUT_COLS - 1);
   DEMHdr->lolat	 =
	 OUTPUT_LOLAT + j * (OutputRows - DUPROW) * DEMHdr->steplat ;
   DEMHdr->lolong	 =
	 OUTPUT_HILON - i * (OutputCols - DUPROW) * DEMHdr->steplong;
/*   DEMHdr->elscale; don't change it*/
   if (INPUT_FORMAT != DEM_DATA_INPUT_WCSDEM)
    {
    switch (DATA_UNITS)
     {
     case DEM_DATA_UNITS_KILOM:
      {
      DEMHdr->elscale	 = ELSCALE_KILOM;
      break;
      }
     case DEM_DATA_UNITS_CENTIM:
      {
      DEMHdr->elscale	 = ELSCALE_CENTIM;
      break;
      }
     case DEM_DATA_UNITS_MILES:
      {
      DEMHdr->elscale	 = ELSCALE_MILES;
      break;
      }
     case DEM_DATA_UNITS_FEET:
      {
      DEMHdr->elscale	 = ELSCALE_FEET;
      break;
      }
     case DEM_DATA_UNITS_INCHES:
      {
      DEMHdr->elscale	 = ELSCALE_INCHES;
      break;
      }
     default:
      {
      DEMHdr->elscale	 = ELSCALE_METERS;
      break;
      }
     } /* switch */

    if (ROWS_EQUAL == DEM_DATA_ROW_LON)
     {
     swmem(&DEMHdr->steplat, &DEMHdr->steplong, 8);
     swmem(&DEMHdr->lolat, &DEMHdr->lolong, 8);
     } /* if rows = longitude */
    } /* if not WCS DEM input */

   FindElMaxMin(DEMHdr, OutputData);
   //write(fOutput, (char *)&Version, 4);
   write_float_BE(fOutput, &Version); // AF, 21.3.23, Write Big Endian
   //write(fOutput, (char *)DEMHdr, ELEVHDRLENV101);
   writeElMapHeaderV101_BE(fOutput,DEMHdr); // AF, 21.3.23, Write Big Endian

   //if ((write(fOutput, (char *)OutputData, OutputDataSize)) != OutputDataSize)
   if ((write_short_Array_BE(fOutput, OutputData, OutputDataSize)) != OutputDataSize) // AF, 21.3.23, Write Big Endian
    {
    error = 5;
    close(fOutput);
    goto Cleanup;
    } /* if write fail */

   close(fOutput);

   OBNexists = 0;
   for (OBN=0; OBN<NoOfObjects; OBN++)
    {
    if (! strcmp(tempfilename, DBase[OBN].Name))
     {
     OBNexists = 1;
     break;
     }
    } /* for OBN=0... */
   if (! OBNexists)
    {
    if (NoOfObjects + 1 > DBaseRecords)
     {
     struct database *NewBase;

     if ((NewBase = DataBase_Expand(DBase, DBaseRecords, NoOfObjects,
	 DBaseRecords + 20)) == NULL)
      {
      error = 12;
      break;
      } /* if new database allocation fails */
     else
      {
      DBase = NewBase;
      DBaseRecords += 20;
      } /* else new database allocated and copied */
     } /* if need more database space */
    strncpy(DBase[OBN].Name, tempfilename, length[0]);
    strcpy(DBase[OBN].Layer1, "  ");
    strcpy(DBase[OBN].Layer2, "TOP");
    DBase[OBN].Color = 2;
    DBase[OBN].LineWidth = 1;
    strcpy(DBase[OBN].Label, "               ");
    DBase[OBN].MaxFract = 9;
    DBase[OBN].Pattern[0] = 'L';
    DBase[OBN].Red = 255;
    DBase[OBN].Grn = 255;
    DBase[OBN].Blu = 255;
    NoOfObjects ++;
    if (DE_Win)
     {
     if (! Add_DE_NewItem())
      {
      error = 1;
      } /* if new list fails */
     } /* if database editor open */
    } /* if object not already exists */
   DBase[OBN].Points = 5;
   DBase[OBN].Mark[0] = 'Y';
   DBase[OBN].Enabled = '*';
   DBase[OBN].Flags = 6;
   strcpy(DBase[OBN].Special, "TOP");

   strmfp(OutFilename, OUTPUT_DIRECTORY, tempfilename);
   strcat(OutFilename, ".Obj");

   Lat[0] = Lat[1] = Lat[2] = Lat[5] = DEMHdr->lolat;
   Lat[3] = Lat[4] = DEMHdr->lolat + (DEMHdr->columns - 1) * DEMHdr->steplat;
   Lon[0] = Lon[1] = Lon[4] = Lon[5] = DEMHdr->lolong - DEMHdr->rows * DEMHdr->steplong;
   Lon[2] = Lon[3] = DEMHdr->lolong;
   memset(&Elev[0], 0, 6 * sizeof (short));

   if (saveobject(OBN, OutFilename, &Lon[0], &Lat[0], &Elev[0]))
    {
    error = 14;
    }
   break;
   } /* WCS DEM */
  case DEM_DATA_OUTPUT_GRAYIFF:
  case DEM_DATA_OUTPUT_COLORIFF:
   {
   long *RowZip, row;
   UBYTE *BitMap[3];

   strcpy(ILBMname, OutFilename);
   if ((RowZip = (long *)get_Memory(rows * sizeof (long), MEMF_ANY)))
    {
    for (row=0; row<rows; row++)
     RowZip[row] = row * cols;
    if (OUTPUT_FORMAT == DEM_DATA_OUTPUT_GRAYIFF)
     saveILBM(8, 0, NULL, (UBYTE **)&OutputData, RowZip, 0, 1, 0, cols, rows);
    else
     {
     BitMap[0] = BitMap[1] = BitMap[2] = OutputData;
     saveILBM(24, 0, NULL, BitMap, RowZip, 0, 1, 0, cols, rows);
     }
    free_Memory(RowZip, rows * sizeof (long));
    } /* if */
   else
    error = 1;
   break;
   } /* iff */
  case DEM_DATA_OUTPUT_ARRAY:
  case DEM_DATA_OUTPUT_COLORMAP:
   {
   if ((fOutput = open(OutFilename, OutFlags, ProtFlags)) < 0)
    {
    error = 4;
    break;
    } /* if open fail */
   // AF: old: if ((write(fOutput, (char *)OutputData, OutputDataSize)) != OutputDataSize)
   // AF, 20.Mar23 writes the Buffer in Big Endian Format, cares for int, unsigned and float, 1,2,4,8 Bytes size
   if((writeDemArray_BE(fOutput,OutputData,OutputDataSize,OUTVALUE_FORMAT,OUTVALUE_SIZE)) != OutputDataSize)
    {
    error = 5;
    close(fOutput);
    goto Cleanup;
    } /* if write fail */
   close(fOutput);
   break;
   } /* array */
  case DEM_DATA_OUTPUT_ZBUF:
   {
   SaveZBuf(0, 0, OutputDataSize / sizeof (float), NULL,
	(float *)OutputData, OutFilename, rows, cols);
   break;
   } /* Z Buffer */
  } /* switch output file format */

Cleanup:

 return (error);

} /* SaveConvertOutput() */
