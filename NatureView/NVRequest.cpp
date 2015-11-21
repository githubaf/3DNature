
#include <windows.h>
#include "NVRequest.h"

// brutally mangled Requester.cpp code -- file requester
char FinalCombo[16384];
char *SimpleRequestFileName(const char *RequesterTitle)
{
OPENFILENAME NativeReq;

FinalCombo[0] = NULL;

NativeReq.lStructSize		= sizeof(OPENFILENAME);
NativeReq.hwndOwner			= NULL;
NativeReq.hInstance			= NULL;
NativeReq.lpstrFilter		= NULL;
NativeReq.lpstrCustomFilter	= NULL;
NativeReq.nMaxCustFilter	= 0;
NativeReq.nFilterIndex		= 0;
NativeReq.lpstrFile			= FinalCombo;
NativeReq.nMaxFile			= 2048; // 2k is current max limit in Windows, has this changed?
NativeReq.lpstrFileTitle	= NULL;
NativeReq.nMaxFileTitle		= NULL;
NativeReq.lpstrInitialDir	= NULL;
NativeReq.nFileOffset		= 0;
NativeReq.nFileExtension	= 0;
NativeReq.lpstrDefExt		= NULL;
NativeReq.lCustData			= NULL;
NativeReq.lpfnHook			= NULL;
NativeReq.lpTemplateName	= NULL;
NativeReq.lpstrFile = FinalCombo;
NativeReq.lpstrTitle = RequesterTitle;
NativeReq.lpstrFilter = "NatureView Scene Files (*.NVW, *.NVZ, *.ZIP)\0*.NVW;*.NVZ;*.ZIP\0All files\0*.*\0"; // an extra null is applied by the compiler
NativeReq.Flags = OFN_EXPLORER | OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
if(GetOpenFileName(&NativeReq))
	{
	return(FinalCombo);
	} // if

return(NULL);
} // SimpleRequestFileName



