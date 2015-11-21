// SwitchBox.cpp

#include <osg/Node>


#include "SwitchBox.h"
#include "NVNodeMasks.h"



SwitchBox::SwitchBox()
{
_HUDSwitch = new osg::Switch;
_SplashSwitch = new osg::Switch;

} // SwitchBox::SwitchBox

SwitchBox::~SwitchBox()
{

// the destruction of the switch objects should happen automatically as they are unref'ed

} // SwitchBox::~SwitchBox


void SwitchBox::AddSwitchBoxToSceneGroups(osg::Group *MasterGroup)
{
// these are children of the Master group, not considered part of the 'scene'
MasterGroup->addChild(_HUDSwitch.get());
MasterGroup->addChild(_SplashSwitch.get());
} // SwitchBox::AddSwitchBoxToSceneGroups






void SwitchBox::EnableHUD(bool NewState)
{
if(_HUDSwitch.valid())
	{
	if(NewState)
		_HUDSwitch->setAllChildrenOn();
	else
		_HUDSwitch->setAllChildrenOff();
	} // if
} // SwitchBox::EnableHUD

void SwitchBox::EnableSplash(bool NewState)
{
if(_SplashSwitch.valid())
	{
	if(NewState)
		_SplashSwitch->setAllChildrenOn();
	else
		_SplashSwitch->setAllChildrenOff();
	} // if
} // SwitchBox::EnableSplash




