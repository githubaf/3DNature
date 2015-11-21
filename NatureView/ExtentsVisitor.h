
#ifndef NVW_EXTENTSVISITOR_H
#define NVW_EXTENTSVISITOR_H

#include <osg/NodeVisitor>
#include <osg/BoundingBox>
#include <osg/Geode>
#include <osg/MatrixTransform>

//////////////////////////////////////////////////////////////////////////////
// class ExtentsVisitor
//
// description: visit all nodes and compute bounding box extents
//
// from
// Jeff Biggs  osg-user@dburns.dhs.org
// Thu, 06 Nov 2003 09:16:44 -0600 (CST)
class ExtentsVisitor : public osg::NodeVisitor
{
public:

    ExtentsVisitor():NodeVisitor(NodeVisitor::TRAVERSE_ALL_CHILDREN) {}
        // constructor
    ~ExtentsVisitor()  {}
        // destructor

    virtual void apply(osg::Geode &node) 
    {
        osg::BoundingBox bb;
		unsigned int i;
        // update bounding box
        for (i = 0; i < node.getNumDrawables(); ++i) {
            // expand overall bounding box
            bb.expandBy(node.getDrawable(i)->getBound());
        }

        osg::BoundingBox xbb;
        // transform corners by current matrix
        for (i = 0; i < 8; ++i) {
            osg::Vec3 xv = bb.corner(i) * m_TransformMatrix;
            xbb.expandBy(xv);    
        }

        // update overall bounding box size
        m_BoundingBox.expandBy(xbb);

        // continue traversing the graph
        traverse(node);
    }
        // handle geode drawable extents to expand the box

    virtual void apply(osg::MatrixTransform &node)
    {
        m_TransformMatrix *= node.getMatrix();

        // continue traversing the graph
        traverse(node);

    }
        // handle transform to expand bounding box

    osg::BoundingBox &GetBound() { return m_BoundingBox; }
        // return bounding box

protected:

    osg::BoundingBox m_BoundingBox;
        // bound box
    osg::Matrix m_TransformMatrix;
        // current transform matrix

};


#endif // !NVW_EXTENTSVISITOR_H
