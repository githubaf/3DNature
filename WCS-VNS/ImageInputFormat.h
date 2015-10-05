// ImageInputFormat.h
// Built from image loading portions of Bitmaps.h
// on 12/06/00 by CXH

#if _MSC_VER > 1000
#pragma once
#endif // _MSC_VER > 1000

#ifndef WCS_IMAGEINPUTFORMAT_H
#define WCS_IMAGEINPUTFORMAT_H

#include "ImageFormatConfig.h"

#include "Types.h"

#ifndef WIN32
#define WIN32
#endif // WIN32

#ifdef WCS_BUILD_ECW_SUPPORT
#include "NCSECWClient.h"
#include "NCSEcwCompressClient.h"
#include "NCSErrors.h"
#endif // WCS_BUILD_ECW_SUPPORT

enum
	{
	WCS_BITMAPS_IDENTIMAGE_ERROR = 0,
	WCS_BITMAPS_IDENTIMAGE_UNKNOWN,
	WCS_BITMAPS_IDENTIMAGE_IFFILBM,
	WCS_BITMAPS_IDENTIMAGE_TARGA,
	WCS_BITMAPS_IDENTIMAGE_BMP,
	WCS_BITMAPS_IDENTIMAGE_PCT,
	WCS_BITMAPS_IDENTIMAGE_PIC,
	WCS_BITMAPS_IDENTIMAGE_PICT,
	WCS_BITMAPS_IDENTIMAGE_WCSDEM,
	WCS_BITMAPS_IDENTIMAGE_JPG,
	WCS_BITMAPS_IDENTIMAGE_JPEG,
	WCS_BITMAPS_IDENTIMAGE_TIF,
	WCS_BITMAPS_IDENTIMAGE_TIFF,
	WCS_BITMAPS_IDENTIMAGE_PNG,
	WCS_BITMAPS_IDENTIMAGE_ECW,
	WCS_BITMAPS_IDENTIMAGE_MRSID,
	WCS_BITMAPS_IDENTIMAGE_JP2,
	WCS_BITMAPS_IMAGE_RAW,
	WCS_BITMAPS_IMAGE_RAWINTER,
	WCS_BITMAPS_IDENTIMAGE_FPBM, // no longer supported
	WCS_BITMAPS_IDENTIMAGE_RLA,
	WCS_BITMAPS_IDENTIMAGE_RPF,
	WCS_BITMAPS_IDENTIMAGE_IFFZBUF,
	WCS_BITMAPS_IDENTIMAGE_SGIRGB,
	WCS_BITMAPS_IDENTIMAGE_LAST_DONT_USE_ME // eliminates the need to omit comma from last real entry
	}; // Image file format types

class ImageCacheControl;

short CheckExistUnknownImageExtension(char *Name);

short LoadRasterImage(char *Name, Raster *LoadRas, short SupressWng, ImageCacheControl *ICC = NULL);

short IdentImage(char *Name);
short IdentImageByExtension(char *Name);
//char *AppendImageExtension(char *Name, short Format, short Caps);

#ifdef WCS_BUILD_ECW_SUPPORT

typedef NCSEcwCompressClient * pNCSEcwCompressClient;

//typedef WINGDIAPI void (APIENTRY * DGLLOOKAT)(GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble, GLdouble);
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
#endif // WCS_BUILD_ECW_SUPPORT

#define JJ_PICT_CODE

#define PICT_OK                0

typedef struct
{
	FILE *d_file;
	char *d_name;
	char NewMode;
	int d_type;
	int d_width;
	int d_height;
	int d_depth;
	int d_result;
	int d_LineBufSize;
	int d_CompBufSize;
	unsigned char *d_LineBuf;
	unsigned char *d_CompBuf;
	unsigned char *Rbuf, *Gbuf, *Bbuf, *Abuf;
	unsigned char DoAlpha;
	int d_rowBytes;
	int d_out;
	int d_padding;
} PICTdata;

typedef struct
{
	short top;
	short left;
	short bottom;
	short right;
} PICTrect;

class Monitor;

#define	PICTruntochar(c)	(257-(c))
#define	PICTcounttochar(c)	((c)-1)
#define	PICT_RUN_THRESH		3
#define	PICT_MAX_RUN			128
#define	PICT_MAX_COUNT		128

typedef unsigned char            ImageValue;
typedef union un_ImageProtocol  *ImageProtocolID;
typedef struct st_ImLoaderLocal {
	void             *priv_data;
	int               result;
	const char       *filename;
	Monitor          *monitor;
	ImageProtocolID (*begin) (void *, int type);
	void            (*done) (void *, ImageProtocolID);
} ImLoaderLocal;
typedef struct st_ImSaverLocal {
	void            *priv_data;
	int              result;
	int              type;
	const char      *filename;
	Monitor         *monitor;
	int            (*sendData) (void *, ImageProtocolID, int);
} ImSaverLocal;
typedef struct st_ColorProtocol {
	int              type;
	void            *priv_data;
	void           (*setSize) (void *, int, int, int);
	int            (*sendLine) (void *, int, const ImageValue *,
				    const ImageValue *);
	int            (*done) (void *, int);
} ColorProtocol;
typedef struct st_IndexProtocol {
	int              type;
	void            *priv_data;
	void           (*setSize) (void *, int, int, int);
	void           (*numColors) (void *, int);
	void           (*setMap) (void *, int, const ImageValue[3]);
	int            (*sendLine) (void *, int, const ImageValue *,
				    const ImageValue *);
	int            (*done) (void *, int);
} IndexProtocol;
typedef union un_ImageProtocol {
	int              type;
	ColorProtocol    color;
	IndexProtocol    index;
} ImageProtocol;
#define PICT_IMG_RGB24       0
#define PICT_IMG_GREY8       1
#define PICT_IMG_INDEX8      2
#define PICT_IMGF_ALPHA               1
#define PICT_IMGF_REVERSE             2
#define PICT_IPSTAT_OK                0
#define PICT_IPSTAT_NOREC             1
#define PICT_IPSTAT_BADFILE           2
#define PICT_IPSTAT_ABORT             3
#define PICT_IPSTAT_FAILED           99
#define PICT_IP_SETSIZE(p,w,h,f)     (*(p)->setSize) ((p)->priv_data,w,h,f)
#define PICT_IP_NUMCOLORS(p,n)       (*(p)->numColors) ((p)->priv_data,n)
#define PICT_IP_SETMAP(p,i,val)      (*(p)->setMap) ((p)->priv_data,i,val)
#define PICT_IP_SENDLINE(p,ln,d,a)   (*(p)->sendLine) ((p)->priv_data,ln,d,a)
#define PICT_IP_DONE(p,err)          (*(p)->done) ((p)->priv_data,err)

ImageProtocolID PICTHostBegin(void *, int type);
int PICTHostColorDone(void *vdat, int error);
void PICTHostDone(void *vdat, ImageProtocolID);
void PICTHostSetSize(void *vdat, int w, int h, int flags);
int PICTHostSendLine(void *vdat, int line, const ImageValue *rgbline, const ImageValue *alphaline);
int PICTHostSendData(void *vdat, ImageProtocolID IPID, int flags);

void PICTputShort(FILE *fd, int i);
int PICTPackBits(unsigned char *rowpixels, unsigned char *packed, int cols, int rowBytes, FILE *fd);
int PICTUnPackBits(FILE *f, char* buf, int rowBytes);
int PICTUnPackBits16(FILE *f, short* buf, int rowBytes);
void WritePICTHeader(PICTdata *dat);
void PICTSetSize(void *vdat, int w, int h, int flags);
int PICTSendLine(void *vdat, int line, const ImageValue *rgbline, const ImageValue *alphaline);
int PICTDone(void *vdat, int error);
int PICTMainSaver(ImSaverLocal *local);
void PICTGetRect(FILE* f, PICTrect *R);
int PICTLoader(ImLoaderLocal *local);

#endif // WCS_IMAGEINPUTFORMAT_H
