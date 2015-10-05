// FenetrePopup.cpp
// Popup menu code
// Created from Fenetre.cpp 11/24/99 by Chris "Xenon" Hanson

#include "stdafx.h"
#include "Application.h"
#include "Fenetre.h"
#include "GUI.h"
#include "AppModule.h"
#include "Project.h"
#include "WCSWidgets.h"
#include "Useful.h"
#include "Log.h"
#include "AppMem.h"
#include "Requester.h"
#include "Interactive.h"
#include "EffectsLib.h"
#include "Conservatory.h"
#include "ViewGUI.h"
#include "Security.h"
#include "resource.h"
#include "FeatureConfig.h"

extern int RTFLoaded;

#ifdef _WIN32

int Fenetre::InternalDoPopup(char FenType, NativeGUIWin NGW, short X, short Y, char LeftButton)
{
char IsDocked = 0, DockEnabled = 0, ViewEnabled = 0, IsView = 0, IsMe, AdvEnabled = 0, AdvChecked = 0,
 IsViewWin = 0, IsDiag = 0, IsRoot = 0, RenderPrevEnabled = 0, RenderPrev = 0, CanSave = 0,
 HasRenderOpt = 0;
HMENU JoySub = NULL;

HMENU ViewSub, ShowSub, RenderSub /*, OptionsSub */;
HMENU ShowTerrain, ShowLandCover, ShowWater, ShowSky, ShowLight, ShowRender, ShowOverlay, ShowGradient;
HMENU RenderOpSet, RenderTerrain, RenderLandCover, RenderWater, RenderSky, RenderLight, RenderRender;
int ItemIdx, ViewPane, ViewNum;
GeneralEffect *MyEffect;
//unsigned char NewState;
ViewGUI *VG = NULL;
ViewContext *VC = NULL;
RenderOpt *RO = NULL;

POINT HitPoints;
HitPoints.x = X;
HitPoints.y = Y;
ScreenToClient(LocalWinSys()->RootWin, &HitPoints);

ViewSub = ShowSub = RenderSub = /*OptionsSub = */ NULL;
ShowTerrain = ShowLandCover = ShowWater = ShowSky = ShowLight = ShowRender = ShowOverlay = ShowGradient = NULL;
RenderOpSet = RenderTerrain = RenderLandCover = RenderWater = RenderSky = RenderLight = RenderRender = NULL;

if(PopupMenu) DestroyMenu(PopupMenu); PopupMenu = NULL;
PopupX = X;
PopupY = Y;

if((LocalWinSys()->ModalLevel > 0) && !ModalInhibit)
	return(0); // no menus for modal windows

#ifndef WCS_BUILD_DEMO

if(!GlobalApp->Sentinal->CheckDongle()) return(0); // none for you!
#endif // WCS_BUILD_DEMO


IsViewWin    = (TestWinManFlags(WCS_FENETRE_WINMAN_ISVIEW) ? 1 : 0);
IsRoot = ((FenID == 'TOOL') ? 1 : 0);
ViewEnabled  = (IsRoot || IsViewWin); // Only toolbar and views support View submenu

if(ViewEnabled)
	{
	ViewPane = GlobalApp->MainProj->ViewPorts.GetPaneFromXY((short)HitPoints.x, (short)HitPoints.y); // use window client coords, not screen coords
	if(ViewPane == -1)
		{
		ViewEnabled = 0;
		} // if
	else
		{
		if(GlobalApp->MainProj->ViewPorts.TestFlag(GlobalApp->MainProj->ViewPorts.GetPaneID(WCS_PROJECT_WINLAYOUT_CURRENT_LAYOUT, ViewPane), WCS_PROJECT_WINLAYOUT_FLAG_ISVIEW))
			{
			IsView = 1;
			} // if
		} // else
	} // if


IsDiag       = (TestWinManFlags(WCS_FENETRE_WINMAN_ISDIAG) ? 1 : 0); // Both Views and Render Preview Window flag this. It isn't the dignostics window itself
IsDocked     = (TestWinManFlags(WCS_FENETRE_WINMAN_ISDOCKED) ? 1 : 0);
DockEnabled  = (TestWinManFlags(WCS_FENETRE_WINMAN_NODOCK) ? 0 : 1); // negative sense
AdvEnabled   = InquireWindowCapabilities(WCS_FENETRE_WINCAP_CANSHOWADV);
AdvChecked   = (TestWinManFlags(WCS_FENETRE_WINMAN_SHOWADV) ? 1 : 0);

if(GlobalApp->MainProj->Prefs.GlobalAdvancedEnabled) // need to check global Advanced override state
	{
	AdvEnabled = 0; // grey out the option if globally overridden
	} // if

if(IsViewWin)
	{
	if(VG = GlobalApp->GUIWins->CVG)
		{
		ViewNum = VG->IdentViewOrigin(Owner->GetEvent());
		if(VC = VG->ViewSpaces[ViewNum])
			{
			RenderPrevEnabled = VC->CheckRendererPresent();
			RenderPrev = VC->GetEnabled(WCS_VIEWGUI_ENABLE_RENDERPREV);
			HasRenderOpt = ((RO = VC->GetRenderOpt()) ? 1 : 0);
			CanSave = 1;
			} // if
		} // if
	} // if

if (IsDiag)
	{
	if ((IsViewWin && RenderPrev && RenderPrevEnabled) || ! IsViewWin)
		CanSave = 1;
	} // if

PopupMenu = CreatePopupMenu();

if(ViewEnabled)
	{
	if(ViewSub = CreatePopupMenu())
		{
		AppendMenu(ViewSub, MF_STRING | MF_ENABLED | (IsView ? 0 : MF_CHECKED), ID_WINMENU_VIEW_EDITORS, "Editors");
		for (ItemIdx = 0, MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_CAMERA); MyEffect; MyEffect = MyEffect->Next)
			{
			IsMe = 0;
			if(VC && MyEffect == VC->GetCamera())
				{
				IsMe = 1;
				} // if
			if(ItemIdx < WCS_VIEWGUI_VIEWS_CAMSINPOPUP_MAX)
				{
				AppendMenu(ViewSub, MF_STRING | MF_ENABLED | (IsMe ? MF_CHECKED : 0), ID_WINMENU_VIEW_CAM1 + (ItemIdx++), MyEffect->GetName());
				} // if
			} // for
		AppendMenu(ViewSub, MF_STRING | MF_ENABLED, ID_WINMENU_VIEW_NEWPERSPEC, "New Perspective Camera");
		AppendMenu(ViewSub, MF_STRING | MF_ENABLED, ID_WINMENU_VIEW_NEWOVER, "New Overhead Camera");
		AppendMenu(ViewSub, MF_STRING | MF_ENABLED, ID_WINMENU_VIEW_NEWPLAN, "New Planimetric Camera");
		} // if
	} // if

if(IsViewWin)
	{
	if(ShowSub = CreatePopupMenu())
		{
		AppendMenu(ShowSub, MF_STRING | MF_ENABLED, ID_WINMENU_VIEW_REFRESH, "Refresh View [V]");
		AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetViewAlive(ViewNum) ? MF_CHECKED : 0), ID_WINMENU_VIEW_WINENABLED, "View Window Enabled");
		ShowTerrain = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowTerrain, "Terrain");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CONTROLPOINTS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CONTROLPOINTS, "Control Points");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_MEASURES) ? MF_CHECKED : 0), IS_WINMENU_SHOW_MEASURES, "Reference Grid");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_ACTIVEOBJ) ? MF_CHECKED : 0), ID_WINMENU_SHOW_ACTIVEOBJECT, "Active Object");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_DEMEDGE) ? MF_CHECKED : 0), ID_WINMENU_SHOW_DEMEDGES, "DEM Edges");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_TERRAIN) ? MF_CHECKED : 0), ID_WINMENU_SHOW_TERRAIN, "Terrain");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_TERRAIN_TRANS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_TERRAIN_TRANS, "Terrain Transparency");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_POLYEDGE) ? MF_CHECKED : 0), ID_WINMENU_SHOW_TERRAIN_POLY, "Terrain Polygon Edges");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_TERRAFX) ? MF_CHECKED : 0), ID_WINMENU_SHOW_TFX, "Terraffectors");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_AREATERRAFX) ? MF_CHECKED : 0), ID_WINMENU_SHOW_ATFX, "Area Terraffectors");
#ifdef WCS_VIEW_TERRAFFECTORS
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (GlobalApp->MainProj->Interactive->GetTfxPreview() ? MF_CHECKED : 0), ID_WINMENU_SHOW_TFXVIEW, "Terraffector Preview (Common)");
			AppendMenu(ShowTerrain, MF_STRING | MF_ENABLED | (GlobalApp->MainProj->Interactive->GetTfxRealtime() ? MF_CHECKED : 0), ID_WINMENU_SHOW_TFXVIEW_AUTO, "TFX Preview AutoUpdate (Common)");
#endif // !WCS_VIEW_TERRAFFECTORS

		ShowLandCover = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowLandCover, "Land Cover");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_GROUND) ? MF_CHECKED : 0), ID_WINMENU_SHOW_GROUND, "Ground Components");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_FOLIAGEFX) ? MF_CHECKED : 0), ID_WINMENU_SHOW_FOLIAGEFX, "Foliage Effects");
			// WCS_VIEWGUI_ENABLE_RTFOLIAGE
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_RTFOLIAGE) ? MF_CHECKED : 0), ID_WINMENU_SHOW_RTFOLIAGE, "Realtime Foliage Images");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_ECOFX) ? MF_CHECKED : 0), ID_WINMENU_SHOW_ECOFX, "Vector Ecosystems");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CMAPS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_COLORMAPS, "Color Maps");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SNOW) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SNOW, "Snow");
			AppendMenu(ShowLandCover, MF_SEPARATOR, 0, NULL);
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_RTFOLFILE) ? MF_CHECKED : 0), ID_WINMENU_SHOW_RTFOLFILE, "Realtime Foliage File");
			AppendMenu(ShowLandCover, MF_STRING | MF_ENABLED, ID_WINMENU_SHOW_EDITRTF, "Realtime Foliage File Preferences...\t/");

		ShowWater = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowWater, "Water");
			AppendMenu(ShowWater, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_LAKES) ? MF_CHECKED : 0), ID_WINMENU_SHOW_WATER, "Lakes");
			AppendMenu(ShowWater, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_STREAMS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_STREAMS, "Streams");
			AppendMenu(ShowWater, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_WAVES) ? MF_CHECKED : 0), ID_WINMENU_SHOW_WAVES, "Wave Models");

		ShowSky = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowSky, "Sky Features");
			AppendMenu(ShowSky, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SKY) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SKY, "Sky");
			AppendMenu(ShowSky, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_ATMOS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_HAZE, "Atmosphere");
			AppendMenu(ShowSky, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CLOUDS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CLOUDS, "Cloud Models");
			AppendMenu(ShowSky, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CELEST) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CELESTIALOBJECTS, "Celestial Objects");

		ShowLight = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowLight, "Light && Shadow");
			//AppendMenu(ShowLight, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_ILLUMFX) ? MF_CHECKED : 0), ID_WINMENU_SHOW_LIGHTFX, "Illumination Effects");
			AppendMenu(ShowLight, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_LIGHTS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_LIGHTS, "Lights");
			AppendMenu(ShowLight, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_TERRAINSHADOWS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SHADOWFX, "Terrain Shadows");

		AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_3DOBJ) ? MF_CHECKED : 0), ID_WINMENU_SHOW_3DOBJS, "3D Objects");
		#ifdef WCS_VIEW_DRAWWALLS
		AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_WALLS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_WALLS, "Walls");
		#endif // WCS_VIEW_DRAWWALLS
		AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_PLAINVEC) ? MF_CHECKED : 0), ID_WINMENU_SHOW_VECTORS, "Plain Vectors");

		ShowRender = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowRender, "Camera");
			AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CAMERAS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CAMERAS, "Cameras");
			AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_TARGETS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_TARGETS, "Targets");
			//AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SAFEAREA) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SAFETITLE, "Safe Title Area");
			//AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SAFEAREA) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SAFEVIDEO, "Safe Video Area");
			AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SAFEAREA) ? MF_CHECKED : 0), ID_WINMENU_SHOW_IMGBOUNDS, "Image Boundaries");
			AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SAFETITLE) ? MF_CHECKED : 0), ID_WINMENU_OPTIONS_SAFETITLE, "Safe Title Area");
			AppendMenu(ShowRender, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_SAFEACTION) ? MF_CHECKED : 0), ID_WINMENU_OPTIONS_SAFEACTION, "Safe Action Area");

		ShowOverlay = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowOverlay, "Overlay");
			AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_CONTOURS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CONTOURS, "Contours");
			AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_SLOPE) ? MF_CHECKED : 0), ID_WINMENU_SHOW_SLOPE, "Slope Map");
			AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_RELEL) ? MF_CHECKED : 0), ID_WINMENU_SHOW_RELEL, "Relative Elevation Map");
			AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_FRACTAL) ? MF_CHECKED : 0), ID_WINMENU_SHOW_LOD, "Fractal Map");
			AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_ECOSYS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_ECOSYSTEMS, "Ecosystem Map");
			//AppendMenu(ShowOverlay, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_OVER_RENDER) ? MF_CHECKED : 0), ID_WINMENU_SHOW_RENDEREDTEXTURE, "Rendered Texture");

		ShowGradient = CreatePopupMenu();
		AppendMenu(ShowSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)ShowGradient, "Gradient");
			AppendMenu(ShowGradient, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_GRAD_GREY) ? MF_CHECKED : 0), ID_WINMENU_SHOW_GREY, "Grey Elevation");
			AppendMenu(ShowGradient, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_GRAD_EARTH) ? MF_CHECKED : 0), ID_WINMENU_SHOW_EARTH, "Earthtone Elevation");
			AppendMenu(ShowGradient, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_GRAD_PRIMARY) ? MF_CHECKED : 0), ID_WINMENU_SHOW_COLOR, "Rainbow Elevation");
			AppendMenu(ShowGradient, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_GRADREPEAT) ? MF_CHECKED : 0), ID_WINMENU_SHOW_MULTIGRAD, "Elev Gradient Repeat");

		AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_CURSOR) ? MF_CHECKED : 0), ID_WINMENU_SHOW_CURSOR, "Cursor");
#ifdef WCS_BUILD_RTX
		if(GlobalApp->SXAuthorized)
			AppendMenu(ShowSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_EXPORTERS) ? MF_CHECKED : 0), ID_WINMENU_SHOW_EXPORTERS, "Scene Exporters");
#endif // WCS_BUILD_RTX
		//AppendMenu(ShowSub, MF_SEPARATOR, 0, NULL);

		AppendMenu(ShowSub, MF_SEPARATOR, 0, NULL);
		AppendMenu(ShowSub, MF_STRING | (IsViewWin && HasRenderOpt ? MF_ENABLED : MF_GRAYED), ID_WINMENU_CREATEFOLFILE, "Create Realtime Foliage File");
		AppendMenu(ShowSub, MF_STRING | (IsViewWin && HasRenderOpt ? MF_ENABLED : MF_GRAYED) | (RTFLoaded ? MF_CHECKED : 0), ID_WINMENU_LOADFOLFILE, "Load Realtime Foliage File");

		AppendMenu(ShowSub, MF_SEPARATOR, 0, NULL);
		AppendMenu(ShowSub, MF_STRING | MF_ENABLED, ID_WINMENU_SHOW_COPYRENDER, "Copy Render Opt");
		} // if
	if(RenderSub = CreatePopupMenu())
		{
		if(HasRenderOpt && RO)
			{
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED, ID_WINMENU_VIEW_EDITRO, "Edit View's Render Options...");
			RenderTerrain = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderTerrain, "Terrain");
				AppendMenu(RenderTerrain, MF_STRING | MF_ENABLED | (RO->TerrainEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_TERRAIN, "Terrain");
				AppendMenu(RenderTerrain, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_TERRAFFECTOR] ? MF_CHECKED : 0), ID_WINMENU_RENDER_TFX, "Terraffectors");
				AppendMenu(RenderTerrain, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_RASTERTA] ? MF_CHECKED : 0), ID_WINMENU_RENDER_ATFX, "Area Terraffectors");

			RenderLandCover = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderLandCover, "Land Cover");
				AppendMenu(RenderLandCover, MF_STRING | MF_ENABLED | (RO->FoliageEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_FOLIAGE, "Other Foliage");
				AppendMenu(RenderLandCover, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FOLIAGE] ? MF_CHECKED : 0), ID_WINMENU_RENDER_FOLIAGEFX, "Foliage Effects");
				AppendMenu(RenderLandCover, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ECOSYSTEM] ? MF_CHECKED : 0), ID_WINMENU_RENDER_ECOFX, "Vector Ecosystems");
				AppendMenu(RenderLandCover, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CMAP] ? MF_CHECKED : 0), ID_WINMENU_RENDER_COLORMAPS, "Color Maps");
				AppendMenu(RenderLandCover, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SNOW] ? MF_CHECKED : 0), ID_WINMENU_RENDER_SNOW, "Snow");

			RenderWater = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderWater, "Water");
				AppendMenu(RenderWater, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LAKE] ? MF_CHECKED : 0), ID_WINMENU_RENDER_WATER, "Lakes");
				AppendMenu(RenderWater, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STREAM] ? MF_CHECKED : 0), ID_WINMENU_RENDER_STREAMS, "Streams");
				AppendMenu(RenderWater, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_WAVE] ? MF_CHECKED : 0), ID_WINMENU_RENDER_WAVES, "Wave Models");
				AppendMenu(RenderWater, MF_STRING | MF_ENABLED | (RO->ReflectionsEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_REFLECTIONS, "Reflections");

			RenderSky = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderSky, "Sky Features");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SKY] ? MF_CHECKED : 0), ID_WINMENU_RENDER_SKY, "Sky");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_ATMOSPHERE] ? MF_CHECKED : 0), ID_WINMENU_RENDER_HAZE, "Atmosphere");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CLOUD] ? MF_CHECKED : 0), ID_WINMENU_RENDER_CLOUDS, "Cloud Models");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->VolumetricsEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_VOLUMETRICS, "Volumetrics");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_CELESTIAL] ? MF_CHECKED : 0), ID_WINMENU_RENDER_CELESTIALOBJECTS, "Celestial Objects");
				AppendMenu(RenderSky, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_STARFIELD] ? MF_CHECKED : 0), ID_WINMENU_RENDER_STARS, "Starfields");

			RenderLight = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderLight, "Light && Shadow");
				//AppendMenu(RenderLight, MF_STRING | MF_ENABLED, ID_WINMENU_RENDER_LIGHTFX, "Illumination Effects");
				AppendMenu(RenderLight, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LIGHT] ? MF_CHECKED : 0), ID_WINMENU_RENDER_LIGHTS, "Lights");
				AppendMenu(RenderLight, MF_STRING | MF_ENABLED | (RO->CloudShadowsEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_CLOUDSHADOW, "Cloud Shadows");
				AppendMenu(RenderLight, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_SHADOW] ? MF_CHECKED : 0), ID_WINMENU_RENDER_SHADOWFX, "Terrain Shadows");
				AppendMenu(RenderLight, MF_STRING | MF_ENABLED | (RO->ObjectShadowsEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_3DOBJSHADOW, "3D Object Shadows");

			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_OBJECT3D] ? MF_CHECKED : 0), ID_WINMENU_RENDER_3DOBJS, "3D Objects");
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_FENCE] ? MF_CHECKED : 0), ID_WINMENU_RENDER_WALLS, "Walls");
			#ifdef WCS_LABEL
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (RO->EffectEnabled[WCS_EFFECTSSUBCLASS_LABEL] ? MF_CHECKED : 0), ID_WINMENU_RENDER_LABELS, "Labels");
			#endif // WCS_LABEL
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (RO->VectorsEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_VECTORS, "Plain Vectors");

			RenderRender = CreatePopupMenu();
			AppendMenu(RenderSub, MF_STRING | MF_POPUP | MF_ENABLED, (UINT)RenderRender, "Camera");
				AppendMenu(RenderRender, MF_STRING | MF_ENABLED | (RO->DepthOfFieldEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_DOF, "Depth-of-Field");
				AppendMenu(RenderRender, MF_STRING | MF_ENABLED | (RO->MultiPassAAEnabled ? MF_CHECKED : 0), ID_WINMENU_RENDER_MULTIAA, "MultiAA/Motion Blur");
				AppendMenu(RenderRender, MF_STRING | MF_ENABLED | (RO->RenderDiagnosticData ? MF_CHECKED : 0), ID_WINMENU_RENDER_DIAG, "Render Diagnostic Data");

			AppendMenu(RenderSub, MF_SEPARATOR, 0, NULL);
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (VG->GetEnabled(ViewNum, WCS_VIEWGUI_ENABLE_LTDREGION) ? MF_CHECKED : 0), ID_WINMENU_OPTIONS_CONSTRAIN, "Constrain Render Area\tF6");
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED | (0 ? MF_CHECKED : 0), ID_WINMENU_OPTIONS_SETAREA, "Set Render Area\tF5");
			AppendMenu(RenderSub, MF_SEPARATOR, 0, NULL);
			//AppendMenu(RenderSub, MF_STRING | MF_ENABLED, ID_WINMENU_RENDER_EDITMORE, "View Preferences...");
			AppendMenu(RenderSub, MF_STRING | MF_ENABLED, ID_WINMENU_RENDER_COPYREALTIME, "Copy Realtime Opt");
			} // if

		} // if
/*	if(OptionsSub = CreatePopupMenu())
		{ // ID_WINMENU_OPTIONS_FOLLOW
		GlobalApp->MainProj->Interactive->GetParam(&NewState, MAKE_ID(WCS_INTERCLASS_MAPVIEW, 0, WCS_INTERMAP_ITEM_FOLLOWTERRAIN, 0));
		AppendMenu(OptionsSub, MF_STRING | MF_ENABLED | (NewState ? MF_CHECKED : 0), ID_WINMENU_OPTIONS_FOLLOW, "Follow");
		} // if
*/
	if(JoySub = CreatePopupMenu())
		{
		if(VG)
			{
			AppendMenu(JoySub, MF_STRING | MF_ENABLED | (VG->GetMotionControl() && !VG->GetMotionControlMode() ? MF_CHECKED : 0), ID_WINMENU_JOY_DRIVE, "Drive\tj");
			AppendMenu(JoySub, MF_STRING | MF_ENABLED | (VG->GetMotionControl() && VG->GetMotionControlMode() ? MF_CHECKED : 0), ID_WINMENU_JOY_SLIDE, "MultiAxis\tJ");
			} // if
		} // if

	} // if

if(PopupMenu)
	{
	AppendMenu(PopupMenu, MF_STRING | MF_ENABLED, ID_WINMENU_HELP, "Help\tF1");
	AppendMenu(PopupMenu, MF_STRING | (AdvEnabled ? MF_ENABLED: MF_GRAYED) | (AdvChecked ? MF_CHECKED : 0), ID_WINMENU_SHOWADVANCED, "Advanced Features");
	AppendMenu(PopupMenu, MF_STRING | (DockEnabled  ? MF_ENABLED : MF_GRAYED) | (IsDocked ? MF_CHECKED : 0), ID_WINMENU_DOCK, "Dock");
	AppendMenu(PopupMenu, MF_STRING | MF_POPUP | (ViewEnabled  ? MF_ENABLED : MF_GRAYED), (UINT)ViewSub, "View");
	AppendMenu(PopupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(PopupMenu, MF_STRING | (IsViewWin  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_RENDER_EDITMORE, "Open View Preferences...\t?");
	if(IsViewWin) AppendMenu(PopupMenu, MF_STRING | (IsViewWin  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_VIEW_EDITCAM, "Edit View's Camera...");
	//AppendMenu(PopupMenu, MF_STRING | (IsDiag  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_DIAGNOSTICS, "Open Diagnostics...\tF4");
	AppendMenu(PopupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(PopupMenu, MF_STRING | MF_POPUP | (IsViewWin  ? MF_ENABLED : MF_GRAYED), (UINT)ShowSub, "Realtime Options");
	AppendMenu(PopupMenu, MF_STRING | (IsViewWin  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_MAKE_QUICK, "Make Quick Sequence...");
	AppendMenu(PopupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(PopupMenu, MF_STRING | MF_POPUP | (IsViewWin  ? MF_ENABLED : MF_GRAYED), (UINT)RenderSub, "Render Options");
	//AppendMenu(PopupMenu, MF_STRING | MF_POPUP | (IsViewWin  ? MF_ENABLED : MF_GRAYED), (UINT)OptionsSub, "Options");
	RenderOpSet = CreatePopupMenu();
	AppendMenu(PopupMenu, MF_STRING | MF_POPUP | (IsViewWin  ? MF_ENABLED : MF_GRAYED), (UINT)RenderOpSet, "Select Option Set");
	for (ItemIdx = 0, MyEffect = GlobalApp->AppEffects->GetListPtr(WCS_EFFECTSSUBCLASS_RENDEROPT); MyEffect; MyEffect = MyEffect->Next)
		{
		IsMe = 0;
		if(VC && MyEffect == VC->GetRenderOpt())
			{
			IsMe = 1;
			} // if
		if(ItemIdx < WCS_VIEWGUI_VIEWS_ROPTINPOPUP_MAX)
			{
			AppendMenu(RenderOpSet, MF_STRING | MF_ENABLED | (IsMe ? MF_CHECKED : 0), ID_WINMENU_VIEW_ROPT1 + ItemIdx++, MyEffect->GetName());
			} // if
		} // for
	AppendMenu(PopupMenu, MF_STRING | (IsViewWin && HasRenderOpt ? MF_ENABLED : MF_GRAYED), ID_WINMENU_RENDERPREV, "Render a Preview\tF9");
	AppendMenu(PopupMenu, MF_STRING | (IsViewWin && RenderPrevEnabled ? MF_ENABLED : MF_GRAYED)| (RenderPrev  ? MF_CHECKED : 0), ID_WINMENU_SHOWPREV, "Show Preview\tF8");
	//AppendMenu(PopupMenu, MF_STRING | (IsViewWin  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_RENDERDETACHED, "Render Detached");
	AppendMenu(PopupMenu, MF_STRING | (CanSave  ? MF_ENABLED : MF_GRAYED), ID_WINMENU_SAVEIMAGE, "Save Displayed Image...\tF7");
	AppendMenu(PopupMenu, MF_SEPARATOR, 0, NULL);
	AppendMenu(PopupMenu, MF_STRING | MF_POPUP | ((VG && IsViewWin)  ? MF_ENABLED : MF_GRAYED), (UINT)JoySub, "Joystick");
	AppendMenu(PopupMenu, MF_STRING | (IsRoot ? MF_GRAYED : MF_ENABLED), ID_WINMENU_CLOSE, "Close Window\tCTRL+F4");
	TrackPopupMenu(PopupMenu, TPM_LEFTALIGN | (LeftButton ? TPM_LEFTBUTTON : TPM_RIGHTBUTTON),
	 X, Y, 0, NGW, NULL);
	}
else
	{
	//if(PopupMenu) DestroyMenu(PopupMenu); PopupMenu = NULL;
	if(ViewSub) DestroyMenu(ViewSub); ViewSub = NULL;
	if(ShowSub) DestroyMenu(ShowSub); ShowSub = NULL;
	if(RenderSub) DestroyMenu(RenderSub); RenderSub = NULL;
	//if(OptionsSub) DestroyMenu(OptionsSub); OptionsSub = NULL;

	if(ShowTerrain) DestroyMenu(ShowTerrain); ShowTerrain = NULL;
	if(ShowLandCover) DestroyMenu(ShowLandCover); ShowLandCover = NULL;
	if(ShowWater) DestroyMenu(ShowWater); ShowWater = NULL;
	if(ShowSky) DestroyMenu(ShowSky); ShowSky = NULL;
	if(ShowLight) DestroyMenu(ShowLight); ShowLight = NULL;
	if(ShowRender) DestroyMenu(ShowRender); ShowRender = NULL;
	if(ShowOverlay) DestroyMenu(ShowOverlay); ShowOverlay = NULL;
	if(ShowGradient) DestroyMenu(ShowGradient); ShowGradient = NULL;

	if(RenderOpSet) DestroyMenu(RenderOpSet); RenderOpSet = NULL;
	if(RenderTerrain) DestroyMenu(RenderTerrain); RenderTerrain = NULL;
	if(RenderLandCover) DestroyMenu(RenderLandCover); RenderLandCover = NULL;
	if(RenderWater) DestroyMenu(RenderWater); RenderWater = NULL;
	if(RenderSky) DestroyMenu(RenderSky); RenderSky = NULL;
	if(RenderLight) DestroyMenu(RenderLight); RenderLight = NULL;
	if(RenderRender) DestroyMenu(RenderRender); RenderRender = NULL;
	} // else
return(0);
} // Fenetre::InternalDoPopup

int DrawingFenetre::DoPopup(short X, short Y, char LeftButton)
{
return(InternalDoPopup(1, NativeWin, X, Y, LeftButton));
} // DrawingFenetre::DoPopup

int GUIFenetre::DoPopup(short X, short Y, char LeftButton)
{
return(InternalDoPopup(0, NativeWin, X, Y, LeftButton));
} // GUIFenetre::DoPopup

#endif // _WIN32
