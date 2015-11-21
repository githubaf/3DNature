// ReaderWriterNVP.cpp
// OSG (pseudo)-loader plugin framework for NatureView internal paging system
// created by CXH on 3/17/05
// Inspired by OSG's ReaderWriterOBJ.cpp

#if defined(_MSC_VER)
    #pragma warning( disable : 4786 ) // suppress "'identifier' : identifier was truncated to 'number' characters in the debug information"
#endif

#include <string>

#include <osgDB/Registry>
#include <osgDB/ReadFile>
#include <osgDB/FileUtils>
#include <osgDB/FileNameUtils>
#include <osg/Group>
#include <osgUtil/Simplifier>


#include "NVTerrain.h" // to pick up NVW_NVTERRAIN_PSEUDO_EXT
#include "NVTerrainSupport.h" // to pick up DecodeTileString()
#include "NVScene.h" // NVScene
#include "LODSharedData.h"
#include "NV3DO.h"

extern NVScene MasterScene;

extern char ThreadLoadMsg[160];


class ReaderWriterNVP : public osgDB::ReaderWriter
{
public:
    ReaderWriterNVP() { }
    virtual const char* className() const { return "NatureView Paging Loader"; }
    virtual bool acceptsExtension(const std::string& extension) const {return osgDB::equalCaseInsensitive(extension,NVW_NVTERRAIN_PSEUDO_EXT);};
    virtual ReadResult readNode(const std::string& fileName, const osgDB::ReaderWriter::Options* options) const;
}; // class ReaderWriterNVP

// register with Registry to instantiate the above reader/writer.
osgDB::RegisterReaderWriterProxy<ReaderWriterNVP> g_nvpReaderWriterProxy;

// load tile as requested
osgDB::ReaderWriter::ReadResult ReaderWriterNVP::readNode(const std::string& file, const osgDB::ReaderWriter::Options* options) const
{
unsigned long int TileNumX = 0, TileNumY = 0, TileLODNum = 0;
osg::ref_ptr<osg::Group> ResultGroup;
osg::ref_ptr<osg::Geode> ResultNode;

// code for setting up the database path so that internally referenced file are searched for on relative paths. 
//osg::ref_ptr<Options> local_opt = options ? static_cast<Options*>(options->clone(osg::CopyOp::SHALLOW_COPY)) : new Options;
//local_opt->setDatabasePath(osgDB::getFilePath(fileName));

char ThreadMsg[256];
sprintf(ThreadMsg, "%s", file.c_str());

// clearing to NULLs before writing should make it somewhat less thread-dangerous
memset(ThreadLoadMsg, 0, 160);
strncpy(ThreadLoadMsg, ThreadMsg, 159);



std::string ext = osgDB::getLowerCaseFileExtension(file);
if (!acceptsExtension(ext)) return (ReadResult::FILE_NOT_HANDLED);

if(DecodeTileString(TileNumX, TileNumY, TileLODNum, file))
	{
	double FolClipDist, TerrainClipDist;
	bool FolLOD = false;
	TerrainTile *CurrentTile;

	if(CurrentTile = GetTerrainManager()->GetTile(TileNumX, TileNumY))
		{
		FolClipDist     = MasterScene.SceneLOD.GetFolClip();
		TerrainClipDist = MasterScene.SceneLOD.GetTerrainClip();
		if(MasterScene.SceneLOD.CheckFolClip() && MasterScene.CheckFolDrapeImageName())
			{
			FolLOD = true;
			} // if

		// prepare for re-use of shared per-Tile data
		osg::ref_ptr<osg::Node> ExistingBillboardData;
		osg::ref_ptr<osg::Node> ExistingObjData;
		LODSharedData *SharedData;

		if(SharedData = GetOrCreateLODSharedData(CurrentTile->TileLODNode.get()))
			{
			ExistingBillboardData = SharedData->GetSharedBillboardData();
			ExistingObjData = SharedData->GetSharedObjData();
			} // if

		// build the terrain part
		unsigned long int LODExp;
		LODExp = 1 << TileLODNum;
		ResultNode = CurrentTile->BuildLODGeometry(FolLOD, LODExp, SharedData);

		if(ResultNode.valid())
			{
			ResultGroup = new osg::Group();
/*			if(TileLODNum < 5)
				{
				float sampleRatio = 0.5f;
				osgUtil::Simplifier simplifier(sampleRatio);
				ResultNode->accept(simplifier);
				} // if
*/
			} // if
		if(ResultGroup.valid())
			{
			ResultGroup->addChild(ResultNode.get());
			// install SharedData object into terrain
			if(SharedData)
				{
				// attach SharedData to newly created LOD node (ResultGroup) so it can be found again later
				ResultGroup->setUserData(SharedData);
				} // if

			double NClip, SClip, EClip, WClip;
			NClip = GetTerrainManager()->CalcTileULY(TileNumY);
			SClip = GetTerrainManager()->CalcTileLRY(TileNumY);
			EClip = GetTerrainManager()->CalcTileLRX(TileNumX);
			WClip = GetTerrainManager()->CalcTileULX(TileNumX);

			if(TileLODNum < NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS && MasterScene.CheckFolFileName())
				{
				// now do foliage and other billboards
				// create Billboard data if needed
				if(!ExistingBillboardData.valid())
					{
					osg::Group* BillboardGroup = NULL;
					// create vegetation only for areas within our tile
					if(BillboardGroup = new osg::Group())
						{
						CreateVegAndLabels(MasterScene.GetFolFileName(), MasterScene.GetFolFileSig(), EClip, WClip, SClip, NClip, BillboardGroup, BillboardGroup);
						ExistingBillboardData = BillboardGroup;
						} // if
					} // if
				// use Billboard if it's now available
				if(ExistingBillboardData.valid())
					{
					// use it
					ResultGroup->addChild(ExistingBillboardData.get());
					
					// record it in SharedData for later use, if possible
					if(SharedData)
						{
						SharedData->SetSharedBillboardData(ExistingBillboardData.get());
						} // if
					} // if
				} // if foliage file



			// now do 3d objects
			if(TileLODNum < NVW_NVTERRAIN_ADDITIONAL_LOD_LEVELS && MasterScene.GetNum3DOInstances() > 0)
				{
				// create Vec/Obj data if needed
				if(!ExistingObjData.valid())
					{
					osg::Group* ObjVecGroup = NULL;
					// create Objects/Vecs only for areas within our tile
					if(ObjVecGroup = new osg::Group())
						{
						CreateObjects(EClip, WClip, SClip, NClip, ObjVecGroup);
						ExistingObjData = ObjVecGroup;
						} // if
					} // if
				// use Obj/Vec if it's now available
				if(ExistingObjData.valid())
					{
					// use it
					ResultGroup->addChild(ExistingObjData.get());
					
					// record it in SharedData for later use, if possible
					if(SharedData)
						{
						SharedData->SetSharedObjData(ExistingObjData.get());
						} // if
					} // if
				} // if have 3DOs
			} // if ResultGroup
		
		} // if
	} // if

memset(ThreadLoadMsg, 0, 160);

osgDB::ReaderWriter::ReadResult Result(ResultGroup.get());
return (Result);
} // ReaderWriterNVP::readNode

