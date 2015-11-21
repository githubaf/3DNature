// NVTerrainSupport.h
// various support functions for terrain operations that aren't in the 3DN DEM library


#ifndef NVW_NVTERRAIN_SUPPORT_H
#define NVW_NVTERRAIN_SUPPORT_H

#include <sstream>
#include <osg/Image>


#define NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS 6
#define NVW_ECW_MAX_SUPPORTED_BANDS	4

class TerrainManager;
class DEM;

const std::string EncodeTileString(const unsigned long int TileNumX, const unsigned long int TileNumY, const unsigned long int TileLODNum);
bool DecodeTileString(unsigned long int &TileNumX, unsigned long int &TileNumY, unsigned long int &TileLODNum, const std::string EncodedInput);

TerrainManager *GetTerrainManager(void);
void FixupELEVFileStepVars(DEM *FileToFix);

// this either loads an old-style ELEV file, or the metadata only via the ECW/J2K loader API
unsigned long int LoadDEMWrapper(const char *InName, DEM *LoadDestination);
unsigned long int LoadDEMMetadataViaECW(const char *InName, DEM *LoadDestination); // loads only the metadata
unsigned long int LoadDEMTileViaECW(const char *InName, DEM *LoadDestination, double TerrainMinElev, double TerrainMaxElev, unsigned long int LX, unsigned long int RX, unsigned long int TY, unsigned long int BY);
unsigned long int LoadImageMetadataViaECW(const char *InName, unsigned long int &Width, unsigned long int &Height);
unsigned long int LoadImageTileViaECW(const char *InName, osg::Image *DestImage, unsigned long int LX, unsigned long int RX, unsigned long int TY, unsigned long int BY, unsigned long int OffsetX, unsigned long int OffsetY, unsigned long int OutputWidth, unsigned long int OutputHeight);
void CleanupOpenECWFiles(void);

#endif // !NVW_NVTERRAIN_H

