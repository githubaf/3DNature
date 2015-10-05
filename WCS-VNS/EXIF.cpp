// EXIF.cpp
// Code to support EXIF tags
// Created by Frank Weed II, 01/28/2009
// Copyright 2009 by 3D Nature, LLC. All rights reserved.

#include "stdafx.h"
#include "EXIF.h"
#include "Application.h"
#include "Project.h"

extern WCSApp *GlobalApp;

class EXIFtool gEXIF;

EXIFtool::EXIFtool()
{

memset(shellParams, 0, sizeof(shellParams));

} // EXIFtool::EXIFtool

/*===========================================================================*/

EXIFtool::~EXIFtool()
{
} // EXIFtool::~EXIFtool

/*===========================================================================*/

bool EXIFtool::CreateEXIF(Renderer *RHost)
{
char tBuffer[1024];

sprintf(tBuffer, "-GPSLatitude=%lf -GPSLatitudeRef=", fabs(RHost->Cam->CamPos->Lat));
if (RHost->Cam->CamPos->Lat >= 0)
	{
	strcat(tBuffer, "N ");
	} // if
else
	{
	strcat(tBuffer, "S ");
	} // else
strcpy(shellParams, tBuffer);

sprintf(tBuffer, "-GPSLongitude=%lf -GPSLongitudeRef=", fabs(RHost->Cam->CamPos->Lon));
if (RHost->Cam->CamPos->Lon > 0)
	{
	strcat(tBuffer, "W ");
	} // if
else
	{
	strcat(tBuffer, "E ");
	} // else
strcat(shellParams, tBuffer);

sprintf(tBuffer, "-GPSAltitude=%lf -GPSAltitudeRef=", fabs(RHost->Cam->CamPos->Elev));
if (RHost->Cam->CamPos->Elev >= 0)
	{
	strcat(tBuffer, "Above Sea Level ");
	} // if
else
	{
	strcat(tBuffer, "Below Sea Level ");
	} // else
strcat(shellParams, tBuffer);

strcat(shellParams, "-overwrite_original_in_place ");
strcat(shellParams, "-q ");

return(true);

} // EXIFtool::CreateEXIF

/*===========================================================================*/

bool EXIFtool::SaveEXIF(const char *completeSavePath)
{
HINSTANCE shellRV;

strcat(shellParams, GlobalApp->MainProj->MungPath(completeSavePath));
shellRV = ShellExecute(NULL, "open", "tools/exiftool().exe", shellParams, GlobalApp->GetProgDir(), SW_HIDE);
if ((int)shellRV < 32)
	{
	char shellMsg[256];

	sprintf(shellMsg, "Shell error #%ld\n", (int)shellRV);
	OutputDebugString(shellMsg);
	} // if

return(true);

} // EXIFtool::SaveEXIF
