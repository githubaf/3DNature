// FSSupport.cpp
// Code for filesystem-related operations
// gathered here from various odd places it started out.

#include "stdafx.h"
#include "FSSupport.h"
#include "WCSVersion.h"

static char CDRoot[50];
static char CDNameBuf[200];
#ifdef _WIN32
static DWORD CompLen, CDFlags;
#endif // _WIN32

char *FindCDRoot(void)
{
char DriveRoot[50];
#ifdef _WIN32

char LoopDrive, CDDrive;
DWORD DriveMask;

CDDrive = 'd';
DriveRoot[0] = ' ';
DriveRoot[1] = ':';
DriveRoot[2] = '\\';
DriveRoot[3] = NULL;

if(DriveMask = GetLogicalDrives())
	{
	for(LoopDrive = 2; LoopDrive < 26; LoopDrive++)
		{
		if(DriveMask & (1 << LoopDrive))
			{
			DriveRoot[0] = 'a' + LoopDrive;
			if(GetVolumeInformation(DriveRoot, CDNameBuf, 200, NULL, &CompLen, &CDFlags, NULL, NULL))
				{
				if(!strncmp(CDNameBuf, APP_TLA, 3))
					{
					CDDrive = 'a' + LoopDrive;
					strcpy(CDRoot, "c:\\");
					CDRoot[0] = CDDrive;
					return(CDRoot);
					} // if
				} // if
			} // if
		} // for
	} // if
#endif // _WIN32

return(NULL);

} // FindCDRoot

/*===========================================================================*/

int IsNetworkPath(char *InputPath)
{
unsigned int DriveType;
int rVal;
char NetPathCurDir[1024];

DriveType = GetDriveType(InputPath);

if (DriveType == DRIVE_NO_ROOT_DIR)
	{ // try harder, using CD technique
	if(GetCurrentDirectory(1024, NetPathCurDir))
		{
		SetCurrentDirectory(InputPath);
		DriveType = GetDriveType(NULL);
		SetCurrentDirectory(NetPathCurDir);
		} // if
	} // if

if(DriveType == DRIVE_REMOTE)
	rVal = 1; // definitely
else if (DriveType == DRIVE_UNKNOWN)
	rVal = -1; // unknown
else
	rVal = 0; // not

return (rVal);

} // IsNetworkPath
