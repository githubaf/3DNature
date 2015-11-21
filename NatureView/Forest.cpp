
#include <sstream>


#include <osg/PositionAttitudeTransform>
#include <osg/Billboard>
#include <osg/LOD>
#include <osg/MatrixTransform>
#include <osg/BlendFunc>
#include <osg/AlphaFunc>
#include <osgDB/ReadFile>
#include <osg/TexEnv>

using namespace osg;

#include "UsefulPathString.h"

#include "NVFoliage.h"
#include "NVScene.h"
#include "NVSigCheck.h"
#include "NVNodeMasks.h"

extern NVScene MasterScene;


/* Forest Code */

int FolInit;
extern unsigned long int DebugVarTotalTreeCountFile;
extern unsigned long int DebugVarTotalTreeCountLoaded;



void Forest::Cell::computeBound()
{
    _bb.init();
    for(CellList::iterator citr=_cells.begin();
        citr!=_cells.end();
        ++citr)
    {
        (*citr)->computeBound();
        _bb.expandBy((*citr)->_bb);
    }

    for(TreeList::iterator titr=_trees.begin();
        titr!=_trees.end();
        ++titr)
    {
        _bb.expandBy((*titr)->_position);
    }
}

bool Forest::Cell::divide(unsigned int maxNumTreesPerCell)
{

    if (_trees.size()<=maxNumTreesPerCell) return false;

    computeBound();

    float radius = _bb.radius();
    float divide_distance = radius*0.7f;
    if (devide((_bb.xMax()-_bb.xMin())>divide_distance,(_bb.yMax()-_bb.yMin())>divide_distance,(_bb.zMax()-_bb.zMin())>divide_distance))
    {
        // recusively divide the new cells till maxNumTreesPerCell is met.
        for(CellList::iterator citr=_cells.begin();
            citr!=_cells.end();
            ++citr)
        {
            (*citr)->divide(maxNumTreesPerCell);
        }
        return true;
   }
   else
   {
        return false;
   }
}

bool Forest::Cell::devide(bool xAxis, bool yAxis, bool zAxis)
{
    if (!(xAxis || yAxis || zAxis)) return false;

    if (_cells.empty())
        _cells.push_back(new Cell(_bb));

    if (xAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float xCenter = (orig_cell->_bb.xMin()+orig_cell->_bb.xMax())*0.5f;
            orig_cell->_bb.xMax() = xCenter;
            new_cell->_bb.xMin() = xCenter;

            _cells.push_back(new_cell);
        }
    }

    if (yAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float yCenter = (orig_cell->_bb.yMin()+orig_cell->_bb.yMax())*0.5f;
            orig_cell->_bb.yMax() = yCenter;
            new_cell->_bb.yMin() = yCenter;

            _cells.push_back(new_cell);
        }
    }

    if (zAxis)
    {
        unsigned int numCellsToDivide=_cells.size();
        for(unsigned int i=0;i<numCellsToDivide;++i)
        {
            Cell* orig_cell = _cells[i].get();
            Cell* new_cell = new Cell(orig_cell->_bb);

            float zCenter = (orig_cell->_bb.zMin()+orig_cell->_bb.zMax())*0.5f;
            orig_cell->_bb.zMax() = zCenter;
            new_cell->_bb.zMin() = zCenter;

            _cells.push_back(new_cell);
        }
    }

    bin();

    return true;

}

void Forest::Cell::bin()
{   
    // put trees in appropriate cells.
    TreeList treesNotAssigned;
    for(TreeList::iterator titr=_trees.begin();
        titr!=_trees.end();
        ++titr)
    {
        Tree* tree = titr->get();
        bool assigned = false;
        for(CellList::iterator citr=_cells.begin();
            citr!=_cells.end() && !assigned;
            ++citr)
        {
            if ((*citr)->contains(tree->_position))
            {
                (*citr)->addTree(tree);
                assigned = true;
            }
        }
        if (!assigned) treesNotAssigned.push_back(tree);
    }

    // put the unassigned trees back into the original local tree list.
    _trees.swap(treesNotAssigned);


    // prune empty cells.
    CellList cellsNotEmpty;
    for(CellList::iterator citr=_cells.begin();
        citr!=_cells.end();
        ++citr)
    {
        if (!((*citr)->_trees.empty()))
        {
            cellsNotEmpty.push_back(*citr);
        }
    }
    _cells.swap(cellsNotEmpty);


}

osg::Node* Forest::createLabelBillboardGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist)
{
    bool needGroup = !(cell->_cells.empty());
    bool needBillboard = !(cell->_trees.empty());
    
    osg::Billboard* billboard = 0;
    osg::LOD* group = 0;
    
    if (needBillboard)
		{
        billboard = new osg::Billboard;
        billboard->setStateSet(stateset);
		billboard->setDataVariance(Object::STATIC);
        for(TreeList::iterator itr=cell->_trees.begin(); itr!=cell->_trees.end(); ++itr)
			{
            Tree& tree = **itr;
			if(tree._FlagBits & Tree::TREE_FLAGBIT_PIVOTLEFT)
				{
	            billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_LEFT, tree._ActionID),tree._position);
				} // if
			else if(tree._FlagBits & Tree::TREE_FLAGBIT_PIVOTLEFT)
				{
	            billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_RIGHT, tree._ActionID),tree._position);
				} // else if
			else // there is no TREE_FLAGBIT_PIVOTCENTER
				{
	            billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_CENTER, tree._ActionID),tree._position);
				} // else
			} // for
		} // if
    
    if (needGroup)
		{
        group = new osg::LOD;
		group->setDataVariance(Object::STATIC);
        for(Cell::CellList::iterator itr=cell->_cells.begin(); itr!=cell->_cells.end(); ++itr)
			{
			osg::Node* Result;
			osg::Group *IsItAGroup;
			Result = createLabelBillboardGraph(itr->get(),stateset, FolClipDist);
			if(IsItAGroup = Result->asGroup()) // is it a group or a billboard?
				{ // group
				group->addChild(Result, 0, FLT_MAX); // don't clip supergroups
				} // if
			else
				{ // billboard
				group->addChild(Result, 0, FolClipDist);
				} // else
            //group->addChild(createBillboardGraph(itr->get(),stateset), 0, 1000);
	        } // for
        if (billboard) group->addChild(billboard, 0, FolClipDist);
		} // if

    if (group) return group;
    else return billboard;
} // createLabelBillboardGraph



osg::Geometry* Forest::createSprite( float w, float h, osg::Vec4ub color, enum Forest::PivotAxis Swivel, signed long int ActionID)
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(4));
    osg::Vec2Array& t = *(new osg::Vec2Array(4));
    osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));

	switch(Swivel)
		{
		case TREE_PIVOTAXIS_LEFT:
			{
			v[0].set(0.0f,0.0f,0.0f);
			v[1].set( w,0.0f,0.0f);
			v[2].set( w,0.0f,h);
			v[3].set(0.0f,0.0f,h);
			break;
			} // 
		case TREE_PIVOTAXIS_RIGHT:
			{
			v[0].set(-w,0.0f,0.0f);
			v[1].set(0.0f,0.0f,0.0f);
			v[2].set(0.0f,0.0f,h);
			v[3].set(-w,0.0f,h);
			break;
			} // 
		default:
		case TREE_PIVOTAXIS_CENTER:
			{
			v[0].set(-w*0.5f,0.0f,0.0f);
			v[1].set( w*0.5f,0.0f,0.0f);
			v[2].set( w*0.5f,0.0f,h);
			v[3].set(-w*0.5f,0.0f,h);
			break;
			} // 
		} // switch


    c[0] = color;

    t[0].set(0.0f,0.0f);
    t[1].set(1.0f,0.0f);
    t[2].set(1.0f,1.0f);
    t[3].set(0.0f,1.0f);

    ActionedGeometry *geom = new ActionedGeometry;
    
    // <<<>>> set ActionID only if other than -1
    if(ActionID != -1)
		{
		geom->SetActionID(ActionID);
		} // if

	geom->setDataVariance(Object::STATIC);
	//geom->setUseDisplayList(false);

    geom->setVertexArray( &v );

    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4) );

    return geom;
} // createSprite

osg::Geometry* Forest::createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color, signed long int ActionID)
{
    // set up the coords
    osg::Vec3Array& v = *(new osg::Vec3Array(4));
    osg::Vec2Array& t = *(new osg::Vec2Array(4));
    osg::Vec4ubArray& c = *(new osg::Vec4ubArray(1));
    
    float rotation = random(0.0f,osg::PI/2.0f);
    float sw = sinf(rotation)*w*0.5f;
    float cw = cosf(rotation)*w*0.5f;

    v[0].set(pos.x()-sw,pos.y()-cw,pos.z()+0.0f);
    v[1].set(pos.x()+sw,pos.y()+cw,pos.z()+0.0f);
    v[2].set(pos.x()+sw,pos.y()+cw,pos.z()+h);
    v[3].set(pos.x()-sw,pos.y()-cw,pos.z()+h);
/*
    v[4].set(pos.x()-cw,pos.y()+sw,pos.z()+0.0f);
    v[5].set(pos.x()+cw,pos.y()-sw,pos.z()+0.0f);
    v[6].set(pos.x()+cw,pos.y()-sw,pos.z()+h);
    v[7].set(pos.x()-cw,pos.y()+sw,pos.z()+h);
*/
    c[0] = color;

    t[0].set(0.0f,0.0f);
    t[1].set(1.0f,0.0f);
    t[2].set(1.0f,1.0f);
    t[3].set(0.0f,1.0f);
/*
    t[4].set(0.0f,0.0f);
    t[5].set(1.0f,0.0f);
    t[6].set(1.0f,1.0f);
    t[7].set(0.0f,1.0f);
*/
    ActionedGeometry *geom = new ActionedGeometry;
    
    // set ActionID only if other than -1
    if(ActionID != -1)
		{
		geom->SetActionID(ActionID);
		} // if

    geom->setVertexArray( &v );

    geom->setTexCoordArray( 0, &t );

    geom->setColorArray( &c );
    geom->setColorBinding( osg::Geometry::BIND_OVERALL );

    geom->addPrimitiveSet( new osg::DrawArrays(osg::PrimitiveSet::QUADS,0,4) );

    return geom;
} // createOrthogonalQuads

// no fancy subdividing anymore
osg::Node* Forest::createBillboardGraph(TreeList &trees, osg::StateSet* stateset, double FolClipDist)
{
osg::Billboard* billboard = NULL;
osg::LOD* group = NULL;

group = new osg::LOD;
group->setDataVariance(Object::STATIC);
billboard = new osg::Billboard;
billboard->setStateSet(stateset);
billboard->setDataVariance(Object::STATIC);

for(TreeList::iterator itr=trees.begin(); itr!=trees.end(); ++itr)
	{
    Tree& tree = **itr;
	billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_CENTER, tree._ActionID),tree._position);
	} // for
group->addChild(billboard, 0, FolClipDist); // need this to do LOD

return group;
} // createBillboardGraph

osg::Node* Forest::createBillboardGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist)
{
    bool needGroup = !(cell->_cells.empty());
    bool needBillboard = !(cell->_trees.empty());
    
    //osg::Billboard* billboard = 0;
    osg::Geode* billboard = 0;
    osg::LOD* group = 0;
    
    if (needBillboard)
		{
        //billboard = new osg::Billboard;
        billboard = new osg::Geode;
        billboard->setStateSet(stateset);
		billboard->setDataVariance(Object::STATIC);
        for(TreeList::iterator itr=cell->_trees.begin(); itr!=cell->_trees.end(); ++itr)
			{
            Tree& tree = **itr;
            //billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_CENTER, tree._ActionID),tree._position);
            //billboard->addDrawable(createSprite(tree._width,tree._height,tree._color, Forest::TREE_PIVOTAXIS_CENTER, tree._ActionID));
            billboard->addDrawable(createOrthogonalQuads(tree._position,tree._width,tree._height,tree._color, tree._ActionID));
			} // for
		} // if
    
    if (needGroup)
		{
        group = new osg::LOD;
		group->setDataVariance(Object::STATIC);
        for(Cell::CellList::iterator itr=cell->_cells.begin(); itr!=cell->_cells.end(); ++itr)
			{
			osg::Node* Result;
			osg::Group *IsItAGroup;
			Result = createBillboardGraph(itr->get(),stateset, FolClipDist);
			if(IsItAGroup = Result->asGroup()) // is it a group or a billboard?
				{ // group
				group->addChild(Result, 0, FLT_MAX); // don't clip supergroups
				} // if
			else
				{ // billboard
				group->addChild(Result, 0, FolClipDist);
				} // else
            //group->addChild(createBillboardGraph(itr->get(),stateset), 0, 1000);
	        } // for
        if (billboard) group->addChild(billboard, 0, FolClipDist);
		} // if

    if (group) return group;
    else return billboard;
} // createBillboardGraph



osg::Node* Forest::createXGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());
    
    osg::Geode* geode = 0;
    osg::LOD* group = 0;
    
    if (needTrees)
		{
        geode = new osg::Geode;
        geode->setStateSet(stateset);
        for(TreeList::iterator itr=cell->_trees.begin(); itr!=cell->_trees.end(); ++itr)
			{
            Tree& tree = **itr;
            geode->addDrawable(createOrthogonalQuads(tree._position,tree._width,tree._height,tree._color, tree._ActionID));
		    } // for
		} // if
    if (needGroup)
		{
        group = new osg::LOD;
        for(Cell::CellList::iterator itr=cell->_cells.begin(); itr!=cell->_cells.end(); ++itr)
			{
			osg::Node* Result;
			osg::Group *IsItAGroup;
			Result = createXGraph(itr->get(), stateset, FolClipDist);
			if(IsItAGroup = Result->asGroup()) // is it a group or a billboard?
				{ // group
				group->addChild(Result, 0, FLT_MAX); // don't clip supergroups
				} // if
			else
				{ // stem
				group->addChild(Result, 0, FolClipDist);
				} // else
			} // for 
        if (geode) group->addChild(geode, 0, FolClipDist);
		} // if
    if (group) return group;
    else return geode;
} // createXGraph

osg::Node* Forest::createTransformGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist)
{
    bool needGroup = !(cell->_cells.empty());
    bool needTrees = !(cell->_trees.empty());
    
    osg::Group* transform_group = 0;
    osg::LOD* group = 0;
    
    if (needTrees)
	    {
        transform_group = new osg::Group;
        
        osg::Geometry* geometry = createOrthogonalQuads(osg::Vec3(0.0f,0.0f,0.0f),1.0f,1.0f,osg::Vec4ub(255,255,255,255));
        osg::Geode* geode = new osg::Geode;
        geode->setStateSet(stateset);
        geode->addDrawable(geometry);
        
        for(TreeList::iterator itr=cell->_trees.begin(); itr!=cell->_trees.end(); ++itr)
		    {
            Tree& tree = **itr;
            osg::MatrixTransform* transform = new osg::MatrixTransform;
            transform->setMatrix(osg::Matrix::scale(tree._width,tree._width,tree._height)*osg::Matrix::translate(tree._position));
			#ifdef NVW_SUPPORT_QUERYACTION
			// Add ACTIONID data for Query/Action
            // put tree._ActionID into the MatrixTransform where it'll be found by the scene graph walk up
            // <<<>>> Text desc is not the most efficient technique ever found...
			if(tree._ActionID >= 0)
				{
				// ActionID can be 0 and still be valid
				std::ostringstream TempNumberAsString;
				TempNumberAsString << tree._ActionID << std::endl; // just like sprintf %d
				transform->addDescription(TempNumberAsString.str());
				} // if
			#endif // NVW_SUPPORT_QUERYACTION
            transform->addChild(geode);
            transform_group->addChild(transform);
			} // for
		} // if
    
    if (needGroup)
		{
        group = new osg::LOD;
        for(Cell::CellList::iterator itr=cell->_cells.begin(); itr!=cell->_cells.end(); ++itr)
			{
			osg::Node* Result;
			osg::Group *IsItAGroup;
			Result = createTransformGraph(itr->get(),stateset, FolClipDist);
			if(IsItAGroup = Result->asGroup()) // is it a group or a billboard?
				{ // group
				group->addChild(Result, 0, FLT_MAX); // don't clip supergroups
				} // if
			else
				{ // stem
				group->addChild(Result, 0, FolClipDist);
				} // else
            //group->addChild(createTransformGraph(itr->get(),stateset), FolClipDist);
			} // for
        if (transform_group) group->addChild(transform_group, 0, FolClipDist);
	    } // if
    if (group) return group;
    else return transform_group;
} // createTransformGraph


osg::ref_ptr<osg::Group> Forest::createForest(TreeList &trees, osg::StateSet *dstate, int TreeType, double ClipDist)
{

osg::ref_ptr<Cell> cell = new Cell;
cell->addTrees(trees);
cell->divide();

osg::ref_ptr<osg::Group> scene = new osg::Group;

if(TreeType == 0)
	{
    // Creating billboard based forest
    scene->addChild(createBillboardGraph(cell.get(), dstate, ClipDist));
    //scene->addChild(createBillboardGraph(trees, dstate, ClipDist)); // flat/fast version for debugging
    } // else if
else if(TreeType == 1)
    {
    // Creating double-quad based forest
    scene->addChild(createXGraph(cell.get(), dstate, ClipDist));
    } // else if
else if(TreeType == 2)
    {
    // Creating matrixTransform based forest
    scene->addChild(createTransformGraph(cell.get(), dstate, ClipDist));
    } // else if
   
return (scene);
} // Forest::createForest


osg::ref_ptr<osg::LOD> Forest::createLabels(TreeList &trees, osg::StateSet *dstate, double ClipDist)
{
osg::ref_ptr<Cell> cell = new Cell;
cell->addTrees(trees);
cell->divide();

osg::ref_ptr<osg::LOD> scene = new osg::LOD;

// Creating billboard based Labels (with LOD disappear)
scene->addChild(createLabelBillboardGraph(cell.get(), dstate, ClipDist), 0, ClipDist);
   
return (scene);
} // Forest::createLabels

/* End Forest Code */


bool CreateVegAndLabels(const char *VegFileName, const char *VegFileSig, double MinXCoord, double MaxXCoord, double MinYCoord, double MaxYCoord, osg::Group *VegParent, osg::Group *LabelParent)
{
FILE *VegFile = NULL;
unsigned long int TotalTreeCount = 0;
unsigned long int CreatedTreeCount = 0;
unsigned long int IMGLSize = 0, IMGLSig = 0, IMGLNumImages = 0;
double FolClipDist, LabelClipDist;

FolClipDist = MasterScene.SceneLOD.GetFolClip();
LabelClipDist = MasterScene.SceneLOD.GetLabelClip();

osg::Group *VegGroup = NULL;
osg::Group *LabelGroup = NULL;

osg::ref_ptr<osg::StateSet> stateset;
osg::ref_ptr<osg::Billboard> BB;

#ifdef NV_CHECK_SIGNATURES
if(!CheckDependentBinaryFileSignature(VegFileSig, VegFileName))
	{
	SetGlobalSigInvalid(true);
	// Hard to back out of creating vegetation
	//VegFileName = NULL;
	return(NULL);
	} // if

#endif // NV_CHECK_SIGNATURES


if(VegFileName && VegFileName[0])
	{
	if(VegFile = fopen(VegFileName, "rb"))
		{
		double LocalOffsetLat = 0.0, LocalOffsetLon = 0.0, LocalOffsetElev = 0.0;

		unsigned long int ByteOrderSignature = 0xaabbccdd, NumEntries, FileSig, Version;
		unsigned char TypeMask = 0, INLSTypeMask = 0;

		// file descriptor
		fread((char *)&FileSig, 4, 1, VegFile);
		fread((char *)&ByteOrderSignature, sizeof (long), 1, VegFile);
		fread((char *)&Version, sizeof (long), 1, VegFile);
		if(Version != 0x00000001 && Version != 0x00000101) // prerelease version && first SX2 release
			return(NULL); // incompatible version

		// extents (blank for now)
		double MinX, MaxX, MinY, MaxY, MinZ, MaxZ, MaxHt, MinHt;
		MinX = MaxX = MinY = MaxY = MinZ = MaxZ = MaxHt = MinHt = 0.0; // clear
		fread((char *)&MinX, sizeof (double), 1, VegFile);
		fread((char *)&MaxX, sizeof (double), 1, VegFile);
		fread((char *)&MinY, sizeof (double), 1, VegFile);
		fread((char *)&MaxY, sizeof (double), 1, VegFile);
		fread((char *)&MinZ, sizeof (double), 1, VegFile);
		fread((char *)&MaxZ, sizeof (double), 1, VegFile);
		fread((char *)&MinHt, sizeof (double), 1, VegFile);
		fread((char *)&MaxHt, sizeof (double), 1, VegFile);

		// Typemask
		TypeMask = 0; // unknown
		fread((char *)&TypeMask, sizeof (char), 1, VegFile);

		// NumEntries
		fread((char *)&NumEntries, sizeof (long), 1, VegFile);

		// read IMGL
		fread((char *)&IMGLSig, sizeof (long), 1, VegFile);
		fread((char *)&IMGLSize, sizeof (long), 1, VegFile);

		// we need this counter
		fread((char *)&IMGLNumImages, sizeof (long), 1, VegFile);
		
		if(!FolInit)
			{

			float WriteWidthFac;
			unsigned char ImageFlag;
			char FileName[255];
			for(unsigned long int ImageLoop = 0; ImageLoop < IMGLNumImages; ImageLoop++)
				{
				NVFolSpecies *FolSpecies;
				fread((char *)&WriteWidthFac, sizeof (float), 1, VegFile);
				fread((char *)&ImageFlag, sizeof (char), 1, VegFile);
				fgetline(FileName, 255, VegFile, 1, 0); // NULL-terminated

				if(FolSpecies = new NVFolSpecies)
					{
					FolSpecies->SetName(FileName);
					FolSpecies->Proportion = WriteWidthFac;
					FolSpecies->Flags = ImageFlag;
					MasterScene.AddFolSpecies(FolSpecies);
					FolSpecies = NULL;
					} // if
				} // for
			} // if !FolInit
		else
			{
			fseek(VegFile, IMGLSize - sizeof (long), SEEK_CUR); // already read IMGLNumImages
			} // else

		FolInit = 1;

		// INLS
		unsigned long int INLSSize = 0, INLSSig = 0;
		fread((char *)&INLSSig, sizeof (long), 1, VegFile);
		fread((char *)&INLSSize, sizeof (long), 1, VegFile);
		// Read INLS here

		// INLS extents
		double INLSMinX, INLSMaxX, INLSMinY, INLSMaxY, INLSMinZ, INLSMaxZ, INLSMaxHt, INLSMinHt;
		INLSMinX = INLSMaxX = INLSMinY = INLSMaxY = INLSMinZ = INLSMaxZ = INLSMaxHt = INLSMinHt = 0.0; // clear
		fread((char *)&INLSMinX, sizeof (double), 1, VegFile);
		fread((char *)&INLSMaxX, sizeof (double), 1, VegFile);
		fread((char *)&INLSMinY, sizeof (double), 1, VegFile);
		fread((char *)&INLSMaxY, sizeof (double), 1, VegFile);
		fread((char *)&INLSMinZ, sizeof (double), 1, VegFile);
		fread((char *)&INLSMaxZ, sizeof (double), 1, VegFile);

		fread((char *)&LocalOffsetLon, sizeof (double), 1, VegFile);
		fread((char *)&LocalOffsetLat, sizeof (double), 1, VegFile);
		fread((char *)&LocalOffsetElev, sizeof (double), 1, VegFile);
		
		fread((char *)&INLSMinHt, sizeof (double), 1, VegFile);
		fread((char *)&INLSMaxHt, sizeof (double), 1, VegFile);

		// INLS Typemask
		fread((char *)&INLSTypeMask, sizeof (char), 1, VegFile);

		#ifdef SIXTYFOUR_BIT_SUPPORTED
		UL64 Seek64;
		Seek64 = CurPos;
		fread((char *)&Seek64, sizeof (UL64), 1, VegFile);
		#else // !SIXTYFOUR_BIT_SUPPORTED
		// <<<>>> implement 32-bit support...
		#endif // !SIXTYFOUR_BIT_SUPPORTED

		// INLS entries point to the 4-byte chunkID of an INFL, for safety/validation after a seek
		
		// INFL
		unsigned long int INFLSize = 0, INFLSig = 0;
		fread((char *)&INFLSig, sizeof (long), 1, VegFile);
		fread((char *)&INFLSize, sizeof (long), 1, VegFile);

		unsigned long int DatPt;
		unsigned char EIDChar;
		unsigned short EIDWord;
		unsigned long int EIDFull, EIDLong;
		for (DatPt = 0; DatPt < NumEntries; DatPt ++)
			{
			unsigned short Height;
			Point3f FolXYZ;

			fread((char *)&EIDChar, sizeof (char), 1, VegFile);

			if(EIDChar & 0x80)
				{
				// could be a 16-bit (net 14-bit) or a 32-bit (net 30-bit) value
				if(EIDChar & 0x40) // if this bit is also on, it's a 32-bit (net 30-bit) value
					{
					EIDLong = ((unsigned long int)EIDChar) << 24;
					fread((char *)&EIDChar, sizeof (char), 1, VegFile);
					EIDLong |= ((unsigned long int)EIDChar) << 16;
					fread((char *)&EIDChar, sizeof (char), 1, VegFile);
					EIDLong |= ((unsigned long int)EIDChar) << 8;
					fread((char *)&EIDChar, sizeof (char), 1, VegFile);
					EIDLong |= ((unsigned long int)EIDChar);
					EIDFull = EIDLong - 1;
					} // if
				else // 16-bit (net 14-bit)
					{
					EIDWord = ((unsigned short)EIDChar) << 8;
					fread((char *)&EIDChar, sizeof (char), 1, VegFile);
					EIDWord |= EIDChar;
					EIDFull = EIDWord - 1;
					} // else
				} // if
			else
				{ // can only be an 8-bit value
				EIDFull = EIDChar - 1;
				} // else

/*			if(FolData.ElementID > 65535)
				{ // write as long
				fread((char *)&FolData.ElementID, sizeof (unsigned long), 1, VegFile);
				INFLSize += 4;
				} // if
			else if(FolData.ElementID > 127)
				{
				unsigned short EIDShort;
				EIDShort = (unsigned short)FolData.ElementID;
				fread((char *)&EIDShort, sizeof (unsigned short), 1, VegFile);
				INFLSize += 2;
				} // if
			else // (FolData.ElementID !> 127)
*/
				{
				//fread((char *)&EIDChar, sizeof (char), 1, VegFile);
				} // if

			unsigned char InstanceFlags = 0; // Flip, colored, label, labelside, others?
			fread((char *)&InstanceFlags, sizeof (char), 1, VegFile);

			// height (WCS encoded)
			fread((char *)&Height, sizeof (unsigned short), 1, VegFile);

			fread((char *)&FolXYZ[0], sizeof (float), 1, VegFile);
			fread((char *)&FolXYZ[1], sizeof (float), 1, VegFile);
			fread((char *)&FolXYZ[2], sizeof (float), 1, VegFile);

			// optional color and opacity values
			unsigned char ImageColorOpacity = 255, TripleValue[3];
			TripleValue[0] = TripleValue[1] = TripleValue[2] = 0;
			if((Version == 0x01 && (InstanceFlags & 0x03)) || (Version != 0x01 && (InstanceFlags & 0x04)))
				{ // <<<>>> Do something with these -- not sure exactly how yet.
				fread((char *)&ImageColorOpacity, sizeof (char), 1, VegFile);
				fread((char *)&TripleValue[0], sizeof (char), 1, VegFile);
				fread((char *)&TripleValue[1], sizeof (char), 1, VegFile);
				fread((char *)&TripleValue[2], sizeof (char), 1, VegFile);
				} // if
			signed long ObjectFileEntry = -1;
			if(Version != 0x01 && (InstanceFlags & 0x40))
				{ // File entry ID for click-to-query
				fread((char *)&ObjectFileEntry, sizeof (signed long), 1, VegFile);
				} // if

			// IsLabel flag processed below where we already have access to the FolSpecies object

			// create an instance
			PartialVertexDEM FolPlace;

			FolPlace.Lat  = FolXYZ[1] + LocalOffsetLat;  // Lat = Y
			FolPlace.Lon  = -(FolXYZ[0] + LocalOffsetLon);  // Lon = X (negated back into WCS convention)
			FolPlace.Elev = FolXYZ[2] + LocalOffsetElev; // Elev = Z

			// skip parts outside our tile
			if(FolPlace.Lat < MaxYCoord && FolPlace.Lat >= MinYCoord && FolPlace.Lon < MaxXCoord && FolPlace.Lon >= MinXCoord)
				{
				MasterScene.DegToCart(FolPlace);

				// Decode Height
				long HtInterp, HeightBits;
				double HeightNum;
				float FloatHeight;
				HtInterp = ((Height & ~0x3fff) >> 14);
				HeightBits = (Height & 0x3fff);
				HeightNum = (double)HeightBits;
				FloatHeight = (float)(HtInterp == 0x00 ? Height :
				HtInterp == 0x01 ? HeightNum * .1 :
				HtInterp == 0x02 ? HeightNum * .01 :
				HeightNum * .001);

				double TreeHeight = FloatHeight;
				double TreeWidth = TreeHeight * .66f;

				static int TexturingSetup;

				if(TotalTreeCount < MasterScene.SceneLOD.GetMaxFoliageStems())
					{
					// X flip not used right now
					//int XFlip;
					//XFlip = InstanceFlags & 0x01; // X flip

					if(EIDFull < IMGLNumImages) // <<<>>> count from 0 or 1?
						{
						NVFolSpecies *MySpecies;
						if(MySpecies = MasterScene.GetFolSpecies(EIDFull))
							{
							Tree* tree = new Tree;
							tree->_position.set(FolPlace.XYZ[0], FolPlace.XYZ[1], FolPlace.XYZ[2]);
							tree->_color.set(255,255,255,255); // <<<>>> substitute with real color
							tree->_width = FloatHeight * MySpecies->Proportion;
							tree->_height = FloatHeight;
							
							if(ObjectFileEntry != -1)
								{
								tree->_ActionID = ObjectFileEntry;
								} // if

							if(InstanceFlags & 0x08) // we're actually a label
								{
								MySpecies->SetIsLabel(true);
								if(InstanceFlags & 0x10)
									{
									tree->_FlagBits |= Tree::TREE_FLAGBIT_PIVOTLEFT;
									} // if
								if(InstanceFlags & 0x20)
									{
									tree->_FlagBits |= Tree::TREE_FLAGBIT_PIVOTRIGHT;
									} // if
								} // if

							MySpecies->SpeciesInstances.push_back(tree);
							CreatedTreeCount++;
							} // if
						} // if

					} // if
				} // if
			TotalTreeCount++;
			} // for

		fclose(VegFile);
		} // if
	} // if

// record for later debugging use
DebugVarTotalTreeCountFile = TotalTreeCount;
DebugVarTotalTreeCountLoaded += CreatedTreeCount;

VegGroup = new osg::Group;
LabelGroup = new osg::Group;

bool FolQual = false;
FolQual = MasterScene.SceneLOD.CheckFoliageQuality();

// now load images, build states, build forests
if(IMGLNumImages)
	{
	unsigned int CurSpeciesNum;
	NVFolSpecies *CurSpecies;
	for(CurSpeciesNum = 0; CurSpeciesNum < IMGLNumImages; CurSpeciesNum++)
		{
		if(CurSpecies = MasterScene.GetFolSpecies(CurSpeciesNum))
			{
			if(!CurSpecies->SpeciesState.valid()) // this may already be valid from a previous load
				{
				osg::ref_ptr<osg::BlendFunc> transp; // only used for high-quality foliage
				if(CurSpecies->CheckName())
					{
					CurSpecies->SpeciesImage = osgDB::readImageFile(CurSpecies->GetName());
					} // if

				if(CurSpecies->SpeciesImage.valid())
					{
					CurSpecies->SpeciesState = new osg::StateSet;
					osg::ref_ptr<osg::Texture2D> texture = new osg::Texture2D;

					texture->setImage(CurSpecies->SpeciesImage.get());
					if(FolQual)
						{
						texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_NEAREST);
						texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_NEAREST);
						} // if
					else
						{
						texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::LINEAR_MIPMAP_NEAREST);
						texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::LINEAR_MIPMAP_NEAREST);
						//texture->setFilter(osg::Texture::MIN_FILTER, osg::Texture::NEAREST);
						//texture->setFilter(osg::Texture::MAG_FILTER, osg::Texture::NEAREST);
						} // else
					if(MasterScene.SceneLOD.CheckCompressFolTex())
						{
						texture->setInternalFormatMode(osg::Texture::USE_ARB_COMPRESSION);
						} // if
					texture->setWrap( osg::Texture2D::WRAP_S, osg::Texture2D::CLAMP );
					texture->setWrap( osg::Texture2D::WRAP_T, osg::Texture2D::CLAMP );
					CurSpecies->SpeciesState->setTextureAttributeAndModes(0,texture.get(),osg::StateAttribute::ON);
					CurSpecies->SpeciesState->setTextureAttribute(0, new TexEnv );
					CurSpecies->SpeciesState->setMode( GL_LIGHTING, StateAttribute::OFF);
					
					if(FolQual)
						{
						transp = new osg::BlendFunc;
						CurSpecies->SpeciesState->setRenderingHint( StateSet::TRANSPARENT_BIN );
						transp->setSource( osg::BlendFunc::SRC_ALPHA );
						transp->setDestination( osg::BlendFunc::ONE_MINUS_SRC_ALPHA );
						CurSpecies->SpeciesState->setAttributeAndModes( transp.get(), osg::StateAttribute::ON);
						} // if
					CurSpecies->SpeciesState->setMode(GL_DEPTH_TEST, osg::StateAttribute::ON);

					osg::ref_ptr<osg::AlphaFunc> alphaFunc = new osg::AlphaFunc;
					alphaFunc->setFunction(AlphaFunc::GEQUAL,0.05f);
					CurSpecies->SpeciesState->setAttributeAndModes( alphaFunc.get(), StateAttribute::ON);
					} // if SpeciesImage valid
				} // if StateSet not setup already

			if(CurSpecies->SpeciesState.valid())
				{
				CurSpecies->SpeciesForest = new Forest;
				if(CurSpecies->SpeciesForest.valid())
					{
					bool IsLabel = CurSpecies->GetIsLabel();
					double ClipDist;
					if(IsLabel)
						{
						osg::ref_ptr<osg::LOD> lnode;
						ClipDist = LabelClipDist;
						lnode = CurSpecies->SpeciesForest->createLabels(CurSpecies->SpeciesInstances, CurSpecies->SpeciesState.get(), ClipDist);
						if(lnode.valid())
							{
							lnode->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_LABEL);
							LabelGroup->addChild(lnode.get());
							} // if
						} // if Label
					else // !Label
						{
						osg::ref_ptr<osg::Group> gnode;
						ClipDist = FolClipDist;
						gnode = CurSpecies->SpeciesForest->createForest(CurSpecies->SpeciesInstances, CurSpecies->SpeciesState.get(), MasterScene.GetFolType(), ClipDist);
						if(gnode.valid())
							{
							gnode->setNodeMask(NVW_NODEMASK_TANGIBLE | NVW_NODEMASK_FOLIAGE);
							VegGroup->addChild(gnode.get());
							} // if
						} // else !Label
					
					// now clear CurSpecies->SpeciesInstances
					CurSpecies->SpeciesInstances.clear();
					
					} // if CurSpecies->SpeciesForest.valid()
				} // if SpeciesState is valid
			} // if CurSpecies = MasterScene.GetFolSpecies
		} // for CurSpeciesNum
	} // if IMGLNumImages


// we should be able to discard the species's treelists at this point
for(unsigned int CurSpeciesNum = 0; CurSpeciesNum < IMGLNumImages; CurSpeciesNum++)
	{
	NVFolSpecies *CurSpecies;
	if(CurSpecies = MasterScene.GetFolSpecies(CurSpeciesNum))
		{
		if(CurSpecies->SpeciesForest.valid())
			{
			CurSpecies->SpeciesForest = NULL; // should free it, being that it is derived from Referenced
			} // if
		} // if
	} // if


VegParent->addChild(VegGroup);
LabelParent->addChild(LabelGroup);
return(true);

} // CreateVegAndLabels()

