// ToolbarButler.cpp
// Module Control Code for Toolbar
// Built from Toolbar.cpp on 10/27/99 by Chris "Xenon" Hanson
// Copyright 1995-1999. All rights reserved.

#include "stdafx.h"
#include "Security.h"
#include "Types.h"
#include "Toolbar.h"
#include "Project.h"
#include "ProjectDispatch.h"
#include "Database.h"
#include "ColorEditGUI.h"
#include "DBEditGUI.h"
#include "EffectListGUI.h"
#include "CloudEditGUI.h"
#include "WaveEditGUI.h"
#include "KeyScaleDeleteGUI.h"
#include "SceneExportGUI.h"
#include "EcotypeEditGUI.h"
#include "ProjectPrefsGUI.h"
#include "InterpDEMGUI.h"	// don't open without project loaded
#include "ProjNewGUI.h"
#include "DigitizeGUI.h"	// don't open without project loaded
#include "ViewGUI.h"
#include "DiagnosticGUI.h"
#include "VersionGUI.h"
#include "ProjUpdateGUI.h"
#include "AuthorizeGUI.h"
//#include "CreditsGUI.h" // retired
#include "SceneImportGUI.h"	// don't open without project loaded
#include "EffectsLibGUI.h"
#include "LakeEditGUI.h"
#include "EcosystemEditGUI.h"
#include "RasterTAEditGUI.h"
#include "TerraffectorEditGUI.h"
#include "ShadowEditGUI.h"
#include "FoliageEffectEditGUI.h"
#include "FoliageEffectFolFileEditGUI.h"
#include "StreamEditGUI.h"
#include "Object3DEditGUI.h"
#include "MaterialEditGUI.h"
#include "ImportWizGUI.h"	// don't open without project loaded
#include "Conservatory.h"
#include "Notify.h"
#include "Useful.h"
#include "Log.h"
#include "EffectsLib.h"
#include "LightEditGUI.h"
#include "CameraEditGUI.h"
#include "TerrainParamEditGUI.h"
#include "SkyEditGUI.h"
#include "CelestialEditGUI.h"
#include "VectorEditGUI.h"
#include "VectorProfGUI.h"
#include "ImageLibGUI.h"
#include "AnimGraphGUI.h"
#include "TextureEditGUI.h"
#include "Texture.h"
#include "SceneViewGUI.h"
#include "DragnDropListGUI.h"
#include "AtmosphereEditGUI.h"
#include "StarfieldEditGUI.h"
#include "CmapEditGUI.h"
#include "EnvironmentEditGUI.h"
#include "GroundEditGUI.h"
#include "PlanetOptEditGUI.h"
#include "RenderJobEditGUI.h"
#include "RenderOptEditGUI.h"
#include "SnowEditGUI.h"
#include "MaterialStrataEditGUI.h"
#include "GalleryGUI.h"
#include "BrowseDataGUI.h"
#include "ImageViewGUI.h"
#include "RenderControlGUI.h"
#include "SunPosGUI.h"
#include "Requester.h"
#include "NumericEntryGUI.h"
#include "TerraGridderEditGUI.h"
#include "TerraGeneratorEditGUI.h"
#include "SearchQueryEditGUI.h"
#include "ThematicMapEditGUI.h"
#include "CoordSysEditGUI.h"
#include "TableListGUI.h"
#include "DEMEditGUI.h"
#include "DEMPaintGUI.h"
#include "FenceEditGUI.h"
#include "TemplateManGUI.h"
#include "PathTransferGUI.h"
#include "EDSSControlGUI.h"
#include "PostProcEditGUI.h"
#include "DEMMergeGUI.h"
#include "ScenarioEditGUI.h"
#include "VecProfExportGUI.h"
#include "CoordsCalculatorGUI.h"
#include "DrillDownInfoGUI.h"
#ifdef WCS_FORESTRY_WIZARD
#include "ForestWizGUI.h"
#endif // WCS_FORESTRY_WIZARD
#include "MergerWizGUI.h"
#include "GridderWizGUI.h"
#ifdef WCS_BUILD_RTX
#include "ExporterEditGUI.h"
#include "ExportControlGUI.h"
#endif // WCS_BUILD_RTX
#include "LabelEditGUI.h"
// <<<>>> ADD_NEW_EFFECTS

//extern char BusyWinAbort;


void Toolbar::ConfigureMatrix(void)
{
GlobalApp->MainProj->ViewPorts.SetCurrent(GlobalApp->MainProj->Prefs.GUIConfiguration);
if (GlobalApp->MainProj)
	GlobalApp->MainProj->Relayout();
if (GlobalApp->WinSys)
	{
	GlobalApp->WinSys->UpdateDocking(0);
	} // 

} // Toolbar::ConfigureMatrix

/*===========================================================================*/

void Toolbar::HandleNotifyEvent(void)
{
NotifyTag ChangeTag;
unsigned char It;
unsigned char OpenState, Action, Item, Page, AnimConfigured = 0;
char ObjDescr[256], *NameTest = NULL;
NotifyTag Changed, *Changes, MatrixInterested[2], NameChangedInterested[2], AnimChangedInterested[5];
RasterAnimHost *CurActive;

Changes = Activity->ChangeNotify->ChangeList;

// matrix change
MatrixInterested[0] = MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0);
MatrixInterested[1] = NULL;
NameChangedInterested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_NAMECHANGED);
NameChangedInterested[1] = NULL;
AnimChangedInterested[0] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_POSITIONCHANGED);
AnimChangedInterested[1] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_NODEADDED);
AnimChangedInterested[2] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_ANIM_NODEREMOVED);
AnimChangedInterested[3] = MAKE_ID(0xff, 0xff, 0xff, WCS_NOTIFYCOMP_OBJECT_CHANGED);
AnimChangedInterested[4] = NULL;

/* // old style
if (Activity->ChangeNotify->ChangeList[0] == MAKE_ID(WCS_PROJECTCLASS_PREFS, WCS_SUBCLASS_PROJPREFS_CONFIG, WCS_PROJPREFS_GUICONFIG, 0))
	{
	ConfigureMatrix();
	} // if
*/

if (Changed = GlobalApp->AppEx->MatchNotifyClass(MatrixInterested, Changes, 0))
	{
	ConfigureMatrix();
	} // if
else if (Changed = GlobalApp->AppEx->MatchNotifyComponent(AnimChangedInterested, Changes, 0))
	{
	ConfigureAnimWidgets();
	} // if
// this matching method matches on a 0xffnnnnnn, which can come from a variety of sources. We need a more specific match
//else if (Changed = GlobalApp->AppEx->MatchNotifyClass(DelayEditInterested, Changes, 0))
else if(NOTIFYCLASSPART(Changes[0]) == WCS_NOTIFYCLASS_DELAYEDEDIT)
	{
	// perform delayed edit (assuming it's the first notify in the ChangeList, Changes[0]
	AppScope->AppEffects->ReGenerateDelayedEditNext(NOTIFYSUBCLASSPART(Changes[0]), Activity->ChangeNotify->NotifyData);
	} // if
else if (GlobalApp->AppEx->MatchNotifyComponent(NameChangedInterested, Changes, 0))
	{
	if (CurActive = RasterAnimHost::GetActiveRAHost())
		{
		if (CurActive->GetRAHostName())
			{
			strcpy(ObjDescr, CurActive->GetRAHostName());
			} // if
		if (CurActive->RAParent)
			{
			if (CurActive->RAParent->RAParent && (NameTest = CurActive->RAParent->RAParent->GetCritterName(CurActive->RAParent)) && NameTest[0])
				{
				if (strcmp(NameTest, ObjDescr))
					{
					strcat(ObjDescr, " ");
					strcat(ObjDescr, NameTest);
					} // if 
				} // if
			NameTest = CurActive->RAParent->GetCritterName(CurActive);
			if (NameTest && NameTest[0])
				{
				strcat(ObjDescr, " ");
				strcat(ObjDescr, NameTest);
				} // if
			} // if
		if ((! NameTest || ! NameTest[0]) && CurActive->GetRAHostTypeString())
			{
			strcat(ObjDescr, " ");
			strcat(ObjDescr, CurActive->GetRAHostTypeString());
			} // if
		SetCurrentObjectText(ObjDescr);
		} // if
	} // else if component name changed
else if (Changes)
	{
	for(It = 0; ChangeTag = Changes[It]; It++)
		{
		//Action    = NOTIFYCLASSPART(ChangeTag);
		//Action = ((unsigned char)((((unsigned long int)ChangeTag) & 0xff000000) >> 24));
		Action    = (unsigned char)(ChangeTag >> 24);
		Item      = NOTIFYITEMPART(ChangeTag);
		OpenState = NOTIFYSUBCLASSPART(ChangeTag);
		Page      = NOTIFYCOMPONENTPART(ChangeTag);
		switch(Action)
			{
			case WCS_NOTIFYCLASS_FREEZE:
				{
				if (OpenState == WCS_NOTIFYSUBCLASS_THAW)
					{
					ConfigureAnimWidgets();
					AnimConfigured = 1;
					} // if
				break;
				} // WCS_NOTIFYCLASS_FREEZE
			case WCS_PROJECTCLASS_PREFS:
				{
				if (! AnimConfigured)
					{
					ConfigureAnimWidgets();
					AnimConfigured = 1;
					} // if
				break;
				} // WCS_PROJECTCLASS_PREFS
			case WCS_PROJECTCLASS_PATHS:
				{
				UpdateRecentProjects();
				break;
				} // WCS_PROJECTCLASS_PATHS
			case WCS_INTERCLASS_TIME:
				{
				FrameNumber();
				break;
				} // WCS_INTERCLASS_TIME
			case WCS_NOTIFYCLASS_EFFECTS:
				{
				ConfigureAnimWidgets();
				AnimConfigured = 1;
				break;
				} // WCS_NOTIFYCLASS_EFFECTS
			case WCS_TOOLBARCLASS_MODULES:
				{
				HandleModule(Item, OpenState, Page);
				break;
				} // WCS_TOOLBARCLASS_MODULES
			} // switch Action
		} // for
	} // if
} // Toolbar::HandleNotifyEvent

/*===========================================================================*/

//lint -save -e648
long Toolbar::HandleEvent(void)
{
static int Count;

#ifdef _WIN32
if (Activity->Type == WCS_APP_EVENTTYPE_MSWIN)
	{
	switch(Activity->GUIMessage.message)
		{
		case WM_SYSCOMMAND:
			{
			return(0);
			} // WM_SYSCOMMAND
		case WM_NOTIFY:
			{
			LPNMHDR nmh = (LPNMHDR)Activity->GUIMessage.lParam;
			if (nmh->code == TBN_GETINFOTIP)
				{ // force tooltip to TOPMOST since doing so at creation time doesn't seem to work on pre-XP
				HWND TTip;
				TTip = (HWND)SendMessage(nmh->hwndFrom, TB_GETTOOLTIPS, 0, 0);
				if (TTip)
					{
					SetWindowPos(TTip, HWND_TOPMOST,0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
					} // if
				} // else if
			return(0);
			} // WM_NOTIFY
		} // switch
	} // if

return(0); // Indicates we didn't deal with the message.

#else // _WIN32

return(0);

#endif // _WIN32
} // Toolbar::HandleEvent
//lint -restore

/*===========================================================================*/

long Toolbar::HandleModule(unsigned char Item, unsigned char OpenState, unsigned char Page)
{
long Result = 0;
HWND CurrentActive = NULL;

if(!OpenState) // are we closing a window? We might encounter a nasty window deactivation issue that kills kb input
	{
	CurrentActive = GetActiveWindow(); // this is used to record the currently active window before closing a window
	SetActiveWindow(GlobalApp->WinSys->RootWin); // we activate our RootWin temporarily during the close, which seems to help
	} // if

if (Item == WCS_TOOLBAR_ITEM_PPG)
	{
	if (OpenState)
		{
		if (!AppScope->GUIWins->PPG)
			{
			if (AppScope->GUIWins->PPG = new ProjectPrefsGUI(AppScope->MainProj, AppScope->MainProj->Interactive))
				{
				if (AppScope->GUIWins->PPG->ConstructError)
					{
					delete AppScope->GUIWins->PPG;
					AppScope->GUIWins->PPG = NULL;
					} // if
				} // if
			} // if
		if (AppScope->GUIWins->PPG)
			{
			AppScope->GUIWins->PPG->Open(AppScope->MainProj);
			} // if
		} // if
	else
		{
		if (AppScope->GUIWins->PPG)
			{
			delete AppScope->GUIWins->PPG;
			AppScope->GUIWins->PPG = NULL;
			} // if
		} // else
	Result = 1;
	} // else if
else if (Item == WCS_TOOLBAR_ITEM_VER)
	{
	if (OpenState)
		{
		if (!AppScope->GUIWins->VER)
			{
			AppScope->GUIWins->VER = new VersionGUI();
			} // if
		if (AppScope->GUIWins->VER)
			{
			AppScope->GUIWins->VER->Open(AppScope->MainProj);
			}
		} // if
	else
		{
		if (AppScope->GUIWins->VER)
			{
			delete AppScope->GUIWins->VER;
			AppScope->GUIWins->VER = NULL;
			} // if
		} // else
	Result = 1;
	} // else if WCS_TOOLBAR_ITEM_VER
else if (Item == WCS_TOOLBAR_ITEM_PUW)
	{
	if (OpenState)
		{
		if (!AppScope->GUIWins->PUG)
			{
			AppScope->GUIWins->PUG = new ProjUpdateGUI();
			} // if
		if (AppScope->GUIWins->PUG)
			{
			if(Page == 1) // otherwise create-but-don't-display-yet
				{
				AppScope->GUIWins->PUG->Open(AppScope->MainProj);
				} // if
			}
		} // if
	else
		{
		if (AppScope->GUIWins->PUG)
			{
			delete AppScope->GUIWins->PUG;
			AppScope->GUIWins->PUG = NULL;
			} // if
		} // else
	Result = 1;
	} // else if WCS_TOOLBAR_ITEM_PUW
else if (Item == WCS_TOOLBAR_ITEM_AUT)
	{
	if (OpenState)
		{
		if (!AppScope->GUIWins->AUT)
			{
			AppScope->GUIWins->AUT = new AuthorizeGUI();
			} // if
		if (AppScope->GUIWins->AUT)
			{
			AppScope->GUIWins->AUT->Open(AppScope->MainProj);
			}
		} // if
	else
		{
		if (AppScope->GUIWins->AUT)
			{
			delete AppScope->GUIWins->AUT;
			AppScope->GUIWins->AUT = NULL;
			} // if
		} // else
	Result = 1;
	} // else if WCS_TOOLBAR_ITEM_VER
/*
else if (Item == WCS_TOOLBAR_ITEM_CRG)
	{
	if (OpenState)
		{
		if (!AppScope->GUIWins->CRG)
			{
			AppScope->GUIWins->CRG = new CreditsGUI();
			} // if
		if (AppScope->GUIWins->CRG)
			{
			AppScope->GUIWins->CRG->Open(AppScope->MainProj);
			}
		} // if
	else
		{
		if (AppScope->GUIWins->CRG)
			{
			delete AppScope->GUIWins->CRG;
			AppScope->GUIWins->CRG = NULL;
			} // if
		} // else
	Result = 1;
	} // else if WCS_TOOLBAR_ITEM_CRG
*/
else if (Item == WCS_TOOLBAR_ITEM_RJG)
 	{
	if (OpenState)
 		{
 		if (!AppScope->GUIWins->RJG)
 			{
 			if (AppScope->GUIWins->RJG = new RenderJobEditGUI(AppScope->AppEffects, new RenderJob(NULL, AppScope->AppEffects, NULL)))
 				{
 				if (AppScope->GUIWins->RJG->ConstructError)
 					{
 					delete AppScope->GUIWins->RJG;
 					AppScope->GUIWins->RJG = NULL;
 					} // if
 				} // if
 			} // if
 		if (AppScope->GUIWins->RJG)
 			{
 			AppScope->GUIWins->RJG->Open(AppScope->MainProj);
 			}
 		} // if
 	else
 		{
 		if (AppScope->GUIWins->RJG)
 			{
 			delete AppScope->GUIWins->RJG;
 			AppScope->GUIWins->RJG = NULL;
 			} // if
 		} // else
 	Result = 1;
 	} // WCS_TOOLBAR_ITEM_RJG
else if (Item == WCS_TOOLBAR_ITEM_ROG)
 	{
	if (OpenState)
 		{
 		if (!AppScope->GUIWins->ROG)
 			{
 			if (AppScope->GUIWins->ROG = new RenderOptEditGUI(AppScope->AppEffects, new RenderOpt(NULL, AppScope->AppEffects, NULL)))
 				{
 				if (AppScope->GUIWins->ROG->ConstructError)
 					{
 					delete AppScope->GUIWins->ROG;
 					AppScope->GUIWins->ROG = NULL;
 					} // if
 				} // if
 			} // if
 		if (AppScope->GUIWins->ROG)
 			{
 			AppScope->GUIWins->ROG->Open(AppScope->MainProj);
 			}
 		} // if
 	else
 		{
 		if (AppScope->GUIWins->ROG)
 			{
 			delete AppScope->GUIWins->ROG;
 			AppScope->GUIWins->ROG = NULL;
 			} // if
 		} // else
 	Result = 1;
 	} // WCS_TOOLBAR_ITEM_ROG
else if (Item == WCS_TOOLBAR_ITEM_RCG)
 	{
	if (OpenState)
 		{
		if (AppScope->GUIWins->RCG)
			{
			delete AppScope->GUIWins->RCG;
			AppScope->GUIWins->RCG = NULL;
			} // if
 		if (AppScope->GUIWins->RCG = new RenderControlGUI(AppScope->AppEffects, AppScope->AppImages, AppScope->AppDB, AppScope->MainProj))
 			{
 			if (AppScope->GUIWins->RCG->ConstructError)
 				{
 				delete AppScope->GUIWins->RCG;
 				AppScope->GUIWins->RCG = NULL;
 				} // if
 			} // if
 		if (AppScope->GUIWins->RCG)
 			{
 			AppScope->GUIWins->RCG->Open(AppScope->MainProj);
 			}
 		} // if
 	else
 		{
 		if (AppScope->GUIWins->RCG)
 			{
 			delete AppScope->GUIWins->RCG;
 			AppScope->GUIWins->RCG = NULL;
 			} // if
 		} // else
 	Result = 1;
 	} // WCS_TOOLBAR_ITEM_RCG
else if (Item == WCS_TOOLBAR_ITEM_STL)
	{
	if (OpenState)
		{
		AppScope->StatusLog->ForceLogOpen();
		AppScope->StatusLog->JumpTop();
		} // if
	else
		{
		AppScope->StatusLog->HandleCloseWin(NULL);
		} // else
	Result = 1;
	} // WCS_TOOLBAR_ITEM_STL
else if (Item == WCS_TOOLBAR_ITEM_RDG)
	{
	if (OpenState)
		{
		if (AppScope->GUIWins->RDG)
			{
			if(Page) GlobalApp->GUIWins->SAG->SwitchToTab(WCS_SCENEVIEWGUI_TAB_DIAG);
			Result = 1;
			} // if
		else
			{
			if (AppScope->GUIWins->RDG = new DiagnosticGUI())
				{
				if (AppScope->GUIWins->RDG->ConstructError)
					{
					delete AppScope->GUIWins->RDG;
					AppScope->GUIWins->RDG = NULL;
					} // if
				} // if
			if (AppScope->GUIWins->RDG)
				{
				AppScope->GUIWins->RDG->Open(AppScope->MainProj);
				if(Page) GlobalApp->GUIWins->SAG->SwitchToTab(WCS_SCENEVIEWGUI_TAB_DIAG);
				} // if
			} // if
		} // if
	else
		{
		if (AppScope->GUIWins->RDG)
			{
			delete AppScope->GUIWins->RDG;
			AppScope->GUIWins->RDG = NULL;
			} // if
		} // else
	Result = 1;
	} // else if

else
	{
	#ifdef WCS_BUILD_DEMO
	DONGLE_INLINE_CHECK()
	#else // !WCS_BUILD_DEMO

	if (OpenState && !AppScope->Sentinal->CheckDongle()) return(1);

	#endif // !WCS_BUILD_DEMO
	switch(Item)
		{
		case WCS_TOOLBAR_ITEM_CEG:
			{
			if (OpenState)
				{
				if (AppScope->GUIWins->CEG)
					{
					AppScope->GUIWins->CEG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->CEG)
					{
					delete AppScope->GUIWins->CEG;
					AppScope->GUIWins->CEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_CLG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->CLG)
					{
					if (AppScope->GUIWins->CLG = new CloudEditGUI(AppScope->AppEffects, AppScope->AppDB, new CloudEffect(NULL, AppScope->AppEffects, NULL)))
						{
						if (AppScope->GUIWins->CLG->ConstructError)
							{
							delete AppScope->GUIWins->CLG;
							AppScope->GUIWins->CLG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->CLG)
					{
					AppScope->GUIWins->CLG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->CLG)
					{
					delete AppScope->GUIWins->CLG;
					AppScope->GUIWins->CLG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_WEG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->WEG)
					{
					if (AppScope->GUIWins->WEG = new WaveEditGUI(AppScope->AppEffects, AppScope->AppDB, new WaveEffect(NULL, AppScope->AppEffects, NULL)))
						{
						if (AppScope->GUIWins->WEG->ConstructError)
							{
							delete AppScope->GUIWins->WEG;
							AppScope->GUIWins->WEG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->WEG)
					{
					AppScope->GUIWins->WEG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->WEG)
					{
					delete AppScope->GUIWins->WEG;
					AppScope->GUIWins->WEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_KSG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->DKG)
					{
					if (AppScope->GUIWins->DKG = new KeyScaleDeleteGUI(AppScope->MainProj, AppScope->AppEffects, NULL, WCS_KEYOPERATION_SCALE))
						{
						if (AppScope->GUIWins->DKG->ConstructError)
							{
							delete AppScope->GUIWins->DKG;
							AppScope->GUIWins->DKG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->DKG)
					{
					AppScope->GUIWins->DKG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->DKG)
					{
					delete AppScope->GUIWins->DKG;
					AppScope->GUIWins->DKG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_DBG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->DBE)
					{
					if (AppScope->GUIWins->DBE = new DBEditGUI(AppScope->AppDB, AppScope->MainProj, AppScope->AppEffects))
						{
						if (AppScope->GUIWins->DBE->ConstructError)
							{
							delete AppScope->GUIWins->DBE;
							AppScope->GUIWins->DBE = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->DBE)
					{
					AppScope->GUIWins->DBE->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->DBE)
					{
					delete AppScope->GUIWins->DBE;
					AppScope->GUIWins->DBE = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_DBO:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->DBO)
					{
					if (AppScope->GUIWins->DBO = new DBExportGUI(AppScope->MainProj, AppScope->AppDB, AppScope->AppEffects, AppScope->GUIWins->DBE))
						{
						if (AppScope->GUIWins->DBO->ConstructError)
							{
							delete AppScope->GUIWins->DBO;
							AppScope->GUIWins->DBO = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->DBO)
					{
					AppScope->GUIWins->DBO->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->DBO)
					{
					delete AppScope->GUIWins->DBO;
					AppScope->GUIWins->DBO = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_LWG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->LWG)
					{
					if (AppScope->GUIWins->LWG = new SceneExportGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->MainProj))
						{
						if (AppScope->GUIWins->LWG->ConstructError)
							{
							delete AppScope->GUIWins->LWG;
							AppScope->GUIWins->LWG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->LWG)
					{
					AppScope->GUIWins->LWG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->LWG)
					{
					delete AppScope->GUIWins->LWG;
					AppScope->GUIWins->LWG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_SIM:
			{
			if (OpenState)
				{
				if (AppScope->MainProj->VerifyProjectLoaded())
					{
					if (!AppScope->GUIWins->SIM)
						{
						if (AppScope->GUIWins->SIM = new SceneImportGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->MainProj))
							{
							if (AppScope->GUIWins->SIM->ConstructError)
								{
								delete AppScope->GUIWins->SIM;
								AppScope->GUIWins->SIM = NULL;
								} // if
							} // if
						} // if
					if (AppScope->GUIWins->SIM)
						{
						AppScope->GUIWins->SIM->Open(AppScope->MainProj);
						} // if
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->SIM)
					{
					delete AppScope->GUIWins->SIM;
					AppScope->GUIWins->SIM = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_FEG:
			{
			if (OpenState)
				{
				if (AppScope->GUIWins->FEG)
					{
					AppScope->GUIWins->FEG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->FEG)
					{
					delete AppScope->GUIWins->FEG;
					AppScope->GUIWins->FEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_IDG:
			{
			if (OpenState)
				{
				if (AppScope->MainProj->VerifyProjectLoaded())
					{
					if (!AppScope->GUIWins->IDG)
						{
						if (AppScope->GUIWins->IDG = new InterpDEMGUI(AppScope->MainProj, AppScope->AppDB, AppScope->AppEffects))
							{
							if (AppScope->GUIWins->IDG->ConstructError)
								{
								delete AppScope->GUIWins->IDG;
								AppScope->GUIWins->IDG = NULL;
								} // if
							} // if
						} // if
					if (AppScope->GUIWins->IDG)
						{
						AppScope->GUIWins->IDG->Open(AppScope->MainProj);
						} // if
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->IDG)
					{
					delete AppScope->GUIWins->IDG;
					AppScope->GUIWins->IDG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_DIG:
			{
			if (OpenState)
				{
				// now explicitly created by ViewGUI
				/*
				if (AppScope->MainProj->VerifyProjectLoaded())
					{
					if (!AppScope->GUIWins->DIG)
						{
						if (AppScope->GUIWins->DIG = new DigitizeGUI(AppScope->MainProj, AppScope->ActivePar, AppScope->AppDB, Page))
							{
							if (AppScope->GUIWins->DIG->ConstructError)
								{
								delete AppScope->GUIWins->DIG;
								AppScope->GUIWins->DIG = NULL;
								} // if
							} // if
						} // if
					if (AppScope->GUIWins->DIG)
						{
						AppScope->GUIWins->DIG->Open(AppScope->MainProj);
						} // if
					} // if
				*/
				} // if
			else
				{
				if (AppScope->GUIWins->DIG)
					{
					delete AppScope->GUIWins->DIG;
					AppScope->GUIWins->DIG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_PNG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->PNG)
					{
					AppScope->GUIWins->PNG = new ProjNewGUI(AppScope->MainProj, AppScope->AppDB, AppScope->AppEffects, AppScope->AppImages);
					} // if
				if (AppScope->GUIWins->PNG)
					{
					AppScope->GUIWins->PNG->Open(AppScope->MainProj);
					} // else
				} // if
			else
				{
				if (AppScope->GUIWins->PNG)
					{
					delete AppScope->GUIWins->PNG;
					AppScope->GUIWins->PNG = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_GRG:
			{
			if (OpenState)
				{
				if (AppScope->GUIWins->GRG[Page])
					{
					AppScope->GUIWins->GRG[Page]->Open(AppScope->MainProj);
					} // else
				} // if
			else
				{
				if (AppScope->GUIWins->GRG[Page])
					{
					delete AppScope->GUIWins->GRG[Page];
					AppScope->GUIWins->GRG[Page] = NULL;
					} // if
				} // else
			Result = 1; break;
			} // GRG
		case WCS_TOOLBAR_ITEM_DDL:
			{
			if (OpenState)
				{
				if (AppScope->GUIWins->DDL[Page])
					{
					AppScope->GUIWins->DDL[Page]->Open(AppScope->MainProj);
					} // else
				} // if
			else
				{
				if (AppScope->GUIWins->DDL[Page])
					{
					delete AppScope->GUIWins->DDL[Page];
					AppScope->GUIWins->DDL[Page] = NULL;
					} // if
				} // else
			Result = 1; break;
			} // DDL
		case WCS_TOOLBAR_ITEM_VIW: // a single ViewGUI View window, specified by the Page code
			{
			if (OpenState)
				{ // we don't open this way, only ViewGUI itself opens
				} // if
			else
				{ // safe delayed-close
				if (AppScope->GUIWins->CVG)
					{
					AppScope->GUIWins->CVG->DoClose(Page);
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_VIW
		case WCS_TOOLBAR_ITEM_CVG: // entire (Cam)ViewGUI
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->CVG)
					{
					if (AppScope->GUIWins->CVG = new ViewGUI(AppScope->MainProj, AppScope->AppDB,
						AppScope->MainProj->Interactive))
						{
						if (AppScope->GUIWins->CVG->ConstructError)
							{
							delete AppScope->GUIWins->CVG;
							AppScope->GUIWins->CVG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->CVG)
					{
					AppScope->GUIWins->CVG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->CVG)
					{
					delete AppScope->GUIWins->CVG;
					AppScope->GUIWins->CVG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_CVG
		case WCS_TOOLBAR_ITEM_LEG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->LEG)
					{
					if (AppScope->GUIWins->LEG = new LakeEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, AppScope->AppImages, new LakeEffect(NULL, AppScope->AppEffects, NULL)))
						{
						if (AppScope->GUIWins->LEG->ConstructError)
							{
							delete AppScope->GUIWins->LEG;
							AppScope->GUIWins->LEG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->LEG)
					{
					AppScope->GUIWins->LEG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->LEG)
					{
					delete AppScope->GUIWins->LEG;
					AppScope->GUIWins->LEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_LEG
		case WCS_TOOLBAR_ITEM_ECG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->ECG)
					{
					if (AppScope->GUIWins->ECG = new EcosystemEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, AppScope->AppImages, new EcosystemEffect(NULL, AppScope->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_ECOSYSTEM)))
						{
						if (AppScope->GUIWins->ECG->ConstructError)
							{
							delete AppScope->GUIWins->ECG;
							AppScope->GUIWins->ECG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->ECG)
					{
					AppScope->GUIWins->ECG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->ECG)
					{
					delete AppScope->GUIWins->ECG;
					AppScope->GUIWins->ECG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_ECG
		case WCS_TOOLBAR_ITEM_AEG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->AEG)
					{
					if (AppScope->GUIWins->AEG = new AtmosphereEditGUI(AppScope->AppEffects, new Atmosphere(NULL, AppScope->AppEffects, NULL)))
						{
						if (AppScope->GUIWins->AEG->ConstructError)
							{
							delete AppScope->GUIWins->AEG;
							AppScope->GUIWins->AEG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->AEG)
					{
					AppScope->GUIWins->AEG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->AEG)
					{
					delete AppScope->GUIWins->AEG;
					AppScope->GUIWins->AEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_AEG
 		case WCS_TOOLBAR_ITEM_RTG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->RTG)
 					{
 					if (AppScope->GUIWins->RTG = new RasterTAEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new RasterTerraffectorEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->RTG->ConstructError)
 							{
 							delete AppScope->GUIWins->RTG;
 							AppScope->GUIWins->RTG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->RTG)
 					{
 					AppScope->GUIWins->RTG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->RTG)
 					{
 					delete AppScope->GUIWins->RTG;
 					AppScope->GUIWins->RTG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_RTG
 		case WCS_TOOLBAR_ITEM_TAG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->TAG)
 					{
 					if (AppScope->GUIWins->TAG = new TerraffectorEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new TerraffectorEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->TAG->ConstructError)
 							{
 							delete AppScope->GUIWins->TAG;
 							AppScope->GUIWins->TAG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->TAG)
 					{
 					AppScope->GUIWins->TAG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->TAG)
 					{
 					delete AppScope->GUIWins->TAG;
 					AppScope->GUIWins->TAG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TAG
 		case WCS_TOOLBAR_ITEM_SHG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SHG)
 					{
 					if (AppScope->GUIWins->SHG = new ShadowEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new ShadowEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SHG->ConstructError)
 							{
 							delete AppScope->GUIWins->SHG;
 							AppScope->GUIWins->SHG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SHG)
 					{
 					AppScope->GUIWins->SHG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SHG)
 					{
 					delete AppScope->GUIWins->SHG;
 					AppScope->GUIWins->SHG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SHG
 		case WCS_TOOLBAR_ITEM_FLG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->FLG)
 					{
 					if (AppScope->GUIWins->FLG = new FoliageEffectEditGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->AppImages, new FoliageEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->FLG->ConstructError)
 							{
 							delete AppScope->GUIWins->FLG;
 							AppScope->GUIWins->FLG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->FLG)
 					{
 					AppScope->GUIWins->FLG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->FLG)
 					{
 					delete AppScope->GUIWins->FLG;
 					AppScope->GUIWins->FLG = NULL;
 					} // if
 				if (AppScope->GUIWins->FFG)
 					{
 					delete AppScope->GUIWins->FFG;
 					AppScope->GUIWins->FFG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_FLG
 		case WCS_TOOLBAR_ITEM_SEG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SEG)
 					{
 					if (AppScope->GUIWins->SEG = new StreamEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, AppScope->AppImages, new StreamEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SEG->ConstructError)
 							{
 							delete AppScope->GUIWins->SEG;
 							AppScope->GUIWins->SEG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SEG)
 					{
 					AppScope->GUIWins->SEG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SEG)
 					{
 					delete AppScope->GUIWins->SEG;
 					AppScope->GUIWins->SEG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SEG
 		case WCS_TOOLBAR_ITEM_OEG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->OEG)
 					{
 					if (AppScope->GUIWins->OEG = new Object3DEditGUI(AppScope->AppEffects, AppScope->AppDB, new Object3DEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->OEG->ConstructError)
 							{
 							delete AppScope->GUIWins->OEG;
 							AppScope->GUIWins->OEG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->OEG)
 					{
 					AppScope->GUIWins->OEG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->OEG)
 					{
 					delete AppScope->GUIWins->OEG;
 					AppScope->GUIWins->OEG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_OEG
 		case WCS_TOOLBAR_ITEM_MAG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->MAG)
 					{
 					if (AppScope->GUIWins->MAG = new MaterialEditGUI(AppScope->AppEffects, new MaterialEffect(NULL, AppScope->AppEffects, NULL, WCS_EFFECTS_MATERIALTYPE_OBJECT3D)))
 						{
 						if (AppScope->GUIWins->MAG->ConstructError)
 							{
 							delete AppScope->GUIWins->MAG;
 							AppScope->GUIWins->MAG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->MAG)
 					{
 					AppScope->GUIWins->MAG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->MAG)
 					{
 					delete AppScope->GUIWins->MAG;
 					AppScope->GUIWins->MAG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_MAG
 		case WCS_TOOLBAR_ITEM_TGN:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->TGN)
 					{
					TerraGenerator *MyTG;

					MyTG = (TerraGenerator *)AppScope->AppEffects->GetDefaultEffect(WCS_EFFECTSSUBCLASS_GENERATOR, 1, AppScope->AppDB);
 					if (AppScope->GUIWins->TGN = new TerraGeneratorEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, MyTG ? MyTG: new TerraGenerator(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->TGN->ConstructError)
 							{
 							delete AppScope->GUIWins->TGN;
 							AppScope->GUIWins->TGN = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->TGN)
 					{
 					AppScope->GUIWins->TGN->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->TGN)
 					{
 					delete AppScope->GUIWins->TGN;
 					AppScope->GUIWins->TGN = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TGN
 		case WCS_TOOLBAR_ITEM_SQU:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SQU)
 					{
 					if (AppScope->GUIWins->SQU = new SearchQueryEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new SearchQuery(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SQU->ConstructError)
 							{
 							delete AppScope->GUIWins->SQU;
 							AppScope->GUIWins->SQU = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SQU)
 					{
 					AppScope->GUIWins->SQU->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SQU)
 					{
 					delete AppScope->GUIWins->SQU;
 					AppScope->GUIWins->SQU = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SQU
 		case WCS_TOOLBAR_ITEM_THM:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->THM)
 					{
 					if (AppScope->GUIWins->THM = new ThematicMapEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new ThematicMap(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->THM->ConstructError)
 							{
 							delete AppScope->GUIWins->THM;
 							AppScope->GUIWins->THM = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->THM)
 					{
 					AppScope->GUIWins->THM->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->THM)
 					{
 					delete AppScope->GUIWins->THM;
 					AppScope->GUIWins->THM = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_THM
 		case WCS_TOOLBAR_ITEM_COS:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->COS)
 					{
 					if (AppScope->GUIWins->COS = new CoordSysEditGUI(AppScope->AppEffects, AppScope->AppDB, new CoordSys(NULL, AppScope->AppEffects, NULL), 0))
 						{
 						if (AppScope->GUIWins->COS->ConstructError)
 							{
 							delete AppScope->GUIWins->COS;
 							AppScope->GUIWins->COS = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->COS)
 					{
 					AppScope->GUIWins->COS->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->COS)
 					{
 					delete AppScope->GUIWins->COS;
 					AppScope->GUIWins->COS = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_COS
 		case WCS_TOOLBAR_ITEM_FCG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->FCG)
 					{
 					if (AppScope->GUIWins->FCG = new FenceEditGUI(AppScope->AppEffects, AppScope->AppDB, new Fence(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->FCG->ConstructError)
 							{
 							delete AppScope->GUIWins->FCG;
 							AppScope->GUIWins->FCG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->FCG)
 					{
 					AppScope->GUIWins->FCG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->FCG)
 					{
 					delete AppScope->GUIWins->FCG;
 					AppScope->GUIWins->FCG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_FCG
 		case WCS_TOOLBAR_ITEM_PPR:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->PPR)
 					{
 					if (AppScope->GUIWins->PPR = new PostProcEditGUI(AppScope->AppEffects, new PostProcess(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->PPR->ConstructError)
 							{
 							delete AppScope->GUIWins->PPR;
 							AppScope->GUIWins->PPR = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->PPR)
 					{
 					AppScope->GUIWins->PPR->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->PPR)
 					{
 					delete AppScope->GUIWins->PPR;
 					AppScope->GUIWins->PPR = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_PPR
 		case WCS_TOOLBAR_ITEM_SCN:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SCN)
 					{
 					if (AppScope->GUIWins->SCN = new ScenarioEditGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->AppImages, new RenderScenario(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SCN->ConstructError)
 							{
 							delete AppScope->GUIWins->SCN;
 							AppScope->GUIWins->SCN = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SCN)
 					{
 					AppScope->GUIWins->SCN->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SCN)
 					{
 					delete AppScope->GUIWins->SCN;
 					AppScope->GUIWins->SCN = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SCN
 		case WCS_TOOLBAR_ITEM_LBL:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->LBL)
 					{
 					if (AppScope->GUIWins->LBL = new LabelEditGUI(AppScope->AppEffects, AppScope->AppDB, new Label(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->LBL->ConstructError)
 							{
 							delete AppScope->GUIWins->LBL;
 							AppScope->GUIWins->LBL = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->LBL)
 					{
 					AppScope->GUIWins->LBL->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->LBL)
 					{
 					delete AppScope->GUIWins->LBL;
 					AppScope->GUIWins->LBL = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_LBL
 		case WCS_TOOLBAR_ITEM_EXP:
 			{
			#ifdef WCS_BUILD_RTX
			// lack of RTX authorization will result in ConstructError = 1
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->EXP)
 					{
 					if (AppScope->GUIWins->EXP = new ExporterEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, AppScope->AppImages, new SceneExporter(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->EXP->ConstructError)
 							{
 							delete AppScope->GUIWins->EXP;
 							AppScope->GUIWins->EXP = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->EXP)
 					{
 					AppScope->GUIWins->EXP->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->EXP)
 					{
 					delete AppScope->GUIWins->EXP;
 					AppScope->GUIWins->EXP = NULL;
 					} // if
 				} // else
			#endif // WCS_BUILD_RTX
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_EXP
 		case WCS_TOOLBAR_ITEM_EXG:
 			{
			#ifdef WCS_BUILD_RTX
			// lack of RTX authorization will result in ConstructError = 1
			if (OpenState)
 				{
				if (AppScope->GUIWins->EXG)
					{
					delete AppScope->GUIWins->EXG;
					AppScope->GUIWins->EXG = NULL;
					} // if
 				if (AppScope->GUIWins->EXG = new ExportControlGUI(AppScope->AppEffects, AppScope->AppImages, AppScope->AppDB, AppScope->MainProj))
 					{
 					if (AppScope->GUIWins->EXG->ConstructError)
 						{
 						delete AppScope->GUIWins->EXG;
 						AppScope->GUIWins->EXG = NULL;
 						} // if
 					} // if
 				if (AppScope->GUIWins->EXG)
 					{
 					AppScope->GUIWins->EXG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->EXG)
 					{
 					delete AppScope->GUIWins->EXG;
 					AppScope->GUIWins->EXG = NULL;
 					} // if
 				} // else
			#endif // WCS_BUILD_RTX
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_EXG
 		case WCS_TOOLBAR_ITEM_FWZ:
 			{
			#ifdef WCS_FORESTRY_WIZARD
			// lack of Forestry authorization will result in ConstructError = 1
			if (OpenState)
 				{
				if (AppScope->GUIWins->FWZ)
					{
					delete AppScope->GUIWins->FWZ;
					AppScope->GUIWins->FWZ = NULL;
					} // if
 				if (AppScope->GUIWins->FWZ = new ForestWizGUI(AppScope->AppEffects, AppScope->AppImages, AppScope->AppDB, AppScope->MainProj))
 					{
 					if (AppScope->GUIWins->FWZ->ConstructError)
 						{
 						delete AppScope->GUIWins->FWZ;
 						AppScope->GUIWins->FWZ = NULL;
						// modal requester to tell them about the Forestry Wizard
						UserMessageOK("Forestry Wizard", "The Forestry Wizard is a VNS add-on product that quickly configures your projects to use foliage species, size and density attributes associated with Database vectors. The Wizard can also make setting up Ecosystem matching for Color Maps easier and more accurate.\n\nThe Forestry Edition of VNS is available from 3D Nature and includes the Forestry Wizard.");
 						} // if
 					} // if
 				if (AppScope->GUIWins->FWZ)
 					{
 					AppScope->GUIWins->FWZ->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->FWZ)
 					{
 					delete AppScope->GUIWins->FWZ;
 					AppScope->GUIWins->FWZ = NULL;
 					} // if
 				} // else
			#endif // WCS_FORESTRY_WIZARD
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_FWZ
 		case WCS_TOOLBAR_ITEM_GWZ:
 			{
			if (OpenState)
 				{
				if (AppScope->GUIWins->GWZ)
					{
					delete AppScope->GUIWins->GWZ;
					AppScope->GUIWins->GWZ = NULL;
					} // if
 				if (AppScope->GUIWins->GWZ = new GridderWizGUI(AppScope->AppEffects, AppScope->AppImages, AppScope->AppDB, AppScope->MainProj))
 					{
 					if (AppScope->GUIWins->GWZ->ConstructError)
 						{
 						delete AppScope->GUIWins->GWZ;
 						AppScope->GUIWins->GWZ = NULL;
 						} // if
 					} // if
 				if (AppScope->GUIWins->GWZ)
 					{
 					AppScope->GUIWins->GWZ->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->GWZ)
 					{
 					delete AppScope->GUIWins->GWZ;
 					AppScope->GUIWins->GWZ = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_GWZ
  		case WCS_TOOLBAR_ITEM_MWZ:
 			{
			#ifdef WCS_BUILD_VNS
			// lack of VNS authorization will result in ConstructError = 1
			if (OpenState)
 				{
				if (AppScope->GUIWins->MWZ)
					{
					delete AppScope->GUIWins->MWZ;
					AppScope->GUIWins->MWZ = NULL;
					} // if
 				if (AppScope->GUIWins->MWZ = new MergerWizGUI(AppScope->AppEffects, AppScope->AppImages, AppScope->AppDB, AppScope->MainProj))
 					{
 					if (AppScope->GUIWins->MWZ->ConstructError)
 						{
 						delete AppScope->GUIWins->MWZ;
 						AppScope->GUIWins->MWZ = NULL;
 						} // if
 					} // if
 				if (AppScope->GUIWins->MWZ)
 					{
 					AppScope->GUIWins->MWZ->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->MWZ)
 					{
 					delete AppScope->GUIWins->MWZ;
 					AppScope->GUIWins->MWZ = NULL;
 					} // if
 				} // else
			#endif // WCS_BUILD_VNS
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_MWZ
		case WCS_TOOLBAR_ITEM_VPX:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->VPX)
 					{
 					if (AppScope->GUIWins->VPX = new VecProfExportGUI(AppScope->MainProj, NULL))
 						{
 						if (AppScope->GUIWins->VPX->ConstructError)
 							{
 							delete AppScope->GUIWins->VPX;
 							AppScope->GUIWins->VPX = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->VPX)
 					{
 					AppScope->GUIWins->VPX->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->VPX)
 					{
 					delete AppScope->GUIWins->VPX;
 					AppScope->GUIWins->VPX = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_VPX
 		case WCS_TOOLBAR_ITEM_CSC:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->CSC)
 					{
 					if (AppScope->GUIWins->CSC = new CoordsCalculatorGUI(AppScope->AppEffects, NULL))
 						{
 						if (AppScope->GUIWins->CSC->ConstructError)
 							{
 							delete AppScope->GUIWins->CSC;
 							AppScope->GUIWins->CSC = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->CSC)
 					{
 					AppScope->GUIWins->CSC->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->CSC)
 					{
 					delete AppScope->GUIWins->CSC;
 					AppScope->GUIWins->CSC = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_CSC
 		case WCS_TOOLBAR_ITEM_DRL:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->DRL)
 					{
 					if (AppScope->GUIWins->DRL = new DrillDownInfoGUI(AppScope->AppEffects, AppScope->AppDB, NULL))
 						{
 						if (AppScope->GUIWins->DRL->ConstructError)
 							{
 							delete AppScope->GUIWins->DRL;
 							AppScope->GUIWins->DRL = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->DRL)
 					{
 					AppScope->GUIWins->DRL->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->DRL)
 					{
 					delete AppScope->GUIWins->DRL;
 					AppScope->GUIWins->DRL = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_DRL
 		case WCS_TOOLBAR_ITEM_DEM:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->DEM)
 					{
 					if (AppScope->GUIWins->DEM = new DEMEditGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->MainProj, NULL))
 						{
 						if (AppScope->GUIWins->DEM->ConstructError)
 							{
 							delete AppScope->GUIWins->DEM;
 							AppScope->GUIWins->DEM = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->DEM)
 					{
 					AppScope->GUIWins->DEM->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->DEM)
 					{
 					delete AppScope->GUIWins->DEM;
 					AppScope->GUIWins->DEM = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_DEM
 		case WCS_TOOLBAR_ITEM_DPG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->DPG)
 					{
 					if (AppScope->GUIWins->DPG = new DEMPaintGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->MainProj, NULL))
 						{
 						if (AppScope->GUIWins->DPG->ConstructError)
 							{
 							delete AppScope->GUIWins->DPG;
 							AppScope->GUIWins->DPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->DPG)
 					{
 					AppScope->GUIWins->DPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->DPG)
 					{
 					delete AppScope->GUIWins->DPG;
 					AppScope->GUIWins->DPG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_DPG
 		case WCS_TOOLBAR_ITEM_TPM:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->TPM)
 					{
 					if (AppScope->GUIWins->TPM = new TemplateManGUI(AppScope->MainProj))
 						{
 						if (AppScope->GUIWins->TPM->ConstructError)
 							{
 							delete AppScope->GUIWins->TPM;
 							AppScope->GUIWins->TPM = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->TPM)
 					{
 					AppScope->GUIWins->TPM->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->TPM)
 					{
 					delete AppScope->GUIWins->TPM;
 					AppScope->GUIWins->TPM = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TPM
 		case WCS_TOOLBAR_ITEM_PTH:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->PTH)
 					{
 					if (AppScope->GUIWins->PTH = new PathTransferGUI(AppScope->MainProj, AppScope->AppDB, AppScope->AppEffects, NULL, NULL))
 						{
 						if (AppScope->GUIWins->PTH->ConstructError)
 							{
 							delete AppScope->GUIWins->PTH;
 							AppScope->GUIWins->PTH = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->PTH)
 					{
 					AppScope->GUIWins->PTH->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->PTH)
 					{
 					delete AppScope->GUIWins->PTH;
 					AppScope->GUIWins->PTH = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_PTH
 		case WCS_TOOLBAR_ITEM_DSS:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->DSS)
 					{
 					if (AppScope->GUIWins->DSS = new EDSSControlGUI(AppScope->StartEDSS, AppScope->AppEffects, AppScope->AppDB, AppScope->AppImages))
 						{
 						if (AppScope->GUIWins->DSS->ConstructError)
 							{
 							delete AppScope->GUIWins->DSS;
 							AppScope->GUIWins->DSS = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->DSS)
 					{
 					AppScope->GUIWins->DSS->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->DSS)
 					{
 					delete AppScope->GUIWins->DSS;
 					AppScope->GUIWins->DSS = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_DSS
 		case WCS_TOOLBAR_ITEM_TBG:
 			{
 			if (! OpenState)
 				{
 				if (AppScope->GUIWins->TBG)
 					{
 					delete AppScope->GUIWins->TBG;
 					AppScope->GUIWins->TBG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TBG
// <<<>>> ADD_NEW_EFFECTS this is for opening and closing the window
 		case WCS_TOOLBAR_ITEM_ILG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->ILG)
 					{
 					if (AppScope->GUIWins->ILG = new ImageLibGUI(AppScope->AppImages, AppScope->AppEffects, AppScope->MainProj))
 						{
 						if (AppScope->GUIWins->ILG->ConstructError)
 							{
 							delete AppScope->GUIWins->ILG;
 							AppScope->GUIWins->ILG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->ILG)
 					{
 					AppScope->GUIWins->ILG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->ILG)
 					{
 					delete AppScope->GUIWins->ILG;
 					AppScope->GUIWins->ILG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_ILG
 		case WCS_TOOLBAR_ITEM_NPG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->NPG)
 					{
 					if (AppScope->GUIWins->NPG = new LightEditGUI(AppScope->AppEffects, new Light(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->NPG->ConstructError)
 							{
 							delete AppScope->GUIWins->NPG;
 							AppScope->GUIWins->NPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->NPG)
 					{
 					AppScope->GUIWins->NPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->NPG)
 					{
 					delete AppScope->GUIWins->NPG;
 					AppScope->GUIWins->NPG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_NPG
 		case WCS_TOOLBAR_ITEM_SPG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SPG)
 					{
 					if (AppScope->GUIWins->SPG = new SunPosGUI(AppScope->MainProj, new Light(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SPG->ConstructError)
 							{
 							delete AppScope->GUIWins->SPG;
 							AppScope->GUIWins->SPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SPG)
 					{
 					AppScope->GUIWins->SPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SPG)
 					{
 					delete AppScope->GUIWins->SPG;
 					AppScope->GUIWins->SPG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SPG
 		case WCS_TOOLBAR_ITEM_CPG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->CPG)
 					{
 					if (AppScope->GUIWins->CPG = new CameraEditGUI(AppScope->AppEffects, AppScope->AppImages, new Camera(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->CPG->ConstructError)
 							{
 							delete AppScope->GUIWins->CPG;
 							AppScope->GUIWins->CPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->CPG)
 					{
 					AppScope->GUIWins->CPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->CPG)
 					{
 					delete AppScope->GUIWins->CPG;
 					AppScope->GUIWins->CPG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_CPG
 		case WCS_TOOLBAR_ITEM_TPG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->TPG)
 					{
 					if (AppScope->GUIWins->TPG = new TerrainParamEditGUI(AppScope->AppEffects, GlobalApp->AppDB, new TerrainParamEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->TPG->ConstructError)
 							{
 							delete AppScope->GUIWins->TPG;
 							AppScope->GUIWins->TPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->TPG)
 					{
 					AppScope->GUIWins->TPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->TPG)
 					{
 					delete AppScope->GUIWins->TPG;
 					AppScope->GUIWins->TPG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TPG
 		case WCS_TOOLBAR_ITEM_KPG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->KPG)
 					{
 					if (AppScope->GUIWins->KPG = new SkyEditGUI(AppScope->AppEffects, new Sky(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->KPG->ConstructError)
 							{
 							delete AppScope->GUIWins->KPG;
 							AppScope->GUIWins->KPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->KPG)
 					{
 					AppScope->GUIWins->KPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->KPG)
 					{
 					delete AppScope->GUIWins->KPG;
 					AppScope->GUIWins->KPG = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_KPG
 		case WCS_TOOLBAR_ITEM_LPG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->LPG)
 					{
 					if (AppScope->GUIWins->LPG = new CelestialEditGUI(AppScope->AppEffects, new CelestialEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->LPG->ConstructError)
 							{
 							delete AppScope->GUIWins->LPG;
 							AppScope->GUIWins->LPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->LPG)
 					{
 					AppScope->GUIWins->LPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->LPG)
 					{
 					delete AppScope->GUIWins->LPG;
 					AppScope->GUIWins->LPG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_LPG
 		case WCS_TOOLBAR_ITEM_STG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->STG)
 					{
 					if (AppScope->GUIWins->STG = new StarfieldEditGUI(AppScope->AppEffects, new StarFieldEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->STG->ConstructError)
 							{
 							delete AppScope->GUIWins->STG;
 							AppScope->GUIWins->STG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->STG)
 					{
 					AppScope->GUIWins->STG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->STG)
 					{
 					delete AppScope->GUIWins->STG;
 					AppScope->GUIWins->STG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_STG
 		case WCS_TOOLBAR_ITEM_CMG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->CMG)
 					{
 					if (AppScope->GUIWins->CMG = new CmapEditGUI(AppScope->AppEffects, AppScope->AppDB, new CmapEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->CMG->ConstructError)
 							{
 							delete AppScope->GUIWins->CMG;
 							AppScope->GUIWins->CMG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->CMG)
 					{
 					AppScope->GUIWins->CMG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->CMG)
 					{
 					delete AppScope->GUIWins->CMG;
 					AppScope->GUIWins->CMG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_CMG
 		case WCS_TOOLBAR_ITEM_ENG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->ENG)
 					{
 					if (AppScope->GUIWins->ENG = new EnvironmentEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new EnvironmentEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->ENG->ConstructError)
 							{
 							delete AppScope->GUIWins->ENG;
 							AppScope->GUIWins->ENG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->ENG)
 					{
 					AppScope->GUIWins->ENG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->ENG)
 					{
 					delete AppScope->GUIWins->ENG;
 					AppScope->GUIWins->ENG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_ENG
 		case WCS_TOOLBAR_ITEM_GNG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->GNG)
 					{
 					if (AppScope->GUIWins->GNG = new GroundEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new GroundEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->GNG->ConstructError)
 							{
 							delete AppScope->GUIWins->GNG;
 							AppScope->GUIWins->GNG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->GNG)
 					{
 					AppScope->GUIWins->GNG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->GNG)
 					{
 					delete AppScope->GUIWins->GNG;
 					AppScope->GUIWins->GNG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_GNG
 		case WCS_TOOLBAR_ITEM_POG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->POG)
 					{
 					if (AppScope->GUIWins->POG = new PlanetOptEditGUI(AppScope->AppEffects, new PlanetOpt(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->POG->ConstructError)
 							{
 							delete AppScope->GUIWins->POG;
 							AppScope->GUIWins->POG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->POG)
 					{
 					AppScope->GUIWins->POG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->POG)
 					{
 					delete AppScope->GUIWins->POG;
 					AppScope->GUIWins->POG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_POG
 		case WCS_TOOLBAR_ITEM_GRD:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->GRD)
					{
					if (AppScope->GUIWins->GRD = new TerraGridderEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new TerraGridder(NULL, AppScope->AppEffects, NULL)))
						{
						if (AppScope->GUIWins->GRD->ConstructError)
							{
							delete AppScope->GUIWins->GRD;
							AppScope->GUIWins->GRD = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->GRD)
					{
					AppScope->GUIWins->GRD->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->GRD)
					{
					delete AppScope->GUIWins->GRD;
					AppScope->GUIWins->GRD = NULL;
					} // if
				} // else
			Result = 1; break;
			} //
		case WCS_TOOLBAR_ITEM_SNG:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->SNG)
 					{
 					if (AppScope->GUIWins->SNG = new SnowEditGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB, new SnowEffect(NULL, AppScope->AppEffects, NULL)))
 						{
 						if (AppScope->GUIWins->SNG->ConstructError)
 							{
 							delete AppScope->GUIWins->SNG;
 							AppScope->GUIWins->SNG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->SNG)
 					{
 					AppScope->GUIWins->SNG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->SNG)
 					{
 					delete AppScope->GUIWins->SNG;
 					AppScope->GUIWins->SNG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_SNG
		case WCS_TOOLBAR_ITEM_EFG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->EFG)
					{
					AppScope->GUIWins->EFG = new EffectsLibGUI(AppScope->AppEffects, AppScope->MainProj, AppScope->AppDB);
					} // if
				if (AppScope->GUIWins->EFG)
					{
					AppScope->GUIWins->EFG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->EFG)
					{
					delete AppScope->GUIWins->EFG;
					AppScope->GUIWins->EFG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_EFG
		case WCS_TOOLBAR_ITEM_DKG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->DKG)
					{
					if (AppScope->GUIWins->DKG = new KeyScaleDeleteGUI(AppScope->MainProj, AppScope->AppEffects, NULL, WCS_KEYOPERATION_DELETE))
						{
						if (AppScope->GUIWins->DKG->ConstructError)
							{
							delete AppScope->GUIWins->DKG;
							AppScope->GUIWins->DKG = NULL;
							} // if
						} // if
					} // if
				if (AppScope->GUIWins->DKG)
					{
					AppScope->GUIWins->DKG->Open(AppScope->MainProj);
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->DKG)
					{
					delete AppScope->GUIWins->DKG;
					AppScope->GUIWins->DKG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // 
		case WCS_TOOLBAR_ITEM_VEG:
			{
			if (OpenState)
				{
				if (!AppScope->GUIWins->VEG)
					{
					if (AppScope->GUIWins->VEG = new VectorEditGUI(AppScope->AppDB, AppScope->MainProj, AppScope->AppEffects, AppScope->MainProj->Interactive))
						{
 						if (AppScope->GUIWins->VEG->ConstructError)
 							{
 							delete AppScope->GUIWins->VEG;
 							AppScope->GUIWins->VEG = NULL;
 							} // if
						} // if
					} // if
				if (AppScope->GUIWins->VEG)
					{
					AppScope->GUIWins->VEG->Open(AppScope->MainProj);
					}
				} // if
			else
				{
				if (AppScope->GUIWins->VEG)
					{
					delete AppScope->GUIWins->VEG;
					AppScope->GUIWins->VEG = NULL;
					} // if
				} // else
			Result = 1; break;
			} // WCS_TOOLBAR_ITEM_VEG
 		case WCS_TOOLBAR_ITEM_VPG:
 			{
 			if (OpenState)
 				{
 				if (!AppScope->GUIWins->VPG)
 					{
 					if (AppScope->GUIWins->VPG = new VectorProfileGUI(AppScope->AppDB, NULL))
 						{
 						if (AppScope->GUIWins->VPG->ConstructError)
 							{
 							delete AppScope->GUIWins->VPG;
 							AppScope->GUIWins->VPG = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->VPG)
 					{
 					AppScope->GUIWins->VPG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->VPG)
 					{
 					delete AppScope->GUIWins->VPG;
 					AppScope->GUIWins->VPG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TAG
  		case WCS_TOOLBAR_ITEM_IWG:
 			{
			if (OpenState)
				{
				if (AppScope->MainProj->VerifyProjectLoaded())
					{
					if (!AppScope->GUIWins->IWG)
						{
						AppScope->GUIWins->IWG = new ImportWizGUI(AppScope->AppDB, AppScope->MainProj, AppScope->AppEffects, AppScope->StatusLog, true);
 						if (AppScope->GUIWins->IWG->ConstructError)
 							{
 							delete AppScope->GUIWins->IWG;
 							AppScope->GUIWins->IWG = NULL;
 							} // if
						} // if
					if (AppScope->GUIWins->IWG)
						{
						AppScope->GUIWins->IWG->Open(AppScope->MainProj);
						}
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->IWG)
					{
					delete AppScope->GUIWins->IWG;
					AppScope->GUIWins->IWG = NULL;
					} // if
				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_IWG
 		case WCS_TOOLBAR_ITEM_TXG:
 			{
			if (OpenState)
 				{
 				if (AppScope->GUIWins->TXG)
 					{
 					AppScope->GUIWins->TXG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->TXG)
 					{
 					delete AppScope->GUIWins->TXG;
 					AppScope->GUIWins->TXG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_TXG
 		case WCS_TOOLBAR_ITEM_MSG:
 			{
			if (OpenState)
 				{
 				if (AppScope->GUIWins->MSG)
 					{
 					AppScope->GUIWins->MSG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->MSG)
 					{
 					delete AppScope->GUIWins->MSG;
 					AppScope->GUIWins->MSG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_MSG
 		case WCS_TOOLBAR_ITEM_LVE:
 			{
			if (OpenState)
 				{
 				if (!AppScope->GUIWins->LVE)
 					{
 					if (AppScope->GUIWins->LVE = new GalleryGUI(AppScope->AppEffects, NULL))
 						{
 						if (AppScope->GUIWins->LVE->ConstructError)
 							{
 							delete AppScope->GUIWins->LVE;
 							AppScope->GUIWins->LVE = NULL;
 							} // if
 						} // if
 					} // if
 				if (AppScope->GUIWins->LVE)
 					{
 					AppScope->GUIWins->LVE->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->LVE)
 					{
 					delete AppScope->GUIWins->LVE;
 					AppScope->GUIWins->LVE = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_LVE
 		case WCS_TOOLBAR_ITEM_BRD:
 			{
			if (OpenState)
 				{
 				if (AppScope->GUIWins->BRD)
 					{
 					AppScope->GUIWins->BRD->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->BRD)
 					{
 					delete AppScope->GUIWins->BRD;
 					AppScope->GUIWins->BRD = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_BRD
		case WCS_TOOLBAR_ITEM_IVG:
 			{
			if (OpenState)
 				{
				if (AppScope->GUIWins->IVG)
					{
					delete AppScope->GUIWins->IVG;
					AppScope->GUIWins->IVG = NULL;
					} // if
 				if (AppScope->GUIWins->IVG = new ImageViewGUI(AppScope->AppImages, NULL, Page, FALSE))
 					{
 					if (AppScope->GUIWins->IVG->ConstructError)
 						{
 						delete AppScope->GUIWins->IVG;
 						AppScope->GUIWins->IVG = NULL;
 						} // if
 					} // if
 				if (AppScope->GUIWins->IVG)
 					{
 					AppScope->GUIWins->IVG->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->IVG)
 					{
 					delete AppScope->GUIWins->IVG;
 					AppScope->GUIWins->IVG = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_IVG
 		case WCS_TOOLBAR_ITEM_EFL:
 			{
			if (OpenState)
 				{
 				if (AppScope->GUIWins->EFL)
 					{
 					AppScope->GUIWins->EFL->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->EFL)
 					{
 					delete AppScope->GUIWins->EFL;
 					AppScope->GUIWins->EFL = NULL;
 					} // if
 				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_EFL
 		case WCS_TOOLBAR_ITEM_NUM:
 			{
			if (OpenState)
 				{
 				if (AppScope->GUIWins->NUM)
 					{
 					AppScope->GUIWins->NUM->Open(AppScope->MainProj);
 					}
 				} // if
 			else
 				{
 				if (AppScope->GUIWins->NUM)
 					{
 					delete AppScope->GUIWins->NUM;
 					AppScope->GUIWins->NUM = NULL;
 					} // if
 				} // else
 			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_NUM
#ifdef WCS_DEM_MERGE
		case WCS_TOOLBAR_ITEM_MRG:
 			{
			if (OpenState)
				{
				if (AppScope->MainProj->VerifyProjectLoaded())
					{
					if (!AppScope->GUIWins->MRG)
						{
						AppScope->GUIWins->MRG = new DEMMergeGUI(AppScope->AppEffects, AppScope->AppDB, AppScope->MainProj, AppScope->MainProj->Interactive,
							new DEMMerger(NULL, AppScope->AppEffects, NULL));
 						if (AppScope->GUIWins->MRG->ConstructError)
 							{
 							delete AppScope->GUIWins->MRG;
 							AppScope->GUIWins->MRG = NULL;
 							} // if
						} // if
					if (AppScope->GUIWins->MRG)
						{
						AppScope->GUIWins->MRG->Open(AppScope->MainProj);
						}
					} // if
				} // if
			else
				{
				if (AppScope->GUIWins->MRG)
					{
					delete AppScope->GUIWins->MRG;
					AppScope->GUIWins->MRG = NULL;
					} // if
				} // else
			Result = 1; break;
 			} // WCS_TOOLBAR_ITEM_MRG
#endif // WCS_DEM_MERGE
		} // Item
	} // else

if(!OpenState) // did we close a window? If so, finalize the window activation trick
	{
	if(IsWindow(CurrentActive)) // is previously-active window still around?
		SetActiveWindow(CurrentActive); // make it active again
	} // if

return(Result);
} // Toolbar::HandleModule

/*===========================================================================*/

void Toolbar::OpenWindows(Project *Proj)
{
int Search;

if (Proj->FenTrack && Proj->FenTrackSize)
	{
	for(Search = 0; Search < Proj->FenTrackSize; Search++)
		{
		if (Proj->FenTrack[Search].WinID == 0)
			{
			break;
			} // if
		if (Proj->FenTrack[Search].Flags & WCS_FENTRACK_FLAGS_OPEN)
			{
			if (Proj->FenTrack[Search].WinID != 'STLG') // prevent status log from auto-opening
				{
				OpenAWindow(Proj->FenTrack[Search].WinID);
				} // if
			} // if flags say open
		} // for
	} // if

} // Toolbar::OpenWindows()

/*===========================================================================*/

void Toolbar::OpenAWindow(unsigned long int WinID)
{
ULONG WinNotify, NotifyChanges[2] = {0, 0};

if (WinNotify = MajorWindowNotifyFromID(WinID))
	{
	NotifyChanges[0] = (MAKE_ID(WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_OPEN_MOD,
	 WinNotify, 0));
	} // if


GenerateNotify(NotifyChanges);

} // Toolbar::OpenAWindow()

/*===========================================================================*/

void Toolbar::RequestWindowClose(unsigned long int WinID)
{
ULONG WinNotify, NotifyChanges[2] = {0, 0};

if (WinNotify = NotifyFromID(WinID))
	{
	NotifyChanges[0] = (MAKE_ID(WCS_TOOLBARCLASS_MODULES, WCS_TOOLBAR_REQUEST_CLOSE_MOD,
	 WinNotify, 0));
	} // if

GenerateNotify(NotifyChanges);

} // Toolbar::RequestWindowClose()

/*===========================================================================*/

unsigned long int Toolbar::NotifyFromID(unsigned long int WinID)
{
unsigned long int rVal;

switch(WinID)
	{
	case 'DBED':
		{
		rVal = WCS_TOOLBAR_ITEM_DBG; break;
		} // DBEditGUI
	case 'EFFL':
		{
		rVal = WCS_TOOLBAR_ITEM_EFG; break;
		} // Component Library
	case 'STLG':
		{
		rVal = WCS_TOOLBAR_ITEM_STL; break;
		} // Status Log
	case 'IMGL':
		{
		rVal = WCS_TOOLBAR_ITEM_ILG; break;
		} // Image Library Dialog
	case 'SAAG':
		{
		rVal = WCS_TOOLBAR_ITEM_SAG; break;
		} // Scene at a Glance Dialog
	case 'ANGR':
		{
		rVal = WCS_TOOLBAR_ITEM_GRG; break;
		} // Animation Graph
	case 'ANIM':
		{
		rVal = WCS_TOOLBAR_ITEM_APG; break;
		} // Animation Preview Control
	case 'ATMG':
		{
		rVal = WCS_TOOLBAR_ITEM_AEG; break;
		} // Atmosphere Editor
	case 'BSIN':
		{
		rVal = WCS_TOOLBAR_ITEM_BRD; break;
		} // Component Signature
	case 'CAMP':
		{
		rVal = WCS_TOOLBAR_ITEM_CPG; break;
		} // Camera Editor
	case 'CELP':
		{
		rVal = WCS_TOOLBAR_ITEM_LPG; break;
		} // Celestial Object Editor
	case 'CLOD':
		{
		rVal = WCS_TOOLBAR_ITEM_CLG; break;
		} // Cloud Model Editor
	case 'CMAP':
		{
		rVal = WCS_TOOLBAR_ITEM_CMG; break;
		} // Color Map Editor
	case 'EDCO':
		{
		rVal = WCS_TOOLBAR_ITEM_CEG; break;
		} // Color Editor
	case 'DBOU':
		{
		rVal = WCS_TOOLBAR_ITEM_DBO; break;
		} // Export Database
	case 'DIAG':
		{
		rVal = WCS_TOOLBAR_ITEM_RDG; break;
		} // Diagnostic Data
	case 'DIGO':
		{
		rVal = WCS_TOOLBAR_ITEM_DIG; break;
		} // Create Palette
	case 'DNDL':
		{
		rVal = WCS_TOOLBAR_ITEM_DDL; break;
		} // Copy Object List
	case 'ECEF':
		{
		rVal = WCS_TOOLBAR_ITEM_ECG; break;
		} // Ecosystem Editor
	case 'ECTP':
		{
		rVal = WCS_TOOLBAR_ITEM_FEG; break;
		} // Ecotype Editor
	case 'GPLG':
		{
		rVal = WCS_TOOLBAR_ITEM_EFL; break;
		} // Effect List
	case 'ENVI':
		{
		rVal = WCS_TOOLBAR_ITEM_ENG; break;
		} // Environment Editor
	case 'EDFO':
		{
		rVal = WCS_TOOLBAR_ITEM_FLG; break;
		} // Foliage Effect Editor
	case 'LOUV':
		{
		rVal = WCS_TOOLBAR_ITEM_LVE; break;
		} // Component Gallery
	case 'GRND':
		{
		rVal = WCS_TOOLBAR_ITEM_GNG; break;
		} // Ground Editor
	case 'ILEF':
		{
		rVal = WCS_TOOLBAR_ITEM_IEG; break;
		} // Illumination Editor
	case 'IMVG':
		{
		rVal = WCS_TOOLBAR_ITEM_IVG; break;
		} // Image View
	case 'IMWZ':
		{
		rVal = WCS_TOOLBAR_ITEM_IWG; break;
		} // Import Wizard
	case 'WEDT':
		{
		rVal = WCS_TOOLBAR_ITEM_IDG; break;
		} // DEM Interpolater
	case 'KDSG':
		{
		rVal = WCS_TOOLBAR_ITEM_DKG; break;
		} // Scale or Remove Key Frames
	case 'LKEF':
		{
		rVal = WCS_TOOLBAR_ITEM_LEG; break;
		} // Lake Editor
	case 'SUNP':
		{
		rVal = WCS_TOOLBAR_ITEM_NPG; break;
		} // Light Editor
	case 'MATE':
		{
		rVal = WCS_TOOLBAR_ITEM_MAG; break;
		} // Material Editor
	case 'MATS':
		{
		rVal = WCS_TOOLBAR_ITEM_MSG; break;
		} // Material Strata Editor
	case 'NUME':
		{
		rVal = WCS_TOOLBAR_ITEM_NUM; break;
		} // Numeric Input
	case '3DOE':
		{
		rVal = WCS_TOOLBAR_ITEM_OEG; break;
		} // 3D Object Editor
	case 'PLAN':
		{
		rVal = WCS_TOOLBAR_ITEM_POG; break;
		} // Planet Options Editor
	case 'PRON':
		{
		rVal = WCS_TOOLBAR_ITEM_PNG; break;
		} // New Project
	case 'PREF':
		{
		rVal = WCS_TOOLBAR_ITEM_PPG; break;
		} // Preferences
	case 'ATEF':
		{
		rVal = WCS_TOOLBAR_ITEM_RTG; break;
		} // Area Terraffector Editor
	case 'RCTL':
		{
		rVal = WCS_TOOLBAR_ITEM_RCG; break;
		} // Render Control
	case 'RNDJ':
		{
		rVal = WCS_TOOLBAR_ITEM_RJG; break;
		} // Render Job Editor
	case 'RNDO':
		{
		rVal = WCS_TOOLBAR_ITEM_ROG; break;
		} // Render Options Editor
	case 'LWMO':
		{
		rVal = WCS_TOOLBAR_ITEM_LWG; break;
		} // Scene Export
	case 'SCIM':
		{
		rVal = WCS_TOOLBAR_ITEM_SIM; break;
		} // Scene Import
	case 'SHEF':
		{
		rVal = WCS_TOOLBAR_ITEM_SHG; break;
		} // Shadow Effect Editor
	case 'SKYP':
		{
		rVal = WCS_TOOLBAR_ITEM_KPG; break;
		} // Sky Editor
	case 'SNOW':
		{
		rVal = WCS_TOOLBAR_ITEM_SNG; break;
		} // Snow Editor
	case 'STAR':
		{
		rVal = WCS_TOOLBAR_ITEM_STG; break;
		} // Starfield Editor
	case 'STRM':
		{
		rVal = WCS_TOOLBAR_ITEM_SEG; break;
		} // Stream Editor
	case 'EPTS':
		{
		rVal = WCS_TOOLBAR_ITEM_SAG; break;
		} // Light Position by Time
	case 'TGRD':
		{
		rVal = WCS_TOOLBAR_ITEM_GRD; break;
		} // Terrain Gridder
	case 'TERF':
		{
		rVal = WCS_TOOLBAR_ITEM_TAG; break;
		} // Terraffector Editor
	case 'TERP':
		{
		rVal = WCS_TOOLBAR_ITEM_TPG; break;
		} // Terrain Parameter Editor
	case 'TEXG':
		{
		rVal = WCS_TOOLBAR_ITEM_TXG; break;
		} // Texture Editor
	case 'VTEG':
		{
		rVal = WCS_TOOLBAR_ITEM_VEG; break;
		} // Vector Editor
	case 'VPEF':
		{
		rVal = WCS_TOOLBAR_ITEM_VPG; break;
		} // Vector Profile Editor
	case 'VERS':
		{
		rVal = WCS_TOOLBAR_ITEM_VER; break;
		} // Version
	case 'WVED':
		{
		rVal = WCS_TOOLBAR_ITEM_WEG; break;
		} // Wave Model Editor
	case 'SEQU':
		{
		rVal = WCS_TOOLBAR_ITEM_SQU; break;
		} // Search Query Editor
	case 'THEM':
		{
		rVal = WCS_TOOLBAR_ITEM_THM; break;
		} // Thematic Map Editor
	case 'COSY':
		{
		rVal = WCS_TOOLBAR_ITEM_COS; break;
		} // Coordinate System Editor
	case 'DEME':
		{
		rVal = WCS_TOOLBAR_ITEM_DEM; break;
		} // DEM Editor
	case 'DEMP':
		{
		rVal = WCS_TOOLBAR_ITEM_DPG; break;
		} // DEM Paint Editor
	case 'FNCE':
		{
		rVal = WCS_TOOLBAR_ITEM_FCG; break;
		} // Fence Editor
	case 'PPRC':
		{
		rVal = WCS_TOOLBAR_ITEM_PPR; break;
		} // Post Process Editor
	case 'PTHT':
		{
		rVal = WCS_TOOLBAR_ITEM_PTH; break;
		} // Path Transfer Editor
	case 'DMRG':
		{
		rVal = WCS_TOOLBAR_ITEM_MRG; break;
		} // DEM Merger Editor
	case 'SCEN':
		{
		rVal = WCS_TOOLBAR_ITEM_SCN; break;
		} // Render Scenario Editor
	case 'VPEX':
		{
		rVal = WCS_TOOLBAR_ITEM_VPX; break;
		} // VecProfExportGUI
	case 'CSCU':
		{
		rVal = WCS_TOOLBAR_ITEM_CSC; break;
		} // CoordsCalculatorGUI
	case 'DRIL':
		{
		rVal = WCS_TOOLBAR_ITEM_DRL; break;
		} // DrillDownInfoGUI
	case 'RTXP':
		{
		rVal = WCS_TOOLBAR_ITEM_EXP; break;
		} // ExporterEditGUI
	case 'ECTL':
		{
		rVal = WCS_TOOLBAR_ITEM_EXG; break;
		} // ExportControlGUI
	case 'AUTH':
		{
		rVal = WCS_TOOLBAR_ITEM_AUT; break;
		} // Authorization
	case 'FOWZ':
		{
		rVal = WCS_TOOLBAR_ITEM_FWZ; break;
		} // ForestWizGUI
	case 'GWIZ':
		{
		rVal = WCS_TOOLBAR_ITEM_GWZ; break;
		} // ForestWizGUI
	case 'MWIZ':
		{
		rVal = WCS_TOOLBAR_ITEM_MWZ; break;
		} // ForestWizGUI
	case 'LABL':
		{
		rVal = WCS_TOOLBAR_ITEM_LBL; break;
		} // LabelEditGUI
	// <<<>>> ADD_NEW_EFFECTS add case for new editor windows
	default:
		rVal = 0;
		break;
	} // switch

return(rVal);

} // Toolbar::NotifyFromID

/*===========================================================================*/

unsigned long int Toolbar::MajorWindowNotifyFromID(unsigned long int WinID)
{
unsigned long int rVal;

switch(WinID)
	{
	case 'DBED':
		{
		rVal = WCS_TOOLBAR_ITEM_DBG; break;
		} // DBEditGUI
	case 'PREF':
		{
		rVal = WCS_TOOLBAR_ITEM_PPG; break;
		} // ProjectPrefsGUI
	case 'EFFL':
		{
		rVal = WCS_TOOLBAR_ITEM_EFG; break;
		} // Effects Lib
	case 'IMGL':
		{
		rVal = WCS_TOOLBAR_ITEM_ILG; break;
		} // Image Library Dialog
	case 'SAAG':
		{
		rVal = WCS_TOOLBAR_ITEM_SAG; break;
		} // Scene at a Glance Dialog
	default:
		rVal = 0;
		break;
	} // switch

return(rVal);

} // Toolbar::MajorWindowNotifyFromID

/*===========================================================================*/

void Toolbar::CloseDangerousWindows(void)
{

if (GlobalApp->GUIWins->VEG)
	{
	delete GlobalApp->GUIWins->VEG;
	GlobalApp->GUIWins->VEG = NULL;
	} // if Vector Editor
if (GlobalApp->GUIWins->VPG)
	{
	delete GlobalApp->GUIWins->VPG;
	GlobalApp->GUIWins->VPG = NULL;
	} // if Vector Profile Editor
if (GlobalApp->GUIWins->DIG)
	{
	delete GlobalApp->GUIWins->DIG;
	GlobalApp->GUIWins->DIG = NULL;
	} // if Digitize GUI Editor
#ifdef WCS_FORESTRY_WIZARD
if (GlobalApp->GUIWins->FWZ)
	{
	delete GlobalApp->GUIWins->FWZ;
	GlobalApp->GUIWins->FWZ = NULL;
	} // if Forestry Wizard
#endif // WCS_FORESTRY_WIZARD

} // Toolbar::CloseDangerousWindows

/*===========================================================================*/

Conservatory::~Conservatory()
{
if (NUM) delete NUM; NUM = NULL;
if (IWG) delete IWG; IWG = NULL;
if (RDG) delete RDG; RDG = NULL;
if (CVG) delete CVG; CVG = NULL;
if (TXG) delete TXG; TXG = NULL;
if (CEG) delete CEG; CEG = NULL;
if (EFL) delete EFL; EFL = NULL;

if (CLG) delete CLG; CLG = NULL;
if (WEG) delete WEG; WEG = NULL;
if (GRD) delete GRD; GRD = NULL;

if (DBE) delete DBE; DBE = NULL;
//delete DBO;	can only be opened from DBE and DBE closes it
if (LWG) delete LWG; LWG = NULL;

if (FEG) delete FEG; FEG = NULL;
if (PPG) delete PPG; PPG = NULL;

if (IDG) delete IDG; IDG = NULL;
if (PNG) delete PNG; PNG = NULL;
if (DIG) delete DIG; DIG = NULL;

if (VER) delete VER; VER = NULL;

if (VEG) delete VEG; VEG = NULL;

//if (CRG) delete CRG; CRG = NULL;

if (SIM) delete SIM; SIM = NULL;
if (LEG) delete LEG; LEG = NULL;
if (EFG) delete EFG; EFG = NULL;

if (ECG) delete ECG; ECG = NULL;
if (RTG) delete RTG; RTG = NULL;
if (TAG) delete TAG; TAG = NULL;

if (SHG) delete SHG; SHG = NULL;
if (SPG) delete SPG; SPG = NULL;
if (NPG) delete NPG; NPG = NULL;
if (CPG) delete CPG; CPG = NULL;
if (TPG) delete TPG; TPG = NULL;
if (KPG) delete KPG; KPG = NULL;

if (LPG) delete LPG; LPG = NULL;
if (FLG) delete FLG; FLG = NULL;
if (FFG) delete FFG; FFG = NULL;
if (SEG) delete SEG; SEG = NULL;

if (VPG) delete VPG; VPG = NULL;
if (OEG) delete OEG; OEG = NULL;
if (MAG) delete MAG; MAG = NULL;

if (ILG) delete ILG; ILG = NULL;
if (DKG) delete DKG; DKG = NULL;

if (AEG) delete AEG; AEG = NULL;
if (CMG) delete CMG; CMG = NULL;

if (ENG) delete ENG; ENG = NULL;
if (GNG) delete GNG; GNG = NULL;
if (POG) delete POG; POG = NULL;
if (RJG) delete RJG; RJG = NULL;
if (ROG) delete ROG; ROG = NULL;

if (SNG) delete SNG; SNG = NULL;
if (STG) delete STG; STG = NULL;
if (MSG) delete MSG; MSG = NULL;
if (LVE) delete LVE; LVE = NULL;
if (IVG) delete IVG; IVG = NULL;

if (RCG) delete RCG; RCG = NULL;
if (TGN) delete TGN; TGN = NULL;
if (SQU) delete SQU; SQU = NULL;
if (THM) delete THM; THM = NULL;
if (TBG) delete TBG; TBG = NULL;
if (COS) delete COS; COS = NULL;
if (FCG) delete FCG; FCG = NULL;
if (DEM) delete DEM; DEM = NULL;
if (DPG) delete DPG; DPG = NULL;
if (TPM) delete TPM; TPM = NULL;
if (PTH) delete PTH; PTH = NULL;
if (DSS) delete DSS; DSS = NULL;
if (PPR) delete PPR; PPR = NULL;
if (SCN) delete SCN; SCN = NULL;
if (LBL) delete LBL; LBL = NULL;
if (VPX) delete VPX; VPX = NULL;
if (MRG) delete MRG; MRG = NULL;
if (CSC) delete CSC; CSC = NULL;
if (DRL) delete DRL; DRL = NULL;
#ifdef WCS_FORESTRY_WIZARD
if (FWZ) delete FWZ; FWZ = NULL;
#endif // WCS_FORESTRY_WIZARD
if (GWZ) delete GWZ; GWZ = NULL;
if (MWZ) delete MWZ; MWZ = NULL;
#ifdef WCS_BUILD_RTX
if (EXP) delete EXP; EXP = NULL;
if (EXG) delete EXG; EXG = NULL;
#endif // WCS_BUILD_RTX

// <<<>>> ADD_NEW_EFFECTS delete the window's pointer when closing the program

if (DDL[0]) delete DDL[0]; DDL[0] = NULL;
if (DDL[1]) delete DDL[1]; DDL[1] = NULL;
if (DDL[2]) delete DDL[2]; DDL[2] = NULL;
if (DDL[3]) delete DDL[3]; DDL[3] = NULL;
if (DDL[4]) delete DDL[4]; DDL[4] = NULL;
if (SAG) delete SAG; SAG = NULL;

if (GRG[0]) delete GRG[0]; GRG[0] = NULL;
if (GRG[1]) delete GRG[1]; GRG[1] = NULL;
if (GRG[2]) delete GRG[2]; GRG[2] = NULL;
if (GRG[3]) delete GRG[3]; GRG[3] = NULL;
if (GRG[4]) delete GRG[4]; GRG[4] = NULL;
if (GRG[5]) delete GRG[5]; GRG[5] = NULL;
if (GRG[6]) delete GRG[6]; GRG[6] = NULL;
if (GRG[7]) delete GRG[7]; GRG[7] = NULL;
if (GRG[8]) delete GRG[8]; GRG[8] = NULL;
if (GRG[9]) delete GRG[9]; GRG[9] = NULL;

} // Conservatory::~Conservatory
