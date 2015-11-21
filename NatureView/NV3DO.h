// NV3DO.h
// 3D Object instance loader
// created by CXH on 11/17/05 from code from main.cpp

#ifndef NVW_NV3DO_H
#define NVW_NV3DO_H

#include <osg/Group>

class NVAnimObject;

osg::Node *CreateObj(NVAnimObject *ObjInstance, unsigned long int ObjVerifiedCount, double ObjClipDist, double ObjBoxDist);
void InitObjectCreationState(void);
int CreateObjects(double MinXCoord, double MaxXCoord, double MinYCoord, double MaxYCoord, osg::Group *ObjVecParent);

#endif // NVW_NV3DO_H
