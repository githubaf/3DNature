
#ifndef NVW_NVFOLIAGE_H
#define NVW_NVFOLIAGE_H

#include <vector>
#include <osg/ref_ptr>

#include <osg/Referenced> 
#include <osg/Vec3> 
#include <osg/Vec4ub> 
#include <osg/BoundingBox>
#include <osg/Geometry>
#include <osg/Switch>

bool CreateVegAndLabels(const char *VegFileName, const char *VegFileSig, double MinXCoord, double MaxXCoord, double MinYCoord, double MaxYCoord, osg::Group *VegParent, osg::Group *LabelParent);

// a Geometry object with a set/get-able ActionID member
class ActionedGeometry : public osg::Geometry
	{
	private:
		unsigned long int _ActionID;
	public:
		ActionedGeometry():
			_ActionID(-1){} // the new C++ way of initializing in constructors...
		void SetActionID(signed long int NewActionID) {_ActionID = NewActionID;};
		unsigned long int GetActionID(void) {return(_ActionID);};
	}; // ActionedGeometry

class Tree : public osg::Referenced
{
public:

    Tree():
        _color(255,255,255,255),
        _width(1.0f),
        _height(1.0f),
        _ActionID(-1),
		_FlagBits(0){}

    Tree(const osg::Vec3& position, const osg::Vec4ub& color, float width, float height, unsigned int type, unsigned long int FlagBits = 0):
        _position(position),
        _color(color),
        _width(width),
        _height(height),
		_FlagBits(0){}

    osg::Vec3       _position;
    osg::Vec4ub     _color;
    float           _width;
    float           _height;
	unsigned long int _FlagBits;
	signed long int _ActionID;

	// FlagBits
	enum // when used, these need a Tree:: prefix
		{
		//TREE_FLAGBIT_PIVOTCENTER is implied by no bits
		// Pivot flags only honored by Label builder
		TREE_FLAGBIT_PIVOTLEFT = 0x01,
		TREE_FLAGBIT_PIVOTRIGHT = 0x02,
		TREE_FLAGBIT_MAXIMUM // avoid comma propogation, never actually used
		}; // Flagbits

}; // class tree

typedef std::vector< osg::ref_ptr<Tree> > TreeList;



// class to create a forest
class Forest : public osg::Referenced
{
public:

    Forest() {}
    
    class Cell : public osg::Referenced
    {
    public:
        typedef std::vector< osg::ref_ptr<Cell> > CellList;

        Cell():_parent(0) {}
        Cell(osg::BoundingBox& bb):_parent(0), _bb(bb) {}
        
        void addCell(Cell* cell) { cell->_parent=this; _cells.push_back(cell); }

        void addTree(Tree* tree) { _trees.push_back(tree); }
        
        void addTrees(const TreeList& trees) { _trees.insert(_trees.end(),trees.begin(),trees.end()); }
        
        void computeBound();
        
        bool contains(const osg::Vec3& position) const { return _bb.contains(position); }
        
        bool divide(unsigned int maxNumTreesPerCell=10);
        
        bool devide(bool xAxis, bool yAxis, bool zAxis);
        
        void bin();


        Cell*               _parent;
        osg::BoundingBox    _bb;
        CellList            _cells;
        TreeList            _trees;
        
    };

    float random(float min,float max) { return min + (max-min)*(float)rand()/(float)RAND_MAX; }
    //int random(int min,int max) { return min + (int)((float)(max-min)*(float)rand()/(float)RAND_MAX); }

	// Used by CreateSprite
	enum PivotAxis // when used, these need a Forest:: prefix
		{
		TREE_PIVOTAXIS_CENTER = 0,
		TREE_PIVOTAXIS_LEFT,
		TREE_PIVOTAXIS_RIGHT,
		TREE_PIVOTAXIS_MAXIMUM // avoid comma propogation, never actually used
		}; // PivotAxis

    osg::Geometry* createSprite( float w, float h, osg::Vec4ub color, enum Forest::PivotAxis Swivel = Forest::TREE_PIVOTAXIS_CENTER, signed long int ActionID = -1);

    osg::Geometry* createOrthogonalQuads( const osg::Vec3& pos, float w, float h, osg::Vec4ub color, signed long int ActionID = -1);

    osg::Node* createBillboardGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist);
    osg::Node* Forest::createBillboardGraph(TreeList &trees, osg::StateSet* stateset, double FolClipDist);

    osg::Node* createLabelBillboardGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist);

    osg::Node* createXGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist);

    osg::Node* createTransformGraph(Cell* cell,osg::StateSet* stateset, double FolClipDist);
    
    osg::ref_ptr<osg::Group> createForest(TreeList &trees, osg::StateSet *dstate, int TreeType, double ClipDist);
    
    osg::ref_ptr<osg::LOD> createLabels(TreeList &trees, osg::StateSet *dstate, double ClipDist);



};



class NVFolSpecies
	{
	public:
		std::string ImgFileName;
		float Proportion;
		unsigned char Flags; // Flag 0x01: Is actually a Label
		osg::ref_ptr<osg::Image> SpeciesImage;
		osg::ref_ptr<osg::StateSet> SpeciesState;
		TreeList SpeciesInstances;
		osg::ref_ptr<Forest> SpeciesForest;

		NVFolSpecies() {Flags = 0; Proportion = 1.0;};

		void SetName(char *NewName) {ImgFileName = NewName;};
		std::string GetName(void) {return(ImgFileName);};
		int CheckName(void) {return(!ImgFileName.empty());};

		void SetIsLabel(bool IsLabel) {if(IsLabel) Flags |= 0x01; else Flags &= (~0x01);};
		bool GetIsLabel(void) {return(Flags & 0x01 ? 1 : 0);};
		
	}; // NVFolSpecies

#endif // !NVW_NVFOLIAGE_H
