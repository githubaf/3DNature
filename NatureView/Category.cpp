
#include <windows.h>
#include <osg/PositionAttitudeTransform>

#include "Types.h"
#include "KeyDefines.h"
#include "EventDispatcher.h"
#include "Category.h"
#include "NVScene.h"


extern NVScene MasterScene;
extern NativeAnyWin NavNativeWindow;


void BuildCategoryMenu(void *MenuHandle)
{
HMENU CategoryMenu;

if(CategoryMenu = (HMENU)MenuHandle)
	{
	AppendMenu(CategoryMenu, MF_STRING | MF_DISABLED, NVW_CATEGORYMENU_IDBASE, "Categories");
	AppendMenu(CategoryMenu, MF_SEPARATOR, NULL, NULL);
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckOceanEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_OCEAN, "Ocean\t"NV_KEY_OCEANTOGGLEDESC); // 0
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckTerrainEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_TERRAIN, "Terrain\t"NV_KEY_TERRAINTOGGLEDESC); // 1
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckVecsEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_VECTORS, "Vectors\t"NV_KEY_VECTOGGLEDESC); // 2
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckFoliageEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_FOLIAGE, "Foliage\t"NV_KEY_FOLIAGETOGGLEDESC); // 3
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckObjectsEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_3DO, "3D Objects\t"NV_KEY_OBJECTTOGGLEDESC); // 4
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.SceneLOD.CheckLabelsEnabled() ? MF_CHECKED : 0), NVW_CATEGORYMENU_LABELS, "Labels\t"NV_KEY_LABELTOGGLEDESC); // 5
	AppendMenu(CategoryMenu, MF_STRING | (MasterScene.Overlay.GetOverlayOn() ? MF_CHECKED : 0), NVW_CATEGORYMENU_OVERLAY, "Overlay\t"NV_KEY_OVERLAYTOGGLEDESC); // 6
	} // if

} // BuildCategoryMenu

int HandleCategoryMenuSelection(int MenuID)
{
int SelectedCat;

SelectedCat = MenuID - (NVW_CATEGORYMENU_IDBASE + 1); // NVW_CATEGORYMENU_IDBASE + 1 = VP0
switch(SelectedCat)
	{
	case 0: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OCEAN); break;
	case 1: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_TERRAIN); break;
	case 2: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_VECTOR); break;
	case 3: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_FOLIAGE); break;
	case 4: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OBJECT); break;
	case 5: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_LABEL); break;
	case 6: EventDispatcher::DispatchEvent(EventDispatcher::NVW_EC_SCENE, EventDispatcher::NVW_ESC_SCENE_OVERLAY); break;
	default: return(-1);
	} // SelectedCat
return(SelectedCat);

} // HandleCategoryMenuSelection



int SelectCategoryViaMenu(int XCoord, int YCoord)
{
HMENU CategoryMenu;
int TrackResult;

if(CategoryMenu = CreatePopupMenu())
	{
	BuildCategoryMenu(CategoryMenu);

	if(TrackResult = TrackPopupMenu(CategoryMenu, TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTALIGN | TPM_VCENTERALIGN | TPM_RIGHTBUTTON, XCoord, YCoord, 0, NavNativeWindow, NULL))
		{
		if(TrackResult > NVW_CATEGORYMENU_IDBASE)
			{
			HandleCategoryMenuSelection(TrackResult);
			} // if
		} // if
	} // if

return(-1);
} // SelectCategoryViaMenu

