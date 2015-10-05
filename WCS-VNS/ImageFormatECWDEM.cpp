// ImageFormatECWDEM.cpp

#include "stdafx.h"

//#define WCS_BUILD_JP2BOX_SUPPORT

#include "ImageFormatConfig.h"
#include "AppMem.h"
#include "Useful.h"
#include "Application.h"
#include "Project.h"
#include "Requester.h"
#include "Log.h"
#include "ImageInputFormat.h"
#include "ImageFormat.h"
#include "ImageFormatIFF.h"
#include "Raster.h"
#include "PixelManager.h"

// These commands tell the linker to not require ECW DLLs right at program startup, just at the point where they're actually called.
// Visual7 and beyond have a UI for specifying this, and don't recognize these commands
// They're only here for Visual 6

#if _MSC_VER <= 1200 // VC++6
#pragma comment(linker, "/ignore:4199")    // suppress "no imports found" warning
#pragma comment(lib, "delayimp.lib")
#pragma comment(linker, "/delayload:NCSUtil.dll")
#pragma comment(linker, "/delayload:NCSEcw.dll")
#pragma comment(linker, "/delayload:NCSEcwC.dll")
#pragma comment(linker, "/delayload:NCScnet.dll")
#endif // Vis6 delayload trickery

#ifdef WCS_BUILD_ECW_SUPPORT
#ifdef WCS_BUILD_ECWDEM_SUPPORT
#ifdef WCS_BUILD_JP2BOX_SUPPORT
#include "NCSJP2File.h"
#endif // WCS_BUILD_JP2BOX_SUPPORT


#include "NCSFile.h" // ECW C++ class definition


#ifdef WCS_BUILD_JP2BOX_SUPPORT
class JP2UUID3DNBox : public CNCSJP2File::CNCSJP2UUIDBox
	{
	private:
		double _OrigMaxVal, _OrigMinVal;
		UINT32 _NormalizedMaxVal, _NormalizedMinVal;
		/** UUID for the 3DN box */
		static NCSUUID sm_UUID;
	public:
		JP2UUID3DNBox();
		JP2UUID3DNBox(double OrigMaxVal, double OrigMinVal, unsigned long int NormalizedMaxVal, unsigned long int NormalizedMinVal);
		virtual CNCSError Parse(class CNCSJP2File &JP2File, CNCSJPCIOStream &Stream);
		virtual CNCSError UnParse(class CNCSJP2File &JP2File, CNCSJPCIOStream &Stream);
		double GetOrigMaxVal(void) const {return(_OrigMaxVal);};
		double GetOrigMinVal(void) const {return(_OrigMinVal);};
		double GetNormalizedMaxVal(void) const {return(_NormalizedMaxVal);};
		double GetNormalizedMinVal(void) const {return(_NormalizedMinVal);};
		void SetOrigMaxVal(double OrigMaxVal) {_OrigMaxVal = OrigMaxVal;};
		void SetOrigMinVal(double OrigMinVal) {_OrigMinVal = OrigMinVal;};
		void SetNormalizedMaxVal(unsigned long int NormalizedMaxVal) {_NormalizedMaxVal = NormalizedMaxVal;};
		void SetNormalizedMinVal(unsigned long int NormalizedMinVal) {_NormalizedMinVal = NormalizedMinVal;};
	}; // JP2UUID3DNBox

//"c73875e4-dd6d-4eb2-a933-45d33324b198"
UINT8 _3DN_UUID[16] = {0xc7,0x38,0x75,0xe4,0xdd,0x6d,0x4e,0xb2,0xa9,0x33,0x45,0xd3,0x33,0x24,0xb1,0x98};
NCSUUID JP2UUID3DNBox::sm_UUID(_3DN_UUID);


JP2UUID3DNBox::JP2UUID3DNBox()
{
memcpy(&m_UUID, &sm_UUID, sizeof(sm_UUID));
} // JP2UUID3DNBox::JP2UUID3DNBox

JP2UUID3DNBox::JP2UUID3DNBox(double OrigMaxVal, double OrigMinVal, unsigned long int NormalizedMaxVal, unsigned long int NormalizedMinVal)
{
_OrigMaxVal = OrigMaxVal;
_OrigMinVal = OrigMinVal;
_NormalizedMaxVal = NormalizedMaxVal;
_NormalizedMinVal = NormalizedMinVal;
memcpy(&m_UUID, &sm_UUID, sizeof(sm_UUID));
} // JP2UUID3DNBox::JP2UUID3DNBox

CNCSError JP2UUID3DNBox::Parse(class CNCSJP2File &JP2File, CNCSJPCIOStream &Stream)
{
CNCSError Error;

//See NCSJPCIOStream.h to get an understanding of these macros
NCSJP2_CHECKIO_BEGIN(Error, Stream);
NCSJP2_CHECKIO(Read(m_UUID.m_UUID, sizeof(m_UUID.m_UUID)));


if (m_UUID == sm_UUID)
	{
	Stream.ReadIEEE8(_OrigMaxVal);
	Stream.ReadIEEE8(_OrigMinVal);
	Stream.ReadUINT32(_NormalizedMaxVal);
	Stream.ReadUINT32(_NormalizedMinVal);
	} // if
NCSJP2_CHECKIO_END();

return(Error);
} // JP2UUID3DNBox::Parse


CNCSError JP2UUID3DNBox::UnParse(CNCSJP2File &JP2File, CNCSJPCIOStream &Stream)
{
CNCSError Error;

Error = CNCSJP2Box::UnParse(JP2File, Stream);
NCSJP2_CHECKIO_BEGIN(Error, Stream);
	Stream.WriteIEEE8(_OrigMaxVal);
	Stream.WriteIEEE8(_OrigMinVal);
	Stream.WriteUINT32(_NormalizedMaxVal);
	Stream.WriteUINT32(_NormalizedMinVal);
NCSJP2_CHECKIO_END();
return(Error);
} // JP2UUID3DNBox::UnParse

#endif // WCS_BUILD_JP2BOX_SUPPORT

#endif // WCS_BUILD_ECWDEM_SUPPORT
#endif // WCS_BUILD_ECW_SUPPORT
#include "ImageInputFormat.h"

/*===========================================================================*/

#if defined WCS_BUILD_ECW_SUPPORT && defined WCS_BUILD_ECWDEM_SUPPORT && defined WCS_BUILD_SX2

//#define ECW_MAX_SUPPORTED_BANDS 18

//extern char ECWErr[1024];


// ECWDEMCompressor class
//
// A subclass of CNCSFile.
class ECWDEMCompressor: public CNCSFile {
public:
	// Default constructor
	ECWDEMCompressor() {};
	// Virtual destructor.  
	virtual ~ECWDEMCompressor() {};

	// Compression
	virtual CNCSError Compress(ImageFormatECWDEM *NewIFECW, RasterBounds *RBounds, char *pDstFile, UINT32 ImgWidth, UINT32 ImgHeight, char Depth, UINT16 nRate);

	// WriteReadLine and WriteStatus are inherited from the base CNCSFile class, 
	// and overriden to perform the compression.

	// WriteReadLine() - called by SDK once for each line read.  
	// The parameters passed are the next line number being read from the input, 
	// and a pointer to a buffer into which that line's data should be loaded by 
	// your code.
	//
	// The cell type of the buffer is the same as that passed into SetFileInfo() in the 
	// NCSFileViewFileInfoEx struct.
	virtual CNCSError WriteReadLine(UINT32 nNextLine, void **ppInputArray);

	// Status update function, called for each line output during compression, and therefore 
	// useful for updating progress indicators in the interface of an application.
	virtual void WriteStatus(UINT32 nCurrentLine);

private:
	// Member variable used for our input
	ImageFormatECWDEM *IFECW;
}; // ECWDEMCompressor

/*===========================================================================*/

CNCSError ECWDEMCompressor::Compress(ImageFormatECWDEM *NewIFECW, RasterBounds *RBounds, char *pDstFile, UINT32 ImgWidth, UINT32 ImgHeight, char Depth, UINT16 nRate)
{
//NCSFileViewFileInfoEx has basic info about the file - dimensions, bands, 
//georeferencing information etc.
NCSFileViewFileInfoEx *pDstInfo = (NCSFileViewFileInfoEx *)NCSMalloc(sizeof(NCSFileViewFileInfoEx), true);
NCSFileViewFileInfoEx DstInfo = *pDstInfo;
CNCSError Error;

IFECW = NewIFECW;
DstInfo.nSizeX = ImgWidth;
DstInfo.nSizeY = ImgHeight;
switch (Depth)
	{
	case 8: DstInfo.eCellType = NCSCT_UINT8; break;
	case 16: DstInfo.eCellType = NCSCT_UINT16; break;
	//case 28: DstInfo.eCellType = NCSCT_IEEE4; break;
	case 28: DstInfo.eCellType = NCSCT_UINT32; break;
	} // switch

DstInfo.nBands = 1;
DstInfo.nCompressionRate = nRate;
DstInfo.eColorSpace = NCSCS_GREYSCALE;

#ifdef WCS_BUILD_VNS
if (RBounds && RBounds->FetchCoordSys())
	{
	IFECW->CreateECWGeoRefNewAPI(RBounds->FetchCoordSys(), &DstInfo, RBounds);
	} // if
#endif // WCS_BUILD_VNS

//DstInfo.szProjection = "RAW";
//DstInfo.szDatum = "RAW";
//DstInfo.eCellSizeUnits = ECW_CELL_UNITS_METERS;
//DstInfo.fCellIncrementX = 1.0;
//DstInfo.fCellIncrementY = 1.0;
//DstInfo.fOriginX = 0.0;
//DstInfo.fOriginY = 0.0;

DstInfo.pBands = (NCSFileBandInfo *)(new NCSFileBandInfo[DstInfo.nBands]);
DstInfo.pBands[0].nBits = Depth;
DstInfo.pBands[0].bSigned = false;
DstInfo.pBands[0].szDesc = "Elevation";

//Call SetFileInfo to establish the file information we are going to 
//use for compression.  The parameters used are those from the NCSFileViewFileInfoEx 
//struct we have populated using metadata derived from our input raster.
Error = SetFileInfo(DstInfo);
Error = Open(pDstFile, false, true);
if (Error == NCS_SUCCESS)
	{
	#ifdef WCS_BUILD_JP2BOX_SUPPORT
	JP2UUID3DNBox RangeEquivs(NewIFECW->HighElev, NewIFECW->LowElev, (unsigned long int)NewIFECW->MaxValue, 0);
	AddBox(&RangeEquivs);
	#endif // WCS_BUILD_JP2BOX_SUPPORT

	Error = Write();
	if (Error == NCS_SUCCESS)
		fprintf(stdout,"Finished compression\n");
	else if (Error == NCS_USER_CANCELLED_COMPRESSION)
		fprintf(stdout,"Compression cancelled\n");
	else fprintf(stdout,"Error during compression: %s\n",Error.GetErrorMessage());

	Error = Close(true);
	} // if

return(Error);

} // ECWDEMCompressor::Compress

/*===========================================================================*/

// This is called once for each output line.
// In this example the input data is obtained pixel by pixel from the input image 
// using the GetPixel method of the GDI+ Bitmap object created from the source image 
// file above.
//
CNCSError ECWDEMCompressor::WriteReadLine(UINT32 nNextLine, void **ppInputArray)
{
CNCSError Error = NCS_SUCCESS;
NCSFileViewFileInfoEx *pInfo = GetFileInfo();

IFECW->WriteReadLineCallback(nNextLine, ppInputArray, pInfo->nSizeX, pInfo->eCellType);

return NCS_SUCCESS;

} // ECWDEMCompressor::WriteReadLine

/*===========================================================================*/

// Status update.
void ECWDEMCompressor::WriteStatus(UINT32 nCurrentLine)
{

/*
NCSFileViewFileInfoEx *pInfo = GetFileInfo();

UINT32 nPercentComplete = (UINT32)((nCurrentLine * 100)/(pInfo->nSizeY));
if (nPercentComplete > m_nPercentComplete)
	{
	m_nPercentComplete = nPercentComplete;
	} // if
*/

} // ECWDEMCompressor::WriteStatus

/*===========================================================================*/

BOOLEAN ImageFormatECWDEM::WriteReadLineCallback(UINT32 nNextLine, void **ppInputArray, UINT32 Width, NCSEcwCellType CellType)
{
UINT8  *pLine8  = (UINT8  *)ppInputArray[0];
UINT16 *pLine16 = (UINT16 *)ppInputArray[0];
UINT32 *pLine32 = (UINT32 *)ppInputArray[0];
IEEE4  *pLineF  = (IEEE4 *)ppInputArray[0];
float TempElev;
UINT32 nCell;

FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_FLOAT);

for(nCell = 0; nCell < Width; nCell++)
	{
	// ElevScale is computed in StartFrame, Low and HighElev must be set before StartFrame is called
	if (CellType == NCSCT_IEEE4)
		{ // no scaling needed
		TempElev = FloatChannelData[0][nCell];
		} // if
	else
		{
		TempElev = WCS_ECW_MINVALUE + ElevScale * (FloatChannelData[0][nCell] - LowElev);
		if (TempElev < WCS_ECW_MINVALUE)
			TempElev = WCS_ECW_MINVALUE;
		else if (TempElev > MaxValue)
			TempElev = MaxValue;
		} // else
	
//lint -save -e787 (not all enumerated values are in switch without a default)
	switch (CellType)
		{
		case NCSCT_UINT8:  pLine8[nCell]  = (UINT8)TempElev; break;
		case NCSCT_UINT16: pLine16[nCell] = (UINT16)TempElev; break;
		case NCSCT_UINT32: pLine32[nCell] = (UINT32)TempElev; break;
		case NCSCT_IEEE4:  pLineF[nCell] =  (IEEE4)TempElev; break;
		} // switch BitDepth
//lint -restore
	
	} // for

return(TRUE);

} // ImageFormatECWDEM::WriteReadLineCallback

/*===========================================================================*/

void ImageFormatECWDEM::CreateECWGeoRefNewAPI(CoordSys *CS, NCSFileViewFileInfoEx *pClient, RasterBounds *RBounds)
{
double N, S, W, E, pX, pY;
unsigned long SetZone = 0;
char string[8];

// Acquire boundaries of raster edges
if (RBounds->FetchBoundsEdgesGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
	{
	pClient->fOriginX = W;
	pClient->fOriginY = N;
	pClient->fCellIncrementX = pX;
	pClient->fCellIncrementY = -pY;

	if (CS->GetGeographic())
		{
		pClient->eCellSizeUnits = ECW_CELL_UNITS_DEGREES;
		} // if
	else
		{
		pClient->eCellSizeUnits = ECW_CELL_UNITS_METERS;
		} // else

	if (CS->Method.GCTPMethod == 9)	// Transverse Mercator
		{
		if (CS->Datum.DatumID == 2)
			{
			strcpy(pClient->szDatum, "NAD27");
			SetZone = 1;
			} // if
		else if (CS->Datum.DatumID == 3)
			{
			strcpy(pClient->szDatum, "NAD83");
			SetZone = 1;
			} // else if
		else if (CS->Datum.DatumID == 9)
			{
			strcpy(pClient->szDatum, "ED50");
			SetZone = 1;
			} // else if
		else if (CS->Datum.DatumID == 10)
			{
			strcpy(pClient->szDatum, "WGS84");
			SetZone = 1;
			} // else if
		if (SetZone)
			{
			if (CS->ZoneID % 2)
				strcpy(pClient->szProjection, "S");
			else
				strcpy(pClient->szProjection, "N");
			sprintf(string, "UTM%02d", CS->ZoneID / 2 + 1);
			strcat(pClient->szProjection, string);
			} // if
		} // if
	} // if

} // ImageFormatECWDEM::CreateECWGeoRefNewAPI

/*===========================================================================*/

// older C-API code

/*

// Status callback function (OPTIONAL)
static void ECWDEMWriteStatusCallback(NCSEcwCompressClient *pClient, UINT32 nCurrentLine)
{
// do nothing for now.
} // ECWDEMWriteStatusCallback

// Cancel callback function (OPTIONAL)
static BOOLEAN ECWDEMWriteCancelCallback(NCSEcwCompressClient *pClient)
{
// Return TRUE to cancel compression
return(FALSE);
} // ECWDEMWriteCancelCallback

BOOLEAN ImageFormatECWDEM::ECWDEMWriteReadCallbackMethod(unsigned int nNextLine, float **ppInputArray)
{
UINT32 nCell;
IEEE4 *pLine;
float TempElev;

// currently only one band is supported, future versions of ECW may support more
pLine = ppInputArray[0];

FloatChannelData[0] = (float *)FloatChannelNode[0]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_FLOAT);

// future versions of ECW may support HDR colors but for now colors are clipped at 255
for(nCell = 0; nCell < ECWWidth; nCell++)
	{
	// ElevScale is computed in StartFrame, Low and HighElev must be set before StartFrame is called
	TempElev = WCS_ECW_MINVALUE + ElevScale * (FloatChannelData[0][nCell] - LowElev);
	if (TempElev < WCS_ECW_MINVALUE)
		TempElev = WCS_ECW_MINVALUE;
	else if (TempElev > MaxValue)
		TempElev = MaxValue;
	pLine[nCell] = TempElev;
	} // for

return(TRUE);
} // ImageFormatECWDEM::ECWDEMWriteReadCallbackMethod

// Read callback function - called once for each input line
static BOOLEAN ECWDEMWriteReadCallback(NCSEcwCompressClient *pClient, unsigned int nNextLine, float **ppInputArray)
{
ImageFormatECWDEM *IFECW;

// Bridge gap back to C++
IFECW = (ImageFormatECWDEM *)pClient->pClientData;
return(IFECW->ECWDEMWriteReadCallbackMethod(nNextLine, ppInputArray));
} // ECWWriteReadCallback

*/

/*===========================================================================*/

int ImageFormatECWDEM::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
//long xout, xscan, scanrow;
unsigned char AllIsWell = 0;
CNCSError OpenErr;
UINT16 Quality = 100;
ECWDEMCompressor ECWDEMC;

if (!GlobalApp->ECWSupport) return(0);


if (IOE)
	{
	if (!strcmp(IOE->Codec, "Low Quality"))
		{
		Quality = 10;
		BitDepth = 8;
		MaxValue = WCS_ECW_MAXVALUE8;
		} // if
	else if (!strcmp(IOE->Codec, "Med Quality"))
		{
		Quality = 7;
		BitDepth = 16;
		MaxValue = WCS_ECW_MAXVALUE16;
		} // if
	else if (!strcmp(IOE->Codec, "High Quality"))
		{
		Quality = 4;
		BitDepth = 28;
		MaxValue = WCS_ECW_MAXVALUE28;
		} // if
	else // max/lossless
		{
		Quality = 1;
		BitDepth = 28;
		MaxValue = WCS_ECW_MAXVALUE28; // 28 bits on
		} // else
	} // if

//if (1) // <<<>>> If we're lacking JPEG2K, switch back to ECW
//	{
//	SetJP2(0);
//	} // if
if (!GetJP2()) // If we're not JPEG2K, switch back to ECW 8-bit limits regardless of quality
	{
	BitDepth = 8;
	MaxValue = WCS_ECW_MAXVALUE8;
	} // if
else
	{ // for JPEG2000 we can adjust the depth precision downward to allow for bigger DEM exports
	if (BitDepth > 8)
		{
		if (BufWidth * BufHeight > 125000000) // exceeds 32-bit capacity, try 16-bit
			{
			BitDepth = 16;
			MaxValue = WCS_ECW_MAXVALUE16;
			} // if
		if (BufWidth * BufHeight > 250000000) // exceeds 16-bit capacity, try 8-bit
			{
			BitDepth = 8;
			MaxValue = WCS_ECW_MAXVALUE8;
			} // if
		} // if
	} // else

// we know we'll need elevation bands so look for them specifically
FloatChannelNode[0] = Buffers->FindBufferNode("ELEVATION", WCS_RASTER_BANDSET_FLOAT);

if (FloatChannelNode[0])
	{
	// ElevScale is computed in here, Low and HighElev must be set before StartFrame is called
	// ElevScale is used to transform the raw elevation data into the acceptable value range for ECW
	// set WCS_ECW_MAXVALUE & WCS_ECW_MINVALUE for the value range acceptable
	if (HighElev < LowElev)
		swmem(&HighElev, &LowElev, sizeof (float));
	if (HighElev == LowElev)
		{
		if (HighElev == 0.0f)
			ElevScale = 0.0f;
		else
			ElevScale = ((MaxValue + WCS_ECW_MINVALUE) * .5f) / HighElev;
		} // if
	else
		ElevScale = (MaxValue - WCS_ECW_MINVALUE) / (HighElev - LowElev);


	OpenErr = ECWDEMC.Compress(this, RBounds, GlobalApp->MainProj->MungPath(GetCompleteOutputPath()), BufWidth, BufHeight, BitDepth, Quality);

	if (OpenErr == NCS_SUCCESS)
		{
		AllIsWell = 1;
		} // if compress ok
	else
		{
		if (OpenErr == NCS_INPUT_SIZE_EXCEEDED)
			{
			UserMessageOK(GetCompleteOutputPath(),
				"DEM size exceeds maximum allowed by limited ECW saver.\nOperation terminated.");
			} // if
		else if (OpenErr == NCS_INPUT_SIZE_TOO_SMALL)
			{
			UserMessageOK(GetCompleteOutputPath(),
				"DEM size less than minimum allowed by ECW saver.\nOperation terminated.");
			} // if
		else
			{
			UserMessageOK(GetCompleteOutputPath(),
				"Can't open ECW DEM file for output!\nOperation terminated.");
			} // else
		} // else
	} // if
else
	{
	// you should never get to this message if everything was done correctly in render init
	UserMessageOK("Image Save", "Unable to save DEM file because elevation buffer has not been rendered.");
	} // else

if (TransferBuf)
	{
	//AppMem_Free(TransferBuf, BufWidth * Channels);
	TransferBuf = NULL;
	} // if

if (!AllIsWell)
	{
	GlobalApp->StatusLog->PostStockError(WCS_LOG_ERR_WRITE_FAIL, GetCompleteOutputPath());
	return(1);
	} // if

return(0);

#endif // !DEMO

} // ImageFormatECWDEM::StartFrame


#endif // WCS_BUILD_ECW_SUPPORT && WCS_BUILD_ECWDEM_SUPPORT && defined WCS_BUILD_SX2
