
#include <direct.h>
#include <string.h>
#include <sstream>

#include <osg/PrimitiveSet> 
#include <osg/PositionAttitudeTransform>
#include <osg/ref_ptr> 
#include <osg/LOD>
#include <osg/PagedLOD>
#include <osg/Geode>
#include <osg/Timer>
#include <osgDB/ReadFile>

using namespace osg;

#include "DEMCore.h"
#include "NVTerrain.h"
#include "NVTerrainSupport.h"
#include "NVScene.h"
#include "NVMathSupport.h"
#include "NVSigCheck.h"
#include "NVNodeMasks.h"
#include "TerrainTexCoordCalc.h"
#include "RawOSGImageAccessSupport.h"
#include "LODSharedData.h"

#include "UsefulTime.h"

#ifdef HJW
#include "TelcontarLib.h"
#ifdef DEBUG
#pragma comment(lib, "TelcontarLibd.lib")
#else
#pragma comment(lib, "TelcontarLib.lib")
#endif // !DEBUG

// only for OutputDebugString
//#include "Windows.h"

bool getServerMapGet(char *serverurl, char *layers, char *styles, char *format, char *exceptions, const unsigned short pWidth, const unsigned short pHeight, struct boxCoords &boxCoords, char *outMapName);
bool getServerMapGetUSGS_IMS(char *serverurl, char *layers, char *styles, char *format, char *exceptions, const unsigned short pWidth, const unsigned short pHeight, struct boxCoords &boxCoords, char *outMapName);

#endif // HJW

extern NVScene MasterScene;

extern char ThreadDebugMsg[160];

osg::ref_ptr<osg::Image> TerrainDrapeImage; // so it can be accessed by tile-builder




// only accessed from within this file
DEM *CreateDownSampled(DEM *InputTile, int DecimationFactor);
DEM *CreateFlappedDEM(DEM *InputTile, TerrainTile *Tile);
bool PagedLOD_AddPagedNode(osg::PagedLOD* This, unsigned int childNo, float min, float max, const std::string& filename, float priorityOffset=0.0f, float priorityScale=1.0f);

void CopySubTileHeader(DEM *SuperTile, DEM *SubTile, unsigned long int CellOffsetX, unsigned long int CellOffsetY, unsigned long int CellsWide, unsigned long int CellsHigh)
{
// Copy over important variables
SubTile->pElScale = SuperTile->pElScale;
SubTile->pElDatum = SuperTile->pElDatum;
SubTile->pLatStep = SuperTile->pLatStep;
SubTile->pLonStep = SuperTile->pLonStep;
SubTile->SetNullReject(SuperTile->GetNullReject());
SubTile->SetNullValue(SuperTile->NullValue());

SubTile->pElMaxEl = SuperTile->pElMaxEl;
SubTile->pElMinEl = SuperTile->pElMinEl;

SubTile->pLatEntries = CellsHigh;
SubTile->pLonEntries = CellsWide;

// calculate and store Northest/Southest/Eastest/Westest
// these are in WCS-style North-positive, West-positive notation
// for geographic coords, LatStep and LonStep are both positive
SubTile->pNorthWest.Lon = SuperTile->Westest() - (SubTile->pLonStep * CellOffsetX);
SubTile->pNorthWest.Lat = SuperTile->Northest() - (SubTile->pLatStep * CellOffsetY);


//SubTile->pSouthEast.Lon = SuperTile->Eastest() + (SubTile->pLonStep * (SuperTile->pLonEntries - (CellOffsetX + SubTile->pLonEntries)));
//SubTile->pSouthEast.Lat = SuperTile->Southest() + (SubTile->pLatStep * (SuperTile->pLatEntries - (CellOffsetY + SubTile->pLatEntries)));
SubTile->pSouthEast.Lon = SuperTile->Westest() - (SubTile->pLonStep * (CellOffsetX + SubTile->pLonEntries - 1));
SubTile->pSouthEast.Lat = SuperTile->Northest() - (SubTile->pLatStep * (CellOffsetY + SubTile->pLatEntries - 1));
} // CopySubTileHeader


bool CopySubTile(DEM *SuperTile, DEM *SubTile, unsigned long int CellOffsetX, unsigned long int CellOffsetY, unsigned long int CellsWide, unsigned long int CellsHigh)
{
CopySubTileHeader(SuperTile, SubTile, CellOffsetX, CellOffsetY, CellsWide, CellsHigh);


if(MasterScene.GetTerrainIsPageable())
	{ // hand off creation of the subtile directly to the paging loader
	if(LoadDEMTileViaECW(MasterScene.GetDEMName(), SubTile, MasterScene.GetTerrainMinElev(), MasterScene.GetTerrainMaxElev(),
	 CellOffsetX, CellOffsetX + CellsWide, CellOffsetY, CellOffsetY + CellsHigh))
		{
		return(true);
		} // if

/*
	// Allocate new storage in SubTile
	if(SubTile->AllocRawMap())
		{
		// copy entries over from SuperTile to SubTile
		for(unsigned long int YLoop = 0; YLoop < SubTile->pLatEntries; YLoop++)
			{
			for(unsigned long int XLoop = 0; XLoop < SubTile->pLonEntries; XLoop++)
				{
				// it doesn't really matter internally if we're counting up/down/left/right in our X and Y loops
				// because both of these accessor methods use the same connotation/sense in their accessor APIs
				// thought the X and Y params are reversed in order
				// 0,0 is lower left in both APIs, so we invert Y through subtraction
				SubTile->StoreElevation((SubTile->pLatEntries - 1) - YLoop, XLoop, 2000);
				} // for
			} // for
		} // if
*/

	} // if
else
	{
	// Allocate new storage in SubTile
	if(SubTile->AllocRawMap())
		{
		// copy entries over from SuperTile to SubTile
		for(unsigned long int YLoop = 0; YLoop < SubTile->pLatEntries; YLoop++)
			{
			for(unsigned long int XLoop = 0; XLoop < SubTile->pLonEntries; XLoop++)
				{
				// it doesn't really matter internally if we're counting up/down/left/right in our X and Y loops
				// because both of these accessor methods use the same connotation/sense in their accessor APIs
				// thought the X and Y params are reversed in order
				// 0,0 is lower left in both APIs, so we invert Y through subtraction
				SubTile->StoreElevation((SubTile->pLatEntries - 1) - YLoop, XLoop, SuperTile->Sample(XLoop + CellOffsetX, (SuperTile->pLatEntries - 1) - (YLoop + CellOffsetY)));
				} // for
			} // for

		return(true); // success!
		} // if
	} // if

return(false);
} // CopySubTile


// if you don't pass a DEM, it will try to use the one in the TerrainTile
void TerrainTile::UpdateEdgeExtrema(DEM *TerrainSource)
{

if(TerrainSource == NULL)
	{
	TerrainSource = TerrainFragment;
	} // if

ClearEdgeExtrema(); // set all mins to FLT_MAX

if(TerrainSource && TerrainSource->pLatEntries && TerrainSource->pLonEntries)
	{
	unsigned long int YLoop, XLoop, InputCell;
	// walk around edge of DEM, recording lowest elevation on edge
	// West
	for(YLoop = 0, InputCell = 0; YLoop < TerrainSource->pLatEntries; YLoop++, InputCell ++)
		{
		if (! TerrainSource->TestNullReject(InputCell))
			{
			WMin = min(WMin, TerrainSource->Sample(0, YLoop));
			WMax = max(WMax, TerrainSource->Sample(0, YLoop));
			} // if
		} // for
	// South
	for(XLoop = 0, InputCell = 0; XLoop < TerrainSource->pLonEntries; XLoop++, InputCell += TerrainSource->LatEntries())
		{
		if (! TerrainSource->TestNullReject(InputCell))
			{
			SMin = min(SMin, TerrainSource->Sample(XLoop, 0));
			SMax = max(SMax, TerrainSource->Sample(XLoop, 0));
			} // if
		} // for
	// East
	for(YLoop = 0, InputCell = (TerrainSource->LonEntries() - 1) * TerrainSource->LatEntries(); YLoop < TerrainSource->pLatEntries; YLoop++, InputCell ++)
		{
		if (! TerrainSource->TestNullReject(InputCell))
			{
			EMin = min(EMin, TerrainSource->Sample(TerrainSource->pLonEntries - 1, YLoop));
			EMax = max(EMax, TerrainSource->Sample(TerrainSource->pLonEntries - 1, YLoop));
			} // if
		} // for
	// North
	for(XLoop = 0, InputCell = TerrainSource->LatEntries() - 1; XLoop < TerrainSource->pLonEntries; XLoop++, InputCell += TerrainSource->LatEntries())
		{
		if (! TerrainSource->TestNullReject(InputCell))
			{
			NMin = min(NMin, TerrainSource->Sample(XLoop, TerrainSource->pLatEntries - 1));
			NMax = max(NMax, TerrainSource->Sample(XLoop, TerrainSource->pLatEntries - 1));
			} // if
		} // for
	
	} // if

} // TerrainTile::UpdateEdgeExtrema

//char GlobalDebugBlitDumpName[128];

osg::ref_ptr<osg::Image> TerrainTile::CreateTileTextureImage(TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor)
{ // figure out the proper subset, create it, and initialize the TerrainTexCoordCalc object for use
osg::ref_ptr<osg::Image> Result, Overlay;

ImageTileBounds SubTileBounds;

GetTerrainManager()->CalculateOptimalTileTextureWindow(GetTileNumX(), GetTileNumY(), SubTileBounds, CoordsHelper, DownsampleFactor);

#ifdef HJW
bool ServedImageOK = false;

if(DownsampleFactor < 4) // don't waste time generating overlays that won't be legible anyway
	{
	double dLat, dLon;	// deltas
	double ppd;	// pixels per degree
	// request tile from server
	//static unsigned long imgNum = 0;
	unsigned long adjHeight;
	//unsigned long ccWidth;	// cosine corrected width
	//unsigned long ccHeight;	// cosine corrected width
	struct boxCoords box;
	//char imgName[80];
	//bool logIt = false;

	//GetTempPath
	box.Lat1 = SubTileBounds.SLat;
	box.Lon1 = -SubTileBounds.WLon;
	box.Lat2 = SubTileBounds.NLat;
	box.Lon2 = -SubTileBounds.ELon;
	dLat = box.Lat2 - box.Lat1;
	dLon = box.Lon2 - box.Lon1;
	ppd = SubTileBounds.Width / dLon;
	//adjHeight = (unsigned long)(ppd * (box.Lat2 - box.Lat1) + 0.5);
	/***
	 Telcontar expects us to ask for an image with extents of dLon and dLat and Width and Height, where dLon is cosine corrected
	 For NV, we adjust the Height to compensate for the uncorrected dLon
	 ***/
	adjHeight = (unsigned long)((dLat / cos((box.Lat1 + box.Lat2) * PI / 360.0)) / dLon * SubTileBounds.Height + 0.5);
	//if (SubTileBounds.Width == 512)
	//	logIt = true;
	//ccWidth = (unsigned long)(SubTileBounds.Width * cos((box.Lat1 + box.Lat2) * PI / 360.0) + 0.5);
	//ccHeight = (unsigned long)(SubTileBounds.Height * cos((box.Lat1 + box.Lat2) * PI / 360.0) + 0.5);
	getTelcontarMap(SubTileBounds.Width, adjHeight, box, "C:/ReturnedMap.png");
	//getServerMap("http://onearth.jpl.nasa.gov/wms.cgi", "global_mosaic", "Pan", "image/jpeg", "application/vnd.ogc.se_inimage&", SubTileBounds.Width, SubTileBounds.Height, box, "C:/ReturnedMap.jpg");
	//getServerMapGet("http://terraservice.net/ogcmap.ashx", "DOQ", "", "image/jpeg", "se_xml", SubTileBounds.Width, SubTileBounds.Height, box, "C:/ReturnedMap.jpg");
	//getServerMapGet("http://terraservice.net/ogcmap.ashx", "UrbanArea", "", "image/jpeg", "se_xml", SubTileBounds.Width, SubTileBounds.Height, box, "C:/ReturnedMap.jpg");
	//getServerMapGetUSGS_IMS("http://ims.cr.usgs.gov/servlet/com.esri.wms.Esrimap/USGS_EDC_Ortho_Urban", "Spring_2002_0.3m_Color_Denver_01", "", "image/jpeg", "", SubTileBounds.Width, SubTileBounds.Height, box, "C:/ReturnedMap.jpg");
	//getServerMapGetUSGS_IMS("http://ims.cr.usgs.gov/servlet/com.esri.wms.Esrimap/USGS_EDC_Ortho_Urban", "Spring_2002_0.3m_Color_Denver_01", "", "image/jpeg", "", SubTileBounds.Width, SubTileBounds.Height, box, "C:/ReturnedMap.jpg");
	//if (SubTileBounds.Width == 512)
	//	{
	//	FILE *logFile;

	//	sprintf(imgName, "ccmImg_%dx_%dy.jpg", GetTileNumX(), GetTileNumY());

	//	logFile = fopen("C:/Bounds.log", "a+");
	//	fprintf(logFile, "%s\n", imgName);
	//	fclose(logFile);
	//	} // if
	//else
	//	sprintf(imgName, "C:/ReturnedMap.jpg");
	//getServerMapGetUSGS_IMS("http://ims.cr.usgs.gov/servlet/com.esri.wms.Esrimap/USGS_EDC_Ortho_Urban", "Spring_2002_0.3m_Color_Denver_01", "", "image/jpeg", "", SubTileBounds.Width, adjHeight, box, imgName, logIt);
	
	// load tile
	Overlay = new osg::Image;
	if (Overlay.valid())
		{
		SubTileBounds.Height = adjHeight;
		SubTileBounds.SubstantialHeightDS = adjHeight;
		SubTileBounds.SubstantialOffsetY = 0;
		Overlay->allocateImage(SubTileBounds.Width, SubTileBounds.Height, 1, GL_RGB, GL_UNSIGNED_BYTE); 
		Overlay = osgDB::readImageFile("C:/ReturnedMap.png");
		} // if

	//Result = osgDB::readImageFile("C:/ReturnedMap.png");
	//Result = osgDB::readImageFile("C:/ReturnedMap.jpg");
	
	// verify tile suitable
	//if(Result) // && Result->s() == SubTileBounds.Width && Result->t() == SubTileBounds.Height)
	//	{
	//	ServedImageOK = true;
	//	} // if
	//else
	//	{
	//	return(NULL); // if we want to fail out completely instead of using base layer
	//	} // else
	} // if
if(!ServedImageOK)
#endif // HJW
	{
	// allocate image and Image-data storage
	Result = new osg::Image;
	if(Result.valid())
		{
		// sanity checking for paging very small files
		//if(SubTileBounds.SubstantialWidthDS > (SubTileBounds.SubstantialWidth - 2)) SubTileBounds.SubstantialWidthDS = (SubTileBounds.SubstantialWidth - 2);
		//if(SubTileBounds.SubstantialHeightDS > (SubTileBounds.SubstantialHeight - 2)) SubTileBounds.SubstantialHeightDS = (SubTileBounds.SubstantialHeight - 2);
		Result->allocateImage(SubTileBounds.Width, SubTileBounds.Height, 1, GL_RGB, GL_UNSIGNED_BYTE); 
		memset(Result->data(), 0, Result->getImageSizeInBytes()); // ensure no uninited data
		// copy subsection rectangle over
		// DEBUG
		//sprintf(GlobalDebugBlitDumpName, "c:/BlitDump%d_%d.raw", GetTileNumX(), GetTileNumY());
		if(MasterScene.GetImageIsPageable())
			{ // paged imagery
			LoadImageTileViaECW(MasterScene.GetDrapeImageNameA(), Result.get(),
			SubTileBounds.SubstantialOriX, 
			SubTileBounds.SubstantialOriX + (SubTileBounds.SubstantialWidth - 1),
			SubTileBounds.SubstantialOriY,
			SubTileBounds.SubstantialOriY + (SubTileBounds.SubstantialHeight - 1),
			SubTileBounds.SubstantialOffsetX,
			SubTileBounds.SubstantialOffsetY,
			SubTileBounds.SubstantialWidthDS,
			SubTileBounds.SubstantialHeightDS);
			} // if
		else
			{ // not paged
			OSGImageBlit(TerrainDrapeImage.get(), // source image
			SubTileBounds.SubstantialOriX, ((GetTerrainManager()->DrapeImageHeight - 1) - (SubTileBounds.SubstantialOriY + SubTileBounds.SubstantialHeight - 1)), // source X, Y
			SubTileBounds.SubstantialWidth, SubTileBounds.SubstantialHeight, // width, height
			Result.get(), // destination image
			SubTileBounds.SubstantialOffsetX, ((SubTileBounds.Height - 1) - (SubTileBounds.SubstantialOffsetY + SubTileBounds.SubstantialHeight - 1))); // dest X, Y
			} // else
		} // if
	} // else (!ServedImageOK)

#ifdef HJW
if (Result.valid() && Overlay.valid())
	{
	unsigned long int pX, pY;

	for (pY = 0; pY < SubTileBounds.Height; pY++)
		{
		for (pX = 0; pX < SubTileBounds.Width; pX++)
			{
			bool changed = true;
			unsigned char rR, rG, rB, rA, oR, oG, oB, oA;	// result & overlay RGBA
			unsigned char cR, cG, cB, cA;					// composite RGBA

			OSGImageReadPixel(Result.get(), pX, pY, rR, rG, rB, rA);
			OSGImageReadPixel(Overlay.get(), pX, pY, oR, oG, oB, oA);
			cA = rA;
			//if ((oR > 230) && (oR < 245) && (oG > 230) && (oG < 240) && (oB > 220)&& (oB < 230))	// tan check
			if ((oR == 240) && (oG == 234) && (oB == 225))	// tan check (Photoshop reports 239/233/224)
				{ // use Result, not overlay
				// 100% transparent (leave Result as is)
				changed = false;
				} // if tan
			//else if ((oR > 150) && (oR < 160) && (oG > 200) && (oG < 210) && (oB > 150)&& (oB < 160))
			else if ((oR == 156) && (oG == 206) && (oB == 156)) // green check (Photoshop reports 155/205/155)
				{
				// 50% transparent
				cR = (unsigned char)(rR * 0.5f + oR * 0.5f);
				cG = (unsigned char)(rG * 0.5f + oG * 0.5f);
				cB = (unsigned char)(rB * 0.5f + oG * 0.5f);
				} // if green
			else	// use overlay
				{
				cR = oR;
				cG = oG;
				cB = oB;
				} // else
			if (changed)
				OSGImageWritePixel(Result.get(), pX, pY, cR, cG, cB, cA);
			} // for pX
		} // for pY
	} // if Overlay
#endif // HJW

return(Result);
} // TerrainTile::CreateTileTextureImage




osg::ref_ptr<osg::StateSet> TerrainTile::CreateTileTextureStateSet(TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor)
{
osg::ref_ptr<osg::StateSet> Result;

Result = new osg::StateSet;
if(Result.valid())
	{
	osg::ref_ptr<osg::Texture2D> TerrainTex;
	osg::ref_ptr<osg::Image> TextureImage;

	// Initialize the base terrain StateSet
	Result->setGlobalDefaults();

	// turn off lighting
	Result->setMode( GL_LIGHTING, osg::StateAttribute::OFF );

	// set up non-foliage texture
	TerrainTex = new osg::Texture2D;
	TextureImage = CreateTileTextureImage(CoordsHelper, DownsampleFactor);
	if(!(TerrainTex.valid() && TextureImage.valid()))
		{
		return(NULL);
		} // if
	TerrainTex->setImage(TextureImage.get());
	// wrap/clamp, filtering, compression
	TerrainTex->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP_TO_EDGE ); // Falls back to CLAMP if unavailable
	TerrainTex->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP_TO_EDGE );
	TerrainTex->setFilter( osg::Texture2D::MIN_FILTER, MasterScene.SceneLOD.GetMipmapMode() );
	TerrainTex->setFilter( osg::Texture2D::MAG_FILTER, osg::Texture2D::LINEAR);
	if(MasterScene.SceneLOD.GetMipmapMode() == osg::Texture2D::NEAREST || MasterScene.SceneLOD.GetMipmapMode() == osg::Texture2D::LINEAR)
		{
		TerrainTex->setNumMipmapLevels(0); // don't need 'em, spare the texture memory
		} // if
	if(MasterScene.SceneLOD.CheckCompressTerrainTex())
		{
		TerrainTex->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
		} // if

	Result->setTextureAttributeAndModes( 0, TerrainTex.get(), osg::StateAttribute::ON );
	} // if

return(Result);
} // TerrainTile::CreateTileTextureStateSet



osg::ref_ptr<osg::Geode> TerrainTile::BuildLODGeometry(bool FolLOD, unsigned long int DownsampleFactor, LODSharedData *SharedData)
{
osg::ref_ptr<osg::Geode> Result;
DEM *DownsampledDEM = NULL;
DEM *TerrainLoaded = NULL, *TerrainSource = NULL, *VersionToGenerateFrom = NULL, *VersionToFlapFrom = NULL, *FlappedDEM = NULL;

double CSTTime = 0.0, CDSTime = 0.0, CFDTime = 0.0, GDTTime = 0.0;
double TotalTimeSeconds;
Timer_t Start, End; // <<<>>> TIMER
double TotalThreadStart = 0.0, TotalThreadEnd = 0.0, TotalThreadElapse = 0.0;
osg::Timer Stopwatch;

// clearing to NULLs before writing should make it somewhat less thread-dangerous
//char LocalDebugMsg[100];
//memset(ThreadDebugMsg, 0, 160);
//sprintf(LocalDebugMsg, "Building %03d,%03d:%d", GetTileNumX(), GetTileNumY(), DownsampleFactor);
//strcpy(ThreadDebugMsg, LocalDebugMsg);
TotalThreadStart = GetProcessTimeFP();

// never delete TerrainSource, VersionToGenerateFrom or VersionToFlapFrom objects, as they're just pointers to either DownsampledDEM or FlappedDEM

if(TerrainFragment)
	{ // already provided
	TerrainSource = TerrainFragment;
	} // if
else
	{ // need to fetch it ourselves
	Start = Stopwatch.tick();
	TerrainLoaded = GetTerrainManager()->CreateSubTile(GetTileNumX(), GetTileNumY());
	End = Stopwatch.tick();
	CSTTime = Stopwatch.delta_s(Start, End);
	if(!TerrainLoaded /* || CurrentTile->ContainsAllNulls() */) // discard failed tiles, <<<>>> or useless tiles
		{
		delete TerrainLoaded;
		TerrainLoaded = NULL;
		} // if
	else
		{
		TerrainSource = TerrainLoaded;
		} // else
	} // else

if(TerrainSource)
	{
	UpdateEdgeExtrema(TerrainSource); // to ensure flaps have necessary max/min values available
	if(DownsampleFactor > 1) // 0  or 1 = no decimation
		{
		Start = Stopwatch.tick();
		if(DownsampledDEM = CreateDownSampled(TerrainSource, DownsampleFactor))
			{
			VersionToFlapFrom = DownsampledDEM;
			} // if
		else
			{ // can't make something that downsampled, return an empty stub so we don't keep trying to load it
			Result = new osg::Geode;
			} // else
		End = Stopwatch.tick();
		CDSTime = Stopwatch.delta_s(Start, End);;
		} // if
	else
		{ // no downsample
		VersionToFlapFrom = TerrainSource;
		} // else

	if(VersionToFlapFrom)
		{
		// generated a flapped version
		Start = Stopwatch.tick();
		if(VersionToGenerateFrom = CreateFlappedDEM(VersionToFlapFrom, this))
			{
			// should be ready to go with no further work...
			} // if
		else
			{
			// failed, fall back to using unflapped version of the DEM (better than nothing)
			VersionToGenerateFrom = VersionToFlapFrom;
			} // else
		End = Stopwatch.tick();
		CFDTime = Stopwatch.delta_s(Start, End);;

		Start = Stopwatch.tick();
		Result = GenerateDEMTile(VersionToGenerateFrom, FolLOD, this, DownsampleFactor, SharedData);
		End = Stopwatch.tick();
		GDTTime = Stopwatch.delta_s(Start, End);;
		} // if
	} // if

TotalThreadEnd = GetProcessTimeFP();
TotalThreadElapse = TotalThreadEnd - TotalThreadStart;
TotalTimeSeconds = CSTTime+CDSTime+CFDTime+GDTTime;

//char ThreadMsg[256];
//sprintf(ThreadMsg, "Built %03d,%03d:%d\nCST:%f CDS:%f\nCFD:%f GDT:%f\nTOT:%f TTHR:%f", GetTileNumX(), GetTileNumY(), DownsampleFactor, CSTTime, CDSTime, CFDTime, GDTTime, TotalTimeSeconds, TotalThreadElapse);
//sprintf(ThreadMsg, "LOAD:%f", TotalTimeSeconds);

// clearing to NULLs before writing should make it somewhat less thread-dangerous
//memset(ThreadDebugMsg, 0, 160); // <<<>>> TIMER
//strncpy(ThreadDebugMsg, ThreadMsg, 159);

// no need to check for NULL
delete FlappedDEM;
FlappedDEM = NULL;
delete DownsampledDEM;
DownsampledDEM = NULL;
delete TerrainLoaded;
TerrainLoaded = NULL;

return(Result);
} // TerrainTile::BuildLODGeometry



double TerrainManager::CalcXCoordFromCell(unsigned long int CellX)
{
return(LocalTerrainModel->Westest() - (CellX * LocalTerrainModel->LonStep()));
} // TerrainManager::CalcXCoordFromCell

double TerrainManager::CalcYCoordFromCell(unsigned long int CellY)
{
return(LocalTerrainModel->Northest() - (CellY * LocalTerrainModel->LatStep()));
} // TerrainManager::CalcYCoordFromCell

bool TerrainManager::CalculateOptimalTileTextureWindow(unsigned long int TileX, unsigned long int TileY, ImageTileBounds &TileBounds, TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor)
{
// X/Y Min/Max are integer pixel coordinates with origin at UL, increasing to LR
signed long int XMin, YMin, XMax, YMax; 
unsigned long int XDim, YDim;
double XFrac, YFrac;
double XWGeo, XEGeo, YSGeo, YNGeo;

TerrainTexCoordCalc CoordsGenerator(LocalTerrainModel->Eastest(), LocalTerrainModel->Westest(), LocalTerrainModel->Southest(), LocalTerrainModel->Northest());

XFrac = CalcTileOffsetX(TileX) / (LocalTerrainModel->LonEntries() - 1.0);
XWGeo = CoordsGenerator.GetGeoXCoord(XFrac);
XMin = (signed long int)(XFrac * (DrapeImageWidth - 1)); // 0...Lon-1 DEM Cells -> 0...1 scalar (not OGL -- UL Origin) ->  0...Width-1 pixel subscripts

XFrac = (CalcTileOffsetX(TileX) + CalcTileWidth(TileX) - 1) / (LocalTerrainModel->LonEntries() - 1.0);
XEGeo = CoordsGenerator.GetGeoXCoord(XFrac);
XMax = (signed long int)((XFrac * (DrapeImageWidth - 1)) + .5); // 0...Lon-1 DEM Cells -> 0...1 scalar (not OGL -- UL Origin) ->  0...Width-1 pixel subscripts (with round up)

YFrac = CalcTileOffsetY(TileY) / (LocalTerrainModel->LatEntries() - 1.0);
YNGeo = CoordsGenerator.GetGeoYCoord(YFrac);
YMin = (signed long int)(YFrac * (DrapeImageHeight - 1)); // 0...Lat-1 DEM Cells -> 0...1 scalar (not OGL -- UL Origin) ->  0...Height-1 pixel subscripts

YFrac = (CalcTileOffsetY(TileY) + CalcTileHeight(TileY) - 1) / (LocalTerrainModel->LatEntries() - 1.0);
YSGeo = CoordsGenerator.GetGeoYCoord(YFrac);
YMax = (signed long int)((YFrac * (DrapeImageHeight - 1)) + .5); // 0...Lat-1 DEM Cells -> 0...1 scalar (not OGL -- UL Origin) ->  0...Height-1 pixel subscripts (with round up)

XDim = (XMax - XMin) + 1;
YDim = (YMax - YMin) + 1;

if(DownsampleFactor != 0 && MasterScene.GetImageIsPageable()) // we can efficiently generate lower image LODs, so use DownsampleFactor to do so
	{
	XDim /= DownsampleFactor;
	YDim /= DownsampleFactor;
	
	// don't downsample too ridiculously far
	if(XDim < 2) XDim = 2;
	if(YDim < 2) YDim = 2;
	} // if
else
	{
	DownsampleFactor = 1; // so as not to mess up scaling calculations below
	} // else

TileBounds.Width = RoundUpPowTwo(XDim); //TileBounds.Width = min(RoundUpPowTwo(XDim), GetMaxTerrainTextureSize());
TileBounds.Height = RoundUpPowTwo(YDim); //TileBounds.Height = min(RoundUpPowTwo(YDim), GetMaxTerrainTextureSize());

// Now, center the useful window within the power-of-two texture image block
// so that the overblit padding can be shared with all edges/flaps
unsigned long int XDif, HalfXDif, YDif, HalfYDif;
XDif = TileBounds.Width - XDim; // possibly in downsampled pixel units
YDif = TileBounds.Height - YDim; // possibly in downsampled pixel units
HalfXDif = XDif >> 1; // (divide by 2) possibly in downsampled pixel units 
HalfYDif = YDif >> 1; // (divide by 2) possibly in downsampled pixel units 
TileBounds.OriX = XMin - (HalfXDif * DownsampleFactor); // (in full-res pixel units) could go negative
TileBounds.OriY = YMin - (HalfYDif * DownsampleFactor); // (in full-res pixel units) could go negative
TileBounds.SubstantialOffsetX = HalfXDif; // possibly in downsampled pixel units
TileBounds.SubstantialOffsetY = HalfYDif; // possibly in downsampled pixel units
TileBounds.SubstantialOriX = XMin; // (in full-res pixel units)
TileBounds.SubstantialOriY = YMin; // (in full-res pixel units)
TileBounds.SubstantialWidthDS = XDim; // possibly in downsampled pixel units
TileBounds.SubstantialHeightDS = YDim; // possibly in downsampled pixel units
TileBounds.SubstantialWidth = XDim * DownsampleFactor; // (in full-res pixel units)
TileBounds.SubstantialHeight = YDim * DownsampleFactor; // (in full-res pixel units)


double DE, DW, DS, DN;

DE = CoordsGenerator.GetGeoXCoord((float)(TileBounds.OriX + (signed long int)(TileBounds.Width * DownsampleFactor) - 1) / (float)(DrapeImageWidth - 1)); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +
DW = CoordsGenerator.GetGeoXCoord((float)(TileBounds.OriX) / (float)(DrapeImageWidth - 1)); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +
DS = CoordsGenerator.GetGeoYCoord(1.0f - ((float)(TileBounds.OriY + (signed long int)(TileBounds.Height * DownsampleFactor) - 1) / (float)(DrapeImageHeight - 1))); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat
DN = CoordsGenerator.GetGeoYCoord(1.0f - ((float)TileBounds.OriY / (float)(DrapeImageHeight - 1))); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat

// these _should_ be the geospatial coordinates of the center of the corner pixels of the
// padding-expanded texture map image
TileBounds.SLat = DS;
TileBounds.WLon = DW;
TileBounds.NLat = DN;
TileBounds.ELon = DE;

// for debugging only
/*
XFrac = (float)(TileBounds.SubstantialOriX + (signed long int)TileBounds.SubstantialWidth - 1) / (float)(DrapeImageWidth - 1);
DE = CoordsGenerator.GetGeoXCoord(XFrac); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +
XFrac = (float)(TileBounds.SubstantialOriX) / (float)(DrapeImageWidth - 1);
DW = CoordsGenerator.GetGeoXCoord(XFrac); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +

YFrac = 1.0f - ((float)(TileBounds.SubstantialOffsetY + (signed long int)TileBounds.SubstantialWidth - 1) / (float)(DrapeImageHeight - 1));
DS = CoordsGenerator.GetGeoYCoord(YFrac); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat
YFrac = 1.0f - ((float)TileBounds.SubstantialOffsetY / (float)(DrapeImageHeight - 1));
DN = CoordsGenerator.GetGeoYCoord(YFrac); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat




XFrac = (float)(TileBounds.OriX + (signed long int)TileBounds.Width - 1) / (float)(DrapeImageWidth - 1);
DE = CoordsGenerator.GetGeoXCoord(XFrac); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +
XFrac = (float)(TileBounds.OriX) / (float)(DrapeImageWidth - 1);
DW = CoordsGenerator.GetGeoXCoord(XFrac); // 0...Width-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial W Lon +
YFrac = 1.0f - ((float)(TileBounds.OriY + (signed long int)TileBounds.Height - 1) / (float)(DrapeImageHeight - 1));
DS = CoordsGenerator.GetGeoYCoord(YFrac); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat
YFrac = 1.0f - ((float)TileBounds.OriY / (float)(DrapeImageHeight - 1));
DN = CoordsGenerator.GetGeoYCoord(YFrac); // 0...Height-1 pixel subscript -> 0...1 scalar (OGL UV LL origin) -> WCS Geospatial Lat
*/
CoordsHelper.SetupCoords(DE,  DW,  DS,  DN);

// pad out texture tiles with real data, if possible
// this doesn't expand window size, only fills in area we already had due to power-of-two roundup

signed long int PadCoord, PadExpansion;

// expand to left
PadCoord = TileBounds.SubstantialOriX - (HalfXDif  * DownsampleFactor); // (in full-res pixel units)
if(PadCoord < 0)
	PadCoord = 0;
PadExpansion = TileBounds.SubstantialOriX - PadCoord; // (in full-res pixel units)
TileBounds.SubstantialOriX = PadCoord; // (in full-res pixel units)
TileBounds.SubstantialOffsetX -= (PadExpansion / DownsampleFactor); // possibly in downsampled pixel units
TileBounds.SubstantialWidth += PadExpansion; // (in full-res pixel units)
TileBounds.SubstantialWidthDS += PadExpansion / DownsampleFactor; // possibly in downsampled pixel units

// expand to top
PadCoord = TileBounds.SubstantialOriY - (HalfYDif  * DownsampleFactor); // (in full-res pixel units)
if(PadCoord < 0)
	PadCoord = 0;
PadExpansion = TileBounds.SubstantialOriY - PadCoord; // (in full-res pixel units)
TileBounds.SubstantialOriY = PadCoord; // (in full-res pixel units)
TileBounds.SubstantialOffsetY -= (PadExpansion / DownsampleFactor); // possibly in downsampled pixel units
TileBounds.SubstantialHeight += PadExpansion; // (in full-res pixel units)
TileBounds.SubstantialHeightDS += PadExpansion / DownsampleFactor; // possibly in downsampled pixel units


// expand to right
PadCoord = TileBounds.SubstantialOriX + (TileBounds.SubstantialWidth - 1) + ((XDif - HalfXDif) * DownsampleFactor); // (in full-res pixel units)
if(PadCoord > (signed)(DrapeImageWidth - 1))
	PadCoord = (signed)(DrapeImageWidth - 1);
PadExpansion = PadCoord - (TileBounds.SubstantialOriX + (TileBounds.SubstantialWidth - 1)) ;  // (in full-res pixel units)
TileBounds.SubstantialWidth += PadExpansion;  // (in full-res pixel units)
TileBounds.SubstantialWidthDS += PadExpansion / DownsampleFactor; // possibly in downsampled pixel units


// expand to bottom
PadCoord = TileBounds.SubstantialOriY + (TileBounds.SubstantialHeight - 1) + ((YDif - HalfYDif) * DownsampleFactor); // (in full-res pixel units)
if(PadCoord > (signed)(DrapeImageHeight - 1))
	PadCoord = (signed)(DrapeImageHeight - 1);
PadExpansion = PadCoord - (TileBounds.SubstantialOriY + (TileBounds.SubstantialHeight - 1)) ;  // (in full-res pixel units)
TileBounds.SubstantialHeight += PadExpansion;  // (in full-res pixel units)
TileBounds.SubstantialHeightDS += PadExpansion / DownsampleFactor; // possibly in downsampled pixel units
/*
signed long int TestE, TestW, TestN, TestS;
double TestEd, TestWd, TestNd, TestSd;
double FE, FW, FN, FS;
FE = CoordsGenerator.GetTexXCoord(DE);
FW = CoordsGenerator.GetTexXCoord(DW);
FN = CoordsGenerator.GetTexYCoord(DN);
FS = CoordsGenerator.GetTexYCoord(DS);

TestEd = FE * (DrapeImageWidth - 1.0);
TestWd = FW * (DrapeImageWidth - 1.0);
TestNd = (1.0f - FN) * (DrapeImageHeight - 1.0);
TestSd = (1.0f - FS) * (DrapeImageHeight - 1.0);

TestE = TestEd + 0.5;
TestW = TestWd + 0.5;
TestN = TestNd + 0.5;
TestS = TestSd + 0.5;
*/

return(true);
} // TerrainManager::CalculateOptimalTileTextureWindow




int TerrainManager::GenerateRootFramework(osg::Group *TerrainGroup, double TerrainClipDist)
{
int Success = 0;
double DEMDiag = 0.0, DX, DY, DZ, ScreenSize;
PartialVertexDEM TerrainVertexA, TerrainVertexB;
int TilesCreated = 0;
TerrainTile *CurrentTile;
DEM *TempDEM;
double ElScaleMult;
double TerrainMedianEl, DEMDiagWithElevUp, DEMDiagWithElevDn, MaxRad, PixelRadiusRatio;

// <<<>>> Need to build this stuff beneath a conventional distance-based LOD node to obey terrain distance disappear

for(std::vector< std::vector<TerrainTile *> >::iterator YTrav = TileSet.begin(); YTrav != TileSet.end(); YTrav++)
	{
	for(std::vector<TerrainTile *>::iterator XTrav = (*YTrav).begin(); XTrav != (*YTrav).end(); XTrav++)
		{
		if(CurrentTile = (*XTrav))
			{
			TempDEM = CreateSubTile(CurrentTile->GetTileNumX(), CurrentTile->GetTileNumY(), false);
			
			TerrainMedianEl = (TempDEM->MinEl() + TempDEM->MaxEl()) * .5;

			if(!TempDEM || CurrentTile->ContainsAllNulls()) // discard failed tiles, or useless tiles
				{
				delete CurrentTile;
				*XTrav = NULL;
				delete TempDEM;
				TempDEM = NULL;
				} // if
			else
				{
				// need VertexA for UserCenter (below) even if we aren't calculating Diag radius
				ElScaleMult = (TempDEM->ElScale() / ELSCALE_METERS);
				TerrainVertexA.Lat  = (TempDEM->Northest() + TempDEM->Southest()) * .5; // middle of DEM
				TerrainVertexA.Lon  = (TempDEM->Westest() + TempDEM->Eastest()) * .5; // middle of DEM
				TerrainVertexA.Elev = TerrainMedianEl;  // median elevation
				MasterScene.DegToCart(TerrainVertexA);
				if(DEMDiag == 0.0) // only calculate for first tile (UL)
					{
					// calculate diagonal DEM radius for this tile

					TerrainVertexB.Lat  = TempDEM->Northest(); // UL corner of DEM
					TerrainVertexB.Lon  = TempDEM->Westest(); // UL corner of DEM
					TerrainVertexB.Elev = TerrainMedianEl; // median elevation
					MasterScene.DegToCart(TerrainVertexB);

					DX = TerrainVertexB.XYZ[0] - TerrainVertexA.XYZ[0];
					DY = TerrainVertexB.XYZ[1] - TerrainVertexA.XYZ[1];
					// we ignore Z here because we don't care, but use it below
					DEMDiag = sqrt(DX * DX + DY * DY);
					
					// Now we need to account for radius incorporating maximum vertical variation
					// Reuse Lat/Lon of VertexB, but new elevation value
					TerrainVertexB.Elev = TempDEM->MaxEl(); // max elevation
					MasterScene.DegToCart(TerrainVertexB);

					DX = TerrainVertexB.XYZ[0] - TerrainVertexA.XYZ[0];
					DY = TerrainVertexB.XYZ[1] - TerrainVertexA.XYZ[1];
					DZ =  TerrainVertexB.XYZ[2] - TerrainVertexA.XYZ[2];
					DEMDiagWithElevUp = sqrt(DX * DX + DY * DY + DZ * DZ);

					TerrainVertexB.Elev = TempDEM->MinEl(); // min elevation
					MasterScene.DegToCart(TerrainVertexB);

					DX = TerrainVertexB.XYZ[0] - TerrainVertexA.XYZ[0];
					DY = TerrainVertexB.XYZ[1] - TerrainVertexA.XYZ[1];
					DZ =  TerrainVertexB.XYZ[2] - TerrainVertexA.XYZ[2];
					DEMDiagWithElevDn = sqrt(DX * DX + DY * DY + DZ * DZ);
					
					// determine maximum radius of all of the above
					MaxRad = max(DEMDiagWithElevDn, DEMDiagWithElevUp);
					MaxRad = max(MaxRad, DEMDiag);
					
					// determine ratio between flat radius and maximum radius accounting for elevation
					// to use for scaling screen pixel size, below
					PixelRadiusRatio = MaxRad / DEMDiag;

					} // if
				CurrentTile->DEMLenDiagFlat = DEMDiag;
				CurrentTile->DEMLenDiagMaxElev = MaxRad;
				
				if(!CurrentTile->TileLODNode.valid())
					{
					CurrentTile->TileLODNode = new PagedLOD;
					} // if
				
				if(CurrentTile->TileLODNode.valid())
					{
					// setup user-defined LOD center in case other sub-item children would screw up the centering
					osg::Vec3 UserCenter (TerrainVertexA.XYZ[0], TerrainVertexA.XYZ[1], TerrainMedianEl);
					CurrentTile->TileLODNode->setCenter(UserCenter);

					CurrentTile->TileLODNode->setRangeMode(osg::LOD::PIXEL_SIZE_ON_SCREEN);
					// Set Radius (common to all LODs of a given tile)
					CurrentTile->TileLODNode->setRadius(MaxRad);

					// ScreenSize is in cells, not in actual length
					ScreenSize = sqrt((double)(TempDEM->LatEntries() * TempDEM->LatEntries()) + (double)(TempDEM->LonEntries() * TempDEM->LonEntries()));  // ScreenSize essentially wants diameter
					ScreenSize *= (20.0f * PixelRadiusRatio);
					
					// seems like nodes need to go into the PagedLOD perRangeDataList in order from lowest LOD to highest
					// add child for full LOD (at highest slot -- NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS)
					// min and max are not distances, they are pixel sizes and are therefore somewhat reversed
					std::string TileName = EncodeTileString(CurrentTile->GetTileNumX(), CurrentTile->GetTileNumY(), 0);
					TileName += ".";
					TileName += NVW_NVTERRAIN_PSEUDO_EXT;
					PagedLOD_AddPagedNode(CurrentTile->TileLODNode.get(), NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS, ScreenSize, FLT_MAX, TileName);
					} // if



				for(int LODLoop = 0; LODLoop < NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS; LODLoop++)
					{
					// Add downsampled LOD
					// min and max are not distances, they are pixel sizes and are therefore somewhat reversed
					std::string TileName = EncodeTileString(CurrentTile->GetTileNumX(), CurrentTile->GetTileNumY(), LODLoop + 1);
					TileName += ".";
					TileName += NVW_NVTERRAIN_PSEUDO_EXT;
					if(LODLoop == NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS - 1) // last iteration
						{
						PagedLOD_AddPagedNode(CurrentTile->TileLODNode.get(), NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS - (LODLoop + 1), 0.0f, ScreenSize, TileName);
						} // if
					else
						{
						PagedLOD_AddPagedNode(CurrentTile->TileLODNode.get(), NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS - (LODLoop + 1), ScreenSize * .5f, ScreenSize, TileName);
						} // else
					ScreenSize *= .5f; // halving resolution as we go further into the distance
					} // for
				
				TerrainGroup->addChild(CurrentTile->TileLODNode.get());

				if(TempDEM)
					{
					delete TempDEM;
					TempDEM = NULL;
					} // if
				TilesCreated++;
				} // else

			} // if
		} // for
	} // for


return(Success);
} // TerrainManager::GenerateRootFramework









void TerrainManager::ExpandArrays(unsigned long int W, unsigned long int H)
{
unsigned long int XLoop, YLoop;

TileSet.resize(H);
YLoop = 0;
for(std::vector< std::vector<TerrainTile *> >::iterator YTrav = TileSet.begin(); YTrav != TileSet.end(); YTrav++, YLoop++)
	{
	(*YTrav).resize(W);
	XLoop = 0;
	for(std::vector<TerrainTile *>::iterator XTrav = (*YTrav).begin(); XTrav != (*YTrav).end(); XTrav++, XLoop++)
		{
		if(!(*XTrav))
			{
			*XTrav = new TerrainTile(XLoop, YLoop);
			} // if
		} // for
	} // for

} // TerrainManager::ExpandArrays




bool TerrainManager::DoItAll(osg::Group *TerrainGroup, bool FolLOD, double TerrainClipDist)
{
PartialVertexDEM TerrainVertexB;
CalculateOptimalTiling();
ExpandArrays(TilesWide, TilesHigh);
UpdateEdgeFlags(); // can be done before we know edge extrema

// Push out scene bounds before generating any DEMs, so origin doesn't float around...
MasterScene.ExpandCoords(LocalTerrainModel->Westest(), LocalTerrainModel->Northest());
MasterScene.ExpandCoords(LocalTerrainModel->Eastest(), LocalTerrainModel->Southest());

// elev remains 0 for next quartet
TerrainVertexB.Elev = 0;

// NW corner
TerrainVertexB.Lat  = LocalTerrainModel->Northest();
TerrainVertexB.Lon  = LocalTerrainModel->Westest();
MasterScene.DegToCart(TerrainVertexB);
MasterScene.ExpandXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);
MasterScene.ExpandTerrainXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);

// NE corner
TerrainVertexB.Lat  = LocalTerrainModel->Northest();
TerrainVertexB.Lon  = LocalTerrainModel->Eastest();
MasterScene.DegToCart(TerrainVertexB);
MasterScene.ExpandXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);
MasterScene.ExpandTerrainXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);

// SW corner
TerrainVertexB.Lat  = LocalTerrainModel->Southest();
TerrainVertexB.Lon  = LocalTerrainModel->Westest();
MasterScene.DegToCart(TerrainVertexB);
MasterScene.ExpandXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);
MasterScene.ExpandTerrainXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);

// SE corner
TerrainVertexB.Lat  = LocalTerrainModel->Southest();
TerrainVertexB.Lon  = LocalTerrainModel->Eastest();
MasterScene.DegToCart(TerrainVertexB);
MasterScene.ExpandXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);
MasterScene.ExpandTerrainXY(TerrainVertexB.XYZ[0], TerrainVertexB.XYZ[1]);

// this does the work now
GenerateRootFramework(TerrainGroup, TerrainClipDist);

return(true);
} // TerrainManager::DoItAll



void TerrainManager::CalculateOptimalTiling(void)
{
TilesWide = TilesHigh = 0;

if(LocalTerrainModel)
	{
	TilesWide = LocalTerrainModel->LonEntries() / GetTileSizeXUnderlap();
	if(LocalTerrainModel->LonEntries() % GetTileSizeXUnderlap() == 0)
		{ // even multiple of TILESIZE;
		} // if
	else
		{
		TilesWide ++;
		} // 

	TilesHigh = LocalTerrainModel->LatEntries() / GetTileSizeYUnderlap();
	if(LocalTerrainModel->LatEntries() % GetTileSizeYUnderlap() == 0)
		{ // even multiple of TILESIZE;
		} // if
	else
		{
		TilesHigh ++;
		} // 
	} // if

} // TerrainManager::CalculateOptimalTiling


unsigned long int TerrainManager::CalcTileWidth(unsigned long int TileX)
{
if(TileX == TilesWide - 1) // count from 0
	{
	return(LocalTerrainModel->pLonEntries - (TileX * GetTileSizeXUnderlap()));
	} // if
else
	{
	return(GetTileSizeX());
	} // else
} // TerrainManager::CalcTileWidth

unsigned long int TerrainManager::CalcTileHeight(unsigned long int TileY)
{
if(TileY == TilesHigh - 1) // count from 0
	{
	return(LocalTerrainModel->pLatEntries - (TileY * GetTileSizeYUnderlap()));
	} // if
else
	{
	return(GetTileSizeX());
	} // else
} // TerrainManager::CalcTileHeight


DEM *TerrainManager::CreateSubTile(unsigned long int TileX, unsigned long int TileY, bool CopyData)
{
DEM *SubTile = NULL;

if(LocalTerrainModel && TilesWide != 0 && TilesHigh != 0)
	{
	unsigned long int CellOffsetX, CellOffsetY, CellsWide, CellsHigh;

	if(SubTile = new DEM)
		{
		// calc offset of this tile within main DEM
		CellOffsetX = CalcTileOffsetX(TileX);
		CellOffsetY = CalcTileOffsetY(TileY);

		// calculate dimensions of SubTile
		CellsWide = CalcTileWidth(TileX);
		CellsHigh = CalcTileHeight(TileY);

		if(CopyData)
			{
			if(CopySubTile(LocalTerrainModel, SubTile, CellOffsetX, CellOffsetY, CellsWide, CellsHigh))
				{
				return(SubTile);
				} // if
			} // if
		else
			{
			CopySubTileHeader(LocalTerrainModel, SubTile, CellOffsetX, CellOffsetY, CellsWide, CellsHigh);
			return(SubTile);
			} // else
		} // if
	} // if

return(SubTile);
} // TerrainManager::CreateSubTile



void TerrainManager::UpdateAllEdgeExtrema(void)
{
for(std::vector< std::vector<TerrainTile *> >::iterator YTrav = TileSet.begin(); YTrav != TileSet.end(); YTrav++)
	{
	for(std::vector<TerrainTile *>::iterator XTrav = (*YTrav).begin(); XTrav != (*YTrav).end(); XTrav++)
		{
		(*XTrav)->UpdateEdgeExtrema();
		} // for
	} // for

} // TerrainManager::UpdateAllEdgeExtrema


void TerrainManager::UpdateEdgeFlags(void)
{
int XLoop, YLoop;

YLoop = 0;
for(std::vector< std::vector<TerrainTile *> >::iterator YTrav = TileSet.begin(); YTrav != TileSet.end(); YTrav++, YLoop++)
	{
	XLoop = 0;
	for(std::vector<TerrainTile *>::iterator XTrav = (*YTrav).begin(); XTrav != (*YTrav).end(); XTrav++, XLoop++)
		{
		if(*XTrav)
			{
			(*XTrav)->EdgeFlags = NULL;
			if(XLoop == 0) // are we the left edge?
				{
				// no need for LEFT collar
				} // if
			else
				{
				//if(GetTile(XLoop - 1, YLoop)) // is there a tile to our left?
					{
					(*XTrav)->EdgeFlags |= TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT;
					} // if
				} // else
			if(XLoop == TilesWide - 1) // are we the right edge?
				{
				// no need for RIGHT collar
				} // if
			else
				{
				//if(GetTile(XLoop + 1, YLoop)) // is there a tile to our right?
					{
					(*XTrav)->EdgeFlags |= TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT;
					} // if
				} // else
			if(YLoop == 0) // are we the top edge?
				{
				// no need for TOP collar
				} // if
			else
				{
				//if(GetTile(XLoop, YLoop - 1)) // is there a tile above us?
					{
					(*XTrav)->EdgeFlags |= TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP;
					} // if
				} // else
			if(YLoop == TilesHigh - 1) // are we the bottom edge?
				{
				// no need for BOTTOM collar
				} // if
			else
				{
				//if(GetTile(XLoop, YLoop + 1)) // is there a tile below us?
					{
					(*XTrav)->EdgeFlags |= TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM;
					} // if
				} // else
			} // if
		} // for
	} // for


} // TerrainManager::UpdateEdgeFlags



TerrainTile *TerrainManager::GetTile(unsigned long int X, unsigned long int Y)
{

return(TileSet[Y][X]);

} // TerrainManager::GetTile















int FetchTerrainHeight(void *CoordSysDefinition, double XCoordinate, double YCoordinate, double &ElevResult)
{
DEM *LocalTerrain;
double ElevVal;

if((LocalTerrain = MasterScene.GetTerrain()) && LocalTerrain->Map()) // make sure data is allocated before we try sampling from it
	{
	ElevVal = LocalTerrain->DEMArrayPointExtract(NULL, YCoordinate, XCoordinate);
	if(ElevVal == (double)LocalTerrain->NullValue())
		{
		return(0);
		} // if
	ElevResult = ElevVal;
	return(1);
	} // if
return(0);
} // FetchTerrainHeight



bool CreateTerrainNew(osg::Group *TerrainGroup)
{
double FolClipDist, TerrainClipDist;
DEM *Terrain = NULL;
bool DeleteTerrain = true;
bool FolLOD = false;

// Items in UPPERCASE have one instance per terrain 'tile'
// items in lowercase are shared between all instances.
//
//                              FolStateSet(override down)
//                             /
//                ...      FOLGEODE <-- (optional)
//              /        /           \
// TerrainGroup->LODNODE<             >GEOMETRY->DRAWABLES
//              \        \           /
//                ...      BAREGEODE
//                             \
//                              BareStateSet(override down)

FolClipDist     = MasterScene.SceneLOD.GetFolClip();
TerrainClipDist = MasterScene.SceneLOD.GetTerrainClip();

if(!MasterScene.CheckDrapeImageNameA())
	{
	return(false);
	} // if

if(MasterScene.SceneLOD.CheckFolClip() && MasterScene.CheckFolDrapeImageName())
	{
	FolLOD = true;
	} // if

// Now create the StateSet and initialize it
//TerrainSSet = new osg::StateSet;

// first, validate DEM signature

#ifdef NV_CHECK_SIGNATURES
if(!CheckDependentBinaryFileSignature(MasterScene.GetDEMSig(), MasterScene.GetDEMName()))
	{
	SetGlobalSigInvalid(true);
	return(false);
	} // if

if(!CheckDependentBinaryFileSignature(MasterScene.GetDrapeImageSigA(), MasterScene.GetDrapeImageName()))
	{
	SetGlobalSigInvalid(true);
	return(false);
	} // if

/*
if(MasterScene.CheckDrapeImageNameB()) // Is there an alternate drape image?
	{
	if(!CheckDependentBinaryFileSignature(MasterScene.GetDrapeImageSigB(), MasterScene.GetDrapeImageNameB()))
		{
		SetGlobalSigInvalid(true);
		return(false);
		} // if
	} // if


if(MasterScene.CheckFolDrapeImageName()) // did we get a foliage drape image?
	{
	if(!CheckDependentBinaryFileSignature(MasterScene.GetFolDrapeImageSig(), MasterScene.GetFolDrapeImageName()))
		{
		SetGlobalSigInvalid(true);
		return(false);
		} // if
	} // if

*/

#endif // NV_CHECK_SIGNATURES


// Handle drape image


// Initialize the base terrain StateSet
//TerrainSSet->setGlobalDefaults();
// turn off lighting
//TerrainSSet->setMode( GL_LIGHTING, osg::StateAttribute::OFF );


// Image needs to be in path
char CurDir[1000];
getcwd(CurDir, 1000);
osg::Image *TempThumbnail = NULL;
unsigned long int DrapeImageWidth = 0, DrapeImageHeight = 0;
bool IsJP2 = false;
const char *InName = MasterScene.GetDrapeImageNameA();

if(InName)
	{
	size_t NameLen;
	NameLen = strlen(InName);
	if(NameLen > 5)
		{
		if(!stricmp(&InName[NameLen - 4], ".jp2") || !stricmp(&InName[NameLen - 4], ".ecw"))
			{
			IsJP2 = true;
			} // if
		} // if
	
	if(!IsJP2)
		{
		TempThumbnail = osgDB::readImageFile(MasterScene.GetDrapeImageNameA());

		if(TempThumbnail)
			{
			TerrainDrapeImage = new osg::Image(*TempThumbnail, CopyOp::DEEP_COPY_ALL);

			TempThumbnail->scaleImage(256,256,1); // make thumbnail version for efficiency
			MasterScene.Overlay.SetMapNailImage(TempThumbnail);
			DrapeImageWidth = TerrainDrapeImage->s();
			DrapeImageHeight = TerrainDrapeImage->t();
			} // if
		} // if
	else
		{
		MasterScene.SetImageIsPageable(true);
		LoadImageMetadataViaECW(InName, DrapeImageWidth, DrapeImageHeight); // load via ECW, retreive width & height
		TempThumbnail = new osg::Image;
		if(TempThumbnail)
			{
			unsigned long int ThumbDim = 256;
			if(DrapeImageWidth - 1 < ThumbDim) ThumbDim = DrapeImageWidth - 1;
			if(DrapeImageHeight - 1 < ThumbDim) ThumbDim = DrapeImageHeight - 1;
			TempThumbnail->allocateImage(ThumbDim, ThumbDim, 1, GL_RGB, GL_UNSIGNED_BYTE); // reserve space for it
			LoadImageTileViaECW(MasterScene.GetDrapeImageNameA(), TempThumbnail,
			 0, DrapeImageWidth - 1, 0, DrapeImageHeight - 1, 0, 0, ThumbDim, ThumbDim); // load whole image into 256x256 thumbnail
			MasterScene.Overlay.SetMapNailImage(TempThumbnail);
			} // if
		} // else
	} // if

if(MasterScene.GetTerrain()) delete MasterScene.GetTerrain();
MasterScene.SetTerrain(NULL);

DEM *FullTerrain;

if(Terrain = new DEM)
	{
	if(1)
		{
		if(LoadDEMWrapper(MasterScene.GetDEMName(), Terrain))
			{
			// validate that we have some cells and that bound coords appear to be geographic
			if(Terrain->LonEntries() && Terrain->LatEntries() && fabs(Terrain->Southest()) <= 90.0 && fabs(Terrain->Northest()) <= 90.0 && fabs(Terrain->Westest()) <= 360.0 && fabs(Terrain->Eastest()) <= 360.0)
				{
				FullTerrain = Terrain;
				// tiled
				GetTerrainManager()->ConfigureInputTerrain(FullTerrain);
				GetTerrainManager()->SetDrapeImageImageDims(DrapeImageWidth, DrapeImageHeight);
				GetTerrainManager()->DoItAll(TerrainGroup, FolLOD, TerrainClipDist);

				MasterScene.SetTerrain(FullTerrain);
				if(FullTerrain == Terrain) // is the terrain-following DEM the same as the 'temporary' DEM we just built?
					{
					DeleteTerrain = false;
					} // if
				} // if
			} // if
		} // if
	if(DeleteTerrain)
		{
		delete Terrain;
		Terrain = NULL;
		} // if
	} // if

return(true);

} // CreateTerrainNew


osg::ref_ptr<osg::Geode> TerrainTile::GenerateDEMTile(DEM *Terrain, bool FolLOD, TerrainTile *Tile, unsigned long int DownsampleFactor, LODSharedData *SharedData)
{
DEM *FullTerrain;
double CurLat, CurLon;
unsigned long int NextBaseVtx, LonCt, LatCt, BaseVtx, Ct;
Point3f TexCoord;
PartialVertexDEM TerrainVertex;
double ElScaleMult;

FullTerrain = GetTerrainManager()->GetLocalTerrainModel();

ElScaleMult = (Terrain->ElScale() / ELSCALE_METERS);

osg::ref_ptr<osg::Geometry> geometry;
osg::ref_ptr<osg::Geode> BareGeode;
TerrainTexCoordCalc CoordsGenerator;

osg::ref_ptr<osg::Vec3Array> coords   = new osg::Vec3Array;
osg::ref_ptr<osg::Vec4Array> color    = new osg::Vec4Array;
osg::ref_ptr<osg::Vec2Array> texcoord = new osg::Vec2Array;

geometry = new osg::Geometry;
geometry->setDataVariance(Object::STATIC);

BareGeode = new osg::Geode;
BareGeode->setDataVariance(Object::STATIC);
BareGeode->addDrawable(geometry.get()); // I think we can share geometry between both Geodes
BareGeode->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_TERRAIN);

// the new tiled texture system plugs in right here...
osg::ref_ptr<osg::StateSet> ExistingTexStateSet;
osg::ref_ptr<osg::StateSet> NewTexStateSet;

if(SharedData)
	{
	ExistingTexStateSet = SharedData->GetSharedTerrainTexData();
	} // if

if(!ExistingTexStateSet)
	{
	NewTexStateSet = CreateTileTextureStateSet(CoordsGenerator, DownsampleFactor);
	} // if
else
	{
	if(MasterScene.GetImageIsPageable())
		{ // We need to try loading the proper resolution for paging
		NewTexStateSet = CreateTileTextureStateSet(CoordsGenerator, DownsampleFactor);
		if(!NewTexStateSet.valid()) // did we fail to get a texture?
			{ // try to reuse existing texture
			NewTexStateSet = ExistingTexStateSet;
			} // if
		} // if
	else
		{
		NewTexStateSet = ExistingTexStateSet;
		} // else
	// still need to set up coords mapping object that we didn't cache
	ImageTileBounds SubTileBounds; // we don't use this here but need to pass it anyway
	GetTerrainManager()->CalculateOptimalTileTextureWindow(GetTileNumX(), GetTileNumY(), SubTileBounds, CoordsGenerator, DownsampleFactor);
	} // else
if(NewTexStateSet.valid()) // apply texture stateset
	{
	BareGeode->setStateSet(NewTexStateSet.get()); // not sure if this is right here...
	if(SharedData)
		{
		SharedData->SetSharedTerrainTexData(NewTexStateSet.get());
		} // if
	} // if


// Project vertices into Cartesian World space
CurLon = Terrain->Westest();
for (Ct = LonCt = 0; LonCt < Terrain->LonEntries(); LonCt ++, CurLon -= Terrain->LonStep()) // changed from -= to += on 9/2/03 -CXH. Changed back on 1/13/04 after fixing bug in SX ELEV writer -CXH
	{
	TerrainVertex.Lon  = CurLon;
	CurLat = Terrain->Southest();
	for (LatCt = 0; LatCt < Terrain->LatEntries(); Ct ++, LatCt ++, CurLat += Terrain->LatStep())
		{
		// TerrainVertex.Lon is initialized above, outside Lat loop
		TerrainVertex.Lat  = CurLat;
		TerrainVertex.Elev = Terrain->RawMap[Ct] * ElScaleMult; // Z=Elev=+Up in OSG
		MasterScene.DegToCart(TerrainVertex);

		coords->push_back( osg::Vec3( TerrainVertex.XYZ[0], TerrainVertex.XYZ[1], TerrainVertex.XYZ[2]));

		// new coordinate calculation for tiled (still works fine with untiled...)
		TexCoord[0] = CoordsGenerator.GetTexXCoord(TerrainVertex.Lon);
		TexCoord[1] = CoordsGenerator.GetTexYCoord(TerrainVertex.Lat);
		texcoord->push_back(osg::Vec2(TexCoord[0], TexCoord[1]));
		} // for
	} // for

// Make polygons
int Stripping = 1;

for (LonCt = 0; LonCt < (Terrain->LonEntries() - 1); LonCt ++)
	{
	int BaseVtxReject, NextBaseVtxReject;
	unsigned int SegmentVertCt = 0;
	osg::ref_ptr<DrawElementsUInt> drawElements = new DrawElementsUInt(PrimitiveSet::QUAD_STRIP);
	drawElements->reserve(min(NV_TERRAIN_VERTPERCELL_MAX, Terrain->LatEntries()));

	BaseVtx = LonCt * (Terrain->LatEntries());
	NextBaseVtx = BaseVtx + (Terrain->LatEntries());
	for (LatCt = 0; LatCt < (Terrain->LatEntries()); LatCt ++)
		{
		// these should only be tested on the actual body of the DEM, not the flaps
		BaseVtxReject     = Terrain->TestNullReject(BaseVtx + LatCt);
		NextBaseVtxReject = Terrain->TestNullReject(NextBaseVtx + LatCt);

		if(BaseVtxReject || NextBaseVtxReject)
			{ // both vertices fail the test. End tristrip.
			if(Stripping)
				{
				geometry->addPrimitiveSet( drawElements.get());
				drawElements = NULL;
				SegmentVertCt = 0;
				Stripping = 0;
				} // if
			} // if
		else
			{
			if(!Stripping)
				{
				drawElements = new DrawElementsUInt(PrimitiveSet::QUAD_STRIP);
				drawElements->reserve(min(NV_TERRAIN_VERTPERCELL_MAX, Terrain->LatEntries()));
				SegmentVertCt = 0;
				Stripping = 1;
				} // if
			} // else

		if(Stripping)
			{
			drawElements->push_back(BaseVtx + LatCt);
			drawElements->push_back(NextBaseVtx + LatCt);
			SegmentVertCt += 2;
			if(SegmentVertCt == NV_TERRAIN_VERTPERCELL_MAX)
				{
				// end a segment and begin a new
				geometry->addPrimitiveSet( drawElements.get());
				drawElements = new DrawElementsUInt(PrimitiveSet::QUAD_STRIP);
				drawElements->reserve(min(NV_TERRAIN_VERTPERCELL_MAX, Terrain->LatEntries()));
				SegmentVertCt = 0;
				// repeat last two verts
				drawElements->push_back(BaseVtx + LatCt);
				drawElements->push_back(NextBaseVtx + LatCt);
				// Referencing by index, we don't need to push an extra copy of the texcoords or normals, these vertices will refer to last pushed set
				SegmentVertCt += 2;
				} // if
			} // if
		} // for
	geometry->addPrimitiveSet( drawElements.get());
	} // for


color->push_back( osg::Vec4(1.0, 1.0, 1.0, 1.0));
geometry->setVertexArray(coords.get());
geometry->setTexCoordArray( 0, texcoord.get() );
geometry->setColorArray(color.get());
geometry->setColorBinding(osg::Geometry::BIND_OVERALL);


return(BareGeode.get());
} //  TerrainTile::GenerateDEMTile



DEM *CreateDownSampled(DEM *InputTile, int DecimationFactor) // DecimationFactor == 0 or 1 means no decimation and we bail
{
float *NewMap = NULL;
//JoeCoordSys *MyAttr;
//CoordSys *HostCoords;
double LatScan, LonScan, LatInc, LonInc, ALBDSLatRange, ALBDSLonRange, ScaleFrac;
unsigned long NewLonEntries, NewLatEntries, NewSize, ALBDSLr, ALBDSLc, Dest;
DEM *NewLOD;

if (InputTile->RawMap)
	{
/*
	if (MyJoe && (MyAttr = (JoeCoordSys *)MyJoe->MatchAttribute(WCS_JOE_ATTRIB_INTERNAL, WCS_JOE_ATTRIB_INTERNAL_COORDSYS)))
		HostCoords = MyAttr->Coord;
	else
		HostCoords = GlobalApp->AppEffects->FetchDefaultCoordSys();
*/

	if(DecimationFactor < 2) return(NULL);
	NewLonEntries = InputTile->pLonEntries;
	NewLatEntries = InputTile->pLatEntries;
	ScaleFrac = 1.0f / (double)DecimationFactor;

	// downsample
	//NewLonEntries = (unsigned long)(ScaleFrac * (double)InputTile->pLonEntries); // <<<>>> should be of the form 1 + (ScaleFrac * (pLonEntries - 1))
	//NewLatEntries = (unsigned long)(ScaleFrac * (double)InputTile->pLatEntries);
	NewLonEntries = 1 + (unsigned long)(ScaleFrac * (double)(InputTile->pLonEntries - 1));
	NewLatEntries = 1 + (unsigned long)(ScaleFrac * (double)(InputTile->pLatEntries - 1));
	if((NewLatEntries < 2) || (NewLonEntries < 2)) return(NULL);
	
	// looks like we're go, allocate the new guy
	if(NewLOD = new DEM)
		{
		ALBDSLatRange = InputTile->Northest() - InputTile->Southest();
		ALBDSLonRange = InputTile->Westest() - InputTile->Eastest();
		LatInc = ALBDSLatRange / (NewLatEntries - 1);
		LonInc = ALBDSLonRange / (NewLonEntries - 1);
		NewSize = NewLonEntries * NewLatEntries * sizeof (float);
		if (NewMap = (float *)calloc(1, NewSize))
			{
			for (LonScan = 0.0, ALBDSLr = Dest = 0; ALBDSLr < NewLonEntries; ALBDSLr ++, LonScan += LonInc)
				{
				for (LatScan = 0.0, ALBDSLc = 0; ALBDSLc < NewLatEntries; ALBDSLc ++, Dest ++, LatScan += LatInc)
					{
					NewMap[Dest] = (float)InputTile->DEMArrayPointExtract(/*HostCoords*/ NULL, InputTile->Southest() + LatScan, InputTile->Westest() - LonScan);
					} // for ALBDSLc=0... 
				} // for ALBDSLr=0... 

			// insert new synthesized values
			NewLOD->pLonEntries = NewLonEntries;
			NewLOD->pLatEntries = NewLatEntries;
			NewLOD->pLatStep = LatInc;
			NewLOD->pLonStep = LonInc;
			NewLOD->RawMap = NewMap;

			// copy existing unchanged values
			NewLOD->pElScale = InputTile->pElScale;
			NewLOD->pElDatum = InputTile->pElDatum;
			NewLOD->SetNullReject(InputTile->GetNullReject());
			NewLOD->SetNullValue(InputTile->NullValue());
			NewLOD->pNorthWest.Lon = InputTile->Westest();
			NewLOD->pNorthWest.Lat = InputTile->Northest();
			NewLOD->pSouthEast.Lon = InputTile->Eastest();
			NewLOD->pSouthEast.Lat = InputTile->Southest();
			return (NewLOD);
			} // if 
		delete NewLOD;
		NewLOD = NULL;
		return(NULL);
		} // if
	} // if

return(NULL);

} // CreateDownSampled


DEM *CreateFlappedDEM(DEM *InputTile, TerrainTile *Tile)
{
// pad out DEM structure geospatial bounds and array row/colums to account for flaps
// set flap elevations, return new DEM object
// don't alter or destroy original DEM object input
DEM *Flapper = NULL;

float WM, EM, NM, SM;

WM = Tile->WMin - (Tile->WMax - Tile->WMin); // hack to extend flaps down a little further as a work-around for now
EM = Tile->EMin - (Tile->EMax - Tile->EMin); // hack to extend flaps down a little further as a work-around for now
NM = Tile->NMin - (Tile->NMax - Tile->NMin); // hack to extend flaps down a little further as a work-around for now
SM = Tile->SMin - (Tile->SMax - Tile->SMin); // hack to extend flaps down a little further as a work-around for now

if(Tile->EdgeFlags) // no need to do this if no flaps are needed
	{
	if(Flapper = new DEM)
		{
		int LOffset = 0, ROffset = 0, TOffset = 0, BOffset = 0;
		// copy existing unchanged values
		Flapper->pLatStep = InputTile->LatStep();
		Flapper->pLonStep = InputTile->LonStep();
		Flapper->pElScale = InputTile->pElScale;
		Flapper->pElDatum = InputTile->pElDatum;
		Flapper->SetNullReject(InputTile->GetNullReject());
		Flapper->SetNullValue(InputTile->NullValue());

		// copy unchanged geospatial bounds, modify later if needed
		Flapper->pNorthWest.Lon = InputTile->Westest();
		Flapper->pNorthWest.Lat = InputTile->Northest();
		Flapper->pSouthEast.Lon = InputTile->Eastest();
		Flapper->pSouthEast.Lat = InputTile->Southest();

		// copy unchanged array sizes, modify later if needed
		Flapper->pLonEntries = InputTile->LonEntries();
		Flapper->pLatEntries = InputTile->LatEntries();

		// offset geospatial bounds and array dimensions as necessary
		if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT)
			{
			Flapper->pNorthWest.Lon += Flapper->pLonStep;
			LOffset = 1;
			} // if
		if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT)
			{
			Flapper->pSouthEast.Lon -= Flapper->pLonStep; 
			ROffset = 1;
			} // if
		if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP)
			{
			Flapper->pNorthWest.Lat += Flapper->pLatStep;
			TOffset = 1;
			} // if
		if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM)
			{
			Flapper->pSouthEast.Lat -= Flapper->pLatStep;
			BOffset = 1;
			} // if

		// increase array dimensions, keep offsets for subscript offsetting
		Flapper->pLatEntries += (TOffset + BOffset);
		Flapper->pLonEntries += (LOffset + ROffset);

		// allocate new array
		if(Flapper->AllocRawMap())
			{
			unsigned long int CopySubscript;
			// copy exiting elevation contents to new array
			for(unsigned int YLoop = 0; YLoop < InputTile->LatEntries(); YLoop++)
				{
				for(unsigned int XLoop = 0; XLoop < InputTile->LonEntries(); XLoop++)
					{
					Flapper->StoreElevation(YLoop + BOffset, XLoop + LOffset, InputTile->Sample(XLoop, YLoop));
					} // for
				} // for

			// write flap elevations to array
			if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT)
				{
				for(CopySubscript = 0; CopySubscript < InputTile->LatEntries(); CopySubscript++)
					{
					Flapper->StoreElevation(CopySubscript + BOffset, 0, WM);
					} // for
				} // if
			if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT)
				{
				for(CopySubscript = 0; CopySubscript < InputTile->LatEntries(); CopySubscript++)
					{
					Flapper->StoreElevation(CopySubscript + BOffset, Flapper->LonEntries() - 1, EM);
					} // for
				} // if
			if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP)
				{
				for(CopySubscript = 0; CopySubscript < InputTile->LonEntries(); CopySubscript++)
					{
					Flapper->StoreElevation(Flapper->LatEntries() - 1, CopySubscript + LOffset, NM);
					} // for
				} // if
			if(Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM)
				{
				for(CopySubscript = 0; CopySubscript < InputTile->LonEntries(); CopySubscript++)
					{
					Flapper->StoreElevation(0, CopySubscript + LOffset, SM);
					} // for
				} // if

			// write flap corner elevations to array
			if((Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT) && (Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP))
				{ // NW corner
				Flapper->StoreElevation(Flapper->LatEntries() - 1, 0, min(NM, WM));
				} // if
			if((Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT) && (Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM))
				{ // SW corner
				Flapper->StoreElevation(0, 0, min(SM, WM));
				} // if
			if((Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT) && (Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP))
				{ // NE corner
				Flapper->StoreElevation(Flapper->LatEntries() - 1, Flapper->LonEntries() - 1, min(NM, EM));
				} // if
			if((Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT) && (Tile->EdgeFlags & TerrainTile::NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM))
				{ // SE corner
				Flapper->StoreElevation(0, Flapper->LonEntries() - 1, min(SM, EM));
				} // if
			return(Flapper); // only returnpoint
			} // if
		delete Flapper;
		Flapper = NULL;
		} // if
	} // if

return(Flapper);
} // CreateFlappedDEM


bool PagedLOD_AddPagedNode(osg::PagedLOD* This, unsigned int childNo, float min, float max, const std::string& filename, float priorityOffset, float priorityScale)
{
This->setFileName(childNo, filename);
This->setPriorityOffset(childNo, priorityOffset);
This->setPriorityScale(childNo, priorityScale);
This->setRange(childNo, min, max);

return true;
} // PagedLOD_AddPagedNode

