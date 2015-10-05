// TextureEditGUI.h
// Header file for ...Editor
// Created from DefaultEditGUI.h on 9/11/98 by Gary R. Huber
// Copyright 1998 by Questar Productions. All rights reserved

#ifndef WCS_TEXTUREEDITGUI_H
#define WCS_TEXTUREEDITGUI_H

#include "Application.h"
#include "Fenetre.h"
#include "Texture.h"
// Material GUI
#include "PortableMaterialGUI.h"

class ImageLib;
class AnimColorTime;
class DiagnosticData;
class EffectsLib;
class TextureEditGUI;
// Material GUI
class PortableMaterialGUI;

// Material GUI
class TextureEditGUIPortableMaterialGUINotifyFunctor : public PortableMaterialGUINotifyFunctor // we like agglutination
	{
	public:
		TextureEditGUI *Host;
		virtual void HandleConfigureMaterial(void);
		virtual void HandleNewActiveGrad(GradientCritter *NewNode);
	}; // TextureEditGUIPortableMaterialGUINotifyFunctor


// Very important that we list the base classes in this order.
class TextureEditGUI : public WCSModule, public GUIFenetre
	{
	private:
		RootTexture *ActiveRoot;
		Texture *Active, *GradientActive, *ActiveParent;
		ImageLib *ImageHost;
		EffectsLib *EffectsHost;
		RasterAnimHost *TextureHost;
		Raster *RootRast, *CompChildRast, *CompRast;
		RootTexture Backup;
		AnimColorTime *DefaultColor;
		GradientCritter *ActiveGrad;
		ColorTextureThing *ActiveNode;
		long *TextureList;
		TextureSampleData *SampleData[3];
		// Material GUI
		PortableMaterialGUI *TexMatGUI;
		TextureEditGUIPortableMaterialGUINotifyFunctor PopDropMaterialNotifier;
		long NumListTextures;
		char TNailDrawn[3], VertexUVWAvailable, VertexCPVAvailable, PostProc;
		unsigned char ParamDropItem[WCS_TEXTURE_TERRAINPARAM_MAXPARAMS], BackgroundInstalled;
		unsigned char ReceivingDiagnostics;
		double LatEvent[2], LonEvent[2];

		void ConfigureXYZWidgets(short DisableAll = 0);
		void ConfigureImageWidgets(short DisableAll = 0);
		void SelectColorPanel(short PanelID);
		void SelectBitmapPanel(short PanelID);
		void SelectPosPanel(short PanelID);
		void SetPanels(void);
		void SyncWidgets(void);
		void DisableWidgets(void);
		void HideWidgets(void);
		void BuildComboList(bool ShowNonRemaps, bool ShowRemaps);
		long FindInComboList(long TexNum);
		long BuildList(Texture *Source = NULL, short Indent = 0, short Enabled = 1, char *TexName = NULL);
		void BuildListEntry(Texture *Host, Texture *Source, short TexNum, short Indent, short Enabled, char *Name);
		void DoKeep(void);
		void Cancel(void);
		void DoTextureType(void);
		short SearchAndReplace(Texture *CurTex, Texture *NewTex);
		void DoTextureEnabled(void);
		void EnableInputParamTexture(unsigned short WidID);
		void EnableTexture(unsigned short WidID, short TexNum);
		void EnableGradientTexture(unsigned short WidID);
		void DoTextureList(void);
		Texture *NewTexture(RasterAnimHost *Parent, short TexType, short TexNum, short CopyActive, int TestActiveForColor,
			AnimColorTime *ColorCopy);
		void DoEditImage(void);
		void DoNewImage(void);
		void AdjustTexturePosition(long Direction);
		void AddNewTexture(void);
		void RemoveTexture(void);
		void CopyTexture(void);
		void PasteTexture(void);
		void UpdateThumbnails(short UpdateRoot = 1);
		void InitNoise(int ParamNum);
		void SetViewDropContents(short Ecosys);
		void SetTerrainDropContents(short Displace);
		long FindTerrainDropItem(unsigned char TerrainItem);
		void DoViewDirection(void);
		void DoSelectParam(void);
		void AddColorNode(void);
		void RemoveColorNode(void);
		void BlendStyle(void);
		int ValidateGeoRefImage(void);
		void ClearItemList(void);
		void BuildItemList(void);
		void BuildItemListEntry(char *ListName, RasterAnimHost *Me);
		void BuildItemTypeList(void);
		void BuildItemTypeListEntry(char *ListName, unsigned char Me);
		void FillClassDrop(void);
		void FillClassTypeDrop(void);
		void AddItem(void);
		void AddItemType(void);
		void RemoveItem(void);
		void RemoveItemType(void);
		void EditItem(void);
		void FillCoordSpaceDrop(void);
		void SetCoordSpaceDrop(void);
		void SelectCoordSpace(void);
		void FillVertexMapCombo(void);
		void SetVertexMapDrop(void);
		void SelectVertexMap(void);

	public:

		int ConstructError;

		TextureEditGUI(ImageLib *ImageSource, RasterAnimHost *HostSource, RootTexture *RootSource, AnimColorTime *DefColorSource);
		~TextureEditGUI();

		NativeGUIWin Construct(void);
		NativeGUIWin Open(Project *Moi);
		
		long HandleCloseWin(NativeGUIWin NW);
		long HandleButtonClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleButtonDoubleClick(NativeControl Handle, NativeGUIWin NW, int ButtonID);
		long HandleCBChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListSel(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDoubleClick(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleListDelItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListCopyItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleListPasteItem(NativeControl Handle, NativeGUIWin NW, int CtrlID, void *ItemData);
		long HandleFIChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSCChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandleSRChange(NativeControl Handle, NativeGUIWin NW, int CtrlID);
		long HandlePageChange(NativeControl Handle, NativeGUIWin NW, int CtrlID, long NewPageID);
		long HandleBackgroundCrunch(int Siblings);
		void HandleNotifyEvent(void);
		void ConfigureWidgets(void);
		RootTexture *GetRootTexture(void) {return (ActiveRoot);};

		virtual bool InquireWindowCapabilities(FenetreWindowCapabilities AskAbout);
		// Material GUI
		void ConfigureColors(void); // so we can call from TextureEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void SetNewActiveGrad(GradientCritter *NewNode) {ActiveGrad = NewNode;}; // so we can call from TextureEditGUIPortableMaterialGUINotifyFunctor without being a friend
		void ShowMaterialPopDrop(bool ShowState);
		virtual void HideSubordinateWindows(void) {ShowMaterialPopDrop(false);}; // hide popdrop when necessary

	}; // class TextureEditGUI

#endif // WCS_TEXTUREEDITGUI_H
