// ExportFormat.cpp
// Code module for base and derived export classes
// Created from scratch 07/01/03 by Gary R. Huber
// Copyright 2003 Questar Productions. All rights reserved.

#include "stdafx.h"
#include "ExportFormat.h"
#include "EffectsLib.h"
#include "ExportControlGUI.h"
#include "IncludeExcludeList.h"
#include "Raster.h"
#include "ImageOutputEvent.h"
#include "Project.h"
#include "Requester.h"
#include "TerrainWriter.h"
#include "CubeSphere.h"
#include "AppMem.h"
#include "RasterResampler.h"
#include "Log.h"
#include "SceneExportGUI.h"
#include "SXExtension.h"
#include "UsefulZip.h"
#include "NatureViewCrypto.h"
#include "zlib.h"

// Most ExportFormat code moved into format-specific files like ExportFormat3DS.cpp, etc
// on 5/18/04 by CXH

ExportFormatCustom::ExportFormatCustom(SceneExporter *MasterSource, Project *ProjectSource, EffectsLib *EffectsSource, Database *DBSource, ImageLib *ImageSource)
: ExportFormat(MasterSource, ProjectSource, EffectsSource, DBSource, ImageSource)
{

} // ExportFormatCustom::ExportFormatCustom

/*===========================================================================*/

ExportFormatCustom::~ExportFormatCustom()
{

} // ExportFormatCustom::~ExportFormatCustom

/*===========================================================================*/
/*===========================================================================*/

int ExportFormat::CheckAndShortenFileName(char *OutputName, const char *FilePath, const char *FileName, long MaxLen)
{
char OriginalFullPath[512], NewFullPath[512];

if (strlen(FileName) > (size_t)MaxLen)
	{
	strcpy(OutputName, FileName);
	sprintf(OriginalFullPath, "Shorten the name of this file to %d characters in order to comply with output file specs.", MaxLen);
	if (GetInputString(OriginalFullPath, WCS_REQUESTER_FILESAFE_ONLY, OutputName) && OutputName[0])
		{
		strmfp(OriginalFullPath, FilePath, FileName);
		strmfp(NewFullPath, FilePath, OutputName);
		if (PROJ_rename(OriginalFullPath, NewFullPath))
			{
			strcpy(OutputName, FileName);
			return (0);
			} // if
		return (1);
		} // if
	return (0);
	} // if
else
	strcpy(OutputName, FileName);

return (1);

} // ExportFormat::CheckAndShortenFileName

/*===========================================================================*/

int ExportFormat::CopyExistingFile(const char *OriginalFullPath, const char *PathName, const char *FileName)
{
FILE *ffin, *ffout;
char *FileMem;
long FileSize;
int Success = 0;
char NewFileName[512];

strmfp(NewFileName, PathName, FileName);
// try to avoid copying if the output name and directory are the same as input file
if (stricmp(NewFileName, OriginalFullPath))
	{
	if (ffin = PROJ_fopen(OriginalFullPath, "rb"))
		{
		// this should fail if output is the same file as the input file
		if (ffout = PROJ_fopen(NewFileName, "wb"))
			{
			if (! fseek(ffin, 0, SEEK_END))
				{
				if ((FileSize = ftell(ffin)) > 0)
					{
					if (! fseek(ffin, 0, SEEK_SET))
						{
						// allocate memory to hold file
						if (FileMem = (char *)AppMem_Alloc(FileSize, 0))
							{
							// read file in
							if (fread(FileMem, FileSize, 1, ffin) == 1)
								{
								// write file out
								if (fwrite(FileMem, FileSize, 1, ffout) == 1)
									Success = 1;
								} // if
							// free memory
							AppMem_Free(FileMem, FileSize);
							} // if
						} // if
					} // if
				} // if
			fclose(ffout);
			} // if
		fclose(ffin);
		} // if
	} // if

return (Success);

} // ExportFormat::CopyExistingFile
