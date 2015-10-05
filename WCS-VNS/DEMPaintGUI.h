// DEMPaintGUI.h
// Header file for DEM painter
// Created from DEM Editor 04/12/02 FPW2
// Copyright 2002 3D Nature, LLC. All rights reserved.

#ifndef WCS_DEMPaintGUI_H
#define WCS_DEMPaintGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "EffectsLib.h"
#include "GraphData.h"
#include "RasterDrawWidget.h"

#define MAX_DPG_BRUSH_SIZE 255

class EffectsLib;
class Database;
class Project;
class Joe;
class DEM;
class JoeDEM;
class CoordSys;

//struct PaintBrush
//	{
//	unsigned short cols, rows;
//	unsigned char *BrushImage;
//	};

// Very important that we list the base classes in this order.
class DEMPaintGUI : public WCSModule, public GUIFenetre
	{
	enum DPG_MODE
		{
		DPG_MODE_MASK,
		DPG_MODE_PAINT,
		DPG_MODE_EYEDROP,
		DPG_MODE_FILL,
		DPG_MODE_GRADIENT,
		DPG_MODE_ERASE,
		DPG_MODE_SMOOTH,
		DPG_MODE_ROUGHEN,
		DPG_MODE_SMEAR
		}; // DPG_MODE

	enum DPG_EFFECT
		{
		DPG_EFFECT_ABS_RAISELOWER,
		DPG_EFFECT_ABS_RAISE,
		DPG_EFFECT_ABS_LOWER,
		DPG_EFFECT_REL_RAISE,
		DPG_EFFECT_REL_LOWER
		}; // DPG_EFFECT

	enum DPG_GRADMODE
		{
		DPG_GRADMODE_LINEAR,
		DPG_GRADMODE_RADIAL,
		DPG_GRADMODE_BUMP
		};

	enum DPG_MASKMODE
		{
		DPG_MASKMODE_NONE,
		DPG_MASKMODE_BOX,
		DPG_MASKMODE_MOVE,
		DPG_MASKMODE_FREEHAND,
		DPG_MASKMODE_WAND
		};

	enum DPG_MASKOPTS
		{
		DPG_MASKOPT_NONE,
		DPG_MASKOPT_ADD,
		DPG_MASKOPT_SUB
		};

	enum
		{
		DPG_ANIMPAR_FOREGROUND,
		DPG_ANIMPAR_BACKGROUND,
		DPG_NUMANIMPAR
		};

	private:
		double ForeElev, BackElev, Tolerance;
		float BrushScale;
		float FreehandVerts[1024][2];
		float MoveRefX, MoveRefY;			// mask move is relative to this point
		float Opacity, SeedVal;
		float MaxElev, MinElev;
		float rbx0, rbx1, rby0, rby1;		// rubber band points
		float boxx0, boxx1, boxy0, boxy1;	// box select points
		float lut[256];						// atan2 related slope table
		unsigned long ActiveBrush, JoeFlags;
		EffectsLib *EffectsHost;
		Database *DBHost;
		Project *ProjHost;
		Joe *Active, *PreviewJoe;
		DEM *ActiveDEM;
		JoeDEM *MyDEM;
		float *BrushWorkArea, *ModifierBaseMap, *ModifierMap, *RevertMap, *UndoMap, *WorkMap;
		static long ActivePage, ActivePanel;
		unsigned long Previewing, Solo;
		long RowOffset, ColOffset, ActiveModified, NULLValue;
		long ElevCols, ElevRows, FreehandCount;
		long RegionXMax, RegionXMin, RegionYMax, RegionYMin, SavedRegion;
		AnimDoubleTime NorthADT, SouthADT, WestADT, EastADT;
		DPG_MODE LastPaintMode, PaintMode;
		DPG_EFFECT Effect;
		DPG_GRADMODE GradMode;
		DPG_MASKMODE MaskMode;
		DPG_MASKOPTS MaskOpts;
		unsigned short BrushWidth, BrushHeight;
		unsigned char *BrushDef, *BrushOutline, *BrushRegion;
		struct RasterWidgetData Elevs, Gradient, Thumb;
		int WidSize[3][2];			// Area = [0][], Thumb = [1][], Gradient = [2][]
//		long BrushWeight;
		bool DefiningMask, MakeTable, MaskNulls, RubberBand, DefaultSettingsValid;
		unsigned char PaintRefresh; // can be tri-state now
		unsigned char *FloodMap, *MaskMap, *NullMap, *TmpMaskMap;
		CoordSys *PaintCoords;
		char OrigName[256];

	public:

		int ConstructError;
		AnimDoubleTime AnimPar[DPG_NUMANIMPAR];

		DEMPaintGUI(EffectsLib *EffectsSource, Database *DBSource, Project *ProjSource, Joe *ActiveSource);
		~DEMPaintGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);

		inline DEM *FetchDEM (void) {return(ActiveDEM);};
		inline Joe *FetchJoe (void) {return(Active);};
		inline JoeDEM *FetchJoeDEM (void) {return(MyDEM);};
		inline unsigned long ShowPreview (void) {return(Previewing);};
		inline unsigned long ShowSolo (void) {return(Solo);};

		void AddMasks(void);
		void BoxMask(void);
		void ClearMaskMap(void);
		void ComputeThumbOverlay(void);
		void ConfigureOverlays(void);
		void ComputeTerrainView(void);
		void ComputeTerrainViewDamaged(void);
		void ConfigureToolsBrushes(void);
		void ConfigureWidgets(void);
		void CreateBrushMap(unsigned long BrushNum, long Size);
		short DisableOrigDEM(char *BaseName);
		void DoGradFill(void);
		void DrawAreaOverlays(void);
		void DrawForegroundIndicator(void);
		void DrawFreehandVerts(void);
		void DrawFreehandVerts2(void);
		void DrawMaskMap(void);
		void DumpPreview(void);
		bool FillWorthy(long Col, long Row);
		Joe *GetActive(void) {return (Active);};
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleCloseWin(NativeGUIWin NW);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleKeyDown(int Key, char Alt, char Control, char Shift);
		long HandleLeftButtonDown(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleLeftButtonUp(int CtrlID, long X, long Y, char Alt, char Control, char Shift);
		long HandleMouseMove(int CtrlID, long X, long Y, char Alt, char Control, char Shift, char Left, char Middle, char Right);
		long HandleMouseWheelVert(long X, long Y, float Amount, char Alt, char Control, char Shift);
		void HandleNotifyEvent(void);
		long HandleReSized(int ReSizeType, long NewWidth, long NewHeight);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleScroll(int ScrollCode, int ScrollPos, NativeControl ScrollCtrlID, int CtrlID);
		unsigned long InitPreview(void);
		void MagicWand(long X, long Y);
		void MakeLUT(void);
		void MakeNullMap(void);
		void MoveSelect(float dx, float dy, bool ShowOnly);
		void RestoreBrushRegion(void);
		void Roughen(long X, long Y, long raw = 1);
		void SaveBrushRegion(long X, long Y);
		void SeedFill(long Col, long Row);
		void SeedPaint(long X, long Y, long raw = 1);
		void SelectPoints(double SelectLat, double SelectLon);
		void SetBrush(unsigned long BrushNum);
		void SetOptions(int ButtonID);
		void SetPaintMode(int ButtonID);
		void SetPaintMode2(DPG_MODE Mode);
		void SetSNDefaults(void);
		void Smear(long X, long Y, bool Init, long raw = 1);
		void Smooth(long X, long Y, long raw = 1);
		void SubMasks(void);
		void UpdatePreview(void);

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);

	private:

		void NewDEMFile(void);
		void DisableWidgets(void);
		void HideWidgets(void);
		void SetNewElev(int CtrlID, long Row, long Col);
		void ReloadDEM(void);
		void SaveDEM(void);
		bool Mask(long x, long y);
		void Paint(long x, long y, long raw = 1);
		void CopyFromPrefsSettings(void);
		void CopyToPrefsSettings();

	}; // class DEMPaintGUI

#endif // WCS_DEMPaintGUI_H
