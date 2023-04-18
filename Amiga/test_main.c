/*
 * test_main.c
 *
 *  Created on: Apr 13, 2023
 *      Author: Alexander Fritsch, Selco, HGW
 */

#define MAIN

#include "WCS.h"
#include "Version.h"
#include "BigEndianReadWrite.h"
#include <assert.h>

#include <exec/types.h>
#include <exec/tasks.h>
#include <clib/exec_protos.h>


USHORT User_Message_Def(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey, int Default)
{
	fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,message);
	return 0;
}

USHORT User_Message(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey)
{
	fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,message);
	return 0;
}

void Log(USHORT StdMesgNum, CONST_STRPTR LogTag)
{
	fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,LogTag);
}

ULONG CheckInput_ID(void)
{
	//fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 0;
}

void BusyWin_Update(struct BusyWindow *This, int Step)
{
	//fprintf(stderr,"ALEXANDER: %s()\n",__func__);
}

struct BusyWindow *BusyWin_New(char *Title, int Steps, int TimeEst, ULONG Section)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return (struct BusyWindow *)1; // invalid address but newer used in this test
}

void BusyWin_Del(struct BusyWindow *This)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
}


short getfilename(long mode, char *requestname, char *pathname,
    char *filename)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 0;
}

void Set_DM_HdrData(struct USGS_DEMHeader *Hdr)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
}

short makesky(short renderseg, struct Window *win)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 0;
}

void ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
}

short GetInputString(char *message, char *reject, char *string)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 0;
}

short Set_DM_Data(struct DEMExtractData *DEMExtract)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 1;
}

short Add_DE_NewItem(void)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return 1;
}

struct database *DataBase_Expand(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords)
{
	static char *DataBase="Database";
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	return (struct database *)DataBase;   // something that is not NULL
}

void Set_DM_ProfData(struct USGS_DEMProfileHeader *ProfHdr)
{
	fprintf(stderr,"ALEXANDER: %s()\n",__func__);
}

// frpm Maputil.c
void setlineseg(struct lineseg *ls, double firstx, double firsty,
	double lastx, double lasty)
{
 ls->oldx = ls->lastx;
 ls->oldy = ls->lasty;
 ls->firstx = firstx;
 ls->firsty = firsty;
 ls->lastx = lastx;
 ls->lasty = lasty;

} /* setlineseg() */

// from, MapUtil.c
short saveobject(long OBN, char *fname, double *Lon, double *Lat, short *Elev)
{
 char filename[255], filetype[10];
 short j, OpenOK, WriteOK = 0;
 long Size;
 float Version;
 double LatSum = 0.0, LonSum = 0.0;
 struct vectorheaderV100 Hdr={0};  // init, AF, 23.Mar.23
 struct DirList *DLItem;
 FILE *fobject;

 Version = VEC_CURRENT_VERSION;
 if (! Lat || ! Lon || ! Elev)
  {
  return (1);
  }

 OpenOK = 0;

 if (! fname)
  {
  DLItem = DL;
  while (DLItem)
   {
   if (DLItem->Read == '*')
    {
    DLItem = DLItem->Next;
    continue;
    } /* if directory is write protected */
   strmfp(filename, DLItem->Name, DBase[OBN].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "rb")) == NULL)
    {
    DLItem = DLItem->Next;
    }
   else
    {
    fclose(fobject);
    if ((fobject = fopen(filename, "wb")) != NULL)
     OpenOK = 1;
    break;
    }
   } /* while */

  if (! OpenOK)
   {
   strmfp(filename, dirname, DBase[OBN].Name);
   strcat(filename, ".Obj");
   if ((fobject = fopen(filename, "wb")) == NULL)
    {
    Log(ERR_OPEN_FAIL, (CONST_STRPTR)DBase[OBN].Name);
    User_Message((CONST_STRPTR)DBase[OBN].Name, (CONST_STRPTR)"Can't open object file!\nObject not saved.",
            (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
    return(1);
    } /* if open fail */
   } /* if pre-existing file not found */
  } /* if no supplied name */
 else
  {
  if ((fobject = fopen(fname, "wb")) == NULL)
   {
   Log(ERR_OPEN_FAIL, (CONST_STRPTR)fname);
   return(1);
   } /* if open fail */
  } /* if name passed to saveobject() */

 strcpy(filetype, "WCSVector");
 strncpy(Hdr.Name, DBase[OBN].Name, 10);
 Hdr.points = DBase[OBN].Points;
 Hdr.elevs = 0;

 for (j=1; j<=DBase[OBN].Points; j++)
  {
  LatSum += Lat[j];
  LonSum += Lon[j];
  } /* for j=0... */
 Hdr.avglat = LatSum / Hdr.points;
 Hdr.avglon = LonSum / Hdr.points;
 Hdr.elscale = ELSCALE_METERS;

 Size = (DBase[OBN].Points + 1) * sizeof (double);

 if (fwrite((char *)filetype, 9, 1, fobject) == 1)
  {
  if (fwrite_float_BE(&Version,fobject)==1)   //(fwrite((char *)&Version, sizeof (float), 1, fobject) == 1) AF: 22.Mar.23
   {
   if (fwriteVectorheaderV100_BE(&Hdr, fobject) == 1) // (fwrite((char *)&Hdr, sizeof (struct vectorheaderV100), 1, fobject) == 1)  AF: 22.Mar.23
    {
    if (fwrite_double_Array_BE(&Lon[0], Size,fobject)==1) //(fwrite((char *)&Lon[0], Size, 1, fobject) == 1) AF: 22.Mar.23
     {
     if (fwrite_double_Array_BE(&Lat[0], Size, fobject) == 1) // (fwrite((char *)&Lat[0], Size, 1, fobject) == 1) AF: 22.Mar.23
      {
      if (fwrite_SHORT_Array_BE(&Elev[0], Size / 4, fobject) == 1) // (fwrite((char *)&Elev[0], Size / 4, 1, fobject) == 1) AF: 22.Mar.23
       {
       WriteOK = 1;
       }
      }
     }
    }
   }
  }
 fclose(fobject);

 if (WriteOK)
  {
  DBase[OBN].Flags &= (255 ^ 1);
  sprintf(str, "%s vector saved. %d points", DBase[OBN].Name, DBase[OBN].Points);
  Log(MSG_NULL, (CONST_STRPTR)str);
  DB_Mod = 1;
  }
 else
  {
  User_Message((CONST_STRPTR)DBase[OBN].Name, (CONST_STRPTR)"Error saving object file!\nObject not saved.",
          (CONST_STRPTR)"OK", (CONST_STRPTR)"o");
  Log(ERR_WRITE_FAIL, (CONST_STRPTR)DBase[OBN].Name);
  return (1);
  }

 return(0);

} /* saveobject() */

void UTM_LatLon(struct UTMLatLonCoords *Coords)
{
double x, y, M, mu, phi_1, C_1, T_1, N_1, R_1, D_1,
	cos_phi_1, tan_phi_1, sin_phi_1, sin_sq_phi_1, C_1_sq, T_1_sq, D_1_sq;


 x 		= Coords->East;
 y 		= Coords->North;
 x 		-= 500000.0;
 M 		= Coords->M_0 + y / Coords->k_0;
 mu = M / (Coords->a * (1.0 - Coords->e_sq / 4.0 - 3.0 * Coords->e_sq_sq
	/ 64 - 5.0 * Coords->e_sq * Coords->e_sq_sq / 256.0));
 phi_1 = (mu + (3.0 * Coords->e_1 / 2.0 - 27.0 * Coords->e_1 * Coords->e_1_sq / 32.0)
	* sin(2.0 * mu)
	+ (21.0 * Coords->e_1_sq / 16.0 - 55.0 * Coords->e_1_sq * Coords->e_1_sq / 32.0)
	* sin(4.0 * mu)
	+ (151.0 * Coords->e_1 * Coords->e_1_sq / 96.0) * sin(6.0 * mu)); /* radians */
 cos_phi_1 	= cos(phi_1);
 tan_phi_1 	= tan(phi_1);
 sin_phi_1 	= sin(phi_1);
 sin_sq_phi_1	= sin_phi_1 * sin_phi_1;
 phi_1 		*= PiUnder180;				/* degrees */
 C_1 		= Coords->e_pr_sq * cos_phi_1 * cos_phi_1;
 C_1_sq 	= C_1 * C_1;
 T_1 		= tan_phi_1 * tan_phi_1;
 T_1_sq 	= T_1 * T_1;
 N_1 		= Coords->a / sqrt(1.0 - Coords->e_sq * sin_sq_phi_1);
 R_1 		= Coords->a * (1.0 - Coords->e_sq) / pow((1.0 - Coords->e_sq * sin_sq_phi_1), 1.5);
 D_1		= x / (N_1 * Coords->k_0);
 D_1_sq 	= D_1 * D_1;

 Coords->Lat = phi_1 - (N_1 * tan_phi_1 / R_1) *
	(D_1_sq / 2.0
	- (5.0 + 3.0 * T_1 + 10.0 * C_1 - 4.0 * C_1_sq - 9.0 * Coords->e_pr_sq)
	* D_1_sq * D_1_sq / 24.0
	+ (61.0 + 90.0 * T_1 + 298.0 * C_1 + 45.0 * T_1_sq - 252.0 * Coords->e_pr_sq
		- 3.0 * C_1_sq)
	* D_1_sq * D_1_sq * D_1_sq / 720.0)
	* PiUnder180;		/* degrees */

/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */

 Coords->Lon = Coords->lam_0 - ((D_1 - (1.0 + 2.0 * T_1 + C_1) * D_1 * D_1_sq / 6.0
	+ (5.0 - 2.0 * C_1 + 28.0 * T_1 - 3.0 * C_1_sq + 8.0 * Coords->e_pr_sq
		+ 24.0 * T_1_sq)
	* D_1 * D_1_sq * D_1_sq / 120.0) / cos_phi_1) * PiUnder180; /* degrees */

} /* UTM_LatLon() */


// from Maputil.c
void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone)
{
 double a, e_sq, k_0, lam_0, M_0, x, y, e_pr_sq, M,
	C, T, N, phi, lam, phi_rad,
	cos_phi, sin_phi, tan_phi, A, A_sq, A_cu, A_fo, A_fi, A_si, T_sq;

 a 	= 6378206.4;
 e_sq 	= 0.00676866;
 k_0 	= 0.9996;
 M_0 	= 0.0;
 phi 	= Coords->Lat;
 lam 	= -Coords->Lon;	/* change sign to conform with GIS standard */
 lam_0 	= -(double)(183 - UTMZone * 6);
 phi_rad = phi * PiOver180;

 e_pr_sq = e_sq / (1.0 - e_sq);
 sin_phi = sin(phi_rad);

/* use a very large number for tan_phi if tan(phi) is undefined */

 tan_phi = fabs(phi) == 90.0 ? (phi >= 0.0 ? 1.0: -1.0) * FLT_MAX / 100.0:
	 tan(phi_rad);
 cos_phi = cos(phi_rad);

 N	= a / sqrt(1.0 - e_sq * sin_phi * sin_phi);
 T	= tan_phi * tan_phi;
 T_sq	= T * T;
 C	= e_pr_sq * cos_phi * cos_phi;
 A	= cos_phi * (lam - lam_0) * PiOver180;
 A_sq	= A * A;
 A_cu	= A * A_sq;
 A_fo	= A * A_cu;
 A_fi	= A * A_fo;
 A_si	= A * A_fi;
 M	= 111132.0894 * phi - 16216.94 * sin (2.0 * phi_rad)
	 + 17.21 * sin(4.0 * phi_rad) - .02 * sin(6.0 * phi_rad);
 x	= k_0 * N * (A + ((1.0 - T + C) * A_cu / 6.0)
	  + ((5.0 - 18.0 * T + T_sq + 72.0 * C - 58.0 * e_pr_sq) * A_fi / 120.0)
	  ) + 500000;
 y	= k_0 * (M - M_0 + N * tan_phi *
	   (A_sq / 2.0 + ((5.0 - T + 9.0 * C + 4.0 * C * C) * A_fo / 24.0)
	   + ((61.0 - 58.0 * T + T_sq + 600.0 * C - 330.0 * e_pr_sq) * A_si / 720.0)
	  ));


/* note: if longitude is desired in negative degrees for West then the central
	meridian longitude must be negative and the first minus sign in the
	following formula should be a plus */


 Coords->North = y;
 Coords->East  = x;

} /* UTM_LatLon() */

// from Maputil.c
void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone)
{

  Coords->a 	= 6378206.4;
  Coords->e_sq 	= 0.00676866;
  Coords->k_0 	= 0.9996;
  Coords->M_0 	= 0.0;
  Coords->lam_0	= (double)(183 - UTMZone * 6);

  Coords->e_pr_sq 	= Coords->e_sq / (1.0 - Coords->e_sq);
  Coords->e_sq_sq 	= Coords->e_sq * Coords->e_sq;
  Coords->e_1 		= (1.0 - sqrt(1.0 - Coords->e_sq)) / (1.0 + sqrt(1.0 - Coords->e_sq));
  Coords->e_1_sq 	= Coords->e_1 * Coords->e_1;

} /* UTMLatLonCoords_Init() */

//from MapUtil.c
double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, short *Data, long Rows, long Cols)
{
 long Row, Col, Col_p, Row_p, TrueRow, TrueCol;
 double RemX = 0.0, RemY = 0.0, P1, P2;

 Row = TrueRow = ((Y - MinY) / IntY);
 Col = TrueCol = ((X - MinX) / IntX);
 if (Row < 0)
  Row = 0;
 else if (Row >= Rows)
  Row = Rows - 1;
 if (Col < 0)
  Col = 0;
 if (Col >= Cols)
  Col = Cols - 1;

 Row_p = Row;
 Col_p = Col;

 if (Row < Rows - 1 && TrueRow >= 0)
  {
  RemY = (Y - (Row * IntY + MinY)) / IntY;
  if (RemY < 0.0)
   RemY = 0.0;
  Row_p ++;
  } /* if not last row */
 if (Col < Cols - 1 && TrueCol >= 0)
  {
  RemX = (X - (Col * IntX + MinX)) / IntX;
  if (RemX < 0.0)
   RemX = 0.0;
  Col_p ++;
  } /* if not last column */

 P1 = RemX * (Data[Col_p * Rows + Row] - Data[Col * Rows + Row])
	 + Data[Col * Rows + Row];
 P2 = RemX * (Data[Col_p * Rows + Row_p] - Data[Col * Rows + Row_p])
	 + Data[Col * Rows + Row_p];

 return ( RemY * (P2 - P1) + P1 );

} /* Point_Extract() */


// ##############################################################################################################
#define OPEN_FILE1_ERROR     1
#define OPEN_FILE2_ERROR     2
#define FILESIZE1_ERROR      3
#define FILESIZE2_ERROR      4
#define SIZE_DIFFERENT_ERROR 5
#define ALLOC1_ERROR         6
#define ALLOC2_ERROR         7
#define FREAD1_ERROR         8
#define FREAD2_ERROR         9
#define MEMCMP_ERROR        10

int GetFileSize(FILE *File)
{
	int Size;
	int Ret;

	Ret=fseek(File,0,SEEK_END);
	if(Ret<0)
	{
		return Ret;
	}
	Size=ftell(File);

	rewind(File); // no return value
	return Size;  // < 0 if error
}

int CompareFileExactly(char *FileName1, char *FileName2)
{
		FILE *File1=NULL;
		FILE *File2=NULL;
		long Size1=0;
		long Size2=0;
		unsigned char *p1=NULL;
		unsigned char *p2=NULL;

		int Error=0;

		File1=fopen(FileName1,"rb");
		if(!File1)
		{
			Error=OPEN_FILE1_ERROR;
			printf("Failed to open %s\n",FileName1);
			goto Cleanup;
		}
		File2=fopen(FileName2,"rb");
		if(!File2)
		{
			printf("Failed to open %s\n",FileName2);
			Error=OPEN_FILE2_ERROR;
			goto Cleanup;
		}

		Size1=GetFileSize(File1);
		fseek(File1,0,SEEK_END);
		Size1=ftell(File1);
		rewind(File1);
		if(Size1<0)
		{
			Error=FILESIZE1_ERROR;
			goto Cleanup;
		}

		Size2=GetFileSize(File2);
		if(Size2<0)
		{
			Error=FILESIZE2_ERROR;
			goto Cleanup;
		}

		if(Size1!=Size2)
		{
			Error=SIZE_DIFFERENT_ERROR;
			goto Cleanup;
		}

		p1=malloc(Size1);
		if(!p1)
		{
			Error=ALLOC1_ERROR;
			goto Cleanup;
		}

		p2=malloc(Size2);
		if(!p1)
		{
			Error=ALLOC2_ERROR;
			goto Cleanup;
		}

		if(fread(p1,Size1,1,File1)!=1)
		{
			Error=FREAD1_ERROR;
			goto Cleanup;
		}

		if(fread(p2,Size2,1,File2)!=1)
		{
			Error=FREAD2_ERROR;
			goto Cleanup;
		}

		if(memcmp(p1,p2,Size1))
		{
			Error=MEMCMP_ERROR;
			goto Cleanup;
		}

Cleanup:
	if(File1) { fclose(File1); }
	if(File2) { fclose(File2); }
	if(p1)    { free(p1); }
	if(p2)    { free(p2); }

	if(Error) {printf("%s() Error=%d\n",__func__,Error); }
	return Error;
}


int Test_ConvertDem(void)
{
	struct DEMConvertData data;
	char *filename="test_files/source/Alps.dem";
	short TestOnly=0;
//	int i;


	// --- Vista DEM -> Bin Array Signed 1 Byte -------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;  //  INPUT_FORMAT 0=binary Array, 1=WCS DEM, 2=Z-Buffer, 3=ASCII Array, 5=IFF, 6=DTED
	data.FormatCy[1]=0;  // OUTPUT_FORMAT 0=ARRAY, 1=WCSDEM, 2=ZBUF,3=COLORMAP,4=GRAYIFF, 5=COLORIFF
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrS1");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrS1","ram:tst_AlpsBinArrS1")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Signed 1 Byte passed\n");


	// --- Vista DEM -> Bin Array Signed 2 Bytes ------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=1;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrS2");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrS2","ram:tst_AlpsBinArrS2")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Signed 2 Bytes passed\n");

	// --- Vista DEM -> Bin Array Signed 4 Bytes ------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=2;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrS4");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrS4","ram:tst_AlpsBinArrS4")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Signed 4 Bytes passed\n");

	// --- Vista DEM -> Bin Array Unsigned 1 Byte -----------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=1;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrU1");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrU1","ram:tst_AlpsBinArrU1")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Unsigned 1 Byte passed\n");

	// --- Vista DEM -> Bin Array Unsigned 2 Bytes ----------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=1;
	data.FormatCy[9]=1;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrU2");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrU2","ram:tst_AlpsBinArrU2")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Unsigned 2 Bytes passed\n");





	// --- Vista DEM -> Bin Array Unsigned 4 Bytes ----------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=1;
	data.FormatCy[9]=2;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrU4");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrU4","ram:tst_AlpsBinArrU4")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Unsigned 4 Bytes passed\n");


	// --- Vista DEM -> Bin Array Float 4 Bytes -------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=2;
	data.FormatCy[9]=2;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrF4");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrF4","ram:tst_AlpsBinArrF4")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Float 4 Bytes passed\n");

	// --- Vista DEM -> Bin Array Float 8 Bytes -------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=0;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=2;
	data.FormatCy[9]=3;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsBinArrF8");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsBinArrF8","ram:tst_AlpsBinArrF8")==0);
	printf("ConvertDem(Vista DEM -> Bin Array Float 8 Bytes passed\n");

	// --- Vista DEM -> WCS DEM -----------------------------------------------------------------------------------------------

	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=1;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_Alps");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_Alps  .Obj","ram:tst_Alps  .Obj")==0);    // Spaces and .OBJ are automatically appended
	assert(CompareFileExactly("test_files/reference/ref_Alps  .elev","ram:ref_Alps  .elev")==0);  // Spaces and .elev are automatically appended

	printf("ConvertDem(Vista DEM -> Bin Array Float 8 Bytes passed\n");


	// --- Vista DEM -> ZBuffer -----------------------------------------------------------------------------------------------
	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=2;
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_Alps");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsZB","ram:tst_AlpsZB")==0);  // ZB is automatically appended
	printf("ConvertDem(Vista DEM -> Bin Array Float 8 Bytes passed\n");


#ifdef TEST_COLORMAP
	// --- Vista DEM -> Color Map ---------------------------------------------------------------------------------------------

	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=3;  // Dest Color Map
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsColor");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsColor","ram:tst_AlpsColor")==0);
	printf("ConvertDem(Vista DEM -> Gray Color Map passed\n");

#endif

	// --- Vista DEM -> Gray IFF ---------------------------------------------------------------------------------------------

	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=4;  // Dest Gray IFF
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsGray.iff");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsGray.iff","ram:tst_AlpsGray.iff")==0);
	printf("ConvertDem(Vista DEM -> Gray IFF passed\n");

	// --- Vista DEM -> Color IFF ---------------------------------------------------------------------------------------------

	data.ActiveFC[0]=0;
	data.ActiveFC[1]=0;
	data.Crop[0]=0;
	data.Crop[1]=0;
	data.Crop[2]=0;
	data.Crop[3]=0;
	data.FloorCeiling[0]=0.000000;
	data.FloorCeiling[1]=0.000000;
	data.FormatCy[0]=4;
	data.FormatCy[1]=5;     // Dest Color IFF
	data.FormatCy[2]=0;
	data.FormatCy[3]=1;
	data.FormatCy[4]=0;
	data.FormatCy[5]=0;
	data.FormatCy[6]=0;
	data.FormatCy[7]=1;
	data.FormatCy[8]=0;
	data.FormatCy[9]=0;
	data.FormatInt[0]=0;
	data.FormatInt[1]=258;
	data.FormatInt[2]=258;
	data.FormatInt[3]=0;
	data.FormatInt[4]=0;
	data.LateralScale[0]=0.069428;
	data.LateralScale[1]=0.000000;
	data.LateralScale[2]=180.069427;
	data.LateralScale[3]=180.000000;
	data.MaxMin[0]=0.000000;
	data.MaxMin[1]=0.000000;
	snprintf(data.NameBase,24,"tst_AlpsColor.iff");
	snprintf(data.OutputDir,256,"Ram Disk:");
	data.OutputMaps[0]=1;
	data.OutputMaps[1]=1;
	data.ScaleType=0;
	data.SplineConstrain=0;
	data.VSOperator=0;
	data.VertScale[0]=0.000000;
	data.VertScale[1]=0.000000;
	data.VertScale[2]=0.000000;
	data.VertScale[3]=0.000000;
	data.VertScale[4]=0.000000;
	data.VertScale[5]=0.000000;
	data.VertScale[6]=0.000000;
	data.VertScale[7]=0.000000;
	data.VertScale[8]=0.000000;
	data.WrapLon=0;

	ConvertDEM(&data, filename, TestOnly);
//	for(i=0;i< 2;i++) { printf("2) data.MaxMin[%d]=%f;\n",i,data.MaxMin[i]); }

	// now compare the resulting file against a WCS.204 reference file
	assert(CompareFileExactly("test_files/reference/ref_AlpsColor.iff","ram:tst_AlpsColor.iff")==0);
	printf("ConvertDem(Vista DEM -> Color IFF passed\n");


	return 0;
}



int main(void)
{
	/* init global variables */
	dbaseloaded = 1;    // must be 1 if destination format is WCS DEM
    paramsloaded = 0;

	Test_ConvertDem();
	return 0;
}
