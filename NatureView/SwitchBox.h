// SwitchBox.h

#ifndef NVW_SWITCHBOX_H
#define NVW_SWITCHBOX_H

#include <osg/Switch>

class SwitchBox
	{
	private:
		osg::ref_ptr<osg::Switch> _HUDSwitch, _SplashSwitch;
        osg::ref_ptr<osg::StateSet> _SharedVecState;
        osg::ref_ptr<osg::StateSet> _SharedObjState;

	public:
		
		SwitchBox();
		~SwitchBox();
		
		void EnableHUD(bool NewState);
		void EnableSplash(bool NewState);

		void AddHUD(osg::Node *NewNode) {_HUDSwitch->addChild(NewNode);};
		void AddSplash(osg::Node *NewNode) {_SplashSwitch->addChild(NewNode);};
		
		osg::StateSet *GetSharedVecLightState(void) {return(_SharedVecState.get());};  // Vector lighting
		void SetSharedVecLightState(osg::StateSet *VecState) {_SharedVecState = VecState;};
		osg::StateSet *GetSharedObjLightState(void) {return(_SharedObjState.get());};  // 3D Object lighting
		void SetSharedObjLightState(osg::StateSet *ObjState) {_SharedObjState = ObjState;};

		void AddSwitchBoxToSceneGroups(osg::Group *MasterGroup);

	}; // SwitchBox

#endif // !NVW_SWITCHBOX_H

