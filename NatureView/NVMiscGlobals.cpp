
#include <osg/PositionAttitudeTransform>

#include "UsefulTime.h"
#include "NVMiscGlobals.h"
#include "NVScene.h"
#include "NavDlg.h" // to sync Goto widget after delayed goto
#include "KeyDefines.h" // to get Query keycode


static bool DelayedHome, TransitionInProgress, AnimationInProgress, TourInProgress;
static bool DelayedGoto;
extern NVScene MasterScene;

double LastMovementMoment;
unsigned long int TotalNumTriangles = 0;
bool MovementDisabled = false;
bool MovementLockout = false;

int GetTourInProgress(void)
{
return(TourInProgress);
} // GetTourInProgress

void SetTourInProgress(bool NewTourInProgress)
{
TourInProgress = NewTourInProgress;
} // SetTourInProgress



int GetAnimationInProgress(void)
{
return(AnimationInProgress);
} // GetAnimationInProgress

void SetAnimationInProgress(bool NewAnimationInProgress)
{
AnimationInProgress = NewAnimationInProgress;
} // SetAnimationInProgress



int GetTransitionInProgress(void)
{
return(TransitionInProgress);
} // GetTransitionInProgress

void SetTransitionInProgress(bool NewTransitionInProgress)
{
TransitionInProgress = NewTransitionInProgress;
} // SetTransitionInProgress



int GetDelayedHome(void)
{
bool TempDH;
TempDH = DelayedHome;
DelayedHome = 0;
return(TempDH);
} // GetDelayedHome

void SetDelayedHome(bool NewDelayedHome)
{
DelayedHome = NewDelayedHome;
} // SetDelayedHome


int GetDelayedGoto(void)
{
bool TempDG;
TempDG = DelayedGoto;
DelayedGoto = 0;
NavDlg::SyncWidgets();
return(TempDG);
} // GetDelayedGoto

void ToggleDelayedGoto(void)
{
SetDelayedGoto(!GetDelayedGoto());
} // ToggleDelayedGoto


int CheckDelayedGoto(void)
{
return(DelayedGoto);
} // GetDelayedGoto

void SetDelayedGoto(bool NewDelayedGoto)
{
DelayedGoto = NewDelayedGoto;
} // SetDelayedGoto


float GetMovementSpeedGlobal(void)
{
return(MasterScene.GetSpeed());
} // GetMovementSpeedGlobal

float GetMovementMaxSpeedGlobal(void)
{
return(MasterScene.GetMaxSpeed());
} // GetMovementMaxSpeedGlobal

float GetMovementFrictionGlobal(void)
{
return(MasterScene.GetFriction());
} // GetMovementFrictionGlobal

float GetMovementInertiaGlobal(void)
{
return(MasterScene.GetInertia());
} // GetMovementInertiaGlobal

float GetMovementAccelerationGlobal(void)
{
return(MasterScene.GetAcceleration());
} // GetMovementAccelerationGlobal

bool GetMovementLockoutGlobal(void)
{
return(MovementLockout);
} // GetMovementLockoutGlobal

void SetMovementLockoutGlobal(bool NewValue)
{
MovementLockout = NewValue;
} // SetMovementDisabledGlobal


unsigned char GetMovementDisabledGlobal(void)
{
return(MovementDisabled);
} // GetMovementDisabledGlobal

float GetFollowTerrainHeightGlobal(void)
{
return(MasterScene.GetFollowTerrainHeight());
} // GetFollowTerrainHeightGlobal

bool CheckFollowTerrainHeightGlobal(void)
{
return(MasterScene.CheckFollowTerrainHeight());
} // CheckFollowTerrainHeightGlobal

float GetFollowTerrainHeightMaxGlobal(void)
{
return(MasterScene.GetFollowTerrainHeightMax());
} // GetFollowTerrainHeightMaxGlobal

bool CheckFollowTerrainHeightMaxGlobal(void)
{
return(MasterScene.CheckFollowTerrainHeightMax());
} // CheckFollowTerrainHeightMaxGlobal

bool CheckFollowTerrainEnabledGlobal(void)
{
return(MasterScene.CheckFollowTerrainEnabled());
} // CheckFollowTerrainEnabledGlobal

bool GetMovementConstrainGlobal(void)
{
return(MasterScene.GetConstrain());
} // GetMovementConstrainGlobal

float GetMovementXMinGlobal(void)
{
return(MasterScene.GetTerXMin());
} // GetMovementXMinGlobal

float GetMovementXMaxGlobal(void)
{
return(MasterScene.GetTerXMax());
} // GetMovementXMaxGlobal

float GetMovementYMinGlobal(void)
{
return(MasterScene.GetTerYMin());
} // GetMovementYMinGlobal

float GetMovementYMaxGlobal(void)
{
return(MasterScene.GetTerYMax());
} // GetMovementYMaxGlobal

void UpdateLastMovementMoment(void)
{
//time(&LastMovementMoment); // note time of last movement of movement optimization
LastMovementMoment = GetSystemTimeFP();
} // UpdateLastMovementMoment

double GetLastMovementMoment(void)
{
return(LastMovementMoment);
} // GetLastMovementMoment

unsigned long int GetTotalNumTriangles(void)
{
return(TotalNumTriangles);
} // GetTotalNumTriangles

void SetTotalNumTriangles(unsigned long int NewTotal)
{
TotalNumTriangles = NewTotal;
} // SetTotalNumTriangles

