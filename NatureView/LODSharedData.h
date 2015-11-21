// LODSharedData.h
// LOD managment code
// created from scratch on 9/30/05 by CXH


#ifndef NVW_LODSHAREDDATA_H
#define NVW_LODSHAREDDATA_H

#include <osg/Referenced>
#include <osg/ref_ptr>
#include <osg/StateSet>
#include <osg/PagedLOD>

class LODSharedData : public osg::Referenced // derived from Referenced in order to be used in OSG's UserData
{
    public:

		osg::Node *GetSharedBillboardData(void) {return(_SharedBBData.get());}; // foliage and labels/etc
		void SetSharedBillboardData(osg::Node *BBData) {_SharedBBData = BBData;};
		osg::StateSet *GetSharedTerrainTexData(void) {return(_SharedTTState.get());};  // terrain texture images
		void SetSharedTerrainTexData(osg::StateSet *TTData) {_SharedTTState = TTData;};
		osg::Node *GetSharedObjData(void) {return(_SharedObjData.get());}; // 3D Objects/Walls
		void SetSharedObjData(osg::Node *ObjData) {_SharedObjData = ObjData;};

    protected:

        /** Object destructor. Note, is protected so that LODSharedDatas cannot
            be deleted other than by being dereferenced and the reference
            count being zero (see osg::Referenced), preventing the deletion
            of nodes which are still in use. This also means that
            LODSharedData cannot be created on stack i.e LODSharedData node will not compile,
            forcing all nodes to be created on the heap i.e
            LODSharedData* LODSD = new LODSharedData(). */
        virtual ~LODSharedData() {}
        

    private:

        /** disallow any copy operator.*/
        LODSharedData& operator = (const LODSharedData&) { return *this; }
        
        osg::ref_ptr<osg::StateSet> _SharedTTState;
        osg::ref_ptr<osg::Node> _SharedBBData;
        osg::ref_ptr<osg::Node> _SharedObjData;
};

// used in terrain and foliage loaders to share and reuse already-loaded data
LODSharedData *GetOrCreateLODSharedData(osg::PagedLOD *TileLODNode);

#endif // !NVW_LODSHAREDDATA_H
