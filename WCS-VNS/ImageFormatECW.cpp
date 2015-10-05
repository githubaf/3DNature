// ImageFormatECW.cpp

// note: 500Mb-limited ECW SDK should allow compression of 8bpp RGB images up to
// 13219 * 13219, 32-bit DEM data (J2K) at up to 11448 * 11448 and 16-bit DEM data
// (J2K) at up to 16190 * 16190.
// Unlimited ECW write license would be necessary for anything beyond that.

#include "stdafx.h"
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

#ifdef WCS_BUILD_ECW_SUPPORT
//#include "NCSECWClient.h"
//#include "NCSEcwCompressClient.h"
//#include "NCSErrors.h"
#endif // WCS_BUILD_ECW_SUPPORT

/*===========================================================================*/

#ifdef WCS_BUILD_ECW_SUPPORT

#define ECW_MAX_SUPPORTED_BANDS 18

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
DLLNCSecwNetBreakdownUrl = NULL;
DLLNCSEcwCompressAllocClient = NULL;
DLLNCSEcwCompressOpen = NULL;
DLLNCSEcwCompress = NULL;
DLLNCSEcwCompressClose = NULL;
DLLNCSEcwCompressFreeClient = NULL;

DLLNCSGetLastErrorNum = NULL;
DLLNCSGetLastErrorTextMsgBox = NULL;
DLLNCSGetLastErrorText = NULL;

if (NCSEcwdll  = LoadLibrary("NCSEcw.dll"))
	{
	DLLNCScbmOpenFileView = (DLLNCSCBMOPENFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmOpenFileView");
	DLLNCScbmGetViewFileInfo = (DLLNCSCBMGETVIEWFILEINFO)GetProcAddress(NCSEcwdll, "NCScbmGetViewFileInfo");
	DLLNCScbmSetFileView = (DLLNCSCBMSETFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmSetFileView");
	DLLNCScbmReadViewLineBIL = (DLLNCSCBMREADVIEWLINEBIL)GetProcAddress(NCSEcwdll, "NCScbmReadViewLineBIL");
	DLLNCScbmCloseFileView = (DLLNCSCBMCLOSEFILEVIEW)GetProcAddress(NCSEcwdll, "NCScbmCloseFileView");
	DLLNCScbmCloseFileViewEx = (DLLNCSCBMCLOSEFILEVIEWEX)GetProcAddress(NCSEcwdll, "NCScbmCloseFileViewEx");
	DLLNCSecwNetBreakdownUrl = (DLLNCSECWNETBREAKDOWNURL)GetProcAddress(NCSEcwdll, "NCSecwNetBreakdownUrl");
	} // if
if (NCSEcwCdll = LoadLibrary("NCSEcwC.dll"))
	{
	DLLNCSEcwCompressAllocClient = (DLLNCSECWCOMPRESSALLOCCLIENT)GetProcAddress(NCSEcwCdll, "NCSEcwCompressAllocClient");
	DLLNCSEcwCompressOpen = (DLLNCSECWCOMPRESSOPEN)GetProcAddress(NCSEcwCdll, "NCSEcwCompressOpen");
	DLLNCSEcwCompress = (DLLNCSECWCOMPRESS)GetProcAddress(NCSEcwCdll, "NCSEcwCompress");
	DLLNCSEcwCompressClose = (DLLNCSECWCOMPRESSCLOSE)GetProcAddress(NCSEcwCdll, "NCSEcwCompressClose");
	DLLNCSEcwCompressFreeClient = (DLLNCSECWCOMPRESSFREECLIENT)GetProcAddress(NCSEcwCdll, "NCSEcwCompressFreeClient");
	} // if
if (NCSUtildll = LoadLibrary("NCSUtil.dll"))
	{
	DLLNCSGetLastErrorNum = (DLLNCSGETLASTERRORNUM)GetProcAddress(NCSUtildll, "NCSGetLastErrorNum");
	DLLNCSGetLastErrorTextMsgBox = (DDLNCSGETLASTERRORTEXTMSGBOX)GetProcAddress(NCSUtildll, "NCSGetLastErrorTextMsgBox");
	DLLNCSGetLastErrorText = (DLLNCSGETLASTERRORTEXT)GetProcAddress(NCSUtildll, "NCSGetLastErrorText");
	} // if

if ((DLLNCScbmOpenFileView && DLLNCScbmGetViewFileInfo && DLLNCScbmSetFileView && DLLNCScbmReadViewLineBIL && DLLNCScbmCloseFileView && DLLNCScbmCloseFileViewEx && DLLNCSecwNetBreakdownUrl) &&
 (DLLNCSEcwCompressAllocClient && DLLNCSEcwCompressOpen && DLLNCSEcwCompress && DLLNCSEcwCompressClose && DLLNCSEcwCompressFreeClient) &&
 (DLLNCSGetLastErrorNum && DLLNCSGetLastErrorTextMsgBox && DLLNCSGetLastErrorText))
	{
	AllIsWell = 1;
	} // if
} // ECWDLLAPI::ECWDLLAPI

ECWDLLAPI::~ECWDLLAPI()
{
if (NCSUtildll)
	{
	FreeLibrary(NCSUtildll);
	NCSUtildll = NULL;
	} // if

if (NCSEcwdll)
	{
	FreeLibrary(NCSEcwdll);
	NCSEcwdll = NULL;
	} // if

if (NCSEcwCdll)
	{
	FreeLibrary(NCSEcwCdll);
	NCSEcwCdll = NULL;
	} // if

} // ECWDLLAPI::~ECWDLLAPI

/*===========================================================================*/

short Raster::LoadECW(char *Name, short SupressWng, ImageCacheControl *ICC)
{
double TNScale, TNScaleRows, TNScaleCols;
int AllocationsFailed = 0;
unsigned char *BandRowPtrs[ECW_MAX_SUPPORTED_BANDS];
long int WorkRow;
NCSFileView *pNCSFileView;
NCSFileViewFileInfo	*pNCSFileInfo;
UINT32	band_list[ECW_MAX_SUPPORTED_BANDS];	// list of individual bands to read, may be subset of actual bands
UINT32	MaxBands = 0, band;
NCSError eError = NCS_SUCCESS;
int LoadingSubTile = 0;
long int LoadOffsetX = 0, LoadOffsetY = 0;
unsigned char Success = 0;

if (ICC)
	{
	#ifdef WCS_IMAGE_MANAGEMENT
	ICC->QueryOnlyFlags = NULL; // clear all flags
	ICC->QueryOnlyFlags = WCS_BITMAPS_IMAGECAPABILITY_ISSMARTTILEABLE | WCS_BITMAPS_IMAGECAPABILITY_SUPPORTVIRTUALRES | WCS_BITMAPS_IMAGECAPABILITY_EFFICIENTTHUMBNAIL;
	LoadingSubTile = ICC->LoadingSubTile;
	#else // !WCS_IMAGE_MANAGEMENT
	ICC->QueryOnlyFlags = NULL; // clear all flags
	#endif // !WCS_IMAGE_MANAGEMENT
	} // if

if (!GlobalApp->ECWSupport)
	{
	GlobalApp->StatusLog->PostError(WCS_LOG_SEVERITY_WNG, "ECW support libraries unavailable.");
	return(Success);
	} // if

if (GlobalApp->ECWSupport->DLLNCScbmOpenFileView(GlobalApp->MainProj->MungPath(Name), &pNCSFileView, NULL) == NCS_SUCCESS)
	{
	char FileIsRemote = 0;
	#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
	int ProtoLen = 0, HostLen = 0, FNLen = 0;
	char *ProtoPart = NULL, *HostPart = NULL, *FNPart = NULL;
	// check and see if file is remote, to determine how to handle errors later...
	GlobalApp->ECWSupport->DLLNCSecwNetBreakdownUrl(GlobalApp->MainProj->MungPath(Name), &ProtoPart, &ProtoLen, &HostPart, &HostLen, &FNPart, &FNLen);
	if (ProtoLen > 0 || HostLen > 0)
		{
		FileIsRemote = 1;
		} // if
	#endif // WCS_BUILD_REMOTEFILE_SUPPORT

	GlobalApp->ECWSupport->DLLNCScbmGetViewFileInfo(pNCSFileView, &pNCSFileInfo);
	if (!LoadingSubTile && ((LoaderGeoRefShell = new GeoRefShell) != NULL)) // skip georef when only loading subtile
		{
		double TempNorth = 1.0, TempSouth = 0.0, TempEast = 1.0, TempWest = 0.0;
		CoordSys *TempCS;
		long Zone;
		char *DStr, *PStr;	// Datum & Projection strings

		LoaderGeoRefShell->BoundsType = WCS_GEOREFSHELL_BOUNDSTYPE_EDGES;

		TempNorth = pNCSFileInfo->fOriginY;
		TempWest = pNCSFileInfo->fOriginX;
		TempEast = (TempWest + ((pNCSFileInfo->nSizeX) * pNCSFileInfo->fCellIncrementX));	// since bounds type is edges, don't use CellSizeX - 1
		TempSouth = (TempNorth + ((pNCSFileInfo->nSizeY) * pNCSFileInfo->fCellIncrementY));
		if (pNCSFileInfo->eCellSizeUnits == ECW_CELL_UNITS_FEET)
			{
			TempNorth = ConvertToMeters(TempNorth, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			TempSouth = ConvertToMeters(TempSouth, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			TempEast = ConvertToMeters(TempEast, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			TempWest = ConvertToMeters(TempWest, WCS_USEFUL_UNIT_FEET_US_SURVEY);
			}

		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_NORTH].SetValue(TempNorth);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(TempWest);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(TempEast);
		LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_SOUTH].SetValue(TempSouth);

		// Set up a coordsys
		if (LoaderGeoRefShell->Host = TempCS = new CoordSys())
			{
			if (stricmp(pNCSFileInfo->szProjection, "GEODETIC") == 0)
				{
				TempCS->SetSystemByCode(0);			// Custom
				TempCS->Method.SetMethodByCode(6);	// Geographic
				TempCS->Datum.SetDatumByPrjName(pNCSFileInfo->szDatum);
				// Negate the sign of the east and west bounds since they're in PosEast notation
				LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_WEST].SetValue(-TempWest);
				LoaderGeoRefShell->GeoReg.AnimPar[WCS_EFFECTS_GEOREGISTER_ANIMPAR_EAST].SetValue(-TempEast);
				} // if
			else
				{
				// Why are there so many brain dead ways to do things???
				DStr = pNCSFileInfo->szDatum;
				PStr = pNCSFileInfo->szProjection;
				// see if it's a NAD27 StatePlane
				if ((strnicmp(DStr, "NAD27", 5) == 0) &&		// ignore units part of string - should be eCellSizeUnits
					((strnicmp(PStr, "TM", 2) == 0) || (strnicmp(PStr, "LM", 2) == 0) || (strnicmp(PStr, "OM", 2) == 0)))
					{
					TempCS->SetSystemByCode(4);	// SP - NAD27
					TempCS->SetStatePlaneZoneByERMName(PStr);
					}
				// see if it's a NAD83 StatePlane
				else if ((strnicmp(DStr, "NAD83", 5) == 0) &&	// ignore units part of string - should be eCellSizeUnits
					((strnicmp(PStr, "TM", 2) == 0) || (strnicmp(PStr, "LM", 2) == 0) || (strnicmp(PStr, "OM", 2) == 0)))
					{
					TempCS->SetSystemByCode(5);	// SP - NAD83
					TempCS->SetStatePlaneZoneByERMName(PStr);
					}
				// see if it's a UTM zone
				else if ((strnicmp(PStr, "NUTM", 4) == 0) || (strnicmp(PStr, "SUTM", 4) == 0))
					{
					// Get zone number (ignore units indicator if present)
					if (PStr[4] == 'F')
						Zone = atol(&PStr[5]);
					else
						Zone = atol(&PStr[4]);
					// Turn Zone # into Zone code
					if (PStr[0] == 'S')
						Zone = 2 * Zone - 1;
					else
						Zone = 2 * Zone - 2;
					if (strnicmp(DStr, "NAD27", 5) == 0)
						{
						TempCS->SetSystemByCode(2);			// UTM - NAD27
						TempCS->SetZoneByCode(Zone);
						}
					else if (strnicmp(DStr, "NAD83", 5) == 0)
						{
						TempCS->SetSystemByCode(3);			// UTM - NAD83
						TempCS->SetZoneByCode(Zone);
						}
					else if (strnicmp(DStr, "ED50", 4) == 0)
						{
						TempCS->SetSystemByCode(9);			// UTM - ED50
						TempCS->SetZoneByCode(Zone);
						}
					else if (strnicmp(DStr, "WGS84", 5) == 0)
						{
						TempCS->SetSystemByCode(10);		// UTM - WGS84
						TempCS->SetZoneByCode(Zone);
						}
					else // UTM in some other datum
						{
						TempCS->SetSystemByCode(10);		// UTM - WGS84
						TempCS->SetZoneByCode(Zone);
						TempCS->Datum.SetDatumByERMName(DStr);
						}
					}
				// see if it's a local system
				else if (stricmp(PStr, "local") == 0)
					{
					if (strnicmp(DStr, "WGS84", 5) == 0)
						{
						TempCS->SetSystemByCode(17);		// Surveyed - WGS84
						}
					else if (strnicmp(DStr, "NAD27", 5) == 0)
						{
						TempCS->SetSystemByCode(14);		// Surveyed - NAD27
						}
					else if (strnicmp(DStr, "NAD83", 5) == 0)
						{
						TempCS->SetSystemByCode(15);		// Surveyed - NAD83
						}
					else if (strnicmp(DStr, "ED50", 4) == 0)
						{
						TempCS->SetSystemByCode(16);		// Surveyed - ED50
						}
					else
						{
						TempCS->SetSystemByCode(17);		// Surveyed - WGS84
						TempCS->Datum.SetDatumByERMName(DStr);
						}
					}
				// see if it's an Australian Map Grid
				else if ((stricmp(DStr, "AGD66") == 0) && (strnicmp(PStr, "TMAMG", 5) == 0))
					{
					TempCS->SetSystemByCode(10);		// UTM - WGS84
					Zone = atol(&PStr[5]);
					// Turn Zone # into Zone code
					Zone = 2 * Zone - 1;
					TempCS->SetZoneByCode(Zone);
					TempCS->Datum.SetDatumByCode(37);	// AGD66
					}
				// see if it's an Australian Integrated Survey Grid
				else if ((stricmp(DStr, "AGD66") == 0) && (strnicmp(PStr, "TMISG", 5) == 0))
					{
					TempCS->SetSystemByCode(0);			// Custom
					TempCS->Method.SetMethodByCode(28);	// Transverse Mercator
					// zone # is in PStr[5..7] (541,542,543,551,552,553,561,562,563)
					TempCS->Datum.SetDatumByCode(37);	// AGD66
					}
				// see if it's a Map Grid of Australia
				else if ((stricmp(DStr, "GDA94") == 0) && (strnicmp(PStr, "MGA", 3) == 0))
					{
					TempCS->SetSystemByCode(10);		// UTM - WGS84 (GDA94 is essentially WGS84)
					Zone = atol(&PStr[4]);
					// Turn Zone # into Zone code
					Zone = 2 * Zone - 1;
					TempCS->SetZoneByCode(Zone);
					}
				}
			} // if ->Host = new CoordSys
		} // if LoaderGeoRefShell

	MaxBands = (UINT32)min(pNCSFileInfo->nBands, ECW_MAX_SUPPORTED_BANDS);
	for (band = 0; band < MaxBands; band++)
		{
		band_list[band] = band;
		} // for
	TNRealCols = Cols = pNCSFileInfo->nSizeX - 1;
	TNRealRows = Rows = pNCSFileInfo->nSizeY - 1;
#ifdef WCS_IMAGE_MANAGEMENT
	if (ICC)
		{
		if (ICC->LoadingSubTile)
			{ // loading a subtile, don't fiddle with settings
			TNRealRows = Rows = ICC->LoadHeight;
			TNRealCols = Cols = ICC->LoadWidth;
			LoadOffsetX = ICC->LoadXOri;
			LoadOffsetY = ICC->LoadYOri;
			} // if
		else
			{ // loading a whole file, pre-set settings
			if ((Cols >= WCS_RASTER_TILECACHE_AUTOENABLE_THRESH) || (Rows >= WCS_RASTER_TILECACHE_AUTOENABLE_THRESH))
				{
				// autoswitch on
				ICC->ImageManagementEnable = 1;
				ICC->NativeTileHeight = 256; // not really, but it's a good default
				ICC->NativeTileWidth = 256; // not really, but it's a good default
				} // if
			} // else
		
		} // if
#endif // WCS_IMAGE_MANAGEMENT
	if (TNRowsReq != 0)
		{ // load only a thumbnail-sized piece
		// we can't load anything smaller than 128x128, so nudge it up a little if asked to do so
		TNRowsReq = max(128, TNRowsReq);
		TNColsReq = max(128, TNColsReq);
		TNScaleRows = (double)TNRowsReq / (double)Rows;
		TNScaleCols = (double)TNColsReq / (double)Cols;
		TNScale = max(TNScaleRows, TNScaleCols);
		Rows = (long)(TNScale * (double)Rows);
		Cols = (long)(TNScale * (double)Cols);
		} // if
	if ((eError = GlobalApp->ECWSupport->DLLNCScbmSetFileView(pNCSFileView, MaxBands, band_list, LoadOffsetX, LoadOffsetY,
	 TNRealCols + LoadOffsetX, TNRealRows + LoadOffsetY, Cols, Rows)) == NCS_SUCCESS)
		{
		// process loaded raster data...
		for (band = 0; band < MaxBands; band++ )
			{
			if (ByteMap[band] = (UBYTE *)AppMem_Alloc(Cols * Rows, 0, "ECW Loader Bitmaps"))
				{
				// Clear
				memset(ByteMap[band], 0, Cols * Rows);
				} // if
			else
				{
				AllocationsFailed = 1;
				break;
				} // else
			} // for

		if (!AllocationsFailed) // everything ok?
			{
			for (WorkRow = 0; WorkRow < Rows; WorkRow++)
				{
				NCSEcwReadStatus ReadStatus;
				for (band = 0; band < MaxBands; band++ )
					{
					BandRowPtrs[band] = &ByteMap[band][WorkRow * Cols];
					} // for
				#ifdef WCS_BUILD_REMOTEFILE_SUPPORT
				if (FileIsRemote)
					{
					ReadStatus = NCSECW_READ_FAILED;
					while(ReadStatus == NCSECW_READ_FAILED)
						{
						NCSError ErrStatus;
						ReadStatus = GlobalApp->ECWSupport->DLLNCScbmReadViewLineBIL( pNCSFileView, BandRowPtrs);
						if (ReadStatus == NCSECW_READ_FAILED)
							{
							ErrStatus = GlobalApp->ECWSupport->DLLNCSGetLastErrorNum();
							if (ErrStatus != NCS_UNKNOWN_ERROR) // we seem to get NCS_UNKNOWN_ERROR to mean aborted due to timeout, work still in progress
								{ // anything else becomes a real abort
								ReadStatus = NCSECW_READ_CANCELLED; // this will cancel us out of the while, and thence the whole WorkRow loop
								break;
								} // if
							} // if
						else if (ReadStatus == NCSECW_READ_OK)
							{
							break; // need to get out of the while
							} // else
						} // while
					if (ReadStatus == NCSECW_READ_CANCELLED)
						{
						break; // we bail on the entire image
						} // else
					} // if
				else
				#endif // WCS_BUILD_REMOTEFILE_SUPPORT
					{ // local, abort on any error
					if ((ReadStatus = GlobalApp->ECWSupport->DLLNCScbmReadViewLineBIL( pNCSFileView, BandRowPtrs)) != NCSECW_READ_OK)
						{
						#ifdef DEBUG
						NCSError ErrStatus;
						ErrStatus = GlobalApp->ECWSupport->DLLNCSGetLastErrorNum();
						#endif // DEBUG
						break;
						} // if
					} // else
				} // for
			if (WorkRow == Rows)
				{
				Success = 1;
				} // if
			} // if
		} // if
	else
		{
		//sprintf(ECWErr, "Error = %s\n", NCSGetErrorText(eError));
		//OutputDebugStr(ECWErr);
		} // else
	GlobalApp->ECWSupport->DLLNCScbmCloseFileViewEx(pNCSFileView, TRUE);
	} // if

if (!Success)
	{
	for (band = 0; band < MaxBands; band++ )
		{
		if (ByteMap[band]) AppMem_Free(ByteMap[band], Cols * Rows); ByteMap[band] = NULL;
		} // for
	} // if

if (Success)
	{
	ByteBands = MaxBands;
	ByteBandSize = Rows * Cols;
	AlphaAvailable = 0;
	AlphaEnabled = 0;
	} // if

return(Success);

} // Raster::LoadECW()

/*===========================================================================*/

// Status callback function (OPTIONAL)
static void ECWWriteStatusCallback(NCSEcwCompressClient *pClient, UINT32 nCurrentLine)
{

// do nothing for now.

} // ECWWriteStatusCallback

/*===========================================================================*/

// Cancel callback function (OPTIONAL)
static BOOLEAN ECWWriteCancelCallback(NCSEcwCompressClient *pClient)
{

// Return TRUE to cancel compression
return(FALSE);

} // ECWWriteCancelCallback

/*===========================================================================*/

BOOLEAN ImageFormatECW::ECWWriteReadCallbackMethod(unsigned int nNextLine, float **ppInputArray)
{
UINT32 nCell;
IEEE4 *pLine[3];
unsigned char TempRGB[3];

// currently only three bands are supported, future versions of ECW may support more
pLine[0] = ppInputArray[0];
pLine[1] = ppInputArray[1];
pLine[2] = ppInputArray[2];

CharChannelData[0] = (unsigned char *)CharChannelNode[0]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_BYTE);
CharChannelData[1] = (unsigned char *)CharChannelNode[1]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_BYTE);
CharChannelData[2] = (unsigned char *)CharChannelNode[2]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_BYTE);
if (ShortChannelNode[0])
	ShortChannelData[0] = (unsigned short *)ShortChannelNode[0]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_SHORT);
else
	ShortChannelData[0] = NULL;

// future versions of ECW may support HDR colors but for now colors are clipped at 255
for (nCell = 0; nCell < ECWWidth; nCell++)
	{
	TempRGB[0] = CharChannelData[0][nCell];
	TempRGB[1] = CharChannelData[1][nCell];
	TempRGB[2] = CharChannelData[2][nCell];
	if (ShortChannelData[0] && ShortChannelData[0][nCell])
		rPixelFragment::ExtractClippedExponentialColors(TempRGB, ShortChannelData[0][nCell]);
	pLine[0][nCell] = (float)TempRGB[0];
	pLine[1][nCell] = (float)TempRGB[1];
	pLine[2][nCell] = (float)TempRGB[2];
	} // for

/* obsolete per GRH 6/5/02
for (nBand = 0; nBand < ECWBands; nBand++)
	{
	IEEE4 *pLine = ppInputArray[nBand];

	CharChannelData[nBand] = (unsigned char *)CharChannelNode[nBand]->GetDataLine(nNextLine, WCS_RASTER_BANDSET_BYTE);

	for (nCell = 0; nCell < ECWWidth; nCell++)
		{
		pLine[nCell] = (float)CharChannelData[nBand][nCell]; // / 255.0f;
		} // for
	} // for
*/

return(TRUE);

} // ImageFormatECW::ECWWriteReadCallbackMethod

/*===========================================================================*/

// Read callback function - called once for each input line
static BOOLEAN ECWWriteReadCallback(NCSEcwCompressClient *pClient, unsigned int nNextLine, float **ppInputArray)
{
ImageFormatECW *IFECW;

// Bridge gap back to C++
IFECW = (ImageFormatECW *)pClient->pClientData;
return(IFECW->ECWWriteReadCallbackMethod(nNextLine, ppInputArray));

} // ECWWriteReadCallback

/*===========================================================================*/

int ImageFormatECW::StartFrame(RasterBounds *RBounds, BufferNode *Buffers, long BufWidth, long BufHeight, long Frame)
{
#ifdef WCS_BUILD_DEMO
UserMessageDemo("Images cannot be saved.");
return(0);
#else // !WCS_BUILD_DEMO
//long xout, xscan, scanrow;
unsigned char AllIsWell = 0;
NCSEcwCompressClient *pClient;
NCSError OpenErr;
float Quality = 100.0f;

if (!GlobalApp->ECWSupport) return(0);


if (IOE)
	{
	if (!strcmp(IOE->Codec, "Low Quality"))
		{
		Quality = 30.0f;
		} // if
	else if (!strcmp(IOE->Codec, "Med Quality"))
		{
		Quality = 20.0f;
		} // if
	else if (!strcmp(IOE->Codec, "High Quality"))
		{
		Quality = 10.0f;
		} // if
	else // max
		{
		Quality = 1.0f; // minimum compression
		} // else
	} // if

// we know we'll need RGB bands so look for them specifically
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE RED"))
	CharChannelNode[0] = Buffers->FindBufferNode("FOLIAGE RED", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[0] = Buffers->FindBufferNode("RED", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE GREEN"))
	CharChannelNode[1] = Buffers->FindBufferNode("FOLIAGE GREEN", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[1] = Buffers->FindBufferNode("GREEN", WCS_RASTER_BANDSET_BYTE);
if (IOE && IOE->SaveEnabledBufferQuery("FOLIAGE BLUE"))
	CharChannelNode[2] = Buffers->FindBufferNode("FOLIAGE BLUE", WCS_RASTER_BANDSET_BYTE);
else
	CharChannelNode[2] = Buffers->FindBufferNode("BLUE", WCS_RASTER_BANDSET_BYTE);
ShortChannelNode[0] = Buffers->FindBufferNode("RGB EXPONENT", WCS_RASTER_BANDSET_SHORT);
ShortChannelData[0] = NULL;

pClient = GlobalApp->ECWSupport->DLLNCSEcwCompressAllocClient();

if (pClient && CharChannelNode[0] && CharChannelNode[1] && CharChannelNode[2])
	{
	ECWBands = pClient->nInputBands = 3;
	ECWWidth = pClient->nInOutSizeX = BufWidth;
	pClient->nInOutSizeY = BufHeight;
	pClient->eCompressFormat = COMPRESS_RGB;
	pClient->fTargetCompression = Quality;
	strcpy(pClient->szOutputFilename, GlobalApp->MainProj->MungPath(GetCompleteOutputPath()));

	pClient->pReadCallback = ECWWriteReadCallback;
	pClient->pStatusCallback = ECWWriteStatusCallback;
	pClient->pCancelCallback = ECWWriteCancelCallback;
	pClient->pClientData = (void*)this;

#ifdef WCS_BUILD_VNS
	// add some georeferencing info if we can
	if (RBounds && RBounds->FetchCoordSys())
		{
		CreateECWGeoRef(RBounds->FetchCoordSys(), pClient, RBounds);
		} // if
#endif // WCS_BUILD_VNS

	if ((OpenErr = GlobalApp->ECWSupport->DLLNCSEcwCompressOpen(pClient, FALSE)) == NCS_SUCCESS)
		{
		GlobalApp->ECWSupport->DLLNCSEcwCompress(pClient);
		GlobalApp->ECWSupport->DLLNCSEcwCompressClose(pClient);
		AllIsWell = 1;
		} // if open ok
	else
		{
		if (OpenErr == NCS_INPUT_SIZE_EXCEEDED)
			{
			UserMessageOK(GetCompleteOutputPath(),
				"Image size exceeds maximum allowed by limited ECW saver.\nOperation terminated.");
			} // if
		else if (OpenErr == NCS_INPUT_SIZE_TOO_SMALL)
			{
			UserMessageOK(GetCompleteOutputPath(),
				"Image size less than minimum allowed by ECW saver.\nOperation terminated.");
			} // if
		else
			{
			UserMessageOK(GetCompleteOutputPath(),
				"Can't open ECW image file for output!\nOperation terminated.");
			} // else
		} // else
	} // if
else
	{
	// you should never get to this message if everything was done correctly in render init
	UserMessageOK("Image Save", "Unable to save image file because not all required buffers have been rendered.");
	} // else

if (pClient)
	{
	GlobalApp->ECWSupport->DLLNCSEcwCompressFreeClient(pClient);
	pClient = NULL;
	} // if

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
} // ImageFormatECW::StartFrame

/*===========================================================================*/

void ImageFormatECW::CreateECWGeoRef(CoordSys *CS, NCSEcwCompressClient *pClient, RasterBounds *RBounds)
{
double N, S, W, E, pX, pY;
unsigned long SetZone = 0;
char string[8];

// Acquire boundaries of raster edges
if (RBounds->FetchBoundsEdgesGIS(N, S, W, E) && RBounds->FetchCellSizeGIS(pY, pX))
	{
	/*
	// Acquire boundaries of raster edge of UL (NW) corner pixel
	Corner1.ScrnXYZ[0] = 0.0;
	Corner1.ScrnXYZ[1] = 0.0;
	Corner1.ScrnXYZ[2] = 1.0;

	// Acquire boundaries of raster edge of SE diagonal pixel to the above
	Corner2.ScrnXYZ[0] = 1.0;
	Corner2.ScrnXYZ[1] = 1.0;
	Corner2.ScrnXYZ[2] = 1.0;

	RHost->Cam->UnProjectVertexDEM(CS, &Corner1, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
	CS->DefDegToProj(&Corner1);
	RHost->Cam->UnProjectVertexDEM(CS, &Corner2, RHost->EarthLatScaleMeters, RHost->PlanetRad, 1);
	CS->DefDegToProj(&Corner2);
	pClient->fOriginX = Corner1.xyz[0]; // WorldFile uses GIS pos=east notation for 'Easting'
	pClient->fOriginY = Corner1.xyz[1];
	pClient->fCellIncrementX = Corner2.xyz[0] - Corner1.xyz[0];
	pClient->fCellIncrementY = Corner2.xyz[1] - Corner1.xyz[1];
	*/
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

} // ImageFormatECW::CreateECWGeoRef

#endif // WCS_BUILD_ECW_SUPPORT
