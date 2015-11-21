
#include <windows.h>
#include <osgDB/ReadFile>
#include "InternalImage.h"


bool ExportImage(char *TempName, unsigned char *DataBlock, unsigned long int DataBlockSize, char *TempFilePath)
{
bool Result = false;

if(GetTempPath(1023, TempFilePath))
	{
	FILE *WriteFile;
	strcat(TempFilePath, TempName);
	if(WriteFile = fopen(TempFilePath, "wb"))
		{
		fwrite(DataBlock, DataBlockSize, 1, WriteFile);
		fclose(WriteFile);
		WriteFile = NULL;
		Result = true;
		} // if
	} // if

return(Result);
} // ExportImage

void CleanupExportedImage(char *TempName)
{
char TempFilePath[1024];

if(GetTempPath(1023, TempFilePath))
	{
	strcat(TempFilePath, TempName);
	remove(TempFilePath); // kill it
	} // if

} // CleanupExportedImage


osg::Image *ExportAndLoadImage(char *TempName, unsigned char *DataBlock, unsigned long int DataBlockSize)
{
osg::Image *Result = NULL;
char TempFilePath[1024];

// will write path to TempFilePath on success
if(ExportImage(TempName, DataBlock, DataBlockSize, TempFilePath))
	{
	Result = osgDB::readImageFile(TempFilePath);
	CleanupExportedImage(TempName); // kill it right away
	} // if

return(Result);
} // ExportAndLoadImage
