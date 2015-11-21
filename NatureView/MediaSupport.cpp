// MediaSupport.cpp
// Routines for playing various media
// Created from scratch by CXH
// Copyright 2005

#include <algorithm> // for tolower

#include <windows.h>
#include <vfw.h>
#include <shlwapi.h>

#include "MediaSupport.h"
#include "InstanceSupport.h"
#include "Viewer.h"
#include "HTMLDlg.h"

char *SafeKnownMediaExtensions [] =
	{
	"mov",
	"avi",
	"mpg",
	"mpeg",
	NULL // must be last
	}; // SafeKnownMediaExtensions

char *SafeKnownSoundExtensions [] =
	{
	"mp3",
	"wav",
	"avi",
	"mpg",
	"mpeg",
	NULL // must be last
	}; // SafeKnownSoundExtensions


char *SafeKnownImageExtensions [] =
	{
	"gif",
	"iff",
	"tiff",
	"tif",
	"tga",
	"bmp",
	"jpg",
	"jpeg",
	"pic",
	"pict",
	"pct",
	"png",
	"ecw",
	"sid",
	NULL // must be last
	}; // SafeKnownImageExtensions

static HWND HiddenSoundPlayerWindow, HiddenSoundPlayerShell;


bool PlaySoundAsync(const char *SoundFilePath)
{
CleanupSounds();
// make player window child of hidden parent so that player is not visible
if(HiddenSoundPlayerShell = CreateWindow("STATIC", "SoundPlayer Shell", WS_OVERLAPPEDWINDOW, 0, 0, 1, 1, NULL, NULL, GetHInstance(), NULL))
	{
	if(HiddenSoundPlayerWindow = MCIWndCreate(HiddenSoundPlayerShell, GetHInstance(), MCIWNDF_NOPLAYBAR | MCIWNDF_NOMENU | MCIWNDF_NOERRORDLG, SoundFilePath))
		{
		ShowWindow(HiddenSoundPlayerWindow, SW_HIDE);
		MCIWndPlay(HiddenSoundPlayerWindow);
		return(true);
		} // if
	CleanupSounds();
	} // if

return(false);
} // PlaySoundAsync


void CancelSounds(void)
{
if(HiddenSoundPlayerWindow)
	{
	MCIWndStop(HiddenSoundPlayerWindow);
	} // if
} // CancelSounds

void CleanupSounds(void)
{
CancelSounds();
if(HiddenSoundPlayerWindow)
	{
	MCIWndDestroy(HiddenSoundPlayerWindow);
	HiddenSoundPlayerWindow = NULL;
	} // if
if(HiddenSoundPlayerShell)
	{
	DestroyWindow(HiddenSoundPlayerShell);
	HiddenSoundPlayerShell = NULL;
	} // if
} // CleanupSounds

bool ValidateURL(const char *Input, bool PermitLocalWebImageFormats)
{
bool SafeTyping = false;
std::string URLLocalCopy(Input);

// lowercase string for case-insensitive comparison
transform (URLLocalCopy.begin(),URLLocalCopy.end(), URLLocalCopy.begin(), tolower);
// check for remote URI service prefix
if(!URLLocalCopy.compare(0, 7, "http://") || !URLLocalCopy.compare(0, 8, "https://") || !URLLocalCopy.compare(0, 6, "ftp://") || !URLLocalCopy.compare(0, 9, "mailto://"))
	{
	SafeTyping = true;
	} // if
if(!SafeTyping)
	{ // try certain local types (.htm, .html)
	if(!URLLocalCopy.compare(0, 7, "file://"))
		{
		// permit only .htm and .html files to be launched locally by file://
		if((!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 4, 4, ".htm")) || (!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 5, 5, ".html")))
			{
			SafeTyping = true;
			} // if
		if(PermitLocalWebImageFormats)
			{
			// permit .jpg, .png and .gif files to be launched locally by file://
			if((!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 4, 4, ".jpg"))
			 || (!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 5, 5, ".jpeg"))
			 || (!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 4, 4, ".png"))
			 || (!URLLocalCopy.compare(strlen(URLLocalCopy.c_str()) - 4, 4, ".gif")))
				{
				SafeTyping = true;
				} // if
			} // if
		} // if
	} // if


return(SafeTyping);
} // ValidateURL



int OpenURLExternally(const char *URL)
{
int Success = 0;
std::string URLLocalCopy(URL);
if(strlen(URLLocalCopy.c_str()) > 4000)
	{
	URLLocalCopy.resize(4000); // prevent buffer-overflow exploit known to be in older Win2k
	} // if


if(ValidateURL(URL))
	{
	if((int)ShellExecute(GetGlobalViewerHWND(), "open", URLLocalCopy.c_str(), NULL, "", SW_SHOWNORMAL) > 32)
		{
		Success = 1;
		} // if
	} // if

return(Success);
} // OpenURLExternally

int OpenURLInternally(const char *URL)
{
int Success = 0;

Success = HTMLDlg::SetHTMLTextFromFile(URL);

return(Success);
} // OpenURLInternally


int OpenImageInternally(const char *URL)
{
int Success = 0;

Success = HTMLDlg::SetImageFromFile(URL);

return(Success);
} // OpenImageInternally


int OpenMediaInternally(const char *URL)
{
int Success = 0;

Success = HTMLDlg::SetMediaFromFile(URL);

return(Success);
} // OpenMediaInternally



bool TestFileExecutable(const char *Input)
{
char TestPath[MAX_PATH];
std::string PathCompose;

// plain
PathCompose = Input;
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .exe
PathCompose = Input;
PathCompose += ".exe";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .bat
PathCompose = Input;
PathCompose += ".bat";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .com
PathCompose = Input;
PathCompose += ".com";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .scr
PathCompose = Input;
PathCompose += ".scr";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .scf
PathCompose = Input;
PathCompose += ".scf";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .pif
PathCompose = Input;
PathCompose += ".pif";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

// .cmd
PathCompose = Input;
PathCompose += ".cmd";
strcpy(TestPath, PathCompose.c_str());
if(PathFindOnPath(TestPath, NULL))
	{
	return(true);
	} // if

return(false);
} // TestFileExecutable

bool TestFileKnownImage(const char *Input)
{
return(TestFileKnownExtension(Input, SafeKnownImageExtensions));
} // TestFileKnownImage

bool TestFileKnownMedia(const char *Input)
{
return(TestFileKnownExtension(Input, SafeKnownMediaExtensions));
} // TestFileKnownMedia

bool TestFileKnownSound(const char *Input)
{
return(TestFileKnownExtension(Input, SafeKnownSoundExtensions));
} // TestFileKnownSound

bool TestFileKnownExtension(const char *Input, char *ExtTable[])
{
unsigned long Len;
char TLA[4], FLA[5];

Len = strlen(Input);

if(Len > 4)
	{
	strcpy(TLA, &Input[Len - 3]);
	strcpy(FLA, &Input[Len - 4]);
	for(int TestLoop = 0; ; TestLoop++)
		{
		if(ExtTable[TestLoop])
			{
			if(ExtTable[TestLoop][3] == NULL) // is it a TLA?
				{
				if(!stricmp(ExtTable[TestLoop], &Input[Len - 3]))
					{
					return(true);
					} // if
				} // if
			else // FLA
				{
				if(!stricmp(ExtTable[TestLoop], &Input[Len - 4]))
					{
					return(true);
					} // if
				} // else
			} // if
		else
			{
			break;
			} // else
		} // for
	} // if

return(false);
} // TestFileKnownExtension


