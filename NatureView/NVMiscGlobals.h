

#ifndef NVW_NVMISCGLOBALS_H
#define NVW_NVMISCGLOBALS_H


int GetAnimationInProgress(void);
void SetAnimationInProgress(bool NewAnimationInProgress = true);

int GetTourInProgress(void);
void SetTourInProgress(bool NewTourInProgress);



int GetTransitionInProgress(void);
void SetTransitionInProgress(bool NewTransitionInProgress = true);

int GetDelayedHome(void);
void SetDelayedHome(bool NewDelayedHome = true);

int GetDelayedGoto(void); // clears goto state
int CheckDelayedGoto(void); // doesn't clear Goto state
void SetDelayedGoto(bool NewDelayedGoto = true);
void ToggleDelayedGoto(void);


float GetMovementSpeedGlobal(void);
float GetMovementMaxSpeedGlobal(void);
float GetMovementFrictionGlobal(void);
float GetMovementInertiaGlobal(void);
float GetMovementAccelerationGlobal(void);
unsigned char GetMovementDisabledGlobal(void);
float GetFollowTerrainHeightGlobal(void);
bool CheckFollowTerrainHeightGlobal(void);
float GetFollowTerrainHeightMaxGlobal(void);
bool CheckFollowTerrainHeightMaxGlobal(void);
bool CheckFollowTerrainEnabledGlobal(void);
bool GetMovementConstrainGlobal(void);
float GetMovementXMinGlobal(void);
float GetMovementXMaxGlobal(void);
float GetMovementYMinGlobal(void);
float GetMovementYMaxGlobal(void);
void UpdateLastMovementMoment(void);
double GetLastMovementMoment(void);

bool GetMovementLockoutGlobal(void);
void SetMovementLockoutGlobal(bool NewValue);

unsigned long int GetTotalNumTriangles(void);
void SetTotalNumTriangles(unsigned long int NewTotal);


#endif // !NVW_NVMISCGLOBALS_H