// ECWDLLAPI.cpp
// Support code to bind to ECW library at runtime if available and needed
// Made from WCS/VNS's ImageFormatECW.cpp on 11/28/05 by CXH

char ECWErr[1024];

ECWDLLAPI::ECWDLLAPI()
{
AllIsWell = 0;

DLLNCScbmOpenFileView = NULL;
DLLNCScbmGetViewFileInfo = NULL;
DLLNCScbmSetFileView = NULL;
DLLNCScbmReadViewLineBIL = NULL;
DLLNCScbmCloseFileView = NULL;
DLLNCScbmCloseFileViewEx = NULL;
DLLNCSEcwCompressAllocClient = NULL;
DLLNCSEcwCompressOpen = NULL;
DLLNCSEcwCompress = NULL;
DLLNCSEcwCompressClose = NULL;
DLLNCSEcwCompressFreeClient = NULL;

DLLNCSGetLastErrorNum = NULL;
DLLNCSGetLastErrorTextMsgBox = NULL;
DLLNCSGetLastErrorText = NULL;

if(NCSEcwdll  = LoadLibrary("NCSEcw.dll"))
	{
	DLLNCScbmOpenFileView = (DLLNCSCBMOPENFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmOpenFileView");
	DLLNCScbmGetViewFileInfo = (DLLNCSCBMGETVIEWFILEINFO)GetProcAddress(NCSEcwdll, "NCScbmGetViewFileInfo");
	DLLNCScbmSetFileView = (DLLNCSCBMSETFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmSetFileView");
	DLLNCScbmReadViewLineBIL = (DLLNCSCBMREADVIEWLINEBIL)GetProcAddress(NCSEcwdll, "NCScbmReadViewLineBIL");
	DLLNCScbmCloseFileView = (DLLNCSCBMCLOSEFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmCloseFileView");
	DLLNCScbmCloseFileViewEx = (DLLNCSCBMCLOSEFILEVIEWEX)GetProcAddress(NCSEcwdll, "NCScbmCloseFileViewEx");
	DLLNCSecwNetBreakdownUrl = (DLLNCSECWNETBREAKDOWNURL)GetProcAddress(NCSEcwdll, "NCSecwNetBreakdownUrl");
	} // if
if(NCSEcwCdll = LoadLibrary("NCSEcwC.dll"))
	{
	DLLNCSEcwCompressAllocClient = (DLLNCSECWCOMPRESSALLOCCLIENT)GetProcAddress(NCSEcwCdll, "NCSEcwCompressAllocClient");
	DLLNCSEcwCompressOpen = (DLLNCSECWCOMPRESSOPEN)GetProcAddress(NCSEcwCdll, "NCSEcwCompressOpen");
	DLLNCSEcwCompress = (DLLNCSECWCOMPRESS)GetProcAddress(NCSEcwCdll, "NCSEcwCompress");
	DLLNCSEcwCompressClose = (DLLNCSECWCOMPRESSCLOSE)GetProcAddress(NCSEcwCdll, "NCSEcwCompressClose");
	DLLNCSEcwCompressFreeClient = (DLLNCSECWCOMPRESSFREECLIENT)GetProcAddress(NCSEcwCdll, "NCSEcwCompressFreeClient");
	} // if
if(NCSUtildll = LoadLibrary("NCSUtil.dll"))
	{
	DLLNCSGetLastErrorNum = (DLLNCSGETLASTERRORNUM)GetProcAddress(NCSUtildll, "NCSGetLastErrorNum");
	DLLNCSGetLastErrorTextMsgBox = (DDLNCSGETLASTERRORTEXTMSGBOX)GetProcAddress(NCSUtildll, "NCSGetLastErrorTextMsgBox");
	DLLNCSGetLastErrorText = (DLLNCSGETLASTERRORTEXT)GetProcAddress(NCSUtildll, "NCSGetLastErrorText");
	} // if

if((DLLNCScbmOpenFileView && DLLNCScbmGetViewFileInfo && DLLNCScbmSetFileView && DLLNCScbmReadViewLineBIL && DLLNCScbmCloseFileView && DLLNCScbmCloseFileViewEx && DLLNCSecwNetBreakdownUrl) &&
 (DLLNCSEcwCompressAllocClient && DLLNCSEcwCompressOpen && DLLNCSEcwCompress && DLLNCSEcwCompressClose && DLLNCSEcwCompressFreeClient) &&
 (DLLNCSGetLastErrorNum && DLLNCSGetLastErrorTextMsgBox && DLLNCSGetLastErrorText))
	{
	AllIsWell = 1;
	} // if
} // ECWDLLAPI::ECWDLLAPI

ECWDLLAPI::~ECWDLLAPI()
{
if(NCSUtildll)
	{
	FreeLibrary(NCSUtildll);
	NCSUtildll = NULL;
	} // if

if(NCSEcwdll)
	{
	FreeLibrary(NCSEcwdll);
	NCSEcwdll = NULL;
	} // if

if(NCSEcwCdll)
	{
	FreeLibrary(NCSEcwCdll);
	NCSEcwCdll = NULL;
	} // if

} // ECWDLLAPI::~ECWDLLAPI

