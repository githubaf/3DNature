// NVLights.h
// Lighting code, broken out of main.cpp on 1/31/06 by CXH

#ifndef NVW_LIGHTS_H
#define NVW_LIGHTS_H

#include <osg/Group>
#include <osg/StateSet>

bool BuildLights(void);
int CreateLights(osg::Group *LightParent, osg::StateSet *LightingStateSet);
int CreateDefaultLights(void);


#endif // NVW_LIGHTS_H
