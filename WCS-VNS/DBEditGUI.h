// DBEditGUI.h
// Header file for Database Editor
// Created from ColorEditGUI.h on 2/27/96 by CXH & GRH
// Copyright 1996 Questar Productions

#ifndef WCS_DBEDITGUI_H
#define WCS_DBEDITGUI_H

class Database;
class GeneralEffect;
class JoeList;
class JoeApproveHook;
class LayerEntry;
class SearchQuery;

#include "Application.h"
#include "Fenetre.h"
#include "Joe.h"
#include "GraphData.h"
#include <vector>

// Stuff for DoSomethingToEach
class DoPatch
	{
	public:
	char *One, *Two;
	LayerEntry *DoLayer;

	void (*DoThreeShort)(Joe *, short, short, short);
	void (*DoOneShort)(Joe *, short);
	void (*DoProjTwoChar)(Joe *, Project *, const char *, const char *);
	void (*DoOneLayer)(Joe *, LayerEntry *);
	DoPatch() {
	 DoThreeShort = NULL;
	 DoOneShort = NULL;
	 DoProjTwoChar = NULL;
	 DoOneLayer = NULL;};
	}; // DoPatch

enum
	{
	WCS_ENUM_DBEDITGUI_FILTER_MODE_NONE = 0,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_VEC,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_DEM,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_LAYER,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_QUERY,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_ENABLED,
	WCS_ENUM_DBEDITGUI_FILTER_MODE_MAX
	}; // ENUM_DBEDITGUI_FILTER_MODE_


enum ENUM_DBEDITGUI_GRID_COLUMN
	{
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_NAME = 0,
	//WCS_ENUM_DBEDITGUI_GRID_COLUMN_LABEL,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_ENABLED,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_VIEW,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_RENDER,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLOR,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_CLASS,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_STYLE,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEIGHT,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXFRD,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_AREA,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_POINTS,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_LENGTH,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_ROWS,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_COLS,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAXEL,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_MINEL,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_NORTH,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_SOUTH,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_EAST,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_WEST,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDNS,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_GRIDEW,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_MAX,
	WCS_ENUM_DBEDITGUI_GRID_COLUMN_NONE = 0xffff
	}; // enum ENUM_DBEDITGUI_GRID_COLUMN

#define WCS_DBEDITGUI_MAX_GRID_COLUMNS	100

// Very important that we list the base classes in this order.
class DBEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		EffectsLib *HostLib;
		long NoOfObjects;
		char LayerName[80];
		unsigned char ImBusy;
		int Frozen, DEMsChanged, ControlPtsChanged, VectorsChanged;
		// Database Editor Toolbar
		NativeControl hwndTB;
		HIMAGELIST toolBarIL;
		int ButtonCount;
		bool GridWidgetColumnsPrepped;
		
		std::vector<Joe *> GridWidgetCache;
		std::vector<GeneralEffect *> ComponentColumns;
		std::vector<LayerEntry *> LayerColumns;
		std::vector<LayerEntry *> AttribColumns;
		std::vector<int> ColumnWidths;

		int DoSomethingToEach(DoPatch *MyDP, short ArgA, short ArgB, short ArgC);
		long GetActiveObject(void);
		//void DoToggleEnable(void); // no longer called, leaving in for the future, Just In Case
		void DoLayerSelect(int Qualifier);
		void DoLayerOn(void);
		void DoLayerOff(void);
		void DoLayerShowAll(void);
		void DoAttribValue(void);
		void DoAddAttr(void);
		void DoLinkAttr(void);
		void DoSearch(char ClearFirst = 0);
		void DoRemove(void);
		void DoSave(void);
		void DoLoad(int ClearFirst = 1); // ClearFirst=0 indicates operation is Append
		void DoAppend(void);
		void DoActivate(void);
		void SetActiveObject(long OBN);
		void ConfigUIForActiveObject(long OBN);
		void SetSelected(long OBN, short Selected);
		void ClearSelected(void);

// Database Direct Access Functions for DBEditGUI.cpp


		void SetLayerName(void);
		void DoRemoveLayer(void);
		void DoRemoveLink(void);
		const char *GetName(Joe *Me) {return(Me->Name());};
		//const char *GetLayerName(Joe *Me, short Layer);
		short GetEnabled(Joe *Me) {return((short)Me->TestFlags(WCS_JOEFLAG_ACTIVATED));};
		short GetDrawEnabled(Joe *Me) {return((short)Me->TestDrawFlags());};
		short GetRenderEnabled(Joe *Me) {return((short)Me->TestRenderFlags());};
		short GetMaxFract(Joe *Me);
		double GetNorth(Joe *Me) {return(Me->GetNorth());};
		double GetSouth(Joe *Me) {return(Me->GetSouth());};
		double GetWest(Joe *Me) {return(Me->GetWest());};
		double GetEast(Joe *Me) {return(Me->GetEast());};
		double GetVecLength(Joe *Me, int ConsiderElev) {return(Me->GetVecLength(ConsiderElev));};
		unsigned long GetNumPoints(Joe *Me) {return(Me->NumPoints());};
		short GetClass(Joe *Me);
		long BuildList();
		long ValidateCachedELEVData(void);
		void BuildEffectList(Joe *Active, long NumListItems);
		long AddEffectToList(JoeAttribute *Me);
		void ReDrawList();
		void OpenEditor(void);
		void DoFilterPopup(void);
		void ConformVector(void);
		void ConformTopo(void);
		void ConformName(void);
		void ConformLabel(void);
		void ConformLayer(void);
		void ConformAttribute(void);
		unsigned long ConformOneObject(Joe *BillyJoe);
		unsigned long ConformOneObjectName(Joe *BillyJoe);
		unsigned long ConformOneObjectLabel(Joe *BillyJoe);
		unsigned long ConformOneObjectLayer(Joe *BillyJoe);
		unsigned long ConformOneObjectAttribute(Joe *BillyJoe, LayerEntry *MyLayer);
		void SplitVector(void);
		void JoinVectors(void);

		void ModifyEffect(void);
		void RemoveEffect(void);

		int PrepGridWidgetForJoeUsage(NativeControl GridWidget);
		int AddColumn(NativeControl GridWidget, int ColumnIndex, const char *ColumnText, int ColumnWidth);
		int AddComponentColumn(NativeControl GridWidget, const char *ColumnText, GeneralEffect *Component);
		int AddLayerColumn(NativeControl GridWidget, const char *ColumnText, LayerEntry *Layer);
		int AddAttributeColumn(NativeControl GridWidget, const char *ColumnText, LayerEntry *Layer);
		void ShowPropertiesColumns(NativeControl GridWidget, bool ShowState);
		void ShowExtentColumns(NativeControl GridWidget, bool ShowState);
		void ShowComponentColumns(NativeControl GridWidget, bool ShowState);
		void ShowLayerColumns(NativeControl GridWidget, bool ShowState);
		void ShowAttributeColumns(NativeControl GridWidget, bool ShowState);
		// Database Editor Toolbar
		NativeControl CreateTopToolbar(HWND hwndParent, HINSTANCE g_hinst);
		bool AddToolbarImageIcon(char *HelpCapt, WORD IconID, unsigned long int ImageNum, bool Tog, bool StartHidden = false);
		void SetToolbarButtonDisabled(WORD IconID, bool NewDisabledState);
		
		
	public:
		Database *Host;
		Project *ProjHost;
		int ConstructError;
		static long ActivePage;
		static bool ShowPropertiesPanel;
		#ifdef WCS_BUILD_VNS
		SearchQuery *QueryFilter; // not static
		#endif // WCS_BUILD_VNS
		AnimColorTime VectorColor; // used from GridWidget
		int ActiveColorEditItem;
		bool IndividualEditInProgress;

		DBEditGUI(Database *DB, Project *ProjSource, EffectsLib *LibSource);
		~DBEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleEvent(void); // for resize calculations
		void HandleNotifyEvent(void);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);

		void ConfigureWidgets(void);
		void ConfigurePropertiesPanel(void);
		void ConfigureToolbarButtons(void);
		void ConfigLayerSection(Joe *Active, long NumListItems);
		void ChangeSelection(Joe *NewSel, char NewState);
		void SetRegionSel(int NewSelState, int Inclusive, double NewNorth, double NewSouth, double NewEast, double NewWest);
		void ApplyEffectToSelected(GeneralEffect *Effect);
		void SelectByJoeList(JoeList *List);
		void DeselectAll(void);
		int MatchSelectedCoordSys(CoordSys *TestCoords, int DEMOnly);
		int GetSelectedBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly);
		int GetSelectedNativeBounds(double &NewNorth, double &NewSouth, double &NewWest, double &NewEast, int DEMOnly);
		Joe *GetNextSelected(long &LastOBN);
		bool CreateSelectedLayer(char *name, bool DEMonly);

		int ConfirmActiveObject(Joe *ConfirmMe);
		int ConfirmSelectedObject(Joe *ConfirmMe);
		int ConfirmEnabledObject(Joe *ConfirmMe);
		int ConfirmRenderEnabledObject(Joe *ConfirmMe);
		void UpdateTitle(void);
		void UpdateWidgets(void);

		void DoLayerSelect(LayerEntry *LayerToSelect, int Qualifier);

		void Freeze(void)	{Frozen = 1;};
		void UnFreeze(void)	{Frozen = 0;};

		void HardFreeze(void);
		void HardUnFreeze(void);
		bool GetFrozen(void) const {return(Frozen == 1);};

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);
		virtual char *InquireWindowCapabilityCaption(FenetreWindowCapabilities AskAbout);

		LayerEntry *GetLayerColumn(int Index) const {return(LayerColumns[Index]);};
		LayerEntry *GetAttribColumn(int Index) const {return(AttribColumns[Index]);};
		GeneralEffect *GetComponentColumn(int Index) const {return(ComponentColumns[Index]);};
		int GetLayerColumnSize(void) const {return(LayerColumns.size());};
		int GetAttribColumnSize(void) const {return(AttribColumns.size());};
		int GetComponentColumnSize(void) const {return(ComponentColumns.size());};
		bool IsColumnALayer(int ColumnNumber);
		bool IsColumnAnAttribute(int ColumnNumber);
		int GetAttribNumFromColumn(int ColumnNumber);
		int GetLayerNumFromColumn(int ColumnNumber);
		

		// called from GridWidget
		void DoEnable(bool NewState);
		void DoDrawEnable(bool NewState);
		void DoRenderEnable(bool NewState);
		const char *ParseEffectNameClass(JoeAttribute *Me, GeneralEffect *Effect, bool DecorateWithPrefix);
		void DoRename(const char *NewName, const char *NewLabel);
		void DoObjectClass(short NewClass);
		void DoLineStyle(short NewStyle);
		void DoMaxFractal(int NewMaxFract);
		void DoLineWeight(int NewLineWeight);
		void UpdateVecColor(unsigned char Red, unsigned char Green, unsigned char Blue);
		void ChangeAttribValue(int Item, int Column, const char *NewValue);
		void BuildLayerList(Joe *Active, long NumListItems);
		void DoAddLayer(LayerEntry *LayerToAdd);
		void DoRemoveLayer(LayerEntry *LayerToRemove);
		Joe *GetJoeFromOBN(unsigned long OBN);
		long GetOBNFromJoe(Joe *FindMe);
		short GetSelected(long OBN);

	}; // class DBEditGUI

void SetMaxFract(Joe *Me, short MaxFract);
void SetEnabled(Joe *Me, short Status);
void SetDrawEnabled(Joe *Me, short Status);
void SetRenderEnabled(Joe *Me, short Status);
void SetRGBPen(Joe *Me, short Red, short Green, short Blue);
void SetLineStyle(Joe *Me, short Style);
void SetLineWeight(Joe *Me, short Weight);
void SetClass(Joe *Me, short Class);
void ChangeNames(Joe *Me, Project *TheProj, const char *O, const char *T);
void LayerAdd(Joe *Me, LayerEntry *AddLayer);
void LayerRemove(Joe *Me, LayerEntry *RemoveLayer);

enum
	{
	WCS_DBEXPORT_FORMAT_DXF,
	WCS_DBEXPORT_FORMAT_SHAPE
	}; // db export formats

enum
	{
	WCS_DBEXPORT_DEMSAS_POINTS,
	WCS_DBEXPORT_DEMSAS_POLYLINE,
	WCS_DBEXPORT_DEMSAS_POLYGONMESH,
	WCS_DBEXPORT_DEMSAS_POLYFACEMESH,
	WCS_DBEXPORT_DEMSAS_3DFACES
	}; // db export dems as

enum
	{
	WCS_DBEXPORT_OBJECTS_ACTIVE,
	WCS_DBEXPORT_OBJECTS_SELECTED,
	WCS_DBEXPORT_OBJECTS_ENABLED,
	WCS_DBEXPORT_OBJECTS_RENDERENABLED,
	WCS_DBEXPORT_OBJECTS_ALL
	}; // db export objects

enum
	{
	WCS_DBEXPORT_XYUNITS_METERS,
	WCS_DBEXPORT_XYUNITS_FEET,
	WCS_DBEXPORT_XYUNITS_USFEET,
	WCS_DBEXPORT_XYUNITS_INCH,
	WCS_DBEXPORT_XYUNITS_DECI,
	WCS_DBEXPORT_XYUNITS_CENTI,
	WCS_DBEXPORT_XYUNITS_MILLI
	}; // db export XY units

enum
	{
	WCS_DBEXPORT_ZUNITS_METERS,
	WCS_DBEXPORT_ZUNITS_FEET,
	WCS_DBEXPORT_ZUNITS_USFEET,
	WCS_DBEXPORT_ZUNITS_INCH,
	WCS_DBEXPORT_ZUNITS_DECI,
	WCS_DBEXPORT_ZUNITS_CENTI,
	WCS_DBEXPORT_ZUNITS_MILLI
	}; // db export Z units

#define WCS_DBEXPORT_CLASS_LANDSCAPES	(1 << 0)
#define WCS_DBEXPORT_CLASS_SURFACES		(1 << 1)
#define WCS_DBEXPORT_CLASS_VECTORS		(1 << 2)
#define WCS_DBEXPORT_CLASS_CONTROLPTS	(1 << 3)

class DBExportGUI : public WCSModule, public GUIFenetre
	{
	public:
		Project *ProjHost;
		Database *DBHost;
		EffectsLib *EffectsHost;
		CoordSys *ExportCoords;
		int ConstructError;
		long ExportLandscapes, ExportSurfaces, ExportVectors, ExportControlPts, ExportFormat;
		long ExportXYUnits, ExportZUnits;
		double ExportXYScale, ExportZScale;

		DBExportGUI(Project *ProjSource, Database *DBSource, EffectsLib *EffectsSource, DBEditGUI *DBEditorSource);
		~DBExportGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		void DisableWidgets(void);

		void DoClassCheck(void);
		void DoFormat(void);
		void DoDEMsAs(void);
		void SelectNewCoords(void);
		void DoExport(void);

	}; // DBExportGUI

int DBEditJoeApprove(JoeApproveHook *JAH);

struct DB3_Table_Field_Descriptor
	{
	struct	DB3_Table_Field_Descriptor *Next;
	char	Name[11];
	char	FieldType;
	ULONG	FieldAddress;
	UBYTE	FieldLength;
	UBYTE	FieldCount;
	char	reserved1[2];
	char	WorkID;
	char	reserved2[2];
	char	SetFields;
	char	reserved3[8];
	};

#endif // WCS_DBEDITGUI_H
