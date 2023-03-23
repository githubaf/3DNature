// AF, 22.Mar.23
// read 2 Obj-Files (WCS vector files for rivers and borders) and print content
/*

m68k-amigaos-gcc af_cmpObjFiles.c -noixemul -lm -o af_cmpObjFiles_68k
vamos cmpObjFiles_68k <file1> <file2>

gcc af_cmpObjFiles.c -I. -lm -o af_cmpObjFiles_linux -Wno-address-of-packed-member
./af_cmpObjFiles_linux ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects/Arizona/SunsetAnim.object/AROS_Alps\ .Obj ~/Desktop/SelcoGit/aros_deadw00d/core-linux-x86_64-d/bin/linux-x86_64/AROS/WCS/WCSProjects/Arizona/SunsetAnim.object/AF_ALPS\ \ \ .Obj

*/


#include <stdio.h>
#include <sys/types.h>
#include <math.h>
#include <string.h>


#if defined  __AROS__ || defined  __ORDER_LITTLE_ENDIAN__
   #define PACKED __attribute__((__packed__))
   typedef unsigned int ULONG;
   typedef int LONG;
   typedef unsigned short USHORT;
   typedef short SHORT;
#else
   #define PACKED
#endif


struct PACKED  vectorheaderV100 {
 char	Name[10];
 LONG	points;
 short	elevs;
 double avglat, avglon, avgelev, elscale;
 short  MaxEl, MinEl;
};

// ########################################################################################################################
// swap primitives
// ########################################################################################################################

// AF, 12.Dec.22
/*
#if defined  __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
   #define ENDIAN_CHANGE_IF_NEEDED(x) x
#else
   #define ENDIAN_CHANGE_IF_NEEDED(x)
#endif
*/
#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
void SimpleEndianFlip64 (double Source64, double *Dest64)  // AF, 12Dec22 for i386-aros
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

void SimpleEndianFlip32F(float Source32, float  *Dest32)  // AF, 10Dec22 for i386-aros
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
void SimpleEndianFlip32U( ULONG Source32, ULONG *Dest32)  // AF, 10Dec22 for i386-aros
{
	(*Dest32) = (ULONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			(ULONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}


void SimpleEndianFlip32S( LONG Source32, LONG *Dest32)  //AF, 10Dec22 for i386-aros
{
	(*Dest32) = ( LONG)( ((Source32 & 0x00ff) << 24) | ((Source32 & 0xff00) << 8) |
			( LONG)( ((Source32 & 0xff0000) >> 8) | ((Source32 & 0xff000000) >> 24)));
}

void SimpleEndianFlip16U(USHORT Source16, USHORT *Dest16) {(*Dest16) = (USHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
void SimpleEndianFlip16S(SHORT Source16, SHORT *Dest16) {(*Dest16) = ( SHORT)( ((Source16 & 0x00ff) << 8) | ((Source16 & 0xff00) >> 8) );}
#endif



// AF: 22.Mar.23 correct endian if necessary and write
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

// AF, HGW, 22.Jan23
int fread_double_BE(double *Value, FILE *file)
{
    int Result=fread(Value, sizeof (double),1,file);

#if __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
    // Flip Endian if host is not Big Endian
    SimpleEndianFlip64(*Value,Value);
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
    // nothing more to do
#else
#error "Unsupported Byte-Order"
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

/////////////////////////////////////////////////////////////////////////////////////
FILE *File1=NULL,*File2=NULL;


void Cleanup(char *String)
{
	if(File2) { fclose(File2); File2=NULL; }
	if(File1) { fclose(File1); File1=NULL; }
	printf("%s",String);
}

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


int main(int argc, char **argv)
{
	int Error=0;

	if(argc!=3)
	{
		printf("usage: %s Obj-File1 Obj-File2\n",argv[0]);
		return 1;
	}
	File1=fopen(argv[1],"r");

	if(File1==NULL)
	{
		Cleanup("File1 open error\n");
		return 1;
	}

	File2=fopen(argv[2],"r");

	if(File2==NULL)
	{
		Cleanup("File2 open error\n");
		return 1;
	}

	char Filetype1[10]={0}; // inkl abschliessender 0 fuer printf
	if(fread(Filetype1,9,1,File1)!=1)
	{
		Cleanup("File1 error reading Filetype1\n");
		return 1;
	}

	char Filetype2[10]={0};  // inkl abschliessender 0 fuer printf
	if(fread(Filetype2,9,1,File2)!=1)
	{
		Cleanup("File1 error reading Filetype2\n");
		return 1;
	}

	float Version1, Version2;

	if(fread(&Version1,sizeof(float),1,File1)!=1)
	{
		Cleanup("File1 error reading Version1\n");
		return 1;
	}


	if(fread(&Version2,sizeof(float),1,File2)!=1)
	{
		Cleanup("File1 error reading Version2\n");
		return 1;
	}


	struct vectorheaderV100 Hdr1, Hdr2;

	if(freadVectorheaderV100_BE(&Hdr1, File1)!=1)
	{
		Cleanup("File1 error reeading header\n");
		return 1;
	}


	if(freadVectorheaderV100_BE(&Hdr2, File2)!=1)
	{
		Cleanup("File1 error reeading header\n");
		return 1;
	}

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


	/*
	 *     if (fwrite_double_Array_BE(&Lon[0], Size,fobject)==1) //(fwrite((char *)&Lon[0], Size, 1, fobject) == 1) AF: 22.Mar.23
     {
     if (fwrite_double_Array_BE(&Lat[0], Size, fobject) == 1) // (fwrite((char *)&Lat[0], Size, 1, fobject) == 1) AF: 22.Mar.23
      {
      if (fwrite_SHORT_Array_BE(&Elev[0], Size / 4, fobject) == 1) // (fwrite((char *)&Elev[0], Size / 4, 1, fobject) == 1) AF: 22.Mar.23
	 *
	 */

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

    Cleanup("");
	return Error;
}
