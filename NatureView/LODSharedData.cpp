// LODSharedData.cpp
// LOD managment code
// created from LODSharedData.h on 11/15/05 by CXH

#include "LODSharedData.h"

// used in terrain and foliage loaders to share and reuse already-loaded data
LODSharedData *GetOrCreateLODSharedData(osg::PagedLOD *TileLODNode)
{
LODSharedData *SharedData = NULL;
osg::Node *ExistingLOD = NULL;
unsigned int ChildScan;

if(TileLODNode) // attempt to located shared data object on any child node of this PageLOD (group)
	{
	for(ChildScan = 0; ChildScan < TileLODNode->getNumChildren(); ChildScan++)
		{
		if(ExistingLOD = TileLODNode->getChild(ChildScan))
			{
			osg::ref_ptr<osg::Referenced> UData;
			UData = ExistingLOD->getUserData();
			if(UData.valid())
				{
				SharedData = dynamic_cast<LODSharedData *>(UData.get());
				if(SharedData)
					{
					break; // found one
					} // if
				} // if
			} // if
		} // for
	} // if

if(!SharedData) // need to create a SharedData object
	{
	if(SharedData = new LODSharedData)
		{ // load into all available child nodes
		// note that if we're loading the first node and creating the first SharedData object, there won't be any child nodes yet
		/*
		for(ChildScan = 0; ChildScan < TileLODNode->getNumChildren(); ChildScan++)
			{
			if(ExistingLOD = TileLODNode->getChild(ChildScan))
				{
				ExistingLOD->setUserData(SharedData);
				} // if
			} // for
		*/
		} // if
	} // if

return(SharedData);
} // GetOrCreateLODSharedData
