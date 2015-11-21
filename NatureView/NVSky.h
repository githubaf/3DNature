// NVSky.h
// Code relating to the SkyDome
// mostly taken from main.cpp on 1/31/06 by CXH

#ifndef NVW_NVSKY_H
#define NVW_NVSKY_H

#include <osg/Node>
#include <osg/NodeCallback>
#include <osg/NodeVisitor>
#include <osgUtil/CullVisitor>

using namespace osg;

Node *makeSkyandStructure( float SkyRadius);

// some callbacks

struct MyCullCallback : public osg::NodeCallback
{
	 virtual void operator()(Node* node, NodeVisitor* nv)
	{
		osgUtil::CullVisitor* cv = dynamic_cast<osgUtil::CullVisitor*>(nv);
		osgUtil::CullVisitor::ComputeNearFarMode saveComputeNearFar;
		if (cv)
		{
			saveComputeNearFar = cv->getComputeNearFarMode();
			cv->setComputeNearFarMode(
				osgUtil::CullVisitor::COMPUTE_NEAR_FAR_USING_BOUNDING_VOLUMES);
		}

		traverse(node,nv);

		if (cv)
		{
			saveComputeNearFar = cv->getComputeNearFarMode();
			cv->setComputeNearFarMode(saveComputeNearFar);
		}
	}
}; // MyCullCallback


class MoveSkyDomeWithEyePointTransform : public osg::Transform
{
public:
    /** Get the transformation matrix which moves from local coords to world coords.*/
    virtual bool computeLocalToWorldMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const 
    { // http://openscenegraph.org/archiver/osg-users/2005-May/0903.html
		matrix.setTrans(0,0,0);
        return true;
    }

    /** Get the transformation matrix which moves from world coords to local coords.*/
    virtual bool computeWorldToLocalMatrix(osg::Matrix& matrix,osg::NodeVisitor* nv) const
    { // http://openscenegraph.org/archiver/osg-users/2005-May/0903.html
        matrix.setTrans(0,0,0);
        return true;
    }
}; // MoveSkyDomeWithEyePointTransform




#endif // NVW_NVSKY_H
