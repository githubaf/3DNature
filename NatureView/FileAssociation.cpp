// FileAssociation.cpp
// Code (primarily from WCS's Application.cpp) to associate a file extension with out executable

#include <windows.h>
#include "FileAssociation.h"
#include "IdentityDefines.h"

int FindExePathAndName(char *ProgName, char *ProgDir)
{
char FindSlash;
ProgName[0] = NULL;
ProgDir[0] = NULL;
LPSTR Chomp;

for(FindSlash = strlen(_pgmptr) - 1; FindSlash > 0; FindSlash--)
	{
	if(_pgmptr[FindSlash] == '\\')
		{
		strcpy(ProgName, &_pgmptr[FindSlash + 1]);
		break;
		} // if
	} // for
if(ProgName[0])
	{
	if(ProgName[strlen(ProgName) - 4] != '.')
		{
		strcat(ProgName, ".exe");
		} // if
	} // if
SearchPath(NULL, ProgName, NULL, 254, ProgDir, &Chomp);
return(1);
} // FindExePathAndName

int SetupFileAssociation(char *Extension, bool Force) // Extension must have a period preceeding it
{
int Success = 0;
char OpenCommand[300];
unsigned char CheckCommand[1000];
char ProgName[32];
char ProgDir[255];
HKEY hkResult, ExtKey = NULL, AppKey = NULL, ShellKey = NULL, OpenKey = NULL, CommandKey = NULL;
DWORD Dispos;
const char *Descript = "NatureView Scene";
char AppSig[100];

strcpy(AppSig, "NVW.Scene"); // AppSig could also be nvw_auto_file if set up by "Open With"


// We'll associate our file extension with our
// executable even if it's already taken (for example by an earlier version)

if(FindExePathAndName(ProgName, ProgDir))
	{
	bool GoAhead = false;
	strcpy(OpenCommand, ProgDir);
	if(Force)
		{
		GoAhead = true;
		} // if
	else if(RegOpenKeyEx(HKEY_CLASSES_ROOT, Extension, 0, KEY_READ, &hkResult) != ERROR_SUCCESS)
		{ // No such extension exists, set us up...
		GoAhead = true;
		RegCloseKey(hkResult);
		hkResult = NULL;
		} // if
	else
		{ // association already exists, if it's not us, leave it alone
		GoAhead = true; // assume we will go ahead, only fail if we don't find something to contradict that
		AppKey = NULL;
		if((RegOpenKeyEx(HKEY_CLASSES_ROOT, AppSig, 0, KEY_READ, &AppKey)) == ERROR_SUCCESS)
			{
			// nothing more to do here. Key gets closed inside the if(AppKey) block below
			} // if
		else if((RegOpenKeyEx(HKEY_CLASSES_ROOT, "nvw_auto_file", 0, KEY_READ, &AppKey)) == ERROR_SUCCESS)
			{
			strcpy(AppSig, "nvw_auto_file");
			// nothing more to do here. Key gets closed inside the if(AppKey) block below
			} // else if
		else
			{
			AppKey = NULL; // to be safe
			GoAhead = true;
			} // else
		if(AppKey)
			{
			if(RegOpenKeyEx(AppKey, "shell", 0, KEY_READ, &ShellKey) == ERROR_SUCCESS)
				{
				if(RegOpenKeyEx(ShellKey, "open", 0, KEY_READ, &OpenKey) == ERROR_SUCCESS)
					{
					if(RegOpenKeyEx(OpenKey, "command", 0, KEY_READ, &CommandKey) == ERROR_SUCCESS)
						{
						DWORD CheckLen = 999;
						DWORD ValType = REG_SZ;
						if(RegQueryValueEx(CommandKey, NULL, NULL, &ValType, CheckCommand, &CheckLen) == ERROR_SUCCESS)
							{ // read the key successfully
							if(ValType == REG_SZ)
								{
								char CheckName[100];
								strcpy(CheckName, NVW_NATUREVIEW_BASENAMETEXT);
								strupr((char *)CheckCommand); // case insensitive
								strupr(CheckName); // case insensitive
								if(!strstr((char *)CheckCommand, CheckName))
									{ // didn't find the word NatureView anywhere in the previous Open Command string, don't overwrite it
									GoAhead = false;
									} // if
								} // if
							} // if
						RegCloseKey(CommandKey);
						CommandKey = NULL;
						} // if
					RegCloseKey(OpenKey);
					OpenKey = NULL;
					} // if
				RegCloseKey(ShellKey);
				ShellKey = NULL;
				} // if
			RegCloseKey(AppKey);
			AppKey = NULL;
			} // if
		} // else
	if(GoAhead)
		{
		if(RegCreateKeyEx(HKEY_CLASSES_ROOT, AppSig, 0, "", REG_OPTION_NON_VOLATILE,
		 KEY_ALL_ACCESS, NULL, &AppKey, &Dispos) == ERROR_SUCCESS)
			{
			strcat(OpenCommand, " \"%1\"");
			RegSetValueEx(AppKey, NULL, 0, REG_SZ, (unsigned char *)Descript, strlen(Descript) + 1);
			if(RegCreateKeyEx(AppKey, "shell", 0, "", REG_OPTION_NON_VOLATILE,
			 KEY_ALL_ACCESS, NULL, &ShellKey, &Dispos) == ERROR_SUCCESS)
				{
				if(RegCreateKeyEx(ShellKey, "open", 0, "", REG_OPTION_NON_VOLATILE,
				 KEY_ALL_ACCESS, NULL, &OpenKey, &Dispos) == ERROR_SUCCESS)
					{
					if(RegCreateKeyEx(OpenKey, "command", 0, "", REG_OPTION_NON_VOLATILE,
					 KEY_ALL_ACCESS, NULL, &CommandKey, &Dispos) == ERROR_SUCCESS)
						{
						RegSetValueEx(CommandKey, NULL, 0, REG_SZ, (unsigned char *)OpenCommand, strlen(OpenCommand) + 1);
						RegCloseKey(CommandKey);
						if(RegCreateKeyEx(HKEY_CLASSES_ROOT, Extension, 0, "", REG_OPTION_NON_VOLATILE,
						 KEY_ALL_ACCESS, NULL, &ExtKey, &Dispos) == ERROR_SUCCESS)
							{
							RegSetValueEx(ExtKey, NULL, 0, REG_SZ, (unsigned char *)AppSig, strlen(AppSig) + 1);
							RegCloseKey(ExtKey);
							Success = 1;
							} // if
						} // if
					RegCloseKey(OpenKey);
					} // if
				RegCloseKey(ShellKey);
				} // if
			RegCloseKey(AppKey);
			} // if
		} // if
	} // if

return(Success);
} // if