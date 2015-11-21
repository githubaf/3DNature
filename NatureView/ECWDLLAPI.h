// ECWDLLAPI.h
// Support code to bind to ECW library at runtime if available and needed
// Made from WCS/VNS's ImageInputFormat.h on 11/28/05 by CXH

#include "NCSECWClient.h"
#include "NCSEcwCompressClient.h"
#include "NCSErrors.h"


typedef NCSEcwCompressClient * pNCSEcwCompressClient;

typedef NCS_IMPORT NCSError (* DLLNCSCBMOPENFILEVIEW)(char *, NCSFileView **, NCSEcwReadStatus (*pRefreshCallback)(NCSFileView *pNCSFileView));
typedef NCS_IMPORT NCSError (* DLLNCSCBMGETVIEWFILEINFO)(NCSFileView *, NCSFileViewFileInfo **);
typedef NCS_IMPORT NCSError (* DLLNCSCBMSETFILEVIEW)(NCSFileView *, UINT32, UINT32 *, UINT32, UINT32, UINT32, UINT32, UINT32, UINT32);
typedef NCS_IMPORT NCSEcwReadStatus (* DLLNCSCBMREADVIEWLINEBIL)(NCSFileView *, UINT8 **);
typedef NCS_IMPORT NCSError (* DLLNCSCBMCLOSEFILEVIEW)(NCSFileView *);
typedef NCS_IMPORT NCSError (* DLLNCSCBMCLOSEFILEVIEWEX)(NCSFileView *, BOOLEAN);
typedef NCS_IMPORT BOOLEAN (* DLLNCSECWNETBREAKDOWNURL)( char *, char **, int *, char **, int *, char **, int *);


typedef NCS_IMPORT pNCSEcwCompressClient(* DLLNCSECWCOMPRESSALLOCCLIENT)(void);
typedef NCS_IMPORT NCSError (* DLLNCSECWCOMPRESSOPEN)(NCSEcwCompressClient *, BOOLEAN); 
typedef NCS_IMPORT NCSError (* DLLNCSECWCOMPRESS)(NCSEcwCompressClient *);
typedef NCS_IMPORT NCSError (* DLLNCSECWCOMPRESSCLOSE)(NCSEcwCompressClient *);
typedef NCS_IMPORT NCSError (* DLLNCSECWCOMPRESSFREECLIENT)(NCSEcwCompressClient *);

// error handling
typedef NCS_IMPORT NCSError (* DLLNCSGETLASTERRORNUM)(void);
typedef NCS_IMPORT void (* DDLNCSGETLASTERRORTEXTMSGBOX)(NCSError, void *);
typedef NCS_IMPORT const char * (* DLLNCSGETLASTERRORTEXT)(NCSError);


class ECWDLLAPI
	{
	private:
		char AllIsWell;
		HINSTANCE NCSEcwdll, NCSEcwCdll, NCSUtildll;
	public:
		ECWDLLAPI();
		~ECWDLLAPI();
		char InitOk(void) {return(AllIsWell);};

		DLLNCSCBMOPENFILEVIEW DLLNCScbmOpenFileView;
		DLLNCSCBMGETVIEWFILEINFO DLLNCScbmGetViewFileInfo;
		DLLNCSCBMSETFILEVIEW DLLNCScbmSetFileView;
		DLLNCSCBMREADVIEWLINEBIL DLLNCScbmReadViewLineBIL;
		DLLNCSCBMCLOSEFILEVIEW DLLNCScbmCloseFileView;
		DLLNCSCBMCLOSEFILEVIEWEX DLLNCScbmCloseFileViewEx;
		DLLNCSECWNETBREAKDOWNURL DLLNCSecwNetBreakdownUrl;

		DLLNCSECWCOMPRESSALLOCCLIENT DLLNCSEcwCompressAllocClient;
		DLLNCSECWCOMPRESSOPEN DLLNCSEcwCompressOpen;
		DLLNCSECWCOMPRESS DLLNCSEcwCompress;
		DLLNCSECWCOMPRESSCLOSE DLLNCSEcwCompressClose;
		DLLNCSECWCOMPRESSFREECLIENT DLLNCSEcwCompressFreeClient;

		DLLNCSGETLASTERRORNUM DLLNCSGetLastErrorNum;
		DDLNCSGETLASTERRORTEXTMSGBOX DLLNCSGetLastErrorTextMsgBox;
		DLLNCSGETLASTERRORTEXT DLLNCSGetLastErrorText;
	}; // ECWDLLAPI
