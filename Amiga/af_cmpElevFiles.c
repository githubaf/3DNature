/*
AF, 14.Dec.23
read 2 elev-Files (WCS-DEM-Files) and print content

m68k-amigaos-gcc -Wall --pedantic -noixemul af_cmpElevFiles.c -lm -o af_cmpElevFiles_68k
vamos af_cmpElevFiles_68k <file1> <file2>

gcc -Wall --pedantic af_cmpElevFiles.c -lm -o af_cmpElevFiles_linux -Wno-address-of-packed-member
./af_cmpElevFiles_linux ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects/Arizona/SunsetAnim.object/AROS_Alps\ .elev ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects/Arizona/SunsetAnim.object/AF_ALPS\ \ \ .elev
 */





#include <stdio.h>
#include <stdarg.h>
#include <math.h>
#include <sys/types.h>



#if defined  __AROS__ || defined  __ORDER_LITTLE_ENDIAN__
#define PACKED __attribute__((__packed__))
typedef unsigned int ULONG;
typedef int LONG;
typedef unsigned short USHORT;
typedef short SHORT;
typedef int8_t		BYTE;
#else
#define PACKED
#endif



unsigned int printMessages=0;
unsigned int Verbose=1;
unsigned int Compare_SumElDifSq=0;     // The value of Compare_SumElDifSq seems to be compiler and/or operating-system-dependant... !!!??? AF, 30.6.2023

#define ELEVHDRLENV101 64

void SimpleEndianFlip32F(             float Source32, float  *Dest32)  // AF, 10Dec22 for i386-aros
{
	float retVal;
	char *floatToConvert = ( char* ) & Source32;
	char *returnFloat = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnFloat[0] = floatToConvert[3];
	returnFloat[1] = floatToConvert[2];
	returnFloat[2] = floatToConvert[1];
	returnFloat[3] = floatToConvert[0];

	*Dest32=retVal;
}

void SimpleEndianFlip32S(  LONG Source32, LONG   *Dest32)  //AF, 10Dec22 for i386-aros
{
	(*Dest32) = ( LONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			( LONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}

void SimpleEndianFlip64 (            double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
{
	double retVal;
	char *doubleToConvert = ( char* ) & Source64;
	char *returnDouble = ( char* ) & retVal;

	// swap the bytes into a temporary buffer
	returnDouble[0] = doubleToConvert[7];
	returnDouble[1] = doubleToConvert[6];
	returnDouble[2] = doubleToConvert[5];
	returnDouble[3] = doubleToConvert[4];
	returnDouble[4] = doubleToConvert[3];
	returnDouble[5] = doubleToConvert[2];
	returnDouble[6] = doubleToConvert[1];
	returnDouble[7] = doubleToConvert[0];

	*Dest64=retVal;

}

void SimpleEndianFlip16S(  signed short int Source16, signed short int   *Dest16) {(*Dest16) = (  signed short int)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}


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

// AF, HGW, 21.Apr23
int fread_float_BE(float *Value, FILE *file)
{
	int Result=fread(Value, sizeof (float),1,file);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	// Flip Endian if host is not Big Endian
	SimpleEndianFlip32F(*Value,Value);
#endif
	return Result;
}

// AF, HGW, 22.Jan23
int fread_short_BE(short *Value, FILE *file)
{
	int Result=fread(Value, sizeof (short),1,file);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
	// Flip Endian if host is not Big Endian
	SimpleEndianFlip16S(*Value,Value);
#endif
	return Result;
}

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

struct elmapheaderV101 {
		/* same as V1.02 in size and members */
		/* 1.02 allowed the elscale variable to reflect whether */
		/* the data was in meters, feet, etc... */
		LONG	rows,columns;
		double lolat,lolong,steplat,steplong,elscale;
		short  MaxEl, MinEl;
		LONG	Samples;
		float	SumElDif, SumElDifSq;
		short	*map;
		LONG	*lmap;
		LONG	size, scrnptrsize, fractalsize;
		float	*scrnptrx,
		*scrnptry,
		*scrnptrq;
		struct	faces *face;
		BYTE	*fractal;
		LONG	facept[3], facect, fracct, Lr, Lc;
		short	MapAsSFC, ForceBath;
		float	LonRange, LatRange;
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
		//goto Cleanup;
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


int main(int argc, char **argv)
{
	if(argc!=3)
	{
		printf("usage: %s <elev-File1> <elev-File2>\n",argv[0]);
		return 1;
	}

	return CmpElevFiles(argv[1], argv[2]);
}
