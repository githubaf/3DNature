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

#include <errno.h>

unsigned int printMessages=0;
unsigned int Verbose=0;
unsigned int Compare_SumElDifSq=0;     // The value of Compare_SumElDifSq seems to be compiler and/or operating-system-dependant... !!!??? AF, 30.6.2023


// the inline functions. exactly once with extern and without inline.
#if defined  __AROS__ && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
extern void SimpleEndianFlip64 (double Source64, double *Dest64);  // AF, 12Dec22 for i386-aros
extern inline void SimpleEndianFlip32F(float Source32, float  *Dest32);  // AF, 10Dec22 for i386-aros
extern void SimpleEndianFlip32U( ULONG Source32, ULONG *Dest32);  // AF, 10Dec22 for i386-aros
extern void SimpleEndianFlip32S( LONG Source32, LONG *Dest32);  //AF, 10Dec22 for i386-aros
extern void SimpleEndianFlip16U(USHORT Source16, USHORT *Dest16);
extern void SimpleEndianFlip16S(SHORT Source16, SHORT *Dest16);
#endif

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
	static struct database DataBase[20]={0};  // must be big enough for the test!
	if(printMessages)
	{
		fprintf(stderr,"ALEXANDER: %s()\n",__func__);
	}
	return DataBase;   // something that is not NULL
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
	printf("%s() Dummy-Function called!\n",__func__);
}


// dummy-mock, original in InteractiveView.c
short initinterview(short boundsdiscrim)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}


void UTM_LatLon(struct UTMLatLonCoords *Coords)
{
	printf("%s() Dummy-Function called!\n",__func__);
}


// from Maputil.c
void LatLon_UTM(struct UTMLatLonCoords *Coords, short UTMZone)
{
}

// from Maputil.c
void UTMLatLonCoords_Init(struct UTMLatLonCoords *Coords, short UTMZone)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//from MapUtil.c
double Point_Extract(double X, double Y, double MinX, double MinY,
	 double IntX, double IntY, short *Data, long Rows, long Cols)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}

// MapGUI.c
void MapGUI_Message(short line, char *Message)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

// MapUtil.c
void ClipDraw(struct Window *win, struct clipbounds *cb, struct lineseg *ls)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

// MapUtil.c
void ClipDrawRPort(struct RastPort *Rast, struct clipbounds *cb, struct lineseg *ls)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//MapGUI.c
void MapIDCMP_Restore(struct Window *win)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//Support.c
struct Window *FetchMultiWindowEvent(struct IntuiMessage *Event, ...)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}

//MapUtil.c
void setclipbounds(struct Window *win, struct clipbounds *cb)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void drawgridview(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveUtils.c
void setcompass(short focusaz)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void constructview(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void DrawInterFresh(short drawgrid)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void makeviewcenter(short erase)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void drawveryquick(short undraw)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void drawquick(short a, short b, short c, short Clear)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void computequick(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
void drawfocprof(short erase)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//EdMoGUI.c
void Update_EM_Item(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveDraw.c
short drawgridpts(short erase)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}

//InteractiveUtils.c
void azimuthcompute(short focusaz, double flat, double flon,
          double tlat, double tlon)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//InteractiveUtils.c
void findfocprof(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//DataBase.c
short Load_Object(short i, char **LastDir)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}

//Support.c
void FetchEvent(struct Window *Win, struct IntuiMessage *Event)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//Params.c
void FreeKeyTable(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
}

//Params.c
short BuildKeyTable(void)
{
	printf("%s() Dummy-Function called!\n",__func__);
	return 0;
}

// ##############################################################################################################
// print only, if verbose is set
void my_printf(int verbose, const char *format, ...)
{
    if (verbose)
    {
        va_list args;
        va_start(args, format);
        vfprintf(stderr, format, args);
        va_end(args);
    }
}


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

char *CompareFileExactly_ErrorStrings[]={
		"OK",
		"OPEN_FILE1_ERROR",
		"OPEN_FILE2_ERROR",
		"FILESIZE1_ERROR",
		"FILESIZE2_ERROR",
		"SIZE_DIFFERENT_ERROR",
		"ALLOC1_ERROR",
		"ALLOC2_ERROR",
		"FREAD1_ERROR",
		"FREAD2_ERROR",
		"MEMCMP_ERROR",

		"READ_VER1_ERROR",
		"READ_VER2_ERROR",
		"READ_ELMAPHDR1_ERROR",
		"READ_ELMAPHDR2_ERROR"
	};

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
	if( fabs(val1-val2) > 0.0001 )
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
		printf("File1 error reading header\n");
		Error= 1;
		goto Cleanup;
	}


    if(strcmp(Filetype1,Filetype2))      {Error=1;}
    if(fpcmp(Version1,Version2))         {Error=1;}

    if(Hdr1.points != Hdr2.points)       {Error=1;}
    if(Hdr1.elevs != Hdr2.elevs)         {Error=1;}
    if(fpcmp(Hdr1.avglat,Hdr2.avglat))   {Error=1;}
    if(fpcmp(Hdr1.avglon,Hdr2.avglon))   {Error=1;}
    if(fpcmp(Hdr1.avgelev,Hdr2.avgelev)) {Error=1;}
    if(fpcmp(Hdr1.elscale,Hdr2.elscale)) {Error=1;}


	my_printf(Error || Verbose,"Filetype: %s       %s",Filetype1,Filetype2);        if(strcmp(Filetype1,Filetype2))      {printf("   <---\n");}
	my_printf(Error || Verbose,"Version:  %f       %f",Version1,Version2);          if(fpcmp(Version1,Version2))         {printf("   <---\n");}
	my_printf(Error || Verbose,"------ Start of Header ----\n");
	my_printf(Error || Verbose,"Name:     %s       %s (ignored, can be different)\n",Hdr1.Name,Hdr2.Name);
	my_printf(Error || Verbose,"points:   %d       %d",Hdr1.points,Hdr2.points);    if(Hdr1.points != Hdr2.points)       {printf("   <---\n");}
	my_printf(Error || Verbose,"elevs:    %d       %d",Hdr1.elevs,Hdr2.elevs);      if(Hdr1.elevs != Hdr2.elevs)         {printf("   <---\n");}
	my_printf(Error || Verbose,"avglat:   %f       %f",Hdr1.avglat,Hdr2.avglat);    if(fpcmp(Hdr1.avglat,Hdr2.avglat))   {printf("   <---\n");}
	my_printf(Error || Verbose,"avglon:   %f       %f",Hdr1.avglon,Hdr2.avglon);    if(fpcmp(Hdr1.avglon,Hdr2.avglon))   {printf("   <---\n");}
	my_printf(Error || Verbose,"avgelev:  %f       %f",Hdr1.avgelev,Hdr2.avgelev);  if(fpcmp(Hdr1.avgelev,Hdr2.avgelev)) {printf("   <---\n");}
	my_printf(Error || Verbose,"elscale:  %f       %f",Hdr1.elscale,Hdr2.elscale);  if(fpcmp(Hdr1.elscale,Hdr2.elscale)) {printf("   <---\n");}
	my_printf(Error || Verbose,"MaxEl:    %d       %d (ignored, not initialized in original WCS)\n",Hdr1.MaxEl,Hdr2.MaxEl);
	my_printf(Error || Verbose,"MinEl:    %d       %d (ignored, not initialized in original WCS)\n",Hdr1.MinEl,Hdr2.MinEl);

	unsigned int i;
	my_printf(Error || Verbose,"----- End of Header ------\n");
	for(i=0;i<Hdr1.points+1;i++)
	{
		double Lon1,Lon2;
		fread_double_BE(&Lon1,File1);
		fread_double_BE(&Lon2,File2);
		Error=fpcmp(Lon1,Lon2);
		my_printf(Error || Verbose,"Lon[%d]=%f   %f",i,Lon1,Lon2); if(Error)   {printf("   <---"); } my_printf(Error || Verbose,"\n");
	}

	for(i=0;i<Hdr1.points+1;i++)
	{
		double Lat1,Lat2;
		fread_double_BE(&Lat1,File1);
		fread_double_BE(&Lat2,File2);
		Error=fpcmp(Lat1,Lat2);
		my_printf(Error || Verbose,"Lat[%d]=%f   %f",i,Lat1,Lat2); if(Error)   {printf("   <---"); } my_printf(Error || Verbose,"\n");
	}

	for(i=0;i<Hdr1.points+1;i++)
	{
		short Elev1,Elev2;
		fread_short_BE(&Elev1,File1);
		fread_short_BE(&Elev2,File2);
		Error=(Elev1 != Elev2);
		my_printf(Error || Verbose,"Elev[%d]=%d   %d",i,Elev1,Elev2); if(Elev1 != Elev2)   {printf("   <---"); } my_printf(Error || Verbose,"\n");
	}

	my_printf(Error || Verbose,"\n");
   	my_printf(Verbose,"OK, identical.\n");
    my_printf(Error,"Error!!! There are differences!\n");

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
	/*  not stored in file
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
    */
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

	//printf("Compare <%s> and <%s>\n",FileName1,FileName2);

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
	struct elmapheaderV101 Hdr1={0};
	struct elmapheaderV101 Hdr2={0};

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

	my_printf(Verbose,"Version1=%f\n",Version1);
	my_printf(Verbose,"Version2=%f\n",Version2);

	if(fpcmp(Version1,Version2))
	{
		printf("Versions differ! %f  %f\n",Version1,Version2);
		Error=1;
	}

	// now compare ElMapHeaderV101 headers
	if(      Hdr1.rows        != Hdr2.rows)         { Error=1; }
	if(      Hdr1.columns     != Hdr2.columns)      { Error=1; }
	if(fpcmp(Hdr1.lolat ,        Hdr2.lolat))       { Error=1; }
	if(fpcmp(Hdr1.lolong,        Hdr2.lolong))      { Error=1; }
	if(fpcmp(Hdr1.steplat,       Hdr2.steplat))     { Error=1; }
	if(fpcmp(Hdr1.steplong,      Hdr2.steplong))    { Error=1; }
	if(fpcmp(Hdr1.elscale,       Hdr2.elscale))     { Error=1; }
	if(      Hdr1.MaxEl       != Hdr2.MaxEl)        { Error=1; }
	if(      Hdr1.MinEl       != Hdr2.MinEl)        { Error=1; }
	if(      Hdr1.Samples     != Hdr2.Samples)      { Error=1; }
	if(fpcmp(Hdr1.SumElDif,      Hdr2.SumElDif))    { Error=1; }
	if (Compare_SumElDifSq)
	{
		if(fpcmp(Hdr1.SumElDifSq,    Hdr2.SumElDifSq))  { Error=1; }  // This values seems to be compiler and/or operating-system-dependant... !!!??? AF, 30.6.2023
	}

	my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"rows:        %d   %d",Hdr1.rows,        Hdr2.rows);        if(      Hdr1.rows        != Hdr2.rows)         {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"columns:     %d   %d",Hdr1.columns,     Hdr2.columns);     if(      Hdr1.columns     != Hdr2.columns)      {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"lolat:       %f   %f",Hdr1.lolat,       Hdr2.lolat);       if(fpcmp(Hdr1.lolat ,        Hdr2.lolat))       {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"lolong:      %f   %f",Hdr1.lolong,      Hdr2.lolong);      if(fpcmp(Hdr1.lolong,        Hdr2.lolong))      {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"steplat:     %f   %f",Hdr1.steplat,     Hdr2.steplat);     if(fpcmp(Hdr1.steplat,       Hdr2.steplat))     {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"steplong:    %f   %f",Hdr1.steplong,    Hdr2.steplong);    if(fpcmp(Hdr1.steplong,      Hdr2.steplong))    {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"elscale:     %f   %f",Hdr1.elscale,     Hdr2.elscale);     if(fpcmp(Hdr1.elscale,       Hdr2.elscale))     {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"MaxEl:       %d   %d",Hdr1.MaxEl,       Hdr2.MaxEl);	   if(      Hdr1.MaxEl       != Hdr2.MaxEl)        {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"MinEl:       %d   %d",Hdr1.MinEl,       Hdr2.MinEl);       if(      Hdr1.MinEl       != Hdr2.MinEl)        {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"Samples:     %d   %d",Hdr1.Samples,     Hdr2.Samples);	   if(      Hdr1.Samples     != Hdr2.Samples)      {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"SumElDif:    %f   %f",Hdr1.SumElDif,    Hdr2.SumElDif);    if(fpcmp(Hdr1.SumElDif,      Hdr2.SumElDif))    {printf(" <---");} my_printf(Error || Verbose,"\n");
	my_printf(Error || Verbose,"SumElDifSq:  %f   %f",Hdr1.SumElDifSq,  Hdr2.SumElDifSq);  if(fpcmp(Hdr1.SumElDifSq,    Hdr2.SumElDifSq))  {printf(" <---");} my_printf(Error || Verbose,"\n");


	// the other elements of ElMapHeaderV101 are not stored in the elev files! (read/write only ELEVHDRLENV101 bytes)


	//printf("Hdr1.rows*Hdr1.columns=%d\n",Hdr1.rows*Hdr1.columns);

	// now the elev data array
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

#define DEM_DATA_UNITS_KILOM		0
#define DEM_DATA_UNITS_METERS		1
#define DEM_DATA_UNITS_CENTIM		2
#define DEM_DATA_UNITS_MILES		3
#define DEM_DATA_UNITS_FEET         4
#define DEM_DATA_UNITS_INCHES		5
#define DEM_DATA_UNITS_OTHER		6


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
	char *TestName;        // free text to display for this test
	char *SourceFileName;
	int InFormat;
	int InValueFormat;
	int InValueSize;
	int HeaderBytes;
	int Rows;
	int Cols;
	int MinEl;
	int MaxEl;
	int DataUnit;        // m, km, ...
	int OutFormat;
	int OutValueFormat;
	int OutValueSize;
	int OutRows;
	int OutCols;
    short SplineConstrain;
	float HiLat;         //0.069428;      // Koordianten, LateralScale[0]
	float LoLat;         //0.000000;      // Koordianten, LateralScale[1]
	float HiLong;        //180.069427;    // Koordianten, LateralScale[2]
	float LoLong;        //180.000000;    // Koordianten, LateralScale[3]
	char *outDir;
	char *outNameBase;
	char *refFileName;
	long LineNumber;     // Codeline of test definition
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
	data->FormatCy[7]=ConvertDemTestData->DataUnit;     // ZBuf->WCSDEM will 0 haben; // DATA_UNITS,  0=KILOM, 1=METERS, 2=CENTIM, 3=MILES, 4=FEET, 5=INCHES, 6=Other
	data->FormatCy[8]=ConvertDemTestData->OutValueFormat;
	data->FormatCy[9]=ConvertDemTestData->OutValueSize;
	data->FormatInt[0]=ConvertDemTestData->HeaderBytes;
	data->FormatInt[1]=ConvertDemTestData->Rows;
	data->FormatInt[2]=ConvertDemTestData->Cols;
	data->FormatInt[3]=ConvertDemTestData->OutRows;     // OUTPUT_ROWS
	data->FormatInt[4]=ConvertDemTestData->OutCols;     // OUTPUT_COLS
	data->LateralScale[0]=ConvertDemTestData->HiLat;
	data->LateralScale[1]=ConvertDemTestData->LoLat;
	data->LateralScale[2]=ConvertDemTestData->HiLong;
	data->LateralScale[3]=ConvertDemTestData->LoLong;
	data->MaxMin[0]=0.000000;
	data->MaxMin[1]=0.000000;
	snprintf(data->NameBase,24,ConvertDemTestData->outNameBase);
	snprintf(data->OutputDir,256,ConvertDemTestData->outDir);
	data->OutputMaps[0]=1;
	data->OutputMaps[1]=1;
	data->ScaleType=0;
	data->SplineConstrain=ConvertDemTestData->SplineConstrain;
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
   //     SourceFileName,                 InFormat,              InValueFormat,           InValueSize,      HeaderBytes,Rows,Cols,MinEl,MaxEl,     OutFormat,              OutValueFormat,           OutValueSize,              Rows, Cols, Spline Constraint,  Hi_Lat,   Lo_Lat,  Hi_Long,    Lo_Long,       outDir,        outNameBase,               refFileName
#define oritzroiuzoritzoiu
#ifdef oritzroiuzoritzoiu
   // header-size is 0 and does not matter for DEM_DATA_INPUT_VISTA
   { "VistaProDEM -> Array Int 1",   "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrS1",    "test_files/reference/ref_BSurBinArrS1",__LINE__   },
   { "VistaProDEM -> Array Int 2",   "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrS2",    "test_files/reference/ref_BSurBinArrS2",__LINE__   },
   { "VistaProDEM -> Array Int 4",   "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrS4",    "test_files/reference/ref_BSurBinArrS4",__LINE__   },
   { "VistaProDEM -> Array UInt 1",  "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrU1",    "test_files/reference/ref_BSurBinArrU1",__LINE__   },
   { "VistaProDEM -> Array UInt 2",  "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrU2",    "test_files/reference/ref_BSurBinArrU2",__LINE__   },
   { "VistaProDEM -> Array UInt 4",  "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrU4",    "test_files/reference/ref_BSurBinArrU4",__LINE__   },
   { "VistaProDEM -> Array Float 4", "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrF4",    "test_files/reference/ref_BSurBinArrF4",__LINE__   },
   { "VistaProDEM -> Array Float 8", "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurBinArrF8",    "test_files/reference/ref_BSurBinArrF8",__LINE__   },
   { "VistaProDEM -> WCSDEM",        "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSur",            "test_files/reference/ref_BSur  ",__LINE__         },
   { "VistaProDEM -> ZBUF",          "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSur",            "test_files/reference/ref_BSurZB",__LINE__         },
   { "VistaProDEM -> COLORMAP",      "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSur",            "test_files/reference/ref_BSur  ",__LINE__         },
   { "VistaProDEM -> GRAYIFF",       "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurGray.iff",    "test_files/reference/ref_BSurGray.iff",__LINE__   },
   { "VistaProDEM -> COLORIFF",      "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSurColor.iff",   "test_files/reference/ref_BSurColor.iff",__LINE__  },


   // header-size is 68 DEM_DATA_INPUT_WCSDEM. data->LateralScale[]=0,0,0,0 -> Data is taken from Input-WCS-Header
   { "WCSDEM -> Array Int 1",  "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IS1",        "test_files/reference/ref_36112IS1",__LINE__       },
   { "WCSDEM -> Array Int 2",  "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IS2",        "test_files/reference/ref_36112IS2",__LINE__       },
   { "WCSDEM -> Array Int 4",  "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IS4",        "test_files/reference/ref_36112IS4",__LINE__       },
   { "WCSDEM -> Array UInt 1", "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IU1",        "test_files/reference/ref_36112IU1",__LINE__       },
   { "WCSDEM -> Array UInt 2", "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IU2",        "test_files/reference/ref_36112IU2",__LINE__       },
   { "WCSDEM -> Array UInt 4", "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IU4",        "test_files/reference/ref_36112IU4",__LINE__       },
   { "WCSDEM -> Array Float 4","test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IF4",        "test_files/reference/ref_36112IF4",__LINE__       },
   { "WCSDEM -> Array Float 8","test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IF8",        "test_files/reference/ref_36112IF8",__LINE__       },
   { "WCSDEM -> WCSDEM",       "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112I",          "test_files/reference/ref_36112I",__LINE__         },
   { "WCSDEM -> ZBUF",         "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112I",          "test_files/reference/ref_36112IZB",__LINE__       },
   { "WCSDEM -> COLORMAP",     "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112I",          "test_files/reference/ref_36112I",__LINE__         },
   { "WCSDEM -> GRAYIFF",      "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IGray.iff",  "test_files/reference/ref_36112IGray.iff",__LINE__ },
   { "WCSDEM -> COLORIFF",     "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112IColor.iff", "test_files/reference/ref_36112IColor.iff",__LINE__},

   // header-size is 64 DEM_DATA_INPUT_ZBUF InValueSize needs to be DEM_DATA_VALSIZE_LONG, ZBuf->WCSDEM needs DEM_DATA_UNITS_KILOM
   { "ZBUF -> Array Int 1",  "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBS1",        "test_files/reference/ref_BSurZBS1",__LINE__       },
   { "ZBUF -> Array Int 2",  "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBS2",        "test_files/reference/ref_BSurZBS2",__LINE__       },
   { "ZBUF -> Array Int 4",  "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBS4",        "test_files/reference/ref_BSurZBS4",__LINE__       },
   { "ZBUF -> Array UInt 1", "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBU1",        "test_files/reference/ref_BSurZBU1",__LINE__       },
   { "ZBUF -> Array UInt 2", "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBU2",        "test_files/reference/ref_BSurZBU2",__LINE__       },
   { "ZBUF -> Array UInt 4", "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBU4",        "test_files/reference/ref_BSurZBU4",__LINE__       },
   { "ZBUF -> Array Float 4","test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBF4",        "test_files/reference/ref_BSurZBF4",__LINE__       },
   { "ZBUF -> Array Float 8","test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBF8",        "test_files/reference/ref_BSurZBF8",__LINE__       },
   { "ZBUF -> WCSDEM",       "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_KILOM,  DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZB",          "test_files/reference/ref_BSurZB",__LINE__         },
   { "ZBUF -> ZBUF",         "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZB",          "test_files/reference/ref_BSurZBZB",__LINE__       },
   { "ZBUF -> COLORMAP",     "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZB",          "test_files/reference/ref_BSurZB",__LINE__         },
   { "ZBUF -> GRAYIFF",      "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBGray.iff",  "test_files/reference/ref_BSurZBGray.iff",__LINE__ },
   { "ZBUF -> COLORIFF",     "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZBColor.iff", "test_files/reference/ref_BSurZBColor.iff",__LINE__},

   // BIN Array S1
   { "Array Int 1 -> Array Int 1",  "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1S1",        "test_files/reference/ref_BSurS1S1",__LINE__       },
   { "Array Int 1 -> Array Int 2",  "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1S2",        "test_files/reference/ref_BSurS1S2",__LINE__       },
   { "Array Int 1 -> Array Int 4",  "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1S4",        "test_files/reference/ref_BSurS1S4",__LINE__       },
   { "Array Int 1 -> Array UInt 1", "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1U1",        "test_files/reference/ref_BSurS1U1",__LINE__       },
   { "Array Int 1 -> Array UInt 2", "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1U2",        "test_files/reference/ref_BSurS1U2",__LINE__       },
   { "Array Int 1 -> Array UInt 4", "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1U4",        "test_files/reference/ref_BSurS1U4",__LINE__       },
   { "Array Int 1 -> Array Float 4","test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1F4",        "test_files/reference/ref_BSurS1F4",__LINE__       },
   { "Array Int 1 -> Array Float 8","test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1F8",        "test_files/reference/ref_BSurS1F8",__LINE__       },
   { "Array Int 1 -> WCSDEM",       "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1",          "test_files/reference/ref_BSurS1",__LINE__         },
   { "Array Int 1 -> ZBUF",         "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1",          "test_files/reference/ref_BSurS1ZB",__LINE__       },
   { "Array Int 1 -> COLORMAP",     "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1",          "test_files/reference/ref_BSurS1",__LINE__         },
   { "Array Int 1 -> GRAYIFF",      "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1Gray.iff",  "test_files/reference/ref_BSurS1Gray.iff",__LINE__ },
   { "Array Int 1 -> COLORIFF",     "test_files/source/BSur.DEMS1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  127, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS1Color.iff", "test_files/reference/ref_BSurS1Color.iff",__LINE__},

   // BIN Array S2
   { "Array Int 2 -> Array Int 1",  "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2S1",        "test_files/reference/ref_BSurS2S1",__LINE__       },
   { "Array Int 2 -> Array Int 2",  "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2S2",        "test_files/reference/ref_BSurS2S2",__LINE__       },
   { "Array Int 2 -> Array Int 4",  "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2S4",        "test_files/reference/ref_BSurS2S4",__LINE__       },
   { "Array Int 2 -> Array UInt 1", "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2U1",        "test_files/reference/ref_BSurS2U1",__LINE__       },
   { "Array Int 2 -> Array UInt 2", "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2U2",        "test_files/reference/ref_BSurS2U2",__LINE__       },
   { "Array Int 2 -> Array UInt 4", "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2U4",        "test_files/reference/ref_BSurS2U4",__LINE__       },
   { "Array Int 2 -> Array Float 4","test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2F4",        "test_files/reference/ref_BSurS2F4",__LINE__       },
   { "Array Int 2 -> Array Float 8","test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2F8",        "test_files/reference/ref_BSurS2F8",__LINE__       },
   { "Array Int 2 -> WCSDEM",       "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2",          "test_files/reference/ref_BSurS2",__LINE__         },
   { "Array Int 2 -> ZBUF",         "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2",          "test_files/reference/ref_BSurS2ZB",__LINE__       },
   { "Array Int 2 -> COLORMAP",     "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2",          "test_files/reference/ref_BSurS2",__LINE__         },
   { "Array Int 2 -> GRAYIFF",      "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2Gray.iff",  "test_files/reference/ref_BSurS2Gray.iff",__LINE__ },
   { "Array Int 2 -> COLORIFF",     "test_files/source/BSur.DEMS2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS2Color.iff", "test_files/reference/ref_BSurS2Color.iff",__LINE__},

   // BIN Array S4
   { "Array Int 4 -> Array Int 1",  "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4S1",        "test_files/reference/ref_BSurS4S1",__LINE__       },
   { "Array Int 4 -> Array Int 2",  "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4S2",        "test_files/reference/ref_BSurS4S2",__LINE__       },
   { "Array Int 4 -> Array Int 4",  "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4S4",        "test_files/reference/ref_BSurS4S4",__LINE__       },
   { "Array Int 4 -> Array UInt 1", "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4U1",        "test_files/reference/ref_BSurS4U1",__LINE__       },
   { "Array Int 4 -> Array UInt 2", "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4U2",        "test_files/reference/ref_BSurS4U2",__LINE__       },
   { "Array Int 4 -> Array UInt 4", "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4U4",        "test_files/reference/ref_BSurS4U4",__LINE__       },
   { "Array Int 4 -> Array Float 4","test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4F4",        "test_files/reference/ref_BSurS4F4",__LINE__       },
   { "Array Int 4 -> Array Float 8","test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4F8",        "test_files/reference/ref_BSurS4F8",__LINE__       },
   { "Array Int 4 -> WCSDEM",       "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4",          "test_files/reference/ref_BSurS4",__LINE__         },
   { "Array Int 4 -> ZBUF",         "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4",          "test_files/reference/ref_BSurS4ZB",__LINE__       },
   { "Array Int 4 -> COLORMAP",     "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4",          "test_files/reference/ref_BSurS4",__LINE__         },
   { "Array Int 4 -> GRAYIFF",      "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4Gray.iff",  "test_files/reference/ref_BSurS4Gray.iff",__LINE__ },
   { "Array Int 4 -> COLORIFF",     "test_files/source/BSur.DEMS4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurS4Color.iff", "test_files/reference/ref_BSurS4Color.iff",__LINE__},

   // BIN Array U1
   { "Array UInt 1 -> Array Int 1",  "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1S1",        "test_files/reference/ref_BSurU1S1",__LINE__       },
   { "Array UInt 1 -> Array Int 2",  "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1S2",        "test_files/reference/ref_BSurU1S2",__LINE__       },
   { "Array UInt 1 -> Array Int 4",  "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1S4",        "test_files/reference/ref_BSurU1S4",__LINE__       },
   { "Array UInt 1 -> Array UInt 1", "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1U1",        "test_files/reference/ref_BSurU1U1",__LINE__       },
   { "Array UInt 1 -> Array UInt 2", "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1U2",        "test_files/reference/ref_BSurU1U2",__LINE__       },
   { "Array UInt 1 -> Array UInt 4", "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1U4",        "test_files/reference/ref_BSurU1U4",__LINE__       },
   { "Array UInt 1 -> Array Float 4","test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1F4",        "test_files/reference/ref_BSurU1F4",__LINE__       },
   { "Array UInt 1 -> Array Float 8","test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1F8",        "test_files/reference/ref_BSurU1F8",__LINE__       },
   { "Array UInt 1 -> WCSDEM",       "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1",          "test_files/reference/ref_BSurU1",__LINE__         },
   { "Array UInt 1 -> ZBUF",         "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1",          "test_files/reference/ref_BSurU1ZB",__LINE__       },
   { "Array UInt 1 -> COLORMAP",     "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1",          "test_files/reference/ref_BSurU1",__LINE__         },
   { "Array UInt 1 -> GRAYIFF",      "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1Gray.iff",  "test_files/reference/ref_BSurU1Gray.iff",__LINE__ },
   { "Array UInt 1 -> COLORIFF",     "test_files/source/BSur.DEMU1",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU1Color.iff", "test_files/reference/ref_BSurU1Color.iff",__LINE__},

   // BIN Array U2
   { "Array UInt 2 -> Array Int 1",  "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2S1",        "test_files/reference/ref_BSurU2S1",__LINE__       },
   { "Array UInt 2 -> Array Int 2",  "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2S2",        "test_files/reference/ref_BSurU2S2",__LINE__       },
   { "Array UInt 2 -> Array Int 4",  "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2S4",        "test_files/reference/ref_BSurU2S4",__LINE__       },
   { "Array UInt 2 -> Array UInt 1", "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2U1",        "test_files/reference/ref_BSurU2U1",__LINE__       },
   { "Array UInt 2 -> Array UInt 2", "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2U2",        "test_files/reference/ref_BSurU2U2",__LINE__       },
   { "Array UInt 2 -> Array UInt 4", "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2U4",        "test_files/reference/ref_BSurU2U4",__LINE__       },
   { "Array UInt 2 -> Array Float 4","test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2F4",        "test_files/reference/ref_BSurU2F4",__LINE__       },
   { "Array UInt 2 -> Array Float 8","test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2F8",        "test_files/reference/ref_BSurU2F8",__LINE__       },
   { "Array UInt 2 -> WCSDEM",       "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2",          "test_files/reference/ref_BSurU2",__LINE__         },
   { "Array UInt 2 -> ZBUF",         "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2",          "test_files/reference/ref_BSurU2ZB",__LINE__       },
   { "Array UInt 2 -> COLORMAP",     "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2",          "test_files/reference/ref_BSurU2",__LINE__         },
   { "Array UInt 2 -> GRAYIFF",      "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2Gray.iff",  "test_files/reference/ref_BSurU2Gray.iff",__LINE__ },
   { "Array UInt 2 -> COLORIFF",     "test_files/source/BSur.DEMU2",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU2Color.iff", "test_files/reference/ref_BSurU2Color.iff",__LINE__},

//   // BIN Array U4
   { "Array UInt 4 -> Array Int 1",  "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4S1",        "test_files/reference/ref_BSurU4S1",__LINE__       },
   { "Array UInt 4 -> Array Int 2",  "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4S2",        "test_files/reference/ref_BSurU4S2",__LINE__       },
   { "Array UInt 4 -> Array Int 4",  "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4S4",        "test_files/reference/ref_BSurU4S4",__LINE__       },
   { "Array UInt 4 -> Array UInt 1", "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4U1",        "test_files/reference/ref_BSurU4U1",__LINE__       },
   { "Array UInt 4 -> Array UInt 2", "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4U2",        "test_files/reference/ref_BSurU4U2",__LINE__       },
   { "Array UInt 4 -> Array UInt 4", "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4U4",        "test_files/reference/ref_BSurU4U4",__LINE__       },
   { "Array UInt 4 -> Array Float 4","test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4F4",        "test_files/reference/ref_BSurU4F4",__LINE__       },
   { "Array UInt 4 -> Array Float 8","test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4F8",        "test_files/reference/ref_BSurU4F8",__LINE__       },
   { "Array UInt 4 -> WCSDEM",       "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4",          "test_files/reference/ref_BSurU4",__LINE__         },
   { "Array UInt 4 -> ZBUF",         "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4",          "test_files/reference/ref_BSurU4ZB",__LINE__       },
   { "Array UInt 4 -> COLORMAP",     "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4",          "test_files/reference/ref_BSurU4",__LINE__         },
   { "Array UInt 4 -> GRAYIFF",      "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4Gray.iff",  "test_files/reference/ref_BSurU4Gray.iff",__LINE__ },
   { "Array UInt 4 -> COLORIFF",     "test_files/source/BSur.DEMU4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurU4Color.iff", "test_files/reference/ref_BSurU4Color.iff",__LINE__},

   // BIN Array F4
   { "Array Float 4 -> Array Int 1",  "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4S1",        "test_files/reference/ref_BSurF4S1",__LINE__       },
   { "Array Float 4 -> Array Int 2",  "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4S2",        "test_files/reference/ref_BSurF4S2",__LINE__       },
   { "Array Float 4 -> Array Int 4",  "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4S4",        "test_files/reference/ref_BSurF4S4",__LINE__       },
   { "Array Float 4 -> Array UInt 1", "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4U1",        "test_files/reference/ref_BSurF4U1",__LINE__       },
   { "Array Float 4 -> Array UInt 2", "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4U2",        "test_files/reference/ref_BSurF4U2",__LINE__       },
   { "Array Float 4 -> Array UInt 4", "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4U4",        "test_files/reference/ref_BSurF4U4",__LINE__       },
   { "Array Float 4 -> Array Float 4","test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4F4",        "test_files/reference/ref_BSurF4F4",__LINE__       },
   { "Array Float 4 -> Array Float 8","test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4F8",        "test_files/reference/ref_BSurF4F8",__LINE__       },
   { "Array Float 4 -> WCSDEM",       "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4",          "test_files/reference/ref_BSurF4",__LINE__         },
   { "Array Float 4 -> ZBUF",         "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4",          "test_files/reference/ref_BSurF4ZB",__LINE__       },
   { "Array Float 4 -> COLORMAP",     "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4",          "test_files/reference/ref_BSurF4",__LINE__         },
   { "Array Float 4 -> GRAYIFF",      "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4Gray.iff",  "test_files/reference/ref_BSurF4Gray.iff",__LINE__ },
   { "Array Float 4 -> COLORIFF",     "test_files/source/BSur.DEMF4",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF4Color.iff", "test_files/reference/ref_BSurF4Color.iff",__LINE__},

//   // BIN Array F8
   { "Array Float 8 -> Array Int 1",  "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8S1",        "test_files/reference/ref_BSurF8S1",__LINE__       },
   { "Array Float 8 -> Array Int 2",  "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8S2",        "test_files/reference/ref_BSurF8S2",__LINE__       },
   { "Array Float 8 -> Array Int 4",  "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8S4",        "test_files/reference/ref_BSurF8S4",__LINE__       },
   { "Array Float 8 -> Array UInt 1", "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8U1",        "test_files/reference/ref_BSurF8U1",__LINE__       },
   { "Array Float 8 -> Array UInt 2", "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8U2",        "test_files/reference/ref_BSurF8U2",__LINE__       },
   { "Array Float 8 -> Array UInt 4", "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8U4",        "test_files/reference/ref_BSurF8U4",__LINE__       },
   { "Array Float 8 -> Array Float 4","test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8F4",        "test_files/reference/ref_BSurF8F4",__LINE__       },
   { "Array Float 8 -> Array Float 8","test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8F8",        "test_files/reference/ref_BSurF8F8",__LINE__       },
   { "Array Float 8 -> WCSDEM",       "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8",          "test_files/reference/ref_BSurF8",__LINE__         },
   { "Array Float 8 -> ZBUF",         "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8",          "test_files/reference/ref_BSurF8ZB",__LINE__       },
   { "Array Float 8 -> COLORMAP",     "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8",          "test_files/reference/ref_BSurF8",__LINE__         },
   { "Array Float 8 -> GRAYIFF",      "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8Gray.iff",  "test_files/reference/ref_BSurF8Gray.iff",__LINE__ },
   { "Array Float 8 -> COLORIFF",     "test_files/source/BSur.DEMF8",      DEM_DATA_INPUT_ARRAY,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurF8Color.iff", "test_files/reference/ref_BSurF8Color.iff",__LINE__},

   // ASCII -> need newer Compiler/libnix, otherwise to slow
   //Input-DataFormat defines also the output-Format of the colormap! If set to float/4bytes(long), the colormap-file will be also float/4bytes
   // in WCS I selected float/long for the source file. The destination must be float/long, too
   { "ASCII Array -> Array Int 1",  "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASS1",        "test_files/reference/ref_BSurASS1",__LINE__       },
   { "ASCII Array -> Array Int 2",  "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASS2",        "test_files/reference/ref_BSurASS2",__LINE__       },
   { "ASCII Array -> Array Int 4",  "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASS4",        "test_files/reference/ref_BSurASS4",__LINE__       },
   { "ASCII Array -> Array UInt 1", "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASU1",        "test_files/reference/ref_BSurASU1",__LINE__       },
   { "ASCII Array -> Array UInt 2", "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASU2",        "test_files/reference/ref_BSurASU2",__LINE__       },
   { "ASCII Array -> Array UInt 4", "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASU4",        "test_files/reference/ref_BSurASU4",__LINE__       },
   { "ASCII Array -> Array Float 4","test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASF4",        "test_files/reference/ref_BSurASF4",__LINE__       },
   { "ASCII Array -> Array Float 8","test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASF8",        "test_files/reference/ref_BSurASF8",__LINE__       },
   { "ASCII Array -> WCSDEM",       "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurAS",          "test_files/reference/ref_BSurAS",__LINE__         },
   { "ASCII Array -> ZBUF",         "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurAS",          "test_files/reference/ref_BSurASZB",__LINE__       },
   { "ASCII Array -> COLORMAP",     "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurAS",          "test_files/reference/ref_BSurAS",__LINE__         },
   { "ASCII Array -> GRAYIFF",      "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASGray.iff",  "test_files/reference/ref_BSurASGray.iff",__LINE__ },
   { "ASCII Array -> COLORIFF",     "test_files/source/BSur.DEMAS",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASColor.iff", "test_files/reference/ref_BSurASColor.iff",__LINE__},

   // IFF
   // BigSur.DEM -> Gray.iff. The Gray-IFF is source for the tests. 8-plane IFF file  Warum DEM_DATA_FORMAT_FLOAT???
   { "Gray IFF -> Array Int 1",  "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrS1",        "test_files/reference/ref_BSurGrS1",__LINE__       },
   { "Gray IFF -> Array Int 2",  "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrS2",        "test_files/reference/ref_BSurGrS2",__LINE__       },
   { "Gray IFF -> Array Int 4",  "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrS4",        "test_files/reference/ref_BSurGrS4",__LINE__       },
   { "Gray IFF -> Array UInt 1", "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrU1",        "test_files/reference/ref_BSurGrU1",__LINE__       },
   { "Gray IFF -> Array UInt 2", "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrU2",        "test_files/reference/ref_BSurGrU2",__LINE__       },
   { "Gray IFF -> Array UInt 4", "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrU4",        "test_files/reference/ref_BSurGrU4",__LINE__       },
   { "Gray IFF -> Array Float 4","test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrF4",        "test_files/reference/ref_BSurGrF4",__LINE__       },
   { "Gray IFF -> Array Float 8","test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrF8",        "test_files/reference/ref_BSurGrF8",__LINE__       },
   { "Gray IFF -> WCSDEM",       "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGr",          "test_files/reference/ref_BSurGr",__LINE__         },
   { "Gray IFF -> ZBUF",         "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGr",          "test_files/reference/ref_BSurGrZB",__LINE__       },
   { "Gray IFF -> COLORMAP",     "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGr",          "test_files/reference/ref_BSurGr",__LINE__         },
   { "Gray IFF -> GRAYIFF",      "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrGray.iff",  "test_files/reference/ref_BSurGrGray.iff",__LINE__ },
   { "Gray IFF -> COLORIFF",     "test_files/source/BSurDEMGray.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurGrColor.iff", "test_files/reference/ref_BSurGrColor.iff",__LINE__},

   // IFF
   // BigSur.DEM -> Color.iff. The Color-IFF is source for the tests. 24 bit deep IFF file Warum DEM_DATA_FORMAT_FLOAT???
   { "Color IFF -> Array Int 1",  "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoS1",        "test_files/reference/ref_BSurCoS1",__LINE__       },
   { "Color IFF -> Array Int 2",  "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoS2",        "test_files/reference/ref_BSurCoS2",__LINE__       },
   { "Color IFF -> Array Int 4",  "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoS4",        "test_files/reference/ref_BSurCoS4",__LINE__       },
   { "Color IFF -> Array UInt 1", "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoU1",        "test_files/reference/ref_BSurCoU1",__LINE__       },
   { "Color IFF -> Array UInt 2", "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoU2",        "test_files/reference/ref_BSurCoU2",__LINE__       },
   { "Color IFF -> Array UInt 4", "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoU4",        "test_files/reference/ref_BSurCoU4",__LINE__       },
   { "Color IFF -> Array Float 4","test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoF4",        "test_files/reference/ref_BSurCoF4",__LINE__       },
   { "Color IFF -> Array Float 8","test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoF8",        "test_files/reference/ref_BSurCoF8",__LINE__       },
   { "Color IFF -> WCSDEM",       "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCo",          "test_files/reference/ref_BSurCo",__LINE__         },
   { "Color IFF -> ZBUF",         "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCo",          "test_files/reference/ref_BSurCoZB",__LINE__       },
   { "Color IFF -> COLORMAP",     "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORMAP,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCo",          "test_files/reference/ref_BSurCo",__LINE__         },
   { "Color IFF -> GRAYIFF",      "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoGray.iff",  "test_files/reference/ref_BSurCoGray.iff",__LINE__ },
   { "Color IFF -> COLORIFF",     "test_files/source/BSurDEMColr.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurCoColor.iff", "test_files/reference/ref_BSurCoColor.iff",__LINE__},

   // DTED, from https://earthexplorer.usgs.gov/ -> Select your Data Sets-> Digital Elevation -> SRTM -> SRTM Void Filled
   // n54_e013_3arc_v2.dt1 is island of Ruegen
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_BYTE,     258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTS1",        "test_files/reference/ref_BSurDTS1",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTS2",        "test_files/reference/ref_BSurDTS2",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,     258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTS4",        "test_files/reference/ref_BSurDTS4",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_BYTE,     258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTU1",        "test_files/reference/ref_BSurDTU1",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_SHORT,    258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTU2",        "test_files/reference/ref_BSurDTU2",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_UNSIGNEDINT, DEM_DATA_VALSIZE_LONG,     258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTU4",        "test_files/reference/ref_BSurDTU4",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,     258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTF4",        "test_files/reference/ref_BSurDTF4",__LINE__       },
//   { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 258, 258,    0, 1122, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_ARRAY,   DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   258,258, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurDTF8",        "test_files/reference/ref_BSurDTF8",__LINE__       },
/*WCSDEM*/
	 //     SourceFileName,                                 InFormat,              InValueFormat,              InValueSize,         HeaderBytes, Rows,Cols, MinEl,MaxEl,                              OutFormat,          OutValueFormat,           OutValueSize,           OutRows, OutCols, Spline Constraint,  Hi_Lat,        Lo_Lat,       Hi_Long,      Lo_Long,            outDir,        outNameBase,               refFileName
     { "DTED 601x1201 -> WCSDEM 601x1201", "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0,        1201,601,  -18,  173, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,  DEM_DATA_VALSIZE_UNKNOWN,     1201,  601,          0,          55.000000,    54.000000,    -13.000000,   -14.000000,        "Ram:WCS_Test/", "tst_RuegDT",          "test_files/reference/ref_RuegenDT",__LINE__         },
     // Scaling
	 { "DTED 601x1201 -> WCSDEM 301x601", "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0,        1201,601,  -12,  170, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,  DEM_DATA_VALSIZE_UNKNOWN,      601,  301,          0,          55.000000,    54.000000,    -13.000000,   -14.000000,        "Ram:WCS_Test/", "tst_RuegSc",          "test_files/reference/ref_RuegenDTScale",__LINE__         },
     // other Scaling (Scaling x!=y) + Spline Constraints
	 { "DTED 601x1201 -> WCSDEM 301x301 Spline Contrain", "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0,        1201,601,  -12,  170, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,  DEM_DATA_VALSIZE_UNKNOWN,      301,  301,          1,          55.000000,    54.000000,    -13.000000,   -14.000000,        "Ram:WCS_Test/", "tst_RuegSS",          "test_files/reference/ref_RuegenDTScaleSplineConstr",__LINE__         },
	 // DTED -> Gray IFF no Scaling
//     { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0,        1201,601,  -18,  173, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF,  DEM_DATA_FORMAT_UNKNOWN,  DEM_DATA_VALSIZE_UNKNOWN,     1201,  601,          0,          55.000000,    54.000000,    -13.000000,   -14.000000,        "Ram:WCS_Test/", "tst_RuegDTGray.iff",          "test_files/reference/ref_RuegenDTGray.iff",__LINE__         },
	 // DTED -> Color IFF no scaling
//     { "test_files/source/n54_e013_3arc_v2.dt1",      DEM_DATA_INPUT_DTED,     DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,    0,        1201,601,  -18,  173, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,  DEM_DATA_FORMAT_UNKNOWN,  DEM_DATA_VALSIZE_UNKNOWN,     1201,  601,          0,          55.000000,    54.000000,    -13.000000,   -14.000000,        "Ram:WCS_Test/", "tst_RuegDTColor.iff",          "test_files/reference/ref_RuegenDTColor.iff",__LINE__         },


	 // -------------------------------------------------------------------
     // more IFF tests (x!=y, Scaling, Spline Constrain)

	 // IFF with x != y (Ruegen converted to gif and further to iff with gdal and image magic)
	 { "IFF 601x1201 -> Color IFF 601x1201",                "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  173, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  1201,601,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_Rug601x1201IFF.iff",          "test_files/reference/ref_RuegenIFF_601x1201.iff",__LINE__         },
     { "IFF 601x1201 -> Color IFF 301x601",                 "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  170, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,   601,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_Rug301x601_IFF.iff",          "test_files/reference/ref_RuegenIFF_301x601.iff",__LINE__         },
	 { "IFF 601x1201 -> Color IFF 301x301 Spline Contrain", "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  170, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,1, 0,        0,          0,          0,        "Ram:WCS_Test/",  "tst_Rug301x301SCIFF.iff",          "test_files/reference/ref_RuegenIFF_301x301SplnCnstr.iff",__LINE__         },

     // -------------------------------------------------------------------

	 // IFF 601x1201 -> Gray IFF 601x1201
//	 { "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  173, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  1201,601,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_Rug601x1201IFF.iff",          "test_files/reference/ref_RuegenIFF_601x1201Gray.iff",__LINE__         },

	 // IFF 601x1201 -> Gray IFF 301x601   geht nicht, nur halbes Bild
//	   { "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  255, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  601,301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_Rug301x601_IFF.iff",          "test_files/reference/ref_Ruegen301x601Gray.iff",__LINE__         },

     // IFF 601x1201 -> Gray IFF 301x301 Spline Constrain
	 { "IFF 601x1201 -> Gray IFF 301x301 Spline Constrain", "test_files/source/n54_e013_3arc_v2.iff", DEM_DATA_INPUT_IFF,      DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_DOUBLE,   0, 1201, 601,    0,  170, DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF,   DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301,301,1,   0,        0,          0,          0,        "Ram:WCS_Test/",  "tst_Rg301301SCIFFGr.iff",          "test_files/reference/ref_RuegenIFF_301x301SplnCnstrGray.iff",__LINE__         },

     // -------------------------------------------------------------------

	 // ASCII 601x1201 -> Color IFF
	 { "ASCII Array 601x1201 -> Color IFF 601x1201",        "test_files/source/n54_e013_3arc_v2.ascarr", DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,   0, 1201, 601,    -18, 173,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN, 1201, 601,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "Rug601x1201ASColr.iff",  "test_files/reference/ref_RuegenASC_601x1201.iff",__LINE__},
	 { "ASCII Array 601x1201 -> Color IFF 301x601",         "test_files/source/n54_e013_3arc_v2.ascarr", DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,   0, 1201, 601,    -12, 170,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  601, 301,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "Rug301x601ASClr.iff",   "test_files/reference/ref_RuegenASC_301x601.iff", __LINE__},
	 { "ASCII Array 601x1201 -> Color IFF 301x601",         "test_files/source/n54_e013_3arc_v2.ascarr", DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_SHORT,   0, 1201, 601,    -12, 170,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_COLORIFF,DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  301, 301,1, 0,        0,          0,          0,        "Ram:WCS_Test/", "Rug301x301ASCSClr.iff", "test_files/reference/ref_RuegenASC_301x301SplnCnstrColor.iff", __LINE__},




	 // ASCII 601x1201 -> Gray IFF
//	 { "ASCII Array -> GRAYIFF",      "test_files/source/n54_e013_3arc_v2.ascarr",      DEM_DATA_INPUT_ASCII,    DEM_DATA_FORMAT_SIGNEDINT,   DEM_DATA_VALSIZE_LONG,   0, 258, 258,    0, 1122,   DEM_DATA_UNITS_METERS, DEM_DATA_OUTPUT_GRAYIFF, DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurASGray.iff",  "test_files/reference/ref_BSurASGray.iff",__LINE__ },


#else

// 1   /* einziger! ZBuf-Test, der fehlschlaegt! */  { "test_files/source/BigSur.DEM",        DEM_DATA_INPUT_VISTA,  DEM_DATA_FORMAT_UNKNOWN,   DEM_DATA_VALSIZE_UNKNOWN,  0, 258, 258,    0, 1122, DEM_DATA_OUTPUT_ZBUF,    DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0.069428, 0.000000, 180.069428, 180.000000, "Ram:WCS_Test/", "tst_BSur",            "test_files/reference/ref_BSurZB",__LINE__         },
// 1 (Elev)   /* geht schief! */   { "test_files/source/36112.I   .elev", DEM_DATA_INPUT_WCSDEM,   DEM_DATA_FORMAT_UNKNOWN,   DEM_DATA_VALSIZE_UNKNOWN, 68, 301, 301,  651, 2316, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_36112I",          "test_files/reference/ref_36112I",__LINE__         },
//  /* geht schief! */   { "test_files/source/BSur.DEMZB",      DEM_DATA_INPUT_ZBUF,     DEM_DATA_FORMAT_FLOAT,       DEM_DATA_VALSIZE_LONG,    64, 258, 258,    0, 1122, DEM_DATA_OUTPUT_WCSDEM,  DEM_DATA_FORMAT_UNKNOWN,     DEM_DATA_VALSIZE_UNKNOWN,  258,258,0, 0,        0,          0,          0,        "Ram:WCS_Test/", "tst_BSurZB",          "test_files/reference/ref_BSurZB",__LINE__         }

	 #endif

};


int Test_ConvertDem(void)
{
	struct DEMConvertData data;
	#define TEST_ONLY 1
#define NO_TEST_ONLY 0
	unsigned int testIndex;
	unsigned int Errors=0;

	printf("Anzahl Tests=%d\n\n",sizeof(ConverDemTestData)/sizeof(struct ConvertDemTestStruct));


	for(testIndex=0;testIndex<sizeof(ConverDemTestData)/sizeof(struct ConvertDemTestStruct);testIndex++)
	{
		char *filename;
		static char tstFileName[256];
		static char refFileNameExtended[256];
		char *tstFileEnding;
		static char tempOutFilename[256]={0};

		filename=ConverDemTestData[testIndex].SourceFileName;

		printf("%3d %s ",testIndex+1,ConverDemTestData[testIndex].TestName);

		InitDEMConvertData(&data,&ConverDemTestData[testIndex]);

		ConvertDEM(&data, filename, TEST_ONLY);
		// ConvertDEM() swaps four valus OUTPUT_ROWS/OUTPUT_COLS and INPUT_ROWS/INPUT_COLS in case of DETD. Undo it!
//		swmem(&data.FormatInt[3], &data.FormatInt[4], sizeof (short));  //   OUTPUT_ROWS/OUTPUT_COLS
//		swmem(&data.FormatInt[1], &data.FormatInt[2], sizeof (short));  //   INPUT_ROWS/INPUT_COLS


		if((data.MaxMin[0]!=ConverDemTestData[testIndex].MinEl) ||     // min Elevation
		   (data.MaxMin[1]!=ConverDemTestData[testIndex].MaxEl))      // max Elevation
		{
			Errors++;
			printf("Line %ld Min/Max Test failed\n",ConverDemTestData[testIndex].LineNumber);
		}

/* Damit geht es! */		InitDEMConvertData(&data,&ConverDemTestData[testIndex]);
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
					printf("Line %ld failed\n",ConverDemTestData[testIndex].LineNumber);
					Errors++;
				}

				break;
			}
			case DEM_DATA_OUTPUT_WCSDEM:
			{
				int CmpElevError=0, CmpObjError=0;

				// once for the elev-File
				snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,tempOutFilename,".elev");
				snprintf(refFileNameExtended,256,"%s%s",ConverDemTestData[testIndex].refFileName,".elev");

//printf("refFileNameExtended,tstFileName= <%s> und <%s>\n",refFileNameExtended,tstFileName);

				if(!CmpElevFiles(refFileNameExtended,tstFileName)==0)
				{
					printf("- Elev-Files problem - ");
					CmpElevError=1;
					Errors++;
				}

				// and once for the Obj-File
				snprintf(tstFileName,256,"%s%s%s",ConverDemTestData[testIndex].outDir,tempOutFilename,".Obj");
				snprintf(refFileNameExtended,256,"%s%s",ConverDemTestData[testIndex].refFileName,".Obj");

				if(CmpObjFiles(refFileNameExtended,tstFileName)!=0)
				{
					printf("- Obj-Files problem - ");
					CmpObjError=1;
					Errors++;
				}

				if(CmpElevError==0 && CmpObjError==0)
				{
					printf("passed\n");
				}
				else
				{
					printf("Line %ld failed\n",ConverDemTestData[testIndex].LineNumber);
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
					printf("Line %ld failed\n",ConverDemTestData[testIndex].LineNumber);
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

#define WCS_TEST
//#define  ELEV_TEST_ONLY

#ifdef WCS_TEST
__stdargs int main(void)   // I compile with -mregparm. Then __stdargs is needed to get real argc/argv
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

    if(!Compare_SumElDifSq)
    {
    	printf("Warning: The value of SumElDifSq will not be checked!\n");
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
#elif defined ELEV_TEST_ONLY
__stdargs int main(int argc, char **argv)   // I compile with -mregparm. Then __stdargs is needed to get real argc/argv
{
	printf("argc=%d\n",argc);
//	if(argc!=3)
//	{
//		printf("usage: %s elev-file1 elev-file2\n",argv[0]);
//		return 1;
//	}

char *File1="ram:tst_Alps_1.elev";
char *File2="ram:tst_Alps_2.elev";

	if(CmpElevFiles(File1, File2))
	{
		printf ("\7Files differ!\n");
		return 1;
	}
	return 0;
}
#endif
