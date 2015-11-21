
#include <sstream>

#include <osg/PositionAttitudeTransform>

#include <osgDB/Registry>


#include "NVScene.h"
#include "SwitchBox.h"
#include "Viewer.h"
#include "CameraSupport.h"
#include "NVTerrain.h"
#include "NVMiscGlobals.h"
#include "FrameStats.h"

extern SwitchBox *MasterSwitches;
extern NVFrameStats FrameStatisticsCounter;

extern NVScene MasterScene;
extern double DXG, DYG, MiscReadout, MiscReadoutTwo;
extern double TargetFramerate;
extern double CurrentInstantFrameRate;

char ThreadDebugMsg[160];
char ThreadLoadMsg[160];


int NVScene::SelectNextCamera(int HowManyNext)
{
int CurCam, NewCam;
CurCam = GetCurrentCameraNum();
NewCam = CurCam + HowManyNext;
if(NewCam > (GetNumCameras() - 1))
	{
	NewCam = 0; // wrap around
	} // if
return(SetCurrentCameraNum(NewCam));
} // NVScene::SelectNextCamera

int NVScene::SelectPrevCamera(int HowManyPrev)
{
int CurCam, NewCam;
CurCam = GetCurrentCameraNum();
NewCam = CurCam - HowManyPrev;
if(NewCam < 0)
	{
	NewCam = (GetNumCameras() - 1); // wrap around
	} // if

return(SetCurrentCameraNum(NewCam));
} // NVScene::SelectPrevCamera


int NVScene::SetCurrentCameraNum(int NewCamNum)
{
int NewCameraNum = max(0, (min(NewCamNum, (GetNumCameras() - 1))));
if(NewCameraNum != CurrentCameraNum)
	{
	CurrentCameraNum = NewCameraNum;
	SyncAnimFlag();
	return(1);
	} // if
else
	{
	return(0);
	} // else
} // NVScene::SetCurrentCameraNum


void NVScene::SyncAnimFlag(void)
{
NVAnimObject *CurCam;
if(CurCam = GetCamera(GetCurrentCameraNum()))
	{
	SetAnimationInProgress(CurCam->GetIsAnimated());
	} // if
} // NVScene::SyncAnimFlag




void NVScene::UpdateHUD()
{

if(1) // no longer care about HUDSwitch -- we don't work that way anymore
	{
	if(1)
		{
		osg::Vec3 Eye, Center, Up;
		PartialVertexDEM CamLoc;
		GetGlobalViewer()->getSceneHandlerList()[0]->getSceneView()->getViewMatrixAsLookAt(Eye, Center, Up);
		CamLoc.XYZ[0] = Eye.x();
		CamLoc.XYZ[1] = Eye.y();
		CamLoc.XYZ[2] = Eye.z();
		CartToDeg(CamLoc);
		if(Overlay.GetOverlayTextTemplate().empty())
			{
			std::ostringstream NewOverlayText;
			if(ThreadDebugMsg[0])
				{
				NewOverlayText << ThreadDebugMsg;
				} // if
			else
				{
				double ElevAGL = -FLT_MAX;
				NewOverlayText.precision(8);
				if(GetNumCameras()) // prevent null pointers when no cameras defined
					{
					NewOverlayText << "Camera: " << GetCameraName(GetCurCameraNum()) << "\n";
					} // if
				else
					{
					NewOverlayText << "\n"; // no camera info, insert blank line
					} // else
				if(fabs(CamLoc.Lat) < 0.00000001)  CamLoc.Lat = 0; // truncate silly scientific notation epsilons...
				if(fabs(CamLoc.Lon) < 0.00000001)  CamLoc.Lon = 0; // truncate silly scientific notation epsilons...
				if(fabs(CamLoc.Elev) < 0.00000001) CamLoc.Elev = 0; // truncate silly scientific notation epsilons...
				NewOverlayText << "  Latitude: ";
				NewOverlayText << (CamLoc.Lat < 0 ? "S" : "N");
				NewOverlayText << fabs(CamLoc.Lat);
				NewOverlayText << "\n  Longitude: ";
				NewOverlayText << (CamLoc.Lon < 0 ? "E" : "W");
				NewOverlayText << fabs(CamLoc.Lon);
				NewOverlayText.precision(6);
				NewOverlayText <<  "\n  Elevation: " << CamLoc.Elev;
				if(FetchTerrainHeight(NULL, CamLoc.Lon, CamLoc.Lat, ElevAGL))
					{
					NewOverlayText <<  " (" << (CamLoc.Elev - ElevAGL) << " AGL)";
					} // if
				//NewOverlayText << "\nDX: " << DXG << " DY: " << DYG << " V: " << MiscReadout;
				//NewOverlayText << "\nDebug: " << MiscReadout << ", " << MiscReadoutTwo;
				//NewOverlayText << "\nDebug: " << MiscReadout;
				} // else
			

			// handle appending framerate info
			if(MasterScene.SceneLOD.GetShowFramerate())
				{
				if(1) // to preserve indent
					{
					double AvgTime = 0.01f; // not 0 to prevent divide by zero
					char FormatStr[200];
					if(FrameStatisticsCounter.QueryAdequateSamples())
						{
						AvgTime = FrameStatisticsCounter.FetchAverageTime();
						} // if
					FormatStr[0] = NULL;

					// can't get the cout formatter to do what I want, here goes sprintf!
					//sprintf(FormatStr, "%#04.1f", (1.0f / TimeDelta));
					NewOverlayText << "\nF:" << FormatStr;

					if(AvgTime > 0.0)
						{
						sprintf(FormatStr, "%#04.1f", (1.0f / AvgTime));
						NewOverlayText << " A:" << FormatStr;
						} // if

					//sprintf(FormatStr, "%d", (long int)TargetFramerate);
					//NewOverlayText << " T:" << FormatStr;

					// <<<>>> Debug: CurrentInstantFrameRate
					sprintf(FormatStr, "%#04.1f", CurrentInstantFrameRate);
					NewOverlayText << " C:" << FormatStr;
					
					sprintf(FormatStr, "%#4.2f", MasterScene.SceneLOD.GetLODAutoFactor());
					NewOverlayText << " D:" << FormatStr;

					unsigned long int TotPoly;
					TotPoly = GetTotalNumTriangles();
					if(TotPoly != 0)
						{
						sprintf(FormatStr, "%d", TotPoly);
						NewOverlayText << " P:" << FormatStr;
						} // if


					bool NewLineSent = false;
					// append pager info
					osgDB::DatabasePager* dp = osgDB::Registry::instance()->getDatabasePager();
					if(dp)
						{
						if(dp->getFileRequestListSize() > 0 || dp->getDataToCompileListSize() > 0)
							{
							if(!NewLineSent)
								{
								NewOverlayText << "\n";
								NewLineSent = true;
								} // if
							NewOverlayText << "L: " << dp->getFileRequestListSize();
							NewOverlayText << " C: " << dp->getDataToCompileListSize() << " ";
							} // if
						} // if
					if(ThreadLoadMsg[0])
						{
						if(!NewLineSent)
							{
							NewOverlayText << "\n";
							//NewLineSent = true;  // who cares at this point?
							} // if
						NewOverlayText << "[" << ThreadLoadMsg << "] ";
						} // if


					} // if
				} // if

			Overlay.SetOverlayText(NewOverlayText.str());
			} // if
		else
			{
			Overlay.SetOverlayText(Overlay.GetProcessedText());
			} // else
		if(Overlay.GetMapOn())
			{
			float Heading = 0;
			GetCurCameraCurHeadingRadians(Heading);
			Overlay.UpdateMapPoint(CamLoc.Lon, CamLoc.Lat, Heading); // Heading is in radians
			} // if
		} // if
	} // if

} // NVScene::UpdateHUD



void NVScene::AddFolSpecies(NVFolSpecies *NewSpecies)
{

FolSpecies.push_back(NewSpecies);

} // NVScene::AddFolSpecies

