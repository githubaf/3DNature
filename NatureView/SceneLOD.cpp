// SceneLOD.cpp

#include "SceneLOD.h"
#include "NVNodeMasks.h"
#include "NVMiscGlobals.h"

#include <windows.h>
#include <osg/Image>

#include "HelpDlg.h"
#include "SwitchBox.h"

extern HelpDlg *GlobalHelpDlg;

extern time_t HelpAppear;

extern SwitchBox *MasterSwitches;

void NVLOD::EnableHelp(bool NewState)
	{
	_HelpShown = NewState;

	if(_HelpShown)
		{
		if(!GlobalHelpDlg)
			{
			GlobalHelpDlg = new HelpDlg;
			time(&HelpAppear); // note time it appeared for timeout auto-hide
			} // if
		} // if
	else
		{
		if(GlobalHelpDlg)
			{
			delete GlobalHelpDlg;
			GlobalHelpDlg = NULL;
			} // if
		} // else
	}; // NVLOD::EnableHelp (positive sense)

osg::Node::NodeMask NVLOD::GetCurrentEnabledNodeMask(void)
{
osg::Node::NodeMask Result = 0;

if(!_FoliageOptimizeHidden)
	{ // show it depending on the user-selected state
	if(_FoliageHidden)
		{
		// don't enable FOLIAGE mask
		} // if
	else
		{
		Result |= NVW_NODEMASK_FOLIAGE;
		} // else
	} // if
else
	{  // hide it temporarily, even if enabled
	// don't enable FOLIAGE mask
	} // else

//Result |= NVW_NODEMASK_INTANGIBLE; // pick up all the other stuff, like Sky

if(CheckSkyEnabled()) Result |= NVW_NODEMASK_SKY;
if(CheckTerrainEnabled()) Result |= NVW_NODEMASK_TERRAIN;
if(CheckObjectsEnabled()) Result |= NVW_NODEMASK_STRUCT;
if(CheckLabelsEnabled()) Result |= NVW_NODEMASK_LABEL;
if(CheckVecsEnabled()) Result |= NVW_NODEMASK_VECTOR;
if(CheckOceanEnabled()) Result |= NVW_NODEMASK_OCEAN;

return(Result);

} // NVLOD::GetCurrentEnabledNodeMask

double NVLOD::GetSceneSettledTime(void)
{
return(GetSystemTimeFP() - GetLastMovementMoment());
} // NVLOD::GetSceneSettledTime
