// NVTerrainSupport.cpp
// various support functions for terrain operations that aren't in the 3DN DEM library
// ECW code adapted over from VNS on 11/30/05 by CXH


#include "NVTerrainSupport.h"
#include "NVTerrain.h"
#include "DEMCore.h"
#include "NVScene.h"


#ifdef NVW_BUILD_ECW_SUPPORT
#include "NCSECWClient.h"
#include "NCSEcwCompressClient.h"
#include "NCSErrors.h"

#include "RawOSGImageAccessSupport.h" // for writing into osg::Images
#pragma comment(lib, "DelayImp.lib")

#ifdef NVW_BUILD_JP2BOX_SUPPORT
#include "NCSJP2File.h"
#endif // NVW_BUILD_JP2BOX_SUPPORT


#endif // NVW_BUILD_ECW_SUPPORT

extern NVScene MasterScene;

const std::string EncodeTileString(const unsigned long int TileNumX, const unsigned long int TileNumY, const unsigned long int TileLODNum)
{
std::ostringstream Result;

Result << TileNumX << " " << TileNumY << " " << TileLODNum;

return(Result.str());
} // EncodeTileString



bool DecodeTileString(unsigned long int &TileNumX, unsigned long int &TileNumY, unsigned long int &TileLODNum, const std::string EncodedInput)
{
bool Success = false;
std::istringstream InputStream(EncodedInput);

InputStream >> TileNumX >> TileNumY >> TileLODNum;
Success = true;

return(Success);
} // DecodeTileString

TerrainManager TileMaster;

TerrainManager *GetTerrainManager(void) {return(&TileMaster);}

void FixupELEVFileStepVars(DEM *FileToFix)
{
double LatDif, LonDif, SynLonStep, SynLatStep;
LonDif = FileToFix->Westest() - FileToFix->Eastest();
LatDif = FileToFix->Northest() - FileToFix->Southest();
SynLonStep = LonDif / (FileToFix->LonEntries() - 1);
SynLatStep = LatDif / (FileToFix->LatEntries() - 1);

FileToFix->pLatStep =SynLatStep;
FileToFix->pLonStep = SynLonStep;

} // FixupELEVFileStepVars

// this either loads an old-style ELEV file, or the metadata only via the ECW/J2K loader API
unsigned long int LoadDEMWrapper(const char *InName, DEM *LoadDestination)
{
unsigned long int Result = 0;
bool IsELEV = false;

if(InName)
	{
	size_t NameLen;
	NameLen = strlen(InName);
	if(NameLen > 5)
		{
		if(!stricmp(&InName[NameLen - 5], ".elev"))
			{
			IsELEV = true;
			} // if
		} // if
	
	if(IsELEV)
		{
		FILE *DEMFile;
		if(DEMFile = fopen(InName, "rb"))
			{
			Result = LoadDestination->LoadDEM(DEMFile, 0, NULL); // load as usual
			fclose(DEMFile);
			// fixup possibly bogus latstep/lonstep values found in old ELEV files written by pre-SX2-release WCS/VNS
			// ECW/JP2 files (below) don't have this problem and calculate latstep/lonstep in their loader
			FixupELEVFileStepVars(LoadDestination);
			} // if
		} // if
	else
		{
		MasterScene.SetTerrainIsPageable(true);
		Result = LoadDEMMetadataViaECW(InName, LoadDestination); // load via ECW, pass min/max from out-of-band
		} // else
	} // if

return(Result);
} // LoadDEMWrapper


static NCSFileView *pNCSFileViewDEM;
static NCSFileViewFileInfoEx	*pNCSFileInfoDEM;
static bool DEMFileIsRemote = false;
static UINT32	DEMMaxBands, DEMband, DEMband_list[NVW_ECW_MAX_SUPPORTED_BANDS];	// list of individual bands to read, may be subset of actual bands


unsigned long int LoadDEMMetadataViaECW(const char *InName, DEM *LoadDestination)
{
unsigned long int Result = 0;
#ifdef NVW_BUILD_ECW_SUPPORT
double FileMaxElValue = FLT_MAX, FileMinElValue = -FLT_MAX;

NCSError eError = NCS_SUCCESS;


if(NCScbmOpenFileView((char *)InName, &pNCSFileViewDEM, NULL) == NCS_SUCCESS)
	{
	int ProtoLen = 0, HostLen = 0, FNLen = 0;
	char *ProtoPart = NULL, *HostPart = NULL, *FNPart = NULL;
	#ifdef NVW_BUILD_JP2BOX_SUPPORT
	JP2UUID3DNBox RangeEquivs;
	CNCSJP2UUIDBox *pUUIDBox = NULL;

	pUUIDBox = GetUUIDBox(CNCSJP2WorldBox::sm_UUID, pUUIDBox);
	if (pUUIDBox) 
		{
		*(CNCSJP2UUIDBox*)&RangeEquivs = *pUUIDBox; // copies common elements of CNCSJP2UUIDBox base class into JP2UUID3DNBox derived class. I think.
		if(Stream.Seek(RangeEquivs.m_nDBoxOffset, CNCSJPCIOStream::START) && RangeEquivs.Parse(*this, Stream) == NCS_SUCCESS) // read the extra data from the stream
			{
			FileMaxElValue = RangeEquivs.GetOrigMaxVal();
			FileMinElValue = RangeEquivs.GetOrigMinVal();
			} // if
		} // if

	#endif // NVW_BUILD_JP2BOX_SUPPORT
	// check and see if file is remote, to determine how to handle errors later...
	NCSecwNetBreakdownUrl((char *)InName, &ProtoPart, &ProtoLen, &HostPart, &HostLen, &FNPart, &FNLen);
	if(ProtoLen > 0 || HostLen > 0)
		{
		DEMFileIsRemote = true;
		} // if

	NCScbmGetViewFileInfoEx(pNCSFileViewDEM, &pNCSFileInfoDEM);
	// the test below is bogus because JP2 files don't mark the values as being degrees :(
	//if (pNCSFileInfo->eCellSizeUnits == ECW_CELL_UNITS_DEGREES) // we can only handle geographic right now
		{
		double TempNorth = 1.0, TempSouth = 0.0, TempEast = 1.0, TempWest = 0.0;

		TempNorth = pNCSFileInfoDEM->fOriginY;
		TempWest = pNCSFileInfoDEM->fOriginX;
		TempEast = (TempWest + ((pNCSFileInfoDEM->nSizeX) * pNCSFileInfoDEM->fCellIncrementX));	// since bounds type is edges, don't use CellSizeX - 1
		TempSouth = (TempNorth + ((pNCSFileInfoDEM->nSizeY) * pNCSFileInfoDEM->fCellIncrementY));
		TempNorth -= .5 * pNCSFileInfoDEM->fCellIncrementY;
		TempSouth += .5 * pNCSFileInfoDEM->fCellIncrementY;
		TempWest += .5 * pNCSFileInfoDEM->fCellIncrementX;
		TempEast -= .5 * pNCSFileInfoDEM->fCellIncrementX;

		// store in DEM according to WCS DEM convention of W lon positive
		if (LoadDestination)
			{
			LoadDestination->pLonEntries = pNCSFileInfoDEM->nSizeX;
			LoadDestination->pLatEntries = pNCSFileInfoDEM->nSizeY;
			LoadDestination->pNorthWest.Lat = TempNorth;
			LoadDestination->pSouthEast.Lat = TempSouth;
			LoadDestination->pNorthWest.Lon = -TempWest;
			LoadDestination->pSouthEast.Lon = -TempEast;
			LoadDestination->pElScale = 0.001; /// .001 means meters
			LoadDestination->pElDatum = 0.0;
			if(FileMaxElValue != FLT_MAX) // did we get max/min values from the JP2 file?
				{
				LoadDestination->pElMaxEl = FileMaxElValue;
				LoadDestination->pElMinEl = FileMinElValue;
				} // if
			if(MasterScene.CheckTerrainMinElev()) // if present, NVW file tags override JP2 embedded data
				{
				LoadDestination->pElMinEl = MasterScene.GetTerrainMinElev();
				} // if
			if(MasterScene.CheckTerrainMaxElev()) // if present, NVW file tags override JP2 embedded data
				{
				LoadDestination->pElMaxEl = MasterScene.GetTerrainMaxElev();
				} // if
			LoadDestination->pElSamples = (LoadDestination->pLonEntries - 1) * (LoadDestination->pLatEntries - 1);
			FixupELEVFileStepVars(LoadDestination); // sets up latstep and lonstep
			Result = 1;
			} // if
		} // if
	} // if

#endif // NVW_BUILD_ECW_SUPPORT

return(Result);
} // LoadDEMMetadataViaECW


unsigned long int LoadDEMTileViaECW(const char *InName, DEM *LoadDestination, double TerrainMinElev, double TerrainMaxElev, unsigned long int LX, unsigned long int RX, unsigned long int TY, unsigned long int BY)
{
unsigned long int Result = 0;
#ifdef NVW_BUILD_ECW_SUPPORT
bool AllocationsFailed = false;
unsigned char *ByteMap[NVW_ECW_MAX_SUPPORTED_BANDS];
unsigned long int Width, Height;
NCSError eError = NCS_SUCCESS;
void *BandRowPtrs[NVW_ECW_MAX_SUPPORTED_BANDS];
unsigned long int WorkRow;
NCSEcwCellType eType;
size_t CellSize = 0; // in Bytes

// metadata should already have been loaded by LoadDEMMetadataViaECW, we won't duplicate that process here

if(!LoadDestination) return(Result);
LoadDestination->FreeRawMap(); // clear out anything existing
if(LoadDestination->MapSize()) LoadDestination->AllocRawMap();

for(DEMband = 0; DEMband < NVW_ECW_MAX_SUPPORTED_BANDS; DEMband++ )
	{
	ByteMap[DEMband] = NULL;
	} // for


if(pNCSFileViewDEM)
	{
	eType = pNCSFileInfoDEM->eCellType;
	switch(eType)
		{
		case NCSCT_UINT8:  CellSize = 1; break;
		case NCSCT_UINT16: CellSize = 2; break;
		case NCSCT_UINT32:  CellSize = 4; break;
		case NCSCT_UINT64:  CellSize = 8; break;
		case NCSCT_IEEE4:  CellSize = 4; break;
		}; // switch
	DEMMaxBands = (UINT32)min(pNCSFileInfoDEM->nBands, NVW_ECW_MAX_SUPPORTED_BANDS);
	for(DEMband = 0; DEMband < DEMMaxBands; DEMband++)
		{
		DEMband_list[DEMband] = DEMband;
		} // for

	Width = RX-LX;
	Height = BY-TY;

	// set up the raster section we want
	if ((eError = NCScbmSetFileView(pNCSFileViewDEM, DEMMaxBands, DEMband_list, LX, TY,
	 LX + (Width - 1), TY + (Height - 1), Width, Height)) == NCS_SUCCESS)
		{
		// load and process the rows of data

		for(DEMband = 0; DEMband < DEMMaxBands; DEMband++ )
			{
			if(ByteMap[DEMband] = (unsigned char *)malloc(Width * Height * CellSize))
				{
				// Clear
				memset(ByteMap[DEMband], 0, Width * Height * CellSize);
				} // if
			else
				{
				AllocationsFailed = true;
				break;
				} // else
			} // for
		if(!AllocationsFailed) // everything ok?
			{
			for(WorkRow = 0; WorkRow < Height; WorkRow++)
				{
				NCSEcwReadStatus ReadStatus;
				for(DEMband = 0; DEMband < DEMMaxBands; DEMband++ )
					{
					BandRowPtrs[DEMband] = (void *)&ByteMap[DEMband][WorkRow * Width * CellSize];
					} // for
				if(DEMFileIsRemote)
					{
					ReadStatus = NCSECW_READ_FAILED;
					while(ReadStatus == NCSECW_READ_FAILED)
						{
						NCSError ErrStatus;
						ReadStatus = NCScbmReadViewLineBILEx( pNCSFileViewDEM, eType, BandRowPtrs);
						if(ReadStatus == NCSECW_READ_FAILED)
							{
							ErrStatus = NCSGetLastErrorNum();
							if(ErrStatus != NCS_UNKNOWN_ERROR) // we seem to get NCS_UNKNOWN_ERROR to mean aborted due to timeout, work still in progress
								{ // anything else becomes a real abort
								ReadStatus = NCSECW_READ_CANCELLED; // this will cancel us out of the while, and thence the whole WorkRow loop
								break;
								} // if
							} // if
						else if(ReadStatus == NCSECW_READ_OK)
							{
							break; // need to get out of the while
							} // else
						} // while
					if(ReadStatus == NCSECW_READ_CANCELLED)
						{
						break; // we bail on the entire image
						} // else
					} // if
				else
					{ // local, abort on any error
					if((ReadStatus = NCScbmReadViewLineBILEx( pNCSFileViewDEM, eType, BandRowPtrs)) != NCSECW_READ_OK)
						{
						#ifdef DEBUG
						NCSError ErrStatus;
						ErrStatus = NCSGetLastErrorNum();
						#endif // DEBUG
						break;
						} // if
					} // else
				} // for
			if(WorkRow == Height)
				{
				Result = 1;
				} // if
			else
				{
				Result = 0;
				} // else
			} // if !AllocationsFailed
		} // if NCScbmSetFileView successful

	} // if

if(Result)
	{ // do corner-turn of DEM data from ECW raster order to WCS DEM order
	float ScaledElevation, ScaleMultiplyFactor = 1.0f, ScaleAddFactor = 0.0f;
	if(TerrainMinElev != -FLT_MAX && TerrainMaxElev != FLT_MAX)
		{
		ScaleAddFactor = TerrainMinElev;
		ScaleMultiplyFactor = (TerrainMaxElev - TerrainMinElev);
		} // if
	// re-cast for interpreting other types
	unsigned short int *ShortMap = (unsigned short *)(ByteMap[0]);
	unsigned long int *LongMap = (unsigned long *)(ByteMap[0]);
	float *FloatMap = (float *)(ByteMap[0]);
	for(WorkRow = 0; WorkRow < Height; WorkRow++)
		{
		unsigned long int RowInvariantA, RowInvariantB;
		RowInvariantA = ((Height - 1) - WorkRow); // used in DEM write access
		RowInvariantB = (WorkRow * Width); // used in Raster read access
		for(unsigned long int WorkCol = 0; WorkCol < Width; WorkCol++)
			{
			// scale and transfer elevation values
			switch(eType)
				{
				case NCSCT_UINT8:  ScaledElevation = ((float)ByteMap[0][RowInvariantB + WorkCol]) * (1.0f / 255.0f); break; // convert to 0...1 range
				case NCSCT_UINT16: ScaledElevation = ((float)ShortMap[RowInvariantB + WorkCol]) * (1.0f / 65535.0f); break; // convert to 0...1 range
				case NCSCT_UINT32:  ScaledElevation = ((float)LongMap[RowInvariantB + WorkCol]) * (1.0f / ((float)0x0fffffff)); break; // convert to 0...1 range (we only get 28 bits, not 32)
				case NCSCT_UINT64:  break; // unsupported right now
				case NCSCT_IEEE4:  FloatMap[RowInvariantB + WorkCol]; break; // no range adjustment needed
				} // switch
			
			ScaledElevation *= ScaleMultiplyFactor;
			ScaledElevation += ScaleAddFactor;
			LoadDestination->RawMap[(WorkCol * Height) + RowInvariantA] = ScaledElevation;
			} // for
		} // for
	LoadDestination->pElMaxEl = TerrainMaxElev;
	LoadDestination->pElMinEl = TerrainMinElev;
	} // if

// clean up the temporary raster-order copy, success or failure
for(DEMband = 0; DEMband < DEMMaxBands; DEMband++ )
	{
	if(ByteMap[DEMband]) free(ByteMap[DEMband]); ByteMap[DEMband] = NULL;
	} // for


#endif // NVW_BUILD_ECW_SUPPORT

return(Result);
} // LoadDEMTileViaECW


static NCSFileView *pNCSFileViewImage;
static NCSFileViewFileInfo	*pNCSFileInfoImage;
static bool ImageFileIsRemote = false;
static UINT32	ImageMaxBands, Imageband, Imageband_list[NVW_ECW_MAX_SUPPORTED_BANDS];	// list of individual bands to read, may be subset of actual bands

unsigned long int LoadImageMetadataViaECW(const char *InName, unsigned long int &Width, unsigned long int &Height)
{
unsigned long int Result = 0;
#ifdef NVW_BUILD_ECW_SUPPORT

NCSError eError = NCS_SUCCESS;

if(NCScbmOpenFileView((char *)InName, &pNCSFileViewImage, NULL) == NCS_SUCCESS)
	{
	int ProtoLen = 0, HostLen = 0, FNLen = 0;
	char *ProtoPart = NULL, *HostPart = NULL, *FNPart = NULL;
	// check and see if file is remote, to determine how to handle errors later...
	NCSecwNetBreakdownUrl((char *)InName, &ProtoPart, &ProtoLen, &HostPart, &HostLen, &FNPart, &FNLen);
	if(ProtoLen > 0 || HostLen > 0)
		{
		ImageFileIsRemote = true;
		} // if

	NCScbmGetViewFileInfo(pNCSFileViewImage, &pNCSFileInfoImage);
	Width = pNCSFileInfoImage->nSizeX;
	Height = pNCSFileInfoImage->nSizeY;

	ImageMaxBands = (UINT32)min(pNCSFileInfoImage->nBands, NVW_ECW_MAX_SUPPORTED_BANDS);
	for(Imageband = 0; Imageband < ImageMaxBands; Imageband++)
		{
		Imageband_list[Imageband] = Imageband;
		} // for

	Result = 1;
	} // if

#endif // NVW_BUILD_ECW_SUPPORT

return(Result);
} // LoadImageMetadataViaECW



unsigned long int LoadImageTileViaECW(const char *InName, osg::Image *DestImage, unsigned long int LX, unsigned long int RX, unsigned long int TY, unsigned long int BY, unsigned long int OffsetX, unsigned long int OffsetY, unsigned long int OutputWidth, unsigned long int OutputHeight)
{
unsigned long int Result = 0;
#ifdef NVW_BUILD_ECW_SUPPORT
bool AllocationsFailed = false;
unsigned char *ByteMap[NVW_ECW_MAX_SUPPORTED_BANDS];
NCSError eError = NCS_SUCCESS;
UINT8 *BandRowPtrs[NVW_ECW_MAX_SUPPORTED_BANDS];
unsigned long int WorkRow;

if(pNCSFileViewImage)
	{
	for(Imageband = 0; Imageband < ImageMaxBands; Imageband++)
		{
		ByteMap[Imageband] = NULL;
		} // for

	// set up the raster section we want
	if ((eError = NCScbmSetFileView(pNCSFileViewImage, ImageMaxBands, Imageband_list, LX, TY,
	 RX, BY, OutputWidth, OutputHeight)) == NCS_SUCCESS)
		{
		// load and process the rows of data

		for(Imageband = 0; Imageband < ImageMaxBands; Imageband++ )
			{
			// We only need one line as we deinterleave on the fly
			if(ByteMap[Imageband] = (unsigned char *)malloc(OutputWidth))
				{
				// Clear
				// We only need one line as we deinterleave on the fly
				memset(ByteMap[Imageband], 0, OutputWidth);
				BandRowPtrs[Imageband] = (UINT8 *)&ByteMap[Imageband][0];
				} // if
			else
				{
				AllocationsFailed = true;
				break;
				} // else
			} // for
		if(!AllocationsFailed) // everything ok?
			{
			for(WorkRow = 0; WorkRow < OutputHeight; WorkRow++)
				{
				NCSEcwReadStatus ReadStatus;
				if(ImageFileIsRemote)
					{
					ReadStatus = NCSECW_READ_FAILED;
					while(ReadStatus == NCSECW_READ_FAILED)
						{
						NCSError ErrStatus;
						ReadStatus = NCScbmReadViewLineBIL( pNCSFileViewImage, BandRowPtrs);
						if(ReadStatus == NCSECW_READ_FAILED)
							{
							ErrStatus = NCSGetLastErrorNum();
							if(ErrStatus != NCS_UNKNOWN_ERROR) // we seem to get NCS_UNKNOWN_ERROR to mean aborted due to timeout, work still in progress
								{ // anything else becomes a real abort
								ReadStatus = NCSECW_READ_CANCELLED; // this will cancel us out of the while, and thence the whole WorkRow loop
								break;
								} // if
							} // if
						else if(ReadStatus == NCSECW_READ_OK)
							{
							break; // need to get out of the while
							} // else
						} // while
					if(ReadStatus == NCSECW_READ_CANCELLED)
						{
						break; // we bail on the entire image
						} // else
					} // if
				else
					{ // local, abort on any error
					if((ReadStatus = NCScbmReadViewLineBIL( pNCSFileViewImage, BandRowPtrs)) == NCSECW_READ_OK)
						{
						// process loaded data
						unsigned long int X, DestHeight;
						unsigned char *DestData;
						unsigned char R, G, B;
						
						DestHeight = DestImage->t();
						DestData = DestImage->data(OffsetX, (DestHeight - 1) - (OffsetY + WorkRow), 0);
						for(X = 0; X < OutputWidth; X++)
							{
							R = ByteMap[0][X];
							G = ByteMap[1][X];
							B = ByteMap[2][X];
							DestData += OSGImageWritePixelRGB(DestData, R, G, B);
							} // for
						} // if
					else
						{ // not ok
						#ifdef DEBUG
						NCSError ErrStatus;
						ErrStatus = NCSGetLastErrorNum();
						#endif // DEBUG
						break;
						} // if
					} // else
				} // for
			if(WorkRow == OutputHeight)
				{
				Result = 1;
				} // if
			else
				{
				Result = 0;
				} // else
			} // if !AllocationsFailed
		} // if NCScbmSetFileView successful
	else
		{
		Result = 0;
		} // else


	} // if

// clean up the temporary raster-order copy, success or failure
for(Imageband = 0; Imageband < ImageMaxBands; Imageband++ )
	{
	if(ByteMap[Imageband]) free(ByteMap[Imageband]); ByteMap[Imageband] = NULL;
	} // for


#endif // NVW_BUILD_ECW_SUPPORT

return(Result);
} // LoadImageTileViaECW

void CleanupOpenECWFiles(void)
{
#ifdef NVW_BUILD_ECW_SUPPORT
NCScbmCloseFileViewEx(pNCSFileViewImage, TRUE);
pNCSFileViewImage = NULL;
NCScbmCloseFileViewEx(pNCSFileViewDEM, TRUE);
pNCSFileViewDEM = NULL;
#endif // NVW_BUILD_ECW_SUPPORT
} // CleanupOpenECWFiles

#ifdef NVW_BUILD_ECW_SUPPORT
#ifdef NVW_BUILD_JP2BOX_SUPPORT
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
		void SetNormalizedMaxVal(NormalizedMaxVal) {_NormalizedMaxVal = NormalizedMaxVal;};
		void SetNormalizedMinVal(NormalizedMinVal) {_NormalizedMinVal = NormalizedMinVal;};
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


if(m_UUID == sm_UUID)
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


#endif // NVW_BUILD_JP2BOX_SUPPORT
#endif // NVW_BUILD_ECW_SUPPORT


