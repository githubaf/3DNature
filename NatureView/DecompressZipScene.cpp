
#include <string.h>
#include "DecompressZipScene.h"

extern "C" {
#include "3DNUnzip.h"
} // extern "C"


// ZIP handling

char NewZipSceneFileName[500];
// returns name of first .nvw file extracted from zipfile
char *DecompressZipScene(const char *FileName)
{
NewZipSceneFileName[0] = NULL;
char *FindResult = NULL;

if(FindResult = CheckForFileExtensionInZIP(FileName, ".nvw"))
	{
	// if the zipfile has an entry in it ending in .nvw, unzip it here and return the relative path/name to the first .nvw file
	strcpy(NewZipSceneFileName, FindResult);

	if(ExtractAllFilesFromZip(FileName) > 0)
		{
		return(NewZipSceneFileName);
		} // if
	else
		{
		return(NULL); // failure
		} // else
	} // if


return(NULL);
} // DecompressZipScene
