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

unsigned int printMessages=0;

USHORT User_Message_Def(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey, int Default)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,message);
	}
	return 0;
}

USHORT User_Message(CONST_STRPTR outlinetxt, CONST_STRPTR message, CONST_STRPTR buttons,
	CONST_STRPTR buttonkey)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,message);
	}
	return 0;
}

void Log(USHORT StdMesgNum, CONST_STRPTR LogTag)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s() %s\n",__func__,LogTag);
	}
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
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return (struct BusyWindow *)1; // invalid address but newer used in this test
}

void BusyWin_Del(struct BusyWindow *This)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
}


short getfilename(long mode, char *requestname, char *pathname,
    char *filename)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return 0;
}

void Set_DM_HdrData(struct USGS_DEMHeader *Hdr)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
}

short makesky(short renderseg, struct Window *win)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return 0;
}

void ScreenPixelPlot(struct Window *win, UBYTE **Bitmap, short x, short y, long zip)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
}

short GetInputString(char *message, char *reject, char *string)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return 0;
}

short Set_DM_Data(struct DEMExtractData *DEMExtract)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return 1;
}

short Add_DE_NewItem(void)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return 1;
}

struct database *DataBase_Expand(struct database *OldBase, short OldRecords,
	short OldUsedRecords, short NewRecords)
{
	static char *DataBase="Database";
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return (struct database *)DataBase;   // something that is not NULL
}

void Set_DM_ProfData(struct USGS_DEMProfileHeader *ProfHdr)
{
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
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
#define OPEN_FILE1_ERROR      1
#define OPEN_FILE2_ERROR      2
#define FILESIZE1_ERROR       3
#define FILESIZE2_ERROR       4
#define SIZE_DIFFERENT_ERROR  5
#define ALLOC1_ERROR          6
#define ALLOC2_ERROR          7
#define FREAD1_ERROR          8
#define FREAD2_ERROR          9
#define MEMCMP_ERROR         10

#define READ_VER1_ERROR      11
#define READ_VER2_ERROR      12
#define READ_ELMAPHDR1_ERROR 13
#define READ_ELMAPHDR2_ERROR 14


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

// --- for CmpObjFiles -----------------

int fpcmp(double val1, double val2)
{
	if( fabs(val1-val2) > 0.001 )
	{
		return 1;  // Werte verschieden
	}
	else
	{
		return 0;  // Werte gleich
	}
}

// AF: 22.Mar.23 correct endian if necessary and write
// should be in BigEndianReadWrite ?
long freadVectorheaderV100_BE(struct vectorheaderV100 *Hdr, FILE *file)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

    long Result=fread(Hdr, sizeof (struct vectorheaderV100), 1, file);

    SimpleEndianFlip32S(Hdr->points, &Hdr->points);
    SimpleEndianFlip16S(Hdr->elevs, &Hdr->elevs);
    SimpleEndianFlip64(Hdr->avglat ,&Hdr->avglat);
    SimpleEndianFlip64(Hdr->avglon ,&Hdr->avglon);
    SimpleEndianFlip64(Hdr->avgelev ,&Hdr->avgelev);
    SimpleEndianFlip64(Hdr->elscale ,&Hdr->elscale);
    SimpleEndianFlip16S(Hdr->MaxEl, &Hdr->MaxEl);
    SimpleEndianFlip16S(Hdr->MinEl, &Hdr->MinEl);

    return Result;

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just write as it is
    return fread(Hdr, sizeof (struct vectorheaderV100), 1, file);
#else
#error "Unsupported Byte-Order"
#endif
}


int CmpObjFiles(char *FileName1, char *FileName2)
{
	int Error=0;
	FILE *File1=NULL;
	FILE *File2=NULL;

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

	// compare filesizes
	long Size1=GetFileSize(File1);
	if(Size1<0)
	{
		Error=FILESIZE1_ERROR;
		goto Cleanup;
	}

	long Size2=GetFileSize(File2);
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


	char Filetype1[10]={0}; // inkl abschliessender 0 fuer printf
	if(fread(Filetype1,9,1,File1)!=1)
	{
		printf("File1 error reading Filetype1\n");
		Error= 1;
		goto Cleanup;
	}

	char Filetype2[10]={0};  // inkl abschliessender 0 fuer printf
	if(fread(Filetype2,9,1,File2)!=1)
	{
		printf("File1 error reading Filetype2\n");
		Error=1;
		goto Cleanup;
	}

	float Version1, Version2;

	if(fread(&Version1,sizeof(float),1,File1)!=1)
	{
		printf("File1 error reading Version1\n");
		Error= 1;
		goto Cleanup;

	}


	if(fread(&Version2,sizeof(float),1,File2)!=1)
	{
		printf("File1 error reading Version2\n");
		Error= 1;
		goto Cleanup;
	}


	struct vectorheaderV100 Hdr1, Hdr2;

	if(freadVectorheaderV100_BE(&Hdr1, File1)!=1)
	{
		printf("File1 error reeading header\n");
		Error= 1;
		goto Cleanup;
	}


	if(freadVectorheaderV100_BE(&Hdr2, File2)!=1)
	{
		printf("File1 error reeading header\n");
		Error= 1;
		goto Cleanup;
	}

#ifdef PRINT_OBJ_COMPARISON
	printf("Filetype: %s       %s",Filetype1,Filetype2);        if(strcmp(Filetype1,Filetype2))      {printf("   <---"); Error=1;} printf("\n");
	printf("Version:  %f       %f",Version1,Version2);          if(fpcmp(Version1,Version2))         {printf("   <---"); Error=1;} printf("\n");
	printf("------ Start of Header ----\n");
	printf("Name:     %s       %s (ignored, can be different)\n",Hdr1.Name,Hdr2.Name);
	printf("points:   %d       %d",Hdr1.points,Hdr2.points);    if(Hdr1.points != Hdr2.points)       {printf("   <---"); Error=1;} printf("\n");
	printf("elevs:    %d       %d",Hdr1.elevs,Hdr2.elevs);      if(Hdr1.elevs != Hdr2.elevs)         {printf("   <---"); Error=1;} printf("\n");
	printf("avglat:   %f       %f",Hdr1.avglat,Hdr2.avglat);    if(fpcmp(Hdr1.avglat,Hdr2.avglat))   {printf("   <---"); Error=1;} printf("\n");
	printf("avglon:   %f       %f",Hdr1.avglon,Hdr2.avglon);    if(fpcmp(Hdr1.avglon,Hdr2.avglon))   {printf("   <---"); Error=1;} printf("\n");
	printf("avgelev:  %f       %f",Hdr1.avgelev,Hdr2.avgelev);  if(fpcmp(Hdr1.avgelev,Hdr2.avgelev)) {printf("   <---"); Error=1;} printf("\n");
	printf("elscale:  %f       %f",Hdr1.elscale,Hdr2.elscale);  if(fpcmp(Hdr1.elscale,Hdr2.elscale)) {printf("   <---"); Error=1;} printf("\n");
	printf("MaxEl:    %d       %d (ignored, not initialized in original WCS)\n",Hdr1.MaxEl,Hdr2.MaxEl);
	printf("MinEl:    %d       %d (ignored, not initialized in original WCS)\n",Hdr1.MinEl,Hdr2.MinEl);

	unsigned int i;
	printf("----- End of Header ------\n");
	for(i=0;i<Hdr1.points+1;i++)
	{
		double Lon1,Lon2;
		fread_double_BE(&Lon1,File1);
		fread_double_BE(&Lon2,File2);
		printf("Lon[%d]=%f   %f",i,Lon1,Lon2); if(fpcmp(Lon1,Lon2))   {printf("   <---"); Error=1;} printf("\n");
	}

	for(i=0;i<Hdr1.points+1;i++)
	{
		double Lat1,Lat2;
		fread_double_BE(&Lat1,File1);
		fread_double_BE(&Lat2,File2);
		printf("Lat[%d]=%f   %f",i,Lat1,Lat2); if(fpcmp(Lat1,Lat2))   {printf("   <---"); Error=1;} printf("\n");
	}

	for(i=0;i<Hdr1.points+1;i++)
	{
		short Elev1,Elev2;
		fread_short_BE(&Elev1,File1);
		fread_short_BE(&Elev2,File2);
		printf("Elev[%d]=%d   %d",i,Elev1,Elev2); if(Elev1 != Elev2)   {printf("   <---"); Error=1;} printf("\n");
	}

	printf("\n");
    if(Error==0)
    {
    	printf("OK, identical.\n");
    }
    else
    {
    	printf("Error!!! There are differences!\n");
    }
#else
#warning compare fehlt hier!
#endif


    Cleanup:
    	if(File1) { fclose(File1); }
    	if(File2) { fclose(File2); }
	return Error;
}

// --------------------------------------------------------------------------
// AF: 21.Apr.23 read and correct endian if necessary
// fread-Version needed in this test
long freadElMapHeaderV101_BE(struct elmapheaderV101 *Hdr,FILE *File)
{
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	long Result=fread(Hdr, ELEVHDRLENV101,1,File);
	SimpleEndianFlip32S(Hdr->rows, &Hdr->rows);
	SimpleEndianFlip32S(Hdr->columns, &Hdr->columns);
	SimpleEndianFlip64(Hdr->lolat,&Hdr->lolat);
	SimpleEndianFlip64(Hdr->lolong,&Hdr->lolong);
	SimpleEndianFlip64(Hdr->steplat,&Hdr->steplat);
	SimpleEndianFlip64(Hdr->steplong,&Hdr->steplong);
	SimpleEndianFlip64(Hdr->elscale,&Hdr->elscale);
	SimpleEndianFlip16S(Hdr->MaxEl,&Hdr->MaxEl);
	SimpleEndianFlip16S(Hdr->MinEl,&Hdr->MinEl);
	SimpleEndianFlip32S(Hdr->Samples,&Hdr->Samples);
	SimpleEndianFlip32F(Hdr->SumElDif,&Hdr->SumElDif);
	SimpleEndianFlip32F(Hdr->SumElDifSq,&Hdr->SumElDifSq);
	SimpleEndianFlip32S(Hdr->size,&Hdr->size);
	SimpleEndianFlip32S(Hdr->scrnptrsize,&Hdr->scrnptrsize);
	SimpleEndianFlip32S(Hdr->fractalsize,&Hdr->fractalsize);
	SimpleEndianFlip32S(Hdr->facept[0],&Hdr->facept[0]);
	SimpleEndianFlip32S(Hdr->facept[1],&Hdr->facept[1]);
	SimpleEndianFlip32S(Hdr->facept[2],&Hdr->facept[2]);
	SimpleEndianFlip32S(Hdr->facect,&Hdr->facect);
	SimpleEndianFlip32S(Hdr->fracct,&Hdr->fracct);
	SimpleEndianFlip32S(Hdr->Lr,&Hdr->Lr);
	SimpleEndianFlip32S(Hdr->Lc,&Hdr->Lc);
	SimpleEndianFlip16S(Hdr->MapAsSFC,&Hdr->MapAsSFC);
	SimpleEndianFlip16S(Hdr->ForceBath,&Hdr->ForceBath);
	SimpleEndianFlip32F(Hdr->LonRange,&Hdr->LonRange);
    SimpleEndianFlip32F(Hdr->LatRange,&Hdr->LatRange);
    return Result;

#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // just read as it is
    return fread(Hdr, ELEVHDRLENV101,1,File);
#else
#error "Unsupported Byte-Order"
#endif

}

#define ELHDRINFO(x) x
// define ELHDRINFO(x)
int CmpElevFiles(char *FileName1, char *FileName2)
{
	int Error=0;
	FILE *File1=NULL;
	FILE *File2=NULL;

	printf("Compare <%s> and <%s>\n",FileName1,FileName2);

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

	// compare filesizes
	long Size1=GetFileSize(File1);
	if(Size1<0)
	{
		Error=FILESIZE1_ERROR;
		goto Cleanup;
	}

	long Size2=GetFileSize(File2);
	if(Size2<0)
	{
		Error=FILESIZE2_ERROR;
		goto Cleanup;
	}

	if(Size1!=Size2)
	{
		Error=SIZE_DIFFERENT_ERROR;
		printf("File size differs!\n");
		goto Cleanup;
	}


	float Version1=0;
	float Version2=0;
	struct elmapheaderV101 Hdr1;
	struct elmapheaderV101 Hdr2;

	if(fread_float_BE(&Version1,File1)!=1)
	{
		printf("Failed to read version from %s\n",FileName1);
		Error=READ_VER1_ERROR;
		goto Cleanup;
	}

	if(freadElMapHeaderV101_BE(&Hdr1,File1)!=1)
	{
		printf("Failed to read ElMapHeader from %s\n",FileName1);
		Error=READ_ELMAPHDR1_ERROR;
		goto Cleanup;
	}

	if(fread_float_BE(&Version2,File2)!=1)
	{
		printf("Failed to read version from %s\n",FileName2);
		Error=READ_VER2_ERROR;
		goto Cleanup;
	}


	if(freadElMapHeaderV101_BE(&Hdr2,File2)!=1)
	{
		printf("Failed to read ElMapHeader from %s\n",FileName2);
		Error=READ_ELMAPHDR2_ERROR;
		goto Cleanup;
	}

	printf("Version1=%f\n",Version1);
	printf("Version2=%f\n",Version2);

	if(fpcmp(Version1,Version2))
	{
		printf("Versions differ! %f  %f\n",Version1,Version2);
		Error=1;
	}

	// now compare ElMapHeaderV101 headers
	printf("rows:        %d   %d",Hdr1.rows,        Hdr2.rows);        if(      Hdr1.rows        != Hdr2.rows)         { Error=1; printf(" <---");} printf("\n");
	printf("columns:     %d   %d",Hdr1.columns,     Hdr2.columns);     if(      Hdr1.columns     != Hdr2.columns)      { Error=1; printf(" <---");} printf("\n");
	printf("lolat:       %f   %f",Hdr1.lolat,       Hdr2.lolat);       if(fpcmp(Hdr1.lolat ,        Hdr2.lolat))       { Error=1; printf(" <---");} printf("\n");
	printf("lolong:      %f   %f",Hdr1.lolong,      Hdr2.lolong);      if(fpcmp(Hdr1.lolong,        Hdr2.lolong))      { Error=1; printf(" <---");} printf("\n");
	printf("steplat:     %f   %f",Hdr1.lolat,       Hdr2.lolat);       if(fpcmp(Hdr1.steplat,       Hdr2.steplat))     { Error=1; printf(" <---");} printf("\n");
	printf("steplong:    %f   %f",Hdr1.steplat,     Hdr2.steplat);     if(fpcmp(Hdr1.steplong,      Hdr2.steplong))    { Error=1; printf(" <---");} printf("\n");
	printf("elscale:     %f   %f",Hdr1.elscale,     Hdr2.elscale);     if(fpcmp(Hdr1.elscale,       Hdr2.elscale))     { Error=1; printf(" <---");} printf("\n");
	printf("MaxEl:       %d   %d",Hdr1.MaxEl,       Hdr2.MaxEl);	   if(      Hdr1.MaxEl       != Hdr2.MaxEl)        { Error=1; printf(" <---");} printf("\n");
	printf("MinEl:       %d   %d",Hdr1.MinEl,       Hdr2.MinEl);       if(      Hdr1.MinEl       != Hdr2.MinEl)        { Error=1; printf(" <---");} printf("\n");
	printf("Samples:     %d   %d",Hdr1.Samples,     Hdr2.Samples);	   if(      Hdr1.Samples     != Hdr2.Samples)      { Error=1; printf(" <---");} printf("\n");
	printf("SumElDif:    %f   %f",Hdr1.SumElDif,    Hdr2.SumElDif);    if(fpcmp(Hdr1.SumElDif,      Hdr2.SumElDif))    { Error=1; printf(" <---");} printf("\n");
	printf("SumElDifSq:  %f   %f",Hdr1.SumElDifSq,  Hdr2.SumElDifSq);  if(fpcmp(Hdr1.SumElDifSq,    Hdr2.SumElDifSq))  { Error=1; printf(" <---");} printf("\n");

// alle weiteren nicht benutzt in DataOps.c !?
#warning Nicht alle Felder des elev-Headers getestet!

//	// short	*map;
//	// LONG	*lmap;
//	printf("size:        %d   %d",Hdr1.size,       Hdr2.size);         if(      Hdr1.size        != Hdr2.size)         { Error=1; printf(" <---");} printf("\n");
//	printf("scrnptrsize: %d   %d",Hdr1.scrnptrsize,Hdr2.scrnptrsize);  if(      Hdr1.scrnptrsize != Hdr2.scrnptrsize)  { Error=1; printf(" <---");} printf("\n");
//	printf("fractalsize: %d   %d",Hdr1.fractalsize,Hdr2.fractalsize);  if(      Hdr1.fractalsize != Hdr2.fractalsize)  { Error=1; printf(" <---");} printf("\n");
	// float	*scrnptrx,
	// *scrnptry,
	// *scrnptrq;
	// struct	faces *face;
	// BYTE	*fractal;
//	printf("facept[0]:   %d   %d",Hdr1.facept[0],Hdr2.facept[0]);	   if(      Hdr1.facept[0]   != Hdr2.facept[0])    { Error=1; printf(" <---");} printf("\n");
//	printf("facept[1]:   %d   %d",Hdr1.facept[1],Hdr2.facept[1]);	   if(      Hdr1.facept[1]   != Hdr2.facept[1])    { Error=1; printf(" <---");} printf("\n");
//	printf("facept[2]:   %d   %d",Hdr1.facept[2],Hdr2.facept[2]);      if(      Hdr1.facept[2]   != Hdr2.facept[2])    { Error=1; printf(" <---");} printf("\n");
//	printf("facect:      %d   %d",Hdr1.facect,   Hdr2.facect);         if(      Hdr1.facect      != Hdr2.facect)       { Error=1; printf(" <---");} printf("\n");
//	printf("fracct:      %d   %d",Hdr1.fracct,   Hdr2.fracct);         if(      Hdr1.fracct      != Hdr2.fracct)       { Error=1; printf(" <---");} printf("\n");
//	printf("Lr:          %d   %d",Hdr1.Lr,       Hdr2.Lr);             if(      Hdr1.Lr          != Hdr2.Lr)           { Error=1; printf(" <---");} printf("\n");
//	printf("Lc:          %d   %d",Hdr1.Lc,       Hdr2.Lc);             if(      Hdr1.Lc          != Hdr2.Lc)           { Error=1; printf(" <---");} printf("\n");
//	printf("MapAsSFC:    %d   %d",Hdr1.MapAsSFC, Hdr2.MapAsSFC);       if(      Hdr1.MapAsSFC    != Hdr2.MapAsSFC)     { Error=1; printf(" <---");} printf("\n");
//	printf("ForceBath:   %d   %d",Hdr1.ForceBath,Hdr2.ForceBath);	   if(      Hdr1.ForceBath   != Hdr2.ForceBath)    { Error=1; printf(" <---");} printf("\n");
//	printf("LonRange:    %f   %f",Hdr1.LonRange, Hdr2.LonRange);       if(fpcmp(Hdr1.LonRange,      Hdr2.LonRange))    { Error=1; printf(" <---");} printf("\n");
//	printf("LatRange:    %f   %f",Hdr1.LatRange, Hdr2.LatRange);       if(fpcmp(Hdr1.LatRange,      Hdr2.LatRange))    { Error=1; printf(" <---");} printf("\n");
//

	printf("Hdr1.rows*Hdr1.columns=%d\n",Hdr1.rows*Hdr1.columns);
	// now the rest
	unsigned int i;
	for(i=0;i<(Hdr1.rows+1)*Hdr1.columns;i++)
	{
		short Elev1,Elev2;
		if((fread_short_BE(&Elev1,File1)!=1) ||
		   (fread_short_BE(&Elev2,File2)!=1))
		   {
			printf("Read Elevation Values falied!\n");
			Error=1;
			goto Cleanup;
		   }
		if(fpcmp(Elev1,Elev2)) { Error=1; printf("Pixel %d differs!\n",i); goto Cleanup;}
	}

	if(ftell(File1)!=Size1)
	{
		printf("Not at end of file after reading elevation data!\n");
		Error=1;
		goto Cleanup;
	}

    Cleanup:
    	if(File1) { fclose(File1); }
    	if(File2) { fclose(File2); }
	return Error;
}

// --------------------------------------------------------------------------
/* Copied from DataOps.c */
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


char *inFormatStrings[]=
{
"Bin Array ",
"WCSDEM ",
"ZBUF ",
"ASCII ",
"VISTA ",
"IFF ",
"DTED "
};

char *outFormatStrings[]=
{
"Bin Array ",
"WCSDEM ",
"ZBUF ",
"COLORMAP ",
"GRAYIFF ",
"COLORIFF "
};

char *dataFormatStrings[]=
{
"Signed Int ",
"Unsigned Int ",
"FltPt ",
"FORMAT_UNKNOWN "
};

char *dataValSizeStrings[]=
{
"1 ",
"2 ",
"4 ",
"8 ",
"VALSIZE_UNKNOWN "
};

struct ConvertDemTestStruct
{
	int InFormat;
	int InValueFormat;
	int InValueSize;
	int OutFormat;
	int OutValueFormat;
	int OutValueSize;
	char *outDir;
	char *outNameBase;
	char *refFileName;
};

void InitDEMConvertData(struct DEMConvertData *data, struct ConvertDemTestStruct *ConvertDemTestData )
{
	data->ActiveFC[0]=0;
	data->ActiveFC[1]=0;
	data->Crop[0]=0;
	data->Crop[1]=0;
	data->Crop[2]=0;
	data->Crop[3]=0;
	data->FloorCeiling[0]=0.000000;
	data->FloorCeiling[1]=0.000000;
	data->FormatCy[0]=ConvertDemTestData->InFormat;      //  INPUT_FORMAT 0=binary Array, 1=WCS DEM, 2=Z-Buffer, 3=ASCII Array, 5=IFF, 6=DTED
	data->FormatCy[1]=ConvertDemTestData->OutFormat;     // OUTPUT_FORMAT 0=ARRAY, 1=WCSDEM, 2=ZBUF,3=COLORMAP,4=GRAYIFF, 5=COLORIFF
	data->FormatCy[2]=ConvertDemTestData->InValueFormat;
	data->FormatCy[3]=ConvertDemTestData->InValueSize;
	data->FormatCy[4]=0;
	data->FormatCy[5]=0;
	data->FormatCy[6]=0;
	data->FormatCy[7]=1;
	data->FormatCy[8]=ConvertDemTestData->OutValueFormat;
	data->FormatCy[9]=ConvertDemTestData->OutValueSize;
	data->FormatInt[0]=0;
	data->FormatInt[1]=258;
	data->FormatInt[2]=258;
	data->FormatInt[3]=0;
	data->FormatInt[4]=0;
	data->LateralScale[0]=0.069428;
	data->LateralScale[1]=0.000000;
	data->LateralScale[2]=180.069427;
	data->LateralScale[3]=180.000000;
	data->MaxMin[0]=0.000000;
	data->MaxMin[1]=0.000000;
	snprintf(data->NameBase,24,ConvertDemTestData->outNameBase);
	snprintf(data->OutputDir,256,ConvertDemTestData->outDir);
	data->OutputMaps[0]=1;
	data->OutputMaps[1]=1;
	data->ScaleType=0;
	data->SplineConstrain=0;
	data->VSOperator=0;
	data->VertScale[0]=0.000000;
	data->VertScale[1]=0.000000;
	data->VertScale[2]=0.000000;
	data->VertScale[3]=0.000000;
	data->VertScale[4]=0.000000;
	data->VertScale[5]=0.000000;
	data->VertScale[6]=0.000000;
	data->VertScale[7]=0.000000;
	data->VertScale[8]=0.000000;
	data->WrapLon=0;
}



struct ConvertDemTestStruct ConverDemTestData[]=
{
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,    "Ram:WCS_Test/", "tst_AlpsBinArrS1",  "test_files/reference/ref_AlpsBinArrS1",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,   "Ram:WCS_Test/", "tst_AlpsBinArrS2",  "test_files/reference/ref_AlpsBinArrS2",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,    "Ram:WCS_Test/", "tst_AlpsBinArrS4",  "test_files/reference/ref_AlpsBinArrS4",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,    "Ram:WCS_Test/", "tst_AlpsBinArrU1",  "test_files/reference/ref_AlpsBinArrU1",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,   "Ram:WCS_Test/", "tst_AlpsBinArrU2",  "test_files/reference/ref_AlpsBinArrU2",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,    "Ram:WCS_Test/", "tst_AlpsBinArrU4",  "test_files/reference/ref_AlpsBinArrU4",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    "Ram:WCS_Test/", "tst_AlpsBinArrF4",  "test_files/reference/ref_AlpsBinArrF4",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,  "Ram:WCS_Test/", "tst_AlpsBinArrF8",  "test_files/reference/ref_AlpsBinArrF8",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, "Ram:WCS_Test/", "tst_Alps",          "test_files/reference/ref_Alps  ",        },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, "Ram:WCS_Test/", "tst_Alps",          "test_files/reference/ref_AlpsZB",        },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, "Ram:WCS_Test/", "tst_Alps",          "test_files/reference/ref_Alps  ",        },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, "Ram:WCS_Test/", "tst_AlpsGray.iff",  "test_files/reference/ref_AlpsGray.iff",  },
		{ DEM_DATA_INPUT_VISTA, DEM_DATA_FORMAT_UNKNOWN, DEM_DATA_VALSIZE_UNKNOWN, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, "Ram:WCS_Test/", "tst_AlpsColor.iff", "test_files/reference/ref_AlpsColor.iff", }
};


int Test_ConvertDem(void)
{
	struct DEMConvertData data;
	char *filename="test_files/source/Alps.dem";
#define TEST_ONLY 1
#define NO_TEST_ONLY 0
	unsigned int testIndex;
	unsigned int Errors=0;

	printf("Anzahl Tests=%d\n\n",sizeof(ConverDemTestData)/sizeof(struct ConvertDemTestStruct));


	for(testIndex=0;testIndex<sizeof(ConverDemTestData)/sizeof(struct ConvertDemTestStruct);testIndex++)
	{
		char *TestNameSourceFormat;
		char *TestNameSourceValueFormat;
		char *TestNameSourceValueSize;

		char *TestNameDestFormat;
		char *TestNameDestValueFormat;
		char *TestNameDestValueSize;

		static char tstFileName[256];
		static char refFileNameExtended[256];
		char *tstFileEnding;
		static char tempOutFilename[256]={0};

		TestNameSourceFormat=inFormatStrings[ConverDemTestData[testIndex].InFormat];    // DEM_DATA_INPUT_VISTA
		TestNameSourceValueFormat="";
		TestNameSourceValueSize="";

		TestNameDestFormat=outFormatStrings[ConverDemTestData[testIndex].OutFormat];
		TestNameDestValueFormat="";
		TestNameDestValueSize="";


		if(ConverDemTestData[testIndex].InFormat==DEM_DATA_INPUT_ARRAY)
		{
			TestNameSourceValueFormat=dataFormatStrings[ConverDemTestData[testIndex].InValueFormat];  // Unsigned int
			TestNameSourceValueSize=dataValSizeStrings[ConverDemTestData[testIndex].InValueSize];     // 4
		}

		if(ConverDemTestData[testIndex].OutFormat==DEM_DATA_OUTPUT_ARRAY)
		{
			TestNameDestValueFormat=dataFormatStrings[ConverDemTestData[testIndex].OutValueFormat];
			TestNameDestValueSize=dataValSizeStrings[ConverDemTestData[testIndex].OutValueSize];
		}

		printf("%2d %s%s%s-> %s%s%s ",testIndex+1,
				TestNameSourceFormat,TestNameSourceValueFormat,TestNameSourceValueSize,
				TestNameDestFormat,  TestNameDestValueFormat,  TestNameDestValueSize);

		InitDEMConvertData(&data,&ConverDemTestData[testIndex]);

		ConvertDEM(&data, filename, TEST_ONLY);
		assert(data.MaxMin[0]==253);   // min Elevation
		assert(data.MaxMin[1]==1385);  // min Elevation of Alps.dem

		ConvertDEM(&data, filename, NO_TEST_ONLY);

		// special handling for automatic filename-extensions
		tstFileEnding="";
		if(ConverDemTestData[testIndex].OutFormat==DEM_DATA_OUTPUT_ZBUF)
		{
			tstFileEnding="ZB";
		}

		// append spaces to the end or trim name until length is length[0] (global WCS variable, i.e. 10)
		// needed for CLORMAP and WCSDEM
		sprintf(tempOutFilename,"%s",ConverDemTestData[testIndex].outNameBase);
		while (strlen(tempOutFilename) < length[0])
		{
			strcat(tempOutFilename, " ");         // append spaces
		}
		tempOutFilename[length[0]] = 0;           // trim name


		// COLOR_MAP and WCSDEM need special care as they have more than 1 resulting file
		switch(ConverDemTestData[testIndex].OutFormat)
		{
			case DEM_DATA_OUTPUT_COLORMAP:
			{
				// now compare the resulting file against a WCS.204 reference file
				int i;
				char *rgbEnding[]={".red",".grn",".blu"};
				int rgbError=0;


				for(i=0;i<3;i++)
				{
					snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,tempOutFilename,rgbEnding[i]);
					snprintf(refFileNameExtended,256,"%s%s",ConverDemTestData[testIndex].refFileName,rgbEnding[i]);

					if(!CompareFileExactly(refFileNameExtended,tstFileName)==0)
					{
						rgbError++;
					}
				}

				if(rgbError==0)
				{
					printf("passed\n");
				}
				else
				{
					printf("failed\n");
					Errors++;
				}

				break;
			}
			case DEM_DATA_OUTPUT_WCSDEM:
			{

				// once for the elev-File
				snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,tempOutFilename,".elev");
				snprintf(refFileNameExtended,256,"%s%s",ConverDemTestData[testIndex].refFileName,".elev");

				if(!CmpElevFiles(refFileNameExtended,tstFileName)==0)
				{
					printf("- Elev-Files problem - ");
					Errors++;
				}

				// and once for the Obj-File
				snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,tempOutFilename,".Obj");
				snprintf(refFileNameExtended,256,"%s%s",ConverDemTestData[testIndex].refFileName,".Obj");

				if(CmpObjFiles(refFileNameExtended,tstFileName)!=0)
				{
					printf("- Obj-Files problem - ");
					Errors++;
				}

				if(Errors==0)
				{
					printf("passed\n");
				}
				else
				{
					printf("failed\n");
				}


				break;
			}
			default:
			{
				// now compare the resulting file against a WCS.204 reference file
				snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,ConverDemTestData[testIndex].outNameBase,tstFileEnding);
				if(CompareFileExactly(ConverDemTestData[testIndex].refFileName,tstFileName)==0)
				{
					printf("passed\n");
				}
				else
				{
					printf("failed\n");
					Errors++;
				}
			}
		}
	}

	return Errors;
}

// #############################################################
// POSIX dependencies
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>

// from https://stackoverflow.com/questions/5467725/how-to-delete-a-directory-and-its-contents-in-posix-c

int rmtree(const char path[])
{
    size_t path_len;
    char *full_path;
    DIR *dir;
    struct stat stat_path, stat_entry;
    struct dirent *entry;

    // stat for the path
    stat(path, &stat_path);

    // if path does not exists or is not dir - exit with status -1
    if (S_ISDIR(stat_path.st_mode) == 0) {
        fprintf(stderr, "%s: %s\n", "Is not directory", path);
        return 0;
    }

    // if not possible to read the directory for this user
    if ((dir = opendir(path)) == NULL) {
        fprintf(stderr, "%s: %s\n", "Can`t open directory", path);
        return 0;
    }

    // the length of the path
    path_len = strlen(path);

    // iteration through entries in the directory
    while ((entry = readdir(dir)) != NULL) {

        // skip entries "." and ".."
        //if (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, ".."))  // AF, weglassen, . ist gueltiger name
        //    continue;

        // determinate a full path of an entry
        full_path = calloc(path_len + strlen(entry->d_name) + 1, sizeof(char));
        strcpy(full_path, path);
        strcat(full_path, "/");
        strcat(full_path, entry->d_name);

        // stat for the entry
        stat(full_path, &stat_entry);

        // recursively remove a nested directory
        if (S_ISDIR(stat_entry.st_mode) != 0) {
            rmtree(full_path);
            continue;
        }

        // remove a file object
        if (DeleteFile((STRPTR)full_path))  // was: (unlink(full_path)==0), but that printed the filename to console before deleting it???
        {
            //printf("Removed a file: %s\n", full_path);
        }
        else
        {
            printf("Can`t remove a file: %s\n", full_path);
        }
        free(full_path);
    }
    closedir(dir);  // moved closedir() from end to here, AF, 20.April.23 otherwise I get "file busy" when calling rmdir()
    // remove the devastated directory and close the object of it
    if (rmdir(path) == 0)
    {
         // printf("Removed a directory: %s\n", path);
    }
    else
    {
        printf("Can`t remove a directory: %s\n", path);
        printf("Error-Text: %s\n",strerror(errno));
        return 0;
    }
    // closedir(dir);
    return 1;
}
// #############################################################



int main(void)
{
	/* init used global(!) variables */
	dbaseloaded = 1;    // must be 1 if destination format is WCS DEM
    paramsloaded = 0;   // ?
    length[0] = 10;     // set in ./DataBase.c:342, seems to be fixed length of filename (without extension) for Obj and elev files (and reg/grn/blu)

    int Errors;

    //rmtree("Ram:WCS_Test");

    int Res=Mkdir("Ram:WCS_Test");
    if(Res!=0)
    {
    	printf("Res=%d! MKdir failed!\n",Res);
    }

	Errors=Test_ConvertDem();
	if(Errors!=0)
	{
		printf("\n\7%2d test failed!!!\n",Errors);
	}
	else
	{
	   printf("\nAll tests passed.\n");
	}
	return 0;
}
