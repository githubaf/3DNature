// SceneViewGUI.h
// Header for Scene at a Glance
// Built from TrackviwGUI.h on 5/5/9 by Gary R. Huber
// Copyright 1999 by Questar Productions. All rights reserved.

#ifndef WCS_SCENEVIEWGUI_H
#define WCS_SCENEVIEWGUI_H

//class EffectsLib;
//class Database;
//class ImageLib;
//class Project;
class RasterAnimHost;
class RasterAnimHostProperties;
struct _IMAGELIST;
struct _TREEITEM;

#include "Application.h"
#include "Fenetre.h"
#include "RasterAnimHost.h"
#include "EffectsLib.h"

enum WCS_SCENEVIEWGUI_TAB
	{
	WCS_SCENEVIEWGUI_TAB_SAG,
	WCS_SCENEVIEWGUI_TAB_DIAG,
	WCS_SCENEVIEWGUI_TAB_POINT,
	WCS_SCENEVIEWGUI_TAB_MAX
	}; // WCS_SCENEVIEWGUI_TAB


// Very important that we list the base classes in this order.
class SceneViewGUI : public WCSModule, public GUIFenetre
	{
	private:
		Database *DBHost;
		EffectsLib *EffectsHost;
		Project *ProjHost;
		ImageLib *ImageHost;
		unsigned long Dragging, Editing;
		int SVGStockX, ExpandedX, AutoResize, Frozen, SystemResize;
		int Image[WCS_RAHOST_NUMICONTYPES + 1];
		RasterAnimHost *DragHost, *HitHost;
		RasterAnimHostProperties *DragProp;
		void *TVListItem[2][WCS_MAXIMPLEMENTED_EFFECTS + 5];
		char TVListItemExpanded[2][WCS_MAXIMPLEMENTED_EFFECTS + 5];
		unsigned char *ModeList;

		#ifdef _WIN32
		RECT TVDragRect;
		struct _IMAGELIST *TVImageList, *TVDragList;
		struct _TREEITEM *HitItem, *DragItem;
		HCURSOR StdCursor, NoDropCursor;
		HWND HitList, DragList;
		#endif // _WIN32

	public:

		int ConstructError;

		SceneViewGUI(Database *DBSource, EffectsLib *EffectsSource, Project *ProjSource, ImageLib *ImageSource);
		~SceneViewGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleTreeMenuSelect(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived);
		long HandleTreeMenuCreate(NativeControl Handle, NativeGUIWin NW, int CtrlID, unsigned long TreeItem, RasterAnimHost *RAH, char *ActionText, int Derived);
		void HandleNotifyEvent(void);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
		long HandleMouseMove(long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandleSplitMove(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandleSplitToggle(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		//long HandleResizer(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		//long HandleResizerToggle(NativeControl Handle, NativeGUIWin NW, int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		void ConfigureWidgets(void);
		void SetTaskMode(int ButtonID, int Selected);
		void SetSAGBottomHtPct(int Percent);
		RasterAnimHost *GetSelectedObjectAndCategory(long &Category);
		int TaskModeFilter(long Group);
		void ActivateActiveItem(void);
		void HandleMatrixResize(void);
		long IdentifyCategory(unsigned long HitItem, long ListNum);
		int AddLowerPanelTab(char *TabName, WCS_SCENEVIEWGUI_TAB Slot, NativeGUIWin TabContents);
		int SwitchToTab(WCS_SCENEVIEWGUI_TAB WhichTab); 

	private:
		void Freeze(void)	{Frozen = 1;};
		void UnFreeze(void)	{Frozen = 0; ConfigureWidgets();};
		void BuildList(short FilterList, short ListID, short ListNum);
		void *InsertRAHost(RasterAnimHost *Insert, RasterAnimHostProperties *Prop, short ListID, short ListNum, 
			void *Parent, void *TVInsert, short FilterList);
		void ExpandListItem(short ListID, short ListNum, long Something, short FilterList);
		void ListItemExpanded(short ListID, short ListNum, long Something);
		void BeginDrag(short ListID, long Something);
		void DoDragItem(long X, long Y);
		void EndDrag(void);
		void ChangeSelection(short ListID, long Something);
		int ParseNotifyEvent(unsigned char Class, unsigned char Subclass, unsigned char Item, unsigned char Comp,
			void *Data, int &Action, int &Group);
		void ProcessNotifyEvent(int Action, int Group, void *Data);
		int IdentifyListRootItem(unsigned char Class, void *&Item1, void *&Item2);
		int BeginLabelEdit(short ListID, long Something);
		void EndLabelEdit(short ListID, long Something);
		void DoWindowExpand(NativeGUIWin NW);
		void EditObject(short WidID, RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);
		void EnableObject(RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);
		void DisableObject(RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);
		void EmbedObject(RasterAnimHost *EditTarget, unsigned long TreeItem);
		RasterAnimHost *AddObject(short WidID, long Item, int OpenGallery, RasterAnimHost *EditTarget = NULL);
		void RemoveObject(short WidgetID, RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);
		void SetModeListPtr(void);
		int HaltImperialistExpansionism(RasterAnimHost *Insert, void *TreeItem, short ListID);
		void UpdateRAHost(RasterAnimHost *Insert, RasterAnimHostProperties *Prop, short ListID, short ListNum, void *Parent, void *InsertStruct, short FilterList);
		void BuildListSection(int Group, void *Data);
		void UpdateListSection(int Group, void *Data);
		void BuildListObject(int Group, void *Data);
		void UpdateListHeader(int Group, void *Data);
		void UpdateListObject(int Group, void *Data);
		void FindAndUpdateAllOtherInstances(int Group, void *Data);
		void RecurseFindAndUpdate(void *Data, RasterAnimHostProperties *Prop, short ListID, short ListNum, void *hItem, void *InsertStruct);
		void CopyObject(short WidgetID, RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);
		void PasteObject(short WidgetID, RasterAnimHost *EditTarget = NULL, unsigned long TreeItem = NULL);

	}; // class SceneViewGUI

enum
	{
	WCS_SCENEVIEW_LISTACTION_BUILDLIST = 1,
	WCS_SCENEVIEW_LISTACTION_BUILDSECTION,
	WCS_SCENEVIEW_LISTACTION_UPDATESECTION,
	WCS_SCENEVIEW_LISTACTION_BUILDOBJECT,
	WCS_SCENEVIEW_LISTACTION_UPDATEOBJPLUSHDR,
	WCS_SCENEVIEW_LISTACTION_UPDATEOBJECT
	}; // list actions

#endif // WCS_SCENEVIEWGUI_H
