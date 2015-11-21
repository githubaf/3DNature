
#ifndef NVW_NVTERRAIN_H
#define NVW_NVTERRAIN_H

#include <osg/Geometry>
#include <osg/PagedLOD>

#include "TerrainTexCoordCalc.h"

#define NVW_NVTERRAIN_PSEUDO_EXT	"nvp"

// regardless of what GL says it can do, make terrain texture tiles no larger than this
#define NVW_NVTERRAIN_TILE_TEXTURE_DIM_MAX	2048

class DEM;
class LODSharedData;

int FetchTerrainHeight(void *CoordSysDefinition, double XCoordinate, double YCoordinate, double &ElevResult);

void CopySubTileHeader(DEM *SuperTile, DEM *SubTile, unsigned long int CellOffsetX, unsigned long int CellOffsetY, unsigned long int CellsWide, unsigned long int CellsHigh);
bool CopySubTile(DEM *SuperTile, DEM *SubTile, unsigned long int CellOffsetX, unsigned long int CellOffsetY, unsigned long int CellsWide, unsigned long int CellsHigh);

bool CreateTerrainNew(osg::Group *TerrainGroup);

class TerrainTile
	{
	public:

		enum
			{
			NVW_NVTERRAIN_TERRAINTILE_EDGE_LEFT   = 0x01,
			NVW_NVTERRAIN_TERRAINTILE_EDGE_RIGHT  = 0x02,
			NVW_NVTERRAIN_TERRAINTILE_EDGE_TOP    = 0x04,
			NVW_NVTERRAIN_TERRAINTILE_EDGE_BOTTOM = 0x08
			}; // EdgeFlags

		DEM *TerrainFragment;
		osg::ref_ptr<osg::PagedLOD> TileLODNode;
		float DEMLenDiagFlat, DEMLenDiagMaxElev;

		unsigned int TileNumX, TileNumY;
		unsigned char EdgeFlags; // see EdgeFlags enum, above

		float NMin, EMin, SMin, WMin;
		float NMax, EMax, SMax, WMax;

		TerrainTile(int TileX, int TileY) {EdgeFlags = 0; TileNumX = TileX; TileNumY = TileY; TerrainFragment = NULL; ClearEdgeExtrema(); DEMLenDiagFlat = 0.0f; DEMLenDiagMaxElev = 0.0f;};
		~TerrainTile() {};

		void UpdateEdgeExtrema(DEM *TerrainSource = NULL); // if you don't pass a DEM, it will try to use the one in the TerrainTile
		void ClearEdgeExtrema(void) {NMin = EMin = SMin = WMin = FLT_MAX; NMax = EMax = SMax = WMax = FLT_MIN;};
		unsigned long int GetTileNumX(void) {return(TileNumX);};
		unsigned long int GetTileNumY(void) {return(TileNumY);};
		bool ContainsAllNulls(void) {return(0);}; // <<<>>> we don't have this implemented yet
		osg::ref_ptr<osg::Geode> BuildLODGeometry(bool FolLOD, unsigned long int DownsampleFactor, LODSharedData *SharedData);
	
	private:
		osg::ref_ptr<osg::Geode> GenerateDEMTile(DEM *Terrain, bool FolLOD, TerrainTile *Tile, unsigned long int DownsampleFactor, LODSharedData *SharedData = NULL);
		osg::ref_ptr<osg::Image> CreateTileTextureImage(TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor);
		osg::ref_ptr<osg::StateSet> CreateTileTextureStateSet(TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor);
		
	}; // TerrainTile

class TerrainManager
	{
	friend class TerrainTile; // TerrainTile needs to access CalculateOptimalTileTextureWindow
	private:
		unsigned long int TilesWide, TilesHigh;
		unsigned long int DrapeImageWidth, DrapeImageHeight;
		unsigned long int TileSizeX, TileSizeY;
		std::vector< std::vector<TerrainTile *> >TileSet;
		DEM *LocalTerrainModel;

		void CalculateOptimalTiling(void);
		void UpdateAllEdgeExtrema(void);
		void UpdateEdgeFlags(void);
		void ExpandArrays(unsigned long int W, unsigned long int H);
		int GenerateRootFramework(osg::Group *TerrainGroup, double TerrainClipDist);

		// this method determines what size texturemap should be created to best represent the region of image covered by this tile, and sets up the math for texture coordinate translation within the sub-image
		bool CalculateOptimalTileTextureWindow(unsigned long int TileX, unsigned long int TileY, ImageTileBounds &TileBounds, TerrainTexCoordCalc &CoordsHelper, unsigned long int DownsampleFactor);
		unsigned long int CalcTileOffsetX(unsigned long int TileX) {return(TileX * GetTileSizeXUnderlap());}; // returns X coord of UL of cell within terrain grid
		unsigned long int CalcTileOffsetY(unsigned long int TileY) {return(TileY * GetTileSizeYUnderlap());}; // returns Y coord of UL of cell within terrain grid
		unsigned long int CalcTileWidth(unsigned long int TileX); // returns number of terrain cells wide a tile is, at full resolution
		unsigned long int CalcTileHeight(unsigned long int TileY); // returns number of terrain cells high a tile is, at full resolution
		double CalcXCoordFromCell(unsigned long int CellX); // returns geospatial X coord of a given terrain grid cell (tiling has no applicability here)
		double CalcYCoordFromCell(unsigned long int CellY); // returns geospatial Y coord of a given terrain grid cell (tiling has no applicability here)
		unsigned long int GetMaxTerrainTextureSize(void) {return(NVW_NVTERRAIN_TILE_TEXTURE_DIM_MAX);}; // currently just a define, could become a runtime query
		
		void SetTileSizeX(unsigned long int NewX) {TileSizeX = NewX;};
		void SetTileSizeY(unsigned long int NewY) {TileSizeY = NewY;};


	public:
		TerrainManager() {LocalTerrainModel = NULL; TilesWide = TilesHigh = 0; TileSizeX = 256; TileSizeY = 256;};
		~TerrainManager() {};
		void ConfigureInputTerrain(DEM *TerrainModel) {LocalTerrainModel = TerrainModel;};
		DEM *CreateSubTile(unsigned long int TileX, unsigned long int TileY, bool CopyData = true);
		TerrainTile *GetTile(unsigned long int X, unsigned long int Y);
		bool DoItAll(osg::Group *TerrainGroup, bool FolLOD, double TerrainClipDist);
		DEM *GetLocalTerrainModel(void) {return(LocalTerrainModel);};
		void SetDrapeImageImageDims(unsigned long int NewWidth, unsigned long int NewHeight) {DrapeImageWidth = NewWidth; DrapeImageHeight = NewHeight;};

		// used for determining what other items are within our Tile for LOD purposes
		double CalcTileULX(unsigned long int TileX) {return(CalcXCoordFromCell(CalcTileOffsetX(TileX)));};  // returns geospatial X coord of a UL corner of a given terrain tile
		double CalcTileULY(unsigned long int TileY) {return(CalcYCoordFromCell(CalcTileOffsetY(TileY)));}; // returns geospatial Y coord of a UL corner of a given terrain tile
		double CalcTileLRX(unsigned long int TileX) {return(CalcXCoordFromCell(CalcTileOffsetX(TileX) + CalcTileWidth(TileX) - 1));}; // returns geospatial X coord of a LR corner of a given terrain tile
		double CalcTileLRY(unsigned long int TileY) {return(CalcYCoordFromCell(CalcTileOffsetY(TileY) + CalcTileHeight(TileY) - 1));}; // returns geospatial Y coord of a LR corner of a given terrain tile
		
		// replaces NVW_NVTERRAIN_TILESIZE_X, NVW_NVTERRAIN_TILESIZE_Y, NVW_NVTERRAIN_TILESIZE_UNDERLAP_X, NVW_NVTERRAIN_TILESIZE_UNDERLAP_Y macros
		// this goofyness is due to the need for one cell of overlap in both directions
		unsigned long int GetTileSizeX(void) const {return(TileSizeX);};
		unsigned long int GetTileSizeY(void) const {return(TileSizeY);};
		unsigned long int GetTileSizeXUnderlap(void) const {return(TileSizeX - 1);};
		unsigned long int GetTileSizeYUnderlap(void) const {return(TileSizeY - 1);};

	}; // TerrainManager

#endif // !NVW_NVTERRAIN_H
