// originally miniunz from zlib
// heavily hacked into a silent internal unzipper
// on 10/29/03 by CXH

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <fcntl.h>

#ifdef unix
# include <unistd.h>
# include <utime.h>
#else
# include <direct.h>
# include <io.h>
#endif

#ifdef WIN32
#include <windows.h>
#endif // WIN32

#include "unzip.h"
#include "3DNUnzip.h"

#define CASESENSITIVITY (0)
#define WRITEBUFFERSIZE (8192)

/*
  mini unzip, demo of unzip package

  usage :
  Usage : miniunz [-exvlo] file.zip [file_to_extract]

  list the file in the zipfile, and print the content of FILE_ID.ZIP or README.TXT
    if it exists
*/


/* change_file_date : change the date/time of a file
    filename : the filename of the file where date/time must be modified
    dosdate : the new date at the MSDos format (4 bytes)
    tmu_date : the SAME new date at the tm_unz format */

/*
void change_file_date(filename,dosdate,tmu_date)
	const char *filename;
	uLong dosdate;
	tm_unz tmu_date;
{
#ifdef WIN32
  HANDLE hFile;
  FILETIME ftm,ftLocal,ftCreate,ftLastAcc,ftLastWrite;

  hFile = CreateFile(filename,GENERIC_READ | GENERIC_WRITE,
                      0,NULL,OPEN_EXISTING,0,NULL);
  GetFileTime(hFile,&ftCreate,&ftLastAcc,&ftLastWrite);
  DosDateTimeToFileTime((WORD)(dosdate>>16),(WORD)dosdate,&ftLocal);
  LocalFileTimeToFileTime(&ftLocal,&ftm);
  SetFileTime(hFile,&ftm,&ftLastAcc,&ftm);
  CloseHandle(hFile);
#else
#ifdef unix
  struct utimbuf ut;
  struct tm newdate;
  newdate.tm_sec = tmu_date.tm_sec;
  newdate.tm_min=tmu_date.tm_min;
  newdate.tm_hour=tmu_date.tm_hour;
  newdate.tm_mday=tmu_date.tm_mday;
  newdate.tm_mon=tmu_date.tm_mon;
  if (tmu_date.tm_year > 1900)
      newdate.tm_year=tmu_date.tm_year - 1900;
  else
      newdate.tm_year=tmu_date.tm_year ;
  newdate.tm_isdst=-1;

  ut.actime=ut.modtime=mktime(&newdate);
  utime(filename,&ut);
#endif
#endif
} // change_file_date
*/


/* mymkdir and change_file_date are not 100 % portable
   As I don't know well Unix, I wait feedback for the unix portion */

int mymkdir(dirname)
	const char* dirname;
{
    int ret=0;
#ifdef WIN32
	ret = mkdir(dirname);
#else
#ifdef unix
	ret = mkdir (dirname,0775);
#endif
#endif
	return ret;
} // mymkdir

int makedir (newdir)
    char *newdir;
{
  char *buffer ;
  char *p;
  int  len = strlen(newdir);  

  if (len <= 0) 
    return 0;

  buffer = (char*)malloc(len+1);
  strcpy(buffer,newdir);
  
  if (buffer[len-1] == '/') {
    buffer[len-1] = '\0';
  }
  if (mymkdir(buffer) == 0)
    {
      free(buffer);
      return 1;
    }

  p = buffer+1;
  while (1)
    {
      char hold;

      while(*p && *p != '\\' && *p != '/')
        p++;
      hold = *p;
      *p = 0;
      if ((mymkdir(buffer) == -1) && (errno == ENOENT))
        {
          printf("couldn't create directory %s\n",buffer);
          free(buffer);
          return 0;
        }
      if (hold == 0)
        break;
      *p++ = hold;
    }
  free(buffer);
  return 1;
} // makedir



int do_extract_currentfile(unzFile uf, const int* popt_extract_without_path, int* popt_overwrite)
{
char filename_inzip[1024];
char* filename_withoutpath;
char* p;
int err=UNZ_OK;
FILE *fout=NULL;
void* buf;
uInt size_buf;

unz_file_info file_info;
uLong ratio=0;
err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);

if (err!=UNZ_OK)
	{
	//printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
	return err;
	} // if

size_buf = WRITEBUFFERSIZE;
buf = (void*)malloc(size_buf);
if (buf==NULL)
	{
    //printf("Error allocating memory\n");
    return UNZ_INTERNALERROR;
	} // if

p = filename_withoutpath = filename_inzip;
while ((*p) != '\0')
	{
	if (((*p)=='/') || ((*p)=='\\'))
		{
		filename_withoutpath = p+1;
		} // if
	p++;
	} // while

if ((*filename_withoutpath)=='\0')
	{
	if ((*popt_extract_without_path)==0)
		{
		//printf("creating directory: %s\n",filename_inzip);
		mymkdir(filename_inzip);
		} // if
	} // if
else
	{
	const char* write_filename;

	if ((*popt_extract_without_path)==0)
		write_filename = filename_inzip;
	else
		write_filename = filename_withoutpath;

	err = unzOpenCurrentFile(uf);
	if (err!=UNZ_OK)
		{
		//printf("error %d with zipfile in unzOpenCurrentFile\n",err);
		} // if

	if (err==UNZ_OK) // we always overwrite
		{
		fout=fopen(write_filename,"wb");

        /* some zipfile don't contain directory alone before file */
        if ((fout==NULL) && ((*popt_extract_without_path)==0) && (filename_withoutpath!=(char*)filename_inzip))
			{
            char c=*(filename_withoutpath-1);
            *(filename_withoutpath-1)='\0';
            makedir(write_filename);
            *(filename_withoutpath-1)=c;
            fout=fopen(write_filename,"wb");
			} // if

		if (fout==NULL)
			{
			//printf("error opening %s\n",write_filename);
			} // if
		} // if

	if (fout!=NULL)
		{
		//printf(" extracting: %s\n",write_filename);

		do
			{
			err = unzReadCurrentFile(uf,buf,size_buf);
			if (err<0)	
				{
				//printf("error %d with zipfile in unzReadCurrentFile\n",err);
				break;
				} // if
			if (err>0)
				{
				if (fwrite(buf,err,1,fout)!=1)
					{
					//printf("error in writing extracted file\n");
                    err=UNZ_ERRNO;
					break;
					} // if
				} // if
			} // do
		while (err>0);

		fclose(fout);
		if (err==0)
			{
			// don't bother updating file date
			//change_file_date(write_filename, file_info.dosDate, file_info.tmu_date);
			} // if
		} // if

    if (err==UNZ_OK)
	    {
		err = unzCloseCurrentFile (uf);
		if (err!=UNZ_OK)
			{
			//printf("error %d with zipfile in unzCloseCurrentFile\n",err);
			} // if
		} // if
    else
		{
        unzCloseCurrentFile(uf); // don't lose the error
		} // else
	} // else

free(buf);    
return err;
}


int do_extract(unzFile uf, int opt_extract_without_path, int opt_overwrite)
{
uLong i;
unz_global_info gi;
int err;
FILE* fout=NULL;	
int FilesSuccessful = 0;

err = unzGetGlobalInfo (uf,&gi);
if (err!=UNZ_OK)
	{
	//printf("error %d with zipfile in unzGetGlobalInfo \n",err);
	return(0);
	} // if

for (i=0;i<gi.number_entry;i++)
	{
    if (do_extract_currentfile(uf,&opt_extract_without_path, &opt_overwrite) != UNZ_OK)
		{
        break;
		} // if
	else
		{
		FilesSuccessful++;
		} // else

	if ((i+1)<gi.number_entry)
		{
		err = unzGoToNextFile(uf);
		if (err!=UNZ_OK)
			{
			//printf("error %d with zipfile in unzGoToNextFile\n",err);
			break;
			} // if
		} // if
	} // for

return(FilesSuccessful);
}



// returns string for success, 0 for failure
char *CheckForFileExtensionInZIP(const char *ZipFileName, char *Extension)
{
unzFile uf=NULL;
char *Result = NULL;

if(ZipFileName && ZipFileName[0] && Extension && Extension[0])
	{
	if(uf = unzOpen(ZipFileName))
		{
		Result = FindFileExtensionInZip(uf, Extension);
		unzCloseCurrentFile(uf);
		return(Result);
		} // if
	}

return(0);
} // CheckForFileExtensionInZIP

// pass extension as ".nvw" or the like
// extension comparison is case-insensitive
// returns name of file found, in original case
char filename_inzip_static[1024];
char *FindFileExtensionInZip(unzFile uf, char *Extension)
{
uLong i;
unz_global_info gi;
int err;
unsigned int ExtLen;
char ExtensionUpr[200]; // excessively long, yes...

ExtLen = strlen(Extension);

if(ExtLen == 0)
	{
	return(0);
	} // if

// we'll do this in uppercase
strcpy(ExtensionUpr, Extension);
strupr(ExtensionUpr);

err = unzGetGlobalInfo (uf,&gi);
if (err!=UNZ_OK)
	{
	//printf("error %d with zipfile in unzGetGlobalInfo \n",err);
	return(0);
	} // if
for (i=0;i<gi.number_entry;i++)
	{
	unsigned int filename_inzip_len;
	char filename_inzip[1024];
	char filename_inzip_upr[1024];
	unz_file_info file_info;
	err = unzGetCurrentFileInfo(uf,&file_info,filename_inzip,sizeof(filename_inzip),NULL,0,NULL,0);
	if (err!=UNZ_OK)
		{
		//printf("error %d with zipfile in unzGetCurrentFileInfo\n",err);
		break;
		} // if
	filename_inzip_len = strlen(filename_inzip);
	if(filename_inzip_len > ExtLen)
		{
		// convert to uppercase for case-insensitive match
		strcpy(filename_inzip_upr, filename_inzip);
		strupr(filename_inzip_upr);
		if(!strcmp(ExtensionUpr, &filename_inzip_upr[filename_inzip_len - ExtLen]))
			{
			strcpy(filename_inzip_static, filename_inzip); // make persistant copy
			return(filename_inzip_static);
			} // if
		} // if
	if ((i+1)<gi.number_entry)
		{
		err = unzGoToNextFile(uf);
		if (err!=UNZ_OK)
			{
			//printf("error %d with zipfile in unzGoToNextFile\n",err);
			break;
			} // if
		} // if
	} // for

return 0;
} // FindFileExtensionInZip

int ExtractAllFilesFromZip(const char *ZipFileName)
{
int FileCount = 0;
unzFile uf=NULL;

if(ZipFileName && ZipFileName[0])
	{
	if(uf = unzOpen(ZipFileName))
		{
		FileCount = do_extract(uf, 0, 1);
		unzCloseCurrentFile(uf);
		return(FileCount);
		} // if
	} // if

return(FileCount);
} // ExtractAllFilesFromZip

